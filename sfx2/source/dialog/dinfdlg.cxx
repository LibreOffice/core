/*************************************************************************
 *
 *  $RCSfile: dinfdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:30 $
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

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. RET_OK
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#include <svtools/urihelper.hxx>

#pragma hdrstop

#include "dinfdlg.hxx"
#include "sfxresid.hxx"
#include "dinfedt.hxx"
#include "frame.hxx"
#include "viewfrm.hxx"
#include "request.hxx"
#include "expfile.hxx"
#include "inimgr.hxx"
#include "exptypes.hxx"
#include "helper.hxx"

#include "sfx.hrc"
#include "dinfdlg.hrc"
#include "sfxlocal.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxDocumentInfoItem, SfxStringItem);

//------------------------------------------------------------------------

String ConvertDateTime_Impl( const SfxStamp& rStamp )
{
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     const International& rInter = Application::GetAppInternational();
     String aStr( rInter.GetDate( rStamp.GetTime() ) );
     aStr += pDelim;
     aStr += rInter.GetTime( rStamp.GetTime(), TRUE, FALSE );
     String aAuthor = rStamp.GetName();
     aAuthor.EraseLeadingChars();
     if ( aAuthor.Len() )
     {
        aStr += pDelim;
        aStr += aAuthor;
     }
     return aStr;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem() :

    SfxStringItem()

{
     bOwnFormat = FALSE;
     bHasTemplate = TRUE;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem( const String& rFile, const SfxDocumentInfo& rInfo ) :

     SfxStringItem( SID_DOCINFO, rFile ),

     aDocInfo( rInfo )

{
     bOwnFormat = FALSE;
     bHasTemplate = TRUE;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem( const String& rFile,
                                          const SfxDocumentInfo& rInfo, BOOL bOwn ) :

    SfxStringItem( SID_DOCINFO, rFile ),

    aDocInfo( rInfo )

{
    bOwnFormat = bOwn;
    bHasTemplate = TRUE;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::SfxDocumentInfoItem( const SfxDocumentInfoItem& rItem ) :

    SfxStringItem( rItem ),

    aDocInfo( rItem.aDocInfo )

{
    bOwnFormat = rItem.bOwnFormat;
    bHasTemplate = rItem.bHasTemplate;
}

//------------------------------------------------------------------------

SfxDocumentInfoItem::~SfxDocumentInfoItem()
{
}

//------------------------------------------------------------------------

SfxPoolItem* SfxDocumentInfoItem::Clone( SfxItemPool * ) const
{
    return new SfxDocumentInfoItem( *this );
}

//------------------------------------------------------------------------

int SfxDocumentInfoItem::operator==( const SfxPoolItem& rItem) const
{
     return rItem.Type() == Type() &&
         SfxStringItem::operator==(rItem) &&
         aDocInfo == ((const SfxDocumentInfoItem &)rItem).aDocInfo;
}


//------------------------------------------------------------------------

BOOL SfxDocumentInfoItem::IsOwnFormat() const
{
     return bOwnFormat;
}

//------------------------------------------------------------------------

SfxDocumentDescPage::SfxDocumentDescPage( Window * pParent, const SfxItemSet &rItemSet)
     : SfxTabPage( pParent, SfxResId(TP_DOCINFODESC), rItemSet),
     aTitleFt( this, ResId( FT_TITLE ) ),
     aThemaFt( this, ResId( FT_THEMA ) ),
     aKeywordsFt( this, ResId( FT_KEYWORDS ) ),
     aCommentFt( this, ResId( FT_COMMENT ) ),
     aTitleEd( this, ResId( ED_TITLE ) ),
     aThemaEd( this, ResId( ED_THEMA ) ),
     aKeywordsEd( this, ResId( ED_KEYWORDS ) ),
     aCommentEd( this, ResId( ED_COMMENT ) ),
     pInfoItem(0)
{
     FreeResource();
}

//------------------------------------------------------------------------

SfxTabPage *SfxDocumentDescPage::Create(Window *pParent, const SfxItemSet &rItemSet)
{
     return new SfxDocumentDescPage(pParent, rItemSet);
}

//------------------------------------------------------------------------

BOOL SfxDocumentDescPage::FillItemSet(SfxItemSet &rSet)
{
    // Pruefung, ob eine Aenderung vorliegt
    const BOOL bTitleMod = aTitleEd.IsModified();
    const BOOL bThemeMod = aThemaEd.IsModified();
    const BOOL bKeywordsMod = aKeywordsEd.IsModified();
    const BOOL bCommentMod = aCommentEd.IsModified();
    if( !( bTitleMod || bThemeMod || bKeywordsMod || bCommentMod ) )
    {
        return FALSE;
    }

    // Erzeugung der Ausgabedaten
    const SfxPoolItem *pItem;
    SfxDocumentInfoItem *pInfo;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet &&
         SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
    {
        pInfo = pInfoItem;
    }
    else
    {
        pInfo = new SfxDocumentInfoItem(
            *( const SfxDocumentInfoItem *) pItem );
    }
    SfxDocumentInfo aInfo( (*pInfo)() );
    if( bTitleMod )
    {
        aInfo.SetTitle( aTitleEd.GetText() );
    }
    if( bThemeMod )
    {
        aInfo.SetTheme( aThemaEd.GetText() );
    }
    if( bKeywordsMod )
    {
        aInfo.SetKeywords( aKeywordsEd.GetText() );
    }
    if( bCommentMod )
    {
        aInfo.SetComment( aCommentEd.GetText() );
    }
    rSet.Put( SfxDocumentInfoItem( pInfo->GetValue(), aInfo ) );
    if( pInfo != pInfoItem )
    {
        delete pInfo;
    }

    return TRUE;
}

//------------------------------------------------------------------------

void SfxDocumentDescPage::Reset(const SfxItemSet &rSet)
{
     pInfoItem = &(SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);
     const SfxDocumentInfo& rInfo = ( *pInfoItem )();

     aTitleEd.SetText( rInfo.GetTitle() );
     aThemaEd.SetText( rInfo.GetTheme() );
     aKeywordsEd.SetText( rInfo.GetKeywords() );
     aCommentEd.SetText( rInfo.GetComment() );

     if ( rInfo.IsReadOnly() )
     {
        aTitleEd.SetReadOnly( TRUE );
        aThemaEd.SetReadOnly( TRUE );
        aKeywordsEd.SetReadOnly( TRUE );
        aCommentEd.SetReadOnly( TRUE );
    }
}

//------------------------------------------------------------------------

SfxDocumentPage::SfxDocumentPage( Window* pParent, const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFODOC ), rItemSet ),

    aBmp1           ( this, ResId( BMP_FILE_1 ) ),
    aNameED         ( this, ResId( ED_FILE_NAME ) ),

    aLine1FL        ( this, ResId( FL_FILE_1 ) ),
    aTypeFT         ( this, ResId( FT_FILE_TYP ) ),
    aShowTypeFT     ( this, ResId( FT_FILE_SHOW_TYP ) ),
    aReadOnlyCB     ( this, ResId( CB_FILE_READONLY ) ),
    aFileFt         ( this, ResId( FT_FILE ) ),
    aFileValFt      ( this, ResId( FT_FILE_VAL ) ),
    aSizeFT         ( this, ResId( FT_FILE_SIZE ) ),
    aShowSizeFT     ( this, ResId( FT_FILE_SHOW_SIZE ) ),

    aLine2FL        ( this, ResId( FL_FILE_2 ) ),
    aCreateFt       ( this, ResId( FT_CREATE ) ),
    aCreateValFt    ( this, ResId( FT_CREATE_VAL ) ),
    aTimeLogFt      ( this, ResId( FT_TIMELOG ) ),
    aTimeLogValFt   ( this, ResId( FT_TIMELOG_VAL ) ),
    aChangeFt       ( this, ResId( FT_CHANGE ) ),
    aChangeValFt    ( this, ResId( FT_CHANGE_VAL ) ),
    aDocNoFt        ( this, ResId( FT_DOCNO ) ),
    aDocNoValFt     ( this, ResId( FT_DOCNO_VAL ) ),
    aPrintValFt     ( this, ResId( FT_PRINT_VAL ) ),
    aPrintFt        ( this, ResId( FT_PRINT ) ),
    aDeleteBtn      ( this, ResId( BTN_DELETE ) ),
    aUseUserDataCB  ( this, ResId( CB_USE_USERDATA ) ),

    aLine3FL        ( this, ResId( FL_FILE_3 ) ),
    aTemplFt        ( this, ResId( FT_TEMPL ) ),
    aTemplValFt     ( this, ResId( FT_TEMPL_VAL ) ),

    aUnknownSize    ( ResId( STR_UNKNOWNSIZE ) ),

    bEnableUseUserData  ( FALSE ),
    bHandleDelete       ( FALSE )

{
    FreeResource();
    aDeleteBtn.SetClickHdl( LINK( this, SfxDocumentPage, DeleteHdl ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SfxDocumentPage, DeleteHdl, PushButton*, EMPTYARG )
{
    SfxStamp aCreated;
    if ( bEnableUseUserData && aUseUserDataCB.IsChecked() )
        aCreated.SetName( SFX_INIMANAGER()->GetUserFullName() );
    aCreateValFt.SetText( ConvertDateTime_Impl( aCreated ) );
    XubString aEmpty;
    aChangeValFt.SetText( aEmpty );
    aPrintValFt.SetText( aEmpty );
    const Time aTime( 0 );
    const International& rInter = Application::GetAppInternational();
    aTimeLogValFt.SetText( rInter.GetDuration( aTime ) );
    aDocNoValFt.SetText( '1' );
    bHandleDelete = TRUE;
    return 0;
}

//------------------------------------------------------------------------

SfxTabPage* SfxDocumentPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
     return new SfxDocumentPage( pParent, rItemSet );
}

//------------------------------------------------------------------------

void SfxDocumentPage::EnableUseUserData()
{
    bEnableUseUserData = TRUE;
    aUseUserDataCB.Show();
    aDeleteBtn.Show();
}

//------------------------------------------------------------------------

BOOL SfxDocumentPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;

    if ( !bHandleDelete && bEnableUseUserData &&
         aUseUserDataCB.GetState() != aUseUserDataCB.GetSavedValue() &&
         GetTabDialog() && GetTabDialog()->GetExampleSet() )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet && SFX_ITEM_SET ==
                        pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            SfxDocumentInfoItem* pInfoItem = (SfxDocumentInfoItem*)pItem;
            SfxDocumentInfo aInfo( (*pInfoItem)() );
            BOOL bUseData = ( STATE_CHECK == aUseUserDataCB.GetState() );
            aInfo.SetUseUserData( bUseData );
/*
            if ( !bUseData )
            {
                // "Benutzerdaten verwenden" ausgeschaltet ->
                // den Benutzer aus den Stamps l"oschen
                String aEmptyUser;
                aInfo.SetCreated(
                    SfxStamp( aEmptyUser, aInfo.GetCreated().GetTime() ) );
                aInfo.SetChanged(
                    SfxStamp( aEmptyUser, aInfo.GetChanged().GetTime() ) );
                aInfo.SetPrinted(
                    SfxStamp( aEmptyUser, aInfo.GetPrinted().GetTime() ) );
            }
*/
            rSet.Put( SfxDocumentInfoItem( pInfoItem->GetValue(), aInfo ) );
            bRet = TRUE;
        }
    }

    if ( bHandleDelete )
    {
        const SfxItemSet* pExpSet = GetTabDialog()->GetExampleSet();
        const SfxPoolItem* pItem;

        if ( pExpSet &&
             SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            SfxDocumentInfoItem* pInfoItem = (SfxDocumentInfoItem*)pItem;
            SfxDocumentInfo aInfo( pInfoItem->GetDocInfo() );
            SfxStamp aCreated;
            if ( bEnableUseUserData && aUseUserDataCB.IsChecked() )
                aCreated.SetName( SFX_INIMANAGER()->GetUserFullName() );
            aInfo.SetCreated( aCreated );
            SfxStamp aInvalid( TIMESTAMP_INVALID_DATETIME );
            aInfo.SetChanged( aInvalid );
            aInfo.SetPrinted( aInvalid );
            aInfo.SetTime( 0L );
            aInfo.SetDocumentNumber( 1 );
            aInfo.SetUseUserData( STATE_CHECK == aUseUserDataCB.GetState() );
            rSet.Put( SfxDocumentInfoItem( pInfoItem->GetValue(), aInfo ) );
            bRet = TRUE;
        }
    }

    if ( aNameED.IsModified() && aNameED.GetText().Len() )
    {
        rSet.Put( SfxStringItem( ID_FILETP_TITLE, aNameED.GetText() ) );
        bRet = TRUE;
    }

    if ( /* aReadOnlyCB.IsModified() */ TRUE )
    {
        rSet.Put( SfxBoolItem( ID_FILETP_READONLY, aReadOnlyCB.IsChecked() ) );
        bRet = TRUE;
    }

    return bRet;
}

