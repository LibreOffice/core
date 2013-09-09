/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include "window.h"

VclContainer::VclContainer(Window *pParent, WinBits nStyle)
    : Window(WINDOW_CONTAINER)
    , m_bLayoutDirty(true)
{
    ImplInit(pParent, nStyle, NULL);
    EnableChildTransparentMode();
    SetPaintTransparent(true);
    SetBackground();
}

sal_uInt16 VclContainer::getDefaultAccessibleRole() const
{
    return com::sun::star::accessibility::AccessibleRole::PANEL;
}

Size VclContainer::GetOptimalSize() const
{
    return calculateRequisition();
}

void VclContainer::setLayoutPosSize(Window &rWindow, const Point &rPos, const Size &rSize)
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

void VclContainer::setLayoutAllocation(Window &rChild, const Point &rAllocPos, const Size &rChildAlloc)
{
    VclAlign eHalign = rChild.get_halign();
    VclAlign eValign = rChild.get_valign();

    //typical case
    if (eHalign == VCL_ALIGN_FILL && eValign == VCL_ALIGN_FILL)
    {
        setLayoutPosSize(rChild, rAllocPos, rChildAlloc);
        return;
    }

    Point aChildPos(rAllocPos);
    Size aChildSize(rChildAlloc);
    Size aChildPreferredSize(getLayoutRequisition(rChild));

    switch (eHalign)
    {
        case VCL_ALIGN_FILL:
            break;
        case VCL_ALIGN_START:
            if (aChildPreferredSize.Width() < rChildAlloc.Width())
                aChildSize.Width() = aChildPreferredSize.Width();
            break;
        case VCL_ALIGN_END:
            if (aChildPreferredSize.Width() < rChildAlloc.Width())
                aChildSize.Width() = aChildPreferredSize.Width();
            aChildPos.X() += rChildAlloc.Width();
            aChildPos.X() -= aChildSize.Width();
            break;
        case VCL_ALIGN_CENTER:
            if (aChildPreferredSize.Width() < aChildSize.Width())
                aChildSize.Width() = aChildPreferredSize.Width();
            aChildPos.X() += (rChildAlloc.Width() - aChildSize.Width()) / 2;
            break;
    }

    switch (eValign)
    {
        case VCL_ALIGN_FILL:
            break;
        case VCL_ALIGN_START:
            if (aChildPreferredSize.Height() < rChildAlloc.Height())
                aChildSize.Height() = aChildPreferredSize.Height();
            break;
        case VCL_ALIGN_END:
            if (aChildPreferredSize.Height() < rChildAlloc.Height())
                aChildSize.Height() = aChildPreferredSize.Height();
            aChildPos.Y() += rChildAlloc.Height();
            aChildPos.Y() -= aChildSize.Height();
            break;
        case VCL_ALIGN_CENTER:
            if (aChildPreferredSize.Height() < aChildSize.Height())
                aChildSize.Height() = aChildPreferredSize.Height();
            aChildPos.Y() += (rChildAlloc.Height() - aChildSize.Height()) / 2;
            break;
    }

    setLayoutPosSize(rChild, aChildPos, aChildSize);
}

Size VclContainer::getLayoutRequisition(const Window &rWindow)
{
    sal_Int32 nBorderWidth = rWindow.get_border_width();
    sal_Int32 nLeft = rWindow.get_margin_left() + nBorderWidth;
    sal_Int32 nTop = rWindow.get_margin_top() + nBorderWidth;
    sal_Int32 nRight = rWindow.get_margin_right() + nBorderWidth;
    sal_Int32 nBottom = rWindow.get_margin_bottom() + nBorderWidth;
    Size aSize(rWindow.get_preferred_size());
    return Size(aSize.Width() + nLeft + nRight, aSize.Height() + nTop + nBottom);
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
    aAllocPos.X() += nBorderWidth + get_margin_left();
    aAllocPos.Y() += nBorderWidth + get_margin_top();

    if (aAllocPos != GetPosPixel())
        Window::SetPosPixel(aAllocPos);
}

void VclContainer::SetSizePixel(const Size& rAllocation)
{
    Size aAllocation = rAllocation;
    sal_Int32 nBorderWidth = get_border_width();
    aAllocation.Width() -= nBorderWidth*2 + get_margin_left() + get_margin_right();
    aAllocation.Height() -= nBorderWidth*2 + get_margin_top() + get_margin_bottom();
    bool bSizeChanged = aAllocation != GetSizePixel();
    if (bSizeChanged)
        Window::SetSizePixel(aAllocation);
    if (m_bLayoutDirty || bSizeChanged)
    {
        m_bLayoutDirty = false;
        setAllocation(aAllocation);
    }
}

void VclBox::accumulateMaxes(const Size &rChildSize, Size &rSize) const
{
    long nSecondaryChildDimension = getSecondaryDimension(rChildSize);
    long nSecondaryBoxDimension = getSecondaryDimension(rSize);
    setSecondaryDimension(rSize, std::max(nSecondaryChildDimension, nSecondaryBoxDimension));

    long nPrimaryChildDimension = getPrimaryDimension(rChildSize);
    long nPrimaryBoxDimension = getPrimaryDimension(rSize);
    if (m_bHomogeneous)
        setPrimaryDimension(rSize, std::max(nPrimaryBoxDimension, nPrimaryChildDimension));
    else
        setPrimaryDimension(rSize, nPrimaryBoxDimension + nPrimaryChildDimension);
}

Size VclBox::calculateRequisition() const
{
    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = getLayoutRequisition(*pChild);

        long nPrimaryDimension = getPrimaryDimension(aChildSize);
        nPrimaryDimension += pChild->get_padding() * 2;
        setPrimaryDimension(aChildSize, nPrimaryDimension);

        accumulateMaxes(aChildSize, aSize);
    }

    return finalizeMaxes(aSize, nVisibleChildren);
}

