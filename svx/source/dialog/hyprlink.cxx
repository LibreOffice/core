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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/configitem.hxx>
#include <unotools/cmdoptions.hxx>
#include <svtools/inetimg.hxx>
#include <svl/urlbmk.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/objsh.hxx>
#include <comphelper/processfactory.hxx>

#include "hlnkitem.hxx"
#include <svx/dialogs.hrc>
#include "hyprlink.hrc"
#include <svx/dialmgr.hxx>
#include "hyprlink.hxx"
#include "hyprdlg.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

// -----------------------------------------------------------------------

class SearchDefaultConfigItem_Impl : public ::utl::ConfigItem
{
    OUString    sDefaultEngine;
public:
    SearchDefaultConfigItem_Impl();
    ~SearchDefaultConfigItem_Impl();

    const OUString&    GetDefaultSearchEngine(){ return sDefaultEngine;}
    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void            Commit();
};

SearchDefaultConfigItem_Impl::SearchDefaultConfigItem_Impl() :
        ConfigItem(OUString(RTL_CONSTASCII_USTRINGPARAM("Inet/DefaultSearchEngine")))
{
    uno::Sequence<OUString> aNames(1);
    aNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
    uno::Sequence< uno::Any > aValues = GetProperties(aNames);
    aValues.getConstArray()[0] >>= sDefaultEngine;
}

SearchDefaultConfigItem_Impl::~SearchDefaultConfigItem_Impl()
{
}

void SearchDefaultConfigItem_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

void SearchDefaultConfigItem_Impl::Commit()
{
}

/*************************************************************************
|*
|* Dialog zum Einf"ugen/"Andern eines Hyperlink
|*
\************************************************************************/

SvxHyperlinkDlg::SvxHyperlinkDlg( SfxBindings *_pBindings, Window* pParent) :

    ToolBox             ( pParent, SVX_RES( RID_SVXDLG_HYPERLINK ) ),
    SfxControllerItem   ( SID_HYPERLINK_SETLINK, *_pBindings ),

    aForwarder          ( SID_HYPERLINK_GETLINK, *this ),
    aHyperlinkDlgForward( SID_HYPERLINK_DIALOG , *this),

    aNameCB             ( this, SVX_RES( CB_NAME ) ),
    aUrlFT              ( this, SVX_RES( FT_URL ) ),
    aUrlCB              ( this, SVX_RES( CB_URL ) ),
    aSearchConfig       (sal_True),
    sAddress            ( SVX_RES( STR_ADDRESS ) ),
    sExplorer           ( SVX_RES( STR_EXPLORER ) ),
    sSearchTitle        ( SVX_RES( STR_BOOKMARK_SEARCH ) ),
    aLinkPopup          ( SVX_RES( RID_SVXMN_HYPERLINK ) ),
    pTargetMenu         ( NULL ),

    bNoDoc              ( TRUE ),
    bSend               ( FALSE ),
    bHasOldName         ( FALSE ),
    bHtmlMode           ( FALSE )

