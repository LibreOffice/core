/*************************************************************************
 *
 *  $RCSfile: svdoimp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2002-08-22 11:48:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_SVDOIMP_HXX
#define _SVX_SVDOIMP_HXX

#include <memory>
#include <vector>

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _SVDSOB_HXX //autogen
#include "svdsob.hxx"
#endif

#ifndef _SVDTYPES_HXX
#include "svdtypes.hxx" // fuer SdrLayerID
#endif

#ifndef _SVDGLUE_HXX
#include "svdglue.hxx" // Klebepunkte
#endif

#ifndef _SVX_XDASH_HXX
#include "xdash.hxx"
#endif

#ifndef _XPOLY_HXX
#include "xpoly.hxx"
#endif

#ifndef _POLY3D_HXX
#include "poly3d.hxx"
#endif

#ifndef _XENUM_HXX
#include "xenum.hxx"
#endif

class SdrObject;
class ExtOutputDevice;
class XFillAttrSetItem;
class XLineAttrSetItem;
class XPolyPolygon;
class XPolygon;
class PolyPolygon;
class SfxItemSet;

///////////////////////////////////////////////////////////////////////////////

// #100127# Bracket filled shapes with a comment, if recording a Mtf
class ImpGraphicFill
{
public:
    ImpGraphicFill( const SdrObject& rObj, const ExtOutputDevice& rXOut );
    ~ImpGraphicFill();

private:
    const SdrObject&        mrObj;
    const ExtOutputDevice&  mrXOut;
    bool                    mbCommentWritten;
};

///////////////////////////////////////////////////////////////////////////////

class ImpLineStyleParameterPack
{
    OutputDevice*               mpOut;
    XDash                       aDash;

    XLineJoint                  eLineJoint;
    XLineStyle                  eLineStyle;

    const XPolygon&             rStartPolygon;
    const XPolygon&             rEndPolygon;

    INT32                       nLineWidth;
    INT32                       nStartWidth;
    INT32                       nEndWidth;

    ::std::vector<double>       aDotDashArray;
    double                      fFullDashDotLen;
    double                      fDegreeStepWidth;

    BOOL                        bStartCentered;
    BOOL                        bEndCentered;
    BOOL                        bForceNoArrowsLeft;
    BOOL                        bForceNoArrowsRight;

public:
    ImpLineStyleParameterPack(const SfxItemSet& rSet,
        BOOL bForceHair, OutputDevice* pOut);
    ~ImpLineStyleParameterPack();

    OutputDevice* GetOutDev() const { return mpOut; }
    INT32 GetLineWidth() const { return nLineWidth; }
    XLineStyle GetLineStyle() const { return eLineStyle; }

    INT32 GetStartWidth() const { return nStartWidth; }
    INT32 GetEndWidth() const { return nEndWidth; }

    const XPolygon& GetStartPolygon() const { return rStartPolygon; }
    const XPolygon& GetEndPolygon() const { return rEndPolygon; }

    double GetDegreeStepWidth() const { return fDegreeStepWidth; }

    XLineJoint GetLineJoint() const { return eLineJoint; }
    double GetLinejointMiterUpperBound() const { return 3.0; }

    XDashStyle GetDashStyle() const { return aDash.GetDashStyle(); }
    UINT16 GetDots() const { return aDash.GetDots(); }
    UINT32 GetDotLen() const { return aDash.GetDotLen(); }
    UINT16 GetDashes() const { return aDash.GetDashes(); }
    UINT32 GetDashLen() const { return aDash.GetDashLen(); }
    UINT32 GetDashDistance() const { return aDash.GetDistance(); }
    double GetFullDashDotLen() const { return fFullDashDotLen; }
    UINT16 GetFirstDashDotIndex(double fPos, double& rfDist) const;
    UINT16 GetNextDashDotIndex(UINT16 nInd, double& rfDist) const;
    ::std::vector< double > GetDotDash() const { return ::std::vector< double >(aDotDashArray); }

    BOOL IsStartCentered() const { return bStartCentered; }
    BOOL IsEndCentered() const { return bEndCentered; }

    BOOL IsStartActive() const { return (!bForceNoArrowsLeft && GetStartPolygon().GetPointCount() && GetStartWidth()); }
    BOOL IsEndActive() const { return (!bForceNoArrowsRight && GetEndPolygon().GetPointCount() && GetEndWidth()); }

    void ForceNoArrowsLeft(BOOL bNew) { bForceNoArrowsLeft = bNew; }
    void ForceNoArrowsRight(BOOL bNew) { bForceNoArrowsRight = bNew; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpLineGeometryCreator
{
    const ImpLineStyleParameterPack&    mrLineAttr;
    PolyPolygon3D&                      mrPolyPoly3D;
    PolyPolygon3D&                      mrPolyLine3D;
    BOOL                                mbLineDraft;

    // private support functions
    // help functions for line geometry creation
    void ImpCreateLineGeometry(const Polygon3D& rSourcePoly);
    double ImpCreateLineStartEnd(Polygon3D& rArrowPoly, const Polygon3D& rSourcePoly, BOOL bFront, double fWantedWidth, BOOL bCentered);
    void ImpCreateSegmentsForLine(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext, double fPolyPos);
    void ImpCreateLineSegment(const Vector3D* pPrev, const Vector3D* pLeft, const Vector3D* pRight, const Vector3D* pNext);
    double ImpSimpleFindCutPoint(const Vector3D& rEdge1Start, const Vector3D& rEdge1Delta, const Vector3D& rEdge2Start, const Vector3D& rEdge2Delta);

public:
    ImpLineGeometryCreator(const ImpLineStyleParameterPack& rAttr, PolyPolygon3D& rPoPo,
        PolyPolygon3D& rPoLi, BOOL bIsLineDraft = FALSE)
    :   mrLineAttr(rAttr),
        mrPolyPoly3D(rPoPo),
        mrPolyLine3D(rPoLi),
        mbLineDraft(bIsLineDraft)
    {
    }

    void AddPolygon3D(const Polygon3D& rPoly) { ImpCreateLineGeometry(rPoly); }
    const PolyPolygon3D& GetPolyPolygon3D() const { return mrPolyPoly3D; }
    const PolyPolygon3D& GetPolyLines3D() const { return mrPolyLine3D; }
    void Clear() { mrPolyPoly3D.Clear(); mrPolyLine3D.Clear(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpLineGeometry
{
    PolyPolygon3D               maPolyPoly3D;
    PolyPolygon3D               maLinePoly3D;
    ImpLineStyleParameterPack   maLineAttr;
    BOOL                        mbForceOnePixel;
    BOOL                        mbForceTwoPixel;

public:
    ImpLineGeometry(const PolyPolygon3D& rPolyPoly, const PolyPolygon3D& rLinePoly,
                    const ImpLineStyleParameterPack& rLineAttr,
                    BOOL bForceOnePixel, BOOL bForceTwoPixel)
    :   maPolyPoly3D(rPolyPoly),
        maLinePoly3D(rLinePoly),
        maLineAttr(rLineAttr),
        mbForceOnePixel(bForceOnePixel),
        mbForceTwoPixel(bForceTwoPixel)
    {}

    PolyPolygon3D& GetPolyPoly3D() { return maPolyPoly3D; }
    PolyPolygon3D& GetLinePoly3D() { return maLinePoly3D; }
    ImpLineStyleParameterPack& GetLineAttr() { return maLineAttr; }
    BOOL DoForceOnePixel() const { return mbForceOnePixel; }
    BOOL DoForceTwoPixel() const { return mbForceTwoPixel; }
};

#endif // _SVX_SVDOIMP_HXX
