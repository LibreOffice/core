/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <ooo/vba/excel/XlEnableSelection.hpp>
#include <ooo/vba/excel/XlSheetVisibility.hpp>
#include <ooo/vba/excel/XWorkbook.hpp>
#include <ooo/vba/XControlProvider.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <vbahelper/vbashapes.hxx>

#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>

#include <tools/string.hxx>

//zhangyun showdataform
#include <sfx2/sfxdlg.hxx>
#include "scabstdlg.hxx"
#include "tabvwsh.hxx"
#include "scitems.hxx"

#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>

#include "cellsuno.hxx"
#include "drwlayer.hxx"
#include "tabprotection.hxx"
#include "scextopt.hxx"
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
#include "markdata.hxx"
#include "dbdata.hxx"

#include "attrib.hxx"

#define STANDARDWIDTH 2267
#define STANDARDHEIGHT 427

using namespace com::sun::star;
using namespace ooo::vba;

static void getNewSpreadsheetName (rtl::OUString &aNewName, rtl::OUString aOldName, uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc )
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getNewSpreadsheetName() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
    static rtl::OUString aUnderScre( RTL_CONSTASCII_USTRINGPARAM( "_" ) );
    int currentNum =2;
    aNewName = aOldName + aUnderScre+ String::CreateFromInt32(currentNum) ;
    SCTAB nTab = 0;
    while ( ScVbaWorksheets::nameExists(xSpreadDoc,aNewName, nTab ) )
    {
        aNewName = aOldName + aUnderScre +
        String::CreateFromInt32(++currentNum) ;
    }
}

static void removeAllSheets( uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc, rtl::OUString aSheetName)
{
    if (!xSpreadDoc.is())
        throw lang::IllegalArgumentException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "removeAllSheets() xSpreadDoc is null" ) ), uno::Reference< uno::XInterface  >(), 1 );
    uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
    uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );

    if ( xIndex.is() )
    {
        uno::Reference<container::XNameContainer> xNameContainer(xSheets,uno::UNO_QUERY_THROW);
        for (sal_Int32 i = xIndex->getCount() -1; i>= 1; i--)
        {
            uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(i), uno::UNO_QUERY);
            uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
            if (xNamed.is())
            {
                xNameContainer->removeByName(xNamed->getName());
            }
        }

        uno::Reference< sheet::XSpreadsheet > xSheet(xIndex->getByIndex(0), uno::UNO_QUERY);
        uno::Reference< container::XNamed > xNamed( xSheet, uno::UNO_QUERY_THROW );
        if (xNamed.is())
        {
            xNamed->setName(aSheetName);
        }
    }
}

static uno::Reference<frame::XModel>
openNewDoc(rtl::OUString aSheetName )
{
    uno::Reference<frame::XModel> xModel;
    try
    {
        uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< uno::XComponentContext > xContext(  xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY_THROW );
        uno::Reference<lang::XMultiComponentFactory > xServiceManager(
                                        xContext->getServiceManager(), uno::UNO_QUERY_THROW );

        uno::Reference <frame::XComponentLoader > xComponentLoader(
                        xServiceManager->createInstanceWithContext(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ),
                        xContext ), uno::UNO_QUERY_THROW );

        uno::Reference<lang::XComponent > xComponent( xComponentLoader->loadComponentFromURL(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:factory/scalc" ) ),
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ), 0,
                uno::Sequence < ::com::sun::star::beans::PropertyValue >() ) );
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xComponent, uno::UNO_QUERY_THROW );
        if ( xSpreadDoc.is() )
        {
            removeAllSheets(xSpreadDoc,aSheetName);
        }
        xModel.set(xSpreadDoc,uno::UNO_QUERY_THROW);
    }
    catch ( uno::Exception & /*e*/ )
    {
    }
    return xModel;
}

ScVbaWorksheet::ScVbaWorksheet(const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< sheet::XSpreadsheet >& xSheet,
        const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException) : WorksheetImpl_BASE( xParent, xContext ), mxSheet( xSheet ), mxModel(xModel)
{
}

