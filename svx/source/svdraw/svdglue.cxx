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
#include <tools/debug.hxx>

#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdlegacy.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrGluePoint::SdrGluePoint()
:   mnEscDir(SDRESC_SMART),
    mnId(0),
    mnAlign(0),
    mbNoPercent(false),
    mbReallyAbsolute(false),
    mbUserDefined(true)
{
}

SdrGluePoint::SdrGluePoint(const basegfx::B2DPoint& rNewPos,
    bool bNewPercent,
    sal_uInt16 nNewAlign)
:   maPos(rNewPos),
    mnEscDir(SDRESC_SMART),
    mnId(0),
    mnAlign(nNewAlign),
    mbNoPercent(!bNewPercent),
    mbReallyAbsolute(false),
    mbUserDefined(true)
{
}

bool SdrGluePoint::operator==(const SdrGluePoint& rCmpGP) const
{
    return GetPos() == rCmpGP.GetPos()
        && GetEscDir() == rCmpGP.GetEscDir()
        && GetId() == rCmpGP.GetId()
        && GetAlign() == rCmpGP.GetAlign()
        && IsPercent() == rCmpGP.IsPercent()
        && IsReallyAbsolute() == rCmpGP.IsReallyAbsolute()
        && IsUserDefined() == rCmpGP.IsUserDefined();
}

basegfx::B2DPoint SdrGluePoint::GetAbsolutePos(const basegfx::B2DRange& rObjectRange) const
{
    if(IsReallyAbsolute() || rObjectRange.isEmpty())
    {
        return GetPos();
    }

    basegfx::B2DPoint aPt(GetPos());
    basegfx::B2DPoint aOfs(rObjectRange.getCenter());

    switch(GetHorzAlign())
    {
        case SDRHORZALIGN_LEFT  :
        {
            aOfs.setX(rObjectRange.getMinX());
            break;
        }
        case SDRHORZALIGN_RIGHT :
        {
            aOfs.setX(rObjectRange.getMaxX());
            break;
        }
    }

    switch(GetVertAlign())
    {
        case SDRVERTALIGN_TOP   :
        {
            aOfs.setY(rObjectRange.getMinY());
            break;
        }
        case SDRVERTALIGN_BOTTOM:
        {
            aOfs.setY(rObjectRange.getMaxY());
            break;
        }
    }

    if(IsPercent())
    {
        const basegfx::B2DVector aScale(rObjectRange.getRange() / basegfx::B2DTuple(10000.0, 10000.0));

        aPt *= aScale;
    }

    aPt+=aOfs;

    // limit to object bound
    aPt = rObjectRange.clamp(aPt);

    return aPt;
}

void SdrGluePoint::SetAbsolutePos(const basegfx::B2DPoint& rNewPos, const basegfx::B2DRange& rObjectRange)
{
    if(IsReallyAbsolute() || rObjectRange.isEmpty())
    {
        SetPos(rNewPos);
        return;
    }

    basegfx::B2DPoint aPt(rNewPos);
    basegfx::B2DPoint aOfs(rObjectRange.getCenter());

    switch(GetHorzAlign())
    {
        case SDRHORZALIGN_LEFT  :
        {
            aOfs.setX(rObjectRange.getMinX());
            break;
        }
        case SDRHORZALIGN_RIGHT :
        {
            aOfs.setX(rObjectRange.getMaxX());
            break;
        }
    }

    switch(GetVertAlign())
    {
        case SDRVERTALIGN_TOP   :
        {
            aOfs.setY(rObjectRange.getMinY());
            break;
        }
        case SDRVERTALIGN_BOTTOM:
        {
            aOfs.setY(rObjectRange.getMaxY());
            break;
        }
    }

    aPt -= aOfs;

    if(IsPercent())
    {
        const basegfx::B2DVector aScale(
            10000.0 / (basegfx::fTools::equalZero(rObjectRange.getWidth()) ? rObjectRange.getWidth() : 1.0),
            10000.0 / (basegfx::fTools::equalZero(rObjectRange.getHeight()) ? rObjectRange.getHeight() : 1.0));

        aPt *= aScale;
    }

    SetPos(aPt);
}

sal_Int32 SdrGluePoint::GetAlignAngle() const
{
    switch(GetAlign())
    {
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER: return 0; // Invalid!
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_CENTER: return 0;
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_TOP   : return 4500;
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP   : return 9000;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_TOP   : return 13500;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_CENTER: return 18000;
        case SDRHORZALIGN_LEFT  |SDRVERTALIGN_BOTTOM: return 22500;
        case SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM: return 27000;
        case SDRHORZALIGN_RIGHT |SDRVERTALIGN_BOTTOM: return 31500;
    }

    return 0;
}

