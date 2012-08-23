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

#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include "window.h"

VclContainer::VclContainer(Window *pParent)
    : Window(WINDOW_CONTAINER)
    , m_nBorderWidth(0)
{
    ImplInit(pParent, 0, NULL);
}

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

    bool bPosChanged = aAllocPos != GetPosPixel();
    bool bSizeChanged = aAllocation != GetSizePixel();
    if (bPosChanged && bSizeChanged)
        Window::SetPosSizePixel(aAllocPos, aAllocation);
    else if (bPosChanged)
        Window::SetPosPixel(aAllocPos);
    else if (bSizeChanged)
        Window::SetSizePixel(aAllocation);

    if (bSizeChanged)
        setAllocation(aAllocation);
}

void VclContainer::SetPosPixel(const Point& rAllocPos)
{
    Point aAllocPos = rAllocPos;
    aAllocPos.X() += m_nBorderWidth;
    aAllocPos.Y() += m_nBorderWidth;

    if (aAllocPos != GetPosPixel())
        Window::SetPosPixel(aAllocPos);
}

void VclContainer::SetSizePixel(const Size& rAllocation)
{
    Size aAllocation = rAllocation;
    aAllocation.Width() -= m_nBorderWidth*2;
    aAllocation.Height() -= m_nBorderWidth*2;

    if (aAllocation != GetSizePixel())
    {
        Window::SetSizePixel(aAllocation);
        setAllocation(aAllocation);
    }
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

namespace
{
    //avoid redraws when size/pos is unchanged
    void setPosSizePixel(Window &rWindow, const Point& rAllocPos, const Size& rAllocation)
    {
        bool bPosChanged = rAllocPos != rWindow.GetPosPixel();
        bool bSizeChanged = rAllocation != rWindow.GetSizePixel();
        if (bPosChanged && bSizeChanged)
            rWindow.SetPosSizePixel(rAllocPos, rAllocation);
        else if (bPosChanged)
            rWindow.SetPosPixel(rAllocPos);
        else if (bSizeChanged)
            rWindow.SetSizePixel(rAllocation);
    }
}

void VclBox::setAllocation(const Size &rAllocation)
{
    //SetBackground( Color(0x00, 0xFF, 0x00) );

    rtl::OString sExpand(RTL_CONSTASCII_STRINGPARAM("expand"));

    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;
    for (Window *pChild = GetWindow(WINDOW_FIRSTCHILD); pChild; pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        bool bExpand = pChild->get_expand();
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

            sal_Int32 ePacking = pChild->get_pack_type();

            if (ePacking != ePackType)
                continue;

            long nPadding = pChild->get_padding();

            Size aBoxSize;
            if (m_bHomogeneous)
                setPrimaryDimension(aBoxSize, nHomogeneousDimension);
            else
            {
                aBoxSize = pChild->get_preferred_size();
                long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding;
                bool bExpand = pChild->get_expand();
                if (bExpand)
                    setPrimaryDimension(aBoxSize, nPrimaryDimension + nExtraSpace);
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

            setPosSizePixel(*pChild, aChildPos, aChildSize);
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

    sal_Int32 nChildMinWidth = DEFAULT_CHILD_MIN_WIDTH; //to-do, pull from theme
    sal_Int32 nChildMinHeight = DEFAULT_CHILD_MIN_HEIGHT; //to-do, pull from theme
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

        setPosSizePixel(*pChild, aPos, aChildSize);

        nPrimaryCoordinate = getPrimaryCoordinate(aPos);
        setPrimaryCoordinate(aPos, nPrimaryCoordinate + nHomogeneousDimension + m_nSpacing);
    }
}

VclGrid::array_type VclGrid::assembleGrid() const
{
    array_type A;

    for (Window* pChild = GetWindow(WINDOW_FIRSTCHILD); pChild;
        pChild = pChild->GetWindow(WINDOW_NEXT))
    {
        if (!pChild->IsVisible())
            continue;

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

        A[nLeftAttach][nTopAttach] = pChild;
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
            const Window *pChild = A[x][y];
            if (pChild)
            {
                sal_Int32 nWidth = pChild->get_grid_width();
                for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    aNonEmptyCols[x+nSpanX] = true;

                sal_Int32 nHeight = pChild->get_grid_height();
                for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    aNonEmptyRows[y+nSpanY] = true;
            }
        }
    }

    sal_Int32 nNonEmptyCols = std::count(aNonEmptyCols.begin(), aNonEmptyCols.end(), true);
    sal_Int32 nNonEmptyRows = std::count(aNonEmptyRows.begin(), aNonEmptyRows.end(), true);

    //no empty rows or cols
    if (nNonEmptyCols == nMaxX && nNonEmptyRows == nMaxY)
        return A;

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
            B[x2][y2++] = A[x][y];
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

    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            const Window *pChild = A[x][y];
            if (!pChild)
                continue;
            Size aChildSize = pChild->get_preferred_size();

            sal_Int32 nWidth = pChild->get_grid_width();
            for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
            {
                rWidths[x+nSpanX].m_nValue = std::max(rWidths[x+nSpanX].m_nValue, aChildSize.Width()/nWidth);
                rWidths[x+nSpanX].m_bExpand = rWidths[x+nSpanX].m_bExpand | pChild->get_hexpand();
            }

            sal_Int32 nHeight = pChild->get_grid_height();
            for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
            {
                rHeights[y+nSpanY].m_nValue = std::max(rHeights[y+nSpanY].m_nValue, aChildSize.Height()/nHeight);
                rHeights[y+nSpanY].m_bExpand = rHeights[y+nSpanY].m_bExpand | pChild->get_vexpand();
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

    nTotalWidth += get_column_spacing() * (aWidths.size()-1);

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

    nTotalHeight += get_row_spacing() * (aHeights.size()-1);

    return Size(nTotalWidth, nTotalHeight);
}

void VclGrid::setAllocation(const Size& rAllocation)
{
    //SetBackground( Color(0xFF, 0x00, 0x00) );

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

    long nAvailableWidth = rAllocation.Width() - (get_column_spacing() * nMaxX);
    if (get_column_homogeneous())
    {
        for (sal_Int32 x = 0; x < nMaxX; ++x)
            aWidths[x].m_nValue = nAvailableWidth/nMaxX;
    }
    else
    {
        long nExtraWidth = 0;
        if (rAllocation.Width() < aRequisition.Width())
            nExtraWidth = (rAllocation.Width() - aRequisition.Width()) / nMaxX;
        else
        {
            sal_Int32 nExpandables = 0;
            for (sal_Int32 x = 0; x < nMaxX; ++x)
                if (aWidths[x].m_bExpand)
                    ++nExpandables;
            nExtraWidth = nExpandables ? (rAllocation.Width() - aRequisition.Width()) / nExpandables : 0;
        }
        if (nExtraWidth)
        {
            for (sal_Int32 x = 0; x < nMaxX; ++x)
                if (aWidths[x].m_bExpand)
                    aWidths[x].m_nValue += nExtraWidth;
        }
    }

    long nAvailableHeight = rAllocation.Height() - (get_row_spacing() * nMaxY);
    if (get_row_homogeneous())
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
            aHeights[y].m_nValue = nAvailableHeight/nMaxY;
    }
    else
    {
        long nExtraHeight = 0;
        if (rAllocation.Height() < aRequisition.Height())
            nExtraHeight = (rAllocation.Height() - aRequisition.Height()) / nMaxY;
        else
        {
            sal_Int32 nExpandables = 0;
            for (sal_Int32 y = 0; y < nMaxY; ++y)
                if (aHeights[y].m_bExpand)
                    ++nExpandables;
            nExtraHeight = nExpandables ? (rAllocation.Height() - aRequisition.Height()) / nExpandables : 0;
        }
        if (nExtraHeight)
        {
            for (sal_Int32 y = 0; y < nMaxY; ++y)
                if (aHeights[y].m_bExpand)
                    aHeights[y].m_nValue += nExtraHeight;
        }
    }

    Point aAllocPos(0, 0);
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            Window *pChild = A[x][y];
            if (pChild)
            {
                Size aChildAlloc(0, 0);

                sal_Int32 nWidth = pChild->get_grid_width();
                for (sal_Int32 nSpanX = 0; nSpanX < nWidth; ++nSpanX)
                    aChildAlloc.Width() += aWidths[x+nSpanX].m_nValue;
                aChildAlloc.Width() += get_column_spacing()*(nWidth-1);

                sal_Int32 nHeight = pChild->get_grid_height();
                for (sal_Int32 nSpanY = 0; nSpanY < nHeight; ++nSpanY)
                    aChildAlloc.Height() += aHeights[y+nSpanY].m_nValue;
                aChildAlloc.Height() += get_row_spacing()*(nHeight-1);

                Point aChildPos(aAllocPos);
                Size aChildSize(aChildAlloc);

                VclAlign eHalign = pChild->get_halign();
                VclAlign eValign = pChild->get_valign();

                Size aChildPreferredSize;

                if (eHalign != VCL_ALIGN_FILL || eValign != VCL_ALIGN_FILL)
                    aChildPreferredSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);

                switch (eHalign)
                {
                    case VCL_ALIGN_FILL:
                        break;
                    case VCL_ALIGN_START:
                        if (aChildPreferredSize.Width() < aChildAlloc.Width())
                            aChildSize.Width() = aChildPreferredSize.Width();
                        break;
                    case VCL_ALIGN_END:
                        if (aChildPreferredSize.Width() < aChildAlloc.Width())
                            aChildSize.Width() = aChildPreferredSize.Width();
                        aChildPos.X() += aChildAlloc.Width();
                        aChildPos.X() -= aChildSize.Width();
                        break;
                    case VCL_ALIGN_CENTER:
                        if (aChildPreferredSize.Width() < aChildSize.Width())
                            aChildSize.Width() = aChildPreferredSize.Width();
                        aChildPos.X() += (aChildAlloc.Width() - aChildSize.Width()) / 2;
                        break;
                }

                switch (eValign)
                {
                    case VCL_ALIGN_FILL:
                        break;
                    case VCL_ALIGN_START:
                        if (aChildPreferredSize.Height() < aChildAlloc.Height())
                            aChildSize.Height() = aChildPreferredSize.Height();
                        break;
                    case VCL_ALIGN_END:
                        if (aChildPreferredSize.Height() < aChildAlloc.Height())
                            aChildSize.Height() = aChildPreferredSize.Height();
                        aChildPos.Y() += aChildAlloc.Height();
                        aChildPos.Y() -= aChildSize.Height();
                        break;
                    case VCL_ALIGN_CENTER:
                        if (aChildPreferredSize.Height() < aChildSize.Height())
                            aChildSize.Height() = aChildPreferredSize.Height();
                        aChildPos.Y() += (aChildAlloc.Height() - aChildSize.Height()) / 2;
                        break;
                }

                setPosSizePixel(*pChild, aChildPos, aChildSize);
            }
            aAllocPos.Y() += aHeights[y].m_nValue + get_row_spacing();
        }
        aAllocPos.X() += aWidths[x].m_nValue + get_column_spacing();
        aAllocPos.Y() = 0;
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
        set_column_spacing(rValue.toInt32());
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
        return pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
    return Size(0, 0);
}

