/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_INC_MISCUNO_HXX
#define INCLUDED_SC_INC_MISCUNO_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include "scdllapi.h"

#define SC_SIMPLE_SERVICE_INFO( ClassName, ClassNameAscii, ServiceAscii )            \
OUString SAL_CALL ClassName::getImplementationName()                      \
    throw(::com::sun::star::uno::RuntimeException, std::exception)                                   \
{                                                                                    \
    return OUString(ClassNameAscii);                         \
}                                                                                    \
sal_Bool SAL_CALL ClassName::supportsService( const OUString& ServiceName ) \
    throw(::com::sun::star::uno::RuntimeException, std::exception)                                   \
{                                                                                    \
    return cppu::supportsService(this, ServiceName);                                \
}                                                                                    \
::com::sun::star::uno::Sequence< OUString >                                   \
    SAL_CALL ClassName::getSupportedServiceNames()                           \
    throw(::com::sun::star::uno::RuntimeException, std::exception)                                   \
{                                                                                    \
    ::com::sun::star::uno::Sequence< OUString > aRet(1);                      \
    OUString* pArray = aRet.getArray();                                       \
    pArray[0] = ServiceAscii;                  \
    return aRet;                                                                     \
}

#define SC_IMPL_DUMMY_PROPERTY_LISTENER( ClassName )                                \
    void SAL_CALL ClassName::addPropertyChangeListener( const OUString&,       \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException, std::exception)    \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::removePropertyChangeListener( const OUString&,    \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException, std::exception)    \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::addVetoableChangeListener( const OUString&,       \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException, std::exception)    \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::removeVetoableChangeListener( const OUString&,    \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
                            throw(beans::UnknownPropertyException,                  \
                            lang::WrappedTargetException, uno::RuntimeException, std::exception)    \
    { OSL_FAIL("not implemented"); }

#define SC_QUERYINTERFACE(x)    \
    if (rType == cppu::UnoType<x>::get())  \
    { return uno::makeAny(uno::Reference<x>(this)); }

// SC_QUERY_MULTIPLE( XElementAccess, XIndexAccess ):
//  use if interface is used several times in one class

#define SC_QUERY_MULTIPLE(x,y)  \
    if (rType == cppu::UnoType<x>::get())  \
    { uno::Any aR; aR <<= uno::Reference<x>(static_cast<y*>(this)); return aR; }

class ScIndexEnumeration : public cppu::WeakImplHelper<
                                com::sun::star::container::XEnumeration,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> xIndex;
    OUString           sServiceName;
    sal_Int32               nPos;

public:
                            ScIndexEnumeration(const com::sun::star::uno::Reference<
                                com::sun::star::container::XIndexAccess>& rInd, const OUString& rServiceName);
    virtual                 ~ScIndexEnumeration();

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

//  new (uno 3) variant
class ScNameToIndexAccess : public cppu::WeakImplHelper<
                                com::sun::star::container::XIndexAccess,
                                com::sun::star::lang::XServiceInfo >
{
private:
    com::sun::star::uno::Reference<com::sun::star::container::XNameAccess> xNameAccess;
    com::sun::star::uno::Sequence<OUString> aNames;

public:
                            ScNameToIndexAccess(
                                const com::sun::star::uno::Reference<
                                    com::sun::star::container::XNameAccess>& rNameObj );
    virtual                 ~ScNameToIndexAccess();

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

class SC_DLLPUBLIC ScUnoHelpFunctions
{
public:
    static com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
                            AnyToInterface( const com::sun::star::uno::Any& rAny );
    static bool             GetBoolProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const OUString& rName, bool bDefault = false );
    static sal_Int32        GetLongProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const OUString& rName, long nDefault = 0 );
    static sal_Int32        GetEnumProperty( const com::sun::star::uno::Reference<
                                                com::sun::star::beans::XPropertySet>& xProp,
                                            const OUString& rName, long nDefault );
    static OUString  GetStringProperty(
        const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xProp,
        const OUString& rName, const OUString& rDefault );

    static bool             GetBoolFromAny( const com::sun::star::uno::Any& aAny );
    static sal_Int16        GetInt16FromAny( const com::sun::star::uno::Any& aAny );
    static sal_Int32        GetInt32FromAny( const com::sun::star::uno::Any& aAny );
    static sal_Int32        GetEnumFromAny( const com::sun::star::uno::Any& aAny );
    static void             SetBoolInAny( com::sun::star::uno::Any& rAny, bool bValue );

    static void             SetOptionalPropertyValue(
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rPropSet,
        const sal_Char* pPropName, const ::com::sun::star::uno::Any& rVal );

    template<typename ValueType>
    static void             SetOptionalPropertyValue(
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rPropSet,
        const sal_Char* pPropName, const ValueType& rVal )
    {
        ::com::sun::star::uno::Any any;
        any <<= rVal;
        SetOptionalPropertyValue(rPropSet, pPropName, any);
    }

    template<typename ValueType>
    static com::sun::star::uno::Sequence<ValueType> VectorToSequence( const std::vector<ValueType>& rVector )
    {
        if (rVector.empty())
            return com::sun::star::uno::Sequence<ValueType>();

        return com::sun::star::uno::Sequence<ValueType>(&rVector[0], static_cast<sal_Int32>(rVector.size()));
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
