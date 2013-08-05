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

#include <editeng/sizeitem.hxx>
#include <tools/shl.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>

#include <cuires.hrc>
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include "svx/svxgrahicitem.hxx"
#include <sfx2/request.hxx>
#include <sfx2/dialoghelper.hxx>
#include "svx/ofaitem.hxx"
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/svdmodel.hxx>
#include <svx/numvset.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/numitem.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/objsh.hxx>
#include <editeng/brushitem.hxx>
#include <svx/gallery.hxx>
#include <unotools/localfilehelper.hxx>
#include "paragrph.hrc"
#include "sfx2/opengrf.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#define MAX_BMP_WIDTH   16
#define MAX_BMP_HEIGHT  16

#define MN_GALLERY         4
#define MN_SYMBOLS         5
#define MN_SYMBOLS_NONE    1
#define MN_SYMBOLS_AUTO    2
#define MN_GALLERY_ENTRY 100


// static ----------------------------------------------------------------

static sal_uInt16 pLineRanges[] =
{
    XATTR_LINETRANSPARENCE,
    XATTR_LINETRANSPARENCE,
    SID_ATTR_LINE_STYLE,
    SID_ATTR_LINE_ENDCENTER,
    0
};

SvxLineTabPage::SvxLineTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SvxTabPage ( pParent
                 ,"LineTabPage"
                 ,"cui/ui/linetabpage.ui"
                 , rInAttrs ),

    pSymbolList(NULL),
    bNewSize(false),
    nNumMenuGalleryItems(0),
    nSymbolType(SVX_SYMBOLTYPE_UNKNOWN), // unknown respectively unchanged
    pSymbolAttr(NULL),

    bLastWidthModified(sal_False),
    aSymbolLastSize(Size(0,0)),
    bSymbols(sal_False),

    rOutAttrs           ( rInAttrs ),
    bObjSelected( sal_False ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( 1 ),
    aXDash              ( OUString(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( OUString(), COL_LIGHTRED ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
     pnLineEndListState( 0 ),
    pnDashListState( 0 ),
    pnColorListState( 0 ),
   nPageType           ( 0 )
{
    get(m_pLbLineStyle,"LB_LINE_STYLE");
    get(m_pLbColor,"LB_COLOR");
    get(m_pBoxStyle,"boxSTYLE_ATTR");
    get(m_pMtrLineWidth,"MTR_FLD_LINE_WIDTH");
    get(m_pMtrTransparent,"MTR_LINE_TRANSPARENT");

    get(m_pFlLineEnds,"FL_LINE_ENDS");
    get(m_pBoxArrowStyles,"boxARROW_STYLES");
    get(m_pLbStartStyle,"LB_START_STYLE");
    get(m_pBoxStart,"boxSTART");
    get(m_pMtrStartWidth,"MTR_FLD_START_WIDTH");
    get(m_pTsbCenterStart,"TSB_CENTER_START");
    get(m_pBoxEnd,"boxEND");
    get(m_pLbEndStyle,"LB_END_STYLE");
    get(m_pMtrEndWidth,"MTR_FLD_END_WIDTH");
    get(m_pTsbCenterEnd,"TSB_CENTER_END");
    get(m_pCbxSynchronize,"CBX_SYNCHRONIZE");
    get(m_pCtlPreview,"CTL_PREVIEW");

    get(m_pGridEdgeCaps,"gridEDGE_CAPS");
    get(m_pFLEdgeStyle,"FL_EDGE_STYLE");
    get(m_pLBEdgeStyle,"LB_EDGE_STYLE");
    // LineCaps
    get(m_pLBCapStyle,"LB_CAP_STYLE");

    //#58425# Symbols on a line (e.g. StarChart)
    get(m_pFlSymbol,"FL_SYMBOL_FORMAT");
    get(m_pGridIconSize,"gridICON_SIZE");
    get(m_pSymbolMB,"MB_SYMBOL_BITMAP");
    get(m_pSymbolWidthMF,"MF_SYMBOL_WIDTH");
    get(m_pSymbolHeightMF,"MF_SYMBOL_HEIGHT");
    get(m_pSymbolRatioCB,"CB_SYMBOL_RATIO");

    // This Page requires ExchangeSupport
    SetExchangeSupport();

    // Metric set
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            // no break -> we now have mm
        case FUNIT_MM:
            m_pMtrLineWidth->SetSpinSize( 50 );
            m_pMtrStartWidth->SetSpinSize( 50 );
            m_pMtrEndWidth->SetSpinSize( 50 );
            break;

            case FUNIT_INCH:
            m_pMtrLineWidth->SetSpinSize( 2 );
            m_pMtrStartWidth->SetSpinSize( 2 );
            m_pMtrEndWidth->SetSpinSize( 2 );
            break;
            default: ;// prevent warning
    }
    SetFieldUnit( *m_pMtrLineWidth, eFUnit );
    SetFieldUnit( *m_pMtrStartWidth, eFUnit );
    SetFieldUnit( *m_pMtrEndWidth, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    m_pLbLineStyle->SetSelectHdl( LINK( this, SvxLineTabPage, ClickInvisibleHdl_Impl ) );
    m_pLbColor->SetSelectHdl( LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    m_pMtrLineWidth->SetModifyHdl( LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    m_pMtrTransparent->SetModifyHdl( LINK( this, SvxLineTabPage, ChangeTransparentHdl_Impl ) );

    Link aStart = LINK( this, SvxLineTabPage, ChangeStartHdl_Impl );
    Link aEnd = LINK( this, SvxLineTabPage, ChangeEndHdl_Impl );
    m_pLbStartStyle->SetSelectHdl( aStart );
    m_pLbEndStyle->SetSelectHdl( aEnd );
    m_pMtrStartWidth->SetModifyHdl( aStart );
    m_pMtrEndWidth->SetModifyHdl( aEnd );
    m_pTsbCenterStart->SetClickHdl( aStart );
    m_pTsbCenterEnd->SetClickHdl( aEnd );

    // #116827#
    Link aEdgeStyle = LINK( this, SvxLineTabPage, ChangeEdgeStyleHdl_Impl );
    m_pLBEdgeStyle->SetSelectHdl( aEdgeStyle );

    // LineCaps
    Link aCapStyle = LINK( this, SvxLineTabPage, ChangeCapStyleHdl_Impl );
    m_pLBCapStyle->SetSelectHdl( aCapStyle );

    // Symbols on a line (eg star charts), MB-handler set
    m_pSymbolMB->SetSelectHdl(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    m_pSymbolMB->SetActivateHdl(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
    m_pSymbolWidthMF->SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_pSymbolHeightMF->SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_pSymbolRatioCB->SetClickHdl(LINK(this, SvxLineTabPage, RatioHdl_Impl));

    m_pSymbolRatioCB->Check(sal_True);
    ShowSymbolControls(sal_False);

    nActLineWidth = -1;
}

void SvxLineTabPage::ShowSymbolControls(sal_Bool bOn)
{
    // Symbols on a line (e.g. StarCharts), symbol-enable controls

    bSymbols=bOn;
    m_pFlSymbol->Show(bOn);
    m_pCtlPreview->ShowSymbol(bOn);
}

SvxLineTabPage::~SvxLineTabPage()
{
    // Symbols on a line (e.g. StarCharts), dtor new!

    delete m_pSymbolMB->GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    if(pSymbolList)
        delete m_pSymbolMB->GetPopupMenu()->GetPopupMenu( MN_SYMBOLS );

    for ( size_t i = 0, n = aGrfBrushItems.size(); i < n; ++i )
    {
        SvxBmpItemInfo* pInfo = aGrfBrushItems[ i ];
        delete pInfo->pBrushItem;
        delete pInfo;
    }
}
void SvxLineTabPage::Construct()
{
    // Color chart
    m_pLbColor->Fill( pColorList );
    FillListboxes();
}

void SvxLineTabPage::InitSymbols(MenuButton* pButton)
{
    // Initialize popup
    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_GALLERY ))
    {
        // Get gallery entries
        GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);

        PopupMenu* pPopup = new PopupMenu;
        OUString aEmptyStr;
        const OUString *pUIName = NULL;
        sal_uInt32 i = 0;
        nNumMenuGalleryItems = aGrfNames.size();
        for(std::vector<OUString>::iterator it = aGrfNames.begin(); it != aGrfNames.end(); ++it, ++i)
        {
            pUIName = &(*it);

            // Convert URL encodings to UI characters (e.g. %20 for spaces)
            OUString aPhysicalName;
            if (utl::LocalFileHelper::ConvertURLToPhysicalName(*it, aPhysicalName))
            {
                pUIName = &aPhysicalName;
            }

            SvxBrushItem* pBrushItem = new SvxBrushItem(*it, aEmptyStr, GPOS_AREA, SID_ATTR_BRUSH);
            pBrushItem->SetDoneLink(STATIC_LINK(this, SvxLineTabPage, GraphicArrivedHdl_Impl));

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (sal_uInt16)(MN_GALLERY_ENTRY + i);
            if ( i < aGrfBrushItems.size() )
            {
                aGrfBrushItems.insert( aGrfBrushItems.begin() + i, pInfo );
            } else
            {
                aGrfBrushItems.push_back( pInfo );
            }
            const Graphic* pGraphic = pBrushItem->GetGraphic();

            if(pGraphic)
            {
                Bitmap aBitmap(pGraphic->GetBitmap());
                Size aSize(aBitmap.GetSizePixel());
                if(aSize.Width()  > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
                {
                    sal_Bool bWidth = aSize.Width() > aSize.Height();
                    double nScale = bWidth ?
                                        (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                        (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                    aBitmap.Scale(nScale, nScale);

                }
                Image aImage(aBitmap);
                pPopup->InsertItem(pInfo->nItemId, *pUIName, aImage );
            }
            else
            {
                Image aImage;
                pPopup->InsertItem(pInfo->nItemId, *pUIName, aImage );
            }
        }
        m_pSymbolMB->GetPopupMenu()->SetPopupMenu( MN_GALLERY, pPopup );

        if(aGrfNames.empty())
            m_pSymbolMB->GetPopupMenu()->EnableItem(MN_GALLERY, sal_False);
    }

    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_SYMBOLS ) && pSymbolList)
    {
        VirtualDevice aVDev;
        aVDev.SetMapMode(MapMode(MAP_100TH_MM));
        SdrModel* pModel = new SdrModel(NULL, NULL, LOADREFCOUNTS);
        pModel->GetItemPool().FreezeIdRanges();
        // Page
        SdrPage* pPage = new SdrPage( *pModel, sal_False );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        // 3D View
        SdrView* pView = new SdrView( pModel, &aVDev );
        pView->hideMarkHandles();
        pView->ShowSdrPage(pPage);

        PopupMenu* pPopup = new PopupMenu;
        OUString aEmptyStr;

        // Generate invisible square to give all symbols a
        // bitmap size, which is independent from specific glyph
        SdrObject *pInvisibleSquare=pSymbolList->GetObj(0);
        pInvisibleSquare=pInvisibleSquare->Clone();
        pPage->NbcInsertObject(pInvisibleSquare);
        pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
        pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

        for(long i=0;; ++i)
        {
            SdrObject *pObj=pSymbolList->GetObj(i);
            if(pObj==NULL)
                break;
            pObj=pObj->Clone();
            aGrfNames.push_back(aEmptyStr);
            pPage->NbcInsertObject(pObj);
            if(pSymbolAttr)
            {
                pObj->SetMergedItemSet(*pSymbolAttr);
            }
            else
            {
                pObj->SetMergedItemSet(rOutAttrs);
            }
            pView->MarkAll();
            BitmapEx aBitmapEx(pView->GetMarkedObjBitmapEx());
            GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());
            pView->UnmarkAll();
            pObj=pPage->RemoveObject(1);
            SdrObject::Free(pObj);

            SvxBrushItem* pBrushItem = new SvxBrushItem(Graphic(aMeta), GPOS_AREA, SID_ATTR_BRUSH);
            pBrushItem->SetDoneLink(STATIC_LINK(this, SvxLineTabPage, GraphicArrivedHdl_Impl));

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (sal_uInt16)(MN_GALLERY_ENTRY + i + nNumMenuGalleryItems);
            if ( (size_t)(nNumMenuGalleryItems + i) < aGrfBrushItems.size() ) {
                aGrfBrushItems.insert( aGrfBrushItems.begin() + nNumMenuGalleryItems + i, pInfo );
            } else {
                aGrfBrushItems.push_back( pInfo );
            }

            Size aSize(aBitmapEx.GetSizePixel());
            if(aSize.Width() > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
            {
                sal_Bool bWidth = aSize.Width() > aSize.Height();
                double nScale = bWidth ?
                                    (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                    (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                aBitmapEx.Scale(nScale, nScale);
            }
            Image aImage(aBitmapEx);
            pPopup->InsertItem(pInfo->nItemId,aEmptyStr,aImage);
        }
        pInvisibleSquare=pPage->RemoveObject(0);
        SdrObject::Free(pInvisibleSquare);

        m_pSymbolMB->GetPopupMenu()->SetPopupMenu( MN_SYMBOLS, pPopup );

        if(aGrfNames.empty())
            m_pSymbolMB->GetPopupMenu()->EnableItem(MN_SYMBOLS, sal_False);

        delete pView;
        delete pModel;
    }
}

void SvxLineTabPage::SymbolSelected(MenuButton* pButton)
{
    sal_uInt16 nItemId = pButton->GetCurItemId();
    const Graphic* pGraphic = 0;
    Graphic aGraphic;
    bool bResetSize = false;
    bool bEnable = true;
    long nPreviousSymbolType = nSymbolType;

    if(nItemId >= MN_GALLERY_ENTRY)
    {
        if( (nItemId-MN_GALLERY_ENTRY) >= nNumMenuGalleryItems)
        {
            nSymbolType=nItemId-MN_GALLERY_ENTRY-nNumMenuGalleryItems; // List's index
        }
        else
        {
            nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
            bResetSize = true;
        }
        SvxBmpItemInfo* pInfo = aGrfBrushItems[ nItemId - MN_GALLERY_ENTRY ];
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else switch(nItemId)
    {
        case MN_SYMBOLS_AUTO:
        {
            pGraphic=&aAutoSymbolGraphic;
            aAutoSymbolGraphic.SetPrefSize( Size(253,253) );
            nSymbolType=SVX_SYMBOLTYPE_AUTO;
        }
        break;

        case MN_SYMBOLS_NONE:
        {
            nSymbolType=SVX_SYMBOLTYPE_NONE;
            pGraphic=NULL;
            bEnable = false;
        }
        break;
        default:
        {
            SvxOpenGraphicDialog aGrfDlg(CUI_RES(RID_SVXSTR_EDIT_GRAPHIC));
            aGrfDlg.EnableLink(false);
            aGrfDlg.AsLink(false);
            if( !aGrfDlg.Execute() )
            {
                // Remember selected filters
                if( !aGrfDlg.GetGraphic(aGraphic) )
                {
                    nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
                    pGraphic = &aGraphic;
                    bResetSize = true;
                }
            }
            if( !pGraphic )
                return;
        }
        break;
    }

    if(pGraphic)
    {
        Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)ePoolUnit);
        aSymbolGraphic=*pGraphic;
        if( bResetSize )
        {
            aSymbolSize=aSize;
        }
        else if( nPreviousSymbolType == SVX_SYMBOLTYPE_BRUSHITEM )
        {   //#i31097# Data Point Symbol size changes when a different symbol is choosen(maoyg)
            if( aSymbolSize.Width() != aSymbolSize.Height() )
            {
                aSize.setWidth( (long)( aSymbolSize.Width() + aSymbolSize.Height() )/2 );
                aSize.setHeight( (long)( aSymbolSize.Width() + aSymbolSize.Height() )/2 );
                aSymbolSize = aSize;
            }
        }
        m_pCtlPreview->SetSymbol(&aSymbolGraphic,aSymbolSize);
    }
    else
    {
        aSymbolGraphic=Graphic();
        m_pCtlPreview->SetSymbol(NULL,aSymbolSize);
        bEnable = false;
    }
    aSymbolLastSize=aSymbolSize;
    SetMetricValue(*m_pSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
    SetMetricValue(*m_pSymbolHeightMF, aSymbolSize.Height(), ePoolUnit);

    m_pGridIconSize->Enable(bEnable);
    m_pCtlPreview->Invalidate();
}

void SvxLineTabPage::FillListboxes()
{
    // Line styles
    sal_uInt16 nOldSelect = m_pLbLineStyle->GetSelectEntryPos();
    // aLbLineStyle.FillStyles();
    m_pLbLineStyle->Fill( pDashList );
    m_pLbLineStyle->SelectEntryPos( nOldSelect );

    // Line end style
    OUString sNone( SVX_RES( RID_SVXSTR_NONE ) );
    nOldSelect = m_pLbStartStyle->GetSelectEntryPos();
    m_pLbStartStyle->Clear();
    m_pLbStartStyle->InsertEntry( sNone );
    m_pLbStartStyle->Fill( pLineEndList );
    m_pLbStartStyle->SelectEntryPos( nOldSelect );
    nOldSelect = m_pLbEndStyle->GetSelectEntryPos();
    m_pLbEndStyle->Clear();
    m_pLbEndStyle->InsertEntry( sNone );
    m_pLbEndStyle->Fill( pLineEndList, sal_False );
    m_pLbEndStyle->SelectEntryPos( nOldSelect );
}

// -----------------------------------------------------------------------

void SvxLineTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,CntUInt16Item,SID_PAGE_TYPE,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if( nDlgType == 0 && pDashList.is() )
    {
        sal_uInt16 nPos;
        sal_uInt16 nCount;

        // Dash list
        if( ( *pnDashListState & CT_MODIFIED ) ||
            ( *pnDashListState & CT_CHANGED ) )
        {
            if( *pnDashListState & CT_CHANGED )
                pDashList = ( (SvxLineTabDialog*) GetParentDialog() )->GetNewDashList();

            *pnDashListState = CT_NONE;

            // Style list
            nPos = m_pLbLineStyle->GetSelectEntryPos();

            m_pLbLineStyle->Clear();
            m_pLbLineStyle->InsertEntry( SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );
            m_pLbLineStyle->InsertEntry( SVX_RESSTR( RID_SVXSTR_SOLID ) );
            m_pLbLineStyle->Fill( pDashList );
            nCount = m_pLbLineStyle->GetEntryCount();

            if ( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbLineStyle->SelectEntryPos( 0 );
            else
                m_pLbLineStyle->SelectEntryPos( nPos );
            // SelectStyleHdl_Impl( this );
        }

        INetURLObject   aDashURL( pDashList->GetPath() );

        aDashURL.Append( pDashList->GetName() );
        DBG_ASSERT( aDashURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        // LineEnd list
        if( ( *pnLineEndListState & CT_MODIFIED ) || ( *pnLineEndListState & CT_CHANGED ) )
        {
            if( *pnLineEndListState & CT_CHANGED )
                pLineEndList = ( (SvxLineTabDialog*) GetParentDialog() )->GetNewLineEndList();

            *pnLineEndListState = CT_NONE;

            nPos = m_pLbLineStyle->GetSelectEntryPos();
            OUString sNone( SVX_RES( RID_SVXSTR_NONE ) );
            m_pLbStartStyle->Clear();
            m_pLbStartStyle->InsertEntry( sNone );

            m_pLbStartStyle->Fill( pLineEndList );
            nCount = m_pLbStartStyle->GetEntryCount();
            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbStartStyle->SelectEntryPos( 0 );
            else
                m_pLbStartStyle->SelectEntryPos( nPos );

            m_pLbEndStyle->Clear();
            m_pLbEndStyle->InsertEntry( sNone );

            m_pLbEndStyle->Fill( pLineEndList, sal_False );
            nCount = m_pLbEndStyle->GetEntryCount();

            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbEndStyle->SelectEntryPos( 0 );
            else
                m_pLbEndStyle->SelectEntryPos( nPos );
        }
        INetURLObject aLineURL( pLineEndList->GetPath() );

        aLineURL.Append( pLineEndList->GetName() );
        DBG_ASSERT( aLineURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        // Evaluate if another TabPage set another fill type
        if( m_pLbLineStyle->GetSelectEntryPos() != 0 )
        {
            if( nPageType == 2 ) // 1
            {
                m_pLbLineStyle->SelectEntryPos( *pPosDashLb + 2 ); // +2 due to SOLID and INVLISIBLE
                ChangePreviewHdl_Impl( this );
            }
            if( nPageType == 3 )
            {
                m_pLbStartStyle->SelectEntryPos( *pPosLineEndLb + 1 );// +1 due to SOLID
                m_pLbEndStyle->SelectEntryPos( *pPosLineEndLb + 1 );// +1 due to SOLID
                ChangePreviewHdl_Impl( this );
            }
        }

            // ColorList
            if( *pnColorListState )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxLineTabDialog*) GetParentDialog() )->GetNewColorList();
                // aLbColor
                sal_uInt16 nColorPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( pColorList );
                nCount = m_pLbColor->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nColorPos )
                    m_pLbColor->SelectEntryPos( 0 );
                else
                    m_pLbColor->SelectEntryPos( nColorPos );

                ChangePreviewHdl_Impl( this );
            }

        nPageType = 0;
    }
    // Page does not yet exist in the ctor, that's why we do it here!

    else if ( nDlgType == 1100 ||
              nDlgType == 1101 )
    {
        m_pFlLineEnds->Hide();
        // #116827#
        m_pFLEdgeStyle->Hide();
    }
}

// -----------------------------------------------------------------------

int SvxLineTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( nDlgType == 0 ) // Line dialog
    {
        nPageType = 1; // possibly for extensions
        *pPosDashLb = m_pLbLineStyle->GetSelectEntryPos() - 2;// First entry SOLID!!!
        sal_uInt16 nPos = m_pLbStartStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            nPos--;
        *pPosLineEndLb = nPos;
    }

    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

sal_Bool SvxLineTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem* pOld = NULL;
    sal_uInt16  nPos;
    sal_Bool    bModified = sal_False;

    // To prevent modifications to the list, we do not set other page's items.
    if( nDlgType != 0 || nPageType != 2 )
    {
        nPos = m_pLbLineStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != m_pLbLineStyle->GetSavedValue() )
        {
            XLineStyleItem* pStyleItem = NULL;

            if( nPos == 0 )
                pStyleItem = new XLineStyleItem( XLINE_NONE );
            else if( nPos == 1 )
                pStyleItem = new XLineStyleItem( XLINE_SOLID );
            else
            {
                pStyleItem = new XLineStyleItem( XLINE_DASH );

                // For added security
                if( pDashList->Count() > (long) ( nPos - 2 ) )
                {
                    XLineDashItem aDashItem( m_pLbLineStyle->GetSelectEntry(),
                                        pDashList->GetDash( nPos - 2 )->GetDash() );
                    pOld = GetOldItem( rAttrs, XATTR_LINEDASH );
                    if ( !pOld || !( *(const XLineDashItem*)pOld == aDashItem ) )
                    {
                        rAttrs.Put( aDashItem );
                        bModified = sal_True;
                    }
                }
            }
            pOld = GetOldItem( rAttrs, XATTR_LINESTYLE );
            if ( !pOld || !( *(const XLineStyleItem*)pOld == *pStyleItem ) )
            {
                rAttrs.Put( *pStyleItem );
                bModified = sal_True;
            }
            delete pStyleItem;
        }
    }
    // Line width
    // GetSavedValue() returns OUString!
    if( m_pMtrLineWidth->GetText() != m_pMtrLineWidth->GetSavedValue() )
    {
        XLineWidthItem aItem( GetCoreValue( *m_pMtrLineWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEWIDTH );
        if ( !pOld || !( *(const XLineWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    // Width line start
    if( m_pMtrStartWidth->GetText() != m_pMtrStartWidth->GetSavedValue() )
    {
        XLineStartWidthItem aItem( GetCoreValue( *m_pMtrStartWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *(const XLineStartWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    // Width line end
    if( m_pMtrEndWidth->GetText() != m_pMtrEndWidth->GetSavedValue() )
    {
        XLineEndWidthItem aItem( GetCoreValue( *m_pMtrEndWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *(const XLineEndWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }

    // Line color
    if( m_pLbColor->GetSelectEntryPos() != m_pLbColor->GetSavedValue() )
    {
        XLineColorItem aItem( m_pLbColor->GetSelectEntry(), m_pLbColor->GetSelectEntryColor() );
        pOld = GetOldItem( rAttrs, XATTR_LINECOLOR );
        if ( !pOld || !( *(const XLineColorItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }

    if( nDlgType != 0 || nPageType != 3 )
    {
        // Line start
        nPos = m_pLbStartStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND && nPos != m_pLbStartStyle->GetSavedValue() )
        {
            XLineStartItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineStartItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineStartItem( m_pLbStartStyle->GetSelectEntry(), pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
            pOld = GetOldItem( rAttrs, XATTR_LINESTART );
            if( pItem && ( !pOld || !( *(const XLineEndItem*)pOld == *pItem ) ) )
            {
                rAttrs.Put( *pItem );
                bModified = sal_True;
            }
            delete pItem;
        }
        // Line end
        nPos = m_pLbEndStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&  nPos != m_pLbEndStyle->GetSavedValue() )
        {
            XLineEndItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineEndItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineEndItem( m_pLbEndStyle->GetSelectEntry(), pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
            pOld = GetOldItem( rAttrs, XATTR_LINEEND );
            if( pItem &&
                ( !pOld || !( *(const XLineEndItem*)pOld == *pItem ) ) )
            {
                rAttrs.Put( *pItem );
                bModified = sal_True;
            }
            delete pItem;
        }
    }

    // Centered line end
    TriState eState = m_pTsbCenterStart->GetState();
    if( eState != m_pTsbCenterStart->GetSavedValue() )
    {
        XLineStartCenterItem aItem( sal::static_int_cast< sal_Bool >( eState ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTCENTER );
        if ( !pOld || !( *(const XLineStartCenterItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    eState = m_pTsbCenterEnd->GetState();
    if( eState != m_pTsbCenterEnd->GetSavedValue() )
    {
        XLineEndCenterItem aItem( sal::static_int_cast< sal_Bool >( eState ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDCENTER );
        if ( !pOld || !( *(const XLineEndCenterItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }

    // Transparency
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    if( nVal != (sal_uInt16)m_pMtrTransparent->GetSavedValue().toInt32() )
    {
        XLineTransparenceItem aItem( nVal );
        pOld = GetOldItem( rAttrs, XATTR_LINETRANSPARENCE );
        if ( !pOld || !( *(const XLineTransparenceItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }

    // #116827#
    nPos = m_pLBEdgeStyle->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && nPos != m_pLBEdgeStyle->GetSavedValue() )
    {
        XLineJointItem* pNew = 0L;

        switch(nPos)
        {
            case 0: // Rounded, default
            {
                pNew = new XLineJointItem(com::sun::star::drawing::LineJoint_ROUND);
                break;
            }
            case 1: // - none -
            {
                pNew = new XLineJointItem(com::sun::star::drawing::LineJoint_NONE);
                break;
            }
            case 2: // Miter
            {
                pNew = new XLineJointItem(com::sun::star::drawing::LineJoint_MITER);
                break;
            }
            case 3: // Bevel
            {
                pNew = new XLineJointItem(com::sun::star::drawing::LineJoint_BEVEL);
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( rAttrs, XATTR_LINEJOINT );

            if(!pOld || !(*(const XLineJointItem*)pOld == *pNew))
            {
                rAttrs.Put( *pNew );
                bModified = sal_True;
            }

            delete pNew;
        }
    }

    // LineCaps
    nPos = m_pLBCapStyle->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && nPos != m_pLBCapStyle->GetSavedValue() )
    {
        XLineCapItem* pNew = 0L;

        switch(nPos)
        {
            case 0: // Butt (=Flat), default
            {
                pNew = new XLineCapItem(com::sun::star::drawing::LineCap_BUTT);
                break;
            }
            case 1: // Round
            {
                pNew = new XLineCapItem(com::sun::star::drawing::LineCap_ROUND);
                break;
            }
            case 2: // Square
            {
                pNew = new XLineCapItem(com::sun::star::drawing::LineCap_SQUARE);
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( rAttrs, XATTR_LINECAP );

            if(!pOld || !(*(const XLineCapItem*)pOld == *pNew))
            {
                rAttrs.Put( *pNew );
                bModified = sal_True;
            }

            delete pNew;
        }
    }

    if(nSymbolType!=SVX_SYMBOLTYPE_UNKNOWN || bNewSize)
    {
        // Was set by selection or the size is different
        SvxSizeItem  aSItem(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),aSymbolSize);
        const SfxPoolItem* pSOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE) );
        bNewSize  = pSOld ? *(const SvxSizeItem *)pSOld != aSItem : bNewSize ;
        if(bNewSize)
        {
            rAttrs.Put(aSItem);
            bModified=sal_True;
        }

        SfxInt32Item aTItem(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),nSymbolType);
        const SfxPoolItem* pTOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE) );
        bool bNewType = pTOld == NULL || *(const SfxInt32Item*)pTOld != aTItem;
        if(bNewType && nSymbolType==SVX_SYMBOLTYPE_UNKNOWN)
            bNewType=false; // a small fix, type wasn't set -> don't create a type item after all!
        if(bNewType)
        {
            rAttrs.Put(aTItem);
            bModified=sal_True;
        }

        if(nSymbolType!=SVX_SYMBOLTYPE_NONE)
        {
            SvxBrushItem aBItem(aSymbolGraphic,GPOS_MM,rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH));
            const SfxPoolItem* pBOld = GetOldItem( rAttrs, rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH) );
            bool bNewBrush =
                pBOld == NULL || *(const SvxBrushItem*)pBOld != aBItem;
            if(bNewBrush)
            {
                rAttrs.Put(aBItem);
                bModified=sal_True;
            }
        }
    }
    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));
    return( bModified );
}

// -----------------------------------------------------------------------

sal_Bool SvxLineTabPage::FillXLSet_Impl()
{
    sal_uInt16 nPos;

    if( m_pLbLineStyle->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    }
    else if( m_pLbLineStyle->IsEntryPosSelected( 0 ) )
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    else if( m_pLbLineStyle->IsEntryPosSelected( 1 ) )
        rXLSet.Put( XLineStyleItem( XLINE_SOLID ) );
    else
    {
        rXLSet.Put( XLineStyleItem( XLINE_DASH ) );

        nPos = m_pLbLineStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rXLSet.Put( XLineDashItem( m_pLbLineStyle->GetSelectEntry(),
                            pDashList->GetDash( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = m_pLbStartStyle->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineStartItem() );
        else
            rXLSet.Put( XLineStartItem( m_pLbStartStyle->GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = m_pLbEndStyle->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineEndItem() );
        else
            rXLSet.Put( XLineEndItem( m_pLbEndStyle->GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }

    // #116827#
    nPos = m_pLBEdgeStyle->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        switch(nPos)
        {
            case 0: // Rounded, default
            {
                rXLSet.Put(XLineJointItem(com::sun::star::drawing::LineJoint_ROUND));
                break;
            }
            case 1: // - none -
            {
                rXLSet.Put(XLineJointItem(com::sun::star::drawing::LineJoint_NONE));
                break;
            }
            case 2: // Miter
            {
                rXLSet.Put(XLineJointItem(com::sun::star::drawing::LineJoint_MITER));
                break;
            }
            case 3: // Bevel
            {
                rXLSet.Put(XLineJointItem(com::sun::star::drawing::LineJoint_BEVEL));
                break;
            }
        }
    }

    // LineCaps
    nPos = m_pLBCapStyle->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        switch(nPos)
        {
            case 0: // Butt (=Flat), default
            {
                rXLSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_BUTT));
                break;
            }
            case 1: // Round
            {
                rXLSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_ROUND));
                break;
            }
            case 2: // Square
            {
                rXLSet.Put(XLineCapItem(com::sun::star::drawing::LineCap_SQUARE));
                break;
            }
        }
    }

    rXLSet.Put( XLineStartWidthItem( GetCoreValue( *m_pMtrStartWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineEndWidthItem( GetCoreValue( *m_pMtrEndWidth, ePoolUnit ) ) );

    rXLSet.Put( XLineWidthItem( GetCoreValue( *m_pMtrLineWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineColorItem( m_pLbColor->GetSelectEntry(), m_pLbColor->GetSelectEntryColor() ) );

    // Centered line end
    if( m_pTsbCenterStart->GetState() == STATE_CHECK )
        rXLSet.Put( XLineStartCenterItem( sal_True ) );
    else if( m_pTsbCenterStart->GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineStartCenterItem( sal_False ) );

    if( m_pTsbCenterEnd->GetState() == STATE_CHECK )
        rXLSet.Put( XLineEndCenterItem( sal_True ) );
    else if( m_pTsbCenterEnd->GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineEndCenterItem( sal_False ) );

    // Transparency
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    rXLSet.Put( XLineTransparenceItem( nVal ) );

    // #116827#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

    return( sal_True );
}

// -----------------------------------------------------------------------

void SvxLineTabPage::Reset( const SfxItemSet& rAttrs )
{
    XLineStyle  eXLS; // XLINE_NONE, XLINE_SOLID, XLINE_DASH

    // Line style
    const SfxPoolItem *pPoolItem;
    long nSymType=SVX_SYMBOLTYPE_UNKNOWN;
    sal_Bool bPrevSym=sal_False;
    sal_Bool bEnable=sal_True;
    sal_Bool bIgnoreGraphic=sal_False;
    sal_Bool bIgnoreSize=sal_False;
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),sal_True,&pPoolItem) == SFX_ITEM_SET)
    {
        nSymType=((const SfxInt32Item *)pPoolItem)->GetValue();
    }

    if(nSymType == SVX_SYMBOLTYPE_AUTO)
    {
        aSymbolGraphic=aAutoSymbolGraphic;
        aSymbolSize=aSymbolLastSize=aAutoSymbolGraphic.GetPrefSize();
        bPrevSym=sal_True;
    }
    else if(nSymType == SVX_SYMBOLTYPE_NONE)
    {
        bEnable=sal_False;
        bIgnoreGraphic=sal_True;
        bIgnoreSize=sal_True;
    }
    else if(nSymType >= 0)
    {
        VirtualDevice aVDev;
        aVDev.SetMapMode(MapMode(MAP_100TH_MM));

        SdrModel* pModel = new SdrModel(NULL, NULL, LOADREFCOUNTS);
        pModel->GetItemPool().FreezeIdRanges();
        SdrPage* pPage = new SdrPage( *pModel, sal_False );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        SdrView* pView = new SdrView( pModel, &aVDev );
        pView->hideMarkHandles();
        pView->ShowSdrPage(pPage);
        SdrObject *pObj=NULL;
        long nSymTmp=nSymType;
        if(pSymbolList)
        {
            if(pSymbolList->GetObjCount())
            {
                nSymTmp=nSymTmp%pSymbolList->GetObjCount(); // Treat list as cyclic!
                pObj=pSymbolList->GetObj(nSymTmp);
                if(pObj)
                {
                    pObj=pObj->Clone();
                    if(pSymbolAttr)
                    {
                        pObj->SetMergedItemSet(*pSymbolAttr);
                    }
                    else
                    {
                        pObj->SetMergedItemSet(rOutAttrs);
                    }

                    pPage->NbcInsertObject(pObj);

                    // Generate invisible square to give all symbol types a
                    // bitmap size, which is independent from specific glyph
                    SdrObject *pInvisibleSquare=pSymbolList->GetObj(0);
                    pInvisibleSquare=pInvisibleSquare->Clone();
                    pPage->NbcInsertObject(pInvisibleSquare);
                    pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
                    pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

                    pView->MarkAll();
                    GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());

                    aSymbolGraphic=Graphic(aMeta);
                    aSymbolSize=pObj->GetSnapRect().GetSize();
                    aSymbolGraphic.SetPrefSize(pInvisibleSquare->GetSnapRect().GetSize());
                    aSymbolGraphic.SetPrefMapMode(MAP_100TH_MM);
                    bPrevSym=sal_True;
                    bEnable=sal_True;
                    bIgnoreGraphic=sal_True;

                    pView->UnmarkAll();
                    pInvisibleSquare=pPage->RemoveObject(1);
                    SdrObject::Free( pInvisibleSquare);
                    pObj=pPage->RemoveObject(0);
                    SdrObject::Free( pObj );
                }
            }
        }
        delete pView;
        delete pModel;
    }
    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_BRUSH),sal_True,&pPoolItem) == SFX_ITEM_SET)
    {
        const Graphic* pGraphic = ((const SvxBrushItem *)pPoolItem)->GetGraphic();
        if( pGraphic )
        {
            if(!bIgnoreGraphic)
            {
                aSymbolGraphic=*pGraphic;
            }
            if(!bIgnoreSize)
            {
                aSymbolSize=OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                                        pGraphic->GetPrefMapMode(),
                                                        MAP_100TH_MM );
            }
            bPrevSym=sal_True;
        }
    }

    if(rAttrs.GetItemState(rAttrs.GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),sal_True,&pPoolItem) == SFX_ITEM_SET)
    {
        aSymbolSize = ((const SvxSizeItem *)pPoolItem)->GetSize();
    }

    m_pGridIconSize->Enable(bEnable);

    if(bPrevSym)
    {
        SetMetricValue(*m_pSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
        SetMetricValue(*m_pSymbolHeightMF, aSymbolSize.Height(),ePoolUnit);
        m_pCtlPreview->SetSymbol(&aSymbolGraphic,aSymbolSize);
        aSymbolLastSize=aSymbolSize;
    }

    if( rAttrs.GetItemState( XATTR_LINESTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXLS = (XLineStyle) ( ( const XLineStyleItem& ) rAttrs.Get( XATTR_LINESTYLE ) ).GetValue();

        switch( eXLS )
        {
            case XLINE_NONE:
                m_pLbLineStyle->SelectEntryPos( 0 );
                break;
            case XLINE_SOLID:
                m_pLbLineStyle->SelectEntryPos( 1 );
                break;

            case XLINE_DASH:
                m_pLbLineStyle->SetNoSelection();
                m_pLbLineStyle->SelectEntry( ( ( const XLineDashItem& ) rAttrs.Get( XATTR_LINEDASH ) ).GetName() );
                break;

            default:
                break;
        }
    }
    else
    {
        m_pLbLineStyle->SetNoSelection();
    }

    // Line strength
    if( rAttrs.GetItemState( XATTR_LINEWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( *m_pMtrLineWidth, ( ( const XLineWidthItem& ) rAttrs.Get( XATTR_LINEWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        m_pMtrLineWidth->SetText( "" );

    // Line color
    m_pLbColor->SetNoSelection();

    if ( rAttrs.GetItemState( XATTR_LINECOLOR ) != SFX_ITEM_DONTCARE )
    {
        Color aCol = ( ( const XLineColorItem& ) rAttrs.Get( XATTR_LINECOLOR ) ).GetColorValue();
        m_pLbColor->SelectEntry( aCol );
        if( m_pLbColor->GetSelectEntryCount() == 0 )
        {
            m_pLbColor->InsertEntry( aCol, OUString() );
            m_pLbColor->SelectEntry( aCol );
        }
    }

    // Line start
    if( bObjSelected && rAttrs.GetItemState( XATTR_LINESTART ) == SFX_ITEM_DEFAULT )
    {
        m_pLbStartStyle->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTART ) != SFX_ITEM_DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        sal_Bool bSelected(sal_False);
        const basegfx::B2DPolyPolygon& rItemPolygon = ((const XLineStartItem&)rAttrs.Get(XATTR_LINESTART)).GetLineStartValue();

        for(sal_Int32 a(0);!bSelected &&  a < pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_pLbStartStyle->SelectEntryPos((sal_uInt16)a + 1);
                bSelected = sal_True;
            }
        }

        if(!bSelected)
            m_pLbStartStyle->SelectEntryPos( 0 );
    }
    else
    {
        m_pLbStartStyle->SetNoSelection();
    }

    // Line end
    if( bObjSelected && rAttrs.GetItemState( XATTR_LINEEND ) == SFX_ITEM_DEFAULT )
    {
        m_pLbEndStyle->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEEND ) != SFX_ITEM_DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        sal_Bool bSelected(sal_False);
        const basegfx::B2DPolyPolygon& rItemPolygon = ((const XLineEndItem&)rAttrs.Get(XATTR_LINEEND)).GetLineEndValue();

        for(sal_Int32 a(0);!bSelected &&  a < pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_pLbEndStyle->SelectEntryPos((sal_uInt16)a + 1);
                bSelected = sal_True;
            }
        }

        if(!bSelected)
            m_pLbEndStyle->SelectEntryPos( 0 );
    }
    else
    {
        m_pLbEndStyle->SetNoSelection();
    }

    // Line start strength
    if( bObjSelected &&  rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) == SFX_ITEM_DEFAULT )
    {
        m_pMtrStartWidth->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( *m_pMtrStartWidth,
                        ( ( const XLineStartWidthItem& ) rAttrs.Get( XATTR_LINESTARTWIDTH ) ).GetValue(),
                        ePoolUnit );
    }
    else
        m_pMtrStartWidth->SetText( "" );

    // Line end strength
    if( bObjSelected && rAttrs.GetItemState( XATTR_LINEENDWIDTH ) == SFX_ITEM_DEFAULT )
    {
        m_pMtrEndWidth->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( *m_pMtrEndWidth,
                        ( ( const XLineEndWidthItem& ) rAttrs.Get( XATTR_LINEENDWIDTH ) ).GetValue(),
                        ePoolUnit );
    }
    else
        m_pMtrEndWidth->SetText( "" );

    // Centered line end (start)
    if( bObjSelected && rAttrs.GetItemState( XATTR_LINESTARTCENTER ) == SFX_ITEM_DEFAULT )
    {
        m_pTsbCenterStart->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTCENTER ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbCenterStart->EnableTriState( sal_False );

        if( ( ( const XLineStartCenterItem& ) rAttrs.Get( XATTR_LINESTARTCENTER ) ).GetValue() )
            m_pTsbCenterStart->SetState( STATE_CHECK );
        else
            m_pTsbCenterStart->SetState( STATE_NOCHECK );
    }
    else
    {
        m_pTsbCenterStart->SetState( STATE_DONTKNOW );
    }

    // Centered line end (end)
    if( bObjSelected && rAttrs.GetItemState( XATTR_LINEENDCENTER ) == SFX_ITEM_DEFAULT )
    {
        m_pTsbCenterEnd->Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDCENTER ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbCenterEnd->EnableTriState( sal_False );

        if( ( ( const XLineEndCenterItem& ) rAttrs.Get( XATTR_LINEENDCENTER ) ).GetValue() )
            m_pTsbCenterEnd->SetState( STATE_CHECK );
        else
            m_pTsbCenterEnd->SetState( STATE_NOCHECK );
    }
    else
    {
        m_pTsbCenterEnd->SetState( STATE_DONTKNOW );
    }

    // Transparency
    if( rAttrs.GetItemState( XATTR_LINETRANSPARENCE ) != SFX_ITEM_DONTCARE )
    {
        sal_uInt16 nTransp = ( ( const XLineTransparenceItem& ) rAttrs.Get( XATTR_LINETRANSPARENCE ) ).GetValue();
        m_pMtrTransparent->SetValue( nTransp );
        ChangeTransparentHdl_Impl( NULL );
    }
    else
        m_pMtrTransparent->SetText( "" );

    if( !m_pLbStartStyle->IsEnabled()  &&
        !m_pLbEndStyle->IsEnabled()    &&
        !m_pMtrStartWidth->IsEnabled() &&
        !m_pMtrEndWidth->IsEnabled()   &&
        !m_pTsbCenterStart->IsEnabled()&&
        !m_pTsbCenterEnd->IsEnabled() )
    {
        m_pCbxSynchronize->Disable();
        m_pFlLineEnds->Disable();
    }

    // Synchronize
    // We get the value from the INI file now
    OUString aStr = GetUserData();
    m_pCbxSynchronize->Check( (sal_Bool)aStr.toInt32() );

    // #116827#
    if(bObjSelected && SFX_ITEM_DEFAULT == rAttrs.GetItemState(XATTR_LINEJOINT))
    {
//         maFTEdgeStyle.Disable();
        m_pLBEdgeStyle->Disable();
    }
    else if(SFX_ITEM_DONTCARE != rAttrs.GetItemState(XATTR_LINEJOINT))
    {
        const com::sun::star::drawing::LineJoint eLineJoint = ((const XLineJointItem&)(rAttrs.Get(XATTR_LINEJOINT))).GetValue();

        switch(eLineJoint)
        {
            case com::sun::star::drawing::LineJoint_MAKE_FIXED_SIZE: // fallback to round, unused value
            case com::sun::star::drawing::LineJoint_MIDDLE : // fallback to round, unused value
            case com::sun::star::drawing::LineJoint_ROUND : m_pLBEdgeStyle->SelectEntryPos(0); break;
            case com::sun::star::drawing::LineJoint_NONE : m_pLBEdgeStyle->SelectEntryPos(1); break;
            case com::sun::star::drawing::LineJoint_MITER : m_pLBEdgeStyle->SelectEntryPos(2); break;
            case com::sun::star::drawing::LineJoint_BEVEL : m_pLBEdgeStyle->SelectEntryPos(3); break;
        }
    }
    else
    {
        m_pLBEdgeStyle->SetNoSelection();
    }

    // fdo#43209
    if(bObjSelected && SFX_ITEM_DEFAULT == rAttrs.GetItemState(XATTR_LINECAP))
    {
//         maFTCapStyle.Disable();
        m_pLBCapStyle->Disable();
    }
    else if(SFX_ITEM_DONTCARE != rAttrs.GetItemState(XATTR_LINECAP))
    {
        const com::sun::star::drawing::LineCap eLineCap(((const XLineCapItem&)(rAttrs.Get(XATTR_LINECAP))).GetValue());

        switch(eLineCap)
        {
            case com::sun::star::drawing::LineCap_ROUND: m_pLBCapStyle->SelectEntryPos(1); break;
            case com::sun::star::drawing::LineCap_SQUARE : m_pLBCapStyle->SelectEntryPos(2); break;
            default /*com::sun::star::drawing::LineCap_BUTT*/: m_pLBCapStyle->SelectEntryPos(0); break;
        }
    }
    else
    {
        m_pLBCapStyle->SetNoSelection();
    }

    // Save values
    m_pLbLineStyle->SaveValue();
    m_pMtrLineWidth->SaveValue();
    m_pLbColor->SaveValue();
    m_pLbStartStyle->SaveValue();
    m_pLbEndStyle->SaveValue();
    m_pMtrStartWidth->SaveValue();
    m_pMtrEndWidth->SaveValue();
    m_pTsbCenterStart->SaveValue();
    m_pTsbCenterEnd->SaveValue();
    m_pMtrTransparent->SaveValue();

    // #116827#
    m_pLBEdgeStyle->SaveValue();

    // LineCaps
    m_pLBCapStyle->SaveValue();

    ClickInvisibleHdl_Impl( this );

    ChangePreviewHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxLineTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxLineTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

sal_uInt16* SvxLineTabPage::GetRanges()
{
    return pLineRanges;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangePreviewHdl_Impl, void *, pCntrl )
{
    if(pCntrl == m_pMtrLineWidth)
    {
        // Line width and start end width
        sal_Int32 nNewLineWidth = GetCoreValue( *m_pMtrLineWidth, ePoolUnit );
        if(nActLineWidth == -1)
        {
            // Don't initialize yet, get the start value
            const SfxPoolItem* pOld = GetOldItem( rXLSet, XATTR_LINEWIDTH );
            sal_Int32 nStartLineWidth = 0;
            if(pOld)
                nStartLineWidth = (( const XLineWidthItem *)pOld)->GetValue();
            nActLineWidth = nStartLineWidth;
        }

        if(nActLineWidth != nNewLineWidth)
        {
            // Adapt start/end width
            sal_Int32 nValAct = GetCoreValue( *m_pMtrStartWidth, ePoolUnit );
            sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_pMtrStartWidth, nValNew, ePoolUnit );

            nValAct = GetCoreValue( *m_pMtrEndWidth, ePoolUnit );
            nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_pMtrEndWidth, nValNew, ePoolUnit );
        }

        // Remember current value
        nActLineWidth = nNewLineWidth;
    }

    FillXLSet_Impl();
    m_pCtlPreview->Invalidate();

    // Make transparency accessible accordingly
    if( m_pLbLineStyle->GetSelectEntryPos() == 0 ) // invisible
    {
//         aFtTransparent.Disable();
        m_pMtrTransparent->Disable();
    }
    else
    {
//         aFtTransparent.Enable();
        m_pMtrTransparent->Enable();
    }

    const bool bHasLineStyle = m_pLbLineStyle->GetSelectEntryPos() !=0;
    const bool bHasLineStart = m_pLbStartStyle->GetSelectEntryPos() != 0;

    m_pBoxStart->Enable(bHasLineStart && bHasLineStyle);

    const bool bHasLineEnd = m_pLbEndStyle->GetSelectEntryPos() != 0;

    m_pBoxEnd->Enable(bHasLineEnd && bHasLineStyle);

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeStartHdl_Impl, void *, p )
{
    if( m_pCbxSynchronize->IsChecked() )
    {
        if( p == m_pMtrStartWidth )
            m_pMtrEndWidth->SetValue( m_pMtrStartWidth->GetValue() );
        if( p == m_pLbStartStyle )
            m_pLbEndStyle->SelectEntryPos( m_pLbStartStyle->GetSelectEntryPos() );
        if( p == m_pTsbCenterStart )
            m_pTsbCenterEnd->SetState( m_pTsbCenterStart->GetState() );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------
// #116827#

IMPL_LINK_NOARG(SvxLineTabPage, ChangeEdgeStyleHdl_Impl)
{
    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------
// fdo#43209

IMPL_LINK( SvxLineTabPage, ChangeCapStyleHdl_Impl, void *, EMPTYARG )
{
    ChangePreviewHdl_Impl( this );

    return( 0L );
}
//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineTabPage, ClickInvisibleHdl_Impl)
{
    if( m_pLbLineStyle->GetSelectEntryPos() == 0 ) // invisible
    {
        if(!bSymbols)
            m_pLbColor->Disable();

        m_pBoxStyle->Disable();

        if( m_pFlLineEnds->IsEnabled() )
        {
            m_pBoxStart->Disable();
            m_pBoxArrowStyles->Disable();


            // #116827#
            m_pGridEdgeCaps->Disable();
        }
    }
    else
    {
        m_pBoxStyle->Enable();

        if( m_pFlLineEnds->IsEnabled() )
        {
            m_pBoxArrowStyles->Enable();
            // #116827#
            m_pGridEdgeCaps->Enable();
        }
    }
    ChangePreviewHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeEndHdl_Impl, void *, p )
{
    if( m_pCbxSynchronize->IsChecked() )
    {
        if( p == m_pMtrEndWidth )
            m_pMtrStartWidth->SetValue( m_pMtrEndWidth->GetValue() );
        if( p == m_pLbEndStyle )
            m_pLbStartStyle->SelectEntryPos( m_pLbEndStyle->GetSelectEntryPos() );
        if( p == m_pTsbCenterEnd )
            m_pTsbCenterStart->SetState( m_pTsbCenterEnd->GetState() );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineTabPage, ChangeTransparentHdl_Impl)
{
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    XLineTransparenceItem aItem( nVal );

    rXLSet.Put( XLineTransparenceItem( aItem ) );

    // #116827#
    FillXLSet_Impl();

    m_pCtlPreview->Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxLineTabPage::PointChanged( Window*, RECT_POINT eRcPt )
{
    eRP = eRcPt;
}

//------------------------------------------------------------------------

void SvxLineTabPage::FillUserData()
{
    // Write the synched value to the INI file
//     OUString aStrUserData = OUString::valueOf( (sal_Int32) m_pCbxSynchronize->IsChecked() );
    OUString aStrUserData = OUString::boolean(m_pCbxSynchronize->IsChecked());
    SetUserData( aStrUserData );
}


// #58425# Symbols on a list (e.g. StarChart)
// Handler for the symbol selection's popup menu (NumMenueButton)
// The following link originates from SvxNumOptionsTabPage
IMPL_LINK( SvxLineTabPage, MenuCreateHdl_Impl, MenuButton *, pButton )
{
    InitSymbols(pButton);
    return 0;
}
// #58425# Symbols on a list (e.g. StarChart)
// Handler for the symbol selection's popup menu (NumMenueButton)
// The following link originates from SvxNumOptionsTabPage
IMPL_STATIC_LINK(SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem*, pItem)
{
    PopupMenu* pPopup = pThis->m_pSymbolMB->GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    SvxBmpItemInfo* pBmpInfo = 0;
    for ( size_t i = 0; i < pThis->aGrfBrushItems.size(); i++ )
    {
        SvxBmpItemInfo* pInfo = pThis->aGrfBrushItems[ i ];
        if( pInfo->pBrushItem == pItem )
        {
            pBmpInfo = pInfo; break;
        }
    }
    if( pBmpInfo )
    {
        if( pItem->GetGraphic() )
        {
            Bitmap aBitmap(pItem->GetGraphic()->GetBitmap());
            Size aSize(aBitmap.GetSizePixel());
            if(aSize.Width()  > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
            {
                sal_Bool bWidth = aSize.Width() > aSize.Height();
                double nScale = bWidth ?
                    (double)MAX_BMP_WIDTH / (double)aSize.Width():
                    (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                aBitmap.Scale(nScale, nScale);
            }
            Image aImage(aBitmap);
            pPopup->SetItemImage( pBmpInfo->nItemId, aImage );
        }
    }

    return 0;
}

// #58425# Symbols on a list (e.g. StarChart)
// Handler for menu button
IMPL_LINK( SvxLineTabPage, GraphicHdl_Impl, MenuButton *, pButton )
{
    SymbolSelected(pButton);
    return 0;
}
IMPL_LINK( SvxLineTabPage, SizeHdl_Impl, MetricField *, pField)
{
    bNewSize=true;
    sal_Bool bWidth = (sal_Bool)(pField == m_pSymbolWidthMF);
    bLastWidthModified = bWidth;
    sal_Bool bRatio = m_pSymbolRatioCB->IsChecked();
    long nWidthVal = static_cast<long>(m_pSymbolWidthMF->Denormalize(m_pSymbolWidthMF->GetValue(FUNIT_100TH_MM)));
    long nHeightVal= static_cast<long>(m_pSymbolHeightMF->Denormalize(m_pSymbolHeightMF->GetValue(FUNIT_100TH_MM)));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal,MAP_100TH_MM,(MapUnit)ePoolUnit );
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal,MAP_100TH_MM,(MapUnit)ePoolUnit);
    aSymbolSize=Size(nWidthVal,nHeightVal);
    double  fSizeRatio = (double)1;

    if(bRatio)
    {
        if (aSymbolLastSize.Height() && aSymbolLastSize.Width())
            fSizeRatio = (double)aSymbolLastSize.Width() / aSymbolLastSize.Height();
    }

    //Size aSymbolSize(aSymbolLastSize);

    if(bWidth)
    {
        long nDelta = nWidthVal - aSymbolLastSize.Width();
        aSymbolSize.Width() = nWidthVal;
        if (bRatio)
        {
            aSymbolSize.Height() = aSymbolLastSize.Height() + (long)((double)nDelta / fSizeRatio);
            aSymbolSize.Height() = OutputDevice::LogicToLogic( aSymbolSize.Height(),(MapUnit)ePoolUnit, MAP_100TH_MM );
            m_pSymbolHeightMF->SetUserValue(m_pSymbolHeightMF->Normalize(aSymbolSize.Height()), FUNIT_100TH_MM);
        }
    }
    else
    {
        long nDelta = nHeightVal - aSymbolLastSize.Height();
        aSymbolSize.Height() = nHeightVal;
        if (bRatio)
        {
            aSymbolSize.Width() = aSymbolLastSize.Width() + (long)((double)nDelta * fSizeRatio);
            aSymbolSize.Width() = OutputDevice::LogicToLogic( aSymbolSize.Width(), (MapUnit)ePoolUnit, MAP_100TH_MM );
            m_pSymbolWidthMF->SetUserValue(m_pSymbolWidthMF->Normalize(aSymbolSize.Width()), FUNIT_100TH_MM);
        }
    }
    m_pCtlPreview->ResizeSymbol(aSymbolSize);
    aSymbolLastSize=aSymbolSize;
    return 0;
}
IMPL_LINK( SvxLineTabPage, RatioHdl_Impl, CheckBox *, pBox )
{
    if (pBox->IsChecked())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(m_pSymbolWidthMF);
        else
            SizeHdl_Impl(m_pSymbolHeightMF);
    }
    return 0;
}


void SvxLineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        FillListboxes();
    }
}

void SvxLineTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pColorListItem,SvxColorListItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDashListItem,SvxDashListItem,SID_DASH_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pLineEndListItem,SvxLineEndListItem,SID_LINEEND_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pSdrObjListItem,OfaPtrItem,SID_OBJECT_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pSymbolAttrItem,SfxTabDialogItem,SID_ATTR_SET,sal_False);
    SFX_ITEMSET_ARG (&aSet,pGraphicItem,SvxGraphicItem,SID_GRAPHIC,sal_False);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pDashListItem)
        SetDashList(pDashListItem->GetDashList());
    if (pLineEndListItem)
        SetLineEndList(pLineEndListItem->GetLineEndList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();

    if(pSdrObjListItem) //symbols
    {
        ShowSymbolControls(sal_True);
        pSymbolList = static_cast<SdrObjList*>(pSdrObjListItem->GetValue());
        if (pSymbolAttrItem)
            pSymbolAttr = new SfxItemSet(pSymbolAttrItem->GetItemSet());
        if(pGraphicItem)
            aAutoSymbolGraphic = pGraphicItem->GetGraphic();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
