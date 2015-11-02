/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/libcall.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <usr/conver.hxx>
#include <usr/uno.hxx>
#include <usr/refl.hxx>
#include <stardiv/one/frame/xcollect.hxx>
#include <stardiv/one/text/offfield.hxx>
#include <stardiv/one/offmisc.hxx>
#include <stardiv/one/sheet/offtable.hxx>
#include <stardiv/one/text/offtext.hxx>
#include <stardiv/one/offstyle.hxx>
#include <stardiv/one/offview.hxx>
#include <stardiv/uno/repos/serinfo.hxx>
#include <stardiv/one/sheet/sctypes.hxx>
#include <stardiv/one/sheet/scmodel.hxx>
#include <stardiv/one/sheet/sccells.hxx>
#include <stardiv/one/sheet/sctables.hxx>
#include <stardiv/one/sheet/sctable.hxx>
#include <stardiv/one/sheet/sccell.hxx>
#include <stardiv/one/sheet/scpostit.hxx>
#include <stardiv/one/sheet/scview.hxx>
#include <stardiv/one/sheet/scdata.hxx>
#include <stardiv/one/sheet/scattr.hxx>

//! this needs be in some IDL file as a constant !!!!
#define TEXTCONTROLCHAR_PARAGRAPH_BREAK         0

class MyFixedText : public FixedText
{
protected:
    void        RequestHelp( const HelpEvent& rHEvt );
public:
    explicit    MyFixedText(vcl::Window* pParent) : FixedText(pParent) {}
};

class MyWindow : public vcl::Window
{
private:
    NumericField    aCountField;
    PushButton      aCountButton;
    MyFixedText     aTimeText;
    NumericField    aColField;
    NumericField    aRowField;
    NumericField    aPosField;
    NumericField    aLenField;
    Edit            aTextEdit;
    PushButton      aTextButton;
    PushButton      aBlaButton;
    PushButton      aTabButton;
    PushButton      aViewButton;

public:
    explicit MyWindow( vcl::Window *pParent );

        DECL_LINK_TYPED(CountHdl, PushButton*, void);
        DECL_LINK_TYPED(TextHdl, PushButton*, void);
        DECL_LINK_TYPED(BlaHdl, PushButton*, void);
        DECL_LINK_TYPED(TabHdl, PushButton*, void);
        DECL_LINK_TYPED(ViewHdl, PushButton*, void);
};

class ScTestListener : public XSelectionChangeListener, public UsrObject
{
private:
    FixedText*  pFixedText;

public:
    expliciti               ScTestListener(FixedText* pF);
    virtual                 ~ScTestListener();

                            SMART_UNO_DECLARATION( ScTestListener, UsrObject );

    virtual XInterface *    queryInterface( UsrUik );
    virtual XIdlClassRef    getIdlClass();

    virtual void            disposing(const EventObject& Source);

                            // XSelectionChangeListener
    virtual void            selectionChanged(const EventObject& aEvent);
};

static long nBla = 0;

static XCellRef xGlobalCell;

ScTestListener::ScTestListener(FixedText* pF) :
    pFixedText( pF )
{
}

ScTestListener::~ScTestListener()
{
}

XInterface* ScTestListener::queryInterface( UsrUik aUIK )
{
    if ( aUIK == XSelectionChangeListener::getSmartUik() )
        return (XSelectionChangeListener*) this;

    return UsrObject::queryInterface( aUIK );
}

XIdlClassRef ScTestListener::getIdlClass()
{
    static XIdlClassRef xClass = createStandardClass( L"ScTestListener",
        UsrObject::getUsrObjectIdlClass(),
        1, XSelectionChangeListener_getReflection() );
    return xClass;
}

void ScTestListener::disposing(const EventObject& Source)
{
}

// XSelectionChangeListener

void ScTestListener::selectionChanged(const EventObject& aEvent)
{
    static sal_uInt16 nBla = 0;
    pFixedText->SetText(++nBla);

    XInterfaceRef xInt = aEvent.Source;
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XCellCollectionRef xCells = (XCellCollection*)
                xSelInt->queryInterface(XCellCollection::getSmartUik());
    if (!xCells) return;

    String aStr = OUStringToString( xCells->getAddress(), CHARSET_SYSTEM );
    pFixedText->SetText(aStr);
}

extern "C" vcl::Window* SAL_CALL CreateWindow( vcl::Window *pParent, const String& rParam )
{
    MyWindow *pWin = new MyWindow( pParent );
    return pWin;
}

void MyFixedText::RequestHelp( const HelpEvent& rHEvt )
{
    String  aTxtStr=GetText();
    Size    aTxtSize=GetTextSize(aTxtStr);
    Point   aShowPoint= OutputToScreenPixel(Point(0,0));
    if ( ( rHEvt.GetMode() & HELPMODE_QUICK ) == HELPMODE_QUICK &&
         aTxtSize.Width()>GetSizePixel().Width())
        Help::ShowQuickHelp( Rectangle(aShowPoint,aTxtSize), aTxtStr, QuickHelpFlags::Top|QuickHelpFlags::Left );
    else
        FixedText::RequestHelp( rHEvt );
}

MyWindow::MyWindow( vcl::Window *pParent ) :
    Window( pParent ),
    aCountField( this, WinBits(WB_SPIN | WB_REPEAT | WB_BORDER) ),
    aCountButton( this ),
    aTimeText( this ),
    aColField( this, WinBits(WB_SPIN | WB_REPEAT | WB_BORDER) ),
    aRowField( this, WinBits(WB_SPIN | WB_REPEAT | WB_BORDER) ),
    aPosField( this, WinBits(WB_SPIN | WB_REPEAT | WB_BORDER) ),
    aLenField( this, WinBits(WB_SPIN | WB_REPEAT | WB_BORDER) ),
    aTextEdit( this, WinBits(WB_BORDER) ),
    aTextButton( this ),
    aBlaButton( this ),
    aTabButton( this ),
    aViewButton( this )
{
    aCountField.SetPosSizePixel( Point(10,10), Size(40,20) );
    aCountField.SetValue(1);

    aCountButton.SetPosSizePixel( Point(10,40), Size(100,30) );
    aCountButton.SetText("increment");

    aTimeText.SetPosSizePixel( Point(10,80), Size(100,20) );

    aColField.SetPosSizePixel( Point(10,120), Size(40,20) );
    aRowField.SetPosSizePixel( Point(60,120), Size(40,20) );
    aPosField.SetPosSizePixel( Point(10,150), Size(40,20) );
    aLenField.SetPosSizePixel( Point(60,150), Size(40,20) );
    aTextEdit.SetPosSizePixel( Point(10,180), Size(100,20) );

    aTextButton.SetPosSizePixel( Point(10,210), Size(100,30) );
    aTextButton.SetText("col/row/pos/len");

    aBlaButton.SetPosSizePixel( Point(10,260), Size(100,30) );
    aBlaButton.SetText("Bla");

    aTabButton.SetPosSizePixel( Point(10,310), Size(100,30) );
    aTabButton.SetText("sheets");

    aViewButton.SetPosSizePixel( Point(10,360), Size(100,30) );
    aViewButton.SetText("Ugh");

    aCountButton.SetClickHdl(LINK(this, MyWindow, CountHdl));
    aTextButton.SetClickHdl(LINK(this, MyWindow, TextHdl));
    aBlaButton.SetClickHdl(LINK(this, MyWindow, BlaHdl));
    aTabButton.SetClickHdl(LINK(this, MyWindow, TabHdl));
    aViewButton.SetClickHdl(LINK(this, MyWindow, ViewHdl));

    aCountField.Show();
    aCountButton.Show();
    aTimeText.Show();
    aColField.Show();
    aRowField.Show();
    aPosField.Show();
    aLenField.Show();
    aTextEdit.Show();
    aTextButton.Show();
    aBlaButton.Show();
    aTabButton.Show();
    aViewButton.Show();
}

