/*************************************************************************
 *
 *  $RCSfile: cfg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:14:27 $
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

#pragma hdrstop

#ifndef _HELP_HXX //autogen
#include <vcl/help.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <vcl/decoview.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/macrconf.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/tbxmgr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svtools/stritem.hxx>
#include <toolkit/unohlp.hxx>

#include <algorithm>

#include "dialogs.hrc"
#include "cfg.hrc"
#include "helpid.hrc"

#include "cfg.hxx"
#include "eventdlg.hxx"
#include "dialmgr.hxx"
#include "svxdlg.hxx"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <drafts/com/sun/star/ui/ItemType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <drafts/com/sun/star/ui/XUIConfiguration.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONLISTENER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONPERSISTENCE_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationPersistence.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONSTORAGE_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationStorage.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENT_HPP_
#include <drafts/com/sun/star/ui/XUIElement.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_UIElementType_HPP_
#include <drafts/com/sun/star/ui/UIElementType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_IMAGETYPE_HPP_
#include <drafts/com/sun/star/ui/ImageType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif

#define PRTSTR(x) rtl::OUStringToOString(x, RTL_TEXTENCODING_ASCII_US).pData->buffer

static const char ITEM_DESCRIPTOR_COMMANDURL[]  = "CommandURL";
static const char ITEM_DESCRIPTOR_HELPURL[]     = "HelpURL";
static const char ITEM_DESCRIPTOR_CONTAINER[]   = "ItemDescriptorContainer";
static const char ITEM_DESCRIPTOR_LABEL[]       = "Label";
static const char ITEM_DESCRIPTOR_TYPE[]        = "Type";
static const char ITEM_DESCRIPTOR_STYLE[]       = "Style";
static const char ITEM_DESCRIPTOR_ISVISIBLE[]   = "IsVisible";
static const char ITEM_DESCRIPTOR_RESOURCEURL[] = "ResourceURL";
static const char ITEM_DESCRIPTOR_UINAME[]      = "UIName";

static const char ITEM_MENUBAR_URL[] = "private:resource/menubar/menubar";
static const char ITEM_TOOLBAR_URL[] = "private:resource/toolbar/";

static const char __FAR_DATA pSeparatorStr[] =
    "----------------------------------";
static const char __FAR_DATA pMenuSeparatorStr[]    = " | ";

#ifdef MSC
#pragma warning (disable:4355)
#endif

using rtl::OUString;
namespace dcss = drafts::com::sun::star;
namespace uno = com::sun::star::uno;
namespace frame = com::sun::star::frame;
namespace lang = com::sun::star::lang;
namespace container = com::sun::star::container;
namespace beans = com::sun::star::beans;
namespace graphic = com::sun::star::graphic;

void printPropertySet(
    const OUString& prefix,
    const uno::Reference< beans::XPropertySet >& xPropSet )
{
    uno::Reference< beans::XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();

    uno::Sequence< beans::Property > aPropDetails =
        xPropSetInfo->getProperties();

    OSL_TRACE("printPropertySet: %d properties", aPropDetails.getLength());

    for ( sal_Int32 i = 0; i < aPropDetails.getLength(); i++ )
    {
        OUString tmp;
        uno::Any a = xPropSet->getPropertyValue( aPropDetails[i].Name );

        if ( ( a >>= tmp ) /* && tmp.getLength() != 0 */ )
        {
            OSL_TRACE("%s: Got property: %s = %s",
                PRTSTR(prefix), PRTSTR(aPropDetails[i].Name), PRTSTR(tmp));
        }
    }
}

void printProperties(
    const OUString& prefix,
    const uno::Sequence< beans::PropertyValue >& aProp )
{
    for ( sal_Int32 i = 0; i < aProp.getLength(); i++ )
    {
        OUString tmp;

        aProp[i].Value >>= tmp;

        OSL_TRACE("%s: Got property: %s = %s",
            PRTSTR(prefix), PRTSTR(aProp[i].Name), PRTSTR(tmp));
    }
}

void printEntries(SvxEntries* entries)
{
    SvxEntries::const_iterator iter = entries->begin();

    for ( ; iter != entries->end(); iter++ )
    {
        SvxConfigEntry* entry = *iter;

        OSL_TRACE("printEntries: %s", PRTSTR(entry->GetName()));
    }
}

OUString
stripHotKey( const OUString& str )
{
    sal_Int32 index = str.indexOf( '~' );
    if ( index == -1 )
    {
        return str;
    }
    else
    {
        return str.replaceAt( index, 1, OUString() );
    }
}

OUString
generateCustomName(
    const OUString& prefix,
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    // find and replace the %n placeholder in the prefix string
    OUString name;
    OUString placeholder = OUString::createFromAscii( "%n" );

    sal_Int32 pos = prefix.indexOf(
        OUString::createFromAscii( "%n" ) );

    if ( pos != -1 )
    {
        name = prefix.replaceAt(
            pos, placeholder.getLength(), OUString::valueOf( suffix ) );
    }
    else
    {
        // no placeholder found so just append the suffix
        name = prefix + OUString::valueOf( suffix );
    }

    // now check is there is an already existing entry with this name
    SvxEntries::const_iterator iter = entries->begin();

    SvxConfigEntry* pEntry;
    while ( iter != entries->end() )
    {
        pEntry = *iter;

        if ( name.equals( pEntry->GetName() ) )
        {
            break;
        }
        iter++;
    }

    if ( iter != entries->end() )
    {
        // name already exists so try the next number up
        return generateCustomName( prefix, entries, ++suffix );
    }

    return name;
}

OUString
generateCustomURL(
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    OUString url = OUString::createFromAscii( ITEM_TOOLBAR_URL );
    url += OUString::createFromAscii( "custom_toolbar_" );
    url += OUString::valueOf( suffix );

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    SvxConfigEntry* pEntry;
    while ( iter != entries->end() )
    {
        pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        iter++;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomURL( entries, ++suffix );
    }

    return url;
}

OUString
generateCustomMenuName(
    SvxEntries* entries,
    sal_Int32 suffix = 1 )
{
    OUString url = OUString::createFromAscii( "vnd.openoffice.org:CustomMenu" );
    url += OUString::valueOf( suffix );

    // now check is there is an already existing entry with this url
    SvxEntries::const_iterator iter = entries->begin();

    SvxConfigEntry* pEntry;
    while ( iter != entries->end() )
    {
        pEntry = *iter;

        if ( url.equals( pEntry->GetCommand() ) )
        {
            break;
        }
        iter++;
    }

    if ( iter != entries->end() )
    {
        // url already exists so try the next number up
        return generateCustomMenuName( entries, ++suffix );
    }

    return url;
}

void RemoveEntry( SvxEntries* pEntries, SvxConfigEntry* pChildEntry )
{
    SvxEntries::iterator iter = pEntries->begin();

    while ( iter != pEntries->end() )
    {
        if ( pChildEntry == *iter )
        {
            pEntries->erase( iter );
            break;
        }
        iter++;
    }
}

OUString GetModuleName( const OUString& aModuleId )
{
    if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.text.TextDocument" ) ) ||
         aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.text.GlobalDocument" ) ) )
    {
        return OUString::createFromAscii("Writer");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.text.WebDocument" ) ) )
    {
        return OUString::createFromAscii("Writer/Web");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.drawing.DrawingDocument" ) ) )
    {
        return OUString::createFromAscii("Draw");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.presentation.PresentationDocument" ) ) )
    {
        return OUString::createFromAscii("Impress");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.sheet.SpreadsheetDocument" ) ) )
    {
        return OUString::createFromAscii("Calc");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.script.BasicIDE" ) ) )
    {
        return OUString::createFromAscii("Basic");
    }
    else if ( aModuleId.equals( OUString::createFromAscii(
            "com.sun.star.formula.FormulaProperties" ) ) )
    {
        return OUString::createFromAscii("Math");
    }
}

bool GetMenuItemData(
    const uno::Reference< container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rHelpURL,
    OUString& rLabel,
    sal_uInt16& rType,
    uno::Reference< container::XIndexAccess >& rSubMenu )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); i++ )
            {
                if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
                {
                    aProp[i].Value >>= rCommandURL;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
                {
                    aProp[i].Value >>= rHelpURL;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_CONTAINER ))
                {
                    aProp[i].Value >>= rSubMenu;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
                {
                    aProp[i].Value >>= rType;
                }
            }

            return sal_True;
        }
    }
    catch ( ::com::sun::star::lang::IndexOutOfBoundsException& )
    {
    }

    return sal_False;
}

bool GetToolbarItemData(
    const uno::Reference< container::XIndexAccess >& rItemContainer,
    sal_Int32 nIndex,
    OUString& rCommandURL,
    OUString& rHelpURL,
    OUString& rLabel,
    sal_uInt16& rType,
    sal_Bool& rIsVisible,
    sal_Int32& rStyle,
    uno::Reference< container::XIndexAccess >& rSubMenu )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aProp;
        if ( rItemContainer->getByIndex( nIndex ) >>= aProp )
        {
            for ( sal_Int32 i = 0; i < aProp.getLength(); i++ )
            {
                if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
                {
                    aProp[i].Value >>= rCommandURL;
                }
                if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_STYLE ))
                {
                    aProp[i].Value >>= rStyle;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
                {
                    aProp[i].Value >>= rHelpURL;
                }
                else if (aProp[i].Name.equalsAscii(ITEM_DESCRIPTOR_CONTAINER))
                {
                    aProp[i].Value >>= rSubMenu;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
                {
                    aProp[i].Value >>= rLabel;
                }
                else if ( aProp[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
                {
                    aProp[i].Value >>= rType;
                }
                else if (aProp[i].Name.equalsAscii(ITEM_DESCRIPTOR_ISVISIBLE))
                {
                    aProp[i].Value >>= rIsVisible;
                }
            }

            return sal_True;
        }
    }
    catch ( ::com::sun::star::lang::IndexOutOfBoundsException& )
    {
    }

    return sal_False;
}

uno::Sequence< beans::PropertyValue >
ConvertSvxConfigEntry(
    const uno::Reference< container::XNameAccess >& xCommandToLabelMap,
    const SvxConfigEntry* pEntry )
{
    static const OUString aDescriptorCommandURL (
        RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ) );

    static const OUString aDescriptorType(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ) );

    static const OUString aDescriptorLabel(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ) );

    static const OUString aDescriptorHelpURL(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ) );

    static const OUString aDescriptorContainer(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ) );

    uno::Sequence< beans::PropertyValue > aPropSeq( 4 );

    aPropSeq[0].Name = aDescriptorCommandURL;
    aPropSeq[0].Value <<= rtl::OUString( pEntry->GetCommand() );

    aPropSeq[1].Name = aDescriptorType;
    aPropSeq[1].Value <<= dcss::ui::ItemType::DEFAULT;

    // If the name has not been changed and the name is the same as
    // in the default command to label map then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = aDescriptorLabel;
    if ( pEntry->HasChangedName() == FALSE && pEntry->GetCommand().getLength() )
    {
        BOOL isDefaultName = FALSE;
        try
        {
            uno::Any a( xCommandToLabelMap->getByName( pEntry->GetCommand() ) );
            uno::Sequence< beans::PropertyValue > tmpPropSeq;
            if ( a >>= tmpPropSeq )
            {
                for ( sal_Int32 i = 0; i < tmpPropSeq.getLength(); i++ )
                {
                    if ( tmpPropSeq[i].Name.equals( aDescriptorLabel ) )
                    {
                        OUString tmpLabel;
                        tmpPropSeq[i].Value >>= tmpLabel;

                        if ( tmpLabel.equals( pEntry->GetName() ) )
                        {
                            isDefaultName = TRUE;
                        }

                        break;
                    }
                }
            }
        }
        catch ( container::NoSuchElementException& nsee )
        {
            // isDefaultName is left as FALSE
        }

        if ( isDefaultName )
        {
            aPropSeq[2].Value <<= rtl::OUString();
        }
        else
        {
            aPropSeq[2].Value <<= rtl::OUString( pEntry->GetName() );
        }
    }
    else
    {
        aPropSeq[2].Value <<= rtl::OUString( pEntry->GetName() );
    }

    aPropSeq[3].Name = aDescriptorHelpURL;
    aPropSeq[3].Value <<= rtl::OUString( pEntry->GetHelpURL() );

    return aPropSeq;
}

uno::Sequence< beans::PropertyValue >
ConvertToolbarEntry(
    const uno::Reference< container::XNameAccess >& xCommandToLabelMap,
    const SvxConfigEntry* pEntry )
{
    static const OUString aDescriptorCommandURL (
        RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ) );

    static const OUString aDescriptorType(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ) );

    static const OUString aDescriptorLabel(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ) );

    static const OUString aDescriptorHelpURL(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ) );

    static const OUString aDescriptorContainer(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ) );

    static const OUString aIsVisible(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_ISVISIBLE ) );

    uno::Sequence< beans::PropertyValue > aPropSeq( 5 );

    aPropSeq[0].Name = aDescriptorCommandURL;
    aPropSeq[0].Value <<= rtl::OUString( pEntry->GetCommand() );

    aPropSeq[1].Name = aDescriptorType;
    aPropSeq[1].Value <<= dcss::ui::ItemType::DEFAULT;

    // If the name has not been changed and the name is the same as
    // in the default command to label map then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = aDescriptorLabel;
    if ( pEntry->HasChangedName() == FALSE && pEntry->GetCommand().getLength() )
    {
        BOOL isDefaultName = FALSE;
        try
        {
            uno::Any a( xCommandToLabelMap->getByName( pEntry->GetCommand() ) );
            uno::Sequence< beans::PropertyValue > tmpPropSeq;
            if ( a >>= tmpPropSeq )
            {
                for ( sal_Int32 i = 0; i < tmpPropSeq.getLength(); i++ )
                {
                    if ( tmpPropSeq[i].Name.equals( aDescriptorLabel ) )
                    {
                        OUString tmpLabel;
                        tmpPropSeq[i].Value >>= tmpLabel;

                        if ( tmpLabel.equals( pEntry->GetName() ) )
                        {
                            isDefaultName = TRUE;
                        }

                        break;
                    }
                }
            }
        }
        catch ( container::NoSuchElementException& )
        {
            // isDefaultName is left as FALSE
        }

        if ( isDefaultName )
        {
            aPropSeq[2].Value <<= rtl::OUString();
        }
        else
        {
            aPropSeq[2].Value <<= rtl::OUString( pEntry->GetName() );
        }
    }
    else
    {
        aPropSeq[2].Value <<= rtl::OUString( pEntry->GetName() );
    }

    aPropSeq[3].Name = aDescriptorHelpURL;
    aPropSeq[3].Value <<= rtl::OUString( pEntry->GetHelpURL() );

    aPropSeq[4].Name = aIsVisible;
    aPropSeq[4].Value <<= pEntry->IsVisible();

    return aPropSeq;
}

SfxTabPage *CreateSvxMenuConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SvxMenuConfigPage( pParent, rSet );
}

SfxTabPage *CreateKeyboardConfigPage( Window *pParent, const SfxItemSet& rSet )
{
       return SfxAbstractDialogFactory::CreateSfxAcceleratorConfigPage( pParent, rSet );
}

SfxTabPage *CreateSvxToolbarConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SvxToolbarConfigPage( pParent, rSet );
}

SfxTabPage *CreateSvxEventConfigPage( Window *pParent, const SfxItemSet& rSet )
{
    return new SvxEventConfigPage( pParent, rSet );
}

/******************************************************************************
 *
 * SvxConfigDialog is the configuration dialog which is brought up from the
 * Tools menu. It includes tabs for customizing menus, toolbars, events and
 * key bindings.
 *
 *****************************************************************************/
SvxConfigDialog::SvxConfigDialog(
    Window * pParent, const SfxItemSet* pSet, SfxViewFrame* pFrame )
    :
        SfxTabDialog( pFrame, pParent,
            ResId( RID_SVXDLG_CUSTOMIZE, DIALOG_MGR() ), pSet )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_MENUS, CreateSvxMenuConfigPage, NULL );
    AddTabPage( RID_SVXPAGE_KEYBOARD, CreateKeyboardConfigPage, NULL );
    AddTabPage( RID_SVXPAGE_TOOLBARS, CreateSvxToolbarConfigPage, NULL );
    AddTabPage( RID_SVXPAGE_EVENTS, CreateSvxEventConfigPage, NULL );

    const SfxPoolItem* pItem =
        pSet->GetItem( pSet->GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = ((const SfxStringItem*)pItem)->GetValue();

        if (text.indexOf(OUString::createFromAscii(ITEM_TOOLBAR_URL)) == 0)
        {
            SetCurPageId( RID_SVXPAGE_TOOLBARS );
        }
    }
}

SvxConfigDialog::~SvxConfigDialog()
{
}

short SvxConfigDialog::Ok()
{
    return SfxTabDialog::Ok();
}

void SvxConfigDialog::PageCreated( USHORT nId, SfxTabPage& rPage )
{
    switch ( nId )
    {
        case RID_SVXPAGE_MENUS:
            break;
        case RID_SVXPAGE_TOOLBARS:
            break;
        default:
            break;
    }
}

void SvxConfigDialog::ActivateTabPage( USHORT nSlotId )
{
    switch ( nSlotId )
    {
    }
}

/******************************************************************************
 *
 * The SaveInData class is used to hold data for entries in the Save In
 * ListBox controls in the menu and toolbar tabs
 *
 ******************************************************************************/

// Initialize static variable which holds default XImageManager
uno::Reference< dcss::ui::XImageManager>* SaveInData::xDefaultImgMgr = NULL;

