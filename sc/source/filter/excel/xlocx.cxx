/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlocx.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:18:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMSSUPPLIER_HPP_
#include <com/sun/star/form/XFormsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XBINDABLEVALUE_HPP_
#include <com/sun/star/form/binding/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XVALUEBINDING_HPP_
#include <com/sun/star/form/binding/XValueBinding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSINK_HPP_
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSOURCE_HPP_
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif

#ifndef SC_FAPIHELPER_HXX
#include "fapihelper.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XEESCHER_HXX
#include "xeescher.hxx"
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::form::XFormComponent;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::com::sun::star::script::ScriptEventDescriptor;
using ::com::sun::star::script::XEventAttacherManager;

// OCX controls ===============================================================

XclOcxConverter::XclOcxConverter( const XclRoot& rRoot ) :
    SvxMSConvertOCXControls( rRoot.GetDocShell(), 0 ),
    mrRoot( rRoot ),
    mnCurrScTab( SCTAB_MAX ),
    mnCachedScTab( SCTAB_MAX )
{
}

XclOcxConverter::~XclOcxConverter()
{
}

void XclOcxConverter::SetScTab( SCTAB nScTab )
{
    /*  Invalidate SvxMSConvertOCXControls::xFormComps whenever sheet index changes,
        otherwise GetDrawPage() will not be called in SvxMSConvertOCXControls::GetFormComps(). */
    if( mnCurrScTab != nScTab )
    {
        xFormComps.clear();
        mnCurrScTab = nScTab;
    }
}

void XclOcxConverter::SetDrawPage( SCTAB nScTab )
{
    SetScTab( nScTab );
    GetDrawPage();
}

const Reference< XDrawPage >& XclOcxConverter::GetDrawPage()
{
    // find and cache draw page if uninitialized or sheet index has been changed
    // xDrawPage is a member of the svx base class SvxMSConvertOCXControls
    if( !xDrawPage.is() || (mnCachedScTab != mnCurrScTab) )
    {
        // mnCurrTab is set in SetScTab() and contains sheet index of current control
        if( SdrPage* pPage = mrRoot.GetSdrPage( mnCurrScTab ) )
        {
            xDrawPage = Reference< XDrawPage >( pPage->getUnoPage(), UNO_QUERY );
            mnCachedScTab = mnCurrScTab;
        }
    }

    return xDrawPage;
}

// ----------------------------------------------------------------------------

XclImpOcxConverter::XclImpOcxConverter( const XclImpRoot& rRoot ) :
    XclOcxConverter( rRoot ),
    XclImpRoot( rRoot ),
    mnLastIndex( -1 )
{
    // get the MultiServiceFactory of the Calc document
    if( SfxObjectShell* pDocShell = GetDocShell() )
        mxDocFactory.set( pDocShell->GetModel(), UNO_QUERY );

    // try to open the 'Ctls' storage stream containing control properties
    mxStrm = OpenStream( EXC_STREAM_CTLS );
}

SdrObject* XclImpOcxConverter::CreateSdrObject( const XclImpOleObj& rOcxCtrlObj, const Rectangle& rAnchorRect )
{
    DBG_ASSERT( rOcxCtrlObj.IsControl(), "XclOcxConverter::CreateSdrObject - no control object" );
    if( mxStrm.Is() && rOcxCtrlObj.IsControl() )
    {
        // virtual call of GetDrawPage() needs current sheet index
        SetScTab( rOcxCtrlObj.GetScTab() );

        // stream position of the extra data for this control
        mxStrm->Seek( rOcxCtrlObj.GetCtlsStreamPos() );

        // the shape to fill
        Reference< XShape > xShape;

        bool bReadSuccess = false;
        try
        {
            // reads from mxStrm into xShape, inserts the control into the document
            bReadSuccess = ReadOCXExcelKludgeStream( mxStrm, &xShape, TRUE );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "XclImpOcxConverter::CreateSdrObject - unexpected exception caught" );
        }

        if( bReadSuccess )
        {
            if( SdrObject* pSdrObj = GetSdrObject( xShape, rAnchorRect ) )
            {
                Reference< XControlShape > xControlShape( xShape, UNO_QUERY );
                if( xControlShape.is() )
                {
                    Reference< XControlModel > xModel( xControlShape->getControl() );
                    if( xModel.is() )
                    {
                        // set the spreadsheet links
                        ConvertSheetLinks( xModel, rOcxCtrlObj );
                        // set additional control properties
                        ScfPropertySet aPropSet( xModel );
                        if( aPropSet.Is() )
                            rOcxCtrlObj.WriteToPropertySet( aPropSet );
                    }
                }
                return pSdrObj;
            }
        }
    }
    return 0;
}

