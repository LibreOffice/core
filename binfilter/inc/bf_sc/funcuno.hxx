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

#ifndef SC_FUNCUNO_HXX
#define SC_FUNCUNO_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XFUNCTIONACCESS_HPP_
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif
namespace binfilter {

class ScDocument;
class ScDocOptions;


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    ScFunctionAccess_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& );


class ScTempDocCache
{
private:
    ScDocument*		pDoc;
    BOOL			bInUse;

public:
                ScTempDocCache();
                ~ScTempDocCache();

    ScDocument*	GetDocument() const		{ return pDoc; }
    BOOL		IsInUse() const			{ return bInUse; }
    void		SetInUse( BOOL bSet )	{ bInUse = bSet; }

    void		SetDocument( ScDocument* pNew );
    void		Clear();
};

class ScFunctionAccess : public cppu::WeakImplHelper3<
                                        ::com::sun::star::sheet::XFunctionAccess,
                                        ::com::sun::star::beans::XPropertySet,
                                        ::com::sun::star::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScTempDocCache	aDocCache;
    ScDocOptions* 	pOptions;
    BOOL			bInvalid;

public:
                            ScFunctionAccess();
    virtual					~ScFunctionAccess();

    static ::rtl::OUString	getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XFunctionAccess
    virtual ::com::sun::star::uno::Any SAL_CALL callFunction(
                                    const ::rtl::OUString& aName,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::uno::Any >& aArguments )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