{
    FreeResource();

    mpManager = SfxImageManager::GetImageManager( 0 );
    mpManager->RegisterToolBox( this );

    SetImages();

    // save initial size
    nMaxWidth = GetSizePixel().Width();
    nMaxHeight = GetSizePixel().Height();


    InsertSeparator( 0 );
    InsertWindow( CB_URL, &aUrlCB, 0, 0 );
    // we need an item text for accessibility
    String sItemText = aUrlCB.GetText();
    if ( sItemText.Len() == 0 )
        sItemText = aUrlCB.GetQuickHelpText();
    if ( sItemText.Len() > 0 )
    {
        SetItemText( CB_URL, sItemText );
        sItemText.Erase();
    }
    InsertWindow( FT_URL, &aUrlFT, 0, 0 );
    InsertSeparator( 0 );
    InsertWindow( CB_NAME, &aNameCB, 0, 0 );
    sItemText = aNameCB.GetText();
    if ( sItemText.Len() == 0 )
        sItemText = aNameCB.GetQuickHelpText();
    if ( sItemText.Len() > 0 )
        SetItemText( CB_NAME, sItemText );

    SetSizePixel(CalcWindowSizePixel());    // Groesse initialisieren

    nMaxHeight = GetSizePixel().Height();   // Hoehe nochmal merken, da sie veraendert wurde
                                            // SetSizePixel ruft Resize-Handler!

    Show();
    Resize();

    long nUrlWidth = aUrlCB.GetResizeWidth();
    long nNameWidth = aNameCB.GetResizeWidth();
    long nSum = nUrlWidth + nNameWidth;

    aUrlCB.SetRatio((nUrlWidth * 100L) / nSum);
    aNameCB.SetRatio((nNameWidth * 100L) / nSum);

    SetClickHdl( LINK( this, SvxHyperlinkDlg, TBClickHdl ) );
    SetSelectHdl( LINK( this, SvxHyperlinkDlg, TBSelectHdl ) );
    SetDropdownClickHdl( LINK( this, SvxHyperlinkDlg, DropdownClick ) );

    SetItemBits( BTN_TARGET, GetItemBits( BTN_TARGET ) | TIB_DROPDOWNONLY );
    SetItemBits( BTN_INET_SEARCH, GetItemBits( BTN_INET_SEARCH ) | TIB_DROPDOWN );
    SetItemBits( BTN_INET_SEARCH, GetItemBits( BTN_TARGET ) | TIB_DROPDOWNONLY );

    aLinkPopup.SetSelectHdl(LINK(this, SvxHyperlinkDlg, LinkPopupSelectHdl));

    Link aLk = LINK(this, SvxHyperlinkDlg, ComboSelectHdl);
    aNameCB.SetSelectHdl(aLk);
    aUrlCB.SetSelectHdl(aLk);

    aLk = LINK(this, SvxHyperlinkDlg, ComboModifyHdl);
    aNameCB.SetModifyHdl(aLk);
    aUrlCB.SetModifyHdl(aLk);

    // Accessibility: Set the quick help text as accessible name for the
    // drop down lists.
    aNameCB.SetAccessibleName (aNameCB.GetQuickHelpText());
    aUrlCB.SetAccessibleName (aUrlCB.GetQuickHelpText());

    // Hide button according to config item.
    SvtCommandOptions aCmdOpts;
    if ( aCmdOpts.Lookup( SvtCommandOptions::CMDOPTION_DISABLED,
                          rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM(
                                  "InternetSearch" ) ) ) )
        HideItem( BTN_INET_SEARCH );
}

SvxHyperlinkDlg::~SvxHyperlinkDlg()
{
    SfxImageManager::GetImageManager( 0 )->ReleaseToolBox(this);

    if (pTargetMenu != NULL)
        delete pTargetMenu;
}

/*--------------------------------------------------------------------
    Beschreibung: Leiste so resizen, dass die Comboboxen automatisch mit
    verkleinert werden
 --------------------------------------------------------------------*/

void SvxHyperlinkDlg::Resize()
{
    long nWidth = GetSizePixel().Width();

    ToolBox::Resize();

    if (nWidth) // nWidth ist manchmal 0
    {
        long nDeltaW = nMaxWidth - nWidth + aUrlCB.LogicToPixel(Size(3, 1)).Width() + 1;

        long nNewUrlWidth = aUrlCB.CalcResizeWidth(nDeltaW);
        long nNewNameWidth = aNameCB.CalcResizeWidth(nDeltaW);

        if (nNewUrlWidth && nNewNameWidth)  // Flackern reduzieren
        {
            SetUpdateMode(FALSE);

            // Comboboxen resizen
            aUrlCB.DoResize(nNewUrlWidth);
            aNameCB.DoResize(nNewNameWidth);
            RecalcItems();  // Alle Elemente neu anordnen

            SetUpdateMode(TRUE);
        }
    }
}

