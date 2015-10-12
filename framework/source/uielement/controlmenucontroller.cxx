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

#include <stdtypes.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/solar.hrc>
#include <tools/rcid.h>
#include <vcl/image.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <osl/mutex.hxx>
#include <memory>

// Copied from svx
// Function-Id's
#define RID_FMSHELL_CONVERSIONMENU (RID_FORMS_START + 4)
#define RID_SVXIMGLIST_FMEXPL      (RID_FORMS_START + 0)

// Forms - Ids, used to address images from image list
#define SID_FMSLOTS_START                   (SID_SVX_START + 592)
#define SID_MORE_FMSLOTS_START              (SID_SVX_START + 702)

#define SID_FM_CONVERTTO_EDIT               (SID_MORE_FMSLOTS_START +  32)
#define SID_FM_CONVERTTO_BUTTON             (SID_MORE_FMSLOTS_START +  33)
#define SID_FM_CONVERTTO_FIXEDTEXT          (SID_MORE_FMSLOTS_START +  34)
#define SID_FM_CONVERTTO_LISTBOX            (SID_MORE_FMSLOTS_START +  35)
#define SID_FM_CONVERTTO_CHECKBOX           (SID_MORE_FMSLOTS_START +  36)
#define SID_FM_CONVERTTO_RADIOBUTTON        (SID_MORE_FMSLOTS_START +  37)
#define SID_FM_CONVERTTO_GROUPBOX           (SID_MORE_FMSLOTS_START +  38)
#define SID_FM_CONVERTTO_COMBOBOX           (SID_MORE_FMSLOTS_START +  39)
#define SID_FM_CONVERTTO_IMAGEBUTTON        (SID_MORE_FMSLOTS_START +  41)
#define SID_FM_CONVERTTO_FILECONTROL        (SID_MORE_FMSLOTS_START +  42)
#define SID_FM_CONVERTTO_DATE               (SID_MORE_FMSLOTS_START +  43)
#define SID_FM_CONVERTTO_TIME               (SID_MORE_FMSLOTS_START +  44)
#define SID_FM_CONVERTTO_NUMERIC            (SID_MORE_FMSLOTS_START +  45)
#define SID_FM_CONVERTTO_CURRENCY           (SID_MORE_FMSLOTS_START +  46)
#define SID_FM_CONVERTTO_PATTERN            (SID_MORE_FMSLOTS_START +  47)
#define SID_FM_CONVERTTO_IMAGECONTROL       (SID_MORE_FMSLOTS_START +  48)
#define SID_FM_CONVERTTO_FORMATTED          (SID_MORE_FMSLOTS_START +  49)
#define SID_FM_CONVERTTO_SCROLLBAR          (SID_MORE_FMSLOTS_START +  68)
#define SID_FM_CONVERTTO_SPINBUTTON         (SID_MORE_FMSLOTS_START +  69)

#define SID_FM_DATEFIELD                    (SID_MORE_FMSLOTS_START +   2)
#define SID_FM_TIMEFIELD                    (SID_MORE_FMSLOTS_START +   3)
#define SID_FM_NUMERICFIELD                 (SID_MORE_FMSLOTS_START +   4)
#define SID_FM_CURRENCYFIELD                (SID_MORE_FMSLOTS_START +   5)
#define SID_FM_PATTERNFIELD                 (SID_MORE_FMSLOTS_START +   6)
#define SID_FM_IMAGECONTROL                 (SID_MORE_FMSLOTS_START +   8)
#define SID_FM_FORMATTEDFIELD               (SID_MORE_FMSLOTS_START +  26)
#define SID_FM_SCROLLBAR                    (SID_MORE_FMSLOTS_START +  66)
#define SID_FM_SPINBUTTON                   (SID_MORE_FMSLOTS_START +  67)
#define SID_FM_PUSHBUTTON                   (SID_FMSLOTS_START + 2)
#define SID_FM_RADIOBUTTON                  (SID_FMSLOTS_START + 3)
#define SID_FM_CHECKBOX                     (SID_FMSLOTS_START + 4)
#define SID_FM_FIXEDTEXT                    (SID_FMSLOTS_START + 5)
#define SID_FM_GROUPBOX                     (SID_FMSLOTS_START + 6)
#define SID_FM_EDIT                         (SID_FMSLOTS_START + 7)
#define SID_FM_LISTBOX                      (SID_FMSLOTS_START + 8)
#define SID_FM_COMBOBOX                     (SID_FMSLOTS_START + 9)
#define SID_FM_IMAGEBUTTON                  (SID_FMSLOTS_START + 12)
#define SID_FM_FILECONTROL                  (SID_FMSLOTS_START + 13)

