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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <comphelper/sequence.hxx>

#include <unordered_map>
#include <vector>

#include "targetpropertiescreator.hxx"
#include <tools.hxx>

namespace slideshow
{
namespace internal
{
    namespace
    {
        // Vector containing all properties for a given shape
        typedef ::std::vector< beans::NamedValue > VectorOfNamedValues;

        /** The hash map key

            This key contains both XShape reference and a paragraph
            index, as we somehow have to handle shape and paragraph
            targets with the same data structure.
         */
        struct ShapeHashKey
        {
            /// Shape target
            uno::Reference< drawing::XShape >   mxRef;

            /** Paragraph index.

                If this is a pure shape target, mnParagraphIndex is
                set to -1.
             */
            sal_Int16                           mnParagraphIndex;

            /// Comparison needed for unordered_map
            bool operator==( const ShapeHashKey& rRHS ) const
            {
                return mxRef == rRHS.mxRef && mnParagraphIndex == rRHS.mnParagraphIndex;
            }
        };

        // A hash functor for ShapeHashKey objects
        struct ShapeKeyHasher
        {
            ::std::size_t operator()( const ShapeHashKey& rKey ) const
            {
                // TODO(P2): Maybe a better hash function would be to
                // spread mnParagraphIndex to 32 bit: a0b0c0d0e0... Hakmem
                // should have a formula.

                // Yes it has:
                // x = (x & 0x0000FF00) << 8) | (x >> 8) & 0x0000FF00 | x & 0xFF0000FF;
                // x = (x & 0x00F000F0) << 4) | (x >> 4) & 0x00F000F0 | x & 0xF00FF00F;
                // x = (x & 0x0C0C0C0C) << 2) | (x >> 2) & 0x0C0C0C0C | x & 0xC3C3C3C3;
                // x = (x & 0x22222222) << 1) | (x >> 1) & 0x22222222 | x & 0x99999999;

                // Costs about 17 cycles on a RISC machine with infinite
                // instruction level parallelism (~42 basic
                // instructions). Thus I truly doubt this pays off...
                return reinterpret_cast< ::std::size_t >(rKey.mxRef.get()) ^ (rKey.mnParagraphIndex << 16);
            }
        };

        // A hash map which maps a XShape to the corresponding vector of initial properties
        typedef std::unordered_map< ShapeHashKey, VectorOfNamedValues, ShapeKeyHasher > XShapeHash;


        class NodeFunctor
        {
        public:
            explicit NodeFunctor(
                XShapeHash& rShapeHash,
                bool bInitial )
            :   mrShapeHash( rShapeHash ),
                mxTargetShape(),
                mnParagraphIndex( -1 ),
                mbInitial( bInitial)
            {
            }

            NodeFunctor( XShapeHash&                                rShapeHash,
                         const uno::Reference< drawing::XShape >&   rTargetShape,
                         sal_Int16                                  nParagraphIndex,
                         bool                                       bInitial) :
                mrShapeHash( rShapeHash ),
                mxTargetShape( rTargetShape ),
                mnParagraphIndex( nParagraphIndex ),
                mbInitial( bInitial )
            {
            }

