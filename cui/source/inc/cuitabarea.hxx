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
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/slider.hxx>
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

class SdrModel;
class SvxBitmapCtl;
class SvxColorListBox;

/************************************************************************/
class ButtonBox
{
    private:
        sal_Int32 mnCurrentButton;
        std::vector< VclPtr<PushButton> > maButtonList;
        std::map< VclPtr<PushButton>, sal_Int32 > maButtonToPos;
        void SelectButtonImpl( sal_Int32 nPos )
        {
            if(mnCurrentButton != NO_BUTTON_SELECTED)
            {
                maButtonList[mnCurrentButton]->SetPressed(false);
            }
            mnCurrentButton = nPos;
            maButtonList[mnCurrentButton]->SetPressed(true);
        };
    public:
        ButtonBox()
        {
            mnCurrentButton = NO_BUTTON_SELECTED;
        };
        void AddButton(VclPtr<PushButton> pButton)
        {
            maButtonList.push_back(pButton);
            maButtonToPos.insert( std::make_pair(pButton, maButtonList.size() - 1) );
        }
        sal_Int32 GetCurrentButtonPos() { return mnCurrentButton; }
        sal_Int32 GetButtonPos( VclPtr<PushButton> pButton )
        {
            std::map< VclPtr<PushButton>, sal_Int32 >::const_iterator aBtnPos = maButtonToPos.find(pButton);
            if(aBtnPos != maButtonToPos.end())
                return aBtnPos->second;
            else
                return -1;
        }
        void SelectButton( VclPtr<PushButton> pButton)
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
    Color,
    Shadow,
    Transparence,
};

class SvxAreaTabDialog : public SfxTabDialog
{
    sal_uInt16            m_nAreaTabPage;
    sal_uInt16            m_nShadowTabPage;
    sal_uInt16            m_nTransparenceTabPage;

private:
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

    const SfxItemSet&   mrOutAttrs;

    ChangeType          mnColorListState;
    ChangeType          mnBitmapListState;
    ChangeType          mnPatternListState;
    ChangeType          mnGradientListState;
    ChangeType          mnHatchingListState;

    sal_Int16           mnPos;
    PageType            mnPageType;
    bool                mbAreaTP;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

protected:
    virtual short       Ok() override;
    DECL_LINK( CancelHdlImpl, Button*, void );
    void                SavePalettes();

public:
    SvxAreaTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, bool bShadow );

    void                SetNewColorList( XColorListRef const & pColorList )
                            { mpNewColorList = pColorList; }
    const XColorListRef&  GetNewColorList() const { return mpNewColorList; }
};

/************************************************************************/

class SvxTransparenceTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pTransparenceRanges[];

    const SfxItemSet&   rOutAttrs;
    RectPoint          eRP;

    PageType               nPageType;
    sal_uInt16             nDlgType;

    // main selection
    VclPtr<RadioButton>        m_pRbtTransOff;
    VclPtr<RadioButton>        m_pRbtTransLinear;
    VclPtr<RadioButton>        m_pRbtTransGradient;

    /// linear transparency
    VclPtr<MetricField>        m_pMtrTransparent;

    // gradient transparency
    VclPtr<VclGrid>            m_pGridGradient;
    VclPtr<ListBox>            m_pLbTrgrGradientType;
    VclPtr<FixedText>          m_pFtTrgrCenterX;
    VclPtr<MetricField>        m_pMtrTrgrCenterX;
    VclPtr<FixedText>          m_pFtTrgrCenterY;
    VclPtr<MetricField>        m_pMtrTrgrCenterY;
    VclPtr<FixedText>          m_pFtTrgrAngle;
    VclPtr<MetricField>        m_pMtrTrgrAngle;
    VclPtr<MetricField>        m_pMtrTrgrBorder;
    VclPtr<MetricField>        m_pMtrTrgrStartValue;
    VclPtr<MetricField>        m_pMtrTrgrEndValue;

    // preview
    VclPtr<SvxXRectPreview>    m_pCtlBitmapPreview;
    VclPtr<SvxXRectPreview>    m_pCtlXRectPreview;
    bool                bBitmap;

    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    DECL_LINK(ClickTransOffHdl_Impl, Button*, void );
    DECL_LINK(ClickTransLinearHdl_Impl, Button*, void );
    DECL_LINK(ClickTransGradientHdl_Impl, Button*, void );
    DECL_LINK(ModifyTransparentHdl_Impl, Edit&, void);
    DECL_LINK(ModifiedTrgrEditHdl_Impl, Edit&, void);
    DECL_LINK(ModifiedTrgrListBoxHdl_Impl, ListBox&, void);
    void ModifiedTrgrHdl_Impl(void*);

    void ActivateLinear(bool bActivate);
    void ActivateGradient(bool bActivate);
    void SetControlState_Impl(css::awt::GradientStyle eXGS);

    bool InitPreview ( const SfxItemSet& rSet );
    void InvalidatePreview (bool bEnable = true );

