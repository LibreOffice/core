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

#include <sal/config.h>

#include <com/sun/star/document/XCmisDocument.hpp>

#include <unotools/localedatawrapper.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/datetime.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/miscopt.hxx>

#include <versdlg.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/sfxuno.hxx>
#include <memory>
#include <vector>

using namespace com::sun::star;

struct SfxVersionInfo
{
    OUString                aName;
    OUString                aComment;
    OUString                aAuthor;
    DateTime                aCreationDate;

                            SfxVersionInfo();
};

class SfxVersionTableDtor
{
private:
    std::vector<std::unique_ptr<SfxVersionInfo>> aTableList;
public:
    explicit                SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo );
    explicit                SfxVersionTableDtor( const uno::Sequence < document::CmisVersion > & rInfo );
                            SfxVersionTableDtor(const SfxVersionTableDtor&) = delete;
    SfxVersionTableDtor&    operator=(const SfxVersionTableDtor&) = delete;

    size_t                  size() const
                            { return aTableList.size(); }

    SfxVersionInfo*         at( size_t i ) const
                            { return aTableList[ i ].get(); }
};

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < util::RevisionTag >& rInfo )
{
    for ( sal_Int32 n=0; n<rInfo.getLength(); n++ )
    {
        std::unique_ptr<SfxVersionInfo> pInfo(new SfxVersionInfo);
        pInfo->aName = rInfo[n].Identifier;
        pInfo->aComment = rInfo[n].Comment;
        pInfo->aAuthor = rInfo[n].Author;

        pInfo->aCreationDate = DateTime( rInfo[n].TimeStamp );
        aTableList.push_back( std::move(pInfo) );
    }
}

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < document::CmisVersion >& rInfo )
{
    for ( sal_Int32 n=0; n<rInfo.getLength(); n++ )
    {
        std::unique_ptr<SfxVersionInfo> pInfo(new SfxVersionInfo);
        pInfo->aName = rInfo[n].Id;
        pInfo->aComment = rInfo[n].Comment;
        pInfo->aAuthor = rInfo[n].Author;

        pInfo->aCreationDate = DateTime( rInfo[n].TimeStamp );
        aTableList.push_back( std::move(pInfo) );
    }
}

SfxVersionInfo::SfxVersionInfo()
    : aCreationDate( DateTime::EMPTY )
{
}

void SfxVersionsTabListBox_Impl::KeyInput(const KeyEvent& rKeyEvent)
{
    const vcl::KeyCode& rCode = rKeyEvent.GetKeyCode();
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
        aAuthors.insert(static_cast<SfxVersionInfo*>(pEntry->GetUserData())->aAuthor);
    }

    long nMaxAuthorWidth = nRest/4;
    for (auto const& author : aAuthors)
    {
        nMaxAuthorWidth = std::max(nMaxAuthorWidth, GetTextWidth(author));
        if (nMaxAuthorWidth > nRest/2)
        {
            nMaxAuthorWidth = nRest/2;
            break;
        }
    }

    long aTabPositions[] = { 0, nMax, nMax + nMaxAuthorWidth };
    SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions, MapUnit::MapPixel);
}

SfxVersionDialog::SfxVersionDialog ( SfxViewFrame* pVwFrame, bool bIsSaveVersionOnClose )
    : SfxModalDialog(nullptr, "VersionsOfDialog", "sfx/ui/versionsofdialog.ui")
    , pViewFrame(pVwFrame)
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
    aControlSize = pContainer->LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    m_pVersionBox = VclPtr<SfxVersionsTabListBox_Impl>::Create(*pContainer, WB_TABSTOP);

    Link<Button*,void> aClickLink = LINK( this, SfxVersionDialog, ButtonHdl_Impl );
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
    m_pVersionBox->SetSelectionMode( SelectionMode::Single );

    long aTabPositions[] = { 0, 0, 0 };
    m_pVersionBox->SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions);
    OUString sHeader1(get<FixedText>("datetime")->GetText());
    OUString sHeader2(get<FixedText>("savedby")->GetText());
    OUString sHeader3(get<FixedText>("comments")->GetText());
    OUString sHeader = sHeader1 + "\t" + sHeader2 + "\t" + sHeader3;
    m_pVersionBox->InsertHeaderEntry(sHeader);

    HeaderBar &rBar = m_pVersionBox->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED;
    nBits &= ~HeaderBarItemBits::CLICKABLE;
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

static OUString ConvertWhiteSpaces_Impl( const OUString& rText )
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
    m_pTable.reset(new SfxVersionTableDtor( aVersions ));
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

    m_pSaveCheckBox->Check( m_bIsSaveVersionOnClose );

    bool bEnable = !pObjShell->IsReadOnly();
    m_pSaveButton->Enable( bEnable );
    m_pSaveCheckBox->Enable( bEnable );

    m_pOpenButton->Disable();
    m_pViewButton->Disable();
    m_pDeleteButton->Disable();
    m_pCompareButton->Disable();

    SvtMiscOptions miscOptions;
    if ( !miscOptions.IsExperimentalMode() )
        m_pCmisButton->Hide( );
    uno::Reference<document::XCmisDocument> xCmisDoc(pObjShell->GetModel(), uno::UNO_QUERY);
    if (xCmisDoc && xCmisDoc->isVersionable())
        m_pCmisButton->Enable();
    else
        m_pCmisButton->Disable();

    SelectHdl_Impl(m_pVersionBox);
}

