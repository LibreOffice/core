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

class SdrModel;
class SdrView;
class SvxBitmapCtl;

/************************************************************************/

class SvxAreaTabDialog : public SfxTabDialog
{
    sal_uInt16            m_nAreaTabPage;
    sal_uInt16            m_nShadowTabPage;
    sal_uInt16            m_nTransparenceTabPage;
    sal_uInt16            m_nColorTabPage;
    sal_uInt16            m_nGradientTabPage;
    sal_uInt16            m_nHatchTabPage;
    sal_uInt16            m_nBitmapTabPage;
    sal_uInt16            m_nPatternTabPage;

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

    sal_uInt16          mnPageType;
    sal_Int32           mnPos;
    bool                mbAreaTP;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

protected:
    virtual short       Ok() override;
    DECL_LINK_TYPED( CancelHdlImpl, Button*, void );
    void                SavePalettes();

public:
    SvxAreaTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel, bool bShadow );

    void                SetNewColorList( XColorListRef const & pColorList )
                            { mpNewColorList = pColorList; }
    const XColorListRef&  GetNewColorList() const { return mpNewColorList; }
    const XColorListRef&  GetColorList() const { return mpColorList; }

    void                  SetNewGradientList( XGradientListRef const & pGrdLst)
                            { mpNewGradientList = pGrdLst; }
    const XGradientListRef&  GetNewGradientList() const
                            { return mpNewGradientList; }

    void                  SetNewHatchingList( XHatchListRef const & pHtchLst)
                            { mpNewHatchingList = pHtchLst; }
    const XHatchListRef&  GetNewHatchingList() const
                            { return mpNewHatchingList; }

    void                  SetNewBitmapList( XBitmapListRef const & pBmpLst)
                            { mpNewBitmapList = pBmpLst; }
    const XBitmapListRef& GetNewBitmapList() const { return mpNewBitmapList; }

    void                  SetNewPatternList( XPatternListRef const & pPtrnLst )
                            { mpNewPatternList = pPtrnLst; }
    const XPatternListRef& GetNewPatternList() const { return mpNewPatternList; }
};

/************************************************************************/

class SvxTransparenceTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pTransparenceRanges[];

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

    sal_uInt16             nPageType;
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

    DECL_LINK_TYPED(ClickTransOffHdl_Impl, Button*, void );
    DECL_LINK_TYPED(ClickTransLinearHdl_Impl, Button*, void );
    DECL_LINK_TYPED(ClickTransGradientHdl_Impl, Button*, void );
    DECL_LINK_TYPED(ModifyTransparentHdl_Impl, Edit&, void);
    DECL_LINK_TYPED(ModifiedTrgrEditHdl_Impl, Edit&, void);
    DECL_LINK_TYPED(ModifiedTrgrListBoxHdl_Impl, ListBox&, void);
    void ModifiedTrgrHdl_Impl(void*);

    void ActivateLinear(bool bActivate);
    void ActivateGradient(bool bActivate);
    void SetControlState_Impl(css::awt::GradientStyle eXGS);

    bool InitPreview ( const SfxItemSet& rSet );
    void InvalidatePreview (bool bEnable = true );

