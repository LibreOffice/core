/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <drawingml/scene3dhelper.hxx>

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeMetalType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>

#include <cmath>

namespace oox
{
/** This struct is used to hold values from the OOXML camera preset types.*/
namespace
{
struct PrstCameraValues
{
    std::u16string_view msCameraPrstName; // identifies the value set

    bool mbIsParallel;

    // values as shown in the UI of MS Office, converted to 1/60000 deg
    double mfRotateAngleX; // unit 1/60000 degree
    double mfRotateAngleY; // unit 1/60000 degree
    double mfRotateAngleZ; // unit 1/60000 degree

    // Position of origin relative to the bounding box of the transformed 2D shape.
    // LibreOffice can handle values outside the ODF range.
    double mfOriginX; // ODF range [-0.5 (left).. 0.5 (right)], fraction of width
    double mfOriginY; // ODF range [-0.5 (top) 0.5 (bottom)], fraction of height

    // mandatory for PARALLEL, ignored for PERSPECTIVE
    double mfSkewAmount; // range 0 to 100, percent of depth used as slant length
    double mfSkewAngle; // unit degree

    // mandatory for PERSPECTIVE, ignored for PARALLEL
    // API type ::com::sun::star::drawing::Position3D; unit 1/100 mm
    double mfViewPointX; // shift from Origin
    double mfViewPointY; // shift from Origin
    double mfViewPointZ; // absolute z-coordinate

