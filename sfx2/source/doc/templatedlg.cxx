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
#include <sfx2/thumbnailviewitem.hxx>
#include <vcl/toolbox.hxx>

#include "doc.hrc"
#include "templatedlg.hrc"

#define MAX_COLUMN_COUNT 4
#define MAX_LINE_COUNT 2

#define PADDING_TOOLBAR_VIEW    15

SfxTemplateManagerDlg::SfxTemplateManagerDlg (Window *parent)
    : ModalDialog(parent, SfxResId(DLG_TEMPLATE_MANAGER)),
      aButtonAll(this,SfxResId(BTN_SELECT_ALL)),
      aButtonDocs(this,SfxResId(BTN_SELECT_DOCS)),
      aButtonPresents(this,SfxResId(BTN_SELECT_PRESENTATIONS)),
      aButtonSheets(this,SfxResId(BTN_SELECT_SHEETS)),
      aButtonDraws(this,SfxResId(BTN_SELECT_DRAWS)),
      maButtonSelMode(this,SfxResId(BTN_SELECTION_MODE)),
      mpViewBar( new ToolBox(this, SfxResId(TBX_ACTION_VIEW))),
      mpActionBar( new ToolBox(this, SfxResId(TBX_ACTION_ACTION))),
      mpTemplateBar( new ToolBox(this, SfxResId(TBX_ACTION_TEMPLATES))),
      maView(new TemplateFolderView(this,SfxResId(TEMPLATE_VIEW))),
      mnSelectionCount(0)
{
    maButtonSelMode.SetStyle(maButtonSelMode.GetStyle() | WB_TOGGLE);

    // Calculate toolboxs size and positions
    Size aWinSize = GetOutputSize();
    Size aViewSize = mpViewBar->CalcMinimumWindowSizePixel();
    Size aActionSize = mpActionBar->CalcMinimumWindowSizePixel();
    Size aTemplateSize = mpTemplateBar->CalcMinimumWindowSizePixel();

    aActionSize.setWidth(2*aActionSize.getWidth());
    aViewSize.setWidth(aWinSize.getWidth()-aActionSize.getWidth()-mpViewBar->GetPosPixel().X());
    aTemplateSize.setWidth(aWinSize.getWidth());

    Point aActionPos = mpActionBar->GetPosPixel();
    aActionPos.setX(aWinSize.getWidth() - aActionSize.getWidth());

    mpViewBar->SetSizePixel(aViewSize);
    mpActionBar->SetPosSizePixel(aActionPos,aActionSize);
    mpTemplateBar->SetSizePixel(aTemplateSize);

    // Set toolbox styles
    mpViewBar->SetButtonType(BUTTON_SYMBOLTEXT);
    mpTemplateBar->SetButtonType(BUTTON_SYMBOLTEXT);

    // Set toolbox handlers
    mpViewBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXViewHdl));
    mpActionBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXActionHdl));
    mpTemplateBar->SetClickHdl(LINK(this,SfxTemplateManagerDlg,TBXTemplateHdl));

    // Set view position below toolbox
    Point aViewPos = maView->GetPosPixel();
    aViewPos.setY(aActionPos.Y() + aActionSize.getHeight() + PADDING_TOOLBAR_VIEW);
    maView->SetPosPixel(aViewPos);

    maView->SetStyle(WB_TABSTOP | WB_VSCROLL);
    maView->SetColCount(MAX_COLUMN_COUNT);
    maView->SetLineCount(MAX_LINE_COUNT);

    maView->setItemStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));
    maView->setTemplateStateHdl(LINK(this,SfxTemplateManagerDlg,TVTemplateStateHdl));

    aButtonAll.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewAllHdl));
    aButtonDocs.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDocsHdl));
    aButtonPresents.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewPresentsHdl));
    aButtonSheets.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewSheetsHdl));
    aButtonDraws.SetClickHdl(LINK(this,SfxTemplateManagerDlg,ViewDrawsHdl));
    maButtonSelMode.SetClickHdl(LINK(this,SfxTemplateManagerDlg,OnClickSelectionMode));

    mpViewBar->Show();
    mpActionBar->Show();

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

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXViewHdl)
{
    switch(mpViewBar->GetCurItemId())
    {
    case TBI_TEMPLATE_CREATE:
        OnTemplateCreate();
        break;
    case TBI_TEMPLATE_IMPORT:
        OnTemplateImport();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXActionHdl)
{
    switch(mpActionBar->GetCurItemId())
    {
    case TBI_TEMPLATE_SEARCH:
        OnTemplateSearch();
        break;
    case TBI_TEMPLATE_ACTION:
        OnTemplateAction();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK_NOARG(SfxTemplateManagerDlg,TBXTemplateHdl)
{
    switch(mpTemplateBar->GetCurItemId())
    {
    case TBI_TEMPLATE_EDIT:
        OnTemplateEdit();
        break;
    case TBI_TEMPLATE_PROPERTIES:
        OnTemplateProperties();
        break;
    case TBI_TEMPLATE_MOVE:
        OnTemplateMove();
        break;
    case TBI_TEMPLATE_DELETE:
        OnTemplateDelete();
        break;
    default:
        break;
    }

    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TVFolderStateHdl, const ThumbnailViewItem*, pItem)
{
    return 0;
}

IMPL_LINK(SfxTemplateManagerDlg, TVTemplateStateHdl, const ThumbnailViewItem*, pItem)
{
    if (pItem->isSelected())
    {
        if (maSelTemplates.empty())
        {
            mpViewBar->Show(false);
            mpActionBar->Show(false);
            mpTemplateBar->Show();
        }

        maSelTemplates.insert(pItem);
    }
    else
    {
        if (maSelTemplates.find(pItem) != maSelTemplates.end())
        {
            maSelTemplates.erase(pItem);

            if (maSelTemplates.empty())
            {
                mpTemplateBar->Show(false);
                mpViewBar->Show();
                mpActionBar->Show();
            }
        }
    }

    return 0;
}

void SfxTemplateManagerDlg::OnTemplateCreate ()
{
}

void SfxTemplateManagerDlg::OnTemplateImport ()
{
}

void SfxTemplateManagerDlg::OnTemplateSearch ()
{
}

void SfxTemplateManagerDlg::OnTemplateAction ()
{
}

void SfxTemplateManagerDlg::OnTemplateEdit ()
{
}

void SfxTemplateManagerDlg::OnTemplateProperties ()
{
}

void SfxTemplateManagerDlg::OnTemplateMove ()
{
}

void SfxTemplateManagerDlg::OnTemplateDelete ()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
