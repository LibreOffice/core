/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <tools/datetime.hxx>
#include "svtools/treelistentry.hxx"

#include "versdlg.hxx"
#include "dialog.hrc"
#include <sfx2/dialoghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/sfxuno.hxx>
#include <vector>

using namespace com::sun::star;
using ::std::vector;

// **************************************************************************
struct SfxVersionInfo
{
    OUString                aName;
    OUString                aComment;
    OUString                aAuthor;
    DateTime                aCreationDate;

                            SfxVersionInfo();
                            SfxVersionInfo( const SfxVersionInfo& rInfo )
                                : aCreationDate( DateTime::EMPTY )
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

typedef vector< SfxVersionInfo* > _SfxVersionTable;

class SfxVersionTableDtor
{
private:
    _SfxVersionTable        aTableList;
public:
                            SfxVersionTableDtor( const SfxVersionTableDtor &rCpy )
                            { *this = rCpy; }

                            SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo );
                            SfxVersionTableDtor( const uno::Sequence < document::CmisVersion > & rInfo );
                            ~SfxVersionTableDtor()
                            { DelDtor(); }

    SfxVersionTableDtor&    operator=( const SfxVersionTableDtor &rCpy );
    void                    DelDtor();
    SvStream&               Read( SvStream & );
    SvStream&               Write( SvStream & ) const;

    size_t                  size() const
                            { return aTableList.size(); }

    SfxVersionInfo*         at( size_t i ) const
                            { return aTableList[ i ]; }
};

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo )
{
    for ( sal_Int32 n=0; n<(sal_Int32)rInfo.getLength(); n++ )
    {
        SfxVersionInfo* pInfo = new SfxVersionInfo;
        pInfo->aName = rInfo[n].Identifier;
        pInfo->aComment = rInfo[n].Comment;
        pInfo->aAuthor = rInfo[n].Author;

        Date aDate ( rInfo[n].TimeStamp.Day,   rInfo[n].TimeStamp.Month,   rInfo[n].TimeStamp.Year );
        Time aTime ( rInfo[n].TimeStamp.Hours, rInfo[n].TimeStamp.Minutes, rInfo[n].TimeStamp.Seconds, rInfo[n].TimeStamp.NanoSeconds );

        pInfo->aCreationDate = DateTime( aDate, aTime );
        aTableList.push_back( pInfo );
    }
}

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < document::CmisVersion >& rInfo )
{
    for ( sal_Int32 n=0; n<(sal_Int32)rInfo.getLength(); n++ )
    {
        SfxVersionInfo* pInfo = new SfxVersionInfo;
        pInfo->aName = rInfo[n].Id;
        pInfo->aComment = rInfo[n].Comment;
        pInfo->aAuthor = rInfo[n].Author;

        Date aDate ( rInfo[n].TimeStamp.Day,   rInfo[n].TimeStamp.Month,   rInfo[n].TimeStamp.Year );
        Time aTime ( rInfo[n].TimeStamp.Hours, rInfo[n].TimeStamp.Minutes, rInfo[n].TimeStamp.Seconds, rInfo[n].TimeStamp.NanoSeconds );

        pInfo->aCreationDate = DateTime( aDate, aTime );
        aTableList.push_back( pInfo );
    }
}

void SfxVersionTableDtor::DelDtor()
{
    for ( size_t i = 0, n = aTableList.size(); i < n; ++i )
        delete aTableList[ i ];
    aTableList.clear();
}

SfxVersionTableDtor& SfxVersionTableDtor::operator=( const SfxVersionTableDtor& rTbl )
{
    DelDtor();
    for ( size_t i = 0, n = rTbl.size(); i < n; ++i )
    {
        SfxVersionInfo* pNew = new SfxVersionInfo( *(rTbl.at( i )) );
        aTableList.push_back( pNew );
    }
    return *this;
}

//----------------------------------------------------------------
SfxVersionInfo::SfxVersionInfo()
    : aCreationDate( DateTime::EMPTY )
{
}

