/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: renderer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:11:58 $
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

#ifndef _GOODIES_RENDERER_HXX
#define _GOODIES_RENDERER_HXX

#include <tools/gen.hxx>

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICRENDERER_HPP_
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#endif


using namespace com::sun::star;

class OutputDevice;

namespace unographic {

// -------------------
// - GraphicRenderer -
// -------------------

class GraphicRendererVCL : public ::cppu::OWeakAggObject,
                           public ::com::sun::star::lang::XServiceInfo,
                           public ::com::sun::star::lang::XTypeProvider,
                           public ::comphelper::PropertySetHelper,
                           public ::com::sun::star::graphic::XGraphicRenderer
{
public:

    GraphicRendererVCL();
    ~GraphicRendererVCL() throw();

    static ::rtl::OUString getImplementationName_Static() throw();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString >  getSupportedServiceNames_Static() throw();

protected:

    static ::comphelper::PropertySetInfo* createPropertySetInfo();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    // XGraphicRenderer
    virtual void SAL_CALL render( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& Graphic ) throw (::com::sun::star::uno::RuntimeException);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > mxDevice;

    OutputDevice*               mpOutDev;
    Rectangle                   maDestRect;
    ::com::sun::star::uno::Any  maRenderData;
};

}

#endif
