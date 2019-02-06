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

#include <osl/file.hxx>
#include <sfx2/new.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <vcl/idle.hxx>
#include <vcl/gdimtf.hxx>
#include <svtools/svmedit.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>

#include <sfx2/strings.hrc>
#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <preview.hxx>
#include <sfx2/printer.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/waitobj.hxx>

void SfxPreviewWin_Impl::SetObjectShell(SfxObjectShell const * pObj)
{
    std::shared_ptr<GDIMetaFile> xFile = pObj
        ? pObj->GetPreviewMetaFile()
        : std::shared_ptr<GDIMetaFile>();
    xMetaFile = xFile;
    Invalidate();
}

SfxPreviewWin_Impl::SfxPreviewWin_Impl()
{
}

void SfxPreviewWin_Impl::ImpPaint(vcl::RenderContext& rRenderContext, GDIMetaFile* pFile)
{
    rRenderContext.SetLineColor();
    rRenderContext.SetFillColor(COL_LIGHTGRAY);
    rRenderContext.DrawRect(tools::Rectangle(Point(0,0), rRenderContext.GetOutputSize()));

    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1, 1);
    DBG_ASSERT(aTmpSize.Height() != 0 && aTmpSize.Width() != 0, "size of first page is 0, override GetFirstPageSize or set visible-area!");

#define FRAME 4

    long nWidth = rRenderContext.GetOutputSize().Width() - 2 * FRAME;
    long nHeight = rRenderContext.GetOutputSize().Height() - 2 * FRAME;
    if (nWidth <= 0 || nHeight <= 0)
        return;

    double dRatio = aTmpSize.Height() ? (double(aTmpSize.Width()) / aTmpSize.Height()) : 1;
    double dRatioPreV = double(nWidth) / nHeight;
    Size aSize;
    Point aPoint;
    if (dRatio > dRatioPreV)
    {
        aSize = Size(nWidth, sal_uInt16(nWidth / dRatio));
        aPoint = Point(0, sal_uInt16((nHeight - aSize.Height()) / 2));
    }
    else
    {
        aSize = Size(sal_uInt16(nHeight * dRatio), nHeight);
        aPoint = Point(sal_uInt16((nWidth - aSize.Width()) / 2), 0);
    }
    Point bPoint = Point(nWidth, nHeight) - aPoint;

    if (pFile)
    {
        rRenderContext.SetLineColor(COL_BLACK);
        rRenderContext.SetFillColor(COL_WHITE);
        rRenderContext.DrawRect(tools::Rectangle(aPoint + Point(FRAME, FRAME), bPoint + Point(FRAME, FRAME)));
        pFile->WindStart();
        pFile->Play(&rRenderContext, aPoint + Point(FRAME, FRAME), aSize);
    }
}

void SfxPreviewWin_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImpPaint(rRenderContext, xMetaFile.get());
}

IMPL_LINK_NOARG(SfxNewFileDialog, Update, Timer*, void)
{
    if (m_xDocShell.Is())
    {
        if (m_xDocShell->GetProgress())
            return;
        m_xDocShell.Clear();
    }

    const sal_uInt16 nEntry = GetSelectedTemplatePos();
    if (!nEntry)
    {
        m_xPreviewController->Invalidate();
        m_xPreviewController->SetObjectShell(nullptr);
        return;
    }

    if (!m_xMoreBt->get_expanded() || (m_nFlags != SfxNewFileDialogMode::Preview))
        return;

    OUString aFileName = m_aTemplates.GetPath(m_xRegionLb->get_selected_index(), nEntry - 1);
    INetURLObject aTestObj(aFileName);
    if (aTestObj.GetProtocol() == INetProtocol::NotValid)
    {
        // temp. fix until Templates are managed by UCB compatible service
        // does NOT work with locally cached components !
        OUString aTemp;
        osl::FileBase::getFileURLFromSystemPath( aFileName, aTemp );
        aFileName = aTemp;
    }

    INetURLObject aObj(aFileName);
    for (SfxObjectShell* pTmp = SfxObjectShell::GetFirst(); pTmp; pTmp = SfxObjectShell::GetNext(*pTmp))
    {
        //! fsys bug op==
        if (pTmp->GetMedium())
            // ??? HasName() MM
            if (INetURLObject( pTmp->GetMedium()->GetName() ) == aObj)
            {
                m_xDocShell = pTmp;
                break;
            }
    }

    if (!m_xDocShell.Is())
    {
        SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE, m_xDialog.get());
        SfxApplication *pSfxApp = SfxGetpApp();
        std::unique_ptr<SfxItemSet> pSet(new SfxAllItemSet(pSfxApp->GetPool()));
        pSet->Put(SfxBoolItem(SID_TEMPLATE, true));
        pSet->Put(SfxBoolItem(SID_PREVIEW, true));
        ErrCode lErr = pSfxApp->LoadTemplate(m_xDocShell, aFileName, std::move(pSet));
        if (lErr)
            ErrorHandler::HandleError(lErr);
        if (!m_xDocShell.Is())
        {
            m_xPreviewController->SetObjectShell(nullptr);
            return;
        }
    }

    m_xPreviewController->SetObjectShell(m_xDocShell);
}

