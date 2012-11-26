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



#ifndef _SVDGLUE_HXX
#define _SVDGLUE_HXX

#include <tools/contnr.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRESC_SMART  0x0000
#define SDRESC_LEFT   0x0001
#define SDRESC_RIGHT  0x0002
#define SDRESC_TOP    0x0004
#define SDRESC_BOTTOM 0x0008
#define SDRESC_LO    0x0010 /* ni */
#define SDRESC_LU    0x0020 /* ni */
#define SDRESC_RO    0x0040 /* ni */
#define SDRESC_RU    0x0080 /* ni */
#define SDRESC_HORZ  (SDRESC_LEFT|SDRESC_RIGHT)
#define SDRESC_VERT  (SDRESC_TOP|SDRESC_BOTTOM)
#define SDRESC_ALL   0x00FF

#define SDRHORZALIGN_CENTER   0x0000
#define SDRHORZALIGN_LEFT     0x0001
#define SDRHORZALIGN_RIGHT    0x0002
#define SDRHORZALIGN_DONTCARE 0x0010
#define SDRVERTALIGN_CENTER   0x0000
#define SDRVERTALIGN_TOP      0x0100
#define SDRVERTALIGN_BOTTOM   0x0200
#define SDRVERTALIGN_DONTCARE 0x1000

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRGLUEPOINT_NOTFOUND 0xffffffff

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrGluePoint
{
private:
    // Bezugspunkt ist SdrObject::GetSnapRect().Center()
    // bNoPercent=false: Position ist -5000..5000 (1/100)% bzw. 0..10000 (je nach Align)
    // bNoPercent=true : Position ist in log Einh, rel zum Bezugspunkt
    basegfx::B2DPoint       maPos;
    sal_uInt16              mnEscDir;
    sal_uInt32              mnId;
    sal_uInt16              mnAlign;

    /// bitfield
    bool                    mbNoPercent : 1;
    bool                    mbReallyAbsolute : 1; // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    bool                    mbUserDefined : 1; // #i38892#

public:
    SdrGluePoint();
    SdrGluePoint(const basegfx::B2DPoint& rNewPos, bool bNewPercent = true, sal_uInt16 nNewAlign = 0);

    bool operator==(const SdrGluePoint& rCmpGP) const;
    bool operator!=(const SdrGluePoint& rCmpGP) const   { return !operator==(rCmpGP); }

    const basegfx::B2DPoint& GetPos() const { return maPos; }
    void SetPos(const basegfx::B2DPoint& rNewPos) { if(maPos != rNewPos) maPos = rNewPos; }

    sal_uInt16 GetEscDir() const { return mnEscDir; }
    void SetEscDir(sal_uInt16 nNewEsc) { if(mnEscDir != nNewEsc) mnEscDir = nNewEsc; }

    sal_uInt32 GetId() const { return mnId; }
    void SetId(sal_uInt32 nNewId) { if(mnId != nNewId) mnId = nNewId; }

    bool IsPercent() const { return !mbNoPercent; }
    void SetPercent(bool bOn) { if(mbNoPercent == bOn) mbNoPercent = !bOn; }

    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    bool IsReallyAbsolute() const { return mbReallyAbsolute; }

    // #i38892#
    bool IsUserDefined() const { return mbUserDefined; }
    void SetUserDefined(bool bNew) { if(mbUserDefined != bNew) mbUserDefined = bNew; }

    sal_uInt16 GetAlign() const { return mnAlign; }
    void SetAlign(sal_uInt16 nAlg) { if(mnAlign != nAlg) mnAlign = nAlg; }

    sal_uInt16 GetHorzAlign() const { return mnAlign & 0x00FF; }
    void SetHorzAlign(sal_uInt16 nAlg) { if((mnAlign & 0x00FF) != nAlg) mnAlign = (mnAlign & 0xFF00)|(nAlg & 0x00FF); }

    sal_uInt16 GetVertAlign() const { return mnAlign & 0xFF00; }
    void SetVertAlign(sal_uInt16 nAlg) { if((mnAlign & 0xFF00) != nAlg) mnAlign = (mnAlign & 0x00FF)|(nAlg & 0xFF00); }

    bool IsHit(const basegfx::B2DPoint& rPnt, double fTolLog, const basegfx::B2DRange& rObjectRange) const;

    basegfx::B2DPoint GetAbsolutePos(const basegfx::B2DRange& rObjectRange) const;
    void SetAbsolutePos(const basegfx::B2DPoint& rNewPos, const basegfx::B2DRange& rObjectRange);

    sal_Int32 GetAlignAngle() const;
    void SetAlignAngle(sal_Int32 nWink);

    sal_Int32 EscDirToAngle(sal_uInt16 nEsc) const;
    sal_uInt16 EscAngleToDir(sal_Int32 nWink) const;

    void Transform(const basegfx::B2DHomMatrix& rTransformation, const basegfx::B2DRange& rObjectRange);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrGluePointList
{
private:
    typedef ::std::vector< SdrGluePoint* > SdrGluePointContainerType;
    SdrGluePointContainerType   maList;

protected:
    SdrGluePoint* GetObject(sal_uInt32 i) const;

public:
    SdrGluePointList();
    SdrGluePointList(const SdrGluePointList& rSrcList);
    ~SdrGluePointList();

    void                Clear();
    void                operator=(const SdrGluePointList& rSrcList);

    sal_uInt32 GetCount() const { return maList.size(); }

    // Beim Insert wird dem Objekt (also dem GluePoint) automatisch eine Id zugewiesen.
    // ReturnCode ist der Index des neuen GluePoints in der Liste
    sal_uInt32 Insert(const SdrGluePoint& rGP);
    void Delete(sal_uInt32 nPos);

    SdrGluePoint& operator[](sal_uInt32 nPos) { return *GetObject(nPos); }
    const SdrGluePoint& operator[](sal_uInt32 nPos) const { return *GetObject(nPos); }

    sal_uInt32 FindGluePoint(sal_uInt32 nId) const;
    sal_uInt32 GPLHitTest(const basegfx::B2DPoint& rPnt, double fTolLog, const basegfx::B2DRange& rObjectRange,
        bool bBack = false, sal_uInt32 nId0 = 0) const;

    // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
    void TransformGluePoints(const basegfx::B2DHomMatrix& rTransformation, const basegfx::B2DRange& rObjectRange);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDGLUE_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
