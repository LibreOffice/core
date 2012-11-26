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

#include <svx/svdhlpln.hxx>
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHelpLine::SdrHelpLine(SdrHelpLineKind eNewKind)
:   maPos(),
    meKind(eNewKind)
{
}

SdrHelpLine::SdrHelpLine(SdrHelpLineKind eNewKind, const basegfx::B2DPoint& rNewPos)
:   maPos(rNewPos),
    meKind(eNewKind)
{
}

bool SdrHelpLine::operator==(const SdrHelpLine& rCmp) const
{
    return GetPos() == rCmp.GetPos() && GetKind() == rCmp.GetKind();
}

Pointer SdrHelpLine::GetPointer() const
{
    switch(GetKind())
    {
        case SDRHELPLINE_VERTICAL  : return Pointer(POINTER_ESIZE);
        case SDRHELPLINE_HORIZONTAL: return Pointer(POINTER_SSIZE);
        default                    : return Pointer(POINTER_MOVE);
    }
}

bool SdrHelpLine::IsHit(const basegfx::B2DPoint& rPnt, double fTolLog) const
{
    basegfx::B2DPoint aTestPoint(GetPos());

    switch(GetKind())
    {
        case SDRHELPLINE_VERTICAL:
        {
            aTestPoint.setY(rPnt.getY());
            break;
        }
        case SDRHELPLINE_HORIZONTAL:
        {
            aTestPoint.setX(rPnt.getX());
            break;
        }
        default: // case SDRHELPLINE_POINT:
        {
            // get multiple precision for point, e.g. 3.0 gets the needed
            // 15x15 size. All in all the HitTest should be moved to use primitives
            fTolLog *= 5.0;
            break;
        }
    }

    const double fDistance(basegfx::B2DVector(aTestPoint - rPnt).getLength());

    return basegfx::fTools::lessOrEqual(fDistance, fTolLog);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrHelpLineList::SdrHelpLineList()
:   maList()
{
}

SdrHelpLineList::SdrHelpLineList(const SdrHelpLineList& rSrcList)
:   maList()
{
    *this = rSrcList;
}

SdrHelpLineList::~SdrHelpLineList()
{
    Clear();
}

SdrHelpLine* SdrHelpLineList::GetObject(sal_uInt32 i) const
{
    if(i < maList.size())
    {
        return *(maList.begin() + i);
    }
    else
    {
        OSL_ENSURE(false, "SdrHelpLineList::GetObject access out of range (!)");
        return 0;
    }
}

void SdrHelpLineList::Clear()
{
    const sal_uInt32 nAnz(GetCount());

    for(sal_uInt32 i(0); i < nAnz; i++)
    {
        delete GetObject(i);
    }

    maList.clear();
}

void SdrHelpLineList::operator=(const SdrHelpLineList& rSrcList)
{
    Clear();
    const sal_uInt32 nAnz(rSrcList.GetCount());

    for(sal_uInt32 i(0); i < nAnz; i++)
    {
        Insert(rSrcList[i]);
    }
}

bool SdrHelpLineList::operator==(const SdrHelpLineList& rSrcList) const
{
    bool bEqual(false);
    const sal_uInt32 nAnz(GetCount());

    if(nAnz == rSrcList.GetCount())
    {
        bEqual = true;

        for(sal_uInt32 i(0); i < nAnz && bEqual; i++)
        {
            if(*GetObject(i) != *rSrcList.GetObject(i))
            {
                bEqual = false;
            }
        }
    }

    return bEqual;
}

void SdrHelpLineList::Insert(const SdrHelpLine& rHL, sal_uInt32 nPos)
{
    if(0xffffffff == nPos)
    {
        maList.push_back(new SdrHelpLine(rHL));
    }
    else
    {
        maList.insert(maList.begin() + nPos, new SdrHelpLine(rHL));
    }
}

void SdrHelpLineList::Delete(sal_uInt32 nPos)
{
    if(nPos < maList.size())
    {
        SdrHelpLineContainerType::iterator a(maList.begin() + nPos);
        delete *a;
        maList.erase(a);
    }
    else
    {
        OSL_ENSURE(false, "SdrHelpLineList::Delete out of range (!)");
    }
}

sal_uInt32 SdrHelpLineList::HLHitTest(const basegfx::B2DPoint& rPnt, double fTolLog) const
{
    const sal_uInt32 nAnz(GetCount());

    for(sal_uInt32 i(nAnz); i > 0;)
    {
        i--;

        if(GetObject(i)->IsHit(rPnt, fTolLog))
        {
            return i;
        }
    }

    return SDRHELPLINE_NOTFOUND;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
