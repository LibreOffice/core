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

#include "precompiled_basic.hxx"
#include "comenumwrapper.hxx"

using namespace ::com::sun::star;

::sal_Bool SAL_CALL ComEnumerationWrapper::hasMoreElements()
    throw ( uno::RuntimeException )
{
    sal_Bool bResult = sal_False;

    try
    {
        if ( m_xInvocation.is() )
        {
            sal_Int32 nLength = 0;
            bResult =
                ( ( m_xInvocation->getValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "length" ) ) ) >>= nLength )
                  && nLength > m_nCurInd );
        }
    }
    catch( uno::Exception& )
    {}

    return bResult;
}

uno::Any SAL_CALL ComEnumerationWrapper::nextElement()
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    try
    {
        if ( m_xInvocation.is() )
        {
            uno::Sequence< sal_Int16 > aNamedParamIndex;
            uno::Sequence< uno::Any > aNamedParam;
            uno::Sequence< uno::Any > aArgs( 1 );

            aArgs[0] <<= m_nCurInd++;

            return m_xInvocation->invoke( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "item" ) ),
                                          aArgs,
                                          aNamedParamIndex,
                                          aNamedParam );
        }
    }
    catch( uno::Exception& )
    {}

    throw container::NoSuchElementException();
}


