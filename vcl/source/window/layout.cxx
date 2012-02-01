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

Size Box::calculateRequisition() const
{
    long nMaxChildDimension = 0;

    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    sal_uInt16 nChildren = GetChildCount();
    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
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

    rtl::OString sBorderWidth(RTL_CONSTASCII_STRINGPARAM("border-width"));
    sal_Int32 nBorderWidth = getWidgetProperty<sal_Int32>(sBorderWidth);

    aSize.Width() += nBorderWidth*2;
    aSize.Height() += nBorderWidth*2;

    return aSize;
}

Size Box::GetOptimalSize(WindowSizeType eType) const
{
    if (eType == WINDOWSIZE_MAXIMUM)
        return Window::GetOptimalSize(eType);
    return calculateRequisition();
}

void Box::setAllocation(const Size &rAllocation)
{
    sal_uInt16 nChildren = GetChildCount();
    if (!nChildren)
        return;

    rtl::OString sExpand(RTL_CONSTASCII_STRINGPARAM("expand"));

    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;
    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        bool bExpand = pChild->getWidgetProperty<sal_Bool>(sExpand);
        if (bExpand)
            ++nExpandChildren;
    }

    if (!nVisibleChildren)
        return;

    rtl::OString sBorderWidth(RTL_CONSTASCII_STRINGPARAM("border-width"));
    sal_Int32 nBorderWidth = getWidgetProperty<sal_Int32>(sBorderWidth);

    Size aAllocation = rAllocation;
    aAllocation.Width() -= nBorderWidth*2;
    aAllocation.Height() -= nBorderWidth*2;

    long nAllocPrimaryDimension = getPrimaryDimension(aAllocation);

    long nHomogeneousDimension, nExtraSpace = 0;
    if (m_bHomogeneous)
    {
        nHomogeneousDimension = ((nAllocPrimaryDimension -
            (nVisibleChildren - 1) * m_nSpacing)) / nVisibleChildren;
    }
    else if (nExpandChildren)
    {
        Size aRequisition = calculateRequisition();
        aRequisition.Width() -= nBorderWidth*2;
        aRequisition.Height() -= nBorderWidth*2;
        nExtraSpace = (getPrimaryDimension(aAllocation) - getPrimaryDimension(aRequisition)) / nExpandChildren;
    }

    rtl::OString sPadding(RTL_CONSTASCII_STRINGPARAM("padding"));
    rtl::OString sPackType(RTL_CONSTASCII_STRINGPARAM("pack-type"));
    rtl::OString sFill(RTL_CONSTASCII_STRINGPARAM("fill"));

    Point aPos(nBorderWidth, nBorderWidth);
    for (sal_Int32 ePackType = VCL_PACK_START; ePackType <= VCL_PACK_END; ++ePackType)
    {
        if (ePackType == VCL_PACK_END)
        {
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);
            setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension);
        }

        for (sal_uInt16 i = 0; i < nChildren; ++i)
        {
            Window *pChild = GetChild(i);
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
                aBoxSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
                long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding;
                bool bExpand = pChild->getWidgetProperty<bool>(sExpand);
                if (bExpand)
                    setPrimaryDimension(aBoxSize, nPrimaryDimension + nExtraSpace);
            }
            setSecondaryDimension(aBoxSize, getSecondaryDimension(aAllocation));

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
                    getPrimaryDimension(pChild->GetOptimalSize(WINDOWSIZE_PREFERRED)));

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
                    getPrimaryDimension(aChildSize));
            }

            pChild->SetPosSizePixel(aChildPos, aChildSize);
        }
    }
}

void Box::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    Window::SetPosSizePixel(rAllocPos, rAllocation);
    setAllocation(rAllocation);
}

#define DEFAULT_CHILD_INTERNAL_PAD_X 4
#define DEFAULT_CHILD_INTERNAL_PAD_Y 0
#define DEFAULT_CHILD_MIN_WIDTH 85
#define DEFAULT_CHILD_MIN_HEIGHT 27

Size ButtonBox::calculateRequisition() const
{
    sal_uInt16 nVisibleChildren = 0;

    rtl::OString sChildMinWidth(RTL_CONSTASCII_STRINGPARAM("child-min-width"));
    sal_Int32 nChildMinWidth = getWidgetStyleProperty<sal_Int32>(sChildMinWidth, DEFAULT_CHILD_MIN_WIDTH);
    rtl::OString sChildMinHeight(RTL_CONSTASCII_STRINGPARAM("child-min-height"));
    sal_Int32 nChildMinHeight = getWidgetStyleProperty<sal_Int32>(sChildMinHeight, DEFAULT_CHILD_MIN_HEIGHT);
    Size aSize(nChildMinWidth, nChildMinHeight);

    sal_uInt16 nChildren = GetChildCount();
    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
        if (aChildSize.Width() > aSize.Width())
            aSize.Width() = aChildSize.Width();
        if (aChildSize.Height() > aSize.Height())
            aSize.Height() = aChildSize.Height();
    }

    if (!nVisibleChildren)
        return Size();

    rtl::OString sChildInternalPadX(RTL_CONSTASCII_STRINGPARAM("child-internal-pad-x"));
    sal_Int32 nChildInternalPadX = getWidgetStyleProperty<sal_Int32>(sChildInternalPadX, DEFAULT_CHILD_INTERNAL_PAD_X);
    rtl::OString sChildInternalPadY(RTL_CONSTASCII_STRINGPARAM("child-internal-pad-y"));
    sal_Int32 nChildInternalPadY = getWidgetStyleProperty<sal_Int32>(sChildInternalPadY, DEFAULT_CHILD_INTERNAL_PAD_Y);
    Size aChildPad(nChildInternalPadX, nChildInternalPadY);

    long nPrimaryDimension =
        (getPrimaryDimension(aSize) * nVisibleChildren) +
        (m_nSpacing * (nVisibleChildren-1)) +
        ((getPrimaryDimension(aChildPad)*2) * nVisibleChildren);
    setPrimaryDimension(aSize, nPrimaryDimension + m_nSpacing);

    long nSecondaryDimension = getSecondaryDimension(aSize);
    setSecondaryDimension(aSize, nSecondaryDimension + getSecondaryDimension(aChildPad)*2);

    return aSize;
}


