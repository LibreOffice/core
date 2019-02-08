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

#include <vbahelper/helperdecl.hxx>
#include <cppuhelper/queryinterface.hxx>

#include "vbaworksheet.hxx"
#include "vbanames.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XSheetCellCursor.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSheetPastable.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <ooo/vba/excel/XApplication.hpp>
#include <ooo/vba/excel/XlEnableSelection.hpp>
#include <ooo/vba/excel/XlSheetVisibility.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <ooo/vba/XControlProvider.hpp>

#include <basic/sberrors.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <vbahelper/vbashapes.hxx>

//zhangyun showdataform
#include <sfx2/sfxdlg.hxx>
#include <scabstdlg.hxx>
#include <tabvwsh.hxx>
#include <scitems.hxx>

#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>

#include <cellsuno.hxx>
#include <drwlayer.hxx>
#include <tabprotection.hxx>
#include <scextopt.hxx>
#include "excelvbahelper.hxx"
#include "service.hxx"
#include "vbaoutline.hxx"
#include "vbarange.hxx"
#include "vbacomments.hxx"
#include "vbachartobjects.hxx"
#include "vbapivottables.hxx"
#include "vbaoleobject.hxx"
#include "vbaoleobjects.hxx"
#include "vbapagesetup.hxx"
#include "vbapagebreaks.hxx"
#include "vbaworksheets.hxx"
#include "vbahyperlinks.hxx"
#include "vbasheetobjects.hxx"
#include <markdata.hxx>
#include <dbdata.hxx>

#include <attrib.hxx>

#define STANDARDWIDTH 2267
#define STANDARDHEIGHT 427

using namespace com::sun::star;
using namespace ooo::vba;

static void getNewSpreadsheetName (OUString &aNewName, const OUString& aOldName, const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( "getNewSpreadsheetName() xSpreadDoc is null", uno::Reference< uno::XInterface  >(), 1 );
    static const char aUnderScore[] =  "_";
    int currentNum =2;
    aNewName = aOldName + aUnderScore + OUString::number(currentNum) ;
    SCTAB nTab = 0;
    while ( ScVbaWorksheets::nameExists(xSpreadDoc,aNewName, nTab ) )
    {
        aNewName = aOldName + aUnderScore + OUString::number(++currentNum);
    }
}

static void removeAllSheets( const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, const OUString& aSheetName)
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( "removeAllSheets() xSpreadDoc is null", uno::Reference< uno::XInterface  >(), 1 );
    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );

    if ( xIndex.is() )
    {
        uno::Reference<container::XNameContainer> xNameContainer(xSheets,uno::UNO_QUERY_THROW);
        for (sal_Int32 i = xIndex->getCount() -1; i>= 1; i--)
        {
            uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            xNameContainer->removeByName(xNamed->getName());
        }

        uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
        xNamed->setName(aSheetName);
    }
}

static uno::Reference<frame::XModel>
openNewDoc(const OUString& aSheetName )
{
    uno::Reference<frame::XModel> xModel;
    try
    {
        uno::Reference< uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );

        uno::Reference <frame::XDesktop2 > xComponentLoader = frame::Desktop::create(xContext);

        uno::Reference<lang::XComponent > xComponent( xComponentLoader->loadComponentFromURL(
                "private:factory/scalc",
                "_blank", 0,
                uno::Sequence < css::beans::PropertyValue >() ) );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xComponent, uno::UNO_QUERY_THROW );
        removeAllSheets(xSpreadDoc,aSheetName);
        xModel.set(xSpreadDoc,uno::UNO_QUERY_THROW);
    }
    catch ( uno::Exception & /*e*/ )
    {
    }
    return xModel;
}

ScVbaWorksheet::ScVbaWorksheet(const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< sheet::XSpreadsheet >& xSheet,
        const uno::Reference< frame::XModel >& xModel ) : WorksheetImpl_BASE( xParent, xContext ), mxSheet( xSheet ), mxModel(xModel), mbVeryHidden( false )
{
}

