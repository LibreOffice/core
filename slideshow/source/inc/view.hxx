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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_VIEW_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_VIEW_HXX

#include "viewlayer.hxx"

#include <memory>
#include <vector>


namespace basegfx { class B2DRange; class B2DVector; }


/* Definition of View interface */

namespace slideshow
{
    namespace internal
    {
        class View : public ViewLayer
        {
        public:
            /** Create a new view layer for this view

                @param rLayerBounds
                Specifies the bound rect of the layer relative to the
                user view coordinate system.

                This method sets the bounds of the view layer in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
             */
            virtual ViewLayerSharedPtr createViewLayer( const basegfx::B2DRange& rLayerBounds ) const = 0;

            /** Update screen representation from backbuffer
             */
            virtual bool updateScreen() const = 0;

            /** Paint screen content unconditionally from backbuffer
             */
            virtual bool paintScreen() const = 0;

            /** Set the size of the user view coordinate system.

                This method sets the width and height of the view in
                document coordinates (i.e. 'logical' coordinates). The
                resulting transformation is then concatenated with the
                underlying view transformation, returned by the
                getTransformation() method.
            */
            virtual void setViewSize( const ::basegfx::B2DSize& ) = 0;

            /** Change the view's mouse cursor.

                @param nPointerShape
                One of the css::awt::SystemPointer
                constant group members.
             */
            virtual void setCursorShape( sal_Int16 nPointerShape ) = 0;
        };

        typedef std::shared_ptr< View >     ViewSharedPtr;
        typedef std::vector< ViewSharedPtr >  ViewVector;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_VIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
