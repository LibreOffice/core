/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OGL_BUFFERCONTEXT_HXX_
#define OGL_BUFFERCONTEXT_HXX_

#include <sal/config.h>
#include <boost/shared_ptr.hpp>

namespace oglcanvas
{
    struct IBufferContext
    {
        virtual ~IBufferContext() {}

        /// start render to buffer. changes gl current context
        virtual bool startBufferRendering() = 0;

        /// end render to buffer. switches to window context, and selects rendered texture
        virtual bool endBufferRendering() = 0;
    };

    typedef ::boost::shared_ptr<IBufferContext> IBufferContextSharedPtr;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
