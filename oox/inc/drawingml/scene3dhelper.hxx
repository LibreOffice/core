/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <drawingml/shape3dproperties.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/helper/propertymap.hxx>

namespace oox
{
class Scene3DHelper
{
public:
    /** Creates extrusion properties in rPropertyMap from the 3D information in p3DProperties and
        gives values which have to be set as shape properties.
        @param [in] p3DProperties a pointer to Shape3DProperties
        @param [in] rnMSOShapeRotation a MSO shape rotation angle in 1/60000 deg
        @param [in, out] rPropertyMap a map that is filled with properties directly usable in
            CustomShapeProperties.pushToPropSet() for property Extrusion.
        @param [out] rRotZ the angle for shape rotation around z-axis in rad, with orientation
            same as API shape property RotateAngle. It has the shape rotation from rnMSOShapeRotation
            integrated.
        @param [out] rExtrusionColor a complex color from which the color of the extruded faces can
            be created. The extrusion color is not handled as extrusion property but as secondary
            color in the style of the shape. If no 3D exist or the camera is invalid, the parameter
            value is unchanged.
        @param [in] This optional parameter bBlockExtrusion can be removed when tdf#159912 is
            fixed. For details see the bugreport.
        @return Returns true if extrusion properties are generated. Returns false if there is no 3D
            or if rendering without 3D is currently better. rPropertyMap is unchanged in such case.
    */
    bool setExtrusionProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                const sal_Int32& rnMSOShapeRotation, oox::PropertyMap& rPropertyMap,
                                double& rRotZ, oox::drawingml::Color& rExtrusionColor,
                                const bool bBlockExtrusion = false);

    /** Creates lighting properties in rPropertyMap from MSO preset and shape rotation
        @param [in] p3DProperties a pointer to Shape3DProperties
        @param [in] rRotZ the shape rotation inclusive camera z-rotation as calculated by
            setExtrusionProperties()
        @param [in, out] rPropertyMap the map, that was already filled by setExtrusionProperties()*/
    void setLightingProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                               const double& rRotZ, oox::PropertyMap& rPropertyMap);

private:
    /** Calculates angles suitable for API from OOXML scene3d angles.
        @param [in] nLat, nLon, nRev in unit 1/60000 deg with same orientation as the attributes lat,
        lon and rev of the <rot> child element of the <scene3d> element in OOXML markup.
        @param [out] fX, fY, fZ values in unit radians with correct orientation for API properties
            EnhancedCustomShapeExtrusion::RotateAngle and RotationDescriptor::RotateAngle*/
    static void getAPIAnglesFromOOXAngle(const sal_Int32 nLat, const sal_Int32 nLon,
                                         const sal_Int32 nRev, double& fX, double& fY, double& fZ);

    /** Calculates angles suitable for API from Shape3DProperties.
        @details It considers the preset camera in case the optional, direct rotation angles have no
            value. It integrates the given rnMSOShapeRotation into fZ.
        @param [in] p3DProperties a pointer to Shape3DProperties
        @param [in] rnMSOShapeRotation rotation in 1/60000 deg as given in 'rot' attribute of 'xfrm'
            element in OOXML.
        @param [out] fX, fY, fZ values in unit radians with correct orientation for API properties
            EnhancedCustomShapeExtrusion::RotateAngle and RotationDescriptor::RotateAngle*/
    void getAPIAnglesFrom3DProperties(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                      const sal_Int32& rnMSOShapeRotation, double& fX, double& fY,
                                      double& fZ);

    /** Adds the rotation angles fX and fY as property RotateAngle to the map.
        @param [in, out] rPropertyMap a map to add the RotateAngle property
        @param [in] fX, fY rotation angle in unit rad with correct orientation for the property.*/
    static void addRotateAngleToMap(oox::PropertyMap& rPropertyMap, const double fX,
                                    const double fY);

    /** Adds the Depth property to the map.
        @details The second component is relative, whereas the z-position in OOXML is absolute. Uses
        360EMU depth in case of zero Depth as otherwise no relative position is possible.
        @param [in] p3DProperties a pointer to Shape3DProperties
        @param [in, out] rPropertyMap a map to add the Depth property*/
    static void addExtrusionDepthToMap(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                       oox::PropertyMap& rPropertyMap);

    /** Adds the projection mode itself and the associated camera parameters to the map.
        @details Both modes add ProjectionMode and Origin properties. Adds Skew property in case of
        mode PARALLEL and ViewPoint property in case of mode PERSPECTIVE. The Skew angles includes the
        shape rotation because MSOffice rotates after creating the projection and ODF before.
        @param [in] p3DProperties a pointer to Shape3DProperties
        @param [in, out] rPropertyMap a map to add ProjectMode, Origin and Skew or ViewPoint
            properties.
        @param [in] bIsParallel true for mode PARALLEL, false for PERSPECTIVE
        @param [in] rnMSOShapeRotation shape rotation in 1/60000 degree*/
    void addProjectionGeometryToMap(const oox::drawingml::Shape3DPropertiesPtr p3DProperties,
                                    oox::PropertyMap& rPropertyMap, const bool bIsParallel,
                                    const sal_Int32 rnMSOShapeRotation);

    // Index into array aPrstCameraValuesArray.
    sal_Int16 mnPrstCameraIndex = -1; // '-1' means invalid or not yet searched
}; // end class Scene3DHelper

} // end namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
