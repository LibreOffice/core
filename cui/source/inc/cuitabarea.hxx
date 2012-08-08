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
#ifdef _SVX_TABAREA_CXX
    DECL_LINK( CancelHdlImpl, void * );
    void                SavePalettes();
#endif

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
    FixedLine           aFlProp;
    RadioButton         aRbtTransOff;
    RadioButton         aRbtTransLinear;
    RadioButton         aRbtTransGradient;

    // linear transparency
    MetricField         aMtrTransparent;

    // gradient transparency
    FixedText           aFtTrgrType;
    ListBox             aLbTrgrGradientType;
    FixedText           aFtTrgrCenterX;
    MetricField         aMtrTrgrCenterX;
    FixedText           aFtTrgrCenterY;
    MetricField         aMtrTrgrCenterY;
    FixedText           aFtTrgrAngle;
    MetricField         aMtrTrgrAngle;
    FixedText           aFtTrgrBorder;
    MetricField         aMtrTrgrBorder;
    FixedText           aFtTrgrStartValue;
    MetricField         aMtrTrgrStartValue;
    FixedText           aFtTrgrEndValue;
    MetricField         aMtrTrgrEndValue;

    // preview
    SvxXRectPreview     aCtlBitmapPreview;
    SvxXRectPreview     aCtlXRectPreview;
    sal_Bool                bBitmap;

    XOutdevItemPool*    pXPool;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;

#ifdef _SVX_TPAREA_CXX
    DECL_LINK(ClickTransOffHdl_Impl, void * );
    DECL_LINK(ClickTransLinearHdl_Impl, void * );
    DECL_LINK(ClickTransGradientHdl_Impl, void * );
    DECL_LINK(ModifyTransparentHdl_Impl, void*);
    DECL_LINK(ChangeTrgrTypeHdl_Impl, void*);
    DECL_LINK(ModifiedTrgrHdl_Impl, void*);
#endif

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
    FixedLine           aFlProp;
    ListBox             aTypeLB;

    ColorLB             aLbColor;
    GradientLB          aLbGradient;
    HatchingLB          aLbHatching;
    BitmapLB            aLbBitmap;
    SvxXRectPreview     aCtlBitmapPreview;

    TriStateBox         aTsbStepCount;
    FixedLine           aFlStepCount;
    NumericField        aNumFldStepCount;

    CheckBox            aCbxHatchBckgrd;
    ColorLB             aLbHatchBckgrdColor;

    FixedLine           aFlSize;
    TriStateBox         aTsbOriginal;
    TriStateBox         aTsbScale;
    FixedText           aFtXSize;
    MetricField         aMtrFldXSize;
    FixedText           aFtYSize;
    MetricField         aMtrFldYSize;
    FixedLine           aFlPosition;
    SvxRectCtl          aCtlPosition;
    FixedText           aFtXOffset;
    MetricField         aMtrFldXOffset;
    FixedText           aFtYOffset;
    MetricField         aMtrFldYOffset;
    TriStateBox         aTsbTile;
    TriStateBox         aTsbStretch;
    FixedLine           aFlOffset;
    RadioButton         aRbtRow;
    RadioButton         aRbtColumn;
    MetricField         aMtrFldOffset;

    SvxXRectPreview     aCtlXRectPreview;

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

#ifdef _SVX_TPAREA_CXX
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
#endif

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
    FixedLine           aFlProp;
    TriStateBox         aTsbShowShadow;
    FixedText           aFtPosition;
    SvxRectCtl          aCtlPosition;
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    FixedText           aFtShadowColor;
    ColorLB             aLbShadowColor;
    FixedText           aFtTransparent;
    MetricField         aMtrTransparent;
    SvxXShadowPreview   aCtlXRectPreview;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

    XColorListRef         pColorList;
    ChangeType*         pnColorListState;
    sal_uInt16              nPageType;
    sal_uInt16              nDlgType;
    sal_uInt16*             pPos;
    sal_Bool*               pbAreaTP;

    sal_Bool                bDisable;

    XOutdevItemPool*    pXPool;
    XFillAttrSetItem    aXFillAttr;
    SfxItemSet&         rXFSet;
    SfxMapUnit          ePoolUnit;

#ifdef _SVX_TPSHADOW_CXX
    DECL_LINK( ClickShadowHdl_Impl, void * );
    DECL_LINK( ModifyShadowHdl_Impl, void * );
