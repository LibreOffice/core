/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <glm/glm.hpp>

#include <tools/color.hxx>
#include <vcl/bitmapex.hxx>

namespace chart {

namespace opengl3D {

class Renderable3DObject
{
public:

    virtual ~Renderable3DObject() {};

    virtual void render() {}
};

class Bar : public Renderable3DObject
{
public:
    Bar( const glm::mat4& rPosition );
private:
    bool mbRoundedCorners;
    glm::mat4 maPos;
    Color maColor; // RGBA fill color
    sal_Int32 nUniqueId;
};

class Line : public Renderable3DObject
{
private:
    glm::vec3 maPosBegin;
    glm::vec3 maPosEnd;
    Color maLineColor; // RGBA line color
    sal_Int32 nUniqueId;
};

class Text : public Renderable3DObject
{
private:
    BitmapEx maText;
    glm::vec3 maTopLeft;
    glm::vec3 maBottomRight;
    sal_Int32 nUniqueId;
};

class Rectangle : public Renderable3DObject
{
private:
    glm::vec3 maTopLeft;
    glm::vec3 maBottomRight;
    sal_Int32 nUniqueId;
    Color maColor; // RGBA fill color
    Color maLineColor; // RGBA line color
};

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
