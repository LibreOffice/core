/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbadialog.hxx,v $
 * $Revision: 1.4 $
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
#ifndef SC_VBA_DIALOG_HXX
#define SC_VBA_DIALOG_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <ooo/vba/XGlobals.hpp>
#include <ooo/vba/excel/XApplication.hpp>
#include <ooo/vba/excel/XDialog.hpp>

#include "vbahelperinterface.hxx"
#include "vbadialog.hxx"

typedef InheritedHelperInterfaceImpl1< ov::excel::XDialog > ScVbaDialog_BASE;

class ScVbaDialog : public ScVbaDialog_BASE
{
    sal_Int32 mnIndex;
public:
    ScVbaDialog( const css::uno::Reference< ov::XHelperInterface >& xParent, sal_Int32 nIndex, const css::uno::Reference< css::uno::XComponentContext > xContext ):ScVbaDialog_BASE( xParent, xContext ), mnIndex( nIndex ) {}
    virtual ~ScVbaDialog() {}

    // Methods
    virtual void SAL_CALL Show() throw (css::uno::RuntimeException);
    rtl::OUString mapIndexToName( sal_Int32 nIndex );
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif /* SC_VBA_DIALOG_HXX */