XSpreadsheetDocumentRef lcl_GetDocument()
{
    XServiceManagerRef xProv = getGlobalServiceManager();
    OSL_ENSURE( xProv.is(), "no ServiceManager!" );

    XServiceRegistryRef xReg = (XServiceRegistry*)xProv->queryInterface(XServiceRegistry::getSmartUik());
    if ( !xReg )
        return NULL;

    Sequence<Uik> aIfaces( 1 );
    aIfaces.getArray()[0] = XModelCollection::getSmartUik();
    XServiceProviderRef xSSI = xProv->getServiceProvider( L"stardiv.desktop.ModelCollection",
                                                            aIfaces, Sequence<Uik>() );

    XModelCollectionRef aCollRef = (XModelCollection*)
                        xSSI->newInstance()->queryInterface( XModelCollection::getSmartUik() );
    sal_uInt16 nCount = aCollRef->getCount();

    XSpreadsheetDocumentRef xModel;                         // calc model
    for (sal_uInt16 nMod=0; nMod<nCount && !xModel; nMod++)     // search calc doc
    {
        XModelRef aRef = aCollRef->getItemByIndex( nMod );
        if ( aRef )
        {
            aRef->acquire();
            xModel = (XSpreadsheetDocument*) aRef->queryInterface( XSpreadsheetDocument::getSmartUik() );
            aRef->release();
        }
    }
    return xModel;
}

XInterfaceRef lcl_GetView()
{
    XInterfaceRef xView;
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
        xView = xDoc->getDDELinks();        //! temporary for test !!!!!!!!!

    return xView;
}

void lcl_OutputNames( const XInterfaceRef& xSource,     // XNameAccess
                        const XSpreadsheetDocumentRef& xDoc,
                        sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nTab )
{
    CellAddress aAdr;
    aAdr.Sheet  = nTab;
    aAdr.Column = nCol;
    aAdr.Row    = nRow;

    XNameAccessRef xNames = (XNameAccess*)xSource->queryInterface(XNameAccess::getSmartUik());
    if (!xNames) return;
    Sequence<UString> aSeq = xNames->getElementNames();

    sal_uInt16 nLen = (sal_uInt16)aSeq.getLen();

    XCellRef xCell = xDoc->getCell(aAdr);
    if (!xCell) return;
    xCell->setValue( nLen );
    ++aAdr.Row;

    UString* pAry = aSeq.getArray();
    for (sal_uInt16 i=0; i<nLen; i++)
    {
        xCell = xDoc->getCell(aAdr);
        if (!xCell) return;
        XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
        if (!xText) return;
        xText->setText( pAry[i] );
        ++aAdr.Row;
    }
}

void lcl_SetText( const XTextRef& xText )
{
    if (!xText.is()) return;
    XTextCursorRef xCursor = xText->createTextCursor();
    if (!xCursor.is()) return;
    XTextPositionRef xPos = (XTextPosition*)xCursor->queryInterface(XTextPosition::getSmartUik());
    XPropertySetRef xProp = (XPropertySet*)xCursor->queryInterface(XPropertySet::getSmartUik());
    XControlCharacterInsertableRef xControl = (XControlCharacterInsertable*)
                            xCursor->queryInterface(XControlCharacterInsertable::getSmartUik());
    XParagraphCursorRef xPara = (XParagraphCursor*)
                            xCursor->queryInterface(XParagraphCursor::getSmartUik());

    if (!xPos.is() || !xControl.is() || !xPara.is()) return;    // PropertySet can be missing

    xText->setText(L"bla babbel");
    xCursor->gotoEnd(false);
    xControl->insertControlCharacter( TEXTCONTROLCHAR_PARAGRAPH_BREAK );
    xPos->collapseToEnd();
    xPos->setText(L"s\xFClz");      // second paragraph

    xCursor->gotoStart(false);
    xPara->gotoEndOfParagraph(false);
    xCursor->goLeft(5, true);               // last 5 characters in the first paragraph
    if (xProp.is())
        xProp->setPropertyValue(L"Bold", UsrAny((sal_Bool)true));
}

void lcl_DoCount()
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        XActionLockableRef xLock = (XActionLockable*)
                            xDoc->queryInterface(XActionLockable::getSmartUik());
        XCalculateRef xCalc = (XCalculate*)
                            xDoc->queryInterface(XCalculate::getSmartUik());
        if (xLock)
            xLock->addActionLock();                         // don't paint in between
        if (xCalc)
            xCalc->setAutomaticCalculation(false);

        CellAddress aPos;
        aPos.Sheet  = 0;

        for (sal_uInt16 nRow = 0; nRow < 20; nRow++)
        {
            aPos.Row = nRow;
            for (sal_uInt16 nCol = 0; nCol < 10; nCol++)
            {
                aPos.Column = nCol;
                XCellRef xCell = xDoc->getCell(aPos);
                if ( xCell )
                {
                    // increment value in cell by 1
                    double fVal = xCell->getValue();
                    fVal += 1.0;
                    xCell->setValue( fVal );
                }
            }
        }

        if (xCalc)
            xCalc->setAutomaticCalculation(true);
        if (xLock)
            xLock->removeActionLock();
    }
}

void lcl_GlobalCell()
{
    if ( xGlobalCell )
    {
        String aStr = OUStringToString( xGlobalCell->getFormula(), CHARSET_SYSTEM );
        aStr+='0';
        xGlobalCell->setFormula( StringToOUString( aStr, CHARSET_SYSTEM ) );
    }
}

void lcl_Annotations( FixedText& aTimeText )
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        CellAddress aPos;
        aPos.Sheet  = 0;
        aPos.Column = 1;
        aPos.Row    = 2;
        XCellRef xCell = xDoc->getCell(aPos);
        if ( xCell )
        {
            XSheetAnnotationAnchorRef xAnchor =
                (XSheetAnnotationAnchor*)xCell->queryInterface(XSheetAnnotationAnchor::getSmartUik());
            if ( xAnchor )
            {
                XSheetAnnotationRef xAnnotation = xAnchor->getAnnotation();
                if ( xAnnotation )
                {
                    String aBlubb = OUStringToString( xAnnotation->getAuthor(), CHARSET_SYSTEM )+
                                    String(" - ")+
                                    OUStringToString( xAnnotation->getDate(), CHARSET_SYSTEM );
                    aTimeText.SetText(aBlubb);

                    XTextRef xAnnotationText =
                        (XText*)xAnnotation->queryInterface(XText::getSmartUik());
                    if ( xAnnotationText )
                    {
                        XTextCursorRef xCursor = xAnnotationText->createTextCursor();
                        if (xCursor)
                        {
                            XTextPositionRef xPos = (XTextPosition*)
                                xCursor->queryInterface(XTextPosition::getSmartUik());
                            XControlCharacterInsertableRef xControl = (XControlCharacterInsertable*)
                                xCursor->queryInterface(XControlCharacterInsertable::getSmartUik());

                            if (xPos && xControl)
                            {
                                sal_uInt64 nStart = tools::Time::GetSystemTicks();

                                xAnnotationText->setText(L"bla");
                                xCursor->gotoEnd(false);
                                xCursor->goLeft(1,true);
                                xPos->setText(L"ubb");
                                for (sal_uInt16 i=0; i<10; i++)
                                {
                                    xPos->collapseToEnd();
                                    xControl->insertControlCharacter( TEXTCONTROLCHAR_PARAGRAPH_BREAK );
                                    xPos->collapseToEnd();
                                    xPos->setText(L"dumdi");
                                }

                                sal_uInt64 nEnd = tools::Time::GetSystemTicks();
                                aTimeText.SetText(String(nEnd-nStart)+String(" ms"));
                            }
                        }
                    }
                }
            }
        }
    }
}