//------------------------------------------------------------------------

void SfxDocumentPage::Reset( const SfxItemSet& rSet )
{
    // Bestimmung der Dokumentinformationen
    const SfxDocumentInfoItem *pInfoItem =
        &(const SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);
    const SfxDocumentInfo& rInfo = ( *pInfoItem )();

    // Vorlagendaten
    if ( pInfoItem->HasTemplate() )
    {
        aTemplValFt.SetText( rInfo.GetTemplateName() );
    }
    else
    {
        aTemplFt.Hide();
        aTemplValFt.Hide();
    }

    // Bestimmung des Dateinamens
    String aFile( pInfoItem->GetValue() );
    String aFactory( aFile );
    if ( aFile.Len() > 2 && aFile.GetChar(0) == '[' )
    {
        USHORT nPos = aFile.Search( ']' );
        aFactory = aFile.Copy( 1, nPos-1  );
        aFile = aFile.Copy( nPos+1 );
    }

    // Bestimmung des Namens
    String aName;
    const SfxPoolItem* pItem = 0;
    if ( SFX_ITEM_SET != rSet.GetItemState( ID_FILETP_TITLE, FALSE, &pItem ) )
    {
        INetURLObject aURL(aFile);
        aName = aURL.GetName( INetURLObject::DECODE_WITH_CHARSET );
        if ( !aName.Len() || aURL.GetProtocol() == INET_PROT_PRIVATE )
            aName = String( SfxResId( STR_NONAME ) );
        aNameED.SetReadOnly( TRUE );
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ), "SfxDocumentPage:<SfxStringItem> erwartet" );
        aName = ( ( SfxStringItem* ) pItem )->GetValue();
    }
    aNameED.SetText( aName );
    aNameED.ClearModifyFlag();

    // Bestimmung des R/O-Flags
    if ( SFX_ITEM_UNKNOWN == rSet.GetItemState( ID_FILETP_READONLY, FALSE, &pItem ) )
        aReadOnlyCB.Hide();
    else
        aReadOnlyCB.Check( ( (SfxBoolItem*)pItem )->GetValue() );

    // Bestimmung des Kontextsymbols
    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetSmartURL( aFactory);
    const String& rMainURL = aURL.GetMainURL();
    aBmp1.SetImage( SfxExplorerFile::GetImage( aURL, TRUE ) );

    // Bestimmung von Groesse und Typ
    String aSizeText( aUnknownSize );
    if ( aURL.GetProtocol() == INET_PROT_FILE )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.PathToFileName() ) );
    aShowSizeFT.SetText( aSizeText );
    String aDescription( SfxResId( STR_SFX_NEWOFFICEDOC ) );
    if ( aURL.GetLastName().Len() )
        aDescription = SfxExplorerFile::GetDescription( rMainURL );
    aShowTypeFT.SetText( aDescription );

    // Bestimmung des Ortes
    aURL.SetSmartURL( aFile);
    if ( aURL.GetProtocol() == INET_PROT_FILE )
    {
        INetURLObject aPath( aURL );
        aPath.setFinalSlash();
        aPath.removeSegment();
        String aText( aPath.PathToFileName() ); //! (pb) MaxLen?
        aFileValFt.SetText( aText );
    }
    else if ( aURL.GetProtocol() != INET_PROT_PRIVATE )
        aFileValFt.SetText( aURL.GetPartBeforeLastName() );

    // Zugriffsdaten
    const SfxStamp& rCreated = rInfo.GetCreated();
    aCreateValFt.SetText( ConvertDateTime_Impl( rCreated ) );
    const SfxStamp& rChanged = rInfo.GetChanged();
    if ( rCreated != rChanged && rChanged.IsValid() )
        aChangeValFt.SetText( ConvertDateTime_Impl( rChanged ) );
    const SfxStamp& rPrinted = rInfo.GetPrinted();
    if ( rPrinted != rCreated && rPrinted.IsValid())
        aPrintValFt.SetText( ConvertDateTime_Impl( rPrinted ) );
    const long nTime = rInfo.GetTime();
    if( 1 || nTime ) //!!!
    {
        const Time aTime( nTime );
        const International& rInter = Application::GetAppInternational();
        aTimeLogValFt.SetText( rInter.GetDuration( aTime ) );
    }
    aDocNoValFt.SetText( String::CreateFromInt32( rInfo.GetDocumentNumber() ) );

    TriState eState = (TriState)rInfo.IsUseUserData();

    if ( STATE_DONTKNOW == eState )
        aUseUserDataCB.EnableTriState( TRUE );

    aUseUserDataCB.SetState( eState );
    aUseUserDataCB.SaveValue();
    aUseUserDataCB.Enable( bEnableUseUserData );
    bHandleDelete = FALSE;
    aDeleteBtn.Enable( bEnableUseUserData );
}