SfxVersionDialog::~SfxVersionDialog()
{
    disposeOnce();
}

void SfxVersionDialog::dispose()
{
    m_pTable.reset();
    m_pVersionBox.disposeAndClear();
    m_pSaveButton.clear();
    m_pSaveCheckBox.clear();
    m_pOpenButton.clear();
    m_pViewButton.clear();
    m_pDeleteButton.clear();
    m_pCompareButton.clear();
    m_pCmisButton.clear();
    SfxModalDialog::dispose();
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();

    SvTreeListEntry *pEntry = m_pVersionBox->FirstSelected();
    sal_uIntPtr nPos = SvTreeList::GetRelPos( pEntry );
    SfxInt16Item aItem( SID_VERSION, static_cast<short>(nPos)+1 );
    SfxStringItem aTarget( SID_TARGETNAME, "_blank" );
    SfxStringItem aReferer( SID_REFERER, "private:user" );
    SfxStringItem aFile( SID_FILE_NAME, pObjShell->GetMedium()->GetName() );

    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( GetEncryptionData_Impl( pObjShell->GetMedium()->GetItemSet(), aEncryptionData ) )
    {
        // there is a password, it should be used during the opening
        SfxUnoAnyItem aEncryptionDataItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData ) );
        pViewFrame->GetDispatcher()->ExecuteList(
            SID_OPENDOC, SfxCallMode::ASYNCHRON,
            { &aFile, &aItem, &aTarget, &aReferer, &aEncryptionDataItem });
    }
    else
    {
        pViewFrame->GetDispatcher()->ExecuteList(
            SID_OPENDOC, SfxCallMode::ASYNCHRON,
            { &aFile, &aItem, &aTarget, &aReferer });
    }

    Close();
}

IMPL_LINK_NOARG(SfxVersionDialog, DClickHdl_Impl, SvTreeListBox*, bool)
{
    Open_Impl();
    return false;
}

IMPL_LINK_NOARG(SfxVersionDialog, SelectHdl_Impl, SvTreeListBox*, void)
{
    bool bEnable = ( m_pVersionBox->FirstSelected() != nullptr );
    SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
    m_pDeleteButton->Enable(bEnable && !pObjShell->IsReadOnly());
    m_pOpenButton->Enable(bEnable);
    m_pViewButton->Enable(bEnable);

    const SfxPoolItem *pDummy=nullptr;
    pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_MERGE, pDummy );
    SfxItemState eState = pViewFrame->GetDispatcher()->QueryState( SID_DOCUMENT_COMPARE, pDummy );
    m_pCompareButton->Enable(bEnable && eState >= SfxItemState::DEFAULT);
}

