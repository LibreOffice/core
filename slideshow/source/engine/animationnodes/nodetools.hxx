/*************************************************************************
 *
 *  $RCSfile: nodetools.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:06:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

        /** generate vector of subsets, to show sequence of given
            DocTreeNode::NodeType.

            The output vector will receive all DocTreeNodes of the
            given type, which are contained in the given shape. Note
            that the output DocTreeNodes are flat, i.e. don't contain
            any children.
         */
        void generateSubsets( DocTreeNode::VectorOfDocTreeNodes&    o_rOutput,
                              const AttributableShapeSharedPtr&     pShape,
                              DocTreeNode::NodeType                 eNodeType );

        /** generate vector of subsets, to show sequence of given
            DocTreeNode::NodeType.

            This method starts subset generation below paragraph
            nPara. This is for iterations with a ParagraphTarget
            target attribute, where the iteration is to be applied
            only to the selected paragraph number.
        */
        void generateSubsets( DocTreeNode::VectorOfDocTreeNodes&    o_rOutput,
                              const AttributableShapeSharedPtr&     pShape,
                              DocTreeNode::NodeType                 eNodeType,
                              sal_Int32                             nPara );

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
