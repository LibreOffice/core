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
#include <sfx2/new.hxx>
#include <vcl/builder.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/svmedit.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>

#include "doc.hrc"
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include "preview.hxx"
#include <sfx2/printer.hxx>
#include <vcl/waitobj.hxx>

void SfxPreviewBase_Impl::SetObjectShell( SfxObjectShell* pObj )
{
    ::boost::shared_ptr<GDIMetaFile> pFile = pObj
        ? pObj->GetPreviewMetaFile()
        : ::boost::shared_ptr<GDIMetaFile>();
    pMetaFile = pFile;
    Invalidate();
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl(
    vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , pMetaFile()
{
}

void SfxPreviewBase_Impl::Resize()
{
    Invalidate();
}

Size SfxPreviewBase_Impl::GetOptimalSize() const
{
    return LogicToPixel(Size(127, 129), MAP_APPFONT);
}

void SfxPreviewWin_Impl::ImpPaint(
    const Rectangle&, GDIMetaFile* pFile, vcl::Window* pWindow )
{
    pWindow->SetLineColor();
    Color aLightGrayCol( COL_LIGHTGRAY );
    pWindow->SetFillColor( aLightGrayCol );
    pWindow->DrawRect( Rectangle( Point( 0,0 ), pWindow->GetOutputSize() ) );

    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
    DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
                "size of first page is 0, overload GetFirstPageSize or set vis-area!" );
#define FRAME 4
    long nWidth = pWindow->GetOutputSize().Width() - 2*FRAME;
    long nHeight = pWindow->GetOutputSize().Height() - 2*FRAME;
    if (nWidth <= 0 || nHeight <= 0)
        return;

    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
    double dRatioPreV=((double) nWidth ) / nHeight;
    Size aSize;
    Point aPoint;
    if (dRatio>dRatioPreV)
    {
        aSize=Size(nWidth, (sal_uInt16)(nWidth/dRatio));
        aPoint=Point( 0, (sal_uInt16)((nHeight-aSize.Height())/2));
    }
    else
    {
        aSize=Size((sal_uInt16)(nHeight*dRatio), nHeight);
        aPoint=Point((sal_uInt16)((nWidth-aSize.Width())/2),0);
    }
    Point bPoint=Point(nWidth,nHeight)-aPoint;

    if ( pFile )
    {
        Color aBlackCol( COL_BLACK );
        Color aWhiteCol( COL_WHITE );
        pWindow->SetLineColor( aBlackCol );
        pWindow->SetFillColor( aWhiteCol );
        pWindow->DrawRect( Rectangle( aPoint + Point( FRAME, FRAME ), bPoint + Point( FRAME, FRAME ) ) );
        pFile->WindStart();
        pFile->Play( pWindow, aPoint + Point( FRAME, FRAME ), aSize  );
    }
}

void SfxPreviewWin_Impl::Paint( const Rectangle& rRect )
{
    ImpPaint( rRect, pMetaFile.get(), this );
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSfxPreviewWin(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new SfxPreviewWin_Impl(pParent, 0);
}

class SfxNewFileDialog_Impl
{
    ListBox*  m_pRegionLb;
    ListBox*  m_pTemplateLb;

    SfxPreviewWin_Impl* m_pPreviewWin;

    CheckBox* m_pTextStyleCB;
    CheckBox* m_pFrameStyleCB;
    CheckBox* m_pPageStyleCB;
    CheckBox* m_pNumStyleCB;
    CheckBox* m_pMergeStyleCB;
    PushButton* m_pLoadFilePB;

    VclExpander* m_pMoreBt;
    Timer aPrevTimer;
    OUString aNone;
    OUString sLoadTemplate;

    sal_uInt16 nFlags;
    SfxDocumentTemplates aTemplates;
    SfxObjectShellLock xDocShell;
    SfxNewFileDialog* pAntiImpl;

    DECL_LINK( Update, void * );

    DECL_LINK( RegionSelect, ListBox * );
    DECL_LINK(TemplateSelect, void *);
    DECL_LINK( DoubleClick, ListBox * );
    DECL_LINK( Expand, void * );
    DECL_LINK(LoadFile, void *);
    sal_uInt16  GetSelectedTemplatePos() const;

public:

    SfxNewFileDialog_Impl( SfxNewFileDialog* pAntiImplP, sal_uInt16 nFlags );
    ~SfxNewFileDialog_Impl();

        // Returns sal_False if '- No -' is set as a template
        // Template name can only be obtained if IsTemplate() is TRUE
        // erfragt werden
    bool IsTemplate() const;
    OUString GetTemplateFileName() const;

    sal_uInt16  GetTemplateFlags()const;
    void    SetTemplateFlags(sal_uInt16 nSet);
};

IMPL_LINK_NOARG(SfxNewFileDialog_Impl, Update)
{
    if ( xDocShell.Is() )
    {
        if ( xDocShell->GetProgress() )
            return sal_False;
        xDocShell.Clear();
    }

    const sal_uInt16 nEntry = GetSelectedTemplatePos();
    if(!nEntry)
    {
        m_pPreviewWin->Invalidate();
        m_pPreviewWin->SetObjectShell( 0);
        return 0;
    }

    if ( m_pMoreBt->get_expanded() && (nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
    {

        OUString aFileName = aTemplates.GetPath( m_pRegionLb->GetSelectEntryPos(), nEntry-1);
        INetURLObject aTestObj( aFileName );
        if( aTestObj.GetProtocol() == INET_PROT_NOT_VALID )
        {
            // temp. fix until Templates are managed by UCB compatible service
            // does NOT work with locally cached components !
            OUString aTemp;
            utl::LocalFileHelper::ConvertPhysicalNameToURL( aFileName, aTemp );
            aFileName = aTemp;
        }

        INetURLObject aObj( aFileName );
        for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
              pTmp;
              pTmp = SfxObjectShell::GetNext(*pTmp) )
        {
            //! fsys bug op==
            if ( pTmp->GetMedium())
                // ??? HasName() MM
                if( INetURLObject( pTmp->GetMedium()->GetName() ) == aObj )
                {
                    xDocShell = pTmp;
                    break;
                }
        }

        if ( !xDocShell.Is() )
        {
            vcl::Window *pParent = Application::GetDefDialogParent();
            Application::SetDefDialogParent( pAntiImpl );
            SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,pAntiImpl);
            SfxApplication *pSfxApp = SfxGetpApp();
            sal_uIntPtr lErr;
            SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
            pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );
            pSet->Put( SfxBoolItem( SID_PREVIEW, true ) );
            lErr = pSfxApp->LoadTemplate( xDocShell, aFileName, true, pSet );
            if( lErr )
                ErrorHandler::HandleError(lErr);
            Application::SetDefDialogParent( pParent );
            if ( !xDocShell.Is() )
            {
                m_pPreviewWin->SetObjectShell( 0 );
                return sal_False;
            }
        }

        m_pPreviewWin->SetObjectShell( xDocShell );
    }
    return sal_True;
}



IMPL_LINK( SfxNewFileDialog_Impl, RegionSelect, ListBox *, pBox )
{
    if ( xDocShell.Is() && xDocShell->GetProgress() )
        return 0;

    const sal_uInt16 nRegion = pBox->GetSelectEntryPos();
    const sal_uInt16 nCount = aTemplates.GetRegionCount()? aTemplates.GetCount(nRegion): 0;
    m_pTemplateLb->SetUpdateMode(false);
    m_pTemplateLb->Clear();
    OUString aSel = m_pRegionLb->GetSelectEntry();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase( SfxResId(STR_STANDARD).toString() ) == 0 )
        m_pTemplateLb->InsertEntry(aNone);
    for (sal_uInt16 i = 0; i < nCount; ++i)
        m_pTemplateLb->InsertEntry(aTemplates.GetName(nRegion, i));
    m_pTemplateLb->SelectEntryPos(0);
    m_pTemplateLb->SetUpdateMode(true);
    m_pTemplateLb->Invalidate();
    m_pTemplateLb->Update();
    TemplateSelect(m_pTemplateLb);
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SfxNewFileDialog_Impl, Expand)
{
    TemplateSelect(m_pTemplateLb);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SfxNewFileDialog_Impl, Expand)