            void operator()( const uno::Reference< animations::XAnimationNode >& xNode ) const
            {
                if( !xNode.is() )
                {
                    OSL_FAIL( "AnimCore: NodeFunctor::operator(): invalid XAnimationNode" );
                    return;
                }

                uno::Reference< drawing::XShape > xTargetShape( mxTargetShape );
                sal_Int16                         nParagraphIndex( mnParagraphIndex );

                switch( xNode->getType() )
                {
                    case animations::AnimationNodeType::ITERATE:
                    {
                        // extract target shape from iterate node
                        // (will override the target for all children)

                        uno::Reference< animations::XIterateContainer > xIterNode( xNode,
                                                                                   uno::UNO_QUERY );

                        // TODO(E1): I'm not too sure what to expect here...
                        if( !xIterNode->getTarget().hasValue() )
                        {
                            OSL_FAIL( "animcore: NodeFunctor::operator(): no target on ITERATE node" );
                            return;
                        }

                        xTargetShape.set( xIterNode->getTarget(),
                                          uno::UNO_QUERY );

                        if( !xTargetShape.is() )
                        {
                            css::presentation::ParagraphTarget aTarget;

                            // no shape provided. Maybe a ParagraphTarget?
                            if( !(xIterNode->getTarget() >>= aTarget) )
                            {
                                OSL_FAIL( "animcore: NodeFunctor::operator(): could not extract any "
                                            "target information" );
                                return;
                            }

                            xTargetShape = aTarget.Shape;
                            nParagraphIndex = aTarget.Paragraph;

                            if( !xTargetShape.is() )
                            {
                                OSL_FAIL( "animcore: NodeFunctor::operator(): invalid shape in ParagraphTarget" );
                                return;
                            }
                        }
                        [[fallthrough]];
                    }
                    case animations::AnimationNodeType::PAR:
                    case animations::AnimationNodeType::SEQ:
                    {
                        /// forward bInitial
                        NodeFunctor aFunctor( mrShapeHash,
                                              xTargetShape,
                                              nParagraphIndex,
                                              mbInitial );
                        if( !for_each_childNode( xNode, aFunctor ) )
                        {
                            OSL_FAIL( "AnimCore: NodeFunctor::operator(): child node iteration failed, "
                                        "or extraneous container nodes encountered" );
                        }
                    }
                    break;

                    case animations::AnimationNodeType::CUSTOM:
                    case animations::AnimationNodeType::ANIMATE:
                    case animations::AnimationNodeType::ANIMATEMOTION:
                    case animations::AnimationNodeType::ANIMATECOLOR:
                    case animations::AnimationNodeType::ANIMATETRANSFORM:
                    case animations::AnimationNodeType::TRANSITIONFILTER:
                    case animations::AnimationNodeType::AUDIO:
                    /*default:
                        // ignore this node, no valuable content for now.
                        break;*/

                    case animations::AnimationNodeType::SET:
                    {
                        // evaluate set node content
                        uno::Reference< animations::XAnimate > xAnimateNode( xNode,
                                                                             uno::UNO_QUERY );

                        if( !xAnimateNode.is() )
                            break; // invalid node

                        // determine target shape (if any)
                        ShapeHashKey aTarget;
                        if( xTargetShape.is() )
                        {
                            // override target shape with parent-supplied
                            aTarget.mxRef = xTargetShape;
                            aTarget.mnParagraphIndex = nParagraphIndex;
                        }
                        else
                        {
                            // no parent-supplied target, retrieve
                            // node target
                            if( xAnimateNode->getTarget() >>= aTarget.mxRef )
                            {
                                // pure shape target - set paragraph
                                // index to magic
                                aTarget.mnParagraphIndex = -1;
                            }
                            else
                            {
                                // not a pure shape target - maybe a
                                // ParagraphTarget?
                                presentation::ParagraphTarget aUnoTarget;

                                if( !(xAnimateNode->getTarget() >>= aUnoTarget) )
                                {
                                    OSL_FAIL( "AnimCore: NodeFunctor::operator(): unknown target type encountered" );
                                    break;
                                }

                                aTarget.mxRef = aUnoTarget.Shape;
                                aTarget.mnParagraphIndex = aUnoTarget.Paragraph;
                            }
                        }

                        if( !aTarget.mxRef.is() )
                        {
                            OSL_FAIL( "AnimCore: NodeFunctor::operator(): Found target, but XShape is NULL" );
                            break; // invalid target XShape
                        }

                        // check whether we already have an entry for
                        // this target (we only take the first set
                        // effect for every shape) - but keep going if
                        // we're requested the final state (which
                        // eventually gets overwritten in the
                        // unordered list, see tdf#96083)
                        if( mbInitial && mrShapeHash.find( aTarget ) != mrShapeHash.end() )
                            break; // already an entry in existence for given XShape

                        // if this is an appear effect, hide shape
                        // initially. This is currently the only place
                        // where a shape effect influences shape
                        // attributes outside it's effective duration.
                        bool bVisible( false );
                        if( xAnimateNode->getAttributeName().equalsIgnoreAsciiCase("visibility") )
                        {

                            uno::Any aAny( xAnimateNode->getTo() );

                            // try to extract bool value
                            if( !(aAny >>= bVisible) )
                            {
                                // try to extract string
                                OUString aString;
                                if( aAny >>= aString )
                                {
                                    // we also take the strings "true" and "false",
                                    // as well as "on" and "off" here
                                    if( aString.equalsIgnoreAsciiCase("true") ||
                                        aString.equalsIgnoreAsciiCase("on") )
                                    {
                                        bVisible = true;
                                    }
                                    if( aString.equalsIgnoreAsciiCase("false") ||
                                        aString.equalsIgnoreAsciiCase("off") )
                                    {
                                        bVisible = false;
                                    }
                                }
                            }
                        }

                        // if initial anim sets shape visible, set it
                        // to invisible. If we're asked for the final
                        // state, don't do anything obviously
                        if(mbInitial)
                            bVisible = !bVisible;

                        // target is set the 'visible' value,
                        // so we should record the opposite value
                        mrShapeHash.emplace(
                                        aTarget,
                                        VectorOfNamedValues(
                                            1,
                                            beans::NamedValue(
                                                //xAnimateNode->getAttributeName(),
                                                "visibility",
                                                uno::makeAny( bVisible ) ) ) );
                        break;
                    }
                }
            }

        private:
            XShapeHash&                         mrShapeHash;
            uno::Reference< drawing::XShape >   mxTargetShape;
            sal_Int16 const                     mnParagraphIndex;

            // get initial or final state
            bool const                          mbInitial;
        };
    }

    uno::Sequence< animations::TargetProperties > TargetPropertiesCreator::createTargetProperties
        (
            const uno::Reference< animations::XAnimationNode >& xRootNode,
            bool bInitial
        ) //throw (uno::RuntimeException, std::exception)
    {
        // scan all nodes for visibility changes, and record first
        // 'visibility=true' for each shape
        XShapeHash aShapeHash( 101 );

        NodeFunctor aFunctor(
            aShapeHash,
            bInitial );

        // TODO(F1): Maybe limit functor application to main sequence
        // alone (CL said something that shape visibility is only
        // affected by effects in the main sequence for PPT).

        // OTOH, client code can pass us only the main sequence (which
        // it actually does right now, for the slideshow implementation).
        aFunctor( xRootNode );

        // output to result sequence
        uno::Sequence< animations::TargetProperties > aRes( aShapeHash.size() );

        ::std::size_t                       nCurrIndex(0);
        for( const auto& rIter : aShapeHash )
        {
            animations::TargetProperties& rCurrProps( aRes[ nCurrIndex++ ] );

            if( rIter.first.mnParagraphIndex == -1 )
            {
                rCurrProps.Target <<= rIter.first.mxRef;
            }
            else
            {
                rCurrProps.Target <<=
                    presentation::ParagraphTarget(
                        rIter.first.mxRef,
                        rIter.first.mnParagraphIndex );
            }

            rCurrProps.Properties = ::comphelper::containerToSequence( rIter.second );
        }

        return aRes;
    }

} // namespace internal
} // namespace slideshow

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