//------------------------------------------------------------------------

SfxInternetPage::SfxInternetPage( Window* pParent, const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFORELOAD ), rItemSet ),

    aReloadEnabled  ( this, ResId( CB_AUTORELOAD ) ),
    aFTSeconds      ( this, ResId( FT_SECONDS ) ),
    aReloadDelay    ( this, ResId( ED_SECONDS ) ),
    aFTURL          ( this, ResId( FT_URL ) ),
    aReloadURL      ( this, ResId( ED_URL ) ),
    aFTTarget       ( this, ResId( FT_DEFAULT ) ),
    aTargets        ( this, ResId( LB_DEFAULT ) ),

    pInfoItem( NULL )

{
    FreeResource();
    pInfoItem = &(SfxDocumentInfoItem &)rItemSet.Get(SID_DOCINFO);
    SfxDocumentInfo& rInfo = (*pInfoItem)();
    TargetList aList;
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if( pFrame && ( pFrame = pFrame->GetTopViewFrame() ))
    {
        pFrame->GetTargetList( aList );

        for( USHORT nPos = (USHORT)aList.Count(); nPos; )
        {
            String* pObj = aList.GetObject( --nPos );
            aTargets.InsertEntry( *pObj );
            delete pObj;
        }
    }
    aTargets.SetText( rInfo.GetDefaultTarget() );
    aReloadEnabled.SetClickHdl(LINK(this, SfxInternetPage, ClickHdl));
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdl, Control*, pCtrl )
{
    if ( pCtrl == &aReloadEnabled )
    {
        aFTSeconds.Enable( aReloadEnabled.IsChecked() );
        aReloadDelay.Enable( aReloadEnabled.IsChecked() );
    }
    return 0;
}

