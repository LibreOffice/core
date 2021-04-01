/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <comphelper/base64.hxx>
#include <o3tl/enumarray.hxx>
#include <o3tl/enumrange.hxx>
#include <tools/stream.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/decoview.hxx>
#include <vcl/toolkit/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <bitmaps.hlst>
#include <messagedialog.hxx>
#include <svdata.hxx>
#include <window.h>
#include <boost/multi_array.hpp>
#include <boost/property_tree/ptree.hpp>
#include <vcl/toolkit/vclmedit.hxx>
#include <sal/log.hxx>
#include <tools/json_writer.hxx>

VclContainer::VclContainer(vcl::Window *pParent, WinBits nStyle)
    : Window(WindowType::CONTAINER)
    , m_bLayoutDirty(true)
{
    ImplInit(pParent, nStyle, nullptr);
    EnableChildTransparentMode();
    SetPaintTransparent(true);
    SetBackground();
}

sal_uInt16 VclContainer::getDefaultAccessibleRole() const
{
    return css::accessibility::AccessibleRole::PANEL;
}

Size VclContainer::GetOptimalSize() const
{
    return calculateRequisition();
}

void VclContainer::setLayoutPosSize(vcl::Window &rWindow, const Point &rPos, const Size &rSize)
{
    sal_Int32 nBorderWidth = rWindow.get_border_width();
    sal_Int32 nLeft = rWindow.get_margin_left() + nBorderWidth;
    sal_Int32 nTop = rWindow.get_margin_top() + nBorderWidth;
    sal_Int32 nRight = rWindow.get_margin_right() + nBorderWidth;
    sal_Int32 nBottom = rWindow.get_margin_bottom() + nBorderWidth;
    Point aPos(rPos.X() + nLeft, rPos.Y() + nTop);
    Size aSize(rSize.Width() - nLeft - nRight, rSize.Height() - nTop - nBottom);
    rWindow.SetPosSizePixel(aPos, aSize);
}

void VclContainer::setLayoutAllocation(vcl::Window &rChild, const Point &rAllocPos, const Size &rChildAlloc)
{
    VclAlign eHalign = rChild.get_halign();
    VclAlign eValign = rChild.get_valign();

    //typical case
    if (eHalign == VclAlign::Fill && eValign == VclAlign::Fill)
    {
        setLayoutPosSize(rChild, rAllocPos, rChildAlloc);
        return;
    }

    Point aChildPos(rAllocPos);
    Size aChildSize(rChildAlloc);
    Size aChildPreferredSize(getLayoutRequisition(rChild));

    switch (eHalign)
    {
        case VclAlign::Fill:
            break;
        case VclAlign::Start:
            if (aChildPreferredSize.Width() < rChildAlloc.Width())
                aChildSize.setWidth( aChildPreferredSize.Width() );
            break;
        case VclAlign::End:
            if (aChildPreferredSize.Width() < rChildAlloc.Width())
                aChildSize.setWidth( aChildPreferredSize.Width() );
            aChildPos.AdjustX(rChildAlloc.Width() );
            aChildPos.AdjustX( -(aChildSize.Width()) );
            break;
        case VclAlign::Center:
            if (aChildPreferredSize.Width() < aChildSize.Width())
                aChildSize.setWidth( aChildPreferredSize.Width() );
            aChildPos.AdjustX((rChildAlloc.Width() - aChildSize.Width()) / 2 );
            break;
    }

    switch (eValign)
    {
        case VclAlign::Fill:
            break;
        case VclAlign::Start:
            if (aChildPreferredSize.Height() < rChildAlloc.Height())
                aChildSize.setHeight( aChildPreferredSize.Height() );
            break;
        case VclAlign::End:
            if (aChildPreferredSize.Height() < rChildAlloc.Height())
                aChildSize.setHeight( aChildPreferredSize.Height() );
            aChildPos.AdjustY(rChildAlloc.Height() );
            aChildPos.AdjustY( -(aChildSize.Height()) );
            break;
        case VclAlign::Center:
            if (aChildPreferredSize.Height() < aChildSize.Height())
                aChildSize.setHeight( aChildPreferredSize.Height() );
            aChildPos.AdjustY((rChildAlloc.Height() - aChildSize.Height()) / 2 );
            break;
    }

    setLayoutPosSize(rChild, aChildPos, aChildSize);
}

namespace
{
    Size subtractBorder(const vcl::Window &rWindow, const Size& rSize)
    {
        sal_Int32 nBorderWidth = rWindow.get_border_width();
        sal_Int32 nLeft = rWindow.get_margin_left() + nBorderWidth;
        sal_Int32 nTop = rWindow.get_margin_top() + nBorderWidth;
        sal_Int32 nRight = rWindow.get_margin_right() + nBorderWidth;
        sal_Int32 nBottom = rWindow.get_margin_bottom() + nBorderWidth;
        Size aSize(rSize);
        return Size(aSize.Width() + nLeft + nRight, aSize.Height() + nTop + nBottom);
    }
}

Size VclContainer::getLayoutRequisition(const vcl::Window &rWindow)
{
    return subtractBorder(rWindow, rWindow.get_preferred_size());
}

void VclContainer::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    bool bSizeChanged = rAllocation != GetOutputSizePixel();
    Window::SetPosSizePixel(rAllocPos, rAllocation);
    if (m_bLayoutDirty || bSizeChanged)
    {
        m_bLayoutDirty = false;
        setAllocation(rAllocation);
    }
}

void VclContainer::SetPosPixel(const Point& rAllocPos)
{
    Point aAllocPos = rAllocPos;
    sal_Int32 nBorderWidth = get_border_width();
    aAllocPos.AdjustX(nBorderWidth + get_margin_left() );
    aAllocPos.AdjustY(nBorderWidth + get_margin_top() );

    if (aAllocPos != GetPosPixel())
        Window::SetPosPixel(aAllocPos);
}

void VclContainer::SetSizePixel(const Size& rAllocation)
{
    Size aAllocation = rAllocation;
    sal_Int32 nBorderWidth = get_border_width();
    aAllocation.AdjustWidth( -(nBorderWidth*2 + get_margin_left() + get_margin_right()) );
    aAllocation.AdjustHeight( -(nBorderWidth*2 + get_margin_top() + get_margin_bottom()) );
    bool bSizeChanged = aAllocation != GetSizePixel();
    if (bSizeChanged)
        Window::SetSizePixel(aAllocation);
    if (m_bLayoutDirty || bSizeChanged)
    {
        m_bLayoutDirty = false;
        setAllocation(aAllocation);
    }
}

void VclContainer::queue_resize(StateChangedType eReason)
{
    m_bLayoutDirty = true;
    Window::queue_resize(eReason);
}

// support for screenshot context menu
void VclContainer::Command(const CommandEvent& rCEvt)
{
    if (CommandEventId::ContextMenu == rCEvt.GetCommand())
    {
        auto pParent = GetParent();
        if (pParent)
        {
            CommandEvent aCEvt(rCEvt.GetMousePosPixel() + GetPosPixel(), rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetEventData());
            pParent->Command(aCEvt);
            return;
        }
    }

    // call parent (do not consume)
    Window::Command(rCEvt);
}

void VclBox::accumulateMaxes(const Size &rChildSize, Size &rSize) const
{
    tools::Long nSecondaryChildDimension = getSecondaryDimension(rChildSize);
    tools::Long nSecondaryBoxDimension = getSecondaryDimension(rSize);
    setSecondaryDimension(rSize, std::max(nSecondaryChildDimension, nSecondaryBoxDimension));

    tools::Long nPrimaryChildDimension = getPrimaryDimension(rChildSize);
    tools::Long nPrimaryBoxDimension = getPrimaryDimension(rSize);
    if (m_bHomogeneous)
        setPrimaryDimension(rSize, std::max(nPrimaryBoxDimension, nPrimaryChildDimension));
    else
        setPrimaryDimension(rSize, nPrimaryBoxDimension + nPrimaryChildDimension);
}

Size VclBox::calculateRequisition() const
{
    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = getLayoutRequisition(*pChild);

        tools::Long nPrimaryDimension = getPrimaryDimension(aChildSize);
        nPrimaryDimension += pChild->get_padding() * 2;
        setPrimaryDimension(aChildSize, nPrimaryDimension);

        accumulateMaxes(aChildSize, aSize);
    }

    return finalizeMaxes(aSize, nVisibleChildren);
}

void VclBox::setAllocation(const Size &rAllocation)
{
    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        bool bExpand = getPrimaryDimensionChildExpand(*pChild);
        if (bExpand)
            ++nExpandChildren;
    }

    if (!nVisibleChildren)
        return;

    tools::Long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);

    tools::Long nHomogeneousDimension = 0, nExtraSpace = 0;
    if (m_bHomogeneous)
    {
        nHomogeneousDimension = (nAllocPrimaryDimension -
            (nVisibleChildren - 1) * m_nSpacing) / nVisibleChildren;
    }
    else if (nExpandChildren)
    {
        Size aRequisition = calculateRequisition();
        nExtraSpace = (getPrimaryDimension(rAllocation) - getPrimaryDimension(aRequisition)) / nExpandChildren;
    }

    //Split into those we pack from the start onwards, and those we pack from the end backwards
    o3tl::enumarray<VclPackType,std::vector<vcl::Window*>> aWindows;
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;

        VclPackType ePacking = pChild->get_pack_type();
        aWindows[ePacking].push_back(pChild);
    }

    //See VclBuilder::sortIntoBestTabTraversalOrder for why they are in visual
    //order under the parent which requires us to reverse them here to
    //pack from the end back
    std::reverse(aWindows[VclPackType::End].begin(),aWindows[VclPackType::End].end());

    for (VclPackType ePackType : o3tl::enumrange<VclPackType>())
    {
        Point aPos(0, 0);
        if (ePackType == VclPackType::End)
        {
            tools::Long nPrimaryCoordinate = getPrimaryCoordinate(aPos);
            setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension);
        }

        for (auto const& window : aWindows[ePackType])
        {
            vcl::Window *pChild = window;

            tools::Long nPadding = pChild->get_padding();

            Size aBoxSize;
            if (m_bHomogeneous)
                setPrimaryDimension(aBoxSize, nHomogeneousDimension);
            else
            {
                aBoxSize = getLayoutRequisition(*pChild);
                tools::Long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding * 2;
                if (getPrimaryDimensionChildExpand(*pChild))
                    nPrimaryDimension += nExtraSpace;
                setPrimaryDimension(aBoxSize, nPrimaryDimension);
            }
            setSecondaryDimension(aBoxSize, getSecondaryDimension(rAllocation));

            Point aChildPos(aPos);
            Size aChildSize(aBoxSize);
            tools::Long nPrimaryCoordinate = getPrimaryCoordinate(aPos);

            bool bFill = pChild->get_fill();
            if (bFill)
            {
                setPrimaryDimension(aChildSize, std::max(static_cast<tools::Long>(1),
                    getPrimaryDimension(aBoxSize) - nPadding * 2));

                setPrimaryCoordinate(aChildPos, nPrimaryCoordinate + nPadding);
            }
            else
            {
                setPrimaryDimension(aChildSize,
                    getPrimaryDimension(getLayoutRequisition(*pChild)));

                setPrimaryCoordinate(aChildPos, nPrimaryCoordinate +
                    (getPrimaryDimension(aBoxSize) - getPrimaryDimension(aChildSize)) / 2);
            }

            tools::Long nDiff = getPrimaryDimension(aBoxSize) + m_nSpacing;
            if (ePackType == VclPackType::Start)
                setPrimaryCoordinate(aPos, nPrimaryCoordinate + nDiff);
            else
            {
                setPrimaryCoordinate(aPos, nPrimaryCoordinate - nDiff);
                setPrimaryCoordinate(aChildPos, getPrimaryCoordinate(aChildPos) -
                    getPrimaryDimension(aBoxSize));
            }

            setLayoutAllocation(*pChild, aChildPos, aChildSize);
        }
    }
}

bool VclBox::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "spacing")
        set_spacing(rValue.toInt32());
    else if (rKey == "homogeneous")
        set_homogeneous(toBool(rValue));
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

void VclBox::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    VclContainer::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("vertical", m_bVerticalContainer);
}

sal_uInt16 VclBox::getDefaultAccessibleRole() const
{
#if defined(_WIN32)
    //fdo#74284 call Boxes Panels, keep then as "Filler" under
    //at least Linux seeing as that's what Gtk does for GtkBoxes
    return css::accessibility::AccessibleRole::PANEL;
#else
    return css::accessibility::AccessibleRole::FILLER;
#endif
}