SaveInData::SaveInData(
    const uno::Reference< dcss::ui::XUIConfigurationManager >& xCfgMgr,
    const OUString& aModuleId,
    bool isDocConfig )
        :
            m_xCfgMgr( xCfgMgr ),
            bReadOnly( FALSE ),
            bDocConfig( isDocConfig ),
            bModified( FALSE )
{
    uno::Reference< beans::XPropertySet > xProps(
        ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY );

    xProps->getPropertyValue(
        OUString::createFromAscii( "DefaultContext" ))
            >>= m_xComponentContext;

    m_aSeparatorSeq.realloc( 1 );
    m_aSeparatorSeq[0].Name  = OUString(
        RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ) );
    m_aSeparatorSeq[0].Value <<= dcss::ui::ItemType::SEPARATOR_LINE;

    if ( bDocConfig )
    {
        uno::Reference< dcss::ui::XUIConfigurationPersistence >
            xDocPersistence( GetConfigManager(), uno::UNO_QUERY );

        bReadOnly = xDocPersistence->isReadOnly();
    }

    m_xServiceManager = uno::Reference< lang::XMultiServiceFactory >(
        ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );

    uno::Reference< container::XNameAccess > xNameAccess(
        m_xServiceManager->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "drafts.com.sun.star.frame.UICommandDescription" ) ) ),
        uno::UNO_QUERY );

    if ( xNameAccess.is() )
        xNameAccess->getByName( aModuleId ) >>= m_xCommandToLabelMap;

    if ( !m_xImgMgr.is() )
    {
        m_xImgMgr = uno::Reference< dcss::ui::XImageManager >(
            GetConfigManager()->getImageManager(), uno::UNO_QUERY );
    }

    // If this is not a document configuration then it is the settings
    // for the module (writer, calc, impress etc.) Use this as the default
    // XImageManager instance
    if ( !IsDocConfig() )
    {
        xDefaultImgMgr = &m_xImgMgr;
    }
}

sal_Int16 GetImageType()
{
    sal_Int16 nImageType( dcss::ui::ImageType::COLOR_NORMAL |
                              dcss::ui::ImageType::SIZE_DEFAULT );

    bool bBig =
        ( Application::GetSettings().GetStyleSettings().GetToolbarIconSize()
            == STYLE_TOOLBAR_ICONSIZE_LARGE );

    bool bHiContrast = FALSE;
    Window* topwin = Application::GetActiveTopWindow();

    if ( topwin != NULL &&
         topwin->GetDisplayBackground().GetColor().IsDark() )
    {
        bHiContrast = TRUE;
    }

    if ( bBig )
    {
        nImageType |= dcss::ui::ImageType::SIZE_LARGE;
    }

    if ( bHiContrast )
    {
        nImageType |= dcss::ui::ImageType::COLOR_HIGHCONTRAST;
    }

    return nImageType;
}

uno::Reference< graphic::XGraphic > GetGraphic(
    const uno::Reference< dcss::ui::XImageManager >& xImageManager,
    const OUString& rCommandURL )
{
    uno::Reference< graphic::XGraphic > result;

    if ( xImageManager.is() )
    {
        // TODO handle large and high contrast graphics
        uno::Sequence< uno::Reference< graphic::XGraphic > > aGraphicSeq;

        uno::Sequence< OUString > aImageCmdSeq( 1 );
        aImageCmdSeq[0] = rCommandURL;

        try
        {
            aGraphicSeq =
                xImageManager->getImages( GetImageType(), aImageCmdSeq );

            if ( aGraphicSeq.getLength() > 0 )
            {
                result =  aGraphicSeq[0];
            }
        }
        catch ( uno::Exception& )
        {
            // will return empty XGraphic
        }
    }

    return result;
}

Image SaveInData::GetImage( const OUString& rCommandURL )
{
    Image aImage;

    uno::Reference< graphic::XGraphic > xGraphic =
        GetGraphic( m_xImgMgr, rCommandURL );

    if ( xGraphic.is() )
    {
        aImage = Image( xGraphic );
    }
    else if ( xDefaultImgMgr != NULL && (*xDefaultImgMgr).is() )
    {
        xGraphic = GetGraphic( (*xDefaultImgMgr), rCommandURL );

        if ( xGraphic.is() )
        {
            aImage = Image( xGraphic );
        }
    }

    return aImage;
}

bool SaveInData::PersistChanges(
    const uno::Reference< uno::XInterface >& xManager )
{
    bool result = TRUE;

    try
    {
        if ( xManager.is() && !IsReadOnly() )
        {
            uno::Reference< dcss::ui::XUIConfigurationPersistence >
                xConfigPersistence( xManager, uno::UNO_QUERY );

            if ( xConfigPersistence->isModified() )
            {
                xConfigPersistence->store();
            }
        }
    }
    catch ( com::sun::star::io::IOException& )
    {
        result = FALSE;
    }

    return result;
}

/******************************************************************************
 *
 * The MenuSaveInData class extends SaveInData and provides menu specific
 * load and store functionality.
 *
 ******************************************************************************/

// Initialize static variable which holds default Menu data
MenuSaveInData* MenuSaveInData::pDefaultData = NULL;

MenuSaveInData::MenuSaveInData(
    const uno::Reference< dcss::ui::XUIConfigurationManager >& cfgmgr,
    const OUString& aModuleId,
    bool isDocConfig )
    :
        SaveInData( cfgmgr, aModuleId, isDocConfig ),
        pRootEntry( 0 ),
        m_aDescriptorContainer(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ) ),
        m_aMenuResourceURL(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_MENUBAR_URL ) )
{
    try
    {
        OUString url( RTL_CONSTASCII_USTRINGPARAM( ITEM_MENUBAR_URL ) );
        m_xMenuSettings = GetConfigManager()->getSettings( url, sal_False );
    }
    catch ( container::NoSuchElementException& )
    {
        // will use menu settings for the module
    }

    // If this is not a document configuration then it is the settings
    // for the module (writer, calc, impress etc.). These settings should
    // be set as the default to be used for SaveIn locations that do not
    // have custom settings
    if ( !IsDocConfig() )
    {
        SetDefaultData( this );
    }
}

MenuSaveInData::~MenuSaveInData()
{
    if ( pRootEntry != NULL )
    {
        delete pRootEntry;
    }
}

SvxEntries*
MenuSaveInData::GetEntries()
{
    if ( pRootEntry == NULL )
    {
        pRootEntry = new SvxConfigEntry(
            0, String::CreateFromAscii("MainMenus"), String(), TRUE );

        if ( m_xMenuSettings.is() )
        {
            LoadSubMenus( m_xMenuSettings, String(), pRootEntry );
        }
        else if ( GetDefaultData() != NULL )
        {
            // If the doc has no config settings use module config settings
            LoadSubMenus( GetDefaultData()->m_xMenuSettings, String(), pRootEntry );
        }
    }

    return pRootEntry->GetEntries();
}

void
MenuSaveInData::SetEntries( SvxEntries* pNewEntries )
{
    // delete old menu hierarchy first
    if ( pRootEntry != NULL )
    {
        delete pRootEntry->GetEntries();
    }

    // now set new menu hierarchy
    pRootEntry->SetEntries( pNewEntries );
}

bool MenuSaveInData::LoadSubMenus(
    const uno::Reference< container::XIndexAccess >& xMenuSettings,
    const OUString& rBaseTitle,
    SvxConfigEntry* pParentData )
{
    Help*           pHelp               = Application::GetHelp();
    SvxEntries*     pEntries            = pParentData->GetEntries();

    for ( sal_Int32 nIndex = 0; nIndex < xMenuSettings->getCount(); nIndex++ )
    {
        uno::Reference< container::XIndexAccess >   xSubMenu;
        OUString                aCommandURL;
        OUString                aHelpURL;
        OUString                aLabel;
        bool                    bIsUserDefined = TRUE;

        sal_uInt16 nType( dcss::ui::ItemType::DEFAULT );

        bool bItem = GetMenuItemData( xMenuSettings, nIndex,
            aCommandURL, aHelpURL, aLabel, nType, xSubMenu );

        if ( bItem )
        {
            if ( nType == dcss::ui::ItemType::DEFAULT )
            {
                OUString aHelpText;
                if ( aHelpURL.toInt32() != 0 )
                {
                    aHelpText = pHelp->GetHelpText( aHelpURL.toInt32(), NULL );
                }

                uno::Any a;
                try
                {
                    a = m_xCommandToLabelMap->getByName( aCommandURL );
                    bIsUserDefined = FALSE;
                }
                catch ( container::NoSuchElementException& )
                {
                    bIsUserDefined = TRUE;
                }

                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.equals( OUString() ) )
                {
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                        {
                            if ( aPropSeq[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ) )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                    }
                }

                if ( xSubMenu.is() )
                {
                    // popup menu
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        1, aLabel, aHelpText, TRUE );

                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetCommand( aCommandURL );
                    pEntry->SetHelpURL( aHelpURL );

                    pEntries->push_back( pEntry );

                    OUString subMenuTitle( rBaseTitle );

                    if ( subMenuTitle.getLength() != 0 )
                    {
                        subMenuTitle +=
                            OUString::createFromAscii(pMenuSeparatorStr);
                    }
                    else
                    {
                        pEntry->SetMain( TRUE );
                    }

                    subMenuTitle += stripHotKey( aLabel );

                    LoadSubMenus( xSubMenu, subMenuTitle, pEntry );
                }
                else
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        1, aLabel, aHelpText, FALSE );
                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetCommand( aCommandURL );
                    pEntry->SetHelpURL( aHelpURL );
                    pEntries->push_back( pEntry );
                }
            }
            else
            {
                SvxConfigEntry* pEntry = new SvxConfigEntry;
                pEntry->SetUserDefined( bIsUserDefined );
                pEntries->push_back( pEntry );
            }
        }
    }
    return true;
}

bool MenuSaveInData::Apply()
{
    bool result = FALSE;

    if ( IsModified() )
    {
        // Apply new menu bar structure to our settings container
        m_xMenuSettings = uno::Reference< container::XIndexAccess >(
            GetConfigManager()->createSettings(), uno::UNO_QUERY );

        uno::Reference< container::XIndexContainer > xIndexContainer (
            m_xMenuSettings, uno::UNO_QUERY );

        uno::Reference< lang::XSingleComponentFactory > xFactory (
            m_xMenuSettings, uno::UNO_QUERY );

        Apply( pRootEntry, xIndexContainer, xFactory, NULL );

        try
        {
            if ( GetConfigManager()->hasSettings( m_aMenuResourceURL ) )
            {
                GetConfigManager()->replaceSettings(
                    m_aMenuResourceURL, m_xMenuSettings );
            }
            else
            {
                GetConfigManager()->insertSettings(
                    m_aMenuResourceURL, m_xMenuSettings );
            }
        }
        catch ( container::NoSuchElementException& )
        {
            OSL_TRACE("caught container::NoSuchElementException saving settings");
        }
        catch ( com::sun::star::io::IOException& )
        {
            OSL_TRACE("caught IOException saving settings");
        }
        catch ( com::sun::star::uno::Exception& )
        {
            OSL_TRACE("caught some other exception saving settings");
        }

        SetModified( FALSE );

        result = PersistChanges( GetConfigManager() );
    }

    return result;
}

void MenuSaveInData::Apply(
    SvxConfigEntry* pRootEntry,
    uno::Reference< container::XIndexContainer >& rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvLBoxEntry *pParentEntry )
{
    SvxEntries::const_iterator iter = GetEntries()->begin();
    SvxEntries::const_iterator end = GetEntries()->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntryData = *iter;

        uno::Sequence< beans::PropertyValue > aPropValueSeq =
            ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntryData );

        uno::Reference< container::XIndexContainer > xSubMenuBar(
            rFactory->createInstanceWithContext( m_xComponentContext ),
            uno::UNO_QUERY );

        sal_Int32 nIndex = aPropValueSeq.getLength();
        aPropValueSeq.realloc( nIndex + 1 );
        aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
        aPropValueSeq[nIndex].Value <<= xSubMenuBar;
        rMenuBar->insertByIndex(
            rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));
        ApplyMenu( xSubMenuBar, rFactory, pEntryData );
    }
}

void MenuSaveInData::ApplyMenu(
    uno::Reference< container::XIndexContainer >& rMenuBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry* pMenuData )
{
    SvxEntries::const_iterator iter = pMenuData->GetEntries()->begin();
    SvxEntries::const_iterator end = pMenuData->GetEntries()->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->IsPopup() )
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntry );

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( m_xComponentContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;

            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));

            ApplyMenu( xSubMenuBar, rFactory, pEntry );
        }
        else if ( pEntry->IsSeparator() )
        {
            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertSvxConfigEntry( m_xCommandToLabelMap, pEntry );

            if ( pEntry->GetCommand().equalsAscii( ".uno:FormatMenu" ))
            {
                // Special popup menu "Format". It must be an empty popup menu!
                uno::Reference< lang::XSingleComponentFactory >
                    xFactory( rMenuBar, uno::UNO_QUERY );

                uno::Reference< container::XIndexContainer >
                    xSubMenuBar( xFactory->createInstanceWithContext(
                        m_xComponentContext ), uno::UNO_QUERY );

                sal_Int32 nIndex = aPropValueSeq.getLength();
                aPropValueSeq.realloc( nIndex + 1 );
                aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
                aPropValueSeq[nIndex].Value <<= xSubMenuBar;
            }

            rMenuBar->insertByIndex(
                rMenuBar->getCount(), uno::makeAny( aPropValueSeq ));
        }
    }
}

void
MenuSaveInData::Reset()
{
    GetConfigManager()->reset();

    delete pRootEntry;
    pRootEntry = NULL;

    try
    {
        m_xMenuSettings = GetConfigManager()->getSettings(
            m_aMenuResourceURL, sal_False );
    }
    catch ( container::NoSuchElementException& )
    {
        // will use default settings
    }
}

/*
 * SvxMenuEntriesListBox is the listbox in which the menu items for a
 * particular menu are shown. We have a custom listbox because we need
 * to add drag'n'drop support from the Macro Selector and within the
 * listbox
 */
SvxMenuEntriesListBox::SvxMenuEntriesListBox(
    Window* pParent, const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
    , pPage( (SvxConfigPage*) pParent )
    , m_bIsInternalDrag( FALSE )
{
    SetWindowBits(
        GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HIDESELECTION );

    SetSpaceBetweenEntries( 0 );
    SetEntryHeight( 16 );

    SetHighlightRange(); // SetHighlightRange( 1, 0xffff );
    SetSelectionMode(SINGLE_SELECTION);

    SetDragDropMode( SV_DRAGDROP_CTRL_MOVE  |
                     SV_DRAGDROP_APP_COPY   |
                     SV_DRAGDROP_ENABLE_TOP |
                     SV_DRAGDROP_APP_DROP);
}

SvxMenuEntriesListBox::~SvxMenuEntriesListBox()
{
    // do nothing
}

// drag and drop support
DragDropMode SvxMenuEntriesListBox::NotifyStartDrag(
    TransferDataContainer& aTransferDataContainer, SvLBoxEntry* pEntry )
{
    m_bIsInternalDrag = TRUE;
    return GetDragDropMode();
}

void SvxMenuEntriesListBox::DragFinished( sal_Int8 nDropAction )
{
    m_bIsInternalDrag = FALSE;
}

sal_Int8 SvxMenuEntriesListBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if ( m_bIsInternalDrag )
    {
        // internal copy isn't allowed!
        if ( rEvt.mnAction == DND_ACTION_COPY )
            return DND_ACTION_NONE;
        else
            return SvTreeListBox::AcceptDrop( rEvt );
    }

    // Always do COPY instead of MOVE if D&D comes from outside!
    AcceptDropEvent aNewAcceptDropEvent( rEvt );
    aNewAcceptDropEvent.mnAction = DND_ACTION_COPY;
    return SvTreeListBox::AcceptDrop( aNewAcceptDropEvent );
}

BOOL SvxMenuEntriesListBox::NotifyAcceptDrop( SvLBoxEntry* )
{
    return TRUE;
}

