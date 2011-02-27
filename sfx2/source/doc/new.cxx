/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <sfx2/new.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/morebtn.hxx>
#include <svtools/svmedit.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>

#include "new.hrc"
#include "doc.hrc"
#include <sfx2/sfx.hrc>
#include "helpid.hrc"
#include "sfxtypes.hxx"
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "fltfnc.hxx"
#include <sfx2/viewsh.hxx>
#include "viewfac.hxx"
#include "sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include "preview.hxx"
#include <sfx2/printer.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/jobset.hxx>
#include <svtools/accessibilityoptions.hxx>

// Draw modes
#define OUTPUT_DRAWMODE_COLOR       (DRAWMODE_DEFAULT)
#define OUTPUT_DRAWMODE_GRAYSCALE   (DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT)
#define OUTPUT_DRAWMODE_BLACKWHITE  (DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT)
#define OUTPUT_DRAWMODE_CONTRAST    (DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT)

//========================================================================

#define MORE_BTN(x) pMoreBt->x

//========================================================================

void SfxPreviewBase_Impl::SetObjectShell( SfxObjectShell* pObj )
{
    ::boost::shared_ptr<GDIMetaFile> pFile = pObj
        ? pObj->GetPreviewMetaFile()
        : ::boost::shared_ptr<GDIMetaFile>();
    pMetaFile = pFile;
    Invalidate();
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl(
    Window* pParent, const ResId& rResId )
    : Window(pParent, rResId), pMetaFile()
{
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl( Window* pParent )
    : Window(pParent, 0 ), pMetaFile()
{
    Resize();
    Show();
}

SfxPreviewBase_Impl::~SfxPreviewBase_Impl()
{
}

void SfxPreviewBase_Impl::Resize()
{
    Invalidate();
}

void SfxPreviewBase_Impl::SetGDIFile( ::boost::shared_ptr<GDIMetaFile> pFile )
{
    pMetaFile = pFile;
    Invalidate();
}

SfxFrameWindow* SfxPreviewWin_Impl::PreviewFactory(
    SfxFrame* pFrame, const String& /*rName*/ )
{
    return new SfxFrameWindow( new SfxPreviewWin_Impl(
        &pFrame->GetCurrentViewFrame()->GetWindow()  ) );
}

void SfxPreviewWin_Impl::ImpPaint(
    const Rectangle&, GDIMetaFile* pFile, Window* pWindow )
{
    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
    DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
                "size of first page is 0, overload GetFirstPageSize or set vis-area!" );
#define FRAME 4
    long nWidth = pWindow->GetOutputSize().Width() - 2*FRAME;
    long nHeight = pWindow->GetOutputSize().Height() - 2*FRAME;
    if( nWidth < 0 ) nWidth = 0;
    if( nHeight < 0 ) nHeight = 0;

    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
    double dRatioPreV=((double) nWidth ) / nHeight;
    Size aSize;
    Point aPoint;
    if (dRatio>dRatioPreV)
    {
        aSize=Size(nWidth, (USHORT)(nWidth/dRatio));
        aPoint=Point( 0, (USHORT)((nHeight-aSize.Height())/2));
    }
    else
    {
        aSize=Size((USHORT)(nHeight*dRatio), nHeight);
        aPoint=Point((USHORT)((nWidth-aSize.Width())/2),0);
    }
    Point bPoint=Point(nWidth,nHeight)-aPoint;


    pWindow->SetLineColor();
    Color aLightGrayCol( COL_LIGHTGRAY );
    pWindow->SetFillColor( aLightGrayCol );
    pWindow->DrawRect( Rectangle( Point( 0,0 ), pWindow->GetOutputSize() ) );
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

SfxPreviewWin::SfxPreviewWin(
    Window* pParent, const ResId& rResId, SfxObjectShellLock &rDocSh )
    : Window(pParent, rResId), rDocShell( rDocSh )
{
    SetHelpId( HID_PREVIEW_FRAME );

    // adjust contrast mode initially
    SetDrawMode( OUTPUT_DRAWMODE_COLOR );

    // This preview window is for document previews.  Therefore
    // right-to-left mode should be off
    EnableRTL( FALSE );
}

void SfxPreviewWin::Paint( const Rectangle& rRect )
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( &rDocShell );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
    {
        return;
    }

    Size            aTmpSize( rDocShell->GetFirstPageSize() );
    GDIMetaFile     aMtf;
    VirtualDevice   aDevice;

    DBG_ASSERT( aTmpSize.Height() * aTmpSize.Width(), "size of first page is 0, overload GetFirstPageSize or set vis-area!" );

    aMtf.SetPrefSize( aTmpSize );
    aDevice.EnableOutput( FALSE );
    aDevice.SetMapMode( rDocShell->GetMapUnit() );
    aDevice.SetDrawMode( GetDrawMode() );
    aMtf.Record( &aDevice );
    rDocShell->DoDraw( &aDevice, Point(0,0), aTmpSize, JobSetup(), ASPECT_THUMBNAIL );
    aMtf.Stop();
    aMtf.WindStart();
    SfxPreviewWin_Impl::ImpPaint( rRect, &aMtf, this );
}

void SfxPreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetDrawMode( OUTPUT_DRAWMODE_COLOR );
    }
}

