/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2015 by Collabora, Ltd.
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

#include <sal/config.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <basegfx/numeric/ftools.hxx>

#include "OGLTrans_Operation.hxx"

SRotate::SRotate(const glm::vec3& Axis, const glm::vec3& Origin,
        double Angle, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(basegfx::deg2rad(Angle))
{
}

SScale::SScale(const glm::vec3& Scale, const glm::vec3& Origin,
        bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    scale(Scale),
    origin(Origin)
{
}

RotateAndScaleDepthByWidth::RotateAndScaleDepthByWidth(const glm::vec3& Axis,
        const glm::vec3& Origin, double Angle, bool bScale, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(basegfx::deg2rad(Angle)),
    scale(bScale)
{
}

RotateAndScaleDepthByHeight::RotateAndScaleDepthByHeight(const glm::vec3& Axis,
        const glm::vec3& Origin, double Angle, bool bScale, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(basegfx::deg2rad(Angle)),
    scale(bScale)
{
}


STranslate::STranslate(const glm::vec3& Vector, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    vector(Vector)
{
}

std::shared_ptr<SRotate>
makeSRotate(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return std::make_shared<SRotate>(Axis, Origin, Angle, bInter, T0, T1);
}

std::shared_ptr<SScale>
makeSScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1)
{
    return std::make_shared<SScale>(Scale, Origin, bInter, T0, T1);
}

std::shared_ptr<STranslate>
makeSTranslate(const glm::vec3& Vector,bool bInter, double T0, double T1)
{
    return std::make_shared<STranslate>(Vector, bInter, T0, T1);
}

std::shared_ptr<SEllipseTranslate>
makeSEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1)
{
    return std::make_shared<SEllipseTranslate>(dWidth, dHeight, dStartPosition, dEndPosition, bInter, T0, T1);
}

std::shared_ptr<RotateAndScaleDepthByWidth>
makeRotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle, bool bScale, bool bInter, double T0, double T1)
{
    return std::make_shared<RotateAndScaleDepthByWidth>(Axis, Origin, Angle, bScale, bInter, T0, T1);
}

std::shared_ptr<RotateAndScaleDepthByHeight>
makeRotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bScale, bool bInter, double T0, double T1)
{
    return std::make_shared<RotateAndScaleDepthByHeight>(Axis, Origin, Angle, bScale, bInter, T0, T1);
}

inline double intervalInter(double t, double T0, double T1)
{
    return ( t - T0 ) / ( T1 - T0 );
}

void STranslate::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    matrix = glm::translate(matrix, glm::vec3(SlideWidthScale*t*vector.x, SlideHeightScale*t*vector.y, t*vector.z));
}

void SRotate::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, origin.z);
    glm::vec3 scale_vector(SlideWidthScale * SlideWidthScale, SlideHeightScale * SlideHeightScale, 1);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::scale(matrix, scale_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    matrix = glm::scale(matrix, 1.f / scale_vector);
    matrix = glm::translate(matrix, -translation_vector);
}

void SScale::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, origin.z);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::scale(matrix, static_cast<float>(1 - t) + static_cast<float>(t) * scale);
    matrix = glm::translate(matrix, -translation_vector);
}

void RotateAndScaleDepthByWidth::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, SlideWidthScale*origin.z);
    glm::vec3 scale_vector(SlideWidthScale * SlideWidthScale, SlideHeightScale * SlideHeightScale, 1);
    matrix = glm::translate(matrix, translation_vector);
    if (scale)
        matrix = glm::scale(matrix, scale_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    if (scale)
        matrix = glm::scale(matrix, 1.f / scale_vector);
    matrix = glm::translate(matrix, -translation_vector);
}

void RotateAndScaleDepthByHeight::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, SlideHeightScale*origin.z);
    glm::vec3 scale_vector(SlideWidthScale * SlideWidthScale, SlideHeightScale * SlideHeightScale, 1);
    matrix = glm::translate(matrix, translation_vector);
    if (scale)
        matrix = glm::scale(matrix, scale_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    if (scale)
        matrix = glm::scale(matrix, 1.f / scale_vector);
    matrix = glm::translate(matrix, -translation_vector);
}

SEllipseTranslate::SEllipseTranslate(double dWidth, double dHeight, double dStartPosition,
        double dEndPosition, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1)
{
    width = dWidth;
    height = dHeight;
    startPosition = dStartPosition;
    endPosition = dEndPosition;
}

void SEllipseTranslate::interpolate(glm::mat4& matrix, double t, double /* SlideWidthScale */, double /* SlideHeightScale */) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);

    double a1, a2, x, y;
    a1 = startPosition*2*M_PI;
    a2 = (startPosition + t*(endPosition - startPosition))*2*M_PI;
    x = width*(cos (a2) - cos (a1))/2;
    y = height*(sin (a2) - sin (a1))/2;

    matrix = glm::translate(matrix, glm::vec3(x, 0, y));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