BOOL SvxMenuEntriesListBox::NotifyMoving(
    SvLBoxEntry* pTarget, SvLBoxEntry* pSource,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos)
{
    // only try to do a move if we are dragging within the list box
    if ( m_bIsInternalDrag )
    {
        if ( pPage->MoveEntryData( pSource, pTarget ) == TRUE )
        {
            SvTreeListBox::NotifyMoving(
                pTarget, pSource, rpNewParent, rNewChildPos );
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return NotifyCopying( pTarget, pSource, rpNewParent, rNewChildPos );
    }
}

BOOL SvxMenuEntriesListBox::NotifyCopying(
    SvLBoxEntry* pTarget, SvLBoxEntry* pSource,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos)
{
    if ( !m_bIsInternalDrag )
    {
        // if the target is NULL then add function to the start of the list
        pPage->AddFunction( pTarget, pTarget == NULL );

        // AddFunction already adds the listbox entry so return FALSE
        // to stop another listbox entry being added
        return FALSE;
    }

    // Copying is only allowed from external controls, not within the listbox
    return FALSE;
}

/******************************************************************************
 *
 * SvxConfigPage is the abstract base class on which the Menu and Toolbar
 * configuration tabpages are based. It includes methods which are common to
 * both tabpages to add, delete, move and rename items etc.
 *
 *****************************************************************************/
SvxConfigPage::SvxConfigPage(
    Window *pParent, const SfxItemSet& rSet )
    :
    SfxTabPage( pParent, ResId( RID_SVXPAGE_MENUS, DIALOG_MGR()), rSet ),
    aTopLevelSeparator( this, ResId( GRP_MENUS ) ),
    aTopLevelLabel( this, ResId( FT_MENUS ) ),
    aTopLevelListBox( this, ResId( LB_MENUS ) ),
    aNewTopLevelButton( this, ResId( BTN_NEW ) ),
    aModifyTopLevelButton( this, ResId( BTN_CHANGE ) ),
    aContentsSeparator( this, ResId( GRP_MENU_SEPARATOR ) ),
    aContentsLabel( this, ResId( GRP_MENU_ENTRIES ) ),
    aContentsListBox( 0 ),
    aAddCommandsButton( this, ResId( BTN_ADD_COMMANDS ) ),
    aModifyCommandButton( this, ResId( BTN_CHANGE_ENTRY ) ),
    aMoveUpButton( this, ResId( BTN_UP ) ),
    aMoveDownButton( this, ResId( BTN_DOWN ) ),
    aSaveInText( this, ResId( TXT_SAVEIN ) ),
    aSaveInListBox( this, ResId( LB_SAVEIN ) ),
    aDescriptionLine( this, ResId( GRP_DESCRIPTION ) ),
    aDescriptionText( this, ResId( TXT_DESCRIPTION ) ),
    pCurrentSaveInData( 0 ),
    pSelectorDlg( 0 ),
    bInitialised( FALSE )
{
    FreeResource();
}

SvxConfigPage::~SvxConfigPage()
{
}

void SvxConfigPage::Reset( const SfxItemSet& )
{
    // If we haven't initialised our XMultiServiceFactory reference
    // then Reset is being called at the opening of the dialog.
    //
    // Load menu configuration data for the module of the currently
    // selected document, for the currently selected document, and for
    // all other open documents of the same module type
    if ( !bInitialised )
    {
        USHORT nPos = 0;
        uno::Reference < dcss::ui::XUIConfigurationManager > xCfgMgr;

        uno::Reference< lang::XMultiServiceFactory > xServiceManager(
            ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );

        SfxViewFrame* pViewFrame = GetTabDialog()->GetViewFrame();

        uno::Reference < frame::XFrame > xFrame =
            uno::Reference< frame::XFrame >(
                pViewFrame->GetFrame()->GetFrameInterface());

        uno::Reference< dcss::frame::XModuleManager > xModuleManager(
            xServiceManager->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "drafts.com.sun.star.frame.ModuleManager" ) ) ),
            uno::UNO_QUERY );

        OUString aModuleId = xModuleManager->identify( xFrame );

        // replace %MODULENAME in the label with the correct module name
        OUString aModuleName = GetModuleName( aModuleId );

        OUString title = aTopLevelSeparator.GetText();

        OUString aSearchString = OUString::createFromAscii( "%MODULENAME" );
        sal_Int32 index = title.indexOf( aSearchString );

        if ( index != -1 )
        {
            title = title.replaceAt(
                index, aSearchString.getLength(), aModuleName );
            aTopLevelSeparator.SetText( title );
        }

        uno::Reference< dcss::ui::XModuleUIConfigurationManagerSupplier >
            xModuleCfgSupplier( xServiceManager->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
            "drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ))),
            uno::UNO_QUERY );

        // Set up data for module specific menus
        SaveInData* pModuleData = NULL;

        try
        {
            xCfgMgr =
                xModuleCfgSupplier->getUIConfigurationManager( aModuleId );

            pModuleData = CreateSaveInData( xCfgMgr, aModuleId, FALSE );
        }
        catch ( container::NoSuchElementException& )
        {
        }

        if ( pModuleData != NULL )
        {
            OUString label = OUString( SFX_APP()->GetName() );
            label += OUString::createFromAscii( " " );
            label += aModuleName;

            nPos = aSaveInListBox.InsertEntry( label );
            aSaveInListBox.SetEntryData( nPos, pModuleData );
        }

        // try to retrieve the document based ui configuration manager
        uno::Reference< frame::XController > xController = xFrame->getController();
        if ( xController.is() )
        {
            uno::Reference< frame::XModel > xModel( xController->getModel() );
            if ( xModel.is() )
            {
                uno::Reference< dcss::ui::XUIConfigurationManagerSupplier > xCfgSupplier(
                    xModel, uno::UNO_QUERY );

                if ( xCfgSupplier.is() )
                    xCfgMgr = xCfgSupplier->getUIConfigurationManager();
            }
        }

        SaveInData* pDocData = NULL;
        if ( xCfgMgr.is() )
        {
            pDocData = CreateSaveInData( xCfgMgr, aModuleId, TRUE );

            if ( !pDocData->IsReadOnly() )
            {
                nPos = aSaveInListBox.InsertEntry(
                    pViewFrame->GetObjectShell()->GetTitle() );

                aSaveInListBox.SetEntryData( nPos, pDocData );
            }
        }

        // if an item to select has been passed in (eg. the ResourceURL for a
        // toolbar) then try to select the SaveInData entry that has that item
        bool bURLToSelectFound = FALSE;
        if ( m_aURLToSelect.getLength() != 0 )
        {
            if ( pDocData->HasURL( m_aURLToSelect ) )
            {
                aSaveInListBox.SelectEntryPos( nPos, TRUE );
                pCurrentSaveInData = pDocData;
                bURLToSelectFound = TRUE;
            }
            else if ( pModuleData->HasURL( m_aURLToSelect ) )
            {
                aSaveInListBox.SelectEntryPos( 0, TRUE );
                pCurrentSaveInData = pModuleData;
                bURLToSelectFound = TRUE;
            }
        }

        if ( bURLToSelectFound == FALSE )
        {
            // if the document has menu configuration settings select it
            // it the SaveIn listbox, otherwise select the module data
            if ( pDocData && pDocData->HasSettings() )
            {
                aSaveInListBox.SelectEntryPos( nPos, TRUE );
                pCurrentSaveInData = pDocData;
            }
            else
            {
                aSaveInListBox.SelectEntryPos( 0, TRUE );
                pCurrentSaveInData = pModuleData;
            }
        }

        // Load menu configuration for other open documents which have
        // same module type
        SfxViewFrame* cur = SfxViewFrame::GetFirst();
        while ( cur != NULL )
        {
            SaveInData* pData = NULL;

            if ( cur != pViewFrame )
            {
                uno::Reference < frame::XFrame > xf = uno::Reference< frame::XFrame >(
                    cur->GetFrame()->GetFrameInterface());

                if ( aModuleId.equals( xModuleManager->identify( xf ) ) )
                {
                    // try to get the document based ui configuration manager
                    uno::Reference< frame::XController > xController = xf->getController();
                    if ( xController.is() )
                    {
                        uno::Reference< frame::XModel > xModel( xController->getModel() );
                        if ( xModel.is() )
                        {
                            uno::Reference< dcss::ui::XUIConfigurationManagerSupplier >
                                xCfgSupplier( xModel, uno::UNO_QUERY );

                            if ( xCfgSupplier.is() )
                            {
                                xCfgMgr =
                                    xCfgSupplier->getUIConfigurationManager();
                            }
                        }
                    }

                    if ( xCfgMgr.is() )
                    {
                        pData = CreateSaveInData( xCfgMgr, aModuleId, TRUE );

                        if ( pData && !pData->IsReadOnly() )
                        {
                            nPos = aSaveInListBox.InsertEntry(
                                cur->GetObjectShell()->GetTitle() );

                            aSaveInListBox.SetEntryData( nPos, pData );
                        }
                    }
                }
            }

            cur = SfxViewFrame::GetNext( *cur );
        }

        aSaveInListBox.SetSelectHdl(
            LINK( this, SvxConfigPage, SelectSaveInLocation ) );

        bInitialised = TRUE;

        Init();
    }
    else
    {
        QueryBox qbox( this, ResId( QBX_CONFIRM_RESET, DIALOG_MGR() ) );

        if ( qbox.Execute() == RET_YES )
        {
            // Reset menu configuration for currently selected SaveInData
            GetSaveInData()->Reset();

            Init();
        }
    }
}

BOOL SvxConfigPage::FillItemSet( SfxItemSet& )
{
    bool result = FALSE;

    for ( sal_Int32 i = 0 ; i < aSaveInListBox.GetEntryCount(); i++ )
    {
        SaveInData* pData =
            (SaveInData*) aSaveInListBox.GetEntryData( i );

        result = pData->Apply();
    }
    return result;
}

void SvxConfigPage::PositionContentsListBox()
{
    if ( aContentsListBox == NULL )
    {
        return;
    }

    Point p, newp;
    Size s, news;
    long x, y, width, height;

    // x and width is same as aTopLevelListBox
    x = aTopLevelListBox.GetPosPixel().X();
    width = aTopLevelListBox.GetSizePixel().Width();

    // y is same as aAddCommandsButton
    y = aAddCommandsButton.GetPosPixel().Y();

    // get gap between aAddCommandsButton and aContentsSeparator
    p = aContentsSeparator.GetPosPixel();
    s = aContentsSeparator.GetSizePixel();
    long gap = y - ( p.Y() + s.Height() );

    height = aSaveInListBox.GetPosPixel().Y() - y - gap;

    aContentsListBox->SetPosPixel( Point( x, y ) );
    aContentsListBox->SetSizePixel( Size( width, height ) );
}

IMPL_LINK( SvxConfigPage, SelectSaveInLocation, ListBox *, pBox )
{
    pCurrentSaveInData = (SaveInData*) aSaveInListBox.GetEntryData(
            aSaveInListBox.GetSelectEntryPos());

    Init();
    return 1;
}

void SvxConfigPage::ReloadTopLevelListBox( SvxConfigEntry* pToSelect )
{
    USHORT nSelectionPos = aTopLevelListBox.GetSelectEntryPos();
    aTopLevelListBox.Clear();

    SvxEntries::const_iterator iter =
        GetSaveInData()->GetEntries()->begin();

    SvxEntries::const_iterator end =
        GetSaveInData()->GetEntries()->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntryData = *iter;

        USHORT nPos = aTopLevelListBox.InsertEntry(
            stripHotKey( pEntryData->GetName() ) );

        aTopLevelListBox.SetEntryData( nPos, pEntryData );

        if ( pEntryData == pToSelect )
        {
            nSelectionPos = nPos;
        }

        AddSubMenusToUI( stripHotKey( pEntryData->GetName() ), pEntryData );
    }

    nSelectionPos = nSelectionPos < aTopLevelListBox.GetEntryCount() ?
        nSelectionPos : aTopLevelListBox.GetEntryCount() - 1;

    aTopLevelListBox.SelectEntryPos( nSelectionPos, TRUE );
    aTopLevelListBox.GetSelectHdl().Call( this );
}

void SvxConfigPage::AddSubMenusToUI(
    const String& rBaseTitle, SvxConfigEntry* pParentData )
{
    SvxEntries::const_iterator iter = pParentData->GetEntries()->begin();
    SvxEntries::const_iterator end = pParentData->GetEntries()->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntryData = *iter;

        if ( pEntryData->IsPopup() )
        {
            OUString subMenuTitle( rBaseTitle );
            subMenuTitle += OUString::createFromAscii( pMenuSeparatorStr );
            subMenuTitle += stripHotKey( pEntryData->GetName() );

            USHORT nPos = aTopLevelListBox.InsertEntry( subMenuTitle );
            aTopLevelListBox.SetEntryData( nPos, pEntryData );

            AddSubMenusToUI( subMenuTitle, pEntryData );
        }
    }
}

SvxEntries* SvxConfigPage::FindParentForChild(
    SvxEntries* pRootEntries, SvxConfigEntry* pChildData )
{
    SvxEntries::const_iterator iter = pRootEntries->begin();
    SvxEntries::const_iterator end = pRootEntries->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntryData = *iter;

        if ( pEntryData == pChildData )
        {
            return pRootEntries;
        }
        else if ( pEntryData->IsPopup() )
        {
            SvxEntries* result =
                FindParentForChild( pEntryData->GetEntries(), pChildData );

            if ( result != NULL )
            {
                return result;
            }
        }
    }
    return NULL;
}

SvLBoxEntry* SvxConfigPage::AddFunction(
    SvLBoxEntry* pTarget, bool bFront, bool bAllowDuplicates )
{
    USHORT nId = pSelectorDlg->GetSelectedId();
    String aDisplayName = pSelectorDlg->GetSelectedDisplayName();
    String aHelpText = pSelectorDlg->GetSelectedHelpText();

    String aCmd;

    // get the slot - even if it is disabled on the dispatcher
    const SfxSlot* pSlot =
        SFX_APP()->GetSlotPool( GetTabDialog()->GetViewFrame() ).GetSlot( nId );

    // create the dispatch name from the slot ID
    String aName( pSlot && pSlot->pUnoName ?
        String::CreateFromAscii(pSlot->GetUnoName()) : String() );

    if ( aName.Len() )
    {
        aCmd = DEFINE_CONST_UNICODE(".uno:");
        aCmd += aName;
    }

    SvxConfigEntry* pNewEntryData =
        new SvxConfigEntry( nId, aDisplayName, aHelpText, FALSE );
    pNewEntryData->SetUserDefined( TRUE );

    if ( aCmd.Len() )
        pNewEntryData->SetCommand( aCmd );

    pNewEntryData->SetHelpURL( String::CreateFromInt32( nId ));

    // check that this function is not already in the menu
    SvxConfigEntry* pParent = GetTopLevelSelection();

    SvxEntries::const_iterator iter = pParent->GetEntries()->begin();
    SvxEntries::const_iterator end = pParent->GetEntries()->end();

    if ( !bAllowDuplicates )
    {
        while ( iter != end )
        {
            SvxConfigEntry *pCurEntry = *iter;

            if ( pCurEntry->GetCommand() == pNewEntryData->GetCommand() )
            {
                // asynchronous error message, because of MsgBoxes
                PostUserEvent(
                    LINK( this, SvxConfigPage, AsyncInfoMsg ), new String() );
                return NULL;
            }

            iter++;
        }
    }

    return InsertEntry( pNewEntryData, pTarget, bFront );
}

SvLBoxEntry* SvxConfigPage::InsertEntry(
    SvxConfigEntry* pNewEntryData,
    SvLBoxEntry* pTarget,
    bool bFront )
{
    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();

    SvLBoxEntry* pNewEntry = NULL;
    SvLBoxEntry* pCurEntry =
        pTarget != NULL ? pTarget : aContentsListBox->GetCurEntry();

    if ( bFront )
    {
        pEntries->insert( pEntries->begin(), pNewEntryData );
        pNewEntry = InsertEntryIntoUI( pNewEntryData, 0 );
    }
    else if ( pCurEntry == NULL || pCurEntry == aContentsListBox->Last() )
    {
        pEntries->push_back( pNewEntryData );
        pNewEntry = InsertEntryIntoUI( pNewEntryData );
    }
    else
    {
        SvxConfigEntry* pEntryData =
            (SvxConfigEntry*) pCurEntry->GetUserData();

        SvxEntries::iterator iter = pEntries->begin();
        SvxEntries::const_iterator end = pEntries->end();

        // Advance the iterator to the data for currently selected entry
        USHORT nPos = 0;
        while (*iter != pEntryData && ++iter != end)
        {
            nPos++;
        }

        // Now step past it to the entry after the currently selected one
        iter++;
        nPos++;

        // Now add the new entry to the UI and to the parent's list
        if ( iter != end )
        {
            pEntries->insert( iter, pNewEntryData );
            pNewEntry = InsertEntryIntoUI( pNewEntryData, nPos );
        }
    }

    if ( pNewEntry != NULL )
    {
        aContentsListBox->Select( pNewEntry );
        aContentsListBox->MakeVisible( pNewEntry );

        GetSaveInData()->SetModified( TRUE );
    }

    return pNewEntry;
}

SvLBoxEntry* SvxConfigPage::InsertEntryIntoUI(
    SvxConfigEntry* pNewEntryData, USHORT nPos )
{
    SvLBoxEntry* pNewEntry = NULL;

    if (pNewEntryData->IsBinding())
    {
        Image aImage = GetSaveInData()->GetImage(
            pNewEntryData->GetCommand());

        if ( !!aImage )
        {
            pNewEntry = aContentsListBox->InsertEntry(
                stripHotKey( pNewEntryData->GetName() ),
                    aImage, aImage, 0, FALSE, nPos, pNewEntryData );
        }
        else
        {
            pNewEntry = aContentsListBox->InsertEntry(
                stripHotKey( pNewEntryData->GetName() ),
                    0, FALSE, nPos, pNewEntryData );
        }
    }
    else if (pNewEntryData->IsSeparator())
    {
        pNewEntry = aContentsListBox->InsertEntry(
            String::CreateFromAscii( pSeparatorStr ),
                0, FALSE, nPos, pNewEntryData );
    }
    else
    {
        pNewEntry = aContentsListBox->InsertEntry(
            stripHotKey( pNewEntryData->GetName() ),
                0, FALSE, nPos, pNewEntryData );
    }

    return pNewEntry;
}

IMPL_LINK( SvxConfigPage, AsyncInfoMsg, String*, pMsg )
{
    // Asynchronous msg because of D&D
    // Window * pWin = Window::GetParent();
    // InfoBox( pWin, *pMsg ).Execute();
    InfoBox( this, ResId( IBX_MNUCFG_ALREADY_INCLUDED ) ).Execute();
    delete pMsg;

    return 0;
}

IMPL_LINK( SvxConfigPage, MoveHdl, Button *, pButton )
{
    MoveEntry( pButton );

    return 0;
}

void SvxConfigPage::MoveEntry( Button *pButton )
{
    SvLBoxEntry *pSourceEntry = aContentsListBox->FirstSelected();
    SvLBoxEntry *pTargetEntry = NULL;

    if ( !pSourceEntry )
    {
        return;
    }

    if ( pButton == &aMoveDownButton )
    {
        pTargetEntry = aContentsListBox->NextSibling( pSourceEntry );
    }
    else if ( pButton == &aMoveUpButton )
    {
        // Move Up is just a Move Down with the source and target reversed
        pTargetEntry = pSourceEntry;
        pSourceEntry = aContentsListBox->PrevSibling( pTargetEntry );
    }

    if ( MoveEntryData( pSourceEntry, pTargetEntry ) )
    {
        aContentsListBox->GetModel()->Move( pSourceEntry, pTargetEntry );
        aContentsListBox->MakeVisible( pSourceEntry );

        UpdateButtonStates();
    }
}