public:
    SvxTransparenceTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxTransparenceTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window*, const SfxItemSet*);
    static const sal_uInt16* GetRanges() { return pTransparenceRanges; }

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
    virtual void ChangesApplied() override;
    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual void PointChanged(vcl::Window* pWindow, RECT_POINT eRP) override;

    void SetPageType(sal_uInt16 nInType) { nPageType = nInType; }
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
    VclPtr<ListBox>            m_pTypeLB;

    VclPtr<VclBox>             m_pFillLB;
    VclPtr<ColorLB>            m_pLbColor;
    VclPtr<GradientLB>         m_pLbGradient;
    VclPtr<HatchingLB>         m_pLbHatching;
    VclPtr<BitmapLB>           m_pLbBitmap;
    VclPtr<SvxXRectPreview>    m_pCtlBitmapPreview;
    VclPtr<SvxXRectPreview>    m_pCtlXRectPreview;

    const SfxItemSet&   m_rOutAttrs;
    RECT_POINT          m_eRP;

    XColorListRef         m_pColorList;
    XGradientListRef      m_pGradientList;
    XHatchListRef         m_pHatchingList;
    XBitmapListRef        m_pBitmapList;

    // Placeholders for pointer-based entries; these will be inited
    // to point to these so that the page is usable without that
    // SvxAreaTabDialog has to call the setter methods (e.g. SetColorChgd).
    // Without that the pages used in SvxAreaTabDialog are not usable
    ChangeType          maFixed_ChangeType;
    bool                maFixed_sal_Bool;

    ChangeType*         m_pnColorListState;
    ChangeType*         m_pnBitmapListState;
    ChangeType*         m_pnGradientListState;
    ChangeType*         m_pnHatchingListState;

    sal_uInt16 m_nPageType;
    sal_uInt16 m_nDlgType;
    sal_Int32  m_nPos;

    bool*               m_pbAreaTP;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    SfxMapUnit          m_ePoolUnit;
    FieldUnit           m_eFUnit;

    //UUUU
    bool                mbOfferImportButton;
    bool                mbDirectGraphicSet;
    Graphic             maDirectGraphic;
    OUString            maDirectName;
    VclPtr<PushButton>         m_pBtnImport;

    DECL_LINK_TYPED(SelectDialogTypeHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED( ModifyColorHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ModifyGradientHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ModifyHatchingHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ModifyBitmapHdl_Impl, ListBox&, void );

    //UUUU
    DECL_LINK_TYPED( ClickImportHdl_Impl, Button*, void );

    DECL_LINK_TYPED( ModifyTileHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ModifyTileClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickScaleHdl_Impl, Button*, void );
    void ClickInvisibleHdl_Impl();
    void ClickHatchingHdl_Impl();
    void ClickGradientHdl_Impl();
    void ClickColorHdl_Impl();
    void ClickBitmapHdl_Impl();

public:
    SvxAreaTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxAreaTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pAreaRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ChangesApplied() override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetGradientList( XGradientListRef const & pGrdLst)
                { m_pGradientList = pGrdLst; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }
    void    SetBitmapList( XBitmapListRef const & pBmpLst) { m_pBitmapList = pBmpLst; }

    void    SetPageType( sal_uInt16 nInType ) { m_nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_uInt16 nInPos ) { m_nPos = nInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }
    virtual void PageCreated(const SfxAllItemSet& aSet) override;
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    void    SetGrdChgd( ChangeType* pIn ) { m_pnGradientListState = pIn; }
    void    SetHtchChgd( ChangeType* pIn ) { m_pnHatchingListState = pIn; }
    void    SetBmpChgd( ChangeType* pIn ) { m_pnBitmapListState = pIn; }
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
    VclPtr<ColorLB>            m_pLbShadowColor;
    VclPtr<MetricField>        m_pMtrTransparent;
    VclPtr<SvxXShadowPreview>  m_pCtlXRectPreview;

    const SfxItemSet&   m_rOutAttrs;
    RECT_POINT          m_eRP;

    XColorListRef       m_pColorList;
    ChangeType*         m_pnColorListState;
    sal_uInt16          m_nPageType;
    sal_uInt16          m_nDlgType;
    bool*               m_pbAreaTP;

    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;
    SfxMapUnit          m_ePoolUnit;

    DECL_LINK_TYPED( ClickShadowHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ModifyShadowHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( SelectShadowHdl_Impl, ListBox&, void );

public:
    SvxShadowTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxShadowTabPage();
    virtual void dispose() override;

    void    Construct();
    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pShadowRanges; }

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPageType( sal_uInt16 nInType ) { m_nPageType = nInType; }
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
    VclPtr<ColorLB>            m_pLbColorFrom;
    VclPtr<MetricField>        m_pMtrColorFrom;
    VclPtr<ColorLB>            m_pLbColorTo;
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
    sal_uInt16*         m_pPageType;
    sal_uInt16          m_nDlgType;
    sal_Int32*          m_pPos;
    bool*               m_pbAreaTP;

    XFillStyleItem      m_aXFStyleItem;
    XFillGradientItem   m_aXGradientItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangeGradientHdl, ValueSet*, void );
    void ChangeGradientHdl_Impl();
    DECL_LINK_TYPED( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ClickDeleteHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ModifiedEditHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ModifiedListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ChangeAutoStepHdl_Impl, CheckBox&, void );
    DECL_LINK_TYPED( ModifiedSliderHdl_Impl, Slider*, void );
    void ModifiedHdl_Impl(void*);

    long CheckChanges_Impl();
    void SetControlState_Impl( css::awt::GradientStyle eXGS );
    sal_Int32 SearchGradientList(const OUString& rGradientName);

