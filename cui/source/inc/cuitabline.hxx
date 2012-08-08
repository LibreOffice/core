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
#ifndef _CUI_TAB_LINE_HXX
#define _CUI_TAB_LINE_HXX

#include <vector>
#include <svx/tabline.hxx>

class SvxLineTabDialog : public SfxTabDialog
{
private:
    SdrModel*           pDrawModel;
    const SdrObject*    pObj;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;
    XColorListRef         mpNewColorList;
    XDashListRef          pDashList;
    XDashListRef          pNewDashList;
    XLineEndListRef       pLineEndList;
    XLineEndListRef       pNewLineEndList;
    sal_Bool            bObjSelected;

    ChangeType          nLineEndListState;
    ChangeType          nDashListState;
    ChangeType          mnColorListState;

    sal_uInt16          nPageType;
    sal_uInt16          nDlgType;
    sal_uInt16          nPosDashLb;
    sal_uInt16          nPosLineEndLb;
    sal_uInt16          mnPos;
    sal_Bool            mbAreaTP;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

protected:
    virtual short       Ok();
#ifdef _SVX_TABLINE_CXX
    DECL_LINK( CancelHdlImpl, void * );
    void                SavePalettes();
#endif

public:
    SvxLineTabDialog( Window* pParent, const SfxItemSet* pAttr,
                      SdrModel* pModel, const SdrObject* pObj = NULL,
                      sal_Bool bHasObj = sal_True );
    ~SvxLineTabDialog();

    void                SetNewDashList( XDashListRef pInLst)
                        { pNewDashList = pInLst; }
    XDashListRef          GetNewDashList() const { return pNewDashList; }
    XDashListRef          GetDashList() const { return pDashList; }

    void                SetNewLineEndList( XLineEndListRef pInLst)
                        { pNewLineEndList = pInLst; }
    XLineEndListRef       GetNewLineEndList() const { return pNewLineEndList; }
    XLineEndListRef       GetLineEndList() const { return pLineEndList; }

    void                SetNewColorList( XColorListRef pColTab ) { mpNewColorList = pColTab; }
    XColorListRef       GetNewColorList() const { return mpNewColorList; }
    XColorListRef       GetColorList() const { return pColorList; }
};

/*************************************************************************/

struct SvxBmpItemInfo;
typedef ::std::vector< SvxBmpItemInfo* > SvxBmpItemInfoList;

class SvxLineTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
private:
    FixedLine           aFlLine;
    FixedText           aFtLineStyle;
    LineLB              aLbLineStyle;
    FixedText           aFtColor;
    ColorLB             aLbColor;
    FixedText           aFtLineWidth;
    MetricField         aMtrLineWidth;
    FixedText           aFtTransparent;
    MetricField         aMtrTransparent;
    FixedLine           aFlLineEnds;
    LineEndLB           aLbStartStyle;
    MetricField         aMtrStartWidth;
    TriStateBox         aTsbCenterStart;
    FixedText           aFtLineEndsStyle;
    LineEndLB           aLbEndStyle;
    FixedText           aFtLineEndsWidth;
    MetricField         aMtrEndWidth;
    TriStateBox         aTsbCenterEnd;
    CheckBox            aCbxSynchronize;
    FixedLine           aFLSeparator;
    SvxXLinePreview     aCtlPreview;

    // #116827#
    FixedLine           maFLEdgeStyle;
    FixedText           maFTEdgeStyle;
    LineEndLB           maLBEdgeStyle;

    //#58425# symbols on a line (e. g. StarChart) ->
    SdrObjList*         pSymbolList; //a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview. The list position is to be used cyclic.
    bool                bNewSize;
    Graphic             aAutoSymbolGraphic; //a graphic to be displayed in the preview in case that an automatic symbol is choosen
    long                nNumMenuGalleryItems;
    long                nSymbolType;
    SfxItemSet*         pSymbolAttr; //attributes for the shown symbols; only necessary if not equal to line properties
    FixedLine           aFlSymbol;
    MenuButton          aSymbolMB;
    FixedText           aSymbolWidthFT;
    MetricField         aSymbolWidthMF;
    FixedText           aSymbolHeightFT;
    MetricField         aSymbolHeightMF;
    CheckBox            aSymbolRatioCB;
    std::vector<rtl::OUString> aGrfNames;
    SvxBmpItemInfoList  aGrfBrushItems;
    sal_Bool            bLastWidthModified;
    Size                aSymbolLastSize;
    Graphic             aSymbolGraphic;
    Size                aSymbolSize;
    sal_Bool            bSymbols;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;
    sal_Bool            bObjSelected;

    XOutdevItemPool*    pXPool;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineDashItem       aXDash;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XColorListRef         pColorList;
    XDashListRef          pDashList;
    XLineEndListRef       pLineEndList;

    ChangeType*         pnLineEndListState;
    ChangeType*         pnDashListState;
    ChangeType*         pnColorListState;
    sal_uInt16          nPageType;
    sal_uInt16          nDlgType;
    sal_uInt16*         pPosDashLb;
    sal_uInt16*         pPosLineEndLb;

    SfxMapUnit          ePoolUnit;

    // #63083#
    sal_Int32           nActLineWidth;

    // handler for gallery popup menu button + size
    DECL_LINK( GraphicHdl_Impl, MenuButton * );
    DECL_LINK( MenuCreateHdl_Impl, MenuButton * );
    DECL_STATIC_LINK( SvxLineTabPage, GraphicArrivedHdl_Impl, SvxBrushItem* );
    DECL_LINK( SizeHdl_Impl, MetricField * );
    DECL_LINK( RatioHdl_Impl, CheckBox * );