void SvxHyperlinkDlg::Resizing(Size& rSize)
{
    if (rSize.Height() > nMaxHeight)
        rSize.Height() = nMaxHeight;

    if (rSize.Width() > nMaxWidth)
        rSize.Width() = nMaxWidth;

    ToolBox::Resizing(rSize);
}

/*--------------------------------------------------------------------
    Beschreibung: Im Dokument selektierten Hyperlink in Leiste anzeigen
 --------------------------------------------------------------------*/

void SvxHyperlinkDlg::StateChanged( USHORT nSID, SfxItemState eState,
                                                    const SfxPoolItem* pState )
{
    if ( nSID == SID_HYPERLINK_DIALOG )
    {
        if( eState != SFX_ITEM_DISABLED)
        {
            EnableItem( BTN_OPENDIALOG, TRUE );

            BOOL bItem = FALSE;
            if ( pState && eState == SFX_ITEM_AVAILABLE )
                    bItem = ((SfxBoolItem*)pState)->GetValue();
            SetItemState ( BTN_OPENDIALOG, bItem ? STATE_CHECK : STATE_NOCHECK );
        }
        else
        {
            SetItemState ( BTN_OPENDIALOG, STATE_NOCHECK );
            EnableItem( BTN_OPENDIALOG, FALSE );
        }
    }

    if (nSID == SID_HYPERLINK_SETLINK)
    {
        if (eState == SFX_ITEM_DISABLED)
            bNoDoc = TRUE;
        else
            bNoDoc = FALSE;
        EnableItem(BTN_TARGET, !bNoDoc);
        EnableLink();
    }

    if (nSID == SID_HYPERLINK_GETLINK)
    {
        if (eState == SFX_ITEM_AVAILABLE)
        {
            const SvxHyperlinkItem& rHLnkItem = *((const SvxHyperlinkItem*)pState);

            USHORT nNamePos = aNameCB.GetEntryPos(aNameCB.GetText());
            USHORT nUrlPos = aUrlCB.GetEntryPos(aUrlCB.GetText());
            USHORT nNotFound = COMBOBOX_ENTRY_NOTFOUND;

            if (!bHasOldName &&
                (nNamePos == nNotFound || nUrlPos == nNotFound))
            {
                sOldName = aNameCB.GetText();
                bHasOldName = TRUE;
            }
            if (rHLnkItem.GetName().Len())
            {
                aNameCB.SetText(rHLnkItem.GetName());
                ComboModifyHdl(&aNameCB);
            }
            if (rHLnkItem.GetURL().Len() || rHLnkItem.GetName().Len())
            {
                String sUrl = INetURLObject(rHLnkItem.GetURL()).GetURLNoPass();
                aUrlCB.SetText(sUrl);
            }
            else if (aUrlCB.GetEntryCount())
            {   // Letzten Eintrag wieder selektieren
                aNameCB.SetText(aNameCB.GetEntry(0));
                aUrlCB.SetText(aUrlCB.GetEntry(0));
            }

            TargetMenu(rHLnkItem.GetTargetFrame(), FALSE);
            bHtmlMode = (rHLnkItem.GetInsertMode() & HLINK_HTMLMODE) != 0;
        }
        else
            return;

        ComboModifyHdl(&aUrlCB);
    }
}

IMPL_LINK( SvxHyperlinkDlg, TBClickHdl, ToolBox *, pBox )
{
    switch (pBox->GetCurItemId())
    {
        case BTN_LINK:
        {
            if (!bSend) // Link ins Dokument einfuegen
                SendToApp(HLINK_DEFAULT);
        }
        break;

        case BTN_OPENDIALOG:
        {
            GetBindings().GetDispatcher()->Execute( SID_HYPERLINK_DIALOG );
        }
        break;
    }

    return TRUE;
}