IMPL_LINK_NOARG(SfxNewFileDialog_Impl, TemplateSelect)
{
    // Still loading
    if ( xDocShell && xDocShell->GetProgress() )
        return 0;

    if (!m_pMoreBt->get_expanded())
        // Dialog is not opened
        return 0;

    aPrevTimer.Start();
    return 0;
}

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )
{
    (void)pListBox;
    // Still loadning
    if ( !xDocShell.Is() || !xDocShell->GetProgress() )
        pAntiImpl->EndDialog(RET_OK);
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )



IMPL_LINK_NOARG_INLINE_START(SfxNewFileDialog_Impl, LoadFile)
{
    pAntiImpl->EndDialog(RET_TEMPLATE_LOAD);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SfxNewFileDialog_Impl, LoadFile)


sal_uInt16  SfxNewFileDialog_Impl::GetSelectedTemplatePos() const
{
    sal_uInt16 nEntry = m_pTemplateLb->GetSelectEntryPos();
    OUString aSel = m_pRegionLb->GetSelectEntry();
    sal_Int32 nc = aSel.indexOf('(');
    if (nc != -1 && nc != 0)
        aSel = aSel.replaceAt(nc-1, 1, "");
    if ( aSel.compareToIgnoreAsciiCase(SfxResId(STR_STANDARD).toString()) != 0 )
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

sal_uInt16  SfxNewFileDialog_Impl::GetTemplateFlags()const
{
    sal_uInt16 nRet = m_pTextStyleCB->IsChecked() ? SFX_LOAD_TEXT_STYLES : 0;
    if(m_pFrameStyleCB->IsChecked())
        nRet |= SFX_LOAD_FRAME_STYLES;
    if(m_pPageStyleCB->IsChecked())
        nRet |= SFX_LOAD_PAGE_STYLES;
    if(m_pNumStyleCB->IsChecked())
        nRet |= SFX_LOAD_NUM_STYLES;
    if(m_pMergeStyleCB->IsChecked())
        nRet |= SFX_MERGE_STYLES;
    return nRet;
}

void    SfxNewFileDialog_Impl::SetTemplateFlags(sal_uInt16 nSet)
{
    m_pTextStyleCB->Check(  0 != (nSet&SFX_LOAD_TEXT_STYLES ));
    m_pFrameStyleCB->Check( 0 != (nSet&SFX_LOAD_FRAME_STYLES));
    m_pPageStyleCB->Check(  0 != (nSet&SFX_LOAD_PAGE_STYLES ));
    m_pNumStyleCB->Check(   0 != (nSet&SFX_LOAD_NUM_STYLES  ));
    m_pMergeStyleCB->Check( 0 != (nSet&SFX_MERGE_STYLES     ));
}



SfxNewFileDialog_Impl::SfxNewFileDialog_Impl(
    SfxNewFileDialog* pAntiImplP, sal_uInt16 nFl)
    : aNone(SfxResId(STR_NONE).toString())
    , nFlags(nFl)
    , pAntiImpl(pAntiImplP)
{
    pAntiImplP->get(m_pRegionLb, "categories");
    pAntiImplP->get(m_pTemplateLb, "templates");

    Size aSize(m_pRegionLb->LogicToPixel(Size(127, 72), MAP_APPFONT));
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

    if (!nFlags)
        m_pMoreBt->Hide();
    else if(SFXWB_LOAD_TEMPLATE == nFlags)
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
    m_pMoreBt->set_expanded(bExpand && nFlags);

    m_pTemplateLb->SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, TemplateSelect));
    m_pTemplateLb->SetDoubleClickHdl(LINK(this, SfxNewFileDialog_Impl, DoubleClick));

    // update the template configuration if necessary
    {
        WaitObject aWaitCursor( pAntiImplP->GetParent() );
        aTemplates.Update( true /* be smart */ );
    }
    // fill the list boxes
    const sal_uInt16 nCount = aTemplates.GetRegionCount();
    if (nCount)
    {
        for(sal_uInt16 i = 0; i < nCount; ++i)
            m_pRegionLb->InsertEntry(aTemplates.GetFullRegionName(i));
        m_pRegionLb->SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, RegionSelect));
    }

    aPrevTimer.SetTimeout( 500 );
    aPrevTimer.SetTimeoutHdl( LINK( this, SfxNewFileDialog_Impl, Update));

    m_pRegionLb->SelectEntryPos(0);
    RegionSelect(m_pRegionLb);
}

SfxNewFileDialog_Impl::~SfxNewFileDialog_Impl()
{
    OUString &rExtra = pAntiImpl->GetExtraData();
    rExtra = m_pMoreBt->get_expanded() ? OUString("Y") : OUString("N");
}

SfxNewFileDialog::SfxNewFileDialog(vcl::Window *pParent, sal_uInt16 nFlags)
    : SfxModalDialog(pParent, "LoadTemplateDialog",
        "sfx/ui/loadtemplatedialog.ui")
{
    pImpl = new SfxNewFileDialog_Impl(this, nFlags);
}

SfxNewFileDialog::~SfxNewFileDialog()
{
    delete pImpl;
}

bool SfxNewFileDialog::IsTemplate() const
{
    return pImpl->IsTemplate();
}

OUString SfxNewFileDialog::GetTemplateFileName() const
{
    return pImpl->GetTemplateFileName();
}

sal_uInt16 SfxNewFileDialog::GetTemplateFlags()const
{
    return pImpl->GetTemplateFlags();

}

void    SfxNewFileDialog::SetTemplateFlags(sal_uInt16 nSet)
{
    pImpl->SetTemplateFlags(nSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
