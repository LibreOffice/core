/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/helper/extrusionpresets.hxx>

#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <drawingml/shape3dproperties.hxx>

namespace oox
{
double CameraSettings::getRotateAngleZ() { return mfRotateAngleZ; }

CameraSettings CameraMapper::getDefaultCameraSettings() { return maCameraPresetArray[0]; }

sal_uInt16 CameraMapper::getPrstIndex(const OUString& rPresetType)
{
    sal_uInt16 nIt(0);
    while (nIt < 62 && maCameraPresetArray[nIt].msType != rPresetType)
        ++nIt;
    if (nIt >= 62)
    {
        SAL_WARN("oox", "camera preset type does not exist in OOXML");
        nIt = 0; // orthographicFront
    }
    return nIt;
}

double CameraMapper::getRotationZDegFromPrst(const sal_uInt16 nPresetIndex)
{
    if (nPresetIndex >= 62)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return 0.0;
    }
    else
        return maCameraPresetArray[nPresetIndex].mfRotateAngleZ;
}

void CameraMapper::getRotationAngleOOXFromPrst(oox::drawingml::RotationProperties& rRot,
                                               const sal_uInt16 nPresetIndex)
{
    // Alternatively, this could be implemented by calculating it from mfRotateAngleFirst,
    // mfRotateAngleSecond and mfRotateAngleZ.
    if (nPresetIndex >= 62)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return;
    }
    rRot.mnLatitude
        = maCameraPresetArray[nPresetIndex].mnRotLat; // ToDo ? cast sal_uInt32 to sal_Int32
    rRot.mnLongitude = maCameraPresetArray[nPresetIndex].mnRotLon;
    rRot.mnRevolution = maCameraPresetArray[nPresetIndex].mnRotRev;
    return;
}

void CameraMapper::addCameraPrstSettingsToExtrusion(oox::drawingml::Extrusion& rExtrusion,
                                                    const sal_uInt16 nPresetIndex)
{
    if (nPresetIndex >= 62)
    {
        SAL_WARN("oox", "invalid camera preset type index");
        return;
    }
    CameraSettings* pCamera = &maCameraPresetArray[nPresetIndex];
    rExtrusion.meProjectionMode = (pCamera->msProjectionMode == "parallel")
                                      ? css::drawing::ProjectionMode_PARALLEL
                                      : css::drawing::ProjectionMode_PERSPECTIVE;
    rExtrusion.maRotateAngle.First.Value <<= pCamera->mfRotateAngleFirst;
    rExtrusion.maRotateAngle.First.Type <<= 0;
    rExtrusion.maRotateAngle.Second.Value <<= pCamera->mfRotateAngleSecond;
    rExtrusion.maRotateAngle.Second.Type <<= 0;
    rExtrusion.maOrigin.First.Value <<= pCamera->mfOriginX;
    rExtrusion.maOrigin.First.Type <<= 0;
    rExtrusion.maOrigin.Second.Value <<= pCamera->mfOriginY;
    rExtrusion.maOrigin.Second.Type <<= 0;
    rExtrusion.maSkew.First.Value <<= pCamera->mfSkewAmount;
    rExtrusion.maSkew.First.Type <<= 0;
    rExtrusion.maSkew.Second.Value <<= pCamera->mfSkewAngle;
    rExtrusion.maSkew.Second.Type <<= 0;
    rExtrusion.maViewPoint.PositionX = pCamera->mfViewPointX;
    rExtrusion.maViewPoint.PositionY = pCamera->mfViewPointY;
    rExtrusion.maViewPoint.PositionZ = pCamera->mfViewPointZ;
    return;
}

