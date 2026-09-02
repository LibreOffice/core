/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/link.hxx>

namespace osl
{
    class Mutex;
}
class Color;
class VCLXWindow;
class VclWindowEvent;
class StyleSettings;
namespace vcl { class Font; }

namespace toolkit
{


    //= WindowStyleSettings

    typedef ::cppu::WeakImplHelper <   css::awt::XStyleSettings
                                    >   WindowStyleSettings_Base;
    class WindowStyleSettings : public WindowStyleSettings_Base
    {
    public:
        WindowStyleSettings( ::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow );
        virtual ~WindowStyleSettings() override;

        void dispose();

        // XStyleSettings
        virtual ::sal_Int32 getActiveBorderColor() override;
        virtual void setActiveBorderColor( ::sal_Int32 _activebordercolor ) override;
        virtual ::sal_Int32 getActiveColor() override;
        virtual void setActiveColor( ::sal_Int32 _activecolor ) override;
        virtual ::sal_Int32 getActiveTabColor() override;
        virtual void setActiveTabColor( ::sal_Int32 _activetabcolor ) override;
        virtual ::sal_Int32 getActiveTextColor() override;
        virtual void setActiveTextColor( ::sal_Int32 _activetextcolor ) override;
        virtual ::sal_Int32 getButtonRolloverTextColor() override;
        virtual void setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor ) override;
        virtual ::sal_Int32 getButtonTextColor() override;
        virtual void setButtonTextColor( ::sal_Int32 _buttontextcolor ) override;
        virtual ::sal_Int32 getCheckedColor() override;
        virtual void setCheckedColor( ::sal_Int32 _checkedcolor ) override;
        virtual ::sal_Int32 getDarkShadowColor() override;
        virtual void setDarkShadowColor( ::sal_Int32 _darkshadowcolor ) override;
        virtual ::sal_Int32 getDeactiveBorderColor() override;
        virtual void setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor ) override;
        virtual ::sal_Int32 getDeactiveColor() override;
        virtual void setDeactiveColor( ::sal_Int32 _deactivecolor ) override;
        virtual ::sal_Int32 getDeactiveTextColor() override;
        virtual void setDeactiveTextColor( ::sal_Int32 _deactivetextcolor ) override;
        virtual ::sal_Int32 getDialogColor() override;
        virtual void setDialogColor( ::sal_Int32 _dialogcolor ) override;
        virtual ::sal_Int32 getDialogTextColor() override;
        virtual void setDialogTextColor( ::sal_Int32 _dialogtextcolor ) override;
        virtual ::sal_Int32 getDisableColor() override;
        virtual void setDisableColor( ::sal_Int32 _disablecolor ) override;
        virtual ::sal_Int32 getFaceColor() override;
        virtual void setFaceColor( ::sal_Int32 _facecolor ) override;
        virtual ::sal_Int32 getFaceGradientColor() override;
        virtual ::sal_Int32 getFieldColor() override;
        virtual void setFieldColor( ::sal_Int32 _fieldcolor ) override;
        virtual ::sal_Int32 getFieldRolloverTextColor() override;
        virtual void setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor ) override;
        virtual ::sal_Int32 getFieldTextColor() override;
        virtual void setFieldTextColor( ::sal_Int32 _fieldtextcolor ) override;
        virtual ::sal_Int32 getGroupTextColor() override;
        virtual void setGroupTextColor( ::sal_Int32 _grouptextcolor ) override;
        virtual ::sal_Int32 getHelpColor() override;
        virtual void setHelpColor( ::sal_Int32 _helpcolor ) override;
        virtual ::sal_Int32 getHelpTextColor() override;
        virtual void setHelpTextColor( ::sal_Int32 _helptextcolor ) override;
        virtual ::sal_Int32 getHighlightColor() override;
        virtual void setHighlightColor( ::sal_Int32 _highlightcolor ) override;
        virtual ::sal_Int32 getHighlightTextColor() override;
        virtual void setHighlightTextColor( ::sal_Int32 _highlighttextcolor ) override;
        virtual ::sal_Int32 getInactiveTabColor() override;
        virtual void setInactiveTabColor( ::sal_Int32 _inactivetabcolor ) override;
        virtual ::sal_Int32 getLabelTextColor() override;
        virtual void setLabelTextColor( ::sal_Int32 _labeltextcolor ) override;
        virtual ::sal_Int32 getLightColor() override;
        virtual void setLightColor( ::sal_Int32 _lightcolor ) override;
        virtual ::sal_Int32 getMenuBarColor() override;
        virtual void setMenuBarColor( ::sal_Int32 _menubarcolor ) override;
        virtual ::sal_Int32 getMenuBarTextColor() override;
        virtual void setMenuBarTextColor( ::sal_Int32 _menubartextcolor ) override;
        virtual ::sal_Int32 getMenuBorderColor() override;
        virtual void setMenuBorderColor( ::sal_Int32 _menubordercolor ) override;
        virtual ::sal_Int32 getMenuColor() override;
        virtual void setMenuColor( ::sal_Int32 _menucolor ) override;
        virtual ::sal_Int32 getMenuHighlightColor() override;
        virtual void setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor ) override;
        virtual ::sal_Int32 getMenuHighlightTextColor() override;
        virtual void setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor ) override;
        virtual ::sal_Int32 getMenuTextColor() override;
        virtual void setMenuTextColor( ::sal_Int32 _menutextcolor ) override;
        virtual ::sal_Int32 getMonoColor() override;
        virtual void setMonoColor( ::sal_Int32 _monocolor ) override;
        virtual ::sal_Int32 getRadioCheckTextColor() override;
        virtual void setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor ) override;
        virtual ::sal_Int32 getSeparatorColor() override;
        virtual ::sal_Int32 getShadowColor() override;
        virtual void setShadowColor( ::sal_Int32 _shadowcolor ) override;
        virtual ::sal_Int32 getWindowColor() override;
        virtual void setWindowColor( ::sal_Int32 _windowcolor ) override;
        virtual ::sal_Int32 getWindowTextColor() override;
        virtual void setWindowTextColor( ::sal_Int32 _windowtextcolor ) override;
        virtual ::sal_Int32 getWorkspaceColor() override;
        virtual void setWorkspaceColor( ::sal_Int32 _workspacecolor ) override;
        virtual bool getHighContrastMode() override;
        virtual void setHighContrastMode( bool _highcontrastmode ) override;
        virtual css::awt::FontDescriptor getApplicationFont() override;
        virtual void setApplicationFont( const css::awt::FontDescriptor& _applicationfont ) override;
        virtual css::awt::FontDescriptor getHelpFont() override;
        virtual void setHelpFont( const css::awt::FontDescriptor& _helpfont ) override;
        virtual css::awt::FontDescriptor getTitleFont() override;
        virtual void setTitleFont( const css::awt::FontDescriptor& _titlefont ) override;
        virtual css::awt::FontDescriptor getFloatTitleFont() override;
        virtual void setFloatTitleFont( const css::awt::FontDescriptor& _floattitlefont ) override;
        virtual css::awt::FontDescriptor getMenuFont() override;
        virtual void setMenuFont( const css::awt::FontDescriptor& _menufont ) override;
        virtual css::awt::FontDescriptor getToolFont() override;
        virtual void setToolFont( const css::awt::FontDescriptor& _toolfont ) override;
        virtual css::awt::FontDescriptor getGroupFont() override;
        virtual void setGroupFont( const css::awt::FontDescriptor& _groupfont ) override;
        virtual css::awt::FontDescriptor getLabelFont() override;
        virtual void setLabelFont( const css::awt::FontDescriptor& _labelfont ) override;
        virtual css::awt::FontDescriptor getRadioCheckFont() override;
        virtual void setRadioCheckFont( const css::awt::FontDescriptor& _radiocheckfont ) override;
        virtual css::awt::FontDescriptor getPushButtonFont() override;
        virtual void setPushButtonFont( const css::awt::FontDescriptor& _pushbuttonfont ) override;
        virtual css::awt::FontDescriptor getFieldFont() override;
        virtual void setFieldFont( const css::awt::FontDescriptor& _fieldfont ) override;
        virtual void addStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) override;
        virtual void removeStyleChangeListener( const css::uno::Reference< css::awt::XStyleChangeListener >& Listener ) override;

    private:
        void ImplSetStyleFont( void (StyleSettings::*i_pSetter)( vcl::Font const &),
            vcl::Font const & (StyleSettings::*i_pGetter)() const, const css::awt::FontDescriptor& i_rFont );
        void ImplSetStyleColor( void (StyleSettings::*i_pSetter)( Color const & ), sal_Int32 i_nColor );
        sal_Int32 ImplGetStyleColor( Color const & (StyleSettings::*i_pGetter)() const ) const;
        css::awt::FontDescriptor ImplGetStyleFont( vcl::Font const & (StyleSettings::*i_pGetter)() const ) const;
        DECL_LINK( OnWindowEvent, VclWindowEvent&, void );

        VCLXWindow*                                pOwningWindow;
        ::comphelper::OInterfaceContainerHelper3<css::awt::XStyleChangeListener> aStyleChangeListeners;
    };


} // namespace toolkit


#endif // INCLUDED_TOOLKIT_SOURCE_AWT_STYLESETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
