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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEPROPERTYPANELBASE_HXX

#include <vcl/weld.hxx>
#include <memory>
#include <svl/poolitem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/sidebar/LineWidthPopup.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/frame/XFrame.hpp>

class ToolbarUnoDispatcher;
class XLineStyleItem;
class XLineDashItem;
class XLineStartItem;
class XLineWidthItem;
class XLineEndItem;
class XLineEndList;
class XLineJointItem;
class XLineCapItem;
class XLineTransparenceItem;
class XDashList;

namespace svx::sidebar
{
class LineStyleNoneChange;

class SVX_DLLPUBLIC LinePropertyPanelBase : public PanelLayout
{
public:
    virtual ~LinePropertyPanelBase() override;

    void SetWidth(tools::Long nWidth);
    void SetWidthIcon(int n);
    void SetWidthIcon();

    void EndLineWidthPopup();

    // constructor/destructor
    LinePropertyPanelBase(weld::Widget* pParent,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void setLineWidth(const XLineWidthItem& rItem) = 0;

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
    virtual void setLineJoint(const XLineJointItem* pItem) = 0;
    virtual void setLineCap(const XLineCapItem* pItem) = 0;

    void updateLineTransparence(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    virtual void updateLineWidth(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineJoint(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);
    void updateLineCap(bool bDisabled, bool bSetOrDefault, const SfxPoolItem* pItem);

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
    //ui controls
    std::unique_ptr<weld::Label> mxFTWidth;
    std::unique_ptr<weld::Toolbar> mxTBWidth;
    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::MetricSpinButton> mxMFTransparent;
    std::unique_ptr<weld::Label> mxFTEdgeStyle;
    std::unique_ptr<weld::ComboBox> mxLBEdgeStyle;
    std::unique_ptr<weld::Label> mxFTCapStyle;
    std::unique_ptr<weld::ComboBox> mxLBCapStyle;
    std::unique_ptr<weld::Widget> mxGridLineProps;
    std::unique_ptr<weld::Widget> mxBoxArrowProps;
    //popup windows
    std::unique_ptr<LineWidthPopup> mxLineWidthPopup;

    std::unique_ptr<LineStyleNoneChange> mxLineStyleNoneChange;

    sal_uInt16 mnTrans;
    MapUnit meMapUnit;

    // images from resource
    OUString maIMGNone;

    // multi-images
    OUString maIMGWidthIcon[8];

    bool mbWidthValuable : 1;
    bool mbArrowSupported;
    bool mbNoneLineStyle;

    void Initialize();

    DECL_LINK(ToolboxWidthSelectHdl, const OString&, void);
    DECL_LINK(ChangeTransparentHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ChangeEdgeStyleHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeCapStyleHdl, weld::ComboBox&, void);
};

} // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
