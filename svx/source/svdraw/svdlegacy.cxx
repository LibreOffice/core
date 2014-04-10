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

#include <svx/svdlegacy.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////
// helpers for old access styles

namespace sdr
{
    namespace legacy
    {
        Rectangle GetBoundRect(const SdrObject& rObject, const SdrView* pSdrView)
        {
            const basegfx::B2DRange& rRange = rObject.getObjectRange(pSdrView);

            if(rRange.isEmpty())
            {
                return Rectangle();
            }
            else
            {
                return Rectangle(
                    (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
                    (sal_Int32)ceil(rRange.getMaxX()), (sal_Int32)ceil(rRange.getMaxY()));
            }
        }

        basegfx::B2DRange GetAllObjBoundRange(const SdrObjectVector& rObjVec, const SdrView* pSdrView)
        {
            basegfx::B2DRange aRetval;

            for(SdrObjectVector::const_iterator aCandidate = rObjVec.begin();
                aCandidate != rObjVec.end();
                ++aCandidate)
            {
                aRetval.expand((*aCandidate)->getObjectRange(pSdrView));
            }

            return aRetval;
        }

        Rectangle GetAllObjBoundRect(const SdrObjectVector& rObjVec, const SdrView* pSdrView)
        {
            const basegfx::B2DRange aBoundRange(GetAllObjBoundRange(rObjVec, pSdrView));

            if(aBoundRange.isEmpty())
            {
                return Rectangle();
            }
            else
            {
                return Rectangle(
                    (sal_Int32)floor(aBoundRange.getMinX()), (sal_Int32)floor(aBoundRange.getMinY()),
                    (sal_Int32)ceil(aBoundRange.getMaxX()), (sal_Int32)ceil(aBoundRange.getMaxY()));
            }
        }

        basegfx::B2DRange GetSnapRange(const SdrObject& rObject)
        {
            return rObject.getSnapRange();
//          return rObject.getSdrObjectTransformation() * basegfx::B2DRange::getUnitB2DRange();
        }

        Rectangle GetSnapRect(const SdrObject& rObject)
        {
            const basegfx::B2DRange aSnapRange(GetSnapRange(rObject));

            if(aSnapRange.isEmpty())
            {
                return Rectangle();
            }
            else
            {
                return Rectangle(
                    (sal_Int32)floor(aSnapRange.getMinX()), (sal_Int32)floor(aSnapRange.getMinY()),
                    (sal_Int32)ceil(aSnapRange.getMaxX()), (sal_Int32)ceil(aSnapRange.getMaxY()));
            }
        }

        void SetSnapRange(SdrObject& rObject, const basegfx::B2DRange& rRange)
        {
            if(rRange.isEmpty())
            {
                rObject.setSdrObjectTransformation(basegfx::B2DHomMatrix());
            }
            else
            {
                if(rObject.isRotatedOrSheared())
                {
                    const basegfx::B2DRange aCurrentSnapRange(GetSnapRange(rObject));

                    if(aCurrentSnapRange != rRange)
                    {
                        basegfx::B2DHomMatrix aRemoveCurrent;
                        basegfx::B2DRange aCorrected(rRange);

                        aRemoveCurrent.translate(-rObject.getSdrObjectTranslate());
                        aRemoveCurrent.rotate(-rObject.getSdrObjectRotate());
                        aRemoveCurrent.shearX(tan(-rObject.getSdrObjectShearX()));
                        aCorrected.transform(aRemoveCurrent);

                        const basegfx::B2DVector aScale(
                            rObject.isMirroredX() ? -aCorrected.getWidth() : aCorrected.getWidth(),
                            rObject.isMirroredY() ? -aCorrected.getHeight() : aCorrected.getHeight());

                        rObject.setSdrObjectTransformation(
                            basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                aScale,
                                rObject.getSdrObjectShearX(),
                                rObject.getSdrObjectRotate(),
                                aCorrected.getMinimum()));
                    }
                }
                else
                {
                    // no rotate and/or shear, thus the same as SetLogicRange
                    // without rotate/shear
                    const basegfx::B2DVector aScale(
                        rObject.isMirroredX() ? -rRange.getWidth() : rRange.getWidth(),
                        rObject.isMirroredY() ? -rRange.getHeight() : rRange.getHeight());

                    rObject.setSdrObjectTransformation(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aScale,
                            rRange.getMinimum()));
                }
            }
        }

        void SetSnapRect(SdrObject& rObject, const Rectangle& rRectangle)
        {
            if(rRectangle.IsEmpty())
            {
                rObject.setSdrObjectTransformation(basegfx::B2DHomMatrix());
            }
            else
            {
                const basegfx::B2DRange aSnapRange(
                    rRectangle.Left(), rRectangle.Top(),
                    rRectangle.Right(), rRectangle.Bottom());

                SetSnapRange(rObject, aSnapRange);
            }
        }