void lcl_Cursor( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        XActionLockableRef xLock = (XActionLockable*)
                            xDoc->queryInterface(XActionLockable::getSmartUik());
        if (xLock)
            xLock->addActionLock();

        CellAddress aPos;
        aPos.Sheet  = 0;
        aPos.Column = 1;
        aPos.Row    = 2;
        XCellRef xCell = xDoc->getCell(aPos);
        if ( xCell )
        {
            XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
            XCellCollectionRef xColl = (XCellCollection*)xCell->queryInterface(XCellCollection::getSmartUik());
            if ( xText && xColl )
            {
                xText->setText(L"bla");
                XLineCursorRef xCursor = xColl->createCursor();
                if ( xCursor )
                {
                    XCellCursorRef xCC = (XCellCursor*)xCursor->queryInterface(XCellCursor::getSmartUik());
                    XCellRangesCursorRef xRC = (XCellRangesCursor*)
                                        xCursor->queryInterface(XCellRangesCursor::getSmartUik());

                    if ( xCC && xRC )
                    {
                        xCursor->goDown( 1, false );

                        xColl = xCC->getRanges();
                        if ( xColl )
                        {
                            // XText is there, if it's a single cell
                            xText = (XText*)xColl->queryInterface(XText::getSmartUik());
                            if ( xText )
                            {
                                xText->setText(L"babbel");
                            }
                        }

                        CellRangeAddress aSecond;
                        aSecond.Sheet       = 0;
                        aSecond.StartColumn = 3;
                        aSecond.StartRow    = 4;
                        aSecond.EndColumn   = 3;
                        aSecond.EndRow      = 4;
                        xRC->gotoUnion(aSecond);

                        xColl = xCC->getRanges();
                        if ( xColl )
                        {
                            XPropertySetRef xProp = (XPropertySet*)
                                                xColl->queryInterface(XPropertySet::getSmartUik());
                            if ( xProp )
                            {
                                UsrAny aAny;

                                aAny = xProp->getPropertyValue(L"ShadowFormat");
                                if ( aAny.getReflection()->getName() ==
                                     ShadowFormat_getReflection()->getName() )
                                {
                                    //ShadowFormat* pOld = (ShadowFormat*)aAny.get();
                                    ShadowFormat aNew;
                                    aNew.Location       = SHADOWLOCATION_BOTTOMRIGHT;
                                    aNew.ShadowWidth    = 100;
                                    aNew.IsTransparent  = false;
                                    aNew.Color          = 0xff0000L;
                                    aAny.set( &aNew, aAny.getReflection() );
                                    xProp->setPropertyValue(L"ShadowFormat", aAny);
                                }

                                aAny = xProp->getPropertyValue(L"RotationValue");
                                aAny.setINT32(4500);
                                xProp->setPropertyValue(L"RotationValue", aAny);

                                aAny = xProp->getPropertyValue(L"FontHeight");
                                aAny.setUINT32(280);
                                xProp->setPropertyValue(L"FontHeight", aAny);

                                aAny = xProp->getPropertyValue(L"TransparentBackground");
                                aAny.setBOOL(false);
                                xProp->setPropertyValue(L"TransparentBackground", aAny);

                                aAny = xProp->getPropertyValue(L"BackgroundColor");
                                aAny.setUINT32(0xffff00);
                                xProp->setPropertyValue(L"BackgroundColor", aAny);

                                aAny = xProp->getPropertyValue(L"CellProtection");
                                if ( aAny.getReflection()->getName() ==
                                     CellProtection_getReflection()->getName() )
                                {
                                    //CellProtection* pOld = (CellProtection*)aAny.get();
                                    CellProtection aNew;
                                    aNew.Locked         = false;
                                    aNew.FormulaHidden  = false;
                                    aNew.Hidden         = false;
                                    aNew.PrintHidden    = false;
                                    aAny.set( &aNew, aAny.getReflection() );
                                    xProp->setPropertyValue(L"CellProtection", aAny);
                                }
                            }

                            // XIndexAccess, only if there are several (??! ??!)
                            XIndexAccessRef xIndex = (XIndexAccess*)
                                                xColl->queryInterface(XIndexAccess::getSmartUik());
                            if ( xIndex )
                            {
                                sal_uInt16 nCount = (sal_uInt16)xIndex->getCount();
                                aTimeText.SetText( String(nCount) );
                            }
                        }
                    }
                }
            }
        }

        if (xLock)
            xLock->removeActionLock();
    }
}

void lcl_Cells( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        long nCount = 0;
        sal_uInt64 nStart = tools::Time::GetSystemTicks();

        XActionLockableRef xLock = (XActionLockable*)
                            xDoc->queryInterface(XActionLockable::getSmartUik());
//      if (xLock)
//          xLock->addActionLock();

        CellRangeAddress aRngAddr;
        aRngAddr.Sheet       = 0;
        aRngAddr.StartColumn = 0;
        aRngAddr.StartRow    = 0;
        aRngAddr.EndColumn   = 9;
        aRngAddr.EndRow      = 19;
        XCellRangeRef xRange = xDoc->getCellRange(aRngAddr);
        if (xRange)
        {
            XCellCollectionRef xColl = (XCellCollection*)
                        xRange->queryInterface(XCellCollection::getSmartUik());
            if (xColl)
            {
                XEnumerationAccessRef xEnAcc = xColl->getCells();
                if (xEnAcc)
                {
                    XEnumerationRef xEnum = xEnAcc->getEnumeration();
                    if (xEnum)
                    {
                        while (xEnum->hasMoreElements())
                        {
                            XInterfaceRef xInt = xEnum->nextElement();
                            if (xInt)
                            {
                                ++nCount;
                            }
                        }
                    }
                }
            }
        }

        sal_uInt64 nEnd = tools::Time::GetSystemTicks();
        aTimeText.SetText(String(nCount)+String(" ")+String(nEnd-nStart)+String(" ms"));

//      if (xLock)
//          xLock->removeActionLock();
    }
}

void lcl_Sheet( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XTableSheetRef xSheet = xSheets->getSheetByIndex(0);
    if (!xSheet) return;
    XNamedRef xNamed = (XNamed*)xSheet->queryInterface(XNamed::getSmartUik());
    if (!xNamed) return;

    String aName = OUStringToString( xNamed->getName(), CHARSET_SYSTEM );
    aName += 'X';
    xNamed->setName(StringToOUString( aName, CHARSET_SYSTEM ));

    XCellRangeRef xRange = (XCellRange*)xSheet->queryInterface(XCellRange::getSmartUik());
    if (!xRange) return;
    XCellRef xCell = xRange->getCell(2,1);
    if (!xCell) return;
    XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
    if (!xText) return;
    String aBla = OUStringToString( xText->getText(), CHARSET_SYSTEM );
    aBla += "bla";
    xText->setText(StringToOUString( aBla, CHARSET_SYSTEM ));

    XColumnRowRangeRef xCRR = (XColumnRowRange*)xSheet->queryInterface(XColumnRowRange::getSmartUik());
    if (!xCRR) return;

    XTableColumnsRef xCols = xCRR->getColumns();
    if (!xCols) return;
    XPropertySetRef xCol = xCols->getColumnByIndex(2);
    if (!xCol) return;

    UINT16 nWidth = TypeConversion::toUINT16(xCol->getPropertyValue(L"Width"));
//  UINT16 nNewWidth = nWidth + 100;
//  xCol->setPropertyValue(L"Width", UsrAny(nNewWidth));

    xCol->setPropertyValue(L"OptimalWidth", UsrAny((sal_Bool)true));
    xCol->setPropertyValue(L"NewPage", UsrAny((sal_Bool)false));

    UsrAny aAny = xCol->getPropertyValue(L"ShadowFormat");
    if ( aAny.getReflection()->getName() ==
         ShadowFormat_getReflection()->getName() )
    {
        //ShadowFormat* pOld = (ShadowFormat*)aAny.get();
        ShadowFormat aNew;
        aNew.Location       = SHADOWLOCATION_BOTTOMRIGHT;
        aNew.ShadowWidth    = 100;
        aNew.IsTransparent  = false;
        aNew.Color          = 0xff0000L;
        aAny.set( &aNew, aAny.getReflection() );
        xCol->setPropertyValue(L"ShadowFormat", aAny);
    }

    XTableRowsRef xRows = xCRR->getRows();
    if (!xRows) return;
    XPropertySetRef xRow = xRows->getRowByIndex(1);
    if (!xRow) return;

    xRows->removeRowsByIndex( 2, 1 );

    UINT16 nHeight = TypeConversion::toUINT16(xRow->getPropertyValue(L"Height"));
    sal_Bool bOptH = TypeConversion::toBOOL(xRow->getPropertyValue(L"OptimalHeight"));

    UINT16 nNewHeight = nHeight + 100;
    xRow->setPropertyValue(L"Height", UsrAny(nNewHeight));

    aTimeText.SetText(String("W:")+String(nWidth)+String(" H:")+String(nHeight)+
                        String(" ")+String((sal_uInt16)bOptH));
}