void VclBox::setAllocation(const Size &rAllocation)
{
    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
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

    long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);

    long nHomogeneousDimension = 0, nExtraSpace = 0;
    if (m_bHomogeneous)
    {
        nHomogeneousDimension = ((nAllocPrimaryDimension -
            (nVisibleChildren - 1) * m_nSpacing)) / nVisibleChildren;
    }
    else if (nExpandChildren)
    {
        Size aRequisition = calculateRequisition();
        nExtraSpace = (getPrimaryDimension(rAllocation) - getPrimaryDimension(aRequisition)) / nExpandChildren;
    }

    //Split into those we pack from the start onwards, and those we pack from the end backwards
    std::vector<Window*> aWindows[2];
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

        sal_Int32 ePacking = pChild->get_pack_type();
        aWindows[ePacking].push_back(pChild);
    }

    //See VclBuilder::sortIntoBestTabTraversalOrder for why they are in visual
    //order under the parent which requires us to reverse them here to
    //pack from the end back
    std::reverse(aWindows[VCL_PACK_END].begin(),aWindows[VCL_PACK_END].end());

    for (sal_Int32 ePackType = VCL_PACK_START; ePackType <= VCL_PACK_END; ++ePackType)
    {
        Point aPos(0, 0);
        if (ePackType == VCL_PACK_END)
        {
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);
            setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension);
        }

        for (std::vector<Window*>::iterator aI = aWindows[ePackType].begin(), aEnd = aWindows[ePackType].end(); aI != aEnd; ++aI)
        {
            Window *pChild = *aI;

            long nPadding = pChild->get_padding();

            Size aBoxSize;
            if (m_bHomogeneous)
                setPrimaryDimension(aBoxSize, nHomogeneousDimension);
            else
            {
                aBoxSize = getLayoutRequisition(*pChild);
                long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding * 2;
                if (getPrimaryDimensionChildExpand(*pChild))
                    nPrimaryDimension += nExtraSpace;
                setPrimaryDimension(aBoxSize, nPrimaryDimension);
            }
            setSecondaryDimension(aBoxSize, getSecondaryDimension(rAllocation));

            Point aChildPos(aPos);
            Size aChildSize(aBoxSize);
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);

            bool bFill = pChild->get_fill();
            if (bFill)
            {
                setPrimaryDimension(aChildSize, std::max(static_cast<long>(1),
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

            long nDiff = getPrimaryDimension(aBoxSize) + m_nSpacing;
            if (ePackType == VCL_PACK_START)
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

bool VclBox::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "spacing")
        set_spacing(rValue.toInt32());
    else if (rKey == "homogeneous")
        set_homogeneous(toBool(rValue));
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

sal_uInt16 VclBox::getDefaultAccessibleRole() const
{
    return com::sun::star::accessibility::AccessibleRole::FILLER;
}

#define DEFAULT_CHILD_MIN_WIDTH 85
#define DEFAULT_CHILD_MIN_HEIGHT 27

Size VclBox::finalizeMaxes(const Size &rSize, sal_uInt16 nVisibleChildren) const
{
    Size aRet;

    if (nVisibleChildren)
    {
        long nPrimaryDimension = getPrimaryDimension(rSize);
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

    long nMainGroupDimension = getPrimaryDimension(rReq.m_aMainGroupSize);
    long nSubGroupDimension = getPrimaryDimension(rReq.m_aSubGroupSize);

    setPrimaryDimension(aRet, nMainGroupDimension + nSubGroupDimension);

    setSecondaryDimension(aRet,
        std::max(getSecondaryDimension(rReq.m_aMainGroupSize),
        getSecondaryDimension(rReq.m_aSubGroupSize)));

    return aRet;
}

static long getMaxNonOutlier(const std::vector<long> &rG, long nAvgDimension)
{
    long nMaxDimensionNonOutlier = 0;
    for (std::vector<long>::const_iterator aI = rG.begin(),
        aEnd = rG.end(); aI != aEnd; ++aI)
    {
        long nPrimaryChildDimension = *aI;
        if (nPrimaryChildDimension < nAvgDimension * 1.5)
        {
            nMaxDimensionNonOutlier = std::max(nPrimaryChildDimension,
                nMaxDimensionNonOutlier);
        }
    }
    return nMaxDimensionNonOutlier;
}

static std::vector<long> setButtonSizes(const std::vector<long> &rG,
    long nAvgDimension, long nMaxNonOutlier, long nMinWidth)
{
    std::vector<long> aVec;
    //set everything < 1.5 times the average to the same width, leave the
    //outliers un-touched
    for (std::vector<long>::const_iterator aI = rG.begin(), aEnd = rG.end();
        aI != aEnd; ++aI)
    {
        long nPrimaryChildDimension = *aI;
        if (nPrimaryChildDimension < nAvgDimension * 1.5)
        {
            aVec.push_back(std::max(nMaxNonOutlier, nMinWidth));
        }
        else
        {
            aVec.push_back(std::max(nPrimaryChildDimension, nMinWidth));
        }
    }
    return aVec;
}

VclButtonBox::Requisition VclButtonBox::calculatePrimarySecondaryRequisitions() const
{
    Requisition aReq;

    Size aMainGroupSize(DEFAULT_CHILD_MIN_WIDTH, DEFAULT_CHILD_MIN_HEIGHT); //to-do, pull from theme
    Size aSubGroupSize(DEFAULT_CHILD_MIN_WIDTH, DEFAULT_CHILD_MIN_HEIGHT); //to-do, pull from theme

    long nMinMainGroupPrimary = getPrimaryDimension(aMainGroupSize);
    long nMinSubGroupPrimary = getPrimaryDimension(aSubGroupSize);
    long nMainGroupSecondary = getSecondaryDimension(aMainGroupSize);
    long nSubGroupSecondary = getSecondaryDimension(aSubGroupSize);

    bool bIgnoreSecondaryPacking = (m_eLayoutStyle == VCL_BUTTONBOX_SPREAD || m_eLayoutStyle == VCL_BUTTONBOX_CENTER);

    std::vector<long> aMainGroupSizes;
    std::vector<long> aSubGroupSizes;

    for (const Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
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
        }
        else
        {
            nSubGroupSecondary = std::max(nSubGroupSecondary, getSecondaryDimension(aChildSize));
            aSubGroupSizes.push_back(getPrimaryDimension(aChildSize));
        }
    }

    if (m_bHomogeneous)
    {
        long nMaxMainDimension = aMainGroupSizes.empty() ? 0 :
            *std::max_element(aMainGroupSizes.begin(), aMainGroupSizes.end());
        nMaxMainDimension = std::max(nMaxMainDimension, nMinMainGroupPrimary);
        long nMaxSubDimension = aSubGroupSizes.empty() ? 0 :
            *std::max_element(aSubGroupSizes.begin(), aSubGroupSizes.end());
        nMaxSubDimension = std::max(nMaxSubDimension, nMinSubGroupPrimary);
        long nMaxDimension = std::max(nMaxMainDimension, nMaxSubDimension);
        aReq.m_aMainGroupDimensions.resize(aMainGroupSizes.size(), nMaxDimension);
        aReq.m_aSubGroupDimensions.resize(aSubGroupSizes.size(), nMaxDimension);
    }
    else
    {
        //Ideally set everything to the same size, but find outlier widgets
        //that are way wider than the average and leave them
        //at their natural size and set the remainder to share the
        //max size of the remaining members of the buttonbox
        long nAccDimension = std::accumulate(aMainGroupSizes.begin(),
            aMainGroupSizes.end(), 0);
        nAccDimension = std::accumulate(aSubGroupSizes.begin(),
            aSubGroupSizes.end(), nAccDimension);

        size_t nTotalSize = aMainGroupSizes.size() + aSubGroupSizes.size();

        long nAvgDimension = nTotalSize ? nAccDimension / nTotalSize : 0;

        long nMaxMainNonOutlier = getMaxNonOutlier(aMainGroupSizes,
            nAvgDimension);
        long nMaxSubNonOutlier = getMaxNonOutlier(aSubGroupSizes,
            nAvgDimension);
        long nMaxNonOutlier = std::max(nMaxMainNonOutlier, nMaxSubNonOutlier);

        aReq.m_aMainGroupDimensions = setButtonSizes(aMainGroupSizes,
            nAvgDimension, nMaxNonOutlier, nMinMainGroupPrimary);
        aReq.m_aSubGroupDimensions = setButtonSizes(aSubGroupSizes,
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
        long nPrimaryDimension = getPrimaryDimension(rSize);
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

bool VclButtonBox::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "layout-style")
    {
        VclButtonBoxStyle eStyle = VCL_BUTTONBOX_DEFAULT_STYLE;
        if (rValue == "spread")
            eStyle = VCL_BUTTONBOX_SPREAD;
        else if (rValue == "edge")
            eStyle = VCL_BUTTONBOX_EDGE;
        else if (rValue == "start")
            eStyle = VCL_BUTTONBOX_START;
        else if (rValue == "end")
            eStyle = VCL_BUTTONBOX_END;
        else if (rValue == "center")
            eStyle = VCL_BUTTONBOX_CENTER;
        else
        {
            SAL_WARN("vcl.layout", "unknown layout style " << rValue.getStr());
        }
        set_layout(eStyle);
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

    long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);

    Point aMainGroupPos, aOtherGroupPos;
    int nSpacing = m_nSpacing;

    //To-Do, other layout styles
    switch (m_eLayoutStyle)
    {
        case VCL_BUTTONBOX_START:
            if (!aReq.m_aSubGroupDimensions.empty())
            {
                long nOtherPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aSubGroupSize, aReq.m_aSubGroupDimensions.size()));
                setPrimaryCoordinate(aOtherGroupPos,
                    nAllocPrimaryDimension - nOtherPrimaryDimension);
            }
            break;
        case VCL_BUTTONBOX_SPREAD:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                long nExtraSpace = nAllocPrimaryDimension - nMainPrimaryDimension;
                nExtraSpace += (aReq.m_aMainGroupDimensions.size()-1) * nSpacing;
                nSpacing = nExtraSpace/(aReq.m_aMainGroupDimensions.size()+1);
                setPrimaryCoordinate(aMainGroupPos, nSpacing);
            }
            break;
        case VCL_BUTTONBOX_CENTER:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                long nExtraSpace = nAllocPrimaryDimension - nMainPrimaryDimension;
                setPrimaryCoordinate(aMainGroupPos, nExtraSpace/2);
            }
            break;
        default:
            SAL_WARN("vcl.layout", "todo unimplemented layout style");
        case VCL_BUTTONBOX_DEFAULT_STYLE:
        case VCL_BUTTONBOX_END:
            if (!aReq.m_aMainGroupDimensions.empty())
            {
                long nMainPrimaryDimension = getPrimaryDimension(
                    addSpacing(aReq.m_aMainGroupSize, aReq.m_aMainGroupDimensions.size()));
                setPrimaryCoordinate(aMainGroupPos,
                    nAllocPrimaryDimension - nMainPrimaryDimension);
            }
            break;
    }

    Size aChildSize;
    setSecondaryDimension(aChildSize, getSecondaryDimension(rAllocation));

    std::vector<long>::const_iterator aPrimaryI = aReq.m_aMainGroupDimensions.begin();
    std::vector<long>::const_iterator aSecondaryI = aReq.m_aSubGroupDimensions.begin();
    bool bIgnoreSecondaryPacking = (m_eLayoutStyle == VCL_BUTTONBOX_SPREAD || m_eLayoutStyle == VCL_BUTTONBOX_CENTER);
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

        if (bIgnoreSecondaryPacking || !pChild->get_secondary())
        {
            long nMainGroupPrimaryDimension = *aPrimaryI++;
            setPrimaryDimension(aChildSize, nMainGroupPrimaryDimension);
            setLayoutAllocation(*pChild, aMainGroupPos, aChildSize);
            long nPrimaryCoordinate = getPrimaryCoordinate(aMainGroupPos);
            setPrimaryCoordinate(aMainGroupPos, nPrimaryCoordinate + nMainGroupPrimaryDimension + nSpacing);
        }
        else
        {
            long nSubGroupPrimaryDimension = *aSecondaryI++;
            setPrimaryDimension(aChildSize, nSubGroupPrimaryDimension);
            setLayoutAllocation(*pChild, aOtherGroupPos, aChildSize);
            long nPrimaryCoordinate = getPrimaryCoordinate(aOtherGroupPos);
            setPrimaryCoordinate(aOtherGroupPos, nPrimaryCoordinate + nSubGroupPrimaryDimension + nSpacing);
        }
    }
}

