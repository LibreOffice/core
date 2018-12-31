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

#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/interfacecontainer2.hxx>
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


    //= WindowStyleSettings_Data

    struct WindowStyleSettings_Data
    {
        VCLXWindow*                         pOwningWindow;
        ::comphelper::OInterfaceContainerHelper2   aStyleChangeListeners;

        WindowStyleSettings_Data( ::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow )
            : pOwningWindow( &i_rOwningWindow )
            ,aStyleChangeListeners( i_rListenerMutex )
        {
        }

        DECL_LINK( OnWindowEvent, VclWindowEvent&, void );
    };


    IMPL_LINK( WindowStyleSettings_Data, OnWindowEvent, VclWindowEvent&, rEvent, void )
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

    class StyleMethodGuard
    {
    public:
        explicit StyleMethodGuard( WindowStyleSettings_Data const & i_rData )
        {
            if ( i_rData.pOwningWindow == nullptr )
                throw DisposedException();
        }

    private:
        SolarMutexGuard const  m_aGuard;
    };


    //= WindowStyleSettings


    WindowStyleSettings::WindowStyleSettings(::osl::Mutex& i_rListenerMutex, VCLXWindow& i_rOwningWindow )
        :m_pData( new WindowStyleSettings_Data(i_rListenerMutex, i_rOwningWindow ) )
    {
        VclPtr<vcl::Window> pWindow = i_rOwningWindow.GetWindow();
        if ( !pWindow )
            throw RuntimeException();
        pWindow->AddEventListener( LINK( m_pData.get(), WindowStyleSettings_Data, OnWindowEvent ) );
    }


    WindowStyleSettings::~WindowStyleSettings()
    {
    }


    void WindowStyleSettings::dispose()
    {
        StyleMethodGuard aGuard( *m_pData );

        VclPtr<vcl::Window> pWindow = m_pData->pOwningWindow->GetWindow();
        OSL_ENSURE( pWindow, "WindowStyleSettings::dispose: window has been reset before we could revoke the listener!" );
        if ( pWindow )
            pWindow->RemoveEventListener( LINK( m_pData.get(), WindowStyleSettings_Data, OnWindowEvent ) );

        EventObject aEvent( *this );
        m_pData->aStyleChangeListeners.disposeAndClear( aEvent );

        m_pData->pOwningWindow = nullptr;
    }


    namespace
    {
        sal_Int32 lcl_getStyleColor( WindowStyleSettings_Data const & i_rData, Color const & (StyleSettings::*i_pGetter)() const )
        {
            const VclPtr<vcl::Window>& pWindow = i_rData.pOwningWindow->GetWindow();
            const AllSettings aAllSettings = pWindow->GetSettings();
            const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
            return sal_Int32((aStyleSettings.*i_pGetter)());
        }

        void lcl_setStyleColor( WindowStyleSettings_Data const & i_rData, void (StyleSettings::*i_pSetter)( Color const & ), sal_Int32 i_nColor )
        {
            VclPtr<vcl::Window> pWindow = i_rData.pOwningWindow->GetWindow();
            AllSettings aAllSettings = pWindow->GetSettings();
            StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
            (aStyleSettings.*i_pSetter)( Color(i_nColor) );
            aAllSettings.SetStyleSettings( aStyleSettings );
            pWindow->SetSettings( aAllSettings );
        }

        FontDescriptor lcl_getStyleFont( WindowStyleSettings_Data const & i_rData, vcl::Font const & (StyleSettings::*i_pGetter)() const )
        {
            const VclPtr<vcl::Window>& pWindow = i_rData.pOwningWindow->GetWindow();
            const AllSettings aAllSettings = pWindow->GetSettings();
            const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
            return VCLUnoHelper::CreateFontDescriptor( (aStyleSettings.*i_pGetter)() );
        }

        void lcl_setStyleFont( WindowStyleSettings_Data const & i_rData, void (StyleSettings::*i_pSetter)( vcl::Font const &),
            vcl::Font const & (StyleSettings::*i_pGetter)() const, const FontDescriptor& i_rFont )
        {
            VclPtr<vcl::Window> pWindow = i_rData.pOwningWindow->GetWindow();
            AllSettings aAllSettings = pWindow->GetSettings();
            StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
            const vcl::Font aNewFont = VCLUnoHelper::CreateFont( i_rFont, (aStyleSettings.*i_pGetter)() );
            (aStyleSettings.*i_pSetter)( aNewFont );
            aAllSettings.SetStyleSettings( aStyleSettings );
            pWindow->SetSettings( aAllSettings );
        }
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveBorderColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetActiveBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveBorderColor( ::sal_Int32 _activebordercolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetActiveBorderColor, _activebordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetActiveColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveColor( ::sal_Int32 _activecolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetActiveColor, _activecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveTabColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetActiveTabColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveTabColor( ::sal_Int32 _activetabcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetActiveTabColor, _activetabcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getActiveTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetActiveTextColor );
    }


    void SAL_CALL WindowStyleSettings::setActiveTextColor( ::sal_Int32 _activetextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetActiveTextColor, _activetextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getButtonRolloverTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetButtonRolloverTextColor );
    }


    void SAL_CALL WindowStyleSettings::setButtonRolloverTextColor( ::sal_Int32 _buttonrollovertextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetButtonRolloverTextColor, _buttonrollovertextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getButtonTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetButtonTextColor );
    }


    void SAL_CALL WindowStyleSettings::setButtonTextColor( ::sal_Int32 _buttontextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetButtonTextColor, _buttontextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getCheckedColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetCheckedColor );
    }


    void SAL_CALL WindowStyleSettings::setCheckedColor( ::sal_Int32 _checkedcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetCheckedColor, _checkedcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDarkShadowColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDarkShadowColor );
    }


    void SAL_CALL WindowStyleSettings::setDarkShadowColor( ::sal_Int32 _darkshadowcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDarkShadowColor, _darkshadowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveBorderColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDeactiveBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveBorderColor( ::sal_Int32 _deactivebordercolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDeactiveBorderColor, _deactivebordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDeactiveColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveColor( ::sal_Int32 _deactivecolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDeactiveColor, _deactivecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDeactiveTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDeactiveTextColor );
    }


    void SAL_CALL WindowStyleSettings::setDeactiveTextColor( ::sal_Int32 _deactivetextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDeactiveTextColor, _deactivetextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDialogColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDialogColor );
    }


    void SAL_CALL WindowStyleSettings::setDialogColor( ::sal_Int32 _dialogcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDialogColor, _dialogcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDialogTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDialogTextColor );
    }


    void SAL_CALL WindowStyleSettings::setDialogTextColor( ::sal_Int32 _dialogtextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDialogTextColor, _dialogtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getDisableColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetDisableColor );
    }


    void SAL_CALL WindowStyleSettings::setDisableColor( ::sal_Int32 _disablecolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetDisableColor, _disablecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFaceColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetFaceColor );
    }


    void SAL_CALL WindowStyleSettings::setFaceColor( ::sal_Int32 _facecolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetFaceColor, _facecolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFaceGradientColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        const VclPtr<vcl::Window>& pWindow = m_pData->pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return sal_Int32(aStyleSettings.GetFaceGradientColor());
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetFieldColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldColor( ::sal_Int32 _fieldcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetFieldColor, _fieldcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldRolloverTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetFieldRolloverTextColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldRolloverTextColor( ::sal_Int32 _fieldrollovertextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetFieldRolloverTextColor, _fieldrollovertextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getFieldTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetFieldTextColor );
    }


    void SAL_CALL WindowStyleSettings::setFieldTextColor( ::sal_Int32 _fieldtextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetFieldTextColor, _fieldtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getGroupTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetGroupTextColor );
    }


    void SAL_CALL WindowStyleSettings::setGroupTextColor( ::sal_Int32 _grouptextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetGroupTextColor, _grouptextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHelpColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetHelpColor );
    }


    void SAL_CALL WindowStyleSettings::setHelpColor( ::sal_Int32 _helpcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetHelpColor, _helpcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHelpTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetHelpTextColor );
    }


    void SAL_CALL WindowStyleSettings::setHelpTextColor( ::sal_Int32 _helptextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetHelpTextColor, _helptextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHighlightColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetHighlightColor );
    }


    void SAL_CALL WindowStyleSettings::setHighlightColor( ::sal_Int32 _highlightcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetHighlightColor, _highlightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getHighlightTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetHighlightTextColor );
    }


    void SAL_CALL WindowStyleSettings::setHighlightTextColor( ::sal_Int32 _highlighttextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetHighlightTextColor, _highlighttextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getInactiveTabColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetInactiveTabColor );
    }


    void SAL_CALL WindowStyleSettings::setInactiveTabColor( ::sal_Int32 _inactivetabcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetInactiveTabColor, _inactivetabcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getLabelTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetLabelTextColor );
    }


    void SAL_CALL WindowStyleSettings::setLabelTextColor( ::sal_Int32 _labeltextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetLabelTextColor, _labeltextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getLightColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetLightColor );
    }


    void SAL_CALL WindowStyleSettings::setLightColor( ::sal_Int32 _lightcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetLightColor, _lightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBarColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuBarColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBarColor( ::sal_Int32 _menubarcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuBarColor, _menubarcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBarTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuBarTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBarTextColor( ::sal_Int32 _menubartextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuBarTextColor, _menubartextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuBorderColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuBorderColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuBorderColor( ::sal_Int32 _menubordercolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuBorderColor, _menubordercolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuColor( ::sal_Int32 _menucolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuColor, _menucolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuHighlightColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuHighlightColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuHighlightColor( ::sal_Int32 _menuhighlightcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuHighlightColor, _menuhighlightcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuHighlightTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuHighlightTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuHighlightTextColor( ::sal_Int32 _menuhighlighttextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuHighlightTextColor, _menuhighlighttextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMenuTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMenuTextColor );
    }


    void SAL_CALL WindowStyleSettings::setMenuTextColor( ::sal_Int32 _menutextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMenuTextColor, _menutextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getMonoColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetMonoColor );
    }


    void SAL_CALL WindowStyleSettings::setMonoColor( ::sal_Int32 _monocolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetMonoColor, _monocolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getRadioCheckTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetRadioCheckTextColor );
    }


    void SAL_CALL WindowStyleSettings::setRadioCheckTextColor( ::sal_Int32 _radiochecktextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetRadioCheckTextColor, _radiochecktextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getSeparatorColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        const VclPtr<vcl::Window>& pWindow = m_pData->pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return sal_Int32(aStyleSettings.GetSeparatorColor());
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getShadowColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetShadowColor );
    }


    void SAL_CALL WindowStyleSettings::setShadowColor( ::sal_Int32 _shadowcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetShadowColor, _shadowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWindowColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetWindowColor );
    }


    void SAL_CALL WindowStyleSettings::setWindowColor( ::sal_Int32 _windowcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetWindowColor, _windowcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWindowTextColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetWindowTextColor );
    }


    void SAL_CALL WindowStyleSettings::setWindowTextColor( ::sal_Int32 _windowtextcolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetWindowTextColor, _windowtextcolor );
    }


    ::sal_Int32 SAL_CALL WindowStyleSettings::getWorkspaceColor()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleColor( *m_pData, &StyleSettings::GetWorkspaceColor );
    }


    void SAL_CALL WindowStyleSettings::setWorkspaceColor( ::sal_Int32 _workspacecolor )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleColor( *m_pData, &StyleSettings::SetWorkspaceColor, _workspacecolor );
    }


    sal_Bool SAL_CALL WindowStyleSettings::getHighContrastMode()
    {
        StyleMethodGuard aGuard( *m_pData );
        const VclPtr<vcl::Window>& pWindow = m_pData->pOwningWindow->GetWindow();
        const AllSettings aAllSettings = pWindow->GetSettings();
        const StyleSettings& aStyleSettings = aAllSettings.GetStyleSettings();
        return aStyleSettings.GetHighContrastMode();
    }


    void SAL_CALL WindowStyleSettings::setHighContrastMode( sal_Bool _highcontrastmode )
    {
        StyleMethodGuard aGuard( *m_pData );
        VclPtr<vcl::Window> pWindow = m_pData->pOwningWindow->GetWindow();
        AllSettings aAllSettings = pWindow->GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        aStyleSettings.SetHighContrastMode( _highcontrastmode );
        aAllSettings.SetStyleSettings( aStyleSettings );
        pWindow->SetSettings( aAllSettings );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getApplicationFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetAppFont );
    }


    void SAL_CALL WindowStyleSettings::setApplicationFont( const FontDescriptor& _applicationfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetAppFont, &StyleSettings::GetAppFont, _applicationfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getHelpFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetHelpFont );
    }


    void SAL_CALL WindowStyleSettings::setHelpFont( const FontDescriptor& _helpfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetHelpFont, &StyleSettings::GetHelpFont, _helpfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getTitleFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetTitleFont );
    }


    void SAL_CALL WindowStyleSettings::setTitleFont( const FontDescriptor& _titlefont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetTitleFont, &StyleSettings::GetTitleFont, _titlefont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getFloatTitleFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetFloatTitleFont );
    }


    void SAL_CALL WindowStyleSettings::setFloatTitleFont( const FontDescriptor& _floattitlefont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetFloatTitleFont, &StyleSettings::GetFloatTitleFont, _floattitlefont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getMenuFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetMenuFont );
    }


    void SAL_CALL WindowStyleSettings::setMenuFont( const FontDescriptor& _menufont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetMenuFont, &StyleSettings::GetMenuFont, _menufont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getToolFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetToolFont );
    }


    void SAL_CALL WindowStyleSettings::setToolFont( const FontDescriptor& _toolfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetToolFont, &StyleSettings::GetToolFont, _toolfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getGroupFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetGroupFont );
    }


    void SAL_CALL WindowStyleSettings::setGroupFont( const FontDescriptor& _groupfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetGroupFont, &StyleSettings::GetGroupFont, _groupfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getLabelFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetLabelFont );
    }


    void SAL_CALL WindowStyleSettings::setLabelFont( const FontDescriptor& _labelfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetLabelFont, &StyleSettings::GetLabelFont, _labelfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getRadioCheckFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetRadioCheckFont );
    }


    void SAL_CALL WindowStyleSettings::setRadioCheckFont( const FontDescriptor& _radiocheckfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetRadioCheckFont, &StyleSettings::GetRadioCheckFont, _radiocheckfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getPushButtonFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetPushButtonFont );
    }


    void SAL_CALL WindowStyleSettings::setPushButtonFont( const FontDescriptor& _pushbuttonfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetPushButtonFont, &StyleSettings::GetPushButtonFont, _pushbuttonfont );
    }


    FontDescriptor SAL_CALL WindowStyleSettings::getFieldFont()
    {
        StyleMethodGuard aGuard( *m_pData );
        return lcl_getStyleFont( *m_pData, &StyleSettings::GetFieldFont );
    }


    void SAL_CALL WindowStyleSettings::setFieldFont( const FontDescriptor& _fieldfont )
    {
        StyleMethodGuard aGuard( *m_pData );
        lcl_setStyleFont( *m_pData, &StyleSettings::SetFieldFont, &StyleSettings::GetFieldFont, _fieldfont );
    }


    void SAL_CALL WindowStyleSettings::addStyleChangeListener( const Reference< XStyleChangeListener >& i_rListener )
    {
        StyleMethodGuard aGuard( *m_pData );
        if ( i_rListener.is() )
            m_pData->aStyleChangeListeners.addInterface( i_rListener );
    }


    void SAL_CALL WindowStyleSettings::removeStyleChangeListener( const Reference< XStyleChangeListener >& i_rListener )
    {
        StyleMethodGuard aGuard( *m_pData );
        if ( i_rListener.is() )
            m_pData->aStyleChangeListeners.removeInterface( i_rListener );
    }


} // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
