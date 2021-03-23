/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>

#include <drawingml/customshapeproperties.hxx>
#include <drawingml/shape3dproperties.hxx>

namespace oox
{
struct CameraSettings
{
    // OOXML Camera, CT_Camera
    std::u16string_view msType; // OOXML attribute "prst"

    // OOXML attribute "fov" (field of view angle), "Perspective" in UI in MS Office
    // unit 1/600000 degree, range [0..180] deg in spec, [0..120] deg in UI in MS Office
    // ToDo remove multiply by 60000 in export
    sal_uInt32 mnFOVAngle;

    // OOXML childelement <dir> (Direction), rotation
    // range [0..360[ deg, unit 1/60000 degree
    sal_uInt32 mnRotLon; // Longitude, MS Office UI X, OOXML attribute "lon", axis vertical
    sal_uInt32 mnRotLat; // Latitude, MS Office UI Y, OOXML attribute "lat", axis horizontal
    sal_uInt32 mnRotRev; // Revolution, MS Office UI Z, OOXML attribute "rev"

    // ODF camera, attributes of element <draw:enhanced-geometry>
    // Settings belong to property set "Extrusion" in "CustomShapeGeometry". Complex components are
    // not defined with their API types. That makes it easier to write a map using an initalizer list

    // dr3d:projection, value "parallel" or "perspective"
    // API css::drawing::ProjectionMode, use string here because of initializer list
    std::u16string_view msProjectionMode;

    // draw:extrusion-skew, type string. API Skew, type EnhancedCustomShapeParameterPair
    // only evaluated for projection mode "parallel"
    double mfSkewAmount; // range 0 to 100, percent of depth used as slant length
    double mfSkewAngle; // range [0..360[, internal unit degree

    // draw:extrusion-origin, type string. API Origin, type EnhancedCustomShapeParameterPair
    // range [-0.5 (left).. 0.5 (right)] and [-0.5 (top) 0.5 (right)], relative to shape
    double mfOriginX;
    double mfOriginY;

    // draw:extrusion-viewpoint, type point3D, length with unit. API ViewPoint, type Position3D
    // Internal unit 1/100mm. Only evaluated for projection 'perspective'
    // LO uses it relative to the rotated scene, evaluated together with 'Origin'.
    // LO has no UI for it, but it is needed as counterpart to mnFOVAngle.
    double mfViewPointX;
    double mfViewPointY;
    double mfViewPointZ;

    // draw:extrusion-rotation-angle, type string.
    // API RotateAngle, type EnhancedCustomShapeParameterPair,
    // internal unit degree, values beyond -360 deg and 360 deg are allowed
    // Rotation center is the center of the shape, see comment below
    double mfRotateAngleFirst; // at x-axis, positive means lower part of front face comes nearer
    double mfRotateAngleSecond; // at y-axis, positiv means right part of front face comes nearer
    // ODF does rotation at z-axis on generating shape, not in the scene camera.
    double mfRotateAngleZ; // counter-clockwise on screen, unit degree

    //ctor with default values
    constexpr CameraSettings()
        : msType(std::u16string_view(u"orthographicFront"))
        , mnFOVAngle(270000) // 45deg
        , mnRotLon(0)
        , mnRotLat(0)
        , mnRotRev(0)
        , msProjectionMode(std::u16string_view(u"parallel"))
        , mfSkewAmount(0.0)
        , mfSkewAngle(0.0)
        , mfOriginX(0.0)
        , mfOriginY(0.0)
        , mfViewPointX(0.0)
        , mfViewPointY(0.0)
        , mfViewPointZ(25000.0)
        , mfRotateAngleFirst(0.0)
        , mfRotateAngleSecond(0.0)
        , mfRotateAngleZ(0.0)
    {
    }

    // ctor with value list
    constexpr CameraSettings(std::u16string_view _msType, sal_uInt32 _mnFOVAngle,
                             sal_uInt32 _mnRotLon, sal_uInt32 _mnRotLat, sal_uInt32 _mnRotRev,
                             std::u16string_view _msProjectionMode, double _mfSkewAmount,
                             double _mfSkewAngle, double _mfOriginX, double _mfOriginY,
                             double _mfViewPointX, double _mfViewPointY, double _mfViewPointZ,
                             double _mfRotateAngleFirst, double _mfRotateAngleSecond,
                             double _mfRotateAngleZ)
        : msType(_msType)
        , mnFOVAngle(_mnFOVAngle)
        , mnRotLon(_mnRotLon)
        , mnRotLat(_mnRotLat)
        , mnRotRev(_mnRotRev)
        , msProjectionMode(_msProjectionMode)
        , mfSkewAmount(_mfSkewAmount)
        , mfSkewAngle(_mfSkewAngle)
        , mfOriginX(_mfOriginX)
        , mfOriginY(_mfOriginY)
        , mfViewPointX(_mfViewPointX)
        , mfViewPointY(_mfViewPointY)
        , mfViewPointZ(_mfViewPointZ)
        , mfRotateAngleFirst(_mfRotateAngleFirst)
        , mfRotateAngleSecond(_mfRotateAngleSecond)
        , mfRotateAngleZ(_mfRotateAngleZ)
    {
    }