//------------------------------------------------------------------------

BOOL SfxInternetPage::FillItemSet( SfxItemSet& rSet )
{
    // Pruefung, ob sich etwas geaendert hat
    const BOOL bReloadEnabled = aReloadEnabled.IsChecked();
    const BOOL bReloadModified = aReloadEnabled.GetSavedValue() !=
        bReloadEnabled;
    const BOOL bReloadURLModified = aReloadURL.IsModified();
    String aTargetFrame( aTargets.GetText() );
    const BOOL bTargetModified = aOldTarget != aTargetFrame;
    const BOOL bReloadDelayModified = aReloadDelay.IsModified();
    if( !( bReloadDelayModified || bTargetModified || bReloadURLModified ||
           bReloadModified ) )
    {
        return FALSE;
    }

    // Speicherung der Aenderungen
    const SfxPoolItem *pItem;
    SfxDocumentInfoItem *pInfo;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet &&
         SFX_ITEM_SET != pExSet->GetItemState(SID_DOCINFO, TRUE, &pItem) )
        pInfo = pInfoItem;
    else
        pInfo = new SfxDocumentInfoItem(*(const SfxDocumentInfoItem *)pItem);

    SfxDocumentInfo& rInfo = (*pInfo)();
    if( bReloadModified )
    {
        rInfo.EnableReload( bReloadEnabled );
    }
    if( bReloadDelayModified )
    {
        rInfo.SetReloadDelay( aReloadDelay.GetValue() );
    }
    if( bReloadURLModified )
    {
        String a2ReloadURL( aReloadURL.GetText() );
        if ( a2ReloadURL.Len() )
        {
            a2ReloadURL = URIHelper::SmartRelToAbs( a2ReloadURL );
        }
        rInfo.SetReloadURL( a2ReloadURL );
    }
    if( bTargetModified )
    {
        rInfo.SetDefaultTarget( aTargetFrame );
    }
    rSet.Put( *pInfo );
    if( pInfo != pInfoItem )
        delete pInfo;
    return TRUE;
}