void SfxVersionsTabListBox_Impl::KeyInput(const KeyEvent& rKeyEvent)
{
    const KeyCode& rCode = rKeyEvent.GetKeyCode();
    switch (rCode.GetCode())
    {
        case KEY_RETURN :
        case KEY_ESCAPE :
        case KEY_TAB :
        {
            Dialog *pParent = GetParentDialog();
            if (pParent)
                pParent->KeyInput(rKeyEvent);
            else
                SvSimpleTable::KeyInput(rKeyEvent);
            break;
        }
        default:
            SvSimpleTable::KeyInput( rKeyEvent );
            break;
    }
}

void SfxVersionsTabListBox_Impl::Resize()
{
    SvSimpleTable::Resize();
    if (isInitialLayout(this))
        setColSizes();
}

void SfxVersionsTabListBox_Impl::setColSizes()
{
    HeaderBar &rBar = GetTheHeaderBar();
    if (rBar.GetItemCount() < 3)
        return;

    // recalculate the datetime column width
    long nWidestTime(GetTextWidth(getWidestTime(Application::GetSettings().GetLocaleDataWrapper())));
    long nW1 = rBar.GetTextWidth(rBar.GetItemText(1));

    long nMax = std::max(nWidestTime, nW1) + 12; // max width + a little offset
    const long nRest = GetSizePixel().Width() - nMax;

    std::set<OUString> aAuthors;
    SfxVersionInfo aInfo;
    aAuthors.insert(SvtUserOptions().GetFullName());

    for (SvTreeListEntry* pEntry = First(); pEntry; pEntry = Next(pEntry))
    {
        aAuthors.insert(((SfxVersionInfo*)pEntry->GetUserData())->aAuthor);
    }

    long nMaxAuthorWidth = nRest/4;
    for (std::set<OUString>::iterator aI = aAuthors.begin(), aEnd = aAuthors.end(); aI != aEnd; ++aI)
    {
        nMaxAuthorWidth = std::max(nMaxAuthorWidth, GetTextWidth(*aI));
        if (nMaxAuthorWidth > nRest/2)
        {
            nMaxAuthorWidth = nRest/2;
            break;
        }
    }

    long aStaticTabs[] = { 3, 0, 0, 0 };
    aStaticTabs[2] = nMax;
    aStaticTabs[3] = nMax + nMaxAuthorWidth;
    SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

SfxVersionDialog::SfxVersionDialog ( SfxViewFrame* pVwFrame, sal_Bool bIsSaveVersionOnClose )
    : SfxModalDialog(NULL, "VersionsOfDialog", "sfx/ui/versionsofdialog.ui")
    , pViewFrame(pVwFrame)
    , m_pTable(NULL)
    , m_bIsSaveVersionOnClose(bIsSaveVersionOnClose)
{
    get(m_pSaveButton, "save");
    get(m_pSaveCheckBox, "always");
    get(m_pOpenButton, "open");
    get(m_pViewButton, "show");
    get(m_pDeleteButton, "delete");
    get(m_pCompareButton, "compare");
    get(m_pCmisButton, "cmis");

    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("versions");
    Size aControlSize(260, 114);
    aControlSize = pContainer->LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    m_pVersionBox = new SfxVersionsTabListBox_Impl(*pContainer, WB_TABSTOP);

    Link aClickLink = LINK( this, SfxVersionDialog, ButtonHdl_Impl );
    m_pViewButton->SetClickHdl ( aClickLink );
    m_pSaveButton->SetClickHdl ( aClickLink );
    m_pDeleteButton->SetClickHdl ( aClickLink );
    m_pCompareButton->SetClickHdl ( aClickLink );
    m_pOpenButton->SetClickHdl ( aClickLink );
    m_pSaveCheckBox->SetClickHdl ( aClickLink );
    m_pCmisButton->SetClickHdl ( aClickLink );

    m_pVersionBox->SetSelectHdl( LINK( this, SfxVersionDialog, SelectHdl_Impl ) );
    m_pVersionBox->SetDoubleClickHdl( LINK( this, SfxVersionDialog, DClickHdl_Impl ) );

    m_pVersionBox->GrabFocus();
    m_pVersionBox->SetStyle( m_pVersionBox->GetStyle() | WB_HSCROLL | WB_CLIPCHILDREN );
    m_pVersionBox->SetSelectionMode( SINGLE_SELECTION );

    long nTabs_Impl[] = { 3, 0, 0, 0 };


    m_pVersionBox->SvSimpleTable::SetTabs(&nTabs_Impl[0]);
    OUString sHeader1(get<FixedText>("datetime")->GetText());
    OUString sHeader2(get<FixedText>("savedby")->GetText());
    OUString sHeader3(get<FixedText>("comments")->GetText());
    OUStringBuffer sHeader;
    sHeader.append(sHeader1).append("\t").append(sHeader2)
        .append("\t ").append(sHeader3);
    m_pVersionBox->InsertHeaderEntry(sHeader.makeStringAndClear());

    HeaderBar &rBar = m_pVersionBox->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HIB_FIXEDPOS | HIB_FIXED;
    nBits &= ~HIB_CLICKABLE;
    rBar.SetItemBits(1, nBits);
    rBar.SetItemBits(2, nBits);
    rBar.SetItemBits(3, nBits);

    m_pVersionBox->Resize();       // OS: Hack for correct selection


    // set dialog title (filename or docinfo title)
    OUString sText = GetText();
    sText = sText + " " + pViewFrame->GetObjectShell()->GetTitle();
    SetText( sText );

    Init_Impl();

    m_pVersionBox->setColSizes();
}

String ConvertWhiteSpaces_Impl( const OUString& rText )
{
    // converted linebreaks and tabs to blanks; it's necessary for the display
    OUStringBuffer sConverted;
    const sal_Unicode* pChars = rText.getStr();
    while ( *pChars )
    {
        switch ( *pChars )
        {
            case '\n' :
            case '\t' :
                sConverted.append(' ');
                break;

            default:
                sConverted.append(*pChars);
        }

        ++pChars;
    }

    return sConverted.makeStringAndClear();
}

void SfxVersionDialog::Init_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SfxMedium* pMedium = pObjShell->GetMedium();
    uno::Sequence < util::RevisionTag > aVersions = pMedium->GetVersionList( true );
    delete m_pTable;
    m_pTable = new SfxVersionTableDtor( aVersions );
    {
        for ( size_t n = 0; n < m_pTable->size(); ++n )
        {
            SfxVersionInfo *pInfo = m_pTable->at( n );
            OUString aEntry = formatTime(pInfo->aCreationDate, Application::GetSettings().GetLocaleDataWrapper());
            aEntry += "\t";
            aEntry += pInfo->aAuthor;
            aEntry += "\t";
            aEntry += ConvertWhiteSpaces_Impl( pInfo->aComment );
            SvTreeListEntry *pEntry = m_pVersionBox->InsertEntry( aEntry );
            pEntry->SetUserData( pInfo );
        }
    }

    m_pSaveCheckBox->Check( m_bIsSaveVersionOnClose );

    sal_Bool bEnable = !pObjShell->IsReadOnly();
    m_pSaveButton->Enable( bEnable );
    m_pSaveCheckBox->Enable( bEnable );

    m_pOpenButton->Disable();
    m_pViewButton->Disable();
    m_pDeleteButton->Disable();
    m_pCompareButton->Disable();
    m_pCmisButton->Enable();

    SelectHdl_Impl(m_pVersionBox);
}

