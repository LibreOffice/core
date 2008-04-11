/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlmenucontroller.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <uielement/controlmenucontroller.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif
#include <tools/rcid.h>
#include <vcl/image.hxx>
#include <svtools/menuoptions.hxx>

// Copied from svx
// Function-Id's
#define SID_LIB_START           10000
#define SID_SVX_START           (SID_LIB_START +   0)

#define RID_FMSHELL_CONVERSIONMENU (RID_FORMS_START + 4)
#define RID_SVXIMGLIST_FMEXPL      (RID_FORMS_START + 0)
#define RID_SVXIMGLIST_FMEXPL_HC   (RID_FORMS_START + 2)

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

sal_Int16 nConvertSlots[] =
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

sal_Int16 nCreateSlots[] =
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
//

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
    PopupMenuControllerBase( xServiceManager ),
    m_pResPopupMenu( 0 )
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrast    = rSettings.GetMenuColor().IsDark();
    m_bShowMenuImages   = SvtMenuOptions().IsMenuIconsEnabled();
}

ControlMenuController::~ControlMenuController()
{
}

// private function
void ControlMenuController::updateImagesPopupMenu( PopupMenu* pPopupMenu )
{
    rtl::OUString aResName( RTL_CONSTASCII_USTRINGPARAM( "svx" ));

    ResMgr* pResMgr = ResMgr::CreateResMgr( rtl::OUStringToOString( aResName, RTL_TEXTENCODING_ASCII_US ));
    ResId aResId( m_bWasHiContrast ? RID_SVXIMGLIST_FMEXPL_HC : RID_SVXIMGLIST_FMEXPL, *pResMgr );
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

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

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

    ResetableGuard aLock( m_aLock );
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
    ResetableGuard aLock( m_aLock );

    USHORT nMenuId = 0;
    for (sal_uInt32 i=0; i < sizeof(aCommands)/sizeof(aCommands[0]); ++i)
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

        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

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
void SAL_CALL ControlMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL ControlMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xRefDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xRefDispatch    = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs( 1 );
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            UrlToDispatchMap::iterator pIter = m_aURLToDispatchMap.find( aTargetURL.Complete );
            if ( pIter != m_aURLToDispatchMap.end() )
            {
                Reference< XDispatch > xDispatch = pIter->second;
                if ( xDispatch.is() )
                    xDispatch->dispatch( aTargetURL, aArgs );
            }
        }
    }
}

void SAL_CALL ControlMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xPopupMenu.is() )
    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        // Check if some modes have changed so we have to update our menu images
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        sal_Bool bIsHiContrast      = rSettings.GetMenuColor().IsDark();
        sal_Bool bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();
        sal_Bool bUpdateImages      = (( m_bWasHiContrast != bIsHiContrast ) || ( bShowMenuImages != m_bShowMenuImages ));

        if ( bUpdateImages )
        {
            // The mode has changed or the complete menu so we have to retrieve all images again
            m_bWasHiContrast    = bIsHiContrast;
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

void SAL_CALL ControlMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL ControlMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( m_pResPopupMenu == 0 )
        {
            rtl::OStringBuffer aBuf( 32 );
            aBuf.append( "svx" );

            ResMgr* pResMgr = ResMgr::CreateResMgr( aBuf.getStr() );
            if ( pResMgr )
            {
                ResId aResId( RID_FMSHELL_CONVERSIONMENU, *pResMgr );
                aResId.SetRT( RSC_MENU );
                if ( pResMgr->IsAvailable( aResId ))
                    m_pResPopupMenu = new PopupMenu( aResId );

                updateImagesPopupMenu( m_pResPopupMenu );
                delete pResMgr;
            }
        }

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
    }
}

void SAL_CALL ControlMenuController::updatePopupMenu() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && m_xPopupMenu.is() )
    {
        URL aTargetURL;
        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );

        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        fillPopupMenu( m_xPopupMenu );
        m_aURLToDispatchMap.free();

        for (sal_uInt32 i=0; i<sizeof(aCommands)/sizeof(aCommands[0]); ++i)
        {
            aTargetURL.Complete = rtl::OUString::createFromAscii( aCommands[i] );
            xURLTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            if ( xDispatch.is() )
            {
                xDispatch->addStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
                xDispatch->removeStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
                m_aURLToDispatchMap.insert( UrlToDispatchMap::value_type( aTargetURL.Complete, xDispatch ));
            }
        }
    }
}

// XInitialization
void SAL_CALL ControlMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_bInitialized = sal_True;
        }
    }
}

}
