#include <QtTest/QtTest>

#include "PDFDoc.h"

#include <poppler-qt4.h>

class TestOptionalContent: public QObject
{
    Q_OBJECT
private slots:
    void checkVisPolicy();
    void checkNestedLayers();
    void checkNoOptionalContent();
    void checkIsVisible();
    void checkVisibilitySetting();
    void checkRadioButtons();
};

void TestOptionalContent::checkVisPolicy()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/vis_policy_test.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;
    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "A" ) );
    index = optContent->index( 0, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), true );
    index = optContent->index( 1, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "B" ) );
    index = optContent->index( 1, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), true );

    delete doc;
}

void TestOptionalContent::checkNestedLayers()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/NestedLayers.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;

    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Black Text and Green Snow" ) );
    index = optContent->index( 0, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), false );

    index = optContent->index( 1, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Mountains and Image" ) );
    index = optContent->index( 1, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), true );

    // This is a sub-item of "Mountains and Image"
    QModelIndex subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Image" ) );
    subindex = optContent->index( 0, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), true );

    index = optContent->index( 2, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Starburst" ) );
    index = optContent->index( 2, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), true );

    index = optContent->index( 3, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Watermark" ) );
    index = optContent->index( 3, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toBool(), false );

    delete doc;
}

void TestOptionalContent::checkNoOptionalContent()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/orientation.pdf");
    QVERIFY( doc );

    QCOMPARE( doc->hasOptionalContent(), false );

    delete doc;
}

void TestOptionalContent::checkIsVisible()
{
    GooString *fileName = new GooString("../../../test/unittestcases/vis_policy_test.pdf"); 
    PDFDoc *doc = new PDFDoc( fileName );
    QVERIFY( doc );

    OCGs *ocgs = doc->getOptContentConfig();
    QVERIFY( ocgs );

    XRef *xref = doc->getXRef();

    Object obj;

    // In this test, both Ref(21,0) and Ref(2,0) are set to On

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QVERIFY( ocgs->optContentIsVisible( &obj ) );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QVERIFY( ocgs->optContentIsVisible( &obj ) );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();


    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    delete doc;
}

void TestOptionalContent::checkVisibilitySetting()
{
    GooString *fileName = new GooString("../../../test/unittestcases/vis_policy_test.pdf"); 
    PDFDoc *doc = new PDFDoc( fileName );
    QVERIFY( doc );

    OCGs *ocgs = doc->getOptContentConfig();
    QVERIFY( ocgs );

    XRef *xref = doc->getXRef();

    Object obj;

    // In this test, both Ref(21,0) and Ref(28,0) start On,
    // based on the file settings
    Object ref21obj;
    ref21obj.initRef( 21, 0 );
    Ref ref21 = ref21obj.getRef();
    OptionalContentGroup *ocgA = ocgs->findOcgByRef( ref21 );
    QVERIFY( ocgA );

    QVERIFY( (ocgA->name()->cmp("A")) == 0 );
    QCOMPARE( ocgA->state(), OptionalContentGroup::On );

    Object ref28obj;
    ref28obj.initRef( 28, 0 );
    Ref ref28 = ref28obj.getRef();
    OptionalContentGroup *ocgB = ocgs->findOcgByRef( ref28 );
    QVERIFY( ocgB );

    QVERIFY( (ocgB->name()->cmp("B")) == 0 );
    QCOMPARE( ocgB->state(), OptionalContentGroup::On );

    // turn one Off
    ocgA->setState( OptionalContentGroup::Off );

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();


    // Turn the other one off as well (i.e. both are Off)
    ocgB->setState(OptionalContentGroup::Off);

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();


    // Turn the first one on again (21 is On, 28 is Off)
    ocgA->setState(OptionalContentGroup::On);

    // AnyOn, one element array:
    // 22 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOn>>endobj
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // Same again, looking for any leaks or dubious free()'s
    xref->fetch( 22, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, one element array:
    // 29 0 obj<</Type/OCMD/OCGs[21 0 R]/P/AnyOff>>endobj
    xref->fetch( 29, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOn, one element array:
    // 36 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOn>>endobj
    xref->fetch( 36, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, one element array:
    // 43 0 obj<</Type/OCMD/OCGs[28 0 R]/P/AllOff>>endobj
    xref->fetch( 43, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOn, multi-element array:
    // 50 0 obj<</Type/OCMD/OCGs[21 0 R 28 0 R]/P/AnyOn>>endobj
    xref->fetch( 50, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AnyOff, multi-element array:
    // 57 0 obj<</Type/OCMD/P/AnyOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 57, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), true );
    obj.free();

    // AllOn, multi-element array:
    // 64 0 obj<</Type/OCMD/P/AllOn/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 64, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    // AllOff, multi-element array:
    // 71 0 obj<</Type/OCMD/P/AllOff/OCGs[21 0 R 28 0 R]>>endobj
    xref->fetch( 71, 0, &obj );
    QVERIFY( obj.isDict() );
    QCOMPARE( ocgs->optContentIsVisible( &obj ), false );
    obj.free();

    delete doc;
}

void TestOptionalContent::checkRadioButtons()
{
    Poppler::Document *doc;
    doc = Poppler::Document::load("../../../test/unittestcases/ClarityOCGs.pdf");
    QVERIFY( doc );

    QVERIFY( doc->hasOptionalContent() );

    Poppler::OptContentModel *optContent = doc->optionalContentModel();
    QModelIndex index;

    index = optContent->index( 0, 0, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ).toString(), QString( "Languages" ) );
    index = optContent->index( 0, 1, QModelIndex() );
    QCOMPARE( optContent->data( index, Qt::DisplayRole ), QVariant() );

    // These are sub-items of the "Languages" label
    QModelIndex subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    subindex = optContent->index( 0, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), true );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    subindex = optContent->index( 1, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    subindex = optContent->index( 2, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    // RBGroup of languages, so turning on Japanese should turn off English
    bool result = optContent->setData( subindex, QVariant( true ) );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    subindex = optContent->index( 0, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    subindex = optContent->index( 2, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), true );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    subindex = optContent->index( 1, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    // and turning on French should turn off Japanese
    result = optContent->setData( subindex, QVariant( true ) );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    subindex = optContent->index( 0, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    subindex = optContent->index( 2, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    subindex = optContent->index( 1, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), true );


    // and turning off French should leave them all off
    result = optContent->setData( subindex, QVariant( false ) );

    subindex = optContent->index( 0, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "English" ) );
    subindex = optContent->index( 0, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 2, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "Japanese" ) );
    subindex = optContent->index( 2, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    subindex = optContent->index( 1, 0, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toString(), QString( "French" ) );
    subindex = optContent->index( 1, 1, index );
    QCOMPARE( optContent->data( subindex, Qt::DisplayRole ).toBool(), false );

    delete doc;
}

QTEST_MAIN(TestOptionalContent)

#include "check_optcontent.moc"

