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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/animations/XTargetPropertiesCreator.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/TargetProperties.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/sequence.hxx>

#include <animations/animationnodehelper.hxx>

#include <vector>
#include <boost/unordered_map.hpp>


using namespace ::com::sun::star;

#define IMPLEMENTATION_NAME "animcore::TargetPropertiesCreator"
#define SERVICE_NAME "com.sun.star.animations.TargetPropertiesCreator"

namespace animcore
{
    typedef ::cppu::WeakComponentImplHelper3< ::com::sun::star::animations::XTargetPropertiesCreator,
                                              lang::XServiceInfo,
                                              lang::XServiceName >  TargetPropertiesCreator_Base;

    class TargetPropertiesCreator : public ::comphelper::OBaseMutex,
                                    public TargetPropertiesCreator_Base
    {
    public:
        static uno::Reference< uno::XInterface > SAL_CALL createInstance( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::Exception )
        {
            return uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>(new TargetPropertiesCreator( xContext )) );
        }

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XTargetPropertiesCreator
        virtual uno::Sequence< animations::TargetProperties > SAL_CALL createInitialTargetProperties( const uno::Reference< animations::XAnimationNode >& rootNode ) throw (uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( uno::RuntimeException );
        virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( uno::RuntimeException );

        // XServiceName
        virtual OUString SAL_CALL getServiceName(  ) throw (uno::RuntimeException);

    protected:
        ~TargetPropertiesCreator(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        TargetPropertiesCreator(const TargetPropertiesCreator&);
        TargetPropertiesCreator& operator=( const TargetPropertiesCreator& );

        TargetPropertiesCreator( const uno::Reference< uno::XComponentContext >& rxContext );
    };

    // --------------------------------------------------------------------

    uno::Reference< uno::XInterface > SAL_CALL createInstance_TargetPropertiesCreator( const uno::Reference< uno::XComponentContext > & rSMgr ) throw (uno::Exception)
    {
        return TargetPropertiesCreator::createInstance( rSMgr );
    }

    OUString getImplementationName_TargetPropertiesCreator()
    {
        return OUString( IMPLEMENTATION_NAME );
    }

    uno::Sequence< OUString > getSupportedServiceNames_TargetPropertiesCreator(void)
    {
        uno::Sequence< OUString > aRet(1);
        aRet.getArray()[0] = SERVICE_NAME;
        return aRet;
    }

    // --------------------------------------------------------------------

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

            /// Comparison needed for boost::unordered_map
            bool operator==( const ShapeHashKey& rRHS ) const
            {
                return mxRef == rRHS.mxRef && mnParagraphIndex == rRHS.mnParagraphIndex;
            }
        };

        // A hash map which maps a XShape to the corresponding vector of initial properties
        typedef ::boost::unordered_map< ShapeHashKey,
                                 VectorOfNamedValues,
                                 ::std::size_t (*)(const ShapeHashKey&) > XShapeHash;

        ::std::size_t refhasher( const ShapeHashKey& rKey )
        {
            // TODO(P2): Maybe a better hash function would be to
            // spread mnParagraphIndex to 32 bit: a0b0c0d0e0... Hakmem
            // should have a formula.
            //
            // Yes it has:
            // x = (x & 0x0000FF00) << 8) | (x >> 8) & 0x0000FF00 | x & 0xFF0000FF;
            // x = (x & 0x00F000F0) << 4) | (x >> 4) & 0x00F000F0 | x & 0xF00FF00F;
            // x = (x & 0x0C0C0C0C) << 2) | (x >> 2) & 0x0C0C0C0C | x & 0xC3C3C3C3;
            // x = (x & 0x22222222) << 1) | (x >> 1) & 0x22222222 | x & 0x99999999;
            //
            // Costs about 17 cycles on a RISC machine with infinite
            // instruction level parallelism (~42 basic
            // instructions). Thus I truly doubt this pays off...
            return reinterpret_cast< ::std::size_t >(rKey.mxRef.get()) ^ (rKey.mnParagraphIndex << 16L);
        }


        class NodeFunctor
        {
        public:
            explicit NodeFunctor( XShapeHash& rShapeHash ) :
                mrShapeHash( rShapeHash ),
                mxTargetShape(),
                mnParagraphIndex( -1 )
            {
            }

            NodeFunctor( XShapeHash&                                rShapeHash,
                         const uno::Reference< drawing::XShape >&   rTargetShape,
                         sal_Int16                                  nParagraphIndex ) :
                mrShapeHash( rShapeHash ),
                mxTargetShape( rTargetShape ),
                mnParagraphIndex( nParagraphIndex )
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
                        // --------------------------------------------------

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
                            ::com::sun::star::presentation::ParagraphTarget aTarget;

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
                    }
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::PAR:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::SEQ:
                    {
                        NodeFunctor aFunctor( mrShapeHash,
                                              xTargetShape,
                                              nParagraphIndex );
                        if( !::anim::for_each_childNode( xNode,
                                                         aFunctor ) )
                        {
                            OSL_FAIL( "AnimCore: NodeFunctor::operator(): child node iteration failed, "
                                        "or extraneous container nodes encountered" );
                        }
                    }
                    break;

