/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/types.h>
#include <rtl/ref.hxx>

namespace basegfx
{
    class B2DHomMatrix;
    class B2DRange;
}
namespace vclcanvas
{
    class Canvas;
    struct ViewState;
}

/* Definition of Action interface */

namespace cppcanvas
    {
        /** Interface for internal render actions

            This interface is implemented by all objects generated
            from the metafile renderer, and corresponds roughly to the
            VCL meta action.
         */
        class Action
        {
        public:

            virtual ~Action() {}

            /** Render this action to the associated canvas

                @param rTransformation
                Transformation matrix to apply before rendering

                @return true, if rendering was successful. If
                rendering failed, false is returned.
             */
            virtual bool render( vclcanvas::Canvas& rCanvas,
                                 const vclcanvas::ViewState& rViewState,
                                 const ::basegfx::B2DHomMatrix& rTransformation ) const = 0;

            /** Query action count.

                This method returns the number of subset actions
                contained in this action. The render( Subset ) method
                must accept subset ranges up to the value returned
                here.

                @return the number of subset actions
             */
            virtual sal_Int32 getActionCount() const = 0;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