#define DEFAULT_CHILD_MIN_WIDTH 85
#define DEFAULT_CHILD_MIN_HEIGHT 27

Size VclBox::finalizeMaxes(const Size &rSize, sal_uInt16 nVisibleChildren) const
{
    Size aRet;

    if (nVisibleChildren)
    {
        tools::Long nPrimaryDimension = getPrimaryDimension(rSize);
        if (m_bHomogeneous)
            nPrimaryDimension *= nVisibleChildren;
        setPrimaryDimension(aRet, nPrimaryDimension + m_nSpacing * (nVisibleChildren-1));
        setSecondaryDimension(aRet, getSecondaryDimension(rSize));
    }

    return aRet;
}

Size VclButtonBox::addReqGroups(const VclButtonBox::Requisition &rReq) const
{
    Size aRet;

    tools::Long nMainGroupDimension = getPrimaryDimension(rReq.m_aMainGroupSize);
    tools::Long nSubGroupDimension = getPrimaryDimension(rReq.m_aSubGroupSize);

    setPrimaryDimension(aRet, nMainGroupDimension + nSubGroupDimension);

    setSecondaryDimension(aRet,
        std::max(getSecondaryDimension(rReq.m_aMainGroupSize),
        getSecondaryDimension(rReq.m_aSubGroupSize)));

    return aRet;
}

static tools::Long getMaxNonOutlier(const std::vector<tools::Long> &rG, tools::Long nAvgDimension)
{
    tools::Long nMaxDimensionNonOutlier = 0;
    for (auto const& nPrimaryChildDimension : rG)
    {
        if (nPrimaryChildDimension < nAvgDimension * 1.5)
        {
            nMaxDimensionNonOutlier = std::max(nPrimaryChildDimension,
                nMaxDimensionNonOutlier);
        }
    }
    return nMaxDimensionNonOutlier;
}

static std::vector<tools::Long> setButtonSizes(const std::vector<tools::Long> &rG,
    const std::vector<bool> &rNonHomogeneous,
    tools::Long nAvgDimension, tools::Long nMaxNonOutlier, tools::Long nMinWidth)
{
    std::vector<tools::Long> aVec;
    //set everything < 1.5 times the average to the same width, leave the
    //outliers un-touched
    std::vector<bool>::const_iterator aJ = rNonHomogeneous.begin();
    auto nNonOutlierWidth = std::max(nMaxNonOutlier, nMinWidth);
    for (auto const& nPrimaryChildDimension : rG)
    {
        bool bNonHomogeneous = *aJ;
        if (!bNonHomogeneous && nPrimaryChildDimension < nAvgDimension * 1.5)
        {
            aVec.push_back(nNonOutlierWidth);
        }
        else
        {
            aVec.push_back(std::max(nPrimaryChildDimension, nMinWidth));
        }
        ++aJ;
    }
    return aVec;
}

VclButtonBox::Requisition VclButtonBox::calculatePrimarySecondaryRequisitions() const
{
    Requisition aReq;

    Size aMainGroupSize(DEFAULT_CHILD_MIN_WIDTH, DEFAULT_CHILD_MIN_HEIGHT); //to-do, pull from theme
    Size aSubGroupSize(DEFAULT_CHILD_MIN_WIDTH, DEFAULT_CHILD_MIN_HEIGHT); //to-do, pull from theme

    tools::Long nMinMainGroupPrimary = getPrimaryDimension(aMainGroupSize);
    tools::Long nMinSubGroupPrimary = getPrimaryDimension(aSubGroupSize);
    tools::Long nMainGroupSecondary = getSecondaryDimension(aMainGroupSize);
    tools::Long nSubGroupSecondary = getSecondaryDimension(aSubGroupSize);

    bool bIgnoreSecondaryPacking = (m_eLayoutStyle == VclButtonBoxStyle::Spread || m_eLayoutStyle == VclButtonBoxStyle::Center);

    std::vector<tools::Long> aMainGroupSizes;
    std::vector<bool> aMainGroupNonHomogeneous;
    std::vector<tools::Long> aSubGroupSizes;
    std::vector<bool> aSubGroupNonHomogeneous;

    for (const vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        Size aChildSize = getLayoutRequisition(*pChild);
        if (bIgnoreSecondaryPacking || !pChild->get_secondary())
        {
            //set the max secondary dimension
            nMainGroupSecondary = std::max(nMainGroupSecondary, getSecondaryDimension(aChildSize));
            //collect the primary dimensions
            aMainGroupSizes.push_back(getPrimaryDimension(aChildSize));
            aMainGroupNonHomogeneous.push_back(pChild->get_non_homogeneous());
        }
        else
        {
            nSubGroupSecondary = std::max(nSubGroupSecondary, getSecondaryDimension(aChildSize));
            aSubGroupSizes.push_back(getPrimaryDimension(aChildSize));
            aSubGroupNonHomogeneous.push_back(pChild->get_non_homogeneous());
        }
    }

    if (m_bHomogeneous)
    {
        tools::Long nMaxMainDimension = aMainGroupSizes.empty() ? 0 :
            *std::max_element(aMainGroupSizes.begin(), aMainGroupSizes.end());
        nMaxMainDimension = std::max(nMaxMainDimension, nMinMainGroupPrimary);
        tools::Long nMaxSubDimension = aSubGroupSizes.empty() ? 0 :
            *std::max_element(aSubGroupSizes.begin(), aSubGroupSizes.end());
        nMaxSubDimension = std::max(nMaxSubDimension, nMinSubGroupPrimary);
        tools::Long nMaxDimension = std::max(nMaxMainDimension, nMaxSubDimension);
        aReq.m_aMainGroupDimensions.resize(aMainGroupSizes.size(), nMaxDimension);
        aReq.m_aSubGroupDimensions.resize(aSubGroupSizes.size(), nMaxDimension);
    }
    else
    {
        //Ideally set everything to the same size, but find outlier widgets
        //that are way wider than the average and leave them
        //at their natural size and set the remainder to share the
        //max size of the remaining members of the buttonbox
        tools::Long nAccDimension = std::accumulate(aMainGroupSizes.begin(),
            aMainGroupSizes.end(), 0);
        nAccDimension = std::accumulate(aSubGroupSizes.begin(),
            aSubGroupSizes.end(), nAccDimension);

        size_t nTotalSize = aMainGroupSizes.size() + aSubGroupSizes.size();

        tools::Long nAvgDimension = nTotalSize ? nAccDimension / nTotalSize : 0;

        tools::Long nMaxMainNonOutlier = getMaxNonOutlier(aMainGroupSizes,
            nAvgDimension);
        tools::Long nMaxSubNonOutlier = getMaxNonOutlier(aSubGroupSizes,
            nAvgDimension);
        tools::Long nMaxNonOutlier = std::max(nMaxMainNonOutlier, nMaxSubNonOutlier);

        aReq.m_aMainGroupDimensions = setButtonSizes(aMainGroupSizes,
            aMainGroupNonHomogeneous,
            nAvgDimension, nMaxNonOutlier, nMinMainGroupPrimary);
        aReq.m_aSubGroupDimensions = setButtonSizes(aSubGroupSizes,
            aSubGroupNonHomogeneous,
            nAvgDimension, nMaxNonOutlier, nMinSubGroupPrimary);
    }

    if (!aReq.m_aMainGroupDimensions.empty())
    {
        setSecondaryDimension(aReq.m_aMainGroupSize, nMainGroupSecondary);
        setPrimaryDimension(aReq.m_aMainGroupSize,
            std::accumulate(aReq.m_aMainGroupDimensions.begin(),
                aReq.m_aMainGroupDimensions.end(), 0));
    }
    if (!aReq.m_aSubGroupDimensions.empty())
    {
        setSecondaryDimension(aReq.m_aSubGroupSize, nSubGroupSecondary);
        setPrimaryDimension(aReq.m_aSubGroupSize,
            std::accumulate(aReq.m_aSubGroupDimensions.begin(),
                aReq.m_aSubGroupDimensions.end(), 0));
    }

    return aReq;
}

Size VclButtonBox::addSpacing(const Size &rSize, sal_uInt16 nVisibleChildren) const
{
    Size aRet;

    if (nVisibleChildren)
    {
        tools::Long nPrimaryDimension = getPrimaryDimension(rSize);
        setPrimaryDimension(aRet,
            nPrimaryDimension + m_nSpacing * (nVisibleChildren-1));
        setSecondaryDimension(aRet, getSecondaryDimension(rSize));
    }

    return aRet;
}

Size VclButtonBox::calculateRequisition() const
{
    Requisition aReq(calculatePrimarySecondaryRequisitions());
    sal_uInt16 nVisibleChildren = aReq.m_aMainGroupDimensions.size() +
        aReq.m_aSubGroupDimensions.size();
    return addSpacing(addReqGroups(aReq), nVisibleChildren);
}

bool VclButtonBox::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "layout-style")
    {
        VclButtonBoxStyle eStyle = VclButtonBoxStyle::Default;
        if (rValue == "spread")
            eStyle = VclButtonBoxStyle::Spread;
        else if (rValue == "edge")
            eStyle = VclButtonBoxStyle::Edge;
        else if (rValue == "start")
            eStyle = VclButtonBoxStyle::Start;
        else if (rValue == "end")
            eStyle = VclButtonBoxStyle::End;
        else if (rValue == "center")
            eStyle = VclButtonBoxStyle::Center;
        else
        {
            SAL_WARN("vcl.layout", "unknown layout style " << rValue);
        }
        m_eLayoutStyle = eStyle;
    }
    else
        return VclBox::set_property(rKey, rValue);
    return true;
}

void VclButtonBox::setAllocation(const Size &rAllocation)
{
    Requisition aReq(calculatePrimarySecondaryRequisitions());

    if (aReq.m_aMainGroupDimensions.empty() && aReq.m_aSubGroupDimensions.empty())
        return;

    tools::Long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);

    Point aMainGroupPos, aOtherGroupPos;
    int nSpacing = m_nSpacing;

    //To-Do, other layout styles
    switch (m_eLayoutStyle)
    {
        case VclButtonBoxStyle::Start:
            if (!aReq.m_aSubGroupDimensions.empty())
            {
                tools::Long nOtherPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aSubGroupSize, aReq.m_aSubGroupDimensions.size()));
                setPrimaryCoordinate(aOtherGroupPos,
                    nAllocPrimaryDimension - nOtherPrimaryDimension);
            }
            break;
        case VclButtonBoxStyle::Spread:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                tools::Long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                tools::Long nExtraSpace = nAllocPrimaryDimension - nMainPrimaryDimension;
                nExtraSpace += (aReq.m_aMainGroupDimensions.size()-1) * nSpacing;
                nSpacing = nExtraSpace/(aReq.m_aMainGroupDimensions.size()+1);
                setPrimaryCoordinate(aMainGroupPos, nSpacing);
            }
            break;
        case VclButtonBoxStyle::Center:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                tools::Long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                tools::Long nExtraSpace = nAllocPrimaryDimension - nMainPrimaryDimension;
                setPrimaryCoordinate(aMainGroupPos, nExtraSpace/2);
            }
            break;
        default:
            SAL_WARN("vcl.layout", "todo unimplemented layout style");
            [[fallthrough]];
        case VclButtonBoxStyle::Default:
        case VclButtonBoxStyle::End:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                tools::Long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                setPrimaryCoordinate(aMainGroupPos,
                    nAllocPrimaryDimension - nMainPrimaryDimension);
            }
            break;
    }

    Size aChildSize;
    setSecondaryDimension(aChildSize, getSecondaryDimension(rAllocation));

    std::vector<tools::Long>::const_iterator aPrimaryI = aReq.m_aMainGroupDimensions.begin();
    std::vector<tools::Long>::const_iterator aSecondaryI = aReq.m_aSubGroupDimensions.begin();
    bool bIgnoreSecondaryPacking = (m_eLayoutStyle == VclButtonBoxStyle::Spread || m_eLayoutStyle == VclButtonBoxStyle::Center);
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;

        if (bIgnoreSecondaryPacking || !pChild->get_secondary())
        {
            tools::Long nMainGroupPrimaryDimension = *aPrimaryI++;
            setPrimaryDimension(aChildSize, nMainGroupPrimaryDimension);
            setLayoutAllocation(*pChild, aMainGroupPos, aChildSize);
            tools::Long nPrimaryCoordinate = getPrimaryCoordinate(aMainGroupPos);
            setPrimaryCoordinate(aMainGroupPos, nPrimaryCoordinate + nMainGroupPrimaryDimension + nSpacing);
        }
        else
        {
            tools::Long nSubGroupPrimaryDimension = *aSecondaryI++;
            setPrimaryDimension(aChildSize, nSubGroupPrimaryDimension);
            setLayoutAllocation(*pChild, aOtherGroupPos, aChildSize);
            tools::Long nPrimaryCoordinate = getPrimaryCoordinate(aOtherGroupPos);
            setPrimaryCoordinate(aOtherGroupPos, nPrimaryCoordinate + nSubGroupPrimaryDimension + nSpacing);
        }
    }
}

