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
    virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& rName ) throw (css::uno::RuntimeException);
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
