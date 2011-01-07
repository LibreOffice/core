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
#include "precompiled_sfx2.hxx"
#include <unotools/localedatawrapper.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <tools/datetime.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include "versdlg.hrc"
#include "versdlg.hxx"
#include <sfx2/viewfrm.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/sfxuno.hxx>

using namespace com::sun::star;

// **************************************************************************
struct SfxVersionInfo
{
    String                  aName;
    String                  aComment;
    String                  aAuthor;
    DateTime                aCreationDate;

                            SfxVersionInfo();
                            SfxVersionInfo( const SfxVersionInfo& rInfo )
                            { *this = rInfo; }

    SfxVersionInfo&         operator=( const SfxVersionInfo &rInfo )
                            {
                                aName = rInfo.aName;
                                aComment = rInfo.aComment;
                                aAuthor = rInfo.aAuthor;
                                aCreationDate = rInfo.aCreationDate;
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

                            SfxVersionTableDtor( const SfxVersionTableDtor &rCpy ) :
                                _SfxVersionTable( rCpy )
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
        pInfo->aAuthor = rInfo[n].Author;

        Date aDate ( rInfo[n].TimeStamp.Day, rInfo[n].TimeStamp.Month, rInfo[n].TimeStamp.Year );
        Time aTime ( rInfo[n].TimeStamp.Hours, rInfo[n].TimeStamp.Minutes, rInfo[n].TimeStamp.Seconds, rInfo[n].TimeStamp.HundredthSeconds );

        pInfo->aCreationDate = DateTime( aDate, aTime );
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

static String ConvertDateTime_Impl(const DateTime& rTime, const LocaleDataWrapper& rWrapper)
{
     const String pDelim ( DEFINE_CONST_UNICODE( ", "));
     String aStr(rWrapper.getDate(rTime));
     aStr += pDelim;
     aStr += rWrapper.getTime(rTime, sal_True, sal_False);
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
        (*pString) += ConvertDateTime_Impl( pInfo->aCreationDate, aLocaleWrapper );
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

SfxVersionDialog::SfxVersionDialog ( SfxViewFrame* pVwFrame, sal_Bool bIsSaveVersionOnClose )
    : SfxModalDialog( NULL, SfxResId( DLG_VERSIONS ) )
    , aNewGroup( this, SfxResId( GB_NEWVERSIONS ) )
    , aSaveButton( this, SfxResId( PB_SAVE ) )
    , aSaveCheckBox( this, SfxResId( CB_SAVEONCLOSE ) )
    , aExistingGroup( this, SfxResId( GB_OLDVERSIONS ) )
    , aDateTimeText( this, SfxResId( FT_DATETIME ) )
    , aSavedByText( this, SfxResId( FT_SAVEDBY ) )
    , aCommentText( this, SfxResId( FT_COMMENTS ) )
    , aVersionBox( this, SfxResId( TLB_VERSIONS ) )
    , aCloseButton( this, SfxResId( PB_CLOSE ) )
    , aOpenButton( this, SfxResId( PB_OPEN ) )
    , aViewButton( this, SfxResId( PB_VIEW ) )
    , aDeleteButton( this, SfxResId( PB_DELETE ) )
    , aCompareButton( this, SfxResId( PB_COMPARE ) )
    , aHelpButton( this, SfxResId( PB_HELP ) )
    , pViewFrame( pVwFrame )
    , mpTable( NULL )
    , mpLocaleWrapper( NULL )
    , mbIsSaveVersionOnClose( bIsSaveVersionOnClose )
{
    FreeResource();

    Link aClickLink = LINK( this, SfxVersionDialog, ButtonHdl_Impl );
    aViewButton.SetClickHdl ( aClickLink );
    aSaveButton.SetClickHdl ( aClickLink );
    aDeleteButton.SetClickHdl ( aClickLink );
    aCompareButton.SetClickHdl ( aClickLink );
    aOpenButton.SetClickHdl ( aClickLink );
    aSaveCheckBox.SetClickHdl ( aClickLink );

    aVersionBox.SetSelectHdl( LINK( this, SfxVersionDialog, SelectHdl_Impl ) );
    aVersionBox.SetDoubleClickHdl( LINK( this, SfxVersionDialog, DClickHdl_Impl ) );

    aVersionBox.GrabFocus();
    aVersionBox.SetStyle( aVersionBox.GetStyle() | WB_HSCROLL | WB_CLIPCHILDREN );
    aVersionBox.SetSelectionMode( SINGLE_SELECTION );
    aVersionBox.SetTabs( &nTabs_Impl[0], MAP_APPFONT );
    aVersionBox.Resize();   // OS: Hack fuer richtige Selektion
    RecalcDateColumn();

    // set dialog title (filename or docinfo title)
    String sText = GetText();
    ( sText += ' ' ) += pViewFrame->GetObjectShell()->GetTitle();
    SetText( sText );

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
    uno::Sequence < util::RevisionTag > aVersions = pMedium->GetVersionList( true );
    delete mpTable;
    mpTable = new SfxVersionTableDtor( aVersions );
    {
        for ( sal_uInt16 n = 0; n < mpTable->Count(); ++n )
        {
            SfxVersionInfo *pInfo = mpTable->GetObject(n);
            String aEntry = ConvertDateTime_Impl( pInfo->aCreationDate, *mpLocaleWrapper );
            aEntry += '\t';
            aEntry += pInfo->aAuthor;
            aEntry += '\t';
            aEntry += ConvertWhiteSpaces_Impl( pInfo->aComment );
            SvLBoxEntry *pEntry = aVersionBox.InsertEntry( aEntry );
            pEntry->SetUserData( pInfo );
        }
    }

    aSaveCheckBox.Check( mbIsSaveVersionOnClose );

    sal_Bool bEnable = !pObjShell->IsReadOnly();
    aSaveButton.Enable( bEnable );
    aSaveCheckBox.Enable( bEnable );

    aOpenButton.Disable();
    aViewButton.Disable();
    aDeleteButton.Disable();
    aCompareButton.Disable();

    SelectHdl_Impl( &aVersionBox );
}

SfxVersionDialog::~SfxVersionDialog ()
{
    delete mpTable;
    delete mpLocaleWrapper;
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();

    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();
    sal_uIntPtr nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
    SfxInt16Item aItem( SID_VERSION, (short)nPos+1 );
    SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
    SfxStringItem aFile( SID_FILE_NAME, pObjShell->GetMedium()->GetName() );

    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( GetEncryptionData_Impl( pObjShell->GetMedium()->GetItemSet(), aEncryptionData ) )
    {
        // there is a password, it should be used during the opening
        SfxUnoAnyItem aEncryptionDataItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) );
        pViewFrame->GetDispatcher()->Execute(
            SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aFile, &aItem, &aTarget, &aReferer, &aEncryptionDataItem, 0L );
    }
    else
        pViewFrame->GetDispatcher()->Execute(
            SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aFile, &aItem, &aTarget, &aReferer, 0L );

    Close();
}

void SfxVersionDialog::RecalcDateColumn()
{
    // recalculate the datetime column width
    DateTime aNow;
    mpLocaleWrapper = new LocaleDataWrapper(
        ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    String sDateTime = ConvertDateTime_Impl( aNow, *mpLocaleWrapper );
    long nWidth = aVersionBox.GetTextWidth( sDateTime );
    nWidth += 15; // a little offset
    long nTab = aVersionBox.GetTab(1);
    if ( nWidth > nTab )
    {
        // resize columns
        long nDelta = nWidth - nTab;
        aVersionBox.SetTab( 1, nTab + nDelta, MAP_PIXEL );
        nTab = aVersionBox.GetTab(2);
        aVersionBox.SetTab( 2, nTab + nDelta, MAP_PIXEL );

        // resize and move header
        Size aSize = aDateTimeText.GetSizePixel();
        aSize.Width() += nDelta;
        aDateTimeText.SetSizePixel( aSize );
        Point aPos = aSavedByText.GetPosPixel();
        aPos.X() += nDelta;
        aSavedByText.SetPosPixel( aPos );
        aPos = aCommentText.GetPosPixel();
        aPos.X() += nDelta;
        aCommentText.SetPosPixel( aPos );
    }
}

IMPL_LINK( SfxVersionDialog, DClickHdl_Impl, Control*, EMPTYARG )
{
    Open_Impl();
    return 0L;
}

IMPL_LINK( SfxVersionDialog, SelectHdl_Impl, Control*, EMPTYARG )
{
    bool bEnable = ( aVersionBox.FirstSelected() != NULL );
    SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
    aDeleteButton.Enable( bEnable!= false && !pObjShell->IsReadOnly() );
    aOpenButton.Enable( bEnable!= false );
    aViewButton.Enable( bEnable!= false );

    const SfxPoolItem *pDummy=NULL;
    SfxItemState eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_MERGE, pDummy );
    eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_COMPARE, pDummy );
    aCompareButton.Enable( bEnable!= false && eState >= SFX_ITEM_AVAILABLE );

