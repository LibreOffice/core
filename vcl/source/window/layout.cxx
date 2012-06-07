/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *        Caolán McNamara <caolanm@redhat.com> (Red Hat, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <vcl/layout.hxx>
#include "window.h"

Size VclContainer::GetOptimalSize(WindowSizeType eType) const
{
    if (eType == WINDOWSIZE_MAXIMUM)
        return Window::GetOptimalSize(eType);

    Size aSize = calculateRequisition();
    aSize.Width() += m_nBorderWidth*2;
    aSize.Height() += m_nBorderWidth*2;
    return aSize;
}

void VclContainer::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    Size aAllocation = rAllocation;
    aAllocation.Width() -= m_nBorderWidth*2;
    aAllocation.Height() -= m_nBorderWidth*2;

    Point aAllocPos = rAllocPos;
    aAllocPos.X() += m_nBorderWidth;
    aAllocPos.Y() += m_nBorderWidth;

    Window::SetPosSizePixel(aAllocPos, aAllocation);
    setAllocation(aAllocation);
}

bool VclContainer::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("border-width")))
        set_border_width(rValue.toInt32());
    else
        return Window::set_property(rKey, rValue);
    return true;
}

Size VclBox::calculateRequisition() const
{
    long nMaxChildDimension = 0;

    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = pChild->get_preferred_size();
        long nSecondaryDimension = getSecondaryDimension(aChildSize);
        if (nSecondaryDimension > getSecondaryDimension(aSize))
            setSecondaryDimension(aSize, nSecondaryDimension);
        if (m_bHomogeneous)
        {
            long nPrimaryDimension = getPrimaryDimension(aChildSize);
            if (nPrimaryDimension > nMaxChildDimension)
                nMaxChildDimension = nPrimaryDimension;
        }
        else
        {
            long nPrimaryDimension = getPrimaryDimension(aSize);
            setPrimaryDimension(aSize, nPrimaryDimension + getPrimaryDimension(aChildSize));
        }
    }

    if (nVisibleChildren)
    {
        long nPrimaryDimension = getPrimaryDimension(aSize);
        if (m_bHomogeneous)
            nPrimaryDimension += nMaxChildDimension * nVisibleChildren;
        setPrimaryDimension(aSize, nPrimaryDimension + m_nSpacing * (nVisibleChildren-1));
    }

    return aSize;
}

void VclBox::setAllocation(const Size &rAllocation)
{
    rtl::OString sExpand(RTL_CONSTASCII_STRINGPARAM("expand"));

    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        bool bExpand = pChild->getWidgetProperty<sal_Bool>(sExpand);
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

    rtl::OString sPadding(RTL_CONSTASCII_STRINGPARAM("padding"));
    rtl::OString sPackType(RTL_CONSTASCII_STRINGPARAM("pack-type"));
    rtl::OString sFill(RTL_CONSTASCII_STRINGPARAM("fill"));

    for (sal_Int32 ePackType = VCL_PACK_START; ePackType <= VCL_PACK_END; ++ePackType)
    {
        Point aPos(0, 0);
        if (ePackType == VCL_PACK_END)
        {
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);
            setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension);
        }

        for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
        {
            if (!pChild->IsVisible())
                continue;

            sal_Int32 ePacking = pChild->getWidgetProperty<sal_Int32>(sPackType);

            if (ePacking != ePackType)
                continue;

            long nPadding = pChild->getWidgetProperty<sal_Int32>(sPadding);

            Size aBoxSize;
            if (m_bHomogeneous)
                setPrimaryDimension(aBoxSize, nHomogeneousDimension);
            else
            {
                aBoxSize = pChild->get_preferred_size();
                long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding;
                bool bExpand = pChild->getWidgetProperty<bool>(sExpand);
                if (bExpand)
                    setPrimaryDimension(aBoxSize, nPrimaryDimension + nExtraSpace);
            }
            setSecondaryDimension(aBoxSize, getSecondaryDimension(rAllocation));

            Point aChildPos(aPos);
            Size aChildSize(aBoxSize);
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);

            bool bFill = pChild->getWidgetProperty<sal_Bool>(sFill, sal_True);
            if (bFill)
            {
                setPrimaryDimension(aChildSize, std::max(static_cast<long>(1),
                    getPrimaryDimension(aBoxSize) - nPadding * 2));

                setPrimaryCoordinate(aChildPos, nPrimaryCoordinate + nPadding);
            }
            else
            {
                setPrimaryDimension(aChildSize,
                    getPrimaryDimension(pChild->get_preferred_size()));

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

            pChild->SetPosSizePixel(aChildPos, aChildSize);
        }
    }
}

