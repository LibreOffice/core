/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: versdlg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:34:51 $
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

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/itemset.hxx>
#include <svtools/useroptions.hxx>
#include <vcl/msgbox.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#include "versdlg.hrc"
#include "versdlg.hxx"
#include "viewfrm.hxx"
#include "sfxresid.hxx"
#include "docfile.hxx"
#include "objsh.hxx"
#include "sfxsids.hrc"
#include "dispatch.hxx"
#include "request.hxx"

using namespace com::sun::star;

// **************************************************************************
struct SfxVersionInfo
{
    String                  aName;
    String                  aComment;
    SfxStamp                aCreateStamp;

                            SfxVersionInfo();
                            SfxVersionInfo( const SfxVersionInfo& rInfo )
                            { *this = rInfo; }

    SfxVersionInfo&         operator=( const SfxVersionInfo &rInfo )
                            {
                                aName = rInfo.aName;
                                aComment = rInfo.aComment;
                                aCreateStamp = rInfo.aCreateStamp;
                                return *this;
                            }
};
DECLARE_LIST( _SfxVersionTable, SfxVersionInfo* )
class SfxVersionTableDtor : public _SfxVersionTable
{
public:
                            SfxVersionTableDtor( const sal_uInt16 nInitSz=0, const sal_uInt16 nReSz=1 )
                                : _SfxVersionTable( nInitSz, nReSz )
                            {}

                            SfxVersionTableDtor( const SfxVersionTableDtor &rCpy )
                            { *this = rCpy; }

                            SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo );

                            ~SfxVersionTableDtor()
                            { DelDtor(); }

    SfxVersionTableDtor&    operator=( const SfxVersionTableDtor &rCpy );
    void                    DelDtor();
    SvStream&               Read( SvStream & );
    SvStream&               Write( SvStream & ) const;
    SvStringsDtor*          GetVersions() const;
};

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo )
{
    for ( sal_Int32 n=0; n<(sal_Int32)rInfo.getLength(); n++ )
    {
        SfxVersionInfo* pInfo = new SfxVersionInfo;
        pInfo->aName = rInfo[n].Identifier;
        pInfo->aComment = rInfo[n].Comment;
        pInfo->aCreateStamp.SetName( rInfo[n].Author );

        Date aDate ( rInfo[n].TimeStamp.Day, rInfo[n].TimeStamp.Month, rInfo[n].TimeStamp.Year );
        Time aTime ( rInfo[n].TimeStamp.Hours, rInfo[n].TimeStamp.Minutes, rInfo[n].TimeStamp.Seconds, rInfo[n].TimeStamp.HundredthSeconds );

        pInfo->aCreateStamp.SetTime( DateTime( aDate, aTime ) );
        Insert( pInfo, Count() );
    }
}

void SfxVersionTableDtor::DelDtor()
{
    SfxVersionInfo* pTmp = First();
    while( pTmp )
    {
        delete pTmp;
        pTmp = Next();
    }
    Clear();
}

