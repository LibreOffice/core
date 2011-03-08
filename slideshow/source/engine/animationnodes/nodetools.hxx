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

#ifndef INCLUDED_SLIDESHOW_NODETOOLS_HXX
#define INCLUDED_SLIDESHOW_NODETOOLS_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include "shapemanager.hxx"
#include "basenode.hxx"
#include "doctreenode.hxx"
#include "attributableshape.hxx"


#if defined(VERBOSE) && defined(DBG_UTIL)
# define DEBUG_NODES_SHOWTREE(a) debugNodesShowTree(a);
# define DEBUG_NODES_SHOWTREE_WITHIN(a) debugNodesShowTreeWithin(a);
#else
# define DEBUG_NODES_SHOWTREE(a)
# define DEBUG_NODES_SHOWTREE_WITHIN(a)
#endif

namespace slideshow
{
    namespace internal
    {

        // Tools
        //=========================================================================

#if defined(VERBOSE) && defined(DBG_UTIL)
        int& debugGetCurrentOffset();
        void debugNodesShowTree( const BaseNode* );
        void debugNodesShowTreeWithin( const BaseNode* );
#endif

        /** Look up an AttributableShape from ShapeManager.

            This method retrieves an AttributableShape pointer, given
            an XShape and a LayerManager.

            Throws a runtime exception if there's no such shape, or if
            it does not implement the AttributableShape interface.
         */
        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const ::com::sun::star::uno::Reference<
                                                                ::com::sun::star::drawing::XShape >&    xShape          );

        /** Predicate whether a Begin, Duration or End timing is
            indefinite, i.e. either contains no value, or the
            value Timing_INDEFINITE.
        */
        bool isIndefiniteTiming( const ::com::sun::star::uno::Any& rAny );

        /// Extract the node type from the user data
        bool getNodeType( sal_Int16&                                 o_rNodeType,
                          const ::com::sun::star::uno::Sequence<
                              ::com::sun::star::beans::NamedValue >& rValues );
    }
}

#endif /* INCLUDED_SLIDESHOW_NODETOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
