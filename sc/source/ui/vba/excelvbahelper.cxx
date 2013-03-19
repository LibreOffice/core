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

#include "excelvbahelper.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include "docuno.hxx"
#include "tabvwsh.hxx"
#include "transobj.hxx"
#include "scmod.hxx"
#include "cellsuno.hxx"

#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace ooo {
namespace vba {
namespace excel {

// ============================================================================

uno::Reference< sheet::XUnnamedDatabaseRanges >
GetUnnamedDataBaseRanges( ScDocShell* pShell ) throw ( uno::RuntimeException )
{
    uno::Reference< frame::XModel > xModel;
    if ( pShell )
        xModel.set( pShell->GetModel(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xModelProps( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XUnnamedDatabaseRanges > xUnnamedDBRanges( xModelProps->getPropertyValue( rtl::OUString("UnnamedDatabaseRanges") ), uno::UNO_QUERY_THROW );
    return xUnnamedDBRanges;
}

// returns the XDatabaseRange for the autofilter on sheet (nSheet)
// also populates sName with the name of range
uno::Reference< sheet::XDatabaseRange >
GetAutoFiltRange( ScDocShell* pShell, sal_Int16 nSheet ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XUnnamedDatabaseRanges > xUnnamedDBRanges( GetUnnamedDataBaseRanges( pShell ), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XDatabaseRange > xDataBaseRange;
    if (xUnnamedDBRanges->hasByTable( nSheet ) )
    {
        uno::Reference< sheet::XDatabaseRange > xDBRange( xUnnamedDBRanges->getByTable( nSheet ) , uno::UNO_QUERY_THROW );
        sal_Bool bHasAuto = false;
        uno::Reference< beans::XPropertySet > xProps( xDBRange, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue( rtl::OUString("AutoFilter") ) >>= bHasAuto;
        if ( bHasAuto )
        {
            xDataBaseRange=xDBRange;
        }
    }
    return xDataBaseRange;
}

ScDocShell* GetDocShellFromRange( const uno::Reference< uno::XInterface >& xRange ) throw ( uno::RuntimeException )
{
    ScCellRangesBase* pScCellRangesBase = ScCellRangesBase::getImplementation( xRange );
    if ( !pScCellRangesBase )
    {
        throw uno::RuntimeException( rtl::OUString( "Failed to access underlying doc shell uno range object" ), uno::Reference< uno::XInterface >() );
    }
    return pScCellRangesBase->GetDocShell();
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xRange, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetRange->getSpreadsheet(), uno::UNO_SET_THROW );
    return getUnoSheetModuleObj( xSheet );
}

void implSetZoom( const uno::Reference< frame::XModel >& xModel, sal_Int16 nZoom, std::vector< SCTAB >& nTabs )
{
    ScTabViewShell* pViewSh = excel::getBestViewShell( xModel );
    Fraction aFract( nZoom, 100 );
    pViewSh->GetViewData()->SetZoom( aFract, aFract, nTabs );
    pViewSh->RefreshZoom();
}

const ::rtl::OUString REPLACE_CELLS_WARNING( "ReplaceCellsWarning");

class PasteCellsWarningReseter
{
private:
    bool bInitialWarningState;
    static uno::Reference< beans::XPropertySet > getGlobalSheetSettings() throw ( uno::RuntimeException )
    {
        static uno::Reference<uno::XComponentContext > xContext(
            comphelper::getProcessComponentContext() );
        static uno::Reference<lang::XMultiComponentFactory > xServiceManager(
                xContext->getServiceManager() );
        static uno::Reference< beans::XPropertySet > xProps( xServiceManager->createInstanceWithContext( rtl::OUString( "com.sun.star.sheet.GlobalSheetSettings" ) ,xContext ), uno::UNO_QUERY_THROW );
        return xProps;
    }

    bool getReplaceCellsWarning() throw ( uno::RuntimeException )
    {
        sal_Bool res = false;
        getGlobalSheetSettings()->getPropertyValue( REPLACE_CELLS_WARNING ) >>= res;
        return ( res == sal_True );
    }

    void setReplaceCellsWarning( bool bState ) throw ( uno::RuntimeException )
    {
        getGlobalSheetSettings()->setPropertyValue( REPLACE_CELLS_WARNING, uno::makeAny( bState ) );
    }
public:
    PasteCellsWarningReseter() throw ( uno::RuntimeException )
    {
        bInitialWarningState = getReplaceCellsWarning();
        if ( bInitialWarningState )
            setReplaceCellsWarning( false );
    }
    ~PasteCellsWarningReseter()
    {
        if ( bInitialWarningState )
        {
            // don't allow dtor to throw
            try
            {
                setReplaceCellsWarning( true );
            }
            catch ( uno::Exception& /*e*/ ){}
        }
    }
};

void
implnPaste( const uno::Reference< frame::XModel>& xModel )
{
    PasteCellsWarningReseter resetWarningBox;
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    if ( pViewShell )
    {
        pViewShell->PasteFromSystem();
        pViewShell->CellContentChanged();
    }
}


void
implnCopy( const uno::Reference< frame::XModel>& xModel )
{
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    if ( pViewShell )
    {
        pViewShell->CopyToClip(NULL,false,false,true);

        // mark the copied transfer object so it is used in ScVbaRange::Insert
        ScTransferObj* pClipObj = ScTransferObj::GetOwnClipboard( NULL );
        if (pClipObj)
            pClipObj->SetUseInApi( true );
    }
}

void
implnCut( const uno::Reference< frame::XModel>& xModel )
{
    ScTabViewShell* pViewShell =  getBestViewShell( xModel );
    if ( pViewShell )
    {
        pViewShell->CutToClip( NULL, sal_True );

        // mark the copied transfer object so it is used in ScVbaRange::Insert
        ScTransferObj* pClipObj = ScTransferObj::GetOwnClipboard( NULL );
        if (pClipObj)
            pClipObj->SetUseInApi( true );
    }
}

void implnPasteSpecial( const uno::Reference< frame::XModel>& xModel, sal_uInt16 nFlags,sal_uInt16 nFunction,sal_Bool bSkipEmpty, sal_Bool bTranspose)
{
    PasteCellsWarningReseter resetWarningBox;
    sal_Bool bAsLink(false), bOtherDoc(false);
    InsCellCmd eMoveMode = INS_NONE;

    ScTabViewShell* pTabViewShell = getBestViewShell( xModel );
    if ( pTabViewShell )
    {
        ScViewData* pView = pTabViewShell->GetViewData();
        Window* pWin = ( pView != NULL ) ? pView->GetActiveWin() : NULL;
        if ( pView && pWin )
        {
            if ( bAsLink && bOtherDoc )
                pTabViewShell->PasteFromSystem(0);//SOT_FORMATSTR_ID_LINK
            else
            {
                ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard( pWin );
                ScDocument* pDoc = NULL;
                if ( pOwnClip )
                    pDoc = pOwnClip->GetDocument();
                pTabViewShell->PasteFromClip( nFlags, pDoc,
                    nFunction, bSkipEmpty, bTranspose, bAsLink,
                    eMoveMode, IDF_NONE, sal_True );
                pTabViewShell->CellContentChanged();
            }
        }
    }

}

ScDocShell*
getDocShell( const css::uno::Reference< css::frame::XModel>& xModel )
{
    uno::Reference< uno::XInterface > xIf( xModel, uno::UNO_QUERY_THROW );
    ScModelObj* pModel = dynamic_cast< ScModelObj* >( xIf.get() );
    ScDocShell* pDocShell = NULL;
    if ( pModel )
        pDocShell = (ScDocShell*)pModel->GetEmbeddedObject();
    return pDocShell;

}

ScTabViewShell*
getBestViewShell( const css::uno::Reference< css::frame::XModel>& xModel )
{
    ScDocShell* pDocShell = getDocShell( xModel );
    if ( pDocShell )
        return pDocShell->GetBestViewShell();
    return NULL;
}

ScTabViewShell*
getCurrentBestViewShell(  const uno::Reference< uno::XComponentContext >& xContext )
{
    uno::Reference< frame::XModel > xModel = getCurrentExcelDoc( xContext );
    return getBestViewShell( xModel );
}

SfxViewFrame*
getViewFrame( const uno::Reference< frame::XModel >& xModel )
{
    ScTabViewShell* pViewShell = getBestViewShell( xModel );
    if ( pViewShell )
        return pViewShell->GetViewFrame();
    return NULL;
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< sheet::XSpreadsheet >& xSheet ) throw ( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySet > xProps( xSheet, uno::UNO_QUERY_THROW );
    rtl::OUString sCodeName;
    xProps->getPropertyValue( rtl::OUString("CodeName") ) >>= sCodeName;
    // #TODO #FIXME ideally we should 'throw' here if we don't get a valid parent, but... it is possible
    // to create a module ( and use 'Option VBASupport 1' ) for a calc document, in this scenario there
    // are *NO* special document module objects ( of course being able to switch between vba/non vba mode at
    // the document in the future could fix this, especially IF the switching of the vba mode takes care to
    // create the special document module objects if they don't exist.
    return getUnoDocModule( sCodeName, GetDocShellFromRange( xSheet ) );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< sheet::XSheetCellRangeContainer >& xRanges ) throw ( uno::RuntimeException )
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( xRanges, uno::UNO_QUERY_THROW );
    uno::Reference< container::XEnumeration > xEnum = xEnumAccess->createEnumeration();
    uno::Reference< table::XCellRange > xRange( xEnum->nextElement(), uno::UNO_QUERY_THROW );
    return getUnoSheetModuleObj( xRange );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< table::XCell >& xCell ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xCell, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetRange->getSpreadsheet(), uno::UNO_SET_THROW );
    return getUnoSheetModuleObj( xSheet );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< frame::XModel >& xModel, SCTAB nTab ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xSheets( xDoc->getSheets(), uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheets->getByIndex( nTab ), uno::UNO_QUERY_THROW );
    return getUnoSheetModuleObj( xSheet );
}

void setUpDocumentModules( const uno::Reference< sheet::XSpreadsheetDocument >& xDoc )
{
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    ScDocShell* pShell = excel::getDocShell( xModel );
    if ( pShell )
    {
        String aPrjName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        pShell->GetBasicManager()->SetName( aPrjName );

        /*  Set library container to VBA compatibility mode. This will create
            the VBA Globals object and store it in the Basic manager of the
            document. */
        uno::Reference<script::XLibraryContainer> xLibContainer = pShell->GetBasicContainer();
        uno::Reference<script::vba::XVBACompatibility> xVBACompat( xLibContainer, uno::UNO_QUERY_THROW );
        xVBACompat->setVBACompatibilityMode( sal_True );

        if( xLibContainer.is() )
        {
            if( !xLibContainer->hasByName( aPrjName ) )
                xLibContainer->createLibrary( aPrjName );
            uno::Any aLibAny = xLibContainer->getByName( aPrjName );
            uno::Reference< container::XNameContainer > xLib;
            aLibAny >>= xLib;
            if( xLib.is()  )
            {
                uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY_THROW );
                uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference< container::XNameAccess > xVBACodeNamedObjectAccess( xSF->createInstance( rtl::OUString( "ooo.vba.VBAObjectModuleObjectProvider")), uno::UNO_QUERY_THROW );
                // set up the module info for the workbook and sheets in the nealy created
                // spreadsheet
                ScDocument* pDoc = pShell->GetDocument();
                String sCodeName = pDoc->GetCodeName();
                if ( sCodeName.Len() == 0 )
                {
                    sCodeName = String( RTL_CONSTASCII_USTRINGPARAM("ThisWorkbook") );
                    pDoc->SetCodeName( sCodeName );
                }

                std::vector< rtl::OUString > sDocModuleNames;
                sDocModuleNames.push_back( sCodeName );

                for ( SCTAB index = 0; index < pDoc->GetTableCount(); index++)
                {
                    rtl::OUString aName;
                    pDoc->GetCodeName( index, aName );
                    sDocModuleNames.push_back( aName );
                }

                std::vector<rtl::OUString>::iterator it_end = sDocModuleNames.end();

                for ( std::vector<rtl::OUString>::iterator it = sDocModuleNames.begin(); it != it_end; ++it )
                {
                    script::ModuleInfo sModuleInfo;

                    uno::Any aName= xVBACodeNamedObjectAccess->getByName( *it );
                    sModuleInfo.ModuleObject.set( aName, uno::UNO_QUERY );
                    sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
                    xVBAModuleInfo->insertModuleInfo( *it, sModuleInfo );
                    if( xLib->hasByName( *it ) )
                        xLib->replaceByName( *it, uno::makeAny( rtl::OUString( "Option VBASupport 1\n") ) );
                    else
                        xLib->insertByName( *it, uno::makeAny( rtl::OUString( "Option VBASupport 1\n" ) ) );
                }
            }
        }

        /*  Trigger the Workbook_Open event, event processor will register
            itself as listener for specific events. */
        try
        {
            uno::Reference< script::vba::XVBAEventProcessor > xVbaEvents( pShell->GetDocument()->GetVbaEventProcessor(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs;
            xVbaEvents->processVbaEvent( script::vba::VBAEventId::WORKBOOK_OPEN, aArgs );
        }
        catch( uno::Exception& )
        {
        }
    }
}

SfxItemSet*
ScVbaCellRangeAccess::GetDataSet( ScCellRangesBase* pRangeObj )
{
    return pRangeObj ? pRangeObj->GetCurrentDataSet( true ) : 0;
}

// ============================================================================

} // namespace excel
} // namespace vba
} // namespace ooo