ScVbaWorksheet::ScVbaWorksheet( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) :  WorksheetImpl_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext ), mxModel( getXSomethingFromArgs< frame::XModel >( args, 1 ) ), mbVeryHidden( false )
{
    if ( args.getLength() < 3 )
        throw lang::IllegalArgumentException();

    OUString sSheetName;
    args[2] >>= sSheetName;

    uno::Reference< sheet::XSpreadsheetDocument > xSpreadDoc( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xNameAccess( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    mxSheet.set( xNameAccess->getByName( sSheetName ), uno::UNO_QUERY_THROW );
}

ScVbaWorksheet::~ScVbaWorksheet()
{
}

namespace
{
    class theScVbaWorksheetUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theScVbaWorksheetUnoTunnelId > {};
}

const uno::Sequence<sal_Int8>& ScVbaWorksheet::getUnoTunnelId()
{
    return theScVbaWorksheetUnoTunnelId::get().getSeq();
}

uno::Reference< ov::excel::XWorksheet >
ScVbaWorksheet::createSheetCopyInNewDoc(const OUString& aCurrSheetName)
{
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = getSheet()->createCursor( );
    uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
    uno::Reference< table::XCellRange > xRange1( xSheetCellCursor, uno::UNO_QUERY);
    uno::Reference<excel::XRange> xRange =  new ScVbaRange( this, mxContext, xRange1);
    if (xRange.is())
        xRange->Select();
    excel::implnCopy(mxModel);
    uno::Reference<frame::XModel> xModel = openNewDoc(aCurrSheetName);
    if (xModel.is())
    {
        excel::implnPaste(xModel);
    }
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY_THROW );
    excel::setUpDocumentModules(xSpreadDoc);
    uno::Reference <sheet::XSpreadsheets> xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    ScDocShell* pShell = excel::getDocShell( xModel );
    OUString aCodeName;
    pShell->GetDocument().GetCodeName( 0, aCodeName );
    return uno::Reference< excel::XWorksheet >( getUnoDocModule( aCodeName, pShell ), uno::UNO_QUERY_THROW );
}

css::uno::Reference< ov::excel::XWorksheet >
ScVbaWorksheet::createSheetCopy(uno::Reference<excel::XWorksheet> const & xSheet, bool bAfter)
{
    OUString aCurrSheetName = getName();
    ScVbaWorksheet* pDestSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSheet );

    uno::Reference <sheet::XSpreadsheetDocument> xDestDoc( pDestSheet->getModel(), uno::UNO_QUERY );
    uno::Reference <sheet::XSpreadsheetDocument> xSrcDoc( getModel(), uno::UNO_QUERY );

    SCTAB nDest = 0;
    SCTAB nSrc = 0;
    OUString aSheetName = xSheet->getName();
    bool bSameDoc = ( pDestSheet->getModel() == getModel() );
    bool bDestSheetExists = ScVbaWorksheets::nameExists (xDestDoc, aSheetName, nDest );
    bool bSheetExists = ScVbaWorksheets::nameExists (xSrcDoc, aCurrSheetName, nSrc );

    // set sheet name to be newSheet name
    aSheetName = aCurrSheetName;
    if ( bSheetExists && bDestSheetExists )
    {
        SCTAB nDummy=0;
        if(bAfter)
              nDest++;
        uno::Reference<sheet::XSpreadsheets> xSheets = xDestDoc->getSheets();
        if ( bSameDoc || ScVbaWorksheets::nameExists( xDestDoc, aCurrSheetName, nDummy ) )
            getNewSpreadsheetName(aSheetName,aCurrSheetName,xDestDoc);
        if ( bSameDoc )
            xSheets->copyByName(aCurrSheetName,aSheetName,nDest);
        else
        {
            ScDocShell* pDestDocShell = excel::getDocShell( pDestSheet->getModel() );
            ScDocShell* pSrcDocShell = excel::getDocShell( getModel() );
            if ( pDestDocShell && pSrcDocShell )
                pDestDocShell->TransferTab( *pSrcDocShell, nSrc, nDest, true, true );
        }
    }
    // return new sheet
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XWorksheet > xNewSheet( xApplication->Worksheets( uno::makeAny( aSheetName ) ), uno::UNO_QUERY_THROW );
    return xNewSheet;
}

OUString
ScVbaWorksheet::getName()
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    return xNamed->getName();
}

void
ScVbaWorksheet::setName(const OUString &rName )
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
}

sal_Int32
ScVbaWorksheet::getVisible()
{
    uno::Reference< beans::XPropertySet > xProps( getSheet(), uno::UNO_QUERY_THROW );
    bool bVisible = false;
    xProps->getPropertyValue( "IsVisible" ) >>= bVisible;
    using namespace ::ooo::vba::excel::XlSheetVisibility;
    return bVisible ? xlSheetVisible : (mbVeryHidden ? xlSheetVeryHidden : xlSheetHidden);
}

