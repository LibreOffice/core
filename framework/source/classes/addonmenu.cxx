/*************************************************************************
 *
 *  $RCSfile: addonmenu.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <limits.h>

#ifndef __FRAMEWORK_CLASSES_ADDONMENU_HXX_
#include "classes/addonmenu.hxx"
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include "classes/addonsoptions.hxx"
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_ASSERTION_HXX_
#include <macros/debug/assertion.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_MENUCONFIGURATION_HXX_
#include <classes/menuconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#include <vcl/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/menuoptions.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

namespace framework
{

static void GetMenuEntry(
    const Sequence< PropertyValue >&        rAddonMenuEntry,
    ::rtl::OUString&                        rTitle,
    ::rtl::OUString&                        rURL,
    ::rtl::OUString&                        rTarget,
    ::rtl::OUString&                        rImageId,
    ::rtl::OUString&                        rContext,
    Sequence< Sequence< PropertyValue > >&  rAddonSubMenu );


sal_Bool AddonMenu::HasElements()
{
    return AddonsOptions().HasAddonsMenu();
}

class AddonMenu_Impl
{
    private:
        static USHORT        m_nMID;

    public:
        AddonMenu*           m_pRoot;
        BOOL                 m_bInitialized;

        AddonMenu_Impl( AddonMenu* pRoot );
        AddonMenu_Impl();
        ~AddonMenu_Impl();

        static USHORT       GetMID();
};

USHORT AddonMenu_Impl::m_nMID = ADDONMENU_ITEMID_START;

AddonMenu_Impl::AddonMenu_Impl( AddonMenu* pRoot ) :
    m_pRoot(pRoot),
    m_bInitialized(FALSE)
{
}

AddonMenu_Impl::AddonMenu_Impl() :
    m_pRoot(0),
    m_bInitialized(FALSE)
{
}

AddonMenu_Impl::~AddonMenu_Impl()
{
}

USHORT AddonMenu_Impl::GetMID()
{
    m_nMID++;
    if( !m_nMID )
        m_nMID = ADDONMENU_ITEMID_START;
    return m_nMID;
}

// ------------------------------------------------------------------------

AddonMenu::AddonMenu( Reference< XFrame >& xFrame, AddonMenu* pRoot ) :
    m_xFrame( xFrame )
{
    _pImp = new AddonMenu_Impl( pRoot );
    Initialize();
}

AddonMenu::AddonMenu( Reference< XFrame >& xFrame ) :
    m_xFrame( xFrame )
{
    _pImp = new AddonMenu_Impl();
    Initialize();
}

AddonMenu::~AddonMenu()
{
    delete _pImp;

    for ( int i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            USHORT nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
        }
    }
}

void AddonMenu::Initialize()
{
    if( _pImp->m_bInitialized )
        return;

    _pImp->m_bInitialized = TRUE;

    AddonsOptions aAddonsOptions;

    const Sequence< Sequence< PropertyValue > >&    rAddonMenuEntries = aAddonsOptions.GetAddonsMenu();
    Sequence< Sequence< PropertyValue > >           aAddonSubMenu;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    BOOL bIsHiContrastMode  = rSettings.GetMenuColor().IsDark();
    BOOL bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();

    UINT32 i, nCount = rAddonMenuEntries.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( rAddonMenuEntries[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !aTitle.getLength() && !aURL.getLength() )
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            InsertSeparator();
        else
        {
            sal_Bool    bImageSet = sal_False;
            USHORT      nId = CreateMenuId();

            if ( bShowMenuImages )
            {
                if ( aImageId.getLength() > 0 )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrastMode );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        InsertItem( nId, aTitle, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aURL, FALSE, bIsHiContrastMode );
                    if ( !aImage )
                        aImage = aAddonsOptions.GetImageFromURL( aURL, FALSE, bIsHiContrastMode );

                    if ( !aImage )
                        InsertItem( nId, aTitle );
                    else
                        InsertItem( nId, aTitle, aImage );
                }
            }
            else
                InsertItem( nId, aTitle );

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes;
            pUserAttributes->aTargetFrame = aTarget;
            pUserAttributes->aImageId = aImageId;
            SetUserValue( nId, (ULONG)pUserAttributes );

            SetItemCommand( nId, aURL );

            if ( aAddonSubMenu.getLength() > 0 )
                SetPopupMenu( nId, BuildSubMenu( aAddonSubMenu ));
        }
    }
}

PopupMenu* AddonMenu::BuildSubMenu( Sequence< Sequence< PropertyValue > > aAddonSubMenuDefinition )
{
    Sequence< Sequence< PropertyValue > >   aAddonSubMenu;
    const StyleSettings&                    rSettings           = Application::GetSettings().GetStyleSettings();
    BOOL                                    bIsHiContrastMode   = rSettings.GetMenuColor().IsDark();
    BOOL                                    bShowMenuImages     = SvtMenuOptions().IsMenuIconsEnabled();
    UINT32                                  i, nCount           = aAddonSubMenuDefinition.getLength();
    PopupMenu*                              pPopupMenu          = new PopupMenu;
    AddonsOptions                           aAddonsOptions;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aAddonSubMenuDefinition[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !aTitle.getLength() && !aURL.getLength() )
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            pPopupMenu->InsertSeparator();
        else
        {
            sal_Bool    bImageSet = sal_False;
            USHORT      nId = CreateMenuId();

            if ( bShowMenuImages )
            {
                if ( aImageId.getLength() > 0 )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrastMode );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        pPopupMenu->InsertItem( nId, aTitle, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aURL, FALSE, bIsHiContrastMode );
                    if ( !aImage )
                        aImage = aAddonsOptions.GetImageFromURL( aURL, FALSE, bIsHiContrastMode );

                    if ( !aImage )
                        pPopupMenu->InsertItem( nId, aTitle );
                    else
                        pPopupMenu->InsertItem( nId, aTitle, aImage );
                }
            }
            else
                pPopupMenu->InsertItem( nId, aTitle );

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes;
            pUserAttributes->aTargetFrame   = aTarget;
            pUserAttributes->aImageId       = aImageId;
            pPopupMenu->SetUserValue( nId, (ULONG)pUserAttributes );

            pPopupMenu->SetItemCommand( nId, aURL );

            if ( aAddonSubMenu.getLength() > 0 )
                pPopupMenu->SetPopupMenu( nId, BuildSubMenu( aAddonSubMenu ));
        }
    }

    return pPopupMenu;
}

USHORT AddonMenu::CreateMenuId()
{
    return AddonMenu_Impl::GetMID();
}

// ------------------------------------------------------------------------

AddonPopupMenu::AddonPopupMenu( Reference< XFrame >& xFrame ) :
    m_xFrame( xFrame )
{
    _pImp = new AddonMenu_Impl();
}

// ------------------------------------------------------------------------

AddonPopupMenu::~AddonPopupMenu()
{
    delete _pImp;

    for ( int i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            USHORT nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
        }
    }
}

// ------------------------------------------------------------------------

USHORT AddonPopupMenu::CreateMenuId()
{
    return AddonMenu_Impl::GetMID();
}

// ------------------------------------------------------------------------

sal_Bool AddonPopupMenu::IsCorrectContext( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel ) const
{
    if ( rModel.is() )
    {
        Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( rModel, UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            sal_Int32 nIndex = 0;
            do
            {
                OUString aToken = m_aContext.getToken( 0, ',', nIndex );

                if ( xServiceInfo->supportsService( aToken ))
                    return sal_True;
            }
            while ( nIndex >= 0 );
        }
    }

    return sal_False;
}

// ------------------------------------------------------------------------
// Get the top-level popup menus for addons. The menubar is used as a container. Every popup menu is from type AddonPopupMenu!
void AddonPopupMenu::GetAddonPopupMenus( Reference< XFrame >& rFrame, MenuBar* pContainer )
{
    if ( pContainer )
    {
        AddonsOptions aAddonsOptions;

        const Sequence< Sequence< PropertyValue > >&    rAddonMenuEntries = aAddonsOptions.GetAddonsMenuBarPart();
        for ( sal_Int32 i = 0; i < rAddonMenuEntries.getLength(); i++ )
        {
            AddonPopupMenu* pAddonPopupMenu = new AddonPopupMenu( rFrame );
            const Sequence< PropertyValue > aAddonPopupMenu = rAddonMenuEntries[i];
            pAddonPopupMenu->Initialize( aAddonPopupMenu );

            // A valid top-level addon popup menu must have an ID, title and URL
            if ( pAddonPopupMenu->GetId() &&
                 pAddonPopupMenu->GetTitle().getLength() &&
                 pAddonPopupMenu->GetCommandURL().getLength() )
            {
                pContainer->InsertItem( pAddonPopupMenu->GetId(), pAddonPopupMenu->GetTitle() );
                pContainer->SetPopupMenu( pAddonPopupMenu->GetId(), pAddonPopupMenu );
            }
            else
                delete pAddonPopupMenu;
        }
    }
}

// ------------------------------------------------------------------------

void AddonPopupMenu::MergeAddonPopupMenus(
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel,
    USHORT                  nMergeAtPos,
    MenuBar*                pMergeMenuBar )
{
    if ( pMergeMenuBar )
    {
        AddonsOptions   aAddonsOptions;
        USHORT          nInsertPos = nMergeAtPos;

        const Sequence< Sequence< PropertyValue > >&    rAddonMenuEntries = aAddonsOptions.GetAddonsMenuBarPart();
        for ( sal_Int32 i = 0; i < rAddonMenuEntries.getLength(); i++ )
        {
            AddonPopupMenu* pAddonPopupMenu = new AddonPopupMenu( rFrame );
            const Sequence< PropertyValue > aAddonPopupMenu = rAddonMenuEntries[i];
            pAddonPopupMenu->Initialize( aAddonPopupMenu );

            // A valid top-level addon popup menu must have an ID, title and URL
            if ( pAddonPopupMenu->GetId() &&
                 pAddonPopupMenu->GetTitle().getLength() &&
                 pAddonPopupMenu->GetCommandURL().getLength() &&
                 pAddonPopupMenu->IsCorrectContext( rModel ))
            {
                pMergeMenuBar->InsertItem( pAddonPopupMenu->GetId(), pAddonPopupMenu->GetTitle(), 0, nInsertPos++ );
                pMergeMenuBar->SetPopupMenu( pAddonPopupMenu->GetId(), pAddonPopupMenu );
            }
            else
                delete pAddonPopupMenu;
        }
    }
}

// ------------------------------------------------------------------------

void AddonPopupMenu::Initialize( const Sequence< PropertyValue >& rAddonPopupMenuDefinition )
{
    if( _pImp->m_bInitialized )
        return;

    _pImp->m_bInitialized = TRUE;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;
    AddonsOptions   aAddonsOptions;
    Sequence< Sequence< PropertyValue > > aAddonSubMenu;

    GetMenuEntry( rAddonPopupMenuDefinition, aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );
    if ( aTitle.getLength() && aURL.getLength() )
    {
        // Set id, title and command URL of this popup menu
        m_nId           = CreateMenuId();
        m_aCommandURL   = aURL;
        m_aTitle        = aTitle;
        m_aContext      = aContext;

        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        BOOL bIsHiContrastMode  = rSettings.GetMenuColor().IsDark();
        BOOL bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();

        UINT32 i, nCount = aAddonSubMenu.getLength();
        for ( i = 0; i < nCount; ++i )
        {
            Sequence< Sequence< PropertyValue > > aNextAddonSubMenu;
            GetMenuEntry( aAddonSubMenu[i], aTitle, aURL, aTarget, aImageId, aContext, aNextAddonSubMenu );

            if ( !aTitle.getLength() && !aURL.getLength() )
                continue;

            if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
                InsertSeparator();
            else
            {
                sal_Bool    bImageSet = sal_False;
                USHORT      nId = CreateMenuId();

                if ( bShowMenuImages )
                {
                    if ( aImageId.getLength() > 0 )
                    {
                        Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrastMode );
                        if ( !!aImage )
                        {
                            bImageSet = sal_True;
                            InsertItem( nId, aTitle, aImage );
                        }
                    }

                    if ( !bImageSet )
                    {
                        Image aImage = GetImageFromURL( m_xFrame, aURL, FALSE, bIsHiContrastMode );
                        if ( !aImage )
                            aImage = aAddonsOptions.GetImageFromURL( aURL, FALSE, bIsHiContrastMode );

                        if ( !aImage )
                            InsertItem( nId, aTitle );
                        else
                            InsertItem( nId, aTitle, aImage );
                    }
                }
                else
                    InsertItem( nId, aTitle );

                // Store values from configuration to the New and Wizard menu entries to enable
                // sfx2 based code to support high contrast mode correctly!
                MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes;
                pUserAttributes->aTargetFrame = aTarget;
                pUserAttributes->aImageId = aImageId;
                SetUserValue( nId, (ULONG)pUserAttributes );

                SetItemCommand( nId, aURL );

                if ( aNextAddonSubMenu.getLength() > 0 )
                    SetPopupMenu( nId, BuildSubMenu( aNextAddonSubMenu ));
            }
        }
    }
}

// ------------------------------------------------------------------------

PopupMenu* AddonPopupMenu::BuildSubMenu( Sequence< Sequence< PropertyValue > > aAddonSubMenuDefinition )
{
    Sequence< Sequence< PropertyValue > >   aAddonSubMenu;
    const StyleSettings&                    rSettings           = Application::GetSettings().GetStyleSettings();
    BOOL                                    bIsHiContrastMode   = rSettings.GetMenuColor().IsDark();
    BOOL                                    bShowMenuImages     = SvtMenuOptions().IsMenuIconsEnabled();
    UINT32                                  i, nCount           = aAddonSubMenuDefinition.getLength();
    PopupMenu*                              pPopupMenu          = new PopupMenu;
    AddonsOptions                           aAddonsOptions;

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget;
    ::rtl::OUString aImageId;
    ::rtl::OUString aContext;

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aAddonSubMenuDefinition[i], aTitle, aURL, aTarget, aImageId, aContext, aAddonSubMenu );

        if ( !aTitle.getLength() && !aURL.getLength() )
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            pPopupMenu->InsertSeparator();
        else
        {
            sal_Bool    bImageSet = sal_False;
            USHORT      nId = CreateMenuId();

            if ( bShowMenuImages )
            {
                if ( aImageId.getLength() > 0 )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrastMode );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        pPopupMenu->InsertItem( nId, aTitle, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aURL, FALSE, bIsHiContrastMode );
                    if ( !aImage )
                        aImage = aAddonsOptions.GetImageFromURL( aURL, FALSE, bIsHiContrastMode );

                    if ( !aImage )
                        pPopupMenu->InsertItem( nId, aTitle );
                    else
                        pPopupMenu->InsertItem( nId, aTitle, aImage );
                }
            }
            else
                pPopupMenu->InsertItem( nId, aTitle );

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes;
            pUserAttributes->aTargetFrame   = aTarget;
            pUserAttributes->aImageId       = aImageId;
            pPopupMenu->SetUserValue( nId, (ULONG)pUserAttributes );

            pPopupMenu->SetItemCommand( nId, aURL );

            if ( aAddonSubMenu.getLength() > 0 )
                pPopupMenu->SetPopupMenu( nId, BuildSubMenu( aAddonSubMenu ));
        }
    }

    return pPopupMenu;
}

// ------------------------------------------------------------------------

void GetMenuEntry
(
    const Sequence< PropertyValue >&                rAddonMenuEntry,
    ::rtl::OUString&                                rTitle,
    ::rtl::OUString&                                rURL,
    ::rtl::OUString&                                rTarget,
    ::rtl::OUString&                                rImageId,
    ::rtl::OUString&                                rContext,
    Sequence< Sequence< PropertyValue > >&          rAddonSubMenu
)
{
    // Reset submenu parameter
    rAddonSubMenu   = Sequence< Sequence< PropertyValue > >();

    for ( int i = 0; i < rAddonMenuEntry.getLength(); i++ )
    {
        OUString aMenuEntryPropName = rAddonMenuEntry[i].Name;
        if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_URL )
            rAddonMenuEntry[i].Value >>= rURL;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_TITLE )
            rAddonMenuEntry[i].Value >>= rTitle;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_TARGET )
            rAddonMenuEntry[i].Value >>= rTarget;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER )
            rAddonMenuEntry[i].Value >>= rImageId;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_SUBMENU )
            rAddonMenuEntry[i].Value >>= rAddonSubMenu;
        else if ( aMenuEntryPropName == ADDONSMENUITEM_PROPERTYNAME_CONTEXT )
            rAddonMenuEntry[i].Value >>= rContext;
    }
}

}
