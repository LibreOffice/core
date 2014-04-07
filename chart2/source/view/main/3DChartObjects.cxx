/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "3DChartObjects.hxx"

namespace chart {

namespace opengl3D {

Bar::Bar(const glm::mat4& rPosition):
    maPos(rPosition)
{
}

namespace temporary {

void TemporaryContext::init()
{
    maContext.init();
}

void TemporaryContext::render()
{

}

}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
