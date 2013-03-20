/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
#if 1

#include <com/sun/star/uno/Reference.hxx>
#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star { namespace drawing
{
    class XShape;
} } } }

/* Definition of ShapeCursorEventHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling view events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle view events.
         */
        class ShapeCursorEventHandler
        {
        public:
            virtual ~ShapeCursorEventHandler() {}

            virtual bool cursorChanged( const ::com::sun::star::uno::Reference<
                                           ::com::sun::star::drawing::XShape>&   xShape,
                                        sal_Int16                                nCursor ) = 0;
        };

        typedef ::boost::shared_ptr< ShapeCursorEventHandler > ShapeCursorEventHandlerSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_SHAPECURSOREVENTHANDLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