IMPL_LINK( SfxVersionDialog, ButtonHdl_Impl, Button*, pButton, void )
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
        SfxViewVersionDialog_Impl aDlg(GetFrameWeld(), aInfo, true);
        short nRet = aDlg.run();
        if ( nRet == RET_OK )
        {
            SfxStringItem aComment( SID_DOCINFO_COMMENTS, aInfo.aComment );
            pObjShell->SetModified();
            const SfxPoolItem* aItems[2];
            aItems[0] = &aComment;
            aItems[1] = nullptr;
            pViewFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, aItems );
            m_pVersionBox->SetUpdateMode( false );
            m_pVersionBox->Clear();
            Init_Impl();
            m_pVersionBox->SetUpdateMode( true );
        }
    }
    if (pButton == m_pDeleteButton && pEntry)
    {
        pObjShell->GetMedium()->RemoveVersion_Impl( static_cast<SfxVersionInfo*>(pEntry->GetUserData())->aName );
        pObjShell->SetModified();
        m_pVersionBox->SetUpdateMode( false );
        m_pVersionBox->Clear();
        Init_Impl();
        m_pVersionBox->SetUpdateMode( true );
    }
    else if (pButton == m_pOpenButton && pEntry)
    {
        Open_Impl();
    }
    else if (pButton == m_pViewButton && pEntry)
    {
        SfxVersionInfo* pInfo = static_cast<SfxVersionInfo*>(pEntry->GetUserData());
        SfxViewVersionDialog_Impl aDlg(GetFrameWeld(), *pInfo, false);
        aDlg.run();
    }
    else if (pEntry && pButton == m_pCompareButton)
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        sal_uIntPtr nPos = SvTreeList::GetRelPos( pEntry );
        aSet.Put( SfxInt16Item( SID_VERSION, static_cast<short>(nPos)+1 ) );
        aSet.Put( SfxStringItem( SID_FILE_NAME, pObjShell->GetMedium()->GetName() ) );

        SfxItemSet* pSet = pObjShell->GetMedium()->GetItemSet();
        const SfxStringItem* pFilterItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_FILTER_NAME, false);
        const SfxStringItem* pFilterOptItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_FILE_FILTEROPTIONS, false);
        if ( pFilterItem )
            aSet.Put( *pFilterItem );
        if ( pFilterOptItem )
            aSet.Put( *pFilterOptItem );

        pViewFrame->GetDispatcher()->Execute( SID_DOCUMENT_COMPARE, SfxCallMode::ASYNCHRON, aSet );
        Close();
    }
    else if (pButton == m_pCmisButton)
    {
        VclPtrInstance< SfxCmisVersionsDialog > pDlg(pViewFrame);
        pDlg->Execute();
    }
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl(weld::Window *pParent, SfxVersionInfo& rInfo, bool bEdit)
    : GenericDialogController(pParent, "sfx/ui/versioncommentdialog.ui", "VersionCommentDialog")
    , m_rInfo(rInfo)
    , m_xDateTimeText(m_xBuilder->weld_label("timestamp"))
    , m_xSavedByText(m_xBuilder->weld_label("author"))
    , m_xEdit(m_xBuilder->weld_text_view("textview"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xCancelButton(m_xBuilder->weld_button("cancel"))
    , m_xCloseButton(m_xBuilder->weld_button("close"))
{
    OUString sAuthor = rInfo.aAuthor.isEmpty() ? SfxResId(STR_NO_NAME_SET) : rInfo.aAuthor;

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_xDateTimeText->set_label(m_xDateTimeText->get_label() + formatTime(rInfo.aCreationDate, rLocaleWrapper));
    m_xSavedByText->set_label(m_xSavedByText->get_label() + sAuthor);
    m_xEdit->set_text(rInfo.aComment);
    m_xEdit->set_size_request(40 * m_xEdit->get_approximate_digit_width(),
                              7 * m_xEdit->get_text_height());
    m_xOKButton->connect_clicked(LINK(this, SfxViewVersionDialog_Impl, ButtonHdl));

    if (!bEdit)
    {
        m_xOKButton->hide();
        m_xCancelButton->hide();
        m_xEdit->set_editable(false);
        m_xDialog->set_title(SfxResId(STR_VIEWVERSIONCOMMENT));
        m_xCloseButton->grab_focus();
    }
    else
    {
        m_xDateTimeText->hide();
        m_xCloseButton->hide();
        m_xEdit->grab_focus();
    }
}

IMPL_LINK(SfxViewVersionDialog_Impl, ButtonHdl, weld::Button&, rButton, void)
{
    assert(&rButton == m_xOKButton.get());
    (void)rButton;
    m_rInfo.aComment = m_xEdit->get_text();
    m_xDialog->response(RET_OK);
}

SfxCmisVersionsDialog::SfxCmisVersionsDialog ( SfxViewFrame* pVwFrame )
    : SfxModalDialog(nullptr, "VersionsCmisDialog", "sfx/ui/versionscmis.ui")
    , pViewFrame(pVwFrame)
{
    get(m_pOpenButton, "open");
    get(m_pViewButton, "show");
    get(m_pDeleteButton, "delete");
    get(m_pCompareButton, "compare");

    SvSimpleTableContainer *pContainer = get<SvSimpleTableContainer>("versions");
    Size aControlSize(260, 114);
    aControlSize = pContainer->LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pContainer->set_width_request(aControlSize.Width());
    pContainer->set_height_request(aControlSize.Height());

    m_pVersionBox = VclPtr<SfxVersionsTabListBox_Impl>::Create(*pContainer, WB_TABSTOP);

    m_pVersionBox->GrabFocus();
    m_pVersionBox->SetStyle( m_pVersionBox->GetStyle() | WB_HSCROLL | WB_CLIPCHILDREN );
    m_pVersionBox->SetSelectionMode( SelectionMode::Single );

    long aTabPositions[] = { 0, 0, 0 };
    m_pVersionBox->SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions);
    OUString sHeader1(get<FixedText>("datetime")->GetText());
    OUString sHeader2(get<FixedText>("savedby")->GetText());
    OUString sHeader3(get<FixedText>("comments")->GetText());
    OUString sHeader = sHeader1 + "\t" + sHeader2 + "\t" + sHeader3;
    m_pVersionBox->InsertHeaderEntry(sHeader);

    HeaderBar &rBar = m_pVersionBox->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED;
    nBits &= ~HeaderBarItemBits::CLICKABLE;
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
    disposeOnce();
}

void SfxCmisVersionsDialog::dispose()
{
    m_pTable.reset();
    m_pVersionBox.disposeAndClear();
    m_pOpenButton.clear();
    m_pViewButton.clear();
    m_pDeleteButton.clear();
    m_pCompareButton.clear();
    SfxModalDialog::dispose();
}

void SfxCmisVersionsDialog::LoadVersions()
{
    SfxObjectShell *pObjShell = pViewFrame->GetObjectShell();
    uno::Sequence < document::CmisVersion > aVersions = pObjShell->GetCmisVersions( );
    m_pTable.reset(new SfxVersionTableDtor( aVersions ));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
