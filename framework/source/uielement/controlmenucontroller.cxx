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
#include <vcl/builder.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/image.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <osl/mutex.hxx>
#include <memory>
#include <string_view>
#include <unordered_map>

#include <bitmaps.hlst>

// See svx/source/form/fmshimp.cxx for other use of this .ui

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
    virtual void impl_setPopupMenu() override;

    class UrlToDispatchMap : public std::unordered_map< OUString,
                                                        uno::Reference< frame::XDispatch > >
    {
        public:
            void free()
            {
                UrlToDispatchMap().swap( *this );// get rid of reserved capacity
            }
    };

    void updateImagesPopupMenu( PopupMenu* pPopupMenu );
    void fillPopupMenu( uno::Reference< awt::XPopupMenu > const & rPopupMenu );

    bool                m_bShowMenuImages : 1;
    std::unique_ptr<VclBuilder> m_xBuilder;
    VclPtr<PopupMenu>   m_xResPopupMenu;
    UrlToDispatchMap    m_aURLToDispatchMap;
};

ControlMenuController::ControlMenuController(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : svt::PopupMenuControllerBase(xContext)
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();

}

// private function
void ControlMenuController::updateImagesPopupMenu( PopupMenu* pPopupMenu )
{
    for (size_t i=0; i < SAL_N_ELEMENTS(aCommands); ++i)
    {
        //ident is .uno:Command without .uno:
        OString sIdent = OString(aCommands[i]).copy(5);
        sal_uInt16 nId = pPopupMenu->GetItemId(sIdent);
        if (m_bShowMenuImages)
            pPopupMenu->SetItemImage(nId, Image(StockImage::Yes, aImgIds[i]));
        else
            pPopupMenu->SetItemImage(nId, Image());
    }
}

// private function
void ControlMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    VCLXPopupMenu* pPopupMenu        = static_cast<VCLXPopupMenu *>(comphelper::getUnoTunnelImplementation<VCLXMenu>( rPopupMenu ));
    PopupMenu*     pVCLPopupMenu     = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if (pVCLPopupMenu && m_xResPopupMenu)
        *pVCLPopupMenu = *m_xResPopupMenu;
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
    m_xResPopupMenu.clear();
    m_xBuilder.reset();
}

// XStatusListener
void SAL_CALL ControlMenuController::statusChanged( const FeatureStateEvent& Event )
{
    osl::MutexGuard aLock( m_aMutex );

    OString sIdent;
    for (size_t i=0; i < SAL_N_ELEMENTS(aCommands); ++i)
    {
        if ( Event.FeatureURL.Complete.equalsAscii( aCommands[i] ))
        {
            //ident is .uno:Command without .uno:
            sIdent = OString(aCommands[i]).copy(5);
            break;
        }
    }

    sal_uInt16 nMenuId = 0;

    VCLXPopupMenu*  pPopupMenu = nullptr;

    if (!sIdent.isEmpty() && m_xResPopupMenu)
    {
        pPopupMenu = static_cast<VCLXPopupMenu *>(comphelper::getUnoTunnelImplementation<VCLXMenu>( m_xPopupMenu ));
        nMenuId = m_xResPopupMenu->GetItemId(sIdent);
    }

    if (!pPopupMenu)
        return;

    SolarMutexGuard aSolarMutexGuard;

    PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( !Event.IsEnabled && pVCLPopupMenu->GetItemPos( nMenuId ) != MENU_ITEM_NOTFOUND )
        pVCLPopupMenu->RemoveItem( pVCLPopupMenu->GetItemPos( nMenuId ));
    else if ( Event.IsEnabled && pVCLPopupMenu->GetItemPos( nMenuId ) == MENU_ITEM_NOTFOUND )
    {
        sal_Int16 nSourcePos = m_xResPopupMenu->GetItemPos(nMenuId);
        sal_Int16 nPrevInSource = nSourcePos;
        sal_uInt16 nPrevInConversion = MENU_ITEM_NOTFOUND;
        while (nPrevInSource>0)
        {
            sal_Int16 nPrevId = m_xResPopupMenu->GetItemId(--nPrevInSource);

            // do we have the source's predecessor in our conversion menu, too ?
            nPrevInConversion = pVCLPopupMenu->GetItemPos( nPrevId );
            if ( nPrevInConversion != MENU_ITEM_NOTFOUND )
                break;
        }

        if ( MENU_ITEM_NOTFOUND == nPrevInConversion )
            // none of the items which precede the nSID-slot in the source menu are present in our conversion menu
            nPrevInConversion = sal::static_int_cast< sal_uInt16 >(-1); // put the item at the first position

        pVCLPopupMenu->InsertItem(nMenuId, m_xResPopupMenu->GetItemText(nMenuId), m_xResPopupMenu->GetItemBits(nMenuId), OString(), ++nPrevInConversion);
        pVCLPopupMenu->SetItemImage(nMenuId, m_xResPopupMenu->GetItemImage(nMenuId));
        pVCLPopupMenu->SetHelpId(nMenuId, m_xResPopupMenu->GetHelpId(nMenuId));
    }
}

// XMenuListener
void SAL_CALL ControlMenuController::itemActivated( const css::awt::MenuEvent& )
{
    osl::MutexGuard aLock( m_aMutex );

    if ( !m_xPopupMenu.is() )
        return;

    SolarMutexGuard aSolarMutexGuard;

    // Check if some modes have changed so we have to update our menu images
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    bool bShowMenuImages    = rSettings.GetUseImagesInMenus();

    if (bShowMenuImages != m_bShowMenuImages)
    {
        m_bShowMenuImages   = bShowMenuImages;

        VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(comphelper::getUnoTunnelImplementation<VCLXMenu>( m_xPopupMenu ));
        if ( pPopupMenu )
        {
            PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());
            if (pVCLPopupMenu)
                updateImagesPopupMenu( pVCLPopupMenu );
        }
    }
}

// XPopupMenuController
void ControlMenuController::impl_setPopupMenu()
{
    if (!m_xResPopupMenu)
    {
        m_xBuilder.reset(new VclBuilder(nullptr, AllSettings::GetUIRootDir(), "svx/ui/convertmenu.ui", ""));
        m_xResPopupMenu = m_xBuilder->get_menu("menu");
        updateImagesPopupMenu(m_xResPopupMenu);
    }
}

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

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ControlMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ControlMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
