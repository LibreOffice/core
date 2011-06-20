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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <com/sun/star/animations/Timing.hpp>

#include <tools.hxx>
#include <nodetools.hxx>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
#if defined(VERBOSE) && defined(DBG_UTIL)
        int& debugGetCurrentOffset()
        {
            static int lcl_nOffset = 0; // to make each tree output distinct

            return lcl_nOffset;
        }

        void debugNodesShowTree( const BaseNode* pNode )
        {
            if( pNode )
                pNode->showState();

            ++debugGetCurrentOffset();
        }

        void debugNodesShowTreeWithin( const BaseNode* pNode )
        {
            if( pNode )
                pNode->showTreeFromWithin();

            ++debugGetCurrentOffset();
        }
#endif

        AttributableShapeSharedPtr lookupAttributableShape( const ShapeManagerSharedPtr&                rShapeManager,
                                                            const uno::Reference< drawing::XShape >&    xShape          )
        {
            ENSURE_OR_THROW( rShapeManager,
                              "lookupAttributableShape(): invalid ShapeManager" );

            ShapeSharedPtr pShape( rShapeManager->lookupShape( xShape ) );

            ENSURE_OR_THROW( pShape,
                              "lookupAttributableShape(): no shape found for given XShape" );

            AttributableShapeSharedPtr pRes(
                ::boost::dynamic_pointer_cast< AttributableShape >( pShape ) );

            // TODO(E3): Cannot throw here, people might set animation info
            // for non-animatable shapes from the API. AnimationNodes must catch
            // the exception and handle that differently
            ENSURE_OR_THROW( pRes,
                              "lookupAttributableShape(): shape found does not implement AttributableShape interface" );

            return pRes;
        }

        bool isIndefiniteTiming( const uno::Any& rAny )
        {
            if( !rAny.hasValue() )
                return true;

            animations::Timing eTiming;

            if( !(rAny >>= eTiming) ||
                eTiming != animations::Timing_INDEFINITE )
            {
                return false;
            }

            return true;
        }

        /// Extract the node type from the user data
        bool getNodeType( sal_Int16&                                            o_rNodeType,
                          const uno::Sequence< beans::NamedValue >&             rValues )
        {
            beans::NamedValue aNamedValue;

            if( findNamedValue( &aNamedValue,
                                rValues,
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("node-type") ) ) )
            {
                if( (aNamedValue.Value >>= o_rNodeType) )
                    return true;
            }

            return false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
