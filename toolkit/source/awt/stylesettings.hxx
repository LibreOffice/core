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
        virtual ~WindowStyleSettings();

        void dispose();

        // XStyleSettings
        virtual ::sal_Int32 SAL_CALL getActiveBorderColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveBorderColor( ::sal_Int32 _activebordercolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getActiveColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveColor( ::sal_Int32 _activecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getActiveTabColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveTabColor( ::sal_Int32 _activetabcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getActiveTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveTextColor( ::sal_Int32 _activetextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getButtonRolloverTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getButtonTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setButtonTextColor( ::sal_Int32 _buttontextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getCheckedColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCheckedColor( ::sal_Int32 _checkedcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDarkShadowColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDarkShadowColor( ::sal_Int32 _darkshadowcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveBorderColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDeactiveColor( ::sal_Int32 _deactivecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDeactiveTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDeactiveTextColor( ::sal_Int32 _deactivetextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDialogColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDialogColor( ::sal_Int32 _dialogcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDialogTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDialogTextColor( ::sal_Int32 _dialogtextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getDisableColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDisableColor( ::sal_Int32 _disablecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getFaceColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFaceColor( ::sal_Int32 _facecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getFaceGradientColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getFieldColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFieldColor( ::sal_Int32 _fieldcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getFieldRolloverTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getFieldTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFieldTextColor( ::sal_Int32 _fieldtextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getGroupTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupTextColor( ::sal_Int32 _grouptextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getHelpColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpColor( ::sal_Int32 _helpcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getHelpTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpTextColor( ::sal_Int32 _helptextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getHighlightColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHighlightColor( ::sal_Int32 _highlightcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getHighlightTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHighlightTextColor( ::sal_Int32 _highlighttextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getInactiveTabColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInactiveTabColor( ::sal_Int32 _inactivetabcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getInfoTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInfoTextColor( ::sal_Int32 _infotextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLabelTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLabelTextColor( ::sal_Int32 _labeltextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getLightColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLightColor( ::sal_Int32 _lightcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuBarColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuBarColor( ::sal_Int32 _menubarcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuBarTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuBarTextColor( ::sal_Int32 _menubartextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuBorderColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuBorderColor( ::sal_Int32 _menubordercolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuColor( ::sal_Int32 _menucolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuHighlightColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuHighlightTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMenuTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuTextColor( ::sal_Int32 _menutextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMonoColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMonoColor( ::sal_Int32 _monocolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getRadioCheckTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getSeparatorColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getShadowColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setShadowColor( ::sal_Int32 _shadowcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getWindowColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setWindowColor( ::sal_Int32 _windowcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getWindowTextColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setWindowTextColor( ::sal_Int32 _windowtextcolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getWorkspaceColor() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setWorkspaceColor( ::sal_Int32 _workspacecolor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getHighContrastMode() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHighContrastMode( sal_Bool _highcontrastmode ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getApplicationFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setApplicationFont( const css::awt::FontDescriptor& _applicationfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getHelpFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHelpFont( const css::awt::FontDescriptor& _helpfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getTitleFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTitleFont( const css::awt::FontDescriptor& _titlefont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getFloatTitleFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFloatTitleFont( const css::awt::FontDescriptor& _floattitlefont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getMenuFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMenuFont( const css::awt::FontDescriptor& _menufont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getToolFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setToolFont( const css::awt::FontDescriptor& _toolfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getGroupFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupFont( const css::awt::FontDescriptor& _groupfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getLabelFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setLabelFont( const css::awt::FontDescriptor& _labelfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getInfoFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setInfoFont( const css::awt::FontDescriptor& _infofont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getRadioCheckFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setRadioCheckFont( const css::awt::FontDescriptor& _radiocheckfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getPushButtonFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPushButtonFont( const css::awt::FontDescriptor& _pushbuttonfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::awt::FontDescriptor SAL_CALL getFieldFont() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFieldFont( const css::awt::FontDescriptor& _fieldfont ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        std::unique_ptr< WindowStyleSettings_Data > m_pData;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_SOURCE_AWT_STYLESETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
