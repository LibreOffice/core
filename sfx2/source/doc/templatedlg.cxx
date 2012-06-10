/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "templatedlg.hxx"

#include <sfx2/sfxresid.hxx>
#include <sfx2/thumbnailview.hxx>

#include "doc.hrc"
#include "templatedlg.hrc"

#define MAX_COLUMN_COUNT 4
#define MAX_LINE_COUNT 2

SfxTemplateManagerDlg::SfxTemplateManagerDlg (Window *parent)
    : ModalDialog(parent, SfxResId(DLG_TEMPLATE_MANAGER)),
      aButtonAll(this,SfxResId(BTN_SELECT_ALL)),
      aButtonDocs(this,SfxResId(BTN_SELECT_DOCS)),
      aButtonPresents(this,SfxResId(BTN_SELECT_PRESENTATIONS)),
      aButtonSheets(this,SfxResId(BTN_SELECT_SHEETS)),
      aButtonDraws(this,SfxResId(BTN_SELECT_DRAWS)),
      maView(new ThumbnailView(this,SfxResId(TEMPLATE_VIEW)))
{
    maView->SetStyle(WB_RADIOSEL | WB_TABSTOP);
    maView->SetColCount(MAX_COLUMN_COUNT);
    maView->SetLineCount(MAX_LINE_COUNT);

    aButtonAll.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewAllHdl));
    aButtonDocs.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDocsHdl));

    maView->Populate();
    maView->Show();

    FreeResource();
}

SfxTemplateManagerDlg::~SfxTemplateManagerDlg ()
{
    delete maView;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewAllHdl)
{
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDocsHdl)
{
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewPresentsHdl)
{
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDrawsHdl)
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
