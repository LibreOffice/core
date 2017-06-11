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

#include <comphelper/string.hxx>
#include <osl/file.hxx>
#include <sfx2/new.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/idle.hxx>
#include <svtools/svmedit.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>

#include "sfx2/strings.hrc"
#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include "preview.hxx"
#include <sfx2/printer.hxx>
#include <vcl/waitobj.hxx>

void SfxPreviewBase_Impl::SetObjectShell(SfxObjectShell* pObj)
{
    std::shared_ptr<GDIMetaFile> xFile = pObj
        ? pObj->GetPreviewMetaFile()
        : std::shared_ptr<GDIMetaFile>();
    xMetaFile = xFile;
    Invalidate();
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl(vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , xMetaFile()
{
}

void SfxPreviewBase_Impl::Resize()
{
    Invalidate();
}

Size SfxPreviewBase_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(127, 129), MapUnit::MapAppFont);
}

void SfxPreviewWin_Impl::ImpPaint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/, GDIMetaFile* pFile)
{
    rRenderContext.SetLineColor();
    Color aLightGrayCol(COL_LIGHTGRAY);
    rRenderContext.SetFillColor(aLightGrayCol);
    rRenderContext.DrawRect(tools::Rectangle(Point(0,0), rRenderContext.GetOutputSize()));

    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1, 1);
    DBG_ASSERT(aTmpSize.Height() != 0 && aTmpSize.Width() != 0, "size of first page is 0, override GetFirstPageSize or set visible-area!");

#define FRAME 4

    long nWidth = rRenderContext.GetOutputSize().Width() - 2 * FRAME;
    long nHeight = rRenderContext.GetOutputSize().Height() - 2 * FRAME;
    if (nWidth <= 0 || nHeight <= 0)
        return;

    double dRatio = double(aTmpSize.Width()) / aTmpSize.Height();
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
        Color aBlackCol(COL_BLACK);
        Color aWhiteCol(COL_WHITE);
        rRenderContext.SetLineColor(aBlackCol);
        rRenderContext.SetFillColor(aWhiteCol);
        rRenderContext.DrawRect(tools::Rectangle(aPoint + Point(FRAME, FRAME), bPoint + Point(FRAME, FRAME)));
        pFile->WindStart();
        pFile->Play(&rRenderContext, aPoint + Point(FRAME, FRAME), aSize);
    }
}

void SfxPreviewWin_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    ImpPaint(rRenderContext, rRect, xMetaFile.get());
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL makeSfxPreviewWin(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<SfxPreviewWin_Impl>::Create(pParent, 0);
}

class SfxNewFileDialog_Impl
{
    VclPtr<ListBox>  m_pRegionLb;
    VclPtr<ListBox>  m_pTemplateLb;

    VclPtr<SfxPreviewWin_Impl> m_pPreviewWin;

    VclPtr<CheckBox> m_pTextStyleCB;
    VclPtr<CheckBox> m_pFrameStyleCB;
    VclPtr<CheckBox> m_pPageStyleCB;
    VclPtr<CheckBox> m_pNumStyleCB;
    VclPtr<CheckBox> m_pMergeStyleCB;
    VclPtr<PushButton> m_pLoadFilePB;

    VclPtr<VclExpander> m_pMoreBt;
    Idle aPrevIdle;
    OUString aNone;
    OUString sLoadTemplate;

    SfxNewFileDialogMode nFlags;
    SfxDocumentTemplates aTemplates;
    SfxObjectShellLock xDocShell;
    VclPtr<SfxNewFileDialog> pAntiImpl;

    DECL_LINK( Update, Timer *, void );

    DECL_LINK(RegionSelect, ListBox&, void);
    DECL_LINK(TemplateSelect, ListBox&, void);
    DECL_LINK(DoubleClick, ListBox&, void);
    DECL_LINK(Expand, VclExpander&, void);
    DECL_LINK(LoadFile, Button*, void);
    sal_uInt16  GetSelectedTemplatePos() const;

public:

    SfxNewFileDialog_Impl(SfxNewFileDialog* pAntiImplP, SfxNewFileDialogMode nFlags);
    ~SfxNewFileDialog_Impl();

    // Returns sal_False if '- No -' is set as a template
    // Template name can only be obtained if IsTemplate() is TRUE
    bool IsTemplate() const;
    OUString GetTemplateFileName() const;

    SfxTemplateFlags GetTemplateFlags() const;
    void             SetTemplateFlags(SfxTemplateFlags nSet);
};

