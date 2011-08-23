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
#ifndef UNOMODEL_HXX
#define UNOMODEL_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif
#ifndef _SFX_SFXBASEMODEL_HXX_
#include <bf_sfx2/sfxbasemodel.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif
namespace binfilter {

class SmFormat;

//-----------------------------------------------------------------------------
class SmModel : public SfxBaseModel,
                public comphelper::PropertySetHelper,
                public ::com::sun::star::lang::XServiceInfo,
                public ::com::sun::star::lang::XUnoTunnel,
                public ::com::sun::star::view::XRenderable
{
protected:
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
public:
    SmModel( SfxObjectShell *pObjSh = 0 );
    virtual	~SmModel() throw ();

    //XInterface
    virtual 	::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual BOOL SAL_CALL supportsService(const ::rtl::OUString& ServiceName)
            throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(void)
            throw( ::com::sun::star::uno::RuntimeException );

    inline ::rtl::OUString getImplementationName_Static() throw(  );
};

inline ::rtl::OUString SmModel::getImplementationName_Static() throw(  )
{
    return ::rtl::OUString::createFromAscii("math.SmModel");
}

} //namespace binfilter
#endif
