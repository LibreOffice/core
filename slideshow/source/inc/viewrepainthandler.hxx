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

#ifndef INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX
#define INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "unoview.hxx"


/* Definition of ViewRepaintHandler interface */

namespace slideshow
{
    namespace internal
    {

        /** Interface for handling view repaint events.

            Classes implementing this interface can be added to an
            EventMultiplexer object, and are called from there to
            handle view repaint events.
         */
        class ViewRepaintHandler
        {
        public:
            virtual ~ViewRepaintHandler() {}

            /** Notify clobbered view.

                Reasons for a viewChanged notification can be
                different view size, transformation, or other device
                properties (color resolution or profile, etc.)

                @param rView
                The changed view
             */
            virtual void viewClobbered( const UnoViewSharedPtr& rView ) = 0;
        };

        typedef ::boost::shared_ptr< ViewRepaintHandler > ViewRepaintHandlerSharedPtr;
        typedef ::boost::weak_ptr< ViewRepaintHandler >   ViewRepaintHandlerWeakPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_VIEWREPAINTHANDLER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
