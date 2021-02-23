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

#include <sal/config.h>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <osl/mutex.hxx>
#include <memory>
#include <string_view>
#include <unordered_map>

#include <classes/fwkresid.hxx>
#include <bitmaps.hlst>
#include <strings.hrc>

static const char* aCommands[] =
{
    ".uno:ConvertToEdit",
    ".uno:ConvertToButton",
    ".uno:ConvertToFixed",
    ".uno:ConvertToList",
    ".uno:ConvertToCheckBox",
    ".uno:ConvertToRadio",
    ".uno:ConvertToGroup",
    ".uno:ConvertToCombo",
    ".uno:ConvertToImageBtn",
    ".uno:ConvertToFileControl",
    ".uno:ConvertToDate",
    ".uno:ConvertToTime",
    ".uno:ConvertToNumeric",
    ".uno:ConvertToCurrency",
    ".uno:ConvertToPattern",
    ".uno:ConvertToImageControl",
    ".uno:ConvertToFormatted",
    ".uno:ConvertToScrollBar",
    ".uno:ConvertToSpinButton",
    ".uno:ConvertToNavigationBar"
};

static const char* aLabels[] =
{
    RID_STR_PROPTITLE_EDIT,
    RID_STR_PROPTITLE_PUSHBUTTON,
    RID_STR_PROPTITLE_FIXEDTEXT,
    RID_STR_PROPTITLE_LISTBOX,
    RID_STR_PROPTITLE_CHECKBOX,
    RID_STR_PROPTITLE_RADIOBUTTON,
    RID_STR_PROPTITLE_GROUPBOX,
    RID_STR_PROPTITLE_COMBOBOX,
    RID_STR_PROPTITLE_IMAGEBUTTON,
    RID_STR_PROPTITLE_FILECONTROL,
    RID_STR_PROPTITLE_DATEFIELD,
    RID_STR_PROPTITLE_TIMEFIELD,
    RID_STR_PROPTITLE_NUMERICFIELD,
    RID_STR_PROPTITLE_CURRENCYFIELD,
    RID_STR_PROPTITLE_PATTERNFIELD,
    RID_STR_PROPTITLE_IMAGECONTROL,
    RID_STR_PROPTITLE_FORMATTED,
    RID_STR_PROPTITLE_SCROLLBAR,
    RID_STR_PROPTITLE_SPINBUTTON,
    RID_STR_PROPTITLE_NAVBAR
};

const std::u16string_view aImgIds[] =
{
    u"" RID_SVXBMP_EDITBOX,
    u"" RID_SVXBMP_BUTTON,
    u"" RID_SVXBMP_FIXEDTEXT,
    u"" RID_SVXBMP_LISTBOX,
    u"" RID_SVXBMP_CHECKBOX,
    u"" RID_SVXBMP_RADIOBUTTON,
    u"" RID_SVXBMP_GROUPBOX,
    u"" RID_SVXBMP_COMBOBOX,
    u"" RID_SVXBMP_IMAGEBUTTON,
    u"" RID_SVXBMP_FILECONTROL,
    u"" RID_SVXBMP_DATEFIELD,
    u"" RID_SVXBMP_TIMEFIELD,
    u"" RID_SVXBMP_NUMERICFIELD,
    u"" RID_SVXBMP_CURRENCYFIELD,
    u"" RID_SVXBMP_PATTERNFIELD,
    u"" RID_SVXBMP_IMAGECONTROL,
    u"" RID_SVXBMP_FORMATTEDFIELD,
    u"" RID_SVXBMP_SCROLLBAR,
    u"" RID_SVXBMP_SPINBUTTON,
    u"" RID_SVXBMP_NAVIGATIONBAR
};

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::beans;