ScVbaWorksheet::ScVbaWorksheet( uno::Sequence< uno::Any> const & args,
    uno::Reference< uno::XComponentContext> const & xContext ) throw ( lang::IllegalArgumentException ) :  WorksheetImpl_BASE( getXSomethingFromArgs< XHelperInterface >( args, 0 ), xContext ), mxModel( getXSomethingFromArgs< frame::XModel >( args, 1 ) ), mbVeryHidden( false )
{
    if ( args.getLength() < 3 )
        throw lang::IllegalArgumentException();

    rtl::OUString sSheetName;
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
ScVbaWorksheet::createSheetCopyInNewDoc(rtl::OUString aCurrSheetName)
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
    rtl::OUString aCodeName;
    pShell->GetDocument()->GetCodeName( 0, aCodeName );
    return uno::Reference< excel::XWorksheet >( getUnoDocModule( aCodeName, pShell ), uno::UNO_QUERY_THROW );
}

css::uno::Reference< ov::excel::XWorksheet >
ScVbaWorksheet::createSheetCopy(uno::Reference<excel::XWorksheet> xSheet, bool bAfter)
{
    rtl::OUString aCurrSheetName = getName();
    ScVbaWorksheet* pDestSheet = excel::getImplFromDocModuleWrapper<ScVbaWorksheet>( xSheet );

    uno::Reference <sheet::XSpreadsheetDocument> xDestDoc( pDestSheet->getModel(), uno::UNO_QUERY );
    uno::Reference <sheet::XSpreadsheetDocument> xSrcDoc( getModel(), uno::UNO_QUERY );

    SCTAB nDest = 0;
    SCTAB nSrc = 0;
    rtl::OUString aSheetName = xSheet->getName();
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
                pDestDocShell->TransferTab( *pSrcDocShell, static_cast<SCTAB>(nSrc), static_cast<SCTAB>(nDest), true, true );
        }
    }
    // return new sheet
    uno::Reference< excel::XApplication > xApplication( Application(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XWorksheet > xNewSheet( xApplication->Worksheets( uno::makeAny( aSheetName ) ), uno::UNO_QUERY_THROW );
    return xNewSheet;
}

::rtl::OUString
ScVbaWorksheet::getName() throw (uno::RuntimeException)
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    return xNamed->getName();
}

void
ScVbaWorksheet::setName(const ::rtl::OUString &rName ) throw (uno::RuntimeException)
{
    uno::Reference< container::XNamed > xNamed( getSheet(), uno::UNO_QUERY_THROW );
    xNamed->setName( rName );
}

sal_Int32
ScVbaWorksheet::getVisible() throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( getSheet(), uno::UNO_QUERY_THROW );
    bool bVisible = false;
    xProps->getPropertyValue( "IsVisible" ) >>= bVisible;
    using namespace ::ooo::vba::excel::XlSheetVisibility;
    return bVisible ? xlSheetVisible : (mbVeryHidden ? xlSheetVeryHidden : xlSheetHidden);
}

void
ScVbaWorksheet::setVisible( sal_Int32 nVisible ) throw (uno::RuntimeException)
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
ScVbaWorksheet::getIndex() throw (uno::RuntimeException)
{
    return getSheetID() + 1;
}

sal_Int32
ScVbaWorksheet::getEnableSelection() throw (uno::RuntimeException)
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nTab = 0;
    if ( ScVbaWorksheets::nameExists(xSpreadDoc, getName(), nTab) )
    {
        uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
        ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
        ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
        sal_Bool bLockedCells = false;
        sal_Bool bUnlockedCells = false;
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
    else
        throw uno::RuntimeException(::rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM( "Sheet Name does not exist. ") ),
                                uno::Reference< XInterface >() );
}


void
ScVbaWorksheet::setEnableSelection( sal_Int32 nSelection ) throw (uno::RuntimeException)
{
    if( (nSelection != excel::XlEnableSelection::xlNoRestrictions) &&
        (nSelection != excel::XlEnableSelection::xlUnlockedCells) &&
        (nSelection != excel::XlEnableSelection::xlNoSelection) )
    {
        DebugHelper::exception(SbERR_BAD_PARAMETER, rtl::OUString() );
    }

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    SCTAB nTab = 0;
    if ( ScVbaWorksheets::nameExists(xSpreadDoc, getName(), nTab) )
    {
        uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
        ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
        ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
        // default is xlNoSelection
        sal_Bool bLockedCells = false;
        sal_Bool bUnlockedCells = false;
        if( nSelection == excel::XlEnableSelection::xlNoRestrictions )
        {
            bLockedCells = sal_True;
            bUnlockedCells = sal_True;
        }
        else if( nSelection == excel::XlEnableSelection::xlUnlockedCells )
        {
            bUnlockedCells = sal_True;
        }
        if( pProtect )
        {
            pProtect->setOption( ScTableProtection::SELECT_LOCKED_CELLS, bLockedCells );
            pProtect->setOption( ScTableProtection::SELECT_UNLOCKED_CELLS, bUnlockedCells );
        }
    }
    else
        throw uno::RuntimeException(::rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM( "Sheet Name does not exist. ") ),
                                uno::Reference< XInterface >() );

}

