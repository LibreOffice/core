/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_MISCUNO_HXX
#define SC_MISCUNO_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <cppuhelper/implbase2.hxx>
namespace binfilter {


#define SC_SIMPLE_SERVICE_INFO( ClassName, ClassNameAscii, ServiceAscii )			 \
::rtl::OUString SAL_CALL ClassName::getImplementationName()						 \
    throw(::com::sun::star::uno::RuntimeException)									 \
{																					 \
    return ::rtl::OUString::createFromAscii(ClassNameAscii);						 \
}																					 \
sal_Bool SAL_CALL ClassName::supportsService( const ::rtl::OUString& ServiceName ) \
    throw(::com::sun::star::uno::RuntimeException)									 \
{																					 \
    return !ServiceName.compareToAscii(ServiceAscii);								 \
}																					 \
::com::sun::star::uno::Sequence< ::rtl::OUString >									 \
    SAL_CALL ClassName::getSupportedServiceNames(void)							 \
    throw(::com::sun::star::uno::RuntimeException)									 \
{																					 \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aRet(1);						 \
    ::rtl::OUString* pArray = aRet.getArray();										 \
    pArray[0] = ::rtl::OUString::createFromAscii(ServiceAscii);					 \
    return aRet;																	 \
}

#define SC_IMPL_DUMMY_PROPERTY_LISTENER( ClassName )								\
    void SAL_CALL ClassName::addPropertyChangeListener( const ::rtl::OUString&,		\
                            const uno::Reference<beans::XPropertyChangeListener>&)	\
                            throw(beans::UnknownPropertyException,					\
                            lang::WrappedTargetException, uno::RuntimeException)	\
    { DBG_ERROR("not implemented"); }												\
    void SAL_CALL ClassName::removePropertyChangeListener( const ::rtl::OUString&,	\
                            const uno::Reference<beans::XPropertyChangeListener>&)	\
                            throw(beans::UnknownPropertyException,					\
                            lang::WrappedTargetException, uno::RuntimeException)	\
    { DBG_ERROR("not implemented"); }												\
    void SAL_CALL ClassName::addVetoableChangeListener( const ::rtl::OUString&,		\
                            const uno::Reference<beans::XVetoableChangeListener>&)	\
                            throw(beans::UnknownPropertyException,					\
                            lang::WrappedTargetException, uno::RuntimeException)	\
    { DBG_ERROR("not implemented"); }												\
    void SAL_CALL ClassName::removeVetoableChangeListener( const ::rtl::OUString&,	\
                            const uno::Reference<beans::XVetoableChangeListener>&)	\
                            throw(beans::UnknownPropertyException,					\
                            lang::WrappedTargetException, uno::RuntimeException)	\
    { DBG_ERROR("not implemented"); }


#define SC_QUERYINTERFACE(x)	\
    if (rType == getCppuType((const uno::Reference<x>*)0))	\
    { uno::Any aR; aR <<= uno::Reference<x>(this); return aR; }

// SC_QUERY_MULTIPLE( XElementAccess, XIndexAccess ):
//	use if interface is used several times in one class

#define SC_QUERY_MULTIPLE(x,y)	\
    if (rType == getCppuType((const uno::Reference<x>*)0))	\
    { uno::Any aR; aR <<= uno::Reference<x>(static_cast<y*>(this)); return aR; }


class ScIndexEnumeration : public cppu::WeakImplHelper2<
                                ::com::sun::star::container::XEnumeration,
                                ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xIndex;
    ::rtl::OUString           sServiceName;
    sal_Int32				nPos;

public:
                            ScIndexEnumeration(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::container::XIndexAccess>& rInd, const ::rtl::OUString& rServiceName);
    virtual					~ScIndexEnumeration();

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScEmptyEnumerationAccess : public cppu::WeakImplHelper2<
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::lang::XServiceInfo >
{
public:
    virtual					~ScEmptyEnumerationAccess();

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScEmptyEnumeration : public cppu::WeakImplHelper2<
                                ::com::sun::star::container::XEnumeration,
                                ::com::sun::star::lang::XServiceInfo >
{
public:
                            ScEmptyEnumeration();
    virtual					~ScEmptyEnumeration();

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};

//	new (uno 3) variant
class ScNameToIndexAccess : public cppu::WeakImplHelper2<
                                ::com::sun::star::container::XIndexAccess,
                                ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xNameAccess;
    ::com::sun::star::uno::Sequence<rtl::OUString> aNames;

public:
                            ScNameToIndexAccess(
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::container::XNameAccess>& rNameObj );
    virtual					~ScNameToIndexAccess();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScPrintSettingsObj : public cppu::WeakImplHelper2<
                                ::com::sun::star::beans::XPropertySet,
                                ::com::sun::star::lang::XServiceInfo >
{
public:
    virtual					~ScPrintSettingsObj();

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
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScUnoHelpFunctions
{
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                            AnyToInterface( const ::com::sun::star::uno::Any& rAny );
    static sal_Bool			GetBoolProperty( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, sal_Bool bDefault = sal_False );
    static sal_Int32		GetLongProperty( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, long nDefault = 0 );
    static sal_Int32		GetEnumProperty( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::beans::XPropertySet>& xProp,
                                            const ::rtl::OUString& rName, long nDefault );

    static sal_Bool			GetBoolFromAny( const ::com::sun::star::uno::Any& aAny );
    static sal_Int16		GetInt16FromAny( const ::com::sun::star::uno::Any& aAny );
    static sal_Int32		GetInt32FromAny( const ::com::sun::star::uno::Any& aAny );
    static sal_Int32		GetEnumFromAny( const ::com::sun::star::uno::Any& aAny );
    static void				SetBoolInAny( ::com::sun::star::uno::Any& rAny, sal_Bool bValue );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
