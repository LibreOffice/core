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
#ifndef INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBAR_HXX
#define INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBAR_HXX

#include <ooo/vba/XCommandBar.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include "vbacommandbarhelper.hxx"

typedef InheritedHelperInterfaceWeakImpl< ov::XCommandBar > CommandBar_BASE;

class ScVbaCommandBar : public CommandBar_BASE
{
private:
    VbaCommandBarHelperRef pCBarHelper;
    css::uno::Reference< css::container::XIndexAccess > m_xBarSettings;
    OUString    m_sResourceUrl;
    bool        m_bIsMenu;

public:
    ScVbaCommandBar( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, VbaCommandBarHelperRef pHelper, const css::uno::Reference< css::container::XIndexAccess >& xBarSettings, const OUString& sResourceUrl, bool bIsMenu ) throw( css::uno::RuntimeException );

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL Type(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL FindControl( const css::uno::Any& aType, const css::uno::Any& aId, const css::uno::Any& aTag, const css::uno::Any& aVisible, const css::uno::Any& aRecursive ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

/** Dummy command bar implementation. Does nothing but provide its name. */
class VbaDummyCommandBar : public CommandBar_BASE
{
public:
    VbaDummyCommandBar(
            const css::uno::Reference< ov::XHelperInterface >& xParent,
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const OUString& rName,
            sal_Int32 nType ) throw( css::uno::RuntimeException );

    // Attributes
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setName( const OUString& _name ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL Controls( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL Type(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL FindControl( const css::uno::Any& aType, const css::uno::Any& aId, const css::uno::Any& aTag, const css::uno::Any& aVisible, const css::uno::Any& aRecursive ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;

private:
    OUString maName;
    sal_Int32 mnType;
};

#endif // INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