//------------------------------------------------------------------------

SfxTabPage *SfxInternetPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return new SfxInternetPage(pParent, rItemSet);
}

//------------------------------------------------------------------------

void SfxInternetPage::Reset( const SfxItemSet& rSet )
{
    pInfoItem = &(SfxDocumentInfoItem &)rSet.Get(SID_DOCINFO);
    SfxDocumentInfo& rInfo = (*pInfoItem)();

    aReloadEnabled.Check( rInfo.IsReloadEnabled() );
    aReloadEnabled.SaveValue();
    aReloadDelay.SetValue( rInfo.GetReloadDelay() );
    aReloadURL.SetText( rInfo.GetReloadURL() );
    aTargets.SetText( rInfo.GetDefaultTarget() );
    ClickHdl( &aReloadEnabled );

    if ( rInfo.IsReadOnly() )
    {
        aReloadEnabled.Disable();
        aReloadDelay.Disable();
        aReloadURL.SetReadOnly( TRUE );
        aTargets.SetReadOnly( TRUE );
    }
}

//------------------------------------------------------------------------

SfxDocumentUserPage::SfxDocumentUserPage( Window* pParent,
                                          const SfxItemSet& rItemSet ) :

    SfxTabPage( pParent, SfxResId( TP_DOCINFOUSER ), rItemSet ),

    aInfo1Ft        ( this, ResId( FT_INFO1 ) ),
    aInfo2Ft        ( this, ResId( FT_INFO2 ) ),
    aInfo3Ft        ( this, ResId( FT_INFO3 ) ),
    aInfo4Ft        ( this, ResId( FT_INFO4 ) ),
    aInfo1Ed        ( this, ResId( ED_INFO1 ) ),
    aInfo2Ed        ( this, ResId( ED_INFO2 ) ),
    aInfo3Ed        ( this, ResId( ED_INFO3 ) ),
    aInfo4Ed        ( this, ResId( ED_INFO4 ) ),
    aEditLabelBtn   ( this, ResId( BTN_EDITLABEL ) ),

    bLabelModified  ( FALSE ),
    pInfoItem       ( NULL )

