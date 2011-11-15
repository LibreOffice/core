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
#include <boost/bind.hpp>

#define callDimension(object,ptrToMember) ((object).*(ptrToMember))()

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
        fprintf(stderr, "child %p wants to be %ld %ld\n", pChild, aChildSize.Width(), aChildSize.Height());
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
            nPrimaryDimension += nMaxChildDimension * (nVisibleChildren-1);
        setPrimaryDimension(aSize, nPrimaryDimension + m_nSpacing * (nVisibleChildren-1));
    }

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

    sal_uInt16 nVisibleChildren = 0, nExpandChildren = 0;;
    for (sal_uInt16 i = 0; i < nChildren; ++i)
    {
        Window *pChild = GetChild(i);
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        bool bExpand = pChild->getChildProperty<sal_Bool>(sExpand);
        if (bExpand)
            ++nExpandChildren;
    }

    if (!nVisibleChildren)
        return;

    Size aSize = rAllocation;

    long nAllocPrimaryDimension = getPrimaryDimension(rAllocation);

    long nHomogeneousDimension, nExtraSpace = 0;
    if (m_bHomogeneous)
    {
        nHomogeneousDimension = ( ( nAllocPrimaryDimension -
                          ( nVisibleChildren - 1 ) * m_nSpacing )) / nVisibleChildren;
    }
    else if (nExpandChildren)
    {
        Size aRequisition = calculateRequisition();
        nExtraSpace = (getPrimaryDimension(rAllocation) - getPrimaryDimension(aRequisition)) / nExpandChildren;
    }

    rtl::OString sPadding(RTL_CONSTASCII_STRINGPARAM("padding"));
    rtl::OString sPackType(RTL_CONSTASCII_STRINGPARAM("pack-type"));
    rtl::OString sFill(RTL_CONSTASCII_STRINGPARAM("fill"));

    Point aPos(0, 0);
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

            sal_Int32 ePacking = pChild->getChildProperty<sal_Int32>(sPackType);
            fprintf(stderr, "child packing is %d\n", ePacking);

            if (ePacking != ePackType)
                continue;

            long nPadding = pChild->getChildProperty<sal_Int32>(sPadding);

            Size aBoxSize;
            if (m_bHomogeneous)
                setPrimaryDimension(aBoxSize, nHomogeneousDimension);
            else
            {
                aBoxSize = pChild->GetOptimalSize(WINDOWSIZE_PREFERRED);
                long nPrimaryDimension = getPrimaryDimension(aBoxSize);
                nPrimaryDimension += nPadding;
                bool bExpand = pChild->getChildProperty<bool>(sExpand);
                if (bExpand)
                    setPrimaryDimension(aBoxSize, nPrimaryDimension + nExtraSpace);
            }
            setSecondaryDimension(aBoxSize, getSecondaryDimension(aSize));

            Point aChildPos(aPos);
            Size aChildSize(aBoxSize);
            long nPrimaryCoordinate = getPrimaryCoordinate(aPos);

            bool bFill = pChild->getChildProperty<sal_Bool>(sFill, sal_True);
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

            fprintf(stderr, "child %p set to %ld %ld : %ld %ld\n",
                pChild, aPos.X(), aPos.Y(), aChildSize.Width(), aChildSize.Height());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