struct ButtonOrder
{
    OString m_aType;
    int m_nPriority;
};

static int getButtonPriority(const OString &rType)
{
    static const size_t N_TYPES = 3;
    static const ButtonOrder aDiscardCancelSave[N_TYPES] =
    {
        { "/discard", 0 },
        { "/cancel", 1 },
        { "/save", 2 }
    };

    static const ButtonOrder aSaveDiscardCancel[N_TYPES] =
    {
        { "/save", 0 },
        { "/discard", 1 },
        { "/cancel", 2 }
    };

    const ButtonOrder* pOrder = &aDiscardCancelSave[0];

    const OUString &rEnv = Application::GetDesktopEnvironment();

    if (rEnv.equalsIgnoreAsciiCase("windows") ||
        rEnv.equalsIgnoreAsciiCase("kde4") ||
        rEnv.equalsIgnoreAsciiCase("tde") ||
        rEnv.equalsIgnoreAsciiCase("kde"))
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

class sortButtons
    : public std::binary_function<const Window*, const Window*, bool>
{
    bool m_bVerticalContainer;
public:
    sortButtons(bool bVerticalContainer)
        : m_bVerticalContainer(bVerticalContainer)
    {
    }
    bool operator()(const Window *pA, const Window *pB) const;
};

bool sortButtons::operator()(const Window *pA, const Window *pB) const
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

void VclButtonBox::sort_native_button_order()
{
    std::vector<Window*> aChilds;
    for (Window* pChild = GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        aChilds.push_back(pChild);
    }

    //sort child order within parent so that we match the platform
    //button order
    std::stable_sort(aChilds.begin(), aChilds.end(), sortButtons(m_bVerticalContainer));
    VclBuilder::reorderWithinParent(aChilds, true);
}

VclGrid::array_type VclGrid::assembleGrid() const
{
    ext_array_type A;

    for (Window* pChild = GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        sal_Int32 nLeftAttach = pChild->get_grid_left_attach();
        sal_Int32 nWidth = pChild->get_grid_width();
        sal_Int32 nMaxXPos = nLeftAttach+nWidth-1;

        sal_Int32 nTopAttach = pChild->get_grid_top_attach();
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

        ExtendedGridEntry &rEntry = A[nLeftAttach][nTopAttach];
        rEntry.pChild = pChild;
        rEntry.nSpanWidth = nWidth;
        rEntry.nSpanHeight = nHeight;
        rEntry.x = nLeftAttach;
        rEntry.y = nTopAttach;

        for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
        {
            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
            {
                ExtendedGridEntry &rSpan = A[nLeftAttach+nSpanX][nTopAttach+nSpanY];
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
            const Window *pChild = rEntry.pChild;
            if (pChild && pChild->IsVisible())
            {
                aNonEmptyCols[x] = true;
                if (get_column_homogeneous())
                {
                    for (sal_Int32 nSpanX = 1; nSpanX < rEntry.nSpanWidth; ++nSpanX)
                        aNonEmptyCols[x+1] = true;
                }
                aNonEmptyRows[y] = true;
                if (get_row_homogeneous())
                {
                    for (sal_Int32 nSpanY = 1; nSpanY < rEntry.nSpanHeight; ++nSpanY)
                        aNonEmptyRows[x+1] = true;
                }
            }
        }
    }

    if (!get_column_homogeneous())
    {
        //reduce the spans of elements that span empty columns
        for (sal_Int32 x = 0; x < nMaxX; ++x)
        {
            std::set<ExtendedGridEntry*> candidates;
            for (sal_Int32 y = 0; y < nMaxY; ++y)
            {
                if (aNonEmptyCols[x])
                    continue;
                ExtendedGridEntry &rSpan = A[x][y];
                //cell x/y is spanned by the widget at cell rSpan.x/rSpan.y,
                //just points back to itself if there's no cell spanning
                if ((rSpan.x == -1) || (rSpan.y == -1))
                {
                    //there is no entry for this cell, i.e. this is a cell
                    //with no widget in it, or spanned by any other widget
                    continue;
                }
                ExtendedGridEntry &rEntry = A[rSpan.x][rSpan.y];
                candidates.insert(&rEntry);
            }
            for (std::set<ExtendedGridEntry*>::iterator aI = candidates.begin(), aEnd = candidates.end();
                aI != aEnd; ++aI)
            {
                ExtendedGridEntry *pEntry = *aI;
                --pEntry->nSpanWidth;
            }
        }
    }

    if (!get_row_homogeneous())
    {
        //reduce the spans of elements that span empty rows
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            std::set<ExtendedGridEntry*> candidates;
            for (sal_Int32 x = 0; x < nMaxX; ++x)
            {
                if (aNonEmptyRows[y])
                    continue;
                ExtendedGridEntry &rSpan = A[x][y];
                //cell x/y is spanned by the widget at cell rSpan.x/rSpan.y,
                //just points back to itself if there's no cell spanning
                if ((rSpan.x == -1) || (rSpan.y == -1))
                {
                    //there is no entry for this cell, i.e. this is a cell
                    //with no widget in it, or spanned by any other widget
                    continue;
                }
                ExtendedGridEntry &rEntry = A[rSpan.x][rSpan.y];
                candidates.insert(&rEntry);
            }
            for (std::set<ExtendedGridEntry*>::iterator aI = candidates.begin(), aEnd = candidates.end();
                aI != aEnd; ++aI)
            {
                ExtendedGridEntry *pEntry = *aI;
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
        if (aNonEmptyCols[x] == false)
            continue;
        for (sal_Int32 y = 0, y2 = 0; y < nMaxY; ++y)
        {
            if (aNonEmptyRows[y] == false)
                continue;
            GridEntry &rEntry = A[x][y];
            B[x2][y2++] = rEntry;
        }
        ++x2;
    }

    return B;
}

bool VclGrid::isNullGrid(const array_type &A) const
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    if (!nMaxX || !nMaxY)
        return true;
    return false;
}

void VclGrid::calcMaxs(const array_type &A, std::vector<Value> &rWidths, std::vector<Value> &rHeights) const
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
            const Window *pChild = rEntry.pChild;
            if (!pChild || !pChild->IsVisible())
                continue;

            sal_Int32 nWidth = rEntry.nSpanWidth;
            sal_Int32 nHeight = rEntry.nSpanHeight;

            for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                rWidths[x+nSpanX].m_bExpand = rWidths[x+nSpanX].m_bExpand | pChild->get_hexpand();

            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                rHeights[y+nSpanY].m_bExpand = rHeights[y+nSpanY].m_bExpand | pChild->get_vexpand();

            if (nWidth == 1 || nHeight == 1)
            {
                Size aChildSize = getLayoutRequisition(*pChild);
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
            const Window *pChild = rEntry.pChild;
            if (!pChild || !pChild->IsVisible())
                continue;

            sal_Int32 nWidth = rEntry.nSpanWidth;
            sal_Int32 nHeight = rEntry.nSpanHeight;

            if (nWidth == 1 && nHeight == 1)
                continue;

            Size aChildSize = getLayoutRequisition(*pChild);

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

bool compareValues(const VclGrid::Value &i, const VclGrid::Value &j)
{
    return i.m_nValue < j.m_nValue;
}

VclGrid::Value accumulateValues(const VclGrid::Value &i, const VclGrid::Value &j)
{
    VclGrid::Value aRet;
    aRet.m_nValue = i.m_nValue + j.m_nValue;
    aRet.m_bExpand = i.m_bExpand | j.m_bExpand;
    return aRet;
}

Size VclGrid::calculateRequisition() const
{
    return calculateRequisitionForSpacings(get_row_spacing(), get_column_spacing());
}

Size VclGrid::calculateRequisitionForSpacings(sal_Int32 nRowSpacing, sal_Int32 nColSpacing) const
{
    array_type A = assembleGrid();

    if (isNullGrid(A))
        return Size();

    std::vector<Value> aWidths;
    std::vector<Value> aHeights;
    calcMaxs(A, aWidths, aHeights);

    long nTotalWidth = 0;
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

    long nTotalHeight = 0;
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
    array_type A = assembleGrid();

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

    long nAvailableWidth = rAllocation.Width();
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
        long nExtraWidthForExpanders = nExpandables ? (rAllocation.Width() - aRequisition.Width()) / nExpandables : 0;

        //We don't fit and there is no volunteer to be shrunk
        if (!nExpandables && rAllocation.Width() < aRequisition.Width())
        {
            //first reduce spacing, to a min of 3
            while (nColSpacing >= 6)
            {
                nColSpacing /= 2;
                aRequisition = calculateRequisitionForSpacings(nRowSpacing, nColSpacing);
                if (aRequisition.Width() >= rAllocation.Width())
                    break;
            }

            //share out the remaining pain to everyone
            long nExtraWidth = (rAllocation.Width() - aRequisition.Width()) / nMaxX;

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

    long nAvailableHeight = rAllocation.Height();
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
        long nExtraHeightForExpanders = nExpandables ? (rAllocation.Height() - aRequisition.Height()) / nExpandables : 0;

        //We don't fit and there is no volunteer to be shrunk
        if (!nExpandables && rAllocation.Height() < aRequisition.Height())
        {
            //first reduce spacing, to a min of 3
            while (nRowSpacing >= 6)
            {
                nRowSpacing /= 2;
                aRequisition = calculateRequisitionForSpacings(nRowSpacing, nColSpacing);
                if (aRequisition.Height() >= rAllocation.Height())
                    break;
            }

            //share out the remaining pain to everyone
            long nExtraHeight = (rAllocation.Height() - aRequisition.Height()) / nMaxY;

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
            Window *pChild = rEntry.pChild;
            if (pChild)
            {
                Size aChildAlloc(0, 0);

                sal_Int32 nWidth = rEntry.nSpanWidth;
                for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    aChildAlloc.Width() += aWidths[x+nSpanX].m_nValue;
                aChildAlloc.Width() += nColSpacing*(nWidth-1);

                sal_Int32 nHeight = rEntry.nSpanHeight;
                for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    aChildAlloc.Height() += aHeights[y+nSpanY].m_nValue;
                aChildAlloc.Height() += nRowSpacing*(nHeight-1);

                setLayoutAllocation(*pChild, aAllocPos, aChildAlloc);
            }
            aAllocPos.Y() += aHeights[y].m_nValue + nRowSpacing;
        }
        aAllocPos.X() += aWidths[x].m_nValue + nColSpacing;
        aAllocPos.Y() = 0;
    }
}

bool toBool(const OString &rValue)
{
    return (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1');
}

bool VclGrid::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "row-spacing")
        set_row_spacing(rValue.toInt32());
    else if (rKey == "column-spacing")
        set_column_spacing(rValue.toInt32());
    else if (rKey == "row-homogeneous")
        set_row_homogeneous(toBool(rValue));
    else if (rKey == "column-homogeneous")
        set_column_homogeneous(toBool(rValue));
    else if (rKey == "n-rows")
        /*nothing to do*/;
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

void setGridAttach(Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nWidth, sal_Int32 nHeight)
{
    rWidget.set_grid_left_attach(nLeft);
    rWidget.set_grid_top_attach(nTop);
    rWidget.set_grid_width(nWidth);
    rWidget.set_grid_height(nHeight);
}

const Window *VclBin::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    return pWindowImpl->mpFirstChild;
}

Window *VclBin::get_child()
{
    return const_cast<Window*>(const_cast<const VclBin*>(this)->get_child());
}

Size VclBin::calculateRequisition() const
{
    const Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        return getLayoutRequisition(*pChild);
    return Size(0, 0);
}

void VclBin::setAllocation(const Size &rAllocation)
{
    Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        setLayoutAllocation(*pChild, Point(0, 0), rAllocation);
}

//To-Do, hook a DecorationView into VclFrame ?

Size VclFrame::calculateRequisition() const
{
    Size aRet(0, 0);

    const Window *pChild = get_child();
    const Window *pLabel = get_label_widget();

    if (pChild && pChild->IsVisible())
        aRet = getLayoutRequisition(*pChild);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aRet.Height() += aLabelSize.Height();
        aRet.Width() = std::max(aLabelSize.Width(), aRet.Width());
    }

    const FrameStyle &rFrameStyle =
        GetSettings().GetStyleSettings().GetFrameStyle();
    aRet.Width() += rFrameStyle.left + rFrameStyle.right;
    aRet.Height() += rFrameStyle.top + rFrameStyle.bottom;

    return aRet;
}

void VclFrame::setAllocation(const Size &rAllocation)
{
    //SetBackground( Color(0xFF, 0x00, 0xFF) );

    const FrameStyle &rFrameStyle =
        GetSettings().GetStyleSettings().GetFrameStyle();
    Size aAllocation(rAllocation.Width() - rFrameStyle.left - rFrameStyle.right,
        rAllocation.Height() - rFrameStyle.top - rFrameStyle.bottom);
    Point aChildPos(rFrameStyle.left, rFrameStyle.top);

    Window *pChild = get_child();
    Window *pLabel = get_label_widget();

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aLabelSize.Height() = std::min(aLabelSize.Height(), aAllocation.Height());
        aLabelSize.Width() = std::min(aLabelSize.Width(), aAllocation.Width());
        setLayoutAllocation(*pLabel, aChildPos, aLabelSize);
        aAllocation.Height() -= aLabelSize.Height();
        aChildPos.Y() += aLabelSize.Height();
    }

    if (pChild && pChild->IsVisible())
        setLayoutAllocation(*pChild, aChildPos, aAllocation);
}