bool VclBox::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("spacing")))
        set_spacing(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("homogeneous")))
        set_homogeneous(toBool(rValue));
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

#define DEFAULT_CHILD_INTERNAL_PAD_X 4
#define DEFAULT_CHILD_INTERNAL_PAD_Y 0
#define DEFAULT_CHILD_MIN_WIDTH 85
#define DEFAULT_CHILD_MIN_HEIGHT 27

Size VclButtonBox::calculateRequisition() const
{
    sal_uInt16 nVisibleChildren = 0;

    rtl::OString sChildMinWidth(RTL_CONSTASCII_STRINGPARAM("child-min-width"));
    sal_Int32 nChildMinWidth = getWidgetStyleProperty<sal_Int32>(sChildMinWidth, DEFAULT_CHILD_MIN_WIDTH);
    rtl::OString sChildMinHeight(RTL_CONSTASCII_STRINGPARAM("child-min-height"));
    sal_Int32 nChildMinHeight = getWidgetStyleProperty<sal_Int32>(sChildMinHeight, DEFAULT_CHILD_MIN_HEIGHT);
    Size aSize(nChildMinWidth, nChildMinHeight);

    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = pChild->get_preferred_size();
        if (aChildSize.Width() > aSize.Width())
            aSize.Width() = aChildSize.Width();
        if (aChildSize.Height() > aSize.Height())
            aSize.Height() = aChildSize.Height();
    }

    if (!nVisibleChildren)
        return Size();

    long nPrimaryDimension =
        (getPrimaryDimension(aSize) * nVisibleChildren) +
        (m_nSpacing * (nVisibleChildren-1));
    setPrimaryDimension(aSize, nPrimaryDimension + m_nSpacing);

    long nSecondaryDimension = getSecondaryDimension(aSize);
    setSecondaryDimension(aSize, nSecondaryDimension);

    fprintf(stderr, "button box asked for %ld %ld\n", aSize.Width(), aSize.Height());

    return aSize;
}

bool VclButtonBox::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("layout-style")))
    {
        VclButtonBoxStyle eStyle = VCL_BUTTONBOX_DEFAULT_STYLE;
        if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("start")))
            eStyle = VCL_BUTTONBOX_START;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("spread")))
            eStyle = VCL_BUTTONBOX_SPREAD;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("edge")))
            eStyle = VCL_BUTTONBOX_EDGE;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("start")))
            eStyle = VCL_BUTTONBOX_START;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("end")))
            eStyle = VCL_BUTTONBOX_END;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("center")))
            eStyle = VCL_BUTTONBOX_CENTER;
        else
            fprintf(stderr, "unknown layout style %s\n", rValue.getStr());
        set_layout(eStyle);
    }
    else
        return VclBox::set_property(rKey, rValue);
    return true;
}

