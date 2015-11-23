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
#ifndef INCLUDED_OGLTRANS_OPERATIONS_HXX_
#define INCLUDED_OGLTRANS_OPERATIONS_HXX_

#include <config_lgpl.h>
#include <glm/gtc/type_ptr.hpp>

#include <boost/noncopyable.hpp>

#include <memory>

/** This class is to be derived to make any operation (transform) you may need in order to construct your transitions
*/
class Operation : private boost::noncopyable
{
public:
    virtual ~Operation(){}

protected:
    /** Should this operation be interpolated . If TRUE, the transform will smoothly move from making no difference from t = 0.0 to mnT0 to being completely transformed from t = mnT1 to 1. If FALSE, the transform will be inneffectual from t = 0 to mnT0, and completely transformed from t = mnT0 to 1.
    */
    bool mbInterpolate;

    /** time to begin the transformation
    */
    double mnT0;

    /** time to finish the transformation
    */
    double mnT1;
public:
    /** this is the function that is called to give the Operation to OpenGL.

        @param t
        time from t = 0 to t = 1

        @param SlideWidthScale
        width of slide divided by width of window

        @param SlideHeightScale
        height of slide divided by height of window

    */
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const = 0;

protected:
    Operation(bool bInterpolate, double nT0, double nT1):
        mbInterpolate(bInterpolate), mnT0(nT0), mnT1(nT1){}
};

/** this class is a generic CounterClockWise(CCW) rotation with an axis angle
*/
class SRotate: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    /** Constructor

        @param Axis
        axis to rotate about

        @param Origin
        position that rotation axis runs through

        @param Angle
        angle in radians of CCW rotation

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SRotate(const glm::vec3& Axis, const glm::vec3& Origin, double Angle,
            bool bInter, double T0, double T1);
    virtual ~SRotate(){}
private:
    /** axis to rotate CCW about
    */
    glm::vec3 axis;

    /** position that rotation axis runs through
    */
    glm::vec3 origin;

    /** angle in radians of CCW rotation
    */
    double angle;
};

std::shared_ptr<SRotate>
makeSRotate(const glm::vec3& Axis, const glm::vec3& Origin, double Angle,
        bool bInter, double T0, double T1);

/** scaling transformation
*/
class SScale: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    /** Constructor

        @param Scale
        amount to scale by

        @param Origin
        position that rotation axis runs through

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1);
    virtual ~SScale(){}
private:
    glm::vec3 scale;
    glm::vec3 origin;
};

std::shared_ptr<SScale>
makeSScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1);

/** translation transformation
*/
class STranslate: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    /** Constructor

        @param Vector
        vector to translate

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    STranslate(const glm::vec3& Vector,bool bInter, double T0, double T1);
    virtual ~STranslate(){}
private:
    /** vector to translate by
    */
    glm::vec3 vector;
};

std::shared_ptr<STranslate>
makeSTranslate(const glm::vec3& Vector,bool bInter, double T0, double T1);

/** translation transformation
*/
class SEllipseTranslate: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    /** Constructor

        @param Vector
        vector to translate

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1);
    virtual ~SEllipseTranslate(){}
private:
    /** width and length of the ellipse
     */
    double width, height;

    /** start and end position on the ellipse <0,1>
     */
    double startPosition;
    double endPosition;
};

std::shared_ptr<SEllipseTranslate>
makeSEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1);

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the width of the window.
*/
class RotateAndScaleDepthByWidth: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    RotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle, bool bScale, bool bInter, double T0, double T1);
    virtual ~RotateAndScaleDepthByWidth(){}
private:
    glm::vec3 axis;
    glm::vec3 origin;
    double angle;
    bool scale;
};

std::shared_ptr<RotateAndScaleDepthByWidth>
makeRotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle, bool bScale, bool bInter, double T0, double T1);

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the height of the window.
*/
class RotateAndScaleDepthByHeight: public Operation
{
public:
    virtual void interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const override;

    RotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle, bool bScale, bool bInter, double T0, double T1);
    virtual ~RotateAndScaleDepthByHeight(){}
private:
    glm::vec3 axis;
    glm::vec3 origin;
    double angle;
    bool scale;
};

std::shared_ptr<RotateAndScaleDepthByHeight>
makeRotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle, bool bScale, bool bInter, double T0, double T1);

#endif // INCLUDED_SLIDESHOW_OPERATIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
