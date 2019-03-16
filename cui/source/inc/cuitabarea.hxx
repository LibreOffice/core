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
#ifndef INCLUDED_CUI_SOURCE_INC_CUITABAREA_HXX
#define INCLUDED_CUI_SOURCE_INC_CUITABAREA_HXX

#include <svtools/valueset.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/xsetit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfillit.hxx>
#include <svx/tabarea.hxx>
#include <svx/hexcolorcontrol.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/SvxPresetListBox.hxx>
#include <svx/Palette.hxx>
#include <svx/PaletteManager.hxx>
#include <svx/svdview.hxx>

#define NO_BUTTON_SELECTED -1

class ColorListBox;
class SdrModel;
class SvxBitmapCtl;
class SvxColorListBox;

/************************************************************************/
class ButtonBox
{
    private:
        sal_Int32 mnCurrentButton;
        std::vector<weld::ToggleButton*> maButtonList;
        std::map<weld::ToggleButton*, sal_Int32 > maButtonToPos;
        void SelectButtonImpl( sal_Int32 nPos )
        {
            if(mnCurrentButton != NO_BUTTON_SELECTED)
            {
                maButtonList[mnCurrentButton]->set_active(false);
            }
            mnCurrentButton = nPos;
            maButtonList[mnCurrentButton]->set_active(true);
        };
    public:
        ButtonBox()
        {
            mnCurrentButton = NO_BUTTON_SELECTED;
        };
        void AddButton(weld::ToggleButton* pButton)
        {
            maButtonList.push_back(pButton);
            maButtonToPos.insert( std::make_pair(pButton, maButtonList.size() - 1) );
        }
        sal_Int32 GetCurrentButtonPos() { return mnCurrentButton; }
        sal_Int32 GetButtonPos(weld::ToggleButton* pButton)
        {
            std::map<weld::ToggleButton*, sal_Int32>::const_iterator aBtnPos = maButtonToPos.find(pButton);
            if(aBtnPos != maButtonToPos.end())
                return aBtnPos->second;
            else
                return -1;
        }
        void SelectButton(weld::ToggleButton* pButton)
        {
            sal_Int32 nPos = GetButtonPos(pButton);
            if(nPos != -1)
                SelectButtonImpl(nPos);
        }
};

enum class PageType
{
    Area,
    Gradient,
    Hatch,
    Bitmap,
    Shadow,
    Transparence,
};

class SvxAreaTabDialog final : public SfxTabDialogController
{
    SdrModel*           mpDrawModel;

    XColorListRef         mpColorList;
    XColorListRef         mpNewColorList;
    XGradientListRef      mpGradientList;
    XGradientListRef      mpNewGradientList;
    XHatchListRef         mpHatchingList;
    XHatchListRef         mpNewHatchingList;
    XBitmapListRef        mpBitmapList;
    XBitmapListRef        mpNewBitmapList;
    XPatternListRef       mpPatternList;
    XPatternListRef       mpNewPatternList;

    ChangeType          mnColorListState;
    ChangeType          mnBitmapListState;
    ChangeType          mnPatternListState;
    ChangeType          mnGradientListState;
    ChangeType          mnHatchingListState;

    virtual void        PageCreated(const OString& rId, SfxTabPage &rPage) override;

    virtual short       Ok() override;
    DECL_LINK(CancelHdlImpl, weld::Button&, void);
    void                SavePalettes();

public:
    SvxAreaTabDialog(weld::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, bool bShadow);

    void                SetNewColorList( XColorListRef const & pColorList )
                            { mpNewColorList = pColorList; }
    const XColorListRef&  GetNewColorList() const { return mpNewColorList; }
};

/************************************************************************/

class SvxTransparenceTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pTransparenceRanges[];

    const SfxItemSet&   rOutAttrs;

    PageType               nPageType;
    sal_uInt16             nDlgType;

    bool                bBitmap;

    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    SvxXRectPreview     m_aCtlBitmapPreview;
    SvxXRectPreview     m_aCtlXRectPreview;

    // main selection
    std::unique_ptr<weld::RadioButton> m_xRbtTransOff;
    std::unique_ptr<weld::RadioButton> m_xRbtTransLinear;
    std::unique_ptr<weld::RadioButton> m_xRbtTransGradient;

    /// linear transparency
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTransparent;

    // gradient transparency
    std::unique_ptr<weld::Widget> m_xGridGradient;
    std::unique_ptr<weld::ComboBox> m_xLbTrgrGradientType;
    std::unique_ptr<weld::Label> m_xFtTrgrCenterX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrCenterX;
    std::unique_ptr<weld::Label> m_xFtTrgrCenterY;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrCenterY;
    std::unique_ptr<weld::Label> m_xFtTrgrAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrBorder;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrStartValue;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTrgrEndValue;
    std::unique_ptr<weld::Widget> m_xCtlBitmapBorder;
    std::unique_ptr<weld::Widget> m_xCtlXRectBorder;

    // preview
    std::unique_ptr<weld::CustomWeld> m_xCtlBitmapPreview;
    std::unique_ptr<weld::CustomWeld> m_xCtlXRectPreview;

    DECL_LINK(ClickTransOffHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ClickTransLinearHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ClickTransGradientHdl_Impl, weld::ToggleButton&, void );
    DECL_LINK(ModifyTransparentHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifiedTrgrEditHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifiedTrgrListBoxHdl_Impl, weld::ComboBox&, void);
    void ModifiedTrgrHdl_Impl(const weld::ComboBox*);

    void ActivateLinear(bool bActivate);
    void ActivateGradient(bool bActivate);
    void SetControlState_Impl(css::awt::GradientStyle eXGS);

    bool InitPreview ( const SfxItemSet& rSet );
    void InvalidatePreview (bool bEnable = true );

