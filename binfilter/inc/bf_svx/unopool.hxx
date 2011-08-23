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

#ifndef _SVX_UNOPOOL_HXX_
#define _SVX_UNOPOOL_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COMPHELPER_PROPERTYSETHELPER_HXX_
#include <comphelper/propertysethelper.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
namespace binfilter {

class SfxItemPool;
class SdrModel;

/** This class implements the service com.sun.star.drawing.Defaults.
    It works on the SfxItemPool from the given model and the global
    draw object item pool.
    The class can work in a read only mode without a model. Derivated
    classes can set a model on demand by overiding getModelPool().
*/
class SvxUnoDrawPool :	public ::cppu::OWeakAggObject,
                        public ::com::sun::star::lang::XServiceInfo,
                        public ::com::sun::star::lang::XTypeProvider,
                        public comphelper::PropertySetHelper
{
public:
    SvxUnoDrawPool( SdrModel* pModel, sal_Int32 nServiceId ) throw();

    /** deprecated */
    SvxUnoDrawPool( SdrModel* pModel ) throw();
    virtual ~SvxUnoDrawPool() throw();

    /** This returns the item pool from the given model, or the default pool if there is no model and bReadOnly is true.
        If bReadOnly is false and there is no model the default implementation returns NULL.
    */
    virtual SfxItemPool* getModelPool( sal_Bool bReadOnly ) throw();

    // overiden helpers from comphelper::PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::beans::PropertyState* pStates ) throw(::com::sun::star::beans::UnknownPropertyException );
    virtual void _setPropertyToDefault( const comphelper::PropertyMapEntry* pEntry )  throw(::com::sun::star::beans::UnknownPropertyException );
    virtual ::com::sun::star::uno::Any _getPropertyDefault( const comphelper::PropertyMapEntry* pEntry ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

protected:
    void init();

    virtual void getAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException); 
    virtual void putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException);

protected:
    SdrModel* mpModel;
    SfxItemPool* mpDefaultsPool;
};

}//end of namespace binfilter
#endif
