/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <epoxy/gl.h>

#include <sal/config.h>
#include <memory>

namespace oglcanvas
{
    struct IBufferContext
    {
        virtual ~IBufferContext() {}

        /// start render to buffer. changes current framebuffer
        virtual void startBufferRendering() = 0;

        /// end render to buffer. switches to default framebuffer
        virtual void endBufferRendering() = 0;

        virtual GLuint getTextureId() = 0;
    };

    typedef std::shared_ptr<IBufferContext> IBufferContextSharedPtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