bool SvxConfigPage::MoveEntryData(
    SvLBoxEntry* pSourceEntry, SvLBoxEntry* pTargetEntry )
{
    if ( pSourceEntry == NULL )
    {
        return FALSE;
    }

    // Grab the entries list for the currently selected menu
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();

    SvxConfigEntry* pSourceData =
        (SvxConfigEntry*) pSourceEntry->GetUserData();

    if ( pTargetEntry == NULL )
    {
        RemoveEntry( pEntries, pSourceData );
        pEntries->insert(
            pEntries->begin(), pSourceData );

        GetSaveInData()->SetModified( TRUE );

        return TRUE;
    }
    else
    {
        SvxConfigEntry* pTargetData =
            (SvxConfigEntry*) pTargetEntry->GetUserData();

        if ( pSourceData != NULL && pTargetData != NULL )
        {
            // remove the source entry from our list
            RemoveEntry( pEntries, pSourceData );

            SvxEntries::iterator iter = pEntries->begin();
            SvxEntries::const_iterator end = pEntries->end();

            // advance the iterator to the position of the target entry
            while (*iter != pTargetData && ++iter != end);

            // insert the source entry at the position after the target
            pEntries->insert( ++iter, pSourceData );

            GetSaveInData()->SetModified( TRUE );

            return TRUE;
        }
    }

    return FALSE;
}

SvxMenuConfigPage::SvxMenuConfigPage(
    Window *pParent, const SfxItemSet& rSet ) :

    SvxConfigPage( pParent, rSet )
{
    aContentsListBox = new SvxMenuEntriesListBox( this, ResId( BOX_ENTRIES ) );
    PositionContentsListBox();

    aTopLevelListBox.SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenu ) );

    aContentsListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    aTopLevelSeparator.SetText(
        ResId(RID_SVXSTR_PRODUCTNAME_TOOLBARS, DIALOG_MGR() ) );

    aMoveUpButton.SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
    aMoveDownButton.SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );

    aNewTopLevelButton.SetClickHdl  (
        LINK( this, SvxMenuConfigPage, NewMenuHdl ) );

    aAddCommandsButton.SetClickHdl  (
        LINK( this, SvxMenuConfigPage, AddCommandsHdl ) );

    PopupMenu* pMenu = new PopupMenu( ResId( MODIFY_MENU, DIALOG_MGR() ) );
    pMenu->SetMenuFlags(
        pMenu->GetMenuFlags() | MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );

    aModifyTopLevelButton.SetPopupMenu( pMenu );
    aModifyTopLevelButton.SetSelectHdl(
        LINK( this, SvxMenuConfigPage, MenuSelectHdl ) );

    PopupMenu* pEntry = new PopupMenu( ResId( MODIFY_ENTRY, DIALOG_MGR() ) );
    pEntry->SetMenuFlags(
        pEntry->GetMenuFlags() | MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );

    aModifyCommandButton.SetPopupMenu( pEntry );
    aModifyCommandButton.SetSelectHdl(
        LINK( this, SvxMenuConfigPage, EntrySelectHdl ) );
}

// Populates the Menu combo box
void SvxMenuConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    aTopLevelListBox.Clear();
    aContentsListBox->Clear();

    ReloadTopLevelListBox();

    aTopLevelListBox.SelectEntryPos(0, TRUE);
    aTopLevelListBox.GetSelectHdl().Call(this);
}

SvxMenuConfigPage::~SvxMenuConfigPage()
{
    for ( sal_Int32 i = 0 ; i < aSaveInListBox.GetEntryCount(); i++ )
    {
        MenuSaveInData* pData =
            (MenuSaveInData*) aSaveInListBox.GetEntryData( i );

        delete pData;
    }

    if ( pSelectorDlg != NULL )
    {
        delete pSelectorDlg;
    }

    delete aContentsListBox;
}

IMPL_LINK( SvxMenuConfigPage, SelectMenuEntry, Control *, pBox )
{
    UpdateButtonStates();

    return 1;
}

void SvxMenuConfigPage::UpdateButtonStates()
{
    PopupMenu* pPopup = aModifyCommandButton.GetPopupMenu();

    // Disable Up and Down buttons depending on current selection
    SvLBoxEntry* selection = aContentsListBox->GetCurEntry();

    if ( aContentsListBox->GetEntryCount() == 0 || selection == NULL )
    {
        aMoveUpButton.Enable( FALSE );
        aMoveDownButton.Enable( FALSE );

        pPopup->EnableItem( ID_BEGIN_GROUP, TRUE );
        pPopup->EnableItem( ID_RENAME, FALSE );
        pPopup->EnableItem( ID_DELETE, FALSE );

        aDescriptionText.SetText( String() );

        return;
    }

    SvLBoxEntry* first = aContentsListBox->First();
    SvLBoxEntry* last = aContentsListBox->Last();

    aMoveUpButton.Enable( selection != first );
    aMoveDownButton.Enable( selection != last );

    SvxConfigEntry* pEntryData =
        (SvxConfigEntry*) selection->GetUserData();

    if ( pEntryData->IsSeparator() )
    {
        pPopup->EnableItem( ID_DELETE, TRUE );
        pPopup->EnableItem( ID_BEGIN_GROUP, FALSE );
        pPopup->EnableItem( ID_RENAME, FALSE );

        aDescriptionText.SetText( String() );
    }
    else
    {
        pPopup->EnableItem( ID_BEGIN_GROUP, TRUE );
        pPopup->EnableItem( ID_DELETE, TRUE );
        pPopup->EnableItem( ID_RENAME, TRUE );

        aDescriptionText.SetText( pEntryData->GetHelpText() );
    }
}

IMPL_LINK( SvxMenuConfigPage, SelectMenu, ListBox *, pBox )
{
    aContentsListBox->Clear();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    PopupMenu* pPopup = aModifyTopLevelButton.GetPopupMenu();
    pPopup->EnableItem( ID_DELETE, pMenuData->IsDeletable() );
    pPopup->EnableItem( ID_RENAME, pMenuData->IsRenamable() );
    pPopup->EnableItem( ID_MOVE, pMenuData->IsMovable() );

    SvxEntries* pEntries = pMenuData->GetEntries();
    SvxEntries::const_iterator iter = pEntries->begin();

    for ( ; iter != pEntries->end(); iter++ )
    {
        SvxConfigEntry* pEntry = *iter;
        InsertEntryIntoUI( pEntry );
    }

    UpdateButtonStates();

    return 0;
}

IMPL_LINK( SvxMenuConfigPage, MenuSelectHdl, MenuButton *, pButton )
{
    switch( pButton->GetCurItemId() )
    {
        case ID_DELETE:
        {
            SvxConfigEntry* pMenuData = GetTopLevelSelection();

            SvxEntries* pParentEntries =
                FindParentForChild( GetSaveInData()->GetEntries(), pMenuData );

            RemoveEntry( pParentEntries, pMenuData );
            delete pMenuData;

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified( TRUE );

            break;
        }
        case ID_RENAME:
        {
            SvxConfigEntry* pMenuData = GetTopLevelSelection();

            String aNewName( stripHotKey( pMenuData->GetName() ) );
            String aDesc( RTL_CONSTASCII_USTRINGPARAM( "Enter New Name" ) );

            SvxAbstractDialogFactory* pFact =
                SvxAbstractDialogFactory::Create();

            AbstractSvxNameDialog* pNameDialog =
                pFact->CreateSvxNameDialog(
                    0, aNewName, aDesc, ResId(RID_SVXDLG_NAME) );

            bool ret = pNameDialog->Execute();

            if ( ret == RET_OK ) {
                pNameDialog->GetName( aNewName );
                pMenuData->SetName( aNewName );

                ReloadTopLevelListBox();

                GetSaveInData()->SetModified( TRUE );
            }

            break;
        }
        case ID_MOVE:
        {
            SvxConfigEntry* pMenuData = GetTopLevelSelection();

            SvxMainMenuOrganizerDialog* pDialog =
                new SvxMainMenuOrganizerDialog( this,
                    GetSaveInData()->GetEntries(), pMenuData );

            bool ret = pDialog->Execute();

            if ( ret == RET_OK )
            {
                GetSaveInData()->SetEntries( pDialog->GetEntries() );

                ReloadTopLevelListBox( pDialog->GetSelectedEntry() );

                GetSaveInData()->SetModified( TRUE );
            }

            delete pDialog;

            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}

IMPL_LINK( SvxMenuConfigPage, EntrySelectHdl, MenuButton *, pButton )
{
    switch( pButton->GetCurItemId() )
    {
        case ID_ADD_SUBMENU:
        {
            String aNewName( RTL_CONSTASCII_USTRINGPARAM( "" ) );
            String aDesc( RTL_CONSTASCII_USTRINGPARAM( "Enter New Name" ) );

            SvxAbstractDialogFactory* pFact =
                SvxAbstractDialogFactory::Create();

            AbstractSvxNameDialog* pNameDialog =
                pFact->CreateSvxNameDialog(
                    0, aNewName, aDesc, ResId(RID_SVXDLG_NAME) );

            bool ret = pNameDialog->Execute();

            if ( ret == RET_OK ) {
                pNameDialog->GetName(aNewName);

                SvxConfigEntry* pNewEntryData =
                    new SvxConfigEntry( 1, aNewName, String(), TRUE );
                pNewEntryData->SetUserDefined( TRUE );
                pNewEntryData->SetCommand( aNewName );

                InsertEntry( pNewEntryData );

                ReloadTopLevelListBox();

                GetSaveInData()->SetModified( TRUE );
            }

            delete pNameDialog;

            break;
        }
        case ID_BEGIN_GROUP:
        {
            SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
            pNewEntryData->SetUserDefined( TRUE );
            InsertEntry( pNewEntryData );

            break;
        }
        case ID_DELETE:
        {
            SvLBoxEntry *pActEntry = aContentsListBox->FirstSelected();

            if ( pActEntry != NULL )
            {
                // get currently selected menu entry
                SvxConfigEntry* pMenuEntry =
                    (SvxConfigEntry*) pActEntry->GetUserData();

                // get currently selected menu
                SvxConfigEntry* pMenu = GetTopLevelSelection();

                // remove menu entry from the list for this menu
                RemoveEntry( pMenu->GetEntries(), pMenuEntry );

                // remove menu entry from UI
                aContentsListBox->GetModel()->Remove( pActEntry );

                // if this is a submenu entry, redraw the menus list box
                if ( pMenuEntry->IsPopup() )
                {
                    ReloadTopLevelListBox();
                }

                // delete data for menu entry
                delete pMenuEntry;

                GetSaveInData()->SetModified( TRUE );
            }

            break;
        }
        case ID_RENAME:
        {
            SvLBoxEntry* pActEntry = aContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                (SvxConfigEntry*) pActEntry->GetUserData();

            String aNewName( stripHotKey( pEntry->GetName() ) );
            String aDesc( RTL_CONSTASCII_USTRINGPARAM( "Enter New Name" ) );

            SvxAbstractDialogFactory* pFact =
                SvxAbstractDialogFactory::Create();

            AbstractSvxNameDialog* pNameDialog =
                pFact->CreateSvxNameDialog(
                    0, aNewName, aDesc, ResId(RID_SVXDLG_NAME) );

            bool ret = pNameDialog->Execute();

            if ( ret == RET_OK ) {
                pNameDialog->GetName(aNewName);

                pEntry->SetName( aNewName );
                aContentsListBox->SetEntryText( pActEntry, aNewName );

                GetSaveInData()->SetModified( TRUE );
            }

            delete pNameDialog;

            break;
        }
        default:
        {
            return FALSE;
        }
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }

    return TRUE;
}

IMPL_LINK( SvxMenuConfigPage, AddFunctionHdl,
    SvxScriptSelectorDialog *, pDialog )
{
    AddFunction();

    return 0;
}

IMPL_LINK( SvxMenuConfigPage, NewMenuHdl, Button *, pButton )
{
    SvxMainMenuOrganizerDialog* pDialog =
        new SvxMainMenuOrganizerDialog( 0,
            GetSaveInData()->GetEntries(), NULL, TRUE );

    bool ret = pDialog->Execute();

    if ( ret == RET_OK )
    {
        GetSaveInData()->SetEntries( pDialog->GetEntries() );

        ReloadTopLevelListBox( pDialog->GetSelectedEntry() );

        GetSaveInData()->SetModified( TRUE );
    }

    delete pDialog;

    return 0;
}

IMPL_LINK( SvxMenuConfigPage, AddCommandsHdl, Button *, pButton )
{
    if ( pSelectorDlg == NULL )
    {
        // Create Script Selector which shows slot commands
        pSelectorDlg = new SvxScriptSelectorDialog( this, TRUE );

        // Position the Script Selector over the Add button so it is
        // beside the menu contents list and does not obscure it
        pSelectorDlg->SetPosPixel( aAddCommandsButton.GetPosPixel() );

        pSelectorDlg->SetAddHdl(
            LINK( this, SvxMenuConfigPage, AddFunctionHdl ) );
    }

    pSelectorDlg->SetImageProvider(
        static_cast< ImageProvider* >( GetSaveInData() ) );

    pSelectorDlg->Show();
    return 1;
}

SaveInData* SvxMenuConfigPage::CreateSaveInData(
    const uno::Reference< dcss::ui::XUIConfigurationManager >& xCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    return static_cast< SaveInData* >(
        new MenuSaveInData( xCfgMgr, aModuleId, bDocConfig ));
}

SvxMainMenuOrganizerDialog::SvxMainMenuOrganizerDialog(
    Window* pParent, SvxEntries* entries,
    SvxConfigEntry* selection, bool bCreateMenu )
    :
    ModalDialog( pParent, ResId( MD_MENU_ORGANISER, DIALOG_MGR() ) ),
    aMenuNameText( this, ResId( TXT_MENU_NAME ) ),
    aMenuNameEdit( this, ResId( EDIT_MENU_NAME ) ),
    aMenuListText( this, ResId( TXT_MENU ) ),
    aMenuListBox( this, ResId( BOX_MAIN_MENUS ) ),
    aMoveUpButton( this, ResId( BTN_MENU_UP ) ),
    aMoveDownButton( this, ResId( BTN_MENU_DOWN ) ),
    aOKButton( this, ResId( BTN_MENU_ADD ) ),
    aCloseButton( this, ResId( BTN_MENU_CLOSE ) ),
    aHelpButton( this, ResId( BTN_MENU_HELP ) ),
    bModified( FALSE )
{
    FreeResource();

    // Copy the entries list passed in
    if ( entries != NULL )
    {
        SvxConfigEntry* pEntry;
        SvLBoxEntry* pLBEntry;

        pEntries = new SvxEntries();
        SvxEntries::const_iterator iter = entries->begin();

        while ( iter != entries->end() )
        {
            pEntry = *iter;
            pLBEntry =
                aMenuListBox.InsertEntry( stripHotKey( pEntry->GetName() ) );
            pLBEntry->SetUserData( pEntry );
            pEntries->push_back( pEntry );

            if ( pEntry == selection )
            {
                aMenuListBox.Select( pLBEntry );
            }
            iter++;
        }
    }

    if ( bCreateMenu )
    {
        // Generate custom name for new menu
        String prefix =
            String( ResId ( RID_SVXSTR_NEW_MENU, DIALOG_MGR() ) );

        OUString newname = generateCustomName( prefix, entries );

        SvxConfigEntry* pNewEntryData =
            new SvxConfigEntry( 1, newname, String(), TRUE );
        pNewEntryData->SetUserDefined( TRUE );
        pNewEntryData->SetMain( TRUE );
        pNewEntryData->SetCommand( generateCustomMenuName( pEntries ) );

        pNewMenuEntry =
            aMenuListBox.InsertEntry( stripHotKey( pNewEntryData->GetName() ) );
        aMenuListBox.Select( pNewMenuEntry );

        pNewMenuEntry->SetUserData( pNewEntryData );

        pEntries->push_back( pNewEntryData );

        aMenuNameEdit.SetText( newname );
        aMenuNameEdit.SetModifyHdl(
            LINK( this, SvxMainMenuOrganizerDialog, ModifyHdl ) );
    }
    else
    {
        Point p, newp;
        Size s, news;

        // get offset to bottom of name textfield from top of dialog
        p = aMenuNameEdit.GetPosPixel();
        s = aMenuNameEdit.GetSizePixel();
        long offset = p.Y() + s.Height();

        // reposition menu list and label
        aMenuListText.SetPosPixel( aMenuNameText.GetPosPixel() );
        aMenuListBox.SetPosPixel( aMenuNameEdit.GetPosPixel() );

        // reposition up and down buttons
        p = aMoveUpButton.GetPosPixel();
        newp = Point( p.X(), p.Y() - offset );
        aMoveUpButton.SetPosPixel( newp );

        p = aMoveDownButton.GetPosPixel();
        newp = Point( p.X(), p.Y() - offset );
        aMoveDownButton.SetPosPixel( newp );

        // change size of dialog
        s = GetSizePixel();
        news = Size( s.Width(), s.Height() - offset );
        SetSizePixel( news );

        // hide name label and textfield
        aMenuNameText.Hide();
        aMenuNameEdit.Hide();

        // change the title
        SetText( ResId( RID_SVXSTR_MOVE_MENU, DIALOG_MGR() ) );
    }

    aMenuListBox.SetSelectHdl(
        LINK( this, SvxMainMenuOrganizerDialog, SelectHdl ) );

    aMoveUpButton.SetClickHdl (
        LINK( this, SvxMainMenuOrganizerDialog, MoveHdl) );
    aMoveDownButton.SetClickHdl (
        LINK( this, SvxMainMenuOrganizerDialog, MoveHdl) );
}

IMPL_LINK(SvxMainMenuOrganizerDialog, ModifyHdl, Edit*, pEdit)
{
    // if the Edit control is empty do not change the name
    if ( aMenuNameEdit.GetText().Equals( String() ) )
    {
        return 0;
    }

    SvxConfigEntry* pNewEntryData =
        (SvxConfigEntry*) pNewMenuEntry->GetUserData();

    pNewEntryData->SetName( aMenuNameEdit.GetText() );

    aMenuListBox.SetEntryText( pNewMenuEntry, pNewEntryData->GetName() );

    return 0;
}

SvxMainMenuOrganizerDialog::~SvxMainMenuOrganizerDialog()
{
}

IMPL_LINK( SvxMainMenuOrganizerDialog, SelectHdl, Control*, pCtrl )
{
    UpdateButtonStates();
    return 1;
}

void SvxMainMenuOrganizerDialog::UpdateButtonStates()
{
    // Disable Up and Down buttons depending on current selection
    SvLBoxEntry* selection = aMenuListBox.GetCurEntry();
    SvLBoxEntry* first = aMenuListBox.First();
    SvLBoxEntry* last = aMenuListBox.Last();

    aMoveUpButton.Enable( selection != first );
    aMoveDownButton.Enable( selection != last );
}

IMPL_LINK( SvxMainMenuOrganizerDialog, MoveHdl, Button *, pButton )
{
    SvLBoxEntry *pSourceEntry = aMenuListBox.FirstSelected();
    SvLBoxEntry *pTargetEntry = NULL;

    if ( !pSourceEntry )
    {
        return 0;
    }

    if ( pButton == &aMoveDownButton )
    {
        pTargetEntry = aMenuListBox.NextSibling( pSourceEntry );
    }
    else if ( pButton == &aMoveUpButton )
    {
        // Move Up is just a Move Down with the source and target reversed
        pTargetEntry = pSourceEntry;
        pSourceEntry = aMenuListBox.PrevSibling( pTargetEntry );
    }

    if ( pSourceEntry != NULL && pTargetEntry != NULL )
    {
        SvxConfigEntry* pSourceData =
            (SvxConfigEntry*) pSourceEntry->GetUserData();
        SvxConfigEntry* pTargetData =
            (SvxConfigEntry*) pTargetEntry->GetUserData();

        SvxEntries::iterator iter1 = GetEntries()->begin();
        SvxEntries::iterator iter2 = GetEntries()->begin();
        SvxEntries::const_iterator end = GetEntries()->end();

        // Advance the iterators to the positions of the source and target
        while (*iter1 != pSourceData && ++iter1 != end);
        while (*iter2 != pTargetData && ++iter2 != end);

        // Now swap the entries in the menu list and in the UI
        if ( iter1 != end && iter2 != end )
        {
            std::swap( *iter1, *iter2 );
            aMenuListBox.GetModel()->Move( pSourceEntry, pTargetEntry );
            aMenuListBox.MakeVisible( pSourceEntry );

            bModified = TRUE;
        }
    }

    if ( bModified )
    {
        UpdateButtonStates();
    }

    return 0;
}

SvxEntries* SvxMainMenuOrganizerDialog::GetEntries()
{
    return pEntries;
}

SvxConfigEntry* SvxMainMenuOrganizerDialog::GetSelectedEntry()
{
    return (SvxConfigEntry*)aMenuListBox.FirstSelected()->GetUserData();
}

SvxConfigEntry::SvxConfigEntry(
    const uno::Sequence< beans::PropertyValue >& rProperties,
    const uno::Reference< container::XNameAccess >& rCommandToLabelMap )
    :
        nId( 0 ),
        bPopUp( FALSE ),
        bIsUserDefined( FALSE ),
        bIsMain( FALSE ),
        bStrEdited( FALSE ),
        bIsVisible( TRUE ),
        nStyle( 0 ),
        pEntries( 0 )
{
    Help* pHelp = Application::GetHelp();
    sal_uInt16 nType( dcss::ui::ItemType::DEFAULT );
    OUString aHelpURL;

    for ( sal_Int32 i = 0; i < rProperties.getLength(); i++ )
    {
        if ( rProperties[i].Name.equalsAscii( ITEM_DESCRIPTOR_COMMANDURL ))
        {
            rProperties[i].Value >>= aCommand;
        }
        else if ( rProperties[i].Name.equalsAscii( ITEM_DESCRIPTOR_HELPURL ))
        {
            rProperties[i].Value >>= aHelpURL;
        }
        else if ( rProperties[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ))
        {
            rProperties[i].Value >>= aLabel;
        }
        else if ( rProperties[i].Name.equalsAscii( ITEM_DESCRIPTOR_TYPE ))
        {
            rProperties[i].Value >>= nType;
        }
    }

    if ( nType == dcss::ui::ItemType::DEFAULT )
    {
        OUString aHelpId = OUString::createFromAscii( "helpid:" );

        if ( aHelpURL.indexOf( aHelpId ) != -1 )
        {
            aHelpURL = aHelpURL.copy( aHelpId.getLength() );
        }

        if ( aHelpURL.toInt32() != 0 )
        {
            aHelpText = pHelp->GetHelpText( aHelpURL.toInt32(), NULL );
        }

        uno::Any a;
        try
        {
            a = rCommandToLabelMap->getByName( aCommand );
            bIsUserDefined = FALSE;
        }
        catch ( container::NoSuchElementException& )
        {
            bIsUserDefined = TRUE;
        }

        // If custom label not set retrieve it from the command to info service
        if ( aLabel.equals( OUString() ) )
        {
            uno::Sequence< beans::PropertyValue > aPropSeq;
            if ( a >>= aPropSeq )
            {
                for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                {
                    if ( aPropSeq[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ) )
                    {
                        aPropSeq[i].Value >>= aLabel;
                        break;
                    }
                }
            }
        }
    }
}

uno::Sequence< beans::PropertyValue >
SvxConfigEntry::GetProperties(
    const uno::Reference< container::XNameAccess >& rCommandToLabelMap )
{
    if ( IsSeparator() )
    {
        uno::Sequence< beans::PropertyValue > aPropSeq( 1 );

        aPropSeq[0].Name  = OUString(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ) );
        aPropSeq[0].Value <<= dcss::ui::ItemType::SEPARATOR_LINE;

        return aPropSeq;
    }

    static const OUString aDescriptorCommandURL (
        RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_COMMANDURL ) );

    static const OUString aDescriptorType(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_TYPE ) );

    static const OUString aDescriptorLabel(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_LABEL ) );

    static const OUString aDescriptorHelpURL(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_HELPURL ) );

    uno::Sequence< beans::PropertyValue > aPropSeq( 4 );

    aPropSeq[0].Name = aDescriptorCommandURL;
    aPropSeq[0].Value <<= rtl::OUString( GetCommand() );

    aPropSeq[1].Name = aDescriptorType;
    aPropSeq[1].Value <<= dcss::ui::ItemType::DEFAULT;

    // If the name has not been changed and the name is the same as
    // in the default command to label map then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    aPropSeq[2].Name = aDescriptorLabel;
    if ( HasChangedName() == FALSE && GetCommand().getLength() )
    {
        BOOL isDefaultName = FALSE;
        try
        {
            uno::Any a( rCommandToLabelMap->getByName( GetCommand() ) );
            uno::Sequence< beans::PropertyValue > tmpPropSeq;
            if ( a >>= tmpPropSeq )
            {
                for ( sal_Int32 i = 0; i < tmpPropSeq.getLength(); i++ )
                {
                    if ( tmpPropSeq[i].Name.equals( aDescriptorLabel ) )
                    {
                        OUString tmpLabel;
                        tmpPropSeq[i].Value >>= tmpLabel;

                        if ( tmpLabel.equals( GetName() ) )
                        {
                            isDefaultName = TRUE;
                        }

                        break;
                    }
                }
            }
        }
        catch ( container::NoSuchElementException& nsee )
        {
            // isDefaultName is left as FALSE
        }

        if ( isDefaultName )
        {
            aPropSeq[2].Value <<= rtl::OUString();
        }
        else
        {
            aPropSeq[2].Value <<= rtl::OUString( GetName() );
        }
    }
    else
    {
        aPropSeq[2].Value <<= rtl::OUString( GetName() );
    }

    aPropSeq[3].Name = aDescriptorHelpURL;
    aPropSeq[3].Value <<= rtl::OUString( GetHelpURL() );

    return aPropSeq;
}

