/*************************************************************************
 *
 *  $RCSfile: attributableshape.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:13:35 $
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

#ifndef _SLIDESHOW_ATTRIBUTABLESHAPE_HXX
#define _SLIDESHOW_ATTRIBUTABLESHAPE_HXX

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


namespace presentation
{
    namespace internal
    {
        class DocTreeNode;

        // forward declaration necessary, because methods use AttributableShapeSharedPtr
        class AttributableShape;

        typedef ::boost::shared_ptr< AttributableShape > AttributableShapeSharedPtr;

        /** Represents an animatable shape, that can have its
            attributes changed.

            Over an animatable shape, this interface adds attribute
            modification methods. Furthermore, the shape can be
            queried for sub items, which in turn can be separated out
            into own AttributableShapes.
         */
        class AttributableShape : public AnimatableShape
        {
        public:
            // Attribute layer methods
            //------------------------------------------------------------------

            /** Create a new shape attribute layer.

                This method creates a new layer for shape attributes,
                which lies atop of all previous attribute layers. That
                is most typically used when a new SMIL animation
                starts (which according to the spec always lies atop
                of all previous animations). Thus, subsequent calls to
                this method generate a sandwich of attribute layers,
                which in total define the shape's attributes.

                Please note that the attribute layers do <em>not</em>
                contain the underlying XShape's attributes as
                default. Instead, attributes not explicitely set by
                animations remain in invalid state, allowing the
                shape's paint method to determine whether they have to
                override the underlying graphical shape
                representation. XShape attributes must be passed
                explicitely to animations which need them (e.g. 'by'
                animations).

                @return the new layer
             */
            virtual ShapeAttributeLayerSharedPtr createAttributeLayer() = 0;

            /** Revoke a previously generated attribute layer.

                This method revokes a previously generated attribute
                layer, and removes the effect of that layer from this
                shape. The layer need not be the current toplevel
                layer, it can also be revoked from in between.

                @param rLayer
                Layer to revoke. Must have been generated by
                createAttributeLayer() at the same Shape.

                @return true, if layer was successfully removed, false
                otherwise (e.g. if the given layer was not generated
                for this shape).
             */
            virtual bool revokeAttributeLayer( const ShapeAttributeLayerSharedPtr& rLayer ) = 0;

            /** Change shape visibility

                This method hides or unhides a shape. Note that every
                attribute layer generated for this shape will override
                the setting given here, until it is revoked.

                @param bVisible
                When true, shape will be visible, when false,
                invisible (modulo attribute layer overrides).
             */
            virtual void setVisibility( bool bVisible ) = 0;


            // Sub item attributes
            //------------------------------------------------------------------

            /** Query number of tree nodes this shape contains.

                The value returned by this method minus one is the
                maximum value permissible at the
                getFormattingDocTreeNode/getLogicalDocTreeNode methods.
             */
            virtual sal_Int32 getNumberOfTreeNodes() const = 0; // throw ImportFailedException;

            /** Query a formatted DocTreeNode for this shape.

                There are two document trees available for shapes:
                this method returns top level nodes for the formatting
                tree, which gives information about paragraph and line
                breaks.
             */
            virtual DocTreeNode getFormattingDocTreeNode( sal_Int32 nIndex ) const = 0; // throw ImportFailedException;

            /** Query a logical DocTreeNode for this shape.

                There are two document trees available for shapes:
                this method returns top level nodes for the logical
                document tree, which gives information about
                paragraphs, sentences, words and character cells.
             */
            virtual DocTreeNode getLogicalDocTreeNode( sal_Int32 nIndex ) const = 0; // throw ImportFailedException;

            /** Query the current subset in effect.

                This method returns a tree node denoting the currently
                active subset. If this shape is not a subset shape,
                an empty tree node should be returned.
             */
            virtual DocTreeNode getEffectiveSubset() const = 0;

            /** Query a subset Shape

                This method queries a clone of this Shape, which
                renders only the selected subset of itself.

                @param rTreeNode
                A DocTreeNode instance queried from this Shape, which
                specifies the subset of the Shape to render

                @return a NULL Shape pointer, if no subset exists for
                the given DocTreeNode.
            */
            virtual AttributableShapeSharedPtr querySubset( const DocTreeNode& rTreeNode ) const = 0;

            /** Create a subset Shape

                This method creates a clone of this Shape, which
                renders only the selected subset of itself. Multiple
                createSubset() calls for the same DocTreeNode will all
                share the same subset shape.

                The original shape (i.e. the one this method is called
                on) will cease to display the selected subset
                part. That is, together the shapes will display the
                original content, but the content of all subset shapes
                and their original shape will always be mutually
                disjunct.

                After deregistering the subset shape a matching number
                of times via revokeSubset(), the original shape will
                resume displaying the subsetted part.

                @attention To maintain integrity, this method should
                only be called from the LayerManager

                @param o_rSubset
                The requested Shape

                @param rTreeNode
                A DocTreeNode instance queried from this Shape, which
                specifies the subset of the Shape to render

                @return true, if the shape was newly created, and
                false, if an already existing subset is returned.
            */
            virtual bool createSubset( AttributableShapeSharedPtr&  o_rSubset,
                                       const DocTreeNode&           rTreeNode ) = 0;

            /** Revoke a previously generated shape subset.

                After revoking a subset shape, the corresponding
                subset part will become visible again on the original
                shape.

                @attention To maintain integrity, this method should
                only be called from the LayerManager

                @param rShape
                The subset to revoke

                @return true, if the last client called
                revokeSubset().
             */
            virtual bool revokeSubset( const AttributableShapeSharedPtr& rShape ) = 0;
        };

        typedef ::boost::shared_ptr< AttributableShape > AttributableShapeSharedPtr;

    }
}

#endif /* _SLIDESHOW_ATTRIBUTABLESHAPE_HXX */
