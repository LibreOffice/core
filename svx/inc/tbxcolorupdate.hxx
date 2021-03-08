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

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/toolbox.hxx>
#include <svx/Palette.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/XFrame.hpp>

class ToolBox;
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
    class ToolboxButtonColorUpdaterBase
    {
    public:
        ToolboxButtonColorUpdaterBase(bool bWideButton, const OUString& rCommandLabel,
                                      const OUString& rCommandURL,
                                      const css::uno::Reference<css::frame::XFrame>& rFrame);

        virtual ~ToolboxButtonColorUpdaterBase();

        void        Update( const NamedColor& rNamedColor );
        void        Update( const Color& rColor, bool bForceUpdate = false );
        Color const & GetCurrentColor() const { return maCurColor; }
        OUString    GetCurrentColorName() const;

    private:
        ToolboxButtonColorUpdaterBase(ToolboxButtonColorUpdaterBase const &) = delete;
        ToolboxButtonColorUpdaterBase& operator =(ToolboxButtonColorUpdaterBase const &) = delete;

    protected:
        bool        mbWideButton;
        bool        mbWasHiContrastMode;
        Color       maCurColor;
        tools::Rectangle   maUpdRect;
        Size        maBmpSize;
        vcl::ImageType meImageType;
        OUString    maCommandLabel;
        OUString    maCommandURL;
        css::uno::Reference<css::frame::XFrame> mxFrame;

        void Init(sal_uInt16 nSlotId);

        virtual void SetQuickHelpText(const OUString& rText) = 0;
        virtual OUString GetQuickHelpText() const = 0;
        virtual void SetImage(VirtualDevice* pVirDev) = 0;
        virtual VclPtr<VirtualDevice> CreateVirtualDevice() const = 0;
        virtual vcl::ImageType GetImageSize() const = 0;
        virtual Size GetItemSize(const Size& rImageSize) const = 0;
    };

    class VclToolboxButtonColorUpdater : public ToolboxButtonColorUpdaterBase
    {
    public:
        VclToolboxButtonColorUpdater(sal_uInt16 nSlotId, ToolBoxItemId nTbxBtnId, ToolBox* ptrTbx, bool bWideButton,
                                     const OUString& rCommandLabel, const OUString& rCommandURL,
                                     const css::uno::Reference<css::frame::XFrame>& rFrame);


    private:
        ToolBoxItemId   mnBtnId;
        VclPtr<ToolBox> mpTbx;

        virtual void SetQuickHelpText(const OUString& rText) override;
        virtual OUString GetQuickHelpText() const override;
        virtual void SetImage(VirtualDevice* pVirDev) override;
        virtual VclPtr<VirtualDevice> CreateVirtualDevice() const override;
        virtual vcl::ImageType GetImageSize() const override;
        virtual Size GetItemSize(const Size& rImageSize) const override;
    };

    class ToolboxButtonColorUpdater : public ToolboxButtonColorUpdaterBase
    {
    public:
        ToolboxButtonColorUpdater(sal_uInt16 nSlotId, const OString& rTbxBtnId, weld::Toolbar* ptrTbx, bool bWideButton,
                                  const OUString& rCommandLabel, const css::uno::Reference<css::frame::XFrame>& rFrame);

    private:
        OString msBtnId;
        weld::Toolbar* mpTbx;

        virtual void SetQuickHelpText(const OUString& rText) override;
        virtual OUString GetQuickHelpText() const override;
        virtual void SetImage(VirtualDevice* pVirDev) override;
        virtual VclPtr<VirtualDevice> CreateVirtualDevice() const override;
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