public:
    SvxTransparenceTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxTransparenceTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window*, const SfxItemSet*);
    static const sal_uInt16* GetRanges() { return pTransparenceRanges; }

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
    virtual void ChangesApplied() override;
    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual void PointChanged(vcl::Window* pWindow, RectPoint eRP) override;

    void SetPageType(PageType nInType) { nPageType = nInType; }
    void SetDlgType(sal_uInt16 nInType) { nDlgType = nInType; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/************************************************************************/

class SvxAreaTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pAreaRanges[];
private:
    ScopedVclPtr<SfxTabPage>   m_pFillTabPage;
    VclPtr<VclBox>             m_pFillTab;
    VclPtr<PushButton>         m_pBtnNone;
    VclPtr<PushButton>         m_pBtnColor;
    VclPtr<PushButton>         m_pBtnGradient;
    VclPtr<PushButton>         m_pBtnHatch;
    VclPtr<PushButton>         m_pBtnBitmap;
    VclPtr<PushButton>         m_pBtnPattern;
    ButtonBox                  maBox;

    XColorListRef         m_pColorList;
    XGradientListRef      m_pGradientList;
    XHatchListRef         m_pHatchingList;
    XBitmapListRef        m_pBitmapList;
    XPatternListRef       m_pPatternList;
    SdrModel*             mpDrawModel;

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

    PageType            m_nPageType;
    sal_Int32           m_nPos;
    sal_uInt16          m_nDlgType;
    bool*               m_pbAreaTP;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    DECL_LINK(SelectFillTypeHdl_Impl, Button*, void);

    template< typename TabPage >
    bool FillItemSet_Impl( SfxItemSet* );
    template< typename TabPage >
    void Reset_Impl( const SfxItemSet* );
    template< typename TabPage >
    DeactivateRC DeactivatePage_Impl( SfxItemSet* pSet );
public:
    SvxAreaTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxAreaTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pAreaRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetGradientList( XGradientListRef const & pGrdLst)
                { m_pGradientList = pGrdLst; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }
    void    SetBitmapList( XBitmapListRef const & pBmpLst) { m_pBitmapList = pBmpLst; }
    void    SetPatternList( XPatternListRef const &pPtrnLst ) { m_pPatternList = pPtrnLst; }
    void    SetDrawModel( SdrModel* pModel ) { mpDrawModel = pModel; }
    void    SetPageType( PageType nInType ) { m_nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_uInt16 nInPos ) { m_nPos = nInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }
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
    VclPtr<TriStateBox>        m_pTsbShowShadow;
    VclPtr<VclGrid>            m_pGridShadow;
    VclPtr<SvxRectCtl>         m_pCtlPosition;
    VclPtr<MetricField>        m_pMtrDistance;
    VclPtr<SvxColorListBox>    m_pLbShadowColor;
    VclPtr<MetricField>        m_pMtrTransparent;
    VclPtr<SvxXShadowPreview>  m_pCtlXRectPreview;

    const SfxItemSet&   m_rOutAttrs;
    RectPoint          m_eRP;

    XColorListRef       m_pColorList;
    ChangeType*         m_pnColorListState;
    PageType            m_nPageType;
    sal_uInt16          m_nDlgType;
    bool*               m_pbAreaTP;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;
    MapUnit             m_ePoolUnit;

    DECL_LINK( ClickShadowHdl_Impl, Button*, void );
    DECL_LINK( ModifyShadowHdl_Impl, Edit&, void );
    DECL_LINK( SelectShadowHdl_Impl, SvxColorListBox&, void );

public:
    SvxShadowTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxShadowTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pShadowRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPageType( PageType nInType ) { m_nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
};

/************************************************************************/

class SvxGradientTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<ListBox>            m_pLbGradientType;
    VclPtr<FixedText>          m_pFtCenter;
    VclPtr<MetricField>        m_pMtrCenterX;
    VclPtr<MetricField>        m_pMtrCenterY;
    VclPtr<FixedText>          m_pFtAngle;
    VclPtr<MetricField>        m_pMtrAngle;
    VclPtr<MetricField>        m_pMtrBorder;
    VclPtr<Slider>             m_pSliderBorder;
    VclPtr<SvxColorListBox>    m_pLbColorFrom;
    VclPtr<MetricField>        m_pMtrColorFrom;
    VclPtr<SvxColorListBox>    m_pLbColorTo;
    VclPtr<MetricField>        m_pMtrColorTo;
    VclPtr<SvxPresetListBox>   m_pGradientLB;
    VclPtr<NumericField>       m_pMtrIncrement;
    VclPtr<CheckBox>           m_pCbIncrement;
    VclPtr<Slider>             m_pSliderIncrement;
    VclPtr<SvxXRectPreview>    m_pCtlPreview;
    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnModify;
    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XGradientListRef      m_pGradientList;

    ChangeType*         m_pnGradientListState;
    ChangeType*         m_pnColorListState;
    sal_Int32*          m_pPos;

    XFillStyleItem      m_aXFStyleItem;
    XFillGradientItem   m_aXGradientItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK( ChangeGradientHdl, ValueSet*, void );
    void ChangeGradientHdl_Impl();
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ModifiedEditHdl_Impl, Edit&, void );
    DECL_LINK( ModifiedColorListBoxHdl_Impl, SvxColorListBox&, void );
    DECL_LINK( ModifiedListBoxHdl_Impl, ListBox&, void );
    DECL_LINK( ChangeAutoStepHdl_Impl, CheckBox&, void );
    DECL_LINK( ModifiedSliderHdl_Impl, Slider*, void );
    void ModifiedHdl_Impl(void*);

    void SetControlState_Impl( css::awt::GradientStyle eXGS );
    sal_Int32 SearchGradientList(const OUString& rGradientName);