#endif

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
    FixedLine           aFlProp;
    FixedText           aFtType;
    ListBox             aLbGradientType;
    FixedText           aFtCenterX;
    MetricField         aMtrCenterX;
    FixedText           aFtCenterY;
    MetricField         aMtrCenterY;
    FixedText           aFtAngle;
    MetricField         aMtrAngle;
    FixedText           aFtBorder;
    MetricField         aMtrBorder;
    FixedText           aFtColorFrom;
    ColorLB             aLbColorFrom;
    MetricField         aMtrColorFrom;
    FixedText           aFtColorTo;
    ColorLB             aLbColorTo;
    MetricField         aMtrColorTo;
    GradientLB          aLbGradients;
    SvxXRectPreview     aCtlPreview;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;

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

#ifdef _SVX_TPGRADNT_CXX
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ChangeGradientHdl_Impl, void * );
    DECL_LINK( ModifiedHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();
    void SetControlState_Impl( XGradientStyle eXGS );
#endif

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
    FixedText           aFtDistance;
    MetricField         aMtrDistance;
    FixedText           aFtAngle;
    MetricField         aMtrAngle;
    SvxRectCtl          aCtlAngle;
    FixedLine           aFlProp;
    FixedText           aFtLineType;
    ListBox             aLbLineType;
    FixedText           aFtLineColor;
    ColorLB             aLbLineColor;
    HatchingLB          aLbHatchings;
    SvxXRectPreview     aCtlPreview;
    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnDelete;
    ImageButton         aBtnLoad;
    ImageButton         aBtnSave;

    const SfxItemSet&   rOutAttrs;
    RECT_POINT          eRP;

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

#ifdef _SVX_TPHATCH_CXX
    DECL_LINK( ChangeHatchHdl_Impl, void * );
    DECL_LINK( ModifiedHdl_Impl, void * );
    DECL_LINK( ClickAddHdl_Impl, void * );
    DECL_LINK( ClickModifyHdl_Impl, void * );
    DECL_LINK( ClickDeleteHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();
#endif

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

#ifdef _SVX_TPBITMAP_CXX
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
#endif

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

// Load save embed functionality
class SvxLoadSaveEmbed {
    XPropertyListType   meType;
    XOutdevItemPool*    mpXPool;

    Window             *mpTopDlg;
    CheckBox            maBoxEmbed;
    ImageButton         maBtnLoad;
    ImageButton         maBtnSave;
    FixedText           maTableName;

    DECL_LINK( EmbedToggleHdl_Impl, void * );
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );
public:
    SvxLoadSaveEmbed( Window *pParent, Window *pDialog,
                      const ResId &aLoad, const ResId &aSave,
                      const ResId &aEmbed, const ResId &aTableName,
                      XPropertyListType t, XOutdevItemPool* mpXPool );
    virtual ~SvxLoadSaveEmbed() {};
    XPropertyListRef GetList();
    void HideLoadSaveEmbed();
    bool GetEmbed();
    void SetEmbed( bool bEmbed );
    void UpdateTableName();
    void EnableSave( bool bCanSave );

    virtual bool IsModified() = 0;
    virtual void SetModified(bool bIsModified) = 0;
    virtual void AddState(ChangeType nState) = 0;
    virtual void Update(bool bLoaded) = 0;
};

/************************************************************************/

struct SvxColorTabPageShadow;
class SvxColorTabPage : public SfxTabPage, public SvxLoadSaveEmbed
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    SvxColorTabPageShadow *pShadow;
    FixedLine           aFlProp;
    FixedText           aFtName;
    Edit                aEdtName;
    FixedText           aFtColor;
    ColorLB             aLbColor;

    ValueSet            aValSetColorList;

    SvxXRectPreview     aCtlPreviewOld;
    SvxXRectPreview     aCtlPreviewNew;

    ListBox             aLbColorModel;
    FixedText           aFtColorModel1;
    MetricField         aMtrFldColorModel1;
    FixedText           aFtColorModel2;
    MetricField         aMtrFldColorModel2;
    FixedText           aFtColorModel3;
    MetricField         aMtrFldColorModel3;

    FixedText           aFtColorModel4;
    MetricField         aMtrFldColorModel4;

    PushButton          aBtnAdd;
    PushButton          aBtnModify;
    PushButton          aBtnWorkOn;
    PushButton          aBtnDelete;

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

#ifdef _SVX_TPCOLOR_CXX
    void    ConvertColorValues (Color& rColor, ColorModel eModell);
    void    RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK );
    void    CmykToRgb_Impl( Color& rColor, const sal_uInt16 nKey );
    sal_uInt16  ColorToPercent_Impl( sal_uInt16 nColor );
    sal_uInt16  PercentToColor_Impl( sal_uInt16 nPercent );

    void    FillValueSet_Impl( ValueSet& rVs );
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
    DECL_LINK( ClickLoadHdl_Impl, void * );
    DECL_LINK( ClickSaveHdl_Impl, void * );

    long CheckChanges_Impl();
#endif

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