void lcl_Names( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XNamedRangesRef xNames = xDoc->getNamedRanges();
    if (!xNames) return;
    XNamedRangeRef xName = xNames->getRangeByName(L"bla");
    if (!xName) return;
    String aCont = OUStringToString( xName->getContent(), CHARSET_SYSTEM );
    aTimeText.SetText(aCont);

    XCellRangeSourceRef xSource = (XCellRangeSource*)
                            xName->queryInterface(XCellRangeSource::getSmartUik());
    if (!xSource) return;
    XCellRangeRef xRange = xSource->getReferredCells();
    if (!xRange) return;
    XPropertySetRef xProp = (XPropertySet*)xRange->queryInterface(XPropertySet::getSmartUik());
    if (!xProp) return;
    UsrAny aAny = xProp->getPropertyValue(L"RotationValue");
    aAny.setINT32(3000);
    xProp->setPropertyValue(L"RotationValue", aAny);
}

void lcl_Sheets( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;

    xSheets->moveSheet(0, 1, true);
    xSheets->moveSheet(0, 2, false);
}

void lcl_Goal( FixedText& aTimeText )
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XGoalSeekRef xGoal = (XGoalSeek*)xDoc->queryInterface(XGoalSeek::getSmartUik());
    if (!xGoal) return;

    double fResult;
    CellAddress aFormula;   // A1
    aFormula.Sheet  = 0;
    aFormula.Column = 0;
    aFormula.Row    = 0;
    CellAddress aVar;       // A2
    aVar.Sheet  = 0;
    aVar.Column = 0;
    aVar.Row    = 1;
    sal_Bool bFound = xGoal->doGoalSeek(fResult, aFormula, aVar, L"42");

    if (bFound)
    {
        CellAddress aOut;       // A3
        aOut.Sheet  = 0;
        aOut.Column = 0;
        aOut.Row    = 2;

        XCellRef xCell = xDoc->getCell(aOut);
        if (!xCell) return;
        xCell->setValue(fResult);
    }
}

void lcl_TabOp( FixedText& aTimeText )
{
    // Multiple operations on sheet2
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;

    XCellRangeRef xRange;
    XTableOperationRef xGoal;
    CellRangeAddress aRangeAddr;
    CellRangeAddress aFormulaRange;
    CellAddress aColumnCell;
    CellAddress aRowCell;
    TableOperationMode nMode;

    aRangeAddr.Sheet            = 1;    // c9:e11
    aRangeAddr.StartColumn      = 2;
    aRangeAddr.StartRow         = 8;
    aRangeAddr.EndColumn        = 4;
    aRangeAddr.EndRow           = 10;
    aFormulaRange.Sheet         = 1;    // c6:c7
    aFormulaRange.StartColumn   = 2;
    aFormulaRange.StartRow      = 5;
    aFormulaRange.EndColumn     = 2;
    aFormulaRange.EndRow        = 6;
    aColumnCell.Sheet           = 0;    // not used
    aColumnCell.Column          = 0;
    aColumnCell.Row             = 0;
    aRowCell.Sheet              = 1;    // c5
    aRowCell.Column             = 2;
    aRowCell.Row                = 4;
    nMode = TABLEOP_ROW;

    xRange = xDoc->getCellRange(aRangeAddr);
    if (!xRange) return;
    xGoal = (XTableOperation*)xRange->queryInterface(XTableOperation::getSmartUik());
    if (!xGoal) return;
    xGoal->setTableOperation( nMode, aFormulaRange, aColumnCell, aRowCell );

    aRangeAddr.Sheet            = 1;    // b19:d21
    aRangeAddr.StartColumn      = 1;
    aRangeAddr.StartRow         = 18;
    aRangeAddr.EndColumn        = 3;
    aRangeAddr.EndRow           = 20;
    aFormulaRange.Sheet         = 1;    // c16:d16
    aFormulaRange.StartColumn   = 2;
    aFormulaRange.StartRow      = 15;
    aFormulaRange.EndColumn     = 3;
    aFormulaRange.EndRow        = 15;
    aColumnCell.Sheet           = 1;    // b16
    aColumnCell.Column          = 1;
    aColumnCell.Row             = 15;
    aRowCell.Sheet              = 0;    // not used
    aRowCell.Column             = 0;
    aRowCell.Row                = 0;
    nMode = TABLEOP_COLUMN;

    xRange = xDoc->getCellRange(aRangeAddr);
    if (!xRange) return;
    xGoal = (XTableOperation*)xRange->queryInterface(XTableOperation::getSmartUik());
    if (!xGoal) return;
    xGoal->setTableOperation( nMode, aFormulaRange, aColumnCell, aRowCell );

    aRangeAddr.Sheet            = 1;    // b29:e32
    aRangeAddr.StartColumn      = 1;
    aRangeAddr.StartRow         = 28;
    aRangeAddr.EndColumn        = 4;
    aRangeAddr.EndRow           = 31;
    aFormulaRange.Sheet         = 1;    // c27:c27
    aFormulaRange.StartColumn   = 2;
    aFormulaRange.StartRow      = 26;
    aFormulaRange.EndColumn     = 2;
    aFormulaRange.EndRow        = 26;
    aColumnCell.Sheet           = 1;    // c25
    aColumnCell.Column          = 2;
    aColumnCell.Row             = 24;
    aRowCell.Sheet              = 1;    // c26
    aRowCell.Column             = 2;
    aRowCell.Row                = 25;
    nMode = TABLEOP_BOTH;

    xRange = xDoc->getCellRange(aRangeAddr);
    if (!xRange) return;
    xGoal = (XTableOperation*)xRange->queryInterface(XTableOperation::getSmartUik());
    if (!xGoal) return;
    xGoal->setTableOperation( nMode, aFormulaRange, aColumnCell, aRowCell );
}

void lcl_Fill( FixedText& aTimeText )
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;

    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;

    XCellSeriesRef xFill = (XCellSeries*)xSelInt->queryInterface(XCellSeries::getSmartUik());
    if (!xFill) return;

//  xFill->fillAuto( FILL_DIRECTION_TO_BOTTOM, 2 );

    xFill->fillSeries( FILL_DIRECTION_TO_LEFT, FILL_MODE_GROWTH, FILL_DATE_DAY,
                        2.0, 1000.0 );
}

void lcl_Audi( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XTableSheetRef xSheet = xSheets->getSheetByIndex(0);
    if (!xSheet) return;

    XSheetAuditingRef xAudi = (XSheetAuditing*)xSheet->queryInterface(XSheetAuditing::getSmartUik());
    if (!xAudi) return;

    CellAddress aPosition;
    aPosition.Sheet     = 0;
    aPosition.Column    = 0;
    aPosition.Row       = 0;
    xAudi->showDependents(aPosition);
}

void lcl_Consoli( FixedText& aTimeText )
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XConsolidationRef xCons = (XConsolidation*)xDoc->queryInterface(XConsolidation::getSmartUik());
    if (!xCons) return;
    XConsolidationDescriptorRef xDesc = xCons->createConsolidationDescriptor(false);
    if (!xDesc) return;
    xDesc->setFunction(SUMMARY_COUNTNUMS);
    xCons->consolidate(xDesc);
}

