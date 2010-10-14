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
#include "vbavariables.hxx"
#include "vbavariable.hxx"
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

uno::Reference< container::XIndexAccess > createVariablesAccess( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertyAccess >& xUserDefined ) throw ( uno::RuntimeException )
{
    // FIXME: the performance is poor?
    XNamedObjectCollectionHelper< word::XVariable >::XNamedVec mVariables;
    const uno::Sequence< beans::PropertyValue > props = xUserDefined->getPropertyValues();
    sal_Int32 nCount = props.getLength();
    mVariables.reserve( nCount );
    for( sal_Int32 i=0; i < nCount; i++ )
        mVariables.push_back( uno::Reference< word::XVariable > ( new SwVbaVariable( xParent, xContext, xUserDefined, props[i].Name ) ) );

    uno::Reference< container::XIndexAccess > xVariables( new XNamedObjectCollectionHelper< word::XVariable >( mVariables ) );
    return xVariables;
}

SwVbaVariables::SwVbaVariables( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertyAccess >& rUserDefined ): SwVbaVariables_BASE( xParent, xContext, createVariablesAccess( xParent, xContext, rUserDefined ) ),  mxUserDefined( rUserDefined )
{
}
// XEnumerationAccess
uno::Type
SwVbaVariables::getElementType() throw (uno::RuntimeException)
{
    return word::XVariable::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaVariables::createEnumeration() throw (uno::RuntimeException)
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
SwVbaVariables::Add( const rtl::OUString& rName, const uno::Any& rValue ) throw (uno::RuntimeException)
{
    uno::Any aValue;
    if( rValue.hasValue() )
        aValue = rValue;
    else
        aValue <<= rtl::OUString();
    uno::Reference< beans::XPropertyContainer > xPropertyContainer( mxUserDefined, uno::UNO_QUERY_THROW );
    xPropertyContainer->addProperty( rName, beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::REMOVEABLE, aValue );

    return uno::makeAny( uno::Reference< word::XVariable >( new SwVbaVariable( getParent(), mxContext, mxUserDefined, rName ) ) );
}

rtl::OUString&
SwVbaVariables::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaVariables") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
SwVbaVariables::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Variables") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
