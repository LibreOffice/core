/*************************************************************************
 *
 *  $RCSfile: xlocx.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:37 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
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

#include "XclImpObjects.hxx"
#include "xcl97rec.hxx"


using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::container::XIndexContainer;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::XFormComponent;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XControlShape;


#define EXC_STREAMNAME_CTLS     String( RTL_CONSTASCII_USTRINGPARAM( "Ctls" ) )


// OCX controls ===============================================================

XclOcxConverter::XclOcxConverter( const XclRoot& rRoot ) :
    SvxMSConvertOCXControls( rRoot.GetDocShell(), NULL ),
    XclRoot( rRoot ),
    mnCurrTab( 0 ),
    mnCachedTab( 0 )
{
}

XclOcxConverter::~XclOcxConverter()
{
}

void XclOcxConverter::SetCurrTab( sal_uInt16 nTab )
{
    mnCurrTab = nTab;
}

const Reference< XDrawPage >& XclOcxConverter::GetDrawPage()
{
    // find and cache draw page if uninitialized or sheet index has been changed
    if( !xDrawPage.is() || (mnCachedTab != mnCurrTab) )
    {
        if( ScDrawLayer* pDrawLayer = GetDoc().GetDrawLayer() )
        {
            // mnCurrTab set in ReadControl() contains sheet index of current control
            if( SdrPage* pPage = pDrawLayer->GetPage( mnCurrTab ) )
            {
                xDrawPage = Reference< XDrawPage >( pPage->getUnoPage(), UNO_QUERY );
                mnCachedTab = mnCurrTab;
            }
        }
    }

    return xDrawPage;
}


// ----------------------------------------------------------------------------

XclImpOcxConverter::XclImpOcxConverter( const XclRoot& rRoot ) :
    XclOcxConverter( rRoot )
{
    mxStrm = ScfTools::OpenStorageStreamRead( GetRootStorage(), EXC_STREAMNAME_CTLS );
}

void XclImpOcxConverter::ReadControl( XclImpEscherOle& rObj )
{
    DBG_ASSERT( rObj.GetObjType() == otCtrl, "XclOcxConverter::ReadControl - no control object" );
    if( mxStrm.Is() && (rObj.GetObjType() == otCtrl) )
    {
        // virtual call of GetDrawPage() needs current sheet index
        SetCurrTab( rObj.GetTab() );

        // stream position of the extra data for this control
        sal_uInt32 nStrmPos = rObj.GetCtrlStreamPos();
        mxStrm->Seek( nStrmPos );

        // the shape to fill
        Reference< XShape > xShape = ::GetXShapeForSdrObject( const_cast< SdrObject* >( rObj.GetSdrObj() ) );

        // reads from mxStrm into xShape, inserts the control into the document
        if( ReadOCXExcelKludgeStream( mxStrm, &xShape, TRUE ) )
        {
            if( SdrObject* pSdrObj = ::GetSdrObjectFromXShape( xShape ) )
                rObj.SetSdrObj( pSdrObj );
        }
    }
}

sal_Bool XclImpOcxConverter::InsertControl(
        const Reference< XFormComponent >& rxFormComp,
        const ::com::sun::star::awt::Size& rSize,
        Reference< XShape >* pxShape,
        BOOL bFloatingCtrl )
{
    sal_Bool bRet = sal_False;
    XShapeRef xShape;

    const Reference< XIndexContainer >& rxFormCompsIC = GetFormComps();
    const Reference< XMultiServiceFactory >& rxServiceFactory = GetServiceFactory();
    if( rxFormCompsIC.is() && rxServiceFactory.is() )
    {
        Any aFormCompAny;
        aFormCompAny <<= rxFormComp;
        rxFormCompsIC->insertByIndex( rxFormCompsIC->getCount(), aFormCompAny );

        // create the control shape
        Reference< XInterface > xCreate = rxServiceFactory->createInstance(
            String( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.ControlShape" ) ) );
        xShape = Reference< XShape >( xCreate, UNO_QUERY );
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


// ----------------------------------------------------------------------------

#if EXC_INCL_EXP_OCX

XclExpOcxConverter::XclExpOcxConverter( const XclRoot& rRoot ) :
    XclOcxConverter( rRoot )
{
}

XclExpObjControl* XclExpOcxConverter::CreateObjRec( const Reference< XShape >& rxShape )
{
    XclExpObjControl* pObjRec = NULL;

    // the shape to export
    const SdrObject* pSdrObj = ::GetSdrObjectFromXShape( rxShape );
    Reference< XControlShape > xControlShape( rxShape, UNO_QUERY );

    if( pSdrObj && xControlShape.is() )
    {
        // the control model
        Reference< XControlModel > xControlModel = xControlShape->getControl();
        if( xControlModel.is() )
        {
            // output stream
            if( !mxStrm.Is() )
                mxStrm = ScfTools::OpenStorageStreamWrite( GetRootStorage(), EXC_STREAMNAME_CTLS );
            if( mxStrm.Is() )
            {
                String aClassName;
                sal_uInt32 nStrmStart = mxStrm->Tell();

                // writes from xControlModel into mxStrm, raw class name returned in aClassName
                if( WriteOCXExcelKludgeStream( mxStrm, xControlModel, rxShape->getSize(), aClassName ) )
                {
                    sal_uInt32 nStrmSize = mxStrm->Tell() - nStrmStart;
                    // adjust the class name to "Forms.***.1"
                    aClassName.InsertAscii( "Forms.", 0 ).AppendAscii( ".1" );
                    pObjRec = new XclExpObjControl( *this, rxShape, aClassName, nStrmStart, nStrmSize );
                }
            }
        }
    }
    return pObjRec;
}

#endif

// ============================================================================

