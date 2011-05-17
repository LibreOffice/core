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
#ifndef SC_VBA_COMMANDBARCONTROLS_HXX
#define SC_VBA_COMMANDBARCONTROLS_HXX

#include <ooo/vba/XCommandBarControls.hpp>
#include <com/sun/star/awt/XMenu.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>
#include "vbacommandbarhelper.hxx"

typedef CollTestImplHelper< ov::XCommandBarControls > CommandBarControls_BASE;

class ScVbaCommandBarControls : public CommandBarControls_BASE
{
private:
    VbaCommandBarHelperRef pCBarHelper;
    css::uno::Reference< css::container::XIndexAccess >          m_xBarSettings;
    rtl::OUString       m_sResourceUrl;
    css::uno::Reference< css::awt::XMenu >                        m_xMenu;
    sal_Bool m_bIsMenu;

    css::uno::Sequence< css::beans::PropertyValue > CreateMenuItemData( const rtl::OUString& sCommandURL, const rtl::OUString& sHelpURL, const rtl::OUString& sLabel, sal_uInt16 nType, const css::uno::Any& aSubMenu );
    css::uno::Sequence< css::beans::PropertyValue > CreateToolbarItemData( const rtl::OUString& sCommandURL, const rtl::OUString& sHelpURL, const rtl::OUString& sLabel, sal_uInt16 nType, const css::uno::Any& aSubMenu, sal_Bool isVisible, sal_Int32 nStyle );

public:
    ScVbaCommandBarControls( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const rtl::OUString& sResourceUrl, const css::uno::Reference< css::awt::XMenu >& xMenu ) throw( css::uno::RuntimeException );
    sal_Bool IsMenu() const { return m_bIsMenu; }

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // Methods
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index, const css::uno::Any& /*Index2*/ ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::XCommandBarControl > SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& Id, const css::uno::Any& Parameter, const css::uno::Any& Before, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

class VbaDummyCommandBarControls : public CommandBarControls_BASE
{
public:
    VbaDummyCommandBarControls(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext ) throw( css::uno::RuntimeException );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // Methods
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index, const css::uno::Any& /*Index2*/ ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::XCommandBarControl > SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& Id, const css::uno::Any& Parameter, const css::uno::Any& Before, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SC_VBA_COMMANDBARCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
