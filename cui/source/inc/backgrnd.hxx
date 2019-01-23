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
#ifndef INCLUDED_CUI_SOURCE_INC_BACKGRND_HXX
#define INCLUDED_CUI_SOURCE_INC_BACKGRND_HXX

#include <vcl/graph.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/dlgctrl.hxx>
#include <editeng/brushitem.hxx>
#include <memory>

#include "cuitabarea.hxx"

class BackgroundPreviewImpl;
class SvxOpenGraphicDialog;
class SvxBrushItem;

/** class SvxBackgroundTabPage --------------------------------------------

    [Description]
    With this TabPage a Brush (e. g. for a frame's background color)
    can be set.
    [Items]
    <SvxBrushItem>:     <SID_ATTR_BRUSH>;
*/

class SvxBackgroundTabPage : public SvxTabPage
{
    using TabPage::DeactivatePage;
    friend class VclPtr<SvxBackgroundTabPage>;
    static const sal_uInt16 pPageRanges[];
public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet );
    // returns the area of the which-values
    static const sal_uInt16* GetRanges() { return pPageRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        FillUserData() override;
    virtual void        PointChanged( weld::DrawingArea* pWindow, RectPoint eRP ) override;

    /// Shift-ListBox activation
    void                ShowSelector();
    /// for the Writer (cells/rows/tables)
    void                ShowTblControl();

    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
protected:
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

private:
    SvxBackgroundTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet);
    virtual ~SvxBackgroundTabPage() override;
    virtual void dispose() override;

    // DDListBox for Writer -------------------------------

    Color       aBgdColor;
    sal_uInt16      nHtmlMode;
    bool        bAllowShowSelector  : 1;
    bool        bIsGraphicValid     : 1;
    bool        bHighlighting       : 1;
    bool        bCharBackColor      : 1;
    bool        m_bColorSelected    : 1;
    Graphic     aBgdGraphic;
    OUString    aBgdGraphicPath;
    OUString    aBgdGraphicFilter;

    std::unique_ptr<Idle> m_pLoadIdle;
    bool        m_bIsImportDlgInExecute = false;

    std::unique_ptr<SvxOpenGraphicDialog> pImportDlg;

    ///< Items for Sw-Table must be corrected
    std::unique_ptr<SvxBrushItem>   m_pCellBrush;
    std::unique_ptr<SvxBrushItem>   m_pRowBrush;
    std::unique_ptr<SvxBrushItem>   m_pTableBrush;
    sal_uInt16      m_nCellWhich = 0;
    sal_uInt16      m_nRowWhich = 0;
    sal_uInt16      m_nTableWhich = 0;
    sal_Int32       m_nActPos = 0;

    std::unique_ptr<SvxBrushItem> pHighlighting;

    std::unique_ptr<SvxRectCtl> m_xWndPosition;
    std::unique_ptr<ColorValueSet> m_xBackgroundColorSet;
    std::unique_ptr<BackgroundPreviewImpl> m_xPreview1;
    std::unique_ptr<BackgroundPreviewImpl> m_xPreview2;

    std::unique_ptr<weld::Label> m_xFindGraphicsFt;
    std::unique_ptr<weld::Widget> m_xAsGrid;
    std::unique_ptr<weld::Label> m_xSelectTxt;
    std::unique_ptr<weld::ComboBox> m_xLbSelect;
    std::unique_ptr<weld::Label> m_xTblDesc;
    std::unique_ptr<weld::ComboBox> m_xTblLBox;

    std::unique_ptr<weld::Label> m_xBackGroundColorLabelFT;
    std::unique_ptr<weld::Widget> m_xBackGroundColorFrame;

    std::unique_ptr<weld::CheckButton> m_xBtnPreview;

    // Background Bitmap ----------------------------------
    std::unique_ptr<weld::Widget> m_xBitmapContainer;
    std::unique_ptr<weld::Widget> m_xFileFrame;
    std::unique_ptr<weld::Button> m_xBtnBrowse;
    std::unique_ptr<weld::CheckButton> m_xBtnLink;
    std::unique_ptr<weld::Label> m_xFtUnlinked;
    std::unique_ptr<weld::Label> m_xFtFile;

    std::unique_ptr<weld::Widget> m_xTypeFrame;
    std::unique_ptr<weld::RadioButton> m_xBtnPosition;
    std::unique_ptr<weld::RadioButton> m_xBtnArea;
    std::unique_ptr<weld::RadioButton> m_xBtnTile;

    std::unique_ptr<weld::CustomWeld> m_xWndPositionWin;
    std::unique_ptr<weld::CustomWeld> m_xBackgroundColorSetWin;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin1;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin2;

    void                FillColorValueSets_Impl();
    void                ShowColorUI_Impl();
    void                HideColorUI_Impl();
    void                ShowBitmapUI_Impl();
    void                HideBitmapUI_Impl();
    bool                LoadLinkedGraphic_Impl();
    void                RaiseLoadError_Impl();
    void                SetGraphicPosition_Impl( SvxGraphicPosition ePos );
    SvxGraphicPosition  GetGraphicPosition_Impl();
    void                FillControls_Impl(const SvxBrushItem& rBgdAttr,
                                          const OUString& rUserData);

    DECL_LINK(LoadIdleHdl_Impl, Timer*, void);
    DECL_LINK(SelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(BrowseHdl_Impl, weld::Button&, void);
    DECL_LINK(RadioClickHdl_Impl, weld::ToggleButton&, void );
    DECL_LINK(FileClickHdl_Impl, weld::ToggleButton&, void);
    DECL_LINK(BackgroundColorHdl_Impl, SvtValueSet*, void);
    DECL_LINK(TblDestinationHdl_Impl, weld::ComboBox&, void);
};

class SvxBkgTabPage : public SvxAreaTabPage
{
    std::unique_ptr<weld::ComboBox> m_xTblLBox;
    bool        bHighlighting       : 1;
    bool        bCharBackColor      : 1;
    SfxItemSet maSet;
    std::unique_ptr<SfxItemSet> m_pResetSet;

    sal_Int32 m_nActPos = -1;

    DECL_LINK(TblDestinationHdl_Impl, weld::ComboBox&, void);
public:
    using SvxAreaTabPage::ActivatePage;
    using SvxAreaTabPage::DeactivatePage;

    SvxBkgTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs);
    virtual ~SvxBkgTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent, const SfxItemSet* );
    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    virtual void PageCreated( const SfxAllItemSet& aSet ) override;
    virtual void Reset( const SfxItemSet * ) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_BACKGRND_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
