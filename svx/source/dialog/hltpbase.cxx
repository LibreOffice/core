/*************************************************************************
 *
 *  $RCSfile: hltpbase.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 11:39:11 $
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

#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#include <sfx2/sfxsids.hrc>
#define ITEMID_MACRO SID_ATTR_MACROITEM
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#include "hyperdlg.hrc"
#ifndef _SVX_TAB_HYPERLINK_HXX
#include "hyperdlg.hxx"
#endif
#include "hltpbase.hxx"

using namespace ucb;

//########################################################################
//#                                                                      #
//# ComboBox-Control, wich is filled with all current framenames         #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxFramesComboBox::SvxFramesComboBox ( Window* pParent, const ResId& rResId,
                                        SfxDispatcher* pDispatch )
:   ComboBox (pParent, rResId)
{
    TargetList* pList = new TargetList;
    SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
    SfxFrame* pFrame = pViewFrame ? pViewFrame->GetTopFrame() : 0;
    if ( pFrame )
    {
        pFrame->GetTargetList(*pList);
        USHORT nCount = (USHORT)pList->Count();
        if( nCount )
        {
            USHORT i;
            for ( i = 0; i < nCount; i++ )
            {
                InsertEntry(*pList->GetObject(i));
            }
            for ( i = nCount; i; i-- )
            {
                delete pList->GetObject( i - 1 );
            }
        }
        delete pList;
    }
}

SvxFramesComboBox::~SvxFramesComboBox ()
{
}
//########################################################################
//#                                                                      #
//# ComboBox-Control for URL's with History and Autocompletion           #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperURLBox::SvxHyperURLBox( Window* pParent, INetProtocol eSmart, BOOL bAddresses )
: SvtURLBox         ( pParent, eSmart ),
  DropTargetHelper  ( this ),
  mbAccessAddress   (bAddresses)
{
}

sal_Int8 SvxHyperURLBox::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( IsDropFormatSupported( FORMAT_STRING ) ? DND_ACTION_COPY : DND_ACTION_NONE );
}

sal_Int8 SvxHyperURLBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
    String                  aString;
    sal_Int8                nRet = DND_ACTION_NONE;

    if( aDataHelper.GetString( FORMAT_STRING, aString ) )
    {
        SetText( aString );
        nRet = DND_ACTION_COPY;
    }

    return nRet;
}

/*
Diese Methode parsed eine EMailadresse aus einem D&D-DataObjekt aus der Adreﬂdatenbank heraus

#ifndef _OFF_OFADBMGR_HXX
#include <offmgr/ofadbmgr.hxx>
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _SDB_SDBCURS_HXX
#include <sdb/sdbcurs.hxx>
#endif

#define DB_DD_DELIM         ((char)11)

String SvxHyperURLBox::GetAllEmailNamesFromDragItem( USHORT nItem )
{
#if !defined( ICC ) && !defined( SOLARIS )
    String aAddress;

    if (  DragServer::HasFormat( nItem, SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) )
    {
        ULONG nLen = DragServer::GetDataLen( nItem, SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) - 1;
        USHORT i = 0;
        String aTxt;
        char *pTxt = aTxt.AllocStrBuf( (USHORT)nLen );
        DragServer::PasteData( nItem, pTxt, nLen, SOT_FORMATSTR_ID_SBA_DATAEXCHANGE );
        String aDBName = aTxt.GetToken( i++, DB_DD_DELIM );
        String aTableName = aTxt.GetToken( i++, DB_DD_DELIM );
        i++;    // Format"anderung
        String aStatement = aTxt.GetToken( i++, DB_DD_DELIM );
        if ( !aStatement )
        {
            aDBName += DB_DELIM;
            aDBName += aTableName;
        }
        else
        {
            // use here another delimiter, because the OfaDBMgr uses two
            // delimiters ( (char)11 and ; )
            aDBName += ';';
            aDBName += aStatement;
        }
        SbaSelectionListRef pSelectionList;
        pSelectionList.Clear();
        pSelectionList = new SbaSelectionList;
        USHORT nTokCnt = aTxt.GetTokenCount( DB_DD_DELIM );

        for ( ; i < nTokCnt; ++i )
            pSelectionList->Insert(
                (void*)(USHORT)aTxt.GetToken( i, DB_DD_DELIM ), LIST_APPEND );

        OfaDBMgr* pDBMgr = OFF_APP()->GetOfaDBMgr();
        BOOL bBasic = DBMGR_STD;

        if ( !pDBMgr->OpenDB( bBasic, aDBName ) )
            return aAddress;

        OfaDBParam& rParam = pDBMgr->GetDBData( bBasic );
        pDBMgr->ChangeStatement( bBasic, aStatement );
        rParam.pSelectionList->Clear();

        if ( pSelectionList.Is() &&
                (long)pSelectionList->GetObject(0) != -1L )
            *rParam.pSelectionList = *pSelectionList;

        if ( !pDBMgr->ToFirstSelectedRecord( bBasic ) )
        {
            pDBMgr->CloseAll();
            return aAddress;
        }

        if ( pDBMgr->IsSuccessful( bBasic ) )
        {
            //  Spaltenk"opfe
            SbaDBDataDefRef aDBDef = pDBMgr->OpenColumnNames( bBasic );

            if ( aDBDef.Is() )
            {
                SbaColumn* pCol = aDBDef->GetColumn("EMAIL");
                ULONG nPos = 0;

                if ( pCol )
                    nPos = aDBDef->GetOriginalColumns().GetPos( pCol ) + 1;
                for ( i = 0 ; nPos && i < pSelectionList->Count(); ++i )
                {
                    ULONG nIndex = (ULONG)pSelectionList->GetObject(i);

                    // N"achsten zu lesenden Datensatz ansteuern

                    BOOL bEnd = rParam.GetCursor()->IsOffRange();

                    if ( !bEnd )
                    {
                        const ODbRowRef aRow =
                            pDBMgr->GetSelectedRecord( bBasic, i );

                        if ( aRow.Is() )
                        {
                            aAddress += pDBMgr->
                                ImportDBField( (USHORT)nPos, &aDBDef, aRow.getBodyPtr() );
                            aAddress += ',';
                        }
                        else
                        {
                            aAddress.Erase();
                            break;
                        }
                    }
                    else
                        break;
                }
            }
        }
        pDBMgr->CloseAll();
    }
    aAddress.EraseTrailingChars( ',' );
    return aAddress;
#else
    return String();
#endif
}
*/