public:
    SvxGradientTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxGradientTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetGradientList( XGradientListRef const & pGrdLst)
                { m_pGradientList = pGrdLst; }
    void    SetPos( sal_Int32* pPos ) { m_pPos = pPos; }
    void    SetGrdChgd( ChangeType* pIn ) { m_pnGradientListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
};

/************************************************************************/

class SvxHatchTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<MetricField>        m_pMtrDistance;
    VclPtr<MetricField>        m_pMtrAngle;
    VclPtr<Slider>             m_pSliderAngle;
    VclPtr<ListBox>            m_pLbLineType;
    VclPtr<SvxColorListBox>    m_pLbLineColor;
    VclPtr<CheckBox>           m_pCbBackgroundColor;
    VclPtr<SvxColorListBox>    m_pLbBackgroundColor;
    VclPtr<SvxPresetListBox>   m_pHatchLB;
    VclPtr<SvxXRectPreview>    m_pCtlPreview;
    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnModify;

    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XHatchListRef         m_pHatchingList;

    ChangeType*         m_pnHatchingListState;
    ChangeType*         m_pnColorListState;
    sal_Int32*          m_pPos;

    XFillStyleItem      m_aXFStyleItem;
    XFillHatchItem      m_aXHatchItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    MapUnit             m_ePoolUnit;

    DECL_LINK( ChangeHatchHdl, ValueSet*, void );
    void ChangeHatchHdl_Impl();
    DECL_LINK( ModifiedEditHdl_Impl, Edit&, void );
    DECL_LINK( ModifiedListBoxHdl_Impl, ListBox&, void );
    DECL_LINK( ModifiedColorListBoxHdl_Impl, SvxColorListBox&, void );
    DECL_LINK( ToggleHatchBackgroundColor_Impl, CheckBox&, void );
    DECL_LINK( ModifiedBackgroundHdl_Impl, SvxColorListBox&, void );
    DECL_LINK( ModifiedSliderHdl_Impl, Slider*, void );
    void ModifiedHdl_Impl(void*);
    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    sal_Int32 SearchHatchList(const OUString& rHatchName);

