/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdoimp.cxx,v $
 * $Revision: 1.8 $
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
#include <svdoimp.hxx>
#include <svtools/itemset.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <xlinjoit.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

///////////////////////////////////////////////////////////////////////////////

ImpLineStyleParameterPack::ImpLineStyleParameterPack(
    const SfxItemSet& rSet,
    bool _bForceHair)
:   mbForceNoArrowsLeft(false),
    mbForceNoArrowsRight(false),
    mbForceHair(_bForceHair)
{
    maStartPolyPolygon = ((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetLineStartValue();

    if(maStartPolyPolygon.count())
    {
        if(maStartPolyPolygon.areControlPointsUsed())
        {
            maStartPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(maStartPolyPolygon);
        }

        if(basegfx::ORIENTATION_NEGATIVE == basegfx::tools::getOrientation(maStartPolyPolygon.getB2DPolygon(0L)))
        {
            maStartPolyPolygon.flip();
        }
    }

    maEndPolyPolygon = ((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetLineEndValue();

    if(maEndPolyPolygon.count())
    {
        if(maEndPolyPolygon.areControlPointsUsed())
        {
            maEndPolyPolygon = basegfx::tools::adaptiveSubdivideByAngle(maEndPolyPolygon);
        }

        if(basegfx::ORIENTATION_NEGATIVE == basegfx::tools::getOrientation(maEndPolyPolygon.getB2DPolygon(0L)))
        {
            maEndPolyPolygon.flip();
        }
    }

    mnLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
    mbLineStyleSolid = (XLINE_SOLID == (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue());

    mnStartWidth = ((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
    if(mnStartWidth < 0)
        mnStartWidth = -mnLineWidth * mnStartWidth / 100L;

    mnEndWidth = ((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
    if(mnEndWidth < 0)
        mnEndWidth = -mnLineWidth * mnEndWidth / 100L;

    mbStartCentered = ((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
    mbEndCentered = ((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();

    mfDegreeStepWidth = 10.0;
    meLineJoint = ((const XLineJointItem&)(rSet.Get(XATTR_LINEJOINT))).GetValue();

    XDash aDash = ((const XLineDashItem&)(rSet.Get(XATTR_LINEDASH))).GetDashValue();

    // fill local dash info
    mfFullDotDashLen = aDash.CreateDotDashArray(maDotDashArray, (double)GetDisplayLineWidth());
}

ImpLineStyleParameterPack::~ImpLineStyleParameterPack()
{
}

bool ImpLineStyleParameterPack::IsStartActive() const
{
    if(!mbForceNoArrowsLeft && maStartPolyPolygon.count() && GetStartWidth())
    {
        return (0L != maStartPolyPolygon.getB2DPolygon(0L).count());
    }

    return false;
}

bool ImpLineStyleParameterPack::IsEndActive() const
{
    if(!mbForceNoArrowsRight && maEndPolyPolygon.count() && GetEndWidth())
    {
        return (0L != maEndPolyPolygon.getB2DPolygon(0L).count());
    }

    return false;
}

void ImpLineGeometryCreator::ImpCreateLineGeometry(const basegfx::B2DPolygon& rSourcePoly)
{
    if(rSourcePoly.count() > 1L)
    {
        basegfx::B2DPolygon aSourceLineGeometry(rSourcePoly);

        if(aSourceLineGeometry.areControlPointsUsed())
        {
            aSourceLineGeometry = basegfx::tools::adaptiveSubdivideByAngle(aSourceLineGeometry);
        }

        sal_uInt32 nCount(aSourceLineGeometry.count());

        if(!aSourceLineGeometry.isClosed())
        {
            nCount--;
            const double fPolyLength(basegfx::tools::getLength(aSourceLineGeometry));
            double fStart(0.0);
            double fEnd(0.0);

            if(mrLineAttr.IsStartActive())
            {
                // create line start polygon and move line end
                basegfx::B2DPolyPolygon aArrowPolyPolygon;
                aArrowPolyPolygon.append(mrLineAttr.GetStartPolyPolygon());

                basegfx::B2DPolyPolygon aArrow = basegfx::tools::createAreaGeometryForLineStartEnd(
                    aSourceLineGeometry,
                    aArrowPolyPolygon,
                    true,
                    (double)mrLineAttr.GetStartWidth(),
                    mrLineAttr.IsStartCentered() ? 0.5 : 0.0,
                    &fStart);

                maAreaPolyPolygon.append(aArrow);
                fStart *= 0.8;
            }

            if(mrLineAttr.IsEndActive())
            {
                // create line end polygon and move line end
                basegfx::B2DPolyPolygon aArrowPolyPolygon;
                aArrowPolyPolygon.append(mrLineAttr.GetEndPolyPolygon());

                basegfx::B2DPolyPolygon aArrow = basegfx::tools::createAreaGeometryForLineStartEnd(
                    aSourceLineGeometry,
                    aArrowPolyPolygon,
                    false,
                    (double)mrLineAttr.GetEndWidth(),
                    mrLineAttr.IsEndCentered() ? 0.5 : 0.0,
                    &fEnd);

                maAreaPolyPolygon.append(aArrow);
                fEnd *= 0.8;
            }

            if(fStart != 0.0 || fEnd != 0.0)
            {
                // build new poly, consume something from old poly
                aSourceLineGeometry = basegfx::tools::getSnippetAbsolute(
                    aSourceLineGeometry, fStart, fPolyLength - fEnd, fPolyLength);
                nCount = aSourceLineGeometry.count() - 1L;
            }
        }

        if(nCount)
        {
            basegfx::B2DPolyPolygon aHairLinePolyPolygon;

            if(mbLineDraft || mrLineAttr.IsLineStyleSolid())
            {
                // no LineStyle
                aHairLinePolyPolygon.append(aSourceLineGeometry);
            }
            else
            {
                // apply LineStyle
                aHairLinePolyPolygon = basegfx::tools::applyLineDashing(aSourceLineGeometry, mrLineAttr.GetDotDash(), mrLineAttr.GetFullDotDashLen());

                // merge LineStyle polygons to bigger parts
                aHairLinePolyPolygon = basegfx::tools::mergeDashedLines(aHairLinePolyPolygon);
            }

            if(!mrLineAttr.GetDisplayLineWidth())
            {
                // LineWidth zero, add directly to linePoly
                maLinePolyPolygon.append(aHairLinePolyPolygon);
            }
            else
            {
                basegfx::tools::B2DLineJoin aB2DLineJoin(basegfx::tools::B2DLINEJOIN_NONE);

                switch(mrLineAttr.GetLineJoint())
                {
                case XLINEJOINT_NONE    : aB2DLineJoin = basegfx::tools::B2DLINEJOIN_NONE;  break;
                case XLINEJOINT_MIDDLE  : aB2DLineJoin = basegfx::tools::B2DLINEJOIN_MIDDLE;    break;
                case XLINEJOINT_BEVEL   : aB2DLineJoin = basegfx::tools::B2DLINEJOIN_BEVEL; break;
                case XLINEJOINT_MITER   : aB2DLineJoin = basegfx::tools::B2DLINEJOIN_MITER; break;
                case XLINEJOINT_ROUND   : aB2DLineJoin = basegfx::tools::B2DLINEJOIN_ROUND; break;
                }

                for(sal_uInt32 a(0L); a < aHairLinePolyPolygon.count(); a++)
                {
                    basegfx::B2DPolygon aCandidate = aHairLinePolyPolygon.getB2DPolygon(a);
                    basegfx::B2DPolyPolygon aAreaPolyPolygon = basegfx::tools::createAreaGeometryForPolygon(
                        aCandidate,
                        (double)mrLineAttr.GetDisplayLineWidth() / 2.0,
                        aB2DLineJoin,
                        (double)mrLineAttr.GetDegreeStepWidth() * F_PI180,
                        (double)mrLineAttr.GetLinejointMiterMinimumAngle() * F_PI180);
                    maAreaPolyPolygon.append(aAreaPolyPolygon);
                }
            }
        }
    }
}

// eof