void SvxHyperURLBox::Select()
{
    SvtURLBox::Select();
}
void SvxHyperURLBox::Modify()
{
    SvtURLBox::Modify();
}
long SvxHyperURLBox::Notify( NotifyEvent& rNEvt )
{
    return SvtURLBox::Notify( rNEvt );
}
long SvxHyperURLBox::PreNotify( NotifyEvent& rNEvt )
{
    return SvtURLBox::PreNotify( rNEvt );
}

//########################################################################
//#                                                                      #
//# Hyperlink-Dialog: Tabpages-Baseclass                                 #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Con/Destructor, Initialize
|*
\************************************************************************/

SvxHyperlinkTabPageBase::SvxHyperlinkTabPageBase ( Window *pParent,
                                                   const ResId &rResId,
                                                   const SfxItemSet& rItemSet)
:   mpGrpMore               ( NULL ),
    mpFtFrame               ( NULL ),
    mpCbbFrame              ( NULL ),
    mpFtForm                ( NULL ),
    mpLbForm                ( NULL ),
    mpFtIndication          ( NULL ),
    mpEdIndication          ( NULL ),
    mpFtText                ( NULL ),
    mpEdText                ( NULL ),
    mpBtScript              ( NULL ),
    IconChoicePage          ( pParent, rResId, rItemSet ),
    mpDialog                ( pParent ),
    mbStdControlsInit       ( FALSE )
{
    // create bookmark-window
    mpMarkWnd = new SvxHlinkDlgMarkWnd ( this );
}

SvxHyperlinkTabPageBase::~SvxHyperlinkTabPageBase ()
{
    maTimer.Stop();

    if ( mbStdControlsInit )
    {
        delete mpGrpMore;
        delete mpFtFrame;
        delete mpCbbFrame;
        delete mpFtForm;
        delete mpLbForm;
        delete mpFtIndication;
        delete mpEdIndication;
        delete mpFtText;
        delete mpEdText ;
        delete mpBtScript;
    }

    delete mpMarkWnd;
}