public:
    SvxTransparenceTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxTransparenceTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent, const SfxItemSet*);
    static const sal_uInt16* GetRanges() { return pTransparenceRanges; }

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
    virtual void ChangesApplied() override;
    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;

    void SetPageType(PageType nInType) { nPageType = nInType; }
    void SetDlgType(sal_uInt16 nInType) { nDlgType = nInType; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/************************************************************************/

class SvxAreaTabPage : public SfxTabPage
{
    static const sal_uInt16 pAreaRanges[];
private:
    ScopedVclPtr<SfxTabPage>   m_pFillTabPage;
    ButtonBox                  maBox;

    XColorListRef         m_pColorList;
    XGradientListRef      m_pGradientList;
    XHatchListRef         m_pHatchingList;
    XBitmapListRef        m_pBitmapList;
    XPatternListRef       m_pPatternList;

    // Placeholders for pointer-based entries; these will be inited
    // to point to these so that the page is usable without that
    // SvxAreaTabDialog has to call the setter methods (e.g. SetColorChgd).
    // Without that the pages used in SvxAreaTabDialog are not usable
    ChangeType          maFixed_ChangeType;

    ChangeType*         m_pnColorListState;
    ChangeType*         m_pnBitmapListState;
    ChangeType*         m_pnPatternListState;
    ChangeType*         m_pnGradientListState;
    ChangeType*         m_pnHatchingListState;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

protected:
    std::unique_ptr<weld::Container> m_xFillTab;
    std::unique_ptr<weld::ToggleButton> m_xBtnNone;
    std::unique_ptr<weld::ToggleButton> m_xBtnColor;
    std::unique_ptr<weld::ToggleButton> m_xBtnGradient;
    std::unique_ptr<weld::ToggleButton> m_xBtnHatch;
    std::unique_ptr<weld::ToggleButton> m_xBtnBitmap;
    std::unique_ptr<weld::ToggleButton> m_xBtnPattern;

    void SetOptimalSize(weld::DialogController* pController);

    void SelectFillType( weld::ToggleButton& rButton, const SfxItemSet* _pSet = nullptr );
    SfxTabPage* GetFillTabPage() { return m_pFillTabPage; }

private:
    DECL_LINK(SelectFillTypeHdl_Impl, weld::ToggleButton&, void);

    template< typename TabPage >
    bool FillItemSet_Impl( SfxItemSet* );
    template< typename TabPage >
    void Reset_Impl( const SfxItemSet* );
    template< typename TabPage >
    DeactivateRC DeactivatePage_Impl( SfxItemSet* pSet );

public:
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    SvxAreaTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxAreaTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pAreaRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetGradientList( XGradientListRef const & pGrdLst)
                { m_pGradientList = pGrdLst; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }
    void    SetBitmapList( XBitmapListRef const & pBmpLst) { m_pBitmapList = pBmpLst; }
    void    SetPatternList( XPatternListRef const &pPtrnLst ) { m_pPatternList = pPtrnLst; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
    void    CreatePage(sal_Int32 nId, SfxTabPage* pTab);
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    void    SetGrdChgd( ChangeType* pIn ) { m_pnGradientListState = pIn; }
    void    SetHtchChgd( ChangeType* pIn ) { m_pnHatchingListState = pIn; }
    void    SetBmpChgd( ChangeType* pIn ) { m_pnBitmapListState = pIn; }
    void    SetPtrnChgd( ChangeType* pIn ) { m_pnPatternListState = pIn; }
};


class SvxShadowTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pShadowRanges[];

private:
    const SfxItemSet&   m_rOutAttrs;

    XColorListRef       m_pColorList;
    ChangeType*         m_pnColorListState;
    PageType            m_nPageType;
    sal_uInt16          m_nDlgType;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;
    MapUnit             m_ePoolUnit;

    SvxRectCtl m_aCtlPosition;
    SvxXShadowPreview  m_aCtlXRectPreview;
    std::unique_ptr<weld::CheckButton> m_xTsbShowShadow;
    std::unique_ptr<weld::Widget> m_xGridShadow;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrDistance;
    std::unique_ptr<ColorListBox> m_xLbShadowColor;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrTransparent;
    std::unique_ptr<weld::CustomWeld> m_xCtlPosition;
    std::unique_ptr<weld::CustomWeld> m_xCtlXRectPreview;

    DECL_LINK(ClickShadowHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(ModifyShadowHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(SelectShadowHdl_Impl, ColorListBox&, void);

public:
    SvxShadowTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxShadowTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pShadowRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPageType( PageType nInType ) { m_nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/************************************************************************/

class SvxGradientTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XGradientListRef      m_pGradientList;

    ChangeType*         m_pnGradientListState;
    ChangeType*         m_pnColorListState;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    SvxXRectPreview m_aCtlPreview;
    std::unique_ptr<weld::ComboBox> m_xLbGradientType;
    std::unique_ptr<weld::Label> m_xFtCenter;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrCenterX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrCenterY;
    std::unique_ptr<weld::Label> m_xFtAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrBorder;
    std::unique_ptr<weld::Scale> m_xSliderBorder;
    std::unique_ptr<ColorListBox> m_xLbColorFrom;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrColorFrom;
    std::unique_ptr<ColorListBox> m_xLbColorTo;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrColorTo;
    std::unique_ptr<SvxPresetListBox> m_xGradientLB;
    std::unique_ptr<weld::SpinButton> m_xMtrIncrement;
    std::unique_ptr<weld::CheckButton> m_xCbIncrement;
    std::unique_ptr<weld::Scale> m_xSliderIncrement;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnModify;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;
    std::unique_ptr<weld::CustomWeld> m_xGradientLBWin;

    DECL_LINK( ClickAddHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickModifyHdl_Impl, weld::Button&, void );
    DECL_LINK( ChangeGradientHdl, SvtValueSet*, void );
    void ChangeGradientHdl_Impl();
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ModifiedEditHdl_Impl, weld::SpinButton&, void );
    DECL_LINK( ModifiedMetricHdl_Impl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifiedColorListBoxHdl_Impl, ColorListBox&, void );
    DECL_LINK( ModifiedListBoxHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( ChangeAutoStepHdl_Impl, weld::ToggleButton&, void );
    DECL_LINK( ModifiedSliderHdl_Impl, weld::Scale&, void );
    void ModifiedHdl_Impl(void const *);

    void SetControlState_Impl( css::awt::GradientStyle eXGS );
    sal_Int32 SearchGradientList(const OUString& rGradientName);

public:
    SvxGradientTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxGradientTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetGradientList( XGradientListRef const & pGrdLst)
                { m_pGradientList = pGrdLst; }
    void    SetGrdChgd( ChangeType* pIn ) { m_pnGradientListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
};

/************************************************************************/

class SvxHatchTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XHatchListRef         m_pHatchingList;

    ChangeType*         m_pnHatchingListState;
    ChangeType*         m_pnColorListState;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    MapUnit             m_ePoolUnit;

    SvxXRectPreview m_aCtlPreview;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrDistance;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrAngle;
    std::unique_ptr<weld::Scale> m_xSliderAngle;
    std::unique_ptr<weld::ComboBox> m_xLbLineType;
    std::unique_ptr<ColorListBox> m_xLbLineColor;
    std::unique_ptr<weld::CheckButton> m_xCbBackgroundColor;
    std::unique_ptr<ColorListBox> m_xLbBackgroundColor;
    std::unique_ptr<SvxPresetListBox> m_xHatchLB;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnModify;
    std::unique_ptr<weld::CustomWeld> m_xHatchLBWin;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;

    DECL_LINK(ChangeHatchHdl, SvtValueSet*, void);
    void ChangeHatchHdl_Impl();
    DECL_LINK( ModifiedEditHdl_Impl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifiedListBoxHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( ModifiedColorListBoxHdl_Impl, ColorListBox&, void );
    DECL_LINK( ToggleHatchBackgroundColor_Impl, weld::ToggleButton&, void );
    DECL_LINK( ModifiedBackgroundHdl_Impl, ColorListBox&, void );
    DECL_LINK( ModifiedSliderHdl_Impl, weld::Scale&, void );
    void ModifiedHdl_Impl(void const *);
    DECL_LINK( ClickAddHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickModifyHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    sal_Int32 SearchHatchList(const OUString& rHatchName);

public:
    SvxHatchTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxHatchTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }

    void    SetHtchChgd( ChangeType* pIn ) { m_pnHatchingListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
};

/************************************************************************/

class SvxBitmapTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pBitmapRanges[];
private:

    const SfxItemSet&          m_rOutAttrs;

    XBitmapListRef             m_pBitmapList;
    ChangeType*                m_pnBitmapListState;

    double                     m_fObjectWidth;
    double                     m_fObjectHeight;
    bool                       m_bLogicalSize;

    XFillAttrSetItem           m_aXFillAttr;
    SfxItemSet&                m_rXFSet;
    const SdrView*             mpView;
    MapUnit                    mePoolUnit;
    FieldUnit                  meFieldUnit;
    Size                       rBitmapSize;
    Size                       rFilledSize;
    Size                       rZoomedSize;

    SvxXRectPreview m_aCtlBitmapPreview;
    std::unique_ptr<SvxPresetListBox>   m_xBitmapLB;
    std::unique_ptr<weld::ComboBox> m_xBitmapStyleLB;
    std::unique_ptr<weld::Container> m_xSizeBox;
    std::unique_ptr<weld::CheckButton> m_xTsbScale;
    std::unique_ptr<weld::MetricSpinButton> m_xBitmapWidth;
    std::unique_ptr<weld::MetricSpinButton> m_xBitmapHeight;
    std::unique_ptr<weld::Container> m_xPositionBox;
    std::unique_ptr<weld::ComboBox> m_xPositionLB;
    std::unique_ptr<weld::Container> m_xPositionOffBox;
    std::unique_ptr<weld::MetricSpinButton> m_xPositionOffX;
    std::unique_ptr<weld::MetricSpinButton> m_xPositionOffY;
    std::unique_ptr<weld::Container> m_xTileOffBox;
    std::unique_ptr<weld::ComboBox> m_xTileOffLB;
    std::unique_ptr<weld::MetricSpinButton> m_xTileOffset;
    std::unique_ptr<weld::Button> m_xBtnImport;
    std::unique_ptr<weld::CustomWeld> m_xCtlBitmapPreview;
    std::unique_ptr<weld::CustomWeld> m_xBitmapLBWin;

    DECL_LINK( ModifyBitmapHdl, SvtValueSet*, void );
    DECL_LINK( ClickScaleHdl, weld::Button&, void );
    DECL_LINK( ModifyBitmapStyleHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyBitmapSizeHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifyBitmapPositionHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyPositionOffsetHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifyTileOffsetHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ClickRenameHdl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl, SvxPresetListBox*, void );
    DECL_LINK( ClickImportHdl, weld::Button&, void );
    void ClickBitmapHdl_Impl();
    void CalculateBitmapPresetSize();
    sal_Int32 SearchBitmapList(const OUString& rBitmapName);
    sal_Int32 SearchBitmapList(const GraphicObject& rGraphicObject);

public:
    SvxBitmapTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxBitmapTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetBitmapList( const XBitmapListRef& pBmpLst) { m_pBitmapList = pBmpLst; }
    void    SetBmpChgd( ChangeType* pIn ) { m_pnBitmapListState = pIn; }
};

/************************************************************************/

class SvxPatternTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XPatternListRef       m_pPatternList;

    ChangeType*         m_pnPatternListState;
    ChangeType*         m_pnColorListState;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    SvxXRectPreview m_aCtlPreview;
    std::unique_ptr<SvxPixelCtl> m_xCtlPixel;
    std::unique_ptr<ColorListBox> m_xLbColor;
    std::unique_ptr<ColorListBox> m_xLbBackgroundColor;
    std::unique_ptr<SvxPresetListBox> m_xPatternLB;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnModify;
    std::unique_ptr<weld::CustomWeld> m_xCtlPixelWin;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreview;
    std::unique_ptr<weld::CustomWeld> m_xPatternLBWin;
    std::unique_ptr<SvxBitmapCtl> m_xBitmapCtl;

    DECL_LINK( ClickAddHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickModifyHdl_Impl, weld::Button&, void );
    DECL_LINK( ChangePatternHdl_Impl, SvtValueSet*, void );
    DECL_LINK( ChangeColorHdl_Impl, ColorListBox&, void );
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    sal_Int32 SearchPatternList(const OUString& rPatternName);

public:
    SvxPatternTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxPatternTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( weld::DrawingArea*, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPatternList( XPatternListRef const & pPatternList) { m_pPatternList = pPatternList; }
    void    SetPtrnChgd( ChangeType* pIn ) { m_pnPatternListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    void    ChangeColor_Impl();
};

/************************************************************************/

enum class ColorModel
{
    RGB,
    CMYK
};

class SvxColorTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<Window>             mpTopDlg;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;

    ChangeType*         pnColorListState;

    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    ColorModel          eCM;

    Color               aPreviousColor;
    Color               aCurrentColor;

    css::uno::Reference< css::uno::XComponentContext > m_context;

    PaletteManager maPaletteManager;
    SvxXRectPreview m_aCtlPreviewOld;
    SvxXRectPreview m_aCtlPreviewNew;
    std::unique_ptr<ColorValueSet> m_xValSetColorList;
    std::unique_ptr<ColorValueSet> m_xValSetRecentList;
    std::unique_ptr<weld::ComboBox> m_xSelectPalette;
    std::unique_ptr<weld::RadioButton> m_xRbRGB;
    std::unique_ptr<weld::RadioButton> m_xRbCMYK;
    std::unique_ptr<weld::Widget> m_xRGBcustom;
    std::unique_ptr<weld::Widget> m_xRGBpreset;
    std::unique_ptr<weld::Entry> m_xRpreset;
    std::unique_ptr<weld::Entry> m_xGpreset;
    std::unique_ptr<weld::Entry> m_xBpreset;
    std::unique_ptr<weld::SpinButton> m_xRcustom;
    std::unique_ptr<weld::SpinButton> m_xGcustom;
    std::unique_ptr<weld::SpinButton> m_xBcustom;
    std::unique_ptr<weld::HexColorControl> m_xHexpreset;
    std::unique_ptr<weld::HexColorControl> m_xHexcustom;
    std::unique_ptr<weld::Widget> m_xCMYKcustom;
    std::unique_ptr<weld::Widget> m_xCMYKpreset;
    std::unique_ptr<weld::Entry> m_xCpreset;
    std::unique_ptr<weld::Entry> m_xYpreset;
    std::unique_ptr<weld::Entry> m_xMpreset;
    std::unique_ptr<weld::Entry> m_xKpreset;
    std::unique_ptr<weld::MetricSpinButton> m_xCcustom;
    std::unique_ptr<weld::MetricSpinButton> m_xYcustom;
    std::unique_ptr<weld::MetricSpinButton> m_xMcustom;
    std::unique_ptr<weld::MetricSpinButton> m_xKcustom;
    std::unique_ptr<weld::Button> m_xBtnAdd;
    std::unique_ptr<weld::Button> m_xBtnDelete;
    std::unique_ptr<weld::Button> m_xBtnWorkOn;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreviewOld;
    std::unique_ptr<weld::CustomWeld> m_xCtlPreviewNew;
    std::unique_ptr<weld::CustomWeld> m_xValSetColorListWin;
    std::unique_ptr<weld::CustomWeld> m_xValSetRecentListWin;

    static void    ConvertColorValues (Color& rColor, ColorModel eModell);
    static void    RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK );
    static void    CmykToRgb_Impl( Color& rColor, const sal_uInt16 nKey );
    sal_uInt16  ColorToPercent_Impl( sal_uInt16 nColor );
    sal_uInt16  PercentToColor_Impl( sal_uInt16 nPercent );

    void ImpColorCountChanged();
    void FillPaletteLB();

    DECL_LINK(ClickAddHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickWorkOnHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickDeleteHdl_Impl, weld::Button&, void);

    DECL_LINK(SelectPaletteLBHdl, weld::ComboBox&, void);
    DECL_LINK( SelectValSetHdl_Impl, SvtValueSet*, void );
    DECL_LINK( SelectColorModeHdl_Impl, weld::ToggleButton&, void );
    void ChangeColor(const Color &rNewColor, bool bUpdatePreset = true);
    void SetColorModel(ColorModel eModel);
    void ChangeColorModel();
    void UpdateColorValues( bool bUpdatePreset = true );
    DECL_LINK(SpinValueHdl_Impl, weld::SpinButton&, void);
    DECL_LINK(MetricSpinValueHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifiedHdl_Impl, weld::Entry&, void);

    void UpdateModified();

    static sal_Int32 FindInCustomColors( OUString const & aColorName );
    sal_Int32 FindInPalette( const Color& rColor );

public:
    SvxColorTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxColorTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef );
    void    SetColorList( const XColorListRef& pColList );


    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    void    SetCtlPreviewOld( SfxItemSet& rAttrs ) { m_aCtlPreviewOld.SetAttributes( rAttrs ); }

    virtual void FillUserData() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABAREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
