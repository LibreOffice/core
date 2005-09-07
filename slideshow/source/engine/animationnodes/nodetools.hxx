/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodetools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:45:19 $
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

#ifndef _SLIDESHOW_NODETOOLS_HXX
#define _SLIDESHOW_NODETOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#include <layermanager.hxx>
#include <basenode.hxx>
#include <doctreenode.hxx>
#include <attributableshape.hxx>


#if defined(VERBOSE) && defined(DBG_UTIL)
# define DEBUG_NODES_SHOWTREE(a) debugNodesShowTree(a);
# define DEBUG_NODES_SHOWTREE_WITHIN(a) debugNodesShowTreeWithin(a);
#else
# define DEBUG_NODES_SHOWTREE(a)
# define DEBUG_NODES_SHOWTREE_WITHIN(a)
#endif

namespace presentation
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

        /** Look up an AttributableShape from LayerManager.

            This method retrieves an AttributableShape pointer, given
            an XShape and a LayerManager.

            Throws a runtime exception if there's no such shape, or if
            it does not implement the AttributableShape interface.
         */
        AttributableShapeSharedPtr lookupAttributableShape( const LayerManagerSharedPtr&                rLayerManager,
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

#endif /* _SLIDESHOW_NODETOOLS_HXX */