#ifdef _SVX_TPLINE_CXX
    DECL_LINK( ClickInvisibleHdl_Impl, void * );
    DECL_LINK( ChangeStartHdl_Impl, void * );
    DECL_LINK( ChangeEndHdl_Impl, void * );
    DECL_LINK( ChangePreviewHdl_Impl, void * );
    DECL_LINK( ChangeTransparentHdl_Impl, void * );

    // #116827#
    DECL_LINK( ChangeEdgeStyleHdl_Impl, void * );

    sal_Bool FillXLSet_Impl();
#endif

    void InitSymbols(MenuButton* pButton);
    void SymbolSelected(MenuButton* pButton);
    void FillListboxes();
public:

    void ShowSymbolControls(sal_Bool bOn);

    SvxLineTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineTabPage();

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  sal_uInt16*    GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet& );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    virtual void FillUserData();

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetDashList( XDashListRef pDshLst ) { pDashList = pDshLst; }
    void    SetLineEndList( XLineEndListRef pLneEndLst) { pLineEndList = pLneEndLst; }
    void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosDashLb( sal_uInt16* pInPos ) { pPosDashLb = pInPos; }
    void    SetPosLineEndLb( sal_uInt16* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }
    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void PageCreated (SfxAllItemSet aSet);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************/

class SvxLineDefTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
private:
    FixedLine           aFlDefinition;
    FixedText           aFTLinestyle;
    LineLB              aLbLineStyles;
    FixedText           aFtType;
    ListBox             aLbType1;
    ListBox             aLbType2;
    FixedText           aFtNumber;
    NumericField        aNumFldNumber1;
    NumericField        aNumFldNumber2;
    FixedText           aFtLength;
    MetricField         aMtrLength1;
    MetricField         aMtrLength2;
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    CheckBox            aCbxSynchronize;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;
    SvxXLinePreview     aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    XDash               aDash;
    sal_Bool            bObjSelected;

    XOutdevItemPool*    pXPool;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineDashItem       aXDash;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XDashListRef          pDashList;

    ChangeType*         pnDashListState;
    sal_uInt16*         pPageType;
    sal_uInt16*         pDlgType;
    sal_uInt16*         pPosDashLb;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eFUnit;

#ifdef _SVX_TPLNEDEF_CXX
    void FillDash_Impl();
    void FillDialog_Impl();

    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( SelectLinestyleHdl_Impl, void * );
    DECL_LINK( ChangePreviewHdl_Impl, void * );
    DECL_LINK( ChangeNumber1Hdl_Impl, void * );
    DECL_LINK( ChangeNumber2Hdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
    DECL_LINK( ChangeMetricHdl_Impl, void * );
    DECL_LINK( SelectTypeHdl_Impl, void * );

    void CheckChanges_Impl();
#endif

public:
    SvxLineDefTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    SetDashList( XDashListRef pDshLst ) { pDashList = pDshLst; }
    void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPosDashLb( sal_uInt16* pInPos ) { pPosDashLb = pInPos; }

    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

/*************************************************************************/

class SvxLineEndDefTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    FixedLine           aFlTip;
    FixedText           aFTTitle;
    Edit                aEdtName;
    FixedText           aFTLineEndStyle;
    LineEndLB           aLbLineEnds;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;
    FixedInfo           aFiTip;
    SvxXLinePreview     aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    const SdrObject*    pPolyObj;
    sal_Bool            bObjSelected;

    XOutdevItemPool*    pXPool;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XLineEndListRef       pLineEndList;

    ChangeType*         pnLineEndListState;
    sal_uInt16*         pPageType;
    sal_uInt16*         pDlgType;
    sal_uInt16*             pPosLineEndLb;

#ifdef _SVX_TPLNEEND_CXX
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
    DECL_LINK( SelectLineEndHdl_Impl, void * );
    long ChangePreviewHdl_Impl( void* p );

    void CheckChanges_Impl();
#endif

public:
    SvxLineEndDefTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    ~SvxLineEndDefTabPage();

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    SetLineEndList( XLineEndListRef pInList ) { pLineEndList = pInList; }
    void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }
    void    SetObjSelected( sal_Bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPosLineEndLb( sal_uInt16* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
};

#endif // _CUI_TAB_LINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
