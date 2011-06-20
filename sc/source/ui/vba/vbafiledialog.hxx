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

#ifndef _vbafiledialog_hxx_
#define _vbafiledialog_hxx_

#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbadialogsbase.hxx>
#include <ooo/vba/XFileDialog.hpp>
#include "vbafiledialogselecteditems.hxx"


typedef cppu::ImplInheritanceHelper1< VbaDialogsBase, ov::XFileDialog > ScVbaFileDialog_BASE;

class ScVbaFileDialog : public ScVbaFileDialog_BASE
{
public:

    ScVbaFileDialog( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > &xContext, const css::uno::Reference< css::frame::XModel >& xModel ) ;
    virtual ~ScVbaFileDialog() ;

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
    //XFileDialog
    virtual css::uno::Reference< ov::XFileDialogSelectedItems > SAL_CALL getSelectedItems() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL Show(  ) throw (::com::sun::star::uno::RuntimeException) ;

private:
    css::uno::Sequence < rtl::OUString > m_sSelectedItems;
    VbaFileDialogSelectedItems *m_pFileDialogSelectedItems;
    VbaFileDialogSelectedObj   m_FileDialogSelectedObj;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
