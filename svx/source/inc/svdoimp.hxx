/*************************************************************************
 *
 *  $RCSfile: svdoimp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:45:54 $
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

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _SVX_RECTENUM_HXX
#include "rectenum.hxx"
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

class SdrObject;
class XOutputDevice;
class XFillAttrSetItem;
class XLineAttrSetItem;
class SfxItemSet;

///////////////////////////////////////////////////////////////////////////////

// #100127# Bracket filled shapes with a comment, if recording a Mtf
class ImpGraphicFill
{
public:
    ImpGraphicFill( const SdrObject& rObj, const XOutputDevice& rXOut, const SfxItemSet& rFillItemSet, bool bIsShadow=false );
    ~ImpGraphicFill();

private:
    const SdrObject&        mrObj;
    const XOutputDevice&    mrXOut;
    bool                    mbCommentWritten;
};

///////////////////////////////////////////////////////////////////////////////

// #104609# Extracted from XOutputDevice::ImpCalcBmpFillStartValues

/** Calc offset and size for bitmap fill

    This method calculates the size and the offset from the left, top
    position of a shape in logical coordinates

    @param rStartOffset
    The offset from the left, top position of the output rectangle is returned

    @param rBmpOutputSize
    The output size of the bitmap is returned herein

    @param rOutputRect
    Specifies the output rectangle into which the bitmap should be tiled into

    @param rOutputMapMode
    Specifies the logical coordinate system the output rectangle is in

    @param rFillBitmap
    Specifies the bitmap to fill with

    @param rBmpSize
    The desired destination bitmap size. If null, size is taken from the bitmap

    @param rBmpPerCent
    Percentage of bitmap size, relative to the output rectangle

    @param rBmpOffPerCent
    Offset for bitmap tiling, in percentage relative to bitmap output size

    @param bBmpLogSize
    True when using the preferred bitmap size, False when using the percentage value

    @param bBmpTile
    True for tiling. False only paints one instance of the bitmap

    @param bBmpStretch
    True if bitmap should be stretched to output rect dimension

    @param eBmpRectPoint
    Position of the start point relative to the bitmap

 */
void ImpCalcBmpFillSizes( Size&            rStartOffset,
                          Size&            rBmpOutputSize,
                          const Rectangle& rOutputRect,
                          const MapMode&   rOutputMapMode,
                          const Bitmap&    rFillBitmap,
                          const Size&      rBmpSize,
                          const Size&      rBmpPerCent,
                          const Size&      rBmpOffPerCent,
                          BOOL             bBmpLogSize,
                          BOOL             bBmpTile,
                          BOOL             bBmpStretch,
                          RECT_POINT       eBmpRectPoint );


///////////////////////////////////////////////////////////////////////////////
// ImpCreateDotDashArray takes a XDash and translates it into an array of doubles
// which describe the lengths of the dashes, dots and empty passages. It returns
// the complete length of a full DashDot sequence and fills the given vetor of
// doubles accordingly (also resizing it).

double ImpCreateDotDashArray(::std::vector< double >& rDotDashArray, const XDash& mrDash, sal_Int32 nLineWidth);

///////////////////////////////////////////////////////////////////////////////

class ImpLineStyleParameterPack
{
    XLineJoint                      meLineJoint;
    ::basegfx::B2DPolygon           maStartPolygon;
    ::basegfx::B2DPolygon           maEndPolygon;

    sal_Int32                       mnLineWidth;
    sal_Int32                       mnStartWidth;
    sal_Int32                       mnEndWidth;

    ::std::vector<double>           maDotDashArray;
    double                          mfFullDashDotLen;
    double                          mfDegreeStepWidth;

    // bitfield
    unsigned                        mbStartCentered : 1;
    unsigned                        mbEndCentered : 1;
    unsigned                        mbForceNoArrowsLeft : 1;
    unsigned                        mbForceNoArrowsRight : 1;
    unsigned                        mbForceHair : 1;

    // flag for LineStyle. True is XLINE_SOLID, false is XLINE_DASH
    unsigned                        mbLineStyleSolid : 1;

public:
    ImpLineStyleParameterPack(const SfxItemSet& rSet, bool bForceHair);
    ~ImpLineStyleParameterPack();

