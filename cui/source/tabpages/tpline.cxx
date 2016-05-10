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
#include <osl/file.hxx>
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
#include "paragrph.hrc"
#include "sfx2/opengrf.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <vcl/settings.hxx>

#define MAX_BMP_WIDTH   16
#define MAX_BMP_HEIGHT  16

#define MN_GALLERY         4
#define MN_SYMBOLS         5
#define MN_SYMBOLS_NONE    1
#define MN_SYMBOLS_AUTO    2
#define MN_GALLERY_ENTRY 100

using namespace com::sun::star;

// static ----------------------------------------------------------------

const sal_uInt16 SvxLineTabPage::pLineRanges[] =
{
    XATTR_LINETRANSPARENCE,
    XATTR_LINETRANSPARENCE,
    SID_ATTR_LINE_STYLE,
    SID_ATTR_LINE_ENDCENTER,
    0
};

SvxLineTabPage::SvxLineTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SvxTabPage ( pParent
                 ,"LineTabPage"
                 ,"cui/ui/linetabpage.ui"
                 , rInAttrs ),

    m_pSymbolList(nullptr),
    m_bNewSize(false),
    m_nNumMenuGalleryItems(0),
    m_nSymbolType(SVX_SYMBOLTYPE_UNKNOWN), // unknown respectively unchanged
    m_pSymbolAttr(nullptr),

    m_bLastWidthModified(false),
    m_aSymbolLastSize(Size(0,0)),
    m_bSymbols(false),

    m_rOutAttrs           ( rInAttrs ),
    m_eRP( RP_LT ),
    m_bObjSelected( false ),

    m_aXLineAttr          ( rInAttrs.GetPool() ),
    m_rXLSet              ( m_aXLineAttr.GetItemSet() ),
     m_pnLineEndListState( nullptr ),
    m_pnDashListState( nullptr ),
    m_pnColorListState( nullptr ),
    m_nPageType           ( 0 ),

    m_nDlgType(0),
    m_pPosDashLb(nullptr),
    m_pPosLineEndLb(nullptr)
{
    get(m_pLbLineStyle,"LB_LINE_STYLE");
    get(m_pBoxColor,"boxCOLOR");
    get(m_pLbColor,"LB_COLOR");
    get(m_pBoxWidth,"boxWIDTH");
    get(m_pMtrLineWidth,"MTR_FLD_LINE_WIDTH");
    get(m_pBoxTransparency,"boxTRANSPARENCY");
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
            SAL_FALLTHROUGH; // we now have mm
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
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    m_pLbLineStyle->SetSelectHdl( LINK( this, SvxLineTabPage, ClickInvisibleHdl_Impl ) );
    m_pLbColor->SetSelectHdl( LINK( this, SvxLineTabPage, ChangePreviewListBoxHdl_Impl ) );
    m_pMtrLineWidth->SetModifyHdl( LINK( this, SvxLineTabPage, ChangePreviewModifyHdl_Impl ) );
    m_pMtrTransparent->SetModifyHdl( LINK( this, SvxLineTabPage, ChangeTransparentHdl_Impl ) );

    m_pLbStartStyle->SetSelectHdl( LINK( this, SvxLineTabPage, ChangeStartListBoxHdl_Impl ) );
    m_pLbEndStyle->SetSelectHdl( LINK( this, SvxLineTabPage, ChangeEndListBoxHdl_Impl ) );
    m_pMtrStartWidth->SetModifyHdl( LINK( this, SvxLineTabPage, ChangeStartModifyHdl_Impl ) );
    m_pMtrEndWidth->SetModifyHdl( LINK( this, SvxLineTabPage, ChangeEndModifyHdl_Impl ) );
    m_pTsbCenterStart->SetClickHdl( LINK( this, SvxLineTabPage, ChangeStartClickHdl_Impl ) );
    m_pTsbCenterEnd->SetClickHdl( LINK( this, SvxLineTabPage, ChangeEndClickHdl_Impl ) );

    Link<ListBox&,void> aEdgeStyle = LINK( this, SvxLineTabPage, ChangeEdgeStyleHdl_Impl );
    m_pLBEdgeStyle->SetSelectHdl( aEdgeStyle );

    // LineCaps
    Link<ListBox&,void> aCapStyle = LINK( this, SvxLineTabPage, ChangeCapStyleHdl_Impl );
    m_pLBCapStyle->SetSelectHdl( aCapStyle );

    // Symbols on a line (eg star charts), MB-handler set
    m_pSymbolMB->SetSelectHdl(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    m_pSymbolMB->SetActivateHdl(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
    m_pSymbolWidthMF->SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_pSymbolHeightMF->SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    m_pSymbolRatioCB->SetClickHdl(LINK(this, SvxLineTabPage, RatioHdl_Impl));

    m_pSymbolRatioCB->Check();
    ShowSymbolControls(false);

    m_nActLineWidth = -1;
}

void SvxLineTabPage::ShowSymbolControls(bool bOn)
{
    // Symbols on a line (e.g. StarCharts), symbol-enable controls

    m_bSymbols=bOn;
    m_pFlSymbol->Show(bOn);
    m_pCtlPreview->ShowSymbol(bOn);
}

SvxLineTabPage::~SvxLineTabPage()
{
    disposeOnce();
}

void SvxLineTabPage::dispose()
{
    // Symbols on a line (e.g. StarCharts), dtor new!
    if (m_pSymbolMB)
    {
        delete m_pSymbolMB->GetPopupMenu()->GetPopupMenu( MN_GALLERY );

        if(m_pSymbolList)
            delete m_pSymbolMB->GetPopupMenu()->GetPopupMenu( MN_SYMBOLS );
        m_pSymbolMB = nullptr;
    }

    for ( size_t i = 0, n = m_aGrfBrushItems.size(); i < n; ++i )
    {
        SvxBmpItemInfo* pInfo = m_aGrfBrushItems[ i ];
        delete pInfo->pBrushItem;
        delete pInfo;
    }
    m_aGrfBrushItems.clear();

    m_pBoxColor.clear();
    m_pLbLineStyle.clear();
    m_pLbColor.clear();
    m_pBoxWidth.clear();
    m_pMtrLineWidth.clear();
    m_pBoxTransparency.clear();
    m_pMtrTransparent.clear();
    m_pFlLineEnds.clear();
    m_pBoxArrowStyles.clear();
    m_pLbStartStyle.clear();
    m_pBoxStart.clear();
    m_pMtrStartWidth.clear();
    m_pTsbCenterStart.clear();
    m_pBoxEnd.clear();
    m_pLbEndStyle.clear();
    m_pMtrEndWidth.clear();
    m_pTsbCenterEnd.clear();
    m_pCbxSynchronize.clear();
    m_pCtlPreview.clear();
    m_pFLEdgeStyle.clear();
    m_pGridEdgeCaps.clear();
    m_pLBEdgeStyle.clear();
    m_pLBCapStyle.clear();
    m_pFlSymbol.clear();
    m_pGridIconSize.clear();
    m_pSymbolMB.clear();
    m_pSymbolWidthMF.clear();
    m_pSymbolHeightMF.clear();
    m_pSymbolRatioCB.clear();
    SvxTabPage::dispose();
}

void SvxLineTabPage::Construct()
{
    // Color chart
    m_pLbColor->Fill( m_pColorList );
    FillListboxes();
}

void SvxLineTabPage::InitSymbols(MenuButton* pButton)
{
    // Initialize popup
    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_GALLERY ))
    {
        // Get gallery entries
        GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, m_aGrfNames);

        PopupMenu* pPopup = new PopupMenu;
        sal_uInt32 i = 0;
        m_nNumMenuGalleryItems = m_aGrfNames.size();
        for(std::vector<OUString>::iterator it = m_aGrfNames.begin(); it != m_aGrfNames.end(); ++it, ++i)
        {
            const OUString *pUIName = &(*it);

            // Convert URL encodings to UI characters (e.g. %20 for spaces)
            OUString aPhysicalName;
            if (osl::FileBase::getSystemPathFromFileURL(*it, aPhysicalName)
                == osl::FileBase::E_None)
            {
                pUIName = &aPhysicalName;
            }

            SvxBrushItem* pBrushItem = new SvxBrushItem(*it, "", GPOS_AREA, SID_ATTR_BRUSH);

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (sal_uInt16)(MN_GALLERY_ENTRY + i);
            if ( i < m_aGrfBrushItems.size() )
            {
                m_aGrfBrushItems.insert( m_aGrfBrushItems.begin() + i, pInfo );
            } else
            {
                m_aGrfBrushItems.push_back( pInfo );
            }
            const Graphic* pGraphic = pBrushItem->GetGraphic();

            if(pGraphic)
            {
                Bitmap aBitmap(pGraphic->GetBitmap());
                Size aSize(aBitmap.GetSizePixel());
                if(aSize.Width()  > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
                {
                    bool bWidth = aSize.Width() > aSize.Height();
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

        if(m_aGrfNames.empty())
            m_pSymbolMB->GetPopupMenu()->EnableItem(MN_GALLERY, false);
    }

    if(!pButton->GetPopupMenu()->GetPopupMenu( MN_SYMBOLS ) && m_pSymbolList)
    {
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        pVDev->SetMapMode(MapMode(MAP_100TH_MM));
        std::unique_ptr<SdrModel> pModel(new SdrModel);
        pModel->GetItemPool().FreezeIdRanges();
        // Page
        SdrPage* pPage = new SdrPage( *pModel, false );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        {
        // 3D View
        std::unique_ptr<SdrView> pView(new SdrView( pModel.get(), pVDev ));
        pView->hideMarkHandles();
        pView->ShowSdrPage(pPage);

        PopupMenu* pPopup = new PopupMenu;

        // Generate invisible square to give all symbols a
        // bitmap size, which is independent from specific glyph
        SdrObject *pInvisibleSquare=m_pSymbolList->GetObj(0);
        pInvisibleSquare=pInvisibleSquare->Clone();
        pPage->NbcInsertObject(pInvisibleSquare);
        pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
        pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

        for(size_t i=0;; ++i)
        {
            SdrObject *pObj=m_pSymbolList->GetObj(i);
            if(pObj==nullptr)
                break;
            pObj=pObj->Clone();
            m_aGrfNames.push_back("");
            pPage->NbcInsertObject(pObj);
            if(m_pSymbolAttr)
            {
                pObj->SetMergedItemSet(*m_pSymbolAttr);
            }
            else
            {
                pObj->SetMergedItemSet(m_rOutAttrs);
            }
            pView->MarkAll();
            BitmapEx aBitmapEx(pView->GetMarkedObjBitmapEx());
            GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());
            pView->UnmarkAll();
            pObj=pPage->RemoveObject(1);
            SdrObject::Free(pObj);

            SvxBrushItem* pBrushItem = new SvxBrushItem(Graphic(aMeta), GPOS_AREA, SID_ATTR_BRUSH);

            SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
            pInfo->pBrushItem = pBrushItem;
            pInfo->nItemId = (sal_uInt16)(MN_GALLERY_ENTRY + i + m_nNumMenuGalleryItems);
            if ( (size_t)(m_nNumMenuGalleryItems + i) < m_aGrfBrushItems.size() ) {
                m_aGrfBrushItems.insert( m_aGrfBrushItems.begin() + m_nNumMenuGalleryItems + i, pInfo );
            } else {
                m_aGrfBrushItems.push_back( pInfo );
            }

            Size aSize(aBitmapEx.GetSizePixel());
            if(aSize.Width() > MAX_BMP_WIDTH || aSize.Height() > MAX_BMP_HEIGHT)
            {
                bool bWidth = aSize.Width() > aSize.Height();
                double nScale = bWidth ?
                                    (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                    (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                aBitmapEx.Scale(nScale, nScale);
            }
            Image aImage(aBitmapEx);
            pPopup->InsertItem(pInfo->nItemId,"",aImage);
        }
        pInvisibleSquare=pPage->RemoveObject(0);
        SdrObject::Free(pInvisibleSquare);

        m_pSymbolMB->GetPopupMenu()->SetPopupMenu( MN_SYMBOLS, pPopup );

        if(m_aGrfNames.empty())
            m_pSymbolMB->GetPopupMenu()->EnableItem(MN_SYMBOLS, false);

        }
    }
}

void SvxLineTabPage::SymbolSelected(MenuButton* pButton)
{
    sal_uInt16 nItemId = pButton->GetCurItemId();
    const Graphic* pGraphic = nullptr;
    Graphic aGraphic;
    bool bResetSize = false;
    bool bEnable = true;
    long nPreviousSymbolType = m_nSymbolType;

    if(nItemId >= MN_GALLERY_ENTRY)
    {
        if( (nItemId-MN_GALLERY_ENTRY) >= m_nNumMenuGalleryItems)
        {
            m_nSymbolType=nItemId-MN_GALLERY_ENTRY-m_nNumMenuGalleryItems; // List's index
        }
        else
        {
            m_nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
            bResetSize = true;
        }
        SvxBmpItemInfo* pInfo = m_aGrfBrushItems[ nItemId - MN_GALLERY_ENTRY ];
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else switch(nItemId)
    {
        case MN_SYMBOLS_AUTO:
        {
            pGraphic=&m_aAutoSymbolGraphic;
            m_aAutoSymbolGraphic.SetPrefSize( Size(253,253) );
            m_nSymbolType=SVX_SYMBOLTYPE_AUTO;
        }
        break;

        case MN_SYMBOLS_NONE:
        {
            m_nSymbolType=SVX_SYMBOLTYPE_NONE;
            pGraphic=nullptr;
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
                    m_nSymbolType=SVX_SYMBOLTYPE_BRUSHITEM;
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
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)m_ePoolUnit);
        m_aSymbolGraphic=*pGraphic;
        if( bResetSize )
        {
            m_aSymbolSize=aSize;
        }
        else if( nPreviousSymbolType == SVX_SYMBOLTYPE_BRUSHITEM )
        {   //#i31097# Data Point Symbol size changes when a different symbol is chosen(maoyg)
            if( m_aSymbolSize.Width() != m_aSymbolSize.Height() )
            {
                aSize.setWidth( (long)( m_aSymbolSize.Width() + m_aSymbolSize.Height() )/2 );
                aSize.setHeight( (long)( m_aSymbolSize.Width() + m_aSymbolSize.Height() )/2 );
                m_aSymbolSize = aSize;
            }
        }
        m_pCtlPreview->SetSymbol(&m_aSymbolGraphic,m_aSymbolSize);
    }
    else
    {
        m_aSymbolGraphic=Graphic();
        m_pCtlPreview->SetSymbol(nullptr,m_aSymbolSize);
        bEnable = false;
    }
    m_aSymbolLastSize=m_aSymbolSize;
    SetMetricValue(*m_pSymbolWidthMF,  m_aSymbolSize.Width(), m_ePoolUnit);
    SetMetricValue(*m_pSymbolHeightMF, m_aSymbolSize.Height(), m_ePoolUnit);

    m_pGridIconSize->Enable(bEnable);
    m_pCtlPreview->Invalidate();
}

void SvxLineTabPage::FillListboxes()
{
    // Line styles
    sal_Int32 nOldSelect = m_pLbLineStyle->GetSelectEntryPos();
    // aLbLineStyle.FillStyles();
    m_pLbLineStyle->Fill( m_pDashList );
    m_pLbLineStyle->SelectEntryPos( nOldSelect );

    // Line end style
    OUString sNone( SVX_RES( RID_SVXSTR_NONE ) );
    nOldSelect = m_pLbStartStyle->GetSelectEntryPos();
    m_pLbStartStyle->Clear();
    m_pLbStartStyle->InsertEntry( sNone );
    m_pLbStartStyle->Fill( m_pLineEndList );
    m_pLbStartStyle->SelectEntryPos( nOldSelect );
    nOldSelect = m_pLbEndStyle->GetSelectEntryPos();
    m_pLbEndStyle->Clear();
    m_pLbEndStyle->InsertEntry( sNone );
    m_pLbEndStyle->Fill( m_pLineEndList, false );
    m_pLbEndStyle->SelectEntryPos( nOldSelect );
}


void SvxLineTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const CntUInt16Item* pPageTypeItem = rSet.GetItem<CntUInt16Item>(SID_PAGE_TYPE, false);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if( m_nDlgType == 0 && m_pDashList.is() )
    {
        sal_Int32 nPos;
        sal_Int32 nCount;

        // Dash list
        if( ( *m_pnDashListState & ChangeType::MODIFIED ) ||
            ( *m_pnDashListState & ChangeType::CHANGED ) )
        {
            if( *m_pnDashListState & ChangeType::CHANGED )
                m_pDashList = static_cast<SvxLineTabDialog*>( GetParentDialog() )->GetNewDashList();

            *m_pnDashListState = ChangeType::NONE;

            // Style list
            nPos = m_pLbLineStyle->GetSelectEntryPos();

            m_pLbLineStyle->Clear();
            m_pLbLineStyle->InsertEntry( SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );
            m_pLbLineStyle->InsertEntry( SVX_RESSTR( RID_SVXSTR_SOLID ) );
            m_pLbLineStyle->Fill( m_pDashList );
            nCount = m_pLbLineStyle->GetEntryCount();

            if ( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbLineStyle->SelectEntryPos( 0 );
            else
                m_pLbLineStyle->SelectEntryPos( nPos );
            // SelectStyleHdl_Impl( this );
        }

        INetURLObject   aDashURL( m_pDashList->GetPath() );

        aDashURL.Append( m_pDashList->GetName() );
        DBG_ASSERT( aDashURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        // LineEnd list
        if( ( *m_pnLineEndListState & ChangeType::MODIFIED ) || ( *m_pnLineEndListState & ChangeType::CHANGED ) )
        {
            if( *m_pnLineEndListState & ChangeType::CHANGED )
                m_pLineEndList = static_cast<SvxLineTabDialog*>( GetParentDialog() )->GetNewLineEndList();

            *m_pnLineEndListState = ChangeType::NONE;

            nPos = m_pLbLineStyle->GetSelectEntryPos();
            OUString sNone( SVX_RES( RID_SVXSTR_NONE ) );
            m_pLbStartStyle->Clear();
            m_pLbStartStyle->InsertEntry( sNone );

            m_pLbStartStyle->Fill( m_pLineEndList );
            nCount = m_pLbStartStyle->GetEntryCount();
            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbStartStyle->SelectEntryPos( 0 );
            else
                m_pLbStartStyle->SelectEntryPos( nPos );

            m_pLbEndStyle->Clear();
            m_pLbEndStyle->InsertEntry( sNone );

            m_pLbEndStyle->Fill( m_pLineEndList, false );
            nCount = m_pLbEndStyle->GetEntryCount();

            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                m_pLbEndStyle->SelectEntryPos( 0 );
            else
                m_pLbEndStyle->SelectEntryPos( nPos );
        }
        INetURLObject aLineURL( m_pLineEndList->GetPath() );

        aLineURL.Append( m_pLineEndList->GetName() );
        DBG_ASSERT( aLineURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        // Evaluate if another TabPage set another fill type
        if( m_pLbLineStyle->GetSelectEntryPos() != 0 )
        {
            if( m_nPageType == 2 ) // 1
            {
                m_pLbLineStyle->SelectEntryPos( *m_pPosDashLb + 2 ); // +2 due to SOLID and INVISIBLE
                ChangePreviewHdl_Impl( nullptr );
            }
            if( m_nPageType == 3 )
            {
                m_pLbStartStyle->SelectEntryPos( *m_pPosLineEndLb + 1 );// +1 due to SOLID
                m_pLbEndStyle->SelectEntryPos( *m_pPosLineEndLb + 1 );// +1 due to SOLID
                ChangePreviewHdl_Impl( nullptr );
            }
        }

            // ColorList
            if( *m_pnColorListState != ChangeType::NONE )
            {
                if( *m_pnColorListState & ChangeType::CHANGED )
                    m_pColorList = static_cast<SvxLineTabDialog*>( GetParentDialog() )->GetNewColorList();
                // aLbColor
                sal_Int32 nColorPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( m_pColorList );
                nCount = m_pLbColor->GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nColorPos )
                    m_pLbColor->SelectEntryPos( 0 );
                else
                    m_pLbColor->SelectEntryPos( nColorPos );

                ChangePreviewHdl_Impl( nullptr );
            }

        m_nPageType = 0;
    }
    // Page does not yet exist in the ctor, that's why we do it here!

    else if ( m_nDlgType == 1100 ||
              m_nDlgType == 1101 )
    {
        m_pFlLineEnds->Hide();
        m_pFLEdgeStyle->Hide();
    }
}


SfxTabPage::sfxpg SvxLineTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( m_nDlgType == 0 ) // Line dialog
    {
        m_nPageType = 1; // possibly for extensions
        *m_pPosDashLb = m_pLbLineStyle->GetSelectEntryPos() - 2;// First entry SOLID!!!
        sal_Int32 nPos = m_pLbStartStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
            nPos--;
        *m_pPosLineEndLb = nPos;
    }

    if( _pSet )
        FillItemSet( _pSet );

    return LEAVE_PAGE;
}


bool SvxLineTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    const SfxPoolItem* pOld = nullptr;
    sal_Int32  nPos;
    bool    bModified = false;

    // To prevent modifications to the list, we do not set other page's items.
    if( m_nDlgType != 0 || m_nPageType != 2 )
    {
        nPos = m_pLbLineStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            m_pLbLineStyle->IsValueChangedFromSaved() )
        {
            std::unique_ptr<XLineStyleItem> pStyleItem;

            if( nPos == 0 )
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_NONE ));
            else if( nPos == 1 )
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_SOLID ));
            else
            {
                pStyleItem.reset(new XLineStyleItem( drawing::LineStyle_DASH ));

                // For added security
                if( m_pDashList->Count() > (long) ( nPos - 2 ) )
                {
                    XLineDashItem aDashItem( m_pLbLineStyle->GetSelectEntry(),
                                        m_pDashList->GetDash( nPos - 2 )->GetDash() );
                    pOld = GetOldItem( *rAttrs, XATTR_LINEDASH );
                    if ( !pOld || !( *static_cast<const XLineDashItem*>(pOld) == aDashItem ) )
                    {
                        rAttrs->Put( aDashItem );
                        bModified = true;
                    }
                }
            }
            pOld = GetOldItem( *rAttrs, XATTR_LINESTYLE );
            if ( !pOld || !( *static_cast<const XLineStyleItem*>(pOld) == *pStyleItem ) )
            {
                rAttrs->Put( *pStyleItem );
                bModified = true;
            }
        }
    }
    // Line width
    // GetSavedValue() returns OUString!
    if( m_pMtrLineWidth->IsValueChangedFromSaved() )
    {
        XLineWidthItem aItem( GetCoreValue( *m_pMtrLineWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINEWIDTH );
        if ( !pOld || !( *static_cast<const XLineWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    // Width line start
    if( m_pMtrStartWidth->IsValueChangedFromSaved() )
    {
        XLineStartWidthItem aItem( GetCoreValue( *m_pMtrStartWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *static_cast<const XLineStartWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    // Width line end
    if( m_pMtrEndWidth->IsValueChangedFromSaved() )
    {
        XLineEndWidthItem aItem( GetCoreValue( *m_pMtrEndWidth, m_ePoolUnit ) );
        pOld = GetOldItem( *rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *static_cast<const XLineEndWidthItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    // Line color
    if( m_pLbColor->IsValueChangedFromSaved() )
    {
        XLineColorItem aItem( m_pLbColor->GetSelectEntry(), m_pLbColor->GetSelectEntryColor() );
        pOld = GetOldItem( *rAttrs, XATTR_LINECOLOR );
        if ( !pOld || !( *static_cast<const XLineColorItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    if( m_nDlgType != 0 || m_nPageType != 3 )
    {
        // Line start
        nPos = m_pLbStartStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND && m_pLbStartStyle->IsValueChangedFromSaved() )
        {
            std::unique_ptr<XLineStartItem> pItem;
            if( nPos == 0 )
                pItem.reset(new XLineStartItem());
            else if( m_pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem.reset(new XLineStartItem( m_pLbStartStyle->GetSelectEntry(), m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
            pOld = GetOldItem( *rAttrs, XATTR_LINESTART );
            if( pItem && ( !pOld || !( *static_cast<const XLineEndItem*>(pOld) == *pItem ) ) )
            {
                rAttrs->Put( *pItem );
                bModified = true;
            }
        }
        // Line end
        nPos = m_pLbEndStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND && m_pLbEndStyle->IsValueChangedFromSaved() )
        {
            std::unique_ptr<XLineEndItem> pItem;
            if( nPos == 0 )
                pItem.reset(new XLineEndItem());
            else if( m_pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem.reset(new XLineEndItem( m_pLbEndStyle->GetSelectEntry(), m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ));
            pOld = GetOldItem( *rAttrs, XATTR_LINEEND );
            if( pItem &&
                ( !pOld || !( *static_cast<const XLineEndItem*>(pOld) == *pItem ) ) )
            {
                rAttrs->Put( *pItem );
                bModified = true;
            }
        }
    }

    // Centered line end
    TriState eState = m_pTsbCenterStart->GetState();
    if( m_pTsbCenterStart->IsValueChangedFromSaved() )
    {
        XLineStartCenterItem aItem( eState != TRISTATE_FALSE );
        pOld = GetOldItem( *rAttrs, XATTR_LINESTARTCENTER );
        if ( !pOld || !( *static_cast<const XLineStartCenterItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }
    eState = m_pTsbCenterEnd->GetState();
    if( m_pTsbCenterEnd->IsValueChangedFromSaved() )
    {
        XLineEndCenterItem aItem( eState != TRISTATE_FALSE );
        pOld = GetOldItem( *rAttrs, XATTR_LINEENDCENTER );
        if ( !pOld || !( *static_cast<const XLineEndCenterItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    // Transparency
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    if( m_pMtrTransparent->IsValueChangedFromSaved() )
    {
        XLineTransparenceItem aItem( nVal );
        pOld = GetOldItem( *rAttrs, XATTR_LINETRANSPARENCE );
        if ( !pOld || !( *static_cast<const XLineTransparenceItem*>(pOld) == aItem ) )
        {
            rAttrs->Put( aItem );
            bModified = true;
        }
    }

    nPos = m_pLBEdgeStyle->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && m_pLBEdgeStyle->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineJointItem> pNew;

        switch(nPos)
        {
            case 0: // Rounded, default
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_ROUND));
                break;
            }
            case 1: // - none -
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_NONE));
                break;
            }
            case 2: // Miter
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_MITER));
                break;
            }
            case 3: // Bevel
            {
                pNew.reset(new XLineJointItem(css::drawing::LineJoint_BEVEL));
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( *rAttrs, XATTR_LINEJOINT );

            if(!pOld || !(*static_cast<const XLineJointItem*>(pOld) == *pNew))
            {
                rAttrs->Put( *pNew );
                bModified = true;
            }
        }
    }

    // LineCaps
    nPos = m_pLBCapStyle->GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && m_pLBCapStyle->IsValueChangedFromSaved() )
    {
        std::unique_ptr<XLineCapItem> pNew;

        switch(nPos)
        {
            case 0: // Butt (=Flat), default
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_BUTT));
                break;
            }
            case 1: // Round
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_ROUND));
                break;
            }
            case 2: // Square
            {
                pNew.reset(new XLineCapItem(css::drawing::LineCap_SQUARE));
                break;
            }
        }

        if(pNew)
        {
            pOld = GetOldItem( *rAttrs, XATTR_LINECAP );

            if(!pOld || !(*static_cast<const XLineCapItem*>(pOld) == *pNew))
            {
                rAttrs->Put( *pNew );
                bModified = true;
            }
        }
    }

    if(m_nSymbolType!=SVX_SYMBOLTYPE_UNKNOWN || m_bNewSize)
    {
        // Was set by selection or the size is different
        SvxSizeItem  aSItem(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),m_aSymbolSize);
        const SfxPoolItem* pSOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE) );
        m_bNewSize  = pSOld ? *static_cast<const SvxSizeItem *>(pSOld) != aSItem : m_bNewSize ;
        if(m_bNewSize)
        {
            rAttrs->Put(aSItem);
            bModified=true;
        }

        SfxInt32Item aTItem(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),m_nSymbolType);
        const SfxPoolItem* pTOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE) );
        bool bNewType = pTOld == nullptr || *static_cast<const SfxInt32Item*>(pTOld) != aTItem;
        if(bNewType && m_nSymbolType==SVX_SYMBOLTYPE_UNKNOWN)
            bNewType=false; // a small fix, type wasn't set -> don't create a type item after all!
        if(bNewType)
        {
            rAttrs->Put(aTItem);
            bModified=true;
        }

        if(m_nSymbolType!=SVX_SYMBOLTYPE_NONE)
        {
            SvxBrushItem aBItem(m_aSymbolGraphic,GPOS_MM,rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH));
            const SfxPoolItem* pBOld = GetOldItem( *rAttrs, rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH) );
            bool bNewBrush =
                pBOld == nullptr || *static_cast<const SvxBrushItem*>(pBOld) != aBItem;
            if(bNewBrush)
            {
                rAttrs->Put(aBItem);
                bModified=true;
            }
        }
    }
    rAttrs->Put (CntUInt16Item(SID_PAGE_TYPE,m_nPageType));
    return bModified;
}



