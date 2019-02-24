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
class ColorListBox;

class SvxLineTabDialog final : public SfxTabDialogController
{
    SdrModel*           pDrawModel;
    const SdrObject*    pObj;

    XColorListRef       pColorList;
    XColorListRef       mpNewColorList;
    XDashListRef        pDashList;
    XDashListRef        pNewDashList;
    XLineEndListRef     pLineEndList;
    XLineEndListRef     pNewLineEndList;
    bool                bObjSelected;

    ChangeType          nLineEndListState;
    ChangeType          nDashListState;
    ChangeType          mnColorListState;

    PageType            nPageType;
    sal_Int32           nPosDashLb;
    sal_Int32           nPosLineEndLb;

    virtual void        PageCreated(const OString& rId, SfxTabPage &rPage) override;

    virtual short       Ok() override;
    DECL_LINK(CancelHdlImpl, weld::Button&, void);
    void                SavePalettes();

public:
    SvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr,
                     SdrModel* pModel, const SdrObject* pObj,
                     bool bHasObj);

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

struct SvxBmpItemInfo
{
    std::unique_ptr<SvxBrushItem> pBrushItem;
    OUString sItemId;
};

class SvxLineTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pLineRanges[];
private:
    //#58425# symbols on a line (e. g. StarChart) ->
    /** a list of symbols to be shown in menu. Symbol at position SID_ATTR_SYMBOLTYPE is to be shown in preview.
        The list position is to be used cyclic. */
    SdrObjList*         m_pSymbolList;
    bool                m_bNewSize;
    /// a graphic to be displayed in the preview in case that an automatic symbol is chosen
    Graphic             m_aAutoSymbolGraphic;
    long                m_nSymbolType;
    /// attributes for the shown symbols; only necessary if not equal to line properties
    SfxItemSet*         m_pSymbolAttr;

    std::vector<OUString>      m_aGrfNames;
    std::vector< std::unique_ptr<SvxBmpItemInfo> >
                               m_aGalleryBrushItems;
    std::vector< std::unique_ptr<SvxBmpItemInfo> >
                               m_aSymbolBrushItems;
    bool                m_bLastWidthModified;
    Size                m_aSymbolLastSize;
    Graphic             m_aSymbolGraphic;
    Size                m_aSymbolSize;
    bool                m_bSymbols;

    const SfxItemSet&   m_rOutAttrs;
    bool                m_bObjSelected;

    XLineAttrSetItem    m_aXLineAttr;
    SfxItemSet&         m_rXLSet;

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

    SvxXLinePreview m_aCtlPreview;
    std::unique_ptr<weld::Widget> m_xBoxColor;
    std::unique_ptr<SvxLineLB> m_xLbLineStyle;
    std::unique_ptr<ColorListBox> m_xLbColor;
    std::unique_ptr<weld::Widget> m_xBoxWidth;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrLineWidth;
    std::unique_ptr<weld::Widget> m_xBoxTransparency;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTransparent;
    std::unique_ptr<weld::Widget> m_xFlLineEnds;
    std::unique_ptr<weld::Widget> m_xBoxArrowStyles;
    std::unique_ptr<SvxLineEndLB> m_xLbStartStyle;
    std::unique_ptr<weld::Widget> m_xBoxStart;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrStartWidth;
    std::unique_ptr<weld::CheckButton> m_xTsbCenterStart;
    std::unique_ptr<weld::Widget> m_xBoxEnd;
    std::unique_ptr<SvxLineEndLB> m_xLbEndStyle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrEndWidth;
    std::unique_ptr<weld::CheckButton> m_xTsbCenterEnd;
    std::unique_ptr<weld::CheckButton> m_xCbxSynchronize;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    std::unique_ptr<weld::Widget> m_xFLEdgeStyle;
    std::unique_ptr<weld::Widget> m_xGridEdgeCaps;
    std::unique_ptr<weld::ComboBox> m_xLBEdgeStyle;

    // LineCaps
    std::unique_ptr<weld::ComboBox> m_xLBCapStyle;

    std::unique_ptr<weld::Widget> m_xFlSymbol;
    std::unique_ptr<weld::Widget> m_xGridIconSize;
    std::unique_ptr<weld::MenuButton> m_xSymbolMB;
    std::unique_ptr<weld::Menu> m_xSymbolsMenu;
    std::unique_ptr<weld::Menu> m_xGalleryMenu;
    std::unique_ptr<weld::MetricSpinButton> m_xSymbolWidthMF;
    std::unique_ptr<weld::MetricSpinButton> m_xSymbolHeightMF;
    std::unique_ptr<weld::CheckButton> m_xSymbolRatioCB;

    // handler for gallery popup menu button + size
    DECL_LINK(GraphicHdl_Impl, const OString&, void);
    DECL_LINK(SizeHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(MenuCreateHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(RatioHdl_Impl, weld::ToggleButton&, void);

    DECL_LINK(ClickInvisibleHdl_Impl, weld::ComboBox&, void);
    void ClickInvisibleHdl_Impl();
    DECL_LINK(ChangeStartClickHdl_Impl, weld::Button&, void);
    DECL_LINK(ChangeStartListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ChangeStartModifyHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeEndListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ChangeEndModifyHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeEndClickHdl_Impl, weld::Button&, void);
    DECL_LINK(ChangePreviewListBoxHdl_Impl, ColorListBox&, void);
    DECL_LINK(ChangePreviewModifyHdl_Impl, weld::MetricSpinButton&, void);
    void ChangePreviewHdl_Impl(const weld::MetricSpinButton*);
    DECL_LINK(ChangeTransparentHdl_Impl, weld::MetricSpinButton&, void);

    DECL_LINK(ChangeEdgeStyleHdl_Impl, weld::ComboBox&, void);

    // LineCaps
    DECL_LINK(ChangeCapStyleHdl_Impl, weld::ComboBox&, void);

    void FillXLSet_Impl();

    void FillListboxes();
public:

    void ShowSymbolControls(bool bOn);

    SvxLineTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxLineTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pLineRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet* ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void FillUserData() override;

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
    const SfxItemSet&   rOutAttrs;
    XDash               aDash;

    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XDashListRef          pDashList;

    ChangeType*         pnDashListState;
    PageType*           pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPosDashLb;

    MapUnit             ePoolUnit;
    FieldUnit           eFUnit;

    SvxXLinePreview m_aCtlPreview;
    std::unique_ptr<SvxLineLB> m_xLbLineStyles;
    std::unique_ptr<weld::ComboBox> m_xLbType1;
    std::unique_ptr<weld::ComboBox> m_xLbType2;
    std::unique_ptr<weld::SpinButton> m_xNumFldNumber1;
    std::unique_ptr<weld::SpinButton> m_xNumFldNumber2;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrLength1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrLength2;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrDistance;
    std::unique_ptr<weld::CheckButton> m_xCbxSynchronize;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnModify;
    std::unique_ptr<weld::Button> m_xBtnDelete;
    std::unique_ptr<weld::Button> m_xBtnLoad;
    std::unique_ptr<weld::Button> m_xBtnSave;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    void FillDash_Impl();
    void FillDialog_Impl();

    DECL_LINK(ClickAddHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickModifyHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickDeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectLinestyleListBoxHdl_Impl, weld::ComboBox&, void);
    void SelectLinestyleHdl_Impl(const weld::ComboBox*);
    DECL_LINK(ChangePreviewHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeNumber1Hdl_Impl, weld::SpinButton&, void);
    DECL_LINK(ChangeNumber2Hdl_Impl, weld::SpinButton&, void);
    DECL_LINK(ClickLoadHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickSaveHdl_Impl, weld::Button&, void);
    DECL_LINK(ChangeMetricHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(SelectTypeListBoxHdl_Impl, weld::ComboBox&, void);
    void SelectTypeHdl_Impl(const weld::ComboBox*);
    void ChangeMetricHdl_Impl(const weld::ToggleButton*);

    void CheckChanges_Impl();

public:
    SvxLineDefTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxLineDefTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
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
    const SfxItemSet&   rOutAttrs;
    const SdrObject*    pPolyObj;

    XLineAttrSetItem    aXLineAttr;
    SfxItemSet&         rXLSet;

    XLineEndListRef       pLineEndList;

    ChangeType*         pnLineEndListState;
    PageType*           pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPosLineEndLb;

    SvxXLinePreview m_aCtlPreview;
    std::unique_ptr<weld::Entry> m_xEdtName;
    std::unique_ptr<SvxLineEndLB> m_xLbLineEnds;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnModify;
    std::unique_ptr<weld::Button> m_xBtnDelete;
    std::unique_ptr<weld::Button> m_xBtnLoad;
    std::unique_ptr<weld::Button> m_xBtnSave;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    DECL_LINK(ClickAddHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickModifyHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickDeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickLoadHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickSaveHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectLineEndHdl_Impl, weld::ComboBox&, void);

    void SelectLineEndHdl_Impl();
    void CheckChanges_Impl();

public:
    SvxLineEndDefTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxLineEndDefTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
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