class SfxNewFileDialog_Impl
{
    FixedText aRegionFt;
    ListBox aRegionLb;
    FixedText aTemplateFt;
    ListBox aTemplateLb;

    CheckBox aPreviewBtn;
    SfxPreviewWin_Impl aPreviewWin;

    FixedText aTitleFt;
    Edit aTitleEd;
    FixedText aThemaFt;
    Edit aThemaEd;
    FixedText aKeywordsFt;
    Edit aKeywordsEd;
    FixedText aDescFt;
    MultiLineEdit aDescEd;
    FixedLine aDocinfoGb;

    CheckBox aTextStyleCB;
    CheckBox aFrameStyleCB;
    CheckBox aPageStyleCB;
    CheckBox aNumStyleCB;
    CheckBox aMergeStyleCB;
    PushButton aLoadFilePB;

    OKButton aOkBt;
    CancelButton aCancelBt;
    HelpButton aHelpBt;
    MoreButton* pMoreBt;
    Timer aPrevTimer;
    String aNone;
    String sLoadTemplate;

    USHORT nFlags;
    SfxDocumentTemplates aTemplates;
    SfxObjectShellLock xDocShell;
    SfxNewFileDialog* pAntiImpl;

    void ClearInfo();
    DECL_LINK( Update, void * );

    DECL_LINK( RegionSelect, ListBox * );
    DECL_LINK( TemplateSelect, ListBox * );
    DECL_LINK( DoubleClick, ListBox * );
    void TogglePreview(CheckBox *);
    DECL_LINK( Expand, MoreButton * );
    DECL_LINK( PreviewClick, CheckBox * );
    DECL_LINK( LoadFile, PushButton* );
    USHORT  GetSelectedTemplatePos() const;

public:

    SfxNewFileDialog_Impl( SfxNewFileDialog* pAntiImplP, USHORT nFlags );
    ~SfxNewFileDialog_Impl();

        // Liefert FALSE, wenn '- Keine -' als Vorlage eingestellt ist
        // Nur wenn IsTemplate() TRUE liefert, koennen Vorlagennamen
        // erfragt werden
    BOOL IsTemplate() const;
    String GetTemplateRegion() const;
    String GetTemplateName() const;
    String GetTemplateFileName() const;

    USHORT  GetTemplateFlags()const;
    void    SetTemplateFlags(USHORT nSet);
};


//-------------------------------------------------------------------------

