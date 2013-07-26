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
#ifndef _CUI_TAB_AREA_HXX
#define _CUI_TAB_AREA_HXX

#include <svtools/valueset.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/xsetit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xfillit.hxx>
#include <svx/tabarea.hxx>
#include <svx/SvxColorValueSet.hxx>

class SdrModel;
class SdrView;

/************************************************************************/

class SvxAreaTabDialog : public SfxTabDialog
{
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

    const SfxItemSet&   mrOutAttrs;

    ChangeType          mnColorListState;
    ChangeType          mnBitmapListState;
    ChangeType          mnGradientListState;
    ChangeType          mnHatchingListState;

    sal_uInt16              mnPageType;
    sal_uInt16              mnDlgType;
    sal_uInt16              mnPos;
    sal_Bool                mbAreaTP;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

protected:
    virtual short       Ok();
    DECL_LINK( CancelHdlImpl, void * );
    void                SavePalettes();

public:
    SvxAreaTabDialog( Window* pParent,
                      const SfxItemSet* pAttr, SdrModel* pModel,
                      const SdrView* pSdrView = NULL );
    ~SvxAreaTabDialog();

    void                SetNewColorList( XColorListRef pColTab )
                            { mpNewColorList = pColTab; }
    XColorListRef         GetNewColorList() const { return mpNewColorList; }
    const XColorListRef   GetColorList() const { return mpColorList; }

    void                SetNewGradientList( XGradientListRef pGrdLst)
                            { mpNewGradientList = pGrdLst; }
    XGradientListRef       GetNewGradientList() const
                            { return mpNewGradientList; }
    const XGradientListRef GetGradientList() const { return mpGradientList; }

    void                 SetNewHatchingList( XHatchListRef pHtchLst)
                            { mpNewHatchingList = pHtchLst; }
    XHatchListRef          GetNewHatchingList() const
                            { return mpNewHatchingList; }
    const XHatchListRef    GetHatchingList() const { return mpHatchingList; }

    void                 SetNewBitmapList( XBitmapListRef pBmpLst)
                            { mpNewBitmapList = pBmpLst; }
    XBitmapListRef         GetNewBitmapList() const { return mpNewBitmapList; }
    const XBitmapListRef   GetBitmapList() const { return mpBitmapList; }
};

/************************************************************************/

class SvxTransparenceTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

    sal_uInt16          nPageType;
    sal_uInt16          nDlgType;

    // main selection
    RadioButton*        m_pRbtTransOff;
    RadioButton*        m_pRbtTransLinear;
    RadioButton*        m_pRbtTransGradient;

    /// linear transparency
    MetricField*        m_pMtrTransparent;

    // gradient transparency
    VclGrid*            m_pGridGradient;
    ListBox*            m_pLbTrgrGradientType;
    FixedText*          m_pFtTrgrCenterX;
    MetricField*        m_pMtrTrgrCenterX;
    FixedText*          m_pFtTrgrCenterY;
    MetricField*        m_pMtrTrgrCenterY;
    FixedText*          m_pFtTrgrAngle;
    MetricField*        m_pMtrTrgrAngle;
    MetricField*        m_pMtrTrgrBorder;
    MetricField*        m_pMtrTrgrStartValue;
    MetricField*        m_pMtrTrgrEndValue;

    // preview
    SvxXRectPreview*    m_pCtlBitmapPreview;
    SvxXRectPreview*    m_pCtlXRectPreview;
    sal_Bool                bBitmap;

    XOutdevItemPool*    pXPool;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    DECL_LINK(ClickTransOffHdl_Impl, void * );
    DECL_LINK(ClickTransLinearHdl_Impl, void * );
    DECL_LINK(ClickTransGradientHdl_Impl, void * );
    DECL_LINK(ModifyTransparentHdl_Impl, void*);
    DECL_LINK(ChangeTrgrTypeHdl_Impl, void*);
    DECL_LINK(ModifiedTrgrHdl_Impl, void*);

    void ActivateLinear(sal_Bool bActivate);
    void ActivateGradient(sal_Bool bActivate);
    void SetControlState_Impl(XGradientStyle eXGS);

    sal_Bool InitPreview ( const SfxItemSet& rSet );
    void InvalidatePreview (sal_Bool bEnable = sal_True );