IMPL_LINK( SvxHyperlinkDlg, TBSelectHdl, ToolBox *, pBox )
{
    switch (pBox->GetCurItemId())
    {
        // Link als Bookmark im Explorer eintragen
        // Soll erst im Loslassen der Maus gerufen werden, daher im Select-Hdl
        case BTN_INSERT_BOOKMARK:
        {
            String sName = aNameCB.GetText();
            if ( !sName.Len() )
                sName = aUrlCB.GetText();

            String aBase = GetBindings().GetDispatcher()->GetFrame()->GetObjectShell()->GetMedium()->GetBaseURL();
            SfxStringItem aName( SID_BOOKMARK_TITLE, sName );
            SfxStringItem aURL( SID_BOOKMARK_URL,
                                URIHelper::SmartRel2Abs( INetURLObject(aBase), aUrlCB.GetText(), URIHelper::GetMaybeFileHdl(), true, false,
                                                          INetURLObject::WAS_ENCODED,
                                                          INetURLObject::DECODE_UNAMBIGUOUS ) );
            GetBindings().GetDispatcher()->Execute(
                SID_CREATELINK, SFX_CALLMODE_ASYNCHRON, &aName, &aURL, 0L );
        }
        break;
    }

    return TRUE;
}

IMPL_LINK( SvxHyperlinkDlg, DropdownClick, ToolBox *, pBox )
{
    switch (pBox->GetCurItemId())
    {
        case BTN_LINK:
        {
            // Link-Popup anstossen
            EndSelection(); // Vor dem Execute, damit Popup den Focus bekommt
            aLinkPopup.EnableItem(MN_BUTTON, !bHtmlMode);
            aLinkPopup.Execute( this, GetItemRect( BTN_LINK ), FLOATWIN_POPUPMODE_DOWN );
        }
        break;

        case BTN_INET_SEARCH:
        {
            // Search-Engines per Popup auswaehlen
            PopupMenu *pMenu = new PopupMenu;
            pMenu->SetSelectHdl(LINK(this, SvxHyperlinkDlg, SearchPopupSelectHdl));
            SearchDefaultConfigItem_Impl aDefaultEngine;
            String sDefault(aDefaultEngine.GetDefaultSearchEngine());
            sDefault.ToLowerAscii();
            const bool bHasDefault = sDefault.Len() > 0;

            sal_uInt16         nCount = aSearchConfig.Count();
            String sFound;
            for (USHORT i = 0; i < nCount; i++)
            {
                const SvxSearchEngineData& rData = aSearchConfig.GetData(i);
                //check if it's the configured default search engine
                String sTest(rData.sEngineName);
                sTest.ToLowerAscii();
                bool bIsDefaultEngine = bHasDefault && STRING_NOTFOUND != sTest.Search( sDefault );
                //then put it at the top
                if(i && bIsDefaultEngine)
                {
                    pMenu->InsertItem( i + 1, rData.sEngineName, 0, 0);
                    pMenu->InsertSeparator(1);
                }
                else
                {
                    if (i)
                        pMenu->InsertSeparator();
                    pMenu->InsertItem( i + 1, rData.sEngineName);
                }
            }
            pBox->SetItemDown(BTN_INET_SEARCH, TRUE, TRUE);
            pMenu->Execute( this, GetItemRect( BTN_INET_SEARCH ), FLOATWIN_POPUPMODE_DOWN );
            pBox->SetItemDown(BTN_INET_SEARCH, FALSE, TRUE);
            EndSelection();
            delete pMenu;
        }
        break;

        case BTN_TARGET:
        {
            // Target Frame einstellen
            TargetMenu(GetSelTarget(), TRUE);
            EndSelection();
        }
        break;
    }

    return TRUE;
}

