/*************************************************************************
 *
 *  $RCSfile: dinfdlg.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:27:58 $
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <filedlghelper.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <svtools/urihelper.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/imagemgr.hxx>

#include <memory>

#pragma hdrstop

#include "dinfdlg.hxx"
#include "sfxresid.hxx"
#include "dinfedt.hxx"
#include "frame.hxx"
#include "viewfrm.hxx"
#include "request.hxx"
#include "exptypes.hxx"
#include "helper.hxx"

#include "sfx.hrc"
#include "dinfdlg.hrc"
#include "sfxlocal.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SfxDocumentInfoItem, SfxStringItem);

//------------------------------------------------------------------------

String ConvertDateTime_Impl( const SfxStamp& rStamp, const LocaleDataWrapper& rWrapper )
{
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     String aStr( rWrapper.getDate( rStamp.GetTime() ) );
     aStr += pDelim;
     aStr += rWrapper.getTime( rStamp.GetTime(), TRUE, FALSE );
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
    bOwnFormat = ( (bOwn&1) != 0 );
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
     return ((bOwnFormat&1) != 0);
}

BOOL SfxDocumentInfoItem::IsDeleteUserData() const
{
     return ((bOwnFormat&2) != 0);
}

void SfxDocumentInfoItem::SetDeleteUserData( BOOL bSet )
{
    if ( bSet )
        bOwnFormat |= 2;
    else
        bOwnFormat &= 2;
}

sal_Bool SfxDocumentInfoItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    String aValue;
    sal_Int32 nValue = 0;
    sal_Bool bValue = sal_False;
    BOOL bField = FALSE;
    BOOL bIsInt = FALSE;
    BOOL bIsString = FALSE;
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bValue = aDocInfo.IsUseUserData();
            break;
        case MID_DOCINFO_DELETEUSERDATA:
            bValue = IsDeleteUserData();
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            bValue = aDocInfo.IsReloadEnabled();
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bIsInt = TRUE;
            nValue = aDocInfo.GetReloadDelay();
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bIsString = TRUE;
            aValue = aDocInfo.GetReloadURL();
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bIsString = TRUE;
            aValue = aDocInfo.GetDefaultTarget();
            break;
        case MID_DOCINFO_DESCRIPTION:
            bIsString = TRUE;
            aValue = aDocInfo.GetComment();
            break;
        case MID_DOCINFO_KEYWORDS:
            bIsString = TRUE;
            aValue = aDocInfo.GetKeywords();
            break;
        case MID_DOCINFO_SUBJECT:
            bIsString = TRUE;
            aValue = aDocInfo.GetTheme();
            break;
        case MID_DOCINFO_TITLE:
            bIsString = TRUE;
            aValue = aDocInfo.GetTitle();
            break;
        case MID_DOCINFO_FIELD1:
        case MID_DOCINFO_FIELD2:
        case MID_DOCINFO_FIELD3:
        case MID_DOCINFO_FIELD4:
            bField = TRUE;
            // fehlendes break beabsichtigt
        case MID_DOCINFO_FIELD1TITLE:
        case MID_DOCINFO_FIELD2TITLE:
        case MID_DOCINFO_FIELD3TITLE:
        case MID_DOCINFO_FIELD4TITLE:
        {
            bIsString = TRUE;
            USHORT nSub = MID_DOCINFO_FIELD1TITLE;
            if ( bField )
            {
                nSub = MID_DOCINFO_FIELD1;
            }
            const SfxDocUserKey &rOld = aDocInfo.GetUserKey( nMemberId - nSub );
            if ( bField )
            {
                DBG_ASSERT( nMemberId == MID_DOCINFO_FIELD1 ||
                            nMemberId == MID_DOCINFO_FIELD2 ||
                            nMemberId == MID_DOCINFO_FIELD3 ||
                            nMemberId == MID_DOCINFO_FIELD4,
                            "SfxDocumentInfoItem:Anpassungsfehler" );
                aValue = rOld.GetWord();
            }
            else
            {
                DBG_ASSERT( nMemberId == MID_DOCINFO_FIELD1TITLE ||
                            nMemberId == MID_DOCINFO_FIELD2TITLE ||
                            nMemberId == MID_DOCINFO_FIELD3TITLE ||
                            nMemberId == MID_DOCINFO_FIELD4TITLE,
                            "SfxDocumentInfoItem:Anpassungsfehler" );
                aValue = rOld.GetTitle();
            }
            break;
        }
        default:
            DBG_ERROR("Wrong MemberId!");
               return sal_False;
     }

    if ( bIsString )
        rVal <<= ::rtl::OUString( aValue );
    else if ( bIsInt )
        rVal <<= nValue;
    else
        rVal <<= bValue;
    return sal_True;
}

sal_Bool SfxDocumentInfoItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    ::rtl::OUString aValue;
    sal_Int32 nValue=0;
    sal_Bool bValue = sal_False;
    sal_Bool bRet = sal_False;
    sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_DOCINFO_USEUSERDATA:
            bRet = (rVal >>= bValue);
            if ( bRet )
                aDocInfo.SetUseUserData( bValue );
            break;
        case MID_DOCINFO_DELETEUSERDATA:
            bRet = (rVal >>= bValue);
            if ( bRet )
            {
                SetDeleteUserData( bValue );
                if ( IsDeleteUserData() )
                    aDocInfo.DeleteUserData( aDocInfo.IsUseUserData() );
            }
            break;
        case MID_DOCINFO_AUTOLOADENABLED:
            bRet = (rVal >>= bValue);
            if ( bRet )
                aDocInfo.EnableReload( bValue );
            break;
        case MID_DOCINFO_AUTOLOADSECS:
            bRet = (rVal >>= nValue);
            if ( bRet )
                aDocInfo.SetReloadDelay( nValue );
            break;
        case MID_DOCINFO_AUTOLOADURL:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetReloadURL( aValue );
            break;
        case MID_DOCINFO_DEFAULTTARGET:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetDefaultTarget( aValue );
            break;
        case MID_DOCINFO_DESCRIPTION:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetComment(aValue);
            break;
        case MID_DOCINFO_KEYWORDS:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetKeywords(aValue);
            break;
        case MID_DOCINFO_SUBJECT:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetTheme(aValue);
            break;
        case MID_DOCINFO_TITLE:
            bRet = (rVal >>= aValue);
            if ( bRet )
                aDocInfo.SetTitle(aValue);
            break;
        case MID_DOCINFO_FIELD1TITLE:
        case MID_DOCINFO_FIELD2TITLE:
        case MID_DOCINFO_FIELD3TITLE:
        case MID_DOCINFO_FIELD4TITLE:
        {
            bRet = (rVal >>= aValue);
            if ( bRet )
            {
                const SfxDocUserKey &rOld = aDocInfo.GetUserKey(nMemberId-MID_DOCINFO_FIELD1TITLE);
                SfxDocUserKey aNew( aValue, rOld.GetWord() );
                aDocInfo.SetUserKey( aNew, nMemberId-MID_DOCINFO_FIELD1TITLE );
            }
            break;
        }
        case MID_DOCINFO_FIELD1:
        case MID_DOCINFO_FIELD2:
        case MID_DOCINFO_FIELD3:
        case MID_DOCINFO_FIELD4:
        {
            bRet = (rVal >>= aValue);
            if ( bRet )
            {
                const SfxDocUserKey &rOld = aDocInfo.GetUserKey(nMemberId-MID_DOCINFO_FIELD1);
                SfxDocUserKey aNew( rOld.GetTitle(), aValue );
                aDocInfo.SetUserKey( aNew, nMemberId-MID_DOCINFO_FIELD1 );
            }
            break;
        }
        default:
            DBG_ERROR("Wrong MemberId!");
            return sal_False;
    }

    return bRet;
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

    if ( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
    {
        pInfo = pInfoItem;
    }
    else
    {
        pInfo = new SfxDocumentInfoItem( *( const SfxDocumentInfoItem *) pItem );
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
        aCreated.SetName( SvtUserOptions().GetFullName() );
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    aCreateValFt.SetText( ConvertDateTime_Impl( aCreated, aLocaleWrapper ) );
    XubString aEmpty;
    aChangeValFt.SetText( aEmpty );
    aPrintValFt.SetText( aEmpty );
    const Time aTime( 0 );
    aTimeLogValFt.SetText( aLocaleWrapper.getDuration( aTime ) );
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

        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
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
        if ( pExpSet && SFX_ITEM_SET == pExpSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        {
            SfxDocumentInfoItem* pInfoItem = (SfxDocumentInfoItem*)pItem;
            SfxDocumentInfo aInfo( pInfoItem->GetDocInfo() );
            BOOL bUseAuthor = bEnableUseUserData && aUseUserDataCB.IsChecked();
            aInfo.DeleteUserData( bUseAuthor );
            aInfo.SetUseUserData( STATE_CHECK == aUseUserDataCB.GetState() );
            SfxDocumentInfoItem aItem( pInfoItem->GetValue(), aInfo );
            aItem.SetDeleteUserData( TRUE );
            rSet.Put( aItem );
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
    if ( SFX_ITEM_UNKNOWN == rSet.GetItemState( ID_FILETP_READONLY, FALSE, &pItem )
         || !pItem )
        aReadOnlyCB.Hide();
    else
        aReadOnlyCB.Check( ( (SfxBoolItem*)pItem )->GetValue() );

    // Bestimmung des Kontextsymbols
    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    aURL.SetSmartURL( aFactory);
    const String& rMainURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
    aBmp1.SetImage( SvFileInformationManager::GetImage( aURL, TRUE ) );

    // Bestimmung von Groesse und Typ
    String aSizeText( aUnknownSize );
    if ( aURL.GetProtocol() == INET_PROT_FILE )
        aSizeText = CreateSizeText( SfxContentHelper::GetSize( aURL.GetMainURL( INetURLObject::NO_DECODE ) ) );
    aShowSizeFT.SetText( aSizeText );

    String aDescription = SvFileInformationManager::GetDescription( rMainURL );
    if ( aDescription.Len() == 0 )
        aDescription = String( SfxResId( STR_SFX_NEWOFFICEDOC ) );
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
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const SfxStamp& rCreated = rInfo.GetCreated();
    aCreateValFt.SetText( ConvertDateTime_Impl( rCreated, aLocaleWrapper ) );
    const SfxStamp& rChanged = rInfo.GetChanged();
    if ( rCreated != rChanged && rChanged.IsValid() )
        aChangeValFt.SetText( ConvertDateTime_Impl( rChanged, aLocaleWrapper ) );
    const SfxStamp& rPrinted = rInfo.GetPrinted();
    if ( rPrinted != rCreated && rPrinted.IsValid())
        aPrintValFt.SetText( ConvertDateTime_Impl( rPrinted, aLocaleWrapper ) );
    const long nTime = rInfo.GetTime();
    if( 1 || nTime ) //!!!
    {
        const Time aTime( nTime );
        aTimeLogValFt.SetText( aLocaleWrapper.getDuration( aTime ) );
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

    aRBNoAutoUpdate     ( this, ResId( RB_NOAUTOUPDATE      ) ),

    aRBReloadUpdate     ( this, ResId( RB_RELOADUPDATE      ) ),
    aFTEvery            ( this, ResId( FT_EVERY             ) ),
    aNFReload           ( this, ResId( ED_RELOAD            ) ),
    aFTReloadSeconds    ( this, ResId( FT_RELOADSECS        ) ),

    aRBForwardUpdate    ( this, ResId( RB_FORWARDUPDATE     ) ),
    aFTAfter            ( this, ResId( FT_AFTER             ) ),
    aNFAfter            ( this, ResId( ED_FORWARD           ) ),
    aFTAfterSeconds     ( this, ResId( FT_FORWARDSECS       ) ),
    aFTURL              ( this, ResId( FT_URL               ) ),
    aEDForwardURL       ( this, ResId( ED_URL               ) ),
    aPBBrowseURL        ( this, ResId( PB_BROWSEURL         ) ),
    aFTFrame            ( this, ResId( FT_FRAME             ) ),
    aCBFrame            ( this, ResId( CB_FRAME             ) ),

    aForwardErrorMessg  (       ResId( STR_FORWARD_ERRMSSG  ) ),
    eState( S_Init ),
    pInfoItem( NULL )

{
    FreeResource();
    pInfoItem = &( SfxDocumentInfoItem& ) rItemSet.Get( SID_DOCINFO );
//  SfxDocumentInfo&    rInfo = pInfoItem->GetDocInfo();
    TargetList          aList;
    SfxViewFrame*       pFrame = SfxViewFrame::Current();
    if( pFrame && ( pFrame = pFrame->GetTopViewFrame() ) )
    {
        pFrame->GetTargetList( aList );

        String*         pObj;
        for( USHORT nPos = ( USHORT ) aList.Count() ; nPos ; )
        {
            pObj = aList.GetObject( --nPos );
            aCBFrame.InsertEntry( *pObj );
            delete pObj;
        }
    }

    aRBNoAutoUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlNoUpdate ) );
    aRBReloadUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlReload ) );
    aRBForwardUpdate.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlForward ) );
    aPBBrowseURL.SetClickHdl( LINK( this, SfxInternetPage, ClickHdlBrowseURL ) );

    aForwardErrorMessg.SearchAndReplaceAscii( "%PLACEHOLDER%", aRBForwardUpdate.GetText() );

    ChangeState( S_NoUpdate );
}


//------------------------------------------------------------------------

void SfxInternetPage::ChangeState( STATE eNewState )
{
    DBG_ASSERT( eNewState != S_Init, "*SfxInternetPage::ChangeState(): new state init is supposed to not work here!" );

    if( eState == eNewState  )
        return;

    switch( eState )
    {
        case S_Init:
            EnableNoUpdate( TRUE );
            EnableReload( FALSE );
            EnableForward( FALSE );
            break;
        case S_NoUpdate:
            EnableNoUpdate( FALSE );
            if( eNewState == S_Reload )
                EnableReload( TRUE );
            else
                EnableForward( TRUE );
            break;
        case S_Reload:
            EnableReload( FALSE );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( TRUE );
            else
                EnableForward( TRUE );
            break;
        case S_Forward:
            EnableForward( FALSE );
            if( eNewState == S_NoUpdate )
                EnableNoUpdate( TRUE );
            else
                EnableReload( TRUE );
            break;
        default:
            DBG_ERROR( "*SfxInternetPage::SetState(): unhandled state!" );
    }

    eState = eNewState;
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableNoUpdate( BOOL bEnable )
{
    if( bEnable )
        aRBNoAutoUpdate.Check();
}


//------------------------------------------------------------------------

void SfxInternetPage::EnableReload( BOOL bEnable )
{
    aFTEvery.Enable( bEnable );
    aNFReload.Enable( bEnable );
    aFTReloadSeconds.Enable( bEnable );

    if( bEnable )
        aRBReloadUpdate.Check();
}

//------------------------------------------------------------------------

void SfxInternetPage::EnableForward( BOOL bEnable )
{
    aFTAfter.Enable( bEnable );
    aNFAfter.Enable( bEnable );
    aFTAfterSeconds.Enable( bEnable );
    aFTURL.Enable( bEnable );
    aEDForwardURL.Enable( bEnable );
    aPBBrowseURL.Enable( bEnable );
    aFTFrame.Enable( bEnable );
    aCBFrame.Enable( bEnable );

    if( bEnable )
        aRBForwardUpdate.Check();
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlNoUpdate, Control*, pCtrl )
{
    ChangeState( S_NoUpdate );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlReload, Control*, pCtrl )
{
    ChangeState( S_Reload );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlForward, Control*, pCtrl )
{
    ChangeState( S_Forward );
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SfxInternetPage, ClickHdlBrowseURL, PushButton*, pButton )
{
    sfx2::FileDialogHelper aHelper( ::sfx2::FILEOPEN_SIMPLE, WB_OPEN );
    aHelper.SetDisplayDirectory( aEDForwardURL.GetText() );

    if( ERRCODE_NONE == aHelper.Execute() )
        aEDForwardURL.SetText( aHelper.GetPath() );

    return 0;
}

//------------------------------------------------------------------------

BOOL SfxInternetPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem*          pItem = NULL;
    SfxDocumentInfoItem*        pInfo = NULL;
    SfxTabDialog*               pDlg = GetTabDialog();
    const SfxItemSet*           pExSet = NULL;

    if( pDlg )
        pExSet = pDlg->GetExampleSet();

    if( pExSet && SFX_ITEM_SET != pExSet->GetItemState( SID_DOCINFO, TRUE, &pItem ) )
        pInfo = pInfoItem;
    else
        pInfo = new SfxDocumentInfoItem( *( const SfxDocumentInfoItem* ) pItem );

    SfxDocumentInfo&            rInfo = pInfo->GetDocInfo();

    DBG_ASSERT( eState != S_Init, "*SfxInternetPage::FillItemSet(): state init is not acceptable at this point!" );

    BOOL                        bEnableReload = FALSE;
    ::std::auto_ptr< String >   aURL( NULL );
    ::std::auto_ptr< String >   aFrame( NULL );
    ULONG                       nDelay = 0;

    switch( eState )
    {
        case S_NoUpdate:
            break;
        case S_Reload:
            bEnableReload = TRUE;
            aURL = ::std::auto_ptr< String >( new String() );
            aFrame = ::std::auto_ptr< String >( new String() );
            nDelay = aNFReload.GetValue();
            break;
        case S_Forward:
            DBG_ASSERT( aEDForwardURL.GetText().Len(), "+SfxInternetPage::FillItemSet(): empty URL should be not possible for forward option!" );

            bEnableReload = TRUE;
            aURL = ::std::auto_ptr< String >( new String( URIHelper::SmartRelToAbs( aEDForwardURL.GetText() ) ) );
            aFrame = ::std::auto_ptr< String >( new String( aCBFrame.GetText() ) );
            nDelay = aNFAfter.GetValue();
            break;
    }

    rInfo.EnableReload( bEnableReload );

    if( bEnableReload )
    {
        rInfo.SetReloadURL( *aURL.get() );
        rInfo.SetDefaultTarget( *aFrame.get() );
        rInfo.SetReloadDelay( nDelay );
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
    pInfoItem = &( SfxDocumentInfoItem& ) rSet.Get( SID_DOCINFO );
    SfxDocumentInfo&    rInfo = pInfoItem->GetDocInfo();
    STATE               eNewState = S_NoUpdate;

    if( rInfo.IsReloadEnabled() )
    {
        const String&   rURL = rInfo.GetReloadURL();

        if( rURL.Len() )
        {
            aNFAfter.SetValue( rInfo.GetReloadDelay() );
            aEDForwardURL.SetText( rInfo.GetReloadURL() );
            aCBFrame.SetText( rInfo.GetDefaultTarget() );
            eNewState = S_Forward;
        }
        else
        {
            aNFReload.SetValue( rInfo.GetReloadDelay() );
            eNewState = S_Reload;
        }
    }

    ChangeState( eNewState );

    // #102907# ------------------------
    if ( rInfo.IsReadOnly() )
    {
        aRBNoAutoUpdate.Disable();
        aRBReloadUpdate.Disable();
        aRBForwardUpdate.Disable();
        aNFReload.Disable();
        aNFAfter.Disable();
        aEDForwardURL.Disable();
        aPBBrowseURL.Disable();
        aCBFrame.Disable();
        aFTEvery.Disable();
        aFTReloadSeconds.Disable();
        aFTAfter.Disable();
        aFTAfterSeconds.Disable();
        aFTURL.Disable();
        aFTFrame.Disable();
    }
}

//------------------------------------------------------------------------

int SfxInternetPage::DeactivatePage( SfxItemSet* pSet )
{
    int             nRet = LEAVE_PAGE;

    if( eState == S_Forward && !aEDForwardURL.GetText().Len() )
    {
        ErrorBox    aErrBox( this, WB_OK, aForwardErrorMessg );
        aErrBox.Execute();

        nRet = KEEP_PAGE;
    }

    return nRet;
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
    pDlg->SetText1( GetLabelText_Impl( &aInfo1Ft ) );
    pDlg->SetText2( GetLabelText_Impl( &aInfo2Ft ) );
    pDlg->SetText3( GetLabelText_Impl( &aInfo3Ft ) );
    pDlg->SetText4( GetLabelText_Impl( &aInfo4Ft ) );

    if ( RET_OK == pDlg->Execute() )
    {
        SetLabelText_Impl( &aInfo1Ft, pDlg->GetText1() );
        SetLabelText_Impl( &aInfo2Ft, pDlg->GetText2() );
        SetLabelText_Impl( &aInfo3Ft, pDlg->GetText3() );
        SetLabelText_Impl( &aInfo4Ft, pDlg->GetText4() );
        bLabelModified = TRUE;
    }
    delete pDlg;
    return 0;
}

//------------------------------------------------------------------------

String SfxDocumentUserPage::GetLabelText_Impl( FixedText* pLabel )
{
    DBG_ASSERT( pLabel, "SfxDocumentUserPage::SetLabelText_Impl(): invalid label" );
    String aLabel = pLabel->GetText();
    aLabel.Erase( 0, aLabel.Search( ' ' ) + 1 );
    return aLabel;
}

//------------------------------------------------------------------------

void SfxDocumentUserPage::SetLabelText_Impl( FixedText* pLabel, const String& rNewLabel )
{
    String aLabel( '~' );
    sal_Int32 nNumber = 0;
    if ( &aInfo1Ft == pLabel )
        nNumber = 1;
    else if ( &aInfo2Ft == pLabel )
        nNumber = 2;
    else if ( &aInfo3Ft == pLabel )
        nNumber = 3;
    else if ( &aInfo4Ft == pLabel )
        nNumber = 4;
    DBG_ASSERT( nNumber > 0, "SfxDocumentUserPage::SetLabelText_Impl(): wrong label" );
    aLabel += String::CreateFromInt32( nNumber );
    aLabel += String( DEFINE_CONST_UNICODE(": ") );
    aLabel += rNewLabel;
    DBG_ASSERT( pLabel, "SfxDocumentUserPage::SetLabelText_Impl(): invalid label" );
    pLabel->SetText( aLabel );
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
        XubString aTitle = GetLabelText_Impl( &aInfo1Ft );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo1Ed.GetText() ), 0 );
    }
    if ( bLabelModified || aInfo2Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo2Ft );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo2Ed.GetText() ), 1 );
    }
    if ( bLabelModified || aInfo3Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo3Ft );
        rInfo.SetUserKey( SfxDocUserKey( aTitle, aInfo3Ed.GetText() ), 2 );
    }
    if ( bLabelModified || aInfo4Ed.IsModified() )
    {
        XubString aTitle = GetLabelText_Impl( &aInfo4Ft );
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
    SetLabelText_Impl( &aInfo1Ft, rInfo.GetUserKey(0).GetTitle() );
    aInfo1Ed.SetText( rInfo.GetUserKey(0).GetWord() );
    SetLabelText_Impl( &aInfo2Ft, rInfo.GetUserKey(1).GetTitle() );
    aInfo2Ed.SetText( rInfo.GetUserKey(1).GetWord() );
    SetLabelText_Impl( &aInfo3Ft, rInfo.GetUserKey(2).GetTitle() );
    aInfo3Ed.SetText( rInfo.GetUserKey(2).GetWord() );
    SetLabelText_Impl( &aInfo4Ft, rInfo.GetUserKey(3).GetTitle() );
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