SfxVersionTableDtor& SfxVersionTableDtor::operator=( const SfxVersionTableDtor& rTbl )
{
    DelDtor();
    SfxVersionInfo* pTmp = ((SfxVersionTableDtor&)rTbl).First();
    while( pTmp )
    {
        SfxVersionInfo *pNew = new SfxVersionInfo( *pTmp );
        Insert( pNew, LIST_APPEND );
        pTmp = ((SfxVersionTableDtor&)rTbl).Next();
    }
    return *this;
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
SfxVersionInfo::SfxVersionInfo()
{
}

static String ConvertDateTime_Impl(const SfxStamp &rTime, const LocaleDataWrapper& rWrapper)
{
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     String aStr(rWrapper.getDate(rTime.GetTime()));
     aStr += pDelim;
     aStr += rWrapper.getTime(rTime.GetTime(), TRUE, FALSE);
     return aStr;
}

SvStringsDtor* SfxVersionTableDtor::GetVersions() const
{
    SvStringsDtor *pList = new SvStringsDtor;
    SfxVersionInfo* pInfo = ((SfxVersionTableDtor*) this)->First();
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    while ( pInfo )
    {
        String *pString = new String( pInfo->aComment );
        (*pString) += DEFINE_CONST_UNICODE( "; " );
        (*pString) += ConvertDateTime_Impl( pInfo->aCreateStamp, aLocaleWrapper );
        pList->Insert( pString, pList->Count() );
        pInfo = ((SfxVersionTableDtor*) this)->Next();
    }

    return pList;
}

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs_Impl[] =
{
    3, // Number of Tabs
    0, 62, 124
};

void SfxVersionsTabListBox_Impl::KeyInput( const KeyEvent& rKeyEvent )
{
    const KeyCode& rCode = rKeyEvent.GetKeyCode();
    switch ( rCode.GetCode() )
    {
        case KEY_RETURN :
        case KEY_ESCAPE :
        case KEY_TAB :
            Window::GetParent()->KeyInput( rKeyEvent );
            break;
        default:
            SvTabListBox::KeyInput( rKeyEvent );
            break;
    }
}

SfxVersionsTabListBox_Impl::SfxVersionsTabListBox_Impl( Window* pParent, const ResId& rResId )
    : SvTabListBox( pParent, rResId )
{
}

SfxVersionDialog::SfxVersionDialog ( SfxViewFrame* pFrame, Window *pParent )
    : SfxModalDialog( pFrame, pParent, SfxResId( DLG_VERSIONS ) )
    , aNewGroup( this, ResId( GB_NEWVERSIONS ) )
    , aSaveButton( this, ResId( PB_SAVE ) )
    , aSaveCheckBox( this, ResId( CB_SAVEONCLOSE ) )
    , aExistingGroup( this, ResId( GB_OLDVERSIONS ) )
    , aDateTimeText( this, ResId( FT_DATETIME ) )
    , aSavedByText( this, ResId( FT_SAVEDBY ) )
    , aCommentText( this, ResId( FT_COMMENTS ) )
    , aVersionBox( this, ResId( TLB_VERSIONS ) )
    , aCloseButton( this, ResId( PB_CLOSE ) )
    , aOpenButton( this, ResId( PB_OPEN ) )
    , aViewButton( this, ResId( PB_VIEW ) )
    , aDeleteButton( this, ResId( PB_DELETE ) )
    , aCompareButton( this, ResId( PB_COMPARE ) )
    , aHelpButton( this, ResId ( PB_HELP ) )
    , pViewFrame( pFrame )
    , mpTable( 0 )
{
    FreeResource();

    aViewButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aSaveButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aDeleteButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aCompareButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aOpenButton.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );
    aVersionBox.SetSelectHdl( LINK( this, SfxVersionDialog, SelectHdl_Impl ) );
    aVersionBox.SetDoubleClickHdl( LINK( this, SfxVersionDialog, DClickHdl_Impl ) );
    aSaveCheckBox.SetClickHdl ( LINK( this, SfxVersionDialog, ButtonHdl_Impl ) );

    aVersionBox.GrabFocus();
    aVersionBox.SetWindowBits( WB_HSCROLL | WB_CLIPCHILDREN );
    aVersionBox.SetSelectionMode( SINGLE_SELECTION );
    aVersionBox.SetTabs( &nTabs_Impl[0], MAP_APPFONT );
    aVersionBox.Resize();   // OS: Hack fuer richtige Selektion
    Init_Impl();
}

String ConvertWhiteSpaces_Impl( const String& rText )
{
    // converted linebreaks and tabs to blanks; it's necessary for the display
    String sConverted;
    const sal_Unicode* pChars = rText.GetBuffer();
    while ( *pChars )
    {
        switch ( *pChars )
        {
            case '\n' :
            case '\t' :
                sConverted += ' ';
                break;

            default:
                sConverted += *pChars;
        }

        ++pChars;
    }

    return sConverted;
}

void SfxVersionDialog::Init_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SfxMedium* pMedium = pObjShell->GetMedium();
    uno::Sequence < util::RevisionTag > aVersions = pMedium->GetVersionList();
    delete mpTable;
    mpTable = new SfxVersionTableDtor( aVersions );
    {
        LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
        for ( USHORT n=0; n<mpTable->Count(); n++ )
        {
            SfxVersionInfo *pInfo = mpTable->GetObject(n);
            String aEntry = ConvertDateTime_Impl( pInfo->aCreateStamp, aLocaleWrapper );
            aEntry += '\t';
            aEntry += pInfo->aCreateStamp.GetName();
            aEntry += '\t';
            aEntry += ConvertWhiteSpaces_Impl( pInfo->aComment );
            SvLBoxEntry *pEntry = aVersionBox.InsertEntry( aEntry );
            pEntry->SetUserData( pInfo );
        }
    }

    aSaveCheckBox.Check( pObjShell->GetDocInfo().IsSaveVersionOnClose() );

    aOpenButton.Enable( FALSE );
    aSaveButton.Enable( !pObjShell->IsReadOnly() );
    aSaveCheckBox.Enable( !pObjShell->IsReadOnly() );
    aDeleteButton.Enable( FALSE );

    const SfxPoolItem *pDummy=NULL;
    SfxItemState eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_MERGE, pDummy );
    eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_COMPARE, pDummy );
    aCompareButton.Enable( eState >= SFX_ITEM_AVAILABLE );

    // set dialog title (filename or docinfo title)
    String sText = GetText();
    ( sText += ' ' ) += pObjShell->GetTitle();
    SetText( sText );
}

SfxVersionDialog::~SfxVersionDialog ()
{
    delete mpTable;
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();

    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();
    ULONG nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
    SfxInt16Item aItem( SID_VERSION, (short)nPos+1 );
    SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
    SfxStringItem aFile( SID_FILE_NAME, pObjShell->GetMedium()->GetName() );
    pViewFrame->GetDispatcher()->Execute(
        SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aFile, &aItem, &aTarget, &aReferer, 0L );
    Close();
}

IMPL_LINK( SfxVersionDialog, DClickHdl_Impl, Control*, pControl )
{
    Open_Impl();
    return 0L;
}