void SvxHyperlinkDlg::TargetMenu(const String& rSelEntry, BOOL bExecute)
{
    if (pTargetMenu && !bExecute)
    {
        for (USHORT i = 1; i <= pTargetMenu->GetItemCount(); i++)
        {
            if (pTargetMenu->GetItemText(i) == rSelEntry)
            {
                pTargetMenu->CheckItem(i);
                return;
            }
        }
    }

    SfxViewFrame* pVwFrm = SfxViewFrame::Current();

    if (pVwFrm) // Alle moeglichen Target Frames zusammensammeln und anzeigen
    {
        TargetList aList;
        pVwFrm->GetTopFrame().GetTargetList(aList);

        USHORT nCount = (USHORT)aList.Count();
        if( nCount )
        {
            BOOL bChecked = FALSE;

            if (pTargetMenu != NULL)
                delete pTargetMenu;

            pTargetMenu = new PopupMenu;
            pTargetMenu->SetMenuFlags( pTargetMenu->GetMenuFlags() |
                                       MENU_FLAG_NOAUTOMNEMONICS );
            USHORT i;
            for ( i = 0; i < nCount; i++ )
            {
                String sEntry(*aList.GetObject(i));
                pTargetMenu->InsertItem(i + 1, sEntry, MIB_RADIOCHECK|MIB_AUTOCHECK);

                if (sEntry == rSelEntry)
                {
                    pTargetMenu->CheckItem(i + 1);
                    bChecked = TRUE;
                }
            }
            for ( i = nCount; i; i-- )
                delete aList.GetObject( i - 1 );

            if (!bChecked)
                pTargetMenu->CheckItem(1);

            if (bExecute)
            {
                USHORT nEntry = pTargetMenu->Execute(
                    this, GetItemRect( BTN_TARGET ), FLOATWIN_POPUPMODE_DOWN );
                if ( nEntry )
                    pTargetMenu->CheckItem( nEntry );
            }
        }
    }
}

IMPL_LINK( SvxHyperlinkDlg, LinkPopupSelectHdl, Menu *, pMenu )
{
    switch (pMenu->GetCurItemId())
    {
        case MN_FIELD:  // URL als Hyperlink ins Dok einfuegen
            SendToApp(HLINK_FIELD);
            break;
        case MN_BUTTON: // URL als Button ins Dok einfuegen
            SendToApp(HLINK_BUTTON);
            break;
    }

    return TRUE;
}

IMPL_LINK( SvxHyperlinkDlg, SearchPopupSelectHdl, Menu *, pMenu )
{
    const SvxSearchEngineData&  rData = aSearchConfig.GetData(pMenu->GetCurItemId() - 1);
    String sText(aNameCB.GetText());
    sText.EraseLeadingChars().EraseTrailingChars();

    rtl::OUString   sPrefix;
    rtl::OUString   sSuffix;
    rtl::OUString   sSeparator;
    sal_Int32       nCaseMatch;
    sal_Unicode cToken = 0;

    if (sText.Search(' ') != STRING_NOTFOUND)
    {
        sPrefix = rData.sExactPrefix;
        sSuffix = rData.sExactSuffix;
        sSeparator = rData.sExactSeparator;
        nCaseMatch = rData.nExactCaseMatch;
        cToken = ' ';
    }
    else if (sText.Search('+') != STRING_NOTFOUND)
    {
        sPrefix = rData.sAndPrefix;
        sSuffix = rData.sAndSuffix;
        sSeparator = rData.sAndSeparator;
        nCaseMatch = rData.nAndCaseMatch;
        cToken = '+';
    }
    else if (sText.Search(',') != STRING_NOTFOUND)
    {
        sPrefix = rData.sOrPrefix;
        sSuffix = rData.sOrSuffix;
        sSeparator = rData.sOrSeparator;
        nCaseMatch = rData.nOrCaseMatch;
        cToken = ',';
    }
    else
    {
        sPrefix = rData.sExactPrefix;
        sSuffix = rData.sExactSuffix;
        nCaseMatch = rData.nExactCaseMatch;
        cToken = 0;
    }

    String sURL = sPrefix;
    xub_StrLen nTok;

    if(1 == nCaseMatch)
        sText.ToUpperAscii();
    else if(2 == nCaseMatch)
        sText.ToLowerAscii();

    if ((nTok = sText.GetTokenCount(cToken)) > 1)
    {
        for (USHORT i = 0; i < nTok; i++)
        {
            sURL += sText.GetToken(i, cToken);
            if(i < nTok -1)
                sURL += (String)sSeparator;
        }
        sURL += (String)sSuffix;
    }
    else
    {
        sURL += sText;
        sURL += (String)sSuffix;
    }
    sURL.EraseAllChars();   // remove all spaces
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
        pViewFrame = pViewFrame->GetTopViewFrame();
    OpenDoc( sURL, pViewFrame );

    return TRUE;
}

