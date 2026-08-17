/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX

#include <vcl/weld.hxx>
#include <memory>
#include <optional>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <rtl/ref.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/frame/XFrame.hpp>

class ToolbarUnoDispatcher;
class XLineStartItem;
class XLineEndItem;
class XLineEndEntry;
class XLineEndList;
typedef rtl::Reference<XLineEndList> XLineEndListRef;
class XLineWidthItem;
class XLineTransparenceItem;

namespace svx::sidebar
{
class LineStyleNoneChange;

class UNLESS_MERGELIBS(SVX_DLLPUBLIC) LinePropertyPanelBase : public PanelLayout
{
public:
    virtual ~LinePropertyPanelBase() override;

    void SetWidth(tools::Long nWidth);

    void EndLineWidthPopup();

    // constructor/destructor
    LinePropertyPanelBase(weld::Widget* pParent,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void setLineWidth(const XLineWidthItem& rItem) = 0;
    virtual void setLineStart(const XLineStartItem& rItem) = 0;
    virtual void setLineEnd(const XLineEndItem& rItem) = 0;

    void SetNoneLineStyle(bool bNoneLineStyle)
    {
        if (bNoneLineStyle != mbNoneLineStyle)
        {
            mbNoneLineStyle = bNoneLineStyle;
            ActivateControls();
        }
    }

protected:
    void ActivateControls();

    virtual void setLineTransparency(const XLineTransparenceItem& rItem) = 0;

    void updateLineTransparence(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    virtual void updateLineWidth(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineStart(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineEnd(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineEndList(const SfxPoolItem* pItem);

    void setMapUnit(MapUnit eMapUnit);

    void enableArrowHead();
    void disableArrowHead();

protected:
    std::unique_ptr<weld::Toolbar> mxTBColor;
    std::unique_ptr<ToolbarUnoDispatcher> mxColorDispatch;

    std::unique_ptr<weld::Toolbar> mxLineStyleTB;
    std::unique_ptr<ToolbarUnoDispatcher> mxLineStyleDispatch;

    sal_Int32 mnWidthCoreValue;

private:
    css::uno::Reference<css::frame::XFrame> mxFrame;

    //ui controls
    std::unique_ptr<weld::Label> mxFTWidth;
    std::unique_ptr<weld::Toolbar> mxTBWidth;
    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::MetricSpinButton> mxMFTransparent;
    std::unique_ptr<weld::Label> mxArrowStartFT;
    std::unique_ptr<weld::Label> mxArrowEndFT;
    std::unique_ptr<weld::ComboBox> mxLBStart;
    std::unique_ptr<weld::ComboBox> mxLBEnd;
    //popup windows
    std::unique_ptr<LineWidthPopup> mxLineWidthPopup;

    std::unique_ptr<LineStyleNoneChange> mxLineStyleNoneChange;

    // the current list of available line ends and the currently
    // selected start/end arrows
    XLineEndListRef mxLineEndList;
    std::optional<basegfx::B2DPolyPolygon> moStartPolygon;
    std::optional<basegfx::B2DPolyPolygon> moEndPolygon;

    sal_uInt16 mnTrans;
    MapUnit meMapUnit;

    bool mbWidthValuable : 1;
    bool mbArrowSupported;
    bool mbNoneLineStyle;

    void Initialize();

    void FillLineEndListBox(weld::ComboBox& rListBox, tools::Long nExtraStyle = -1);
    void SelectLineEndEntry(weld::ComboBox& rListBox,
                            const std::optional<basegfx::B2DPolyPolygon>& roPolygon);
    const XLineEndEntry* GetSelectedLineEnd(const weld::ComboBox& rListBox) const;
    template <class ItemType>
    std::optional<ItemType> PickLineEnd(weld::ComboBox& rListBox,
                                        std::optional<basegfx::B2DPolyPolygon>& roPolygon);
    // draw one dropdown entry for the given id
    void RenderLineEndEntry(const weld::ComboBox::render_args& rArgs, bool bStart);

    DECL_DLLPRIVATE_LINK(ToolboxWidthSelectHdl, const OUString&, void);
    DECL_DLLPRIVATE_LINK(ToolboxWidthToggleMenuHdl, const OUString&, void);
    DECL_DLLPRIVATE_LINK(ChangeTransparentHdl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ChangeStartHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(ChangeEndHdl, weld::ComboBox&, void);
    DECL_DLLPRIVATE_LINK(RenderStartHdl, weld::ComboBox::render_args, void);
    DECL_DLLPRIVATE_LINK(RenderEndHdl, weld::ComboBox::render_args, void);
    DECL_DLLPRIVATE_LINK(GetSizeHdl, vcl::RenderContext&, Size);
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