    return 0L;
}

IMPL_LINK( SfxVersionDialog, ButtonHdl_Impl, Button*, pButton )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SvLBoxEntry *pEntry = aVersionBox.FirstSelected();

    if ( pButton == &aSaveCheckBox )
    {
        mbIsSaveVersionOnClose = aSaveCheckBox.IsChecked();
    }
    else if ( pButton == &aSaveButton )
    {
        SfxVersionInfo aInfo;
        aInfo.aAuthor = SvtUserOptions().GetFullName();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, aInfo, sal_True );
        short nRet = pDlg->Execute();
        if ( nRet == RET_OK )
        {
            SfxStringItem aComment( SID_DOCINFO_COMMENTS, aInfo.aComment );
            pObjShell->SetModified( sal_True );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aComment;
            aItems[1] = NULL;
            pViewFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, aItems, 0 );
            aVersionBox.SetUpdateMode( sal_False );
            aVersionBox.Clear();
            Init_Impl();
            aVersionBox.SetUpdateMode( sal_True );
        }

        delete pDlg;
    }
    if ( pButton == &aDeleteButton && pEntry )
    {
        pObjShell->GetMedium()->RemoveVersion_Impl( ((SfxVersionInfo*) pEntry->GetUserData())->aName );
        pObjShell->SetModified( sal_True );
        aVersionBox.SetUpdateMode( sal_False );
        aVersionBox.Clear();
        Init_Impl();
        aVersionBox.SetUpdateMode( sal_True );
    }
    else if ( pButton == &aOpenButton && pEntry )
    {
        Open_Impl();
    }
    else if ( pButton == &aViewButton && pEntry )
    {
        SfxVersionInfo* pInfo = (SfxVersionInfo*) pEntry->GetUserData();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl( this, *pInfo, sal_False );
        pDlg->Execute();
        delete pDlg;
    }
    else if ( pEntry && pButton == &aCompareButton )
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        sal_uIntPtr nPos = aVersionBox.GetModel()->GetRelPos( pEntry );
        aSet.Put( SfxInt16Item( SID_VERSION, (short)nPos+1 ) );
        aSet.Put( SfxStringItem( SID_FILE_NAME, pObjShell->GetMedium()->GetName() ) );

        SfxItemSet* pSet = pObjShell->GetMedium()->GetItemSet();
        SFX_ITEMSET_ARG( pSet, pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        SFX_ITEMSET_ARG( pSet, pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
        if ( pFilterItem )
            aSet.Put( *pFilterItem );
        if ( pFilterOptItem )
            aSet.Put( *pFilterOptItem );

        pViewFrame->GetDispatcher()->Execute( SID_DOCUMENT_COMPARE, SFX_CALLMODE_ASYNCHRON, aSet );
        Close();
    }

    return 0L;
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl ( Window *pParent, SfxVersionInfo& rInfo, sal_Bool bEdit )
    : SfxModalDialog( pParent, SfxResId( DLG_COMMENTS ) )
    , aDateTimeText( this, SfxResId( FT_DATETIME ) )
    , aSavedByText( this, SfxResId( FT_SAVEDBY ) )
    , aEdit( this, SfxResId( ME_VERSIONS ) )
    , aOKButton( this, SfxResId( PB_OK ) )
    , aCancelButton( this, SfxResId( PB_CANCEL ) )
    , aCloseButton( this, SfxResId( PB_CLOSE ) )
    , aHelpButton( this, SfxResId( PB_HELP ) )
    , pInfo( &rInfo )
{
    FreeResource();

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    aDateTimeText.SetText( aDateTimeText.GetText().Append(ConvertDateTime_Impl( pInfo->aCreationDate, aLocaleWrapper )) );
    aSavedByText.SetText( aSavedByText.GetText().Append(pInfo->aAuthor) );
    aEdit.SetText( rInfo.aComment );

    aCloseButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );
    aOKButton.SetClickHdl ( LINK( this, SfxViewVersionDialog_Impl, ButtonHdl ) );

    aEdit.GrabFocus();
    if ( !bEdit )
    {
        aOKButton.Hide();
        aCancelButton.Hide();
        aEdit.SetReadOnly( sal_True );
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