{
    FreeResource();
    aEditLabelBtn.SetClickHdl( LINK( this, SfxDocumentUserPage, EditLabelHdl ) );
}

//------------------------------------------------------------------------

IMPL_LINK( SfxDocumentUserPage, EditLabelHdl, PushButton *, pPushButton )
{
    SfxDocInfoEditDlg* pDlg = new SfxDocInfoEditDlg( this );
    pDlg->SetText1( aInfo1Ft.GetText().EraseAllChars( '~' ) );
    pDlg->SetText2( aInfo2Ft.GetText().EraseAllChars( '~' ) );
    pDlg->SetText3( aInfo3Ft.GetText().EraseAllChars( '~' ) );
    pDlg->SetText4( aInfo4Ft.GetText().EraseAllChars( '~' ) );

    if ( RET_OK == pDlg->Execute() )
    {
        aShortcuts.Erase();
        aInfo1Ft.SetText( CreateShortcutTitle( pDlg->GetText1() ) );
        aInfo2Ft.SetText( CreateShortcutTitle( pDlg->GetText2() ) );
        aInfo3Ft.SetText( CreateShortcutTitle( pDlg->GetText3() ) );
        aInfo4Ft.SetText( CreateShortcutTitle( pDlg->GetText4() ) );
        bLabelModified = TRUE;
    }
    delete pDlg;
    return 0;
}