public:
    SvxGradientTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxGradientTabPage();
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

    void    SetPageType( sal_uInt16* pInType ) { m_pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_Int32* pInPos ) { m_pPos = pInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }

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
    VclPtr<ColorLB>            m_pLbLineColor;
    VclPtr<CheckBox>           m_pCbBackgroundColor;
    VclPtr<ColorLB>            m_pLbBackgroundColor;
    VclPtr<SvxPresetListBox>   m_pHatchLB;
    VclPtr<SvxXRectPreview>    m_pCtlPreview;
    VclPtr<PushButton>         m_pBtnAdd;
    VclPtr<PushButton>         m_pBtnModify;

    const SfxItemSet&   m_rOutAttrs;

    XColorListRef         m_pColorList;
    XHatchListRef         m_pHatchingList;

    ChangeType*         m_pnHatchingListState;
    ChangeType*         m_pnColorListState;
    sal_uInt16*         m_pPageType;
    sal_uInt16          m_nDlgType;
    sal_Int32*          m_pPos;
    bool*               m_pbAreaTP;

    XFillStyleItem      m_aXFStyleItem;
    XFillHatchItem      m_aXHatchItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    SfxMapUnit          m_ePoolUnit;

    DECL_LINK_TYPED( ChangeHatchHdl, ValueSet*, void );
    void ChangeHatchHdl_Impl();
    DECL_LINK_TYPED( ModifiedEditHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ModifiedListBoxHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ToggleHatchBackgroundColor_Impl, CheckBox&, void );
    DECL_LINK_TYPED( ModifiedBackgroundHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ModifiedSliderHdl_Impl, Slider*, void );
    void ModifiedHdl_Impl(void*);
    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    long CheckChanges_Impl();
    sal_Int32 SearchHatchList(const OUString& rHatchName);

public:
    SvxHatchTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxHatchTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetHatchingList( XHatchListRef const & pHtchLst)
                { m_pHatchingList = pHtchLst; }

    void    SetPageType( sal_uInt16* pInType ) { m_pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_Int32* pInPos ) { m_pPos = pInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }

    void    SetHtchChgd( ChangeType* pIn ) { m_pnHatchingListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
};

/************************************************************************/

class SvxBitmapTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;
private:

    VclPtr<SvxPresetListBox>   m_pBitmapLB;
    VclPtr<PushButton>         m_pBtnImport;
    VclPtr<SvxXRectPreview>    m_pCtlBitmapPreview;

    const SfxItemSet&          m_rOutAttrs;

    XBitmapListRef             m_pBitmapList;
    ChangeType*                m_pnBitmapListState;
    XFillStyleItem             m_aXFStyleItem;
    XFillBitmapItem            m_aXBitmapItem;

    sal_uInt16*                m_nPageType;
    sal_uInt16                 m_nDlgType;
    sal_Int32*                 m_nPos;

    bool*                      m_pbAreaTP;

    XFillAttrSetItem           m_aXFillAttr;
    SfxItemSet&                m_rXFSet;

    DECL_LINK_TYPED( ModifyBitmapHdl, ValueSet*, void );
    DECL_LINK_TYPED( ClickRenameHdl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ClickDeleteHdl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ClickImportHdl, Button*, void );
    void ClickBitmapHdl_Impl();
    sal_Int32 SearchBitmapList(const OUString& rBitmapName);