namespace {

struct ButtonOrder
{
    const char* m_aType;
    int m_nPriority;
};

}

static int getButtonPriority(const OString &rType)
{
    static const size_t N_TYPES = 6;
    static const ButtonOrder aDiscardCancelSave[N_TYPES] =
    {
        { "/discard", 0 },
        { "/cancel", 1 },
        { "/no", 2 },
        { "/save", 3 },
        { "/yes", 3 },
        { "/ok", 3 }
    };

    static const ButtonOrder aSaveDiscardCancel[N_TYPES] =
    {
        { "/save", 0 },
        { "/yes", 0 },
        { "/ok", 0 },
        { "/discard", 1 },
        { "/no", 1 },
        { "/cancel", 2 }
    };

    const ButtonOrder* pOrder = &aDiscardCancelSave[0];

    const OUString &rEnv = Application::GetDesktopEnvironment();

    if (rEnv.equalsIgnoreAsciiCase("windows") ||
        rEnv.equalsIgnoreAsciiCase("lxqt") ||
        rEnv.startsWithIgnoreAsciiCase("plasma"))
    {
        pOrder = &aSaveDiscardCancel[0];
    }

    for (size_t i = 0; i < N_TYPES; ++i, ++pOrder)
    {
        if (rType.endsWith(pOrder->m_aType))
            return pOrder->m_nPriority;
    }

    return -1;
}

namespace {

class sortButtons
{
    bool m_bVerticalContainer;
public:
    explicit sortButtons(bool bVerticalContainer)
        : m_bVerticalContainer(bVerticalContainer)
    {
    }
    bool operator()(const vcl::Window *pA, const vcl::Window *pB) const;
};

}

bool sortButtons::operator()(const vcl::Window *pA, const vcl::Window *pB) const
{
    //sort into two groups of pack start and pack end
    VclPackType ePackA = pA->get_pack_type();
    VclPackType ePackB = pB->get_pack_type();
    if (ePackA < ePackB)
        return true;
    if (ePackA > ePackB)
        return false;
    bool bPackA = pA->get_secondary();
    bool bPackB = pB->get_secondary();
    if (!m_bVerticalContainer)
    {
        //for horizontal boxes group secondaries before primaries
        if (bPackA > bPackB)
            return true;
        if (bPackA < bPackB)
            return false;
    }
    else
    {
        //for vertical boxes group secondaries after primaries
        if (bPackA < bPackB)
            return true;
        if (bPackA > bPackB)
            return false;
    }

    //now order within groups according to platform rules
    return getButtonPriority(pA->GetHelpId()) < getButtonPriority(pB->GetHelpId());
}

void sort_native_button_order(const VclBox& rContainer)
{
    std::vector<vcl::Window*> aChilds;
    for (vcl::Window* pChild = rContainer.GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        aChilds.push_back(pChild);
    }

    //sort child order within parent so that we match the platform
    //button order
    std::stable_sort(aChilds.begin(), aChilds.end(), sortButtons(rContainer.get_orientation()));
    BuilderUtils::reorderWithinParent(aChilds, true);
}

namespace {

struct GridEntry
{
    VclPtr<vcl::Window> pChild;
    sal_Int32 nSpanWidth;
    sal_Int32 nSpanHeight;
    int x;
    int y;
    GridEntry()
        : pChild(nullptr)
        , nSpanWidth(0)
        , nSpanHeight(0)
        , x(-1)
        , y(-1)
    {
    }
};

}

typedef boost::multi_array<GridEntry, 2> array_type;

static array_type assembleGrid(const VclGrid &rGrid);
static bool isNullGrid(const array_type& A);
static void calcMaxs(const array_type &A, std::vector<VclGrid::Value> &rWidths, std::vector<VclGrid::Value> &rHeights);

array_type assembleGrid(const VclGrid &rGrid)
{
    array_type A;

    for (vcl::Window* pChild = rGrid.GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        sal_Int32 nLeftAttach = std::max<sal_Int32>(pChild->get_grid_left_attach(), 0);
        sal_Int32 nWidth = pChild->get_grid_width();
        sal_Int32 nMaxXPos = nLeftAttach+nWidth-1;

        sal_Int32 nTopAttach = std::max<sal_Int32>(pChild->get_grid_top_attach(), 0);
        sal_Int32 nHeight = pChild->get_grid_height();
        sal_Int32 nMaxYPos = nTopAttach+nHeight-1;

        sal_Int32 nCurrentMaxXPos = A.shape()[0]-1;
        sal_Int32 nCurrentMaxYPos = A.shape()[1]-1;
        if (nMaxXPos > nCurrentMaxXPos || nMaxYPos > nCurrentMaxYPos)
        {
            nCurrentMaxXPos = std::max(nMaxXPos, nCurrentMaxXPos);
            nCurrentMaxYPos = std::max(nMaxYPos, nCurrentMaxYPos);
            A.resize(boost::extents[nCurrentMaxXPos+1][nCurrentMaxYPos+1]);
        }

        GridEntry &rEntry = A[nLeftAttach][nTopAttach];
        rEntry.pChild = pChild;
        rEntry.nSpanWidth = nWidth;
        rEntry.nSpanHeight = nHeight;
        rEntry.x = nLeftAttach;
        rEntry.y = nTopAttach;

        for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
        {
            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
            {
                GridEntry &rSpan = A[nLeftAttach+nSpanX][nTopAttach+nSpanY];
                rSpan.x = nLeftAttach;
                rSpan.y = nTopAttach;
            }
        }
    }

    //see if we have any empty rows/cols
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    std::vector<bool> aNonEmptyCols(nMaxX);
    std::vector<bool> aNonEmptyRows(nMaxY);

    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            const GridEntry &rEntry = A[x][y];
            const vcl::Window *pChild = rEntry.pChild;
            if (pChild && pChild->IsVisible())
            {
                aNonEmptyCols[x] = true;
                if (rGrid.get_column_homogeneous())
                {
                    for (sal_Int32 nSpanX = 1; nSpanX < rEntry.nSpanWidth; ++nSpanX)
                        aNonEmptyCols[x+nSpanX] = true;
                }
                aNonEmptyRows[y] = true;
                if (rGrid.get_row_homogeneous())
                {
                    for (sal_Int32 nSpanY = 1; nSpanY < rEntry.nSpanHeight; ++nSpanY)
                        aNonEmptyRows[y+nSpanY] = true;
                }
            }
        }
    }

    if (!rGrid.get_column_homogeneous())
    {
        //reduce the spans of elements that span empty columns
        for (sal_Int32 x = 0; x < nMaxX; ++x)
        {
            std::set<GridEntry*> candidates;
            for (sal_Int32 y = 0; y < nMaxY; ++y)
            {
                if (aNonEmptyCols[x])
                    continue;
                GridEntry &rSpan = A[x][y];
                //cell x/y is spanned by the widget at cell rSpan.x/rSpan.y,
                //just points back to itself if there's no cell spanning
                if ((rSpan.x == -1) || (rSpan.y == -1))
                {
                    //there is no entry for this cell, i.e. this is a cell
                    //with no widget in it, or spanned by any other widget
                    continue;
                }
                GridEntry &rEntry = A[rSpan.x][rSpan.y];
                candidates.insert(&rEntry);
            }
            for (auto const& candidate : candidates)
            {
                GridEntry *pEntry = candidate;
                --pEntry->nSpanWidth;
            }
        }
    }

    if (!rGrid.get_row_homogeneous())
    {
        //reduce the spans of elements that span empty rows
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            std::set<GridEntry*> candidates;
            for (sal_Int32 x = 0; x < nMaxX; ++x)
            {
                if (aNonEmptyRows[y])
                    continue;
                GridEntry &rSpan = A[x][y];
                //cell x/y is spanned by the widget at cell rSpan.x/rSpan.y,
                //just points back to itself if there's no cell spanning
                if ((rSpan.x == -1) || (rSpan.y == -1))
                {
                    //there is no entry for this cell, i.e. this is a cell
                    //with no widget in it, or spanned by any other widget
                    continue;
                }
                GridEntry &rEntry = A[rSpan.x][rSpan.y];
                candidates.insert(&rEntry);
            }
            for (auto const& candidate : candidates)
            {
                GridEntry *pEntry = candidate;
                --pEntry->nSpanHeight;
            }
        }
    }

    sal_Int32 nNonEmptyCols = std::count(aNonEmptyCols.begin(), aNonEmptyCols.end(), true);
    sal_Int32 nNonEmptyRows = std::count(aNonEmptyRows.begin(), aNonEmptyRows.end(), true);

    //make new grid without empty rows and columns
    array_type B(boost::extents[nNonEmptyCols][nNonEmptyRows]);
    for (sal_Int32 x = 0, x2 = 0; x < nMaxX; ++x)
    {
        if (!aNonEmptyCols[x])
            continue;
        for (sal_Int32 y = 0, y2 = 0; y < nMaxY; ++y)
        {
            if (!aNonEmptyRows[y])
                continue;
            GridEntry &rEntry = A[x][y];
            B[x2][y2++] = rEntry;
        }
        ++x2;
    }

    return B;
}

static bool isNullGrid(const array_type &A)
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    return !nMaxX || !nMaxY;
}

static void calcMaxs(const array_type &A, std::vector<VclGrid::Value> &rWidths, std::vector<VclGrid::Value> &rHeights)
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    rWidths.resize(nMaxX);
    rHeights.resize(nMaxY);

    //first use the non spanning entries to set default width/heights
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            const GridEntry &rEntry = A[x][y];
            const vcl::Window *pChild = rEntry.pChild;
            if (!pChild || !pChild->IsVisible())
                continue;

            sal_Int32 nWidth = rEntry.nSpanWidth;
            sal_Int32 nHeight = rEntry.nSpanHeight;

            for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                rWidths[x+nSpanX].m_bExpand |= pChild->get_hexpand();

            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                rHeights[y+nSpanY].m_bExpand |= pChild->get_vexpand();

            if (nWidth == 1 || nHeight == 1)
            {
                Size aChildSize = VclContainer::getLayoutRequisition(*pChild);
                if (nWidth == 1)
                    rWidths[x].m_nValue = std::max(rWidths[x].m_nValue, aChildSize.Width());
                if (nHeight == 1)
                    rHeights[y].m_nValue = std::max(rHeights[y].m_nValue, aChildSize.Height());
            }
        }
    }

    //now use the spanning entries and split any extra sizes across expanding rows/cols
    //where possible
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            const GridEntry &rEntry = A[x][y];
            const vcl::Window *pChild = rEntry.pChild;
            if (!pChild || !pChild->IsVisible())
                continue;

            sal_Int32 nWidth = rEntry.nSpanWidth;
            sal_Int32 nHeight = rEntry.nSpanHeight;

            if (nWidth == 1 && nHeight == 1)
                continue;

            Size aChildSize = VclContainer::getLayoutRequisition(*pChild);

            if (nWidth > 1)
            {
                sal_Int32 nExistingWidth = 0;
                for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    nExistingWidth += rWidths[x+nSpanX].m_nValue;

                sal_Int32 nExtraWidth = aChildSize.Width() - nExistingWidth;

                if (nExtraWidth > 0)
                {
                    bool bForceExpandAll = false;
                    sal_Int32 nExpandables = 0;
                    for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                        if (rWidths[x+nSpanX].m_bExpand)
                            ++nExpandables;
                    if (nExpandables == 0)
                    {
                        nExpandables = nWidth;
                        bForceExpandAll = true;
                    }

                    for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    {
                        if (rWidths[x+nSpanX].m_bExpand || bForceExpandAll)
                            rWidths[x+nSpanX].m_nValue += nExtraWidth/nExpandables;
                    }
                }
            }

            if (nHeight > 1)
            {
                sal_Int32 nExistingHeight = 0;
                for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    nExistingHeight += rHeights[y+nSpanY].m_nValue;

                sal_Int32 nExtraHeight = aChildSize.Height() - nExistingHeight;

                if (nExtraHeight > 0)
                {
                    bool bForceExpandAll = false;
                    sal_Int32 nExpandables = 0;
                    for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                        if (rHeights[y+nSpanY].m_bExpand)
                            ++nExpandables;
                    if (nExpandables == 0)
                    {
                        nExpandables = nHeight;
                        bForceExpandAll = true;
                    }

                    for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    {
                        if (rHeights[y+nSpanY].m_bExpand || bForceExpandAll)
                            rHeights[y+nSpanY].m_nValue += nExtraHeight/nExpandables;
                    }
                }
            }
        }
    }
}