SvxMenuConfigEntry::SvxMenuConfigEntry(
    const uno::Sequence< beans::PropertyValue >& rProperties,
    const uno::Reference< container::XNameAccess >& rCommandToLabelMap )
    :
        SvxConfigEntry( rProperties, rCommandToLabelMap )
{
    uno::Reference< container::XIndexAccess > aChildren;

    for ( sal_Int32 i = 0; i < rProperties.getLength(); i++ )
    {
        if ( rProperties[i].Name.equalsAscii( ITEM_DESCRIPTOR_CONTAINER ))
        {
            rProperties[i].Value >>= aChildren;
        }
    }

    if ( aChildren.is() )
    {
        SetPopup( TRUE );
        SetEntries( new SvxEntries() );

           uno::Sequence< beans::PropertyValue > aProps;
        for ( sal_Int32 i = 0; i < aChildren->getCount(); i++ )
        {
               if ( aChildren->getByIndex( i ) >>= aProps )
            {
                SvxConfigEntry* pEntry =
                    new SvxMenuConfigEntry( aProps, rCommandToLabelMap );
                GetEntries()->push_back( pEntry );
            }
        }
    }
}

SvxConfigEntry::SvxConfigEntry( USHORT nInitId, const String& rInitStr,
                                const String& rHelpText, bool bPopup )
    : nId( nInitId )
    , bPopUp(bPopup)
    , aHelpText(rHelpText)
    , aLabel(rInitStr)
    , bIsUserDefined( FALSE )
    , bIsMain( FALSE )
    , bStrEdited( FALSE )
    , bIsVisible( TRUE )
    , nStyle( 0 )
    , pEntries( 0 )
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        OSL_TRACE("Creating SvxConfigEntry for macro slot");

        SFX_APP()->GetMacroConfig()->RegisterSlotId( nId );
        SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo( nId );
        if ( pInfo )
            aCommand = pInfo->GetURL();
        bStrEdited = TRUE;
    }

    if (bPopUp)
    {
        pEntries = new SvxEntries();
    }
}

void SvxConfigEntry::SetId( USHORT nNew )
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->ReleaseSlotId( nId );
    nId = nNew;
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
    {
        SFX_APP()->GetMacroConfig()->RegisterSlotId( nId );
        SfxMacroInfo* pInfo = SFX_APP()->GetMacroConfig()->GetMacroInfo( nId );
        if ( pInfo )
            aCommand = pInfo->GetURL();
        bStrEdited = TRUE;
    }
    else
    {
        // Remove command string to fix problems if macro menu entry
        // is overwritten by a normal function!
        aCommand = String();
    }
}

SvxConfigEntry::~SvxConfigEntry()
{
    if ( SfxMacroConfig::IsMacroSlot( nId ) )
        SFX_APP()->GetMacroConfig()->ReleaseSlotId( nId );

    if ( pEntries != NULL )
    {
        SvxEntries::const_iterator iter = pEntries->begin();

        for ( ; iter != pEntries->end(); iter++ )
        {
            delete *iter;
        }
        delete pEntries;
    }
}

bool SvxConfigEntry::IsMovable()
{
    if ( IsPopup() && !IsMain() )
    {
        return FALSE;
    }
    return TRUE;
}

bool SvxConfigEntry::IsDeletable()
{
    if ( IsMain() && !IsUserDefined() )
    {
        return FALSE;
    }
    return TRUE;
}

bool SvxConfigEntry::IsRenamable()
{
    if ( IsMain() && !IsUserDefined() )
    {
        return FALSE;
    }
    return TRUE;
}

SvxToolbarConfigPage::SvxToolbarConfigPage(
    Window *pParent, const SfxItemSet& rSet )
    :   SvxConfigPage( pParent, rSet )
{
    SetHelpId( HID_SVX_CONFIG_TOOLBAR );

    aContentsListBox = new SvxToolbarEntriesListBox(
        this, ResId( BOX_ENTRIES ) );
    PositionContentsListBox();

    aContentsListBox->SetHelpId( HID_SVX_CONFIG_TOOLBAR_CONTENTS );

    aTopLevelSeparator.SetText(
        ResId ( RID_SVXSTR_PRODUCTNAME_TOOLBARS, DIALOG_MGR() ) );

    aTopLevelLabel.SetText( ResId ( RID_SVXSTR_TOOLBAR, DIALOG_MGR() ) );
    aModifyTopLevelButton.SetText( ResId ( RID_SVXSTR_TOOLBAR, DIALOG_MGR() ) );
    aContentsSeparator.SetText( ResId ( RID_SVXSTR_TOOLBAR_CONTENT, DIALOG_MGR() ) );
    aContentsLabel.SetText( ResId ( RID_SVXSTR_COMMANDS, DIALOG_MGR() ) );
    aModifyCommandButton.SetText( ResId ( RID_SVXSTR_COMMAND, DIALOG_MGR() ) );

    aTopLevelListBox.SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbar ) );
    aContentsListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbarEntry ) );

    aNewTopLevelButton.SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, NewToolbarHdl ) );

    aAddCommandsButton.SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, AddCommandsHdl ) );

    aMoveUpButton.SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    aMoveDownButton.SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );

    PopupMenu* pMenu = new PopupMenu( ResId( MODIFY_TOOLBAR, DIALOG_MGR() ) );
    pMenu->SetMenuFlags(
        pMenu->GetMenuFlags() | MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );

    aModifyTopLevelButton.SetPopupMenu( pMenu );
    aModifyTopLevelButton.SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, ToolbarSelectHdl ) );

    PopupMenu* pEntry = new PopupMenu(
        ResId( MODIFY_TOOLBAR_CONTENT, DIALOG_MGR() ) );
    pEntry->SetMenuFlags(
        pEntry->GetMenuFlags() | MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );

    aModifyCommandButton.SetPopupMenu( pEntry );
    aModifyCommandButton.SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, EntrySelectHdl ) );

    // default toolbar to select is standardbar unless a different one
    // has been passed in
    m_aURLToSelect = OUString::createFromAscii( ITEM_TOOLBAR_URL );
    m_aURLToSelect += OUString::createFromAscii( "standardbar" );

    const SfxPoolItem* pItem =
        rSet.GetItem( rSet.GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = ((const SfxStringItem*)pItem)->GetValue();
        if (text.indexOf(OUString::createFromAscii(ITEM_TOOLBAR_URL)) == 0)
        {
            m_aURLToSelect = text.copy( 0 );
        }
    }
}

SvxToolbarConfigPage::~SvxToolbarConfigPage()
{
    for ( sal_Int32 i = 0 ; i < aSaveInListBox.GetEntryCount(); i++ )
    {
        ToolbarSaveInData* pData =
            (ToolbarSaveInData*) aSaveInListBox.GetEntryData( i );

        delete pData;
    }

    if ( pSelectorDlg != NULL )
    {
        delete pSelectorDlg;
    }


    delete aContentsListBox;
}

IMPL_LINK( SvxToolbarConfigPage, MoveHdl, Button *, pButton )
{
    MoveEntry( pButton );

    // Apply change to currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    ((ToolbarSaveInData*)GetSaveInData())->ApplyToolbar( pToolbar );

    return 0;
}

IMPL_LINK( SvxToolbarConfigPage, ToolbarSelectHdl, MenuButton *, pButton )
{
    USHORT nSelectionPos = aTopLevelListBox.GetSelectEntryPos();

    SvxConfigEntry* pToolbar =
        (SvxConfigEntry*)aTopLevelListBox.GetEntryData( nSelectionPos );

    ToolbarSaveInData* pSaveInData = (ToolbarSaveInData*) GetSaveInData();

    switch( pButton->GetCurItemId() )
    {
        case ID_DELETE:
        {
            pSaveInData->RemoveToolbar( pToolbar );

            if ( aTopLevelListBox.GetEntryCount() > 1 )
            {
                // select next entry after the one being deleted
                // selection position is indexed from 0 so need to
                // subtract one from the entry count
                if ( nSelectionPos != aTopLevelListBox.GetEntryCount() - 1 )
                {
                    aTopLevelListBox.SelectEntryPos( nSelectionPos + 1, TRUE );
                }
                else
                {
                    aTopLevelListBox.SelectEntryPos( nSelectionPos - 1, TRUE );
                }
                aTopLevelListBox.GetSelectHdl().Call( this );

                // and now remove the entry
                aTopLevelListBox.RemoveEntry( nSelectionPos );
            }
            else
            {
                ReloadTopLevelListBox();
            }

            break;
        }
        case ID_RENAME:
        {
            String aNewName( stripHotKey( pToolbar->GetName() ) );
            String aDesc( RTL_CONSTASCII_USTRINGPARAM( "Enter New Name" ) );

            SvxAbstractDialogFactory* pFact =
                SvxAbstractDialogFactory::Create();

            AbstractSvxNameDialog* pNameDialog =
                pFact->CreateSvxNameDialog(
                    0, aNewName, aDesc, ResId(RID_SVXDLG_NAME) );

            bool ret = pNameDialog->Execute();

            if ( ret == RET_OK )
            {
                pNameDialog->GetName(aNewName);

                pToolbar->SetName( aNewName );
                pSaveInData->ApplyToolbar( pToolbar );

                // have to use remove and insert to change the name
                aTopLevelListBox.RemoveEntry( nSelectionPos );
                nSelectionPos =
                    aTopLevelListBox.InsertEntry( aNewName, nSelectionPos );
                aTopLevelListBox.SetEntryData( nSelectionPos, pToolbar );
                aTopLevelListBox.SelectEntryPos( nSelectionPos );
            }

            delete pNameDialog;

            break;
        }
        case ID_DEFAULT_STYLE:
        {
            QueryBox qbox( this,
                ResId( QBX_CONFIRM_RESTORE_DEFAULT, DIALOG_MGR() ) );

            if ( qbox.Execute() == RET_YES )
            {
                ToolbarSaveInData* pSaveInData =
                    (ToolbarSaveInData*) GetSaveInData();

                pSaveInData->RestoreToolbar( pToolbar );

                aTopLevelListBox.GetSelectHdl().Call( this );
            }

            break;
        }
        case ID_ICONS_ONLY:
        {
            SfxViewFrame* pViewFrame = GetTabDialog()->GetViewFrame();

            uno::Reference < frame::XFrame > xFrame =
                uno::Reference< frame::XFrame >(
                    pViewFrame->GetFrame()->GetFrameInterface());

            pToolbar->SetStyle( 0 );
            pSaveInData->SetSystemStyle( xFrame, pToolbar->GetCommand(), 0 );

            aTopLevelListBox.GetSelectHdl().Call( this );

            break;
        }
        case ID_TEXT_ONLY:
        {
            SfxViewFrame* pViewFrame = GetTabDialog()->GetViewFrame();

            uno::Reference < frame::XFrame > xFrame =
                uno::Reference< frame::XFrame >(
                    pViewFrame->GetFrame()->GetFrameInterface());

            pToolbar->SetStyle( 1 );
            pSaveInData->SetSystemStyle( xFrame, pToolbar->GetCommand(), 1 );

            aTopLevelListBox.GetSelectHdl().Call( this );

            break;
        }
        case ID_ICONS_AND_TEXT:
        {
            SfxViewFrame* pViewFrame = GetTabDialog()->GetViewFrame();

            uno::Reference < frame::XFrame > xFrame =
                uno::Reference< frame::XFrame >(
                    pViewFrame->GetFrame()->GetFrameInterface());

            pToolbar->SetStyle( 2 );
            pSaveInData->SetSystemStyle( xFrame, pToolbar->GetCommand(), 2 );

            aTopLevelListBox.GetSelectHdl().Call( this );

            break;
        }
    }
    return 1;
}