void SfxNewFileDialog_Impl::ClearInfo()
{
    const String aNo;
    aTitleEd.SetText(aNo);
    aThemaEd.SetText(aNo);
    aKeywordsEd.SetText(aNo);
    aDescEd.SetText(aNo);
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, Update, void *, EMPTYARG )
{
    if ( xDocShell.Is() )
    {
        if ( xDocShell->GetProgress() )
            return FALSE;
        xDocShell.Clear();
    }

    const USHORT nEntry = GetSelectedTemplatePos();
    if(!nEntry)
    {
        ClearInfo();
        aPreviewWin.Invalidate();
        aPreviewWin.SetObjectShell( 0);
        return 0;
    }

    if ( aPreviewBtn.IsChecked() && (nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
    {

        String aFileName = aTemplates.GetPath( aRegionLb.GetSelectEntryPos(), nEntry-1);
        INetURLObject aTestObj( aFileName );
        if( aTestObj.GetProtocol() == INET_PROT_NOT_VALID )
        {
            // temp. fix until Templates are managed by UCB compatible service
            // does NOT work with locally cached components !
            String aTemp;
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
            Window *pParent = Application::GetDefDialogParent();
            Application::SetDefDialogParent( pAntiImpl );
            SfxErrorContext eEC(ERRCTX_SFX_LOADTEMPLATE,pAntiImpl);
            SfxApplication *pSfxApp = SFX_APP();
            ULONG lErr;
            SfxItemSet* pSet = new SfxAllItemSet( pSfxApp->GetPool() );
            pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
            pSet->Put( SfxBoolItem( SID_PREVIEW, TRUE ) );
            lErr = pSfxApp->LoadTemplate( xDocShell, aFileName, TRUE, pSet );
            if( lErr )
                ErrorHandler::HandleError(lErr);
            Application::SetDefDialogParent( pParent );
            if ( !xDocShell.Is() )
            {
                aPreviewWin.SetObjectShell( 0 );
                return FALSE;
            }
        }

        aPreviewWin.SetObjectShell( xDocShell );
    }
    return TRUE;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, RegionSelect, ListBox *, pBox )
{
    if ( xDocShell.Is() && xDocShell->GetProgress() )
        return 0;

    const USHORT nRegion = pBox->GetSelectEntryPos();
    const USHORT nCount = aTemplates.GetRegionCount()? aTemplates.GetCount(nRegion): 0;
    aTemplateLb.SetUpdateMode(FALSE);
    aTemplateLb.Clear();
    String aSel=aRegionLb.GetSelectEntry();
    USHORT nc=aSel.Search('(');
    if (nc-1&&nc!=STRING_NOTFOUND)
        aSel.Erase(nc-1);
    if (aSel.CompareIgnoreCaseToAscii( String(SfxResId(STR_STANDARD)) )==COMPARE_EQUAL)
        aTemplateLb.InsertEntry(aNone);
    for (USHORT i = 0; i < nCount; ++i)
        aTemplateLb.InsertEntry(aTemplates.GetName(nRegion, i));
    aTemplateLb.SelectEntryPos(0);
    aTemplateLb.SetUpdateMode(TRUE);
    aTemplateLb.Invalidate();
    aTemplateLb.Update();
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, Expand, MoreButton *, EMPTYARG )
{
    TemplateSelect(&aTemplateLb);
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, Expand, MoreButton *, pMoreButton )

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, PreviewClick, CheckBox *, pBox )
{
    if ( xDocShell.Is() && xDocShell->GetProgress() )
        return 0;

    USHORT nEntry = GetSelectedTemplatePos();
    if ( nEntry && pBox->IsChecked() )
    {
        if(!Update(0))
            aPreviewWin.Invalidate();
    }
    else
    {
        if (xDocShell.Is())
            xDocShell.Clear();
        aPreviewWin.SetObjectShell( 0 );
    }
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxNewFileDialog_Impl, TemplateSelect, ListBox *, EMPTYARG )
{
    // noch am Laden
    if ( xDocShell && xDocShell->GetProgress() )
        return 0;

    if ( !MORE_BTN(GetState()) )
        // Dialog nicht aufgeklappt
        return 0;

    aPrevTimer.Start();
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )
{
    (void)pListBox;
    // noch am Laden
    if ( !xDocShell.Is() || !xDocShell->GetProgress() )
        pAntiImpl->EndDialog(RET_OK);
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, LoadFile, PushButton *, EMPTYARG )
{
    pAntiImpl->EndDialog(RET_TEMPLATE_LOAD);
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, LoadFile, PushButton *, EMPTYARG )
//-------------------------------------------------------------------------

USHORT  SfxNewFileDialog_Impl::GetSelectedTemplatePos() const
{
    USHORT nEntry=aTemplateLb.GetSelectEntryPos();
    String aSel=aRegionLb.GetSelectEntry().Copy();
    USHORT nc=aSel.Search('(');
    if (nc-1&&nc!=STRING_NOTFOUND)
        aSel.Erase(nc-1);
    if (aSel.CompareIgnoreCaseToAscii(String(SfxResId(STR_STANDARD)))!=COMPARE_EQUAL)
        nEntry++;
    if (!aTemplateLb.GetSelectEntryCount())
        nEntry=0;
    return nEntry;
}

//-------------------------------------------------------------------------

BOOL SfxNewFileDialog_Impl::IsTemplate() const
{
    return GetSelectedTemplatePos()!=0;

}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateFileName() const
{
    if(!IsTemplate() || !aTemplates.GetRegionCount())
        return String();
    return aTemplates.GetPath(aRegionLb.GetSelectEntryPos(),
                              GetSelectedTemplatePos()-1);
}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateRegion() const
{
    if(!IsTemplate() || !aTemplates.GetRegionCount())
        return String();
    return aRegionLb.GetSelectEntry();
}

//-------------------------------------------------------------------------

String SfxNewFileDialog_Impl::GetTemplateName() const
{
    if(!IsTemplate() || !aTemplates.GetRegionCount())
        return String();
    return aTemplateLb.GetSelectEntry();
}

//-------------------------------------------------------------------------

void AdjustPosSize_Impl(Window *pWin, short nMoveOffset, short nSizeOffset)
{
    Point aPos(pWin->GetPosPixel());
    Size aSize(pWin->GetSizePixel());
    aPos.X() -= nMoveOffset;
    aSize.Width() += nSizeOffset;
    pWin->SetPosSizePixel(aPos, aSize);
}
//-------------------------------------------------------------------------
USHORT  SfxNewFileDialog_Impl::GetTemplateFlags()const
{
    USHORT nRet = aTextStyleCB.IsChecked() ? SFX_LOAD_TEXT_STYLES : 0;
    if(aFrameStyleCB.IsChecked())
        nRet |= SFX_LOAD_FRAME_STYLES;
    if(aPageStyleCB.IsChecked())
        nRet |= SFX_LOAD_PAGE_STYLES;
    if(aNumStyleCB.IsChecked())
        nRet |= SFX_LOAD_NUM_STYLES;
    if(aMergeStyleCB.IsChecked())
        nRet |= SFX_MERGE_STYLES;
    return nRet;
}
//-------------------------------------------------------------------------
void    SfxNewFileDialog_Impl::SetTemplateFlags(USHORT nSet)
{
    aTextStyleCB.Check(  0 != (nSet&SFX_LOAD_TEXT_STYLES ));
    aFrameStyleCB.Check( 0 != (nSet&SFX_LOAD_FRAME_STYLES));
    aPageStyleCB.Check(  0 != (nSet&SFX_LOAD_PAGE_STYLES ));
    aNumStyleCB.Check(   0 != (nSet&SFX_LOAD_NUM_STYLES  ));
    aMergeStyleCB.Check( 0 != (nSet&SFX_MERGE_STYLES     ));
}

//-------------------------------------------------------------------------

SfxNewFileDialog_Impl::SfxNewFileDialog_Impl(
    SfxNewFileDialog* pAntiImplP, USHORT nFl)
    :   aRegionFt( pAntiImplP, SfxResId( FT_REGION ) ),
        aRegionLb( pAntiImplP, SfxResId( LB_REGION ) ),
        aTemplateFt( pAntiImplP, SfxResId( FT_TEMPLATE ) ),
        aTemplateLb( pAntiImplP, SfxResId( LB_TEMPLATE ) ),
        aPreviewBtn( pAntiImplP, SfxResId( BTN_PREVIEW ) ),
        aPreviewWin( pAntiImplP, SfxResId( WIN_PREVIEW ) ),
        aTitleFt( pAntiImplP, SfxResId( FT_TITLE ) ),
        aTitleEd( pAntiImplP, SfxResId( ED_TITLE ) ),
        aThemaFt( pAntiImplP, SfxResId( FT_THEMA ) ),
        aThemaEd( pAntiImplP, SfxResId( ED_THEMA ) ),
        aKeywordsFt( pAntiImplP, SfxResId( FT_KEYWORDS ) ),
        aKeywordsEd( pAntiImplP, SfxResId( ED_KEYWORDS ) ),
        aDescFt( pAntiImplP, SfxResId( FT_DESC ) ),
        aDescEd( pAntiImplP, SfxResId( ED_DESC ) ),
        aDocinfoGb( pAntiImplP, SfxResId( GB_DOCINFO ) ),
        aTextStyleCB( pAntiImplP, SfxResId(  CB_TEXT_STYLE )),
        aFrameStyleCB( pAntiImplP, SfxResId( CB_FRAME_STYLE )),
        aPageStyleCB( pAntiImplP, SfxResId(  CB_PAGE_STYLE )),
        aNumStyleCB( pAntiImplP, SfxResId(   CB_NUM_STYLE  )),
        aMergeStyleCB( pAntiImplP, SfxResId( CB_MERGE_STYLE )),
        aLoadFilePB( pAntiImplP, SfxResId(   PB_LOAD_FILE )),
        aOkBt( pAntiImplP, SfxResId( BT_OK ) ),
        aCancelBt( pAntiImplP, SfxResId( BT_CANCEL ) ),
        aHelpBt( pAntiImplP, SfxResId( BT_HELP ) ),
        pMoreBt( new MoreButton( pAntiImplP, SfxResId( BT_MORE ) ) ),
        aNone( SfxResId(STR_NONE) ),
        sLoadTemplate( SfxResId(STR_LOAD_TEMPLATE)),
        nFlags(nFl),
        pAntiImpl( pAntiImplP )
{
    short nMoveOffset = *(short *)pAntiImplP->GetClassRes();
    pAntiImplP->IncrementRes(sizeof(short));
    short nExpandSize= *(short *)pAntiImplP->GetClassRes();
    pAntiImplP->IncrementRes(sizeof(short));
    pAntiImplP->FreeResource();

    if (!nFlags)
        MORE_BTN(Hide());
    else if(SFXWB_LOAD_TEMPLATE == nFlags)
    {
        aLoadFilePB.SetClickHdl(LINK(this, SfxNewFileDialog_Impl, LoadFile));
        aLoadFilePB.Show();
        aTextStyleCB.Show();
        aFrameStyleCB.Show();
        aPageStyleCB.Show();
        aNumStyleCB.Show();
        aMergeStyleCB.Show();
        Size aSize(pAntiImplP->GetOutputSizePixel());
        Size aTmp(pAntiImplP->LogicToPixel(Size(16, 16), MAP_APPFONT));
        aSize.Height() += aTmp.Height();
        pAntiImplP->SetOutputSizePixel(aSize);
        pMoreBt->Hide();
        aTextStyleCB.Check();
        pAntiImplP->SetText(sLoadTemplate);
    }
    else
    {
        MORE_BTN(SetClickHdl(LINK(this, SfxNewFileDialog_Impl, Expand)));
        if((nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
        {
            MORE_BTN(AddWindow(&aPreviewBtn));
            MORE_BTN(AddWindow(&aPreviewWin));
            aPreviewBtn.SetClickHdl(LINK(this, SfxNewFileDialog_Impl, PreviewClick));
        }
        else
        {
            aPreviewBtn.Hide();
            aPreviewWin.Hide();
            nMoveOffset = (short)pAntiImplP->LogicToPixel(
                Size(nMoveOffset, nMoveOffset), MAP_APPFONT).Width();
            nExpandSize = (short)pAntiImplP->LogicToPixel(
                Size(nExpandSize, nExpandSize), MAP_APPFONT).Width();
            AdjustPosSize_Impl(&aTitleFt, nMoveOffset, 0);
            AdjustPosSize_Impl(&aTitleEd, nMoveOffset, nExpandSize);
            AdjustPosSize_Impl(&aThemaFt, nMoveOffset, 0);
            AdjustPosSize_Impl(&aThemaEd, nMoveOffset, nExpandSize);
            AdjustPosSize_Impl(&aKeywordsFt, nMoveOffset, 0);
            AdjustPosSize_Impl(&aKeywordsEd, nMoveOffset, nExpandSize);
            AdjustPosSize_Impl(&aDescFt , nMoveOffset, 0);
            AdjustPosSize_Impl(&aDescEd , nMoveOffset, nExpandSize);
            AdjustPosSize_Impl(&aDocinfoGb, nMoveOffset, nExpandSize);
        }
    }

    String &rExtra = pAntiImplP->GetExtraData();
    USHORT nTokCount = rExtra.GetTokenCount( '|' );
    if( nTokCount > 0 && nFlags )
        MORE_BTN(SetState( rExtra.GetToken( 0, '|' ) == 'Y' ));
    if( nTokCount > 1 && nFlags )
        aPreviewBtn.Check( rExtra.GetToken( 1 ,'|' ) == 'Y' );

    aTemplateLb.SetDoubleClickHdl(LINK(this, SfxNewFileDialog_Impl, DoubleClick));

    // update the template configuration if necessary
    {
        WaitObject aWaitCursor( pAntiImplP->GetParent() );
        aTemplates.Update( sal_True /* be smart */ );
    }
    // fill the list boxes
    const USHORT nCount = aTemplates.GetRegionCount();
    if (nCount)
    {
        for(USHORT i = 0; i < nCount; ++i)
            aRegionLb.InsertEntry(aTemplates.GetFullRegionName(i));
        aRegionLb.SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, RegionSelect));
    }

    aPrevTimer.SetTimeout( 500 );
    aPrevTimer.SetTimeoutHdl( LINK( this, SfxNewFileDialog_Impl, Update));

    aRegionLb.SelectEntryPos(0);
    RegionSelect(&aRegionLb);
}

//-------------------------------------------------------------------------

SfxNewFileDialog_Impl::~SfxNewFileDialog_Impl()
{
    String &rExtra = pAntiImpl->GetExtraData();
    rExtra = MORE_BTN(GetState()) ? 'Y' : 'N';
    rExtra += '|';
    rExtra += aPreviewBtn.IsChecked() ? 'Y' : 'N';

    delete pMoreBt;
}
//-------------------------------------------------------------------------
SfxNewFileDialog::SfxNewFileDialog(Window *pParent, USHORT nFlags)
    : SfxModalDialog( pParent, SfxResId( DLG_NEW_FILE ) )
{
    pImpl = new SfxNewFileDialog_Impl( this, nFlags );
}
//-------------------------------------------------------------------------
SfxNewFileDialog::~SfxNewFileDialog()
{
    delete pImpl;
}
//-------------------------------------------------------------------------
BOOL SfxNewFileDialog::IsTemplate() const
{
    return pImpl->IsTemplate();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateRegion() const
{
    return pImpl->GetTemplateRegion();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateName() const
{
    return pImpl->GetTemplateName();
}
//-------------------------------------------------------------------------
String SfxNewFileDialog::GetTemplateFileName() const
{
    return pImpl->GetTemplateFileName();
}
//-------------------------------------------------------------------------
USHORT SfxNewFileDialog::GetTemplateFlags()const
{
    return pImpl->GetTemplateFlags();

}
//-------------------------------------------------------------------------
void    SfxNewFileDialog::SetTemplateFlags(USHORT nSet)
{
    pImpl->SetTemplateFlags(nSet);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