void
ScVbaWorksheet::setVisible( sal_Int32 nVisible )
{
    using namespace ::ooo::vba::excel::XlSheetVisibility;
    bool bVisible = true;
    switch( nVisible )
    {
        case xlSheetVisible: case 1:  // Excel accepts -1 and 1 for visible sheets
            bVisible = true;
            mbVeryHidden = false;
        break;
        case xlSheetHidden:
            bVisible = false;
            mbVeryHidden = false;
        break;
        case xlSheetVeryHidden:
            bVisible = false;
            mbVeryHidden = true;
        break;
        default:
            throw uno::RuntimeException();
    }
    uno::Reference< beans::XPropertySet > xProps( getSheet(), uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( "IsVisible", uno::Any( bVisible ) );
}

sal_Int16
ScVbaWorksheet::getIndex()
{
    return getSheetID() + 1;
}

sal_Int32
ScVbaWorksheet::getEnableSelection()
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nTab = 0;
    if ( !ScVbaWorksheets::nameExists(xSpreadDoc, getName(), nTab) )
        throw uno::RuntimeException("Sheet Name does not exist." );

    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
    ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    bool bLockedCells = false;
    bool bUnlockedCells = false;
    if( pProtect )
    {
        bLockedCells   = pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bUnlockedCells = pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }
    if( bLockedCells )
        return excel::XlEnableSelection::xlNoRestrictions;
    if( bUnlockedCells )
        return excel::XlEnableSelection::xlUnlockedCells;
    return excel::XlEnableSelection::xlNoSelection;

}

void
ScVbaWorksheet::setEnableSelection( sal_Int32 nSelection )
{
    if( (nSelection != excel::XlEnableSelection::xlNoRestrictions) &&
        (nSelection != excel::XlEnableSelection::xlUnlockedCells) &&
        (nSelection != excel::XlEnableSelection::xlNoSelection) )
    {
        DebugHelper::runtimeexception(ERRCODE_BASIC_BAD_PARAMETER);
    }

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nTab = 0;
    if ( !ScVbaWorksheets::nameExists(xSpreadDoc, getName(), nTab) )
        throw uno::RuntimeException("Sheet Name does not exist." );

    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
    ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    // default is xlNoSelection
    bool bLockedCells = false;
    bool bUnlockedCells = false;
    if( nSelection == excel::XlEnableSelection::xlNoRestrictions )
    {
        bLockedCells = true;
        bUnlockedCells = true;
    }
    else if( nSelection == excel::XlEnableSelection::xlUnlockedCells )
    {
        bUnlockedCells = true;
    }
    if( pProtect )
    {
        pProtect->setOption( ScTableProtection::SELECT_LOCKED_CELLS, bLockedCells );
        pProtect->setOption( ScTableProtection::SELECT_UNLOCKED_CELLS, bUnlockedCells );
    }


}

sal_Bool SAL_CALL ScVbaWorksheet::getAutoFilterMode()
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
    ScDBData* pDBData = rDoc.GetAnonymousDBData(getSheetID());
    if (pDBData)
        return pDBData->HasAutoFilter();
    return false;
}