IMPL_LINK( SvxToolbarConfigPage, EntrySelectHdl, MenuButton *, pButton )
{
    bool bModified = FALSE;

    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    switch( pButton->GetCurItemId() )
    {
        case ID_RENAME:
        {
            SvLBoxEntry* pActEntry = aContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                (SvxConfigEntry*) pActEntry->GetUserData();

            String aNewName( stripHotKey( pEntry->GetName() ) );
            String aDesc( RTL_CONSTASCII_USTRINGPARAM( "Enter New Name" ) );

            SvxAbstractDialogFactory* pFact =
                SvxAbstractDialogFactory::Create();

            AbstractSvxNameDialog* pNameDialog =
                pFact->CreateSvxNameDialog(
                    0, aNewName, aDesc, ResId(RID_SVXDLG_NAME) );

            bool ret = pNameDialog->Execute();

            if ( ret == RET_OK ) {
                pNameDialog->GetName(aNewName);

                pEntry->SetName( aNewName );
                aContentsListBox->SetEntryText( pActEntry, aNewName );

                bModified = TRUE;
            }

            delete pNameDialog;
            break;
        }
        case ID_DEFAULT_STYLE:
        {
            SvLBoxEntry* pActEntry = aContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                (SvxConfigEntry*) pActEntry->GetUserData();

            short nSelectionPos = 0;

            // find position of entry within the list
            for ( short i = 0; i < aContentsListBox->GetEntryCount(); i++ )
            {
                if ( aContentsListBox->GetEntry( 0, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            ToolbarSaveInData* pSaveInData =
                (ToolbarSaveInData*) GetSaveInData();

            OUString aSystemName =
                pSaveInData->GetSystemUIName( pEntry->GetCommand() );

            if ( !pEntry->GetName().equals( aSystemName ) )
            {
                pEntry->SetName( aSystemName );
                aContentsListBox->SetEntryText(
                    pActEntry, stripHotKey( aSystemName ) );
                bModified = TRUE;
            }

            uno::Reference< graphic::XGraphic > backup =
                pEntry->GetBackupGraphic();

            if ( backup.is() )
            {
                uno::Sequence< uno::Reference< graphic::XGraphic > >
                    aGraphicSeq( 1 );
                aGraphicSeq[ 0 ] = backup;

                uno::Sequence< OUString > aURLSeq( 1 );
                aURLSeq[ 0 ] = pEntry->GetCommand();

                try
                {
                    GetSaveInData()->GetImageManager()->replaceImages(
                        GetImageType(), aURLSeq, aGraphicSeq );

                    Image aImage( backup );
                    aContentsListBox->GetModel()->Remove( pActEntry );

                    SvLBoxEntry* pNewLBEntry =
                        aContentsListBox->InsertEntry(
                            stripHotKey( pEntry->GetName() ),
                            aImage, aImage, 0, FALSE,
                            nSelectionPos, pEntry );

                    aContentsListBox->SetCheckButtonState( pNewLBEntry,
                        pEntry->IsVisible() ?
                            SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                    aContentsListBox->Select( pNewLBEntry );
                    aContentsListBox->MakeVisible( pNewLBEntry );

                    // reset backup in entry
                    pEntry->SetBackupGraphic(
                        uno::Reference< graphic::XGraphic >() );

                    GetSaveInData()->PersistChanges(
                        GetSaveInData()->GetImageManager() );

                    bModified = TRUE;
                }
                   catch ( uno::Exception& )
                   {
                    OSL_TRACE("Error replacing image");
                   }
            }
            break;
        }
        case ID_BEGIN_GROUP:
        {
            SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
            pNewEntryData->SetUserDefined( TRUE );

            SvLBoxEntry* pNewLBEntry = InsertEntry( pNewEntryData );

            aContentsListBox->SetCheckButtonState(
                pNewLBEntry, SV_BUTTON_TRISTATE );

            bModified = TRUE;
            break;
        }
        case ID_DELETE:
        {
            SvLBoxEntry *pActEntry = aContentsListBox->FirstSelected();

            if ( pActEntry != NULL )
            {
                // get currently selected entry
                SvxConfigEntry* pEntry =
                    (SvxConfigEntry*) pActEntry->GetUserData();

                // remove entry from the list for this toolbar
                RemoveEntry( pToolbar->GetEntries(), pEntry );

                // remove toolbar entry from UI
                aContentsListBox->GetModel()->Remove( pActEntry );

                // delete data for toolbar entry
                delete pEntry;

                bModified = TRUE;
            }

            break;
        }
        case ID_ICON_ONLY:
        {
            break;
        }
        case ID_TEXT_ONLY:
        {
            break;
        }
        case ID_ICON_AND_TEXT:
        {
            break;
        }
        case ID_CHANGE_SYMBOL:
        {
            SvLBoxEntry* pActEntry = aContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                (SvxConfigEntry*) pActEntry->GetUserData();

            short nSelectionPos = 0;

            // find position of entry within the list
            for ( short i = 0; i < aContentsListBox->GetEntryCount(); i++ )
            {
                if ( aContentsListBox->GetEntry( 0, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            SvxIconSelectorDialog* pIconDialog =
                new SvxIconSelectorDialog( 0,
                    GetSaveInData()->GetImageManager() );

            bool ret = pIconDialog->Execute();

            if ( ret == RET_OK )
            {
                uno::Reference< graphic::XGraphic > newgraphic =
                    pIconDialog->GetSelectedIcon();

                if ( newgraphic.is() )
                {
                    uno::Sequence< uno::Reference< graphic::XGraphic > >
                        aGraphicSeq( 1 );

                    uno::Sequence< OUString > aURLSeq( 1 );
                    aURLSeq[ 0 ] = pEntry->GetCommand();

                    if ( !pEntry->GetBackupGraphic().is() )
                    {
                        uno::Reference< graphic::XGraphic > backup;
                        backup = GetGraphic(
                            GetSaveInData()->GetImageManager(), aURLSeq[ 0 ] );

                        if ( backup.is() )
                        {
                            pEntry->SetBackupGraphic( backup );
                        }
                    }

                    aGraphicSeq[ 0 ] = newgraphic;
                    try
                    {
                        GetSaveInData()->GetImageManager()->replaceImages(
                            GetImageType(), aURLSeq, aGraphicSeq );

                        Image aImage( newgraphic );
                        aContentsListBox->GetModel()->Remove( pActEntry );

                        SvLBoxEntry* pNewLBEntry =
                            aContentsListBox->InsertEntry(
                                stripHotKey( pEntry->GetName() ),
                                aImage, aImage, 0, FALSE,
                                nSelectionPos, pEntry );

                        aContentsListBox->SetCheckButtonState( pNewLBEntry,
                            pEntry->IsVisible() ?
                                SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                        aContentsListBox->Select( pNewLBEntry );
                        aContentsListBox->MakeVisible( pNewLBEntry );

                        GetSaveInData()->PersistChanges(
                            GetSaveInData()->GetImageManager() );

                        bModified = TRUE;
                    }
                    catch ( uno::Exception& )
                    {
                        OSL_TRACE("Error replacing image");
                    }
                }
            }

            delete pIconDialog;

            break;
        }
        case ID_RESET_SYMBOL:
        {
            SvLBoxEntry* pActEntry = aContentsListBox->GetCurEntry();
            SvxConfigEntry* pEntry =
                (SvxConfigEntry*) pActEntry->GetUserData();

            short nSelectionPos = 0;

            // find position of entry within the list
            for ( short i = 0; i < aContentsListBox->GetEntryCount(); i++ )
            {
                if ( aContentsListBox->GetEntry( 0, i ) == pActEntry )
                {
                    nSelectionPos = i;
                    break;
                }
            }

            uno::Reference< graphic::XGraphic > backup =
                pEntry->GetBackupGraphic();

            uno::Sequence< uno::Reference< graphic::XGraphic > >
                aGraphicSeq( 1 );
            aGraphicSeq[ 0 ] = backup;

            uno::Sequence< OUString > aURLSeq( 1 );
            aURLSeq[ 0 ] = pEntry->GetCommand();

            try
            {
                GetSaveInData()->GetImageManager()->replaceImages(
                    GetImageType(), aURLSeq, aGraphicSeq );

                Image aImage( backup );
                aContentsListBox->GetModel()->Remove( pActEntry );

                SvLBoxEntry* pNewLBEntry =
                    aContentsListBox->InsertEntry(
                        stripHotKey( pEntry->GetName() ),
                        aImage, aImage, 0, FALSE,
                        nSelectionPos, pEntry );

                aContentsListBox->SetCheckButtonState( pNewLBEntry,
                    pEntry->IsVisible() ?
                        SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );

                aContentsListBox->Select( pNewLBEntry );
                aContentsListBox->MakeVisible( pNewLBEntry );

                // reset backup in entry
                pEntry->SetBackupGraphic(
                    uno::Reference< graphic::XGraphic >() );

                GetSaveInData()->PersistChanges(
                    GetSaveInData()->GetImageManager() );

                bModified = TRUE;
            }
            catch ( uno::Exception& )
            {
                OSL_TRACE("Error resetting image");
            }
            break;
        }
    }

    if ( bModified == TRUE )
    {
        (( ToolbarSaveInData* ) GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();
    }

    return 1;
}

void SvxToolbarConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    aTopLevelListBox.Clear();
    aContentsListBox->Clear();

    ReloadTopLevelListBox();

    USHORT nPos = 0;
    if ( m_aURLToSelect.getLength() != 0 )
    {
        for ( sal_Int32 i = 0 ; i < aTopLevelListBox.GetEntryCount(); i++ )
        {
            SvxConfigEntry* pData =
                (SvxConfigEntry*) aTopLevelListBox.GetEntryData( i );

            if ( pData->GetCommand().equals( m_aURLToSelect ) )
            {
                nPos = i;
                break;
            }
        }
        m_aURLToSelect = OUString();
    }

    aTopLevelListBox.SelectEntryPos(nPos, TRUE);
    aTopLevelListBox.GetSelectHdl().Call(this);
}

SaveInData* SvxToolbarConfigPage::CreateSaveInData(
    const uno::Reference< dcss::ui::XUIConfigurationManager >& xCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    return static_cast< SaveInData* >(
        new ToolbarSaveInData( xCfgMgr, aModuleId, bDocConfig ));
}

ToolbarSaveInData::ToolbarSaveInData(
    const uno::Reference <
        dcss::ui::XUIConfigurationManager >& xCfgMgr,
    const OUString& aModuleId,
    bool docConfig )
    :
        SaveInData ( xCfgMgr, aModuleId, docConfig ),
        m_aDescriptorContainer(
            RTL_CONSTASCII_USTRINGPARAM( ITEM_DESCRIPTOR_CONTAINER ) ),
        pRootEntry( 0 )
{
    // Initialize the m_xPersistentWindowState variable which is used
    // to get the default properties of system toolbars such as name
    uno::Reference< container::XNameAccess > xPWSS(
        m_xServiceManager->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "drafts.com.sun.star.ui.WindowStateConfiguration" ) ) ),
        uno::UNO_QUERY );

    if ( xPWSS.is() )
        xPWSS->getByName( aModuleId ) >>= m_xPersistentWindowState;
}

ToolbarSaveInData::~ToolbarSaveInData()
{
    if ( pRootEntry != NULL )
    {
        delete pRootEntry;
    }
}

void ToolbarSaveInData::SetSystemStyle(
    uno::Reference< frame::XFrame > xFrame,
    const OUString& rResourceURL,
    sal_Int32 nStyle )
{
    // change the style using the API
    SetSystemStyle( rResourceURL, nStyle );

    // this code is a temporary hack as the UI is not updating after
    // changing the toolbar style via the API
    uno::Reference< dcss::frame::XLayoutManager > xLayoutManager;
    Window *window = NULL;

    uno::Reference< beans::XPropertySet > xPropSet( xFrame, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any a = xPropSet->getPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) );
        a >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
    {
        uno::Reference< dcss::ui::XUIElement > xUIElement =
            xLayoutManager->getElement( rResourceURL );

        // check reference before we call getRealInterface. The layout manager
        // can only provide references for elements that have been created
        // before. It's possible that the current element is not available.
        uno::Reference< com::sun::star::awt::XWindow > xWindow;
        if ( xUIElement.is() )
            xWindow = uno::Reference< com::sun::star::awt::XWindow >(
                        xUIElement->getRealInterface(), uno::UNO_QUERY );

        window = VCLUnoHelper::GetWindow( xWindow );
    }

    if ( window != NULL && window->GetType() == WINDOW_TOOLBOX )
    {
        ToolBox* toolbox = (ToolBox*)window;

        if ( nStyle == 0 )
        {
            toolbox->SetButtonType( BUTTON_SYMBOL );
        }
        else if ( nStyle == 1 )
        {
            toolbox->SetButtonType( BUTTON_TEXT );
        }
        if ( nStyle == 2 )
        {
            toolbox->SetButtonType( BUTTON_SYMBOLTEXT );
        }
    }
}

void ToolbarSaveInData::SetSystemStyle(
    const OUString& rResourceURL,
    sal_Int32 nStyle )
{
    if ( rResourceURL.indexOf( OUString::createFromAscii( "private" ) ) == 0 &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;

            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); i++ )
                {
                    if ( aProps[ i ].Name.equalsAscii( ITEM_DESCRIPTOR_STYLE) )
                    {
                        aProps[ i ].Value = uno::makeAny( nStyle );
                        break;
                    }
                }
            }

            uno::Reference< container::XNameReplace >
                xNameReplace( m_xPersistentWindowState, uno::UNO_QUERY );

            xNameReplace->replaceByName( rResourceURL, uno::makeAny( aProps ) );
        }
        catch ( uno::Exception& )
        {
            // do nothing, a default value is returned
            OSL_TRACE("Exception setting toolbar style");
        }
    }
}

sal_Int32 ToolbarSaveInData::GetSystemStyle( const OUString& rResourceURL )
{
    sal_Int32 result = 0;

    if ( rResourceURL.indexOf( OUString::createFromAscii( "private" ) ) == 0 &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;
            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); i++ )
                {
                    if ( aProps[ i ].Name.equalsAscii( ITEM_DESCRIPTOR_STYLE) )
                    {
                        aProps[i].Value >>= result;
                        break;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // do nothing, a default value is returned
        }
    }

    return result;
}

