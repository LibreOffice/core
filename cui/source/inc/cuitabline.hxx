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

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

protected:
    virtual short       Ok() override;
    DECL_LINK_TYPED( CancelHdlImpl, Button*, void );
    void                SavePalettes();

public:
    SvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                      SdrModel* pModel, const SdrObject* pObj = nullptr,
                      bool bHasObj = true );

    void                SetNewDashList( XDashListRef pInLst)
                        { pNewDashList = pInLst; }
    XDashListRef        GetNewDashList() const { return pNewDashList; }

    void                SetNewLineEndList( XLineEndListRef pInLst)
                        { pNewLineEndList = pInLst; }
    XLineEndListRef     GetNewLineEndList() const { return pNewLineEndList; }

    void                SetNewColorList( XColorListRef pColTab ) { mpNewColorList = pColTab; }
    XColorListRef       GetNewColorList() const { return mpNewColorList; }
    XColorListRef       GetColorList() const { return pColorList; }
};

/*************************************************************************/

struct SvxBmpItemInfo;

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

    VclPtr<VclFrame>           m_pFLEdgeStyle;
    VclPtr<VclGrid>            m_pGridEdgeCaps;
    VclPtr<ListBox>            m_pLBEdgeStyle;

    // LineCaps
    VclPtr<ListBox>             m_pLBCapStyle;

    //#58425# symbols on a line (e. g. StarChart) ->
    /** a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview.
        The list position is to be used cyclic. */
    SdrObjList*         m_pSymbolList;
    bool                m_bNewSize;
    /// a graphic to be displayed in the preview in case that an automatic symbol is chosen
    Graphic             m_aAutoSymbolGraphic;
    long                m_nNumMenuGalleryItems;
    long                m_nSymbolType;
    /// attributes for the shown symbols; only necessary if not equal to line properties
    SfxItemSet*         m_pSymbolAttr;
    VclPtr<VclFrame>           m_pFlSymbol;
    VclPtr<VclGrid>            m_pGridIconSize;
    VclPtr<MenuButton>         m_pSymbolMB;
    VclPtr<MetricField>        m_pSymbolWidthMF;
    VclPtr<MetricField>        m_pSymbolHeightMF;
    VclPtr<CheckBox>           m_pSymbolRatioCB;
    std::vector<OUString>      m_aGrfNames;
    ::std::vector< SvxBmpItemInfo* >
                               m_aGrfBrushItems;
    bool                m_bLastWidthModified;
    Size                m_aSymbolLastSize;
    Graphic             m_aSymbolGraphic;
    Size                m_aSymbolSize;
    bool                m_bSymbols;

    const SfxItemSet&   m_rOutAttrs;
    RECT_POINT          m_eRP;
    bool                m_bObjSelected;

    XLineStyleItem      m_aXLStyle;
    XLineWidthItem      m_aXWidth;
    XLineDashItem       m_aXDash;
    XLineColorItem      m_aXColor;
    XLineAttrSetItem    m_aXLineAttr;
    SfxItemSet&         m_rXLSet;

    XColorListRef         m_pColorList;
    XDashListRef          m_pDashList;
    XLineEndListRef       m_pLineEndList;

    ChangeType*         m_pnLineEndListState;
    ChangeType*         m_pnDashListState;
    ChangeType*         m_pnColorListState;
    sal_uInt16          m_nPageType;
    sal_uInt16          m_nDlgType;
    sal_Int32*          m_pPosDashLb;
    sal_Int32*          m_pPosLineEndLb;

    SfxMapUnit          m_ePoolUnit;

    sal_Int32           m_nActLineWidth;

    // handler for gallery popup menu button + size
    DECL_LINK_TYPED( GraphicHdl_Impl, MenuButton *, void );
    DECL_LINK_TYPED( MenuCreateHdl_Impl, MenuButton *, void );
    DECL_LINK_TYPED( SizeHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( RatioHdl_Impl, Button*, void );

    DECL_LINK_TYPED( ClickInvisibleHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangeStartClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangeStartListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangeStartModifyHdl_Impl, Edit&, void );
    void ChangeStartHdl_Impl(void*);
    DECL_LINK_TYPED( ChangeEndListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangeEndModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ChangeEndClickHdl_Impl, Button*, void );
    void ChangeEndHdl_Impl(void*);
    DECL_LINK_TYPED( ChangePreviewListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangePreviewModifyHdl_Impl, Edit&, void );
    void ChangePreviewHdl_Impl(void*);
    DECL_LINK_TYPED( ChangeTransparentHdl_Impl, Edit&, void );

    DECL_LINK_TYPED( ChangeEdgeStyleHdl_Impl, ListBox&, void );

    // LineCaps
    DECL_LINK_TYPED( ChangeCapStyleHdl_Impl, ListBox&, void );

    bool FillXLSet_Impl();

    void InitSymbols(MenuButton* pButton);
    void SymbolSelected(MenuButton* pButton);
    void FillListboxes();
public:

    void ShowSymbolControls(bool bOn);

    SvxLineTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pLineRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet* ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    virtual void FillUserData() override;

    void    SetColorList( XColorListRef pColorList ) { m_pColorList = pColorList; }
    void    SetDashList( XDashListRef pDshLst ) { m_pDashList = pDshLst; }
    void    SetLineEndList( XLineEndListRef pLneEndLst) { m_pLineEndList = pLneEndLst; }
    void    SetObjSelected( bool bHasObj ) { m_bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16 nInType ) { m_nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPosDashLb( sal_Int32* pInPos ) { m_pPosDashLb = pInPos; }
    void    SetPosLineEndLb( sal_Int32* pInPos ) { m_pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { m_pnLineEndListState = pIn; }
    void    SetDashChgd( ChangeType* pIn ) { m_pnDashListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }

    virtual void PageCreated(const SfxAllItemSet& aSet) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
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

    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickDeleteHdl_Impl, Button*, void );
    DECL_LINK_TYPED( SelectLinestyleListBoxHdl_Impl, ListBox&, void );
    void SelectLinestyleHdl_Impl(ListBox*);
    DECL_LINK_TYPED( ChangePreviewHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ChangeNumber1Hdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ChangeNumber2Hdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ClickLoadHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickSaveHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangeMetricHdl_Impl, Button*, void );
    DECL_LINK_TYPED( SelectTypeListBoxHdl_Impl, ListBox&, void );
    void SelectTypeHdl_Impl(ListBox*);

    void CheckChanges_Impl();

public:
    SvxLineDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxLineDefTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) override;

    void    SetDashList( XDashListRef pDshLst ) { pDashList = pDshLst; }
    void    SetObjSelected( bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosDashLb( sal_Int32* pInPos ) { pPosDashLb = pInPos; }

    void    SetDashChgd( ChangeType* pIn ) { pnDashListState = pIn; }

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
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

    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickDeleteHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickLoadHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickSaveHdl_Impl, Button*, void );
    DECL_LINK_TYPED( SelectLineEndHdl_Impl, ListBox&, void );
    long ChangePreviewHdl_Impl( void* p );

    void CheckChanges_Impl();

public:
    SvxLineEndDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineEndDefTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual sfxpg DeactivatePage( SfxItemSet* pSet ) override;

    void    SetLineEndList( XLineEndListRef pInList ) { pLineEndList = pInList; }
    void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }
    void    SetObjSelected( bool bHasObj ) { bObjSelected = bHasObj; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosLineEndLb( sal_Int32* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void Resize() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