void VclButtonBox::setAllocation(const Size &rAllocation)
{
    fprintf(stderr, "button box got %ld %ld\n", rAllocation.Width(), rAllocation.Height());

    sal_uInt16 nVisibleChildren = 0;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
    }

    if (!nVisibleChildren)
        return;

    Size aSize = rAllocation;

    long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);
    Size aRequisition = calculateRequisition();
    long nHomogeneousDimension = ((getPrimaryDimension(aRequisition) -
        (nVisibleChildren - 1) * m_nSpacing)) / nVisibleChildren;

    Point aPos(0, 0);
    long nPrimaryCoordinate = getPrimaryCoordinate(aPos);

    //To-Do, other layout styles
    switch (m_eLayoutStyle)
    {
        case VCL_BUTTONBOX_START:
            break;
        default:
            fprintf(stderr, "todo unimplemented layout style\n");
        case VCL_BUTTONBOX_DEFAULT_STYLE:
        case VCL_BUTTONBOX_END:
            setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension
                - getPrimaryDimension(aRequisition));
            break;
    }

    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

        Size aChildSize;
        setSecondaryDimension(aChildSize, getSecondaryDimension(aSize));
        setPrimaryDimension(aChildSize, nHomogeneousDimension);

        pChild->SetPosSizePixel(aPos, aChildSize);

        nPrimaryCoordinate = getPrimaryCoordinate(aPos);
        setPrimaryCoordinate(aPos, nPrimaryCoordinate + nHomogeneousDimension + m_nSpacing);
    }
}

VclGrid::array_type VclGrid::assembleGrid() const
{
    array_type A;

    rtl::OString sLeftAttach(RTL_CONSTASCII_STRINGPARAM("left-attach"));
    rtl::OString sWidth(RTL_CONSTASCII_STRINGPARAM("width"));
    rtl::OString sTopAttach(RTL_CONSTASCII_STRINGPARAM("top-attach"));
    rtl::OString sHeight(RTL_CONSTASCII_STRINGPARAM("height"));

    for (Window* pChild = GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

        sal_Int32 nLeftAttach = pChild->getWidgetProperty<sal_Int32>(sLeftAttach);
        sal_Int32 nWidth = pChild->getWidgetProperty<sal_Int32>(sWidth, 1);
        sal_Int32 nMaxXPos = nLeftAttach+nWidth-1;

        sal_Int32 nTopAttach = pChild->getWidgetProperty<sal_Int32>(sTopAttach);
        sal_Int32 nHeight = pChild->getWidgetProperty<sal_Int32>(sHeight, 1);
        sal_Int32 nMaxYPos = nTopAttach+nHeight-1;

        sal_Int32 nCurrentMaxXPos = A.shape()[0]-1;
        sal_Int32 nCurrentMaxYPos = A.shape()[1]-1;
        if (nMaxXPos > nCurrentMaxXPos || nMaxYPos > nCurrentMaxYPos)
        {
            nCurrentMaxXPos = std::max(nMaxXPos, nCurrentMaxXPos);
            nCurrentMaxYPos = std::max(nMaxYPos, nCurrentMaxYPos);
            A.resize(boost::extents[nCurrentMaxXPos+1][nCurrentMaxYPos+1]);
        }

        A[nLeftAttach][nTopAttach] = pChild;
    }

    return A;
}

bool VclGrid::isNullGrid(const array_type &A) const
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    if (!nMaxX || !nMaxY)
        return true;
    return false;
}

void VclGrid::calcMaxs(const array_type &A, std::vector<long> &rWidths, std::vector<long> &rHeights) const
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    rWidths.resize(nMaxX);
    rHeights.resize(nMaxY);

    rtl::OString sWidth(RTL_CONSTASCII_STRINGPARAM("width"));
    rtl::OString sHeight(RTL_CONSTASCII_STRINGPARAM("height"));

    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            const Window *pChild = A[x][y];
            if (!pChild)
                continue;
            Size aChildSize = pChild->get_preferred_size();

            sal_Int32 nWidth = pChild->getWidgetProperty<sal_Int32>(sWidth, 1);
            for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                rWidths[x+nSpanX] = std::max(rWidths[x+nSpanX], aChildSize.Width()/nWidth);

            sal_Int32 nHeight = pChild->getWidgetProperty<sal_Int32>(sHeight, 1);
            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                rHeights[y+nSpanY] = std::max(rHeights[y+nSpanY], aChildSize.Height()/nHeight);
        }
    }
}