void VclFrame::designate_label(Window *pWindow)
{
    assert(pWindow->GetParent() == this);
    m_pLabel = pWindow;
}

const Window *VclFrame::get_label_widget() const
{
    assert(GetChildCount() == 2);
    if (m_pLabel)
        return m_pLabel;
    //The label widget is normally the first (of two) children
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    if (pWindowImpl->mpFirstChild == pWindowImpl->mpLastChild) //no label exists
        return NULL;
    return pWindowImpl->mpFirstChild;
}

Window *VclFrame::get_label_widget()
{
    return const_cast<Window*>(const_cast<const VclFrame*>(this)->get_label_widget());
}

const Window *VclFrame::get_child() const
{
    assert(GetChildCount() == 2);
    //The child widget is the normally the last (of two) children
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    if (!m_pLabel)
        return pWindowImpl->mpLastChild;
    if (pWindowImpl->mpFirstChild == pWindowImpl->mpLastChild) //only label exists
        return NULL;
    return pWindowImpl->mpLastChild;
}

Window *VclFrame::get_child()
{
    return const_cast<Window*>(const_cast<const VclFrame*>(this)->get_child());
}

void VclFrame::set_label(const OUString &rLabel)
{
    Window *pLabel = get_label_widget();
    assert(pLabel);
    pLabel->SetText(rLabel);
}

