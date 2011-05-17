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

#ifndef SC_VBA_FILESEARCH_HXX
#define SC_VBA_FILESEARCH_HXX

#include <ooo/vba/XFileSearch.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace css = ::com::sun::star;

typedef InheritedHelperInterfaceImpl1< ooo::vba::XFileSearch > ScVbaFileSearchImpl_BASE;

class ScVbaApplication;

class ScVbaFileSearch : public ScVbaFileSearchImpl_BASE
{
private:
    rtl::OUString   m_sFileName;
    rtl::OUString   m_sLookIn;
    sal_Bool          m_bSearchSubFolders;
    sal_Bool          m_bMatchTextExactly;
    ScVbaApplication* m_pApplication;
    css::uno::Sequence< rtl::OUString > m_aSearchedFiles;

    ::rtl::OUString getInitPath() throw (css::uno::RuntimeException);

public:
    ScVbaFileSearch( ScVbaApplication* pApp, const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~ScVbaFileSearch();

    // Attributes
    virtual ::rtl::OUString SAL_CALL getFileName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFileName(const ::rtl::OUString& _fileName ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLookIn() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLookIn(const ::rtl::OUString& _lookIn ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getSearchSubFolders() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSearchSubFolders( sal_Bool _searchSubFolders ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getMatchTextExactly() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMatchTextExactly( sal_Bool _matchTextExactly ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::XFoundFiles > SAL_CALL getFoundFiles() throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL Execute(  )  throw (css::uno::RuntimeException);
    virtual void SAL_CALL NewSearch(  )  throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence< rtl::OUString > getServiceNames();
};

#endif /* SC_VBA_FILESEARCH_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