sal_Bool SAL_CALL ScVbaWorksheet::getAutoFilterMode() throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
    ScDBData* pDBData = pDoc->GetAnonymousDBData(getSheetID());
    if (pDBData)
        return pDBData->HasAutoFilter();
    return false;
}

void SAL_CALL ScVbaWorksheet::setAutoFilterMode( sal_Bool bAutoFilterMode ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScDocShell* pDocShell = excel::getDocShell( xModel );
    ScDocument* pDoc = pDocShell->GetDocument();
    ScDBData* pDBData = pDoc->GetAnonymousDBData(getSheetID());
    if (pDBData)
    {
        pDBData->SetAutoFilter(bAutoFilterMode);
        ScRange aRange;
        pDBData->GetArea(aRange);
        if (bAutoFilterMode && pDoc)
            pDoc->ApplyFlagsTab( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aStart.Row(),
                                    aRange.aStart.Tab(), SC_MF_AUTO );
        else  if (!bAutoFilterMode && pDoc)
            pDoc->RemoveFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aStart.Row(),
                                    aRange.aStart.Tab(), SC_MF_AUTO );
        ScRange aPaintRange(aRange.aStart, aRange.aEnd);
        aPaintRange.aEnd.SetRow(aPaintRange.aStart.Row());
        pDocShell->PostPaint(aPaintRange, PAINT_GRID);
    }
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getUsedRange() throw (uno::RuntimeException)
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
ScVbaWorksheet::Outline( ) throw (uno::RuntimeException)
{
    uno::Reference<sheet::XSheetOutline> xOutline(getSheet(),uno::UNO_QUERY_THROW);
    return new ScVbaOutline( this, mxContext, xOutline);
}

uno::Reference< excel::XPageSetup >
ScVbaWorksheet::PageSetup( ) throw (uno::RuntimeException)
{
    return new ScVbaPageSetup( this, mxContext, getSheet(), getModel() );
}

uno::Any
ScVbaWorksheet::HPageBreaks( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak(getSheet(),uno::UNO_QUERY_THROW);
    uno::Reference< excel::XHPageBreaks > xHPageBreaks( new ScVbaHPageBreaks( this, mxContext, xSheetPageBreak));
   if ( aIndex.hasValue() )
      return xHPageBreaks->Item( aIndex, uno::Any());
   return uno::makeAny( xHPageBreaks );
}

uno::Any
ScVbaWorksheet::VPageBreaks( const uno::Any& aIndex ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak( getSheet(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XVPageBreaks > xVPageBreaks( new ScVbaVPageBreaks( this, mxContext, xSheetPageBreak ) );
    if( aIndex.hasValue() )
        return xVPageBreaks->Item( aIndex, uno::Any());
    return uno::makeAny( xVPageBreaks );
}

sal_Int32
ScVbaWorksheet::getStandardWidth() throw (uno::RuntimeException)
{
    return STANDARDWIDTH ;
}

sal_Int32
ScVbaWorksheet::getStandardHeight() throw (uno::RuntimeException)
{
    return STANDARDHEIGHT;
}

sal_Bool
ScVbaWorksheet::getProtectionMode() throw (uno::RuntimeException)
{
    return false;
}

sal_Bool
ScVbaWorksheet::getProtectContents()throw (uno::RuntimeException)
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    return xProtectable->isProtected();
}

sal_Bool
ScVbaWorksheet::getProtectDrawingObjects() throw (uno::RuntimeException)
{
    SCTAB nTab = 0;
    rtl::OUString aSheetName = getName();
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    bool bSheetExists = ScVbaWorksheets::nameExists (xSpreadDoc, aSheetName, nTab);
    if ( bSheetExists )
    {
        uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
        ScDocument* pDoc = excel::getDocShell( xModel )->GetDocument();
        ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
        if ( pProtect )
            return pProtect->isOptionEnabled( ScTableProtection::OBJECTS );
    }
    return false;
}

sal_Bool
ScVbaWorksheet::getProtectScenarios() throw (uno::RuntimeException)
{
    return false;
}

void
ScVbaWorksheet::Activate() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSpreadsheetView > xSpreadsheet(
            getModel()->getCurrentController(), uno::UNO_QUERY_THROW );
    xSpreadsheet->setActiveSheet(getSheet());
}