IMPL_LINK_NOARG(SfxNewFileDialog_Impl, Update, Timer*, void)
{
    if (xDocShell.Is())
    {
        if (xDocShell->GetProgress())
            return;
        xDocShell.Clear();
    }

    const sal_uInt16 nEntry = GetSelectedTemplatePos();
    if (!nEntry)
    {
        m_pPreviewWin->Invalidate();
        m_pPreviewWin->SetObjectShell( nullptr);
        return;
    }

    if (m_pMoreBt->get_expanded() && (nFlags == SfxNewFileDialogMode::Preview))
    {

        OUString aFileName = aTemplates.GetPath(m_pRegionLb->GetSelectEntryPos(), nEntry - 1);
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
                    xDocShell = pTmp;
                    break;
                }
        }

        if (!xDocShell.Is())
        {
            SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,pAntiImpl);
            SfxApplication *pSfxApp = SfxGetpApp();
            SfxItemSet* pSet = new SfxAllItemSet(pSfxApp->GetPool());
            pSet->Put(SfxBoolItem(SID_TEMPLATE, true));
            pSet->Put(SfxBoolItem(SID_PREVIEW, true));
            ErrCode lErr = pSfxApp->LoadTemplate(xDocShell, aFileName, pSet);
            if (lErr)
                ErrorHandler::HandleError(lErr);
            if (!xDocShell.Is())
            {
                m_pPreviewWin->SetObjectShell(nullptr);
                return;
            }
        }

        m_pPreviewWin->SetObjectShell(xDocShell);
    }
}

IMPL_LINK( SfxNewFileDialog_Impl, RegionSelect, ListBox&, rBox, void )
{
    if (xDocShell.Is() && xDocShell->GetProgress())
        return;

    const sal_uInt16 nRegion = rBox.GetSelectEntryPos();
    const sal_uInt16 nCount = aTemplates.GetRegionCount()? aTemplates.GetCount(nRegion): 0;
    m_pTemplateLb->SetUpdateMode(false);
    m_pTemplateLb->Clear();
    OUString aSel = m_pRegionLb->GetSelectEntry();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase( SfxResId(STR_STANDARD) ) == 0 )
        m_pTemplateLb->InsertEntry(aNone);
    for (sal_uInt16 i = 0; i < nCount; ++i)
        m_pTemplateLb->InsertEntry(aTemplates.GetName(nRegion, i));
    m_pTemplateLb->SelectEntryPos(0);
    m_pTemplateLb->SetUpdateMode(true);
    m_pTemplateLb->Invalidate();
    m_pTemplateLb->Update();
    TemplateSelect(*m_pTemplateLb);
}

IMPL_LINK_NOARG(SfxNewFileDialog_Impl, Expand, VclExpander&, void)
{
    TemplateSelect(*m_pTemplateLb);
}

IMPL_LINK_NOARG(SfxNewFileDialog_Impl, TemplateSelect, ListBox&, void)
{
    // Still loading
    if ( xDocShell && xDocShell->GetProgress() )
        return;

    if (!m_pMoreBt->get_expanded())
        // Dialog is not opened
        return;

    aPrevIdle.Start();
}

IMPL_LINK_NOARG( SfxNewFileDialog_Impl, DoubleClick, ListBox&, void )
{
    // Still loading
    if ( !xDocShell.Is() || !xDocShell->GetProgress() )
        pAntiImpl->EndDialog(RET_OK);
}

IMPL_LINK_NOARG(SfxNewFileDialog_Impl, LoadFile, Button*, void)
{
    pAntiImpl->EndDialog(RET_TEMPLATE_LOAD);
}

sal_uInt16  SfxNewFileDialog_Impl::GetSelectedTemplatePos() const
{
    sal_uInt16 nEntry = m_pTemplateLb->GetSelectEntryPos();
    OUString aSel = m_pRegionLb->GetSelectEntry();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase(SfxResId(STR_STANDARD)) != 0 )
        nEntry++;
    if (!m_pTemplateLb->GetSelectEntryCount())
        nEntry = 0;
    return nEntry;
}

bool SfxNewFileDialog_Impl::IsTemplate() const
{
    return GetSelectedTemplatePos()!=0;

}

OUString SfxNewFileDialog_Impl::GetTemplateFileName() const
{
    if(!IsTemplate() || !aTemplates.GetRegionCount())
        return OUString();
    return aTemplates.GetPath(m_pRegionLb->GetSelectEntryPos(),
                              GetSelectedTemplatePos()-1);
}

SfxTemplateFlags  SfxNewFileDialog_Impl::GetTemplateFlags()const
{
    SfxTemplateFlags nRet = m_pTextStyleCB->IsChecked() ? SfxTemplateFlags::LOAD_TEXT_STYLES : SfxTemplateFlags::NONE;
    if(m_pFrameStyleCB->IsChecked())
        nRet |= SfxTemplateFlags::LOAD_FRAME_STYLES;
    if(m_pPageStyleCB->IsChecked())
        nRet |= SfxTemplateFlags::LOAD_PAGE_STYLES;
    if(m_pNumStyleCB->IsChecked())
        nRet |= SfxTemplateFlags::LOAD_NUM_STYLES;
    if(m_pMergeStyleCB->IsChecked())
        nRet |= SfxTemplateFlags::MERGE_STYLES;
    return nRet;
}

void    SfxNewFileDialog_Impl::SetTemplateFlags(SfxTemplateFlags nSet)
{
    m_pTextStyleCB->Check(  bool(nSet & SfxTemplateFlags::LOAD_TEXT_STYLES ));
    m_pFrameStyleCB->Check( bool(nSet & SfxTemplateFlags::LOAD_FRAME_STYLES));
    m_pPageStyleCB->Check(  bool(nSet & SfxTemplateFlags::LOAD_PAGE_STYLES ));
    m_pNumStyleCB->Check(   bool(nSet & SfxTemplateFlags::LOAD_NUM_STYLES  ));
    m_pMergeStyleCB->Check( bool(nSet & SfxTemplateFlags::MERGE_STYLES     ));
}


