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

#pragma once

#include <vector>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include "scdllapi.h"

#define SC_SIMPLE_SERVICE_INFO_IMPL( ClassName, ClassNameAscii )            \
OUString SAL_CALL ClassName::getImplementationName()                        \
{                                                                           \
    return ClassNameAscii;                                                  \
}                                                                           \
sal_Bool SAL_CALL ClassName::supportsService( const OUString& ServiceName ) \
{                                                                           \
    return cppu::supportsService(this, ServiceName);                        \
}

#define SC_SIMPLE_SERVICE_INFO_NAME( ClassName, ServiceAscii ) \
css::uno::Sequence< OUString >                                 \
    SAL_CALL ClassName::getSupportedServiceNames()             \
{                                                              \
    css::uno::Sequence< OUString > aRet { ServiceAscii };      \
    return aRet;                                               \
}

// Place the old mistyped variant as first element so existing code can
// continue to ask aRet[0] if it doesn't iterate; new code can iterate over the
// sequence. This mostly should be used by supportsService() iterating anyway.
#define SC_SIMPLE_SERVICE_INFO_TYPO( ClassName, ServiceAscii, ServiceAsciiMistyped ) \
css::uno::Sequence< OUString >                                                       \
    SAL_CALL ClassName::getSupportedServiceNames()                                   \
{                                                                                    \
    css::uno::Sequence< OUString > aRet { ServiceAsciiMistyped, ServiceAscii };      \
    return aRet;                                                                     \
}

#define SC_SIMPLE_SERVICE_INFO( ClassName, ClassNameAscii, ServiceAscii ) \
    SC_SIMPLE_SERVICE_INFO_IMPL( ClassName, ClassNameAscii )              \
    SC_SIMPLE_SERVICE_INFO_NAME( ClassName, ServiceAscii )

#define SC_SIMPLE_SERVICE_INFO_COMPAT( ClassName, ClassNameAscii, ServiceAscii, ServiceAsciiMistyped ) \
    SC_SIMPLE_SERVICE_INFO_IMPL( ClassName, ClassNameAscii )                                           \
    SC_SIMPLE_SERVICE_INFO_TYPO( ClassName, ServiceAscii, ServiceAsciiMistyped )


#define SC_IMPL_DUMMY_PROPERTY_LISTENER( ClassName )                                \
    void SAL_CALL ClassName::addPropertyChangeListener( const OUString&,       \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::removePropertyChangeListener( const OUString&,    \
                            const uno::Reference<beans::XPropertyChangeListener>&)  \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::addVetoableChangeListener( const OUString&,       \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
    { OSL_FAIL("not implemented"); }                                                \
    void SAL_CALL ClassName::removeVetoableChangeListener( const OUString&,    \
                            const uno::Reference<beans::XVetoableChangeListener>&)  \
    { OSL_FAIL("not implemented"); }

#define SC_QUERYINTERFACE(x)    \
    if (rType == cppu::UnoType<x>::get())  \
    { return uno::makeAny(uno::Reference<x>(this)); }

// SC_QUERY_MULTIPLE( XElementAccess, XIndexAccess ):
//  use if interface is used several times in one class

#define SC_QUERY_MULTIPLE(x,y)  \
    if (rType == cppu::UnoType<x>::get())  \
    { uno::Any aR; aR <<= uno::Reference<x>(static_cast<y*>(this)); return aR; }

class ScIndexEnumeration final : public cppu::WeakImplHelper<
                                css::container::XEnumeration,
                                css::lang::XServiceInfo >
{
private:
    css::uno::Reference<css::container::XIndexAccess> xIndex;
    OUString                sServiceName;
    sal_Int32               nPos;

public:
                            ScIndexEnumeration(const css::uno::Reference<
                                css::container::XIndexAccess>& rInd, const OUString& rServiceName);
    virtual                 ~ScIndexEnumeration() override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

//  new (uno 3) variant
class ScNameToIndexAccess final : public cppu::WeakImplHelper<
                                css::container::XIndexAccess,
                                css::lang::XServiceInfo >
{
private:
    css::uno::Reference<css::container::XNameAccess> xNameAccess;
    css::uno::Sequence<OUString> aNames;

public:
                            ScNameToIndexAccess(
                                const css::uno::Reference< css::container::XNameAccess>& rNameObj );
    virtual                 ~ScNameToIndexAccess() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

class SC_DLLPUBLIC ScUnoHelpFunctions
{
public:
    static bool             GetBoolProperty( const css::uno::Reference< css::beans::XPropertySet>& xProp,
                                            const OUString& rName, bool bDefault = false );
    static sal_Int16        GetShortProperty( const css::uno::Reference< css::beans::XPropertySet>& xProp,
                                            const OUString& rName, sal_Int16 nDefault );
    static sal_Int32        GetLongProperty( const css::uno::Reference< css::beans::XPropertySet>& xProp,
                                            const OUString& rName );
    template<typename EnumT>
    static EnumT            GetEnumProperty( const css::uno::Reference< css::beans::XPropertySet>& xProp,
                                            const OUString& rName, EnumT nDefault )
    { return static_cast<EnumT>(GetEnumPropertyImpl(xProp, rName, static_cast<sal_Int32>(nDefault))); }

    static OUString  GetStringProperty(
        const css::uno::Reference<css::beans::XPropertySet>& xProp,
        const OUString& rName, const OUString& rDefault );

    static bool             GetBoolFromAny( const css::uno::Any& aAny );
    static sal_Int16        GetInt16FromAny( const css::uno::Any& aAny );
    static sal_Int32        GetInt32FromAny( const css::uno::Any& aAny );
    static sal_Int32        GetEnumFromAny( const css::uno::Any& aAny );

    static void             SetOptionalPropertyValue(
        const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
        const char* pPropName, const css::uno::Any& rVal );

    template<typename ValueType>
    static void             SetOptionalPropertyValue(
        const css::uno::Reference< css::beans::XPropertySet >& rPropSet,
        const char* pPropName, const ValueType& rVal )
    {
        css::uno::Any any;
        any <<= rVal;
        SetOptionalPropertyValue(rPropSet, pPropName, any);
    }

    template<typename ValueType>
    static css::uno::Sequence<ValueType> VectorToSequence( const std::vector<ValueType>& rVector )
    {
        if (rVector.empty())
            return css::uno::Sequence<ValueType>();

        return css::uno::Sequence<ValueType>(&rVector[0], static_cast<sal_Int32>(rVector.size()));
    }
private:
    static sal_Int32        GetEnumPropertyImpl( const css::uno::Reference< css::beans::XPropertySet>& xProp,
                                            const OUString& rName, sal_Int32 nDefault );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