//------------------------------------------------------------------------

XubString SfxDocumentUserPage::CreateShortcutTitle( const XubString& rTitle )
{
    USHORT nPos;

    if ( !aShortcuts.Len() )
    {
        XubString aText = aEditLabelBtn.GetText().ToLowerAscii();
        nPos = aText.Search( '~' );
        if ( nPos != STRING_NOTFOUND && nPos != aText.Len() - 1 )
            aShortcuts = aText.GetChar( nPos + 1 );
    }
    XubString aTitle = rTitle;
    aTitle.ToLowerAscii();
    const char* pcTitle = U2S(aTitle).getStr();
    FASTBOOL bFound = FALSE;
    nPos = 0;

    while ( *pcTitle && !bFound )
    {
        char cChar = *pcTitle;
        if ( ByteString( cChar ).IsAlphaNumericAscii() &&
             aShortcuts.Search( cChar ) == STRING_NOTFOUND )
        {
            bFound = TRUE;
            aShortcuts += cChar;
        }
        else
        {
            pcTitle++;
            nPos++;
        }
    }

    if ( !bFound )
        nPos = 0;

    XubString aShortcutTitle = rTitle;
    aShortcutTitle.Insert( '~', nPos );
    return aShortcutTitle;
}

//------------------------------------------------------------------------

SfxTabPage* SfxDocumentUserPage::Create( Window* pParent, const SfxItemSet& rItemSet )
{
    return new SfxDocumentUserPage(pParent, rItemSet);
}

//------------------------------------------------------------------------

BOOL SfxDocumentUserPage::FillItemSet( SfxItemSet& rSet )
{
    const BOOL bMod = bLabelModified ||
                      aInfo1Ed.IsModified() || aInfo2Ed.IsModified() ||
                      aInfo3Ed.IsModified() || aInfo4Ed.IsModified();
    if ( !bMod )
        return FALSE;

    const SfxPoolItem* pItem = 0;
    SfxDocumentInfoItem* pInfo = 0;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( pExSet &&
         SFX_ITEM_SET != pExSet->GetItemState(SID_DOCINFO, TRUE, &pItem) )
        pInfo = pInfoItem;
    else
        pInfo = new SfxDocumentInfoItem( *(const SfxDocumentInfoItem*)pItem );
    SfxDocumentInfo& rInfo = pInfo->GetDocInfo();

    if ( bLabelModified || aInfo1Ed.IsModified() )
    {
        XubString aTitle = aInfo1Ft.GetText().EraseAllChars( '~' );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo1Ed.GetText() ), 0 );
    }
    if ( bLabelModified || aInfo2Ed.IsModified() )
    {
        XubString aTitle = aInfo2Ft.GetText().EraseAllChars( '~' );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo2Ed.GetText() ), 1 );
    }
    if ( bLabelModified || aInfo3Ed.IsModified() )
    {
        XubString aTitle = aInfo3Ft.GetText().EraseAllChars( '~' );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo3Ed.GetText() ), 2 );
    }
    if ( bLabelModified || aInfo4Ed.IsModified() )
    {
        XubString aTitle = aInfo4Ft.GetText().EraseAllChars( '~' );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo4Ed.GetText() ), 3 );
    }
    rSet.Put( *pInfo );
    if ( pInfo != pInfoItem )
        delete pInfo;
    return bMod;
}