    // copy ctor
    constexpr CameraSettings(const CameraSettings& rOther)
        : msType(rOther.msType)
        , mnFOVAngle(rOther.mnFOVAngle)
        , mnRotLon(rOther.mnRotLon)
        , mnRotLat(rOther.mnRotLat)
        , mnRotRev(rOther.mnRotRev)
        , msProjectionMode(rOther.msProjectionMode)
        , mfSkewAmount(rOther.mfSkewAmount)
        , mfSkewAngle(rOther.mfSkewAngle)
        , mfOriginX(rOther.mfOriginX)
        , mfOriginY(rOther.mfOriginY)
        , mfViewPointX(rOther.mfViewPointX)
        , mfViewPointY(rOther.mfViewPointY)
        , mfViewPointZ(rOther.mfViewPointZ)
        , mfRotateAngleFirst(rOther.mfRotateAngleFirst)
        , mfRotateAngleSecond(rOther.mfRotateAngleSecond)
        , mfRotateAngleZ(rOther.mfRotateAngleZ)
    {
    }

    // assignment
    CameraSettings& operator=(const CameraSettings& rOther)
    {
        if (this != &rOther)
        {
            msType = rOther.msType;
            mnFOVAngle = rOther.mnFOVAngle;
            mnRotLon = rOther.mnRotLon;
            mnRotLat = rOther.mnRotLat;
            mnRotRev = rOther.mnRotRev;
            msProjectionMode = rOther.msProjectionMode;
            mfSkewAmount = rOther.mfSkewAmount;
            mfSkewAngle = rOther.mfSkewAngle;
            mfOriginX = rOther.mfOriginX;
            mfOriginY = rOther.mfOriginY;
            mfViewPointX = rOther.mfViewPointX;
            mfViewPointY = rOther.mfViewPointY;
            mfViewPointZ = rOther.mfViewPointZ;
            mfRotateAngleFirst = rOther.mfRotateAngleFirst;
            mfRotateAngleSecond = rOther.mfRotateAngleSecond;
            mfRotateAngleZ = rOther.mfRotateAngleZ;
        }
    }

    // dummy to test compiling
    double getRotateAngleZ();
};

// The OOXML camera attribute "zoom" is not included in above struct, because ODF has no corresponding
// attribute, Powerpoint has no UI for it and LO cannot render it in custom shape extrusion scene.
// ToDo: Examine, whether preserve in grabBag is possible and meaningful.

// The ODF attribute draw:extrusion-rotation-center is not included in above struct. LO can read and
// render it, but has no UI for it. OOXML has no corresponding property but assumes center of shape
// (without extrusion part) as rotation center, which is default in LO and ODF too.
// API "RotationCenter", internal unit 1/100 mm, right hand system with x-axis to right, y-axis down,
// z-axis into the screen. Problem: Read/Write to ODF is not in shape-size fraction as specified in
// ODF and described in API, but in 1/100 mm without unit.

struct CameraMapper
{
    CameraMapper() {}

    // Dummy method to test Compilation
    CameraSettings getDefaultCameraSettings();
    void fillExtrusionWithPresetValues(const OUString sPresetName,
                                       oox::drawingml::Extrusion& rExtrusion,
                                       double& rShapeRotation);
    sal_uInt16 getPrstIndex(const OUString& rPresetType);
    double getRotationZDegFromPrst(const sal_uInt16 nPresetIndex);
    void getRotationAngleOOXFromPrst(oox::drawingml::RotationProperties& rRot,
                                     const sal_uInt16 nPresetIndex);
    void addCameraPrstSettingsToExtrusion(oox::drawingml::Extrusion& rExtrusion,
                                          const sal_uInt16 nPresetIndex);
    void calculateRotationAngleLODegFromAngleOOX(
        css::drawing::EnhancedCustomShapeParameterPair& rSceneRotDeg, double& rShapeRotDeg,
        const oox::drawingml::RotationProperties& aOOXRot);
}; // end struct */

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