void lcl_Sort( FixedText& aTimeText )
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XSortableRef xSort = (XSortable*)xSelInt->queryInterface(XSortable::getSmartUik());
    if (!xSort) return;
    XSortDescriptorRef xDesc = xSort->createSortDescriptor(false);
    if (!xDesc) return;
    Sequence<SortField> aFields = xDesc->getSortFields();
    if (aFields.getLen())
    {
        // reverse first field
        SortField* pAry = aFields.getArray();
        if (!pAry) return;
        pAry[0].Ascending = !pAry[0].Ascending;
    }
    else    // new sequence, 1st column in ascending order
    {
        aFields = Sequence<SortField>(1);
        SortField* pAry = aFields.getArray();
        if (!pAry) return;
        pAry[0].Field     = 0;
        pAry[0].Ascending = true;
        pAry[0].Type      = SORT_FIELD_AUTOMATIC;
    }
    xDesc->setSortFields(aFields);

    XTableSortDescriptorRef xTableSort = (XTableSortDescriptor*)
                            xDesc->queryInterface(XTableSortDescriptor::getSmartUik());
    if (!xTableSort) return;
    CellAddress aOutPos;
    aOutPos.Sheet  = 2;
    aOutPos.Column = 0;
    aOutPos.Row    = 0;
    xTableSort->setUseOutputPosition(true);
    xTableSort->setOutputPosition(aOutPos);

    XPropertySetRef xPropSet = (XPropertySet*)
                            xDesc->queryInterface(XPropertySet::getSmartUik());
    if (!xPropSet) return;
    xPropSet->setPropertyValue(L"IncludeFormats", UsrAny((sal_Bool)false));

    xSort->sort(xDesc);
}

void lcl_Filter( FixedText& aTimeText )
{
    aTimeText.SetText("...");

    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XFilterableRef xFilter = (XFilterable*)xSelInt->queryInterface(XFilterable::getSmartUik());
    if (!xFilter) return;

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    CellRangeAddress aAddress;
    aAddress.Sheet       = 3;
    aAddress.StartColumn = 0;
    aAddress.StartRow    = 0;
    aAddress.EndColumn   = 1;
    aAddress.EndRow      = 2;
    XCellRangeRef xRange = xDoc->getCellRange(aAddress);
    if (!xRange) return;
    XAdvancedFilterSourceRef xSource = (XAdvancedFilterSource*)
                            xRange->queryInterface(XAdvancedFilterSource::getSmartUik());
    if (!xSource) return;

    XTableFilterDescriptorRef xDesc = xSource->createAdvancedFilter(xFilter);
    if (!xDesc)
    {
        aTimeText.SetText("no filter");
        return;
    }
    aTimeText.SetText("filter found");
    xFilter->filter(xDesc);
}

void lcl_AutoFilter( FixedText& aTimeText )
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XFilterableRef xFilter = (XFilterable*)xSelInt->queryInterface(XFilterable::getSmartUik());
    if (!xFilter) return;

    sal_Bool bAuto = xFilter->getAutoFilter();
    xFilter->setAutoFilter(!bAuto);
}

void lcl_Merge( FixedText& aTimeText )
{
    static sal_Bool bMerged = false;

    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XMergeableRef xMerge = (XMergeable*)xSelInt->queryInterface(XMergeable::getSmartUik());
    if (!xMerge) return;

    if (bMerged)
        xMerge->unmergeCells();
    else
        xMerge->mergeCells();
    bMerged = !bMerged;
}

void lcl_Outline( FixedText& aTimeText )
{
    static sal_Bool bOutline = false;

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XTableSheetRef xSheet = xSheets->getSheetByIndex(0);
    if (!xSheet) return;
    XSheetOutlineRef xOut = (XSheetOutline*)xSheet->queryInterface(XSheetOutline::getSmartUik());
    if (!xOut) return;

    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XAddressableCellRangeRef xRange = (XAddressableCellRange*)
                                xSelInt->queryInterface(XAddressableCellRange::getSmartUik());
    if (!xRange) return;
    CellRangeAddress aRange = xRange->getRangeAddress();

    if (bOutline)
        xOut->showDetail( aRange );
    else
        xOut->hideDetail( aRange );

    bOutline = !bOutline;
}

void lcl_Bla( FixedText& aTimeText )
{
    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XActionLockableRef xLock = (XActionLockable*)xDoc->queryInterface(XActionLockable::getSmartUik());
    if (!xLock) return;
    xLock->addActionLock();
    xLock->addActionLock();
    sal_uInt16 nCount = xLock->resetActionLocks();      // should be 2
    String aBla = nCount;
    xLock->setActionLocks(nCount);
    xLock->removeActionLock();
    xLock->removeActionLock();

    aBla += '/'; aBla += xLock->resetActionLocks(); // should be 0

    aTimeText.SetText(aBla);
}

void lcl_CellCursor( FixedText& aTimeText )
{
    static int nCursorCount = 0;

    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XCellCollectionRef xColl = (XCellCollection*)xSelInt->queryInterface(XCellCollection::getSmartUik());
    if (!xColl) return;

    XLineCursorRef xCursor = xColl->createCursor();
    if (!xCursor) return;
    XCellCursorRef xCC = (XCellCursor*)xCursor->queryInterface(XCellCursor::getSmartUik());
    if (!xCC) return;
    XCellRangesCursorRef xCRC = (XCellRangesCursor*)xCursor->queryInterface(XCellRangesCursor::getSmartUik());
    if (!xCRC) return;
    XCellRangeCursorRef xCR = (XCellRangeCursor*)xCursor->queryInterface(XCellRangeCursor::getSmartUik());
    if (!xCR) return;
    XCellContentCursorRef xCCC = (XCellContentCursor*)xCursor->queryInterface(XCellContentCursor::getSmartUik());
    if (!xCCC) return;
    XFormulaCursorRef xFC = (XFormulaCursor*)xCursor->queryInterface(XFormulaCursor::getSmartUik());
    if (!xFC) return;

    CellAddress aPos;
    aPos.Sheet  = 0;        // ignored
    aPos.Row    = 3;
    aPos.Column = 2;

    switch (nCursorCount++)
    {
        case 0:
            xFC->gotoDependents(false);
            break;
        case 1:
            xFC->gotoDependents(true);
            break;
        case 2:
            xFC->gotoPrecedents(false);
            break;
        case 3:
            xFC->gotoPrecedents(true);

            nCursorCount = 0;
            break;
    }

    XCellCollectionRef xNew = xCC->getRanges();
    if (!xNew) return;
    xView->select( xNew );
}

void lcl_Notes( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XTableSheetRef xSheet = xSheets->getSheetByIndex(0);
    if (!xSheet) return;

    XSheetAnnotationsRef xNotes = xSheet->getAnnotations();
    if (!xNotes) return;
    XIndexAccessRef xNIndex = (XIndexAccess*)xNotes->queryInterface(XIndexAccess::getSmartUik());
    if (!xNIndex) return;

    CellAddress aPos;
    aPos.Column = 0;
    aPos.Row    = 0;
    aPos.Sheet  = 0;
    xNotes->addAnnotation( L"new", aPos );

    sal_uLong nCount = xNIndex->getCount();
    for (sal_uLong i=0; i<nCount; i++)
    {
        XSheetAnnotationRef xAnn = xNotes->getAnnotationByIndex((UINT16)i);
        XTextRef xText = (XText*)xAnn->queryInterface(XText::getSmartUik());
        if (xText)
        {
            String aStr = OUStringToString( xText->getText(), CHARSET_SYSTEM );
            aStr += "x";
            xText->setText(StringToOUString( aStr, CHARSET_SYSTEM ));
        }
    }
}

