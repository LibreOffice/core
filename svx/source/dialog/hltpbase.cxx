/*************************************************************************
 *
 *  $RCSfile: hltpbase.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-13 17:03:18 $
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

#ifndef _SV_DRAG_HXX
#include <vcl/drag.hxx>
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

SvxFramesComboBox::SvxFramesComboBox ( Window* pParent, WinBits nStyle )
:   ComboBox (pParent, nStyle)
{
    GetCurrentFrameNames ();
}

SvxFramesComboBox::SvxFramesComboBox ( Window* pParent, const ResId& rResId )
:   ComboBox (pParent, rResId)
{
    GetCurrentFrameNames ();
}

SvxFramesComboBox::~SvxFramesComboBox ()
{
}

/*************************************************************************
|*
|* Fill list with curretn known framenames
|*
|************************************************************************/

BOOL SvxFramesComboBox::GetCurrentFrameNames ()
{
    TargetList* pList = new TargetList;
    const SfxFrame* pFrame = SFX_APP()->GetViewFrame()->GetTopFrame();
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

    return pFrame!=NULL;
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
: SfxURLBox         ( pParent, eSmart ),
  mbAccessAddress   (bAddresses)
{}

BOOL SvxHyperURLBox::QueryDrop( DropEvent& rEvt )
{
    for( int i=0; i<DragServer::GetItemCount(); i++)
    {
        if ( DragServer::HasFormat( i, FORMAT_STRING ) )
            /*|| ( DragServer::HasFormat( i, SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) && mbAccessAddress ) )*/
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL SvxHyperURLBox::Drop( const DropEvent &rEvt )
{
    if( rEvt.IsDefaultAction() )
        ( (DropEvent&)rEvt).SetAction( DROP_COPY );

    BOOL bDone = FALSE;
    USHORT nCount = DragServer::GetItemCount();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        if( !bDone && DragServer::HasFormat( i, FORMAT_STRING ) )
        {
            SetText( DragServer::PasteString(i) );
            bDone = TRUE;
            break;
        }
        /*
        Falls Adresse aus Datenbank gedragged werden soll....
        if( !bDone && mbAccessAddress &&
            DragServer::HasFormat( i, SOT_FORMATSTR_ID_SBA_DATAEXCHANGE ) )
        {
            String aAddress = GetAllEmailNamesFromDragItem(i);

            if ( aAddress.Len() )
            {
                SetText( aAddress );
                bDone = TRUE;
                break;
            }
        }
        */
    }

    return TRUE;
}

/*
Diese Methode parsed eine EMailadresse aus einem D&D-DataObjekt aus der Adreßdatenbank heraus

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
    SfxURLBox::Select();
}
void SvxHyperURLBox::Modify()
{
    SfxURLBox::Modify();
}
long SvxHyperURLBox::Notify( NotifyEvent& rNEvt )
{
    return SfxURLBox::Notify( rNEvt );
}
long SvxHyperURLBox::PreNotify( NotifyEvent& rNEvt )
{
    // Drag & Drop
    if( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == EVENT_QUERYDROP )
    {
        rNEvt.SetReturnValue ( (long) QueryDrop ( *(rNEvt.GetDropEvent() ) ) );
        return  1;
    } else if( rNEvt.GetWindow() == GetSubEdit() && rNEvt.GetType() == EVENT_DROP )
    {
        return (long) Drop( *(rNEvt.GetDropEvent() ) );
    }

    return SfxURLBox::PreNotify( rNEvt );
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
    mbNewName               ( FALSE ),
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
        mpGrpMore     = new GroupBox            ( this, ResId (GRP_MORE) );
        mpFtFrame     = new FixedText           ( this, ResId (FT_FRAME) );
        mpCbbFrame    = new SvxFramesComboBox   ( this, ResId (CB_FRAME) );
        mpFtForm      = new FixedText           ( this, ResId (FT_FORM) );
        mpLbForm      = new ListBox             ( this, ResId (LB_FORM) );
        mpFtIndication= new FixedText           ( this, ResId (FT_INDICATION) );
        mpEdIndication= new Edit                ( this, ResId (ED_INDICATION) );
        mpFtText      = new FixedText           ( this, ResId (FT_TEXT) );
        mpEdText      = new Edit                ( this, ResId (ED_TEXT) );
        mpBtScript    = new ImageButton         ( this, ResId (BTN_SCRIPT) );

        mpBtScript->SetClickHdl ( LINK ( this, SvxHyperlinkTabPageBase, ClickScriptHdl_Impl ) );
        mpEdIndication->SetModifyHdl( LINK ( this, SvxHyperlinkTabPageBase, ModifiedIndicationHdl_Impl ) );
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

    // Size of dialog-window
    Point aDlgPos ( mpDialog->GetPosPixel () );
    Size aDlgSize ( mpDialog->GetSizePixel () );

    // Size of Office-Main-Window
    Size aWindowSize( SFX_APP()->GetTopWindow()->GetSizePixel() );

    // Size of Extrawindow
    Size aExtraWndSize( mpMarkWnd->GetSizePixel () );

    if( aDlgPos.X()+(1.02*aDlgSize.Width())+aExtraWndSize.Width() > aWindowSize.Width() )
    {
        if( aDlgPos.X() - ( 0.02*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
        {
            // Pos Extrawindow anywhere
            MoveToExtraWnd( Point( 1, long(1.1*aDlgPos.Y()) ) );
            mpMarkWnd->ConnectToDialog( FALSE );
        }
        else
        {
            // Pos Extrawindow on the left side of Dialog
            MoveToExtraWnd( aDlgPos - Point( long(0.02*aDlgSize.Width()), 0 ) - Point( aExtraWndSize.Width(), 0 ) );
        }
    }
    else
    {
        // Pos Extrawindow on the right side of Dialog
        MoveToExtraWnd ( aDlgPos + Point( long(1.02*aDlgSize.getWidth()), 0 ) );
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
|* Modified indication-text
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkTabPageBase, ModifiedIndicationHdl_Impl , void *, EMPTYARG )
{
    // if I have edited the indication, I don't want the dialog
    // do set the indication automatically !

    mbNewName = FALSE;

    return 0L;
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
        SfxSmallMacroTabPage *pMacroPage = (SfxSmallMacroTabPage*) aDlg.GetTabPage();

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
            Content     aCnt( rURL.GetMainURL(), ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
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