SdrObject* XclImpOcxConverter::CreateSdrObject( const XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect )
{
    // virtual call of GetDrawPage() needs current sheet index
    SetScTab( rTbxCtrlObj.GetScTab() );

    const Reference< XMultiServiceFactory >& rxServiceFactory = GetServiceFactory();
    if( rxServiceFactory.is() )
    {
        Reference< XInterface > xInt( rxServiceFactory->createInstance( rTbxCtrlObj.GetServiceName() ) );
        Reference< XFormComponent > xFormComp( xInt, UNO_QUERY );
        Reference< XControlModel > xModel( xInt, UNO_QUERY );
        if( xFormComp.is() && xModel.is() )
        {
            // the shape to fill
            Reference< XShape > xShape;
            // dummy size
            ::com::sun::star::awt::Size aSize;

            // try to insert the control into the form
            if( InsertControl( xFormComp, aSize, &xShape, TRUE ) )
            {
                if( SdrObject* pSdrObj = GetSdrObject( xShape, rAnchorRect ) )
                {
                    // set the links to the spreadsheet
                    ConvertSheetLinks( xModel, rTbxCtrlObj );
                    // set the control properties
                    ScfPropertySet aPropSet( xModel );
                    if( aPropSet.Is() )
                        rTbxCtrlObj.WriteToPropertySet( aPropSet );
                    // try to attach a macro to the control
                    RegisterTbxMacro( rTbxCtrlObj );
                    return pSdrObj;
                }
            }
        }
    }
    return 0;
}

