/*************************************************************************
 *
 *  $RCSfile: xlocx.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:06:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

bool XclImpOcxConverter::CreateSdrUnoObj( XclImpEscherOle& rOcxCtrl )
{
    DBG_ASSERT( rOcxCtrl.IsControl(), "XclOcxConverter::CreateSdrUnoObj - no control object" );
    if( mxStrm.Is() && rOcxCtrl.IsControl() )
    {
        // virtual call of GetDrawPage() needs current sheet index
        SetScTab( rOcxCtrl.GetScTab() );

        // stream position of the extra data for this control
        sal_uInt32 nStrmPos = rOcxCtrl.GetCtrlStreamPos();
        mxStrm->Seek( static_cast< ULONG >( nStrmPos ) );

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
            DBG_ERRORFILE( "XclImpOcxConverter::CreateSdrUnoObj - unexpected exception caught" );
        }

        if( bReadSuccess )
        {
            if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
            {
                Reference< XControlShape > xControlShape( xShape, UNO_QUERY );
                if( xControlShape.is() )
                {
                    Reference< XControlModel > xModel( xControlShape->getControl() );
                    Reference< XPropertySet > xPropSet( xModel, UNO_QUERY );
                    if( xModel.is() && xPropSet.is() )
                    {
                        // set the spreadsheet links
                        ConvertSheetLinks( xModel, rOcxCtrl );
                        // set additional control properties
                        rOcxCtrl.SetProperties( xPropSet );
                    }
                }

                rOcxCtrl.SetSdrObj( pSdrObj );
                return true;
            }
        }
    }
    return false;
}

bool XclImpOcxConverter::CreateSdrUnoObj( XclImpEscherTbxCtrl& rTbxCtrl )
{
    // virtual call of GetDrawPage() needs current sheet index
    SetScTab( rTbxCtrl.GetScTab() );

    const Reference< XMultiServiceFactory >& rxServiceFactory = GetServiceFactory();
    if( rxServiceFactory.is() )
    {
        Reference< XInterface > xInt( rxServiceFactory->createInstance( rTbxCtrl.GetServiceName() ) );
        Reference< XFormComponent > xFormComp( xInt, UNO_QUERY );
        Reference< XControlModel > xModel( xInt, UNO_QUERY );
        Reference< XPropertySet > xPropSet( xInt, UNO_QUERY );
        if( xFormComp.is() && xModel.is() && xPropSet.is() )
        {
            // the shape to fill
            Reference< XShape > xShape;
            // dummy size -> is done in XclImpEscherTbxCtrl::Apply
            ::com::sun::star::awt::Size aSize;

            // try to insert the control into the form
            if( InsertControl( xFormComp, aSize, &xShape, TRUE ) )
            {
                if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
                {
                    // store the created SdrObject in the object
                    rTbxCtrl.SetSdrObj( pSdrObj );
                    // set the links to the spreadsheet
                    ConvertSheetLinks( xModel, rTbxCtrl );
                    // set the control properties
                    rTbxCtrl.SetProperties( xPropSet );
                    // try to attach a macro to the control
                    RegisterTbxMacro( rTbxCtrl );

                    return true;
                }
            }
        }
    }
    return false;
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
            xShape->setSize( rSize );
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

void XclImpOcxConverter::RegisterTbxMacro( XclImpEscherTbxCtrl& rTbxCtrl )
{
    ScriptEventDescriptor aEvent;
    if( (mnLastIndex >= 0) && rTbxCtrl.FillMacroDescriptor( aEvent ) )
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
        Reference< XPropertySet > xPropSet( xServInfo, UNO_QUERY );
        if( xPropSet.is() && xServInfo.is() &&
            xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_VALBIND ) ) )
        {
            CellAddress aApiAddress;
            if( ::getPropValue( aApiAddress, xPropSet, CREATE_OUSTRING( SC_UNONAME_BOUNDCELL ) ) )
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
        Reference< XPropertySet > xPropSet( xServInfo, UNO_QUERY );
        if( xPropSet.is() && xServInfo.is() &&
            xServInfo->supportsService( CREATE_OUSTRING( SC_SERVICENAME_LISTSOURCE ) ) )
        {
            CellRangeAddress aApiRange;
            if( ::getPropValue( aApiRange, xPropSet, CREATE_OUSTRING( SC_UNONAME_CELLRANGE ) ) )
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
    XclExpObjTbxCtrl& rTbxCtrl, const Reference< XControlModel >& rxModel )
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
                bFound = rTbxCtrl.SetMacroLink( aEventSeq[ nEventIdx ] );
            }
        }
    }
}

#endif

// ============================================================================

