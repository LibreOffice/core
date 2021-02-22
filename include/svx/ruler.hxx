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
#ifndef INCLUDED_SVX_RULER_HXX
#define INCLUDED_SVX_RULER_HXX

#include <svtools/ruler.hxx>
#include <svl/lstner.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

#include <memory>

class Menu;
class SvxProtectItem;
class SvxRulerItem;
class SfxBindings;
class SvxLongLRSpaceItem;
class SvxLongULSpaceItem;
class SvxTabStopItem;
class SvxLRSpaceItem;
class SvxPagePosSizeItem;
class SvxColumnItem;
class SfxRectangleItem;
class SvxObjectItem;
class SfxBoolItem;
struct SvxRuler_Impl;

enum class RulerChangeType
{
    MARGIN1,
    MARGIN2
};

enum class SvxRulerDragFlags
{
    NONE                       = 0x00,
    OBJECT                     = 0x01,
    // reduce size of the last column, shift
    OBJECT_SIZE_LINEAR         = 0x02,
    OBJECT_SIZE_PROPORTIONAL   = 0x04, // proportional, Ctrl
    // only current line (table; Shift-Ctrl)
    OBJECT_ACTLINE_ONLY        = 0x08,
    // currently same key assignment
    OBJECT_LEFT_INDENT_ONLY    = OBJECT_SIZE_PROPORTIONAL,
};
namespace o3tl
{
    template<> struct typed_flags<SvxRulerDragFlags> : is_typed_flags<SvxRulerDragFlags, 0x0f> {};
}

enum class SvxRulerSupportFlags
{
    TABS                       = 0x0001,
    PARAGRAPH_MARGINS          = 0x0002,
    BORDERS                    = 0x0004,
    OBJECT                     = 0x0008,
    SET_NULLOFFSET             = 0x0010,
    NEGATIVE_MARGINS           = 0x0020,
    PARAGRAPH_MARGINS_VERTICAL = 0x0040,
    REDUCED_METRIC             = 0x0080, //shorten the context menu to select metric
};
namespace o3tl
{
    template<> struct typed_flags<SvxRulerSupportFlags> : is_typed_flags<SvxRulerSupportFlags, 0x00ff> {};
}

class SVX_DLLPUBLIC SvxRuler: public Ruler, public SfxListener
{
    friend class SvxRulerItem;

    std::vector<std::unique_ptr<SvxRulerItem> > pCtrlItems;

    std::unique_ptr<SvxLongLRSpaceItem> mxLRSpaceItem;    // left and right edge
    std::unique_ptr<SfxRectangleItem>   mxMinMaxItem;     // maxima for dragging
    std::unique_ptr<SvxLongULSpaceItem> mxULSpaceItem;    // upper and lower edge
    std::unique_ptr<SvxTabStopItem>     mxTabStopItem;    // tab stops
    std::unique_ptr<SvxLRSpaceItem>     mxParaItem;       // paragraphs
    std::unique_ptr<SvxPagePosSizeItem> mxPagePosItem;    // page distance to the rule
    std::unique_ptr<SvxColumnItem>      mxColumnItem;     // columns
    std::unique_ptr<SvxObjectItem>      mxObjectItem;     // object

    VclPtr<vcl::Window>         pEditWin;

    std::unique_ptr<SvxRuler_Impl> mxRulerImpl;

    bool            bAppSetNullOffset :1;
    bool            bHorz :1;
    tools::Long            lLogicNullOffset;     // in logic coordinates
    tools::Long            lAppNullOffset;       // in logic coordinates
    tools::Long            lInitialDragPos;
    SvxRulerSupportFlags nFlags;
    SvxRulerDragFlags    nDragType;
    sal_uInt16      nDefTabType;
    sal_uInt16      nTabCount;
    sal_uInt16      nTabBufSize;
    tools::Long            lDefTabDist;
    tools::Long            lTabPos;

    std::vector<RulerTab>    mpTabs;    // tab positions in pixel
    std::vector<RulerIndent> mpIndents; // paragraph margins in pixel
    std::vector<RulerBorder> mpBorders;
    std::vector<RulerBorder> mpObjectBorders;

    SfxBindings*    pBindings;
    tools::Long            nDragOffset;
    tools::Long            nMaxLeft;
    tools::Long            nMaxRight;
    bool            bValid;
    bool            bListening;
    bool            bActive;

    bool mbCoarseSnapping;
    bool mbSnapping;

    void StartListening_Impl();
    tools::Long GetCorrectedDragPos(bool bLeft = true, bool bRight = true );
    void DrawLine_Impl(tools::Long &lTabPos, int, bool Horizontal);
    sal_uInt16 GetObjectBordersOff(sal_uInt16 nIdx) const;