SfxVersionDialog::~SfxVersionDialog()
{
    delete m_pTable;
    delete m_pVersionBox;
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();

    SvTreeListEntry *pEntry = m_pVersionBox->FirstSelected();
    sal_uIntPtr nPos = m_pVersionBox->GetModel()->GetRelPos( pEntry );
    SfxInt16Item aItem( SID_VERSION, (short)nPos+1 );
    SfxStringItem aTarget( SID_TARGETNAME, "_blank" );
    SfxStringItem aReferer( SID_REFERER, "private:user" );
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

IMPL_LINK_NOARG(SfxVersionDialog, DClickHdl_Impl)
{
    Open_Impl();
    return 0L;
}

IMPL_LINK_NOARG(SfxVersionDialog, SelectHdl_Impl)
{
    bool bEnable = ( m_pVersionBox->FirstSelected() != NULL );
    SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
    m_pDeleteButton->Enable( bEnable!= false && !pObjShell->IsReadOnly() );
    m_pOpenButton->Enable( bEnable!= false );
    m_pViewButton->Enable( bEnable!= false );

    const SfxPoolItem *pDummy=NULL;
    SfxItemState eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_MERGE, pDummy );
    eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_COMPARE, pDummy );
    m_pCompareButton->Enable( bEnable!= false && eState >= SFX_ITEM_AVAILABLE );

    return 0L;
}