static bool compareValues(const VclGrid::Value &i, const VclGrid::Value &j)
{
    return i.m_nValue < j.m_nValue;
}

static VclGrid::Value accumulateValues(const VclGrid::Value &i, const VclGrid::Value &j)
{
    VclGrid::Value aRet;
    aRet.m_nValue = i.m_nValue + j.m_nValue;
    aRet.m_bExpand = i.m_bExpand || j.m_bExpand;
    return aRet;
}

Size VclGrid::calculateRequisition() const
{
    return calculateRequisitionForSpacings(get_row_spacing(), get_column_spacing());
}

Size VclGrid::calculateRequisitionForSpacings(sal_Int32 nRowSpacing, sal_Int32 nColSpacing) const
{
    array_type A = assembleGrid(*this);

    if (isNullGrid(A))
        return Size();

    std::vector<Value> aWidths;
    std::vector<Value> aHeights;
    calcMaxs(A, aWidths, aHeights);

    tools::Long nTotalWidth = 0;
    if (get_column_homogeneous())
    {
        nTotalWidth = std::max_element(aWidths.begin(), aWidths.end(), compareValues)->m_nValue;
        nTotalWidth *= aWidths.size();
    }
    else
    {
        nTotalWidth = std::accumulate(aWidths.begin(), aWidths.end(), Value(), accumulateValues).m_nValue;
    }

    nTotalWidth += nColSpacing * (aWidths.size()-1);

    tools::Long nTotalHeight = 0;
    if (get_row_homogeneous())
    {
        nTotalHeight = std::max_element(aHeights.begin(), aHeights.end(), compareValues)->m_nValue;
        nTotalHeight *= aHeights.size();
    }
    else
    {
        nTotalHeight = std::accumulate(aHeights.begin(), aHeights.end(), Value(), accumulateValues).m_nValue;
    }

    nTotalHeight += nRowSpacing * (aHeights.size()-1);

    return Size(nTotalWidth, nTotalHeight);
}

void VclGrid::setAllocation(const Size& rAllocation)
{
    array_type A = assembleGrid(*this);

    if (isNullGrid(A))
        return;

    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    Size aRequisition;
    std::vector<Value> aWidths(nMaxX);
    std::vector<Value> aHeights(nMaxY);
    if (!get_column_homogeneous() || !get_row_homogeneous())
    {
        aRequisition = calculateRequisition();
        calcMaxs(A, aWidths, aHeights);
    }

    sal_Int32 nColSpacing(get_column_spacing());
    sal_Int32 nRowSpacing(get_row_spacing());

    tools::Long nAvailableWidth = rAllocation.Width();
    if (nMaxX)
        nAvailableWidth -= nColSpacing * (nMaxX - 1);
    if (get_column_homogeneous())
    {
        for (sal_Int32 x = 0; x < nMaxX; ++x)
            aWidths[x].m_nValue = nAvailableWidth/nMaxX;
    }
    else if (rAllocation.Width() != aRequisition.Width())
    {
        sal_Int32 nExpandables = 0;
        for (sal_Int32 x = 0; x < nMaxX; ++x)
            if (aWidths[x].m_bExpand)
                ++nExpandables;
        tools::Long nExtraWidthForExpanders = nExpandables ? (rAllocation.Width() - aRequisition.Width()) / nExpandables : 0;

        //We don't fit and there is no volunteer to be shrunk
        if (!nExpandables && rAllocation.Width() < aRequisition.Width())
        {
            //first reduce spacing
            while (nColSpacing)
            {
                nColSpacing /= 2;
                aRequisition = calculateRequisitionForSpacings(nRowSpacing, nColSpacing);
                if (aRequisition.Width() <= rAllocation.Width())
                    break;
            }

            //share out the remaining pain to everyone
            tools::Long nExtraWidth = (rAllocation.Width() - aRequisition.Width()) / nMaxX;

            for (sal_Int32 x = 0; x < nMaxX; ++x)
                aWidths[x].m_nValue += nExtraWidth;
        }

        if (nExtraWidthForExpanders)
        {
            for (sal_Int32 x = 0; x < nMaxX; ++x)
                if (aWidths[x].m_bExpand)
                    aWidths[x].m_nValue += nExtraWidthForExpanders;
        }
    }

    tools::Long nAvailableHeight = rAllocation.Height();
    if (nMaxY)
        nAvailableHeight -= nRowSpacing * (nMaxY - 1);
    if (get_row_homogeneous())
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
            aHeights[y].m_nValue = nAvailableHeight/nMaxY;
    }
    else if (rAllocation.Height() != aRequisition.Height())
    {
        sal_Int32 nExpandables = 0;
        for (sal_Int32 y = 0; y < nMaxY; ++y)
            if (aHeights[y].m_bExpand)
                ++nExpandables;
        tools::Long nExtraHeightForExpanders = nExpandables ? (rAllocation.Height() - aRequisition.Height()) / nExpandables : 0;

        //We don't fit and there is no volunteer to be shrunk
        if (!nExpandables && rAllocation.Height() < aRequisition.Height())
        {
            //first reduce spacing
            while (nRowSpacing)
            {
                nRowSpacing /= 2;
                aRequisition = calculateRequisitionForSpacings(nRowSpacing, nColSpacing);
                if (aRequisition.Height() <= rAllocation.Height())
                    break;
            }

            //share out the remaining pain to everyone
            tools::Long nExtraHeight = (rAllocation.Height() - aRequisition.Height()) / nMaxY;

            for (sal_Int32 y = 0; y < nMaxY; ++y)
                aHeights[y].m_nValue += nExtraHeight;
        }

        if (nExtraHeightForExpanders)
        {
            for (sal_Int32 y = 0; y < nMaxY; ++y)
                if (aHeights[y].m_bExpand)
                    aHeights[y].m_nValue += nExtraHeightForExpanders;
        }
    }

    Point aAllocPos(0, 0);
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            GridEntry &rEntry = A[x][y];
            vcl::Window *pChild = rEntry.pChild;
            if (pChild)
            {
                Size aChildAlloc(0, 0);

                sal_Int32 nWidth = rEntry.nSpanWidth;
                for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    aChildAlloc.AdjustWidth(aWidths[x+nSpanX].m_nValue );
                aChildAlloc.AdjustWidth(nColSpacing*(nWidth-1) );

                sal_Int32 nHeight = rEntry.nSpanHeight;
                for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    aChildAlloc.AdjustHeight(aHeights[y+nSpanY].m_nValue );
                aChildAlloc.AdjustHeight(nRowSpacing*(nHeight-1) );

                setLayoutAllocation(*pChild, aAllocPos, aChildAlloc);
            }
            aAllocPos.AdjustY(aHeights[y].m_nValue + nRowSpacing );
        }
        aAllocPos.AdjustX(aWidths[x].m_nValue + nColSpacing );
        aAllocPos.setY( 0 );
    }
}

void VclGrid::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    VclContainer::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("type", "grid");
}

bool toBool(std::u16string_view rValue)
{
    return (!rValue.empty() && (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1'));
}

bool VclGrid::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "row-spacing")
        set_row_spacing(rValue.toInt32());
    else if (rKey == "column-spacing")
        set_column_spacing(rValue.toInt32());
    else if (rKey == "row-homogeneous")
        m_bRowHomogeneous = toBool(rValue);
    else if (rKey == "column-homogeneous")
        m_bColumnHomogeneous = toBool(rValue);
    else if (rKey == "n-rows")
        /*nothing to do*/;
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

const vcl::Window *VclBin::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    return pWindowImpl->mpFirstChild;
}

vcl::Window *VclBin::get_child()
{
    return const_cast<vcl::Window*>(const_cast<const VclBin*>(this)->get_child());
}

Size VclBin::calculateRequisition() const
{
    const vcl::Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        return getLayoutRequisition(*pChild);
    return Size(0, 0);
}

void VclBin::setAllocation(const Size &rAllocation)
{
    vcl::Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        setLayoutAllocation(*pChild, Point(0, 0), rAllocation);
}

VclFrame::~VclFrame()
{
    disposeOnce();
}

void VclFrame::dispose()
{
    m_pLabel.clear();
    VclBin::dispose();
}

//To-Do, hook a DecorationView into VclFrame ?

Size VclFrame::calculateRequisition() const
{
    Size aRet(0, 0);

    const vcl::Window *pChild = get_child();
    const vcl::Window *pLabel = get_label_widget();

    if (pChild && pChild->IsVisible())
        aRet = getLayoutRequisition(*pChild);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aRet.AdjustHeight(aLabelSize.Height() );
        aRet.setWidth( std::max(aLabelSize.Width(), aRet.Width()) );
    }

    return aRet;
}

void VclFrame::setAllocation(const Size &rAllocation)
{
    //SetBackground( Color(0xFF, 0x00, 0xFF) );

    Size aAllocation(rAllocation);
    Point aChildPos;

    vcl::Window *pChild = get_child();
    vcl::Window *pLabel = get_label_widget();

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aLabelSize.setHeight( std::min(aLabelSize.Height(), aAllocation.Height()) );
        aLabelSize.setWidth( std::min(aLabelSize.Width(), aAllocation.Width()) );
        setLayoutAllocation(*pLabel, aChildPos, aLabelSize);
        aAllocation.AdjustHeight( -(aLabelSize.Height()) );
        aChildPos.AdjustY(aLabelSize.Height() );
    }

    if (pChild && pChild->IsVisible())
        setLayoutAllocation(*pChild, aChildPos, aAllocation);
}

IMPL_LINK(VclFrame, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    if (rEvent.GetId() == VclEventId::ObjectDying)
        designate_label(nullptr);
}

void VclFrame::designate_label(vcl::Window *pWindow)
{
    assert(!pWindow || pWindow->GetParent() == this);
    if (m_pLabel)
        m_pLabel->RemoveEventListener(LINK(this, VclFrame, WindowEventListener));
    m_pLabel = pWindow;
    if (m_pLabel)
        m_pLabel->AddEventListener(LINK(this, VclFrame, WindowEventListener));
}

const vcl::Window *VclFrame::get_label_widget() const
{
    assert(GetChildCount() == 2);
    if (m_pLabel)
        return m_pLabel;
    //The label widget is normally the first (of two) children
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    if (pWindowImpl->mpFirstChild == pWindowImpl->mpLastChild) //no label exists
        return nullptr;
    return pWindowImpl->mpFirstChild;
}

vcl::Window *VclFrame::get_label_widget()
{
    return const_cast<vcl::Window*>(const_cast<const VclFrame*>(this)->get_label_widget());
}

const vcl::Window *VclFrame::get_child() const
{
    //The child widget is the normally the last (of two) children
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    assert(GetChildCount() == 2 || pWindowImpl->mbInDispose);
    if (!m_pLabel)
        return pWindowImpl->mpLastChild;
    if (pWindowImpl->mpFirstChild == pWindowImpl->mpLastChild) //only label exists
        return nullptr;
    return pWindowImpl->mpLastChild;
}

vcl::Window *VclFrame::get_child()
{
    return const_cast<vcl::Window*>(const_cast<const VclFrame*>(this)->get_child());
}

void VclFrame::set_label(const OUString &rLabel)
{
    vcl::Window *pLabel = get_label_widget();
    assert(pLabel);
    pLabel->SetText(rLabel);
}

OUString VclFrame::get_label() const
{
    const vcl::Window *pLabel = get_label_widget();
    assert(pLabel);
    return pLabel->GetText();
}

OUString VclFrame::getDefaultAccessibleName() const
{
    const vcl::Window *pLabel = get_label_widget();
    if (pLabel)
        return pLabel->GetAccessibleName();
    return VclBin::getDefaultAccessibleName();
}

void VclFrame::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    VclBin::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("type", "frame");
}

Size VclAlignment::calculateRequisition() const
{
    Size aRet(m_nLeftPadding + m_nRightPadding,
        m_nTopPadding + m_nBottomPadding);

    const vcl::Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
    {
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.AdjustWidth(aChildSize.Width() );
        aRet.AdjustHeight(aChildSize.Height() );
    }

    return aRet;
}

