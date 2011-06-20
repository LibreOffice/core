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

#include "vbafiledialogselecteditems.hxx"

using namespace ::com::sun::star;

VbaFileDialogSelectedItems::VbaFileDialogSelectedItems( const css::uno::Reference< ov::XHelperInterface >& xParent,
                                                       const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                                       const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess
                                                       )
    : FileDialogSelectedItems_BASE( xParent, xContext,  xIndexAccess  )
{
}


rtl::OUString& VbaFileDialogSelectedItems::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaFileDialogSelectedItems") );
    return sImplName;

}
css::uno::Sequence<rtl::OUString> VbaFileDialogSelectedItems::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.FileDialogSelectedItems" ) );
    }
    return aServiceNames;

}

css::uno::Any VbaFileDialogSelectedItems::createCollectionObject( const css::uno::Any& aSource )
{
    css::uno::Any aRet;
    aRet = aSource;
    return aRet;
}


css::uno::Type SAL_CALL
VbaFileDialogSelectedItems::getElementType() throw (css::uno::RuntimeException)
{
    return ooo::vba::XFileDialogSelectedItems::static_type(0);
}

css::uno::Reference< css::container::XEnumeration > SAL_CALL
VbaFileDialogSelectedItems::createEnumeration() throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XEnumeration > xEnumRet(m_xIndexAccess, css::uno::UNO_QUERY);
    return xEnumRet;
}


//VbaFileDialogSelectedObj
//////////////////////////////////////////////////////////////////////////

VbaFileDialogSelectedObj::VbaFileDialogSelectedObj()
{
    m_nIndex = 0;
}


sal_Bool
VbaFileDialogSelectedObj::SetSelectedFile(css::uno::Sequence<rtl::OUString> &sFList)
{
    m_sFileList = sFList;
    return sal_True;
}

sal_Int32 SAL_CALL
VbaFileDialogSelectedObj::getCount() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nListCnt = m_sFileList.getLength();
    return nListCnt;
}

::com::sun::star::uno::Any SAL_CALL VbaFileDialogSelectedObj::getByIndex( sal_Int32 nIndex )
throw(::com::sun::star::lang::IndexOutOfBoundsException,
      ::com::sun::star::lang::WrappedTargetException,
      ::com::sun::star::uno::RuntimeException)
{
    css::uno::Any  aRet;

    if ( nIndex >= getCount() )
    {
        throw css::lang::IndexOutOfBoundsException();
    }

    return uno::makeAny(m_sFileList[nIndex]);
}

::com::sun::star::uno::Type SAL_CALL
VbaFileDialogSelectedObj::getElementType()
throw(::com::sun::star::uno::RuntimeException)
{
    return getCppuType((uno::Reference<com::sun::star::container::XIndexAccess>*)0);
}

sal_Bool SAL_CALL VbaFileDialogSelectedObj::hasElements()
throw(::com::sun::star::uno::RuntimeException)
{
    return ( getCount() != 0 );
}

::sal_Bool SAL_CALL
VbaFileDialogSelectedObj::hasMoreElements(  )
throw (uno::RuntimeException)
{
    if (getCount() > m_nIndex)
    {
        return sal_True;
    }
    return false;
}

uno::Any SAL_CALL
VbaFileDialogSelectedObj::nextElement(  )
throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!hasMoreElements())
    {
        throw container::NoSuchElementException();
    }

    return uno::makeAny(m_sFileList[m_nIndex++]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
