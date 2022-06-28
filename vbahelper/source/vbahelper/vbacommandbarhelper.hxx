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
#ifndef INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBARHELPER_HXX
#define INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBARHELPER_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <memory>

inline constexpr OUStringLiteral ITEM_DESCRIPTOR_COMMANDURL = u"CommandURL";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_HELPURL = u"HelpURL";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_CONTAINER = u"ItemDescriptorContainer";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_LABEL = u"Label";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_TYPE = u"Type";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_STYLE = u"Style";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_ISVISIBLE = u"IsVisible";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_UINAME = u"UIName";
inline constexpr OUStringLiteral ITEM_DESCRIPTOR_ENABLED = u"Enabled";

inline constexpr OUStringLiteral ITEM_MENUBAR_URL = u"private:resource/menubar/menubar";
constexpr char16_t ITEM_TOOLBAR_URL[] = u"private:resource/toolbar/";

inline constexpr OUStringLiteral CUSTOM_TOOLBAR_STR = u"custom_toolbar_";
inline constexpr OUStringLiteral CUSTOM_MENU_STR = u"vnd.openoffice.org:CustomMenu";

class VbaCommandBarHelper;
typedef std::shared_ptr< VbaCommandBarHelper > VbaCommandBarHelperRef;

class VbaCommandBarHelper
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xDocCfgMgr; // current document
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xAppCfgMgr;
    css::uno::Reference< css::container::XNameAccess > m_xWindowState;
    OUString maModuleId;

    /// @throws css::uno::RuntimeException
    void Init();
    /// @throws css::uno::RuntimeException
    bool hasToolbar( const OUString& sResourceUrl, std::u16string_view sName );
public:
    /// @throws css::uno::RuntimeException
    VbaCommandBarHelper( css::uno::Reference< css::uno::XComponentContext > xContext, css::uno::Reference< css::frame::XModel > xModel );

    const css::uno::Reference< css::frame::XModel >& getModel() const { return mxModel; }

    /// @throws css::uno::RuntimeException
    const css::uno::Reference< css::container::XNameAccess >& getPersistentWindowState() const
    {
        return m_xWindowState;
    }
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::container::XIndexAccess > getSettings( const OUString& sResourceUrl );
    /// @throws css::uno::RuntimeException
    void removeSettings( const OUString& sResourceUrl );
    /// @throws css::uno::RuntimeException
    void ApplyTempChange( const OUString& sResourceUrl, const css::uno::Reference< css::container::XIndexAccess >& xSettings);

    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::frame::XLayoutManager > getLayoutManager() const;

    const OUString& getModuleId() const { return maModuleId; }
    /// @throws css::uno::RuntimeException
    OUString findToolbarByName( const css::uno::Reference< css::container::XNameAccess >& xNameAccess, const OUString& sName );
    /// @throws css::uno::RuntimeException
    static sal_Int32 findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, std::u16string_view sName, bool bMenu );
    static OUString generateCustomURL();
};

#endif // INCLUDED_VBAHELPER_SOURCE_VBAHELPER_VBACOMMANDBARHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
