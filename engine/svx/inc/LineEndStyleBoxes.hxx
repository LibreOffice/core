/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#pragma once

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <functional>
#include <optional>
#include <rtl/ref.hxx>
#include <svl/poolitem.hxx>
#include <vcl/weld.hxx>

class XLineEndEntry;
class XLineEndItem;
class XLineEndList;
class XLineStartItem;
typedef rtl::Reference<XLineEndList> XLineEndListRef;

namespace svx
{
/** The "Start" / "End" arrow style pickers, shared by the Line section of the
    sidebar and the one of the notebookbar. */
class LineEndStyleBoxes final
{
public:
    LineEndStyleBoxes(std::unique_ptr<weld::Label> xStartLabel,
                      std::unique_ptr<weld::ComboBox> xStartBox,
                      std::unique_ptr<weld::Label> xEndLabel,
                      std::unique_ptr<weld::ComboBox> xEndBox,
                      std::function<void(const XLineStartItem&)> aSetLineStart,
                      std::function<void(const XLineEndItem&)> aSetLineEnd,
                      css::uno::Reference<css::frame::XFrame> xFrame);
    ~LineEndStyleBoxes();

    /// Takes the state of SID_ATTR_LINE_START, SID_ATTR_LINE_END and
    /// SID_LINEEND_LIST, and says whether the slot was one of them.
    bool NotifyItemUpdate(sal_uInt16 nSId, SfxItemState eState, const SfxPoolItem* pState);

    void set_sensitive(bool bSensitive);
    void set_visible(bool bVisible);

private:
    struct End
    {
        std::unique_ptr<weld::Label> mxLabel;
        std::unique_ptr<weld::ComboBox> mxBox;
        std::optional<basegfx::B2DPolyPolygon> moPolygon;
    };

    End maStart;
    End maEnd;
    std::function<void(const XLineStartItem&)> maSetLineStart;
    std::function<void(const XLineEndItem&)> maSetLineEnd;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    // the current list of available line ends
    XLineEndListRef mxLineEndList;

    void updateLineStart(bool bSetOrDefault, const SfxPoolItem* pState);
    void updateLineEnd(bool bSetOrDefault, const SfxPoolItem* pState);
    void updateLineEndList();
    void FillBox(End& rEnd, tools::Long nExtraStyle = -1);
    void SelectEntry(End& rEnd);
    const XLineEndEntry* GetSelected(const End& rEnd) const;
    template <class ItemType>
    void UpdateEnd(End& rEnd, bool bSetOrDefault, const SfxPoolItem* pState);
    template <class ItemType> std::optional<ItemType> Pick(End& rEnd);
    void RenderEntry(const weld::ComboBox::render_args& rArgs, bool bStart);

    DECL_LINK(ChangeStartHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeEndHdl, weld::ComboBox&, void);
    DECL_LINK(RenderStartHdl, weld::ComboBox::render_args, void);
    DECL_LINK(RenderEndHdl, weld::ComboBox::render_args, void);
    DECL_LINK(GetSizeHdl, vcl::RenderContext&, Size);
};

} // end of namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