IMPL_LINK( SvxHyperlinkDlg, BookmarkFoundHdl, String *, pUrl )
{
    if (pUrl && pUrl->Len())
    {
        aUrlCB.SetText(*pUrl);
        ComboModifyHdl(&aUrlCB);
    }

    return TRUE;
}
/*--------------------------------------------------------------------
    Beschreibung: Link-Button enablen/disablen
 --------------------------------------------------------------------*/

void SvxHyperlinkDlg::EnableLink()
{
    BOOL bEnable = aUrlCB.GetText().Len() != 0;

    EnableItem(BTN_LINK, (!bNoDoc) & bEnable);
}

/*--------------------------------------------------------------------
    Beschreibung: URL im Dokument einfuegen
 --------------------------------------------------------------------*/

void SvxHyperlinkDlg::SendToApp(USHORT nType)
{
    BOOL bIsFile = FALSE;
    bSend = TRUE;
    String sURL( aUrlCB.GetText() );

    if ( !sURL.Len() )
        return;

    String aBase = GetBindings().GetDispatcher()->GetFrame()->GetObjectShell()->GetMedium()->GetBaseURL();
    INetURLObject aObj( URIHelper::SmartRel2Abs( INetURLObject(aBase), sURL, URIHelper::GetMaybeFileHdl(), true, false,
                                                  INetURLObject::WAS_ENCODED,
                                                  INetURLObject::DECODE_UNAMBIGUOUS ) );
    sURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
    if ( aObj.GetProtocol() == INET_PROT_FILE )
        bIsFile = TRUE;

    if ( bIsFile )
    {
        EnterWait();
        SfxMedium aMedium( sURL, STREAM_STD_READ, TRUE );
        if ( aMedium.Exists( FALSE ) == FALSE )
        {
            LeaveWait();
            QueryBox aBox( this, SVX_RES( RID_SVXQB_DONTEXIST ) );
            if ( aBox.Execute() == RET_NO )
                return;
        }
        else
            LeaveWait();
    }

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK );

    if (aNameCB.GetText().Len())
        aItem.SetName( aNameCB.GetText() );
    else
        aItem.SetName( sURL );

    sOldName = aNameCB.GetText();

    aItem.SetURL(sURL);
    aItem.SetInsertMode( (SvxLinkInsertMode)nType );
    aItem.SetTargetFrame( GetSelTarget() );

    GetBindings().GetDispatcher()->Execute(
        SID_HYPERLINK_SETLINK, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    AddToHistory( aNameCB.GetText(), sURL );

    if ( sURL != aUrlCB.GetText() )
        aUrlCB.SetText( sURL );
}

/*--------------------------------------------------------------------
    Beschreibung: Selektierten Target Frame Eintrag im Popup ermitteln
 --------------------------------------------------------------------*/

String SvxHyperlinkDlg::GetSelTarget()
{
    String sTarget;

    if (pTargetMenu != NULL)
    {
        for (USHORT i = 1; i <= pTargetMenu->GetItemCount(); i++)
        {
            if (pTargetMenu->IsItemChecked(i))
            {
                sTarget = pTargetMenu->GetItemText(i);
                break;
            }
        }
    }

    return sTarget;
}

/*--------------------------------------------------------------------
    Beschreibung: URL in History der Leiste aufnehmen
 --------------------------------------------------------------------*/