void CameraMapper::fillExtrusionWithPresetValues(const OUString sPresetName,
                                                 oox::drawingml::Extrusion& rExtrusion,
                                                 double& fShapeRotation)
{
    sal_uInt16 nIt(0);
    while (nIt < 62 && maCameraPresetArray[nIt].msType != sPresetName)
        ++nIt;
    if (nIt >= 62)
    {
        SAL_WARN("oox", "camera preset type does not exist in OOXML");
        return;
    }
    CameraSettings* pCamera = &maCameraPresetArray[nIt];
    rExtrusion.meProjectionMode = (pCamera->msProjectionMode == "parallel")
                                      ? css::drawing::ProjectionMode_PARALLEL
                                      : css::drawing::ProjectionMode_PERSPECTIVE;
    rExtrusion.maRotateAngle.First.Value <<= pCamera->mfRotateAngleFirst;
    rExtrusion.maRotateAngle.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maRotateAngle.Second.Value <<= pCamera->mfRotateAngleSecond;
    rExtrusion.maRotateAngle.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    fShapeRotation = pCamera->mfRotateAngleZ;
    rExtrusion.maOrigin.First.Value <<= pCamera->mfOriginX;
    rExtrusion.maOrigin.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maOrigin.Second.Value <<= pCamera->mfOriginY;
    rExtrusion.maOrigin.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maSkew.First.Value <<= pCamera->mfSkewAmount;
    rExtrusion.maSkew.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maSkew.Second.Value <<= pCamera->mfSkewAngle;
    rExtrusion.maSkew.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rExtrusion.maViewPoint.PositionX = pCamera->mfViewPointX;
    rExtrusion.maViewPoint.PositionY = pCamera->mfViewPointY;
    rExtrusion.maViewPoint.PositionZ = pCamera->mfViewPointZ;
    return;
}

void CameraMapper::calculateRotationAngleLODegFromAngleOOX(
    css::drawing::EnhancedCustomShapeParameterPair& rSceneRotDeg, double& rShapeRotDeg,
    const oox::drawingml::RotationProperties& aOOXRot)
{
    // Convert OOX angles to rad and correct sign to usual orientation in 3D right-hand
    // coordinate system
    double fLat = basegfx::deg2rad(aOOXRot.mnLatitude.get(0) / 60000.0); // horizontal
    double fLon = -basegfx::deg2rad(aOOXRot.mnLongitude.get(0) / 60000.0); // vertical
    double fRev = basegfx::deg2rad(aOOXRot.mnRevolution.get(0) / 60000.0); // depth
    // MS Office rotates first at vertical axis, then at horizontal axis and last at depth-axis
    // rotate uses order first X, then Y, last Z. Parameter order (X, Y, Z)
    basegfx::B3DHomMatrix aRot;
    aRot.rotate(0.0, fLon, 0.0);
    aRot.rotate(fLat, 0.0, fRev);
    // ODF specifies order first Z, then Y, last X. Decompose aRot as if this order was used.
    // caution: matrix element indices start with 0 so we have first row 00, 01, 02
    // second row 10, 11, 12 and third row 20, 21, 22. Forth row is default and not used.
    double fRotX;
    double fRotY;
    double fRotZ;
    if (basegfx::fTools::equal(fabs(aRot.get(0, 2)), 1.0)) // I expect rounding errors
    {
        // The rotation at vertical axis is about 90deg or 270deg. The x-axis is mapped to the
        // z-axis. It is only possible to determine x+y or x-y respectively, but not single
        // values for x and z. Set z=0 here.
        fRotY = aRot.get(0, 2) > 0.0 ? F_PI2 : -F_PI2;
        fRotZ = 0.0;
        fRotX = atan2(aRot.get(2, 1), aRot.get(1, 1));
    }
    else
    {
        fRotZ = atan2(-aRot.get(0, 1), aRot.get(0, 0));
        fRotX = atan2(-aRot.get(1, 2), aRot.get(2, 2));
        if (basegfx::fTools::equalZero(cos(fRotZ)))
        {
            // cos(angle)==0  =>  sin(angle)!= 0
            fRotY = atan2(-aRot.get(0, 1), -aRot.get(0, 1) / sin(fRotZ));
        }
        else
            fRotY = atan2(aRot.get(0, 2), aRot.get(0, 0) / cos(fRotZ));
    }
    // LO uses rotation angles in Api not with the usual sign, so we need to correct it here.
    rSceneRotDeg.First.Value <<= basegfx::rad2deg(-fRotX);
    rSceneRotDeg.First.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rSceneRotDeg.Second.Value <<= basegfx::rad2deg(-fRotY);
    rSceneRotDeg.Second.Type <<= css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rShapeRotDeg = basegfx::rad2deg(fRotZ);
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