void SvxHyperlinkTabPageBase::InitStdControls ()
{
    if ( !mbStdControlsInit )
    {
        mpGrpMore     = new FixedLine           ( this, ResId (GRP_MORE) );
        mpFtFrame     = new FixedText           ( this, ResId (FT_FRAME) );
        mpCbbFrame    = new SvxFramesComboBox   ( this, ResId (CB_FRAME), GetDispatcher() );
        mpFtForm      = new FixedText           ( this, ResId (FT_FORM) );
        mpLbForm      = new ListBox             ( this, ResId (LB_FORM) );
        mpFtIndication= new FixedText           ( this, ResId (FT_INDICATION) );
        mpEdIndication= new Edit                ( this, ResId (ED_INDICATION) );
        mpFtText      = new FixedText           ( this, ResId (FT_TEXT) );
        mpEdText      = new Edit                ( this, ResId (ED_TEXT) );
        mpBtScript    = new ImageButton         ( this, ResId (BTN_SCRIPT) );

        mpBtScript->SetClickHdl ( LINK ( this, SvxHyperlinkTabPageBase, ClickScriptHdl_Impl ) );

        mpBtScript->SetModeImage( Image( ResId( IMG_SCRIPT_HC ) ), BMP_COLOR_HIGHCONTRAST );
        mpBtScript->EnableTextDisplay (FALSE);
    }

    mbStdControlsInit = TRUE;
}

/*************************************************************************
|*
|* Move Extra-Window
|*
\************************************************************************/

BOOL SvxHyperlinkTabPageBase::MoveToExtraWnd( Point aNewPos, BOOL bDisConnectDlg )
{
    BOOL bReturn =  mpMarkWnd->MoveTo ( aNewPos );

    if( bDisConnectDlg )
        mpMarkWnd->ConnectToDialog( FALSE );

    return ( !bReturn && IsMarkWndVisible() );
}

/*************************************************************************
|*
|* Show Extra-Window
|*
\************************************************************************/

