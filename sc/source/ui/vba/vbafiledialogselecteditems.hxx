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

#ifndef _vbafiledialogselecteditems_hxx_
#define _vbafiledialogselecteditems_hxx_

#include <ooo/vba/XFileDialogSelectedItems.hpp>
#include <vbahelper/vbacollectionimpl.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

typedef CollTestImplHelper< ov::XFileDialogSelectedItems > FileDialogSelectedItems_BASE;


class VbaFileDialogSelectedItems : public FileDialogSelectedItems_BASE
{
public:
    VbaFileDialogSelectedItems( const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

protected:
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
private:
};


class VbaFileDialogSelectedObj : public cppu::WeakImplHelper2<css::container::XIndexAccess, css::container::XEnumeration>
{
public:
    VbaFileDialogSelectedObj();
    sal_Bool SetSelectedFile(css::uno::Sequence<rtl::OUString> &sFList);
    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex )
        throw(::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    //XEnumeration
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL nextElement(  )  throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);

protected:
private:
    css::uno::Sequence<rtl::OUString> m_sFileList;
    sal_Int32 m_nIndex;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