Size VclGrid::calculateRequisition() const
{
    array_type A = assembleGrid();

    if (isNullGrid(A))
        return Size();

    std::vector<long> aWidths;
    std::vector<long> aHeights;
    calcMaxs(A, aWidths, aHeights);

    long nTotalWidth = 0;
    if (get_column_homogeneous())
    {
        nTotalWidth = *std::max_element(aWidths.begin(), aWidths.end());
        nTotalWidth *= aWidths.size();
    }
    else
    {
        nTotalWidth = std::accumulate(aWidths.begin(), aWidths.end(), 0);
    }

    nTotalWidth += get_column_spacing() * (aWidths.size()-1);

    long nTotalHeight = 0;
    if (get_row_homogeneous())
    {
        nTotalHeight = *std::max_element(aHeights.begin(), aHeights.end());
        nTotalHeight *= aHeights.size();
    }
    else
    {
        nTotalHeight = std::accumulate(aHeights.begin(), aHeights.end(), 0);
    }

    nTotalHeight += get_row_spacing() * (aHeights.size()-1);

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
    std::vector<long> aWidths(nMaxX);
    std::vector<long> aHeights(nMaxY);
    if (!get_column_homogeneous() || !get_row_homogeneous())
    {
        aRequisition = calculateRequisition();
        calcMaxs(A, aWidths, aHeights);
    }

    long nAvailableWidth = rAllocation.Width() - (get_column_spacing() * nMaxX);
    if (get_column_homogeneous())
        std::fill(aWidths.begin(), aWidths.end(), nAvailableWidth/nMaxX);
    else
    {
        long nExtraWidth = (rAllocation.Width() - aRequisition.Width()) / nMaxX;
        for (sal_Int32 x = 0; x < nMaxX; ++x)
            aWidths[x] += nExtraWidth;
    }

    long nAvailableHeight = rAllocation.Height() - (get_row_spacing() * nMaxY);
    if (get_row_homogeneous())
        std::fill(aHeights.begin(), aHeights.end(), nAvailableHeight/nMaxY);
    else
    {
        long nExtraHeight = (rAllocation.Height() - aRequisition.Height()) / nMaxY;
        for (sal_Int32 y = 0; y < nMaxY; ++y)
            aHeights[y] += nExtraHeight;
    }

    rtl::OString sWidth(RTL_CONSTASCII_STRINGPARAM("width"));
    rtl::OString sHeight(RTL_CONSTASCII_STRINGPARAM("height"));

    Point aPosition(0, 0);
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            Window *pChild = A[x][y];
            if (pChild)
            {
                sal_Int32 nWidth = pChild->getWidgetProperty<sal_Int32>(sWidth, 1);
                sal_Int32 nHeight = pChild->getWidgetProperty<sal_Int32>(sHeight, 1);
                pChild->SetPosSizePixel(aPosition, Size(aWidths[x]*nWidth, aHeights[y]*nHeight));
            }
            aPosition.Y() += aHeights[y] + get_row_spacing();
        }
        aPosition.X() += aWidths[x] + get_column_spacing();
        aPosition.Y() = 0;
    }
}

bool toBool(const rtl::OString &rValue)
{
    return (rValue[0] == 't' || rValue[0] == 'T' || rValue[0] == '1');
}

bool VclGrid::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("row-spacing")))
        set_row_spacing(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("column-spacing")))
        set_row_spacing(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("row-homogeneous")))
        set_row_homogeneous(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("column-homogeneous")))
        set_column_homogeneous(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("n-rows")))
        /*nothing to do*/;
    else
        return VclContainer::set_property(rKey, rValue);
    return true;
}

