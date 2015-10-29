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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_NODETOOLS_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_NODETOOLS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include "shapemanager.hxx"
#include "basenode.hxx"
#include "doctreenode.hxx"
#include "attributableshape.hxx"


#if defined(DBG_UTIL)
# define DEBUG_NODES_SHOWTREE(a) debugNodesShowTree(a);
#else
# define DEBUG_NODES_SHOWTREE(a)
#endif

namespace slideshow
{
    namespace internal
    {

        // Tools


#if defined(DBG_UTIL)
        int& debugGetCurrentOffset();
        void debugNodesShowTree( const BaseNode* );
#endif

        /** Look up an AttributableShape from ShapeManager.

            This method retrieves an AttributableShape pointer, given
            an XShape and a LayerManager.

            Throws a runtime exception if there's no such shape, or if
            it does not implement the AttributableShape interface.
         */
        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const css::uno::Reference< css::drawing::XShape >&    xShape );

        /** Predicate whether a Begin, Duration or End timing is
            indefinite, i.e. either contains no value, or the
            value Timing_INDEFINITE.
        */
        bool isIndefiniteTiming( const css::uno::Any& rAny );

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ANIMATIONNODES_NODETOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
