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

enum class PageType;
class SvxColorListBox;

class SvxLineTabDialog final : public SfxTabDialog
{
    sal_uInt16            m_nLineTabPage;
    sal_uInt16            m_nShadowTabPage;
    sal_uInt16            m_nStyleTabPage;
    sal_uInt16            m_nEndTabPage;

    SdrModel*           pDrawModel;
    const SdrObject*    pObj;

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

    PageType            nPageType;
    sal_Int32           nPosDashLb;
    sal_Int32           nPosLineEndLb;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

    virtual short       Ok() override;
    DECL_LINK( CancelHdlImpl, Button*, void );
    void                SavePalettes();

public:
    SvxLineTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                      SdrModel* pModel, const SdrObject* pObj,
                      bool bHasObj );

    void                SetNewDashList( XDashListRef const & pInLst)
                        { pNewDashList = pInLst; }
    const XDashListRef& GetNewDashList() const { return pNewDashList; }

    void                SetNewLineEndList( XLineEndListRef const & pInLst)
                        { pNewLineEndList = pInLst; }
    const XLineEndListRef& GetNewLineEndList() const { return pNewLineEndList; }

    void                SetNewColorList( XColorListRef const & pColTab ) { mpNewColorList = pColTab; }
    const XColorListRef& GetNewColorList() const { return mpNewColorList; }
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
    VclPtr<SvxColorListBox>    m_pLbColor;
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
    std::vector< SvxBmpItemInfo* >
                               m_aGrfBrushItems;
    bool                m_bLastWidthModified;
    Size                m_aSymbolLastSize;
    Graphic             m_aSymbolGraphic;
    Size                m_aSymbolSize;
    bool                m_bSymbols;

    const SfxItemSet&   m_rOutAttrs;
    bool                m_bObjSelected;

    XLineAttrSetItem    m_aXLineAttr;
    SfxItemSet&         m_rXLSet;

    XColorListRef         m_pColorList;
    XDashListRef          m_pDashList;
    XLineEndListRef       m_pLineEndList;

    ChangeType*         m_pnLineEndListState;
    ChangeType*         m_pnDashListState;
    ChangeType*         m_pnColorListState;
    PageType            m_nPageType;
    sal_uInt16          m_nDlgType;
    sal_Int32*          m_pPosDashLb;
    sal_Int32*          m_pPosLineEndLb;

    MapUnit             m_ePoolUnit;

    sal_Int32           m_nActLineWidth;

    // handler for gallery popup menu button + size
    DECL_LINK( GraphicHdl_Impl, MenuButton *, void );
    DECL_LINK( MenuCreateHdl_Impl, MenuButton *, void );
    DECL_LINK( SizeHdl_Impl, Edit&, void );
    DECL_LINK( RatioHdl_Impl, Button*, void );

    DECL_LINK( ClickInvisibleHdl_Impl, ListBox&, void );
    DECL_LINK( ChangeStartClickHdl_Impl, Button*, void );
    DECL_LINK( ChangeStartListBoxHdl_Impl, ListBox&, void );
    DECL_LINK( ChangeStartModifyHdl_Impl, Edit&, void );
    void ChangeStartHdl_Impl(void const *);
    DECL_LINK( ChangeEndListBoxHdl_Impl, ListBox&, void );
    DECL_LINK( ChangeEndModifyHdl_Impl, Edit&, void );
    DECL_LINK( ChangeEndClickHdl_Impl, Button*, void );
    void ChangeEndHdl_Impl(void const *);
    DECL_LINK( ChangePreviewListBoxHdl_Impl, SvxColorListBox&, void );
    DECL_LINK( ChangePreviewModifyHdl_Impl, Edit&, void );
    void ChangePreviewHdl_Impl(void const *);
    DECL_LINK( ChangeTransparentHdl_Impl, Edit&, void );

    DECL_LINK( ChangeEdgeStyleHdl_Impl, ListBox&, void );

    // LineCaps
    DECL_LINK( ChangeCapStyleHdl_Impl, ListBox&, void );

    void FillXLSet_Impl();

    void InitSymbols(MenuButton const * pButton);
    void SymbolSelected(MenuButton const * pButton);
    void FillListboxes();
public:

    void ShowSymbolControls(bool bOn);

    SvxLineTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pLineRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet* ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    virtual void FillUserData() override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetDashList( XDashListRef const & pDshLst ) { m_pDashList = pDshLst; }
    void    SetLineEndList( XLineEndListRef const & pLneEndLst) { m_pLineEndList = pLneEndLst; }
    void    SetObjSelected( bool bHasObj ) { m_bObjSelected = bHasObj; }

    void    SetPageType( PageType nInType ) { m_nPageType = nInType; }
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

    XLineDashItem       aXDash;
    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XDashListRef          pDashList;

    ChangeType*         pnDashListState;
    PageType*           pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPosDashLb;

    MapUnit             ePoolUnit;
    FieldUnit           eFUnit;

    void FillDash_Impl();
    void FillDialog_Impl();

    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK( ClickDeleteHdl_Impl, Button*, void );
    DECL_LINK( SelectLinestyleListBoxHdl_Impl, ListBox&, void );
    void SelectLinestyleHdl_Impl(ListBox const *);
    DECL_LINK( ChangePreviewHdl_Impl, Edit&, void );
    DECL_LINK( ChangeNumber1Hdl_Impl, Edit&, void );
    DECL_LINK( ChangeNumber2Hdl_Impl, Edit&, void );
    DECL_LINK( ClickLoadHdl_Impl, Button*, void );
    DECL_LINK( ClickSaveHdl_Impl, Button*, void );
    DECL_LINK( ChangeMetricHdl_Impl, Button*, void );
    DECL_LINK( SelectTypeListBoxHdl_Impl, ListBox&, void );
    void SelectTypeHdl_Impl(ListBox*);

    void CheckChanges_Impl();

public:
    SvxLineDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxLineDefTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetDashList( XDashListRef const & pDshLst ) { pDashList = pDshLst; }

    void    SetPageType( PageType* pInType ) { pPageType = pInType; }
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

    XLineColorItem      aXColor;
    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XLineEndListRef       pLineEndList;

    ChangeType*         pnLineEndListState;
    PageType*           pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPosLineEndLb;

    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK( ClickDeleteHdl_Impl, Button*, void );
    DECL_LINK( ClickLoadHdl_Impl, Button*, void );
    DECL_LINK( ClickSaveHdl_Impl, Button*, void );
    DECL_LINK( SelectLineEndHdl_Impl, ListBox&, void );

    void CheckChanges_Impl();

public:
    SvxLineEndDefTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxLineEndDefTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetLineEndList( XLineEndListRef const & pInList ) { pLineEndList = pInList; }
    void    SetPolyObj( const SdrObject* pObj ) { pPolyObj = pObj; }

    void    SetPageType( PageType* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPosLineEndLb( sal_Int32* pInPos ) { pPosLineEndLb = pInPos; }

    void    SetLineEndChgd( ChangeType* pIn ) { pnLineEndListState = pIn; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void Resize() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