    // page borders or surrounding frame
    void UpdateFrame(const SvxLongLRSpaceItem* pItem);
    void UpdateFrame(const SvxLongULSpaceItem* pItem);
    void UpdateFrameMinMax(const SfxRectangleItem* pItem);
    // paragraph indentations
    void UpdatePara(const SvxLRSpaceItem* pItem);
    // Border distance
    void UpdateParaBorder();
    // Tabs
    void Update(const SvxTabStopItem* pItem);
    // page position and width
    void Update(const SvxPagePosSizeItem* pItem);
    // columns
    void Update(const SvxColumnItem* pItem, sal_uInt16 nSID);
    // object selection
    void Update(const SvxObjectItem* pItem);
    // protect
    void Update(const SvxProtectItem* pItem );
    // left-to-right text
    void UpdateTextRTL(const SfxBoolItem* pItem);
    // paragraph indentations
    void UpdatePara();
    void UpdateTabs();
    void UpdatePage();
    void UpdateFrame();
    void UpdateColumns();
    void UpdateObject();

    // Convert position to stick to ruler ticks
    tools::Long MakePositionSticky(tools::Long rValue, tools::Long aPointOfReference, bool aSnapToFrameMargin = true) const;

    tools::Long PixelHAdjust(tools::Long lPos, tools::Long lPos2) const;
    tools::Long PixelVAdjust(tools::Long lPos, tools::Long lPos2) const;
    tools::Long PixelAdjust(tools::Long lPos, tools::Long lPos2) const;

    tools::Long ConvertHPosPixel(tools::Long lPos) const;
    tools::Long ConvertVPosPixel(tools::Long lPos) const;
    tools::Long ConvertHSizePixel(tools::Long lSize) const;
    tools::Long ConvertVSizePixel(tools::Long lSize) const;

    tools::Long ConvertPosPixel(tools::Long lPos) const;
    tools::Long ConvertSizePixel(tools::Long lSize) const;

    tools::Long ConvertHPosLogic(tools::Long lPos) const;
    tools::Long ConvertVPosLogic(tools::Long lPos) const;
    tools::Long ConvertHSizeLogic(tools::Long lSize) const;
    tools::Long ConvertVSizeLogic(tools::Long lSize) const;

    tools::Long ConvertPosLogic(tools::Long lPos) const;
    tools::Long ConvertSizeLogic(tools::Long lSize) const;

    tools::Long RoundToCurrentMapMode(tools::Long lValue) const;

    tools::Long GetFirstLineIndent() const;
    tools::Long GetLeftIndent() const;
    tools::Long GetRightIndent() const;
    tools::Long GetLogicRightIndent() const;

    tools::Long GetLeftFrameMargin() const;
    tools::Long GetRightFrameMargin() const;

    void CalcMinMax();

    void EvalModifier();
    void DragMargin1();
    //adjust the left margin either after DragMargin1() or after DragBorders()
    void AdjustMargin1(tools::Long lDiff);
    void DragMargin2();
    void DragIndents();
    void DragTabs();
    void DragBorders();
    void DragObjectBorder();

    void ApplyMargins();
    void ApplyIndents();
    void ApplyTabs();
    void ApplyBorders();
    void ApplyObject();

    tools::Long GetFrameLeft() const;

    tools::Long GetLeftMin() const;
    tools::Long GetRightMax() const;

    void TabMenuSelect(const OString& rIdent);
    void MenuSelect(std::string_view ident);
    void PrepareProportional_Impl(RulerType);

    enum class UpdateType
    {
        MoveLeft,
        MoveRight
    };
    void UpdateParaContents_Impl(tools::Long lDiff, UpdateType);

protected:
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    Click() override;
    virtual bool    StartDrag() override;
    virtual void    Drag() override;
    virtual void    EndDrag() override;
    virtual void    ExtraDown() override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void    Update();

    bool IsActLastColumn(
                bool bForceDontConsiderHidden = false,
                sal_uInt16 nAct=USHRT_MAX) const;
    bool IsActFirstColumn(
                bool bForceDontConsiderHidden = false,
                sal_uInt16 nAct=USHRT_MAX) const;
    sal_uInt16 GetActLeftColumn(
                bool bForceDontConsiderHidden = false,
                sal_uInt16 nAct=USHRT_MAX ) const;
    sal_uInt16 GetActRightColumn (
                bool bForceDontConsiderHidden = false,
                sal_uInt16 nAct=USHRT_MAX ) const;
    tools::Long CalcPropMaxRight(sal_uInt16 nCol = USHRT_MAX) const;

public:

    SvxRuler(vcl::Window* pParent, vcl::Window *pEditWin, SvxRulerSupportFlags nRulerFlags,
             SfxBindings &rBindings, WinBits nWinStyle);
    virtual ~SvxRuler() override;
    virtual void dispose() override;

    void SetDefTabDist(tools::Long);

    // set/get NullOffset in logic units
    void SetNullOffsetLogic(tools::Long lOff);

    void SetActive(bool bOn = true);

    void ForceUpdate()
    {
        Update();
    }

    //#i24363# tab stops relative to indent
    void SetTabsRelativeToIndent( bool bRel );
    void SetValues(RulerChangeType type, tools::Long value);
    tools::Long GetPageWidth() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
