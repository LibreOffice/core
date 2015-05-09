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

#ifndef INCLUDED_CUI_SOURCE_INC_CUITABLINE_HXX
#define INCLUDED_CUI_SOURCE_INC_CUITABLINE_HXX

#include <vector>
#include <svx/tabline.hxx>
#include <svx/tabarea.hxx>

class SvxLineTabDialog : public SfxTabDialog
{
    sal_uInt16            m_nLineTabPage;
    sal_uInt16            m_nShadowTabPage;
    sal_uInt16            m_nStyleTabPage;
    sal_uInt16            m_nEndTabPage;

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
    bool            bObjSelected;

    ChangeType          nLineEndListState;
    ChangeType          nDashListState;
    ChangeType          mnColorListState;

    sal_uInt16          nPageType;
    sal_Int32           nPosDashLb;
    sal_Int32           nPosLineEndLb;
    bool            mbAreaTP;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) SAL_OVERRIDE;

protected:
    virtual short       Ok() SAL_OVERRIDE;
    DECL_LINK( CancelHdlImpl, void * );
    void                SavePalettes();

public:
    SvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                      SdrModel* pModel, const SdrObject* pObj = NULL,
                      bool bHasObj = true );

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
    static const sal_uInt16 pLineRanges[];
private:
    VclPtr<VclBox>             m_pBoxColor;
    VclPtr<LineLB>             m_pLbLineStyle;
    VclPtr<ColorLB>            m_pLbColor;
    VclPtr<VclBox>             m_pBoxWidth;
    VclPtr<MetricField>        m_pMtrLineWidth;
    VclPtr<VclBox>             m_pBoxTransparency;
    VclPtr<MetricField>        m_pMtrTransparent;

    VclPtr<VclFrame>           m_pFlLineEnds;
    VclPtr<VclBox>             m_pBoxArrowStyles;
    VclPtr<LineEndLB>          m_pLbStartStyle;
    VclPtr<VclBox>             m_pBoxStart;
    VclPtr<MetricField>        m_pMtrStartWidth;
    VclPtr<TriStateBox>        m_pTsbCenterStart;
    VclPtr<VclBox>             m_pBoxEnd;
    VclPtr<LineEndLB>          m_pLbEndStyle;
    VclPtr<MetricField>        m_pMtrEndWidth;
    VclPtr<TriStateBox>        m_pTsbCenterEnd;
    VclPtr<CheckBox>           m_pCbxSynchronize;
    VclPtr<SvxXLinePreview>    m_pCtlPreview;

    // #116827#
    VclPtr<VclFrame>           m_pFLEdgeStyle;
    VclPtr<VclGrid>            m_pGridEdgeCaps;
    VclPtr<ListBox>            m_pLBEdgeStyle;

    // LineCaps
    VclPtr<ListBox>             m_pLBCapStyle;

    //#58425# symbols on a line (e. g. StarChart) ->
    /** a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview.
        The list position is to be used cyclic. */
    SdrObjList*         pSymbolList;
    bool                bNewSize;
    /// a graphic to be displayed in the preview in case that an automatic symbol is chosen
    Graphic             aAutoSymbolGraphic;
    long                nNumMenuGalleryItems;
    long                nSymbolType;
    /// attributes for the shown symbols; only necessary if not equal to line properties
    SfxItemSet*         pSymbolAttr;
    VclPtr<VclFrame>           m_pFlSymbol;
    VclPtr<VclGrid>            m_pGridIconSize;
    VclPtr<MenuButton>         m_pSymbolMB;
    VclPtr<MetricField>        m_pSymbolWidthMF;
    VclPtr<MetricField>        m_pSymbolHeightMF;
    VclPtr<CheckBox>           m_pSymbolRatioCB;
    std::vector<OUString> aGrfNames;
    SvxBmpItemInfoList  aGrfBrushItems;
    bool            bLastWidthModified;
    Size                aSymbolLastSize;
    Graphic             aSymbolGraphic;
    Size                aSymbolSize;
    bool            bSymbols;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;
    bool            bObjSelected;

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
    sal_Int32*          pPosDashLb;
    sal_Int32*          pPosLineEndLb;

    SfxMapUnit          ePoolUnit;

    // #63083#
    sal_Int32           nActLineWidth;

    // handler for gallery popup menu button + size
    DECL_LINK_TYPED( GraphicHdl_Impl, MenuButton *, void );
    DECL_LINK( MenuCreateHdl_Impl, MenuButton * );
    DECL_LINK( GraphicArrivedHdl_Impl, SvxBrushItem* );
    DECL_LINK( SizeHdl_Impl, MetricField * );
    DECL_LINK( RatioHdl_Impl, CheckBox * );

    DECL_LINK( ClickInvisibleHdl_Impl, void * );
    DECL_LINK( ChangeStartHdl_Impl, void * );
    DECL_LINK( ChangeEndHdl_Impl, void * );
    DECL_LINK( ChangePreviewHdl_Impl, void * );
    DECL_LINK( ChangeTransparentHdl_Impl, void * );

    // #116827#
    DECL_LINK( ChangeEdgeStyleHdl_Impl, void * );

     // LineCaps
     DECL_LINK ( ChangeCapStyleHdl_Impl, void * );

    bool FillXLSet_Impl();

    void InitSymbols(MenuButton* pButton);
    void SymbolSelected(MenuButton* pButton);
    void FillListboxes();