OUString VclFrame::get_label() const
{
    const Window *pLabel = get_label_widget();
    assert(pLabel);
    return pLabel->GetText();
}

OUString VclFrame::getDefaultAccessibleName() const
{
    const Window *pLabel = get_label_widget();
    if (pLabel)
        return pLabel->GetAccessibleName();
    return VclBin::getDefaultAccessibleName();
}

Size VclAlignment::calculateRequisition() const
{
    Size aRet(m_nLeftPadding + m_nRightPadding,
        m_nTopPadding + m_nBottomPadding);

    const Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
    {
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.Width() += aChildSize.Width();
        aRet.Height() += aChildSize.Height();
    }

    return aRet;
}

void VclAlignment::setAllocation(const Size &rAllocation)
{
    Window *pChild = get_child();
    if (!pChild || !pChild->IsVisible())
        return;

    Point aChildPos(m_nLeftPadding, m_nTopPadding);

    Size aAllocation;
    aAllocation.Width() = rAllocation.Width() - (m_nLeftPadding + m_nRightPadding);
    aAllocation.Height() = rAllocation.Height() - (m_nTopPadding + m_nBottomPadding);

    setLayoutAllocation(*pChild, aChildPos, aAllocation);
}

bool VclAlignment::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "bottom-padding")
        m_nBottomPadding = rValue.toInt32();
    else if (rKey == "left-padding")
        m_nLeftPadding = rValue.toInt32();
    else if (rKey == "right-padding")
        m_nRightPadding = rValue.toInt32();
    else if (rKey == "top-padding")
        m_nTopPadding = rValue.toInt32();
    else if (rKey == "xalign")
        m_fXAlign = rValue.toFloat();
    else if (rKey == "xscale")
        m_fXScale = rValue.toFloat();
    else if (rKey == "yalign")
        m_fYAlign = rValue.toFloat();
    else if (rKey == "yscale")
        m_fYScale = rValue.toFloat();
    else
        return VclBin::set_property(rKey, rValue);
    return true;
}

const Window *VclExpander::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    assert(pWindowImpl->mpFirstChild == &m_aDisclosureButton);

    return pWindowImpl->mpFirstChild->GetWindow(WINDOW_NEXT);
}

Window *VclExpander::get_child()
{
    return const_cast<Window*>(const_cast<const VclExpander*>(this)->get_child());
}