IMPL_LINK( SfxVersionDialog, ButtonHdl_Impl, Button*, pButton )
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    SvTreeListEntry *pEntry = m_pVersionBox->FirstSelected();

    if (pButton == m_pSaveCheckBox)
    {
        m_bIsSaveVersionOnClose = m_pSaveCheckBox->IsChecked();
    }
    else if (pButton == m_pSaveButton)
    {
        SfxVersionInfo aInfo;
        aInfo.aAuthor = SvtUserOptions().GetFullName();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl(this, aInfo, true);
        short nRet = pDlg->Execute();
        if ( nRet == RET_OK )
        {
            SfxStringItem aComment( SID_DOCINFO_COMMENTS, aInfo.aComment );
            pObjShell->SetModified( sal_True );
            const SfxPoolItem* aItems[2];
            aItems[0] = &aComment;
            aItems[1] = NULL;
            pViewFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, aItems, 0 );
            m_pVersionBox->SetUpdateMode( sal_False );
            m_pVersionBox->Clear();
            Init_Impl();
            m_pVersionBox->SetUpdateMode( sal_True );
        }

        delete pDlg;
    }
    if (pButton == m_pDeleteButton && pEntry)
    {
        pObjShell->GetMedium()->RemoveVersion_Impl( ((SfxVersionInfo*) pEntry->GetUserData())->aName );
        pObjShell->SetModified( sal_True );
        m_pVersionBox->SetUpdateMode( sal_False );
        m_pVersionBox->Clear();
        Init_Impl();
        m_pVersionBox->SetUpdateMode( sal_True );
    }
    else if (pButton == m_pOpenButton && pEntry)
    {
        Open_Impl();
    }
    else if (pButton == m_pViewButton && pEntry)
    {
        SfxVersionInfo* pInfo = (SfxVersionInfo*) pEntry->GetUserData();
        SfxViewVersionDialog_Impl* pDlg = new SfxViewVersionDialog_Impl(this, *pInfo, false);
        pDlg->Execute();
        delete pDlg;
    }
    else if (pEntry && pButton == m_pCompareButton)
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        sal_uIntPtr nPos = m_pVersionBox->GetModel()->GetRelPos( pEntry );
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
    else if (pButton == m_pCmisButton)
    {
        SfxCmisVersionsDialog* pDlg = new SfxCmisVersionsDialog(pViewFrame, false);
        pDlg->Execute();
        delete pDlg;
    }

    return 0L;
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl(Window *pParent, SfxVersionInfo& rInfo, bool bEdit)
    : SfxModalDialog(pParent, "VersionCommentDialog", "sfx/ui/versioncommentdialog.ui")
    , m_rInfo(rInfo)
{
    get(m_pDateTimeText, "timestamp");
    get(m_pSavedByText, "author");
    get(m_pEdit, "textview");
    get(m_pOKButton, "ok");
    get(m_pCancelButton, "cancel");
    get(m_pCloseButton, "close");

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_pDateTimeText->SetText(m_pDateTimeText->GetText() + formatTime(rInfo.aCreationDate, rLocaleWrapper));
    m_pSavedByText->SetText(m_pSavedByText->GetText() + rInfo.aAuthor);
    m_pEdit->SetText(rInfo.aComment);
    m_pEdit->set_height_request(7 * m_pEdit->GetTextHeight());
    m_pEdit->set_width_request(40 * m_pEdit->approximate_char_width());
    m_pOKButton->SetClickHdl(LINK(this, SfxViewVersionDialog_Impl, ButtonHdl));

    if (!bEdit)
    {
        m_pOKButton->Hide();
        m_pCancelButton->Hide();
        m_pEdit->SetReadOnly(true);
        SetText(SfxResId(STR_VIEWVERSIONCOMMENT));
        m_pCloseButton->GrabFocus();
    }
    else
    {
        m_pDateTimeText->Hide();
        m_pCloseButton->Hide();
        m_pEdit->GrabFocus();
    }
}