OUString ToolbarSaveInData::GetSystemUIName( const OUString& rResourceURL )
{
    OUString result;

    if ( rResourceURL.indexOf( OUString::createFromAscii( "private" ) ) == 0 &&
         m_xPersistentWindowState.is() &&
         m_xPersistentWindowState->hasByName( rResourceURL ) )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aProps;
            uno::Any a( m_xPersistentWindowState->getByName( rResourceURL ) );

            if ( a >>= aProps )
            {
                for ( sal_Int32 i = 0; i < aProps.getLength(); i++ )
                {
                    if ( aProps[ i ].Name.equalsAscii( ITEM_DESCRIPTOR_UINAME) )
                    {
                        aProps[ i ].Value >>= result;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // do nothing, an empty UIName will be returned
        }
    }

    if ( rResourceURL.indexOf( OUString::createFromAscii( ".uno" ) ) == 0 &&
         m_xCommandToLabelMap.is() &&
         m_xCommandToLabelMap->hasByName( rResourceURL ) )
    {
        uno::Any a;
        try
        {
            a = m_xCommandToLabelMap->getByName( rResourceURL );

            uno::Sequence< beans::PropertyValue > aPropSeq;
            if ( a >>= aPropSeq )
            {
                for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                {
                    if ( aPropSeq[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ) )
                    {
                        aPropSeq[i].Value >>= result;
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            // not a system command name
        }
    }

    return result;
}

bool EntrySort( SvxConfigEntry* a, SvxConfigEntry* b )
{
    return a->GetName().compareTo( b->GetName() ) < 0;
}

SvxEntries* ToolbarSaveInData::GetEntries()
{
    if ( pRootEntry == NULL )
    {
        pRootEntry = new SvxConfigEntry(
            0, String::CreateFromAscii("MainToolbars"), String(), TRUE );

        uno::Sequence< uno::Sequence < beans::PropertyValue > > info =
            GetConfigManager()->getUIElementsInfo(
                dcss::ui::UIElementType::TOOLBAR );

        for ( sal_Int32 i = 0; i < info.getLength(); i++ )
        {
            uno::Sequence< beans::PropertyValue > props = info[ i ];

            OUString url;
            OUString systemname;
            OUString uiname;

            for ( sal_Int32 j = 0; j < props.getLength(); j++ )
            {
                if ( props[ j ].Name.equalsAscii( ITEM_DESCRIPTOR_RESOURCEURL) )
                {
                    props[ j ].Value >>= url;
                    systemname = url.copy( url.lastIndexOf( '/' ) + 1 );
                }
                else if ( props[ j ].Name.equalsAscii( ITEM_DESCRIPTOR_UINAME) )
                {
                    props[ j ].Value >>= uiname;
                }
            }

            try
            {
                uno::Reference< container::XIndexAccess > xToolbarSettings =
                    GetConfigManager()->getSettings( url, sal_False );

                uno::Reference< beans::XPropertySet > props(
                    xToolbarSettings, uno::UNO_QUERY );

                if ( uiname.getLength() == 0 )
                {
                    // try to get the name from m_xPersistentWindowState
                    uiname = GetSystemUIName( url );

                    if ( uiname.getLength() == 0 )
                    {
                        uiname = systemname;
                    }
                }

                SvxConfigEntry* pEntry = new SvxConfigEntry(
                    1, uiname, String(), TRUE );

                pEntry->SetCommand( url );
                pEntry->SetMain( TRUE );
                pEntry->SetStyle( GetSystemStyle( url ) );

                OUString custom = OUString::createFromAscii( "custom_" );
                if ( systemname.indexOf( custom ) == 0 )
                {
                    pEntry->SetUserDefined( TRUE );
                }
                else
                {
                    pEntry->SetUserDefined( FALSE );
                }

                pRootEntry->GetEntries()->push_back( pEntry );

                LoadToolbar( xToolbarSettings, pEntry );
            }
            catch ( container::NoSuchElementException& )
            {
                // TODO, handle resourceURL with no settings
            }
        }
        std::sort( GetEntries()->begin(), GetEntries()->end(), EntrySort );
    }

    return pRootEntry->GetEntries();
}

void
ToolbarSaveInData::SetEntries( SvxEntries* pNewEntries )
{
    // delete old menu hierarchy first
    if ( pRootEntry != NULL && pRootEntry->GetEntries() != NULL )
    {
        delete pRootEntry->GetEntries();
    }

    // now set new menu hierarchy
    pRootEntry->SetEntries( pNewEntries );
}

bool
ToolbarSaveInData::HasURL( const OUString& rURL )
{
    SvxEntries::const_iterator iter = GetEntries()->begin();
    SvxEntries::const_iterator end = GetEntries()->end();

    while ( iter != end )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->GetCommand().equals( rURL ) )
        {
            return TRUE;
        }

        iter++;
    }
    return FALSE;
}

bool ToolbarSaveInData::HasSettings()
{
    // return true if there is at least one toolbar entry
    if ( GetEntries()->size() > 0 )
    {
        return TRUE;
    }
    return FALSE;
}

void ToolbarSaveInData::Reset()
{
    // reset functionality is provided at the individual toolbar
    // or toolbar item level
}

bool ToolbarSaveInData::Apply()
{
    // toolbar changes are instantly applied
    return FALSE;
}

void ToolbarSaveInData::ApplyToolbar(
    uno::Reference< container::XIndexContainer >& rToolbarBar,
    uno::Reference< lang::XSingleComponentFactory >& rFactory,
    SvxConfigEntry* pToolbarData )
{
    SvxEntries::const_iterator iter = pToolbarData->GetEntries()->begin();
    SvxEntries::const_iterator end = pToolbarData->GetEntries()->end();

    for ( ; iter != end; iter++ )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->IsPopup() )
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertToolbarEntry( m_xCommandToLabelMap, pEntry );

            uno::Reference< container::XIndexContainer > xSubMenuBar(
                rFactory->createInstanceWithContext( m_xComponentContext ),
                    uno::UNO_QUERY );

            sal_Int32 nIndex = aPropValueSeq.getLength();
            aPropValueSeq.realloc( nIndex + 1 );
            aPropValueSeq[nIndex].Name = m_aDescriptorContainer;
            aPropValueSeq[nIndex].Value <<= xSubMenuBar;
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( aPropValueSeq ));

            ApplyToolbar( xSubMenuBar, rFactory, pEntry );
        }
        else if ( pEntry->IsSeparator() )
        {
            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( m_aSeparatorSeq ));
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aPropValueSeq =
                ConvertToolbarEntry( m_xCommandToLabelMap, pEntry );

            rToolbarBar->insertByIndex(
                rToolbarBar->getCount(), uno::makeAny( aPropValueSeq ));
        }
    }
}

void ToolbarSaveInData::ApplyToolbar( SvxConfigEntry* pToolbar )
{
    // Apply new toolbar structure to our settings container
    uno::Reference< container::XIndexAccess > xSettings(
        GetConfigManager()->createSettings(), uno::UNO_QUERY );

    uno::Reference< container::XIndexContainer > xIndexContainer (
        xSettings, uno::UNO_QUERY );

    uno::Reference< lang::XSingleComponentFactory > xFactory (
        xSettings, uno::UNO_QUERY );

    ApplyToolbar( xIndexContainer, xFactory, pToolbar );

    uno::Reference< beans::XPropertySet > xProps(
        xSettings, uno::UNO_QUERY );

    if ( pToolbar->IsUserDefined() )
    {
        xProps->setPropertyValue(
            OUString::createFromAscii( ITEM_DESCRIPTOR_UINAME ),
            uno::makeAny( OUString( pToolbar->GetName() ) ) );
    }

    try
    {
        if ( GetConfigManager()->hasSettings( pToolbar->GetCommand() ) )
        {
            GetConfigManager()->replaceSettings(
                pToolbar->GetCommand(), xSettings );
        }
        else
        {
            GetConfigManager()->insertSettings(
                pToolbar->GetCommand(), xSettings );
        }
    }
    catch ( container::NoSuchElementException& )
    {
        OSL_TRACE("caught container::NoSuchElementException saving settings");
    }
    catch ( com::sun::star::io::IOException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( com::sun::star::uno::Exception& )
    {
        OSL_TRACE("caught some other exception saving settings");
    }

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::CreateToolbar( SvxConfigEntry* pToolbar )
{
    // show the new toolbar in the UI also
    uno::Reference< container::XIndexAccess >
        xSettings( GetConfigManager()->createSettings(), uno::UNO_QUERY );

    uno::Reference< container::XIndexContainer >
        xIndexContainer ( xSettings, uno::UNO_QUERY );

    uno::Reference< beans::XPropertySet >
        xPropertySet( xSettings, uno::UNO_QUERY );

    xPropertySet->setPropertyValue(
        OUString::createFromAscii( ITEM_DESCRIPTOR_UINAME ),
            uno::makeAny( pToolbar->GetName() ) );

    try
    {
        GetConfigManager()->insertSettings( pToolbar->GetCommand(), xSettings );
    }
    catch ( container::ElementExistException& )
    {
        OSL_TRACE("caught ElementExistsException saving settings");
    }
    catch ( com::sun::star::lang::IllegalArgumentException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( com::sun::star::lang::IllegalAccessException& )
    {
        OSL_TRACE("caught IOException saving settings");
    }
    catch ( com::sun::star::uno::Exception& )
    {
        OSL_TRACE("caught some other exception saving settings");
    }

    GetEntries()->push_back( pToolbar );

    PersistChanges( GetConfigManager() );
}

void ToolbarSaveInData::RemoveToolbar( SvxConfigEntry* pToolbar )
{
    try
    {
        OUString url = pToolbar->GetCommand();
        GetConfigManager()->removeSettings( url );
        RemoveEntry( GetEntries(), pToolbar );
        delete pToolbar;

        PersistChanges( GetConfigManager() );
    }
    catch ( uno::Exception& )
    {
        // error occured removing the settings
    }
}

void ToolbarSaveInData::RestoreToolbar( SvxConfigEntry* pToolbar )
{
    OUString url = pToolbar->GetCommand();

    // Restore of toolbar is done by removing it from
    // it's configuration manager and then getting it again
    try
    {
        GetConfigManager()->removeSettings( url );
        pToolbar->GetEntries()->clear();
        PersistChanges( GetConfigManager() );
    }
    catch ( uno::Exception& )
    {
        // if an error occurs removing the settings then just return
        return;
    }

    // Now reload the toolbar settings
    try
    {
        uno::Reference< container::XIndexAccess > xToolbarSettings =
            GetConfigManager()->getSettings( url, sal_False );

        LoadToolbar( xToolbarSettings, pToolbar );
    }
    catch ( container::NoSuchElementException& )
    {
        // cannot find the resource URL after removing it
        // so no entry will appear in the toolbar list
    }
}

void ToolbarSaveInData::ReloadToolbar( const OUString& rResourceURL )
{
    SvxEntries::const_iterator iter = GetEntries()->begin();
    SvxConfigEntry* pToolbar = NULL;

    for ( ; iter != GetEntries()->end(); iter++ )
    {
        SvxConfigEntry* pEntry = *iter;

        if ( pEntry->GetCommand().equals( rResourceURL ) )
        {
            pToolbar = pEntry;
            break;
        }
    }

    if ( pToolbar != NULL )
    {
        delete pToolbar->GetEntries();

        try
        {
            uno::Reference< container::XIndexAccess > xToolbarSettings =
                GetConfigManager()->getSettings(
                    pToolbar->GetCommand(), sal_False);

            LoadToolbar( xToolbarSettings, pToolbar );
        }
        catch ( container::NoSuchElementException& )
        {
            // toolbar not found for some reason
            // it will not appear in the toolbar list
        }
    }
}

bool ToolbarSaveInData::LoadToolbar(
    const uno::Reference< container::XIndexAccess >& xToolbarSettings,
    SvxConfigEntry* pParentData )
{
    Help*               pHelp               = Application::GetHelp();
    SvxEntries*         pEntries            = pParentData->GetEntries();

    for ( sal_Int32 nIndex = 0; nIndex < xToolbarSettings->getCount(); nIndex++ )
    {
        uno::Reference< container::XIndexAccess >   xSubMenu;
        OUString                aCommandURL;
        OUString                aHelpURL;
        OUString                aLabel;
        bool                    bIsUserDefined = TRUE;
        sal_Bool                bIsVisible;
        sal_Int32               nStyle;

        sal_uInt16 nType( dcss::ui::ItemType::DEFAULT );

        bool bItem = GetToolbarItemData( xToolbarSettings, nIndex, aCommandURL,
            aHelpURL, aLabel, nType, bIsVisible, nStyle, xSubMenu );

        if ( bItem )
        {
            if ( nType == dcss::ui::ItemType::DEFAULT )
            {
                OUString aHelpText;
                OUString helpid =
                    OUString::createFromAscii( "helpid:" );

                if ( aHelpURL.indexOf( helpid ) != -1 )
                {
                    aHelpURL = aHelpURL.copy( helpid.getLength() );
                }

                if ( aHelpURL.toInt32() != 0 )
                {
                    aHelpText = pHelp->GetHelpText( aHelpURL.toInt32(), NULL );
                }

                uno::Any a;
                try
                {
                    a = m_xCommandToLabelMap->getByName( aCommandURL );
                    bIsUserDefined = FALSE;
                }
                catch ( container::NoSuchElementException& )
                {
                    bIsUserDefined = TRUE;
                }

                // If custom label not set retrieve it from the command
                // to info service
                if ( aLabel.equals( OUString() ) )
                {
                    uno::Sequence< beans::PropertyValue > aPropSeq;
                    if ( a >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                        {
                            if ( aPropSeq[i].Name.equalsAscii( ITEM_DESCRIPTOR_LABEL ) )
                            {
                                aPropSeq[i].Value >>= aLabel;
                                break;
                            }
                        }
                    }
                }

                if ( xSubMenu.is() )
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        1, aLabel, aHelpText, TRUE );

                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetCommand( aCommandURL );
                    pEntry->SetHelpURL( aHelpURL );
                    pEntry->SetVisible( bIsVisible );

                    pEntries->push_back( pEntry );

                    LoadToolbar( xSubMenu, pEntry );
                }
                else
                {
                    SvxConfigEntry* pEntry = new SvxConfigEntry(
                        1, aLabel, aHelpText, FALSE );
                    pEntry->SetUserDefined( bIsUserDefined );
                    pEntry->SetCommand( aCommandURL );
                    pEntry->SetHelpURL( aHelpURL );
                    pEntry->SetVisible( bIsVisible );
                    pEntries->push_back( pEntry );
                }
            }
            else
            {
                SvxConfigEntry* pEntry = new SvxConfigEntry;
                pEntry->SetUserDefined( bIsUserDefined );
                pEntries->push_back( pEntry );
            }
        }
    }

    return true;
}

IMPL_LINK( SvxToolbarConfigPage, SelectToolbarEntry, Control *, pBox )
{
    UpdateButtonStates();
    return 1;
}

void SvxToolbarConfigPage::UpdateButtonStates()
{
    PopupMenu* pPopup = aModifyCommandButton.GetPopupMenu();

    // disable all buttons first and then re-enable buttons as needed
    aMoveUpButton.Enable( FALSE );
    aMoveDownButton.Enable( FALSE );

    pPopup->EnableItem( ID_RENAME, FALSE );
    pPopup->EnableItem( ID_DELETE, FALSE );
    pPopup->EnableItem( ID_BEGIN_GROUP, FALSE );
    pPopup->EnableItem( ID_DEFAULT_STYLE, FALSE );
    pPopup->EnableItem( ID_ICON_ONLY, FALSE );
    pPopup->EnableItem( ID_ICON_AND_TEXT, FALSE );
    pPopup->EnableItem( ID_TEXT_ONLY, FALSE );
    pPopup->EnableItem( ID_CHANGE_SYMBOL, FALSE );
    pPopup->EnableItem( ID_RESET_SYMBOL, FALSE );

    aDescriptionText.SetText( String() );

    SvLBoxEntry* selection = aContentsListBox->GetCurEntry();
    if ( aContentsListBox->GetEntryCount() == 0 || selection == NULL )
    {
        return;
    }

    SvLBoxEntry* first = aContentsListBox->First();
    SvLBoxEntry* last = aContentsListBox->Last();

    aMoveUpButton.Enable( selection != first );
    aMoveDownButton.Enable( selection != last );

    SvxConfigEntry* pEntryData =
        (SvxConfigEntry*) selection->GetUserData();

    if ( pEntryData->IsSeparator() )
    {
        pPopup->EnableItem( ID_DELETE, TRUE );
        pPopup->EnableItem( ID_BEGIN_GROUP, FALSE );
        pPopup->EnableItem( ID_RENAME, FALSE );
    }
    else
    {
        pPopup->EnableItem( ID_BEGIN_GROUP, TRUE );
        pPopup->EnableItem( ID_DELETE, TRUE );
        pPopup->EnableItem( ID_RENAME, TRUE );
        pPopup->EnableItem( ID_ICON_ONLY, TRUE );
        pPopup->EnableItem( ID_ICON_AND_TEXT, TRUE );
        pPopup->EnableItem( ID_TEXT_ONLY, TRUE );
        pPopup->EnableItem( ID_CHANGE_SYMBOL, TRUE );

        if ( !pEntryData->IsUserDefined() )
        {
            pPopup->EnableItem( ID_DEFAULT_STYLE, TRUE );
        }

        if ( pEntryData->IsIconModified() )
        {
            pPopup->EnableItem( ID_RESET_SYMBOL, TRUE );
        }

        aDescriptionText.SetText( pEntryData->GetHelpText() );
    }
}

IMPL_LINK( SvxToolbarConfigPage, SelectToolbar, ListBox *, pBox )
{
    aContentsListBox->Clear();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar == NULL )
    {
        aModifyTopLevelButton.Enable( FALSE );
        aModifyCommandButton.Enable( FALSE );
        aAddCommandsButton.Enable( FALSE );

        return 0;
    }

    aModifyTopLevelButton.Enable( TRUE );
    aModifyCommandButton.Enable( TRUE );
    aAddCommandsButton.Enable( TRUE );

    PopupMenu* pPopup = aModifyTopLevelButton.GetPopupMenu();

    pPopup->EnableItem( ID_DELETE, pToolbar->IsDeletable() );
    pPopup->EnableItem( ID_RENAME, pToolbar->IsRenamable() );
    pPopup->EnableItem( ID_DEFAULT_STYLE, !pToolbar->IsRenamable() );

    switch( pToolbar->GetStyle() )
    {
        case 0:
        {
            pPopup->CheckItem( ID_ICONS_ONLY );
            break;
        }
        case 1:
        {
            pPopup->CheckItem( ID_TEXT_ONLY );
            break;
        }
        case 2:
        {
            pPopup->CheckItem( ID_ICONS_AND_TEXT );
            break;
        }
    }

    SvxEntries* pEntries = pToolbar->GetEntries();
    SvxEntries::const_iterator iter = pEntries->begin();

    for ( ; iter != pEntries->end(); iter++ )
    {
        SvxConfigEntry* pEntry = *iter;

        SvLBoxEntry* pNewLBEntry = InsertEntryIntoUI( pEntry );

        if (pEntry->IsBinding())
        {
            aContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        }
        else
        {
            aContentsListBox->SetCheckButtonState(
                pNewLBEntry, SV_BUTTON_TRISTATE );
        }
    }

    UpdateButtonStates();

    return 0;
}