public:
    SvxTransparenceTabPage(Window* pParent, const SfxItemSet& rInAttrs);
    void Construct();

    static SfxTabPage* Create(Window*, const SfxItemSet&);
    static sal_uInt16* GetRanges();

    virtual sal_Bool FillItemSet(SfxItemSet&);
    virtual void Reset(const SfxItemSet&);
    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet);
    virtual void PointChanged(Window* pWindow, RECT_POINT eRP);

    void SetPageType(sal_uInt16 nInType) { nPageType = nInType; }
    void SetDlgType(sal_uInt16 nInType) { nDlgType = nInType; }
    virtual void PageCreated (SfxAllItemSet aSet);
};

/************************************************************************/

class SvxAreaTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    ListBox*            m_pTypeLB;

    VclBox*             m_pFillLB;
    ColorLB*            m_pLbColor;
    GradientLB*         m_pLbGradient;
    HatchingLB*         m_pLbHatching;
    BitmapLB*           m_pLbBitmap;
    SvxXRectPreview*    m_pCtlBitmapPreview;

    TriStateBox*        m_pTsbStepCount;
    VclFrame*           m_pFlStepCount;
    NumericField*       m_pNumFldStepCount;

    VclFrame*           m_pFlHatchBckgrd;
    CheckBox*           m_pCbxHatchBckgrd;
    ColorLB*            m_pLbHatchBckgrdColor;

    VclBox*             m_pBxBitmap;

    VclFrame*           m_pFlSize;
    TriStateBox*        m_pTsbOriginal;
    TriStateBox*        m_pTsbScale;
    VclGrid*            m_pGridX_Y;
    FixedText*          m_pFtXSize;
    MetricField*        m_pMtrFldXSize;
    FixedText*          m_pFtYSize;
    MetricField*        m_pMtrFldYSize;

    VclFrame*           m_pFlPosition;
    SvxRectCtl*         m_pCtlPosition;
    VclGrid*            m_pGridOffset;
    MetricField*        m_pMtrFldXOffset;
    MetricField*        m_pMtrFldYOffset;
    VclBox*             m_pBxTile;
    TriStateBox*        m_pTsbTile;
    TriStateBox*        m_pTsbStretch;
    VclFrame*           m_pFlOffset;
    RadioButton*        m_pRbtRow;
    RadioButton*        m_pRbtColumn;
    MetricField*        m_pMtrFldOffset;

    SvxXRectPreview*    m_pCtlXRectPreview;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

    XColorListRef         pColorList;
    XGradientListRef      pGradientList;
    XHatchListRef         pHatchingList;
    XBitmapListRef        pBitmapList;

    ChangeType*         pnColorListState;
    ChangeType*         pnBitmapListState;
    ChangeType*         pnGradientListState;
    ChangeType*         pnHatchingListState;

    sal_uInt16 nPageType;
    sal_uInt16 nDlgType;
    sal_uInt16 nPos;

    sal_Bool*               pbAreaTP;

    XOutdevItemPool*    pXPool;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    SfxMapUnit          ePoolUnit;
    FieldUnit           eFUnit;

    DECL_LINK(SelectDialogTypeHdl_Impl, void *);
    DECL_LINK( ModifyColorHdl_Impl, void * );
    DECL_LINK( ModifyHatchBckgrdColorHdl_Impl, void * );
    DECL_LINK( ModifyGradientHdl_Impl, void * );
    DECL_LINK( ModifyHatchingHdl_Impl, void * );
    DECL_LINK( ToggleHatchBckgrdColorHdl_Impl, void * );
    DECL_LINK( ModifyBitmapHdl_Impl, void * );
    DECL_LINK( ModifyStepCountHdl_Impl, void * );
    DECL_LINK( ModifyTileHdl_Impl, void * );
    DECL_LINK( ClickScaleHdl_Impl, void * );
    void ClickInvisibleHdl_Impl();
    void ClickHatchingHdl_Impl();
    void ClickGradientHdl_Impl();
    void ClickColorHdl_Impl();
    void ClickBitmapHdl_Impl();