Size VclExpander::calculateRequisition() const
{
    Size aRet(0, 0);

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    const Window *pChild = get_child();
    const Window *pLabel = pChild != pWindowImpl->mpLastChild ? pWindowImpl->mpLastChild : NULL;

    if (pChild && pChild->IsVisible() && m_aDisclosureButton.IsChecked())
        aRet = getLayoutRequisition(*pChild);

    Size aExpanderSize = getLayoutRequisition(m_aDisclosureButton);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = getLayoutRequisition(*pLabel);
        aExpanderSize.Height() = std::max(aExpanderSize.Height(), aLabelSize.Height());
        aExpanderSize.Width() += aLabelSize.Width();
    }

    aRet.Height() += aExpanderSize.Height();
    aRet.Width() = std::max(aExpanderSize.Width(), aRet.Width());

    const FrameStyle &rFrameStyle =
        GetSettings().GetStyleSettings().GetFrameStyle();
    aRet.Width() += rFrameStyle.left + rFrameStyle.right;
    aRet.Height() += rFrameStyle.top + rFrameStyle.bottom;

    return aRet;
}

void VclExpander::setAllocation(const Size &rAllocation)
{
    const FrameStyle &rFrameStyle =
        GetSettings().GetStyleSettings().GetFrameStyle();
    Size aAllocation(rAllocation.Width() - rFrameStyle.left - rFrameStyle.right,
        rAllocation.Height() - rFrameStyle.top - rFrameStyle.bottom);
    Point aChildPos(rFrameStyle.left, rFrameStyle.top);

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    //The label widget is the last (of two) children
    Window *pChild = get_child();
    Window *pLabel = pChild != pWindowImpl->mpLastChild ? pWindowImpl->mpLastChild : NULL;

    Size aButtonSize = getLayoutRequisition(m_aDisclosureButton);
    Size aLabelSize;
    Size aExpanderSize = aButtonSize;
    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize = getLayoutRequisition(*pLabel);
        aExpanderSize.Height() = std::max(aExpanderSize.Height(), aLabelSize.Height());
        aExpanderSize.Width() += aLabelSize.Width();
    }

    aExpanderSize.Height() = std::min(aExpanderSize.Height(), aAllocation.Height());
    aExpanderSize.Width() = std::min(aExpanderSize.Width(), aAllocation.Width());

    aButtonSize.Height() = std::min(aButtonSize.Height(), aExpanderSize.Height());
    aButtonSize.Width() = std::min(aButtonSize.Width(), aExpanderSize.Width());

    long nExtraExpanderHeight = aExpanderSize.Height() - aButtonSize.Height();
    Point aButtonPos(aChildPos.X(), aChildPos.Y() + nExtraExpanderHeight/2);
    setLayoutAllocation(m_aDisclosureButton, aButtonPos, aButtonSize);

    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize.Height() = std::min(aLabelSize.Height(), aExpanderSize.Height());
        aLabelSize.Width() = std::min(aLabelSize.Width(),
            aExpanderSize.Width() - aButtonSize.Width());

        long nExtraLabelHeight = aExpanderSize.Height() - aLabelSize.Height();
        Point aLabelPos(aChildPos.X() + aButtonSize.Width(), aChildPos.Y() + nExtraLabelHeight/2);
        setLayoutAllocation(*pLabel, aLabelPos, aLabelSize);
    }

    aAllocation.Height() -= aExpanderSize.Height();
    aChildPos.Y() += aExpanderSize.Height();

    if (pChild && pChild->IsVisible())
    {
        if (!m_aDisclosureButton.IsChecked())
            aAllocation = Size();
        setLayoutAllocation(*pChild, aChildPos, aAllocation);
    }
}

bool VclExpander::set_property(const OString &rKey, const OString &rValue)
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

    if (nType == STATE_CHANGE_INITSHOW)
    {
        Window *pChild = get_child();
        if (pChild)
            pChild->Show(m_aDisclosureButton.IsChecked());
    }
}

IMPL_LINK( VclExpander, ClickHdl, DisclosureButton*, pBtn )
{
    Window *pChild = get_child();
    if (pChild)
    {
        pChild->Show(pBtn->IsChecked());
        queue_resize();
        Dialog* pResizeDialog = m_bResizeTopLevel ? GetParentDialog() : NULL;
        if (pResizeDialog)
            pResizeDialog->setOptimalLayoutSize();
    }
    maExpandedHdl.Call(this);
    return 0;
}

const Window *VclScrolledWindow::get_child() const
{
    assert(GetChildCount() == 3);
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();
    return pWindowImpl->mpLastChild;
}

Window *VclScrolledWindow::get_child()
{
    return const_cast<Window*>(const_cast<const VclScrolledWindow*>(this)->get_child());
}

Size VclScrolledWindow::calculateRequisition() const
{
    Size aRet(0, 0);

    const Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        aRet = getLayoutRequisition(*pChild);

    if (m_aVScroll.IsVisible())
        aRet.Width() += getLayoutRequisition(m_aVScroll).Width();

    if (m_aHScroll.IsVisible())
        aRet.Height() += getLayoutRequisition(m_aHScroll).Height();

    return aRet;
}

void VclScrolledWindow::setAllocation(const Size &rAllocation)
{
    Size aChildAllocation(rAllocation);
    Size aChildReq;

    Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        aChildReq = getLayoutRequisition(*pChild);

    if (m_aVScroll.IsVisible())
    {
        long nScrollBarWidth = getLayoutRequisition(m_aVScroll).Width();
        Point aScrollPos(rAllocation.Width() - nScrollBarWidth, 0);
        Size aScrollSize(nScrollBarWidth, rAllocation.Height());
        setLayoutAllocation(m_aVScroll, aScrollPos, aScrollSize);
        aChildAllocation.Width() -= nScrollBarWidth;
        aChildAllocation.Height() = aChildReq.Height();
    }

    if (m_aHScroll.IsVisible())
    {
        long nScrollBarHeight = getLayoutRequisition(m_aHScroll).Height();
        Point aScrollPos(0, rAllocation.Height() - nScrollBarHeight);
        Size aScrollSize(rAllocation.Width(), nScrollBarHeight);
        setLayoutAllocation(m_aHScroll, aScrollPos, aScrollSize);
        aChildAllocation.Height() -= nScrollBarHeight;
        aChildAllocation.Width() = aChildReq.Width();
    }

    if (pChild && pChild->IsVisible())
    {
        Point aChildPos(pChild->GetPosPixel());
        if (!m_aHScroll.IsVisible())
            aChildPos.X() = 0;
        if (!m_aVScroll.IsVisible())
            aChildPos.Y() = 0;
        setLayoutAllocation(*pChild, aChildPos, aChildAllocation);
    }
}

Size VclScrolledWindow::getVisibleChildSize() const
{
    Size aRet(GetSizePixel());
    if (m_aVScroll.IsVisible())
        aRet.Width() -= m_aVScroll.GetSizePixel().Width();
    if (m_aHScroll.IsVisible())
        aRet.Height() -= m_aHScroll.GetSizePixel().Height();
    return aRet;
}

bool VclScrolledWindow::set_property(const OString &rKey, const OString &rValue)
{
    bool bRet = VclBin::set_property(rKey, rValue);
    m_aVScroll.Show((GetStyle() & WB_VSCROLL) != 0);
    m_aHScroll.Show((GetStyle() & WB_HSCROLL) != 0);
    return bRet;
}

const Window *VclEventBox::get_child() const
{
    const WindowImpl* pWindowImpl = ImplGetWindowImpl();

    assert(pWindowImpl->mpFirstChild == &m_aEventBoxHelper);

    return pWindowImpl->mpFirstChild->GetWindow(WINDOW_NEXT);
}

Window *VclEventBox::get_child()
{
    return const_cast<Window*>(const_cast<const VclEventBox*>(this)->get_child());
}

