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

#include <sfx2/objsh.hxx>
#include <svx/drawitem.hxx>
#include <tbxcolorupdate.hxx>
#include <svx/svxids.hrc>
#include <svx/xdef.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

namespace svx
{
    ToolboxButtonColorUpdaterBase::ToolboxButtonColorUpdaterBase(bool bWideButton, const OUString& rCommandLabel,
                                                                 const OUString& rCommandURL,
                                                                 const css::uno::Reference<css::frame::XFrame>& rFrame)
        : mbWideButton(bWideButton)
        , mbWasHiContrastMode(Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        , maCurColor(COL_TRANSPARENT)
        , meImageType(vcl::ImageType::Size16)
        , maCommandLabel(rCommandLabel)
        , maCommandURL(rCommandURL)
        , mxFrame(rFrame)
    {
    }

    void ToolboxButtonColorUpdaterBase::Init(sal_uInt16 nSlotId)
    {
        switch (nSlotId)
        {
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_COLOR2:
                Update(NamedColor(COL_DEFAULT_FONT, SvxResId(RID_SVXSTR_COLOR_DEFAULT_FONT)));
                break;
            case SID_FRAME_LINECOLOR:
                Update(NamedColor(COL_DEFAULT_FRAMELINE, SvxResId(RID_SVXSTR_COLOR_DEFAULT_FRAMELINE)));
                break;
            case SID_ATTR_CHAR_COLOR_BACKGROUND:
            case SID_ATTR_CHAR_BACK_COLOR:
            case SID_BACKGROUND_COLOR:
            case SID_TABLE_CELL_BACKGROUND_COLOR:
                Update(NamedColor(COL_DEFAULT_HIGHLIGHT, SvxResId(RID_SVXSTR_COLOR_DEFAULT_HIGHLIGHT)));
                break;
            case SID_ATTR_LINE_COLOR:
                Update(NamedColor(COL_DEFAULT_SHAPE_STROKE, SvxResId(RID_SVXSTR_COLOR_DEFAULT_SHAPE_STROKE)));
                break;
            case SID_ATTR_FILL_COLOR:
                Update(NamedColor(COL_DEFAULT_SHAPE_FILLING, SvxResId(RID_SVXSTR_COLOR_DEFAULT_SHAPE_FILLING)));
                break;
            default:
                Update(COL_TRANSPARENT);
        }
    }

    VclToolboxButtonColorUpdater::VclToolboxButtonColorUpdater(
            sal_uInt16 nSlotId, sal_uInt16 nTbxBtnId, ToolBox* pToolBox, bool bWideButton,
            const OUString& rCommandLabel, const OUString& rCommandURL,
            const css::uno::Reference<css::frame::XFrame>& rFrame)
        : ToolboxButtonColorUpdaterBase(bWideButton, rCommandLabel, rCommandURL, rFrame)
        , mnBtnId(nTbxBtnId)
        , mpTbx(pToolBox)
    {
        Init(nSlotId);
    }

    void VclToolboxButtonColorUpdater::SetQuickHelpText(const OUString& rText)
    {
        mpTbx->SetQuickHelpText(mnBtnId, rText);
    }

    OUString VclToolboxButtonColorUpdater::GetQuickHelpText() const
    {
        return mpTbx->GetQuickHelpText(mnBtnId);
    }

    void VclToolboxButtonColorUpdater::SetImage(VirtualDevice* pVirDev)
    {
        mpTbx->SetItemImage(mnBtnId, Image(pVirDev->GetBitmapEx(Point(0,0), maBmpSize)));
    }

    VclPtr<VirtualDevice> VclToolboxButtonColorUpdater::CreateVirtualDevice() const
    {
        auto xRet = VclPtr<VirtualDevice>::Create(*mpTbx,
            DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        xRet->SetBackground(mpTbx->GetControlBackground());
        return xRet;
    }

    vcl::ImageType VclToolboxButtonColorUpdater::GetImageSize() const
    {
        return mpTbx->GetImageSize();
    }

    Size VclToolboxButtonColorUpdater::GetItemSize(const Size& rImageSize) const
    {
        if (mbWideButton)
            return mpTbx->GetItemContentSize(mnBtnId);
        return rImageSize;
    }

    ToolboxButtonColorUpdaterBase::~ToolboxButtonColorUpdaterBase()
    {}

    void ToolboxButtonColorUpdaterBase::Update(const NamedColor &rNamedColor)
    {
        Update(rNamedColor.first);
        if (!mbWideButton)
        {
            // Also show the current color as QuickHelpText
            OUString colorSuffix = OUString(" (%1)").replaceFirst("%1", rNamedColor.second);
            OUString colorHelpText = maCommandLabel + colorSuffix;

            SetQuickHelpText(colorHelpText);
        }
    }

    void ToolboxButtonColorUpdaterBase::Update(const Color& rColor, bool bForceUpdate)
    {
        vcl::ImageType eImageType = GetImageSize();

#ifdef IOS // tdf#126966
        eImageType = vcl::ImageType::Size32;
#endif

        const bool bSizeChanged = (meImageType != eImageType);
        meImageType = eImageType;
        const bool bDisplayModeChanged = (mbWasHiContrastMode != Application::GetSettings().GetStyleSettings().GetHighContrastMode());
        Color aColor(rColor);

        // !!! #109290# Workaround for SetFillColor with COL_AUTO
        if (aColor == COL_AUTO)
            aColor = COL_TRANSPARENT;

        if ((maCurColor == aColor) && !bSizeChanged && !bDisplayModeChanged && !bForceUpdate)
            return;

        auto xImage = vcl::CommandInfoProvider::GetXGraphicForCommand(maCommandURL, mxFrame, meImageType);
        Image aImage(xImage);

        Size aItemSize = GetItemSize(aImage.GetSizePixel());
        if (!aItemSize.Width() || !aItemSize.Height())
            return;

        ScopedVclPtr<VirtualDevice> pVirDev(CreateVirtualDevice());
        pVirDev->SetOutputSizePixel(aItemSize);
        maBmpSize = aItemSize;

        if (maBmpSize.Width() == maBmpSize.Height())
            // tdf#84985 align color bar with icon bottom edge; integer arithmetic e.g. 26 - 26/4 <> 26 * 3/4
            maUpdRect = tools::Rectangle(Point( 0, maBmpSize.Height() - maBmpSize.Height() / 4), Size(maBmpSize.Width(), maBmpSize.Height() / 4));
        else
            maUpdRect = tools::Rectangle(Point( maBmpSize.Height() + 2, 2), Point(maBmpSize.Width() - 3, maBmpSize.Height() - 3));

        pVirDev->Push(PushFlags::CLIPREGION);

        // tdf#135121 don't include the part of the image which we will
        // overwrite with the target color so that for the transparent color
        // case the original background of the device is shown
        vcl::Region aRegion(tools::Rectangle(Point(0, 0), maBmpSize));
        aRegion.Exclude(maUpdRect);
        pVirDev->SetClipRegion(aRegion);

        pVirDev->DrawImage(Point(0, 0), aImage);

        pVirDev->Pop();

        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        mbWasHiContrastMode = rStyleSettings.GetHighContrastMode();

        if ((COL_TRANSPARENT != aColor) && (maBmpSize.Width() == maBmpSize.Height()))
            pVirDev->SetLineColor(aColor);
        else
            pVirDev->SetLineColor(rStyleSettings.GetDisableColor());

        // use not only COL_TRANSPARENT for detection of transparence,
        // but the method/way which is designed to do that
        const bool bIsTransparent(0xff == aColor.GetTransparency());
        maCurColor = aColor;

        if (bIsTransparent)
        {
            pVirDev->SetFillColor();
        }
        else
        {
            pVirDev->SetFillColor(maCurColor);
        }

        pVirDev->DrawRect(maUpdRect);

        SetImage(pVirDev.get());
    }

    OUString ToolboxButtonColorUpdaterBase::GetCurrentColorName() const
    {
        OUString sColorName = GetQuickHelpText();
        // The obtained string is of format: color context (color name)
        // Generate a substring which contains only the color name
        sal_Int32 nStart = sColorName.indexOf('(');
        sColorName = sColorName.copy( nStart + 1 );
        sal_Int32 nLength = sColorName.getLength();
        if(nLength > 0)
            sColorName = sColorName.copy( 0, nLength - 1);
        return sColorName;
    }

    ToolboxButtonColorUpdater::ToolboxButtonColorUpdater(sal_uInt16 nSlotId, const OString& rTbxBtnId, weld::Toolbar* ptrTbx, bool bWideButton,
                                                         const OUString& rCommandLabel, const css::uno::Reference<css::frame::XFrame>& rFrame)
        : ToolboxButtonColorUpdaterBase(bWideButton, rCommandLabel, OUString::fromUtf8(rTbxBtnId), rFrame)
        , msBtnId(rTbxBtnId)
        , mpTbx(ptrTbx)
    {
        Init(nSlotId);
    }

    void ToolboxButtonColorUpdater::SetQuickHelpText(const OUString& rText)
    {
        mpTbx->set_item_tooltip_text(msBtnId, rText);
    }

    OUString ToolboxButtonColorUpdater::GetQuickHelpText() const
    {
        return mpTbx->get_item_tooltip_text(msBtnId);
    }

    void ToolboxButtonColorUpdater::SetImage(VirtualDevice* pVirDev)
    {
        mpTbx->set_item_image(msBtnId, pVirDev);
    }

    VclPtr<VirtualDevice> ToolboxButtonColorUpdater::CreateVirtualDevice() const
    {
        return mpTbx->create_virtual_device();
    }

    vcl::ImageType ToolboxButtonColorUpdater::GetImageSize() const
    {
        return mpTbx->get_icon_size();
    }

    Size ToolboxButtonColorUpdater::GetItemSize(const Size& rImageSize) const
    {
        auto nWidth = rImageSize.Width();
        if (mbWideButton)
            nWidth = nWidth * 5;
        return Size(nWidth, rImageSize.Height());
    }

    ToolboxButtonLineStyleUpdater::ToolboxButtonLineStyleUpdater()
        : m_eXLS(css::drawing::LineStyle_NONE)
        , m_nDashStyleIndex(-1)
    {
    }

    void ToolboxButtonLineStyleUpdater::Update(const com::sun::star::frame::FeatureStateEvent& rEvent)
    {
        if (rEvent.FeatureURL.Complete == ".uno:LineDash")
        {
            m_nDashStyleIndex = -1;

            SfxObjectShell* pSh = SfxObjectShell::Current();
            if (!pSh)
                return;
            const SvxDashListItem* pItem = pSh->GetItem( SID_DASH_LIST );
            if (!pItem)
                return;

            XLineDashItem aDashItem;
            aDashItem.PutValue(rEvent.State, 0);
            const XDash& rDash = aDashItem.GetDashValue();

            XDashListRef xLineStyleList = pItem->GetDashList();
            for (long i = 0; i < xLineStyleList->Count(); ++i)
            {
                const XDashEntry* pEntry = xLineStyleList->GetDash(i);
                const XDash& rEntry = pEntry->GetDash();
                if (rDash == rEntry)
                {
                    m_nDashStyleIndex = i;
                    break;
                }
            }
        }
        else if (rEvent.FeatureURL.Complete == ".uno:XLineStyle")
        {
            XLineStyleItem aLineStyleItem;
            aLineStyleItem.PutValue(rEvent.State, 0);

            m_eXLS = aLineStyleItem.GetValue();
        }
    }

    int ToolboxButtonLineStyleUpdater::GetStyleIndex() const
    {
        int nRet;
        switch (m_eXLS)
        {
            case css::drawing::LineStyle_NONE:
                nRet = 0;
                break;
            case css::drawing::LineStyle_SOLID:
                nRet = 1;
                break;
            default:
                nRet = m_nDashStyleIndex + 2;
                break;
        }
        return nRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