void VclAlignment::setAllocation(const Size &rAllocation)
{
    vcl::Window *pChild = get_child();
    if (!pChild || !pChild->IsVisible())
        return;

    Point aChildPos(m_nLeftPadding, m_nTopPadding);

    Size aAllocation;
    aAllocation.setWidth( rAllocation.Width() - (m_nLeftPadding + m_nRightPadding) );
    aAllocation.setHeight( rAllocation.Height() - (m_nTopPadding + m_nBottomPadding) );

    setLayoutAllocation(*pChild, aChildPos, aAllocation);
}

bool VclAlignment::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "bottom-padding")
        m_nBottomPadding = rValue.toInt32();
    else if (rKey == "left-padding")
        m_nLeftPadding = rValue.toInt32();
    else if (rKey == "right-padding")
        m_nRightPadding = rValue.toInt32();
    else if (rKey == "top-padding")
        m_nTopPadding = rValue.toInt32();
    else
        return VclBin::set_property(rKey, rValue);
    return true;
}

class DisclosureButton final : public CheckBox
{
    virtual void ImplDrawCheckBoxState(vcl::RenderContext& rRenderContext) override
    {
        /* HACK: DisclosureButton is currently assuming, that the disclosure sign
           will fit into the rectangle occupied by a normal checkbox on all themes.
           If this does not hold true for some theme, ImplGetCheckImageSize
           would have to be overridden for DisclosureButton; also GetNativeControlRegion
           for ControlType::ListNode would have to be implemented and taken into account
        */

        tools::Rectangle aStateRect(GetStateRect());

        ImplControlValue aControlValue(GetState() == TRISTATE_TRUE ? ButtonValue::On : ButtonValue::Off);
        tools::Rectangle aCtrlRegion(aStateRect);
        ControlState nState = ControlState::NONE;

        if (HasFocus())
            nState |= ControlState::FOCUSED;
        if (GetButtonState() & DrawButtonFlags::Default)
            nState |= ControlState::DEFAULT;
        if (Window::IsEnabled())
            nState |= ControlState::ENABLED;
        if (IsMouseOver() && GetMouseRect().IsInside(GetPointerPosPixel()))
            nState |= ControlState::ROLLOVER;

        if (rRenderContext.DrawNativeControl(ControlType::ListNode, ControlPart::Entire, aCtrlRegion,
                                              nState, aControlValue, OUString()))
            return;

        ImplSVCtrlData& rCtrlData(ImplGetSVData()->maCtrlData);
        if (!rCtrlData.mpDisclosurePlus)
            rCtrlData.mpDisclosurePlus.reset(new Image(StockImage::Yes, SV_DISCLOSURE_PLUS));
        if (!rCtrlData.mpDisclosureMinus)
            rCtrlData.mpDisclosureMinus.reset(new Image(StockImage::Yes, SV_DISCLOSURE_MINUS));

        Image* pImg
            = IsChecked() ? rCtrlData.mpDisclosureMinus.get() : rCtrlData.mpDisclosurePlus.get();

        DrawImageFlags nStyle = DrawImageFlags::NONE;
        if (!IsEnabled())
            nStyle |= DrawImageFlags::Disable;

        Size aSize(aStateRect.GetSize());
        Size aImgSize(pImg->GetSizePixel());
        Point aOff((aSize.Width() - aImgSize.Width()) / 2,
                   (aSize.Height() - aImgSize.Height()) / 2);
        aOff += aStateRect.TopLeft();
        rRenderContext.DrawImage(aOff, *pImg, nStyle);
    }

public:
    explicit DisclosureButton(vcl::Window* pParent)
        : CheckBox(pParent, 0)
    {
    }

    virtual void KeyInput( const KeyEvent& rKEvt ) override
    {
        vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

        if( !aKeyCode.GetModifier()  &&
            ( ( aKeyCode.GetCode() == KEY_ADD ) ||
              ( aKeyCode.GetCode() == KEY_SUBTRACT ) )
            )
        {
            Check( aKeyCode.GetCode() == KEY_ADD );
        }
        else
            CheckBox::KeyInput( rKEvt );
    }
};

VclExpander::VclExpander(vcl::Window *pParent)
    : VclBin(pParent)
    , m_bResizeTopLevel(false)
    , m_pDisclosureButton(VclPtr<DisclosureButton>::Create(this))
{
    m_pDisclosureButton->SetToggleHdl(LINK(this, VclExpander, ClickHdl));
    m_pDisclosureButton->Show();
}

VclExpander::~VclExpander()
{
    disposeOnce();
}

bool VclExpander::get_expanded() const
{
    return m_pDisclosureButton->IsChecked();
}

void VclExpander::set_expanded(bool bExpanded)
{
    m_pDisclosureButton->Check(bExpanded);
}

void VclExpander::set_label(const OUString& rLabel)
{
    m_pDisclosureButton->SetText(rLabel);
}

OUString VclExpander::get_label() const
{
    return m_pDisclosureButton->GetText();
}

void VclExpander::dispose()
{
    m_pDisclosureButton.disposeAndClear();
    VclBin::dispose();
}

const vcl::Window *VclExpander::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    assert(pWindowImpl->mpFirstChild == m_pDisclosureButton);

    return pWindowImpl->mpFirstChild->GetWindow(GetWindowType::Next);
}

vcl::Window *VclExpander::get_child()
{
    return const_cast<vcl::Window*>(const_cast<const VclExpander*>(this)->get_child());
}

Size VclExpander::calculateRequisition() const
{
    Size aRet(0, 0);

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    const vcl::Window *pChild = get_child();
    const vcl::Window *pLabel = pChild != pWindowImpl->mpLastChild ? pWindowImpl->mpLastChild.get() : nullptr;

    if (pChild && pChild->IsVisible() && m_pDisclosureButton->IsChecked())
        aRet = getLayoutRequisition(*pChild);

    Size aExpanderSize = getLayoutRequisition(*m_pDisclosureButton);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aExpanderSize.setHeight( std::max(aExpanderSize.Height(), aLabelSize.Height()) );
        aExpanderSize.AdjustWidth(aLabelSize.Width() );
    }

    aRet.AdjustHeight(aExpanderSize.Height() );
    aRet.setWidth( std::max(aExpanderSize.Width(), aRet.Width()) );

    return aRet;
}

void VclExpander::setAllocation(const Size &rAllocation)
{
    Size aAllocation(rAllocation);
    Point aChildPos;

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    //The label widget is the last (of two) children
    vcl::Window *pChild = get_child();
    vcl::Window *pLabel = pChild != pWindowImpl->mpLastChild.get() ? pWindowImpl->mpLastChild.get() : nullptr;

    Size aButtonSize = getLayoutRequisition(*m_pDisclosureButton);
    Size aLabelSize;
    Size aExpanderSize = aButtonSize;
    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize = getLayoutRequisition(*pLabel);
        aExpanderSize.setHeight( std::max(aExpanderSize.Height(), aLabelSize.Height()) );
        aExpanderSize.AdjustWidth(aLabelSize.Width() );
    }

    aExpanderSize.setHeight( std::min(aExpanderSize.Height(), aAllocation.Height()) );
    aExpanderSize.setWidth( std::min(aExpanderSize.Width(), aAllocation.Width()) );

    aButtonSize.setHeight( std::min(aButtonSize.Height(), aExpanderSize.Height()) );
    aButtonSize.setWidth( std::min(aButtonSize.Width(), aExpanderSize.Width()) );

    tools::Long nExtraExpanderHeight = aExpanderSize.Height() - aButtonSize.Height();
    Point aButtonPos(aChildPos.X(), aChildPos.Y() + nExtraExpanderHeight/2);
    setLayoutAllocation(*m_pDisclosureButton, aButtonPos, aButtonSize);

    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize.setHeight( std::min(aLabelSize.Height(), aExpanderSize.Height()) );
        aLabelSize.setWidth( std::min(aLabelSize.Width(),
            aExpanderSize.Width() - aButtonSize.Width()) );

        tools::Long nExtraLabelHeight = aExpanderSize.Height() - aLabelSize.Height();
        Point aLabelPos(aChildPos.X() + aButtonSize.Width(), aChildPos.Y() + nExtraLabelHeight/2);
        setLayoutAllocation(*pLabel, aLabelPos, aLabelSize);
    }

    aAllocation.AdjustHeight( -(aExpanderSize.Height()) );
    aChildPos.AdjustY(aExpanderSize.Height() );

    if (pChild && pChild->IsVisible())
    {
        if (!m_pDisclosureButton->IsChecked())
            aAllocation = Size();
        setLayoutAllocation(*pChild, aChildPos, aAllocation);
    }
}

bool VclExpander::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "expanded")
        set_expanded(toBool(rValue));
    else if (rKey == "resize-toplevel")
        m_bResizeTopLevel = toBool(rValue);
    else
        return VclBin::set_property(rKey, rValue);
    return true;
}

void VclExpander::StateChanged(StateChangedType nType)
{
    VclBin::StateChanged( nType );

    if (nType == StateChangedType::InitShow)
    {
        vcl::Window *pChild = get_child();
        if (pChild)
            pChild->Show(m_pDisclosureButton->IsChecked());
    }
}

const vcl::Window *VclExpander::get_label_widget() const
{
    return m_pDisclosureButton;
}

vcl::Window *VclExpander::get_label_widget()
{
    return const_cast<vcl::Window*>(const_cast<const VclExpander*>(this)->get_label_widget());
}


IMPL_LINK( VclExpander, ClickHdl, CheckBox&, rBtn, void )
{
    vcl::Window *pChild = get_child();
    if (pChild)
    {
        pChild->Show(rBtn.IsChecked());
        queue_resize();
        Dialog* pResizeDialog = m_bResizeTopLevel ? GetParentDialog() : nullptr;
        if (pResizeDialog)
            pResizeDialog->setOptimalLayoutSize();
    }
    maExpandedHdl.Call(*this);
}

VclScrolledWindow::VclScrolledWindow(vcl::Window *pParent)
    : VclBin(pParent, WB_HIDE | WB_CLIPCHILDREN | WB_AUTOHSCROLL | WB_AUTOVSCROLL | WB_TABSTOP)
    , m_bUserManagedScrolling(false)
    , m_nBorderWidth(1)
    , m_eDrawFrameStyle(DrawFrameStyle::NONE)
    , m_eDrawFrameFlags(DrawFrameFlags::NONE)
    , m_pVScroll(VclPtr<ScrollBar>::Create(this, WB_HIDE | WB_VERT))
    , m_pHScroll(VclPtr<ScrollBar>::Create(this, WB_HIDE | WB_HORZ))
    , m_aScrollBarBox(VclPtr<ScrollBarBox>::Create(this, WB_HIDE))
{
    SetType(WindowType::SCROLLWINDOW);

    AllSettings aAllSettings = GetSettings();
    StyleSettings aStyle = aAllSettings.GetStyleSettings();
    aStyle.SetMonoColor(aStyle.GetShadowColor());
    aAllSettings.SetStyleSettings(aStyle);
    SetSettings(aAllSettings);

    Link<ScrollBar*,void> aLink( LINK( this, VclScrolledWindow, ScrollBarHdl ) );
    m_pVScroll->SetScrollHdl(aLink);
    m_pHScroll->SetScrollHdl(aLink);
}

void VclScrolledWindow::dispose()
{
    m_pVScroll.disposeAndClear();
    m_pHScroll.disposeAndClear();
    m_aScrollBarBox.disposeAndClear();
    VclBin::dispose();
}

IMPL_LINK_NOARG(VclScrolledWindow, ScrollBarHdl, ScrollBar*, void)
{
    vcl::Window *pChild = get_child();
    if (!pChild)
        return;

    assert(dynamic_cast<VclViewport*>(pChild) && "scrolledwindow child should be a Viewport");

    pChild = pChild->GetWindow(GetWindowType::FirstChild);

    if (!pChild)
        return;

    Point aWinPos(-m_pHScroll->GetThumbPos(), -m_pVScroll->GetThumbPos());
    pChild->SetPosPixel(aWinPos);
}

const vcl::Window *VclScrolledWindow::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    assert(GetChildCount() == 4 || pWindowImpl->mbInDispose);
    return pWindowImpl->mpLastChild;
}

vcl::Window *VclScrolledWindow::get_child()
{
    return const_cast<vcl::Window*>(const_cast<const VclScrolledWindow*>(this)->get_child());
}

Size VclScrolledWindow::calculateRequisition() const
{
    Size aRet(0, 0);

    const vcl::Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        aRet = getLayoutRequisition(*pChild);

    if (GetStyle() & WB_VSCROLL)
        aRet.AdjustWidth(getLayoutRequisition(*m_pVScroll).Width() );

    if (GetStyle() & WB_HSCROLL)
        aRet.AdjustHeight(getLayoutRequisition(*m_pHScroll).Height() );

    aRet.AdjustHeight(2 * m_nBorderWidth);
    aRet.AdjustWidth(2 * m_nBorderWidth);

    return aRet;
}