void lcl_Scenario( FixedText& aTimeText )
{
    aTimeText.SetText( "..." );
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XTableSheetRef xSheet = xSheets->getSheetByIndex(0);
    if (!xSheet) return;

    XScenariosRef xColl = xSheet->getScenarios();
    if (!xColl) return;

    Sequence<CellRangeAddress> aRanges(2);
    CellRangeAddress* pAry = aRanges.getArray();
    if (!pAry) return;
    pAry[0].Sheet       = 0;
    pAry[0].StartColumn = 0;
    pAry[0].StartRow    = 0;
    pAry[0].EndColumn   = 1;
    pAry[0].EndRow      = 1;
    pAry[1].Sheet       = 0;
    pAry[1].StartColumn = 3;
    pAry[1].StartRow    = 3;
    pAry[1].EndColumn   = 4;
    pAry[1].EndRow      = 4;

    xColl->addScenario( aRanges, L"bla", L"bla blubb" );

    XIndexAccessRef xIndex = (XIndexAccess*)xColl->queryInterface(XIndexAccess::getSmartUik());
    if (!xIndex) return;
    sal_uLong nCount = xIndex->getCount();
    aTimeText.SetText( nCount );

    XScenarioRef xScen = xColl->getScenarioByIndex(0);
    if (!xScen) return;

    aRanges = Sequence<CellRangeAddress>(1);
    pAry = aRanges.getArray();
    if (!pAry) return;
    pAry[0].Sheet       = 0;
    pAry[0].StartColumn = 6;
    pAry[0].StartRow    = 6;
    pAry[0].EndColumn   = 7;
    pAry[0].EndRow      = 7;

    xScen->addRanges( aRanges );

    XTableSheetRef xSh2 = xSheets->getSheetByIndex(1);
    if (!xSh2) return;

    xSh2->setVisible( true );
    xSh2->setVisible( false );
}

void lcl_Formula( FixedText& aTimeText )
{
    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    CellAddress aPos;
    aPos.Sheet  = 0;
    aPos.Column = 0;
    aPos.Row    = 0;
    XCellRef xCell = xDoc->getCell(aPos);
    if (!xCell) return;

//  String aStr = OUStringToString( xCell->getFormula(), CHARSET_SYSTEM );
//  aTimeText.SetText(aStr);

    XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
    if (!xText) return;
    String aStr = OUStringToString( xText->getText(), CHARSET_SYSTEM );
    aTimeText.SetText(aStr);
}

void lcl_DBRange( FixedText& aTimeText )    // 23
{
    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XDatabaseRangesRef xDBs = xDoc->getDatabaseRanges();
    if (!xDBs) return;

    CellRangeAddress aRange;
    aRange.Sheet        = 0;
    aRange.StartColumn  = 1;
    aRange.StartRow     = 1;
    aRange.EndColumn    = 3;
    aRange.EndRow       = 10;

    xDBs->addRange( L"blubb", aRange );

    xDBs->removeRangeByName( L"gaga" );

    XDatabaseRangeRef xDB = xDBs->getRangeByName( L"blubb" );
    if (!xDB) return;

    String aName = OUStringToString( xDB->getName(), CHARSET_SYSTEM );
    aTimeText.SetText(aName);

    xDB->setName( L"gaga" );

    CellRangeAddress aDBRange = xDB->getDataArea();
    ++aDBRange.Sheet;
    xDB->setDataArea(aDBRange);
}

void lcl_FillTab( FixedText& aTimeText )    // 24
{
    aTimeText.SetText("...");

    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;

    XCellRangesRef xRanges = (XCellRanges*)xSelInt->queryInterface(XCellRanges::getSmartUik());
    XIndexAccessRef xIndex = (XIndexAccess*)xSelInt->queryInterface(XIndexAccess::getSmartUik());
    if (!xRanges || !xIndex) return;

    sal_uLong nCount = xIndex->getCount();
    aTimeText.SetText(nCount);
}

void lcl_Listener( FixedText& aTimeText )   // 25
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XStarCalcViewRef xView = (XStarCalcView*)xInt->queryInterface(XStarCalcView::getSmartUik());
    if (!xView) return;
    xView->addSelectionChangeListener( new ScTestListener(&aTimeText) );
}

void lcl_CellAttrib( FixedText& aTimeText ) // 26
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;
    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;
    XTextRef xText = (XText*)xSelInt->queryInterface(XText::getSmartUik());
    if (!xText) return;

    XTextCursorRef xCursor = xText->createTextCursor();
    if (!xCursor) return;

    XTextPositionRef xPos = (XTextPosition*)xCursor->queryInterface(XTextPosition::getSmartUik());
    XPropertySetRef xProp = (XPropertySet*)xCursor->queryInterface(XPropertySet::getSmartUik());
    XParagraphCursorRef xPar = (XParagraphCursor*)xCursor->queryInterface(XParagraphCursor::getSmartUik());
    if (!xPos || !xProp || !xPar) return;

    xCursor->gotoStart(false);
    xCursor->goRight(1,false);
    xCursor->goRight(1,true);

    UsrAny aAny = xProp->getPropertyValue(L"FontHeight");
    sal_uInt32 nOld = aAny.getUINT32();
    aAny.setUINT32(nOld*11/10);
    xProp->setPropertyValue(L"FontHeight", aAny);

    xPos->collapseToEnd();
    xCursor->goRight(1,true);

    xProp->setPropertyValue(L"Bold", UsrAny((sal_Bool)true));

    xPos->setText(L"x");

    xPos->collapseToEnd();
    xPar->gotoNextParagraph(false,true);
    xProp->setPropertyValue(L"Italic", UsrAny((sal_Bool)true));
    xProp->setPropertyValue(L"Underlined", UsrAny((sal_Bool)true));
}

void lcl_Styles( FixedText& aTimeText ) // 27
{
    aTimeText.SetText("...");
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XStyleFamiliesRef xFamilies = xDoc->getStyleFamilies();
    if (!xFamilies) return;
    XStyleFamilyRef xFamily = xFamilies->getStyleFamilyByType( STYLE_FAMILY_CELL );
//  XStyleFamilyRef xFamily = xFamilies->getStyleFamilyByType( STYLE_FAMILY_PAGE );
    if (!xFamily) return;
    long nCount = xFamily->getCount();
    aTimeText.SetText(nCount);

    XStyleRef xStyle = xFamily->getStyleByName(L"red");
    if (!xStyle) return;
//  XPropertySetRef xProp = (XPropertySet*)xStyle->queryInterface(XPropertySet::getSmartUik());
//  if (!xProp) return;

    XStyleRef xNew = xFamily->addStyle( L"gaga", xStyle );
    if (!xNew) return;
    XPropertySetRef xProp = (XPropertySet*)xNew->queryInterface(XPropertySet::getSmartUik());
    if (!xProp) return;

    UsrAny aAny;
    aAny = xProp->getPropertyValue(L"TransparentBackground");
    aAny.setBOOL(false);
    xProp->setPropertyValue(L"TransparentBackground", aAny);
    aAny = xProp->getPropertyValue(L"BackgroundColor");
    aAny.setUINT32(0xffff00);
    xProp->setPropertyValue(L"BackgroundColor", aAny);

    xFamily->removeStyle( L"red" );
}

void lcl_PageStyle( FixedText& aTimeText )  // 28
{
    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;
    XStyleFamiliesRef xFamilies = xDoc->getStyleFamilies();
    if (!xFamilies) return;
    XStyleFamilyRef xFamily = xFamilies->getStyleFamilyByType( STYLE_FAMILY_PAGE );
    if (!xFamily) return;
    XStyleRef xStyle = xFamily->getStyleByName(L"Standard");
    if (!xStyle) return;
    XPropertySetRef xProp = (XPropertySet*)xStyle->queryInterface(XPropertySet::getSmartUik());
    if (!xProp) return;

    UsrAny aAny;
    aAny = xProp->getPropertyValue(L"RightPageHeaderContent");

    // does not work:
//  if ( !XHeaderFooterContent_getReflection()->equals(*aAny.getReflection()) )
//      return;

    XHeaderFooterContentRef* pxContent = (XHeaderFooterContentRef*)aAny.get();
    if (!pxContent || !pxContent->is()) return;

    XTextRef xText = (*pxContent)->getCenterText();
    if (!xText) return;

    String aVal = OUStringToString(xText->getText(), CHARSET_SYSTEM);
    aTimeText.SetText(aVal);

//  xText->setText(L"Bla fasel s\xFClz");
    lcl_SetText(xText);

    xProp->setPropertyValue(L"RightPageHeaderContent", aAny);
}