        basegfx::B2DRange GetAllObjSnapRange(const SdrObjectVector& rObjVec)
        {
            basegfx::B2DRange aRetval;

            for(SdrObjectVector::const_iterator aCandidate = rObjVec.begin();
                aCandidate != rObjVec.end();
                ++aCandidate)
            {
                aRetval.expand(GetSnapRange(*(*aCandidate)));
            }

            return aRetval;
        }

        Rectangle GetAllObjSnapRect(const SdrObjectVector& rObjVec)
        {
            const basegfx::B2DRange aSnapRange(GetAllObjSnapRange(rObjVec));

            if(aSnapRange.isEmpty())
            {
                return Rectangle();
            }
            else
            {
                return Rectangle(
                    (sal_Int32)floor(aSnapRange.getMinX()), (sal_Int32)floor(aSnapRange.getMinY()),
                    (sal_Int32)ceil(aSnapRange.getMaxX()), (sal_Int32)ceil(aSnapRange.getMaxY()));
            }
        }

        basegfx::B2DRange GetLogicRange(const SdrObject& rObject)
        {
            const basegfx::B2DHomMatrix& rObjectMatrix(rObject.getSdrObjectTransformation());

            if(rObjectMatrix.isIdentity())
            {
                return basegfx::B2DRange();
            }
            else
            {
                // build range without rotation, mirror and shear
                return basegfx::B2DRange(
                    rObject.getSdrObjectTranslate(),
                    rObject.getSdrObjectTranslate() + basegfx::absolute(rObject.getSdrObjectScale()));
            }
        }

        Rectangle GetLogicRect(const SdrObject& rObject)
        {
            const basegfx::B2DRange aLogicRange(GetLogicRange(rObject));

            if(aLogicRange.isEmpty())
            {
                return Rectangle();
            }
            else
            {
                return Rectangle(
                    (sal_Int32)floor(aLogicRange.getMinX()), (sal_Int32)floor(aLogicRange.getMinY()),
                    (sal_Int32)ceil(aLogicRange.getMaxX()), (sal_Int32)ceil(aLogicRange.getMaxY()));
            }
        }

