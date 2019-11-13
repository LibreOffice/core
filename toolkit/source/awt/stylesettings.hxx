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

#ifndef INCLUDED_TOOLKIT_SOURCE_AWT_STYLESETTINGS_HXX
#define INCLUDED_TOOLKIT_SOURCE_AWT_STYLESETTINGS_HXX

#include <com/sun/star/awt/XStyleSettings.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace osl
{
    class Mutex;
}

class VCLXWindow;


namespace toolkit
{


    //= WindowStyleSettings

    struct WindowStyleSettings_Data;
    typedef ::cppu::WeakImplHelper <   css::awt::XStyleSettings
                                    >   WindowStyleSettings_Base;
    class WindowStyleSettings : public WindowStyleSettings_Base
    {
    public:
        WindowStyleSettings( ::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow );
        virtual ~WindowStyleSettings() override;

        void dispose();

        // XStyleSettings
        virtual ::sal_Int32 SAL_CALL getActiveBorderColor() override;
        virtual void SAL_CALL setActiveBorderColor( ::sal_Int32 _activebordercolor ) override;
        virtual ::sal_Int32 SAL_CALL getActiveColor() override;
        virtual void SAL_CALL setActiveColor( ::sal_Int32 _activecolor ) override;
        virtual ::sal_Int32 SAL_CALL getActiveTabColor() override;
        virtual void SAL_CALL setActiveTabColor( ::sal_Int32 _activetabcolor ) override;
        virtual ::sal_Int32 SAL_CALL getActiveTextColor() override;
        virtual void SAL_CALL setActiveTextColor( ::sal_Int32 _activetextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getButtonRolloverTextColor() override;
        virtual void SAL_CALL setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getButtonTextColor() override;
        virtual void SAL_CALL setButtonTextColor( ::sal_Int32 _buttontextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getCheckedColor() override;
        virtual void SAL_CALL setCheckedColor( ::sal_Int32 _checkedcolor ) override;
        virtual ::sal_Int32 SAL_CALL getDarkShadowColor() override;
        virtual void SAL_CALL setDarkShadowColor( ::sal_Int32 _darkshadowcolor ) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveBorderColor() override;
        virtual void SAL_CALL setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor ) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveColor() override;
        virtual void SAL_CALL setDeactiveColor( ::sal_Int32 _deactivecolor ) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveTextColor() override;
        virtual void SAL_CALL setDeactiveTextColor( ::sal_Int32 _deactivetextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getDialogColor() override;
        virtual void SAL_CALL setDialogColor( ::sal_Int32 _dialogcolor ) override;
        virtual ::sal_Int32 SAL_CALL getDialogTextColor() override;
        virtual void SAL_CALL setDialogTextColor( ::sal_Int32 _dialogtextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getDisableColor() override;
        virtual void SAL_CALL setDisableColor( ::sal_Int32 _disablecolor ) override;
        virtual ::sal_Int32 SAL_CALL getFaceColor() override;
        virtual void SAL_CALL setFaceColor( ::sal_Int32 _facecolor ) override;
        virtual ::sal_Int32 SAL_CALL getFaceGradientColor() override;
        virtual ::sal_Int32 SAL_CALL getFieldColor() override;
        virtual void SAL_CALL setFieldColor( ::sal_Int32 _fieldcolor ) override;
        virtual ::sal_Int32 SAL_CALL getFieldRolloverTextColor() override;
        virtual void SAL_CALL setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getFieldTextColor() override;
        virtual void SAL_CALL setFieldTextColor( ::sal_Int32 _fieldtextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getGroupTextColor() override;
        virtual void SAL_CALL setGroupTextColor( ::sal_Int32 _grouptextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getHelpColor() override;
        virtual void SAL_CALL setHelpColor( ::sal_Int32 _helpcolor ) override;
        virtual ::sal_Int32 SAL_CALL getHelpTextColor() override;
        virtual void SAL_CALL setHelpTextColor( ::sal_Int32 _helptextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getHighlightColor() override;
        virtual void SAL_CALL setHighlightColor( ::sal_Int32 _highlightcolor ) override;
        virtual ::sal_Int32 SAL_CALL getHighlightTextColor() override;
        virtual void SAL_CALL setHighlightTextColor( ::sal_Int32 _highlighttextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getInactiveTabColor() override;
        virtual void SAL_CALL setInactiveTabColor( ::sal_Int32 _inactivetabcolor ) override;
        virtual ::sal_Int32 SAL_CALL getLabelTextColor() override;
        virtual void SAL_CALL setLabelTextColor( ::sal_Int32 _labeltextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getLightColor() override;
        virtual void SAL_CALL setLightColor( ::sal_Int32 _lightcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuBarColor() override;
        virtual void SAL_CALL setMenuBarColor( ::sal_Int32 _menubarcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuBarTextColor() override;
        virtual void SAL_CALL setMenuBarTextColor( ::sal_Int32 _menubartextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuBorderColor() override;
        virtual void SAL_CALL setMenuBorderColor( ::sal_Int32 _menubordercolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuColor() override;
        virtual void SAL_CALL setMenuColor( ::sal_Int32 _menucolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuHighlightColor() override;
        virtual void SAL_CALL setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuHighlightTextColor() override;
        virtual void SAL_CALL setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMenuTextColor() override;
        virtual void SAL_CALL setMenuTextColor( ::sal_Int32 _menutextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getMonoColor() override;
        virtual void SAL_CALL setMonoColor( ::sal_Int32 _monocolor ) override;
        virtual ::sal_Int32 SAL_CALL getRadioCheckTextColor() override;
        virtual void SAL_CALL setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getSeparatorColor() override;
        virtual ::sal_Int32 SAL_CALL getShadowColor() override;
        virtual void SAL_CALL setShadowColor( ::sal_Int32 _shadowcolor ) override;
        virtual ::sal_Int32 SAL_CALL getWindowColor() override;
        virtual void SAL_CALL setWindowColor( ::sal_Int32 _windowcolor ) override;
        virtual ::sal_Int32 SAL_CALL getWindowTextColor() override;
        virtual void SAL_CALL setWindowTextColor( ::sal_Int32 _windowtextcolor ) override;
        virtual ::sal_Int32 SAL_CALL getWorkspaceColor() override;
        virtual void SAL_CALL setWorkspaceColor( ::sal_Int32 _workspacecolor ) override;
        virtual sal_Bool SAL_CALL getHighContrastMode() override;
        virtual void SAL_CALL setHighContrastMode( sal_Bool _highcontrastmode ) override;
        virtual css::awt::FontDescriptor SAL_CALL getApplicationFont() override;
        virtual void SAL_CALL setApplicationFont( const css::awt::FontDescriptor& _applicationfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getHelpFont() override;
        virtual void SAL_CALL setHelpFont( const css::awt::FontDescriptor& _helpfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getTitleFont() override;
        virtual void SAL_CALL setTitleFont( const css::awt::FontDescriptor& _titlefont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getFloatTitleFont() override;
        virtual void SAL_CALL setFloatTitleFont( const css::awt::FontDescriptor& _floattitlefont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getMenuFont() override;
        virtual void SAL_CALL setMenuFont( const css::awt::FontDescriptor& _menufont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getToolFont() override;
        virtual void SAL_CALL setToolFont( const css::awt::FontDescriptor& _toolfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getGroupFont() override;
        virtual void SAL_CALL setGroupFont( const css::awt::FontDescriptor& _groupfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getLabelFont() override;
        virtual void SAL_CALL setLabelFont( const css::awt::FontDescriptor& _labelfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getRadioCheckFont() override;
        virtual void SAL_CALL setRadioCheckFont( const css::awt::FontDescriptor& _radiocheckfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getPushButtonFont() override;
        virtual void SAL_CALL setPushButtonFont( const css::awt::FontDescriptor& _pushbuttonfont ) override;
        virtual css::awt::FontDescriptor SAL_CALL getFieldFont() override;
        virtual void SAL_CALL setFieldFont( const css::awt::FontDescriptor& _fieldfont ) override;
        virtual void SAL_CALL addStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) override;
        virtual void SAL_CALL removeStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) override;

    private:
        std::unique_ptr< WindowStyleSettings_Data > m_pData;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_SOURCE_AWT_STYLESETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