    // The OOXML camera attribute "zoom" is not contained, because it is not set in preset camera
    // types and LO cannot render it in custom shape extrusion scene.
};
} // end anonymous namespace

// The values were found experimental using MS Office. A spreadsheet with remarks is attached
// to tdf#70039.
constexpr sal_uInt16 nCameraPresetCount(62); // Fixed, specified in OOXML standard.
constexpr PrstCameraValues aPrstCameraValuesArray[nCameraPresetCount] = {
    { u"isometricBottomDown", true, 2124000, 18882000, 17988000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricBottomUp", true, 2124000, 2718000, 3612000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricLeftDown", true, 2100000, 2700000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricLeftUp", true, 19500000, 2700000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis1Left", true, 1080000, 3840000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis1Right", true, 1080000, 20040000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis1Top", true, 18078000, 18390000, 3456000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis2Left", true, 1080000, 1560000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis2Right", true, 1080000, 17760000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis2Top", true, 18078000, 3210000, 18144000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis3Bottom", true, 3522000, 18390000, 18144000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis3Left", true, 20520000, 3840000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis3Right", true, 20520000, 20040000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis4Bottom", true, 3522000, 3210000, 3456000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis4Left", true, 20520000, 1560000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricOffAxis4Right", true, 20520000, 17760000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricRightDown", true, 19500000, 18900000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricRightUp", true, 2100000, 18900000, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricTopDown", true, 19476000, 2718000, 17988000, 0, 0, 0, 0, 0, 0, 0 },
    { u"isometricTopUp", true, 19476000, 18882000, 3612000, 0, 0, 0, 0, 0, 0, 0 },
    { u"legacyObliqueBottom", true, 0, 0, 0, 0, 0.5, 50, 90, 0, 0, 0 },
    { u"legacyObliqueBottomLeft", true, 0, 0, 0, -0.5, 0.5, 50, 45, 0, 0, 0 },
    { u"legacyObliqueBottomRight", true, 0, 0, 0, 0.5, 0.5, 50, 135, 0, 0, 0 },
    { u"legacyObliqueFront", true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"legacyObliqueLeft", true, 0, 0, 0, -0.5, 0, 50, -360, 0, 0, 0 },
    { u"legacyObliqueRight", true, 0, 0, 0, 0.5, 0, 50, 180, 0, 0, 0 },
    { u"legacyObliqueTop", true, 0, 0, 0, 0, -0.5, 50, -90, 0, 0, 0 },
    { u"legacyObliqueTopLeft", true, 0, 0, 0, -0.5, -0.5, 50, -45, 0, 0, 0 },
    { u"legacyObliqueTopRight", true, 0, 0, 0, 0.5, -0.5, 50, -135, 0, 0, 0 },
    { u"legacyPerspectiveBottom", false, 0, 0, 0, 0, 0.5, 50, 90, 0, 3472, 25000 },
    { u"legacyPerspectiveBottomLeft", false, 0, 0, 0, -0.5, 0.5, 50, 45, -3472, 3472, 25000 },
    { u"legacyPerspectiveBottomRight", false, 0, 0, 0, 0.5, 0.5, 50, 135, 3472, 3472, 25000 },
    { u"legacyPerspectiveFront", false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25000 },
    { u"legacyPerspectiveLeft", false, 0, 0, 0, -0.5, 0, 50, -360, -3472, 0, 25000 },
    { u"legacyPerspectiveRight", false, 0, 0, 0, 0.5, 0, 50, 180, 3472, 0, 25000 },
    { u"legacyPerspectiveTop", false, 0, 0, 0, 0, -0.5, 50, -90, 0, -3472, 25000 },
    { u"legacyPerspectiveTopLeft", false, 0, 0, 0, -0.5, -0.5, 50, -45, -3472, -3472, 25000 },
    { u"legacyPerspectiveTopRight", false, 0, 0, 0, 0.5, -0.5, 50, -135, 3472, -3472, 25000 },
    { u"obliqueBottom", true, 0, 0, 0, 0, 0.5, 30, 90, 0, 0, 0 },
    { u"obliqueBottomLeft", true, 0, 0, 0, -0.5, 0.5, 30, 45, 0, 0, 0 },
    { u"obliqueBottomRight", true, 0, 0, 0, 0.5, 0.5, 30, 135, 0, 0, 0 },
    { u"obliqueLeft", true, 0, 0, 0, -0.5, 0, 30, -360, 0, 0, 0 },
    { u"obliqueRight", true, 0, 0, 0, 0.5, 0, 30, 180, 0, 0, 0 },
    { u"obliqueTop", true, 0, 0, 0, 0, -0.5, 30, -90, 0, 0, 0 },
    { u"obliqueTopLeft", true, 0, 0, 0, -0.5, -0.5, 30, -45, 0, 0, 0 },
    { u"obliqueTopRight", true, 0, 0, 0, 0.5, -0.5, 30, -135, 0, 0, 0 },
    { u"orthographicFront", true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { u"perspectiveAbove", false, 20400000, 0, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveAboveLeftFacing", false, 2358000, 858000, 20466000, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveAboveRightFacing", false, 2358000, 20742000, 1134000, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveBelow", false, 1200000, 0, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveContrastingLeftFacing", false, 624000, 2634000, 21384000, 0, 0, 0, 0, 0, 0,
      38451 },
    { u"perspectiveContrastingRightFacing", false, 624000, 18966000, 216000, 0, 0, 0, 0, 0, 0,
      38451 },
    { u"perspectiveFront", false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveHeroicExtremeLeftFacing", false, 486000, 2070000, 21426000, 0, 0, 0, 0, 0, 0,
      18981 },
    { u"perspectiveHeroicExtremeRightFacing", false, 486000, 19530000, 174000, 0, 0, 0, 0, 0, 0,
      18981 },
    { u"perspectiveHeroicLeftFacing", false, 20940000, 858000, 156000, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveHeroicRightFacing", false, 20940000, 20742000, 21444000, 0, 0, 0, 0, 0, 0,
      38451 },
    { u"perspectiveLeft", false, 0, 1200000, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveRelaxed", false, 18576000, 0, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveRelaxedModerately", false, 19488000, 0, 0, 0, 0, 0, 0, 0, 0, 38451 },
    { u"perspectiveRight", false, 0, 20400000, 0, 0, 0, 0, 0, 0, 0, 38451 }
};

namespace
{
/** Searches for the item in aPrstCameraValuesArray with given sPresetName.
    @param [in] sPresetName name as specified in OOXML standard
    @return returns the index if item exists, otherwise -1*/
sal_Int16 getPrstCameraIndex(std::u16string_view sPresetName)
{
    sal_Int16 nIt(0);
    while (nIt < nCameraPresetCount && aPrstCameraValuesArray[nIt].msCameraPrstName != sPresetName)
        ++nIt;
    if (nIt >= nCameraPresetCount)
    {
        nIt = -1; // Error is handled by caller
    }
    return nIt;
}
} // end anonymous namespace

void Scene3DHelper::getAPIAnglesFromOOXAngle(const sal_Int32 nLat, const sal_Int32 nLon,
                                             const sal_Int32 nRev, double& fX, double& fY,
                                             double& fZ)
{
    // MS Office applies the rotations in the order first around y-axis by nLon, then around x-axis
    // by nLat and last around z-axis by nRev. The extrusion mode in ODF and also the API
    // first rotate around the z-axis, then around the y-axis and last around the x-axis. In ODF, the
    // rotation around the z-axis is integrated into the shape transformation and the others are
    // specified in the enhanced geometry of the shape.
    // The orientation of the resulting angles equals the orientation in API, but the angles are in
    // radians.

    // First we build the total rotation matrix from the OOX angles. y-axis points down.
    basegfx::B3DHomMatrix aXMat;
    const double fLatRad = basegfx::deg2rad<60000>(nLat);
    aXMat.set(1, 1, cos(fLatRad));
    aXMat.set(2, 2, cos(fLatRad));
    aXMat.set(1, 2, sin(fLatRad));
    aXMat.set(2, 1, -sin(fLatRad));

    basegfx::B3DHomMatrix aYMat;
    const double fLonRad = basegfx::deg2rad<60000>(nLon);
    aYMat.set(0, 0, cos(fLonRad));
    aYMat.set(2, 2, cos(fLonRad));
    aYMat.set(0, 2, -sin(fLonRad));
    aYMat.set(2, 0, sin(fLonRad));

    basegfx::B3DHomMatrix aZMat;
    const double fRevRad = basegfx::deg2rad<60000>(nRev);
    aZMat.set(0, 0, cos(fRevRad));
    aZMat.set(1, 1, cos(fRevRad));
    aZMat.set(0, 1, sin(fRevRad));
    aZMat.set(1, 0, -sin(fRevRad));
    basegfx::B3DHomMatrix aTotalMat = aZMat * aXMat * aYMat;

    // Now we decompose it so that rotation around z-axis is the first rotation. We know it is a
    // orthonormal matrix, so some steps seen in B3DHomMatrix::decompose() are not needed.
    // The solution fY2 = pi - fY results in the same projection, thus we do not consider it.
    fY = std::asin(-aTotalMat.get(0, 2));

    if (basegfx::fTools::equalZero(cos(fY)))
    {
        // This case has zeros at positions (0,0), (0,1), (1,2) and (2,2) in aTotalMat.
        // This special case means, that the shape is rotated around the y-axis so, that the user
        // looks on the extruded faces. Front face and back face are orthogonal to the xy-plane. The
        // rotation around the x-axis cannot be distinguished from an initial rotation of the shape
        // outside 3D. Thus there exist no unique solution.
        fX = 0.0;
        fZ = std::atan2(aTotalMat.get(2, 1), aTotalMat.get(1, 1));
    }
    else
    {
        fX = std::atan2(-aTotalMat.get(1, 2) / cos(fY), aTotalMat.get(2, 2) / cos(fY));
        fZ = std::atan2(aTotalMat.get(0, 1) / cos(fY), aTotalMat.get(0, 0) / cos(fY));
    }
}

void Scene3DHelper::getAPIAnglesFrom3DProperties(
    const oox::drawingml::Shape3DPropertiesPtr p3DProperties, const sal_Int32& rnMSOShapeRotation,
    double& fX, double& fY, double& fZ)
{
    if (!p3DProperties)
        return;

    // on x-axis, unit 1/60000 deg
    sal_Int32 nLatitude = (*p3DProperties).maCameraRotation.mnLatitude.value_or(0);
    // on y-axis, unit 1/60000 deg
    sal_Int32 nLongitude = (*p3DProperties).maCameraRotation.mnLongitude.value_or(0);
    // on z-axis, unit 1/60000 deg
    sal_Int32 nRevolution = (*p3DProperties).maCameraRotation.mnRevolution.value_or(0);

    // Some projection types need special treatment:
    if (29 <= mnPrstCameraIndex && mnPrstCameraIndex <= 37)
    {
        // legacyPerspective. MS Office does not use z-rotation but writes it to file. We need to
        // ignore it. The preset cameras have no rotation.
        nRevolution = 0;
    }
    else if (47 <= mnPrstCameraIndex)
    {
        assert(mnPrstCameraIndex <= 61
               && "by definition we don't set anything >= nCameraPresetCount (62)");
        // perspective. MS Office has a strange rendering behavior: If the shape rotation is not zero
        // and the angle for rotation on x-axis (=latitude) is >90deg and <=270deg, then MSO renders
        // the shape with an addition 180deg rotation on the z-axis. This happens only with user
        // entered angles.
        if (rnMSOShapeRotation != 0 && nLatitude > 5400000 && nLatitude <= 16200000)
            nRevolution += 10800000;
    }

    // In case attributes lat, lon and rev of the <rot> child element of the <scene3d> element in
    // OOXML markup are given, they overwrite the values from the preset camera type. Otherwise the
    // values from the preset camera are used. OOXML requires that all three attributes must exist at
    // the same time. Thus it is enough to test one of them.
    if (!(*p3DProperties).maCameraRotation.mnRevolution.has_value())
    {
        // The angles are given in 1/60000 deg in aPrstCameraValuesArray.
        nLatitude = aPrstCameraValuesArray[mnPrstCameraIndex].mfRotateAngleX;
        nLongitude = aPrstCameraValuesArray[mnPrstCameraIndex].mfRotateAngleY;
        nRevolution = aPrstCameraValuesArray[mnPrstCameraIndex].mfRotateAngleZ;
    }

    // MS Office applies the shape rotation after the rotations from camera in case of non-legacy
    // cameras, and before for legacy cameras. ODF specifies to first rotate the shape. Thus we need
    // to add shape rotation to nRevolution in case of non-legacy cameras. The shape rotation has
    // opposite orientation than camera z-rotation.
    bool bIsLegacyCamera = 20 <= mnPrstCameraIndex && mnPrstCameraIndex <= 37;
    if (!bIsLegacyCamera)
        nRevolution -= rnMSOShapeRotation;

    // Now calculate the angles for LO rotation order and orientation.
    Scene3DHelper::getAPIAnglesFromOOXAngle(nLatitude, nLongitude, nRevolution, fX, fY, fZ);

    if (bIsLegacyCamera)
        fZ -= basegfx::deg2rad<60000>(rnMSOShapeRotation);
}

void Scene3DHelper::addRotateAngleToMap(oox::PropertyMap& rPropertyMap, const double fX,
                                        const double fY)
{
    css::drawing::EnhancedCustomShapeParameterPair aAnglePair;
    aAnglePair.First.Value <<= basegfx::rad2deg(fX);
    aAnglePair.First.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    aAnglePair.Second.Value <<= basegfx::rad2deg(fY);
    aAnglePair.Second.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rPropertyMap.setAnyProperty(oox::PROP_RotateAngle, css::uno::Any(aAnglePair));
}

void Scene3DHelper::addExtrusionDepthToMap(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                           oox::PropertyMap& rPropertyMap)
{
    // Amount of extrusion and its position relative to the original shape face. This moves the
    // shape inside the scene.
    // The GetExtrusionDepth() method in EnhancedCustomShape3d.cxx expects type double for both.
    sal_Int32 nDepthAmount = (*p3DProperties).mnExtrusionH.value_or(0); // unit EMU
    double fDepthAmount = o3tl::convert(nDepthAmount, o3tl::Length::emu, o3tl::Length::mm100);
    sal_Int32 nZPosition = (*p3DProperties).mnShapeZ.value_or(0); // unit EMU
    double fZPosition = o3tl::convert(nZPosition, o3tl::Length::emu, o3tl::Length::mm100);
    double fDepthRelPos = 0.0;
    if (nDepthAmount == 0 && nZPosition != 0)
    {
        // We cannot express the position relative to the extrusion depth.
        // Use an artificial, small depth of 1Hmm
        fDepthRelPos = fZPosition;
        fDepthAmount = 1.0; // unit Hmm
    }
    else if (nDepthAmount != 0)
        fDepthRelPos = fZPosition / fDepthAmount;

    css::drawing::EnhancedCustomShapeParameterPair aPair;
    css::drawing::EnhancedCustomShapeParameter& rDepthAmount = aPair.First;
    rDepthAmount.Value <<= fDepthAmount;
    rDepthAmount.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    css::drawing::EnhancedCustomShapeParameter& rDepthFraction = aPair.Second;
    rDepthFraction.Value <<= fDepthRelPos;
    rDepthFraction.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rPropertyMap.setProperty(oox::PROP_Depth, aPair);
}

void Scene3DHelper::addProjectionGeometryToMap(
    const oox::drawingml::Shape3DPropertiesPtr p3DProperties, oox::PropertyMap& rPropertyMap,
    const bool bIsParallel, const sal_Int32 rnMSOShapeRotation)
{
    // origin is needed for parallel and perspective as well
    css::drawing::EnhancedCustomShapeParameterPair aOrigin;
    aOrigin.First.Value <<= aPrstCameraValuesArray[mnPrstCameraIndex].mfOriginX;
    aOrigin.First.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    aOrigin.Second.Value <<= aPrstCameraValuesArray[mnPrstCameraIndex].mfOriginY;
    aOrigin.Second.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
    rPropertyMap.setProperty(oox::PROP_Origin, aOrigin);

    if (bIsParallel)
    {
        // PARALLEL needs API property Skew.
        // orthographicFront and isometric projections do not use skew. We write it nevertheless
        // to prevent LO defaults. Zeros are contained in aPrstCameraValuesArray for these cases.
        double fSkewAngle = aPrstCameraValuesArray[mnPrstCameraIndex].mfSkewAngle; // unit degree
        double fSkewAmount = aPrstCameraValuesArray[mnPrstCameraIndex].mfSkewAmount;
        // oblique projections (index [38..45]) need special treatment. MS Office rotates around the
        // z-axis after the projection was created. Thus the rotation affects the skew direction. ODF
        // rotates the shape before creating the projection. Thus we need to incorporate the shape
        // rotation into the skew angle.
        if (38 <= mnPrstCameraIndex && mnPrstCameraIndex <= 45)
        {
            fSkewAngle -= rnMSOShapeRotation / 60000.0;
        }
        css::drawing::EnhancedCustomShapeParameterPair aSkew;
        aSkew.First.Value <<= fSkewAmount;
        aSkew.First.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
        aSkew.Second.Value <<= fSkewAngle;
        aSkew.Second.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
        rPropertyMap.setProperty(oox::PROP_Skew, aSkew);
    }
    else
    {
        // PERSPECTIVE needs API property ViewPoint.
        css::drawing::Position3D aViewPoint;

        // x- and y-coordinate depend on preset camera type.
        aViewPoint.PositionX = aPrstCameraValuesArray[mnPrstCameraIndex].mfViewPointX;
        aViewPoint.PositionY = aPrstCameraValuesArray[mnPrstCameraIndex].mfViewPointY;

        // The z-coordinate is determined bei a field of view angle in OOXML and by a
        // distance in LibreOffice. MS Office users can change its value.
        if ((*p3DProperties).mfFieldOfVision.has_value())
        {
            double fFov = (*p3DProperties).mfFieldOfVision.value();
            fFov = std::clamp(fFov, 0.5, 179.5);
            // 15976 = 25000 * tan(32.5°) as in legacy. Better ideas to calculate the distance are
            // welcome.
            aViewPoint.PositionZ = 15976.0 / tan(basegfx::deg2rad(fFov / 2.0));
        }
        else
            aViewPoint.PositionZ = aPrstCameraValuesArray[mnPrstCameraIndex].mfViewPointZ;

        rPropertyMap.setProperty(oox::PROP_ViewPoint, aViewPoint);
    }
    // ToDo: It is possible in OOXML to set a 3D-scene on a group. It is not clear yet how that can
    // be mimicked in LO. In case of perspective projection, it produces a horizontal or vertical
    // shift of the viewpoint in relation to the shapes of the group, for example.
}

bool Scene3DHelper::setExtrusionProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                           const sal_Int32& rnMSOShapeRotation,
                                           oox::PropertyMap& rPropertyMap, double& rRotZ,
                                           oox::drawingml::Color& rExtrusionColor,
                                           const bool bBlockExtrusion)
{
    // We convert rnMSOShapeRotation, so that Shape::createAndInsert() can use rRotZ the same way in
    // all cases.
    rRotZ = basegfx::deg2rad<60000>(-rnMSOShapeRotation);

    if (!p3DProperties || (p3DProperties && !(*p3DProperties).mnPreset.has_value()))
        return false;

    const sal_Int32 nCameraPrstID((*p3DProperties).mnPreset.value());
    sal_Int16 nPrstCameraIndex
        = getPrstCameraIndex(oox::drawingml::Generic3DProperties::getCameraPrstName(nCameraPrstID));
    if (nPrstCameraIndex < 0 or nPrstCameraIndex >= nCameraPresetCount)
        return false; // error in document. OOXML specifies a fixed set of preset camera types.
    mnPrstCameraIndex = nPrstCameraIndex;

    // We use extrusion, if there is a rotation around x-axis or y-axis,
    // or if there is no such rotation but we have a perspective projection with true depth,
    // or we have a parallel projection other than a 'front' type.
    // In other cases the rendering as normal shape is better than any current extrusion.
    double fX = 0.0;
    double fY = 0.0;
    Scene3DHelper::getAPIAnglesFrom3DProperties(p3DProperties, rnMSOShapeRotation, fX, fY, rRotZ);
    sal_Int32 nDepthAmount = (*p3DProperties).mnExtrusionH.value_or(0);
    bool bIsParallel = aPrstCameraValuesArray[mnPrstCameraIndex].mbIsParallel;
    bool bIsParallelFrontType
        = (nCameraPrstID == XML_legacyObliqueFront) || (nCameraPrstID == XML_orthographicFront);
    bool bCreateExtrusion = (!basegfx::fTools::equalZero(fX) || !basegfx::fTools::equalZero(fY))
                            || (!bIsParallel && nDepthAmount > 0)
                            || (bIsParallel && !bIsParallelFrontType);

    // Extrusion color is not handled as extrusion property but as shape property. Thus deliver it
    // in any case, so that Shape::createAndInsert() knows about it.
    rExtrusionColor = (*p3DProperties).maExtrusionColor;

    if (!bCreateExtrusion || bBlockExtrusion)
        return false;

    // Create the extrusion properties in rPropertyMap so that they can be directly used.
    // Turn extrusion on
    rPropertyMap.setProperty(oox::PROP_Extrusion, true);

    // Dummy value. Will be changed from evaluating the material properties, when implemented.
    rPropertyMap.setProperty(oox::PROP_Diffusion, 100.0);

    // Camera properties
    css::drawing::ProjectionMode eProjectionMode = bIsParallel
                                                       ? css::drawing::ProjectionMode_PARALLEL
                                                       : css::drawing::ProjectionMode_PERSPECTIVE;
    rPropertyMap.setProperty(oox::PROP_ProjectionMode, eProjectionMode);

    Scene3DHelper::addRotateAngleToMap(rPropertyMap, fX, fY);

    Scene3DHelper::addProjectionGeometryToMap(p3DProperties, rPropertyMap, bIsParallel,
                                              rnMSOShapeRotation);

    // Shape properties
    Scene3DHelper::addExtrusionDepthToMap(p3DProperties, rPropertyMap);

    // The 'automatic' extrusion color is different in MS Office. Thus we enable it in any case.
    // CreateAndInsert method will set a suitable 'automatic' color, if rExtrusionColor is not used.
    rPropertyMap.setProperty(oox::PROP_Color, true);
    // ToDo: Some materials might need ShadeMode_Smooth or ShadeMode_PHONG.
    rPropertyMap.setProperty(oox::PROP_ShadeMode, css::drawing::ShadeMode_FLAT);

    return true;
}

namespace
{
/* This struct is used to hold light properties for a light in a preset light rig.*/
struct MSOLight
{
    // Values are as specified in [MS-OI29500], see commit message.
    // The color is specified as RGBA, but alpha value is always 1.0 and ignored anyway, so it is
    // dropped here. The RGB values are in decimal, but might exceed the usual [0;1] range.
    double fMSOColorR;
    double fMSOColorG;
    double fMSOColorB;
    // MSO uses 4 decimals precision, some light directions are not normalized.
    double fMSOLightDirectionX;
    double fMSOLightDirectionY;
    double fMSOLightDirectionZ;
    double fScale;
    double fOffset;
    bool bSpecular;
    bool bDiffuse;
};

/* This struct is used to hold properties of a light rig*/
struct PrstLightRigValues
{
    // values are as specified in [MS-OI29500], see commit message.
    std::u16string_view sLightRigName; // identifies the light rig, mandatory in OOXML
    // The ambient color is specified as RGBA, but alpha value is always 1.0 and R = B = G. Thus we
    // store here only one value.
    std::optional<double> fAmbient;
    // Each rig has at least one light and maximal four lights
    MSOLight aLight1;
    std::optional<MSOLight> aLight2;
    std::optional<MSOLight> aLight3;
    std::optional<MSOLight> aLight4;
    // Light rig rotation is not contained in the presets.
};
} // end anonymous namespace

// The values are taken from [MS-OI29500]. For details see the spreadsheet attached to
// tdf#70039 and the commit message.
constexpr sal_uInt16 nLightRigPresetCount(27); // Fix value, specified in OOXML standard.
constexpr PrstLightRigValues aPrstLightRigValuesArray[nLightRigPresetCount] = {
    { u"balanced",
      { 0.13 },
      { 1.05, 1.05, 1.05, 0.5263, -0.4092, -0.7453, 1, 0, true, true },
      { { 1, 1, 1, -0.9386, 0.3426, -0.041, 1, 0, true, true } },
      { { 0.5, 0.5, 0.5, 0.0934, 0.763, 0.6396, 1, 0, true, true } },
      {} },
    { u"brightRoom",
      { 1.5 },
      { 1, 1, 1, 0, -1, 0, 1, 0, false, true },
      { { 1, 1, 1, 0.8227, -0.1882, -0.5364, 1, 0, true, false } },
      { { -0.5, -0.5, -0.5, 0, 0, -1, 1, 0, false, true } },
      { { 0.5, 0.5, 0.5, 0, 1, 0, 1, 0, false, true } } },
    { u"chilly",
      { 0.11 },
      { 0.31, 0.32, 0.32, 0.6574, -0.7316, -0.1806, 1, 0, true, true },
      { { 0.45, 0.45, 0.45, -0.3539, -0.1505, -0.9231, 1, 0, false, true } },
      { { 1.03, 1.02, 1.15, 0.672, -0.6185, -0.4073, 1, 0, false, true } },
      { { 0.41, 0.45, 0.48, -0.5781, 0.7976, 0.1722, 1, 0, true, true } } },
    { u"contrasting",
      { 1 },
      { 1, 1, 1, 0, -1, 0, 1, 0, true, false },
      { { 1, 1, 1, 0, 1, 0, 1, 0, true, false } },
      {},
      {} },
    { u"flat",
      { 1 },
      { 0.821, 0.821, 0.821, -0.9546, -0.1619, -0.2502, 1, 0, true, false },
      { { 2.072, 2.54, 2.91, 0.0009, 0.8605, 0.5095, 1, 0, true, false } },
      { { 3.843, 3.843, 3.843, 0.6574, -0.7316, -0.1806, 1, 0, true, false } },
      {} },
    { u"flood",
      { 0.13 },
      { 1.1, 1.1, 1.1, 0.5685, -0.7651, -0.3022, 1, 0, true, true },
      { { 1.1, 1.1, 1.1, -0.2366, -0.9595, -0.1531, 1, 0, true, true } },
      { { 0.55, 0.55, 0.55, -0.8982, 0.1386, -0.4171, 1, 0, true, true } },
      {} },
    { u"freezing",
      {},
      { 0.53, 0.567, 0.661, 0.6574, -0.7316, -0.1806, 1, 0, true, true },
      { { 0.37, 0.461, 0.461, -0.2781, -0.4509, -0.8482, 1, 0, false, true } },
      { { 0.649, 0.638, 0.904, 0.672, -0.6185, -0.4073, 1, 0, false, true } },
      { { 0.971, 1.19, 1.363, -0.1825, 0.968, 0.1722, 1, 0, true, true } } },
    { u"glow",
      { 1 },
      { 1, 1, 1, 0, -1, 0, 1, 0, true, true },
      { { 0.7, 0.7, 0.7, 0, 1, 0, 1, 0, true, true } },
      {},
      {} },
    { u"harsh",
      { 0.28 },
      { 0.88, 0.88, 0.88, 0.6689, -0.6755, -0.3104, 1, 0, true, true },
      { { 0.88, 0.88, 0.88, -0.592, -0.7371, -0.326, 1, 0, true, true } },
      {},
      {} },
    { u"legacyFlat1",
      { 0.305 },
      { 0.58, 0.58, 0.58, 0, 0, -0.2, 1, 0, true, true },
      { { 0.58, 0.58, 0.58, 0, 0, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyFlat2",
      { 0.305 },
      { 0.58, 0.58, 0.58, -1, -1, -0.2, 1, 0, true, true },
      { { 0.58, 0.58, 0.58, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyFlat3",
      { 0.305 },
      { 0.58, 0.58, 0.58, 0, -1, -0.2, 1, 0, true, true },
      { { 0.58, 0.58, 0.58, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyFlat4",
      { 0.305 },
      { 0.58, 0.58, 0.58, 1, -1, -0.2, 1, 0, true, true },
      { { 0.58, 0.58, 0.58, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyHarsh1",
      { 0.061 },
      { 0.793, 0.793, 0.793, 0, 0, -0.2, 1, 0, true, true },
      { { 0.214, 0.214, 0.214, 0, 0, -0.2, 1, 0, false, true } },
      {},
      {} },
    { u"legacyHarsh2",
      { 0.061 },
      { 0.793, 0.793, 0.793, -1, -1, -0.2, 1, 0, true, true },
      { { 0.214, 0.214, 0.214, 0, 1, -0.2, 1, 0, false, true } },
      {},
      {} },
    { u"legacyHarsh3",
      { 0.061 },
      { 0.793, 0.793, 0.793, 0, -1, -0.2, 1, 0, true, true },
      { { 0.214, 0.214, 0.214, 0, 1, -0.2, 1, 0, false, true } },
      {},
      {} },
    { u"legacyHarsh4",
      { 0.061 },
      { 0.793, 0.793, 0.793, 1, -1, -0.2, 1, 0, true, true },
      { { 0.214, 0.214, 0.214, 0, 1, -0.2, 1, 0, false, true } },
      {},
      {} },
    { u"legacyNormal1",
      { 0.153 },
      { 0.671, 0.671, 0.671, 0, 0, -0.2, 1, 0, true, true },
      { { 0.366, 0.366, 0.366, 0, 0, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyNormal2",
      { 0.153 },
      { 0.671, 0.671, 0.671, -1, -1, -0.2, 1, 0, true, true },
      { { 0.366, 0.366, 0.366, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyNormal3",
      { 0.153 },
      { 0.671, 0.671, 0.671, 0, -1, -0.2, 1, 0, true, true },
      { { 0.366, 0.366, 0.366, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"legacyNormal4",
      { 0.153 },
      { 0.671, 0.671, 0.671, 1, -1, -0.2, 1, 0, true, true },
      { { 0.366, 0.366, 0.366, 0, 1, -0.2, 0.5, 0, false, true } },
      {},
      {} },
    { u"morning",
      {},
      { 0.669, 0.648, 0.596, 0.6574, -0.7316, -0.1806, 0.5, 0.5, true, true },
      { { 0.459, 0.454, 0.385, -0.2781, -0.4509, -0.8482, 1, 0, false, true } },
      { { 0.9, 0.86, 0.83, 0.672, -0.6185, -0.4073, 1, 0, false, true } },
      { { 0.911, 0.846, 0.728, -0.1825, 0.968, 0.1722, 1, 0, true, true } } },
    { u"soft", { 0.3 }, { 0.8, 0.8, 0.8, -0.6897, 0.2484, -0.6802, 1, 0, true, true }, {}, {}, {} },
    { u"sunrise",
      {},
      { 0.667, 0.63, 0.527, 0.6574, -0.7316, -0.1806, 1, 0, true, true },
      { { 0.459, 0.459, 0.371, -0.2781, -0.4509, -0.8482, 1, 0, false, true } },
      { { 0.826, 0.712, 0.638, 0.672, -0.6185, -0.4073, 1, 0, false, true } },
      { { 1.511, 1.319, 0.994, -0.1825, 0.968, 0.1722, 1, 0, false, true } } },
    { u"sunset",
      {},
      { 0.672, 0.169, 0.169, 0.6574, -0.7316, -0.1806, 1, 0, true, true },
      { { 0.459, 0.448, 0.327, 0.0922, -0.3551, -0.9303, 1, 0, false, true } },
      { { 0.775, 0.612, 0.502, 0.672, -0.6185, -0.4073, 1, 0, false, true } },
      { { 0.761, 0.69, 0.397, -0.424, 0.8891, 0.1722, 1, 0, false, true } } },
    { u"threePt",
      {},
      { 1.141, 1.141, 1.141, -0.6515, -0.2693, -0.7093, 1, 0, true, true },
      { { 0.5, 0.5, 0.5, 0.8482, 0.2469, -0.4686, 1, 0, true, true } },
      { { 1, 1, 1, 0.5634, -0.2812, 0.7769, 1, 0, true, true } },
      {} },
    { u"twoPt",
      { 0.25 },
      { 0.84, 0.84, 0.84, 0.5266, -0.4089, -0.7454, 0, 0, true, true },
      { { 0.3, 0.3, 0.3, -0.8983, 0.2365, -0.3704, 1, 0, true, true } },
      {},
      {} }
};

namespace
{
/** Searches for the item in aPrstLightRigValuesArray with given sPresetName.
    @param [in] sPresetName name as specified in OOXML standard
    @return returns the index if item exists, otherwise -1.*/
sal_Int16 lcl_getPrstLightRigIndex(std::u16string_view sPresetName)
{
    sal_Int16 nIt(0);
    while (nIt < nLightRigPresetCount && aPrstLightRigValuesArray[nIt].sLightRigName != sPresetName)
        ++nIt;
    if (nIt >= nLightRigPresetCount)
    {
        nIt = -1; // Error is handled by caller
    }
    return nIt;
}

/** Extracts the light directions from the preset lightRig.
    @param [in] rLightRig from which the lights are extracted
    @param [out] rLightDirVec contains the preset lights but each as B3DVector*/
void lcl_getLightDirectionsFromRig(const PrstLightRigValues& rLightRig,
                                   std::vector<basegfx::B3DVector>& rLightDirVec)
{
    auto addLightDir = [&](const MSOLight& aMSOLight) {
        basegfx::B3DVector aLightDir(aMSOLight.fMSOLightDirectionX, aMSOLight.fMSOLightDirectionY,
                                     aMSOLight.fMSOLightDirectionZ);
        rLightDirVec.push_back(std::move(aLightDir));
    };
    // aLight1 always exists, the others are optional
    addLightDir(rLightRig.aLight1);
    if (rLightRig.aLight2.has_value())
        addLightDir(rLightRig.aLight2.value());
    if (rLightRig.aLight3.has_value())
        addLightDir(rLightRig.aLight3.value());
    if (rLightRig.aLight4.has_value())
        addLightDir(rLightRig.aLight4.value());
}

/** Converts the directions from MSO specification to coordinates in the shape coordinate system.
    @details The extruded shape uses a left-hand Cartesian coordinate system with x-axis right, y-axis
    down and z-axis towards observer. When L(Lx,Ly,Lz) is the specified light direction, then
    V(-Ly, -Lx, Lz) is the direction in the shape coordinate system.
    @param [in,out] rLightDirVec contains for each individual light its direction.*/
void lcl_AdaptAndNormalizeLightDirections(std::vector<basegfx::B3DVector>& rLightDirVec)
{
    basegfx::B3DHomMatrix aTransform; // unit matrix
    aTransform.set(0, 0, 0.0);
    aTransform.set(0, 1, -1.0);
    aTransform.set(1, 0, -1.0);
    aTransform.set(1, 1, 0.0);
    for (auto& rDirection : rLightDirVec)
    {
        rDirection *= aTransform;
        rDirection.normalize();
    }
}

/** Gets the rotation angles fX and fY from the extrusion property RotateAngle in the map.
    Does nothing if property does not exist.
    @param [in] rPropertyMap should contain valid value in RotateAngle property
    @param [out] fX, fY rotation angle in unit rad with orientation as in API.*/
void lcl_getXYAnglesFromMap(oox::PropertyMap& rPropertyMap, double& rfX, double& rfY)
{
    if (!rPropertyMap.hasProperty(oox::PROP_RotateAngle))
        return;
    css::drawing::EnhancedCustomShapeParameterPair aAnglePair;
    css::uno::Any aAny = rPropertyMap.getProperty(oox::PROP_RotateAngle);
    if (aAny >>= aAnglePair)
    {
        rfX = basegfx::deg2rad(aAnglePair.First.Value.get<double>());
        rfY = basegfx::deg2rad(aAnglePair.Second.Value.get<double>());
    }
}

/** Applies the rotations given in fX, fY, fZ to the light directions.
    @details The rotations are applied in the order fZ, fY, fX. All angles have unit rad. The
        orientation of the angles fX and fY is the same as in the extrusion property RotateAngle in
        API. The orientation of angle fZ is the same as in shape property RotateAngle in API.
    @param [in, out] rLightDirVec contains the to be transformed light directions
    @param [in] fX angle for rotation around x-axis
    @param [in] fY angle for rotation around y-axis
    @param {in] fZ angle for rotation around z-axis*/
void lcl_ApplyShapeRotationToLights(std::vector<basegfx::B3DVector>& rLightDirVec, const double& fX,
                                    const double& fY, const double& fZ)
{
    basegfx::B3DHomMatrix aTransform; // unit matrix
    // rotate has the order first x, then y, last z. We need order z, y, x.
    aTransform.rotate(0.0, 0.0, -fZ);
    aTransform.rotate(0.0, -fY, 0.0);
    aTransform.rotate(fX, 0.0, 0.0);
    for (auto it = rLightDirVec.begin(); it != rLightDirVec.end(); ++it)
        (*it) *= aTransform;
}

/** Applies the light rig rotation to the directions of the individual lights
    @details A light rig has a mandatory attribute 'dir' for rotating the rig in 45deg steps. It might
        have an element 'rot', that describes a rotation by spherical coordinates 'lat', 'lon' and
        'rev'. The element has precedence over the attribute.
    @param [in] p3DProperties contains info about light rig.
    @param {in, out] rLightDirVec contains for each individual light its direction in shape coordinate
        system with x-axis right, y-axis down, z-axis toward observer.*/
void lcl_IncorporateRigRotationIntoLightDirections(
    const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
    std::vector<basegfx::B3DVector>& rLightDirVec)
{
    basegfx::B3DHomMatrix aTransform; // unit matrix
    // if a 'rot' element exists, then all of 'lat', 'lon' and 'rev' needs to exist.
    if ((*p3DProperties).maLightRigRotation.mnLatitude.has_value())
    {
        double fLat
            = basegfx::deg2rad<60000>((*p3DProperties).maLightRigRotation.mnLatitude.value_or(0));
        double fLon
            = basegfx::deg2rad<60000>((*p3DProperties).maLightRigRotation.mnLongitude.value_or(0));
        double fRev
            = basegfx::deg2rad<60000>((*p3DProperties).maLightRigRotation.mnRevolution.value_or(0));
        aTransform.rotate(0.0, 0.0, fRev);
        aTransform.rotate(fLat, fLon, 0.0);
    }
    else
    {
        sal_Int32 nDir = 0;
        switch ((*p3DProperties).mnLightRigDirection.value_or(XML_t))
        {
            case XML_t:
                nDir = 0;
                break;
            case XML_tr:
                nDir = 45;
                break;
            case XML_r:
                nDir = 90;
                break;
            case XML_br:
                nDir = 135;
                break;
            case XML_b:
                nDir = 180;
                break; // or -180
            case XML_bl:
                nDir = -135;
                break;
            case XML_l:
                nDir = -90;
                break;
            case XML_tl:
                nDir = -45;
                break;
            default:
                nDir = 0;
        }
        // Rotation is always only around z-axis
        aTransform.rotate(0.0, 0.0, basegfx::deg2rad(nDir));
    }
    for (auto& rDirection : rLightDirVec)
        rDirection *= aTransform;
}

/** The lights in OOXML are basically incompatible with our lights. We try to tweak some rigs to
    reduce obvious problems.
    @param [in, out] rLightDirVec light directions with already incorporated rotations
    @param [in, out] rLightRig the to be tweaked rig
*/
void lcl_tweakLightRig(std::vector<basegfx::B3DVector>& rLightDirVec, PrstLightRigValues& rLightRig)
{
    if (rLightRig.sLightRigName == u"brightRoom")
    {
        // The fourth light has more significant direction.
        if (rLightDirVec.size() >= 4 && rLightRig.aLight2.has_value()
            && rLightRig.aLight4.has_value())
        {
            std::swap(rLightDirVec[1], rLightDirVec[3]);
            // swap fourth and second in light rig too, swap their other properties too.
            MSOLight aTemp = rLightRig.aLight4.value();
            rLightRig.aLight4 = rLightRig.aLight2.value();
            rLightRig.aLight2 = aTemp;
            // and make it brighter, 1.0 instead of 0.5
            rLightRig.aLight2.value().fMSOColorR = 1.0;
            rLightRig.aLight2.value().fMSOColorG = 1.0;
            rLightRig.aLight2.value().fMSOColorB = 1.0;
        }
        // The object is far too bright.
        rLightRig.fAmbient = 0.6; // instead 1.5
    }
    else if (rLightRig.sLightRigName == u"chilly" || rLightRig.sLightRigName == u"flood")
    {
        // They are too dark.
        rLightRig.fAmbient = 0.35; // instead 0.11 resp. 0.13
    }
    else if (rLightRig.sLightRigName == u"freezing" || rLightRig.sLightRigName == u"morning"
             || rLightRig.sLightRigName == u"sunrise" || rLightRig.sLightRigName == u"threePt")
    {
        // These rigs have no ambient color but three or four lights. The objects are too dark with
        // only two lights.
        rLightRig.fAmbient = 0.4;
    }
    else if (rLightRig.sLightRigName == u"sunset")
    {
        // The fourth light is more significant.
        if (rLightDirVec.size() >= 4 && rLightRig.aLight4.has_value())
        {
            MSOLight aTemp = rLightRig.aLight2.value();
            rLightRig.aLight2 = rLightRig.aLight4.value();
            rLightRig.aLight4 = aTemp;
            std::swap(rLightDirVec[1], rLightDirVec[3]);
        }
    }
    else if (rLightRig.sLightRigName == u"soft")
    {
        // This is the only modern light rig with Scale=0.5 and Offset=0.5. It would be harsh=false
        // and specular=true at the same time. We switch specular off as that is used to set harsh on.
        rLightRig.aLight1.bSpecular = false;
    }
}

} // end anonymous namespace

void Scene3DHelper::setLightingProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                          const double& rfRotZ, oox::PropertyMap& rPropertyMap)
{
    if (!p3DProperties || (p3DProperties && !(*p3DProperties).mnLightRigType.has_value()))
        return;

    // get index of light rig in aPrstLightRigValuesArray
    const sal_Int32 nLightRigPrstID((*p3DProperties).mnLightRigType.value()); // token
    sal_Int16 nPrstLightRigIndex = lcl_getPrstLightRigIndex(
        oox::drawingml::Generic3DProperties::getLightRigName(nLightRigPrstID));
    if (nPrstLightRigIndex < 0 or nPrstLightRigIndex >= nLightRigPresetCount)
        return; // error in document. OOXML specifies a fixed set of preset light rig types.

    // The light rig is copied because it might be tweaked later.
    PrstLightRigValues aLightRig = aPrstLightRigValuesArray[nPrstLightRigIndex];

    std::vector<basegfx::B3DVector> aLightDirVec;
    aLightDirVec.reserve(4);
    lcl_getLightDirectionsFromRig(aLightRig, aLightDirVec);
    lcl_AdaptAndNormalizeLightDirections(aLightDirVec);

    lcl_IncorporateRigRotationIntoLightDirections(p3DProperties, aLightDirVec);

    // Parts (1) to (6) are workarounds for the problem that our current model as well as API and
    // ODF are not able to describe or use the capabilities of extruded custom shapes of MS Office.
    // If the implementation is improved one day, the parts will need to be adapted.

    // (1) Moving the camera around does not change shape or light directions for modern cameras in
    // MS Office. For legacy cameras MS Office behaves same as LibreOffice: Not the camera is moved
    // but the shape is rotated. For modern cameras we need to rotate the light rig the same way as
    // the shape to get a similar illumination as in MS Office.
    if (mnPrstCameraIndex < 20 || 37 < mnPrstCameraIndex)
    {
        double fX = 0.0; // unit rad, orientation as in API
        double fY = 0.0; // unit rad, orientation as in API
        lcl_getXYAnglesFromMap(rPropertyMap, fX, fY);
        lcl_ApplyShapeRotationToLights(aLightDirVec, fX, fY, rfRotZ);
    }

    // (2) We try to tweak some light rigs a little bit, e.g. make sure the first light is specular
    // or add some ambient light instead of not possible third or forth light.
    lcl_tweakLightRig(aLightDirVec, aLightRig);

    rPropertyMap.setProperty(oox::PROP_Brightness, aLightRig.fAmbient.value_or(0) * 100);

    // (3) A 3D-scene of an extruded custom shape has currently no colored light, but only a
    // level. We get the level from Red.
    rPropertyMap.setProperty(oox::PROP_FirstLightLevel, aLightRig.aLight1.fMSOColorR * 100);

    // (4) 'Specular' and 'Diffuse' in the MSO specification belong to modern 3D geometry. That is not
    // available in our legacy one. Here we treat 'Specular' as property 'Harsh' and ignore 'Diffuse'.
    rPropertyMap.setProperty(oox::PROP_FirstLightHarsh, aLightRig.aLight1.bSpecular);

    // (5) In fact we have stored position in FirstLightDirection and SecondLightDirection,
    // not direction, thus the minus sign.
    css::drawing::Direction3D aLightPos;
    aLightPos.DirectionX = -aLightDirVec[0].getX();
    aLightPos.DirectionY = -aLightDirVec[0].getY();
    aLightPos.DirectionZ = -aLightDirVec[0].getZ();
    rPropertyMap.setProperty(oox::PROP_FirstLightDirection, aLightPos);

    // (6) For extruded custom shapes only two lights are possible although our rendering engine has
    // eight lights. We will loose lights.
    if (aLightDirVec.size() > 1)
    {
        rPropertyMap.setProperty(oox::PROP_SecondLightLevel,
                                 aLightRig.aLight2.value().fMSOColorR * 100);
        rPropertyMap.setProperty(oox::PROP_SecondLightHarsh, aLightRig.aLight2.value().bSpecular);
        aLightPos.DirectionX = -aLightDirVec[1].getX();
        aLightPos.DirectionY = -aLightDirVec[1].getY();
        aLightPos.DirectionZ = -aLightDirVec[1].getZ();
        rPropertyMap.setProperty(oox::PROP_SecondLightDirection, aLightPos);
    }
    else
        rPropertyMap.setProperty(oox::PROP_SecondLightLevel, 0.0); // prevent defaults.
}

namespace
/** This struct is used to hold material values for extruded custom shapes. Because we cannot yet
    render all material properties MS Office uses, the values are adapted to our current abilities.*/
{
struct MaterialValues
{
    std::u16string_view msMaterialPrstName; // identifies the material type
    // Corresponds to MS Office 'Diffuse Color' and 'Ambient Color'.
    double fDiffusion;
    double fSpecularity; // Corresponds to MS Office 'Specular Color'.
    // Corresponds to our 'Shininess' as 2^(Shininess/10) = nSpecularPower.
    sal_uInt8 nSpecularPower;
    bool bMetal; // Corresponds to MS Office 'Metal'
    // constants com::sun::star::drawing::EnhancedCustomShapeMetalType
    // MetalMSCompatible belongs to 'legacyMetal' material type.
    std::optional<sal_Int16> oMetalType; // MetalODF = 0, MetalMSCompatible = 1
    // MS Office properties 'Emissive Color', 'Diffuse Fresnel', 'Alpha Fresnel' and 'Blinn Highlight'
    // are not contained.
};
} // end anonymous namespace

// OOXML standard has a fixed amount of 15 material types. The type 'legacyWireframe' is special and
// thus is handled separately. A spreadsheet with further remarks is attached to tdf#70039.
constexpr sal_uInt16 nPrstMaterialCount(14);
constexpr MaterialValues aPrstMaterialArray[nPrstMaterialCount]
    = { { u"clear", 100, 60, 20, false, {} },
        { u"dkEdge", 70, 100, 35, false, {} },
        { u"flat", 100, 80, 50, false, {} },
        { u"legacyMatte", 100, 0, 0, false, {} },
        { u"legacyMetal", 66.69921875, 122.0703125, 32, true, { 1 } },
        { u"legacyPlastic", 100, 122.0703125, 32, false, {} },
        { u"matte", 100, 0, 0, false, {} },
        { u"metal", 100, 100, 12, true, { 0 } },
        { u"plastic", 100, 60, 12, true, { 0 } },
        { u"powder", 100, 30, 10, false, {} },
        { u"softEdge", 100, 100, 35, false, {} },
        { u"softmetal", 100, 100, 8, true, { 0 } },
        { u"translucentPowder", 100, 30, 10, true, { 0 } },
        { u"warmMatte", 100, 30, 8, false, {} } };

void Scene3DHelper::setMaterialProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                          oox::PropertyMap& rPropertyMap)
{
    if (!p3DProperties)
        return;

    // PowerPoint does not write aus prstMaterial="warmMatte", but handles it as default.
    const sal_Int32 nMaterialID = (*p3DProperties).mnMaterial.value_or(XML_warmMatte); // token

    // special handling for 'legacyWireframe'
    if (nMaterialID == XML_legacyWireframe)
    {
        // This is handled via shade mode of the scene.
        rPropertyMap.setProperty(oox::PROP_ShadeMode, css::drawing::ShadeMode_DRAFT);
        // Notice, the color of the strokes will be different from MS Office, because LO uses the
        // shape line color even if the line style is 'none', whereas MS Office uses contour color or
        // Black.
        return;
    }

    sal_Int16 nIdx(0); // Index into aPrstMaterialArray
    while (nIdx < nPrstMaterialCount
           && aPrstMaterialArray[nIdx].msMaterialPrstName
                  != oox::drawingml::Generic3DProperties::getPresetMaterialTypeString(nMaterialID))
        ++nIdx;
    if (nIdx >= nPrstMaterialCount)
        return; // error in document

    // extrusion-diffuse, extrusion-specularity-loext
    rPropertyMap.setProperty(oox::PROP_Diffusion, aPrstMaterialArray[nIdx].fDiffusion);
    rPropertyMap.setProperty(oox::PROP_Specularity, aPrstMaterialArray[nIdx].fSpecularity);

    // extrusion-shininess
    double fShininess = 0.0;
    // Conversion 2^(fShininess/10) = nSpecularPower
    if (aPrstMaterialArray[nIdx].nSpecularPower > 0)
        fShininess = 10.0 * std::log2(aPrstMaterialArray[nIdx].nSpecularPower);
    rPropertyMap.setProperty(oox::PROP_Shininess, fShininess);

    // extrusion-metal, extrusion-metal-type
    rPropertyMap.setProperty(oox::PROP_Metal, aPrstMaterialArray[nIdx].bMetal);
    if (aPrstMaterialArray[nIdx].bMetal)
    {
        sal_Int16 eMetalType = aPrstMaterialArray[nIdx].oMetalType.value_or(0) == 1
                                   ? css::drawing::EnhancedCustomShapeMetalType::MetalMSCompatible
                                   : css::drawing::EnhancedCustomShapeMetalType::MetalODF;
        rPropertyMap.setProperty(oox::PROP_MetalType, eMetalType);
    }
}

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
