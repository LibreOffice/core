/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
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

#include "vbafoundfiles.hxx"

////////////////////////////////VbaFoundFilesEnum//////////////////////////////////////////
VbaFoundFilesEnum::VbaFoundFilesEnum() : m_nIndex(0)
{

}

VbaFoundFilesEnum::VbaFoundFilesEnum( css::uno::Sequence<rtl::OUString>& sFileList ) : m_nIndex(0), m_sFileList(sFileList)
{

}

VbaFoundFilesEnum::~VbaFoundFilesEnum()
{

}

void VbaFoundFilesEnum::SetFileList( css::uno::Sequence<rtl::OUString>& sFileList )
{
    m_nIndex = 0;
    m_sFileList = sFileList;
}

sal_Int32 SAL_CALL VbaFoundFilesEnum::getCount() throw (css::uno::RuntimeException)
{
    return m_sFileList.getLength();
}

css::uno::Any SAL_CALL VbaFoundFilesEnum::getByIndex( sal_Int32 nIndex )
    throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    if ( nIndex >= getCount() )
    {
        throw css::lang::IndexOutOfBoundsException();
    }

    return css::uno::makeAny( m_sFileList[nIndex] );
}

css::uno::Type SAL_CALL VbaFoundFilesEnum::getElementType() throw (css::uno::RuntimeException)
{
    return getCppuType((css::uno::Reference< css::container::XIndexAccess >*)0);
}

sal_Bool SAL_CALL VbaFoundFilesEnum::hasElements() throw (css::uno::RuntimeException)
{
    return ( getCount() != 0 );
}

sal_Bool SAL_CALL VbaFoundFilesEnum::hasMoreElements() throw (css::uno::RuntimeException)
{
    if ( getCount() > m_nIndex )
    {
        return sal_True;
    }
    return false;
}

css::uno::Any SAL_CALL VbaFoundFilesEnum::nextElement() throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException)
{
    if ( !hasMoreElements() )
    {
        throw css::container::NoSuchElementException();
    }

    return css::uno::makeAny( m_sFileList[m_nIndex++] );
}

////////////////////////////////VbaFoundFiles//////////////////////////////////////////
VbaFoundFiles::VbaFoundFiles( const css::uno::Reference< ov::XHelperInterface >& xParent,
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess
    ) : VbaFoundFilesImpl_BASE( xParent, xContext, xIndexAccess )
{

}

VbaFoundFiles::~VbaFoundFiles()
{

}

css::uno::Reference< css::container::XEnumeration > VbaFoundFiles::createEnumeration() throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XEnumeration > xEnumRet( m_xIndexAccess, css::uno::UNO_QUERY );
    return xEnumRet;
}

css::uno::Any VbaFoundFiles::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

css::uno::Type VbaFoundFiles::getElementType() throw (css::uno::RuntimeException)
{
    return ov::XFoundFiles::static_type(0);
}

rtl::OUString& VbaFoundFiles::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaFoundFiles") );
    return sImplName;
}

css::uno::Sequence< rtl::OUString > VbaFoundFiles::getServiceNames()
{
    static css::uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.FoundFiles") );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