namespace {

class ControlMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    explicit ControlMenuController( const uno::Reference< uno::XComponentContext >& xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.ControlMenuController";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.frame.PopupMenuController"};
    }

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() override;

    // XInitialization
    virtual void SAL_CALL initialize( const uno::Sequence< uno::Any >& aArguments ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) override;

    // XMenuListener
    virtual void SAL_CALL itemActivated( const awt::MenuEvent& rEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

private:
    class UrlToDispatchMap : public std::unordered_map< OUString,
                                                        uno::Reference< frame::XDispatch > >
    {
        public:
            void free()
            {
                UrlToDispatchMap().swap( *this );// get rid of reserved capacity
            }
    };

    void updateImagesPopupMenu(Reference<awt::XPopupMenu> const& rPopupMenu);
    void fillPopupMenu(uno::Reference<awt::XPopupMenu> const& rPopupMenu);

    bool                m_bShowMenuImages : 1;
    UrlToDispatchMap    m_aURLToDispatchMap;
};

ControlMenuController::ControlMenuController(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : svt::PopupMenuControllerBase(xContext)
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();

}

// private function
void ControlMenuController::updateImagesPopupMenu(Reference<awt::XPopupMenu> const& rPopupMenu)
{
    if (!rPopupMenu)
        return;
    for (size_t i=0; i < SAL_N_ELEMENTS(aCommands); ++i)
    {
        sal_Int16 nItemId = i + 1;
        if (m_bShowMenuImages)
        {
            Image aImage(StockImage::Yes, OUString(aImgIds[i]));
            Graphic aGraphic(aImage);
            rPopupMenu->setItemImage(nItemId, aGraphic.GetXGraphic(), false);
        }
        else
            rPopupMenu->setItemImage(nItemId, nullptr, false);
    }
}

// private function
void ControlMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    resetPopupMenu( rPopupMenu );

    for (size_t i=0; i < SAL_N_ELEMENTS(aCommands); ++i)
    {
        sal_Int16 nItemId = i + 1;
        OUString sCommand(OUString::createFromAscii(aCommands[i]));
        rPopupMenu->insertItem(nItemId, FwkResId(aLabels[i]), 0, i);
        rPopupMenu->setCommand(nItemId, sCommand);
        rPopupMenu->enableItem(nItemId, false);
    }

    updateImagesPopupMenu(rPopupMenu);

    rPopupMenu->hideDisabledEntries(true);
}

}

// XEventListener
void SAL_CALL ControlMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ControlMenuController::statusChanged( const FeatureStateEvent& Event )
{
    osl::MutexGuard aLock( m_aMutex );

    if (!m_xPopupMenu)
        return;

    sal_Int16 nItemId = 0;
    for (size_t i=0; i < SAL_N_ELEMENTS(aCommands); ++i)
    {
        if ( Event.FeatureURL.Complete.equalsAscii( aCommands[i] ))
        {
            nItemId = i + 1;
            break;
        }
    }

    if (!nItemId)
        return;

    m_xPopupMenu->enableItem(nItemId, Event.IsEnabled);
}

// XMenuListener
void SAL_CALL ControlMenuController::itemActivated( const css::awt::MenuEvent& )
{
    osl::MutexGuard aLock( m_aMutex );

    SolarMutexGuard aSolarMutexGuard;

    // Check if some modes have changed so we have to update our menu images
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    bool bShowMenuImages = rSettings.GetUseImagesInMenus();

    if (bShowMenuImages != m_bShowMenuImages)
    {
        m_bShowMenuImages = bShowMenuImages;
        updateImagesPopupMenu(m_xPopupMenu);
    }
}

// XPopupMenuController
void SAL_CALL ControlMenuController::updatePopupMenu()
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( !(m_xFrame.is() && m_xPopupMenu.is()) )
        return;

    css::util::URL aTargetURL;
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    fillPopupMenu( m_xPopupMenu );
    m_aURLToDispatchMap.free();

    for (const char* aCommand : aCommands)
    {
        aTargetURL.Complete = OUString::createFromAscii( aCommand );
        m_xURLTransformer->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        if ( xDispatch.is() )
        {
            xDispatch->addStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
            xDispatch->removeStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
            m_aURLToDispatchMap.emplace( aTargetURL.Complete, xDispatch );
        }
    }
}

// XInitialization
void SAL_CALL ControlMenuController::initialize( const Sequence< Any >& aArguments )
{
    osl::MutexGuard aLock( m_aMutex );
    svt::PopupMenuControllerBase::initialize(aArguments);
    m_aBaseURL.clear();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ControlMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ControlMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
