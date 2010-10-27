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

#ifndef SC_VBA_FOUNDFILES_HXX
#define SC_VBA_FOUNDFILES_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <ooo/vba/XFoundFiles.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

namespace css = ::com::sun::star;

typedef CollTestImplHelper< ooo::vba::XFoundFiles > VbaFoundFilesImpl_BASE;

class VbaFoundFilesEnum : public cppu::WeakImplHelper2< css::container::XIndexAccess, css::container::XEnumeration >
{
private:
    sal_Int32 m_nIndex;
    css::uno::Sequence< rtl::OUString > m_sFileList;

public:
    VbaFoundFilesEnum();
    VbaFoundFilesEnum( css::uno::Sequence< rtl::OUString >& sFileList );
    ~VbaFoundFilesEnum();

    void SetFileList( css::uno::Sequence< rtl::OUString >& sFileList );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL nextElement() throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException);
};

class VbaFoundFiles : public VbaFoundFilesImpl_BASE
{
private:

public:
    VbaFoundFiles( const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess );
    virtual ~VbaFoundFiles();

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence< rtl::OUString > getServiceNames();
};

#endif /* SC_VBA_FOUNDFILES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
