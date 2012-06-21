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
#include <sfx2/templatefolderview.hxx>

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
      maButtonSelMode(this,SfxResId(BTN_SELECTION_MODE)),
      maView(new TemplateFolderView(this,SfxResId(TEMPLATE_VIEW)))
{
    maButtonSelMode.SetStyle(maButtonSelMode.GetStyle() | WB_TOGGLE);

    maView->SetStyle(WB_TABSTOP | WB_VSCROLL);
    maView->SetColCount(MAX_COLUMN_COUNT);
    maView->SetLineCount(MAX_LINE_COUNT);

    aButtonAll.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewAllHdl));
    aButtonDocs.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDocsHdl));
    aButtonPresents.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewPresentsHdl));
    aButtonSheets.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewSheetsHdl));
    aButtonDraws.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDrawsHdl));
    maButtonSelMode.SetClickHdl(LINK(this,SfxTemplateManagerDlg,OnClickSelectionMode));

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
    maView->filterTemplatesByApp(FILTER_APP_NONE);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDocsHdl)
{
    maView->filterTemplatesByApp(FILTER_APP_WRITER);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewPresentsHdl)
{
    maView->filterTemplatesByApp(FILTER_APP_IMPRESS);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewSheetsHdl)
{
    maView->filterTemplatesByApp(FILTER_APP_CALC);
    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,ViewDrawsHdl)
{
    maView->filterTemplatesByApp(FILTER_APP_DRAW);
    return 0;
}

void SfxTemplateManagerDlg::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!maView->GetActiveClipRegion().IsInside(rMEvt.GetPosPixel()) && maView->isOverlayVisible())
        maView->showOverlay(false);
}

IMPL_LINK (SfxTemplateManagerDlg, OnClickSelectionMode, ImageButton*, pButton)
{
    maView->setSelectionMode(pButton->GetState() == STATE_CHECK);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