void
ScVbaWorksheet::Select() throw (uno::RuntimeException)
{
    Activate();
}

void
ScVbaWorksheet::Move( const uno::Any& Before, const uno::Any& After ) throw (uno::RuntimeException)
{
    uno::Reference<excel::XWorksheet> xSheet;
    rtl::OUString aCurrSheetName = getName();

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
        sal_Bool bAfter = After.hasValue();
        if (bAfter)
            nDest++;
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        xSheets->moveByName(aCurrSheetName,nDest);
    }
}

void
ScVbaWorksheet::Copy( const uno::Any& Before, const uno::Any& After ) throw (uno::RuntimeException)
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
ScVbaWorksheet::Paste( const uno::Any& Destination, const uno::Any& /*Link*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# Link is not used
    uno::Reference<excel::XRange> xRange( Destination, uno::UNO_QUERY );
    if ( xRange.is() )
        xRange->Select();
    excel::implnPaste( mxModel );
}

void
ScVbaWorksheet::Delete() throw (uno::RuntimeException)
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( getModel(), uno::UNO_QUERY_THROW );
    rtl::OUString aSheetName = getName();
    if ( xSpreadDoc.is() )
    {
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
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getSheetAtOffset(SCTAB offset) throw (uno::RuntimeException)
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
ScVbaWorksheet::getNext() throw (uno::RuntimeException)
{
    return getSheetAtOffset(static_cast<SCTAB>(1));
}

uno::Reference< excel::XWorksheet >
ScVbaWorksheet::getPrevious() throw (uno::RuntimeException)
{
    return getSheetAtOffset(-1);
}


void
ScVbaWorksheet::Protect( const uno::Any& Password, const uno::Any& /*DrawingObjects*/, const uno::Any& /*Contents*/, const uno::Any& /*Scenarios*/, const uno::Any& /*UserInterfaceOnly*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# is there anything we can do witht the unused param
    // can the implementation use anything else here
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    ::rtl::OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->protect( aPasswd );
}

void
ScVbaWorksheet::Unprotect( const uno::Any& Password ) throw (uno::RuntimeException)
{
    uno::Reference<util::XProtectable > xProtectable(getSheet(), uno::UNO_QUERY_THROW);
    ::rtl::OUString aPasswd;
    Password >>= aPasswd;
    xProtectable->unprotect( aPasswd );
}

void
ScVbaWorksheet::Calculate() throw (uno::RuntimeException)
{
    uno::Reference <sheet::XCalculatable> xReCalculate(getModel(), uno::UNO_QUERY_THROW);
    xReCalculate->calculate();
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Range( const ::uno::Any& Cell1, const ::uno::Any& Cell2 ) throw (uno::RuntimeException)
{
    uno::Reference< excel::XRange > xSheetRange( new ScVbaRange( this, mxContext
, uno::Reference< table::XCellRange >( getSheet(), uno::UNO_QUERY_THROW ) ) );
    return xSheetRange->Range( Cell1, Cell2 );
}

void
ScVbaWorksheet::CheckSpelling( const uno::Any& /*CustomDictionary*/,const uno::Any& /*IgnoreUppercase*/,const uno::Any& /*AlwaysSuggest*/, const uno::Any& /*SpellingLang*/ ) throw (uno::RuntimeException)
{
    // #TODO# #FIXME# unused params above, can we do anything with those
    rtl::OUString url = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SpellDialog"));
    uno::Reference< frame::XModel > xModel( getModel() );
    dispatchRequests(xModel,url);
}

uno::Reference< excel::XRange >
ScVbaWorksheet::getSheetRange() throw (uno::RuntimeException)
{
    uno::Reference< table::XCellRange > xRange( getSheet(),uno::UNO_QUERY_THROW );
    return uno::Reference< excel::XRange >( new ScVbaRange( this, mxContext, xRange ) );
}

