/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdview.hxx>
#include <svx/svdglob.hxx>  // Stringcache
#include <svx/svdstr.hrc>   // Objektname
#include <svx/svdoashp.hxx>
#include <tools/bigint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrTextObj::hasSpecialDrag() const
{
    return true;
}

String SdrTextObj::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    XubString aStr;
    TakeMarkedDescriptionString(STR_DragRectResize,aStr);
    return aStr;
}

bool SdrTextObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bRetval(SdrAttrObj::EndCreate(rStat, eCmd));

    if(bTextFrame)
    {
        const bool bAutoGrowHeight(IsAutoGrowHeight());
        const bool bAutoGrowWidth(IsAutoGrowWidth());

        if(bAutoGrowHeight || bAutoGrowWidth)
        {
            const basegfx::B2DVector aAbsScale(basegfx::absolute(getSdrObjectScale()));

            if(bAutoGrowHeight)
            {
                // MinTextHeight
                SetMinTextFrameHeight(basegfx::fround(aAbsScale.getY()));
            }

            if(bAutoGrowWidth)
            {
                // MinTextWidth
                SetMinTextFrameWidth(basegfx::fround(aAbsScale.getX()));
            }

            // Textrahmen neu berechnen
            AdjustTextFrameWidthAndHeight();
            ActionChanged();
        }
    }

    return bRetval;
}

void SdrTextObj::BrkCreate(SdrDragStat& /*rStat*/)
{
}

bool SdrTextObj::BckCreate(SdrDragStat& /*rStat*/)
{
    return true;
}

Pointer SdrTextObj::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    if (IsTextFrame())
    {
        return Pointer(POINTER_DRAW_TEXT);
    }

    return Pointer(POINTER_CROSS);
}

// eof