void lcl_AutoForm( FixedText& aTimeText )   // 29
{
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;

    XTableAutoFormatsRef xFormats;

    XTableAutoFormatRef xFormat = xFormats->getAutoFormatByName(L"gaga");
    if (!xFormat) return;
    XPropertySetRef xProp = (XPropertySet*)xFormat->queryInterface(XPropertySet::getSmartUik());
    if (!xProp) return;

    sal_Bool bVal = TypeConversion::toBOOL(xProp->getPropertyValue(L"IncludeBackground"));
    xProp->setPropertyValue(L"IncludeBackground", UsrAny(sal_Bool(!bVal)));

    XNamedRef xNamed = (XNamed*)xFormat->queryInterface(XNamed::getSmartUik());
    if (!xNamed) return;
    xNamed->setName(L"zzz");

    xFormats->addAutoFormat(L"gaga");
    XTableAutoFormatRef xNew = xFormats->getAutoFormatByName(L"gaga");
    if (!xNew) return;

    for (sal_uInt16 i=0; i<16; i++)
    {
        XPropertySetRef xNewProp = xNew->getFieldByIndex(i);
        if (!xNewProp) return;

        xNewProp->setPropertyValue(L"TransparentBackground", UsrAny(sal_Bool(false)));
        sal_uInt32 nColor = 0x111100 * i;
        xNewProp->setPropertyValue(L"BackgroundColor", UsrAny(nColor));
    }
}

void lcl_Pivot( FixedText& aTimeText )  // 30
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (!xDoc) return;

    XSpreadsheetsRef xSheets = xDoc->getSheets();
    if (!xSheets) return;
    XIndexAccessRef xInd = (XIndexAccess*)xSheets->queryInterface(XIndexAccess::getSmartUik());
    if (!xInd) return;
    sal_uInt16 nCount = (sal_uInt16)xInd->getCount();

    for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
    {
        XTableSheetRef xSheet = xSheets->getSheetByIndex(nTab);
        if (!xSheet) return;
        XDataPilotTablesRef xPivots = xSheet->getDataPilotTables();
        if (!xPivots) return;
        lcl_OutputNames( xPivots, xDoc, nTab,0,0 );
        XIndexAccessRef xPInd = (XIndexAccess*)xPivots->queryInterface(XIndexAccess::getSmartUik());
        if (!xPInd) return;
        sal_uInt16 nPCount = (sal_uInt16)xPInd->getCount();
        for (sal_uInt16 nP=0; nP<nPCount; nP++)
        {
            XDataPilotTableRef xTable = xPivots->getTableByIndex(nP);
            if (!xTable) return;

//          xTable->refreshTable();

            XDataPilotDescriptorRef xDesc = (XDataPilotDescriptor*)
                            xTable->queryInterface(XDataPilotDescriptor::getSmartUik());
            if (!xDesc) return;
            CellRangeAddress aSource = xDesc->getSourceRange();
            ++aSource.Sheet;
            xDesc->setSourceRange(aSource);

            CellRangeAddress aAddr = xTable->getOutputRange();
            XCellRangeRef xRange = xDoc->getCellRange(aAddr);
            if (!xRange) return;
            XPropertySetRef xProp = (XPropertySet*)xRange->queryInterface(XPropertySet::getSmartUik());
            if (!xProp) return;
            xProp->setPropertyValue(L"TransparentBackground", UsrAny(sal_Bool(false)));
            xProp->setPropertyValue(L"BackgroundColor", UsrAny((sal_uInt32)0x00FF00));
        }
    }
}

IMPL_LINK_NOARG_TYPED(MyWindow, CountHdl, PushButton*, void)
{

    long nCount = aCountField.GetValue();
    switch ( nCount )
    {
        case 0:
            {
                sal_uInt64 nStart = tools::Time::GetSystemTicks();
                lcl_DoCount();
                sal_uInt64 nEnd = tools::Time::GetSystemTicks();
                aTimeText.SetText(String("Count: ")+String(nEnd-nStart)+String(" ms"));
            }
            break;
        case 1:
            lcl_GlobalCell();
            break;
        case 2:
            lcl_Annotations(aTimeText);
            break;
        case 3:
            lcl_Cursor(aTimeText);
            break;
        case 4:
            lcl_Cells(aTimeText);
            break;
        case 5:
            lcl_Sheet(aTimeText);
            break;
        case 6:
            lcl_Names(aTimeText);
            break;
        case 7:
            lcl_Sheets(aTimeText);
            break;
        case 8:
            lcl_Goal(aTimeText);
            break;
        case 9:
            lcl_TabOp(aTimeText);
            break;
        case 10:
            lcl_Fill(aTimeText);
            break;
        case 11:
            lcl_Audi(aTimeText);
            break;
        case 12:
            lcl_Consoli(aTimeText);
            break;
        case 13:
            lcl_Sort(aTimeText);
            break;
        case 14:
            lcl_Filter(aTimeText);
            break;
        case 15:
            lcl_AutoFilter(aTimeText);
            break;
        case 16:
            lcl_Merge(aTimeText);
            break;
        case 17:
            lcl_Outline(aTimeText);
            break;
        case 18:
            lcl_Bla(aTimeText);
            break;
        case 19:
            lcl_CellCursor(aTimeText);
            break;
        case 20:
            lcl_Notes(aTimeText);
            break;
        case 21:
            lcl_Scenario(aTimeText);
            break;
        case 22:
            lcl_Formula(aTimeText);
            break;
        case 23:
            lcl_DBRange(aTimeText);
            break;
        case 24:
            lcl_FillTab(aTimeText);
            break;
        case 25:
            lcl_Listener(aTimeText);
            break;
        case 26:
            lcl_CellAttrib(aTimeText);
            break;
        case 27:
            lcl_Styles(aTimeText);
            break;
        case 28:
            lcl_PageStyle(aTimeText);
            break;
        case 29:
            lcl_AutoForm(aTimeText);
            break;
        case 30:
            lcl_Pivot(aTimeText);
            break;
    }
}

