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
#include <i18nlangtag/languagetag.hxx>

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
EPUBExportDialog::EPUBExportDialog(weld::Window* pParent,
                                   comphelper::SequenceAsHashMap& rFilterData,
                                   uno::Reference<uno::XComponentContext> xContext,
                                   css::uno::Reference<css::lang::XComponent> xDocument)
    : GenericDialogController(pParent, "writerperfect/ui/exportepub.ui", "EpubDialog")
    , m_xContext(std::move(xContext))
    , m_rFilterData(rFilterData)
    , m_xSourceDocument(std::move(xDocument))
    , m_xVersion(m_xBuilder->weld_combo_box("versionlb"))
    , m_xSplit(m_xBuilder->weld_combo_box("splitlb"))
    , m_xLayout(m_xBuilder->weld_combo_box("layoutlb"))
    , m_xCoverPath(m_xBuilder->weld_entry("coverpath"))
    , m_xCoverButton(m_xBuilder->weld_button("coverbutton"))
    , m_xMediaDir(m_xBuilder->weld_entry("mediadir"))
    , m_xMediaButton(m_xBuilder->weld_button("mediabutton"))
    , m_xOKButton(m_xBuilder->weld_button("ok"))
    , m_xIdentifier(m_xBuilder->weld_entry("identifier"))
    , m_xTitle(m_xBuilder->weld_entry("title"))
    , m_xInitialCreator(m_xBuilder->weld_entry("author"))
    , m_xLanguage(m_xBuilder->weld_entry("language"))
    , m_xDate(m_xBuilder->weld_entry("date"))

{
    assert(PositionToVersion(m_xVersion->get_active()) == EPUBExportFilter::GetDefaultVersion());

    auto it = rFilterData.find("EPUBVersion");
    if (it != rFilterData.end())
    {
        sal_Int32 nVersion = 0;
        if (it->second >>= nVersion)
            m_xVersion->set_active(VersionToPosition(nVersion));
    }
    m_xVersion->connect_changed(LINK(this, EPUBExportDialog, VersionSelectHdl));

    it = rFilterData.find("EPUBSplitMethod");
    if (it != rFilterData.end())
    {
        sal_Int32 nSplitMethod = 0;
        if (it->second >>= nSplitMethod)
            // No conversion, 1:1 mapping between libepubgen::EPUBSplitMethod
            // and entry positions.
            m_xSplit->set_active(nSplitMethod);
    }
    else
        m_xSplit->set_active(EPUBExportFilter::GetDefaultSplitMethod());
    m_xSplit->connect_changed(LINK(this, EPUBExportDialog, SplitSelectHdl));

    it = rFilterData.find("EPUBLayoutMethod");
    if (it != rFilterData.end())
    {
        sal_Int32 nLayoutMethod = 0;
        if (it->second >>= nLayoutMethod)
            // No conversion, 1:1 mapping between libepubgen::EPUBLayoutMethod
            // and entry positions.
            m_xLayout->set_active(nLayoutMethod);
    }
    else
        m_xLayout->set_active(EPUBExportFilter::GetDefaultLayoutMethod());
    m_xLayout->connect_changed(LINK(this, EPUBExportDialog, LayoutSelectHdl));

    m_xCoverButton->connect_clicked(LINK(this, EPUBExportDialog, CoverClickHdl));

    m_xMediaButton->connect_clicked(LINK(this, EPUBExportDialog, MediaClickHdl));

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(m_xSourceDocument, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDP;
    if (xDPS.is())
        xDP = xDPS->getDocumentProperties();
    if (xDP.is())
    {
        m_xTitle->set_text(xDP->getTitle());
        m_xInitialCreator->set_text(xDP->getAuthor());

        OUString aLanguage(LanguageTag::convertToBcp47(xDP->getLanguage(), false));
        m_xLanguage->set_text(aLanguage);

        OUStringBuffer aBuffer;
        util::DateTime aDate(xDP->getModificationDate());
        sax::Converter::convertDateTime(aBuffer, aDate, nullptr, true);
        m_xDate->set_text(aBuffer.makeStringAndClear());
    }

    m_xOKButton->connect_clicked(LINK(this, EPUBExportDialog, OKClickHdl));
}

IMPL_LINK_NOARG(EPUBExportDialog, VersionSelectHdl, weld::ComboBox&, void)
{
    m_rFilterData["EPUBVersion"] <<= PositionToVersion(m_xVersion->get_active());
}

IMPL_LINK_NOARG(EPUBExportDialog, SplitSelectHdl, weld::ComboBox&, void)
{
    // No conversion, 1:1 mapping between entry positions and
    // libepubgen::EPUBSplitMethod.
    m_rFilterData["EPUBSplitMethod"] <<= static_cast<sal_Int32>(m_xSplit->get_active());
}

IMPL_LINK_NOARG(EPUBExportDialog, LayoutSelectHdl, weld::ComboBox&, void)
{
    // No conversion, 1:1 mapping between entry positions and
    // libepubgen::EPUBLayoutMethod.
    m_rFilterData["EPUBLayoutMethod"] <<= static_cast<sal_Int32>(m_xLayout->get_active());
    m_xSplit->set_sensitive(m_xLayout->get_active() != libepubgen::EPUB_LAYOUT_METHOD_FIXED);
}

IMPL_LINK_NOARG(EPUBExportDialog, CoverClickHdl, weld::Button&, void)
{
    SvxOpenGraphicDialog aDlg("Import", m_xDialog.get());
    aDlg.EnableLink(false);
    if (aDlg.Execute() == ERRCODE_NONE)
        m_xCoverPath->set_text(aDlg.GetPath());
}

IMPL_LINK_NOARG(EPUBExportDialog, MediaClickHdl, weld::Button&, void)
{
    uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker
        = ui::dialogs::FolderPicker::create(m_xContext);
    if (xFolderPicker->execute() != ui::dialogs::ExecutableDialogResults::OK)
        return;

    m_xMediaDir->set_text(xFolderPicker->getDirectory());
}

IMPL_LINK_NOARG(EPUBExportDialog, OKClickHdl, weld::Button&, void)
{
    // General
    if (!m_xCoverPath->get_text().isEmpty())
        m_rFilterData["RVNGCoverImage"] <<= m_xCoverPath->get_text();
    if (!m_xMediaDir->get_text().isEmpty())
        m_rFilterData["RVNGMediaDir"] <<= m_xMediaDir->get_text();

    // Metadata
    if (!m_xIdentifier->get_text().isEmpty())
        m_rFilterData["RVNGIdentifier"] <<= m_xIdentifier->get_text();
    if (!m_xTitle->get_text().isEmpty())
        m_rFilterData["RVNGTitle"] <<= m_xTitle->get_text();
    if (!m_xInitialCreator->get_text().isEmpty())
        m_rFilterData["RVNGInitialCreator"] <<= m_xInitialCreator->get_text();
    if (!m_xLanguage->get_text().isEmpty())
        m_rFilterData["RVNGLanguage"] <<= m_xLanguage->get_text();
    if (!m_xDate->get_text().isEmpty())
        m_rFilterData["RVNGDate"] <<= m_xDate->get_text();

    m_xDialog->response(RET_OK);
}

EPUBExportDialog::~EPUBExportDialog() = default;

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