void SvxHyperlinkDlg::AddToHistory(const String& rName, const String& rURL)
{
    String sName(rName);

    if (bHasOldName && sOldName.Len())
    {
        sName = sOldName;
        bHasOldName = FALSE;
    }

    if (!sName.Len())
        sName = rURL;

    if (rURL.Len())
    {
        USHORT nNamePos = aNameCB.GetEntryPos(sName);
        USHORT nUrlPos = aUrlCB.GetEntryPos(rURL);
        USHORT nPos = COMBOBOX_ENTRY_NOTFOUND;

        if (nNamePos != COMBOBOX_ENTRY_NOTFOUND)
            nPos = nNamePos;
        else
            nPos = nUrlPos;

        // Alten Eintrag durch neuen Eintrag ersetzen
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            aNameCB.RemoveEntry(nPos);
            aUrlCB.RemoveEntry(nPos);
            aNameCB.SetText(rName);
            aUrlCB.SetText(rURL);
        }

        aNameCB.InsertEntry(sName, 0);
        aUrlCB.InsertEntry(rURL, 0);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Bookmark und Search-Button enablen/disablen
 --------------------------------------------------------------------*/

IMPL_LINK( SvxHyperlinkDlg, ComboSelectHdl, ComboBox *, pCombo )
{
    USHORT nPos = pCombo->GetEntryPos(pCombo->GetText());

    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
    {
        aNameCB.SetText(aNameCB.GetEntry(nPos));
        aUrlCB.SetText(aUrlCB.GetEntry(nPos));

        EnableLink();
        EnableItem(BTN_INSERT_BOOKMARK, TRUE);
        EnableItem(BTN_INET_SEARCH, TRUE);
    }
    return TRUE;
}

IMPL_LINK( SvxHyperlinkDlg, ComboModifyHdl, ComboBox *, pCombo )
{
    BOOL bEnable = TRUE;

    if (!pCombo->GetText().Len())
        bEnable = FALSE;

    if (pCombo == &aNameCB)
    {
        if (IsItemEnabled(BTN_INET_SEARCH) != bEnable)
            EnableItem(BTN_INET_SEARCH, bEnable);
    }

    EnableLink();

    if (aUrlCB.GetText().Len())
        bEnable = TRUE;
    else
        bEnable = FALSE;

    if (IsItemEnabled(BTN_INSERT_BOOKMARK) != bEnable)
        EnableItem(BTN_INSERT_BOOKMARK, bEnable);

    return TRUE;
}

// --------------------------------------------------------------------

void SvxHyperlinkDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE))
        SetImages();
}

// --------------------------------------------------------------------

void SvxHyperlinkDlg::SetImages()
{
    bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    SetItemImage( BTN_LINK, mpManager->GetImage( BTN_LINK, bHighContrast ) );
    SetItemImage( BTN_INSERT_BOOKMARK, mpManager->GetImage( BTN_INSERT_BOOKMARK, bHighContrast ) );
    SetItemImage( BTN_INET_SEARCH, mpManager->GetImage( BTN_INET_SEARCH, bHighContrast ) );
    SetItemImage( BTN_TARGET, mpManager->GetImage( BTN_TARGET, bHighContrast ) );
    SetItemImage( BTN_OPENDIALOG, mpManager->GetImage( BTN_OPENDIALOG, bHighContrast ) );
}

/*--------------------------------------------------------------------
    Beschreibung: Comboboxen
 --------------------------------------------------------------------*/

HyperCombo::HyperCombo( SvxHyperlinkDlg* pDialog, const ResId& rResId ) :
        ComboBox( (Window *)pDialog, rResId )
{
    pDlg = pDialog;

    nMaxWidth = GetSizePixel().Width();
    if (this == &pDlg->aUrlCB)
        nMinWidth = GetTextWidth(String::CreateFromAscii("http://www.stardiv.dewww"));
    else
        nMinWidth = GetTextWidth(String::CreateFromAscii("StarDivision GmbHwww"));
}

/*--------------------------------------------------------------------
    Beschreibung: Comboboxen KeyInput
 --------------------------------------------------------------------*/

