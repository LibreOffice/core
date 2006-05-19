/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polygontubeprimitive3d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2006-05-19 09:34:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYGONTUBEPRIMITIVE3D_HXX
#include <drawinglayer/primitive3d/polygontubeprimitive3d.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_MATERIALATTRIBUTE3D_HXX
#include <drawinglayer/primitive3d/materialattribute3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE3D_POLYPOLYGONPRIMITIVE_HXX
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#endif

#ifndef _DRAWINGLAYER_PRIMITIVE_PRIMITIVELIST_HXX
#include <drawinglayer/primitive/primitivelist.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B3DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        namespace // anonymous namespace
        {
            const primitiveList& getLineTubeSegments(sal_uInt32 nSegments, const materialAttribute3D& rMaterial)
            {
                // static data for buffered tube primitives
                static primitiveList aLineTubeList;
                static sal_uInt32 nLineTubeSegments(0L);
                static materialAttribute3D aLineMaterial;

                // may exclusively change static data, use mutex
                ::osl::Mutex m_mutex;

                if(nSegments != nLineTubeSegments || rMaterial != aLineMaterial)
                {
                    nLineTubeSegments = nSegments;
                    aLineMaterial = rMaterial;
                    aLineTubeList.clear();
                }

                if(0L == aLineTubeList.count() && 0L != nLineTubeSegments)
                {
                    const ::basegfx::B3DPoint aLeft(0.0, 0.0, 0.0);
                    const ::basegfx::B3DPoint aRight(1.0, 0.0, 0.0);
                    ::basegfx::B3DPoint aLastLeft(0.0, 1.0, 0.0);
                    ::basegfx::B3DPoint aLastRight(1.0, 1.0, 0.0);
                    ::basegfx::B3DHomMatrix aRot;
                    aRot.rotate(F_2PI / (double)nLineTubeSegments, 0.0, 0.0);

                    for(sal_uInt32 a(0L); a < nLineTubeSegments; a++)
                    {
                        const ::basegfx::B3DPoint aNextLeft(aRot * aLastLeft);
                        const ::basegfx::B3DPoint aNextRight(aRot * aLastRight);
                        ::basegfx::B3DPolygon aNew;

                        aNew.append(aNextLeft);
                        aNew.setNormal(0L, ::basegfx::B3DVector(aNextLeft - aLeft));

                        aNew.append(aLastLeft);
                        aNew.setNormal(1L, ::basegfx::B3DVector(aLastLeft - aLeft));

                        aNew.append(aLastRight);
                        aNew.setNormal(2L, ::basegfx::B3DVector(aLastRight - aRight));

                        aNew.append(aNextRight);
                        aNew.setNormal(3L, ::basegfx::B3DVector(aNextRight - aRight));

                        aNew.setClosed(true);

                        basePrimitive* pNew = new polyPolygonMaterialPrimitive3D(::basegfx::B3DPolyPolygon(aNew), aLineMaterial, false);
                        aLineTubeList.append(referencedPrimitive(*pNew));

                        aLastLeft = aNextLeft;
                        aLastRight = aNextRight;
                    }
                }

                return aLineTubeList;
            }

            const primitiveList& getLineCapSegments(sal_uInt32 nSegments, const materialAttribute3D& rMaterial)
            {
                // static data for buffered tube primitives
                static primitiveList aLineCapList;
                static sal_uInt32 nLineCapSegments(0L);
                static materialAttribute3D aLineMaterial;

                // may exclusively change static data, use mutex
                ::osl::Mutex m_mutex;

                if(nSegments != nLineCapSegments || rMaterial != aLineMaterial)
                {
                    nLineCapSegments = nSegments;
                    aLineMaterial = rMaterial;
                    aLineCapList.clear();
                }

                if(0L == aLineCapList.count() && 0L != nLineCapSegments)
                {
                    const ::basegfx::B3DPoint aNull(0.0, 0.0, 0.0);
                    ::basegfx::B3DPoint aLast(0.0, 1.0, 0.0);
                    ::basegfx::B3DHomMatrix aRot;
                    aRot.rotate(F_2PI / (double)nLineCapSegments, 0.0, 0.0);

                    for(sal_uInt32 a(0L); a < nLineCapSegments; a++)
                    {
                        const ::basegfx::B3DPoint aNext(aRot * aLast);
                        ::basegfx::B3DPolygon aNew;

                        aNew.append(aLast);
                        aNew.setNormal(0L, ::basegfx::B3DVector(aLast - aNull));

                        aNew.append(aNext);
                        aNew.setNormal(1L, ::basegfx::B3DVector(aNext - aNull));

                        aNew.append(aNull);
                        aNew.setNormal(2L, ::basegfx::B3DVector(-1.0, 0.0, 0.0));

                        aNew.setClosed(true);

                        basePrimitive* pNew = new polyPolygonMaterialPrimitive3D(::basegfx::B3DPolyPolygon(aNew), aLineMaterial, false);
                        aLineCapList.append(referencedPrimitive(*pNew));

                        aLast = aNext;
                    }
                }

                return aLineCapList;
            }

            void getLineJoinSegments(primitiveList& rDest, sal_uInt32 nSegments, const materialAttribute3D& rMaterial, double fAngle,
                double fDegreeStepWidth, double fMiterMinimumAngle, ::basegfx::tools::B2DLineJoin aLineJoin)
            {
                // nSegments is for whole circle, adapt to half circle
                const sal_uInt32 nVerSeg(nSegments >> 1L);

                if(nVerSeg)
                {
                    if(::basegfx::tools::B2DLINEJOIN_ROUND == aLineJoin)
                    {
                        // calculate new horizontal segments
                        const sal_uInt32 nHorSeg((sal_uInt32)((fAngle / F_2PI) * (double)nSegments));

                        if(nHorSeg)
                        {
                            // create half-sphere
                            const ::basegfx::B3DPolyPolygon aSphere(::basegfx::tools::createUnitSphereFillPolyPolygon(nHorSeg, nVerSeg, true, F_PI2, -F_PI2, 0.0, fAngle));

                            for(sal_uInt32 a(0L); a < aSphere.count(); a++)
                            {
                                basePrimitive* pNew = new polyPolygonMaterialPrimitive3D(::basegfx::B3DPolyPolygon(aSphere.getB3DPolygon(a)), rMaterial, false);
                                rDest.append(referencedPrimitive(*pNew));
                            }
                        }
                        else
                        {
                            // fallback to bevel when there is not at least one segment hor and ver
                            aLineJoin = ::basegfx::tools::B2DLINEJOIN_BEVEL;
                        }
                    }

                    if(::basegfx::tools::B2DLINEJOIN_MIDDLE == aLineJoin
                        || ::basegfx::tools::B2DLINEJOIN_BEVEL == aLineJoin
                        || ::basegfx::tools::B2DLINEJOIN_MITER == aLineJoin)
                    {
                        if(::basegfx::tools::B2DLINEJOIN_MITER == aLineJoin)
                        {
                            const double fMiterAngle(fAngle/2.0);

                            if(fMiterAngle < fMiterMinimumAngle)
                            {
                                // fallback to bevel when miter's angle is too small
                                aLineJoin = ::basegfx::tools::B2DLINEJOIN_BEVEL;
                            }
                        }

                        const double fInc(F_PI / (double)nVerSeg);
                        const double fSin(sin(-fAngle));
                        const double fCos(cos(-fAngle));
                        const bool bMiter(::basegfx::tools::B2DLINEJOIN_MITER == aLineJoin);
                        const double fMiterSin(bMiter ? sin(-(fAngle/2.0)) : 0.0);
                        const double fMiterCos(bMiter ? cos(-(fAngle/2.0)) : 0.0);
                        double fPos(-F_PI2);
                        ::basegfx::B3DPoint aPointOnXY, aPointRotY, aNextPointOnXY, aNextPointRotY;
                        ::basegfx::B3DPoint aCurrMiter, aNextMiter;
                        ::basegfx::B3DPolygon aNew, aMiter;

                        // close polygon
                        aNew.setClosed(true);
                        aMiter.setClosed(true);

                        for(sal_uInt32 a(0L); a < nVerSeg; a++)
                        {
                            const bool bFirst(0L == a);
                            const bool bLast(a + 1L == nVerSeg);

                            if(bFirst || !bLast)
                            {
                                fPos += fInc;

                                aNextPointOnXY = ::basegfx::B3DPoint(
                                    cos(fPos),
                                    sin(fPos),
                                    0.0);

                                aNextPointRotY = ::basegfx::B3DPoint(
                                    aNextPointOnXY.getX() * fCos,
                                    aNextPointOnXY.getY(),
                                    aNextPointOnXY.getX() * fSin);

                                if(bMiter)
                                {
                                    aNextMiter = ::basegfx::B3DPoint(
                                        aNextPointOnXY.getX(),
                                        aNextPointOnXY.getY(),
                                        fMiterSin * (aNextPointOnXY.getX() / fMiterCos));
                                }
                            }

                            if(bFirst)
                            {
                                aNew.clear();

                                if(bMiter)
                                {
                                    aNew.append(::basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aNew.append(aNextPointOnXY);
                                    aNew.append(aNextMiter);

                                    aMiter.clear();
                                    aMiter.append(::basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aMiter.append(aNextMiter);
                                    aMiter.append(aNextPointRotY);
                                }
                                else
                                {
                                    aNew.append(::basegfx::B3DPoint(0.0, -1.0, 0.0));
                                    aNew.append(aNextPointOnXY);
                                    aNew.append(aNextPointRotY);
                                }
                            }
                            else if(bLast)
                            {
                                aNew.clear();

                                if(bMiter)
                                {
                                    aNew.append(::basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aNew.append(aCurrMiter);
                                    aNew.append(aPointOnXY);

                                    aMiter.clear();
                                    aMiter.append(::basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aMiter.append(aPointRotY);
                                    aMiter.append(aCurrMiter);
                                }
                                else
                                {
                                    aNew.append(::basegfx::B3DPoint(0.0, 1.0, 0.0));
                                    aNew.append(aPointRotY);
                                    aNew.append(aPointOnXY);
                                }
                            }
                            else
                            {
                                aNew.clear();

                                if(bMiter)
                                {
                                    aNew.append(aPointOnXY);
                                    aNew.append(aNextPointOnXY);
                                    aNew.append(aNextMiter);
                                    aNew.append(aCurrMiter);

                                    aMiter.clear();
                                    aMiter.append(aCurrMiter);
                                    aMiter.append(aNextMiter);
                                    aMiter.append(aNextPointRotY);
                                    aMiter.append(aPointRotY);
                                }
                                else
                                {
                                    aNew.append(aPointRotY);
                                    aNew.append(aPointOnXY);
                                    aNew.append(aNextPointOnXY);
                                    aNew.append(aNextPointRotY);
                                }
                            }

                            // set normals
                            for(sal_uInt32 b(0L); b < aNew.count(); b++)
                            {
                                aNew.setNormal(b, ::basegfx::B3DVector(aNew.getB3DPoint(b)));
                            }

                            // create primitive
                            if(aNew.count())
                            {
                                basePrimitive* pNew = new polyPolygonMaterialPrimitive3D(::basegfx::B3DPolyPolygon(aNew), rMaterial, false);
                                rDest.append(referencedPrimitive(*pNew));
                            }

                            if(bMiter && aMiter.count())
                            {
                                // set normals
                                for(sal_uInt32 c(0L); c < aMiter.count(); c++)
                                {
                                    aMiter.setNormal(c, ::basegfx::B3DVector(aMiter.getB3DPoint(c)));
                                }

                                // create primitive
                                basePrimitive* pNew = new polyPolygonMaterialPrimitive3D(::basegfx::B3DPolyPolygon(aMiter), rMaterial, false);
                                rDest.append(referencedPrimitive(*pNew));
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
            }

            ::basegfx::B3DHomMatrix getRotationFromVector(const ::basegfx::B3DVector& rVector)
            {
                // build transformation from unit vector to vector
                ::basegfx::B3DHomMatrix aRetval;

                // get applied rotations from angles in XY and in XZ (cartesian)
                const double fRotInXY(atan2(rVector.getY(), rVector.getXZLength()));
                const double fRotInXZ(atan2(-rVector.getZ(), rVector.getX()));

                // apply rotations. Rot around Z needs to be done first, so apply in two steps
                aRetval.rotate(0.0, 0.0, fRotInXY);
                aRetval.rotate(0.0, fRotInXZ, 0.0);

                return aRetval;
            }
        } // end of anonymous namespace
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive
    {
        void polygonTubePrimitive3D::decompose(primitiveList& rTarget, const ::drawinglayer::geometry::viewInformation& rViewInformation)
        {
            const sal_uInt32 nPointCount(maPolygon.count());

            if(0L != nPointCount)
            {
                if(::basegfx::fTools::more(mfRadius, 0.0))
                {
                    const materialAttribute3D aMaterial(maBColor);
                    static sal_uInt32 nSegments(8L); // default for 3d line segments, for more quality just raise this value (in even steps)
                    const bool bClosed(maPolygon.isClosed());
                    const bool bNoLineJoin(::basegfx::tools::B2DLINEJOIN_NONE == maLineJoin);
                    const sal_uInt32 nLoopCount(bClosed ? nPointCount : nPointCount - 1L);
                    ::basegfx::B3DPoint aLast(maPolygon.getB3DPoint(nPointCount - 1L));
                    ::basegfx::B3DPoint aCurr(maPolygon.getB3DPoint(0L));

                    for(sal_uInt32 a(0L); a < nLoopCount; a++)
                    {
                        // get next data
                        const ::basegfx::B3DPoint aNext(maPolygon.getB3DPoint((a + 1L) % nPointCount));
                        const ::basegfx::B3DVector aForw(aNext - aCurr);
                        const double fForwLen(aForw.getLength());
                        primitiveList aNewList;

                        if(::basegfx::fTools::more(fForwLen, 0.0))
                        {
                            // get rotation from vector, this describes rotation from (1, 0, 0) to aForw
                            ::basegfx::B3DHomMatrix aRotVector(getRotationFromVector(aForw));

                            // create default transformation with scale and rotate
                            ::basegfx::B3DHomMatrix aVectorTrans;
                            aVectorTrans.scale(fForwLen, mfRadius, mfRadius);
                            aVectorTrans *= aRotVector;
                            aVectorTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                            if(bNoLineJoin || (!bClosed && !a))
                            {
                                // line start edge
                                aNewList = getLineCapSegments(nSegments, aMaterial);
                                aNewList.transform(aVectorTrans);
                                rTarget.append(aNewList);
                            }
                            else
                            {
                                const ::basegfx::B3DVector aBack(aCurr - aLast);
                                const double fCross(::basegfx::cross(aBack, aForw).getLength());

                                if(!::basegfx::fTools::equalZero(fCross))
                                {
                                    // line connect non-parallel, aBack, aForw, use maLineJoin
                                    const double fAngle(acos(aBack.scalar(aForw) / (fForwLen * aBack.getLength()))); // 0.0 .. F_PI2
                                    aNewList.clear();
                                    getLineJoinSegments(aNewList, nSegments, aMaterial, fAngle, mfDegreeStepWidth, mfMiterMinimumAngle, maLineJoin);

                                    // calculate transformation. First, get angle in YZ between nForw projected on (1, 0, 0) and nBack
                                    ::basegfx::B3DHomMatrix aInvRotVector(aRotVector);
                                    aInvRotVector.invert();
                                    ::basegfx::B3DVector aTransBack(aInvRotVector * aBack);
                                    const double fRotInYZ(atan2(aTransBack.getY(), aTransBack.getZ()));

                                    // create trans by rotating unit sphere with angle 90 degrees around Y, then 180-fRot in X.
                                    // Also apply usual scaling and translation
                                    ::basegfx::B3DHomMatrix aSphereTrans;
                                    aSphereTrans.rotate(0.0, F_PI2, 0.0);
                                    aSphereTrans.rotate(F_PI - fRotInYZ, 0.0, 0.0);
                                    aSphereTrans *= aRotVector;
                                    aSphereTrans.scale(mfRadius, mfRadius, mfRadius);
                                    aSphereTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                                    // apply to list and append
                                    aNewList.transform(aSphereTrans);
                                    rTarget.append(aNewList);
                                }
                            }

                            // create line segments
                            aNewList = getLineTubeSegments(nSegments, aMaterial);
                            aNewList.transform(aVectorTrans);
                            rTarget.append(aNewList);

                            if(bNoLineJoin || (!bClosed && ((a + 1L) == nLoopCount)))
                            {
                                // line end edge, first rotate (mirror) and translate, then use use aRotVector
                                ::basegfx::B3DHomMatrix aBackTrans;
                                aBackTrans.rotate(0.0, F_PI, 0.0);
                                aBackTrans.translate(1.0, 0.0, 0.0);
                                aBackTrans.scale(fForwLen, mfRadius, mfRadius);
                                aBackTrans *= aRotVector;
                                aBackTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                                aNewList = getLineCapSegments(nSegments, aMaterial);
                                aNewList.transform(aBackTrans);
                                rTarget.append(aNewList);
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
                    polygonHairlinePrimitive3D* pNew = new polygonHairlinePrimitive3D(maPolygon, maBColor);
                    rTarget.append(referencedPrimitive(*pNew));
                }
            }
        }

        polygonTubePrimitive3D::polygonTubePrimitive3D(
            const ::basegfx::B3DPolygon& rPolygon,
            const ::basegfx::BColor& rBColor,
            double fRadius, ::basegfx::tools::B2DLineJoin aLineJoin,
            double fDegreeStepWidth,
            double fMiterMinimumAngle)
        :   polygonHairlinePrimitive3D(rPolygon, rBColor),
            mfRadius(fRadius),
            mfDegreeStepWidth(fDegreeStepWidth),
            mfMiterMinimumAngle(fMiterMinimumAngle),
            maLineJoin(aLineJoin)
        {
        }

        polygonTubePrimitive3D::~polygonTubePrimitive3D()
        {
        }

        bool polygonTubePrimitive3D::operator==(const basePrimitive& rPrimitive) const
        {
            if(polygonHairlinePrimitive3D::operator==(rPrimitive))
            {
                const polygonTubePrimitive3D& rCompare = (polygonTubePrimitive3D&)rPrimitive;

                return (mfRadius == rCompare.mfRadius
                    && mfDegreeStepWidth == rCompare.mfDegreeStepWidth
                    && mfMiterMinimumAngle == rCompare.mfMiterMinimumAngle
                    && maLineJoin == rCompare.maLineJoin);
            }

            return false;
        }

        basePrimitive* polygonTubePrimitive3D::createNewClone() const
        {
            return new polygonTubePrimitive3D(maPolygon, maBColor, mfRadius, maLineJoin, mfDegreeStepWidth, mfMiterMinimumAngle);
        }

        PrimitiveID polygonTubePrimitive3D::getID() const
        {
            return CreatePrimitiveID('T', 'U', 'B', '3');
        }
    } // end of namespace primitive
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