    sal_Int32 GetLineWidth() const { return mnLineWidth; }
    sal_Int32 GetDisplayLineWidth() const { return mbForceHair ? 0L : mnLineWidth; }
    bool IsLineStyleSolid() const { return mbLineStyleSolid; }

    sal_Int32 GetStartWidth() const { return mnStartWidth; }
    sal_Int32 GetEndWidth() const { return mnEndWidth; }

    const ::basegfx::B2DPolygon& GetStartPolygon() const { return maStartPolygon; }
    const ::basegfx::B2DPolygon& GetEndPolygon() const { return maEndPolygon; }

    double GetDegreeStepWidth() const { return mfDegreeStepWidth; }

    XLineJoint GetLineJoint() const { return meLineJoint; }
    double GetLinejointMiterMinimumAngle() const { return 15.0; }

    double GetFullDashDotLen() const { return mfFullDashDotLen; }
    const ::std::vector< double >& GetDotDash() const { return maDotDashArray; }

    bool IsStartCentered() const { return mbStartCentered; }
    bool IsEndCentered() const { return mbEndCentered; }

    bool IsStartActive() const { return (!mbForceNoArrowsLeft && maStartPolygon.count() && GetStartWidth()); }
    bool IsEndActive() const { return (!mbForceNoArrowsRight && maEndPolygon.count() && GetEndWidth()); }

    void ForceNoArrowsLeft(bool bNew) { mbForceNoArrowsLeft = bNew; }
    void ForceNoArrowsRight(bool bNew) { mbForceNoArrowsRight = bNew; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpLineGeometryCreator
{
    const ImpLineStyleParameterPack&        mrLineAttr;
    ::basegfx::B2DPolyPolygon&              maAreaPolyPolygon;
    ::basegfx::B2DPolyPolygon&              maLinePolyPolygon;

    // bitfield
    unsigned                                mbLineDraft : 1;

    // private support functions
    // help functions for line geometry creation
    void ImpCreateLineGeometry(
        const ::basegfx::B2DPolygon& rSourcePoly);

public:
    ImpLineGeometryCreator(
        const ImpLineStyleParameterPack& rAttr,
        ::basegfx::B2DPolyPolygon& rPoPo,
        ::basegfx::B2DPolyPolygon& rPoLi,
        bool bIsLineDraft = false)
    :   mrLineAttr(rAttr),
        maAreaPolyPolygon(rPoPo),
        maLinePolyPolygon(rPoLi),
        mbLineDraft(bIsLineDraft)
    {
    }

    void AddPolygon(const ::basegfx::B2DPolygon& rPoly) { ImpCreateLineGeometry(rPoly); }
    const ::basegfx::B2DPolyPolygon& GetAreaPolyPolygon() const { return maAreaPolyPolygon; }
    const ::basegfx::B2DPolyPolygon& GetLinePolyPolygon() const { return maLinePolyPolygon; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrLineGeometry
{
    ::basegfx::B2DPolyPolygon               maAreaPolyPolygon;
    ::basegfx::B2DPolyPolygon               maLinePolyPolygon;
    ImpLineStyleParameterPack               maLineAttr;

    // bitfield
    unsigned                                mbForceOnePixel : 1;
    unsigned                                mbForceTwoPixel : 1;

public:
    SdrLineGeometry(
        const ::basegfx::B2DPolyPolygon& rAreaPolyPolygon,
        const ::basegfx::B2DPolyPolygon& rLinePolyPolygon,
        const ImpLineStyleParameterPack& rLineAttr,
        bool bForceOnePixel,
        bool bForceTwoPixel)
    :   maAreaPolyPolygon(rAreaPolyPolygon),
        maLinePolyPolygon(rLinePolyPolygon),
        maLineAttr(rLineAttr),
        mbForceOnePixel(bForceOnePixel),
        mbForceTwoPixel(bForceTwoPixel)
    {}

    const ::basegfx::B2DPolyPolygon& GetAreaPolyPolygon() { return maAreaPolyPolygon; }
    const ::basegfx::B2DPolyPolygon& GetLinePolyPolygon() { return maLinePolyPolygon; }
    const ImpLineStyleParameterPack& GetLineAttr() { return maLineAttr; }
    bool DoForceOnePixel() const { return mbForceOnePixel; }
    bool DoForceTwoPixel() const { return mbForceTwoPixel; }
};

#endif // _SVX_SVDOIMP_HXX

// eof
