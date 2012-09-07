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
    VbaCommandBarHelperRef                              pCBarHelper;
    css::uno::Reference< css::container::XIndexAccess > m_xBarSettings;
    OUString                                            m_sResourceUrl;
    sal_Bool                                            m_bIsMenu;

    css::uno::Sequence< css::beans::PropertyValue > CreateMenuItemData( const OUString& sCommandURL,
                                                                        const OUString& sHelpURL,
                                                                        const OUString& sLabel,
                                                                        sal_uInt16 nType,
                                                                        const css::uno::Any& aSubMenu,
                                                                        sal_Bool isVisible,
                                                                        sal_Bool isEnabled );
    css::uno::Sequence< css::beans::PropertyValue > CreateToolbarItemData( const OUString& sCommandURL, const OUString& sHelpURL, const OUString& sLabel, sal_uInt16 nType, const css::uno::Any& aSubMenu, sal_Bool isVisible, sal_Int32 nStyle );

public:
    ScVbaCommandBarControls( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const OUString& sResourceUrl ) throw( css::uno::RuntimeException );
    sal_Bool IsMenu(){ return m_bIsMenu; }

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    // Methods
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index, const css::uno::Any& /*Index2*/ ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ov::XCommandBarControl > SAL_CALL Add( const css::uno::Any& Type, const css::uno::Any& Id, const css::uno::Any& Parameter, const css::uno::Any& Before, const css::uno::Any& Temporary ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
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
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};

#endif//SC_VBA_COMMANDBARCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