// These are hacks - we prolly (somehow) need to inherit
// the vbarange functionality here ...
uno::Reference< excel::XRange >
ScVbaWorksheet::Cells( const ::uno::Any &nRow, const ::uno::Any &nCol )
        throw (uno::RuntimeException)
{
    // Performance optimization for often-called Cells method:
    // Use a common helper method instead of creating a new ScVbaRange object
    uno::Reference< table::XCellRange > xRange( getSheet(), uno::UNO_QUERY_THROW );
    return ScVbaRange::CellsHelper( this, mxContext, xRange, nRow, nCol );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Rows(const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return getSheetRange()->Rows( aIndex );
}

uno::Reference< excel::XRange >
ScVbaWorksheet::Columns( const uno::Any& aIndex ) throw (uno::RuntimeException)
{
    return getSheetRange()->Columns( aIndex );
}

uno::Any SAL_CALL
ScVbaWorksheet::ChartObjects( const uno::Any& Index ) throw (uno::RuntimeException)
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
ScVbaWorksheet::PivotTables( const uno::Any& Index ) throw (uno::RuntimeException)
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
ScVbaWorksheet::Comments( const uno::Any& Index ) throw (uno::RuntimeException)
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
ScVbaWorksheet::Hyperlinks( const uno::Any& aIndex ) throw (uno::RuntimeException)
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
ScVbaWorksheet::Names( const css::uno::Any& aIndex ) throw (uno::RuntimeException)
{
    // fake sheet-local names by returning all global names
    // #163498# initialize Names object with correct parent (this worksheet)
    // TODO: real sheet-local names...
    uno::Reference< beans::XPropertySet > xProps( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XNamedRanges > xNamedRanges(  xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("NamedRanges") ) ), uno::UNO_QUERY_THROW );
    uno::Reference< XCollection > xNames( new ScVbaNames( this, mxContext, xNamedRanges, mxModel ) );
    if ( aIndex.hasValue() )
        return uno::Any( xNames->Item( aIndex, uno::Any() ) );
    return uno::Any( xNames );
}

uno::Any SAL_CALL
ScVbaWorksheet::OLEObjects( const uno::Any& Index ) throw (uno::RuntimeException)
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
ScVbaWorksheet::Shapes( const uno::Any& aIndex ) throw (uno::RuntimeException)
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

uno::Any SAL_CALL
ScVbaWorksheet::Buttons( const uno::Any& rIndex ) throw (uno::RuntimeException)
{
    if( !mxButtons.is() )
        mxButtons.set( new ScVbaButtons( this, mxContext, mxModel, mxSheet ) );
    else
        mxButtons->collectShapes();
    if( rIndex.hasValue() )
        return mxButtons->Item( rIndex, uno::Any() );
    return uno::Any( uno::Reference< XCollection >( mxButtons.get() ) );
}

uno::Any SAL_CALL
ScVbaWorksheet::CheckBoxes( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::DropDowns( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::GroupBoxes( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::Labels( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::ListBoxes( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::OptionButtons( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::ScrollBars( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

uno::Any SAL_CALL
ScVbaWorksheet::Spinners( const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException();
}

void SAL_CALL
ScVbaWorksheet::ShowDataForm( ) throw (uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    ScTabViewShell* pTabViewShell = excel::getBestViewShell( xModel );

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

    AbstractScDataFormDlg* pDlg = pFact->CreateScDataFormDlg( pTabViewShell->GetDialogParent(),RID_SCDLG_DATAFORM, pTabViewShell);
    OSL_ENSURE(pDlg, "Dialog create fail!");

    pDlg->Execute();
}

uno::Any SAL_CALL
ScVbaWorksheet::Evaluate( const ::rtl::OUString& Name ) throw (uno::RuntimeException)
{
    // #TODO Evaluate allows other things to be evaluated, e.g. functions
    // I think ( like SIN(3) etc. ) need to investigate that
    // named Ranges also? e.g. [MyRange] if so need a list of named ranges
    uno::Any aVoid;
    return uno::Any( Range( uno::Any( Name ), aVoid ) );
}


uno::Reference< beans::XIntrospectionAccess > SAL_CALL
ScVbaWorksheet::getIntrospection(  ) throw (uno::RuntimeException)
{
    return uno::Reference< beans::XIntrospectionAccess >();
}

uno::Any SAL_CALL
ScVbaWorksheet::invoke( const ::rtl::OUString& aFunctionName, const uno::Sequence< uno::Any >& /*aParams*/, uno::Sequence< ::sal_Int16 >& /*aOutParamIndex*/, uno::Sequence< uno::Any >& /*aOutParam*/ ) throw (lang::IllegalArgumentException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    OSL_TRACE("** ScVbaWorksheet::invoke( %s ), will barf",
        rtl::OUStringToOString( aFunctionName, RTL_TEXTENCODING_UTF8 ).getStr() );

    throw uno::RuntimeException(); // unsupported operation
}

void SAL_CALL
ScVbaWorksheet::setValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, script::CannotConvertException, reflection::InvocationTargetException, uno::RuntimeException)
{
    setDefaultPropByIntrospection( uno::makeAny( getValue( aPropertyName ) ), aValue );
}
uno::Any SAL_CALL
ScVbaWorksheet::getValue( const ::rtl::OUString& aPropertyName ) throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Reference< drawing::XControlShape > xControlShape( getControlShape( aPropertyName ), uno::UNO_QUERY_THROW );

    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.ControlProvider" ) ), mxContext ), uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xControl( xControlProvider->createControl(  xControlShape, getModel() ) );
    return uno::makeAny( xControl );
}

::sal_Bool SAL_CALL
ScVbaWorksheet::hasMethod( const ::rtl::OUString& /*aName*/ ) throw (uno::RuntimeException)
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
        xFormControls.set( xIndexAccess->getByIndex(0), uno::UNO_QUERY_THROW );

    }
    catch( uno::Exception& )
    {
    }
    return xFormControls;

                }
::sal_Bool SAL_CALL
ScVbaWorksheet::hasProperty( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
{
    uno::Reference< container::XNameAccess > xFormControls( getFormControls() );
    if ( xFormControls.is() )
        return xFormControls->hasByName( aName );
    return false;
}

uno::Any
ScVbaWorksheet::getControlShape( const ::rtl::OUString& sName )
{
    // ideally we would get an XControl object but it appears an XControl
    // implementation only exists for a Control implementation optained from the
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
        if( sName.equals( xNamed->getName() ))
        {
            return aUnoObj;
        }
         }
    }
    return uno::Any();
}


rtl::OUString
ScVbaWorksheet::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaWorksheet"));
}

void SAL_CALL
ScVbaWorksheet::setEnableCalculation( ::sal_Bool bEnableCalculation ) throw ( script::BasicErrorException, uno::RuntimeException)
{
    uno::Reference <sheet::XCalculatable> xCalculatable(getModel(), uno::UNO_QUERY_THROW);
        xCalculatable->enableAutomaticCalculation( bEnableCalculation);
}
::sal_Bool SAL_CALL
ScVbaWorksheet::getEnableCalculation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    uno::Reference <sheet::XCalculatable> xCalculatable(getModel(), uno::UNO_QUERY_THROW);
    return xCalculatable->isAutomaticCalculationEnabled();
}

