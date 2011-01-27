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
#ifndef VBA_DIALOGS_BASE_HXX
#define VBA_DIALOGS_BASE_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/XHelperInterface.hpp>
#include <ooo/vba/XDialogsBase.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/frame/XModel.hpp>

typedef InheritedHelperInterfaceImpl1< ov::XDialogsBase > VbaDialogsBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDialogsBase : public VbaDialogsBase_BASE
{
protected:
        css::uno::Reference< css::frame::XModel > m_xModel;
public:
    VbaDialogsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > &xContext, const css::uno::Reference< css::frame::XModel >& xModel ): VbaDialogsBase_BASE( xParent, xContext ), m_xModel( xModel ) {}
    virtual ~VbaDialogsBase() {}

    // XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
};

#endif /* VBA_DIALOGS_BASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
