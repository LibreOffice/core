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

#define _SVX_TPLINE_CXX

#include <cuires.hrc>
#include "tabline.hrc"
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
#include <svx/xbitmap.hxx>
#include <unotools/localfilehelper.hxx>
#include "paragrph.hrc"
#include "sfx2/opengrf.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#define MAX_BMP_WIDTH   16
#define MAX_BMP_HEIGHT  16

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
    SvxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_LINE ), rInAttrs ),
    aFlLine             ( this, CUI_RES( FL_LINE ) ),
    aFtLineStyle        ( this, CUI_RES( FT_LINE_STYLE ) ),
    aLbLineStyle        ( this, CUI_RES( LB_LINE_STYLE ) ),
    aFtColor            ( this, CUI_RES( FT_COLOR ) ),
    aLbColor            ( this, CUI_RES( LB_COLOR ) ),
    aFtLineWidth        ( this, CUI_RES( FT_LINE_WIDTH ) ),
    aMtrLineWidth       ( this, CUI_RES( MTR_FLD_LINE_WIDTH ) ),
    aFtTransparent      ( this, CUI_RES( FT_TRANSPARENT ) ),
    aMtrTransparent     ( this, CUI_RES( MTR_LINE_TRANSPARENT ) ),
    aFlLineEnds         ( this, CUI_RES( FL_LINE_ENDS ) ),
    aLbStartStyle       ( this, CUI_RES( LB_START_STYLE ) ),
    aMtrStartWidth      ( this, CUI_RES( MTR_FLD_START_WIDTH ) ),
    aTsbCenterStart     ( this, CUI_RES( TSB_CENTER_START ) ),
    aFtLineEndsStyle    ( this, CUI_RES( FT_LINE_ENDS_STYLE ) ),
    aLbEndStyle         ( this, CUI_RES( LB_END_STYLE ) ),
    aFtLineEndsWidth    ( this, CUI_RES( FT_LINE_ENDS_WIDTH ) ),
    aMtrEndWidth        ( this, CUI_RES( MTR_FLD_END_WIDTH ) ),
    aTsbCenterEnd       ( this, CUI_RES( TSB_CENTER_END ) ),
    aCbxSynchronize     ( this, CUI_RES( CBX_SYNCHRONIZE ) ),
    aFLSeparator        ( this, CUI_RES( FL_SEPARATOR ) ),
    aCtlPreview         ( this, CUI_RES( CTL_PREVIEW ) ),

    // #116827#
    maFLEdgeStyle       ( this, CUI_RES( FL_EDGE_STYLE ) ),
    maFTEdgeStyle       ( this, CUI_RES( FT_EDGE_STYLE ) ),
    maLBEdgeStyle       ( this, CUI_RES( LB_EDGE_STYLE ) ),

    // LineCaps
    maFTCapStyle        ( this, CUI_RES( FT_CAP_STYLE ) ),
    maLBCapStyle        ( this, CUI_RES( LB_CAP_STYLE ) ),

    pSymbolList(NULL),
    bNewSize(false),
    nNumMenuGalleryItems(0),
    nSymbolType(SVX_SYMBOLTYPE_UNKNOWN), // unknown respectively unchanged
    pSymbolAttr(NULL),
    //#58425# Symbols on a line (e.g. StarChart)
    aFlSymbol           ( this, CUI_RES(FL_SYMBOL_FORMAT)),
    aSymbolMB           ( this, CUI_RES(MB_SYMBOL_BITMAP)),
    aSymbolWidthFT      ( this, CUI_RES(FT_SYMBOL_WIDTH)),
    aSymbolWidthMF      ( this, CUI_RES(MF_SYMBOL_WIDTH)),
    aSymbolHeightFT     ( this, CUI_RES(FT_SYMBOL_HEIGHT)),
    aSymbolHeightMF     ( this, CUI_RES(MF_SYMBOL_HEIGHT)),
    aSymbolRatioCB      ( this, CUI_RES(CB_SYMBOL_RATIO)),

    bLastWidthModified(sal_False),
    aSymbolLastSize(Size(0,0)),
    bSymbols(sal_False),

    rOutAttrs           ( rInAttrs ),
    bObjSelected( sal_False ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXLStyle            ( XLINE_DASH ),
    aXWidth             ( 1 ),
    aXDash              ( String(), XDash( XDASH_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( String(), COL_LIGHTRED ),
    aXLineAttr          ( pXPool ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
     pnLineEndListState( 0 ),
    pnDashListState( 0 ),
    pnColorListState( 0 ),
   nPageType           ( 0 )
{
    aLbEndStyle.SetAccessibleName(String(CUI_RES(STR_STYLE)));
    aLbStartStyle.SetAccessibleName(String(CUI_RES( STR_LB_START_STYLE ) ) );
    aMtrStartWidth.SetAccessibleName(String(CUI_RES( STR_MTR_FLD_START_WIDTH ) ) );
    aLbEndStyle.SetAccessibleName(String(CUI_RES( STR_LB_END_STYLE ) ) );
    aMtrEndWidth.SetAccessibleName(String(CUI_RES( STR_MTR_FLD_END_WIDTH ) ) );
    aTsbCenterStart.SetAccessibleName(String(CUI_RES( STR_CENTER_START ) ) );
    aTsbCenterEnd.SetAccessibleName(String(CUI_RES( STR_CENTER_END ) ) );

    FreeResource();

    aCtlPreview.SetAccessibleName(String(CUI_RES(STR_EXAMPLE)));

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
            aMtrLineWidth.SetSpinSize( 50 );
            aMtrStartWidth.SetSpinSize( 50 );
            aMtrEndWidth.SetSpinSize( 50 );
            break;

            case FUNIT_INCH:
            aMtrLineWidth.SetSpinSize( 2 );
            aMtrStartWidth.SetSpinSize( 2 );
            aMtrEndWidth.SetSpinSize( 2 );
            break;
            default: ;// prevent warning
    }
    SetFieldUnit( aMtrLineWidth, eFUnit );
    SetFieldUnit( aMtrStartWidth, eFUnit );
    SetFieldUnit( aMtrEndWidth, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    aLbLineStyle.SetSelectHdl(
        LINK( this, SvxLineTabPage, ClickInvisibleHdl_Impl ) );
    aLbColor.SetSelectHdl(
        LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    aMtrLineWidth.SetModifyHdl(
        LINK( this, SvxLineTabPage, ChangePreviewHdl_Impl ) );
    aMtrTransparent.SetModifyHdl(
        LINK( this, SvxLineTabPage, ChangeTransparentHdl_Impl ) );

    Link aStart = LINK( this, SvxLineTabPage, ChangeStartHdl_Impl );
    Link aEnd = LINK( this, SvxLineTabPage, ChangeEndHdl_Impl );
    aLbStartStyle.SetSelectHdl( aStart );
    aLbEndStyle.SetSelectHdl( aEnd );
    aMtrStartWidth.SetModifyHdl( aStart );
    aMtrEndWidth.SetModifyHdl( aEnd );
    aTsbCenterStart.SetClickHdl( aStart );
    aTsbCenterEnd.SetClickHdl( aEnd );

    // #116827#
    Link aEdgeStyle = LINK( this, SvxLineTabPage, ChangeEdgeStyleHdl_Impl );
    maLBEdgeStyle.SetSelectHdl( aEdgeStyle );

    // LineCaps
    Link aCapStyle = LINK( this, SvxLineTabPage, ChangeCapStyleHdl_Impl );
    maLBCapStyle.SetSelectHdl( aCapStyle );

    // Symbols on a line (eg star charts), MB-handler set
    aSymbolMB.SetSelectHdl(LINK(this, SvxLineTabPage, GraphicHdl_Impl));
    aSymbolMB.SetActivateHdl(LINK(this, SvxLineTabPage, MenuCreateHdl_Impl));
    aSymbolWidthMF.SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    aSymbolHeightMF.SetModifyHdl(LINK(this, SvxLineTabPage, SizeHdl_Impl));
    aSymbolRatioCB.SetClickHdl(LINK(this, SvxLineTabPage, RatioHdl_Impl));

    aSymbolRatioCB.Check(sal_True);
    ShowSymbolControls(sal_False);

    nActLineWidth = -1;
}

void SvxLineTabPage::ShowSymbolControls(sal_Bool bOn)
{
    // Symbols on a line (e.g. StarCharts), symbol-enable controls

    bSymbols=bOn;
    aSymbolWidthFT.Show(bOn);
    aSymbolWidthMF.Show(bOn);
    aSymbolHeightFT.Show(bOn);
    aSymbolHeightMF.Show(bOn);
    aFlSymbol.Show(bOn);
    aSymbolRatioCB.Show(bOn);
    aSymbolMB.Show(bOn);
    aCtlPreview.ShowSymbol(bOn);
}

SvxLineTabPage::~SvxLineTabPage()
{
    // Symbols on a line (e.g. StarCharts), dtor new!

    delete aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    if(pSymbolList)
        delete aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_SYMBOLS );

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
    aLbColor.Fill( pColorList );
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
            if ( i < aGrfBrushItems.size() ) {
                aGrfBrushItems.insert( aGrfBrushItems.begin() + i, pInfo );
            } else {
                aGrfBrushItems.push_back( pInfo );
            }
            const Graphic* pGraphic = pBrushItem->GetGraphic();

            if(pGraphic)
            {
                Bitmap aBitmap(pGraphic->GetBitmap());
                Size aSize(aBitmap.GetSizePixel());
                if(aSize.Width()  > MAX_BMP_WIDTH ||
                   aSize.Height() > MAX_BMP_HEIGHT)
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
        aSymbolMB.GetPopupMenu()->SetPopupMenu( MN_GALLERY, pPopup );

        if(aGrfNames.empty())
            aSymbolMB.GetPopupMenu()->EnableItem(MN_GALLERY, sal_False);
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

        aSymbolMB.GetPopupMenu()->SetPopupMenu( MN_SYMBOLS, pPopup );

        if(aGrfNames.empty())
            aSymbolMB.GetPopupMenu()->EnableItem(MN_SYMBOLS, sal_False);

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
        aCtlPreview.SetSymbol(&aSymbolGraphic,aSymbolSize);
    }
    else
    {
        aSymbolGraphic=Graphic();
        aCtlPreview.SetSymbol(NULL,aSymbolSize);
        bEnable = false;
    }
    aSymbolLastSize=aSymbolSize;
    SetMetricValue(aSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
    SetMetricValue(aSymbolHeightMF, aSymbolSize.Height(), ePoolUnit);
    aSymbolRatioCB.Enable(bEnable);
    aSymbolHeightFT.Enable(bEnable);
    aSymbolWidthFT.Enable(bEnable);
    aSymbolWidthMF.Enable(bEnable);
    aSymbolHeightMF.Enable(bEnable);
    aCtlPreview.Invalidate();
}

void SvxLineTabPage::FillListboxes()
{
    // Line styles
    sal_uInt16 nOldSelect = aLbLineStyle.GetSelectEntryPos();
    aLbLineStyle.FillStyles();
    aLbLineStyle.Fill( pDashList );
    aLbLineStyle.SelectEntryPos( nOldSelect );

    // Line end style
    String sNone( SVX_RES( RID_SVXSTR_NONE ) );
    nOldSelect = aLbStartStyle.GetSelectEntryPos();
    aLbStartStyle.Clear();
    aLbStartStyle.InsertEntry( sNone );
    aLbStartStyle.Fill( pLineEndList );
    aLbStartStyle.SelectEntryPos( nOldSelect );
    nOldSelect = aLbEndStyle.GetSelectEntryPos();
    aLbEndStyle.Clear();
    aLbEndStyle.InsertEntry( sNone );
    aLbEndStyle.Fill( pLineEndList, sal_False );
    aLbEndStyle.SelectEntryPos( nOldSelect );
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
                pDashList = ( (SvxLineTabDialog*) GetParentDialog() )->
                                        GetNewDashList();
            *pnDashListState = CT_NONE;

            // Style list
            nPos = aLbLineStyle.GetSelectEntryPos();

            aLbLineStyle.Clear();
            aLbLineStyle.InsertEntry(
                SVX_RESSTR( RID_SVXSTR_INVISIBLE ) );
            aLbLineStyle.InsertEntry(
                SVX_RESSTR( RID_SVXSTR_SOLID ) );
            aLbLineStyle.Fill( pDashList );
            nCount = aLbLineStyle.GetEntryCount();

            if ( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                aLbLineStyle.SelectEntryPos( 0 );
            else
                aLbLineStyle.SelectEntryPos( nPos );
            // SelectStyleHdl_Impl( this );
        }

        INetURLObject   aDashURL( pDashList->GetPath() );

        aDashURL.Append( pDashList->GetName() );
        DBG_ASSERT( aDashURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        // LineEnd list
        if( ( *pnLineEndListState & CT_MODIFIED ) ||
            ( *pnLineEndListState & CT_CHANGED ) )
        {
            if( *pnLineEndListState & CT_CHANGED )
                pLineEndList = ( (SvxLineTabDialog*) GetParentDialog() )->
                                        GetNewLineEndList();
            *pnLineEndListState = CT_NONE;

            nPos = aLbLineStyle.GetSelectEntryPos();
            String sNone( SVX_RES( RID_SVXSTR_NONE ) );
            aLbStartStyle.Clear();
            aLbStartStyle.InsertEntry( sNone );

            aLbStartStyle.Fill( pLineEndList );
            nCount = aLbStartStyle.GetEntryCount();
            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                aLbStartStyle.SelectEntryPos( 0 );
            else
                aLbStartStyle.SelectEntryPos( nPos );

            aLbEndStyle.Clear();
            aLbEndStyle.InsertEntry( sNone );

            aLbEndStyle.Fill( pLineEndList, sal_False );
            nCount = aLbEndStyle.GetEntryCount();

            if( nCount == 0 )
                ; // This case should never occur
            else if( nCount <= nPos )
                aLbEndStyle.SelectEntryPos( 0 );
            else
                aLbEndStyle.SelectEntryPos( nPos );
        }
        INetURLObject aLineURL( pLineEndList->GetPath() );

        aLineURL.Append( pLineEndList->GetName() );
        DBG_ASSERT( aLineURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        // Evaluate if another TabPage set another fill type
        if( aLbLineStyle.GetSelectEntryPos() != 0 )
        {
            if( nPageType == 2 ) // 1
            {
                aLbLineStyle.SelectEntryPos( *pPosDashLb + 2 ); // +2 due to SOLID and INVLISIBLE
                ChangePreviewHdl_Impl( this );
            }
            if( nPageType == 3 )
            {
                aLbStartStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 due to SOLID
                aLbEndStyle.SelectEntryPos( *pPosLineEndLb + 1 );// +1 due to SOLID
                ChangePreviewHdl_Impl( this );
            }
        }

            // ColorList
            if( *pnColorListState )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxLineTabDialog*) GetParentDialog() )->GetNewColorList();
                // aLbColor
                sal_uInt16 nColorPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorList );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nColorPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( nColorPos );

                ChangePreviewHdl_Impl( this );
            }

        nPageType = 0;
    }
    // Page does not yet exist in the ctor, that's why we do it here!

    else if ( nDlgType == 1100 ||
              nDlgType == 1101 )
    {
        aFtLineEndsStyle.Hide();
        aFtLineEndsWidth.Hide();
        aLbStartStyle.Hide();
        aMtrStartWidth.Hide();
        aTsbCenterStart.Hide();
        aLbEndStyle.Hide();
        aMtrEndWidth.Hide();
        aTsbCenterEnd.Hide();
        aCbxSynchronize.Hide();
        aFlLineEnds.Hide();

        // #116827#
        maFLEdgeStyle.Hide();
        maFTEdgeStyle.Hide();
        maLBEdgeStyle.Hide();

        // LineCaps
        maFTCapStyle.Hide();
        maLBCapStyle.Hide();
    }
}

// -----------------------------------------------------------------------

int SvxLineTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( nDlgType == 0 ) // Line dialog
    {
        nPageType = 1; // possibly for extensions
        *pPosDashLb = aLbLineStyle.GetSelectEntryPos() - 2;// First entry SOLID!!!
        sal_uInt16 nPos = aLbStartStyle.GetSelectEntryPos();
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
        nPos = aLbLineStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbLineStyle.GetSavedValue() )
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
                    XLineDashItem aDashItem( aLbLineStyle.GetSelectEntry(),
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
    if( aMtrLineWidth.GetText() != aMtrLineWidth.GetSavedValue() )
    {
        XLineWidthItem aItem( GetCoreValue( aMtrLineWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEWIDTH );
        if ( !pOld || !( *(const XLineWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    // Width line start
    if( aMtrStartWidth.GetText() != aMtrStartWidth.GetSavedValue() )
    {
        XLineStartWidthItem aItem( GetCoreValue( aMtrStartWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTWIDTH );
        if ( !pOld || !( *(const XLineStartWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    // Width line end
    if( aMtrEndWidth.GetText() != aMtrEndWidth.GetSavedValue() )
    {
        XLineEndWidthItem aItem( GetCoreValue( aMtrEndWidth, ePoolUnit ) );
        pOld = GetOldItem( rAttrs, XATTR_LINEENDWIDTH );
        if ( !pOld || !( *(const XLineEndWidthItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }

    // Line color
    if( aLbColor.GetSelectEntryPos() != aLbColor.GetSavedValue() )
    {
        XLineColorItem aItem( aLbColor.GetSelectEntry(),
                              aLbColor.GetSelectEntryColor() );
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
        nPos = aLbStartStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbStartStyle.GetSavedValue() )
        {
            XLineStartItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineStartItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineStartItem( aLbStartStyle.GetSelectEntry(),
                            pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
            pOld = GetOldItem( rAttrs, XATTR_LINESTART );
            if( pItem &&
                ( !pOld || !( *(const XLineEndItem*)pOld == *pItem ) ) )
            {
                rAttrs.Put( *pItem );
                bModified = sal_True;
            }
            delete pItem;
        }
        // Line end
        nPos = aLbEndStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbEndStyle.GetSavedValue() )
        {
            XLineEndItem* pItem = NULL;
            if( nPos == 0 )
                pItem = new XLineEndItem();
            else if( pLineEndList->Count() > (long) ( nPos - 1 ) )
                pItem = new XLineEndItem( aLbEndStyle.GetSelectEntry(),
                            pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
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
    TriState eState = aTsbCenterStart.GetState();
    if( eState != aTsbCenterStart.GetSavedValue() )
    {
        XLineStartCenterItem aItem( sal::static_int_cast< sal_Bool >( eState ) );
        pOld = GetOldItem( rAttrs, XATTR_LINESTARTCENTER );
        if ( !pOld || !( *(const XLineStartCenterItem*)pOld == aItem ) )
        {
            rAttrs.Put( aItem );
            bModified = sal_True;
        }
    }
    eState = aTsbCenterEnd.GetState();
    if( eState != aTsbCenterEnd.GetSavedValue() )
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
    sal_uInt16 nVal = (sal_uInt16)aMtrTransparent.GetValue();
    if( nVal != (sal_uInt16)aMtrTransparent.GetSavedValue().toInt32() )
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
    nPos = maLBEdgeStyle.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && nPos != maLBEdgeStyle.GetSavedValue() )
    {
        XLineJointItem* pNew = 0L;

        switch(nPos)
        {
            case 0: // Rounded, default
            {
                pNew = new XLineJointItem(XLINEJOINT_ROUND);
                break;
            }
            case 1: // - none -
            {
                pNew = new XLineJointItem(XLINEJOINT_NONE);
                break;
            }
            case 2: // Miter
            {
                pNew = new XLineJointItem(XLINEJOINT_MITER);
                break;
            }
            case 3: // Bevel
            {
                pNew = new XLineJointItem(XLINEJOINT_BEVEL);
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
    nPos = maLBCapStyle.GetSelectEntryPos();
    if( LISTBOX_ENTRY_NOTFOUND != nPos && nPos != maLBCapStyle.GetSavedValue() )
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

    if( aLbLineStyle.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    }
    else if( aLbLineStyle.IsEntryPosSelected( 0 ) )
        rXLSet.Put( XLineStyleItem( XLINE_NONE ) );
    else if( aLbLineStyle.IsEntryPosSelected( 1 ) )
        rXLSet.Put( XLineStyleItem( XLINE_SOLID ) );
    else
    {
        rXLSet.Put( XLineStyleItem( XLINE_DASH ) );

        nPos = aLbLineStyle.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rXLSet.Put( XLineDashItem( aLbLineStyle.GetSelectEntry(),
                            pDashList->GetDash( nPos - 2 )->GetDash() ) );
        }
    }

    nPos = aLbStartStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineStartItem() );
        else
            rXLSet.Put( XLineStartItem( aLbStartStyle.GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }
    nPos = aLbEndStyle.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( nPos == 0 )
            rXLSet.Put( XLineEndItem() );
        else
            rXLSet.Put( XLineEndItem( aLbEndStyle.GetSelectEntry(),
                        pLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() ) );
    }

    // #116827#
    nPos = maLBEdgeStyle.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        switch(nPos)
        {
            case 0: // Rounded, default
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_ROUND));
                break;
            }
            case 1: // - none -
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_NONE));
                break;
            }
            case 2: // Miter
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_MITER));
                break;
            }
            case 3: // Bevel
            {
                rXLSet.Put(XLineJointItem(XLINEJOINT_BEVEL));
                break;
            }
        }
    }

    // LineCaps
    nPos = maLBCapStyle.GetSelectEntryPos();
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

    rXLSet.Put( XLineStartWidthItem( GetCoreValue( aMtrStartWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineEndWidthItem( GetCoreValue( aMtrEndWidth, ePoolUnit ) ) );

    rXLSet.Put( XLineWidthItem( GetCoreValue( aMtrLineWidth, ePoolUnit ) ) );
    rXLSet.Put( XLineColorItem( aLbColor.GetSelectEntry(),
                                    aLbColor.GetSelectEntryColor() ) );

    // Centered line end
    if( aTsbCenterStart.GetState() == STATE_CHECK )
        rXLSet.Put( XLineStartCenterItem( sal_True ) );
    else if( aTsbCenterStart.GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineStartCenterItem( sal_False ) );

    if( aTsbCenterEnd.GetState() == STATE_CHECK )
        rXLSet.Put( XLineEndCenterItem( sal_True ) );
    else if( aTsbCenterEnd.GetState() == STATE_NOCHECK )
        rXLSet.Put( XLineEndCenterItem( sal_False ) );

    // Transparency
    sal_uInt16 nVal = (sal_uInt16)aMtrTransparent.GetValue();
    rXLSet.Put( XLineTransparenceItem( nVal ) );

    // #116827#
    aCtlPreview.SetLineAttributes(aXLineAttr.GetItemSet());

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
                    // bitmap size, which is indepedent from specific glyph
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

    aSymbolRatioCB.Enable(bEnable);
    aSymbolHeightFT.Enable(bEnable);
    aSymbolWidthFT.Enable(bEnable);
    aSymbolWidthMF.Enable(bEnable);
    aSymbolHeightMF.Enable(bEnable);
    if(bPrevSym)
    {
        SetMetricValue(aSymbolWidthMF,  aSymbolSize.Width(), ePoolUnit);
        SetMetricValue(aSymbolHeightMF, aSymbolSize.Height(),ePoolUnit);
        aCtlPreview.SetSymbol(&aSymbolGraphic,aSymbolSize);
        aSymbolLastSize=aSymbolSize;
    }

    if( rAttrs.GetItemState( XATTR_LINESTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXLS = (XLineStyle) ( ( const XLineStyleItem& ) rAttrs.Get( XATTR_LINESTYLE ) ).GetValue();

        switch( eXLS )
        {
            case XLINE_NONE:
                aLbLineStyle.SelectEntryPos( 0 );
                break;
            case XLINE_SOLID:
                aLbLineStyle.SelectEntryPos( 1 );
                break;

            case XLINE_DASH:
                aLbLineStyle.SetNoSelection();
                aLbLineStyle.SelectEntry( ( ( const XLineDashItem& ) rAttrs.
                                Get( XATTR_LINEDASH ) ).GetName() );
                break;

            default:
                break;
        }
    }
    else
    {
        aLbLineStyle.SetNoSelection();
    }

    // Line strength
    if( rAttrs.GetItemState( XATTR_LINEWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrLineWidth, ( ( const XLineWidthItem& ) rAttrs.
                            Get( XATTR_LINEWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrLineWidth.SetText( String() );

    // Line color
    aLbColor.SetNoSelection();

    if ( rAttrs.GetItemState( XATTR_LINECOLOR ) != SFX_ITEM_DONTCARE )
    {
        Color aCol = ( ( const XLineColorItem& ) rAttrs.Get( XATTR_LINECOLOR ) ).GetColorValue();
        aLbColor.SelectEntry( aCol );
        if( aLbColor.GetSelectEntryCount() == 0 )
        {
            aLbColor.InsertEntry( aCol, String() );
            aLbColor.SelectEntry( aCol );
        }
    }

    // Line start
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTART ) == SFX_ITEM_DEFAULT )
    {
        aLbStartStyle.Disable();
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
                aLbStartStyle.SelectEntryPos((sal_uInt16)a + 1);
                bSelected = sal_True;
            }
        }

        if(!bSelected)
            aLbStartStyle.SelectEntryPos( 0 );
    }
    else
    {
        aLbStartStyle.SetNoSelection();
    }

    // Line end
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEEND ) == SFX_ITEM_DEFAULT )
    {
        aLbEndStyle.Disable();
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
                aLbEndStyle.SelectEntryPos((sal_uInt16)a + 1);
                bSelected = sal_True;
            }
        }

        if(!bSelected)
            aLbEndStyle.SelectEntryPos( 0 );
    }
    else
    {
        aLbEndStyle.SetNoSelection();
    }

    // Line start strength
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) == SFX_ITEM_DEFAULT )
    {
        aMtrStartWidth.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrStartWidth, ( ( const XLineStartWidthItem& ) rAttrs.
                            Get( XATTR_LINESTARTWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrStartWidth.SetText( String() );

    // Line end strength
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEENDWIDTH ) == SFX_ITEM_DEFAULT )
    {
        aMtrEndWidth.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDWIDTH ) != SFX_ITEM_DONTCARE )
    {
        SetMetricValue( aMtrEndWidth, ( ( const XLineEndWidthItem& ) rAttrs.
                            Get( XATTR_LINEENDWIDTH ) ).GetValue(), ePoolUnit );
    }
    else
        aMtrEndWidth.SetText( String() );

    // Centered line end (start)
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINESTARTCENTER ) == SFX_ITEM_DEFAULT )
    {
        aTsbCenterStart.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINESTARTCENTER ) != SFX_ITEM_DONTCARE )
    {
        aTsbCenterStart.EnableTriState( sal_False );

        if( ( ( const XLineStartCenterItem& ) rAttrs.Get( XATTR_LINESTARTCENTER ) ).GetValue() )
            aTsbCenterStart.SetState( STATE_CHECK );
        else
            aTsbCenterStart.SetState( STATE_NOCHECK );
    }
    else
    {
        aTsbCenterStart.SetState( STATE_DONTKNOW );
    }

    // Centered line end (end)
    if( bObjSelected &&
        rAttrs.GetItemState( XATTR_LINEENDCENTER ) == SFX_ITEM_DEFAULT )
    {
        aTsbCenterEnd.Disable();
    }
    else if( rAttrs.GetItemState( XATTR_LINEENDCENTER ) != SFX_ITEM_DONTCARE )
    {
        aTsbCenterEnd.EnableTriState( sal_False );

        if( ( ( const XLineEndCenterItem& ) rAttrs.Get( XATTR_LINEENDCENTER ) ).GetValue() )
            aTsbCenterEnd.SetState( STATE_CHECK );
        else
            aTsbCenterEnd.SetState( STATE_NOCHECK );
    }
    else
    {
        aTsbCenterEnd.SetState( STATE_DONTKNOW );
    }

    // Transparency
    if( rAttrs.GetItemState( XATTR_LINETRANSPARENCE ) != SFX_ITEM_DONTCARE )
    {
        sal_uInt16 nTransp = ( ( const XLineTransparenceItem& ) rAttrs.
                                Get( XATTR_LINETRANSPARENCE ) ).GetValue();
        aMtrTransparent.SetValue( nTransp );
        ChangeTransparentHdl_Impl( NULL );
    }
    else
        aMtrTransparent.SetText( String() );

    if( !aLbStartStyle.IsEnabled()  &&
        !aLbEndStyle.IsEnabled()    &&
        !aMtrStartWidth.IsEnabled() &&
        !aMtrEndWidth.IsEnabled()   &&
        !aTsbCenterStart.IsEnabled()&&
        !aTsbCenterEnd.IsEnabled() )
    {
        aCbxSynchronize.Disable();
        aFtLineEndsStyle.Disable();
        aFtLineEndsWidth.Disable();
        aFlLineEnds.Disable();
    }

    // Synchronize
    // We get the value from the INI file now
    String aStr = GetUserData();
    aCbxSynchronize.Check( (sal_Bool)aStr.ToInt32() );

    // #116827#
    if(bObjSelected && SFX_ITEM_DEFAULT == rAttrs.GetItemState(XATTR_LINEJOINT))
    {
        maFTEdgeStyle.Disable();
        maLBEdgeStyle.Disable();
    }
    else if(SFX_ITEM_DONTCARE != rAttrs.GetItemState(XATTR_LINEJOINT))
    {
        XLineJoint eLineJoint = ((const XLineJointItem&)(rAttrs.Get(XATTR_LINEJOINT))).GetValue();

        switch(eLineJoint)
        {
            case XLINEJOINT_ROUND : maLBEdgeStyle.SelectEntryPos(0); break;
            case XLINEJOINT_NONE : maLBEdgeStyle.SelectEntryPos(1); break;
            case XLINEJOINT_MITER : maLBEdgeStyle.SelectEntryPos(2); break;
            case XLINEJOINT_BEVEL : maLBEdgeStyle.SelectEntryPos(3); break;
            case XLINEJOINT_MIDDLE : break;
        }
    }
    else
    {
        maLBEdgeStyle.SetNoSelection();
    }

    // fdo#43209
    if(bObjSelected && SFX_ITEM_DEFAULT == rAttrs.GetItemState(XATTR_LINECAP))
    {
        maFTCapStyle.Disable();
        maLBCapStyle.Disable();
    }
    else if(SFX_ITEM_DONTCARE != rAttrs.GetItemState(XATTR_LINECAP))
    {
        const com::sun::star::drawing::LineCap eLineCap(((const XLineCapItem&)(rAttrs.Get(XATTR_LINECAP))).GetValue());

        switch(eLineCap)
        {
            case com::sun::star::drawing::LineCap_ROUND: maLBCapStyle.SelectEntryPos(1); break;
            case com::sun::star::drawing::LineCap_SQUARE : maLBCapStyle.SelectEntryPos(2); break;
            default /*com::sun::star::drawing::LineCap_BUTT*/: maLBCapStyle.SelectEntryPos(0); break;
        }
    }
    else
    {
        maLBCapStyle.SetNoSelection();
    }

    // Save values
    aLbLineStyle.SaveValue();
    aMtrLineWidth.SaveValue();
    aLbColor.SaveValue();
    aLbStartStyle.SaveValue();
    aLbEndStyle.SaveValue();
    aMtrStartWidth.SaveValue();
    aMtrEndWidth.SaveValue();
    aTsbCenterStart.SaveValue();
    aTsbCenterEnd.SaveValue();
    aMtrTransparent.SaveValue();

    // #116827#
    maLBEdgeStyle.SaveValue();

    // LineCaps
    maLBCapStyle.SaveValue();

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
    if(pCntrl == &aMtrLineWidth)
    {
        // Line width and start end width
        sal_Int32 nNewLineWidth = GetCoreValue( aMtrLineWidth, ePoolUnit );
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
            sal_Int32 nValAct = GetCoreValue( aMtrStartWidth, ePoolUnit );
            sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( aMtrStartWidth, nValNew, ePoolUnit );

            nValAct = GetCoreValue( aMtrEndWidth, ePoolUnit );
            nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
            if(nValNew < 0)
                nValNew = 0;
            SetMetricValue( aMtrEndWidth, nValNew, ePoolUnit );
        }

        // Remember current value
        nActLineWidth = nNewLineWidth;
    }

    FillXLSet_Impl();
    aCtlPreview.Invalidate();

    // Make transparency accessible accordingly
    if( aLbLineStyle.GetSelectEntryPos() == 0 ) // invisible
    {
        aFtTransparent.Disable();
        aMtrTransparent.Disable();
    }
    else
    {
        aFtTransparent.Enable();
        aMtrTransparent.Enable();
    }

    const bool bHasLineStart = aLbStartStyle.GetSelectEntryPos() != 0;
    const bool bHasLineEnd = aLbEndStyle.GetSelectEntryPos() != 0;

    aFtLineEndsWidth.Enable( bHasLineStart || bHasLineEnd );
    aMtrStartWidth.Enable( bHasLineStart );
    aTsbCenterStart.Enable( bHasLineStart );
    aMtrEndWidth.Enable( bHasLineEnd );
    aTsbCenterEnd.Enable( bHasLineEnd );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeStartHdl_Impl, void *, p )
{
    if( aCbxSynchronize.IsChecked() )
    {
        if( p == &aMtrStartWidth )
            aMtrEndWidth.SetValue( aMtrStartWidth.GetValue() );
        if( p == &aLbStartStyle )
            aLbEndStyle.SelectEntryPos( aLbStartStyle.GetSelectEntryPos() );
        if( p == &aTsbCenterStart )
            aTsbCenterEnd.SetState( aTsbCenterStart.GetState() );
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
    if( aLbLineStyle.GetSelectEntryPos() == 0 ) // invisible
    {
        aFtColor.Disable();
        if(!bSymbols)
            aLbColor.Disable();
        aFtLineWidth.Disable();
        aMtrLineWidth.Disable();

        if( aFlLineEnds.IsEnabled() )
        {
            aFtLineEndsStyle.Disable();
            aFtLineEndsWidth.Disable();
            aLbStartStyle.Disable();
            aMtrStartWidth.Disable();
            aTsbCenterStart.Disable();
            aLbEndStyle.Disable();
            aMtrEndWidth.Disable();
            aTsbCenterEnd.Disable();
            aCbxSynchronize.Disable();

            // #116827#
            maFTEdgeStyle.Disable();
            maLBEdgeStyle.Disable();

            // LineCaps
            maFTCapStyle.Disable();
            maLBCapStyle.Disable();
        }
    }
    else
    {
        aFtColor.Enable();
        aLbColor.Enable();
        aFtLineWidth.Enable();
        aMtrLineWidth.Enable();

        if( aFlLineEnds.IsEnabled() )
        {
            aFtLineEndsStyle.Enable();
            aFtLineEndsWidth.Enable();
            aLbStartStyle.Enable();
            aMtrStartWidth.Enable();
            aTsbCenterStart.Enable();
            aLbEndStyle.Enable();
            aMtrEndWidth.Enable();
            aTsbCenterEnd.Enable();
            aCbxSynchronize.Enable();

            // #116827#
            maFTEdgeStyle.Enable();
            maLBEdgeStyle.Enable();

            // LineCaps
            maFTCapStyle.Enable();
            maLBCapStyle.Enable();
        }
    }
    ChangePreviewHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxLineTabPage, ChangeEndHdl_Impl, void *, p )
{
    if( aCbxSynchronize.IsChecked() )
    {
        if( p == &aMtrEndWidth )
            aMtrStartWidth.SetValue( aMtrEndWidth.GetValue() );
        if( p == &aLbEndStyle )
            aLbStartStyle.SelectEntryPos( aLbEndStyle.GetSelectEntryPos() );
        if( p == &aTsbCenterEnd )
            aTsbCenterStart.SetState( aTsbCenterEnd.GetState() );
    }

    ChangePreviewHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLineTabPage, ChangeTransparentHdl_Impl)
{
    sal_uInt16 nVal = (sal_uInt16)aMtrTransparent.GetValue();
    XLineTransparenceItem aItem( nVal );

    rXLSet.Put( XLineTransparenceItem( aItem ) );

    // #116827#
    FillXLSet_Impl();

    aCtlPreview.Invalidate();

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
    OUString aStrUserData = OUString::valueOf( (sal_Int32) aCbxSynchronize.IsChecked() );
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
    PopupMenu* pPopup = pThis->aSymbolMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

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
            if(aSize.Width()  > MAX_BMP_WIDTH ||
               aSize.Height() > MAX_BMP_HEIGHT)
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
    sal_Bool bWidth = (sal_Bool)(pField == &aSymbolWidthMF);
    bLastWidthModified = bWidth;
    sal_Bool bRatio = aSymbolRatioCB.IsChecked();
    long nWidthVal = static_cast<long>(aSymbolWidthMF.Denormalize(aSymbolWidthMF.GetValue(FUNIT_100TH_MM)));
    long nHeightVal= static_cast<long>(aSymbolHeightMF.Denormalize(aSymbolHeightMF.GetValue(FUNIT_100TH_MM)));
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
            aSymbolHeightMF.SetUserValue(aSymbolHeightMF.Normalize(aSymbolSize.Height()), FUNIT_100TH_MM);
        }
    }
    else
    {
        long nDelta = nHeightVal - aSymbolLastSize.Height();
        aSymbolSize.Height() = nHeightVal;
        if (bRatio)
        {
            aSymbolSize.Width() = aSymbolLastSize.Width() + (long)((double)nDelta * fSizeRatio);
            aSymbolSize.Width() = OutputDevice::LogicToLogic( aSymbolSize.Width(),
                                (MapUnit)ePoolUnit, MAP_100TH_MM );
            aSymbolWidthMF.SetUserValue(aSymbolWidthMF.Normalize(aSymbolSize.Width()), FUNIT_100TH_MM);
        }
    }
    aCtlPreview.ResizeSymbol(aSymbolSize);
    aSymbolLastSize=aSymbolSize;
    return 0;
}
IMPL_LINK( SvxLineTabPage, RatioHdl_Impl, CheckBox *, pBox )
{
    if (pBox->IsChecked())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(&aSymbolWidthMF);
        else
            SizeHdl_Impl(&aSymbolHeightMF);
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