void SAL_CALL ScVbaWorksheet::setAutoFilterMode( sal_Bool bAutoFilterMode )
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocShell* pDocShell = excel::getDocShell( xModel );
    ScDocument& rDoc = pDocShell->GetDocument();
    ScDBData* pDBData = rDoc.GetAnonymousDBData(getSheetID());
    if (pDBData)
    {
        pDBData->SetAutoFilter(bAutoFilterMode);
        ScRange aRange;
        pDBData->GetArea(aRange);
        if (bAutoFilterMode)
            rDoc.ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aStart.Row(),
                                    aRange.aStart.Tab(), ScMF::Auto );
        else if (!bAutoFilterMode)
            rDoc.RemoveFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aStart.Row(),
                                    aRange.aStart.Tab(), ScMF::Auto );
        ScRange aPaintRange(aRange.aStart, aRange.aEnd);
        aPaintRange.aEnd.SetRow(aPaintRange.aStart.Row());
        pDocShell->PostPaint(aPaintRange, PaintPartFlags::Grid);
    }
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getUsedRange()
{
    uno::Reference< sheet::XSheetCellRange > xSheetCellRange(getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor( getSheet()->createCursorByRange( xSheetCellRange ), uno::UNO_QUERY_THROW );
    uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
    xUsedCursor->gotoStartOfUsedArea( false );
    xUsedCursor->gotoEndOfUsedArea( true );
    uno::Reference< table::XCellRange > xRange( xSheetCellCursor, uno::UNO_QUERY);
    return new ScVbaRange(this, mxContext, xRange);
}

uno::Reference< excel::XOutline >
ScVbaWorksheet::Outline( )
{
    uno::Reference<sheet::XSheetOutline> xOutline(getSheet(),uno::UNO_QUERY_THROW);
    return new ScVbaOutline( this, mxContext, xOutline);
}

uno::Reference< excel::XPageSetup >
ScVbaWorksheet::PageSetup( )
{
    return new ScVbaPageSetup( this, mxContext, getSheet(), getModel() );
}

uno::Any
ScVbaWorksheet::HPageBreaks( const uno::Any& aIndex )
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak(getSheet(),uno::UNO_QUERY_THROW);
    uno::Reference< excel::XHPageBreaks > xHPageBreaks( new ScVbaHPageBreaks( this, mxContext, xSheetPageBreak));
    if ( aIndex.hasValue() )
        return xHPageBreaks->Item( aIndex, uno::Any());
    return uno::makeAny( xHPageBreaks );
}

uno::Any
ScVbaWorksheet::VPageBreaks( const uno::Any& aIndex )
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak( getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XVPageBreaks > xVPageBreaks( new ScVbaVPageBreaks( this, mxContext, xSheetPageBreak ) );
    if( aIndex.hasValue() )
        return xVPageBreaks->Item( aIndex, uno::Any());
    return uno::makeAny( xVPageBreaks );
}

sal_Int32
ScVbaWorksheet::getStandardWidth()
{
    return STANDARDWIDTH ;
}

sal_Int32
ScVbaWorksheet::getStandardHeight()
{
    return STANDARDHEIGHT;
}

sal_Bool
ScVbaWorksheet::getProtectionMode()
{
    return false;
}

sal_Bool
ScVbaWorksheet::getProtectContents()
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    return xProtectable->isProtected();
}

sal_Bool
ScVbaWorksheet::getProtectDrawingObjects()
{
    SCTAB nTab = 0;
    OUString aSheetName = getName();
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    bool bSheetExists = ScVbaWorksheets::nameExists (xSpreadDoc, aSheetName, nTab);
    if ( bSheetExists )
    {
        uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
        ScDocument& rDoc = excel::getDocShell( xModel )->GetDocument();
        ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
        if ( pProtect )
            return pProtect->isOptionEnabled( ScTableProtection::OBJECTS );
    }
    return false;
}

sal_Bool
ScVbaWorksheet::getProtectScenarios()
{
    return false;
}

void
ScVbaWorksheet::Activate()
{
    uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
            getModel()->getCurrentController(), uno::UNO_QUERY_THROW );
    xSpreadsheet->setActiveSheet(getSheet());
}

void
ScVbaWorksheet::Select()
{
    Activate();
}

void
ScVbaWorksheet::Move( const uno::Any& Before, const uno::Any& After )
{
    uno::Reference<excel::XWorksheet> xSheet;
    OUString aCurrSheetName = getName();

    if (!(Before >>= xSheet) && !(After >>=xSheet)&& !(Before.hasValue()) && !(After.hasValue()))
    {
        uno::Reference< sheet::XSheetCellCursor > xSheetCellCursor = getSheet()->createCursor( );
        uno::Reference<sheet::XUsedAreaCursor> xUsedCursor(xSheetCellCursor,uno::UNO_QUERY_THROW);
        uno::Reference< table::XCellRange > xRange1( xSheetCellCursor, uno::UNO_QUERY);
        // #FIXME needs worksheet as parent
        uno::Reference<excel::XRange> xRange =  new ScVbaRange( this, mxContext, xRange1);
        if (xRange.is())
            xRange->Select();
        excel::implnCopy(mxModel);
        uno::Reference<frame::XModel> xModel = openNewDoc(aCurrSheetName);
        if (xModel.is())
        {
            excel::implnPaste(xModel);
            Delete();
        }
        return ;
    }

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nDest = 0;
    if ( ScVbaWorksheets::nameExists (xSpreadDoc, xSheet->getName(), nDest) )
    {
        bool bAfter = After.hasValue();
        if (bAfter)
            nDest++;
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        xSheets->moveByName(aCurrSheetName,nDest);
    }
}

