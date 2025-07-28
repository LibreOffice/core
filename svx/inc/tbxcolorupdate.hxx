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

#pragma once

#include <sfx2/namedcolor.hxx>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/toolboxid.hxx>
#include <svl/lstner.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrame.hpp>

class VirtualDevice;

namespace weld
{
class Toolbar;
}

namespace svx
{
//= ToolboxButtonColorUpdater

/** helper class to update a color in a toolbox button image

    formerly known as SvxTbxButtonColorUpdater_Impl, residing in svx/source/tbxctrls/colorwindow.hxx.
*/
class ToolboxButtonColorUpdaterBase : public SfxListener
{
public:
    ToolboxButtonColorUpdaterBase(bool bWideButton, OUString aCommandLabel, OUString aCommandURL,
                                  sal_uInt16 nSlotId,
                                  css::uno::Reference<css::frame::XFrame> xFrame);

    virtual ~ToolboxButtonColorUpdaterBase();

    void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    void SetRecentColor(const NamedColor& rNamedColor, bool bBroadcast = true);
    void Update(const NamedColor& rNamedColor);
    void Update(const Color& rColor, bool bForceUpdate = false);
    Color const& GetCurrentColor() const { return maCurColor; }
    OUString GetCurrentColorName() const;

private:
    ToolboxButtonColorUpdaterBase(ToolboxButtonColorUpdaterBase const&) = delete;
    ToolboxButtonColorUpdaterBase& operator=(ToolboxButtonColorUpdaterBase const&) = delete;

protected:
    bool mbWideButton;
    bool mbWasHiContrastMode;
    sal_uInt16 mnSlotId;
    Color maCurColor;
    vcl::ImageType meImageType;
    OUString maCommandLabel;
    OUString maCommandURL;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    void Init(sal_uInt16 nSlotId);

    virtual void SetQuickHelpText(const OUString& rText) = 0;
    virtual OUString GetQuickHelpText() const = 0;
    virtual void SetImage(VirtualDevice* pVirDev) = 0;
    virtual VclPtr<VirtualDevice> CreateVirtualDevice() const = 0;
    // true -> use Device to Record to Metafile, false -> Render to Device
    virtual bool RecordVirtualDevice() const = 0;
    virtual vcl::ImageType GetImageSize() const = 0;
    virtual Size GetItemSize(const Size& rImageSize) const = 0;
};

class VclToolboxButtonColorUpdater final : public ToolboxButtonColorUpdaterBase
{
public:
    VclToolboxButtonColorUpdater(sal_uInt16 nSlotId, ToolBoxItemId nTbxBtnId, ToolBox* ptrTbx,
                                 bool bWideButton, const OUString& rCommandLabel,
                                 const OUString& rCommandURL,
                                 const css::uno::Reference<css::frame::XFrame>& rFrame);

private:
    ToolBoxItemId mnBtnId;
    VclPtr<ToolBox> mpTbx;

    virtual void SetQuickHelpText(const OUString& rText) override;
    virtual OUString GetQuickHelpText() const override;
    virtual void SetImage(VirtualDevice* pVirDev) override;
    virtual VclPtr<VirtualDevice> CreateVirtualDevice() const override;
    virtual bool RecordVirtualDevice() const override { return true; }
    virtual vcl::ImageType GetImageSize() const override;
    virtual Size GetItemSize(const Size& rImageSize) const override;
};

class ToolboxButtonColorUpdater final : public ToolboxButtonColorUpdaterBase
{
public:
    ToolboxButtonColorUpdater(sal_uInt16 nSlotId, const OUString& rTbxBtnId, weld::Toolbar* ptrTbx,
                              bool bWideButton, const OUString& rCommandLabel,
                              const css::uno::Reference<css::frame::XFrame>& rFrame);

private:
    OUString msBtnId;
    weld::Toolbar* mpTbx;

    virtual void SetQuickHelpText(const OUString& rText) override;
    virtual OUString GetQuickHelpText() const override;
    virtual void SetImage(VirtualDevice* pVirDev) override;
    virtual VclPtr<VirtualDevice> CreateVirtualDevice() const override;
    virtual bool RecordVirtualDevice() const override { return false; }
    virtual vcl::ImageType GetImageSize() const override;
    virtual Size GetItemSize(const Size& rImageSize) const override;
};

class ToolboxButtonLineStyleUpdater
{
private:
    css::drawing::LineStyle m_eXLS;
    int m_nDashStyleIndex;

public:
    ToolboxButtonLineStyleUpdater();
    void Update(const css::frame::FeatureStateEvent& rEvent);
    int GetStyleIndex() const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
