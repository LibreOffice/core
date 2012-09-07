/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef VBAHELPER_MSFORMS_VBANEWFONT_HXX
#define VBAHELPER_MSFORMS_VBANEWFONT_HXX

#include <ooo/vba/msforms/XNewFont.hpp>
#include <vbahelper/vbahelperinterface.hxx>

// ============================================================================

typedef InheritedHelperInterfaceImpl1< ov::msforms::XNewFont > VbaNewFont_BASE;

class VbaNewFont : public VbaNewFont_BASE
{
public:
    VbaNewFont(
        const css::uno::Reference< ov::XHelperInterface >& rxParent,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::beans::XPropertySet >& rxModelProps ) throw (css::uno::RuntimeException);

    // XNewFont attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const OUString& rName ) throw (css::uno::RuntimeException);
    virtual double SAL_CALL getSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSize( double fSize ) throw (css::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getCharset() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCharset( sal_Int16 nCharset ) throw (css::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getWeight() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setWeight( sal_Int16 nWeight ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBold( sal_Bool bBold ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setItalic( sal_Bool bItalic ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getUnderline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUnderline( sal_Bool bUnderline ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStrikethrough( sal_Bool bStrikethrough ) throw (css::uno::RuntimeException);

    // XHelperInterface
    VBAHELPER_DECL_XHELPERINTERFACE

private:
    css::uno::Reference< css::beans::XPropertySet > mxProps;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