void SvxHyperlinkTabPageBase::ShowMarkWnd ()
{
    ( ( Window* ) mpMarkWnd )->Show();

    // Size of dialog-window in screen pixels
    Rectangle aDlgRect( mpDialog->GetWindowExtentsRelative( NULL ) );
    Point aDlgPos ( aDlgRect.TopLeft() );
    Size aDlgSize ( mpDialog->GetSizePixel () );

    // Absolute size of the screen
    Rectangle aScreen( mpDialog->GetDesktopRectPixel() );

    // Size of Extrawindow
    Size aExtraWndSize( mpMarkWnd->GetSizePixel () );

    // mpMarkWnd is a child of mpDialog, so coordinates for positioning must be relative to mpDialog
    if( aDlgPos.X()+(1.05*aDlgSize.Width())+aExtraWndSize.Width() > aScreen.Right() )
    {
        if( aDlgPos.X() - ( 0.05*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
        {
            // Pos Extrawindow anywhere
            MoveToExtraWnd( Point(10,10) );  // very unlikely
            mpMarkWnd->ConnectToDialog( FALSE );
        }
        else
        {
            // Pos Extrawindow on the left side of Dialog
            MoveToExtraWnd( Point(0,0) - Point( long(0.05*aDlgSize.Width()), 0 ) - Point( aExtraWndSize.Width(), 0 ) );
        }
    }
    else
    {
        // Pos Extrawindow on the right side of Dialog
        MoveToExtraWnd ( Point( long(1.05*aDlgSize.getWidth()), 0 ) );
    }

    // Set size of Extra-Window
    mpMarkWnd->SetSizePixel( Size( aExtraWndSize.Width(), aDlgSize.Height() ) );
}

/*************************************************************************
|*
|* Fill Dialogfields
|*
\************************************************************************/

void SvxHyperlinkTabPageBase::FillStandardDlgFields ( SvxHyperlinkItem* pHyperlinkItem )
{
    // Frame
    USHORT nPos = mpCbbFrame->GetEntryPos ( pHyperlinkItem->GetTargetFrame() );
    if ( nPos != LISTBOX_ENTRY_NOTFOUND)
        mpCbbFrame->SetText ( pHyperlinkItem->GetTargetFrame() );

    // Form
    String aStrFormText = SVX_RESSTR( RID_SVXSTR_HYPERDLG_FROM_TEXT );
    String aStrFormButton = SVX_RESSTR( RID_SVXSTR_HYPERDLG_FORM_BUTTON );

    if( pHyperlinkItem->GetInsertMode() & HLINK_HTMLMODE )
    {
        mpLbForm->Clear();
        mpLbForm->InsertEntry( aStrFormText );
        mpLbForm->SelectEntryPos ( 0 );
    }
    else
    {
        mpLbForm->Clear();
        mpLbForm->InsertEntry( aStrFormText );
        mpLbForm->InsertEntry( aStrFormButton );
        mpLbForm->SelectEntryPos ( pHyperlinkItem->GetInsertMode() == HLINK_BUTTON ? 1 : 0 );
    }

    // URL
    mpEdIndication->SetText ( pHyperlinkItem->GetName() );

    // Name
    mpEdText->SetText ( pHyperlinkItem->GetIntName() );

    // Script-button
    if ( !pHyperlinkItem->GetMacroEvents() )
        mpBtScript->Disable();
    else
        mpBtScript->Enable();
}

/*************************************************************************
|*
|* Any action to do after apply-button is pressed
|*
\************************************************************************/

void SvxHyperlinkTabPageBase::DoApply ()
{
    // default-implemtation : do nothing
}

/*************************************************************************
|*
|* Ask page whether an insert is possible
|*
\************************************************************************/

BOOL SvxHyperlinkTabPageBase::AskApply ()
{
    // default-implementation
    return TRUE;
}

/*************************************************************************
|*
|* This method would be called from bookmark-window to set new mark-string
|*
\************************************************************************/

void SvxHyperlinkTabPageBase::SetMarkStr ( String& aStrMark )
{
    // default-implemtation : do nothing
}

/*************************************************************************
|*
|* This method will be called from the dialog-class if the state off
|* the online-mode has changed.
|*
\************************************************************************/

void SvxHyperlinkTabPageBase::SetOnlineMode( BOOL bEnable )
{
    // default-implemtation : do nothing
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkTabPageBase::SetInitFocus()
{
    GrabFocus();
}

/*************************************************************************
|*
|* Ask dialog whether the curretn doc is a HTML-doc
|*
|************************************************************************/

const BOOL SvxHyperlinkTabPageBase::IsHTMLDoc() const
{
    return ((SvxHpLinkDlg*)mpDialog)->IsHTMLDoc();
}

/*************************************************************************
|*
|* retrieve dispatcher
|*
|************************************************************************/

SfxDispatcher* SvxHyperlinkTabPageBase::GetDispatcher() const
{
    return ((SvxHpLinkDlg*)mpDialog)->GetDispatcher();
}

/*************************************************************************
|*
|* Click on imagebutton : Script
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkTabPageBase, ClickScriptHdl_Impl, void *, EMPTYARG )
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem->GetMacroEvents() )
    {
        // get macros from itemset
        const SvxMacroTableDtor* pMacroTbl = pHyperlinkItem->GetMacroTbl();
        SvxMacroItem aItem ( GetWhich(SID_ATTR_MACROITEM) );
        if( pMacroTbl )
            aItem.SetMacroTable( *pMacroTbl );

        // create empty itemset for macro-dlg
        SfxItemSet* pItemSet = new SfxItemSet(SFX_APP()->GetPool(),
                                              SID_ATTR_MACROITEM,
                                              SID_ATTR_MACROITEM );
        pItemSet->Put ( aItem, SID_ATTR_MACROITEM );

        SfxMacroAssignDlg aDlg (this, *pItemSet );

        // add events
        SfxMacroTabPage *pMacroPage = (SfxMacroTabPage*) aDlg.GetTabPage();

        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSEOVER_OBJECT )
            pMacroPage->AddEvent( String( SVX_RESSTR(RID_SVXSTR_HYPDLG_MACROACT1) ),
                                  SFX_EVENT_MOUSEOVER_OBJECT );
        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSECLICK_OBJECT )
            pMacroPage->AddEvent( String( SVX_RESSTR(RID_SVXSTR_HYPDLG_MACROACT2) ),
                                  SFX_EVENT_MOUSECLICK_OBJECT);
        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSEOUT_OBJECT )
            pMacroPage->AddEvent( String( SVX_RESSTR(RID_SVXSTR_HYPDLG_MACROACT3) ),
                                  SFX_EVENT_MOUSEOUT_OBJECT);

        // execute dlg
        if ( RET_OK == aDlg.Execute() )
        {
            const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, FALSE, &pItem ))
            {
                pHyperlinkItem->SetMacroTable( ((SvxMacroItem*)pItem)->GetMacroTable() );
            }
        }

        delete pItemSet;
    }

    return( 0L );
}

/*************************************************************************
|*
|* Get Macro-Infos
|*
|************************************************************************/

USHORT SvxHyperlinkTabPageBase::GetMacroEvents()
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return pHyperlinkItem->GetMacroEvents();
}

SvxMacroTableDtor* SvxHyperlinkTabPageBase::GetMacroTable()
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return ( (SvxMacroTableDtor*)pHyperlinkItem->GetMacroTbl() );
}

/*************************************************************************
|*
|* Does the given file exists ?
|*
|************************************************************************/

BOOL SvxHyperlinkTabPageBase::FileExists( const INetURLObject& rURL )
{
    BOOL bRet = FALSE;

    if( rURL.GetFull().Len() > 0 )
    {
        try
        {
            Content     aCnt( rURL.GetMainURL( INetURLObject::NO_DECODE ), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
            ::rtl::OUString aTitle;

            aCnt.getPropertyValue( ::rtl::OUString::createFromAscii( "Title" ) ) >>= aTitle;
            bRet = ( aTitle.getLength() > 0 );
        }
        catch( ... )
        {
            DBG_ERROR( "FileExists: ucb error" );
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* try to detect the current protocol that is used in aStrURL
|*
|************************************************************************/

String SvxHyperlinkTabPageBase::GetSchemeFromURL( String aStrURL )
{
    String aStrScheme;

    INetURLObject aURL( aStrURL );
    INetProtocol aProtocol = aURL.GetProtocol();

    // #77696#
    // our new INetUrlObject now has the ability
    // to detect if an Url is valid or not :-(
    if ( aProtocol == INET_PROT_NOT_VALID )
    {
        if ( aStrURL.EqualsIgnoreCaseAscii( INET_HTTP_SCHEME, 0, 7 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_HTTP_SCHEME );
        }
        else if ( aStrURL.EqualsIgnoreCaseAscii( INET_HTTPS_SCHEME, 0, 8 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_HTTPS_SCHEME );
        }
        else if ( aStrURL.EqualsIgnoreCaseAscii( INET_FTP_SCHEME, 0, 6 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_FTP_SCHEME );
        }
        else if ( aStrURL.EqualsIgnoreCaseAscii( INET_MAILTO_SCHEME, 0, 7 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_MAILTO_SCHEME );
        }
        else if ( aStrURL.EqualsIgnoreCaseAscii( INET_NEWS_SCHEME, 0, 5 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_NEWS_SCHEME );
        }
        else if ( aStrURL.EqualsIgnoreCaseAscii( INET_TELNET_SCHEME, 0, 9 ) )
        {
            aStrScheme = String::CreateFromAscii( INET_TELNET_SCHEME );
        }
    }
    else
        aStrScheme = INetURLObject::GetScheme( aProtocol );
    return aStrScheme;
}


void SvxHyperlinkTabPageBase::GetDataFromCommonFields( String& aStrName,
                                             String& aStrIntName, String& aStrFrame,
                                             SvxLinkInsertMode& eMode )
{
    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( UINT16(eMode) | HLINK_HTMLMODE );
}

/*************************************************************************
|*
|* reset dialog-fields
|*
|************************************************************************/

void SvxHyperlinkTabPageBase::Reset( const SfxItemSet& rItemSet)
{
    ///////////////////////////////////////
    // Set dialog-fields from create-itemset
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (String&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();
    }
}

/*************************************************************************
|*
|* Fill output-ItemSet
|*
|************************************************************************/

BOOL SvxHyperlinkTabPageBase::FillItemSet( SfxItemSet& rOut)
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);
    if ( !aStrName.Len() ) //automatically create a visible name if the link is created without name
        aStrName = CreateUiNameFromURL(aStrURL);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut.Put (aItem);

    return TRUE;
}

String SvxHyperlinkTabPageBase::CreateUiNameFromURL( const String& aStrURL )
{
    String          aStrUiURL;
    INetURLObject   aURLObj( aStrURL );

    switch(aURLObj.GetProtocol())
    {
        case INET_PROT_FILE:
            utl::LocalFileHelper::ConvertURLToSystemPath( aURLObj.GetMainURL(INetURLObject::NO_DECODE), aStrUiURL );
            break;
        case INET_PROT_FTP :
            {
                //remove password from name
                INetURLObject   aTmpURL(aURLObj);
                aTmpURL.SetPass(aEmptyStr);
                aStrUiURL = aTmpURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            }
            break;
        default :
            {
                aStrUiURL = aURLObj.GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS);
            }
    }
    if(!aStrUiURL.Len())
        return aStrURL;
    return aStrUiURL;
}

/*************************************************************************
|*
|* Activate / Deactivate Tabpage
|*
|************************************************************************/

void SvxHyperlinkTabPageBase::ActivatePage( const SfxItemSet& rItemSet )
{
    ///////////////////////////////////////
    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);
    }

    // show mark-window if it was open before
    if ( ShouldOpenMarkWnd () )
        ShowMarkWnd ();
}

int SvxHyperlinkTabPageBase::DeactivatePage( SfxItemSet* pSet)
{
    // hide mark-wnd
    SetMarkWndShouldOpen( IsMarkWndVisible () );
    HideMarkWnd ();

    // retrieve data of dialog
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    if( pSet )
    {
        SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                                aStrIntName, eMode, nEvents, pTable );
        pSet->Put( aItem );
    }

    return( LEAVE_PAGE );
}
