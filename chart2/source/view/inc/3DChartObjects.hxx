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

#include <vcl/OpenGLContext.hxx>

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
    glm::mat4 maPos;
    Color maColor; // RGBA fill color
};

class Line : public Renderable3DObject
{
private:
    glm::vec3 maPosBegin;
    glm::vec3 maPosEnd;
    Color maLineColor; // RGBA line color
};

class Text : public Renderable3DObject
{
private:
    BitmapEx maText;
    glm::vec3 maTopLeft;
    glm::vec3 maTopRight;
    glm::vec3 maBottomRight;
};

class Rectangle : public Renderable3DObject
{
private:
    glm::vec3 maTopLeft;
    glm::vec3 maTopRight;
    glm::vec3 maBottomRight;
    Color maColor; // RGBA fill color
    Color maLineColor; // RGBA line color
};

class Camera : public Renderable3DObject
{
public:
    Camera();
private:
    glm::vec3 maPos;
    glm::vec3 maDirection;
};

namespace temporary {

class TemporaryContext
{
public:
    TemporaryContext();

    void init();
    void render();

private:
    OpenGLContext maContext;

    int miWidth;
    int miHeight;
};

}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