void VclScrolledWindow::InitScrollBars(const Size &rRequest)
{
    const vcl::Window *pChild = get_child();
    if (!pChild || !pChild->IsVisible())
        return;

    Size aOutSize(getVisibleChildSize());

    m_pVScroll->SetRangeMax(rRequest.Height());
    m_pVScroll->SetVisibleSize(aOutSize.Height());
    m_pVScroll->SetPageSize(16);

    m_pHScroll->SetRangeMax(rRequest.Width());
    m_pHScroll->SetVisibleSize(aOutSize.Width());
    m_pHScroll->SetPageSize(16);

    m_pVScroll->Scroll();
    m_pHScroll->Scroll();
}

void VclScrolledWindow::doSetAllocation(const Size &rAllocation, bool bRetryOnFailure)
{
    Size aChildReq;

    vcl::Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        aChildReq = getLayoutRequisition(*pChild);

    tools::Long nAvailHeight = rAllocation.Height() - 2 * m_nBorderWidth;
    tools::Long nAvailWidth = rAllocation.Width() - 2 * m_nBorderWidth;

    // vert. ScrollBar
    bool bShowVScroll;
    if (GetStyle() & WB_AUTOVSCROLL)
        bShowVScroll = nAvailHeight < aChildReq.Height();
    else
        bShowVScroll = (GetStyle() & WB_VSCROLL) != 0;

    if (bShowVScroll)
        nAvailWidth -= getLayoutRequisition(*m_pVScroll).Width();

    // horz. ScrollBar
    bool bShowHScroll;
    if (GetStyle() & WB_AUTOHSCROLL)
    {
        bShowHScroll = nAvailWidth < aChildReq.Width();

        if (bShowHScroll)
            nAvailHeight -= getLayoutRequisition(*m_pHScroll).Height();

        if (GetStyle() & WB_AUTOVSCROLL)
            bShowVScroll = nAvailHeight < aChildReq.Height();
    }
    else
        bShowHScroll = (GetStyle() & WB_HSCROLL) != 0;

    if (m_pHScroll->IsVisible() != bShowHScroll)
        m_pHScroll->Show(bShowHScroll);
    if (m_pVScroll->IsVisible() != bShowVScroll)
        m_pVScroll->Show(bShowVScroll);

    Size aInnerSize(rAllocation);
    aInnerSize.AdjustWidth(-2 * m_nBorderWidth);
    aInnerSize.AdjustHeight(-2 * m_nBorderWidth);

    bool bBothVisible = m_pVScroll->IsVisible() && m_pHScroll->IsVisible();
    auto nScrollBarWidth = getLayoutRequisition(*m_pVScroll).Width();
    auto nScrollBarHeight = getLayoutRequisition(*m_pHScroll).Height();

    if (m_pVScroll->IsVisible())
    {
        Point aScrollPos(rAllocation.Width() - nScrollBarWidth - m_nBorderWidth, m_nBorderWidth);
        Size aScrollSize(nScrollBarWidth, rAllocation.Height() - 2 * m_nBorderWidth);
        if (bBothVisible)
            aScrollSize.AdjustHeight(-nScrollBarHeight);
        setLayoutAllocation(*m_pVScroll, aScrollPos, aScrollSize);
        aInnerSize.AdjustWidth( -nScrollBarWidth );
    }

    if (m_pHScroll->IsVisible())
    {
        Point aScrollPos(m_nBorderWidth, rAllocation.Height() - nScrollBarHeight);
        Size aScrollSize(rAllocation.Width() - 2 * m_nBorderWidth, nScrollBarHeight);
        if (bBothVisible)
            aScrollSize.AdjustWidth(-nScrollBarWidth);
        setLayoutAllocation(*m_pHScroll, aScrollPos, aScrollSize);
        aInnerSize.AdjustHeight( -nScrollBarHeight );
    }

    if (bBothVisible)
    {
        Point aBoxPos(aInnerSize.Width() + m_nBorderWidth, aInnerSize.Height() + m_nBorderWidth);
        m_aScrollBarBox->SetPosSizePixel(aBoxPos, Size(nScrollBarWidth, nScrollBarHeight));
        m_aScrollBarBox->Show();
    }
    else
    {
        m_aScrollBarBox->Hide();
    }

    if (pChild && pChild->IsVisible())
    {
        assert(dynamic_cast<VclViewport*>(pChild) && "scrolledwindow child should be a Viewport");

        WinBits nOldBits = (GetStyle() & (WB_AUTOVSCROLL | WB_VSCROLL | WB_AUTOHSCROLL | WB_HSCROLL));

        setLayoutAllocation(*pChild, Point(m_nBorderWidth, m_nBorderWidth), aInnerSize);

        // tdf#128758 if the layout allocation triggered some callback that
        // immediately invalidates the layout by adding scrollbars then
        // normally this would simply retrigger layout and another toplevel
        // attempt is made later. But the initial layout attempt blocks
        // relayouts, so just make another single effort here.
        WinBits nNewBits = (GetStyle() & (WB_AUTOVSCROLL | WB_VSCROLL | WB_AUTOHSCROLL | WB_HSCROLL));
        if (nOldBits != nNewBits && bRetryOnFailure)
        {
            doSetAllocation(rAllocation, false);
            return;
        }
    }

    if (!m_bUserManagedScrolling)
        InitScrollBars(aChildReq);
}

void VclScrolledWindow::setAllocation(const Size &rAllocation)
{
    doSetAllocation(rAllocation, true);
}

Size VclScrolledWindow::getVisibleChildSize() const
{
    Size aRet(GetSizePixel());
    if (m_pVScroll->IsVisible())
        aRet.AdjustWidth( -(m_pVScroll->GetSizePixel().Width()) );
    if (m_pHScroll->IsVisible())
        aRet.AdjustHeight( -(m_pHScroll->GetSizePixel().Height()) );
    aRet.AdjustHeight(-2 * m_nBorderWidth);
    aRet.AdjustWidth(-2 * m_nBorderWidth);
    return aRet;
}

bool VclScrolledWindow::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "shadow-type")
    {
        // despite the style names, this looks like the best mapping
        if (rValue == "in")
            m_eDrawFrameStyle = DrawFrameStyle::Out;
        else if (rValue == "out")
            m_eDrawFrameStyle = DrawFrameStyle::In;
        else if (rValue == "etched-in")
            m_eDrawFrameStyle = DrawFrameStyle::DoubleOut;
        else if (rValue == "etched-out")
            m_eDrawFrameStyle = DrawFrameStyle::DoubleIn;
        else if (rValue == "none")
            m_eDrawFrameStyle = DrawFrameStyle::NONE;
        return true;
    }
    else if (rKey == "name")
    {
        m_eDrawFrameFlags = rValue == "monoborder" ? DrawFrameFlags::Mono : DrawFrameFlags::NONE;
    }

    bool bRet = VclBin::set_property(rKey, rValue);
    m_pVScroll->Show((GetStyle() & WB_VSCROLL) != 0);
    m_pHScroll->Show((GetStyle() & WB_HSCROLL) != 0);
    return bRet;
}

bool VclScrolledWindow::EventNotify(NotifyEvent& rNEvt)
{
    bool bDone = false;
    if ( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
        if ( rCEvt.GetCommand() == CommandEventId::Wheel )
        {
            const CommandWheelData* pData = rCEvt.GetWheelData();
            if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
            {
                // tdf#140537 only handle scroll commands in the valid shown scrollbars
                bDone = HandleScrollCommand(rCEvt,
                                            m_pHScroll->IsVisible() ? m_pHScroll : nullptr,
                                            m_pVScroll->IsVisible() ? m_pVScroll : nullptr);
            }
        }
    }

    return bDone || VclBin::EventNotify( rNEvt );
}

void VclScrolledWindow::updateBorderWidth(tools::Long nBorderWidth)
{
    if (m_nBorderWidth == nBorderWidth || nBorderWidth < 1)
        return;

    m_nBorderWidth = nBorderWidth;
    // update scrollbars and child window
    doSetAllocation(GetSizePixel(), false);
};

void VclScrolledWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    const tools::Rectangle aRect(tools::Rectangle(Point(0,0), GetSizePixel()));
    DecorationView aDecoView(&rRenderContext);
    const tools::Rectangle aContentRect = aDecoView.DrawFrame(aRect, m_eDrawFrameStyle, m_eDrawFrameFlags);

    // take potentially changed frame size into account before rendering content
    const tools::Long nFrameWidth = (aRect.GetWidth() - aContentRect.GetWidth()) / 2;
    updateBorderWidth(nFrameWidth);

    VclBin::Paint(rRenderContext, rRect);
}

void VclViewport::setAllocation(const Size &rAllocation)
{
    vcl::Window *pChild = get_child();
    if (!(pChild && pChild->IsVisible()))
        return;

    Size aReq(getLayoutRequisition(*pChild));
    aReq.setWidth( std::max(aReq.Width(), rAllocation.Width()) );
    aReq.setHeight( std::max(aReq.Height(), rAllocation.Height()) );
    Point aKeepPos(pChild->GetPosPixel());
    if (m_bInitialAllocation)
    {
        aKeepPos = Point(0, 0);
        m_bInitialAllocation = false;
    }
    setLayoutAllocation(*pChild, aKeepPos, aReq);
}

const vcl::Window *VclEventBox::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    assert(pWindowImpl->mpFirstChild.get() == m_aEventBoxHelper.get());

    return pWindowImpl->mpFirstChild->GetWindow(GetWindowType::Next);
}

vcl::Window *VclEventBox::get_child()
{
    return const_cast<vcl::Window*>(const_cast<const VclEventBox*>(this)->get_child());
}

void VclEventBox::setAllocation(const Size& rAllocation)
{
    Point aChildPos(0, 0);
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        setLayoutAllocation(*pChild, aChildPos, rAllocation);
    }
}

Size VclEventBox::calculateRequisition() const
{
    Size aRet(0, 0);

    for (const vcl::Window* pChild = get_child(); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.setWidth( std::max(aRet.Width(), aChildSize.Width()) );
        aRet.setHeight( std::max(aRet.Height(), aChildSize.Height()) );
    }

    return aRet;
}

void VclEventBox::Command(const CommandEvent&)
{
    //discard events by default to block them reaching children
}

VclEventBox::~VclEventBox()
{
    disposeOnce();
}

void VclEventBox::dispose()
{
    m_aEventBoxHelper.disposeAndClear();
    VclBin::dispose();
}

void VclSizeGroup::trigger_queue_resize()
{
    //sufficient to trigger one widget to trigger all of them
    if (!m_aWindows.empty())
    {
        (*m_aWindows.begin())->queue_resize();
    }
}

void VclSizeGroup::set_ignore_hidden(bool bIgnoreHidden)
{
    if (bIgnoreHidden != m_bIgnoreHidden)
    {
        m_bIgnoreHidden = bIgnoreHidden;
        trigger_queue_resize();
    }
}

void VclSizeGroup::set_mode(VclSizeGroupMode eMode)
{
    if (eMode != m_eMode)
    {
        m_eMode = eMode;
        trigger_queue_resize();
    }

}

void VclSizeGroup::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "ignore-hidden")
        set_ignore_hidden(toBool(rValue));
    else if (rKey == "mode")
    {
        VclSizeGroupMode eMode = VclSizeGroupMode::Horizontal;
        if (rValue == "none")
            eMode = VclSizeGroupMode::NONE;
        else if (rValue == "horizontal")
            eMode = VclSizeGroupMode::Horizontal;
        else if (rValue == "vertical")
            eMode = VclSizeGroupMode::Vertical;
        else if (rValue == "both")
            eMode = VclSizeGroupMode::Both;
        else
        {
            SAL_WARN("vcl.layout", "unknown size group mode" << rValue);
        }
        set_mode(eMode);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled property: " << rKey);
    }
}