void VclEventBox::setAllocation(const Size& rAllocation)
{
    Point aChildPos(0, 0);
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        setLayoutAllocation(*pChild, aChildPos, rAllocation);
    }
}

Size VclEventBox::calculateRequisition() const
{
    Size aRet(0, 0);

    for (const Window* pChild = get_child(); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        Size aChildSize = getLayoutRequisition(*pChild);
        aRet.Width() = std::max(aRet.Width(), aChildSize.Width());
        aRet.Height() = std::max(aRet.Height(), aChildSize.Height());
    }

    return aRet;
}

void VclEventBox::Command(const CommandEvent&)
{
    //discard events by default to block them reaching children
}

void VclSizeGroup::trigger_queue_resize()
{
    //sufficient to trigger one widget to trigger all of them
    if (!m_aWindows.empty())
    {
        Window *pWindow = *m_aWindows.begin();
        pWindow->queue_resize();
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

bool VclSizeGroup::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "ignore-hidden")
        set_ignore_hidden(toBool(rValue));
    else if (rKey == "mode")
    {
        VclSizeGroupMode eMode = VCL_SIZE_GROUP_HORIZONTAL;
        if (rValue.equals("none"))
            eMode = VCL_SIZE_GROUP_NONE;
        else if (rValue.equals("horizontal"))
            eMode = VCL_SIZE_GROUP_HORIZONTAL;
        else if (rValue.equals("vertical"))
            eMode = VCL_SIZE_GROUP_VERTICAL;
        else if (rValue.equals("both"))
            eMode = VCL_SIZE_GROUP_BOTH;
        else
        {
            SAL_WARN("vcl.layout", "unknown size group mode" << rValue.getStr());
        }
        set_mode(eMode);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled property: " << rKey.getStr());
        return false;
    }
    return true;
}

MessageDialog::MessageDialog(Window* pParent, WinBits nStyle)
    : Dialog(pParent, nStyle)
    , m_eButtonsType(VCL_BUTTONS_NONE)
    , m_eMessageType(VCL_MESSAGE_INFO)
    , m_pGrid(NULL)
    , m_pImage(NULL)
    , m_pPrimaryMessage(NULL)
    , m_pSecondaryMessage(NULL)
{
    SetType(WINDOW_MESSBOX);
}

MessageDialog::MessageDialog(Window* pParent,
    const OUString &rMessage,
    VclMessageType eMessageType,
    VclButtonsType eButtonsType,
    WinBits nStyle)
    : Dialog(pParent, nStyle)
    , m_eButtonsType(eButtonsType)
    , m_eMessageType(eMessageType)
    , m_pGrid(NULL)
    , m_pImage(NULL)
    , m_pPrimaryMessage(NULL)
    , m_pSecondaryMessage(NULL)
    , m_sPrimaryString(rMessage)
{
    SetType(WINDOW_MESSBOX);
}

MessageDialog::MessageDialog(Window* pParent, const OString& rID, const OUString& rUIXMLDescription)
    : Dialog(pParent, rID, rUIXMLDescription, WINDOW_MESSBOX)
    , m_eButtonsType(VCL_BUTTONS_NONE)
    , m_eMessageType(VCL_MESSAGE_INFO)
    , m_pGrid(NULL)
    , m_pImage(NULL)
    , m_pPrimaryMessage(NULL)
    , m_pSecondaryMessage(NULL)
{
}

MessageDialog::~MessageDialog()
{
    for (size_t i = 0; i < m_aOwnedButtons.size(); ++i)
        delete m_aOwnedButtons[i];
    delete m_pSecondaryMessage;
    delete m_pPrimaryMessage;
    delete m_pImage;
    delete m_pGrid;
}

IMPL_LINK(MessageDialog, ButtonHdl, Button *, pButton)
{
    EndDialog(get_response(pButton));
    return 0;
}

short MessageDialog::get_response(const Window *pWindow) const
{
    std::map<const Window*, short>::const_iterator aFind = m_aResponses.find(pWindow);
    if (aFind != m_aResponses.end())
        return aFind->second;
    return m_pUIBuilder->get_response(pWindow);
}

void MessageDialog::setButtonHandlers(VclButtonBox *pButtonBox)
{
    assert(pButtonBox);
    for (Window* pChild = pButtonBox->GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        switch (pChild->GetType())
        {
            case WINDOW_PUSHBUTTON:
            {
                PushButton* pButton = (PushButton*)pChild;
                pButton->SetClickHdl(LINK(this, MessageDialog, ButtonHdl));
                break;
            }
            //insist that the response ids match the default actions for those
            //widgets, and leave their default handlers in place
            case WINDOW_OKBUTTON:
                assert(get_response(pChild) == RET_OK);
                break;
            case WINDOW_CANCELBUTTON:
                assert(get_response(pChild) == RET_CANCEL);
                break;
            case WINDOW_HELPBUTTON:
                assert(get_response(pChild) == RET_HELP);
                break;
            default:
                SAL_WARN("vcl.layout", "The type of widget " <<
                    pChild->GetHelpId() << " is currently not handled");
                break;
        }
        //The default is to stick the focus into the first widget
        //that accepts it, and if that happens and its a button
        //then that becomes the new default button, so explicitly
        //put the focus into the default button
        if (pChild->GetStyle() & WB_DEFBUTTON)
            pChild->GrabFocus();
    }
}

void MessageDialog::SetMessagesWidths(Window *pParent,
    VclMultiLineEdit *pPrimaryMessage, VclMultiLineEdit *pSecondaryMessage)
{
    if (pSecondaryMessage)
    {
        assert(pPrimaryMessage);
        Font aFont = pParent->GetSettings().GetStyleSettings().GetLabelFont();
        aFont.SetSize(Size(0, aFont.GetSize().Height() * 1.2));
        aFont.SetWeight(WEIGHT_BOLD);
        pPrimaryMessage->SetControlFont(aFont);
        pPrimaryMessage->SetMaxTextWidth(pPrimaryMessage->approximate_char_width() * 60);
        pSecondaryMessage->SetMaxTextWidth(pSecondaryMessage->approximate_char_width() * 80);
    }
    else
        pPrimaryMessage->SetMaxTextWidth(pPrimaryMessage->approximate_char_width() * 80);
}