void setGridAttach(Window &rWidget, sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nWidth, sal_Int32 nHeight)
{
    rtl::OString sLeftAttach(RTL_CONSTASCII_STRINGPARAM("left-attach"));
    rWidget.setChildProperty<sal_Int32>(sLeftAttach, nLeft);
    rtl::OString sTopAttach(RTL_CONSTASCII_STRINGPARAM("top-attach"));
    rWidget.setChildProperty<sal_Int32>(sTopAttach, nTop);
    rtl::OString sWidth(RTL_CONSTASCII_STRINGPARAM("width"));
    rWidget.setChildProperty<sal_Int32>(sWidth, nWidth);
    rtl::OString sHeight(RTL_CONSTASCII_STRINGPARAM("height"));
    rWidget.setChildProperty<sal_Int32>(sHeight, nHeight);
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
        return pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
    return Size(0, 0);
}

void VclBin::setAllocation(const Size &rAllocation)
{
    Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        pChild->SetPosSizePixel(Point(0, 0), rAllocation);
}

//To-Do, hook a DecorationView into VclFrame ?

Size VclFrame::calculateRequisition() const
{
    Size aRet(0, 0);

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    const Window *pChild = get_child();
    const Window *pLabel = pChild != pWindowImpl->mpLastChild ? pWindowImpl->mpLastChild : NULL;

    if (pChild && pChild->IsVisible())
        aRet = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = pLabel->GetOptimalSize(WINDOWSIZE_PREFERRED);
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
    const FrameStyle &rFrameStyle =
        GetSettings().GetStyleSettings().GetFrameStyle();
    Size aAllocation(rAllocation.Width() - rFrameStyle.left - rFrameStyle.right,
        rAllocation.Height() - rFrameStyle.top - rFrameStyle.bottom);
    Point aChildPos(rFrameStyle.left, rFrameStyle.top);

    WindowImpl* pWindowImpl = ImplGetWindowImpl();

    //The label widget is the last (of two) children
    Window *pChild = get_child();
    Window *pLabel = pChild != pWindowImpl->mpLastChild ? pWindowImpl->mpLastChild : NULL;

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = pLabel->GetOptimalSize(WINDOWSIZE_PREFERRED);
        aLabelSize.Height() = std::min(aLabelSize.Height(), aAllocation.Height());
        aLabelSize.Width() = std::min(aLabelSize.Width(), aAllocation.Width());
        pLabel->SetPosSizePixel(aChildPos, aLabelSize);
        aAllocation.Height() -= aLabelSize.Height();
        aChildPos.Y() += aLabelSize.Height();
    }

    if (pChild && pChild->IsVisible())
        pChild->SetPosSizePixel(aChildPos, aAllocation);
}

Size VclAlignment::calculateRequisition() const
{
    Size aRet(m_nLeftPadding + m_nRightPadding,
        m_nTopPadding + m_nBottomPadding);

    const Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
    {
        Size aChildSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
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

    pChild->SetPosSizePixel(aChildPos, aAllocation);
}

bool VclAlignment::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("bottom-padding")))
        m_nBottomPadding = rValue.toInt32();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("left-padding")))
        m_nLeftPadding = rValue.toInt32();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("right-padding")))
        m_nRightPadding = rValue.toInt32();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("top-padding")))
        m_nTopPadding = rValue.toInt32();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("xalign")))
        m_fXAlign = rValue.toFloat();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("xscale")))
        m_fXScale = rValue.toFloat();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("yalign")))
        m_fYAlign = rValue.toFloat();
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("yscale")))
        m_fYScale = rValue.toFloat();
    else
        return VclBin::set_property(rKey, rValue);
    return true;
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

Window* getLegacyNonLayoutParent(Window *pParent)
{
    while (pParent && dynamic_cast<const VclContainer*>(pParent))
    {
        pParent = pParent->GetParent();
    }
    return pParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
