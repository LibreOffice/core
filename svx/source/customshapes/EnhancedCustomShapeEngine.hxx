/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedCustomShapeEngine.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 14:56:40 $
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

#ifndef _ENHANCED_CUSTOMSHAPE_ENGINE_HXX
#define _ENHANCED_CUSTOMSHAPE_ENGINE_HXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STACK_HXX
#include <tools/stack.hxx>
#endif

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef __com_sun_star_awt_Rectangle_hpp_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCUSTOMSHAPEENGINE_HPP_
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#endif

// -----------------------------------------------------------------------------

#define NMSP_IO         com::sun::star::io
#define NMSP_UNO        com::sun::star::uno
#define NMSP_BEANS      com::sun::star::beans
#define NMSP_LANG       com::sun::star::lang
#define NMSP_UTIL       com::sun::star::util
#define NMSP_SAX        com::sun::star::xml::sax
#define NMSP_LOGGING    NMSP_UTIL::logging


#define REF( _def_Obj )         NMSP_UNO::Reference< _def_Obj >
#define SEQ( _def_Obj )         NMSP_UNO::Sequence< _def_Obj >
#define B2UCONST( _def_pChar )  (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(_def_pChar )))

// ---------------------------
// - EnhancedCustomShapeEngine -
// ---------------------------
//

class SdrObject;
class SdrObjCustomShape;
class EnhancedCustomShapeEngine : public cppu::WeakImplHelper3
<
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo,
    com::sun::star::drawing::XCustomShapeEngine
>
{
    REF( NMSP_LANG::XMultiServiceFactory )      mxFact;
    REF( com::sun::star::drawing::XShape )      mxShape;
    sal_Bool                                    mbForceGroupWithText;

    SdrObject* ImplForceGroupWithText( const SdrObjCustomShape* pCustoObj, SdrObject* pRenderedShape );

public:
                            EnhancedCustomShapeEngine( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr );
    virtual                 ~EnhancedCustomShapeEngine();

    // XInterface
    virtual void SAL_CALL   acquire() throw();
    virtual void SAL_CALL   release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const SEQ( NMSP_UNO::Any )& aArguments )
        throw ( NMSP_UNO::Exception, NMSP_UNO::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( NMSP_UNO::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& rServiceName )
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( rtl::OUString ) SAL_CALL getSupportedServiceNames()
        throw ( NMSP_UNO::RuntimeException );

    // XCustomShapeEngine
    virtual REF( com::sun::star::drawing::XShape ) SAL_CALL render()
        throw ( NMSP_UNO::RuntimeException );
    virtual com::sun::star::awt::Rectangle SAL_CALL getTextBounds()
        throw ( NMSP_UNO::RuntimeException );
    virtual com::sun::star::drawing::PolyPolygonBezierCoords SAL_CALL getLineGeometry()
        throw ( NMSP_UNO::RuntimeException );
    virtual SEQ( REF( com::sun::star::drawing::XCustomShapeHandle ) ) SAL_CALL getInteraction()
        throw ( NMSP_UNO::RuntimeException );
};

rtl::OUString EnhancedCustomShapeEngine_getImplementationName()
    throw ( NMSP_UNO::RuntimeException );
sal_Bool SAL_CALL EnhancedCustomShapeEngine_supportsService( const rtl::OUString& rServiceName )
    throw( NMSP_UNO::RuntimeException );
SEQ( rtl::OUString ) SAL_CALL EnhancedCustomShapeEngine_getSupportedServiceNames()
    throw( NMSP_UNO::RuntimeException );

#endif
