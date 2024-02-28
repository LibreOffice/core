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
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>

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

    // Extrusion color enabled?
    rPropertyMap.setProperty(oox::PROP_Color, rExtrusionColor.isUsed());

    return true;
}

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
