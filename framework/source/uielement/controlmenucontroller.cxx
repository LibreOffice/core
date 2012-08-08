/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sal/macros.h>
#include <uielement/controlmenucontroller.hxx>

#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <svl/solar.hrc>
#include <tools/rcid.h>
#include <vcl/image.hxx>
#include <svtools/menuoptions.hxx>
#include <osl/mutex.hxx>

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
#define SID_FM_CONVERTTO_GRID               (SID_MORE_FMSLOTS_START +  40)
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
#define SID_FM_CONFIG                       (SID_FMSLOTS_START + 1)
#define SID_FM_PUSHBUTTON                   (SID_FMSLOTS_START + 2)
#define SID_FM_RADIOBUTTON                  (SID_FMSLOTS_START + 3)
#define SID_FM_CHECKBOX                     (SID_FMSLOTS_START + 4)
#define SID_FM_FIXEDTEXT                    (SID_FMSLOTS_START + 5)
#define SID_FM_GROUPBOX                     (SID_FMSLOTS_START + 6)
#define SID_FM_EDIT                         (SID_FMSLOTS_START + 7)
#define SID_FM_LISTBOX                      (SID_FMSLOTS_START + 8)
#define SID_FM_COMBOBOX                     (SID_FMSLOTS_START + 9)
#define SID_FM_URLBUTTON                    (SID_FMSLOTS_START + 10)
#define SID_FM_DBGRID                       (SID_FMSLOTS_START + 11)
#define SID_FM_IMAGEBUTTON                  (SID_FMSLOTS_START + 12)
#define SID_FM_FILECONTROL                  (SID_FMSLOTS_START + 13)

static sal_Int16 nConvertSlots[] =
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

static sal_Int16 nCreateSlots[] =
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

const char* aCommands[] =
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

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::style;
using namespace com::sun::star::container;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   ControlMenuController                   ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_CONTROLMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   ControlMenuController, {} )

ControlMenuController::ControlMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    svt::PopupMenuControllerBase( xServiceManager ),
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
    ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILocale());
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

    delete pResMgr;
}

// private function
void ControlMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu && m_pResPopupMenu )
        *pVCLPopupMenu = *m_pResPopupMenu;
}

// XEventListener
void SAL_CALL ControlMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    osl::ResettableMutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
    delete m_pResPopupMenu;
}

// XStatusListener
void SAL_CALL ControlMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
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

    if ( nMenuId )
    {
        VCLXPopupMenu*  pPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( m_xPopupMenu );

        SolarMutexGuard aSolarMutexGuard;

        PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

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

            pVCLPopupMenu->InsertItem( nMenuId, m_pResPopupMenu->GetItemText( nMenuId ), m_pResPopupMenu->GetItemBits( nMenuId ), ++nPrevInConversion );
            pVCLPopupMenu->SetItemImage( nMenuId, m_pResPopupMenu->GetItemImage( nMenuId ));
            pVCLPopupMenu->SetHelpId( nMenuId, m_pResPopupMenu->GetHelpId( nMenuId ));
        }
    }
}

// XMenuListener
void ControlMenuController::impl_select(const Reference< XDispatch >& /*_xDispatch*/,const ::com::sun::star::util::URL& aURL)
{
    UrlToDispatchMap::iterator pIter = m_aURLToDispatchMap.find( aURL.Complete );
    if ( pIter != m_aURLToDispatchMap.end() )
    {
        Sequence<PropertyValue>      aArgs;
        Reference< XDispatch > xDispatch = pIter->second;
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, aArgs );
    }
}

void SAL_CALL ControlMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    if ( m_xPopupMenu.is() )
    {
        SolarMutexGuard aSolarMutexGuard;

        // Check if some modes have changed so we have to update our menu images
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bShowMenuImages    = rSettings.GetUseImagesInMenus();
        sal_Bool bUpdateImages      = (bShowMenuImages != m_bShowMenuImages);

        if ( bUpdateImages )
        {
            m_bShowMenuImages   = bShowMenuImages;

            VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( m_xPopupMenu );
            if ( pPopupMenu )
            {
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
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
        ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILocale());
        if ( pResMgr )
        {
            ResId aResId( RID_FMSHELL_CONVERSIONMENU, *pResMgr );
            aResId.SetRT( RSC_MENU );
            if ( pResMgr->IsAvailable( aResId ))
                m_pResPopupMenu = new PopupMenu( aResId );

            updateImagesPopupMenu( m_pResPopupMenu );
            delete pResMgr;
        }
    } // if ( m_pResPopupMenu == 0 )
}

void SAL_CALL ControlMenuController::updatePopupMenu() throw (::com::sun::star::uno::RuntimeException)
{
    osl::ResettableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        URL aTargetURL;
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        fillPopupMenu( m_xPopupMenu );
        m_aURLToDispatchMap.free();

        for (sal_uInt32 i=0; i<sizeof(aCommands)/sizeof(aCommands[0]); ++i)
        {
            aTargetURL.Complete = rtl::OUString::createFromAscii( aCommands[i] );
            m_xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
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
void SAL_CALL ControlMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    osl::ResettableMutexGuard aLock( m_aMutex );
    svt::PopupMenuControllerBase::initialize(aArguments);
    m_aBaseURL = ::rtl::OUString();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