public:
    SvxAreaTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );
    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );
    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetGradientList( XGradientListRef pGrdLst)
                { pGradientList = pGrdLst; }
    void    SetHatchingList( XHatchListRef pHtchLst)
                { pHatchingList = pHtchLst; }
    void    SetBitmapList( XBitmapListRef pBmpLst) { pBitmapList = pBmpLst; }

    void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetPos( sal_uInt16 nInPos ) { nPos = nInPos; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }
    virtual void PageCreated (SfxAllItemSet aSet);
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }
    void    SetGrdChgd( ChangeType* pIn ) { pnGradientListState = pIn; }
    void    SetHtchChgd( ChangeType* pIn ) { pnHatchingListState = pIn; }
    void    SetBmpChgd( ChangeType* pIn ) { pnBitmapListState = pIn; }
};


class SvxShadowTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    TriStateBox*        m_pTsbShowShadow;
    VclGrid*            m_pGridShadow;
    SvxRectCtl*         m_pCtlPosition;
    MetricField*        m_pMtrDistance;
    ColorLB*            m_pLbShadowColor;
    MetricField*        m_pMtrTransparent;
    SvxXShadowPreview*  m_pCtlXRectPreview;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

    XColorListRef         pColorList;
    ChangeType*         pnColorListState;
    sal_uInt16              nPageType;
    sal_uInt16              nDlgType;
    sal_Bool*               pbAreaTP;

    sal_Bool                bDisable;

    XOutdevItemPool*    pXPool;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;
    SfxMapUnit          ePoolUnit;

    DECL_LINK( ClickShadowHdl_Impl, void * );
    DECL_LINK( ModifyShadowHdl_Impl, void * );

public:
    SvxShadowTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();
    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );
    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );
    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetPageType( sal_uInt16 nInType ) { nPageType = nInType; }
    void    SetDlgType( sal_uInt16 nInType ) { nDlgType = nInType; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }
    virtual void PageCreated (SfxAllItemSet aSet);
    void    DisablePage( sal_Bool bIn ) { bDisable = bIn; }
};

/************************************************************************/

class SvxGradientTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    ListBox*            m_pLbGradientType;
    FixedText*          m_pFtCenterX;
    MetricField*        m_pMtrCenterX;
    FixedText*          m_pFtCenterY;
    MetricField*        m_pMtrCenterY;
    FixedText*          m_pFtAngle;
    MetricField*        m_pMtrAngle;
    MetricField*        m_pMtrBorder;
    ColorLB*            m_pLbColorFrom;
    MetricField*        m_pMtrColorFrom;
    ColorLB*            m_pLbColorTo;
    MetricField*        m_pMtrColorTo;
    GradientLB*         m_pLbGradients;
    SvxXRectPreview*    m_pCtlPreview;
    PushButton*         m_pBtnAdd;
    PushButton*         m_pBtnModify;
    PushButton*         m_pBtnDelete;
    PushButton*         m_pBtnLoad;
    PushButton*         m_pBtnSave;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;
    XGradientListRef      pGradientList;

    ChangeType*         pnGradientListState;
    ChangeType*         pnColorListState;
    sal_uInt16*             pPageType;
    sal_uInt16*             pDlgType;
    sal_uInt16*             pPos;
    sal_Bool*               pbAreaTP;

    XOutdevItemPool*    pXPool;
    XFillStyleItem      aXFStyleItem;
    XFillGradientItem   aXGradientItem;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ChangeGradientHdl_Impl, void * );
    DECL_LINK( ModifiedHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();
    void SetControlState_Impl( XGradientStyle eXGS );

public:
    SvxGradientTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetGradientList( XGradientListRef pGrdLst)
                { pGradientList = pGrdLst; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

    void    SetGrdChgd( ChangeType* pIn ) { pnGradientListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }
};

/************************************************************************/

class SvxHatchTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    MetricField*        m_pMtrDistance;
    MetricField*        m_pMtrAngle;
    SvxRectCtl*         m_pCtlAngle;
    ListBox*            m_pLbLineType;
    ColorLB*            m_pLbLineColor;
    HatchingLB*         m_pLbHatchings;
    SvxXRectPreview*    m_pCtlPreview;
    PushButton*         m_pBtnAdd;
    PushButton*         m_pBtnModify;
    PushButton*         m_pBtnDelete;
    PushButton*         m_pBtnLoad;
    PushButton*         m_pBtnSave;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;
    XHatchListRef         pHatchingList;

    ChangeType*         pnHatchingListState;
    ChangeType*         pnColorListState;
    sal_uInt16*             pPageType;
    sal_uInt16*             pDlgType;
    sal_uInt16*             pPos;
    sal_Bool*               pbAreaTP;

    XOutdevItemPool*    pXPool;
    XFillStyleItem      aXFStyleItem;
    XFillHatchItem      aXHatchItem;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    SfxMapUnit          ePoolUnit;

    DECL_LINK( ChangeHatchHdl_Impl, void * );
    DECL_LINK( ModifiedHdl_Impl, void * );
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();

public:
    SvxHatchTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetHatchingList( XHatchListRef pHtchLst)
                { pHatchingList = pHtchLst; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

    void    SetHtchChgd( ChangeType* pIn ) { pnHatchingListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
};

/************************************************************************/

class SvxBitmapTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    SvxPixelCtl         aCtlPixel;
    FixedText           aFtPixelEdit;
    FixedText           aFtColor;
    ColorLB             aLbColor;
    FixedText           aFtBackgroundColor;
    ColorLB             aLbBackgroundColor;
    FixedText           aLbBitmapsHidden;
    BitmapLB            aLbBitmaps;
    FixedLine           aFlProp;
    SvxXRectPreview     aCtlPreview;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnImport;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;

    SvxBitmapCtl        aBitmapCtl;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;
    XBitmapListRef        pBitmapList;

    ChangeType*         pnBitmapListState;
    ChangeType*         pnColorListState;
    sal_uInt16*             pPageType;
    sal_uInt16*             pDlgType;
    sal_uInt16*             pPos;
    sal_Bool*               pbAreaTP;

    sal_Bool                bBmpChanged;

    XOutdevItemPool*    pXPool;
    XFillStyleItem      aXFStyleItem;
    XFillBitmapItem     aXBitmapItem;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickImportHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ChangeBitmapHdl_Impl, void * );
    DECL_LINK( ChangePixelColorHdl_Impl, void * );
    DECL_LINK( ChangeBackgrndColorHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();

public:
    SvxBitmapTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void    SetColorList( XColorListRef pColTab ) { pColorList = pColTab; }
    void    SetBitmapList( XBitmapListRef pBmpLst) { pBitmapList = pBmpLst; }

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

    void    SetBmpChgd( ChangeType* pIn ) { pnBitmapListState = pIn; }
    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    /** Return a label that is associated with the given control.  This
        label is used to the determine the name for the control.
        @param pLabeled
            The control for which to return a label.
        @return
            Return a label control that provides a name for the specified
            control.
    */
    virtual Window* GetParentLabeledBy( const Window* pLabeled ) const;
};

/************************************************************************/

struct SvxColorTabPageShadow;
class SvxColorTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    XPropertyListType   meType;
    XOutdevItemPool*    mpXPool;

    Window             *mpTopDlg;
    CheckBox           *m_pBoxEmbed;
    PushButton         *m_pBtnLoad;
    PushButton         *m_pBtnSave;
    FixedText          *m_pTableName;

    DECL_LINK( EmbedToggleHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    XPropertyListRef GetList();
    void HideLoadSaveEmbed();
    bool GetEmbed();
    void SetEmbed( bool bEmbed );
    void UpdateTableName();
    void EnableSave( bool bCanSave );

    SvxColorTabPageShadow *pShadow;
    Edit*               m_pEdtName;
    ColorLB*            m_pLbColor;

    SvxColorValueSet*   m_pValSetColorList;

    SvxXRectPreview*    m_pCtlPreviewOld;
    SvxXRectPreview*    m_pCtlPreviewNew;

    ListBox*            m_pLbColorModel;

    VclContainer*       m_pRGB;
    NumericField*       m_pR;
    NumericField*       m_pG;
    NumericField*       m_pB;

    VclContainer*       m_pCYMK;
    MetricField*        m_pC;
    MetricField*        m_pY;
    MetricField*        m_pM;
    MetricField*        m_pK;

    PushButton*         m_pBtnAdd;
    PushButton*         m_pBtnModify;
    PushButton*         m_pBtnWorkOn;
    PushButton*         m_pBtnDelete;

    const SfxItemSet&   rOutAttrs;

    XColorListRef         pColorList;

    ChangeType*         pnColorListState;
    sal_uInt16*             pPageType;
    sal_uInt16*             pDlgType;
    sal_uInt16*             pPos;
    sal_Bool*               pbAreaTP;

    XFillStyleItem      aXFStyleItem;
    XFillColorItem      aXFillColorItem;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

    ColorModel          eCM;

    Color               aAktuellColor;

    void    ConvertColorValues (Color& rColor, ColorModel eModell);
    void    RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK );
    void    CmykToRgb_Impl( Color& rColor, const sal_uInt16 nKey );
    sal_uInt16  ColorToPercent_Impl( sal_uInt16 nColor );
    sal_uInt16  PercentToColor_Impl( sal_uInt16 nPercent );

    void ImpColorCountChanged();

    //-----------------------------------------------------------------------------------------------------
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickWorkOnHdl_Impl, void * );

    DECL_LINK( SelectColorLBHdl_Impl, void * );
    DECL_LINK( SelectValSetHdl_Impl, void * );
    DECL_LINK( SelectColorModelHdl_Impl, void * );
    long ChangeColorHdl_Impl( void* p );
    DECL_LINK( ModifiedHdl_Impl, void * );

    long CheckChanges_Impl();

    void UpdateModified();