static const sal_Int16 nConvertSlots[] =
{
    SID_FM_CONVERTTO_EDIT,
    SID_FM_CONVERTTO_BUTTON,
    SID_FM_CONVERTTO_FIXEDTEXT,
    SID_FM_CONVERTTO_LISTBOX,
    SID_FM_CONVERTTO_CHECKBOX,
    SID_FM_CONVERTTO_RADIOBUTTON,
    SID_FM_CONVERTTO_GROUPBOX,
    SID_FM_CONVERTTO_COMBOBOX,
//  SID_FM_CONVERTTO_GRID,
    SID_FM_CONVERTTO_IMAGEBUTTON,
    SID_FM_CONVERTTO_FILECONTROL,
    SID_FM_CONVERTTO_DATE,
    SID_FM_CONVERTTO_TIME,
    SID_FM_CONVERTTO_NUMERIC,
    SID_FM_CONVERTTO_CURRENCY,
    SID_FM_CONVERTTO_PATTERN,
    SID_FM_CONVERTTO_IMAGECONTROL,
    SID_FM_CONVERTTO_FORMATTED,
    SID_FM_CONVERTTO_SCROLLBAR,
    SID_FM_CONVERTTO_SPINBUTTON
};

static const sal_Int16 nCreateSlots[] =
{
    SID_FM_EDIT,
    SID_FM_PUSHBUTTON,
    SID_FM_FIXEDTEXT,
    SID_FM_LISTBOX,
    SID_FM_CHECKBOX,
    SID_FM_RADIOBUTTON,
    SID_FM_GROUPBOX,
    SID_FM_COMBOBOX,
//  SID_FM_DBGRID,
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_DATEFIELD,
    SID_FM_TIMEFIELD,
    SID_FM_NUMERICFIELD,
    SID_FM_CURRENCYFIELD,
    SID_FM_PATTERNFIELD,
    SID_FM_IMAGECONTROL,
    SID_FM_FORMATTEDFIELD,
    SID_FM_SCROLLBAR,
    SID_FM_SPINBUTTON
};

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
//    ".uno:ConvertToGrid",
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
    ".uno:ConvertToSpinButton"
};

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

namespace {

class ControlMenuController :  public svt::PopupMenuControllerBase
{
    using svt::PopupMenuControllerBase::disposing;

public:
    ControlMenuController( const uno::Reference< uno::XComponentContext >& xContext );
    virtual ~ControlMenuController();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.ControlMenuController");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.frame.PopupMenuController";
        return aSeq;
    }

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() throw (uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException, std::exception) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const frame::FeatureStateEvent& Event ) throw ( uno::RuntimeException, std::exception ) override;

    // XMenuListener
    virtual void SAL_CALL itemActivated( const awt::MenuEvent& rEvent ) throw (uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) throw ( uno::RuntimeException, std::exception ) override;

private:
    virtual void impl_setPopupMenu() override;

    class UrlToDispatchMap : public std::unordered_map< OUString,
                                                        uno::Reference< frame::XDispatch >,
                                                        OUStringHash,
                                                        std::equal_to< OUString > >
    {
        public:
            inline void free()
            {
                UrlToDispatchMap().swap( *this );// get rid of reserved capacity
            }
    };

    void updateImagesPopupMenu( PopupMenu* pPopupMenu );
    void fillPopupMenu( uno::Reference< awt::XPopupMenu >& rPopupMenu );

    bool            m_bShowMenuImages : 1;
    PopupMenu*          m_pResPopupMenu;
    UrlToDispatchMap    m_aURLToDispatchMap;
};

ControlMenuController::ControlMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_pResPopupMenu( 0 )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bShowMenuImages   = rSettings.GetUseImagesInMenus();

}

ControlMenuController::~ControlMenuController()
{
}

// private function
void ControlMenuController::updateImagesPopupMenu( PopupMenu* pPopupMenu )
{
    std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILanguageTag()));
    ResId aResId( RID_SVXIMGLIST_FMEXPL, *pResMgr );
    aResId.SetRT( RSC_IMAGELIST );

    if ( pResMgr->IsAvailable( aResId ))
    {
        ImageList aImageList( aResId );
        for ( sal_uInt32 i=0; i < sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i )
        {
            // das entsprechende Image dran
            if ( m_bShowMenuImages )
                pPopupMenu->SetItemImage( nConvertSlots[i], aImageList.GetImage(nCreateSlots[i]));
            else
                pPopupMenu->SetItemImage( nConvertSlots[i], Image() );
        }
    }
}

// private function
void ControlMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*                                     pPopupMenu        = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*                                         pVCLPopupMenu     = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu && m_pResPopupMenu )
        *pVCLPopupMenu = *m_pResPopupMenu;
}

// XEventListener
void SAL_CALL ControlMenuController::disposing( const EventObject& ) throw ( RuntimeException, std::exception )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::ResettableMutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
    delete m_pResPopupMenu;
}

