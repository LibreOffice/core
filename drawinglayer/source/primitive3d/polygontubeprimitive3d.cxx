/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        namespace // anonymous namespace
        {
            Primitive3DSequence getLineTubeSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial)
            {
                // static data for buffered tube primitives
                static Primitive3DSequence aLineTubeList;
                static sal_uInt32 nLineTubeSegments(0L);
                static attribute::MaterialAttribute3D aLineMaterial;

                // may exclusively change static data, use mutex
                ::osl::Mutex m_mutex;

                if(nSegments != nLineTubeSegments || !(rMaterial == aLineMaterial))
                {
                    nLineTubeSegments = nSegments;
                    aLineMaterial = rMaterial;
                    aLineTubeList = Primitive3DSequence();
                }

                if(!aLineTubeList.hasElements() && 0L != nLineTubeSegments)
                {
                    const basegfx::B3DPoint aLeft(0.0, 0.0, 0.0);
                    const basegfx::B3DPoint aRight(1.0, 0.0, 0.0);
                    basegfx::B3DPoint aLastLeft(0.0, 1.0, 0.0);
                    basegfx::B3DPoint aLastRight(1.0, 1.0, 0.0);
                    basegfx::B3DHomMatrix aRot;
                    aRot.rotate(F_2PI / (double)nLineTubeSegments, 0.0, 0.0);
                    aLineTubeList.realloc(nLineTubeSegments);

                    for(sal_uInt32 a(0L); a < nLineTubeSegments; a++)
                    {
                        const basegfx::B3DPoint aNextLeft(aRot * aLastLeft);
                        const basegfx::B3DPoint aNextRight(aRot * aLastRight);
                        basegfx::B3DPolygon aNewPolygon;

                        aNewPolygon.append(aNextLeft);
                        aNewPolygon.setNormal(0L, basegfx::B3DVector(aNextLeft - aLeft));

                        aNewPolygon.append(aLastLeft);
                        aNewPolygon.setNormal(1L, basegfx::B3DVector(aLastLeft - aLeft));

                        aNewPolygon.append(aLastRight);
                        aNewPolygon.setNormal(2L, basegfx::B3DVector(aLastRight - aRight));

                        aNewPolygon.append(aNextRight);
                        aNewPolygon.setNormal(3L, basegfx::B3DVector(aNextRight - aRight));

                        aNewPolygon.setClosed(true);

                        const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                        const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, aLineMaterial, false));
                        aLineTubeList[a] = xRef;

                        aLastLeft = aNextLeft;
                        aLastRight = aNextRight;
                    }
                }

                return aLineTubeList;
            }

            Primitive3DSequence getLineCapSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial)
            {
                // static data for buffered tube primitives
                static Primitive3DSequence aLineCapList;
                static sal_uInt32 nLineCapSegments(0L);
                static attribute::MaterialAttribute3D aLineMaterial;

                // may exclusively change static data, use mutex
                ::osl::Mutex m_mutex;

                if(nSegments != nLineCapSegments || !(rMaterial == aLineMaterial))
                {
                    nLineCapSegments = nSegments;
                    aLineMaterial = rMaterial;
                    aLineCapList = Primitive3DSequence();
                }

                if(!aLineCapList.hasElements() && 0L != nLineCapSegments)
                {
                    const basegfx::B3DPoint aNull(0.0, 0.0, 0.0);
                    basegfx::B3DPoint aLast(0.0, 1.0, 0.0);
                    basegfx::B3DHomMatrix aRot;
                    aRot.rotate(F_2PI / (double)nLineCapSegments, 0.0, 0.0);
                    aLineCapList.realloc(nLineCapSegments);

                    for(sal_uInt32 a(0L); a < nLineCapSegments; a++)
                    {
                        const basegfx::B3DPoint aNext(aRot * aLast);
                        basegfx::B3DPolygon aNewPolygon;

                        aNewPolygon.append(aLast);
                        aNewPolygon.setNormal(0L, basegfx::B3DVector(aLast - aNull));

                        aNewPolygon.append(aNext);
                        aNewPolygon.setNormal(1L, basegfx::B3DVector(aNext - aNull));

                        aNewPolygon.append(aNull);
                        aNewPolygon.setNormal(2L, basegfx::B3DVector(-1.0, 0.0, 0.0));

                        aNewPolygon.setClosed(true);

                        const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                        const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, aLineMaterial, false));
                        aLineCapList[a] = xRef;

                        aLast = aNext;
                    }
                }

                return aLineCapList;
            }

            Primitive3DSequence getLineJoinSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial,
                double fAngle,
                double /*fDegreeStepWidth*/,
                double fMiterMinimumAngle,
                basegfx::B2DLineJoin aLineJoin)
            {
                // nSegments is for whole circle, adapt to half circle
                const sal_uInt32 nVerSeg(nSegments >> 1L);
                std::vector< BasePrimitive3D* > aResultVector;

                if(nVerSeg)
                {
                    if(basegfx::B2DLINEJOIN_ROUND == aLineJoin)
                    {
                        // calculate new horizontal segments
                        const sal_uInt32 nHorSeg((sal_uInt32)((fAngle / F_2PI) * (double)nSegments));

                        if(nHorSeg)
                        {
                            // create half-sphere
                            const basegfx::B3DPolyPolygon aSphere(basegfx::tools::createUnitSphereFillPolyPolygon(nHorSeg, nVerSeg, true, F_PI2, -F_PI2, 0.0, fAngle));

                            for(sal_uInt32 a(0L); a < aSphere.count(); a++)
                            {
                                const basegfx::B3DPolygon aPartPolygon(aSphere.getB3DPolygon(a));
                                const basegfx::B3DPolyPolygon aPartPolyPolygon(aPartPolygon);
                                BasePrimitive3D* pNew = new PolyPolygonMaterialPrimitive3D(aPartPolyPolygon, rMaterial, false);
                                aResultVector.push_back(pNew);
                            }
                        }
                        else
                        {
                            // fallback to bevel when there is not at least one segment hor and ver
                            aLineJoin = basegfx::B2DLINEJOIN_BEVEL;
                        }
                    }

                    if(basegfx::B2DLINEJOIN_MIDDLE == aLineJoin
                        || basegfx::B2DLINEJOIN_BEVEL == aLineJoin
                        || basegfx::B2DLINEJOIN_MITER == aLineJoin)
                    {
                        if(basegfx::B2DLINEJOIN_MITER == aLineJoin)
                        {
                            const double fMiterAngle(fAngle/2.0);

                            if(fMiterAngle < fMiterMinimumAngle)
                            {
                                // fallback to bevel when miter's angle is too small
                                aLineJoin = basegfx::B2DLINEJOIN_BEVEL;
                            }
                        }

                        const double fInc(F_PI / (double)nVerSeg);
                        const double fSin(sin(-fAngle));
                        const double fCos(cos(-fAngle));
                        const bool bMiter(basegfx::B2DLINEJOIN_MITER == aLineJoin);
                        const double fMiterSin(bMiter ? sin(-(fAngle/2.0)) : 0.0);
                        const double fMiterCos(bMiter ? cos(-(fAngle/2.0)) : 0.0);
                        double fPos(-F_PI2);
                        basegfx::B3DPoint aPointOnXY, aPointRotY, aNextPointOnXY, aNextPointRotY;
                        basegfx::B3DPoint aCurrMiter, aNextMiter;
                        basegfx::B3DPolygon aNewPolygon, aMiterPolygon;

                        // close polygon
                        aNewPolygon.setClosed(true);
                        aMiterPolygon.setClosed(true);

                        for(sal_uInt32 a(0L); a < nVerSeg; a++)
                        {
                            const bool bFirst(0L == a);
                            const bool bLast(a + 1L == nVerSeg);

                            if(bFirst || !bLast)
                            {
                                fPos += fInc;

                                aNextPointOnXY = basegfx::B3DPoint(
                                    cos(fPos),
                                    sin(fPos),
                                    0.0);

                                aNextPointRotY = basegfx::B3DPoint(
                                    aNextPointOnXY.getX() * fCos,
                                    aNextPointOnXY.getY(),
                                    aNextPointOnXY.getX() * fSin);

                                if(bMiter)
                                {
                                    aNextMiter = basegfx::B3DPoint(
                                        aNextPointOnXY.getX(),
                                        aNextPointOnXY.getY(),
                                        fMiterSin * (aNextPointOnXY.getX() / fMiterCos));
                                }
                            }

                            if(bFirst)
                            {
                                aNewPolygon.clear();

                                if(bMiter)
                                {
                                    aNewPolygon.append(basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aNewPolygon.append(aNextPointOnXY);
                                    aNewPolygon.append(aNextMiter);

                                    aMiterPolygon.clear();
                                    aMiterPolygon.append(basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aMiterPolygon.append(aNextMiter);
                                    aMiterPolygon.append(aNextPointRotY);
                                }
                                else
                                {
                                    aNewPolygon.append(basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aNewPolygon.append(aNextPointOnXY);
                                    aNewPolygon.append(aNextPointRotY);
                                }
                            }
                            else if(bLast)
                            {
                                aNewPolygon.clear();

                                if(bMiter)
                                {
                                    aNewPolygon.append(basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aNewPolygon.append(aCurrMiter);
                                    aNewPolygon.append(aPointOnXY);

                                    aMiterPolygon.clear();
                                    aMiterPolygon.append(basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aMiterPolygon.append(aPointRotY);
                                    aMiterPolygon.append(aCurrMiter);
                                }
                                else
                                {
                                    aNewPolygon.append(basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aNewPolygon.append(aPointRotY);
                                    aNewPolygon.append(aPointOnXY);
                                }
                            }
                            else
                            {
                                aNewPolygon.clear();

                                if(bMiter)
                                {
                                    aNewPolygon.append(aPointOnXY);
                                    aNewPolygon.append(aNextPointOnXY);
                                    aNewPolygon.append(aNextMiter);
                                    aNewPolygon.append(aCurrMiter);

                                    aMiterPolygon.clear();
                                    aMiterPolygon.append(aCurrMiter);
                                    aMiterPolygon.append(aNextMiter);
                                    aMiterPolygon.append(aNextPointRotY);
                                    aMiterPolygon.append(aPointRotY);
                                }
                                else
                                {
                                    aNewPolygon.append(aPointRotY);
                                    aNewPolygon.append(aPointOnXY);
                                    aNewPolygon.append(aNextPointOnXY);
                                    aNewPolygon.append(aNextPointRotY);
                                }
                            }

                            // set normals
                            for(sal_uInt32 b(0L); b < aNewPolygon.count(); b++)
                            {
                                aNewPolygon.setNormal(b, basegfx::B3DVector(aNewPolygon.getB3DPoint(b)));
                            }

                            // create primitive
                            if(aNewPolygon.count())
                            {
                                const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                                BasePrimitive3D* pNew = new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, rMaterial, false);
                                aResultVector.push_back(pNew);
                            }

                            if(bMiter && aMiterPolygon.count())
                            {
                                // set normals
                                for(sal_uInt32 c(0L); c < aMiterPolygon.count(); c++)
                                {
                                    aMiterPolygon.setNormal(c, basegfx::B3DVector(aMiterPolygon.getB3DPoint(c)));
                                }

                                // create primitive
                                const basegfx::B3DPolyPolygon aMiterPolyPolygon(aMiterPolygon);
                                BasePrimitive3D* pNew = new PolyPolygonMaterialPrimitive3D(aMiterPolyPolygon, rMaterial, false);
                                aResultVector.push_back(pNew);
                            }

                            // prepare next step
                            if(bFirst || !bLast)
                            {
                                aPointOnXY = aNextPointOnXY;
                                aPointRotY = aNextPointRotY;

                                if(bMiter)
                                {
                                    aCurrMiter = aNextMiter;
                                }
                            }
                        }
                    }
                }

                Primitive3DSequence aRetval(aResultVector.size());

                for(sal_uInt32 a(0L); a < aResultVector.size(); a++)
                {
                    aRetval[a] = Primitive3DReference(aResultVector[a]);
                }

                return aRetval;
            }

            basegfx::B3DHomMatrix getRotationFromVector(const basegfx::B3DVector& rVector)
            {
                // build transformation from unit vector to vector
                basegfx::B3DHomMatrix aRetval;

                // get applied rotations from angles in XY and in XZ (cartesian)
                const double fRotInXY(atan2(rVector.getY(), rVector.getXZLength()));
                const double fRotInXZ(atan2(-rVector.getZ(), rVector.getX()));

                // apply rotations. Rot around Z needs to be done first, so apply in two steps
                aRetval.rotate(0.0, 0.0, fRotInXY);
                aRetval.rotate(0.0, fRotInXZ, 0.0);

                return aRetval;
            }
        } // end of anonymous namespace
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        Primitive3DSequence PolygonTubePrimitive3D::impCreate3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            const sal_uInt32 nPointCount(getB3DPolygon().count());
            std::vector< BasePrimitive3D* > aResultVector;

            if(0L != nPointCount)
            {
                if(basegfx::fTools::more(getRadius(), 0.0))
                {
                    const attribute::MaterialAttribute3D aMaterial(getBColor());
                    static sal_uInt32 nSegments(8L); // default for 3d line segments, for more quality just raise this value (in even steps)
                    const bool bClosed(getB3DPolygon().isClosed());
                    const bool bNoLineJoin(basegfx::B2DLINEJOIN_NONE == getLineJoin());
                    const sal_uInt32 nLoopCount(bClosed ? nPointCount : nPointCount - 1L);
                    basegfx::B3DPoint aLast(getB3DPolygon().getB3DPoint(nPointCount - 1L));
                    basegfx::B3DPoint aCurr(getB3DPolygon().getB3DPoint(0L));

                    for(sal_uInt32 a(0L); a < nLoopCount; a++)
                    {
                        // get next data
                        const basegfx::B3DPoint aNext(getB3DPolygon().getB3DPoint((a + 1L) % nPointCount));
                        const basegfx::B3DVector aForw(aNext - aCurr);
                        const double fForwLen(aForw.getLength());

                        if(basegfx::fTools::more(fForwLen, 0.0))
                        {
                            // get rotation from vector, this describes rotation from (1, 0, 0) to aForw
                            basegfx::B3DHomMatrix aRotVector(getRotationFromVector(aForw));

                            // create default transformation with scale and rotate
                            basegfx::B3DHomMatrix aVectorTrans;
                            aVectorTrans.scale(fForwLen, getRadius(), getRadius());
                            aVectorTrans *= aRotVector;
                            aVectorTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                            if(bNoLineJoin || (!bClosed && !a))
                            {
                                // line start edge, build transformed primitiveVector3D
                                TransformPrimitive3D* pNewTransformedA = new TransformPrimitive3D(aVectorTrans, getLineCapSegments(nSegments, aMaterial));
                                aResultVector.push_back(pNewTransformedA);
                            }
                            else
                            {
                                const basegfx::B3DVector aBack(aCurr - aLast);
                                const double fCross(basegfx::cross(aBack, aForw).getLength());

                                if(!basegfx::fTools::equalZero(fCross))
                                {
                                    // line connect non-parallel, aBack, aForw, use getLineJoin()
                                    const double fAngle(acos(aBack.scalar(aForw) / (fForwLen * aBack.getLength()))); // 0.0 .. F_PI2
                                    Primitive3DSequence aNewList(getLineJoinSegments(nSegments, aMaterial, fAngle, getDegreeStepWidth(), getMiterMinimumAngle(), getLineJoin()));

                                    // calculate transformation. First, get angle in YZ between nForw projected on (1, 0, 0) and nBack
                                    basegfx::B3DHomMatrix aInvRotVector(aRotVector);
                                    aInvRotVector.invert();
                                    basegfx::B3DVector aTransBack(aInvRotVector * aBack);
                                    const double fRotInYZ(atan2(aTransBack.getY(), aTransBack.getZ()));

                                    // create trans by rotating unit sphere with angle 90 degrees around Y, then 180-fRot in X.
                                    // Also apply usual scaling and translation
                                    basegfx::B3DHomMatrix aSphereTrans;
                                    aSphereTrans.rotate(0.0, F_PI2, 0.0);
                                    aSphereTrans.rotate(F_PI - fRotInYZ, 0.0, 0.0);
                                    aSphereTrans *= aRotVector;
                                    aSphereTrans.scale(getRadius(), getRadius(), getRadius());
                                    aSphereTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                                    // line start edge, build transformed primitiveVector3D
                                    TransformPrimitive3D* pNewTransformedB = new TransformPrimitive3D(aSphereTrans, aNewList);
                                    aResultVector.push_back(pNewTransformedB);
                                }
                            }

                            // create line segments, build transformed primitiveVector3D
                            TransformPrimitive3D* pNewTransformedC = new TransformPrimitive3D(aVectorTrans, getLineTubeSegments(nSegments, aMaterial));
                            aResultVector.push_back(pNewTransformedC);

                            if(bNoLineJoin || (!bClosed && ((a + 1L) == nLoopCount)))
                            {
                                // line end edge, first rotate (mirror) and translate, then use use aRotVector
                                basegfx::B3DHomMatrix aBackTrans;
                                aBackTrans.rotate(0.0, F_PI, 0.0);
                                aBackTrans.translate(1.0, 0.0, 0.0);
                                aBackTrans.scale(fForwLen, getRadius(), getRadius());
                                aBackTrans *= aRotVector;
                                aBackTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                                // line end edge, build transformed primitiveVector3D
                                TransformPrimitive3D* pNewTransformedD = new TransformPrimitive3D(aBackTrans, getLineCapSegments(nSegments, aMaterial));
                                aResultVector.push_back(pNewTransformedD);
                            }
                        }

                        // prepare next loop step
                        aLast = aCurr;
                        aCurr = aNext;
                    }
                }
                else
                {
                    // create hairline
                    PolygonHairlinePrimitive3D* pNew = new PolygonHairlinePrimitive3D(getB3DPolygon(), getBColor());
                    aResultVector.push_back(pNew);
                }
            }

            // prepare return value
            Primitive3DSequence aRetval(aResultVector.size());

            for(sal_uInt32 a(0L); a < aResultVector.size(); a++)
            {
                aRetval[a] = Primitive3DReference(aResultVector[a]);
            }

            return aRetval;
        }

        PolygonTubePrimitive3D::PolygonTubePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const basegfx::BColor& rBColor,
            double fRadius, basegfx::B2DLineJoin aLineJoin,
            double fDegreeStepWidth,
            double fMiterMinimumAngle)
        :   PolygonHairlinePrimitive3D(rPolygon, rBColor),
            maLast3DDecomposition(),
            mfRadius(fRadius),
            mfDegreeStepWidth(fDegreeStepWidth),
            mfMiterMinimumAngle(fMiterMinimumAngle),
            maLineJoin(aLineJoin)
        {
        }

        bool PolygonTubePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(PolygonHairlinePrimitive3D::operator==(rPrimitive))
            {
                const PolygonTubePrimitive3D& rCompare = (PolygonTubePrimitive3D&)rPrimitive;

                return (getRadius() == rCompare.getRadius()
                    && getDegreeStepWidth() == rCompare.getDegreeStepWidth()
                    && getMiterMinimumAngle() == rCompare.getMiterMinimumAngle()
                    && getLineJoin() == rCompare.getLineJoin());
            }

            return false;
        }

        Primitive3DSequence PolygonTubePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(!getLast3DDecomposition().hasElements())
            {
                const Primitive3DSequence aNewSequence(impCreate3DDecomposition(rViewInformation));
                const_cast< PolygonTubePrimitive3D* >(this)->setLast3DDecomposition(aNewSequence);
            }

            return getLast3DDecomposition();
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(PolygonTubePrimitive3D, PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
