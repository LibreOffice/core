/*************************************************************************
 *
 *  $RCSfile: new.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-09-28 11:45:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "new.hxx"

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SVMEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#include <svtools/itemset.hxx>
#include <svtools/eitem.hxx>
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <tools/urlobj.hxx>

#include "new.hrc"
#include "doc.hrc"
#include "sfx.hrc"
#include "helpid.hrc"
#include "sfxtypes.hxx"
#include "app.hxx"
#include "docinf.hxx"
#include "viewfrm.hxx"
#include "docfac.hxx"
#include "objsh.hxx"
#include "fltfnc.hxx"
#include "viewsh.hxx"
#include "viewfac.hxx"
#include "interno.hxx"
#include "sfxresid.hxx"
#include "docfile.hxx"
#include "preview.hxx"

//========================================================================

#define MORE_BTN(x) pMoreBt->x

//========================================================================

void SfxPreviewBase_Impl::SetObjectShell( SfxObjectShell* pObj )
{
    GDIMetaFile* pFile = pObj ? pObj->GetPreviewMetaFile( ) : 0;
    delete pMetaFile;
    pMetaFile = pFile;
    Invalidate();
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl(
    Window* pParent, const ResId& rResId )
    : Window(pParent, rResId), pMetaFile( 0 )
{
}

SfxPreviewBase_Impl::SfxPreviewBase_Impl( Window* pParent )
    : Window(pParent, 0 ), pMetaFile( 0 )
{
    Resize();
    Show();
}

SfxPreviewBase_Impl::~SfxPreviewBase_Impl()
{
    delete pMetaFile;
}

void SfxPreviewBase_Impl::Resize()
{
    Invalidate();
}

void SfxPreviewBase_Impl::SetGDIFile( GDIMetaFile* pFile )
{
    delete pMetaFile;
    pMetaFile = pFile;
    Invalidate();
}

SfxFrameWindow* SfxPreviewWin_Impl::PreviewFactory(
    SfxFrame* pFrame, const String& rName )
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
//!     pFile->Move( Point( FRAME, FRAME ) );
//!     pFile->Scale( Fraction( aTmpSize.Width(), aSize.Width() ),
//!                   Fraction( aTmpSize.Height(), aSize.Height() ) );
        pFile->WindStart();
        pFile->Play( pWindow, aPoint + Point( FRAME, FRAME ), aSize  );
    }
}

void SfxPreviewWin_Impl::Paint( const Rectangle& rRect )
{
    ImpPaint( rRect, pMetaFile, this );
}

SfxPreviewWin::SfxPreviewWin(
    Window* pParent, const ResId& rResId, SfxObjectShellLock &rDocSh )
    : Window(pParent, rResId), rDocShell( rDocSh )
{
    SetHelpId( HID_PREVIEW_FRAME );
}

void SfxPreviewWin::Paint( const Rectangle& rRect )
{
    GDIMetaFile* pFile = rDocShell->GetPreviewMetaFile();
    SfxPreviewWin_Impl::ImpPaint( rRect, pFile, this );
    delete pFile;
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
    GroupBox aDocinfoGb;

    OKButton aOkBt;
    CancelButton aCancelBt;
    HelpButton aHelpBt;
    MoreButton* pMoreBt;
    Timer aPrevTimer;
    String aNone;

    USHORT nFlags;
    SfxDocumentTemplates aTemplates;
    SfxObjectShellLock xDocShell;
    SfxDocumentInfo *pDocInfo;
    SfxNewFileDialog* pAntiImpl;

    void ClearInfo();
    DECL_LINK( Update, void * );

    DECL_LINK( RegionSelect, ListBox * );
    DECL_LINK( TemplateSelect, ListBox * );
    DECL_LINK( DoubleClick, ListBox * );
    void TogglePreview(CheckBox *);
    DECL_LINK( Expand, MoreButton * );
    DECL_LINK( PreviewClick, CheckBox * );
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

    if (nFlags & SFXWB_DOCINFO)
    {
        // DocInfo anzeigen
        const String aFile(
            aTemplates.GetPath(aRegionLb.GetSelectEntryPos(), nEntry-1) );

        // Dokumentinfo lesen und anzeigen
        if (pAntiImpl->FillDocumentInfo(aFile, *pDocInfo))
        {
            aTitleEd.SetText(pDocInfo->GetTitle());
            aThemaEd.SetText(pDocInfo->GetTheme());
            aKeywordsEd.SetText(pDocInfo->GetKeywords());
            aDescEd.SetText(pDocInfo->GetComment());
        }
        else
            ClearInfo();
    }

    if ( aPreviewBtn.IsChecked() && (nFlags & SFXWB_PREVIEW) == SFXWB_PREVIEW)
    {

        String aFileName = aTemplates.GetPath( aRegionLb.GetSelectEntryPos(), nEntry-1);
        INetURLObject aObj( aFileName, INET_PROT_FILE );

        for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
              pTmp;
              pTmp = SfxObjectShell::GetNext(*pTmp) )
        {
            //! fsys bug op==
            if ( pTmp->GetMedium())
                // ??? HasName() MM
                if( INetURLObject( pTmp->GetMedium()->GetName(), INET_PROT_FILE ) == aObj )
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
            if( lErr = pSfxApp->LoadTemplate( xDocShell, aFileName, String(), TRUE, pSet ) )
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
    if (nFlags & SFXWB_DOCINFO && aTemplateLb.GetEntryCount() >= 1)
        TemplateSelect(&aTemplateLb);
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SfxNewFileDialog_Impl, Expand, MoreButton *, pMoreButton )
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

IMPL_LINK( SfxNewFileDialog_Impl, TemplateSelect, ListBox *, pListBox )
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
    // noch am Laden
    if ( !xDocShell.Is() || !xDocShell->GetProgress() )
        pAntiImpl->EndDialog(RET_OK);
    return 0;
}
IMPL_LINK_INLINE_END( SfxNewFileDialog_Impl, DoubleClick, ListBox *, pListBox )

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

SfxNewFileDialog_Impl::SfxNewFileDialog_Impl(
    SfxNewFileDialog* pAntiImplP, USHORT nFl)
    :   aOkBt( pAntiImplP, ResId( BT_OK ) ),
        aCancelBt( pAntiImplP, ResId( BT_CANCEL ) ),
        aTemplateLb( pAntiImplP, ResId( LB_TEMPLATE ) ),
        aRegionFt( pAntiImplP, ResId( FT_REGION ) ),
        aThemaFt( pAntiImplP, ResId( FT_THEMA ) ),
        aPreviewWin( pAntiImplP, ResId( WIN_PREVIEW ) ),
        aTemplateFt( pAntiImplP, ResId( FT_TEMPLATE ) ),
        aRegionLb( pAntiImplP, ResId( LB_REGION ) ),
        aHelpBt( pAntiImplP, ResId( BT_HELP ) ),
        aKeywordsEd( pAntiImplP, ResId( ED_KEYWORDS ) ),
        aTitleEd( pAntiImplP, ResId( ED_TITLE ) ),
        aThemaEd( pAntiImplP, ResId( ED_THEMA ) ),
        pMoreBt( new MoreButton( pAntiImplP, ResId( BT_MORE ) ) ),
        aPreviewBtn( pAntiImplP, ResId( BTN_PREVIEW ) ),
        aDocinfoGb( pAntiImplP, ResId( GB_DOCINFO ) ),
        aTitleFt( pAntiImplP, ResId( FT_TITLE ) ),
        aKeywordsFt( pAntiImplP, ResId( FT_KEYWORDS ) ),
        aDescFt( pAntiImplP, ResId( FT_DESC ) ),
        aDescEd( pAntiImplP, ResId( ED_DESC ) ),
        aNone( ResId(STR_NONE) ),
        nFlags(nFl),
        pDocInfo(0),
        pAntiImpl( pAntiImplP )
{
    short nMoveOffset = *(short *)pAntiImplP->GetClassRes();
    pAntiImplP->IncrementRes(sizeof(short));
    short nExpandSize= *(short *)pAntiImplP->GetClassRes();
    pAntiImplP->IncrementRes(sizeof(short));
    pAntiImplP->FreeResource();
    aTemplates.Construct();

    if (!nFlags)
        MORE_BTN(Hide());
    else
    {
        MORE_BTN(SetClickHdl(LINK(this, SfxNewFileDialog_Impl, Expand)));
        if(nFlags & SFXWB_DOCINFO)
        {
            MORE_BTN(AddWindow(&aTitleFt));
            MORE_BTN(AddWindow(&aTitleEd));
            MORE_BTN(AddWindow(&aThemaFt));
            MORE_BTN(AddWindow(&aThemaEd));
            MORE_BTN(AddWindow(&aKeywordsFt));
            MORE_BTN(AddWindow(&aKeywordsEd));
            MORE_BTN(AddWindow(&aDescFt));
            MORE_BTN(AddWindow(&aDescEd));
            MORE_BTN(AddWindow(&aDocinfoGb));
            aTemplateLb.SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, TemplateSelect));
            pDocInfo = new SfxDocumentInfo;
        }
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
    const USHORT nCount = aTemplates.GetRegionCount();
    if (nCount)
    {
        for(USHORT i = 0; i < nCount; ++i)
            aRegionLb.InsertEntry(aTemplates.GetFullRegionName(i));
        aRegionLb.SetSelectHdl(LINK(this, SfxNewFileDialog_Impl, RegionSelect));
    }

    aPrevTimer.SetTimeout( 500 );
    aPrevTimer.SetTimeoutHdl( LINK( this, SfxNewFileDialog_Impl, Update));

//   else
//        aRegionLb.InsertEntry(String(SfxResId(STR_STANDARD)));
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

    delete pDocInfo;
    delete pMoreBt;
}


SfxNewFileDialog::SfxNewFileDialog(Window *pParent, USHORT nFlags)
    : SfxModalDialog( pParent, SfxResId( DLG_NEW_FILE ) )
{
    pImpl = new SfxNewFileDialog_Impl( this, nFlags );
}

SfxNewFileDialog::~SfxNewFileDialog()
{
    delete pImpl;
}

BOOL SfxNewFileDialog::IsTemplate() const
{
    return pImpl->IsTemplate();
}

String SfxNewFileDialog::GetTemplateRegion() const
{
    return pImpl->GetTemplateRegion();
}

String SfxNewFileDialog::GetTemplateName() const
{
    return pImpl->GetTemplateName();
}

String SfxNewFileDialog::GetTemplateFileName() const
{
    return pImpl->GetTemplateFileName();
}

BOOL SfxNewFileDialog::FillDocumentInfo
(
    const String &rFile,    // Datei incl. Pfad, deren DocInfo gelesen werden soll
    SfxDocumentInfo &rInfo  // DocInfo, die gefuellt werden soll
)
{
    SvStorageRef aStor = new SvStorage(
        rFile, STREAM_READ |STREAM_NOCREATE | STREAM_SHARE_DENYWRITE, STORAGE_TRANSACTED );
    if ( SVSTREAM_OK != aStor->GetError() )
        return FALSE;
    BOOL bLoadOk;
    bLoadOk=rInfo.Load(aStor);
    return bLoadOk;
}