IMPL_LINK( SvxToolbarConfigPage, NewToolbarHdl, Button *, pButton )
{
    String prefix =
        String( ResId( RID_SVXSTR_NEW_TOOLBAR, DIALOG_MGR() ) );

    OUString aNewName =
        generateCustomName( prefix, GetSaveInData()->GetEntries() );

    OUString aNewURL =
        generateCustomURL( GetSaveInData()->GetEntries() );

    SvxNewToolbarDialog* pNameDialog = new SvxNewToolbarDialog( 0, aNewName );

    USHORT nInsertPos;
    for ( sal_Int32 i = 0 ; i < aSaveInListBox.GetEntryCount(); i++ )
    {
        SaveInData* pData =
            (SaveInData*) aSaveInListBox.GetEntryData( i );

        nInsertPos = pNameDialog->aSaveInListBox.InsertEntry(
            aSaveInListBox.GetEntry( i ) );

        pNameDialog->aSaveInListBox.SetEntryData( nInsertPos, pData );
    }

    pNameDialog->aSaveInListBox.SelectEntryPos(
        aSaveInListBox.GetSelectEntryPos(), TRUE );

    bool ret = pNameDialog->Execute();
    if ( ret == RET_OK )
    {
        pNameDialog->GetName( aNewName );

        nInsertPos = pNameDialog->aSaveInListBox.GetSelectEntryPos();

        ToolbarSaveInData* pData = (ToolbarSaveInData*)
            pNameDialog->aSaveInListBox.GetEntryData( nInsertPos );

        SvxConfigEntry* pToolbar =
            new SvxConfigEntry( 1, aNewName, String(), TRUE );

        pToolbar->SetUserDefined( TRUE );
        pToolbar->SetMain( TRUE );
        pToolbar->SetCommand( aNewURL );

        pData->CreateToolbar( pToolbar );

        if ( GetSaveInData() != pData )
        {
            aSaveInListBox.SelectEntryPos( nInsertPos, TRUE );
            aSaveInListBox.GetSelectHdl().Call(this);
        }

        nInsertPos = aTopLevelListBox.InsertEntry( pToolbar->GetName() );
        aTopLevelListBox.SetEntryData( nInsertPos, pToolbar );
        aTopLevelListBox.SelectEntryPos( nInsertPos, TRUE );
        aTopLevelListBox.GetSelectHdl().Call(this);

        pData->SetModified( TRUE );
    }

    delete pNameDialog;

    return 0;
}

IMPL_LINK( SvxToolbarConfigPage, AddCommandsHdl, Button *, pButton )
{
    if ( pSelectorDlg == NULL )
    {
        // Create Script Selector which shows slot commands
        pSelectorDlg = new SvxScriptSelectorDialog( this, TRUE );

        // Position the Script Selector over the Add button so it is
        // beside the menu contents list and does not obscure it
        pSelectorDlg->SetPosPixel( aAddCommandsButton.GetPosPixel() );

        pSelectorDlg->SetAddHdl(
            LINK( this, SvxToolbarConfigPage, AddFunctionHdl ) );
    }

    pSelectorDlg->SetImageProvider(
        static_cast< ImageProvider* >( GetSaveInData() ) );

    pSelectorDlg->Show();
    return 1;
}

IMPL_LINK( SvxToolbarConfigPage, AddFunctionHdl,
    SvxScriptSelectorDialog *, pDialog )
{
    AddFunction();

    return 0;
}

SvLBoxEntry* SvxToolbarConfigPage::AddFunction(
    SvLBoxEntry* pTarget, bool bFront, bool bAllowDuplicates )
{
    SvLBoxEntry* pNewLBEntry =
        SvxConfigPage::AddFunction( pTarget, bFront, bAllowDuplicates );

    SvxConfigEntry* pEntry = (SvxConfigEntry*) pNewLBEntry->GetUserData();

    if ( pEntry->IsBinding() )
    {
        pEntry->SetVisible( TRUE );
        aContentsListBox->SetCheckButtonState(
            pNewLBEntry, SV_BUTTON_CHECKED );
    }
    else
    {
        aContentsListBox->SetCheckButtonState(
            pNewLBEntry, SV_BUTTON_TRISTATE );
    }

    // get currently selected toolbar and apply change
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if ( pToolbar != NULL )
    {
        ( ( ToolbarSaveInData* ) GetSaveInData() )->ApplyToolbar( pToolbar );
    }

    return pNewLBEntry;
}

// -----------------------------------------------------------------------

SvxToolbarEntriesListBox::SvxToolbarEntriesListBox(
    Window* pParent, const ResId& aResId )
    :
        SvxMenuEntriesListBox( pParent, aResId ),
        pPage( ( SvxConfigPage* ) pParent )
{
    m_pButtonData = new SvLBoxButtonData( this );
    BuildCheckBoxButtonImages( m_pButtonData );
    EnableCheckButton( m_pButtonData );

    m_bHiContrastMode = GetDisplayBackground().GetColor().IsDark();
}

// --------------------------------------------------------

SvxToolbarEntriesListBox::~SvxToolbarEntriesListBox()
{
    delete m_pButtonData;
}

// --------------------------------------------------------

void SvxToolbarEntriesListBox::BuildCheckBoxButtonImages( SvLBoxButtonData* pData )
{
    // Build checkbox images according to the current application
    // settings. This is necessary to be able to have correct colors
    // in all color modes, like high contrast.
    const AllSettings& rSettings = Application::GetSettings();

    VirtualDevice   aDev;
    Size            aSize( 26, 20 );

    aDev.SetOutputSizePixel( aSize );

    Image aImage = GetSizedImage( aDev, aSize,
        CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DEFAULT ));

    // Fill button data struct with new images
    pData->aBmps[SV_BMP_UNCHECKED]      = aImage;
    pData->aBmps[SV_BMP_CHECKED]        = GetSizedImage( aDev, aSize, CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_CHECKED ));
    pData->aBmps[SV_BMP_HICHECKED]      = GetSizedImage( aDev, aSize, CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_CHECKED | BUTTON_DRAW_PRESSED ));
    pData->aBmps[SV_BMP_HIUNCHECKED]    = GetSizedImage( aDev, aSize, CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DEFAULT | BUTTON_DRAW_PRESSED));
    pData->aBmps[SV_BMP_TRISTATE]       = GetSizedImage( aDev, aSize, Image() ); // Use tristate bitmaps to have no checkbox for separator entries
    pData->aBmps[SV_BMP_HITRISTATE]     = GetSizedImage( aDev, aSize, Image() );

    // Get image size
    m_aCheckBoxImageSizePixel = aImage.GetSizePixel();
}

Image SvxToolbarEntriesListBox::GetSizedImage(
    VirtualDevice& aDev, const Size& aNewSize, const Image& aImage )
{
    // Create new checkbox images for treelistbox. They must have a
    // decent width to have a clear column for the visibility checkbox.

    // Standard transparent color is light magenta as is won't be
    // used for other things
    Color   aFillColor( COL_LIGHTMAGENTA );

    // Position image at the center of (width-2),(height) rectangle.
    // We need 2 pixels to have a bigger border to the next button image
    USHORT  nPosX = std::max( (USHORT) (((( aNewSize.Width() - 2 ) - aImage.GetSizePixel().Width() ) / 2 ) - 1), (USHORT) 0 );
    USHORT  nPosY = std::max( (USHORT) (((( aNewSize.Height() - 2 ) - aImage.GetSizePixel().Height() ) / 2 ) + 1), (USHORT) 0 );
    Point   aPos( nPosX > 0 ? nPosX : 0, nPosY > 0 ? nPosY : 0 );
    aDev.SetFillColor( aFillColor );
    aDev.SetLineColor( aFillColor );
    aDev.DrawRect( Rectangle( Point(), aNewSize ));
    aDev.DrawImage( aPos, aImage );

    // Draw separator line 2 pixels left from the right border
    Color aLineColor = GetDisplayBackground().GetColor().IsDark() ? Color( COL_WHITE ) : Color( COL_BLACK );
    aDev.SetLineColor( aLineColor );
    aDev.DrawLine( Point( aNewSize.Width()-3, 0 ), Point( aNewSize.Width()-3, aNewSize.Height()-1 ));

    // Create new image that uses the fillcolor as transparent
    return Image( aDev.GetBitmap( Point(), aNewSize ), aFillColor );
}

void SvxToolbarEntriesListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );

    if (( rDCEvt.GetType() == DATACHANGED_SETTINGS ) &&
        ( rDCEvt.GetFlags() & SETTINGS_STYLE ))
    {
        if ( m_bHiContrastMode != GetDisplayBackground().GetColor().IsDark() )
        {
            // We have to reset all images because we change to/from high contrast mode
            m_bHiContrastMode = GetDisplayBackground().GetColor().IsDark();

            /*
            SfxImageManager*    pImgMgr = m_pBindings->GetImageManager();
            SvLBoxEntry*        pEntry  = NULL;
            for ( ULONG n = 0; n < GetEntryCount(); n++ )
            {
                pEntry = GetEntry( 0, n );
                SvxToolbarEntryInfo_Impl* pInfo =
                    (SvxToolbarEntryInfo_Impl*)pEntry->GetUserData();

                if ( pInfo )
                {
                    // Retrieve image for the correct color mode
                    Image aImage = pImgMgr->SeekImage( pInfo->nId, m_bHiContrastMode );
                    SetExpandedEntryBmp( pEntry, aImage );
                    SetCollapsedEntryBmp( pEntry, aImage );
                }
            }
            */
        }

        BuildCheckBoxButtonImages( m_pButtonData );
        Invalidate();
    }
}

// --------------------------------------------------------

void  SvxToolbarEntriesListBox::CheckButtonHdl()
{
    SvLBoxEntry* pEntry = GetHdlEntry();

    SvxConfigEntry* pEntryData =
        (SvxConfigEntry*) pEntry->GetUserData();

    pEntryData->SetVisible( !pEntryData->IsVisible() );

    SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();

    ToolbarSaveInData* pToolbarSaveInData = ( ToolbarSaveInData* )
        pPage->GetSaveInData();

    pToolbarSaveInData->ApplyToolbar( pToolbar );
}

BOOL SvxToolbarEntriesListBox::NotifyMoving(
    SvLBoxEntry* pTarget, SvLBoxEntry* pSource,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos)
{
    bool result = SvxMenuEntriesListBox::NotifyMoving(
        pTarget, pSource, rpNewParent, rNewChildPos );

    if ( result == TRUE )
    {
        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != NULL )
        {
            ToolbarSaveInData* pSaveInData =
                ( ToolbarSaveInData*) pPage->GetSaveInData();
            pSaveInData->ApplyToolbar( pToolbar );
        }
    }

    return result;
}

BOOL SvxToolbarEntriesListBox::NotifyCopying(
    SvLBoxEntry*  pTarget,
    SvLBoxEntry*  pSource,
    SvLBoxEntry*& rpNewParent,
    ULONG&      rNewChildPos)
{
    if ( !m_bIsInternalDrag )
    {
        ULONG target = pTarget == NULL ? 0 : GetModel()->GetAbsPos( pTarget );

        // if the target is NULL then add function to the start of the list
        ((SvxToolbarConfigPage*)pPage)->AddFunction( pTarget, pTarget == NULL );

        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != NULL )
        {
            ToolbarSaveInData* pSaveInData =
                ( ToolbarSaveInData*) pPage->GetSaveInData();
            pSaveInData->ApplyToolbar( pToolbar );
        }

        // AddFunction already adds the listbox entry so return FALSE
        // to stop another listbox entry being added
        return FALSE;
    }

    // Copying is only allowed from external controls, not within the listbox
    return FALSE;
}

SvxNewToolbarDialog::SvxNewToolbarDialog(
    Window* pWindow, const String& rName )
    :
    ModalDialog     ( pWindow, ResId( MD_NEW_TOOLBAR, DIALOG_MGR() ) ),
    aFtDescription  ( this, ResId( FT_DESCRIPTION ) ),
    aEdtName        ( this, ResId( EDT_STRING ) ),
    aSaveInText     ( this, ResId( TXT_SAVEIN ) ),
    aSaveInListBox  ( this, ResId( LB_SAVEIN ) ),
    aBtnOK          ( this, ResId( BTN_OK ) ),
    aBtnCancel      ( this, ResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ResId( BTN_HELP ) )
{
    FreeResource();

    aEdtName.SetText( rName );
    aEdtName.SetSelection(Selection(SELECTION_MIN, SELECTION_MAX));
    ModifyHdl(&aEdtName);
    aEdtName.SetModifyHdl(LINK(this, SvxNewToolbarDialog, ModifyHdl));
}

IMPL_LINK(SvxNewToolbarDialog, ModifyHdl, Edit*, pEdit)
{
    if(aCheckNameHdl.IsSet())
        aBtnOK.Enable(aCheckNameHdl.Call(this) > 0);

    return 0;
}

/*******************************************************************************
*
* The SvxIconSelectorDialog class
*
*******************************************************************************/
SvxIconSelectorDialog::SvxIconSelectorDialog( Window *pWindow,
    const uno::Reference< dcss::ui::XImageManager >& rXImageManager )
    :
    ModalDialog     ( pWindow, ResId( MD_ICONSELECTOR, DIALOG_MGR() ) ),
    aFtDescription  ( this, ResId( FT_SYMBOLS ) ),
    aTbSymbol       ( this, ResId( TB_SYMBOLS ) ),
    aFtNote         ( this, ResId( FT_NOTE ) ),
    aBtnOK          ( this, ResId( BTN_OK ) ),
    aBtnCancel      ( this, ResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ResId( BTN_HELP ) ),
    aBtnImport      ( this, ResId( BTN_IMPORT ) ),
    m_xImageManager ( rXImageManager )
{
    FreeResource();

    uno::Sequence< OUString > names =
        m_xImageManager->getAllImageNames( GetImageType() );

    // large growth factor, expecting many entries
    ImageList aImages( names.getLength(), 32 );

    USHORT nId = 1;
    uno::Sequence< OUString > name( 1 );
    for ( sal_Int32 i = 0; i < names.getLength(); ++i )
    {
        name[ 0 ] = names[ i ];

        uno::Sequence< uno::Reference< graphic::XGraphic> > graphics;

        try
        {
            graphics = m_xImageManager->getImages( GetImageType(), name );
        }
        catch ( uno::Exception& )
        {
            // can't get sequence for this name so it will not be
            // added to the list
        }

        if ( graphics.getLength() > 0 )
        {
            Image img = Image( graphics[ 0 ] );
            aTbSymbol.InsertItem( nId, names[ i ] );
            aTbSymbol.SetItemData(
                nId, static_cast< void * > ( graphics[ 0 ].get() ) );
            aImages.AddImage( nId, img );

            ++nId;
        }
    }

    // Set imagelist to the toolbox
    aTbSymbol.SetImageList( aImages );

    aTbSymbol.SetSelectHdl( LINK(this, SvxIconSelectorDialog, SelectHdl) );
    aBtnImport.SetClickHdl( LINK(this, SvxIconSelectorDialog, ImportHdl) );
}

uno::Reference< graphic::XGraphic> SvxIconSelectorDialog::GetSelectedIcon()
{
    uno::Reference< graphic::XGraphic > result;

    USHORT nId;
    for ( USHORT n = 0; n < aTbSymbol.GetItemCount(); n++ )
    {
        nId = aTbSymbol.GetItemId( n );
        if ( aTbSymbol.IsItemChecked( nId ) )
        {
            result = GetGraphic(m_xImageManager, aTbSymbol.GetItemText( nId ));
        }
    }

    return result;
}

IMPL_LINK( SvxIconSelectorDialog, SelectHdl, ToolBox *, pToolBox )
{
    USHORT nCount = aTbSymbol.GetItemCount();

    for (USHORT n = 0; n < nCount; n++ )
    {
        USHORT nId = aTbSymbol.GetItemId( n );

        if ( aTbSymbol.IsItemChecked( nId ) )
        {
            aTbSymbol.CheckItem( nId, FALSE );
        }
    }

    USHORT nId = aTbSymbol.GetCurItemId();
    aTbSymbol.CheckItem( nId );

    return 0;
}

IMPL_LINK( SvxIconSelectorDialog, ImportHdl, PushButton *, pButton )
{
    sfx2::FileDialogHelper aImportDialog(
        sfx2::FILEOPEN_SIMPLE, SFXWB_MULTISELECTION | WB_OPEN );
    static String* sLastVisited = NULL;

    if ( sLastVisited != NULL && sLastVisited->Len() != 0 )
    {
        aImportDialog.SetDisplayDirectory( *sLastVisited );
    }

    aImportDialog.AddFilter( String::CreateFromAscii( "PNG" ),
                             String::CreateFromAscii( "*.png" ));

    if ( ERRCODE_NONE == aImportDialog.Execute() )
    {
        // uno::Sequence< OUString > paths = aImportDialog.GetMPath();
        // ImportGraphics ( paths );
    }

    return 0;
}

void SvxIconSelectorDialog::ImportGraphics(
    const uno::Sequence< OUString >& rPaths )
{
    for ( sal_Int32 i = 0; i < rPaths.getLength(); i++ )
    {
        ImportGraphic( rPaths[ i ] );
    }
}

void SvxIconSelectorDialog::ImportGraphic( const OUString& aURL )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceManager =
        ::comphelper::getProcessServiceFactory();

    if ( xServiceManager.is() )
    {
        try
        {
            uno::Reference< graphic::XGraphicProvider > xGraphProvider(
                xServiceManager->createInstance(
                    ::rtl::OUString::createFromAscii(
                        "com.sun.star.graphic.GraphicProvider" ) ),
                    uno::UNO_QUERY );

            if ( xGraphProvider.is() )
            {
                uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
                aMediaProps[0].Name = ::rtl::OUString::createFromAscii( "URL" );
                aMediaProps[0].Value <<= aURL;

                uno::Reference< graphic::XGraphic > xGraphic =
                    xGraphProvider->queryGraphic( aMediaProps );

                if ( xGraphic.is() )
                {
                    Image aImage( xGraphic );

                    if ( !!aImage )
                    {
                        aTbSymbol.InsertItem( aTbSymbol.GetItemCount(),
                            aImage, aURL );
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
            OSL_TRACE("Some sort of exception importing graphic");
        }
    }
}