void MessageDialog::create_message_area()
{
    setDeferredProperties();

    if (m_pGrid)
        return;

    VclContainer *pContainer = get_content_area();
    assert(pContainer);

    m_pGrid.set( VclPtr<VclGrid>::Create(pContainer) );
    m_pGrid->reorderWithinParent(0);
    m_pGrid->set_column_spacing(12);
    m_pMessageBox.set(VclPtr<VclVBox>::Create(m_pGrid));
    m_pMessageBox->set_grid_left_attach(1);
    m_pMessageBox->set_grid_top_attach(0);
    m_pMessageBox->set_spacing(GetTextHeight());

    m_pImage = VclPtr<FixedImage>::Create(m_pGrid, WB_CENTER | WB_VCENTER | WB_3DLOOK);
    switch (m_eMessageType)
    {
        case VclMessageType::Info:
            m_pImage->SetImage(GetStandardInfoBoxImage());
            break;
        case VclMessageType::Warning:
            m_pImage->SetImage(GetStandardWarningBoxImage());
            break;
        case VclMessageType::Question:
            m_pImage->SetImage(GetStandardQueryBoxImage());
            break;
        case VclMessageType::Error:
            m_pImage->SetImage(GetStandardErrorBoxImage());
            break;
        case VclMessageType::Other:
            break;
    }
    m_pImage->set_grid_left_attach(0);
    m_pImage->set_grid_top_attach(0);
    m_pImage->set_valign(VclAlign::Start);
    m_pImage->Show(m_eMessageType != VclMessageType::Other);

    WinBits nWinStyle = WB_CLIPCHILDREN | WB_LEFT | WB_VCENTER | WB_NOLABEL | WB_NOTABSTOP;

    bool bHasSecondaryText = !m_sSecondaryString.isEmpty();

    m_pPrimaryMessage = VclPtr<VclMultiLineEdit>::Create(m_pMessageBox, nWinStyle);
    m_pPrimaryMessage->SetPaintTransparent(true);
    m_pPrimaryMessage->EnableCursor(false);

    m_pPrimaryMessage->set_hexpand(true);
    m_pPrimaryMessage->SetText(m_sPrimaryString);
    m_pPrimaryMessage->Show(!m_sPrimaryString.isEmpty());

    m_pSecondaryMessage = VclPtr<VclMultiLineEdit>::Create(m_pMessageBox, nWinStyle);
    m_pSecondaryMessage->SetPaintTransparent(true);
    m_pSecondaryMessage->EnableCursor(false);
    m_pSecondaryMessage->set_hexpand(true);
    m_pSecondaryMessage->SetText(m_sSecondaryString);
    m_pSecondaryMessage->Show(bHasSecondaryText);

    MessageDialog::SetMessagesWidths(this, m_pPrimaryMessage, bHasSecondaryText ? m_pSecondaryMessage.get() : nullptr);

    VclButtonBox *pButtonBox = get_action_area();
    assert(pButtonBox);

    VclPtr<PushButton> pBtn;
    short nDefaultResponse = get_default_response();
    switch (m_eButtonsType)
    {
        case VclButtonsType::NONE:
            break;
        case VclButtonsType::Ok:
            pBtn.set( VclPtr<OKButton>::Create(pButtonBox) );
            pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
            pBtn->Show();
            pBtn->set_id("ok");
            add_button(pBtn, RET_OK, true);
            nDefaultResponse = RET_OK;
            break;
        case VclButtonsType::Close:
            pBtn.set( VclPtr<CloseButton>::Create(pButtonBox) );
            pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
            pBtn->Show();
            pBtn->set_id("close");
            add_button(pBtn, RET_CLOSE, true);
            nDefaultResponse = RET_CLOSE;
            break;
        case VclButtonsType::Cancel:
            pBtn.set( VclPtr<CancelButton>::Create(pButtonBox) );
            pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
            pBtn->Show();
            pBtn->set_id("cancel");
            add_button(pBtn, RET_CANCEL, true);
            nDefaultResponse = RET_CANCEL;
            break;
        case VclButtonsType::YesNo:
            pBtn = VclPtr<PushButton>::Create(pButtonBox);
            pBtn->SetText(GetStandardText(StandardButtonType::Yes));
            pBtn->Show();
            pBtn->set_id("yes");
            add_button(pBtn, RET_YES, true);

            pBtn.set( VclPtr<PushButton>::Create(pButtonBox) );
            pBtn->SetText(GetStandardText(StandardButtonType::No));
            pBtn->Show();
            pBtn->set_id("no");
            add_button(pBtn, RET_NO, true);
            nDefaultResponse = RET_NO;
            break;
        case VclButtonsType::OkCancel:
            pBtn.set( VclPtr<OKButton>::Create(pButtonBox) );
            pBtn->Show();
            pBtn->set_id("ok");
            add_button(pBtn, RET_OK, true);

            pBtn.set( VclPtr<CancelButton>::Create(pButtonBox) );
            pBtn->Show();
            pBtn->set_id("cancel");
            add_button(pBtn, RET_CANCEL, true);
            nDefaultResponse = RET_CANCEL;
            break;
    }
    set_default_response(nDefaultResponse);
    sort_native_button_order(*pButtonBox);
    m_pMessageBox->Show();
    m_pGrid->Show();
}

void MessageDialog::create_owned_areas()
{
#if defined _WIN32
    set_border_width(3);
#else
    set_border_width(12);
#endif
    m_pOwnedContentArea.set(VclPtr<VclVBox>::Create(this, false, 24));
    set_content_area(m_pOwnedContentArea);
    m_pOwnedContentArea->Show();
    m_pOwnedActionArea.set( VclPtr<VclHButtonBox>::Create(m_pOwnedContentArea) );
    set_action_area(m_pOwnedActionArea);
    m_pOwnedActionArea->Show();
}

MessageDialog::MessageDialog(vcl::Window* pParent, WinBits nStyle)
    : Dialog(pParent, nStyle)
    , m_eButtonsType(VclButtonsType::NONE)
    , m_eMessageType(VclMessageType::Info)
    , m_pOwnedContentArea(nullptr)
    , m_pOwnedActionArea(nullptr)
    , m_pGrid(nullptr)
    , m_pMessageBox(nullptr)
    , m_pImage(nullptr)
    , m_pPrimaryMessage(nullptr)
    , m_pSecondaryMessage(nullptr)
{
    SetType(WindowType::MESSBOX);
}

MessageDialog::MessageDialog(vcl::Window* pParent,
    const OUString &rMessage,
    VclMessageType eMessageType,
    VclButtonsType eButtonsType)
    : Dialog(pParent, WB_MOVEABLE | WB_3DLOOK | WB_CLOSEABLE)
    , m_eButtonsType(eButtonsType)
    , m_eMessageType(eMessageType)
    , m_pGrid(nullptr)
    , m_pMessageBox(nullptr)
    , m_pImage(nullptr)
    , m_pPrimaryMessage(nullptr)
    , m_pSecondaryMessage(nullptr)
    , m_sPrimaryString(rMessage)
{
    SetType(WindowType::MESSBOX);
    create_owned_areas();
    create_message_area();

    switch (m_eMessageType)
    {
        case VclMessageType::Info:
            SetText(GetStandardInfoBoxText());
            break;
        case VclMessageType::Warning:
            SetText(GetStandardWarningBoxText());
            break;
        case VclMessageType::Question:
            SetText(GetStandardQueryBoxText());
            break;
        case VclMessageType::Error:
            SetText(GetStandardErrorBoxText());
            break;
        case VclMessageType::Other:
            SetText(Application::GetDisplayName());
            break;
    }
}

void MessageDialog::dispose()
{
    disposeOwnedButtons();
    m_pPrimaryMessage.disposeAndClear();
    m_pSecondaryMessage.disposeAndClear();
    m_pImage.disposeAndClear();
    m_pMessageBox.disposeAndClear();
    m_pGrid.disposeAndClear();
    m_pOwnedActionArea.disposeAndClear();
    m_pOwnedContentArea.disposeAndClear();
    Dialog::dispose();
}

MessageDialog::~MessageDialog()
{
    disposeOnce();
}

void MessageDialog::SetMessagesWidths(vcl::Window const *pParent,
    VclMultiLineEdit *pPrimaryMessage, VclMultiLineEdit *pSecondaryMessage)
{
    if (pSecondaryMessage)
    {
        assert(pPrimaryMessage);
        vcl::Font aFont = pParent->GetSettings().GetStyleSettings().GetLabelFont();
        aFont.SetFontSize(Size(0, aFont.GetFontSize().Height() * 1.2));
        aFont.SetWeight(WEIGHT_BOLD);
        pPrimaryMessage->SetControlFont(aFont);
        pPrimaryMessage->SetMaxTextWidth(pPrimaryMessage->approximate_char_width() * 44);
        pSecondaryMessage->SetMaxTextWidth(pSecondaryMessage->approximate_char_width() * 60);
    }
    else
        pPrimaryMessage->SetMaxTextWidth(pPrimaryMessage->approximate_char_width() * 60);
}

OUString const & MessageDialog::get_primary_text() const
{
    const_cast<MessageDialog*>(this)->setDeferredProperties();

    return m_sPrimaryString;
}

OUString const & MessageDialog::get_secondary_text() const
{
    const_cast<MessageDialog*>(this)->setDeferredProperties();

    return m_sSecondaryString;
}

bool MessageDialog::set_property(const OString &rKey, const OUString &rValue)
{
    if (rKey == "text")
        set_primary_text(rValue);
    else if (rKey == "secondary-text")
        set_secondary_text(rValue);
    else if (rKey == "message-type")
    {
        VclMessageType eMode = VclMessageType::Info;
        if (rValue == "info")
            eMode = VclMessageType::Info;
        else if (rValue == "warning")
            eMode = VclMessageType::Warning;
        else if (rValue == "question")
            eMode = VclMessageType::Question;
        else if (rValue == "error")
            eMode = VclMessageType::Error;
        else if (rValue == "other")
            eMode = VclMessageType::Other;
        else
        {
            SAL_WARN("vcl.layout", "unknown message type mode" << rValue);
        }
        m_eMessageType = eMode;
    }
    else if (rKey == "buttons")
    {
        VclButtonsType eMode = VclButtonsType::NONE;
        if (rValue == "none")
            eMode = VclButtonsType::NONE;
        else if (rValue == "ok")
            eMode = VclButtonsType::Ok;
        else if (rValue == "cancel")
            eMode = VclButtonsType::Cancel;
        else if (rValue == "close")
            eMode = VclButtonsType::Close;
        else if (rValue == "yes-no")
            eMode = VclButtonsType::YesNo;
        else if (rValue == "ok-cancel")
            eMode = VclButtonsType::OkCancel;
        else
        {
            SAL_WARN("vcl.layout", "unknown buttons type mode" << rValue);
        }
        m_eButtonsType = eMode;
    }
    else
        return Dialog::set_property(rKey, rValue);
    return true;
}

void MessageDialog::set_primary_text(const OUString &rPrimaryString)
{
    m_sPrimaryString = rPrimaryString;
    if (m_pPrimaryMessage)
    {
        m_pPrimaryMessage->SetText(m_sPrimaryString);
        m_pPrimaryMessage->Show(!m_sPrimaryString.isEmpty());
        MessageDialog::SetMessagesWidths(this, m_pPrimaryMessage, !m_sSecondaryString.isEmpty() ? m_pSecondaryMessage.get() : nullptr);
    }
}

void MessageDialog::set_secondary_text(const OUString &rSecondaryString)
{
    m_sSecondaryString = rSecondaryString;
    if (m_pSecondaryMessage)
    {
        m_pSecondaryMessage->SetText("\n" + m_sSecondaryString);
        m_pSecondaryMessage->Show(!m_sSecondaryString.isEmpty());
        MessageDialog::SetMessagesWidths(this, m_pPrimaryMessage, !m_sSecondaryString.isEmpty() ? m_pSecondaryMessage.get() : nullptr);
    }
}

void MessageDialog::StateChanged(StateChangedType nType)
{
    Dialog::StateChanged(nType);
    if (nType == StateChangedType::InitShow)
    {
        // MessageBox should be at least as wide as to see the title
        auto nTitleWidth = CalcTitleWidth();
        // Extra-Width for Close button
        nTitleWidth += mpWindowImpl->mnTopBorder;
        if (get_preferred_size().Width() < nTitleWidth)
        {
            set_width_request(nTitleWidth);
            DoInitialLayout();
        }
    }
}

VclPaned::VclPaned(vcl::Window *pParent, bool bVertical)
    : VclContainer(pParent, WB_HIDE | WB_CLIPCHILDREN)
    , m_pSplitter(VclPtr<Splitter>::Create(this, bVertical ? WB_VSCROLL : WB_HSCROLL))
    , m_nPosition(-1)
{
    m_pSplitter->SetBackground(Wallpaper(Application::GetSettings().GetStyleSettings().GetFaceColor()));
    m_pSplitter->Show();
}

void VclPaned::dispose()
{
    m_pSplitter.disposeAndClear();
    VclContainer::dispose();
}

