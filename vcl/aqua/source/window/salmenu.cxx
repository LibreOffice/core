/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 16:32:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <saldata.hxx>
#include <salinst.h>
#include <salmenu.h>
#include <vcl/impbmp.hxx>
#include <vcl/svids.hrc>


// nPos is 0 based, MenuItemIndex is 1 based
#define GETITEMINDEXBYPOS(nPos) (nPos+1)

// =======================================================================

SalMenu* AquaSalInstance::CreateMenu( BOOL bMenuBar )
{
    AquaSalMenu *pAquaSalMenu = new AquaSalMenu();
    static MenuID inMenuID=0;

    pAquaSalMenu->mbMenuBar = bMenuBar;

    CreateNewMenu (inMenuID++, 0, &pAquaSalMenu->mrMenuRef);

    /* Associate the pointer to this SalMenu with the menu */
    SetMenuItemProperty (pAquaSalMenu->mrMenuRef, 0,
                         APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_POINTER,
                         sizeof(pAquaSalMenu), &pAquaSalMenu);

    static bool bOnce = true;
    if( bOnce )
    {
        bOnce = false;
        EnableMenuCommand(NULL, kHICommandPreferences); // enable pref menu

        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            String aAbout( ResId( SV_STDTEXT_ABOUT, *pMgr ) );
            MenuRef rApplicationMenu;
            MenuItemIndex outIndex[1];
            GetIndMenuItemWithCommandID(NULL, kHICommandPreferences, 1, &rApplicationMenu, outIndex);
            CFStringRef rStr = CreateCFString( aAbout );
            if ( rStr )
            {
                InsertMenuItemTextWithCFString(rApplicationMenu, rStr, (short) 0, 0, kHICommandAbout);
                CFRelease( rStr );
            }
        }
    }
    return pAquaSalMenu;
}

void AquaSalInstance::DestroyMenu( SalMenu* pSalMenu )
{

    AquaSalMenu *pAquaSalMenu = (AquaSalMenu *) pSalMenu;

    RemoveMenuItemProperty (pAquaSalMenu->mrMenuRef, 0,
                            APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_POINTER);
    delete pSalMenu;
}

SalMenuItem* AquaSalInstance::CreateMenuItem( const SalItemParams* pItemData )
{
    if( !pItemData )
        return NULL;

    AquaSalMenuItem *pSalMenuItem = new AquaSalMenuItem();

    pSalMenuItem->maMenuAttributes = 0;
    pSalMenuItem->mpSubMenu = NULL;
    pSalMenuItem->mnId  = pItemData->nId;
    pSalMenuItem->mText = pItemData->aText;

    // Delete mnemonics
    pSalMenuItem->mText.EraseAllChars( '~' );

    pSalMenuItem->mpMenu = pItemData->pMenu;

    if (pItemData->eType == MENUITEM_SEPARATOR)
        pSalMenuItem->maMenuAttributes |= kMenuItemAttrSeparator;

    return pSalMenuItem;
}

void AquaSalInstance::DestroyMenuItem( SalMenuItem* pSalMenuItem )
{
    delete pSalMenuItem;
}


// =======================================================================


/*
 * AquaSalMenu
 */

AquaSalMenu::~AquaSalMenu()
{
    if (this->mrMenuRef)
    {
        DisposeMenu(this->mrMenuRef);
    }
}

BOOL AquaSalMenu::VisibleMenuBar()
{
    // Enable/disable experimental native menus code?
    //
    // To disable native menus, set the environment variable AQUA_NATIVE_MENUS to FALSE

    static const char *pExperimental = getenv ("AQUA_NATIVE_MENUS");

    if ( pExperimental && !strcasecmp(pExperimental, "FALSE") )
        return FALSE;

    // End of experimental code enable/disable part

    return TRUE;
}

void AquaSalMenu::SetFrame( const SalFrame *pFrame )
{
    AquaLog( ">*>_> %s\n",__func__);
}

void AquaSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;

    // save MenuRef of the menu for future reference
    pAquaSalMenuItem->mrParentMenuRef = mrMenuRef;

    if (mbMenuBar)
        AppendMenuItemTextWithCFString( mrMenuRef, NULL, pAquaSalMenuItem->maMenuAttributes, 0, &pAquaSalMenuItem->mnMenuItemIndex);
    else
    {
        CFStringRef menuText = CreateCFString(pAquaSalMenuItem->mText);
        if ( menuText )
        {
            AppendMenuItemTextWithCFString( mrMenuRef, menuText, pAquaSalMenuItem->maMenuAttributes, 0, &pAquaSalMenuItem->mnMenuItemIndex);
            CFRelease(menuText);
        }
    }

    /* Associate the pointer to this SalMenuItem with the menu item */
    SetMenuItemProperty (mrMenuRef, pAquaSalMenuItem->mnMenuItemIndex,
                         APP_PROPERTY_CREATOR, APP_PROPERTY_TAG_MENU_ITEM_POINTER,
                         sizeof(pSalMenuItem), &pSalMenuItem);
}

void AquaSalMenu::RemoveItem( unsigned nPos )
{
    if (mrMenuRef)
        DeleteMenuItem (mrMenuRef, GETITEMINDEXBYPOS(nPos));
}

void AquaSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;

    AquaSalMenu *subAquaSalMenu = (AquaSalMenu *) pSubMenu;

    // FIXME: in svtools - workben - svdem, pSubMenu is NULL!
    if (subAquaSalMenu)
    {
        CFStringRef menuText = CreateCFString(pAquaSalMenuItem->mText);
        if ( menuText )
        {
            pAquaSalMenuItem->mpSubMenu = pSubMenu;
            SetMenuTitleWithCFString(subAquaSalMenu->mrMenuRef, menuText);
            CFRelease(menuText);
        }
        SetMenuItemHierarchicalMenu (mrMenuRef, pAquaSalMenuItem->mnMenuItemIndex, subAquaSalMenu->mrMenuRef);
    }
}

void AquaSalMenu::CheckItem( unsigned nPos, BOOL bCheck )
{
    CheckMenuItem(this->mrMenuRef, GETITEMINDEXBYPOS(nPos), bCheck);
}

void AquaSalMenu::EnableItem( unsigned nPos, BOOL bEnable )
{
    if (bEnable)
        EnableMenuItem(this->mrMenuRef, GETITEMINDEXBYPOS(nPos));
    else
        DisableMenuItem(this->mrMenuRef, GETITEMINDEXBYPOS(nPos));
}

void AquaSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage )
{
    AquaLog( ">*>_> %s\n",__func__);

    if (!pSalMenuItem)
        return;

    AquaSalMenuItem *pAquaSalMenuItem = static_cast<AquaSalMenuItem *>(pSalMenuItem);

    BitmapEx aBitmapEx = rImage.GetBitmapEx();
    aBitmapEx.Mirror(BMP_MIRROR_VERT);

    pAquaSalMenuItem->maBitmap = aBitmapEx.GetBitmap();

    Bitmap aBitmap (pAquaSalMenuItem->maBitmap);

    ImpBitmap *aImpBitmap = aBitmap.ImplGetImpBitmap();

    if (aImpBitmap)
    {
        AquaSalBitmap *pSalBmp = static_cast<AquaSalBitmap *>(aImpBitmap->ImplGetSalBitmap());
        CGImageRef xImage = NULL;

        // check alpha
        if (aBitmapEx.IsAlpha())
        {
            AquaSalBitmap *pSalMask = static_cast<AquaSalBitmap *>(aBitmapEx.GetAlpha().GetBitmap().ImplGetImpBitmap()->ImplGetSalBitmap());

            if (pSalMask)
                xImage = pSalBmp->CreateWithMask ( *pSalMask, 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight);
        }

        if (!xImage)
            xImage = pSalBmp->CreateCroppedImage( 0, 0, pSalBmp->mnWidth, pSalBmp->mnHeight );

        SetMenuItemIconHandle(mrMenuRef, pAquaSalMenuItem->mnMenuItemIndex, kMenuCGImageRefType, (Handle)xImage);

        CGImageRelease(xImage);
    }
}

void AquaSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )
{
    if (!pSalMenuItem)
        return;

    AquaSalMenuItem *pAquaSalMenuItem = (AquaSalMenuItem *) pSalMenuItem;

    pAquaSalMenuItem->mText = rText;

    // Delete mnemonics
    pAquaSalMenuItem->mText.EraseAllChars( '~' );

    CFStringRef menuText = CreateCFString(pAquaSalMenuItem->mText);
    if ( menuText )
    {
        SetMenuItemTextWithCFString( mrMenuRef, pAquaSalMenuItem->mnMenuItemIndex, menuText);

        // if the menu item has a submenu, change its title as well
        if (pAquaSalMenuItem->mpSubMenu)
        {
            AquaSalMenu *subMenu = (AquaSalMenu *) pAquaSalMenuItem->mpSubMenu;
            SetMenuTitleWithCFString(subMenu->mrMenuRef, menuText);
        }
        CFRelease(menuText);
    }
}

void AquaSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName )
{
    AquaLog( ">*>_> %s\n",__func__);

    USHORT nModifier;
    int nCommandKey = -1, nKeyGlyph = -1;

    if (rKeyCode.GetCode())
    {
        USHORT nKeyCode=rKeyCode.GetCode();

        if ((nKeyCode>=KEY_A) && (nKeyCode<=KEY_Z))           // letter A..Z
            nCommandKey=nKeyCode-KEY_A+'A';
        else if ((nKeyCode>=KEY_0) && (nKeyCode<=KEY_9))      // numbers 0..9
            nCommandKey=nKeyCode-KEY_0+'0';
        else if ((nKeyCode>=KEY_F1) && (nKeyCode<=KEY_F12))   // function keys F1..F12
            nKeyGlyph=kMenuF1Glyph+nKeyCode-KEY_F1;           // only for n<13!
        else if ((nKeyCode>=KEY_F13) && (nKeyCode<=KEY_F15))
                nKeyGlyph=kMenuF13Glyph+nKeyCode-KEY_F13;     // function keys F13, F14, F15
#if 0
        else if (nKeyCode==KEY_SPACE)
            nKeyGlyph=kMenuSpaceGlyph;
#endif
        else
            switch (nKeyCode)
            {
            case KEY_ADD:
                nCommandKey='+';
                break;
            case KEY_SUBTRACT:
                nCommandKey='-';
                break;
            case KEY_MULTIPLY:
                nCommandKey='*';
                break;
            case KEY_DIVIDE:
                nCommandKey='/';
                break;
            case KEY_POINT:
                nCommandKey='.';
                break;
            case KEY_LESS:
                nCommandKey='<';
                break;
            case KEY_GREATER:
                nCommandKey='>';
                break;
            case KEY_EQUAL:
                nCommandKey='=';
                break;
            }

        if (nCommandKey > -1)
            SetMenuItemCommandKey (mrMenuRef, GETITEMINDEXBYPOS(nPos), 0, nCommandKey);

        if (nKeyGlyph > -1)
            SetMenuItemKeyGlyph (mrMenuRef, GETITEMINDEXBYPOS(nPos), nKeyGlyph);

        if((nKeyGlyph <= -1) && (nCommandKey <= -1))
            AquaLog( "%s: Couldn't process rKeyCode.GetCode()=%d\n",__func__, rKeyCode.GetCode());
    }

    nModifier=rKeyCode.GetAllModifier();

    int nItemModifier = kMenuNoCommandModifier;

    if (nModifier & KEY_MOD1)
        nItemModifier = kMenuNoModifiers; // for the cmd key
        // = and not += because should replace kMenuNoCommandModifier

    if (nModifier & KEY_SHIFT)
        nItemModifier += kMenuShiftModifier;

    if (nModifier & KEY_MOD5)
        //should use mod3 or 2
        nItemModifier += kMenuOptionModifier; // for the ALT key

    if(nModifier & KEY_MOD2)
        nItemModifier += kMenuControlModifier; // maybe for the ctrl key

    SetMenuItemModifiers (mrMenuRef, GETITEMINDEXBYPOS(nPos), nItemModifier);
}

void AquaSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    AquaLog( ">*>_> %s\n",__func__);
}

// =======================================================================

/*
 * SalMenuItem
 */

AquaSalMenuItem::~AquaSalMenuItem()
{
}

// -------------------------------------------------------------------