uno::Sequence< rtl::OUString >
ScVbaWorksheet::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Worksheet" ) );
    }
    return aServiceNames;
}

rtl::OUString SAL_CALL
ScVbaWorksheet::getCodeName() throw (css::uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xSheetProp( mxSheet, uno::UNO_QUERY_THROW );
    return xSheetProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CodeName" ) ) ).get< ::rtl::OUString >();
}

sal_Int16
ScVbaWorksheet::getSheetID() throw (uno::RuntimeException)
{
    uno::Reference< sheet::XCellRangeAddressable > xAddressable( mxSheet, uno::UNO_QUERY_THROW ); // if ActiveSheet, mxSheet is null.
    return xAddressable->getRangeAddress().Sheet;
}

void SAL_CALL
ScVbaWorksheet::PrintOut( const uno::Any& From, const uno::Any& To, const uno::Any& Copies, const uno::Any& Preview, const uno::Any& ActivePrinter, const uno::Any& PrintToFile, const uno::Any& Collate, const uno::Any& PrToFileName, const uno::Any& IgnorePrintAreas ) throw (uno::RuntimeException)
{
    sal_Int32 nTo = 0;
    sal_Int32 nFrom = 0;
    sal_Int16 nCopies = 1;
    sal_Bool bCollate = false;
    sal_Bool bSelection = false;
    sal_Bool bIgnorePrintAreas = false;
    From >>= nFrom;
    To >>= nTo;
    Copies >>= nCopies;
    IgnorePrintAreas >>= bIgnorePrintAreas;
    if ( nCopies > 1 ) // Collate only useful when more that 1 copy
        Collate >>= bCollate;

    if ( !( nFrom || nTo ) )
        bSelection = sal_True;

    uno::Reference< frame::XModel > xModel( getModel(), uno::UNO_QUERY_THROW );
    PrintOutHelper( excel::getBestViewShell( xModel ), From, To, Copies, Preview, ActivePrinter, PrintToFile, Collate, PrToFileName, bSelection );
}

sal_Int64 SAL_CALL
ScVbaWorksheet::getSomething(const uno::Sequence<sal_Int8 > & rId) throw(uno::RuntimeException)
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
sdecl::vba_service_class_<ScVbaWorksheet, sdecl::with_args<true> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ScVbaWorksheet",
    "ooo.vba.excel.Worksheet" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
