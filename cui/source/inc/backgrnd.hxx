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
#ifndef _SVX_BACKGRND_HXX
#define _SVX_BACKGRND_HXX

#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/graph.hxx>
#include <svtools/valueset.hxx>
#include <svx/dlgctrl.hxx>

//------------------------------------------------------------------------
// forwards:

class BackgroundPreviewImpl;
class SvxOpenGraphicDialog;
struct SvxBackgroundTable_Impl;
struct SvxBackgroundPara_Impl;
struct SvxBackgroundPage_Impl;
class SvxBrushItem;
/** class SvxBackgroundTabPage --------------------------------------------
{k:\svx\prototyp\dialog\backgrnd.bmp}
    [Description]
    With this TabPage a Brush (e. g. for a frame's background color)
    can be set.
    [Items]
    <SvxBrushItem>:     <SID_ATTR_BRUSH>;
*/

class SvxBackgroundTabPage : public SvxTabPage
{
    using TabPage::DeactivatePage;
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        FillUserData();
    virtual void        PointChanged( Window* pWindow, RECT_POINT eRP );

    /// Shift-ListBox activation
    void                ShowSelector();
    /// for the Writer (cells/rows/tables)
    void                ShowTblControl();
    /// for the Writer (paragraph/characters)
    void                ShowParaControl(sal_Bool bCharOnly = sal_False);

    void                EnableTransparency(sal_Bool bColor, sal_Bool bGraphic);
    virtual void        PageCreated (SfxAllItemSet aSet);
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
    SvxBackgroundTabPage( Window* pParent, const SfxItemSet& rCoreSet );
    ~SvxBackgroundTabPage();

    FixedText               aSelectTxt;
    ListBox                 aLbSelect;
    const String            aStrBrowse;
    const String            aStrUnlinked;
     FixedText               aTblDesc;
     ListBox                 aTblLBox;
     ListBox                 aParaLBox;
    Control                 aBorderWin;
    ValueSet                aBackgroundColorSet;
    FixedLine               aBackgroundColorBox;
    BackgroundPreviewImpl*  pPreviewWin1;

    FixedText               aColTransFT;///<color transparency
    MetricField             aColTransMF;
    CheckBox                aBtnPreview;
    // Background Bitmap ----------------------------------
     FixedLine               aGbFile;
    PushButton              aBtnBrowse;
    CheckBox                aBtnLink;
      FixedLine               aGbPosition;
    RadioButton             aBtnPosition;
    RadioButton             aBtnArea;
    RadioButton             aBtnTile;
    SvxRectCtl              aWndPosition;
    FixedInfo               aFtFile;

    FixedLine               aGraphTransFL;///<transparency of graphics
    MetricField             aGraphTransMF;

    BackgroundPreviewImpl*  pPreviewWin2;

    // DDListBox for Writer -------------------------------
    //------------------------------------------------------
    Color       aBgdColor;
    sal_uInt16      nHtmlMode;
    sal_Bool        bAllowShowSelector  : 1;
    sal_Bool        bIsGraphicValid     : 1;
    sal_Bool        bLinkOnly           : 1;
    sal_Bool        bResized            : 1;
    sal_Bool        bColTransparency    : 1;
    sal_Bool        bGraphTransparency  : 1;
    Graphic     aBgdGraphic;
    String      aBgdGraphicPath;
    String      aBgdGraphicFilter;

    SvxBackgroundPage_Impl* pPageImpl;
    SvxOpenGraphicDialog* pImportDlg;

    SvxBackgroundTable_Impl*    pTableBck_Impl;///< Items for Sw-Table must be corrected
    SvxBackgroundPara_Impl* pParaBck_Impl;///< also for the paragraph style

#ifdef _SVX_BACKGRND_CXX
    void                FillColorValueSets_Impl();
    void                ShowColorUI_Impl();
    void                ShowBitmapUI_Impl();
    sal_Bool                LoadLinkedGraphic_Impl();
    void                RaiseLoadError_Impl();
    void                SetGraphicPosition_Impl( SvxGraphicPosition ePos );
    SvxGraphicPosition  GetGraphicPosition_Impl();
    void                FillControls_Impl(const SvxBrushItem& rBgdAttr,
                                            const String& rUserData);
    sal_Bool                FillItemSetWithWallpaperItem( SfxItemSet& rCoreSet, sal_uInt16 nSlot);
    void                ResetFromWallpaperItem( const SfxItemSet& rSet );

    DECL_LINK( LoadTimerHdl_Impl, Timer* );
    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(BrowseHdl_Impl, void *);
    DECL_LINK( RadioClickHdl_Impl, RadioButton* );
    DECL_LINK( FileClickHdl_Impl, CheckBox* );
    DECL_LINK(BackgroundColorHdl_Impl, void *);
    DECL_LINK( TblDestinationHdl_Impl, ListBox* );
    DECL_LINK( ParaDestinationHdl_Impl, ListBox* );
#endif
};

#endif // #ifndef _SVX_BACKGRND_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
