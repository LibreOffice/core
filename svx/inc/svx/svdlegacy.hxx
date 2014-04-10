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

#ifndef _SVDLEGACY_HXX
#define _SVDLEGACY_HXX

#include <svx/svdobj.hxx>

//////////////////////////////////////////////////////////////////////////////
// helpers for old access styles

namespace sdr
{
    namespace legacy
    {
        // old Rectangle-based access to BoundRectangle
        SVX_DLLPUBLIC Rectangle GetBoundRect(const SdrObject& rObject, const SdrView* pSdrView = 0); // TTTT: need to remove default and search for usages

        // old access to vector of SdrObjects BoundRect, various DataTypes
        SVX_DLLPUBLIC basegfx::B2DRange GetAllObjBoundRange(const SdrObjectVector& rObjVec, const SdrView* pSdrView = 0);  // TTTT: need to remove default and search for usages
        SVX_DLLPUBLIC Rectangle GetAllObjBoundRect(const SdrObjectVector& rObjVec, const SdrView* pSdrView = 0);  // TTTT: need to remove default and search for usages

        // old access to SnapRect, various DataTypes
        // SnapRect is the range of the hairline geometry, including all transforms
        SVX_DLLPUBLIC basegfx::B2DRange GetSnapRange(const SdrObject& rObject);
        SVX_DLLPUBLIC Rectangle GetSnapRect(const SdrObject& rObject);
        SVX_DLLPUBLIC void SetSnapRange(SdrObject& rObject, const basegfx::B2DRange& rRange);
        SVX_DLLPUBLIC void SetSnapRect(SdrObject& rObject, const Rectangle& rRectangle);

        // old access to vector of SdrObjects SnapRect, various DataTypes
        SVX_DLLPUBLIC basegfx::B2DRange GetAllObjSnapRange(const SdrObjectVector& rObjVec);
        SVX_DLLPUBLIC Rectangle GetAllObjSnapRect(const SdrObjectVector& rObjVec);

        // old access to LogicRect
        // LogicRect is the object geometry without rotation and shear
        SVX_DLLPUBLIC basegfx::B2DRange GetLogicRange(const SdrObject& rObject);
        SVX_DLLPUBLIC Rectangle GetLogicRect(const SdrObject& rObject);
        SVX_DLLPUBLIC void SetLogicRange(SdrObject& rObject, const basegfx::B2DRange& rRange);
        SVX_DLLPUBLIC void SetLogicRect(SdrObject& rObject, const Rectangle& rRectangle);

        // old access to anchor as point
        SVX_DLLPUBLIC Point GetAnchorPos(const SdrObject& rObject);

        // back and forth converters for Rotation new (angle in rad [0.0 .. F_2PI[) and
        // old (mirrored angle in deg * 100 [0 .. 36000[)
        // These converters are used for places where the correct new values are used
        // for the UI and similar
        long convertRotateAngleNewToLegacy(double fNew);
        double convertRotateAngleLegacyToNew(long nOld);

        // back and forth converters for ShearX new (shear x in rad [-F_PI .. F_PI[) and
        // old (mirrored shear x in deg * 100 [-18000 .. 18000[)
        // These converters are used for places where the correct new values are used
        // for the UI and similar
        long convertShearAngleXNewToLegacy(double fNew);
        double convertShearAngleXLegacyToNew(long nOld);

        // old access to rotate and shear (including wrong orientation and
        // integer nature, see above converters)
        SVX_DLLPUBLIC long GetRotateAngle(const SdrObject& rObject);
        SVX_DLLPUBLIC long GetShearAngleX(const SdrObject& rObject);

        // helper for object transform in one step
        SVX_DLLPUBLIC void transformSdrObject(SdrObject& rObject, const basegfx::B2DHomMatrix& rTransform);

        // old transformation accessors
        SVX_DLLPUBLIC void MoveSdrObject(SdrObject& rObject, const Size& rSiz); // Move
        SVX_DLLPUBLIC void ResizeSdrObject(SdrObject& rObject, const Point& rRef, const Fraction& xFact, const Fraction& yFact); // Resize
        SVX_DLLPUBLIC void RotateSdrObject(SdrObject& rObject, const Point& rRef, long nWink); // Rotate
        SVX_DLLPUBLIC void MirrorSdrObject(SdrObject& rObject, const Point& rRef1, const Point& rRef2); // Mirror
        SVX_DLLPUBLIC void ShearSdrObject(SdrObject& rObject, const Point& rRef, long nWink, bool bVShear); // Shear

    } // end of namespace legacy
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDLEGACY_HXX