public:
    SvxColorTabPage( Window* pParent, const SfxItemSet& rInAttrs );
    ~SvxColorTabPage();

    void    Construct();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );
    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    virtual XPropertyListRef GetPropertyList( XPropertyListType t );
    virtual void             SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef );

    void    SetColorList( XColorListRef pColList );
    XColorListRef GetColorList() { return pColorList; }
    void    SaveToViewFrame( SfxViewFrame *pViewFrame );
    void    SetupForViewFrame( SfxViewFrame *pViewFrame );

    void    SetPageType( sal_uInt16* pInType ) { pPageType = pInType; }
    void    SetDlgType( sal_uInt16* pInType ) { pDlgType = pInType; }
    void    SetPos( sal_uInt16* pInPos ) { pPos = pInPos; }
    void    SetAreaTP( sal_Bool* pIn ) { pbAreaTP = pIn; }

    void    SetColorChgd( ChangeType* pIn ) { pnColorListState = pIn; }

    virtual void FillUserData();

    virtual bool IsModified()
    {
        return *pnColorListState & CT_MODIFIED;
    }
    virtual void SetModified(bool bIsModified)
    {
        if (bIsModified)
            *pnColorListState |= CT_MODIFIED;
        else
            *pnColorListState &= ~CT_MODIFIED;
    }
    virtual void AddState(ChangeType nState)
    {
        *pnColorListState |= nState;
    }
    virtual void Update(bool bLoaded);
};

#endif // _CUI_TAB_AREA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
