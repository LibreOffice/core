/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportDialog.hxx"

#include <libepubgen/libepubgen.h>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <comphelper/sequenceashashmap.hxx>
#include <sfx2/opengrf.hxx>
#include <sax/tools/converter.hxx>

#include "EPUBExportFilter.hxx"

using namespace com::sun::star;

namespace
{
/// Converts version value to a listbox entry position.
sal_Int32 VersionToPosition(sal_Int32 nVersion)
{
    sal_Int32 nPosition = 0;

    switch (nVersion)
    {
    case 30:
        nPosition = 0;
        break;
    case 20:
        nPosition = 1;
        break;
    default:
        assert(false);
        break;
    }

    return nPosition;
}

/// Converts listbox entry position to a version value.
sal_Int32 PositionToVersion(sal_Int32 nPosition)
{
    sal_Int32 nVersion = 0;

    switch (nPosition)
    {
    case 0:
        nVersion = 30;
        break;
    case 1:
        nVersion = 20;
        break;
    default:
        assert(false);
        break;
    }

    return nVersion;
}
}

namespace writerperfect
{

EPUBExportDialog::EPUBExportDialog(vcl::Window *pParent, comphelper::SequenceAsHashMap &rFilterData, uno::Reference<uno::XComponentContext> xContext, const css::uno::Reference<css::lang::XComponent> &xDocument)
    : ModalDialog(pParent, "EpubDialog", "writerperfect/ui/exportepub.ui"),
      mxContext(std::move(xContext)),
      mrFilterData(rFilterData),
      mxSourceDocument(xDocument)
{
    get(m_pVersion, "versionlb");
    assert(PositionToVersion(m_pVersion->GetSelectedEntryPos()) == EPUBExportFilter::GetDefaultVersion());

    auto it = rFilterData.find("EPUBVersion");
    if (it != rFilterData.end())
    {
        sal_Int32 nVersion = 0;
        if (it->second >>= nVersion)
            m_pVersion->SelectEntryPos(VersionToPosition(nVersion));
    }
    m_pVersion->SetSelectHdl(LINK(this, EPUBExportDialog, VersionSelectHdl));

    get(m_pSplit, "splitlb");
    it = rFilterData.find("EPUBSplitMethod");
    if (it != rFilterData.end())
    {
        sal_Int32 nSplitMethod = 0;
        if (it->second >>= nSplitMethod)
            // No conversion, 1:1 mapping between libepubgen::EPUBSplitMethod
            // and entry positions.
            m_pSplit->SelectEntryPos(nSplitMethod);
    }
    else
        m_pSplit->SelectEntryPos(EPUBExportFilter::GetDefaultSplitMethod());
    m_pSplit->SetSelectHdl(LINK(this, EPUBExportDialog, SplitSelectHdl));

    get(m_pLayout, "layoutlb");
    it = rFilterData.find("EPUBLayoutMethod");
    if (it != rFilterData.end())
    {
        sal_Int32 nLayoutMethod = 0;
        if (it->second >>= nLayoutMethod)
            // No conversion, 1:1 mapping between libepubgen::EPUBLayoutMethod
            // and entry positions.
            m_pLayout->SelectEntryPos(nLayoutMethod);
    }
    else
        m_pLayout->SelectEntryPos(EPUBExportFilter::GetDefaultLayoutMethod());
    m_pLayout->SetSelectHdl(LINK(this, EPUBExportDialog, LayoutSelectHdl));

    get(m_pCoverPath, "coverpath");

    get(m_pCoverButton, "coverbutton");
    m_pCoverButton->SetClickHdl(LINK(this, EPUBExportDialog, CoverClickHdl));

    get(m_pMediaDir, "mediadir");

    get(m_pMediaButton, "mediabutton");
    m_pMediaButton->SetClickHdl(LINK(this, EPUBExportDialog, MediaClickHdl));

    get(m_pIdentifier, "identifier");

    get(m_pTitle, "title");
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mxSourceDocument, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDP;
    if (xDPS.is())
        xDP = xDPS->getDocumentProperties();
    if (xDP.is())
        m_pTitle->SetText(xDP->getTitle());

    get(m_pInitialCreator, "author");
    if (xDP.is())
        m_pInitialCreator->SetText(xDP->getAuthor());

    get(m_pLanguage, "language");
    if (xDP.is())
    {
        OUString aLanguage(LanguageTag::convertToBcp47(xDP->getLanguage(), false));
        m_pLanguage->SetText(aLanguage);
    }

    get(m_pDate, "date");
    if (xDP.is())
    {
        OUStringBuffer aBuffer;
        util::DateTime aDate(xDP->getModificationDate());
        sax::Converter::convertDateTime(aBuffer, aDate, nullptr, true);
        m_pDate->SetText(aBuffer.makeStringAndClear());
    }

    get(m_pOKButton, "ok");
    m_pOKButton->SetClickHdl(LINK(this, EPUBExportDialog, OKClickHdl));
}

IMPL_LINK_NOARG(EPUBExportDialog, VersionSelectHdl, ListBox &, void)
{
    mrFilterData["EPUBVersion"] <<= PositionToVersion(m_pVersion->GetSelectedEntryPos());
}

IMPL_LINK_NOARG(EPUBExportDialog, SplitSelectHdl, ListBox &, void)
{
    // No conversion, 1:1 mapping between entry positions and
    // libepubgen::EPUBSplitMethod.
    mrFilterData["EPUBSplitMethod"] <<= m_pSplit->GetSelectedEntryPos();
}

IMPL_LINK_NOARG(EPUBExportDialog, LayoutSelectHdl, ListBox &, void)
{
    // No conversion, 1:1 mapping between entry positions and
    // libepubgen::EPUBLayoutMethod.
    mrFilterData["EPUBLayoutMethod"] <<= m_pLayout->GetSelectedEntryPos();
    m_pSplit->Enable(m_pLayout->GetSelectedEntryPos() != libepubgen::EPUB_LAYOUT_METHOD_FIXED);
}

IMPL_LINK_NOARG(EPUBExportDialog, CoverClickHdl, Button *, void)
{
    SvxOpenGraphicDialog aDlg("Import", this);
    aDlg.EnableLink(false);
    if (aDlg.Execute() == ERRCODE_NONE)
        m_pCoverPath->SetText(aDlg.GetPath());
}

IMPL_LINK_NOARG(EPUBExportDialog, MediaClickHdl, Button *, void)
{
    uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker = ui::dialogs::FolderPicker::create(mxContext);
    if (xFolderPicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        return;

    m_pMediaDir->SetText(xFolderPicker->getDirectory());
}

IMPL_LINK_NOARG(EPUBExportDialog, OKClickHdl, Button *, void)
{
    // General
    if (!m_pCoverPath->GetText().isEmpty())
        mrFilterData["RVNGCoverImage"] <<= m_pCoverPath->GetText();
    if (!m_pMediaDir->GetText().isEmpty())
        mrFilterData["RVNGMediaDir"] <<= m_pMediaDir->GetText();

    // Metadata
    if (!m_pIdentifier->GetText().isEmpty())
        mrFilterData["RVNGIdentifier"] <<= m_pIdentifier->GetText();
    if (!m_pTitle->GetText().isEmpty())
        mrFilterData["RVNGTitle"] <<= m_pTitle->GetText();
    if (!m_pInitialCreator->GetText().isEmpty())
        mrFilterData["RVNGInitialCreator"] <<= m_pInitialCreator->GetText();
    if (!m_pLanguage->GetText().isEmpty())
        mrFilterData["RVNGLanguage"] <<= m_pLanguage->GetText();
    if (!m_pDate->GetText().isEmpty())
        mrFilterData["RVNGDate"] <<= m_pDate->GetText();

    EndDialog(RET_OK);
}

EPUBExportDialog::~EPUBExportDialog()
{
    disposeOnce();
}

void EPUBExportDialog::dispose()
{
    m_pVersion.clear();
    m_pSplit.clear();
    m_pCoverPath.clear();
    m_pCoverButton.clear();
    m_pOKButton.clear();
    m_pIdentifier.clear();
    m_pTitle.clear();
    m_pInitialCreator.clear();
    m_pLanguage.clear();
    m_pDate.clear();
    m_pMediaDir.clear();
    m_pMediaButton.clear();
    m_pLayout.clear();
    ModalDialog::dispose();
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
