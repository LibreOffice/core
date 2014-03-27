/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace chart {

namespace 3d {

class Bar
{
private:
    bool mbRoundedCorners;
    glm::mat4 maPos;
    Color maColor; // RGBA fill color
    sal_Int32 nUniqueId;
}:

class Line
{
private:
    glm::vec3 maPosBegin;
    glm::vec3 maPosEnd;
    Color maLineColor; // RGBA line color
    sal_Int32 nUniqueId;
}:

class Text
{
private:
    BitmapEx maText;
    glm::vec3 maTopLeft;
    glm::vec3 maBottomRight;
    sal_Int32 nUniqueId;
};

class Rectangle
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