bool SvxLineTabPage::FillXLSet_Impl()
{
    sal_Int32 nPos;

    if( m_pLbLineStyle->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    }
    else if( m_pLbLineStyle->IsEntryPosSelected( 0 ) )
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_NONE ) );
    else if( m_pLbLineStyle->IsEntryPosSelected( 1 ) )
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_SOLID ) );
    else
    {
        m_rXLSet.Put( XLineStyleItem( drawing::LineStyle_DASH ) );

        nPos = m_pLbLineStyle->GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            m_rXLSet.Put( XLineDashItem( m_pLbLineStyle->GetSelectEntry(),
                            m_pDashList->GetDash( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = m_pLbStartStyle->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            m_rXLSet.Put( XLineStartItem() );
        else
            m_rXLSet.Put( XLineStartItem( m_pLbStartStyle->GetSelectEntry(),
                        m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = m_pLbEndStyle->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            m_rXLSet.Put( XLineEndItem() );
        else
            m_rXLSet.Put( XLineEndItem( m_pLbEndStyle->GetSelectEntry(),
                        m_pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }

    nPos = m_pLBEdgeStyle->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        switch(nPos)
        {
            case 0: // Rounded, default
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_ROUND));
                break;
            }
            case 1: // - none -
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_NONE));
                break;
            }
            case 2: // Miter
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_MITER));
                break;
            }
            case 3: // Bevel
            {
                m_rXLSet.Put(XLineJointItem(css::drawing::LineJoint_BEVEL));
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
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_BUTT));
                break;
            }
            case 1: // Round
            {
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_ROUND));
                break;
            }
            case 2: // Square
            {
                m_rXLSet.Put(XLineCapItem(css::drawing::LineCap_SQUARE));
                break;
            }
        }
    }

    m_rXLSet.Put( XLineStartWidthItem( GetCoreValue( *m_pMtrStartWidth, m_ePoolUnit ) ) );
    m_rXLSet.Put( XLineEndWidthItem( GetCoreValue( *m_pMtrEndWidth, m_ePoolUnit ) ) );

    m_rXLSet.Put( XLineWidthItem( GetCoreValue( *m_pMtrLineWidth, m_ePoolUnit ) ) );
    m_rXLSet.Put( XLineColorItem( m_pLbColor->GetSelectEntry(), m_pLbColor->GetSelectEntryColor() ) );

    // Centered line end
    if( m_pTsbCenterStart->GetState() == TRISTATE_TRUE )
        m_rXLSet.Put( XLineStartCenterItem( true ) );
    else if( m_pTsbCenterStart->GetState() == TRISTATE_FALSE )
        m_rXLSet.Put( XLineStartCenterItem( false ) );

    if( m_pTsbCenterEnd->GetState() == TRISTATE_TRUE )
        m_rXLSet.Put( XLineEndCenterItem( true ) );
    else if( m_pTsbCenterEnd->GetState() == TRISTATE_FALSE )
        m_rXLSet.Put( XLineEndCenterItem( false ) );

    // Transparency
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    m_rXLSet.Put( XLineTransparenceItem( nVal ) );

    m_pCtlPreview->SetLineAttributes(m_aXLineAttr.GetItemSet());

    return true;
}


