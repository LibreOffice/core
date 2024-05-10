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
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <officecfg/Office/Common.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <tools/datetime.hxx>

#include <versdlg.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dispatch.hxx>

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
    for ( const auto& rItem : rInfo )
    {
        std::unique_ptr<SfxVersionInfo> pInfo(new SfxVersionInfo);
        pInfo->aName = rItem.Identifier;
        pInfo->aComment = rItem.Comment;
        pInfo->aAuthor = rItem.Author;

        pInfo->aCreationDate = DateTime( rItem.TimeStamp );
        aTableList.push_back( std::move(pInfo) );
    }
}

SfxVersionTableDtor::SfxVersionTableDtor( const uno::Sequence < document::CmisVersion >& rInfo )
{
    for ( const auto& rItem : rInfo )
    {
        std::unique_ptr<SfxVersionInfo> pInfo(new SfxVersionInfo);
        pInfo->aName = rItem.Id;
        pInfo->aComment = rItem.Comment;
        pInfo->aAuthor = rItem.Author;

        pInfo->aCreationDate = DateTime( rItem.TimeStamp );
        aTableList.push_back( std::move(pInfo) );
    }
}

SfxVersionInfo::SfxVersionInfo()
    : aCreationDate( DateTime::EMPTY )
{
}

namespace
{
    void setColSizes(weld::TreeView& rVersionBox)
    {
        // recalculate the datetime column width
        int nWidestTime(rVersionBox.get_pixel_size(getWidestDateTime(Application::GetSettings().GetLocaleDataWrapper(), false)).Width());
        int nW1 = rVersionBox.get_pixel_size(rVersionBox.get_column_title(1)).Width();

        int nMax = std::max(nWidestTime, nW1) + 12; // max width + a little offset
        const int nRest = rVersionBox.get_preferred_size().Width() - nMax;

        std::set<OUString> aAuthors;
        aAuthors.insert(SvtUserOptions().GetFullName());

        for (int i = 0; i < rVersionBox.n_children(); ++i)
        {
            aAuthors.insert(weld::fromId<SfxVersionInfo*>(rVersionBox.get_id(i))->aAuthor);
        }

        int nMaxAuthorWidth = nRest/4;
        for (auto const& author : aAuthors)
        {
            nMaxAuthorWidth = std::max<int>(nMaxAuthorWidth, rVersionBox.get_pixel_size(author).Width());
            if (nMaxAuthorWidth > nRest/2)
            {
                nMaxAuthorWidth = nRest/2;
                break;
            }
        }

        rVersionBox.set_column_fixed_widths({ nMax, nMaxAuthorWidth });
    }
}