IMPL_LINK( SfxVersionDialog, SelectHdl_Impl, Control*, pControl )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    aVersionBox.FirstSelected(); // -Wall required??
    aDeleteButton.Enable( !pObjShell->IsReadOnly() );
    aOpenButton.Enable( TRUE );
    return 0L;
}

IMPL_LINK( SfxVersionDialog, ButtonHdl_Impl, Button*, pButton )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();

    if ( pButton == &aSaveCheckBox )
    {
        SfxBoolItem aSave( SID_SAVE_VERSION_ON_CLOSE, aSaveCheckBox.IsChecked() );
        const SfxPoolItem* aItems[2];
        aItems[0] = &aSave;
        aItems[1] = NULL;
        pViewFrame->GetBindings().ExecuteSynchron( SID_SAVE_VERSION_ON_CLOSE, aItems, 0 );
    }
    else if ( pButton == &aSaveButton )
    {
        SfxVersionInfo aInfo;
        aInfo.aCreateStamp = SfxStamp( SvtUserOptions().GetFullName() );
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, aInfo, TRUE );
        short nRet = pDlg->Execute();
        if ( nRet == RET_OK )
        {
            SfxStringItem aComment( SID_DOCINFO_COMMENTS, aInfo.aComment );
            pObjShell->SetModified( TRUE );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aComment;
            aItems[1] = NULL;
            pViewFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, aItems, 0 );
            aVersionBox.SetUpdateMode( FALSE );
            aVersionBox.Clear();
            Init_Impl();
            aVersionBox.SetUpdateMode( TRUE );
        }

        delete pDlg;
    }
    if ( pButton == &aDeleteButton && pEntry )
    {
        pObjShell->GetMedium()->RemoveVersion_Impl( ((SfxVersionInfo*) pEntry->GetUserData())->aName );
        pObjShell->SetModified( TRUE );
            aVersionBox.SetUpdateMode( FALSE );
        aVersionBox.Clear();
        Init_Impl();
            aVersionBox.SetUpdateMode( TRUE );
    }
    else if ( pButton == &aOpenButton && pEntry )
    {
        Open_Impl();
    }
    else if ( pButton == &aViewButton && pEntry )
    {
        SfxVersionInfo* pInfo = (SfxVersionInfo*) pEntry->GetUserData();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, *pInfo, FALSE );
        pDlg->Execute();
        delete pDlg;
    }
    else if ( pEntry && pButton == &aCompareButton )
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        ULONG nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
        aSet.Put( SfxInt16Item( SID_VERSION, (short)nPos+1 ) );
        aSet.Put( SfxStringItem( SID_FILE_NAME, pObjShell->GetMedium()->GetName() ) );

        SfxItemSet* pSet = pObjShell->GetMedium()->GetItemSet();
        SFX_ITEMSET_ARG( pSet, pFilterItem, SfxStringItem, SID_FILTER_NAME, FALSE );
        SFX_ITEMSET_ARG( pSet, pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, FALSE );
        if ( pFilterItem )
            aSet.Put( *pFilterItem );
        if ( pFilterOptItem )
            aSet.Put( *pFilterOptItem );

        pViewFrame->GetDispatcher()->Execute( SID_DOCUMENT_COMPARE, SFX_CALLMODE_ASYNCHRON, aSet );
        Close();
    }

    return 0L;
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl ( Window *pParent, SfxVersionInfo& rInfo, BOOL bEdit )
    : SfxModalDialog( pParent, SfxResId( DLG_COMMENTS ) )
    , aDateTimeText( this, ResId( FT_DATETIME ) )
    , aSavedByText( this, ResId( FT_SAVEDBY ) )
    , aEdit( this, ResId ( ME_VERSIONS ) )
    , aOKButton( this, ResId( PB_OK ) )
    , aCancelButton( this, ResId( PB_CANCEL ) )
    , aCloseButton( this, ResId( PB_CLOSE ) )
    , aHelpButton( this, ResId ( PB_HELP ) )
    , pInfo( &rInfo )
{
    FreeResource();

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    aDateTimeText.SetText( aDateTimeText.GetText().Append(ConvertDateTime_Impl( pInfo->aCreateStamp, aLocaleWrapper )) );
    aSavedByText.SetText( aSavedByText.GetText().Append(pInfo->aCreateStamp.GetName()) );
    aEdit.SetText( rInfo.aComment );

    aCloseButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );
    aOKButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );

    aEdit.GrabFocus();
    if ( !bEdit )
    {
        aOKButton.Hide();
        aCancelButton.Hide();
        aEdit.SetReadOnly( TRUE );
    }
    else
        aCloseButton.Hide();
}

IMPL_LINK( SfxViewVersionDialog_Impl, ButtonHdl, Button*, pButton )
{
    if ( pButton == &aCloseButton )
    {
        EndDialog( RET_CANCEL );
    }
    else if ( pButton == &aOKButton )
    {
        pInfo->aComment = aEdit.GetText();
        EndDialog( RET_OK );
    }

    return 0L;
}