sal_Bool XclImpOcxConverter::InsertControl(
        const Reference< XFormComponent >& rxFormComp,
        const ::com::sun::star::awt::Size& rSize,
        Reference< XShape >* pxShape,
        BOOL bFloatingCtrl )
{
    sal_Bool bRet = sal_False;

    const Reference< XIndexContainer >& rxFormCompsIC = GetFormComps();
    const Reference< XMultiServiceFactory >& rxServiceFactory = GetServiceFactory();
    if( rxFormCompsIC.is() && rxServiceFactory.is() )
    {
        // store new index of the control for later use
        mnLastIndex = rxFormCompsIC->getCount();
        // insert the new control into the form
        rxFormCompsIC->insertByIndex( mnLastIndex, makeAny( rxFormComp ) );

        // create the control shape
        Reference< XShape > xShape( rxServiceFactory->createInstance(
            CREATE_OUSTRING( "com.sun.star.drawing.ControlShape" ) ), UNO_QUERY );
        if( xShape.is() )
        {
            // set control model at the shape
            Reference< XControlShape > xControlShape( xShape, UNO_QUERY );
            Reference< XControlModel > xControlModel( rxFormComp, UNO_QUERY );
            if( xControlShape.is() && xControlModel.is() )
            {
                xControlShape->setControl( xControlModel );
                if( pxShape )
                    *pxShape = xShape;
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

SdrObject* XclImpOcxConverter::GetSdrObject( const Reference< XShape >& rxShape, const Rectangle& rAnchorRect ) const
{
    SdrObject* pSdrObj = ::GetSdrObjectFromXShape( rxShape );
    if( pSdrObj )
    {
        pSdrObj->NbcSetSnapRect( rAnchorRect );
        // #i30543# insert into control layer
        pSdrObj->NbcSetLayer( SC_LAYER_CONTROLS );
    }
    return pSdrObj;
}

void XclImpOcxConverter::ConvertSheetLinks(
        const Reference< XControlModel >& rxModel, const XclImpCtrlLinkHelper& rControl ) const
{
    if( !mxDocFactory.is() ) return;

    // *** cell link *** ------------------------------------------------------

    const ScAddress* pCellLink = rControl.GetCellLink();
    Reference< XBindableValue > xBindable( rxModel, UNO_QUERY );
    if( pCellLink && xBindable.is() )
    {
        // create argument sequence for createInstanceWithArguments()
        CellAddress aApiAddress;
        ScUnoConversion::FillApiAddress( aApiAddress, *pCellLink );

        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( SC_UNONAME_BOUNDCELL );
        aValue.Value <<= aApiAddress;

        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the CellValueBinding instance
        Reference< XInterface > xInt;
        try
        {
            switch( rControl.GetBindingMode() )
            {
                case xlBindContent:
                    xInt = mxDocFactory->createInstanceWithArguments(
                        CREATE_OUSTRING( SC_SERVICENAME_VALBIND ), aArgs );
                break;
                case xlBindPosition:
                    xInt = mxDocFactory->createInstanceWithArguments(
                        CREATE_OUSTRING( SC_SERVICENAME_LISTCELLBIND ), aArgs );
                break;
                default:
                    DBG_ERRORFILE( "XclImpOcxConverter::ConvertSheetLinks - unknown binding mode" );
            }
        }
        catch( const Exception& )
        {
        }

        // set the binding at the control
        Reference< XValueBinding > xBinding( xInt, UNO_QUERY );
        if( xBinding.is() )
            xBindable->setValueBinding( xBinding );
    }

    // *** source range *** ---------------------------------------------------

    const ScRange* pSrcRange = rControl.GetSourceRange();
    Reference< XListEntrySink > xEntrySink( rxModel, UNO_QUERY );
    if( pSrcRange && xEntrySink.is() )
    {
        // create argument sequence for createInstanceWithArguments()
        CellRangeAddress aApiRange;
        ScUnoConversion::FillApiRange( aApiRange, *pSrcRange );

        NamedValue aValue;
        aValue.Name = CREATE_OUSTRING( SC_UNONAME_CELLRANGE );
        aValue.Value <<= aApiRange;

        Sequence< Any > aArgs( 1 );
        aArgs[ 0 ] <<= aValue;

        // create the CellValueBinding instance
        Reference< XInterface > xInt;
        try
        {
            xInt = mxDocFactory->createInstanceWithArguments(
                CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ), aArgs );
        }
        catch( const Exception& )
        {
        }

        // set the binding at the control
        Reference< XListEntrySource > xEntrySource( xInt, UNO_QUERY );
        if( xEntrySource.is() )
            xEntrySink->setListEntrySource( xEntrySource );
    }
}

void XclImpOcxConverter::RegisterTbxMacro( const XclImpTbxControlObj& rTbxCtrlObj )
{
    ScriptEventDescriptor aEvent;
    if( (mnLastIndex >= 0) && rTbxCtrlObj.FillMacroDescriptor( aEvent ) )
    {
        Reference< XEventAttacherManager > xEventMgr( GetFormComps(), UNO_QUERY );
        if( xEventMgr.is() )
        {
            try
            {
                xEventMgr->registerScriptEvent( mnLastIndex, aEvent );
            }
            catch( Exception& )
            {
                DBG_ERRORFILE( "XclImpOcxConverter::RegisterTbxMacro - cannot register macro" );
            }
        }
    }
}

// ----------------------------------------------------------------------------

XclExpOcxConverter::XclExpOcxConverter( const XclExpRoot& rRoot ) :
    XclOcxConverter( rRoot ),
    XclExpRoot( rRoot )
{
}

#if EXC_EXP_OCX_CTRL

XclExpObjOcxCtrl* XclExpOcxConverter::CreateCtrlObj( const Reference< XShape >& rxShape )
{
    XclExpObjOcxCtrl* pOcxCtrl = 0;

    // the shape to export
    Reference< XControlShape > xControlShape( rxShape, UNO_QUERY );
    if( xControlShape.is() )
    {
        // the control model
        Reference< XControlModel > xControlModel = xControlShape->getControl();
        if( xControlModel.is() )
        {
            // output stream
            if( !mxStrm.Is() )
                mxStrm = OpenStream( EXC_STREAM_CTLS );
            if( mxStrm.Is() )
            {
                String aClassName;
                sal_uInt32 nStrmStart = static_cast< sal_uInt32 >( mxStrm->Tell() );

                // writes from xControlModel into mxStrm, raw class name returned in aClassName
                if( WriteOCXExcelKludgeStream( mxStrm, xControlModel, rxShape->getSize(), aClassName ) )
                {
                    sal_uInt32 nStrmSize = static_cast< sal_uInt32 >( mxStrm->Tell() - nStrmStart );
                    // adjust the class name to "Forms.***.1"
                    aClassName.InsertAscii( "Forms.", 0 ).AppendAscii( ".1" );
                    pOcxCtrl = new XclExpObjOcxCtrl( GetRoot(), rxShape, xControlModel, aClassName, nStrmStart, nStrmSize );
                    ConvertSheetLinks( *pOcxCtrl, xControlModel );
                }
            }
        }
    }
    return pOcxCtrl;
}

#else

XclExpObjTbxCtrl* XclExpOcxConverter::CreateCtrlObj( const Reference< XShape >& rxShape )
{
    XclExpObjTbxCtrl* pTbxCtrl = 0;

    // the shape to export
    Reference< XControlShape > xControlShape( rxShape, UNO_QUERY );
    if( xControlShape.is() )
    {
        // the control model
        Reference< XControlModel > xControlModel = xControlShape->getControl();
        if( xControlModel.is() )
        {
            pTbxCtrl = new XclExpObjTbxCtrl( GetRoot(), rxShape, xControlModel );
            if( pTbxCtrl->GetObjType() == EXC_OBJ_CMO_UNKNOWN )
                DELETEZ( pTbxCtrl );

            if( pTbxCtrl )
            {
                // get the links to the spreadsheet
                ConvertSheetLinks( *pTbxCtrl, xControlModel );
                // find attached macro
                ConvertTbxMacro( *pTbxCtrl, xControlModel );
            }
        }
    }
    return pTbxCtrl;
}

#endif

void XclExpOcxConverter::ConvertSheetLinks(
        XclExpCtrlLinkHelper& rControl, const Reference< XControlModel >& rxModel ) const
{
    // *** cell link *** ------------------------------------------------------

    Reference< XBindableValue > xBindable( rxModel, UNO_QUERY );
    if( xBindable.is() )
    {
        Reference< XServiceInfo > xServInfo( xBindable->getValueBinding(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_VALBIND ) ) )
        {
            ScfPropertySet aBindProp( xServInfo );
            CellAddress aApiAddress;
            if( aBindProp.GetProperty( aApiAddress, CREATE_OUSTRING( SC_UNONAME_BOUNDCELL ) ) )
            {
                ScAddress aCellLink;
                ScUnoConversion::FillScAddress( aCellLink, aApiAddress );
                rControl.SetCellLink( aCellLink );
            }
        }
    }

    // *** source range *** ---------------------------------------------------

    Reference< XListEntrySink > xEntrySink( rxModel, UNO_QUERY );
    if( xEntrySink.is() )
    {
        Reference< XServiceInfo > xServInfo( xEntrySink->getListEntrySource(), UNO_QUERY );
        if( xServInfo.is() && xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ) ) )
        {
            ScfPropertySet aSinkProp( xServInfo );
            CellRangeAddress aApiRange;
            if( aSinkProp.GetProperty( aApiRange, CREATE_OUSTRING( SC_UNONAME_CELLRANGE ) ) )
            {
                ScRange aSrcRange;
                ScUnoConversion::FillScRange( aSrcRange, aApiRange );
                rControl.SetSourceRange( aSrcRange );
            }
        }
    }
}