void ButtonBox::setAllocation(const Size &rAllocation)
{
    sal_uInt16 nChildren = GetChildCount();
    if (!nChildren)
        return;

    sal_uInt16 nVisibleChildren = 0;
    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
    }

    if (!nVisibleChildren)
        return;

    rtl::OString sChildInternalPadX(RTL_CONSTASCII_STRINGPARAM("child-internal-pad-x"));
    sal_Int32 nChildInternalPadX = getWidgetStyleProperty<sal_Int32>(sChildInternalPadX, DEFAULT_CHILD_INTERNAL_PAD_X);
    rtl::OString sChildInternalPadY(RTL_CONSTASCII_STRINGPARAM("child-internal-pad-y"));
    sal_Int32 nChildInternalPadY = getWidgetStyleProperty<sal_Int32>(sChildInternalPadY, DEFAULT_CHILD_INTERNAL_PAD_Y);
    Size aChildPad(nChildInternalPadX, nChildInternalPadY);

    Size aSize = rAllocation;

    long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);
    Size aRequisition = calculateRequisition();
    long nHomogeneousDimension = ((getPrimaryDimension(aRequisition) -
        (nVisibleChildren - 1) * m_nSpacing)) / nVisibleChildren;

    nHomogeneousDimension = nHomogeneousDimension - getPrimaryDimension(aChildPad) * 2;

    Point aPos(nChildInternalPadX, nChildInternalPadY);
    long nPrimaryCoordinate = getPrimaryCoordinate(aPos);
    setPrimaryCoordinate(aPos, nPrimaryCoordinate + nAllocPrimaryDimension
        - getPrimaryDimension(aRequisition));

    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;

        Size aChildSize;
        setSecondaryDimension(aChildSize, getSecondaryDimension(aSize));
        setPrimaryDimension(aChildSize, nHomogeneousDimension);

        pChild->SetPosSizePixel(aPos, aChildSize);

        nPrimaryCoordinate = getPrimaryCoordinate(aPos);
        setPrimaryCoordinate(aPos, nPrimaryCoordinate + nHomogeneousDimension + m_nSpacing +
            getPrimaryDimension(aChildPad) * 2);
    }
}

Grid::array_type Grid::assembleGrid() const
{
    array_type A;

    rtl::OString sLeftAttach(RTL_CONSTASCII_STRINGPARAM("left-attach"));
    rtl::OString sWidth(RTL_CONSTASCII_STRINGPARAM("width"));
    rtl::OString sTopAttach(RTL_CONSTASCII_STRINGPARAM("top-attach"));
    rtl::OString sHeight(RTL_CONSTASCII_STRINGPARAM("height"));

    int i = 0;

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

bool Grid::isNullGrid(const array_type &A) const
{
    sal_Int32 nMaxX = A.shape()[0];
    sal_Int32 nMaxY = A.shape()[1];

    if (!nMaxX || !nMaxY)
        return true;
    return false;
}

void Grid::calcMaxs(const array_type &A, std::vector<long> &rWidths, std::vector<long> &rHeights) const
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
            Size aChildSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
            rWidths[x] = std::max(rWidths[x], aChildSize.Width());
            rHeights[y] = std::max(rHeights[y], aChildSize.Height());
        }
    }
}

Size Grid::GetOptimalSize(WindowSizeType eType) const
{
    if (eType == WINDOWSIZE_MAXIMUM)
        return Window::GetOptimalSize(eType);
    return calculateRequisition();
}

//To-Do, col/row spanning ignored for now
Size Grid::calculateRequisition() const
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

void Grid::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    Window::SetPosSizePixel(rAllocPos, rAllocation);
    setAllocation(rAllocation);
}

void Grid::setAllocation(const Size& rAllocation)
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

    Point aPosition(0, 0);
    for (sal_Int32 x = 0; x < nMaxX; ++x)
    {
        for (sal_Int32 y = 0; y < nMaxY; ++y)
        {
            Window *pChild = A[x][y];
            if (pChild)
                pChild->SetPosSizePixel(aPosition, Size(aWidths[x], aHeights[y]));
            aPosition.Y() += aHeights[y] + get_row_spacing();
        }
        aPosition.X() += aWidths[x] + get_column_spacing();
        aPosition.Y() = 0;
    }
}

Size getLegacyBestSizeForChildren(const Window &rWindow)
{
    Rectangle aBounds;

    for (Window* pChild = rWindow.GetWindow(WINDOW_FIRSTCHILD); pChild;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
