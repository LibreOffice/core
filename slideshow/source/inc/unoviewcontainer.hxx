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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEWCONTAINER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEWCONTAINER_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <vector>
#include <memory>

#include "unoview.hxx"


namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }

/* Definition of UnoViewContainer class */

namespace slideshow
{
    namespace internal
    {
        /** Contains UnoViews
         */
        class UnoViewContainer
        {
        public:
            UnoViewContainer();
            UnoViewContainer(const UnoViewContainer&) = delete;
            UnoViewContainer& operator=(const UnoViewContainer&) = delete;

            /** Add a view to this container

                @return true, if the view was successfully added
                (false is e.g. returned, if the view was already
                added)
             */
            bool addView( const UnoViewSharedPtr& rView );

            /** Remove a previously added a view from this container

                @return the View object, if this view was successfully
                removed, and an empty shared_ptr otherwise (e.g. if
                this view wasn't added in the first place)
            */
            UnoViewSharedPtr removeView( const css::uno::Reference<css::presentation::XSlideShowView >& xView );

            /// Dispose all stored views. Implies clear().
            void dispose();

            // the following parrots STL container concept methods
            // ===================================================

            bool empty() const { return maViews.empty(); }

            UnoViewVector::iterator         begin() { return maViews.begin(); }
            UnoViewVector::const_iterator   begin() const { return maViews.begin(); }
            UnoViewVector::iterator         end() { return maViews.end(); }
            UnoViewVector::const_iterator   end() const { return maViews.end(); }

        private:
            /// All added views
            UnoViewVector   maViews;
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEWCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
