/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef INCLUDED_SLIDESHOW_DISPOSABLE_HXX
#define INCLUDED_SLIDESHOW_DISPOSABLE_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


/* Definition of Disposable interface */

namespace slideshow
{
    namespace internal
    {
        /** Disposable interface

            With ref-counted objects, deleting object networks
            containing cycles requires a dispose() call, to enforce
            every object to call dispose on and release local
            references.
         */
        class Disposable
        {
        public:
            virtual ~Disposable() {}

            /** Dispose all object references.

                An implementor of this method must first call
                dispose() on any of its external references, and
                release them after that.
             */
            virtual void dispose() = 0;
        };

    }
}

#endif /* INCLUDED_SLIDESHOW_DISPOSABLE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