void SvxLineTabPage::Reset( const SfxItemSet* rAttrs )
{
    drawing::LineStyle  eXLS; // drawing::LineStyle_NONE, drawing::LineStyle_SOLID, drawing::LineStyle_DASH

    // Line style
    const SfxPoolItem *pPoolItem;
    long nSymType=SVX_SYMBOLTYPE_UNKNOWN;
    bool bPrevSym=false;
    bool bEnable=true;
    bool bIgnoreGraphic=false;
    bool bIgnoreSize=false;
    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLTYPE),true,&pPoolItem) == SfxItemState::SET)
    {
        nSymType=static_cast<const SfxInt32Item *>(pPoolItem)->GetValue();
    }

    if(nSymType == SVX_SYMBOLTYPE_AUTO)
    {
        m_aSymbolGraphic=m_aAutoSymbolGraphic;
        m_aSymbolSize=m_aSymbolLastSize=m_aAutoSymbolGraphic.GetPrefSize();
        bPrevSym=true;
    }
    else if(nSymType == SVX_SYMBOLTYPE_NONE)
    {
        bEnable=false;
        bIgnoreGraphic=true;
        bIgnoreSize=true;
    }
    else if(nSymType >= 0)
    {
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        pVDev->SetMapMode(MapMode(MAP_100TH_MM));

        std::unique_ptr<SdrModel> pModel(new SdrModel);
        pModel->GetItemPool().FreezeIdRanges();
        SdrPage* pPage = new SdrPage( *pModel, false );
        pPage->SetSize(Size(1000,1000));
        pModel->InsertPage( pPage, 0 );
        {
        std::unique_ptr<SdrView> pView(new SdrView( pModel.get(), pVDev ));
        pView->hideMarkHandles();
        pView->ShowSdrPage(pPage);
        SdrObject *pObj=nullptr;
        size_t nSymTmp = static_cast<size_t>(nSymType);
        if(m_pSymbolList)
        {
            if(m_pSymbolList->GetObjCount())
            {
                nSymTmp %= m_pSymbolList->GetObjCount(); // Treat list as cyclic!
                pObj=m_pSymbolList->GetObj(nSymTmp);
                if(pObj)
                {
                    pObj=pObj->Clone();
                    if(m_pSymbolAttr)
                    {
                        pObj->SetMergedItemSet(*m_pSymbolAttr);
                    }
                    else
                    {
                        pObj->SetMergedItemSet(m_rOutAttrs);
                    }

                    pPage->NbcInsertObject(pObj);

                    // Generate invisible square to give all symbol types a
                    // bitmap size, which is independent from specific glyph
                    SdrObject *pInvisibleSquare=m_pSymbolList->GetObj(0);
                    pInvisibleSquare=pInvisibleSquare->Clone();
                    pPage->NbcInsertObject(pInvisibleSquare);
                    pInvisibleSquare->SetMergedItem(XFillTransparenceItem(100));
                    pInvisibleSquare->SetMergedItem(XLineTransparenceItem(100));

                    pView->MarkAll();
                    GDIMetaFile aMeta(pView->GetMarkedObjMetaFile());

                    m_aSymbolGraphic=Graphic(aMeta);
                    m_aSymbolSize=pObj->GetSnapRect().GetSize();
                    m_aSymbolGraphic.SetPrefSize(pInvisibleSquare->GetSnapRect().GetSize());
                    m_aSymbolGraphic.SetPrefMapMode(MAP_100TH_MM);
                    bPrevSym=true;
                    bEnable=true;
                    bIgnoreGraphic=true;

                    pView->UnmarkAll();
                    pInvisibleSquare=pPage->RemoveObject(1);
                    SdrObject::Free( pInvisibleSquare);
                    pObj=pPage->RemoveObject(0);
                    SdrObject::Free( pObj );
                }
            }
        }
        }
    }
    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_BRUSH),true,&pPoolItem) == SfxItemState::SET)
    {
        const Graphic* pGraphic = static_cast<const SvxBrushItem *>(pPoolItem)->GetGraphic();
        if( pGraphic )
        {
            if(!bIgnoreGraphic)
            {
                m_aSymbolGraphic=*pGraphic;
            }
            if(!bIgnoreSize)
            {
                m_aSymbolSize=OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                                        pGraphic->GetPrefMapMode(),
                                                        MAP_100TH_MM );
            }
            bPrevSym=true;
        }
    }

    if(rAttrs->GetItemState(rAttrs->GetPool()->GetWhich(SID_ATTR_SYMBOLSIZE),true,&pPoolItem) == SfxItemState::SET)
    {
        m_aSymbolSize = static_cast<const SvxSizeItem *>(pPoolItem)->GetSize();
    }

    m_pGridIconSize->Enable(bEnable);

    if(bPrevSym)
    {
        SetMetricValue(*m_pSymbolWidthMF,  m_aSymbolSize.Width(), m_ePoolUnit);
        SetMetricValue(*m_pSymbolHeightMF, m_aSymbolSize.Height(),m_ePoolUnit);
        m_pCtlPreview->SetSymbol(&m_aSymbolGraphic,m_aSymbolSize);
        m_aSymbolLastSize=m_aSymbolSize;
    }

    if( rAttrs->GetItemState( XATTR_LINESTYLE ) != SfxItemState::DONTCARE )
    {
        eXLS = (drawing::LineStyle) static_cast<const XLineStyleItem&>( rAttrs->Get( XATTR_LINESTYLE ) ).GetValue();

        switch( eXLS )
        {
            case drawing::LineStyle_NONE:
                m_pLbLineStyle->SelectEntryPos( 0 );
                break;
            case drawing::LineStyle_SOLID:
                m_pLbLineStyle->SelectEntryPos( 1 );
                break;

            case drawing::LineStyle_DASH:
                m_pLbLineStyle->SetNoSelection();
                m_pLbLineStyle->SelectEntry( static_cast<const XLineDashItem&>( rAttrs->Get( XATTR_LINEDASH ) ).GetName() );
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
    if( rAttrs->GetItemState( XATTR_LINEWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_pMtrLineWidth, static_cast<const XLineWidthItem&>( rAttrs->Get( XATTR_LINEWIDTH ) ).GetValue(), m_ePoolUnit );
    }
    else
        m_pMtrLineWidth->SetText( "" );

    // Line color
    m_pLbColor->SetNoSelection();

    if ( rAttrs->GetItemState( XATTR_LINECOLOR ) != SfxItemState::DONTCARE )
    {
        Color aCol = static_cast<const XLineColorItem&>( rAttrs->Get( XATTR_LINECOLOR ) ).GetColorValue();
        m_pLbColor->SelectEntry( aCol );
        if( m_pLbColor->GetSelectEntryCount() == 0 )
        {
            m_pLbColor->InsertEntry( aCol, OUString() );
            m_pLbColor->SelectEntry( aCol );
        }
    }

    // Line start
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINESTART ) == SfxItemState::DEFAULT )
    {
        m_pLbStartStyle->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINESTART ) != SfxItemState::DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        bool bSelected(false);
        const basegfx::B2DPolyPolygon& rItemPolygon = static_cast<const XLineStartItem&>(rAttrs->Get(XATTR_LINESTART)).GetLineStartValue();

        for(sal_Int32 a(0);!bSelected &&  a < m_pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = m_pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_pLbStartStyle->SelectEntryPos(a + 1);
                bSelected = true;
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
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEEND ) == SfxItemState::DEFAULT )
    {
        m_pLbEndStyle->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINEEND ) != SfxItemState::DONTCARE )
    {
        // #86265# select entry using list and polygon, not string
        bool bSelected(false);
        const basegfx::B2DPolyPolygon& rItemPolygon = static_cast<const XLineEndItem&>(rAttrs->Get(XATTR_LINEEND)).GetLineEndValue();

        for(sal_Int32 a(0);!bSelected &&  a < m_pLineEndList->Count(); a++)
        {
            XLineEndEntry* pEntry = m_pLineEndList->GetLineEnd(a);
            const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();

            if(rItemPolygon == rEntryPolygon)
            {
                // select this entry
                m_pLbEndStyle->SelectEntryPos(a + 1);
                bSelected = true;
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
    if( m_bObjSelected &&  rAttrs->GetItemState( XATTR_LINESTARTWIDTH ) == SfxItemState::DEFAULT )
    {
        m_pMtrStartWidth->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINESTARTWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_pMtrStartWidth,
                        static_cast<const XLineStartWidthItem&>( rAttrs->Get( XATTR_LINESTARTWIDTH ) ).GetValue(),
                        m_ePoolUnit );
    }
    else
        m_pMtrStartWidth->SetText( "" );

    // Line end strength
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEENDWIDTH ) == SfxItemState::DEFAULT )
    {
        m_pMtrEndWidth->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINEENDWIDTH ) != SfxItemState::DONTCARE )
    {
        SetMetricValue( *m_pMtrEndWidth,
                        static_cast<const XLineEndWidthItem&>( rAttrs->Get( XATTR_LINEENDWIDTH ) ).GetValue(),
                        m_ePoolUnit );
    }
    else
        m_pMtrEndWidth->SetText( "" );

    // Centered line end (start)
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINESTARTCENTER ) == SfxItemState::DEFAULT )
    {
        m_pTsbCenterStart->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINESTARTCENTER ) != SfxItemState::DONTCARE )
    {
        m_pTsbCenterStart->EnableTriState( false );

        if( static_cast<const XLineStartCenterItem&>( rAttrs->Get( XATTR_LINESTARTCENTER ) ).GetValue() )
            m_pTsbCenterStart->SetState( TRISTATE_TRUE );
        else
            m_pTsbCenterStart->SetState( TRISTATE_FALSE );
    }
    else
    {
        m_pTsbCenterStart->SetState( TRISTATE_INDET );
    }

    // Centered line end (end)
    if( m_bObjSelected && rAttrs->GetItemState( XATTR_LINEENDCENTER ) == SfxItemState::DEFAULT )
    {
        m_pTsbCenterEnd->Disable();
    }
    else if( rAttrs->GetItemState( XATTR_LINEENDCENTER ) != SfxItemState::DONTCARE )
    {
        m_pTsbCenterEnd->EnableTriState( false );

        if( static_cast<const XLineEndCenterItem&>( rAttrs->Get( XATTR_LINEENDCENTER ) ).GetValue() )
            m_pTsbCenterEnd->SetState( TRISTATE_TRUE );
        else
            m_pTsbCenterEnd->SetState( TRISTATE_FALSE );
    }
    else
    {
        m_pTsbCenterEnd->SetState( TRISTATE_INDET );
    }

    // Transparency
    if( rAttrs->GetItemState( XATTR_LINETRANSPARENCE ) != SfxItemState::DONTCARE )
    {
        sal_uInt16 nTransp = static_cast<const XLineTransparenceItem&>( rAttrs->Get( XATTR_LINETRANSPARENCE ) ).GetValue();
        m_pMtrTransparent->SetValue( nTransp );
        ChangeTransparentHdl_Impl( *m_pMtrTransparent );
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
    m_pCbxSynchronize->Check( aStr.toInt32() != 0 );

    if(m_bObjSelected && SfxItemState::DEFAULT == rAttrs->GetItemState(XATTR_LINEJOINT))
    {
//         maFTEdgeStyle.Disable();
        m_pLBEdgeStyle->Disable();
    }
    else if(SfxItemState::DONTCARE != rAttrs->GetItemState(XATTR_LINEJOINT))
    {
        const css::drawing::LineJoint eLineJoint = static_cast<const XLineJointItem&>(rAttrs->Get(XATTR_LINEJOINT)).GetValue();

        switch(eLineJoint)
        {
            case css::drawing::LineJoint_MAKE_FIXED_SIZE: // fallback to round, unused value
            case css::drawing::LineJoint_MIDDLE : // fallback to round, unused value
            case css::drawing::LineJoint_ROUND : m_pLBEdgeStyle->SelectEntryPos(0); break;
            case css::drawing::LineJoint_NONE : m_pLBEdgeStyle->SelectEntryPos(1); break;
            case css::drawing::LineJoint_MITER : m_pLBEdgeStyle->SelectEntryPos(2); break;
            case css::drawing::LineJoint_BEVEL : m_pLBEdgeStyle->SelectEntryPos(3); break;
        }
    }
    else
    {
        m_pLBEdgeStyle->SetNoSelection();
    }

    // fdo#43209
    if(m_bObjSelected && SfxItemState::DEFAULT == rAttrs->GetItemState(XATTR_LINECAP))
    {
//         maFTCapStyle.Disable();
        m_pLBCapStyle->Disable();
    }
    else if(SfxItemState::DONTCARE != rAttrs->GetItemState(XATTR_LINECAP))
    {
        const css::drawing::LineCap eLineCap(static_cast<const XLineCapItem&>(rAttrs->Get(XATTR_LINECAP)).GetValue());

        switch(eLineCap)
        {
            case css::drawing::LineCap_ROUND: m_pLBCapStyle->SelectEntryPos(1); break;
            case css::drawing::LineCap_SQUARE : m_pLBCapStyle->SelectEntryPos(2); break;
            default /*css::drawing::LineCap_BUTT*/: m_pLBCapStyle->SelectEntryPos(0); break;
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

    m_pLBEdgeStyle->SaveValue();

    // LineCaps
    m_pLBCapStyle->SaveValue();

    ClickInvisibleHdl_Impl( *m_pLbLineStyle );

    ChangePreviewHdl_Impl( nullptr );
}


VclPtr<SfxTabPage> SvxLineTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxLineTabPage>::Create( pWindow, *rAttrs );
}


IMPL_LINK_TYPED( SvxLineTabPage, ChangePreviewListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ChangePreviewHdl_Impl(&rListBox);
}
IMPL_LINK_TYPED( SvxLineTabPage, ChangePreviewModifyHdl_Impl, Edit&, rEdit, void )
{
    ChangePreviewHdl_Impl(&rEdit);
}
void SvxLineTabPage::ChangePreviewHdl_Impl(void * pCntrl )
{
    if(pCntrl == m_pMtrLineWidth)
    {
        // Line width and start end width
        sal_Int32 nNewLineWidth = GetCoreValue( *m_pMtrLineWidth, m_ePoolUnit );
        if(m_nActLineWidth == -1)
        {
            // Don't initialize yet, get the start value
            const SfxPoolItem* pOld = GetOldItem( m_rXLSet, XATTR_LINEWIDTH );
            sal_Int32 nStartLineWidth = 0;
            if(pOld)
                nStartLineWidth = static_cast<const XLineWidthItem *>(pOld)->GetValue();
            m_nActLineWidth = nStartLineWidth;
        }

        if(m_nActLineWidth != nNewLineWidth)
        {
            // Adapt start/end width
            sal_Int32 nValAct = GetCoreValue( *m_pMtrStartWidth, m_ePoolUnit );
            sal_Int32 nValNew = nValAct + (((nNewLineWidth - m_nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_pMtrStartWidth, nValNew, m_ePoolUnit );

            nValAct = GetCoreValue( *m_pMtrEndWidth, m_ePoolUnit );
            nValNew = nValAct + (((nNewLineWidth - m_nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( *m_pMtrEndWidth, nValNew, m_ePoolUnit );
        }

        // Remember current value
        m_nActLineWidth = nNewLineWidth;
    }

    FillXLSet_Impl();
    m_pCtlPreview->Invalidate();

    // Make transparency accessible accordingly
    if( m_pLbLineStyle->GetSelectEntryPos() == 0 ) // invisible
    {
        m_pBoxTransparency->Disable();
    }
    else
    {
        m_pBoxTransparency->Enable();
    }

    const bool bHasLineStyle = m_pLbLineStyle->GetSelectEntryPos() !=0;
    const bool bHasLineStart = m_pLbStartStyle->GetSelectEntryPos() != 0;

    m_pBoxStart->Enable(bHasLineStart && bHasLineStyle);

    const bool bHasLineEnd = m_pLbEndStyle->GetSelectEntryPos() != 0;

    m_pBoxEnd->Enable(bHasLineEnd && bHasLineStyle);
}


IMPL_LINK_TYPED( SvxLineTabPage, ChangeStartClickHdl_Impl, Button*, p, void )
{
    ChangeStartHdl_Impl(p);
}
IMPL_LINK_TYPED( SvxLineTabPage, ChangeStartListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ChangeStartHdl_Impl(&rListBox);
}
IMPL_LINK_TYPED( SvxLineTabPage, ChangeStartModifyHdl_Impl, Edit&, rEdit, void )
{
    ChangeStartHdl_Impl(&rEdit);
}
void SvxLineTabPage::ChangeStartHdl_Impl( void * p )
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

    ChangePreviewHdl_Impl( nullptr );
}


IMPL_LINK_NOARG_TYPED(SvxLineTabPage, ChangeEdgeStyleHdl_Impl, ListBox&, void)
{
    ChangePreviewHdl_Impl( nullptr );
}


// fdo#43209

IMPL_LINK_NOARG_TYPED( SvxLineTabPage, ChangeCapStyleHdl_Impl, ListBox&, void )
{
    ChangePreviewHdl_Impl( nullptr );
}


IMPL_LINK_NOARG_TYPED(SvxLineTabPage, ClickInvisibleHdl_Impl, ListBox&, void)
{
    if( m_pLbLineStyle->GetSelectEntryPos() == 0 ) // invisible
    {
        if(!m_bSymbols)
            m_pBoxColor->Disable();

        m_pBoxWidth->Disable();

        if( m_pFlLineEnds->IsEnabled() )
        {
            m_pBoxStart->Disable();
            m_pBoxArrowStyles->Disable();


            m_pGridEdgeCaps->Disable();
        }
    }
    else
    {
        m_pBoxColor->Enable();
        m_pBoxWidth->Enable();

        if( m_pFlLineEnds->IsEnabled() )
        {
            m_pBoxArrowStyles->Enable();
            m_pGridEdgeCaps->Enable();
        }
    }
    ChangePreviewHdl_Impl( nullptr );
}


IMPL_LINK_TYPED( SvxLineTabPage, ChangeEndClickHdl_Impl, Button*, p, void )
{
    ChangeEndHdl_Impl(p);
}
IMPL_LINK_TYPED( SvxLineTabPage, ChangeEndListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ChangeEndHdl_Impl(&rListBox);
}
IMPL_LINK_TYPED( SvxLineTabPage, ChangeEndModifyHdl_Impl, Edit&, rEdit, void )
{
    ChangeEndHdl_Impl(&rEdit);
}
void SvxLineTabPage::ChangeEndHdl_Impl( void * p )
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

    ChangePreviewHdl_Impl( nullptr );
}


IMPL_LINK_NOARG_TYPED(SvxLineTabPage, ChangeTransparentHdl_Impl, Edit&, void)
{
    sal_uInt16 nVal = (sal_uInt16)m_pMtrTransparent->GetValue();
    XLineTransparenceItem aItem( nVal );

    m_rXLSet.Put( XLineTransparenceItem( aItem ) );

    FillXLSet_Impl();

    m_pCtlPreview->Invalidate();
}


void SvxLineTabPage::PointChanged( vcl::Window*, RECT_POINT eRcPt )
{
    m_eRP = eRcPt;
}


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
IMPL_LINK_TYPED( SvxLineTabPage, MenuCreateHdl_Impl, MenuButton *, pButton, void )
{
    InitSymbols(pButton);
}

// #58425# Symbols on a list (e.g. StarChart)
// Handler for menu button
IMPL_LINK_TYPED( SvxLineTabPage, GraphicHdl_Impl, MenuButton *, pButton, void )
{
    SymbolSelected(pButton);
}

IMPL_LINK_TYPED( SvxLineTabPage, SizeHdl_Impl, Edit&, rField, void)
{
    m_bNewSize = true;
    bool bWidth = &rField == m_pSymbolWidthMF;
    m_bLastWidthModified = bWidth;
    bool bRatio = m_pSymbolRatioCB->IsChecked();
    long nWidthVal = static_cast<long>(m_pSymbolWidthMF->Denormalize(m_pSymbolWidthMF->GetValue(FUNIT_100TH_MM)));
    long nHeightVal= static_cast<long>(m_pSymbolHeightMF->Denormalize(m_pSymbolHeightMF->GetValue(FUNIT_100TH_MM)));
    nWidthVal = OutputDevice::LogicToLogic(nWidthVal,MAP_100TH_MM,(MapUnit)m_ePoolUnit );
    nHeightVal = OutputDevice::LogicToLogic(nHeightVal,MAP_100TH_MM,(MapUnit)m_ePoolUnit);
    m_aSymbolSize = Size(nWidthVal,nHeightVal);
    double fSizeRatio = (double)1;

    if(bRatio)
    {
        if (m_aSymbolLastSize.Height() && m_aSymbolLastSize.Width())
            fSizeRatio = (double)m_aSymbolLastSize.Width() / m_aSymbolLastSize.Height();
    }

    //Size aSymbolSize(aSymbolLastSize);

    if(bWidth)
    {
        long nDelta = nWidthVal - m_aSymbolLastSize.Width();
        m_aSymbolSize.Width() = nWidthVal;
        if (bRatio)
        {
            m_aSymbolSize.Height() = m_aSymbolLastSize.Height() + (long)((double)nDelta / fSizeRatio);
            m_aSymbolSize.Height() = OutputDevice::LogicToLogic( m_aSymbolSize.Height(),(MapUnit)m_ePoolUnit, MAP_100TH_MM );
            m_pSymbolHeightMF->SetUserValue(m_pSymbolHeightMF->Normalize(m_aSymbolSize.Height()), FUNIT_100TH_MM);
        }
    }
    else
    {
        long nDelta = nHeightVal - m_aSymbolLastSize.Height();
        m_aSymbolSize.Height() = nHeightVal;
        if (bRatio)
        {
            m_aSymbolSize.Width() = m_aSymbolLastSize.Width() + (long)((double)nDelta * fSizeRatio);
            m_aSymbolSize.Width() = OutputDevice::LogicToLogic( m_aSymbolSize.Width(), (MapUnit)m_ePoolUnit, MAP_100TH_MM );
            m_pSymbolWidthMF->SetUserValue(m_pSymbolWidthMF->Normalize(m_aSymbolSize.Width()), FUNIT_100TH_MM);
        }
    }
    m_pCtlPreview->ResizeSymbol(m_aSymbolSize);
    m_aSymbolLastSize=m_aSymbolSize;
}
IMPL_LINK_TYPED( SvxLineTabPage, RatioHdl_Impl, Button*, pBox, void )
{
    if (static_cast<CheckBox*>(pBox)->IsChecked())
    {
        if (m_bLastWidthModified)
            SizeHdl_Impl(*m_pSymbolWidthMF);
        else
            SizeHdl_Impl(*m_pSymbolHeightMF);
    }
}


void SvxLineTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        FillListboxes();
    }
}

void SvxLineTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxColorListItem* pColorListItem = aSet.GetItem<SvxColorListItem>(SID_COLOR_TABLE, false);
    const SvxDashListItem* pDashListItem = aSet.GetItem<SvxDashListItem>(SID_DASH_LIST, false);
    const SvxLineEndListItem* pLineEndListItem = aSet.GetItem<SvxLineEndListItem>(SID_LINEEND_LIST, false);
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);
    const OfaPtrItem* pSdrObjListItem = aSet.GetItem<OfaPtrItem>(SID_OBJECT_LIST, false);
    const SfxTabDialogItem* pSymbolAttrItem = aSet.GetItem<SfxTabDialogItem>(SID_ATTR_SET, false);
    const SvxGraphicItem* pGraphicItem = aSet.GetItem<SvxGraphicItem>(SID_GRAPHIC, false);

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
        ShowSymbolControls(true);
        m_pSymbolList = static_cast<SdrObjList*>(pSdrObjListItem->GetValue());
        if (pSymbolAttrItem)
            m_pSymbolAttr = new SfxItemSet(pSymbolAttrItem->GetItemSet());
        if(pGraphicItem)
            m_aAutoSymbolGraphic = pGraphicItem->GetGraphic();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