        void SetLogicRange(SdrObject& rObject, const basegfx::B2DRange& rRange)
        {
            if(rRange.isEmpty())
            {
                rObject.setSdrObjectTransformation(basegfx::B2DHomMatrix());
            }
            else
            {
                // keep mirroring, rotation and shear, replace absolute size and translation
                const basegfx::B2DVector aScale(
                    rObject.isMirroredX() ? -rRange.getWidth() : rRange.getWidth(),
                    rObject.isMirroredY() ? -rRange.getHeight() : rRange.getHeight());

                if(rObject.isRotatedOrSheared())
                {
                    rObject.setSdrObjectTransformation(
                        basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            aScale,
                            rObject.getSdrObjectShearX(),
                            rObject.getSdrObjectRotate(),
                            rRange.getMinimum()));
                }
                else
                {
                    rObject.setSdrObjectTransformation(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aScale,
                            rRange.getMinimum()));
                }
            }
        }

        void SetLogicRect(SdrObject& rObject, const Rectangle& rRectangle)
        {
            if(rRectangle.IsEmpty())
            {
                rObject.setSdrObjectTransformation(basegfx::B2DHomMatrix());
            }
            else
            {
                const basegfx::B2DRange aLogicRange(
                    rRectangle.Left(), rRectangle.Top(),
                    rRectangle.Right(), rRectangle.Bottom());

                SetLogicRange(rObject, aLogicRange);
            }
        }

        Point GetAnchorPos(const SdrObject& rObject)
        {
            const basegfx::B2DPoint& rAnchor = rObject.GetAnchorPos();

            return Point(basegfx::fround(rAnchor.getX()), basegfx::fround(rAnchor.getY()));
        }

        long convertRotateAngleNewToLegacy(double fNew)
        {
            return basegfx::fround(basegfx::snapToZeroRange(-fNew / F_PI18000, 36000.0));
        }

        double convertRotateAngleLegacyToNew(long nOld)
        {
            return basegfx::snapToZeroRange(static_cast< double >(nOld) * -F_PI18000, F_2PI);
        }

        long GetRotateAngle(const SdrObject& rObject)
        {
            if(rObject.isRotated())
            {
                return convertRotateAngleNewToLegacy(rObject.getSdrObjectRotate());
            }
            else
            {
                return 0;
            }
        }

        long convertShearAngleXNewToLegacy(double fNew)
        {
            return basegfx::fround(basegfx::snapToRange(-fNew / F_PI18000, -18000.0, 18000.0));
        }

        double convertShearAngleXLegacyToNew(long nOld)
        {
            return basegfx::snapToRange(static_cast< double >(nOld) * -F_PI18000, -F_PI, F_PI);
        }

        long GetShearAngleX(const SdrObject& rObject)
        {
            if(rObject.isSheared())
            {
                return convertShearAngleXNewToLegacy(rObject.getSdrObjectShearX());
            }
            else
            {
                return 0;
            }
        }

        void transformSdrObject(SdrObject& rObject, const basegfx::B2DHomMatrix& rTransform)
        {
            if(!rTransform.isIdentity())
            {
                basegfx::B2DHomMatrix aObjectMatrix(rObject.getSdrObjectTransformation());

                if(aObjectMatrix.isIdentity())
                {
                    rObject.setSdrObjectTransformation(rTransform);
                }
                else
                {
                    aObjectMatrix = rTransform * aObjectMatrix;

                    rObject.setSdrObjectTransformation(aObjectMatrix);
                }
            }
        }

        void MoveSdrObject(SdrObject& rObject, const Size& rSiz)
        {
            const long nWidth(rSiz.Width());
            const long nHeight(rSiz.Height());

            if(nWidth || nHeight)
            {
                transformSdrObject(rObject, basegfx::tools::createTranslateB2DHomMatrix(nWidth, nHeight));
            }
        }

        void ResizeSdrObject(SdrObject& rObject, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
        {
            const basegfx::B2DTuple aScale(xFact, yFact);

            if(!aScale.equalZero())
            {
                const basegfx::B2DTuple aRefPoint(rRef.X(), rRef.Y());

                if(aRefPoint.equalZero())
                {
                    transformSdrObject(rObject, basegfx::tools::createScaleB2DHomMatrix(aScale));
                }
                else
                {
                    basegfx::B2DHomMatrix aTransform;

                    aTransform.translate(-aRefPoint);
                    aTransform.scale(aScale);
                    aTransform.translate(aRefPoint);

                    transformSdrObject(rObject, aTransform);
                }
            }
        }

        void RotateSdrObject(SdrObject& rObject, const Point& rRef, long nWink)
        {
            if(0 != nWink)
            {
                const double fAngle((-nWink * F_PI) / 18000.0);
                const basegfx::B2DTuple aRefPoint(rRef.X(), rRef.Y());

                if(aRefPoint.equalZero())
                {
                    transformSdrObject(rObject, basegfx::tools::createRotateB2DHomMatrix(fAngle));
                }
                else
                {
                    basegfx::B2DHomMatrix aTransform;

                    aTransform.translate(-aRefPoint);
                    aTransform.rotate(fAngle);
                    aTransform.translate(aRefPoint);

                    transformSdrObject(rObject, aTransform);
                }
            }
        }

        void MirrorSdrObject(SdrObject& rObject, const Point& rRef1, const Point& rRef2)
        {
            if(rRef1 != rRef2)
            {
                const basegfx::B2DVector aMirrorEdge(rRef2.X() - rRef1.X(), rRef2.Y() - rRef1.Y());
                const double fAngleToXAxis(atan2(aMirrorEdge.getY(), aMirrorEdge.getX()));
                basegfx::B2DHomMatrix aTransform;

                aTransform.translate(-rRef1.X(), -rRef1.Y());
                aTransform.rotate(-fAngleToXAxis);
                aTransform.scale(1.0, -1.0);
                aTransform.rotate(fAngleToXAxis);
                aTransform.translate(rRef1.X(), rRef1.Y());

                transformSdrObject(rObject, aTransform);
            }
        }

        void ShearSdrObject(SdrObject& rObject, const Point& rRef, long nWink, bool bVShear)
        {
            if(nWink)
            {
                const double fAngle((-nWink * F_PI) / 18000.0);
                const basegfx::B2DTuple aRefPoint(rRef.X(), rRef.Y());

                if(aRefPoint.equalZero())
                {
                    if(bVShear)
                    {
                        transformSdrObject(rObject, basegfx::tools::createShearYB2DHomMatrix(tan(fAngle)));
                    }
                    else
                    {
                        transformSdrObject(rObject, basegfx::tools::createShearXB2DHomMatrix(tan(fAngle)));
                    }
                }
                else
                {
                    basegfx::B2DHomMatrix aTransform;

                    aTransform.translate(-aRefPoint);

                    if(bVShear)
                    {
                        aTransform.shearY(tan(fAngle));
                    }
                    else
                    {
                        aTransform.shearX(tan(fAngle));
                    }

                    aTransform.translate(aRefPoint);

                    transformSdrObject(rObject, aTransform);
                }
            }
        }

    } // end of namespace legacy
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