void SdrGluePoint::SetAlignAngle(sal_Int32 nWink)
{
    nWink=NormAngle360(nWink);

    if(nWink >= 33750 || nWink < 2250)
    {
        SetAlign(SDRHORZALIGN_RIGHT |SDRVERTALIGN_CENTER);
    }
    else if(nWink < 6750)
    {
        SetAlign(SDRHORZALIGN_RIGHT |SDRVERTALIGN_TOP   );
    }
    else if(nWink < 11250)
    {
        SetAlign(SDRHORZALIGN_CENTER|SDRVERTALIGN_TOP   );
    }
    else if(nWink < 15750)
    {
        SetAlign(SDRHORZALIGN_LEFT  |SDRVERTALIGN_TOP   );
    }
    else if(nWink < 20250)
    {
        SetAlign(SDRHORZALIGN_LEFT  |SDRVERTALIGN_CENTER);
    }
    else if(nWink < 24750)
    {
        SetAlign(SDRHORZALIGN_LEFT  |SDRVERTALIGN_BOTTOM);
    }
    else if(nWink < 29250)
    {
        SetAlign(SDRHORZALIGN_CENTER|SDRVERTALIGN_BOTTOM);
    }
    else if(nWink < 33750)
    {
        SetAlign(SDRHORZALIGN_RIGHT |SDRVERTALIGN_BOTTOM);
    }
}

sal_Int32 SdrGluePoint::EscDirToAngle(sal_uInt16 nEsc) const
{
    switch(nEsc)
    {
        case SDRESC_RIGHT : return 0;
        case SDRESC_TOP   : return 9000;
        case SDRESC_LEFT  : return 18000;
        case SDRESC_BOTTOM: return 27000;
    }

    return 0;
}

sal_uInt16 SdrGluePoint::EscAngleToDir(sal_Int32 nWink) const
{
    nWink=NormAngle360(nWink);

    if(nWink >= 31500 || nWink < 4500)
    {
        return SDRESC_RIGHT;
    }
    if(nWink < 13500)
    {
        return SDRESC_TOP;
    }
    if(nWink < 22500)
    {
        return SDRESC_LEFT;
    }
    if(nWink < 31500)
    {
        return SDRESC_BOTTOM;
    }

    return 0;
}

void SdrGluePoint::Transform(const basegfx::B2DHomMatrix& rTransformation, const basegfx::B2DRange& rObjectRange)
{
    basegfx::B2DPoint aPt(GetAbsolutePos(rObjectRange));
    aPt = rTransformation * aPt;

    // check if old angle is needed
    sal_Int32 nOldAngle(0);
    const bool bTransformEscapes(GetEscDir() & (SDRESC_HORZ|SDRESC_VERT));
    const bool bTransformAligns(GetAlign() != (SDRHORZALIGN_CENTER|SDRVERTALIGN_CENTER));

    if(bTransformEscapes || bTransformAligns)
    {
        const basegfx::B2DPoint aRotated(rTransformation * basegfx::B2DPoint(1.0, 0.0));
        const double fAngleToXAxis(atan2(aRotated.getY(), aRotated.getX()));
        nOldAngle = basegfx::fround((-fAngleToXAxis * 18000.0) / F_PI) % 36000;
    }

    // transform escape directions
    if(bTransformEscapes)
    {
        sal_uInt16 nNewEscDir(0);

        if(GetEscDir() & SDRESC_LEFT)
        {
            nNewEscDir |= EscAngleToDir(EscDirToAngle(SDRESC_LEFT) + nOldAngle);
        }

        if(GetEscDir() & SDRESC_TOP)
        {
            nNewEscDir |= EscAngleToDir(EscDirToAngle(SDRESC_TOP) + nOldAngle);
        }

        if(GetEscDir() & SDRESC_RIGHT)
        {
            nNewEscDir |= EscAngleToDir(EscDirToAngle(SDRESC_RIGHT) + nOldAngle);
        }

        if(GetEscDir() & SDRESC_BOTTOM)
        {
            nNewEscDir |= EscAngleToDir(EscDirToAngle(SDRESC_BOTTOM) + nOldAngle);
        }

        SetEscDir(nNewEscDir);
    }

    // transform alignment edge
    if(bTransformAligns)
    {
        SetAlignAngle(GetAlignAngle() + nOldAngle);
    }

    SetAbsolutePos(aPt, rObjectRange);
}