//------------------------------------------------------------------------

void SfxDocumentUserPage::Reset(const SfxItemSet &rSet)
{
    pInfoItem = &(SfxDocumentInfoItem&)rSet.Get( SID_DOCINFO );
    const SfxDocumentInfo& rInfo = pInfoItem->GetDocInfo();
    aShortcuts.Erase();
    aInfo1Ft.SetText( CreateShortcutTitle( rInfo.GetUserKey(0).GetTitle() ) );
    aInfo1Ed.SetText( rInfo.GetUserKey(0).GetWord() );
    aInfo2Ft.SetText( CreateShortcutTitle( rInfo.GetUserKey(1).GetTitle() ) );
    aInfo2Ed.SetText( rInfo.GetUserKey(1).GetWord() );
    aInfo3Ft.SetText( CreateShortcutTitle( rInfo.GetUserKey(2).GetTitle() ) );
    aInfo3Ed.SetText( rInfo.GetUserKey(2).GetWord() );
    aInfo4Ft.SetText( CreateShortcutTitle( rInfo.GetUserKey(3).GetTitle() ) );
    aInfo4Ed.SetText( rInfo.GetUserKey(3).GetWord() );
    bLabelModified = FALSE;

    if ( rInfo.IsReadOnly() )
    {
        aInfo1Ed.SetReadOnly( TRUE );
        aInfo2Ed.SetReadOnly( TRUE );
        aInfo3Ed.SetReadOnly( TRUE );
        aInfo4Ed.SetReadOnly( TRUE );
        aEditLabelBtn.Disable();
    }
}

//------------------------------------------------------------------------

SfxDocumentInfoDialog::SfxDocumentInfoDialog( Window* pParent,
                                              const SfxItemSet& rItemSet ) :

    SfxTabDialog( 0, pParent, SfxResId( SID_DOCINFO ), &rItemSet )

{
    FreeResource();

     const SfxDocumentInfoItem* pInfoItem =
        &(const SfxDocumentInfoItem &)rItemSet.Get( SID_DOCINFO );

     // Bestimmung des Titels
    const SfxPoolItem* pItem = 0;
    String aTitle( GetText() );
    if ( SFX_ITEM_SET !=
         rItemSet.GetItemState( SID_EXPLORER_PROPS_START, FALSE, &pItem ) )
    {
        // Dateiname
        String aFile( pInfoItem->GetValue() );
#ifdef WIN
        if ( aFile.Len() <= 8 )
        {
            String sTmp( SfxResId( STR_NONAME ) );
            USHORT nLen = Min( (USHORT)8, sTmp.Len() );

            if ( sTmp.Copy( 0, nLen ).Lower() ==
                 aFile.Copy( 0, nLen ).Lower() )
            {
                aFile = pInfoItem->GetValue();
            }
        }
#endif

        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( aFile);
        if ( INET_PROT_PRIV_SOFFICE != aURL.GetProtocol() )
        {
            String aLastName( aURL.GetLastName() );
            if ( aLastName.Len() )
                aTitle += aLastName;
            else
                aTitle += aFile;
        }
        else
            aTitle += String( SfxResId( STR_NONAME ) );
    }
    else
    {
        DBG_ASSERT( pItem->IsA( TYPE( SfxStringItem ) ),
                    "SfxDocumentInfoDialog:<SfxStringItem> erwartet" );
        aTitle += ( ( SfxStringItem* ) pItem )->GetValue();
    }
    SetText( aTitle );

    // Eigenschaftenseiten
    AddTabPage(TP_DOCINFODESC, SfxDocumentDescPage::Create, 0);
    AddTabPage(TP_DOCINFODOC, SfxDocumentPage::Create, 0);
    AddTabPage(TP_DOCINFOUSER, SfxDocumentUserPage::Create, 0);
    AddTabPage(TP_DOCINFORELOAD, SfxInternetPage::Create, 0);
}

// -----------------------------------------------------------------------

void SfxDocumentInfoDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    if ( TP_DOCINFODOC == nId )
        ( (SfxDocumentPage&)rPage ).EnableUseUserData();
}