void
ScVbaWorksheet::Copy( const uno::Any& Before, const uno::Any& After )
{
    uno::Reference<excel::XWorksheet> xSheet;
    if (!(Before >>= xSheet) && !(After >>=xSheet)&& !(Before.hasValue()) && !(After.hasValue()))
    {
        createSheetCopyInNewDoc(getName());
        return;
    }

    uno::Reference<excel::XWorksheet> xNewSheet = createSheetCopy(xSheet, After.hasValue());
    xNewSheet->Activate();
}

void
ScVbaWorksheet::Paste( const uno::Any& Destination, const uno::Any& /*Link*/ )
{
    // #TODO# #FIXME# Link is not used
    uno::Reference<excel::XRange> xRange( Destination, uno::UNO_QUERY );
    if ( xRange.is() )
        xRange->Select();
    excel::implnPaste( mxModel );
}

void
ScVbaWorksheet::Delete()
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    OUString aSheetName = getName();
    SCTAB nTab = 0;
    if (!ScVbaWorksheets::nameExists(xSpreadDoc, aSheetName, nTab ))
    {
        return;
    }
    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
    uno::Reference<container::XNameContainer> xNameContainer(xSheets,uno::UNO_QUERY_THROW);
    xNameContainer->removeByName(aSheetName);
    mxSheet.clear();
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getSheetAtOffset(SCTAB offset)
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    uno::Reference <sheet::XSpreadsheets> xSheets( xSpreadDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY_THROW );

    SCTAB nIdx = 0;
    if ( !ScVbaWorksheets::nameExists (xSpreadDoc, getName(), nIdx ) )
        return uno::Reference< excel::XWorksheet >();
    nIdx = nIdx + offset;
    uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(nIdx), uno::UNO_QUERY_THROW);
    // parent will be the parent of 'this' worksheet
    return new ScVbaWorksheet (getParent(), mxContext, xSheet, getModel());
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getNext()
{
    return getSheetAtOffset(static_cast<SCTAB>(1));
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getPrevious()
{
    return getSheetAtOffset(-1);
}

void
ScVbaWorksheet::Protect( const uno::Any& Password, const uno::Any& /*DrawingObjects*/, const uno::Any& /*Contents*/, const uno::Any& /*Scenarios*/, const uno::Any& /*UserInterfaceOnly*/ )
{
    // #TODO# #FIXME# is there anything we can do with the unused param
    // can the implementation use anything else here
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->protect( aPasswd );
}

void
ScVbaWorksheet::Unprotect( const uno::Any& Password )
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->unprotect( aPasswd );
}

