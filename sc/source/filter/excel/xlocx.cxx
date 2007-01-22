/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlocx.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-22 13:18:10 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
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
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormsSupplier;
using ::com::sun::star::form::XFormComponent;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;
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
    // try to open the 'Ctls' storage stream containing control properties
    mxStrm = OpenStream( EXC_STREAM_CTLS );
}

SdrObject* XclImpOcxConverter::CreateSdrObject( XclImpOleObj& rOcxCtrlObj, const Rectangle& rAnchorRect )
{
    SdrObject* pSdrObj = 0;

    DBG_ASSERT( rOcxCtrlObj.IsControl(), "XclImpOcxConverter::CreateSdrObject - no control object" );
    if( mxStrm.Is() && rOcxCtrlObj.IsControl() )
    {
        // virtual call of GetDrawPage() needs current sheet index
        SetScTab( rOcxCtrlObj.GetScTab() );

        // stream position of the extra data for this control
        mxStrm->Seek( rOcxCtrlObj.GetCtlsStreamPos() );

        try
        {
            Reference< XShape > xShape;
            // reads from mxStrm into xShape, inserts the control model into the form of the sheet
            if( ReadOCXExcelKludgeStream( mxStrm, &xShape, TRUE ) )
                pSdrObj = FinalizeSdrObject( rOcxCtrlObj, xShape, rAnchorRect );
        }
        catch( Exception& )
        {
            DBG_ERRORFILE( "XclImpOcxConverter::CreateSdrObject - unexpected exception caught" );
        }
    }
    return pSdrObj;
}

SdrObject* XclImpOcxConverter::CreateSdrObject( XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect )
{
    SdrObject* pSdrObj = 0;

    // virtual call of GetDrawPage() needs current sheet index
    SetScTab( rTbxCtrlObj.GetScTab() );

    Reference< XMultiServiceFactory > xFactory = GetServiceFactory();
    if( xFactory.is() )
    {
        Reference< XInterface > xInt( xFactory->createInstance( rTbxCtrlObj.GetServiceName() ) );
        Reference< XFormComponent > xFormComp( xInt, UNO_QUERY );
        if( xFormComp.is() )
        {
            Reference< XShape > xShape;             // the shape to fill
            ::com::sun::star::awt::Size aSize;      // dummy size

            // try to insert the control into the form
            if( InsertControl( xFormComp, aSize, &xShape, TRUE ) )
            {
                pSdrObj = FinalizeSdrObject( rTbxCtrlObj, xShape, rAnchorRect );
                // try to attach a macro to the control
                RegisterTbxMacro( rTbxCtrlObj );
            }
        }
    }
    return pSdrObj;
}

sal_Bool XclImpOcxConverter::InsertControl(
        const Reference< XFormComponent >& rxFormComp,
        const ::com::sun::star::awt::Size& rSize,
        Reference< XShape >* pxShape,
        BOOL bFloatingCtrl )
{
    sal_Bool bRet = sal_False;

    Reference< XIndexContainer > xFormCompsIC = GetFormComps();
    Reference< XMultiServiceFactory > xFactory = GetServiceFactory();
    if( xFormCompsIC.is() && xFactory.is() )
    {
        // store new index of the control for later use
        mnLastIndex = xFormCompsIC->getCount();
        // insert the new control into the form
        xFormCompsIC->insertByIndex( mnLastIndex, makeAny( rxFormComp ) );

        // create the control shape
        Reference< XShape > xShape( xFactory->createInstance(
            CREATE_OUSTRING( "com.sun.star.drawing.ControlShape" ) ), UNO_QUERY );
        if( xShape.is() )
        {
            // set control model at the shape
            Reference< XControlShape > xCtrlShape( xShape, UNO_QUERY );
            Reference< XControlModel > xCtrlModel( rxFormComp, UNO_QUERY );
            if( xCtrlShape.is() && xCtrlModel.is() )
            {
                xCtrlShape->setControl( xCtrlModel );
                if( pxShape )
                    *pxShape = xShape;
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

SdrObject* XclImpOcxConverter::FinalizeSdrObject( XclImpControlObjHelper& rCtrlObj,
        Reference< XShape > xShape, const Rectangle& rAnchorRect ) const
{
    // store control model at passed Excel import object, used later to set more properties
    rCtrlObj.SetControlModel( XclControlObjHelper::GetModelFromShape( xShape ) );

    // get SdrObject from the shape and set anchor rectangle and layer ID
    SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape );
    if( pSdrObj )
    {
        pSdrObj->NbcSetSnapRect( rAnchorRect );     // anchor rectangle
        pSdrObj->NbcSetLayer( SC_LAYER_CONTROLS );  // #i30543# insert into control layer
    }
    return pSdrObj;
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

XclExpOcxControlObj* XclExpOcxConverter::CreateCtrlObj( Reference< XShape > xShape )
{
    ::std::auto_ptr< XclExpOcxControlObj > xOcxCtrl;

    Reference< XControlModel > xCtrlModel = XclControlObjHelper::GetModelFromShape( xShape );
    if( xCtrlModel.is() )
    {
        // output stream
        if( !mxStrm.Is() )
            mxStrm = OpenStream( EXC_STREAM_CTLS );
        if( mxStrm.Is() )
        {
            String aClassName;
            sal_uInt32 nStrmStart = static_cast< sal_uInt32 >( mxStrm->Tell() );

            // writes from xCtrlModel into mxStrm, raw class name returned in aClassName
            if( WriteOCXExcelKludgeStream( mxStrm, xCtrlModel, xShape->getSize(), aClassName ) )
            {
                sal_uInt32 nStrmSize = static_cast< sal_uInt32 >( mxStrm->Tell() - nStrmStart );
                // adjust the class name to "Forms.***.1"
                aClassName.InsertAscii( "Forms.", 0 ).AppendAscii( ".1" );
                xOcxCtrl.reset( new XclExpOcxControlObj( GetRoot(), xShape, aClassName, nStrmStart, nStrmSize ) );
            }
        }
    }
    return xOcxCtrl.release();
}

#else

XclExpTbxControlObj* XclExpOcxConverter::CreateCtrlObj( Reference< XShape > xShape )
{
    ::std::auto_ptr< XclExpTbxControlObj > xTbxCtrl( new XclExpTbxControlObj( GetRoot(), xShape ) );
    if( xTbxCtrl->GetObjType() == EXC_OBJ_CMO_UNKNOWN )
        xTbxCtrl.reset();

    if( xTbxCtrl.get() )
    {
        // find attached macro
        Reference< XControlModel > xCtrlModel = XclControlObjHelper::GetModelFromShape( xShape );
        ConvertTbxMacro( *xTbxCtrl, xCtrlModel );
    }
    return xTbxCtrl.release();
}

void XclExpOcxConverter::ConvertTbxMacro( XclExpTbxControlObj& rTbxCtrlObj, Reference< XControlModel > xCtrlModel )
{
    // *** 1) try to find the index of the processed control in the form ***

    Reference< XIndexAccess > xFormIA;  // needed in step 2) below
    sal_Int32 nFoundIdx = -1;

    // update xDrawPage member of svx base class with draw page of current sheet
    SetDrawPage( GetCurrScTab() );
    Reference< XFormsSupplier > xFormsSup( xDrawPage, UNO_QUERY );

    if( xFormsSup.is() && xCtrlModel.is() )
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
                        if( xCtrlModel.get() == xCurrModel.get() )
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