SfxVersionDialog::SfxVersionDialog(weld::Window* pParent, SfxViewFrame* pVwFrame, bool bIsSaveVersionOnClose)
    : SfxDialogController(pParent, u"sfx/ui/versionsofdialog.ui"_ustr, u"VersionsOfDialog"_ustr)
    , m_pViewFrame(pVwFrame)
    , m_bIsSaveVersionOnClose(bIsSaveVersionOnClose)
    , m_xSaveButton(m_xBuilder->weld_button(u"save"_ustr))
    , m_xSaveCheckBox(m_xBuilder->weld_check_button(u"always"_ustr))
    , m_xOpenButton(m_xBuilder->weld_button(u"open"_ustr))
    , m_xViewButton(m_xBuilder->weld_button(u"show"_ustr))
    , m_xDeleteButton(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xCompareButton(m_xBuilder->weld_button(u"compare"_ustr))
    , m_xCmisButton(m_xBuilder->weld_button(u"cmis"_ustr))
    , m_xVersionBox(m_xBuilder->weld_tree_view(u"versions"_ustr))
{
    m_xVersionBox->set_size_request(m_xVersionBox->get_approximate_digit_width() * 90,
                                    m_xVersionBox->get_height_rows(15));
    setColSizes(*m_xVersionBox);

    Link<weld::Button&,void> aClickLink = LINK( this, SfxVersionDialog, ButtonHdl_Impl );
    m_xViewButton->connect_clicked( aClickLink );
    m_xSaveButton->connect_clicked( aClickLink );
    m_xDeleteButton->connect_clicked( aClickLink );
    m_xCompareButton->connect_clicked( aClickLink );
    m_xOpenButton->connect_clicked( aClickLink );
    m_xSaveCheckBox->connect_toggled(LINK(this, SfxVersionDialog, ToggleHdl_Impl));
    m_xCmisButton->connect_clicked( aClickLink );

    m_xVersionBox->connect_changed( LINK( this, SfxVersionDialog, SelectHdl_Impl ) );
    m_xVersionBox->connect_row_activated( LINK( this, SfxVersionDialog, DClickHdl_Impl ) );

    m_xVersionBox->grab_focus();

    // set dialog title (filename or docinfo title)
    OUString sText = m_xDialog->get_title() +
                    " " + m_pViewFrame->GetObjectShell()->GetTitle();
    m_xDialog->set_title(sText);

    Init_Impl();
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
    SfxObjectShell *pObjShell = m_pViewFrame->GetObjectShell();
    SfxMedium* pMedium = pObjShell->GetMedium();
    uno::Sequence < util::RevisionTag > aVersions = pMedium->GetVersionList( true );
    m_pTable.reset(new SfxVersionTableDtor( aVersions ));
    m_xVersionBox->freeze();
    for (size_t n = 0; n < m_pTable->size(); ++n)
    {
        SfxVersionInfo *pInfo = m_pTable->at( n );
        OUString aEntry = formatDateTime(pInfo->aCreationDate, Application::GetSettings().GetLocaleDataWrapper(), false);
        m_xVersionBox->append(weld::toId(pInfo), aEntry);
        auto nLastRow = m_xVersionBox->n_children() - 1;
        m_xVersionBox->set_text(nLastRow, pInfo->aAuthor, 1);
        m_xVersionBox->set_text(nLastRow, ConvertWhiteSpaces_Impl(pInfo->aComment), 2);
    }
    m_xVersionBox->thaw();

    if (auto nCount = m_pTable->size())
        m_xVersionBox->select(nCount - 1);

    m_xSaveCheckBox->set_active(m_bIsSaveVersionOnClose);

    bool bEnable = !pObjShell->IsReadOnly();
    m_xSaveButton->set_sensitive( bEnable );
    m_xSaveCheckBox->set_sensitive( bEnable );

    m_xOpenButton->set_sensitive(false);
    m_xViewButton->set_sensitive(false);
    m_xDeleteButton->set_sensitive(false);
    m_xCompareButton->set_sensitive(false);

    if ( !officecfg::Office::Common::Misc::ExperimentalMode::get() )
        m_xCmisButton->hide( );
    uno::Reference<document::XCmisDocument> xCmisDoc(pObjShell->GetModel(), uno::UNO_QUERY);
    if (xCmisDoc && xCmisDoc->isVersionable())
        m_xCmisButton->set_sensitive(true);
    else
        m_xCmisButton->set_sensitive(false);

    SelectHdl_Impl(*m_xVersionBox);
}

SfxVersionDialog::~SfxVersionDialog()
{
}

void SfxVersionDialog::Open_Impl()
{
    SfxObjectShell *pObjShell = m_pViewFrame->GetObjectShell();

    auto nPos = m_xVersionBox->get_selected_index();
    SfxInt16Item aItem( SID_VERSION, nPos + 1);
    SfxStringItem aTarget( SID_TARGETNAME, u"_blank"_ustr );
    SfxStringItem aReferer( SID_REFERER, u"private:user"_ustr );
    SfxStringItem aFile( SID_FILE_NAME, pObjShell->GetMedium()->GetName() );

    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( GetEncryptionData_Impl( &pObjShell->GetMedium()->GetItemSet(), aEncryptionData ) )
    {
        // there is a password, it should be used during the opening
        SfxUnoAnyItem aEncryptionDataItem( SID_ENCRYPTIONDATA, uno::Any( aEncryptionData ) );
        m_pViewFrame->GetDispatcher()->ExecuteList(
            SID_OPENDOC, SfxCallMode::ASYNCHRON,
            { &aFile, &aItem, &aTarget, &aReferer, &aEncryptionDataItem });
    }
    else
    {
        m_pViewFrame->GetDispatcher()->ExecuteList(
            SID_OPENDOC, SfxCallMode::ASYNCHRON,
            { &aFile, &aItem, &aTarget, &aReferer });
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SfxVersionDialog, DClickHdl_Impl, weld::TreeView&, bool)
{
    Open_Impl();
    return true;
}

IMPL_LINK_NOARG(SfxVersionDialog, SelectHdl_Impl, weld::TreeView&, void)
{
    bool bEnable = m_xVersionBox->get_selected_index() != -1;
    SfxObjectShell* pObjShell = m_pViewFrame->GetObjectShell();
    m_xDeleteButton->set_sensitive(bEnable && !pObjShell->IsReadOnly());
    m_xOpenButton->set_sensitive(bEnable);
    m_xViewButton->set_sensitive(bEnable);

    SfxPoolItemHolder aResult;
    m_pViewFrame->GetDispatcher()->QueryState(SID_DOCUMENT_MERGE, aResult);
    SfxItemState eState = m_pViewFrame->GetDispatcher()->QueryState(SID_DOCUMENT_COMPARE, aResult);
    m_xCompareButton->set_sensitive(bEnable && eState >= SfxItemState::DEFAULT);
}

IMPL_LINK(SfxVersionDialog, ButtonHdl_Impl, weld::Button&, rButton, void)
{
    SfxObjectShell *pObjShell = m_pViewFrame->GetObjectShell();

    int nEntry = m_xVersionBox->get_selected_index();

    if (&rButton == m_xSaveButton.get())
    {
        SfxVersionInfo aInfo;
        aInfo.aAuthor = SvtUserOptions().GetFullName();
        SfxViewVersionDialog_Impl aDlg(m_xDialog.get(), aInfo, true);
        short nRet = aDlg.run();
        if (nRet == RET_OK)
        {
            SfxStringItem aComment( SID_DOCINFO_COMMENTS, aInfo.aComment );
            pObjShell->SetModified();
            const SfxPoolItem* aItems[2];
            aItems[0] = &aComment;
            aItems[1] = nullptr;
            m_pViewFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, aItems );
            m_xVersionBox->freeze();
            m_xVersionBox->clear();
            m_xVersionBox->thaw();
            Init_Impl();
        }
    }
    else if (&rButton == m_xDeleteButton.get() && nEntry != -1)
    {
        SfxVersionInfo* pInfo = weld::fromId<SfxVersionInfo*>(m_xVersionBox->get_id(nEntry));
        pObjShell->GetMedium()->RemoveVersion_Impl(pInfo->aName);
        pObjShell->SetModified();
        m_xVersionBox->freeze();
        m_xVersionBox->clear();
        m_xVersionBox->thaw();
        Init_Impl();
    }
    else if (&rButton == m_xOpenButton.get() && nEntry != -1)
    {
        Open_Impl();
    }
    else if (&rButton == m_xViewButton.get() && nEntry != -1)
    {
        SfxVersionInfo* pInfo = weld::fromId<SfxVersionInfo*>(m_xVersionBox->get_id(nEntry));
        SfxViewVersionDialog_Impl aDlg(m_xDialog.get(), *pInfo, false);
        aDlg.run();
    }
    else if (&rButton == m_xCompareButton.get() && nEntry != -1)
    {
        SfxAllItemSet aSet( pObjShell->GetPool() );
        aSet.Put(SfxInt16Item(SID_VERSION, nEntry + 1));
        aSet.Put(SfxStringItem(SID_FILE_NAME, pObjShell->GetMedium()->GetName()));

        SfxItemSet& rSet = pObjShell->GetMedium()->GetItemSet();
        const SfxStringItem* pFilterItem = rSet.GetItem(SID_FILTER_NAME, false);
        const SfxStringItem* pFilterOptItem = rSet.GetItem(SID_FILE_FILTEROPTIONS, false);
        if ( pFilterItem )
            aSet.Put( *pFilterItem );
        if ( pFilterOptItem )
            aSet.Put( *pFilterOptItem );

        m_pViewFrame->GetDispatcher()->Execute( SID_DOCUMENT_COMPARE, SfxCallMode::ASYNCHRON, aSet );
        m_xDialog->response(RET_CLOSE);
    }
    else if (&rButton == m_xCmisButton.get())
    {
        SfxCmisVersionsDialog aDlg(m_xDialog.get(), m_pViewFrame);
        aDlg.run();
    }
}

IMPL_LINK(SfxVersionDialog, ToggleHdl_Impl, weld::Toggleable&, rButton, void)
{
    if (&rButton == m_xSaveCheckBox.get())
    {
        m_bIsSaveVersionOnClose = m_xSaveCheckBox->get_active();
    }
}

SfxViewVersionDialog_Impl::SfxViewVersionDialog_Impl(weld::Window *pParent, SfxVersionInfo& rInfo, bool bEdit)
    : SfxDialogController(pParent, u"sfx/ui/versioncommentdialog.ui"_ustr, u"VersionCommentDialog"_ustr)
    , m_rInfo(rInfo)
    , m_xDateTimeText(m_xBuilder->weld_label(u"timestamp"_ustr))
    , m_xSavedByText(m_xBuilder->weld_label(u"author"_ustr))
    , m_xEdit(m_xBuilder->weld_text_view(u"textview"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xCancelButton(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xCloseButton(m_xBuilder->weld_button(u"close"_ustr))
{
    OUString sAuthor = rInfo.aAuthor.isEmpty() ? SfxResId(STR_NO_NAME_SET) : rInfo.aAuthor;

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    m_xDateTimeText->set_label(m_xDateTimeText->get_label() + formatDateTime(rInfo.aCreationDate, rLocaleWrapper, false));
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

SfxCmisVersionsDialog::SfxCmisVersionsDialog(weld::Window* pParent, SfxViewFrame* pVwFrame)
    : SfxDialogController(pParent, u"sfx/ui/versionscmis.ui"_ustr, u"VersionsCmisDialog"_ustr)
    , m_pViewFrame(pVwFrame)
    , m_xVersionBox(m_xBuilder->weld_tree_view(u"versions"_ustr))
{
    m_xVersionBox->set_size_request(m_xVersionBox->get_approximate_digit_width() * 90,
                                    m_xVersionBox->get_height_rows(15));
    setColSizes(*m_xVersionBox);

    m_xVersionBox->grab_focus();

    OUString sText = m_xDialog->get_title() +
                    " " + m_pViewFrame->GetObjectShell()->GetTitle();
    m_xDialog->set_title(sText);

    LoadVersions();
}

SfxCmisVersionsDialog::~SfxCmisVersionsDialog()
{
}

void SfxCmisVersionsDialog::LoadVersions()
{
    SfxObjectShell *pObjShell = m_pViewFrame->GetObjectShell();
    uno::Sequence < document::CmisVersion > aVersions = pObjShell->GetCmisVersions( );
    m_pTable.reset(new SfxVersionTableDtor( aVersions ));
    for (size_t n = 0; n < m_pTable->size(); ++n)
    {
        SfxVersionInfo *pInfo = m_pTable->at( n );
        OUString aEntry = formatDateTime(pInfo->aCreationDate, Application::GetSettings().GetLocaleDataWrapper(), false);
        m_xVersionBox->append(weld::toId(pInfo), aEntry);
        auto nLastRow = m_xVersionBox->n_children() - 1;
        m_xVersionBox->set_text(nLastRow, pInfo->aAuthor, 1);
        m_xVersionBox->set_text(nLastRow, ConvertWhiteSpaces_Impl(pInfo->aComment), 2);
    }

    if (auto nCount = m_pTable->size())
        m_xVersionBox->select(nCount - 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