IMPL_LINK( SfxNewFileDialog, RegionSelect, weld::TreeView&, rBox, void )
{
    if (m_xDocShell.Is() && m_xDocShell->GetProgress())
        return;

    const sal_uInt16 nRegion = rBox.get_selected_index();
    const sal_uInt16 nCount = m_aTemplates.GetRegionCount() ? m_aTemplates.GetCount(nRegion): 0;
    m_xTemplateLb->freeze();
    m_xTemplateLb->clear();
    OUString aSel = m_xRegionLb->get_selected_text();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase( SfxResId(STR_STANDARD) ) == 0 )
        m_xTemplateLb->append_text(SfxResId(STR_NONE));
    for (sal_uInt16 i = 0; i < nCount; ++i)
        m_xTemplateLb->append_text(m_aTemplates.GetName(nRegion, i));
    m_xTemplateLb->thaw();
    if (nCount)
        m_xTemplateLb->select(0);
    TemplateSelect(*m_xTemplateLb);
}

IMPL_LINK_NOARG(SfxNewFileDialog, Expand, weld::Expander&, void)
{
    TemplateSelect(*m_xTemplateLb);
}

IMPL_LINK_NOARG(SfxNewFileDialog, TemplateSelect, weld::TreeView&, void)
{
    // Still loading
    if (m_xDocShell && m_xDocShell->GetProgress())
        return;

    if (!m_xMoreBt->get_expanded())
    {
        // Dialog is not opened
        return;
    }

    m_aPrevIdle.Start();
}

IMPL_LINK_NOARG( SfxNewFileDialog, DoubleClick, weld::TreeView&, void )
{
    // Still loading
    if (!m_xDocShell.Is() || !m_xDocShell->GetProgress())
        m_xDialog->response(RET_OK);
}

sal_uInt16  SfxNewFileDialog::GetSelectedTemplatePos() const
{
    int nEntry = m_xTemplateLb->get_selected_index();
    if (nEntry == -1)
        return 0;
    OUString aSel = m_xRegionLb->get_selected_text();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase(SfxResId(STR_STANDARD)) != 0 )
        nEntry++;
    return nEntry;
}

