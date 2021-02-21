/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <primitive3d/polygontubeprimitive3d.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <drawinglayer/primitive3d/polypolygonprimitive3d.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer::primitive3d
{
        namespace // anonymous namespace
        {
            class TubeBuffer
            {
            private:
                // data for buffered tube primitives
                Primitive3DContainer m_aLineTubeList;
                sal_uInt32 m_nLineTubeSegments;
                attribute::MaterialAttribute3D m_aLineMaterial;
                ::osl::Mutex m_aMutex;
            public:
                TubeBuffer()
                    : m_nLineTubeSegments(0)
                {
                }

                TubeBuffer(const TubeBuffer&) = delete;
                const TubeBuffer& operator=(const TubeBuffer&) = delete;

                Primitive3DContainer getLineTubeSegments(
                    sal_uInt32 nSegments,
                    const attribute::MaterialAttribute3D& rMaterial)
                {
                    // may exclusively change cached data, use mutex
                    ::osl::MutexGuard aGuard(m_aMutex);

                    if (nSegments != m_nLineTubeSegments || !(rMaterial == m_aLineMaterial))
                    {
                        m_nLineTubeSegments = nSegments;
                        m_aLineMaterial = rMaterial;
                        m_aLineTubeList = Primitive3DContainer();
                    }

                    if (m_aLineTubeList.empty() && m_nLineTubeSegments != 0)
                    {
                        const basegfx::B3DPoint aLeft(0.0, 0.0, 0.0);
                        const basegfx::B3DPoint aRight(1.0, 0.0, 0.0);
                        basegfx::B3DPoint aLastLeft(0.0, 1.0, 0.0);
                        basegfx::B3DPoint aLastRight(1.0, 1.0, 0.0);
                        basegfx::B3DHomMatrix aRot;
                        aRot.rotate(F_2PI / static_cast<double>(m_nLineTubeSegments), 0.0, 0.0);
                        m_aLineTubeList.resize(m_nLineTubeSegments);

                        for(sal_uInt32 a = 0; a < m_nLineTubeSegments; ++a)
                        {
                            const basegfx::B3DPoint aNextLeft(aRot * aLastLeft);
                            const basegfx::B3DPoint aNextRight(aRot * aLastRight);
                            basegfx::B3DPolygon aNewPolygon;

                            aNewPolygon.append(aNextLeft);
                            aNewPolygon.setNormal(0, basegfx::B3DVector(aNextLeft - aLeft));

                            aNewPolygon.append(aLastLeft);
                            aNewPolygon.setNormal(1, basegfx::B3DVector(aLastLeft - aLeft));

                            aNewPolygon.append(aLastRight);
                            aNewPolygon.setNormal(2, basegfx::B3DVector(aLastRight - aRight));

                            aNewPolygon.append(aNextRight);
                            aNewPolygon.setNormal(3, basegfx::B3DVector(aNextRight - aRight));

                            aNewPolygon.setClosed(true);

                            const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                            const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, m_aLineMaterial, false));
                            m_aLineTubeList[a] = xRef;

                            aLastLeft = aNextLeft;
                            aLastRight = aNextRight;
                        }
                    }
                    return m_aLineTubeList;
                }
            };

            struct theTubeBuffer :
                public rtl::Static< TubeBuffer, theTubeBuffer > {};

            Primitive3DContainer getLineTubeSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial)
            {
                // static data for buffered tube primitives
                TubeBuffer &rTheBuffer = theTubeBuffer::get();
                return rTheBuffer.getLineTubeSegments(nSegments, rMaterial);
            }

            class CapBuffer
            {
            private:
                // data for buffered cap primitives
                Primitive3DContainer m_aLineCapList;
                sal_uInt32 m_nLineCapSegments;
                attribute::MaterialAttribute3D m_aLineMaterial;
                ::osl::Mutex m_aMutex;
            public:
                CapBuffer()
                    : m_nLineCapSegments(0)
                {
                }
                CapBuffer(const CapBuffer&) = delete;
                const CapBuffer& operator=(const CapBuffer&) = delete;

                Primitive3DContainer getLineCapSegments(
                    sal_uInt32 nSegments,
                    const attribute::MaterialAttribute3D& rMaterial)
                {
                    // may exclusively change cached data, use mutex
                    ::osl::MutexGuard aGuard(m_aMutex);

                    if (nSegments != m_nLineCapSegments || !(rMaterial == m_aLineMaterial))
                    {
                        m_nLineCapSegments = nSegments;
                        m_aLineMaterial = rMaterial;
                        m_aLineCapList = Primitive3DContainer();
                    }

                    if (m_aLineCapList.empty() && m_nLineCapSegments != 0)
                    {
                        const basegfx::B3DPoint aNull(0.0, 0.0, 0.0);
                        basegfx::B3DPoint aLast(0.0, 1.0, 0.0);
                        basegfx::B3DHomMatrix aRot;
                        aRot.rotate(F_2PI / static_cast<double>(m_nLineCapSegments), 0.0, 0.0);
                        m_aLineCapList.resize(m_nLineCapSegments);

                        for(sal_uInt32 a = 0; a < m_nLineCapSegments; ++a)
                        {
                            const basegfx::B3DPoint aNext(aRot * aLast);
                            basegfx::B3DPolygon aNewPolygon;

                            aNewPolygon.append(aLast);
                            aNewPolygon.setNormal(0, basegfx::B3DVector(aLast - aNull));

                            aNewPolygon.append(aNext);
                            aNewPolygon.setNormal(1, basegfx::B3DVector(aNext - aNull));

                            aNewPolygon.append(aNull);
                            aNewPolygon.setNormal(2, basegfx::B3DVector(-1.0, 0.0, 0.0));

                            aNewPolygon.setClosed(true);

                            const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                            const Primitive3DReference xRef(new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, m_aLineMaterial, false));
                            m_aLineCapList[a] = xRef;

                            aLast = aNext;
                        }
                    }

                    return m_aLineCapList;
                }
            };

            struct theCapBuffer :
                public rtl::Static< CapBuffer, theCapBuffer > {};

            Primitive3DContainer getLineCapSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial)
            {
                // static data for buffered cap primitives
                CapBuffer &rTheBuffer = theCapBuffer::get();
                return rTheBuffer.getLineCapSegments(nSegments, rMaterial);
            }

            class CapRoundBuffer
            {
            private:
                // data for buffered capround primitives
                Primitive3DContainer m_aLineCapRoundList;
                sal_uInt32 m_nLineCapRoundSegments;
                attribute::MaterialAttribute3D m_aLineMaterial;
                ::osl::Mutex m_aMutex;
            public:
                CapRoundBuffer()
                    : m_nLineCapRoundSegments(0)
                {
                }
                CapRoundBuffer(const CapRoundBuffer&) = delete;
                const CapRoundBuffer& operator=(const CapRoundBuffer&) = delete;

                Primitive3DContainer getLineCapRoundSegments(
                    sal_uInt32 nSegments,
                    const attribute::MaterialAttribute3D& rMaterial)
                {
                    // may exclusively change cached data, use mutex
                    ::osl::MutexGuard aGuard(m_aMutex);

                    if (nSegments != m_nLineCapRoundSegments || !(rMaterial == m_aLineMaterial))
                    {
                        m_nLineCapRoundSegments = nSegments;
                        m_aLineMaterial = rMaterial;
                        m_aLineCapRoundList = Primitive3DContainer();
                    }

                    if (m_aLineCapRoundList.empty() && m_nLineCapRoundSegments)
                    {
                        // calculate new horizontal segments
                        sal_uInt32 nVerSeg(nSegments / 2);

                        if (nVerSeg < 1)
                        {
                            nVerSeg = 1;
                        }

                        // create half-sphere; upper half of unit sphere
                        basegfx::B3DPolyPolygon aSphere(
                            basegfx::utils::createUnitSphereFillPolyPolygon(
                                nSegments,
                                nVerSeg,
                                true,
                                F_PI2, 0.0,
                                0.0, F_2PI));
                        const sal_uInt32 nCount(aSphere.count());

                        if (nCount)
                        {
                            // rotate to have sphere cap oriented to negative X-Axis; do not
                            // forget to transform normals, too
                            basegfx::B3DHomMatrix aSphereTrans;

                            aSphereTrans.rotate(0.0, 0.0, F_PI2);
                            aSphere.transform(aSphereTrans);
                            aSphere.transformNormals(aSphereTrans);

                            // realloc for primitives and create based on polygon snippets
                            m_aLineCapRoundList.resize(nCount);

                            for (sal_uInt32 a = 0; a < nCount; ++a)
                            {
                                const basegfx::B3DPolygon& aPartPolygon(aSphere.getB3DPolygon(a));
                                const basegfx::B3DPolyPolygon aPartPolyPolygon(aPartPolygon);

                                // need to create one primitive per Polygon since the primitive
                                // is for planar PolyPolygons which is definitely not the case here
                                m_aLineCapRoundList[a] = new PolyPolygonMaterialPrimitive3D(
                                    aPartPolyPolygon,
                                    rMaterial,
                                    false);
                            }
                        }
                    }

                    return m_aLineCapRoundList;
                }

            };

            struct theCapRoundBuffer :
                public rtl::Static< CapRoundBuffer, theCapRoundBuffer > {};


            Primitive3DContainer getLineCapRoundSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial)
            {
                // static data for buffered cap primitives
                CapRoundBuffer &rTheBuffer = theCapRoundBuffer::get();
                return rTheBuffer.getLineCapRoundSegments(nSegments, rMaterial);
            }

            Primitive3DContainer getLineJoinSegments(
                sal_uInt32 nSegments,
                const attribute::MaterialAttribute3D& rMaterial,
                double fAngle,
                double fMiterMinimumAngle,
                basegfx::B2DLineJoin aLineJoin)
            {
                // nSegments is for whole circle, adapt to half circle
                const sal_uInt32 nVerSeg(nSegments >> 1);
                std::vector< BasePrimitive3D* > aResultVector;

                if(nVerSeg)
                {
                    if(basegfx::B2DLineJoin::Round == aLineJoin)
                    {
                        // calculate new horizontal segments
                        const sal_uInt32 nHorSeg(basegfx::fround((fAngle / F_2PI) * static_cast<double>(nSegments)));

                        if(nHorSeg)
                        {
                            // create half-sphere
                            const basegfx::B3DPolyPolygon aSphere(basegfx::utils::createUnitSphereFillPolyPolygon(nHorSeg, nVerSeg, true, F_PI2, -F_PI2, 0.0, fAngle));

                            for(sal_uInt32 a(0); a < aSphere.count(); a++)
                            {
                                const basegfx::B3DPolygon& aPartPolygon(aSphere.getB3DPolygon(a));
                                const basegfx::B3DPolyPolygon aPartPolyPolygon(aPartPolygon);
                                aResultVector.push_back(new PolyPolygonMaterialPrimitive3D(aPartPolyPolygon, rMaterial, false));
                            }
                        }
                        else
                        {
                            // fallback to bevel when there is not at least one segment hor and ver
                            aLineJoin = basegfx::B2DLineJoin::Bevel;
                        }
                    }

                    if (basegfx::B2DLineJoin::Bevel == aLineJoin ||
                        basegfx::B2DLineJoin::Miter == aLineJoin)
                    {
                        if(basegfx::B2DLineJoin::Miter == aLineJoin)
                        {
                            const double fMiterAngle(fAngle/2.0);

                            if(fMiterAngle < fMiterMinimumAngle)
                            {
                                // fallback to bevel when miter's angle is too small
                                aLineJoin = basegfx::B2DLineJoin::Bevel;
                            }
                        }

                        const double fInc(F_PI / static_cast<double>(nVerSeg));
                        const double fSin(sin(-fAngle));
                        const double fCos(cos(-fAngle));
                        const bool bMiter(basegfx::B2DLineJoin::Miter == aLineJoin);
                        const double fMiterSin(bMiter ? sin(-(fAngle/2.0)) : 0.0);
                        const double fMiterCos(bMiter ? cos(-(fAngle/2.0)) : 0.0);
                        double fPos(-F_PI2);
                        basegfx::B3DPoint aPointOnXY, aPointRotY, aNextPointOnXY, aNextPointRotY;
                        basegfx::B3DPoint aCurrMiter, aNextMiter;
                        basegfx::B3DPolygon aNewPolygon, aMiterPolygon;

                        // close polygon
                        aNewPolygon.setClosed(true);
                        aMiterPolygon.setClosed(true);

                        for(sal_uInt32 a(0); a < nVerSeg; a++)
                        {
                            const bool bFirst(0 == a);
                            const bool bLast(a + 1 == nVerSeg);

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
                            for(sal_uInt32 b(0); b < aNewPolygon.count(); b++)
                            {
                                aNewPolygon.setNormal(b, basegfx::B3DVector(aNewPolygon.getB3DPoint(b)));
                            }

                            // create primitive
                            if(aNewPolygon.count())
                            {
                                const basegfx::B3DPolyPolygon aNewPolyPolygon(aNewPolygon);
                                aResultVector.push_back(new PolyPolygonMaterialPrimitive3D(aNewPolyPolygon, rMaterial, false));
                            }

                            if(bMiter && aMiterPolygon.count())
                            {
                                // set normals
                                for(sal_uInt32 c(0); c < aMiterPolygon.count(); c++)
                                {
                                    aMiterPolygon.setNormal(c, basegfx::B3DVector(aMiterPolygon.getB3DPoint(c)));
                                }

                                // create primitive
                                const basegfx::B3DPolyPolygon aMiterPolyPolygon(aMiterPolygon);
                                aResultVector.push_back(new PolyPolygonMaterialPrimitive3D(aMiterPolyPolygon, rMaterial, false));
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

                Primitive3DContainer aRetval(aResultVector.size());

                for(size_t a(0); a < aResultVector.size(); a++)
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


using namespace com::sun::star;

        Primitive3DContainer PolygonTubePrimitive3D::impCreate3DDecomposition(const geometry::ViewInformation3D& /*rViewInformation*/) const
        {
            const sal_uInt32 nPointCount(getB3DPolygon().count());
            std::vector< BasePrimitive3D* > aResultVector;

            if(nPointCount)
            {
                if(basegfx::fTools::more(getRadius(), 0.0))
                {
                    const attribute::MaterialAttribute3D aMaterial(getBColor());
                    static const sal_uInt32 nSegments(8); // default for 3d line segments, for more quality just raise this value (in even steps)
                    const bool bClosed(getB3DPolygon().isClosed());
                    const bool bNoLineJoin(basegfx::B2DLineJoin::NONE == getLineJoin());
                    const sal_uInt32 nLoopCount(bClosed ? nPointCount : nPointCount - 1);
                    basegfx::B3DPoint aLast(getB3DPolygon().getB3DPoint(nPointCount - 1));
                    basegfx::B3DPoint aCurr(getB3DPolygon().getB3DPoint(0));

                    for(sal_uInt32 a(0); a < nLoopCount; a++)
                    {
                        // get next data
                        const basegfx::B3DPoint aNext(getB3DPolygon().getB3DPoint((a + 1) % nPointCount));
                        const basegfx::B3DVector aForw(aNext - aCurr);
                        const double fForwLen(aForw.getLength());

                        if(basegfx::fTools::more(fForwLen, 0.0))
                        {
                            // find out if linecap is active
                            const bool bFirst(!a);
                            const bool bLast(a + 1 == nLoopCount);
                            const bool bLineCapPossible(!bClosed && (bFirst || bLast));
                            const bool bLineCapRound(bLineCapPossible && css::drawing::LineCap_ROUND == getLineCap());
                            const bool bLineCapSquare(bLineCapPossible && css::drawing::LineCap_SQUARE == getLineCap());

                            // get rotation from vector, this describes rotation from (1, 0, 0) to aForw
                            basegfx::B3DHomMatrix aRotVector(getRotationFromVector(aForw));

                            // prepare transformations for tube and cap
                            basegfx::B3DHomMatrix aTubeTrans;
                            basegfx::B3DHomMatrix aCapTrans;

                            // cap gets radius size
                            aCapTrans.scale(getRadius(), getRadius(), getRadius());

                            if(bLineCapSquare)
                            {
                                // when square line cap just prolong line segment in X, maybe 2 x radius when
                                // first and last (simple line segment)
                                const double fExtraLength(bFirst && bLast ? getRadius() * 2.0 : getRadius());

                                aTubeTrans.scale(fForwLen + fExtraLength, getRadius(), getRadius());

                                if(bFirst)
                                {
                                    // correct start positions for tube and cap when first and square prolonged
                                    aTubeTrans.translate(-getRadius(), 0.0, 0.0);
                                    aCapTrans.translate(-getRadius(), 0.0, 0.0);
                                }
                            }
                            else
                            {
                                // normal tube size
                                aTubeTrans.scale(fForwLen, getRadius(), getRadius());
                            }

                            // rotate and translate tube and cap
                            aTubeTrans *= aRotVector;
                            aTubeTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());
                            aCapTrans *= aRotVector;
                            aCapTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                            if(bNoLineJoin || (!bClosed && bFirst))
                            {
                                // line start edge, build transformed primitiveVector3D
                                Primitive3DContainer aSequence;

                                if(bLineCapRound && bFirst)
                                {
                                    // LineCapRound used
                                    aSequence = getLineCapRoundSegments(nSegments, aMaterial);
                                }
                                else
                                {
                                    // simple closing cap
                                    aSequence = getLineCapSegments(nSegments, aMaterial);
                                }

                                aResultVector.push_back(new TransformPrimitive3D(aCapTrans, aSequence));
                            }
                            else
                            {
                                const basegfx::B3DVector aBack(aCurr - aLast);
                                const double fCross(basegfx::cross(aBack, aForw).getLength());

                                if(!basegfx::fTools::equalZero(fCross))
                                {
                                    // line connect non-parallel, aBack, aForw, use getLineJoin()
                                    const double fAngle(acos(aBack.scalar(aForw) / (fForwLen * aBack.getLength()))); // 0.0 .. F_PI2
                                    Primitive3DContainer aNewList(
                                        getLineJoinSegments(
                                            nSegments,
                                            aMaterial,
                                            fAngle,
                                            getMiterMinimumAngle(),
                                            getLineJoin()));

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
                                    aResultVector.push_back(
                                        new TransformPrimitive3D(
                                            aSphereTrans,
                                            aNewList));
                                }
                            }

                            // create line segments, build transformed primitiveVector3D
                            aResultVector.push_back(
                                new TransformPrimitive3D(
                                    aTubeTrans,
                                    getLineTubeSegments(nSegments, aMaterial)));

                            if(bNoLineJoin || (!bClosed && bLast))
                            {
                                // line end edge
                                basegfx::B3DHomMatrix aBackCapTrans;

                                // Mirror (line end) and radius scale
                                aBackCapTrans.rotate(0.0, F_PI, 0.0);
                                aBackCapTrans.scale(getRadius(), getRadius(), getRadius());

                                if(bLineCapSquare && bLast)
                                {
                                    // correct position when square and prolonged
                                    aBackCapTrans.translate(fForwLen + getRadius(), 0.0, 0.0);
                                }
                                else
                                {
                                    // standard position
                                    aBackCapTrans.translate(fForwLen, 0.0, 0.0);
                                }

                                // rotate and translate to destination
                                aBackCapTrans *= aRotVector;
                                aBackCapTrans.translate(aCurr.getX(), aCurr.getY(), aCurr.getZ());

                                // get primitiveVector3D
                                Primitive3DContainer aSequence;

                                if(bLineCapRound && bLast)
                                {
                                    // LineCapRound used
                                    aSequence = getLineCapRoundSegments(nSegments, aMaterial);
                                }
                                else
                                {
                                    // simple closing cap
                                    aSequence = getLineCapSegments(nSegments, aMaterial);
                                }

                                aResultVector.push_back(
                                    new TransformPrimitive3D(
                                        aBackCapTrans,
                                        aSequence));
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
                    aResultVector.push_back(new PolygonHairlinePrimitive3D(getB3DPolygon(), getBColor()));
                }
            }

            // prepare return value
            Primitive3DContainer aRetval(aResultVector.size());

            for(size_t a(0); a < aResultVector.size(); a++)
            {
                aRetval[a] = Primitive3DReference(aResultVector[a]);
            }

            return aRetval;
        }

        PolygonTubePrimitive3D::PolygonTubePrimitive3D(
            const basegfx::B3DPolygon& rPolygon,
            const basegfx::BColor& rBColor,
            double fRadius, basegfx::B2DLineJoin aLineJoin,
            css::drawing::LineCap aLineCap,
            double fDegreeStepWidth,
            double fMiterMinimumAngle)
        :   PolygonHairlinePrimitive3D(rPolygon, rBColor),
            maLast3DDecomposition(),
            mfRadius(fRadius),
            mfDegreeStepWidth(fDegreeStepWidth),
            mfMiterMinimumAngle(fMiterMinimumAngle),
            maLineJoin(aLineJoin),
            maLineCap(aLineCap)
        {
        }

        bool PolygonTubePrimitive3D::operator==(const BasePrimitive3D& rPrimitive) const
        {
            if(PolygonHairlinePrimitive3D::operator==(rPrimitive))
            {
                const PolygonTubePrimitive3D& rCompare = static_cast<const PolygonTubePrimitive3D&>(rPrimitive);

                return (getRadius() == rCompare.getRadius()
                    && getDegreeStepWidth() == rCompare.getDegreeStepWidth()
                    && getMiterMinimumAngle() == rCompare.getMiterMinimumAngle()
                    && getLineJoin() == rCompare.getLineJoin()
                    && getLineCap() == rCompare.getLineCap());
            }

            return false;
        }

        Primitive3DContainer PolygonTubePrimitive3D::get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            if(getLast3DDecomposition().empty())
            {
                const Primitive3DContainer aNewSequence(impCreate3DDecomposition(rViewInformation));
                const_cast< PolygonTubePrimitive3D* >(this)->maLast3DDecomposition = aNewSequence;
            }

            return getLast3DDecomposition();
        }

        // provide unique ID
        ImplPrimitive3DIDBlock(PolygonTubePrimitive3D, PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D)

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