IMPL_LINK(SfxViewVersionDialog_Impl, ButtonHdl, Button*, pButton)
{
    assert(pButton == m_pOKButton);
    (void)pButton;
    m_rInfo.aComment = m_pEdit->GetText();
    EndDialog(RET_OK);
    return 0L;
}

SfxCmisVersionsDialog::SfxCmisVersionsDialog ( SfxViewFrame* pVwFrame, sal_Bool bIsSaveVersionOnClose )
    : SfxModalDialog(NULL, "VersionsCmisDialog", "sfx/ui/versionscmis.ui")
    , pViewFrame(pVwFrame)
    , m_pTable(NULL)
    , m_bIsSaveVersionOnClose(bIsSaveVersionOnClose)
{
    get(m_pSaveButton, "save");
    get(m_pSaveCheckBox, "always");
    get(m_pOpenButton, "open");
    get(m_pViewButton, "show");
    get(m_pDeleteButton, "delete");
    get(m_pCompareButton, "compare");

    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("versions");
    Size aControlSize(260, 114);
    aControlSize = pContainer->LogicToPixel(aControlSize, MAP_APPFONT);
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    m_pVersionBox = new SfxVersionsTabListBox_Impl(*pContainer, WB_TABSTOP);

    m_pVersionBox->GrabFocus();
    m_pVersionBox->SetStyle( m_pVersionBox->GetStyle() | WB_HSCROLL | WB_CLIPCHILDREN );
    m_pVersionBox->SetSelectionMode( SINGLE_SELECTION );

    long nTabs_Impl[] = { 3, 0, 0, 0 };

    m_pVersionBox->SvSimpleTable::SetTabs(&nTabs_Impl[0]);
    OUString sHeader1(get<FixedText>("datetime")->GetText());
    OUString sHeader2(get<FixedText>("savedby")->GetText());
    OUString sHeader3(get<FixedText>("comments")->GetText());
    OUStringBuffer sHeader;
    sHeader.append(sHeader1).append("\t").append(sHeader2)
        .append("\t ").append(sHeader3);
    m_pVersionBox->InsertHeaderEntry(sHeader.makeStringAndClear());

    HeaderBar &rBar = m_pVersionBox->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HIB_FIXEDPOS | HIB_FIXED;
    nBits &= ~HIB_CLICKABLE;
    rBar.SetItemBits(1, nBits);
    rBar.SetItemBits(2, nBits);
    rBar.SetItemBits(3, nBits);

    m_pVersionBox->Resize();

    OUString sText = GetText();
    sText = sText + " " + pViewFrame->GetObjectShell()->GetTitle();
    SetText( sText );

    LoadVersions();

    m_pVersionBox->setColSizes();

}

SfxCmisVersionsDialog::~SfxCmisVersionsDialog()
{
    delete m_pTable;
    delete m_pVersionBox;
}

void SfxCmisVersionsDialog::LoadVersions()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    uno::Sequence < document::CmisVersion > aVersions = pObjShell->GetCmisVersions( );
    delete m_pTable;
    m_pTable = new SfxVersionTableDtor( aVersions );
    {
        for ( size_t n = 0; n < m_pTable->size(); ++n )
        {
            SfxVersionInfo *pInfo = m_pTable->at( n );
            OUString aEntry = formatTime(pInfo->aCreationDate, Application::GetSettings().GetLocaleDataWrapper());
            aEntry += "\t";
            aEntry += pInfo->aAuthor;
            aEntry += "\t";
            aEntry += ConvertWhiteSpaces_Impl( pInfo->aComment );
            SvTreeListEntry *pEntry = m_pVersionBox->InsertEntry( aEntry );
            pEntry->SetUserData( pInfo );
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
