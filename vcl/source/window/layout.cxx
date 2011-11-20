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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