#if !EXC_EXP_OCX_CTRL

void XclExpOcxConverter::ConvertTbxMacro(
    XclExpObjTbxCtrl& rTbxCtrlObj, const Reference< XControlModel >& rxModel )
{
    // *** 1) try to find the index of the processed control in the form ***

    Reference< XIndexAccess > xFormIA;  // needed in step 2) below
    sal_Int32 nFoundIdx = -1;

    // update xDrawPage member of svx base class with draw page of current sheet
    SetDrawPage( GetCurrScTab() );
    Reference< XFormsSupplier > xFormsSup( xDrawPage, UNO_QUERY );

    if( xFormsSup.is() && rxModel.is() )
    {
        // search all existing forms in the draw page
        Reference< XIndexAccess > xFormsIA( xFormsSup->getForms(), UNO_QUERY );
        if( xFormsIA.is() )
        {
            for( sal_Int32 nFormIdx = 0, nFormCount = xFormsIA->getCount();
                    (nFoundIdx < 0) && (nFormIdx < nFormCount); ++nFormIdx )
            {
                // get the XIndexAccess interface of the form with index nFormIdx
                if( xFormIA.set( xFormsIA->getByIndex( nFormIdx ), UNO_QUERY ) )
                {
                    // search all elements (controls) of the current form by index
                    for( sal_Int32 nCtrlIdx = 0, nCtrlCount = xFormIA->getCount();
                            (nFoundIdx < 0) && (nCtrlIdx < nCtrlCount); ++nCtrlIdx )
                    {
                        // compare implementation pointers of the control models
                        Reference< XControlModel > xCurrModel( xFormIA->getByIndex( nCtrlIdx ), UNO_QUERY );
                        if( rxModel.get() == xCurrModel.get() )
                            nFoundIdx = nCtrlIdx;
                    }
                }
            }
        }
    }

    // *** 2) try to find an attached macro ***

    if( xFormIA.is() && (nFoundIdx >= 0) )
    {
        Reference< XEventAttacherManager > xEventMgr( xFormIA, UNO_QUERY );
        if( xEventMgr.is() )
        {
            // loop over all events attached to the found control
            const Sequence< ScriptEventDescriptor > aEventSeq( xEventMgr->getScriptEvents( nFoundIdx ) );
            bool bFound = false;
            for( sal_Int32 nEventIdx = 0, nEventCount = aEventSeq.getLength();
                    !bFound && (nEventIdx < nEventCount); ++nEventIdx )
            {
                // try to set the event data at the Excel control object, returns true on success
                bFound = rTbxCtrlObj.SetMacroLink( aEventSeq[ nEventIdx ] );
            }
        }
    }
}

#endif

// ============================================================================

