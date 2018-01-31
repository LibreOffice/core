/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBEXPORTDIALOG_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBEXPORTDIALOG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>

namespace comphelper
{
class SequenceAsHashMap;
}

namespace writerperfect
{

/// EPUB export options dialog.
class EPUBExportDialog : public ModalDialog
{
public:
    EPUBExportDialog(vcl::Window *pParent, comphelper::SequenceAsHashMap &rFilterData, css::uno::Reference<css::uno::XComponentContext> xContext, css::uno::Reference<css::lang::XComponent> xDocument);
    ~EPUBExportDialog() override;
    void dispose() override;

private:
    DECL_LINK(VersionSelectHdl, ListBox &, void);
    DECL_LINK(SplitSelectHdl, ListBox &, void);
    DECL_LINK(LayoutSelectHdl, ListBox &, void);
    DECL_LINK(CoverClickHdl, Button *, void);
    DECL_LINK(MediaClickHdl, Button *, void);
    DECL_LINK(OKClickHdl, Button *, void);

    css::uno::Reference<css::uno::XComponentContext> mxContext;
    comphelper::SequenceAsHashMap &mrFilterData;
    css::uno::Reference<css::lang::XComponent> mxSourceDocument;
    VclPtr<ListBox> m_pVersion;
    VclPtr<ListBox> m_pSplit;
    VclPtr<ListBox> m_pLayout;
    VclPtr<Edit> m_pCoverPath;
    VclPtr<PushButton> m_pCoverButton;
    VclPtr<Edit> m_pMediaDir;
    VclPtr<PushButton> m_pMediaButton;
    VclPtr<PushButton> m_pOKButton;
    VclPtr<Edit> m_pIdentifier;
    VclPtr<Edit> m_pTitle;
    VclPtr<Edit> m_pInitialCreator;
    VclPtr<Edit> m_pLanguage;
    VclPtr<Edit> m_pDate;
};

} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