public:
    SvxBitmapTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxBitmapTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;
    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void    SetBitmapList( XBitmapListRef pBmpLst) { m_pBitmapList = pBmpLst; }

    void    SetPageType( sal_uInt16* pInType ) { m_nPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_Int32* pInPos ) { m_nPos = pInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }
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
    VclPtr<ColorLB>            m_pLbColor;
    VclPtr<ColorLB>            m_pLbBackgroundColor;
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
    sal_uInt16*         m_pPageType;
    sal_uInt16          m_nDlgType;
    sal_Int32*          m_pPos;
    bool*               m_pbAreaTP;

    bool                m_bPtrnChanged;

    XFillStyleItem      m_aXFStyleItem;
    XFillBitmapItem     m_aXPatternItem;
    XFillAttrSetItem    m_aXFillAttr;
    SfxItemSet&         m_rXFSet;

    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickModifyHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangePatternHdl_Impl, ValueSet*, void );
    DECL_LINK_TYPED( ChangeColorHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( ClickRenameHdl_Impl, SvxPresetListBox*, void );
    DECL_LINK_TYPED( ClickDeleteHdl_Impl, SvxPresetListBox*, void );

    long CheckChanges_Impl();

public:
    SvxPatternTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxPatternTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    virtual void PointChanged( vcl::Window* pWindow, RECT_POINT eRP ) override;

    void    SetColorList( XColorListRef const & pColorList ) { m_pColorList = pColorList; }
    void    SetPatternList( XPatternListRef const & pPatternList) { m_pPatternList = pPatternList; }

    void    SetPageType( sal_uInt16* pInType ) { m_pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { m_nDlgType = nInType; }
    void    SetPos( sal_Int32* pInPos ) { m_pPos = pInPos; }
    void    SetAreaTP( bool* pIn ) { m_pbAreaTP = pIn; }

    void    SetPtrnChgd( ChangeType* pIn ) { m_pnPatternListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { m_pnColorListState = pIn; }
    void    ChangeColor_Impl();
};

/************************************************************************/

struct SvxColorTabPageShadow;
class SvxColorTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    XPropertyListType   meType;

    VclPtr<Window>             mpTopDlg;

    SvxColorTabPageShadow *pShadow;

    std::vector< std::unique_ptr<Palette> > m_Palettes;
    VclPtr<ListBox>            m_pSelectPalette;
    VclPtr<SvxColorValueSet>   m_pValSetColorList;
    VclPtr<SvxColorValueSet>   m_pValSetCustomList;

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
    VclPtr<PushButton>         m_pBtnWorkOn;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;
    XColorListRef         pUserColorList;

    ChangeType*         pnColorListState;
    sal_uInt16*         pPageType;
    sal_uInt16          nDlgType;
    sal_Int32*          pPos;
    sal_Int32           nUserPos;
    bool*               pbAreaTP;

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
    void LoadPalettes();
    void FillPaletteLB();

    DECL_LINK_TYPED( ClickAddHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ClickWorkOnHdl_Impl, Button*, void );

    DECL_LINK_TYPED( SelectPaletteLBHdl, ListBox&, void );
    void SelectPaletteLBHdl_Impl();
    DECL_LINK_TYPED( SelectValSetHdl_Impl, ValueSet*, void );
    DECL_LINK_TYPED( SelectColorModeHdl_Impl, RadioButton&, void );
    void ChangeColor(const Color &rNewColor);
    void SetColorModel(ColorModel eModel);
    void ChangeColorModel();
    void UpdateColorValues();
    sal_Int32 SearchColorList(OUString const & aColorName);
    DECL_LINK_TYPED( ModifiedHdl_Impl, Edit&, void );

    long CheckChanges_Impl();

    void UpdateModified();
public:
    SvxColorTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxColorTabPage();
    virtual void dispose() override;

    void    Construct();

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual void ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void             SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef );

    void    SetColorList( const XColorListRef& pColList );
    const XColorListRef& GetColorList() { return pColorList; }
    void    SaveToViewFrame( SfxViewFrame *pViewFrame );
    void    SetupForViewFrame( SfxViewFrame *pViewFrame );

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPos( sal_Int32* pInPos ) { pPos = pInPos; }
    void    SetAreaTP( bool* pIn ) { pbAreaTP = pIn; }

    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void FillUserData() override;

    bool IsModified()
    {
        return bool(*pnColorListState & ChangeType::MODIFIED);
    }
    void SetModified(bool bIsModified)
    {
        if (bIsModified)
            *pnColorListState |= ChangeType::MODIFIED;
        else
            *pnColorListState &= ~ChangeType::MODIFIED;
    }
    void AddState(ChangeType nState)
    {
        *pnColorListState |= nState;
    }
    void Update(bool bLoaded);
};

#endif // INCLUDED_CUI_SOURCE_INC_CUITABAREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
