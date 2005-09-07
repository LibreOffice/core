/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodetools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:45:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#ifndef _COM_SUN_STAR_ANIMATIONS_TIMING_HPP_
#include <com/sun/star/animations/Timing.hpp>
#endif

#include <tools.hxx>
#include <nodetools.hxx>


using namespace ::com::sun::star;

namespace presentation
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

        AttributableShapeSharedPtr lookupAttributableShape( const LayerManagerSharedPtr&                rLayerManager,
                                                            const uno::Reference< drawing::XShape >&    xShape          )
        {
            ShapeSharedPtr pShape( rLayerManager->lookupShape( xShape ) );

            ENSURE_AND_THROW( pShape.get(),
                              "lookupAttributableShape(): no shape found for given XShape" );

            AttributableShapeSharedPtr pRes(
                ::boost::dynamic_pointer_cast< AttributableShape >( pShape ) );

            // TODO(E3): Cannot throw here, people might set animation info
            // for non-animatable shapes from the API. AnimationNodes must catch
            // the exception and handle that differently
            ENSURE_AND_THROW( pRes.get(),
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