IMPL_LINK_NOARG_TYPED(MyWindow, TextHdl, PushButton*, void)
{
    sal_uInt16 nCol = (sal_uInt16)aColField.GetValue();
    sal_uInt16 nRow = (sal_uInt16)aRowField.GetValue();
    sal_uInt16 nPos = (sal_uInt16)aPosField.GetValue();
    sal_uInt16 nLen = (sal_uInt16)aLenField.GetValue();
    String aStr = aTextEdit.GetText();

    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        sal_uInt16 nTab = 0;
        CellAddress aPos;
        aPos.Sheet  = 0;
        aPos.Column = nCol;
        aPos.Row    = nRow;
        XCellRef xCell = xDoc->getCell(aPos);
        if ( xCell )
        {
            XTextRef xCellText = (XText*)xCell->queryInterface(XText::getSmartUik());
            if (xCellText)
            {
                XTextCursorRef xCursor = xCellText->createTextCursor();
                if (xCursor)
                {
                    XTextPositionRef xPos = (XTextPosition*)
                        xCursor->queryInterface(XTextPosition::getSmartUik());
                    XControlCharacterInsertableRef xControl = (XControlCharacterInsertable*)
                        xCursor->queryInterface(XControlCharacterInsertable::getSmartUik());

                    if (xPos && xControl)
                    {
                        xCursor->gotoStart(false);
                        xCursor->goRight(11,true);
                        String aVal = OUStringToString( xPos->getText(), CHARSET_SYSTEM );
                        aTimeText.SetText(aVal);
                    }
                }
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(MyWindow, BlaHdl, PushButton*, void)
{
    aTimeText.SetText("...");

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        CellAddress aPos;
        aPos.Sheet  = 0;
        aPos.Column = 1;
        aPos.Row    = 2;
        XCellRef xCell = xDoc->getCell(aPos);
        if ( xCell )
        {
            XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
            XFieldContainerRef xCont = (XFieldContainer*)
                                xCell->queryInterface(XFieldContainer::getSmartUik());

            if ( xText && xCont )
            {
                XFieldTypesRef xTypes = xCont->getFieldTypes();
                if ( xTypes )
                {
                    XTextFieldTypeRef xType = xTypes->getFieldType( FIELDTYPE_INTERNET );
                    XTextCursorRef xCursor = xText->createTextCursor();
                    if ( xCursor && xType )
                    {

                        // delete last field
                        XIndexAccessRef xIndex = (XIndexAccess*)
                                    xType->queryInterface(XIndexAccess::getSmartUik());
                        if (xIndex)
                        {
                            String aBla;
                            sal_uLong nCount = xIndex->getCount();
                            for (sal_uLong i=0; i<nCount; i++)
                            {
                                XInterfaceRef xInt = xIndex->getElementByIndex(i);
                                if (xInt)
                                {
                                    XPropertySetRef xProp = (XPropertySet*)xInt->
                                                    queryInterface(XPropertySet::getSmartUik());
                                    if ( xProp )
                                    {
                                        if (aBla.Len()) aBla += ',';
                                        aBla += OUStringToString(
                                                    TypeConversion::toString(
                                                        xProp->getPropertyValue(L"URL") ),
                                                    CHARSET_SYSTEM );
                                    }
                                    if ( i+1 == nCount )        // last
                                    {
                                        XTextFieldRef xField = (XTextField*)xInt->
                                                    queryInterface(XTextField::getSmartUik());
                                        if (xField)
                                            xTypes->removeTextField(xField);
                                    }
                                }
                            }
                            aTimeText.SetText(aBla);
                        }
                    }
                }
            }
        }

        xGlobalCell = xCell;
    }
}

IMPL_LINK_NOARG_TYPED(MyWindow, TabHdl, PushButton*, void)
{
    String aResult;

    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    if (xDoc)
    {
        XSpreadsheetsRef xSheets = xDoc->getSheets();
        if (xSheets)
        {
            XIndexAccessRef xIndex = (XIndexAccess*)xSheets->queryInterface(XIndexAccess::getSmartUik());
            if (xIndex)
            {
                sal_uInt16 nCount = (sal_uInt16) xIndex->getCount();
                for (sal_uInt16 nTab=0; nTab<nCount; nTab++)
                {
                    XInterfaceRef xInt = xIndex->getElementByIndex(nTab);
                    if (xInt)
                    {
                        XNamedRef xNamed = (XNamed*)xInt->queryInterface(XNamed::getSmartUik());
                        if (xNamed)
                        {
                            if (nTab)
                                aResult += ",";
                            aResult += OUStringToString( xNamed->getName(), CHARSET_SYSTEM );
                        }
                    }
                }
            }

            CellAddress aPos;
            aPos.Sheet  = 0;
            aPos.Column = 0;
            aPos.Row    = 0;

            XEnumerationAccessRef xEAcc = (XEnumerationAccess*)
                            xSheets->queryInterface(XEnumerationAccess::getSmartUik());
            if (xEAcc)
            {
                XEnumerationRef xEnum = xEAcc->getEnumeration();
                if (xEnum)
                {
                    while (xEnum->hasMoreElements())
                    {
                        XInterfaceRef xInt = xEnum->nextElement();
                        if (xInt)
                        {
                            XNamedRef xNamed = (XNamed*)xInt->queryInterface(XNamed::getSmartUik());
                            if (xNamed)
                            {
                                UString aName = xNamed->getName();
                                XCellRef xCell = xDoc->getCell(aPos);
                                if ( xCell )
                                {
                                    XTextRef xText = (XText*)xCell->queryInterface(XText::getSmartUik());
                                    xText->setText( aName );
                                    ++aPos.Row;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    aTimeText.SetText(aResult);
}

void lcl_FillCells(XCellCollectionRef xColl)
{
    XEnumerationAccessRef xEnAcc = xColl->getCells();
    if (!xEnAcc) return;
    XEnumerationRef xEnum = xEnAcc->getEnumeration();
    if (!xEnum) return;
    while (xEnum->hasMoreElements())
    {
        XInterfaceRef xInt = xEnum->nextElement();
        if (xInt)
        {
            XCellRef xCell = (XCell*)xInt->queryInterface(XCell::getSmartUik());
            if (xCell)
            {
                xCell->setValue(42.0);
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(MyWindow, ViewHdl, PushButton*, void)
{
    XSpreadsheetDocumentRef xDoc = lcl_GetDocument();           // calc model
    XInterfaceRef xInt = lcl_GetView();
    if (!xInt) return;
    XDocumentViewRef xView = (XDocumentView*)xInt->queryInterface(XDocumentView::getSmartUik());
    if (!xView) return;

    XInterfaceRef xSelInt = xView->getSelection();
    if (!xSelInt) return;

    XAutoFormattableRef xAuto = (XAutoFormattable*)xSelInt->
                                    queryInterface(XAutoFormattable::getSmartUik());
    if ( xAuto )
        xAuto->applyAutoFormat( L"gaga" );

    XFormulaArrayRef xArr = (XFormulaArray*)xSelInt->queryInterface(XFormulaArray::getSmartUik());
    if ( xArr )
    {
//      xArr->setFormulaArray( "123" );
        String aFormula = OUStringToString( xArr->getFormulaArray(), CHARSET_SYSTEM );
        aTimeText.SetText(aFormula);
    }
    else
        aTimeText.SetText("...");

    XTextRef xText = (XText*)xSelInt->queryInterface(XText::getSmartUik());
    if ( xText )
    {
        String aStr = OUStringToString( xText->getText(), CHARSET_SYSTEM );
        aStr += 'X';
        xText->setText(StringToOUString(aStr, CHARSET_SYSTEM));
    }

    XPrintableRef xPrint = (XPrintable*)xInt->queryInterface(XPrintable::getSmartUik());
    String aName = OUStringToString( xPrint->getPrinterName(), CHARSET_SYSTEM );
//  aTimeText.SetText(aName);

    xPrint->setPrinterName(L"HP5_2");
//  xPrint->setPrinterName(L"blubb");

//  XPropertySetRef xOptions;
//  xPrint->print(xOptions);

/*  XViewPaneRef xPane = (XViewPane*)xInt->queryInterface(XViewPane::getSmartUik());
    if (!xPane) return 0;
    xPane->setScrollRow( 2 );
*/

    XCellRangeSourceRef xSrc = (XCellRangeSource*)
                                    xInt->queryInterface(XCellRangeSource::getSmartUik());
    if (!xSrc) return;
    XCellRangeRef xRange = xSrc->getReferredCells();
    if (!xRange) return;
    XCellCollectionRef xColl = (XCellCollection*)
                                    xRange->queryInterface(XCellCollection::getSmartUik());
    if (!xColl) return;

    XActionLockableRef xLock = (XActionLockable*)
                        xDoc->queryInterface(XActionLockable::getSmartUik());
    if (xLock)
        xLock->addActionLock();                         // don't paint in between

//  lcl_FillCells(xColl);

    if (xLock)
        xLock->removeActionLock();                      // don't paint in between

    XStarCalcViewRef xCalc = (XStarCalcView*)xInt->queryInterface(XStarCalcView::getSmartUik());
    if (!xCalc) return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