SfxNewFileDialog::SfxNewFileDialog(weld::Window *pParent, SfxNewFileDialogMode nFlags)
    : SfxDialogController(pParent, "sfx/ui/loadtemplatedialog.ui", "LoadTemplateDialog")
    , m_nFlags(nFlags)
    , m_xPreviewController(new SfxPreviewWin_Impl)
    , m_xRegionLb(m_xBuilder->weld_tree_view("categories"))
    , m_xTemplateLb(m_xBuilder->weld_tree_view("templates"))
    , m_xTextStyleCB(m_xBuilder->weld_check_button("text"))
    , m_xFrameStyleCB(m_xBuilder->weld_check_button("frame"))
    , m_xPageStyleCB(m_xBuilder->weld_check_button("pages"))
    , m_xNumStyleCB(m_xBuilder->weld_check_button("numbering"))
    , m_xMergeStyleCB(m_xBuilder->weld_check_button("overwrite"))
    , m_xLoadFilePB(m_xBuilder->weld_button("fromfile"))
    , m_xMoreBt(m_xBuilder->weld_expander("expander"))
    , m_xPreviewWin(new weld::CustomWeld(*m_xBuilder, "image", *m_xPreviewController))
    , m_xAltTitleFt(m_xBuilder->weld_label("alttitle"))
{
    const int nWidth = m_xRegionLb->get_approximate_digit_width() * 32;
    const int nHeight = m_xRegionLb->get_height_rows(8);
    m_xRegionLb->set_size_request(nWidth, nHeight);
    m_xTemplateLb->set_size_request(nWidth, nHeight);
    m_xPreviewWin->set_size_request(nWidth, nWidth);

    if (nFlags == SfxNewFileDialogMode::NONE)
        m_xMoreBt->hide();
    else if(SfxNewFileDialogMode::LoadTemplate == nFlags)
    {
        m_xLoadFilePB->show();
        m_xTextStyleCB->show();
        m_xFrameStyleCB->show();
        m_xPageStyleCB->show();
        m_xNumStyleCB->show();
        m_xMergeStyleCB->show();
        m_xMoreBt->hide();
        m_xTextStyleCB->set_active(true);
        m_xDialog->set_title(m_xAltTitleFt->get_label());
    }
    else
    {
        m_xMoreBt->connect_expanded(LINK(this, SfxNewFileDialog, Expand));
        m_xPreviewWin->show();
    }

    OUString sExtraData;
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    if (aDlgOpt.Exists())
    {
        css::uno::Any aUserItem = aDlgOpt.GetUserItem("UserItem");
        aUserItem >>= sExtraData;
    }

    bool bExpand = !sExtraData.isEmpty() && sExtraData[0] == 'Y';
    m_xMoreBt->set_expanded(bExpand && (nFlags != SfxNewFileDialogMode::NONE));

    m_xTemplateLb->connect_changed(LINK(this, SfxNewFileDialog, TemplateSelect));
    m_xTemplateLb->connect_row_activated(LINK(this, SfxNewFileDialog, DoubleClick));

    // update the template configuration if necessary
    {
        weld::WaitObject aWaitCursor(m_xDialog.get());
        m_aTemplates.Update();
    }
    // fill the list boxes
    const sal_uInt16 nCount = m_aTemplates.GetRegionCount();
    if (nCount)
    {
        for(sal_uInt16 i = 0; i < nCount; ++i)
            m_xRegionLb->append_text(m_aTemplates.GetFullRegionName(i));
        m_xRegionLb->connect_changed(LINK(this, SfxNewFileDialog, RegionSelect));
    }

    m_aPrevIdle.SetPriority( TaskPriority::LOWEST );
    m_aPrevIdle.SetInvokeHandler( LINK( this, SfxNewFileDialog, Update));

    m_xRegionLb->select(0);
    RegionSelect(*m_xRegionLb);
}

SfxNewFileDialog::~SfxNewFileDialog()
{
    SvtViewOptions aDlgOpt(EViewType::Dialog, OStringToOUString(m_xDialog->get_help_id(), RTL_TEXTENCODING_UTF8));
    aDlgOpt.SetUserItem("UserItem", css::uno::makeAny(m_xMoreBt->get_expanded() ? OUString("Y") : OUString("N")));
}

bool SfxNewFileDialog::IsTemplate() const
{
    return GetSelectedTemplatePos()!=0;
}

OUString SfxNewFileDialog::GetTemplateFileName() const
{
    if (!IsTemplate() || !m_aTemplates.GetRegionCount())
        return OUString();
    return m_aTemplates.GetPath(m_xRegionLb->get_selected_index(),
                              GetSelectedTemplatePos()-1);
}

SfxTemplateFlags SfxNewFileDialog::GetTemplateFlags()const
{
    SfxTemplateFlags nRet = m_xTextStyleCB->get_active() ? SfxTemplateFlags::LOAD_TEXT_STYLES : SfxTemplateFlags::NONE;
    if(m_xFrameStyleCB->get_active())
        nRet |= SfxTemplateFlags::LOAD_FRAME_STYLES;
    if(m_xPageStyleCB->get_active())
        nRet |= SfxTemplateFlags::LOAD_PAGE_STYLES;
    if(m_xNumStyleCB->get_active())
        nRet |= SfxTemplateFlags::LOAD_NUM_STYLES;
    if(m_xMergeStyleCB->get_active())
        nRet |= SfxTemplateFlags::MERGE_STYLES;
    return nRet;
}

void SfxNewFileDialog::SetTemplateFlags(SfxTemplateFlags nSet)
{
    m_xTextStyleCB->set_active(  bool(nSet & SfxTemplateFlags::LOAD_TEXT_STYLES ));
    m_xFrameStyleCB->set_active( bool(nSet & SfxTemplateFlags::LOAD_FRAME_STYLES));
    m_xPageStyleCB->set_active(  bool(nSet & SfxTemplateFlags::LOAD_PAGE_STYLES ));
    m_xNumStyleCB->set_active(   bool(nSet & SfxTemplateFlags::LOAD_NUM_STYLES  ));
    m_xMergeStyleCB->set_active( bool(nSet & SfxTemplateFlags::MERGE_STYLES     ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