long HyperCombo::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;
    static BOOL bLocked = FALSE;

    if (bLocked)    // Keine weiteren RETURNs annehmen (nicht Reentrant!)
        return nHandled;

    bLocked = TRUE;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();

        if (aKeyCode.GetCode() == KEY_RETURN)
        {
            pDlg->SendToApp(HLINK_DEFAULT);
            nHandled = 1;
        }
    }
    if (this == &pDlg->aNameCB)
        pDlg->sOldName = GetText();

    if (!nHandled)
        nHandled = ComboBox::Notify( rNEvt );

    bLocked = FALSE;

    return nHandled;
}

/*--------------------------------------------------------------------
    Beschreibung: Drag&Drop vor dem Combo-Edit abfangen und weiterleiten
 --------------------------------------------------------------------*/

long HyperCombo::PreNotify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    nHandled = ComboBox::PreNotify( rNEvt );

    return nHandled;
}

long HyperCombo::CalcResizeWidth( long nW )
{
    long nNewWidth = Max(nMaxWidth - nW * GetRatio() / 100L, nMinWidth);

    if (nNewWidth > nMaxWidth)
        nNewWidth = nMaxWidth;

    if (nNewWidth != GetSizePixel().Width())
        return nNewWidth;
    else
        return 0;   // Kein Resize notwendig
}

void HyperCombo::DoResize( long nNewWidth )
{
    SetSizePixel(Size(nNewWidth, GetSizePixel().Height()));
}

/*--------------------------------------------------------------------
    Beschreibung: FixedText
 --------------------------------------------------------------------*/

HyperFixedText::HyperFixedText( SvxHyperlinkDlg* pDialog, const ResId& rResId ) :
        FixedInfo( (Window *)pDialog, rResId )
{
    pDlg = pDialog;
}

/*--------------------------------------------------------------------
    Beschreibung: Webseite der Search-Engine mit Suchergebnissen anzeigen
 --------------------------------------------------------------------*/
void SvxHyperlinkDlg::OpenDoc( const String& rURL, SfxViewFrame* pViewFrame )
{
    SfxStringItem aName( SID_FILE_NAME, rURL );
    SfxStringItem aReferer( SID_REFERER, String::CreateFromAscii("private:user") );
    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, TRUE );
    SfxBoolItem aSilent( SID_SILENT, TRUE );
    SfxBoolItem aReadOnly( SID_DOC_READONLY, TRUE );
    SfxBoolItem aExternal( SID_BROWSE, TRUE );
    SfxDispatcher* pDisp = SfxViewFrame::Current() ? SfxViewFrame::Current()->GetDispatcher() : NULL;

    if ( pViewFrame )
    {
        SfxFrameItem aView( SID_DOCFRAME, pViewFrame ? &pViewFrame->GetFrame() : NULL );
        if ( pDisp )
            pDisp->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                            &aName, &aView, &aNewView, &aSilent, &aReadOnly, &aReferer, &aExternal, 0L );
    }
    else if ( pDisp )
        pDisp->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                        &aName, &aNewView, &aSilent, &aReadOnly, &aReferer, 0L );
}
// class SvxHyperlinkDialogWrapper ------------------------------------------

SFX_IMPL_CHILDWINDOW(SvxHyperlinkDlgWrapper, SID_HYPERLINK_INSERT)

/*--------------------------------------------------------------------
    Beschreibung: Wrapper fuer Hyperlinkleiste
 --------------------------------------------------------------------*/

SvxHyperlinkDlgWrapper::SvxHyperlinkDlgWrapper( Window* _pParent, USHORT nId,
                                                SfxBindings* _pBindings, SfxChildWinInfo* /*pInfo*/ ) :

    SfxChildWindow( _pParent, nId )

{
    pWindow = new SvxHyperlinkDlg( _pBindings, _pParent );
    eChildAlignment = SFX_ALIGN_TOP;
}

SfxChildWinInfo SvxHyperlinkDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