public:

    void ShowSymbolControls(bool bOn);

    SvxLineTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineTabPage();
    virtual void dispose() SAL_OVERRIDE;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pLineRanges; }

    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet* ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) SAL_OVERRIDE;

    virtual void FillUserData() SAL_OVERRIDE;

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetDashList( XDashListRef pDshLst ) { pDashList = pDshLst; }
    void    SetLineEndList( XLineEndListRef pLneEndLst) { pLineEndList = pLneEndLst; }
    void    SetObjSelected( bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosDashLb( sal_Int32* pInPos ) { pPosDashLb = pInPos; }
    void    SetPosLineEndLb( sal_Int32* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }
    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};

/*************************************************************************/

class SvxLineDefTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
private:
    VclPtr<LineLB>              m_pLbLineStyles;
    VclPtr<ListBox>             m_pLbType1;
    VclPtr<ListBox>             m_pLbType2;
    VclPtr<NumericField>        m_pNumFldNumber1;
    VclPtr<NumericField>        m_pNumFldNumber2;
    VclPtr<MetricField>         m_pMtrLength1;
    VclPtr<MetricField>         m_pMtrLength2;
    VclPtr<MetricField>         m_pMtrDistance;
    VclPtr<CheckBox>            m_pCbxSynchronize;
    VclPtr<PushButton>          m_pBtnAdd;
    VclPtr<PushButton>          m_pBtnModify;
    VclPtr<PushButton>          m_pBtnDelete;
    VclPtr<PushButton>          m_pBtnLoad;
    VclPtr<PushButton>          m_pBtnSave;
    VclPtr<SvxXLinePreview>     m_pCtlPreview;

    const SfxItemSet&   rOutAttrs;
    XDash               aDash;
    bool            bObjSelected;

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
    sal_uInt16          nDlgType;
    sal_Int32*          pPosDashLb;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eFUnit;

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

public:
    SvxLineDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxLineDefTabPage();
    virtual void dispose() SAL_OVERRIDE;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet * ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    void    SetDashList( XDashListRef pDshLst ) { pDashList = pDshLst; }
    void    SetObjSelected( bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosDashLb( sal_Int32* pInPos ) { pPosDashLb = pInPos; }

    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};

/*************************************************************************/

class SvxLineEndDefTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<Edit>                m_pEdtName;
    VclPtr<LineEndLB>           m_pLbLineEnds;
    VclPtr<PushButton>          m_pBtnAdd;
    VclPtr<PushButton>          m_pBtnModify;
    VclPtr<PushButton>          m_pBtnDelete;
    VclPtr<PushButton>          m_pBtnLoad;
    VclPtr<PushButton>          m_pBtnSave;
    VclPtr<SvxXLinePreview>     m_pCtlPreview;

    const SfxItemSet&   rOutAttrs;
    const SdrObject*    pPolyObj;
    bool            bObjSelected;

    XOutdevItemPool*    pXPool;
    XLineStyleItem      aXLStyle;
    XLineWidthItem      aXWidth;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XLineEndListRef       pLineEndList;

    ChangeType*         pnLineEndListState;
    sal_uInt16*         pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPosLineEndLb;

    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
    DECL_LINK( SelectLineEndHdl_Impl, void * );
    long ChangePreviewHdl_Impl( void* p );

    void CheckChanges_Impl();

public:
    SvxLineEndDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineEndDefTabPage();
    virtual void dispose() SAL_OVERRIDE;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet * ) SAL_OVERRIDE;

    virtual void ActivatePage( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;

    void    SetLineEndList( XLineEndListRef pInList ) { pLineEndList = pInList; }
    void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }
    void    SetObjSelected( bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosLineEndLb( sal_Int32* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
