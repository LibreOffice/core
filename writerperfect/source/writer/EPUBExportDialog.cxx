/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBExportDialog.hxx"

#include <sfx2/opengrf.hxx>

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

EPUBExportDialog::EPUBExportDialog(vcl::Window *pParent, comphelper::SequenceAsHashMap &rFilterData)
    : ModalDialog(pParent, "EpubDialog", "writerperfect/ui/exportepub.ui"),
      mrFilterData(rFilterData)
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
            m_pVersion->SelectEntryPos(nSplitMethod);
    }
    else
        m_pSplit->SelectEntryPos(EPUBExportFilter::GetDefaultSplitMethod());
    m_pSplit->SetSelectHdl(LINK(this, EPUBExportDialog, SplitSelectHdl));

    get(m_pCoverPath, "coverpath");

    get(m_pCoverButton, "coverbutton");
    m_pCoverButton->SetClickHdl(LINK(this, EPUBExportDialog, CoverClickHdl));

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

IMPL_LINK_NOARG(EPUBExportDialog, CoverClickHdl, Button *, void)
{
    SvxOpenGraphicDialog aDlg("Import", this);
    aDlg.EnableLink(false);
    if (aDlg.Execute() == ERRCODE_NONE)
        m_pCoverPath->SetText(aDlg.GetPath());
}

IMPL_LINK_NOARG(EPUBExportDialog, OKClickHdl, Button *, void)
{
    if (!m_pCoverPath->GetText().isEmpty())
        mrFilterData["RVNGCoverImage"] <<= m_pCoverPath->GetText();

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
    ModalDialog::dispose();
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