public:
    SvxHatchTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxHatchTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }

    void    SetPos( sal_Int32* pPos ) { m_pPos = pPos; }
    void    SetHtchChgd( ChangeType* pIn ) { m_pnHatchingListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
};

/************************************************************************/

class SvxBitmapTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pBitmapRanges[];
private:

    VclPtr<SvxPresetListBox>   m_pBitmapLB;
    VclPtr<ListBox>            m_pBitmapStyleLB;
    VclPtr<VclBox>             m_pSizeBox;
    VclPtr<CheckBox>           m_pTsbScale;
    VclPtr<MetricField>        m_pBitmapWidth;
    VclPtr<MetricField>        m_pBitmapHeight;
    VclPtr<VclBox>             m_pPositionBox;
    VclPtr<ListBox>            m_pPositionLB;
    VclPtr<VclBox>             m_pPositionOffBox;
    VclPtr<MetricField>        m_pPositionOffX;
    VclPtr<MetricField>        m_pPositionOffY;
    VclPtr<VclBox>             m_pTileOffBox;
    VclPtr<ListBox>            m_pTileOffLB;
    VclPtr<MetricField>        m_pTileOffset;
    VclPtr<PushButton>         m_pBtnImport;
    VclPtr<SvxXRectPreview>    m_pCtlBitmapPreview;

    const SfxItemSet&          m_rOutAttrs;

    XBitmapListRef             m_pBitmapList;
    ChangeType*                m_pnBitmapListState;
    XFillStyleItem             m_aXFStyleItem;
    XFillBitmapItem            m_aXBitmapItem;

    double                     m_fObjectWidth;
    double                     m_fObjectHeight;
    sal_Int32*                 m_pPos;

    XFillAttrSetItem           m_aXFillAttr;
    SfxItemSet&                m_rXFSet;
    const SdrView*             mpView;
    MapUnit                    mePoolUnit;
    Size                       rBitmapSize;
    Size                       rFilledSize;
    Size                       rZoomedSize;
    sal_Int64                  nFilledWidthPercent;
    sal_Int64                  nFilledHeightPercent;
    sal_Int64                  nZoomedWidthPercent;
    sal_Int64                  nZoomedHeightPercent;
    DECL_LINK( ModifyBitmapHdl, ValueSet*, void );
    DECL_LINK( ClickScaleHdl, Button*, void );
    DECL_LINK( ModifyBitmapStyleHdl, ListBox&, void );
    DECL_LINK( ModifyBitmapSizeHdl, Edit&, void );
    DECL_LINK( ModifyBitmapPositionHdl, ListBox&, void );
    DECL_LINK( ModifyPositionOffsetHdl, Edit&, void );
    DECL_LINK( ModifyTileOffsetHdl, Edit&, void );
    DECL_LINK( ClickRenameHdl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl, SvxPresetListBox*, void );
    DECL_LINK( ClickImportHdl, Button*, void );
    void ClickBitmapHdl_Impl();
    void CalculateBitmapPresetSize();
    sal_Int32 SearchBitmapList(const OUString& rBitmapName);

public:
    SvxBitmapTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxBitmapTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void    SetBitmapList( const XBitmapListRef& pBmpLst) { m_pBitmapList = pBmpLst; }
    void    SetPos( sal_Int32* pPos ) { m_pPos = pPos; }
    void    SetBmpChgd( ChangeType* pIn ) { m_pnBitmapListState = pIn; }
};

/************************************************************************/

class SvxPatternTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    VclPtr<VclBox>             m_pBxPixelEditor;
    VclPtr<SvxPixelCtl>        m_pCtlPixel;
    VclPtr<SvxColorListBox>    m_pLbColor;
    VclPtr<SvxColorListBox>    m_pLbBackgroundColor;
    VclPtr<SvxPresetListBox>   m_pPatternLB;
    VclPtr<SvxXRectPreview>    m_pCtlPreview;
    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnModify;
    VclPtr<PushButton>         m_pBtnDelete;

    SvxBitmapCtl*       m_pBitmapCtl;

    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XPatternListRef       m_pPatternList;

    ChangeType*         m_pnPatternListState;
    ChangeType*         m_pnColorListState;
    sal_Int32*          m_pPos;

    bool                m_bPtrnChanged;

    XFillStyleItem      m_aXFStyleItem;
    XFillBitmapItem     m_aXPatternItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK( ChangePatternHdl_Impl, ValueSet*, void );
    DECL_LINK( ChangeColorHdl_Impl, SvxColorListBox&, void );
    DECL_LINK( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    sal_Int32 SearchPatternList(const OUString& rPatternName);

public:
    SvxPatternTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxPatternTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPatternList( XPatternListRef const & pPatternList) { m_pPatternList = pPatternList; }
    void    SetPos( sal_Int32* pPos ) { m_pPos = pPos; }
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
    XPropertyListType   meType;

    VclPtr<Window>             mpTopDlg;

    PaletteManager             maPaletteManager;
    VclPtr<ListBox>            m_pSelectPalette;
    VclPtr<SvxColorValueSet>   m_pValSetColorList;
    VclPtr<SvxColorValueSet>   m_pValSetRecentList;

    VclPtr<SvxXRectPreview>    m_pCtlPreviewOld;
    VclPtr<SvxXRectPreview>    m_pCtlPreviewNew;

    VclPtr<RadioButton>        m_pRbRGB;
    VclPtr<RadioButton>        m_pRbCMYK;

    VclPtr<VclContainer>       m_pRGBcustom;
    VclPtr<VclContainer>       m_pRGBpreset;
    VclPtr<NumericField>       m_pRcustom;
    VclPtr<NumericField>       m_pRpreset;
    VclPtr<NumericField>       m_pGcustom;
    VclPtr<NumericField>       m_pGpreset;
    VclPtr<NumericField>       m_pBcustom;
    VclPtr<NumericField>       m_pBpreset;
    VclPtr<HexColorControl>    m_pHexpreset;
    VclPtr<HexColorControl>    m_pHexcustom;

    VclPtr<VclContainer>       m_pCMYKcustom;
    VclPtr<VclContainer>       m_pCMYKpreset;
    VclPtr<MetricField>        m_pCcustom;
    VclPtr<MetricField>        m_pCpreset;
    VclPtr<MetricField>        m_pYcustom;
    VclPtr<MetricField>        m_pYpreset;
    VclPtr<MetricField>        m_pMcustom;
    VclPtr<MetricField>        m_pMpreset;
    VclPtr<MetricField>        m_pKcustom;
    VclPtr<MetricField>        m_pKpreset;

    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnDelete;
    VclPtr<PushButton>         m_pBtnWorkOn;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;

    ChangeType*         pnColorListState;
    sal_Int32*          pPos;

    XFillStyleItem      aXFStyleItem;
    XFillColorItem      aXFillColorItem;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    ColorModel          eCM;

    Color               aPreviousColor;
    Color               aCurrentColor;

    static void    ConvertColorValues (Color& rColor, ColorModel eModell);
    static void    RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK );
    static void    CmykToRgb_Impl( Color& rColor, const sal_uInt16 nKey );
    sal_uInt16  ColorToPercent_Impl( sal_uInt16 nColor );
    sal_uInt16  PercentToColor_Impl( sal_uInt16 nPercent );

    void ImpColorCountChanged();
    void FillPaletteLB();

    DECL_LINK( ClickAddHdl_Impl, Button*, void );
    DECL_LINK( ClickWorkOnHdl_Impl, Button*, void );
    DECL_LINK( ClickDeleteHdl_Impl, Button*, void );

    DECL_LINK( SelectPaletteLBHdl, ListBox&, void );
    DECL_LINK( SelectValSetHdl_Impl, ValueSet*, void );
    DECL_LINK( SelectColorModeHdl_Impl, RadioButton&, void );
    void ChangeColor(const Color &rNewColor, bool bUpdatePreset = true);
    void SetColorModel(ColorModel eModel);
    void ChangeColorModel();
    void UpdateColorValues( bool bUpdatePreset = true );
    static sal_Int32 SearchColorList(OUString const & aColorName);
    DECL_LINK( ModifiedHdl_Impl, Edit&, void );

    void UpdateModified();
    css::uno::Reference< css::uno::XComponentContext > m_context;
public:
    SvxColorTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxColorTabPage() override;
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef );
    void    SetPos( sal_Int32* pInPos ) { pPos = pInPos; }
    void    SetColorList( const XColorListRef& pColList );


    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void FillUserData() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABAREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