SfxNewFileDialog_Impl::SfxNewFileDialog_Impl(
    SfxNewFileDialog* pAntiImplP, SfxNewFileDialogMode nFl)
    : aNone(SfxResId(STR_NONE))
    , nFlags(nFl)
    , pAntiImpl(pAntiImplP)
{
    pAntiImplP->get(m_pRegionLb, "categories");
    pAntiImplP->get(m_pTemplateLb, "templates");

    Size aSize(m_pRegionLb->LogicToPixel(Size(127, 72), MapUnit::MapAppFont));
    m_pRegionLb->set_width_request(aSize.Width());
    m_pRegionLb->set_height_request(aSize.Height());
    m_pTemplateLb->set_width_request(aSize.Width());
    m_pTemplateLb->set_height_request(aSize.Height());

    pAntiImplP->get(m_pTextStyleCB, "text");
    pAntiImplP->get(m_pFrameStyleCB, "frame");
    pAntiImplP->get(m_pPageStyleCB, "pages");
    pAntiImplP->get(m_pNumStyleCB, "numbering");
    pAntiImplP->get(m_pMergeStyleCB, "overwrite");
    pAntiImplP->get(m_pMoreBt, "expander");
    pAntiImplP->get(m_pPreviewWin, "image");
    pAntiImplP->get(m_pLoadFilePB, "fromfile");
    sLoadTemplate = pAntiImplP->get<FixedText>("alttitle")->GetText();

    if (nFlags == SfxNewFileDialogMode::NONE)
        m_pMoreBt->Hide();
    else if(SfxNewFileDialogMode::LoadTemplate == nFlags)
    {
        m_pLoadFilePB->SetClickHdl(LINK(this, SfxNewFileDialog_Impl, LoadFile));
        m_pLoadFilePB->Show();
        m_pTextStyleCB->Show();
        m_pFrameStyleCB->Show();
        m_pPageStyleCB->Show();
        m_pNumStyleCB->Show();
        m_pMergeStyleCB->Show();
        m_pMoreBt->Hide();
        m_pTextStyleCB->Check();
        pAntiImplP->SetText(sLoadTemplate);
    }
    else
    {
        m_pMoreBt->SetExpandedHdl(LINK(this, SfxNewFileDialog_Impl, Expand));
        m_pPreviewWin->Show();
    }

    OUString &rExtra = pAntiImplP->GetExtraData();
    bool bExpand = !rExtra.isEmpty() && rExtra[0] == 'Y';
    m_pMoreBt->set_expanded(bExpand && (nFlags != SfxNewFileDialogMode::NONE));

    m_pTemplateLb->SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, TemplateSelect));
    m_pTemplateLb->SetDoubleClickHdl(LINK(this, SfxNewFileDialog_Impl, DoubleClick));

    // update the template configuration if necessary
    {
        WaitObject aWaitCursor( pAntiImplP->GetParent() );
        aTemplates.Update();
    }
    // fill the list boxes
    const sal_uInt16 nCount = aTemplates.GetRegionCount();
    if (nCount)
    {
        for(sal_uInt16 i = 0; i < nCount; ++i)
            m_pRegionLb->InsertEntry(aTemplates.GetFullRegionName(i));
        m_pRegionLb->SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, RegionSelect));
    }

    aPrevIdle.SetPriority( TaskPriority::LOWEST );
    aPrevIdle.SetInvokeHandler( LINK( this, SfxNewFileDialog_Impl, Update));

    m_pRegionLb->SelectEntryPos(0);
    RegionSelect(*m_pRegionLb);
}

SfxNewFileDialog_Impl::~SfxNewFileDialog_Impl()
{
    OUString &rExtra = pAntiImpl->GetExtraData();
    rExtra = m_pMoreBt->get_expanded() ? OUString("Y") : OUString("N");
}

SfxNewFileDialog::SfxNewFileDialog(vcl::Window *pParent, SfxNewFileDialogMode nFlags)
    : SfxModalDialog(pParent, "LoadTemplateDialog",
        "sfx/ui/loadtemplatedialog.ui"),
      pImpl( new SfxNewFileDialog_Impl(this, nFlags) )
{
}

SfxNewFileDialog::~SfxNewFileDialog()
{
    disposeOnce();
}

void SfxNewFileDialog::dispose()
{
    pImpl.reset();
    SfxModalDialog::dispose();
}

bool SfxNewFileDialog::IsTemplate() const
{
    return pImpl->IsTemplate();
}

OUString SfxNewFileDialog::GetTemplateFileName() const
{
    return pImpl->GetTemplateFileName();
}

SfxTemplateFlags SfxNewFileDialog::GetTemplateFlags()const
{
    return pImpl->GetTemplateFlags();
}

void SfxNewFileDialog::SetTemplateFlags(SfxTemplateFlags nSet)
{
    pImpl->SetTemplateFlags(nSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
