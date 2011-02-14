/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/properties/itemsettools.hxx>
#include <tools/debug.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>

#include <vector>
#include <svx/svdogrp.hxx>
#include <svx/svditer.hxx>

//////////////////////////////////////////////////////////////////////////////
// class to remember broadcast start positions

namespace sdr
{
    namespace properties
    {
        // helper vector to remember rectangles
        typedef ::std::vector< Rectangle > RectangleVector;

        ItemChangeBroadcaster::ItemChangeBroadcaster(const SdrObject& rObj)
        {
            if(rObj.ISA(SdrObjGroup))
            {
                SdrObjListIter aIter((const SdrObjGroup&)rObj, IM_DEEPNOGROUPS);
                mpData = new RectangleVector;
                DBG_ASSERT(mpData, "ItemChangeBroadcaster: No memory (!)");
                ((RectangleVector*)mpData)->reserve(aIter.Count());

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();

                    if(pObj)
                    {
                        ((RectangleVector*)mpData)->push_back(pObj->GetLastBoundRect());
                    }
                }

                mnCount = ((RectangleVector*)mpData)->size();
            }
            else
            {
                mpData = new Rectangle(rObj.GetLastBoundRect());
                mnCount = 1L;
            }
        }

        ItemChangeBroadcaster::~ItemChangeBroadcaster()
        {
            if(mnCount > 1)
            {
                delete ((RectangleVector*)mpData);
            }
            else
            {
                delete ((Rectangle*)mpData);
            }
        }

        sal_uInt32 ItemChangeBroadcaster::GetRectangleCount() const
        {
            return mnCount;
        }

        const Rectangle& ItemChangeBroadcaster::GetRectangle(sal_uInt32 nIndex) const
        {
            if(mnCount > 1)
            {
                return (*((RectangleVector*)mpData))[nIndex];
            }
            else
            {
                return *((Rectangle*)mpData);
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        void ScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
        {
            sal_Int32 nMul(rScale.GetNumerator());
            sal_Int32 nDiv(rScale.GetDenominator());

            if(!rScale.IsValid() || !nDiv)
            {
                return;
            }

            SfxWhichIter aIter(rSet);
            sal_uInt16 nWhich(aIter.FirstWhich());
            const SfxPoolItem *pItem = NULL;

            while(nWhich)
            {
                if(SFX_ITEM_SET == rSet.GetItemState(nWhich, sal_False, &pItem))
                {
                    if(pItem->HasMetrics())
                    {
                        SfxPoolItem* pNewItem = pItem->Clone();
                        pNewItem->ScaleMetrics(nMul, nDiv);
                        rSet.Put(*pNewItem);
                    }
                }
                nWhich = aIter.NextWhich();
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