bool SdrGluePoint::IsHit(const basegfx::B2DPoint& rPnt, double fTolLog, const basegfx::B2DRange& rObjectRange) const
{
    const basegfx::B2DPoint aPt(GetAbsolutePos(rObjectRange));
    const double fDist(basegfx::B2DVector(aPt - rPnt).getLength());

    return basegfx::fTools::lessOrEqual(fDist, fTolLog);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrGluePointList::SdrGluePointList()
:   maList()
{
}

SdrGluePointList::SdrGluePointList(const SdrGluePointList& rSrcList)
:   maList()
{
    *this = rSrcList;
}

SdrGluePointList::~SdrGluePointList()
{
    Clear();
}

SdrGluePoint* SdrGluePointList::GetObject(sal_uInt32 i) const
{
    if(i < maList.size())
    {
        return *(maList.begin() + i);
    }
    else
    {
        OSL_ENSURE(false, "SdrGluePointList::GetObject access out of range (!)");
        return 0;
    }
}

void SdrGluePointList::Clear()
{
    const sal_uInt32 nAnz(GetCount());

    for(sal_uInt32 i(0); i < nAnz; i++)
    {
        delete GetObject(i);
    }

    maList.clear();
}

void SdrGluePointList::operator=(const SdrGluePointList& rSrcList)
{
    if(GetCount())
    {
        Clear();
    }

    const sal_uInt32 nAnz(rSrcList.GetCount());

    for(sal_uInt32 i(0); i < nAnz; i++)
    {
        Insert(rSrcList[i]);
    }
}

// Die Id's der Klebepunkte in der Liste sind stets streng monoton steigend!
// Ggf. wird dem neuen Klebepunkt eine neue Id zugewiesen (wenn diese bereits
// vergeben ist). Die Id 0 ist reserviert.
sal_uInt32 SdrGluePointList::Insert(const SdrGluePoint& rGP)
{
    SdrGluePoint* pGP=new SdrGluePoint(rGP);
    sal_uInt16 nId(pGP->GetId());
    const sal_uInt32 nAnz(GetCount());
    sal_uInt32 nInsPos(nAnz);
    const sal_uInt16 nLastId(nAnz ? GetObject(nAnz - 1)->GetId() : 0);
    DBG_ASSERT(nLastId>=nAnz,"SdrGluePointList::Insert(): nLastId<nAnz");
    const bool bHole(nLastId > nAnz);

    if(nId <= nLastId)
    {
        if(!bHole || 0 == nId)
        {
            nId=nLastId+1;
        }
        else
        {
            bool bBrk(false);

            for(sal_uInt32 nNum(0); nNum < nAnz && !bBrk; nNum++)
            {
                const SdrGluePoint* pGP2=GetObject(nNum);
                const sal_uInt16 nTmpId(pGP2->GetId());

                if(nTmpId == nId)
                {
                    nId=nLastId+1; // bereits vorhanden
                    bBrk = true;
                }

                if(nTmpId > nId)
                {
                    nInsPos=nNum; // Hier einfuegen (einsortieren)
                    bBrk = true;
                }
            }
        }

        pGP->SetId(nId);
    }

    maList.insert(maList.begin() + nInsPos, pGP);

    return nInsPos;
}

void SdrGluePointList::Delete(sal_uInt32 nPos)
{
    if(nPos < maList.size())
    {
        SdrGluePointContainerType::iterator a(maList.begin() + nPos);
        delete *a;
        maList.erase(a);
    }
    else
    {
        OSL_ENSURE(false, "SdrGluePointList::Delete out of range (!)");
    }
}

sal_uInt32 SdrGluePointList::FindGluePoint(sal_uInt32 nId) const
{
    // Hier noch einen optimaleren Suchalgorithmus implementieren.
    // Die Liste sollte stets sortiert sein!!!!
    const sal_uInt32 nAnz(GetCount());
    sal_uInt32 nRet(SDRGLUEPOINT_NOTFOUND);

    for(sal_uInt32 nNum(0); nNum < nAnz && SDRGLUEPOINT_NOTFOUND == nRet; nNum++)
    {
        const SdrGluePoint* pGP=GetObject(nNum);

        if(pGP->GetId() == nId)
        {
            nRet = nNum;
        }
    }

    return nRet;
}

sal_uInt32 SdrGluePointList::GPLHitTest(const basegfx::B2DPoint& rPnt, double fTolLog, const basegfx::B2DRange& rObjectRange,
    bool bBack, sal_uInt32 nId0) const
{
    const sal_uInt32 nAnz(GetCount());
    sal_uInt32 nRet(SDRGLUEPOINT_NOTFOUND);
    sal_uInt32 nNum(bBack ? 0 : nAnz);

    while((bBack ? nNum < nAnz : nNum > 0) && SDRGLUEPOINT_NOTFOUND == nRet)
    {
        if(!bBack)
        {
            nNum--;
        }

        const SdrGluePoint* pGP = GetObject(nNum);

        if(pGP->IsHit(rPnt, fTolLog, rObjectRange))
        {
            nRet = nNum;
        }

        if(bBack)
        {
            nNum++;
        }
    }

    return nRet;
}

void SdrGluePointList::TransformGluePoints(const basegfx::B2DHomMatrix& rTransformation, const basegfx::B2DRange& rObjectRange)
{
    const sal_uInt32 nAnz(GetCount());

    for(sal_uInt32 nNum(0); nNum < nAnz; nNum++)
    {
        GetObject(nNum)->Transform(rTransformation, rObjectRange);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
