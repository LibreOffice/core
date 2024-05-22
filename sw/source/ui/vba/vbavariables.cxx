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
#include "vbavariables.hxx"
#include "vbavariable.hxx"
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/// @throws uno::RuntimeException
static uno::Reference< container::XIndexAccess > createVariablesAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertyAccess >& xUserDefined )
{
    // FIXME: the performance is poor?
    XNamedObjectCollectionHelper< word::XVariable >::XNamedVec aVariables;
    const uno::Sequence< beans::PropertyValue > props = xUserDefined->getPropertyValues();
    sal_Int32 nCount = props.getLength();
    aVariables.reserve( nCount );
    std::transform(props.begin(), props.end(), std::back_inserter(aVariables),
        [&xParent, &xContext, &xUserDefined](const beans::PropertyValue& rProp) -> uno::Reference< word::XVariable > {
            return uno::Reference< word::XVariable > ( new SwVbaVariable( xParent, xContext, xUserDefined, rProp.Name ) ); });

    uno::Reference< container::XIndexAccess > xVariables( new XNamedObjectCollectionHelper< word::XVariable >( std::move(aVariables) ) );
    return xVariables;
}

SwVbaVariables::SwVbaVariables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertyAccess >& rUserDefined ): SwVbaVariables_BASE( xParent, xContext, createVariablesAccess( xParent, xContext, rUserDefined ) ),  mxUserDefined( rUserDefined )
{
}
// XEnumerationAccess
uno::Type
SwVbaVariables::getElementType()
{
    return cppu::UnoType<word::XVariable>::get();
}
uno::Reference< container::XEnumeration >
SwVbaVariables::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return xEnumerationAccess->createEnumeration();
}

uno::Any
SwVbaVariables::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

uno::Any SAL_CALL
SwVbaVariables::Add( const OUString& rName, const uno::Any& rValue )
{
    uno::Any aValue;
    if( rValue.hasValue() )
        aValue = rValue;
    else
        aValue <<= OUString();
    uno::Reference< beans::XPropertyContainer > xPropertyContainer( mxUserDefined, uno::UNO_QUERY_THROW );
    xPropertyContainer->addProperty( rName, beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::REMOVABLE, aValue );

    return uno::Any( uno::Reference< word::XVariable >( new SwVbaVariable( getParent(), mxContext, mxUserDefined, rName ) ) );
}

OUString
SwVbaVariables::getServiceImplName()
{
    return u"SwVbaVariables"_ustr;
}

css::uno::Sequence<OUString>
SwVbaVariables::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.Variables"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