// XStatusListener
void SAL_CALL ControlMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    sal_uInt16 nMenuId = 0;
    for (sal_uInt32 i=0; i < sizeof(aCommands) / sizeof (aCommands[0]); ++i)
    {
        if ( Event.FeatureURL.Complete.equalsAscii( aCommands[i] ))
        {
            nMenuId = nConvertSlots[i];
            break;
        }
    }

    VCLXPopupMenu*  pPopupMenu = NULL;

    if ( nMenuId )
        pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( m_xPopupMenu ));

    if (pPopupMenu)
    {
        SolarMutexGuard aSolarMutexGuard;

        PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

        if ( !Event.IsEnabled && pVCLPopupMenu->GetItemPos( nMenuId ) != MENU_ITEM_NOTFOUND )
            pVCLPopupMenu->RemoveItem( pVCLPopupMenu->GetItemPos( nMenuId ));
        else if ( Event.IsEnabled && pVCLPopupMenu->GetItemPos( nMenuId ) == MENU_ITEM_NOTFOUND )
        {
            sal_Int16 nSourcePos = m_pResPopupMenu->GetItemPos(nMenuId);
            sal_Int16 nPrevInSource = nSourcePos;
            sal_uInt16 nPrevInConversion = MENU_ITEM_NOTFOUND;
            while (nPrevInSource>0)
            {
                sal_Int16 nPrevId = m_pResPopupMenu->GetItemId(--nPrevInSource);

                // do we have the source's predecessor in our conversion menu, too ?
                nPrevInConversion = pVCLPopupMenu->GetItemPos( nPrevId );
                if ( nPrevInConversion != MENU_ITEM_NOTFOUND )
                    break;
            }

          if ( MENU_ITEM_NOTFOUND == nPrevInConversion )
                // none of the items which precede the nSID-slot in the source menu are present in our conversion menu
                nPrevInConversion = sal::static_int_cast< sal_uInt16 >(-1); // put the item at the first position

            pVCLPopupMenu->InsertItem( nMenuId, m_pResPopupMenu->GetItemText( nMenuId ), m_pResPopupMenu->GetItemBits( nMenuId ), OString(), ++nPrevInConversion );
            pVCLPopupMenu->SetItemImage( nMenuId, m_pResPopupMenu->GetItemImage( nMenuId ));
            pVCLPopupMenu->SetHelpId( nMenuId, m_pResPopupMenu->GetHelpId( nMenuId ));
        }
    }
}

// XMenuListener
void SAL_CALL ControlMenuController::itemActivated( const css::awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    if ( m_xPopupMenu.is() )
    {
        SolarMutexGuard aSolarMutexGuard;

        // Check if some modes have changed so we have to update our menu images
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        bool bShowMenuImages    = rSettings.GetUseImagesInMenus();
        bool bUpdateImages      = (bShowMenuImages != m_bShowMenuImages);

        if ( bUpdateImages )
        {
            m_bShowMenuImages   = bShowMenuImages;

            VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( m_xPopupMenu ));
            if ( pPopupMenu )
            {
                PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());
                if ( pVCLPopupMenu && bUpdateImages )
                    updateImagesPopupMenu( pVCLPopupMenu );
            }
        }
    }
}

// XPopupMenuController
void ControlMenuController::impl_setPopupMenu()
{
    if ( m_pResPopupMenu == 0 )
    {
        std::unique_ptr<ResMgr> pResMgr(ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILanguageTag()));
        if ( pResMgr )
        {
            ResId aResId( RID_FMSHELL_CONVERSIONMENU, *pResMgr );
            aResId.SetRT( RSC_MENU );
            if ( pResMgr->IsAvailable( aResId ))
            {
                m_pResPopupMenu = new PopupMenu( aResId );
                updateImagesPopupMenu( m_pResPopupMenu );
            }
        }
    } // if ( m_pResPopupMenu == 0 )
}

void SAL_CALL ControlMenuController::updatePopupMenu() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        css::util::URL aTargetURL;
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        fillPopupMenu( m_xPopupMenu );
        m_aURLToDispatchMap.free();

        for (sal_uInt32 i=0; i<sizeof(aCommands)/sizeof(aCommands[0]); ++i)
        {
            aTargetURL.Complete = OUString::createFromAscii( aCommands[i] );
            m_xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
            if ( xDispatch.is() )
            {
                xDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
                xDispatch->removeStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
                m_aURLToDispatchMap.insert( UrlToDispatchMap::value_type( aTargetURL.Complete, xDispatch ));
            }
        }
    }
}

// XInitialization
void SAL_CALL ControlMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException, std::exception )
{
    osl::ResettableMutexGuard aLock( m_aMutex );
    svt::PopupMenuControllerBase::initialize(aArguments);
    m_aBaseURL.clear();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ControlMenuController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ControlMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