                    case animations::AnimationNodeType::CUSTOM:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::ANIMATE:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::ANIMATEMOTION:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::ANIMATECOLOR:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::ANIMATETRANSFORM:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::TRANSITIONFILTER:
                        // FALLTHROUGH intended
                    case animations::AnimationNodeType::AUDIO:
                        // FALLTHROUGH intended
                    default:
                        // ignore this node, no valuable content for now.
                        break;

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
                            if( (xAnimateNode->getTarget() >>= aTarget.mxRef) )
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
                        // effect for every shape)
                        XShapeHash::const_iterator aIter;
                        if( (aIter=mrShapeHash.find( aTarget )) != mrShapeHash.end() )
                            break; // already an entry in existence for given XShape

                        // if this is an appear effect, hide shape
                        // initially. This is currently the only place
                        // where a shape effect influences shape
                        // attributes outside it's effective duration.
                        if( xAnimateNode->getAttributeName().equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("visibility")) )
                        {
                            sal_Bool bVisible( sal_False );

                            uno::Any aAny( xAnimateNode->getTo() );

                            // try to extract bool value
                            if( !(aAny >>= bVisible) )
                            {
                                // try to extract string
                                OUString aString;
                                if( (aAny >>= aString) )
                                {
                                    // we also take the strings "true" and "false",
                                    // as well as "on" and "off" here
                                    if( aString.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("true")) ||
                                        aString.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("on")) )
                                    {
                                        bVisible = sal_True;
                                    }
                                    if( aString.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("false")) ||
                                        aString.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("off")) )
                                    {
                                        bVisible = sal_False;
                                    }
                                }
                            }

                            if( bVisible )
                            {
                                // target is set to 'visible' at the
                                // first relevant effect. Thus, target
                                // must be initially _hidden_, for the
                                // effect to have visible impact.
                                mrShapeHash.insert(
                                    XShapeHash::value_type(
                                        aTarget,
                                        VectorOfNamedValues(
                                            1,
                                            beans::NamedValue(
                                                xAnimateNode->getAttributeName(),
                                                uno::makeAny( sal_False ) ) ) ) );
                            }
                        }
                    }
                    break;
                }
            }

        private:
            XShapeHash&                         mrShapeHash;
            uno::Reference< drawing::XShape >   mxTargetShape;
            sal_Int16                           mnParagraphIndex;
        };
    }

    // --------------------------------------------------------------------

    TargetPropertiesCreator::TargetPropertiesCreator( const uno::Reference< uno::XComponentContext >&  ) :
        TargetPropertiesCreator_Base( m_aMutex )
    {
    }

    TargetPropertiesCreator::~TargetPropertiesCreator()
    {
    }

    void SAL_CALL TargetPropertiesCreator::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
    }

    // XTargetPropertiesCreator
    uno::Sequence< animations::TargetProperties > SAL_CALL TargetPropertiesCreator::createInitialTargetProperties
        (
            const uno::Reference< animations::XAnimationNode >& xRootNode
        ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // scan all nodes for visibility changes, and record first
        // 'visibility=true' for each shape
        XShapeHash aShapeHash( 101,
                               &refhasher );

        NodeFunctor aFunctor( aShapeHash );

        // TODO(F1): Maybe limit functor application to main sequence
        // alone (CL said something that shape visibility is only
        // affected by effects in the main sequence for PPT).
        //
        // OTOH, client code can pass us only the main sequence (which
        // it actually does right now, for the slideshow implementation).
        aFunctor( xRootNode );


        // output to result sequence
        // ----------------------------------------------------------------------

        uno::Sequence< animations::TargetProperties > aRes( aShapeHash.size() );

        ::std::size_t                       nCurrIndex(0);
        XShapeHash::const_iterator          aCurr( aShapeHash.begin() );
        const XShapeHash::const_iterator    aEnd ( aShapeHash.end()   );
        while( aCurr != aEnd )
        {
            animations::TargetProperties& rCurrProps( aRes[ nCurrIndex++ ] );

            if( aCurr->first.mnParagraphIndex == -1 )
            {
                rCurrProps.Target = uno::makeAny( aCurr->first.mxRef );
            }
            else
            {
                rCurrProps.Target = uno::makeAny(
                    presentation::ParagraphTarget(
                        aCurr->first.mxRef,
                        aCurr->first.mnParagraphIndex ) );
            }

            rCurrProps.Properties = ::comphelper::containerToSequence( aCurr->second );

            ++aCurr;
        }

        return aRes;
    }

    // XServiceInfo
    OUString SAL_CALL TargetPropertiesCreator::getImplementationName() throw( uno::RuntimeException )
    {
        return OUString( IMPLEMENTATION_NAME );
    }

    sal_Bool SAL_CALL TargetPropertiesCreator::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(SERVICE_NAME));
    }

    uno::Sequence< OUString > SAL_CALL TargetPropertiesCreator::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = SERVICE_NAME;

        return aRet;
    }

    // XServiceName
    OUString SAL_CALL TargetPropertiesCreator::getServiceName(  ) throw (uno::RuntimeException)
    {
        return OUString( SERVICE_NAME );
    }

} // namespace animcore

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