VclVPaned::VclVPaned(vcl::Window *pParent)
    : VclPaned(pParent, true)
{
    m_pSplitter->SetSplitHdl(LINK(this, VclVPaned, SplitHdl));
}

IMPL_LINK(VclVPaned, SplitHdl, Splitter*, pSplitter, void)
{
    tools::Long nSize = pSplitter->GetSplitPosPixel();
    Size aSplitterSize(m_pSplitter->GetSizePixel());
    Size aAllocation(GetSizePixel());
    arrange(aAllocation, nSize, aAllocation.Height() - nSize - aSplitterSize.Height());
}

void VclVPaned::arrange(const Size& rAllocation, tools::Long nFirstHeight, tools::Long nSecondHeight)
{
    Size aSplitterSize(rAllocation.Width(), getLayoutRequisition(*m_pSplitter).Height());
    Size aFirstChildSize(rAllocation.Width(), nFirstHeight);
    Size aSecondChildSize(rAllocation.Width(), nSecondHeight);
    int nElement = 0;
    for (vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        if (nElement == 0)
        {
            Point aSplitterPos(0, aFirstChildSize.Height());
            setLayoutAllocation(*m_pSplitter, aSplitterPos, aSplitterSize);
            m_nPosition = aSplitterPos.Y() + aSplitterSize.Height() / 2;
        }
        else if (nElement == 1)
        {
            Point aChildPos(0, 0);
            setLayoutAllocation(*pChild, aChildPos, aFirstChildSize);
        }
        else if (nElement == 2)
        {
            Point aChildPos(0, aFirstChildSize.Height() + aSplitterSize.Height());
            setLayoutAllocation(*pChild, aChildPos, aSecondChildSize);
        }
        ++nElement;
    }
}

void VclVPaned::set_position(tools::Long nPosition)
{
    VclPaned::set_position(nPosition);

    Size aAllocation(GetSizePixel());
    Size aSplitterSize(m_pSplitter->GetSizePixel());

    nPosition -= aSplitterSize.Height() / 2;

    arrange(aAllocation, nPosition, aAllocation.Height() - nPosition - aSplitterSize.Height());
}

void VclVPaned::setAllocation(const Size& rAllocation)
{
    //supporting "shrink" could be done by adjusting the allowed drag rectangle
    m_pSplitter->SetDragRectPixel(tools::Rectangle(Point(0, 0), rAllocation));
    Size aSplitterSize(rAllocation.Width(), getLayoutRequisition(*m_pSplitter).Height());
    const tools::Long nHeight = rAllocation.Height() - aSplitterSize.Height();

    tools::Long nFirstHeight = 0;
    tools::Long nSecondHeight = 0;
    bool bFirstCanResize = true;
    bool bSecondCanResize = true;
    const bool bInitialAllocation = get_position() < 0;
    int nElement = 0;
    for (const vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        if (nElement == 1)
        {
            if (bInitialAllocation)
                nFirstHeight = getLayoutRequisition(*pChild).Height();
            else
                nFirstHeight = pChild->GetSizePixel().Height();
            bFirstCanResize = pChild->get_expand();
        }
        else if (nElement == 2)
        {
            if (bInitialAllocation)
                nSecondHeight = getLayoutRequisition(*pChild).Height();
            else
                nSecondHeight = pChild->GetSizePixel().Height();
            bSecondCanResize = pChild->get_expand();
        }
        ++nElement;
    }
    tools::Long nHeightRequest = nFirstHeight + nSecondHeight;
    tools::Long nHeightDiff = nHeight - nHeightRequest;
    if (bFirstCanResize == bSecondCanResize)
        nFirstHeight += nHeightDiff/2;
    else if (bFirstCanResize)
        nFirstHeight += nHeightDiff;
    arrange(rAllocation, nFirstHeight, rAllocation.Height() - nFirstHeight - aSplitterSize.Height());
}

Size VclVPaned::calculateRequisition() const
{
    Size aRet(0, 0);

    for (const vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.setWidth( std::max(aRet.Width(), aChildSize.Width()) );
        aRet.AdjustHeight(aChildSize.Height() );
    }

    return aRet;
}

VclHPaned::VclHPaned(vcl::Window *pParent)
    : VclPaned(pParent, false)
{
    m_pSplitter->SetSplitHdl(LINK(this, VclHPaned, SplitHdl));
}

IMPL_LINK(VclHPaned, SplitHdl, Splitter*, pSplitter, void)
{
    tools::Long nSize = pSplitter->GetSplitPosPixel();
    Size aSplitterSize(m_pSplitter->GetSizePixel());
    Size aAllocation(GetSizePixel());
    arrange(aAllocation, nSize, aAllocation.Width() - nSize - aSplitterSize.Width());
}

void VclHPaned::arrange(const Size& rAllocation, tools::Long nFirstWidth, tools::Long nSecondWidth)
{
    Size aSplitterSize(getLayoutRequisition(*m_pSplitter).Width(), rAllocation.Height());
    Size aFirstChildSize(nFirstWidth, rAllocation.Height());
    Size aSecondChildSize(nSecondWidth, rAllocation.Height());
    int nElement = 0;
    for (vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        if (nElement == 0)
        {
            Point aSplitterPos(aFirstChildSize.Width(), 0);
            setLayoutAllocation(*m_pSplitter, aSplitterPos, aSplitterSize);
            m_nPosition = aSplitterPos.X() + aSplitterSize.Width() / 2;
        }
        else if (nElement == 1)
        {
            Point aChildPos(0, 0);
            setLayoutAllocation(*pChild, aChildPos, aFirstChildSize);
        }
        else if (nElement == 2)
        {
            Point aChildPos(aFirstChildSize.Width() + aSplitterSize.Width(), 0);
            setLayoutAllocation(*pChild, aChildPos, aSecondChildSize);
        }
        ++nElement;
    }
}

void VclHPaned::set_position(tools::Long nPosition)
{
    VclPaned::set_position(nPosition);

    Size aAllocation(GetSizePixel());
    Size aSplitterSize(m_pSplitter->GetSizePixel());

    nPosition -= aSplitterSize.Width() / 2;

    arrange(aAllocation, nPosition, aAllocation.Width() - nPosition - aSplitterSize.Width());
}

void VclHPaned::setAllocation(const Size& rAllocation)
{
    //supporting "shrink" could be done by adjusting the allowed drag rectangle
    m_pSplitter->SetDragRectPixel(tools::Rectangle(Point(0, 0), rAllocation));
    Size aSplitterSize(getLayoutRequisition(*m_pSplitter).Width(), rAllocation.Height());
    const tools::Long nWidth = rAllocation.Width() - aSplitterSize.Width();

    tools::Long nFirstWidth = 0;
    tools::Long nSecondWidth = 0;
    bool bFirstCanResize = true;
    bool bSecondCanResize = true;
    const bool bInitialAllocation = get_position() < 0;
    int nElement = 0;
    for (const vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        if (nElement == 1)
        {
            if (bInitialAllocation)
                nFirstWidth = getLayoutRequisition(*pChild).Width();
            else
                nFirstWidth = pChild->GetSizePixel().Width();
            bFirstCanResize = pChild->get_expand();
        }
        else if (nElement == 2)
        {
            if (bInitialAllocation)
                nSecondWidth = getLayoutRequisition(*pChild).Width();
            else
                nSecondWidth = pChild->GetSizePixel().Width();
            bSecondCanResize = pChild->get_expand();
        }
        ++nElement;
    }
    tools::Long nWidthRequest = nFirstWidth + nSecondWidth;
    tools::Long nWidthDiff = nWidth - nWidthRequest;
    if (bFirstCanResize == bSecondCanResize)
        nFirstWidth += nWidthDiff/2;
    else if (bFirstCanResize)
        nFirstWidth += nWidthDiff;
    arrange(rAllocation, nFirstWidth, rAllocation.Width() - nFirstWidth - aSplitterSize.Width());
}

Size VclHPaned::calculateRequisition() const
{
    Size aRet(0, 0);

    for (const vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.setHeight( std::max(aRet.Height(), aChildSize.Height()) );
        aRet.AdjustWidth(aChildSize.Width() );
    }

    return aRet;
}

Size getLegacyBestSizeForChildren(const vcl::Window &rWindow)
{
    tools::Rectangle aBounds;

    for (const vcl::Window* pChild = rWindow.GetWindow(GetWindowType::FirstChild); pChild;
        pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;

        tools::Rectangle aChildBounds(pChild->GetPosPixel(), pChild->GetSizePixel());
        aBounds.Union(aChildBounds);
    }

    if (aBounds.IsEmpty())
        return rWindow.GetSizePixel();

    Size aRet(aBounds.GetSize());
    Point aTopLeft(aBounds.TopLeft());
    aRet.AdjustWidth(aTopLeft.X()*2 );
    aRet.AdjustHeight(aTopLeft.Y()*2 );

    return aRet;
}

vcl::Window* getNonLayoutParent(vcl::Window *pWindow)
{
    while (pWindow)
    {
        pWindow = pWindow->GetParent();
        if (!pWindow || !isContainerWindow(*pWindow))
            break;
    }
    return pWindow;
}

bool isVisibleInLayout(const vcl::Window *pWindow)
{
    bool bVisible = true;
    while (bVisible)
    {
        bVisible = pWindow->IsVisible();
        pWindow = pWindow->GetParent();
        if (!pWindow || !isContainerWindow(*pWindow))
            break;
    }
    return bVisible;
}

bool isEnabledInLayout(const vcl::Window *pWindow)
{
    bool bEnabled = true;
    while (bEnabled)
    {
        bEnabled = pWindow->IsEnabled();
        pWindow = pWindow->GetParent();
        if (!pWindow || !isContainerWindow(*pWindow))
            break;
    }
    return bEnabled;
}

bool isLayoutEnabled(const vcl::Window *pWindow)
{
    //Child is a container => we're layout enabled
    const vcl::Window *pChild = pWindow ? pWindow->GetWindow(GetWindowType::FirstChild) : nullptr;
    return pChild && isContainerWindow(*pChild) && !pChild->GetWindow(GetWindowType::Next);
}

void VclDrawingArea::StartDrag(sal_Int8, const Point&)
{
    if (m_aStartDragHdl.Call(this))
        return;

    rtl::Reference<TransferDataContainer> xContainer = m_xTransferHelper;
    if (!m_xTransferHelper.is())
        return;

    xContainer->StartDrag(this, m_nDragAction);
}

OUString VclDrawingArea::GetSurroundingText() const
{
    if (!m_aGetSurroundingHdl.IsSet())
        return Control::GetSurroundingText();
    OUString sSurroundingText;
    m_aGetSurroundingHdl.Call(sSurroundingText);
    return sSurroundingText;
}

Selection VclDrawingArea::GetSurroundingTextSelection() const
{
    if (!m_aGetSurroundingHdl.IsSet())
        return Control::GetSurroundingTextSelection();
    OUString sSurroundingText;
    int nCursor = m_aGetSurroundingHdl.Call(sSurroundingText);
    return Selection(nCursor, nCursor);
}

bool VclDrawingArea::DeleteSurroundingText(const Selection& rSelection)
{
    if (!m_aDeleteSurroundingHdl.IsSet())
        return Control::DeleteSurroundingText(rSelection);
    return m_aDeleteSurroundingHdl.Call(rSelection);
}

VclHPaned::~VclHPaned()
{
}

VclVPaned::~VclVPaned()
{
}

VclPaned::~VclPaned()
{
    disposeOnce();
}

VclScrolledWindow::~VclScrolledWindow()
{
    disposeOnce();
}

void VclDrawingArea::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    Control::DumpAsPropertyTree(rJsonWriter);
    rJsonWriter.put("type", "drawingarea");

    ScopedVclPtrInstance<VirtualDevice> pDevice;
    pDevice->SetOutputSize( GetSizePixel() );
    tools::Rectangle aRect(Point(0,0), GetSizePixel());
    Paint(*pDevice, aRect);
    BitmapEx aImage = pDevice->GetBitmapEx( Point(0,0), GetSizePixel() );
    SvMemoryStream aOStm(65535, 65535);
    if(GraphicConverter::Export(aOStm, aImage, ConvertDataFormat::PNG) == ERRCODE_NONE)
    {
        css::uno::Sequence<sal_Int8> aSeq( static_cast<sal_Int8 const *>(aOStm.GetData()), aOStm.Tell());
        OUStringBuffer aBuffer("data:image/png;base64,");
        ::comphelper::Base64::encode(aBuffer, aSeq);
        rJsonWriter.put("image", aBuffer.makeStringAndClear());
    }
    rJsonWriter.put("text", GetQuickHelpText());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