void
ScVbaWorksheet::Calculate()
{
    uno::Reference <sheet::XCalculatable> xReCalculate(getModel(), uno::UNO_QUERY_THROW);
    xReCalculate->calculate();
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Range( const ::uno::Any& Cell1, const ::uno::Any& Cell2 )
{
    uno::Reference< excel::XRange > xSheetRange( new ScVbaRange( this, mxContext
, uno::Reference< table::XCellRange >( getSheet(), uno::UNO_QUERY_THROW ) ) );
    return xSheetRange->Range( Cell1, Cell2 );
}

void
ScVbaWorksheet::CheckSpelling( const uno::Any& /*CustomDictionary*/,const uno::Any& /*IgnoreUppercase*/,const uno::Any& /*AlwaysSuggest*/, const uno::Any& /*SpellingLang*/ )
{
    // #TODO# #FIXME# unused params above, can we do anything with those
    OUString url = ".uno:SpellDialog";
    uno::Reference< frame::XModel > xModel( getModel() );
    dispatchRequests(xModel,url);
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getSheetRange()
{
    uno::Reference< table::XCellRange > xRange( getSheet(),uno::UNO_QUERY_THROW );
    return uno::Reference< excel::XRange >( new ScVbaRange( this, mxContext, xRange ) );
}

// These are hacks - we prolly (somehow) need to inherit
// the vbarange functionality here ...
uno::Reference< excel::XRange >
ScVbaWorksheet::Cells( const ::uno::Any &nRow, const ::uno::Any &nCol )
{
    // Performance optimization for often-called Cells method:
    // Use a common helper method instead of creating a new ScVbaRange object
    uno::Reference< table::XCellRange > xRange( getSheet(), uno::UNO_QUERY_THROW );
    return ScVbaRange::CellsHelper( this, mxContext, xRange, nRow, nCol );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Rows(const uno::Any& aIndex )
{
    return getSheetRange()->Rows( aIndex );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Columns( const uno::Any& aIndex )
{
    return getSheetRange()->Columns( aIndex );
}

uno::Any SAL_CALL
ScVbaWorksheet::ChartObjects( const uno::Any& Index )
{
    if ( !mxCharts.is() )
    {
        uno::Reference< table::XTableChartsSupplier > xChartSupplier( getSheet(), uno::UNO_QUERY_THROW );
        uno::Reference< table::XTableCharts > xTableCharts = xChartSupplier->getCharts();

        uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( mxSheet, uno::UNO_QUERY_THROW );
        mxCharts = new ScVbaChartObjects(  this, mxContext, xTableCharts, xDrawPageSupplier );
    }
    if ( Index.hasValue() )
    {
        uno::Reference< XCollection > xColl( mxCharts, uno::UNO_QUERY_THROW );
        return xColl->Item( Index, uno::Any() );
    }
    else
        return uno::makeAny( mxCharts );

}

uno::Any SAL_CALL
ScVbaWorksheet::PivotTables( const uno::Any& Index )
{
    uno::Reference< css::sheet::XSpreadsheet > xSheet = getSheet();
    uno::Reference< sheet::XDataPilotTablesSupplier > xTables(xSheet, uno::UNO_QUERY_THROW ) ;
    uno::Reference< container::XIndexAccess > xIndexAccess( xTables->getDataPilotTables(), uno::UNO_QUERY_THROW );

    uno::Reference< XCollection > xColl(  new ScVbaPivotTables( this, mxContext, xIndexAccess ) );
    if ( Index.hasValue() )
        return xColl->Item( Index, uno::Any() );
    return uno::makeAny( xColl );
}

uno::Any SAL_CALL
ScVbaWorksheet::Comments( const uno::Any& Index )
{
    uno::Reference< css::sheet::XSpreadsheet > xSheet = getSheet();
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnosSupp( xSheet, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSheetAnnotations > xAnnos( xAnnosSupp->getAnnotations(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xAnnos, uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xColl(  new ScVbaComments( this, mxContext, mxModel, xIndexAccess ) );
    if ( Index.hasValue() )
        return xColl->Item( Index, uno::Any() );
    return uno::makeAny( xColl );
}

uno::Any SAL_CALL
ScVbaWorksheet::Hyperlinks( const uno::Any& aIndex )
{
    /*  The worksheet always returns the same Hyperlinks object.
        See vbahyperlinks.hxx for more details. */
    if( !mxHlinks.is() )
        mxHlinks.set( new ScVbaHyperlinks( this, mxContext ) );
    if( aIndex.hasValue() )
        return uno::Reference< XCollection >( mxHlinks, uno::UNO_QUERY_THROW )->Item( aIndex, uno::Any() );
    return uno::Any( mxHlinks );
}

uno::Any SAL_CALL
ScVbaWorksheet::Names( const css::uno::Any& aIndex )
{
    // fake sheet-local names by returning all global names
    // #163498# initialize Names object with correct parent (this worksheet)
    // TODO: real sheet-local names...
    uno::Reference< beans::XPropertySet > xProps( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges(  xProps->getPropertyValue("NamedRanges"), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xNames( new ScVbaNames( this, mxContext, xNamedRanges, mxModel ) );
    if ( aIndex.hasValue() )
        return xNames->Item( aIndex, uno::Any() );
    return uno::Any( xNames );
}

uno::Any SAL_CALL
ScVbaWorksheet::OLEObjects( const uno::Any& Index )
{
    uno::Reference< sheet::XSpreadsheet > xSpreadsheet( getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xSpreadsheet, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPage, uno::UNO_QUERY_THROW );

    uno::Reference< excel::XOLEObjects >xOleObjects( new ScVbaOLEObjects( this, mxContext, xIndexAccess ) );
    if( Index.hasValue() )
        return xOleObjects->Item( Index, uno::Any() );
    return uno::Any( xOleObjects );
}

uno::Any SAL_CALL
ScVbaWorksheet::Shapes( const uno::Any& aIndex )
{
    uno::Reference< sheet::XSpreadsheet > xSpreadsheet( getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xSpreadsheet, uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShapes > xShapes( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xShapes, uno::UNO_QUERY_THROW );

    uno::Reference< msforms::XShapes> xVbaShapes( new ScVbaShapes( this, mxContext, xIndexAccess, getModel() ) );
    if ( aIndex.hasValue() )
        return xVbaShapes->Item( aIndex, uno::Any() );
    return uno::makeAny( xVbaShapes );
}

uno::Any
ScVbaWorksheet::getButtons( const uno::Any &rIndex, bool bOptionButtons )
{
    ::rtl::Reference< ScVbaSheetObjectsBase > &rxButtons = bOptionButtons ? mxButtons[0] : mxButtons[1];

    if( !rxButtons.is() )
        rxButtons.set( new ScVbaButtons( this, mxContext, mxModel, mxSheet, bOptionButtons ) );
    else
        rxButtons->collectShapes();
    if( rIndex.hasValue() )
        return rxButtons->Item( rIndex, uno::Any() );
    return uno::Any( uno::Reference< XCollection >( rxButtons.get() ) );
}

uno::Any SAL_CALL
ScVbaWorksheet::Buttons( const uno::Any& rIndex )
{
    return getButtons( rIndex, false );
}

uno::Any SAL_CALL
ScVbaWorksheet::CheckBoxes( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::DropDowns( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::GroupBoxes( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::Labels( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::ListBoxes( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::OptionButtons( const uno::Any& rIndex )
{
    return getButtons( rIndex, true );
}

uno::Any SAL_CALL
ScVbaWorksheet::ScrollBars( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::Spinners( const uno::Any& /*rIndex*/ )
{
    throw uno::RuntimeException();
}

void SAL_CALL
ScVbaWorksheet::ShowDataForm( )
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScTabViewShell* pTabViewShell = excel::getBestViewShell( xModel );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

    ScopedVclPtr<AbstractScDataFormDlg> pDlg(pFact->CreateScDataFormDlg(pTabViewShell->GetDialogParent(),
                                                                        pTabViewShell));

    pDlg->Execute();
}

uno::Any SAL_CALL
ScVbaWorksheet::Evaluate( const OUString& Name )
{
    // #TODO Evaluate allows other things to be evaluated, e.g. functions
    // I think ( like SIN(3) etc. ) need to investigate that
    // named Ranges also? e.g. [MyRange] if so need a list of named ranges
    uno::Any aVoid;
    return uno::Any( Range( uno::Any( Name ), aVoid ) );
}

uno::Reference< beans::XIntrospectionAccess > SAL_CALL
ScVbaWorksheet::getIntrospection(  )
{
    return uno::Reference< beans::XIntrospectionAccess >();
}

uno::Any SAL_CALL
ScVbaWorksheet::invoke( const OUString& /*aFunctionName*/, const uno::Sequence< uno::Any >& /*aParams*/, uno::Sequence< ::sal_Int16 >& /*aOutParamIndex*/, uno::Sequence< uno::Any >& /*aOutParam*/ )
{
    throw uno::RuntimeException("Unsupported"); // unsupported operation
}

void SAL_CALL
ScVbaWorksheet::setValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    setDefaultPropByIntrospection( getValue( aPropertyName ), aValue );
}
uno::Any SAL_CALL
ScVbaWorksheet::getValue( const OUString& aPropertyName )
{
    uno::Reference< drawing::XControlShape > xControlShape( getControlShape( aPropertyName ), uno::UNO_QUERY_THROW );

    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext("ooo.vba.ControlProvider", mxContext ), uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xControl( xControlProvider->createControl(  xControlShape, getModel() ) );
    return uno::makeAny( xControl );
}

sal_Bool SAL_CALL
ScVbaWorksheet::hasMethod( const OUString& /*aName*/ )
{
    return false;
}

uno::Reference< container::XNameAccess >
ScVbaWorksheet::getFormControls()
{
    uno::Reference< container::XNameAccess > xFormControls;
    try
    {
        uno::Reference< sheet::XSpreadsheet > xSpreadsheet( getSheet(), uno::UNO_QUERY_THROW );
        uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xSpreadsheet, uno::UNO_QUERY_THROW );
        uno::Reference< form::XFormsSupplier >  xFormSupplier( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );
        uno::Reference< container::XIndexAccess > xIndexAccess( xFormSupplier->getForms(), uno::UNO_QUERY_THROW );
        // get the www-standard container ( maybe we should access the
        // 'www-standard' by name rather than index, this seems an
        // implementation detail
        if( xIndexAccess->hasElements() )
            xFormControls.set( xIndexAccess->getByIndex(0), uno::UNO_QUERY );

    }
    catch( uno::Exception& )
    {
    }
    return xFormControls;

                }
sal_Bool SAL_CALL
ScVbaWorksheet::hasProperty( const OUString& aName )
{
    uno::Reference< container::XNameAccess > xFormControls( getFormControls() );
    if ( xFormControls.is() )
        return xFormControls->hasByName( aName );
    return false;
}

uno::Any
ScVbaWorksheet::getControlShape( const OUString& sName )
{
    // ideally we would get an XControl object but it appears an XControl
    // implementation only exists for a Control implementation obtained from the
    // view ( e.g. in basic you would get this from
    // thiscomponent.currentcontroller.getControl( controlModel ) )
    // and the thing to realise is that it is only possible to get an XControl
    // for a currently displayed control :-( often we would want to modify
    // a control not on the active sheet. But.. you can always access the
    // XControlShape from the DrawPage whether that is the active drawpage or not

    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndexAccess( xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW );

    sal_Int32 nCount = xIndexAccess->getCount();
    for( int index = 0; index < nCount; index++ )
    {
        uno::Any aUnoObj =  xIndexAccess->getByIndex( index );
         // It seems there are some drawing objects that can not query into Control shapes?
        uno::Reference< drawing::XControlShape > xControlShape( aUnoObj, uno::UNO_QUERY );
        if( xControlShape.is() )
        {
            uno::Reference< container::XNamed > xNamed( xControlShape->getControl(), uno::UNO_QUERY_THROW );
            if( sName == xNamed->getName() )
            {
                return aUnoObj;
            }
        }
    }
    return uno::Any();
}

OUString
ScVbaWorksheet::getServiceImplName()
{
    return OUString("ScVbaWorksheet");
}

void SAL_CALL
ScVbaWorksheet::setEnableCalculation( sal_Bool bEnableCalculation )
{
    uno::Reference <sheet::XCalculatable> xCalculatable(getModel(), uno::UNO_QUERY_THROW);
    xCalculatable->enableAutomaticCalculation( bEnableCalculation);
}
sal_Bool SAL_CALL
ScVbaWorksheet::getEnableCalculation(  )
{
    uno::Reference <sheet::XCalculatable> xCalculatable(getModel(), uno::UNO_QUERY_THROW);
    return xCalculatable->isAutomaticCalculationEnabled();
}

uno::Sequence< OUString >
ScVbaWorksheet::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Worksheet"
    };
    return aServiceNames;
}

OUString SAL_CALL
ScVbaWorksheet::getCodeName()
{
    uno::Reference< beans::XPropertySet > xSheetProp( mxSheet, uno::UNO_QUERY_THROW );
    return xSheetProp->getPropertyValue("CodeName").get< OUString >();
}

sal_Int16
ScVbaWorksheet::getSheetID()
{
    uno::Reference< sheet::XCellRangeAddressable > xAddressable( mxSheet, uno::UNO_QUERY_THROW ); // if ActiveSheet, mxSheet is null.
    return xAddressable->getRangeAddress().Sheet;
}

void SAL_CALL
ScVbaWorksheet::PrintOut( const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& ActivePrinter, const uno::Any& PrintToFile, const uno::Any& Collate, const uno::Any& PrToFileName, const uno::Any& )
{
    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    bool bSelection = false;
    From >>= nFrom;
    To >>= nTo;

    if ( !( nFrom || nTo ) )
        bSelection = true;

    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    PrintOutHelper( excel::getBestViewShell( xModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, bSelection );
}

sal_Int64 SAL_CALL
ScVbaWorksheet::getSomething(const uno::Sequence<sal_Int8 > & rId)
{
    if (rId.getLength() == 16 &&
        0 == memcmp( ScVbaWorksheet::getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace worksheet
{
namespace sdecl = comphelper::service_decl;
sdecl::vba_service_class_<ScVbaWorksheet, sdecl::with_args<true> > const serviceImpl;
sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWorksheet",
    "ooo.vba.excel.Worksheet" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