void VclBin::setAllocation(const Size &rAllocation)
{
    Window *pChild = get_child();
    if (pChild && pChild->IsVisible())
        setPosSizePixel(*pChild, Point(0, 0), rAllocation);
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
    //SetBackground( Color(0xFF, 0x00, 0xFF) );

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
        setPosSizePixel(*pLabel, aChildPos, aLabelSize);
        aAllocation.Height() -= aLabelSize.Height();
        aChildPos.Y() += aLabelSize.Height();
    }

    if (pChild && pChild->IsVisible())
        setPosSizePixel(*pChild, aChildPos, aAllocation);
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

    setPosSizePixel(*pChild, aChildPos, aAllocation);
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
        aRet = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);

    Size aExpanderSize = m_aDisclosureButton.GetOptimalSize(WINDOWSIZE_PREFERRED);

    if (pLabel && pLabel->IsVisible())
    {
        Size aLabelSize = pLabel->GetOptimalSize(WINDOWSIZE_PREFERRED);
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

    Size aButtonSize = m_aDisclosureButton.GetOptimalSize(WINDOWSIZE_PREFERRED);
    Size aLabelSize;
    Size aExpanderSize = aButtonSize;
    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize = pLabel->GetOptimalSize(WINDOWSIZE_PREFERRED);
        aExpanderSize.Height() = std::max(aExpanderSize.Height(), aLabelSize.Height());
        aExpanderSize.Width() += aLabelSize.Width();
    }

    aExpanderSize.Height() = std::min(aExpanderSize.Height(), aAllocation.Height());
    aExpanderSize.Width() = std::min(aExpanderSize.Width(), aAllocation.Width());

    aButtonSize.Height() = std::min(aButtonSize.Height(), aExpanderSize.Height());
    aButtonSize.Width() = std::min(aButtonSize.Width(), aExpanderSize.Width());

    long nExtraExpanderHeight = aExpanderSize.Height() - aButtonSize.Height();
    Point aButtonPos(aChildPos.X(), aChildPos.Y() + nExtraExpanderHeight/2);
    setPosSizePixel(m_aDisclosureButton, aButtonPos, aButtonSize);

    if (pLabel && pLabel->IsVisible())
    {
        aLabelSize.Height() = std::min(aLabelSize.Height(), aExpanderSize.Height());
        aLabelSize.Width() = std::min(aLabelSize.Width(),
            aExpanderSize.Width() - aButtonSize.Width());

        long nExtraLabelHeight = aExpanderSize.Height() - aLabelSize.Height();
        Point aLabelPos(aChildPos.X() + aButtonSize.Width(), aChildPos.Y() + nExtraLabelHeight/2);
        setPosSizePixel(*pLabel, aLabelPos, aLabelSize);
    }

    aAllocation.Height() -= aExpanderSize.Height();
    aChildPos.Y() += aExpanderSize.Height();

    if (pChild && pChild->IsVisible())
    {
        if (!m_aDisclosureButton.IsChecked())
            aAllocation = Size();
        setPosSizePixel(*pChild, aChildPos, aAllocation);
    }
}

bool VclExpander::set_property(const rtl::OString &rKey, const rtl::OString &rValue)
{
    if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("expanded")))
        m_aDisclosureButton.Check(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("resize-toplevel")))
        m_bResizeTopLevel = toBool(rValue);
    else
        return VclBin::set_property(rKey, rValue);
    return true;
}

IMPL_LINK( VclExpander, ClickHdl, DisclosureButton*, pBtn )
{
    Window *pChild = get_child();
    if (pChild)
    {
        pChild->Show(pBtn->IsChecked());
        Dialog* pResizeDialog = m_bResizeTopLevel ? GetParentDialog() : NULL;
        if (pResizeDialog)
            pResizeDialog->setInitialLayoutSize();
        else
            queue_resize();
    }
    return 0;
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
    while (pParent && pParent->GetType() == WINDOW_CONTAINER)
    {
        pParent = pParent->GetParent();
    }
    return pParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
