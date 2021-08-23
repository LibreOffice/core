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


#include "stylesettings.hxx"
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>


namespace toolkit
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::FontDescriptor;
    using ::com::sun::star::awt::XStyleChangeListener;


    IMPL_LINK( WindowStyleSettings, OnWindowEvent, VclWindowEvent&, rEvent, void )
    {
        if ( rEvent.GetId() != VclEventId::WindowDataChanged )
            return;
        const DataChangedEvent* pDataChangedEvent = static_cast< const DataChangedEvent* >( rEvent.GetData() );
        if ( !pDataChangedEvent || ( pDataChangedEvent->GetType() != DataChangedEventType::SETTINGS ) )
            return;
        if ( !( pDataChangedEvent->GetFlags() & AllSettingsFlags::STYLE ) )
            return;

        EventObject aEvent( *pOwningWindow );
        aStyleChangeListeners.notifyEach( &XStyleChangeListener::styleSettingsChanged, aEvent );
    }


    //= StyleMethodGuard

    namespace {

    class StyleMethodGuard
    {
    public:
        explicit StyleMethodGuard( VCLXWindow* pOwningWindow )
        {
            if ( pOwningWindow == nullptr )
                throw DisposedException();
        }

    private:
        SolarMutexGuard  m_aGuard;
    };

    }

    //= WindowStyleSettings


    WindowStyleSettings::WindowStyleSettings(::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow )
        : pOwningWindow( &i_rOwningWindow )
        ,aStyleChangeListeners( i_rListenerMutex )
    {
        VclPtr<vcl::Window> pWindow = i_rOwningWindow.GetWindow();
        if ( !pWindow )
            throw RuntimeException();
        pWindow->AddEventListener( LINK( this, WindowStyleSettings, OnWindowEvent ) );
    }


    WindowStyleSettings::~WindowStyleSettings()
    {
    }


    void WindowStyleSettings::dispose()
    {
        StyleMethodGuard aGuard( pOwningWindow );

        VclPtr<vcl::Window> pWindow = pOwningWindow->GetWindow();
        OSL_ENSURE( pWindow, "WindowStyleSettings::dispose: window has been reset before we could revoke the listener!" );
        if ( pWindow )
            pWindow->RemoveEventListener( LINK( this, WindowStyleSettings, OnWindowEvent ) );

        EventObject aEvent( *this );
        aStyleChangeListeners.disposeAndClear( aEvent );

        pOwningWindow = nullptr;
    }


    sal_Int32 WindowStyleSettings::ImplGetStyleColor( Color const & (StyleSettings::*i_pGetter)() const )
    {
        const VclPtr<vcl::Window>& pWindow = pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return sal_Int32((aStyleSettings.*i_pGetter)());
    }

    void WindowStyleSettings::ImplSetStyleColor( void (StyleSettings::*i_pSetter)( Color const & ), sal_Int32 i_nColor )
    {
        VclPtr<vcl::Window> pWindow = pOwningWindow->GetWindow();
        AllSettings aAllSettings = pWindow->GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        (aStyleSettings.*i_pSetter)( Color(ColorTransparency, i_nColor) );
        aAllSettings.SetStyleSettings( aStyleSettings );
        pWindow->SetSettings( aAllSettings );
    }

    FontDescriptor WindowStyleSettings::ImplGetStyleFont( vcl::Font const & (StyleSettings::*i_pGetter)() const )
    {
        const VclPtr<vcl::Window>& pWindow = pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return VCLUnoHelper::CreateFontDescriptor( (aStyleSettings.*i_pGetter)() );
    }

    void WindowStyleSettings::ImplSetStyleFont( void (StyleSettings::*i_pSetter)( vcl::Font const &),
        vcl::Font const & (StyleSettings::*i_pGetter)() const, const FontDescriptor& i_rFont )
    {
        VclPtr<vcl::Window> pWindow = pOwningWindow->GetWindow();
        AllSettings aAllSettings = pWindow->GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        const vcl::Font aNewFont = VCLUnoHelper::CreateFont( i_rFont, (aStyleSettings.*i_pGetter)() );
        (aStyleSettings.*i_pSetter)( aNewFont );
        aAllSettings.SetStyleSettings( aStyleSettings );
        pWindow->SetSettings( aAllSettings );
    }

    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveBorderColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetActiveBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveBorderColor( ::sal_Int32 _activebordercolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetActiveBorderColor, _activebordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetActiveColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveColor( ::sal_Int32 _activecolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetActiveColor, _activecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveTabColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetActiveTabColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveTabColor( ::sal_Int32 _activetabcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetActiveTabColor, _activetabcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetActiveTextColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveTextColor( ::sal_Int32 _activetextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetActiveTextColor, _activetextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getButtonRolloverTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetButtonRolloverTextColor );
    }


    void SAL_CALL WindowStyleSettings::setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetButtonRolloverTextColor, _buttonrollovertextcolor );
        // Also need to set ActionButtonRolloverTextColor as this setting can't be
        // set through the UNO interface otherwise.
        // Previously this setting was used to set colors for both scenarios,
        // but action button setting was added to differentiate the buttons from
        // "normal" buttons in some themes.
        ImplSetStyleColor( &StyleSettings::SetActionButtonRolloverTextColor, _buttonrollovertextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getButtonTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetButtonTextColor );
    }


    void SAL_CALL WindowStyleSettings::setButtonTextColor( ::sal_Int32 _buttontextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetButtonTextColor, _buttontextcolor );
        // Also need to set ActionButtonTextColor and DefaultActionButtonTextColor
        // as this two settings can't be set through the UNO interface otherwise.
        // Previously this setting was used to set colors for all three scenarios,
        // but action button setting was added to differentiate the buttons from
        // "normal" buttons in some themes.
        ImplSetStyleColor( &StyleSettings::SetActionButtonTextColor, _buttontextcolor );
        ImplSetStyleColor( &StyleSettings::SetDefaultActionButtonTextColor, _buttontextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getCheckedColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetCheckedColor );
    }


    void SAL_CALL WindowStyleSettings::setCheckedColor( ::sal_Int32 _checkedcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetCheckedColor, _checkedcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDarkShadowColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDarkShadowColor );
    }


    void SAL_CALL WindowStyleSettings::setDarkShadowColor( ::sal_Int32 _darkshadowcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDarkShadowColor, _darkshadowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveBorderColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDeactiveBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDeactiveBorderColor, _deactivebordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDeactiveColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveColor( ::sal_Int32 _deactivecolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDeactiveColor, _deactivecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDeactiveTextColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveTextColor( ::sal_Int32 _deactivetextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDeactiveTextColor, _deactivetextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDialogColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDialogColor );
    }


    void SAL_CALL WindowStyleSettings::setDialogColor( ::sal_Int32 _dialogcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDialogColor, _dialogcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDialogTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDialogTextColor );
    }


    void SAL_CALL WindowStyleSettings::setDialogTextColor( ::sal_Int32 _dialogtextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDialogTextColor, _dialogtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDisableColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetDisableColor );
    }


    void SAL_CALL WindowStyleSettings::setDisableColor( ::sal_Int32 _disablecolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetDisableColor, _disablecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFaceColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetFaceColor );
    }


    void SAL_CALL WindowStyleSettings::setFaceColor( ::sal_Int32 _facecolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetFaceColor, _facecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFaceGradientColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        const VclPtr<vcl::Window>& pWindow = pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return sal_Int32(aStyleSettings.GetFaceGradientColor());
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetFieldColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldColor( ::sal_Int32 _fieldcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetFieldColor, _fieldcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldRolloverTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetFieldRolloverTextColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetFieldRolloverTextColor, _fieldrollovertextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetFieldTextColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldTextColor( ::sal_Int32 _fieldtextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetFieldTextColor, _fieldtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getGroupTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetGroupTextColor );
    }


    void SAL_CALL WindowStyleSettings::setGroupTextColor( ::sal_Int32 _grouptextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetGroupTextColor, _grouptextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHelpColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetHelpColor );
    }


    void SAL_CALL WindowStyleSettings::setHelpColor( ::sal_Int32 _helpcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetHelpColor, _helpcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHelpTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetHelpTextColor );
    }


    void SAL_CALL WindowStyleSettings::setHelpTextColor( ::sal_Int32 _helptextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetHelpTextColor, _helptextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHighlightColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetHighlightColor );
    }


    void SAL_CALL WindowStyleSettings::setHighlightColor( ::sal_Int32 _highlightcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetHighlightColor, _highlightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHighlightTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetHighlightTextColor );
    }


    void SAL_CALL WindowStyleSettings::setHighlightTextColor( ::sal_Int32 _highlighttextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetHighlightTextColor, _highlighttextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getInactiveTabColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetInactiveTabColor );
    }


    void SAL_CALL WindowStyleSettings::setInactiveTabColor( ::sal_Int32 _inactivetabcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetInactiveTabColor, _inactivetabcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getLabelTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetLabelTextColor );
    }


    void SAL_CALL WindowStyleSettings::setLabelTextColor( ::sal_Int32 _labeltextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetLabelTextColor, _labeltextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getLightColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetLightColor );
    }


    void SAL_CALL WindowStyleSettings::setLightColor( ::sal_Int32 _lightcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetLightColor, _lightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBarColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuBarColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBarColor( ::sal_Int32 _menubarcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuBarColor, _menubarcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBarTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuBarTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBarTextColor( ::sal_Int32 _menubartextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuBarTextColor, _menubartextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBorderColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBorderColor( ::sal_Int32 _menubordercolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuBorderColor, _menubordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuColor( ::sal_Int32 _menucolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuColor, _menucolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuHighlightColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuHighlightColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuHighlightColor, _menuhighlightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuHighlightTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuHighlightTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuHighlightTextColor, _menuhighlighttextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMenuTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuTextColor( ::sal_Int32 _menutextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMenuTextColor, _menutextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMonoColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetMonoColor );
    }


    void SAL_CALL WindowStyleSettings::setMonoColor( ::sal_Int32 _monocolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetMonoColor, _monocolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getRadioCheckTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetRadioCheckTextColor );
    }


    void SAL_CALL WindowStyleSettings::setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetRadioCheckTextColor, _radiochecktextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getSeparatorColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        const VclPtr<vcl::Window>& pWindow = pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return sal_Int32(aStyleSettings.GetSeparatorColor());
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getShadowColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetShadowColor );
    }


    void SAL_CALL WindowStyleSettings::setShadowColor( ::sal_Int32 _shadowcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetShadowColor, _shadowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWindowColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetWindowColor );
    }


    void SAL_CALL WindowStyleSettings::setWindowColor( ::sal_Int32 _windowcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetWindowColor, _windowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWindowTextColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetWindowTextColor );
    }


    void SAL_CALL WindowStyleSettings::setWindowTextColor( ::sal_Int32 _windowtextcolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetWindowTextColor, _windowtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWorkspaceColor()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleColor( &StyleSettings::GetWorkspaceColor );
    }


    void SAL_CALL WindowStyleSettings::setWorkspaceColor( ::sal_Int32 _workspacecolor )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleColor( &StyleSettings::SetWorkspaceColor, _workspacecolor );
    }


    sal_Bool SAL_CALL WindowStyleSettings::getHighContrastMode()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        const VclPtr<vcl::Window>& pWindow = pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return aStyleSettings.GetHighContrastMode();
    }


    void SAL_CALL WindowStyleSettings::setHighContrastMode( sal_Bool _highcontrastmode )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        VclPtr<vcl::Window> pWindow = pOwningWindow->GetWindow();
        AllSettings aAllSettings = pWindow->GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        aStyleSettings.SetHighContrastMode( _highcontrastmode );
        aAllSettings.SetStyleSettings( aStyleSettings );
        pWindow->SetSettings( aAllSettings );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getApplicationFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetAppFont );
    }


    void SAL_CALL WindowStyleSettings::setApplicationFont( const FontDescriptor& _applicationfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetAppFont, &StyleSettings::GetAppFont, _applicationfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getHelpFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetHelpFont );
    }


    void SAL_CALL WindowStyleSettings::setHelpFont( const FontDescriptor& _helpfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetHelpFont, &StyleSettings::GetHelpFont, _helpfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getTitleFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetTitleFont );
    }


    void SAL_CALL WindowStyleSettings::setTitleFont( const FontDescriptor& _titlefont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetTitleFont, &StyleSettings::GetTitleFont, _titlefont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getFloatTitleFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetFloatTitleFont );
    }


    void SAL_CALL WindowStyleSettings::setFloatTitleFont( const FontDescriptor& _floattitlefont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetFloatTitleFont, &StyleSettings::GetFloatTitleFont, _floattitlefont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getMenuFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetMenuFont );
    }


    void SAL_CALL WindowStyleSettings::setMenuFont( const FontDescriptor& _menufont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetMenuFont, &StyleSettings::GetMenuFont, _menufont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getToolFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetToolFont );
    }


    void SAL_CALL WindowStyleSettings::setToolFont( const FontDescriptor& _toolfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetToolFont, &StyleSettings::GetToolFont, _toolfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getGroupFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetGroupFont );
    }


    void SAL_CALL WindowStyleSettings::setGroupFont( const FontDescriptor& _groupfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetGroupFont, &StyleSettings::GetGroupFont, _groupfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getLabelFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetLabelFont );
    }


    void SAL_CALL WindowStyleSettings::setLabelFont( const FontDescriptor& _labelfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetLabelFont, &StyleSettings::GetLabelFont, _labelfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getRadioCheckFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetRadioCheckFont );
    }


    void SAL_CALL WindowStyleSettings::setRadioCheckFont( const FontDescriptor& _radiocheckfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetRadioCheckFont, &StyleSettings::GetRadioCheckFont, _radiocheckfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getPushButtonFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetPushButtonFont );
    }


    void SAL_CALL WindowStyleSettings::setPushButtonFont( const FontDescriptor& _pushbuttonfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetPushButtonFont, &StyleSettings::GetPushButtonFont, _pushbuttonfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getFieldFont()
    {
        StyleMethodGuard aGuard( pOwningWindow );
        return ImplGetStyleFont( &StyleSettings::GetFieldFont );
    }


    void SAL_CALL WindowStyleSettings::setFieldFont( const FontDescriptor& _fieldfont )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        ImplSetStyleFont( &StyleSettings::SetFieldFont, &StyleSettings::GetFieldFont, _fieldfont );
    }


    void SAL_CALL WindowStyleSettings::addStyleChangeListener( const Reference< XStyleChangeListener >& i_rListener )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        if ( i_rListener.is() )
            aStyleChangeListeners.addInterface( i_rListener );
    }


    void SAL_CALL WindowStyleSettings::removeStyleChangeListener( const Reference< XStyleChangeListener >& i_rListener )
    {
        StyleMethodGuard aGuard( pOwningWindow );
        if ( i_rListener.is() )
            aStyleChangeListeners.removeInterface( i_rListener );
    }


} // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