short MessageDialog::Execute()
{
    setDeferredProperties();

    if (!m_pGrid)
    {
        VclContainer *pContainer = get_content_area();
        assert(pContainer);

        m_pGrid = new VclGrid(pContainer);
        m_pGrid->set_column_spacing(12);

        m_pImage = new FixedImage(m_pGrid, WB_CENTER | WB_VCENTER | WB_3DLOOK);
        switch (m_eMessageType)
        {
            case VCL_MESSAGE_INFO:
                m_pImage->SetImage(InfoBox::GetStandardImage());
                break;
            case VCL_MESSAGE_WARNING:
                m_pImage->SetImage(WarningBox::GetStandardImage());
                break;
            case VCL_MESSAGE_QUESTION:
                m_pImage->SetImage(QueryBox::GetStandardImage());
                break;
            case VCL_MESSAGE_ERROR:
                m_pImage->SetImage(ErrorBox::GetStandardImage());
                break;
        }
        m_pImage->set_grid_left_attach(0);
        m_pImage->set_grid_top_attach(0);
        m_pImage->set_valign(VCL_ALIGN_START);
        m_pImage->Show();

        WinBits nWinStyle = WB_LEFT | WB_VCENTER | WB_WORDBREAK | WB_NOLABEL | WB_NOTABSTOP;

        bool bHasSecondaryText = !m_sSecondaryString.isEmpty();

        m_pPrimaryMessage = new VclMultiLineEdit(m_pGrid, nWinStyle);
        m_pPrimaryMessage->SetPaintTransparent(true);
        m_pPrimaryMessage->EnableCursor(false);

        m_pPrimaryMessage->set_grid_left_attach(1);
        m_pPrimaryMessage->set_grid_top_attach(0);
        m_pPrimaryMessage->set_hexpand(true);
        m_pPrimaryMessage->SetText(m_sPrimaryString);
        m_pPrimaryMessage->Show(!m_sPrimaryString.isEmpty());

        m_pSecondaryMessage = new VclMultiLineEdit(m_pGrid, nWinStyle);
        m_pSecondaryMessage->SetPaintTransparent(true);
        m_pSecondaryMessage->EnableCursor(false);
        m_pSecondaryMessage->set_grid_left_attach(1);
        m_pSecondaryMessage->set_grid_top_attach(1);
        m_pSecondaryMessage->set_hexpand(true);
        m_pSecondaryMessage->SetText(m_sSecondaryString);
        m_pSecondaryMessage->Show(bHasSecondaryText);

        MessageDialog::SetMessagesWidths(this, m_pPrimaryMessage, bHasSecondaryText ? m_pSecondaryMessage : NULL);

        VclButtonBox *pButtonBox = get_action_area();
        assert(pButtonBox);
        PushButton *pBtn;
        switch (m_eButtonsType)
        {
            case VCL_BUTTONS_NONE:
                break;
            case VCL_BUTTONS_OK:
                pBtn = new OKButton(pButtonBox);
                pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_OK;
                break;
            case VCL_BUTTONS_CLOSE:
                pBtn = new CloseButton(pButtonBox);
                pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_CLOSE;
                break;
            case VCL_BUTTONS_CANCEL:
                pBtn = new CancelButton(pButtonBox);
                pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_CANCEL;
                break;
            case VCL_BUTTONS_YES_NO:
                pBtn = new PushButton(pButtonBox);
                pBtn->SetText(Button::GetStandardText(BUTTON_YES));
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_YES;

                pBtn = new PushButton(pButtonBox);
                pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
                pBtn->SetText(Button::GetStandardText(BUTTON_NO));
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_NO;
                break;
            case VCL_BUTTONS_OK_CANCEL:
                pBtn = new OKButton(pButtonBox);
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_OK;

                pBtn = new CancelButton(pButtonBox);
                pBtn->SetStyle(pBtn->GetStyle() & WB_DEFBUTTON);
                pBtn->Show();
                m_aOwnedButtons.push_back(pBtn);
                m_aResponses[pBtn] = RET_CANCEL;
                break;
        }
        setButtonHandlers(pButtonBox);
        pButtonBox->sort_native_button_order();
        m_pGrid->Show();

    }
    return Dialog::Execute();
}

OUString MessageDialog::get_primary_text() const
{
    const_cast<MessageDialog*>(this)->setDeferredProperties();

    return m_sPrimaryString;
}

OUString MessageDialog::get_secondary_text() const
{
    const_cast<MessageDialog*>(this)->setDeferredProperties();

    return m_sSecondaryString;
}

bool MessageDialog::set_property(const OString &rKey, const OString &rValue)
{
    if (rKey == "text")
        set_primary_text(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "secondary-text")
        set_secondary_text(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "message-type")
    {
        VclMessageType eMode = VCL_MESSAGE_INFO;
        if (rValue.equals("info"))
            eMode = VCL_MESSAGE_INFO;
        else if (rValue.equals("warning"))
            eMode = VCL_MESSAGE_WARNING;
        else if (rValue.equals("question"))
            eMode = VCL_MESSAGE_QUESTION;
        else if (rValue.equals("error"))
            eMode = VCL_MESSAGE_ERROR;
        else
        {
            SAL_WARN("vcl.layout", "unknown message type mode" << rValue.getStr());
        }
        m_eMessageType = eMode;
    }
    else if (rKey == "buttons")
    {
        VclButtonsType eMode = VCL_BUTTONS_NONE;
        if (rValue.equals("none"))
            eMode = VCL_BUTTONS_NONE;
        else if (rValue.equals("ok"))
            eMode = VCL_BUTTONS_OK;
        else if (rValue.equals("cancel"))
            eMode = VCL_BUTTONS_CANCEL;
        else if (rValue.equals("close"))
            eMode = VCL_BUTTONS_CLOSE;
        else if (rValue.equals("yes-no"))
            eMode = VCL_BUTTONS_YES_NO;
        else if (rValue.equals("ok-cancel"))
            eMode = VCL_BUTTONS_OK_CANCEL;
        else
        {
            SAL_WARN("vcl.layout", "unknown buttons type mode" << rValue.getStr());
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
    }
}

void MessageDialog::set_secondary_text(const OUString &rSecondaryString)
{
    m_sSecondaryString = rSecondaryString;
    if (m_pSecondaryMessage)
    {
        m_pSecondaryMessage->SetText(OUString("\n") + m_sSecondaryString);
        m_pSecondaryMessage->Show(!m_sSecondaryString.isEmpty());
    }
}

Size getLegacyBestSizeForChildren(const Window &rWindow)
{
    Rectangle aBounds;

    for (const Window* pChild = rWindow.GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

        Rectangle aChildBounds(pChild->GetPosPixel(), pChild->GetSizePixel());
        aBounds.Union(aChildBounds);
    }

    if (aBounds.IsEmpty())
        return rWindow.GetSizePixel();

    Size aRet(aBounds.GetSize());
    Point aTopLeft(aBounds.TopLeft());
    aRet.Width() += aTopLeft.X()*2;
    aRet.Height() += aTopLeft.Y()*2;

    return aRet;
}

Window* getNonLayoutParent(Window *pWindow)
{
    while (pWindow)
    {
        pWindow = pWindow->GetParent();
        if (!pWindow || !isContainerWindow(*pWindow))
            break;
    }
    return pWindow;
}

Window* getNonLayoutRealParent(Window *pWindow)
{
    while (pWindow)
    {
        pWindow = pWindow->ImplGetParent();
        if (!pWindow || !isContainerWindow(*pWindow))
            break;
    }
    return pWindow;
}

bool isVisibleInLayout(const Window *pWindow)
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

bool isEnabledInLayout(const Window *pWindow)
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

bool isLayoutEnabled(const Window *pWindow)
{
    //Child is a container => we're layout enabled
    const Window *pChild = pWindow ? pWindow->GetWindow(WINDOW_FIRSTCHILD) : NULL;
    return pChild && isContainerWindow(*pChild) && !pChild->GetWindow(WINDOW_NEXT);
}

bool isInitialLayout(const Window *pWindow)
{
    Dialog *pParentDialog = pWindow ? pWindow->GetParentDialog() : NULL;
    return pParentDialog && pParentDialog->isCalculatingInitialLayoutSize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
