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
#ifndef VBA_DIALOG_BASE_HXX
#define VBA_DIALOG_BASE_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XDialogBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/frame/XModel.hpp>

typedef InheritedHelperInterfaceImpl1< ov::XDialogBase > VbaDialogBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDialogBase : public VbaDialogBase_BASE
{
protected:
    sal_Int32 mnIndex;
        css::uno::Reference< css::frame::XModel > m_xModel;
public:
    VbaDialogBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nIndex ):VbaDialogBase_BASE( xParent, xContext ), mnIndex( nIndex ), m_xModel( xModel ) {}
    virtual ~VbaDialogBase() {}

    // Methods
    virtual sal_Bool SAL_CALL Show() throw (css::uno::RuntimeException);
    virtual rtl::OUString mapIndexToName( sal_Int32 nIndex ) = 0;
};

#endif /* VBA_DIALOG_BASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
