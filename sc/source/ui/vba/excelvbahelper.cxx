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

#include "excelvbahelper.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include "docuno.hxx"
#include "tabvwsh.hxx"
#include "transobj.hxx"
#include "scmod.hxx"
#include "cellsuno.hxx"

namespace ooo {
namespace vba {
namespace excel {

using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

ScDocShell* GetDocShellFromRange( const uno::Reference< uno::XInterface >& xRange ) throw ( uno::RuntimeException )
{
    ScCellRangesBase* pScCellRangesBase = ScCellRangesBase::getImplementation( xRange );
    if ( !pScCellRangesBase )
    {
        throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying doc shell uno range object" ) ), uno::Reference< uno::XInterface >() );
    }
    return pScCellRangesBase->GetDocShell();
}

ScDocument* GetDocumentFromRange( const uno::Reference< uno::XInterface >& xRange ) throw ( uno::RuntimeException )
{
        ScDocShell* pDocShell = GetDocShellFromRange( xRange );
        if ( !pDocShell )
        {
                throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Failed to access underlying document from uno range object" ) ), uno::Reference< uno::XInterface >() );
        }
        return pDocShell->GetDocument();
}

void implSetZoom( const uno::Reference< frame::XModel >& xModel, sal_Int16 nZoom, std::vector< SCTAB >& nTabs )
{
    ScTabViewShell* pViewSh = excel::getBestViewShell( xModel );
    Fraction aFract( nZoom, 100 );
    pViewSh->GetViewData()->SetZoom( aFract, aFract, nTabs );
    pViewSh->RefreshZoom();
}
bool isInPrintPreview( SfxViewFrame* pView )
{
    sal_uInt16 nViewNo = SID_VIEWSHELL1 - SID_VIEWSHELL0;
    if ( pView->GetObjectShell()->GetFactory().GetViewFactoryCount() >
nViewNo && !pView->GetObjectShell()->IsInPlaceActive() )
    {
        SfxViewFactory &rViewFactory =
            pView->GetObjectShell()->GetFactory().GetViewFactory(nViewNo);
        if (  pView->GetCurViewId() == rViewFactory.GetOrdinal() )
            return true;
    }
    return false;
}

const ::rtl::OUString REPLACE_CELLS_WARNING(  RTL_CONSTASCII_USTRINGPARAM( "ReplaceCellsWarning"));

class PasteCellsWarningReseter
{
private:
    bool bInitialWarningState;
    static uno::Reference< beans::XPropertySet > getGlobalSheetSettings() throw ( uno::RuntimeException )
    {
        static uno::Reference< beans::XPropertySet > xTmpProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        static uno::Reference<uno::XComponentContext > xContext( xTmpProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY_THROW );
        static uno::Reference<lang::XMultiComponentFactory > xServiceManager(
                xContext->getServiceManager(), uno::UNO_QUERY_THROW );
        static uno::Reference< beans::XPropertySet > xProps( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.GlobalSheetSettings" ) ) ,xContext ), uno::UNO_QUERY_THROW );
        return xProps;
    }

    bool getReplaceCellsWarning() throw ( uno::RuntimeException )
    {
        sal_Bool res = sal_False;
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
    sal_Bool bAsLink(sal_False), bOtherDoc(sal_False);
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
                {
                    pDoc = pOwnClip->GetDocument();
                    pOwnClip->SetUseInApi( false );     // don't use in Insert after it was pasted once
                }
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
    xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CodeName") ) ) >>= sCodeName;
    // #TODO #FIXME ideally we should 'throw' here if we don't get a valid parent, but... it is possible
    // to create a module ( and use 'Option VBASupport 1' ) for a calc document, in this scenario there
    // are *NO* special document module objects ( of course being able to switch between vba/non vba mode at
    // the document in the future could fix this, especially IF the switching of the vba mode takes care to
    // create the special document module objects if they don't exist.
    return getUnoDocModule( sCodeName, GetDocShellFromRange( xSheet ) );
}

uno::Reference< XHelperInterface >
getUnoSheetModuleObj( const uno::Reference< table::XCellRange >& xRange ) throw ( uno::RuntimeException )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xRange, uno::UNO_QUERY_THROW );
    uno::Reference< sheet::XSpreadsheet > xSheet( xSheetRange->getSpreadsheet(), uno::UNO_SET_THROW );
    return getUnoSheetModuleObj( xSheet );
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

SfxItemSet*
ScVbaCellRangeAccess::GetDataSet( ScCellRangesBase* pRangeObj )
{
    return pRangeObj ? pRangeObj->GetCurrentDataSet( true ) : 0;
}

// ============================================================================

} // namespace excel
} // namespace vba
} // namespace ooo
