/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

#include <animations/animationnodehelper.hxx>
#include <boost/unordered_map.hpp>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/sequence.hxx>
#include <vector>


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

        
        virtual void SAL_CALL disposing();

        
        virtual uno::Sequence< animations::TargetProperties > SAL_CALL createInitialTargetProperties( const uno::Reference< animations::XAnimationNode >& rootNode ) throw (uno::RuntimeException);

        
        virtual OUString SAL_CALL getImplementationName() throw( uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( uno::RuntimeException );
        virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( uno::RuntimeException );

        
        virtual OUString SAL_CALL getServiceName(  ) throw (uno::RuntimeException);

    protected:
        ~TargetPropertiesCreator(); 

    private:
        
        TargetPropertiesCreator(const TargetPropertiesCreator&);
        TargetPropertiesCreator& operator=( const TargetPropertiesCreator& );

        TargetPropertiesCreator( const uno::Reference< uno::XComponentContext >& rxContext );
    };

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

    namespace
    {
        
        typedef ::std::vector< beans::NamedValue > VectorOfNamedValues;

        /** The hash map key

            This key contains both XShape reference and a paragraph
            index, as we somehow have to handle shape and paragraph
            targets with the same data structure.
         */
        struct ShapeHashKey
        {
            
            uno::Reference< drawing::XShape >   mxRef;

            /** Paragraph index.

                If this is a pure shape target, mnParagraphIndex is
                set to -1.
             */
            sal_Int16                           mnParagraphIndex;

            
            bool operator==( const ShapeHashKey& rRHS ) const
            {
                return mxRef == rRHS.mxRef && mnParagraphIndex == rRHS.mnParagraphIndex;
            }
        };

        
        struct ShapeKeyHasher
        {
            ::std::size_t operator()( const ShapeHashKey& rKey ) const
            {
                
                
                
                //
                
                
                
                
                
                //
                
                
                
                return reinterpret_cast< ::std::size_t >(rKey.mxRef.get()) ^ (rKey.mnParagraphIndex << 16L);
            }
        };

        
        typedef ::boost::unordered_map< ShapeHashKey, VectorOfNamedValues, ShapeKeyHasher > XShapeHash;


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
                        
                        

                        uno::Reference< animations::XIterateContainer > xIterNode( xNode,
                                                                                   uno::UNO_QUERY );

                        
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
                        
                    case animations::AnimationNodeType::PAR:
                        
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
                        
                    case animations::AnimationNodeType::ANIMATE:
                        
                    case animations::AnimationNodeType::ANIMATEMOTION:
                        
                    case animations::AnimationNodeType::ANIMATECOLOR:
                        
                    case animations::AnimationNodeType::ANIMATETRANSFORM:
                        
                    case animations::AnimationNodeType::TRANSITIONFILTER:
                        
                    case animations::AnimationNodeType::AUDIO:
                        
                    /*default:
                        
                        break;*/

                    case animations::AnimationNodeType::SET:
                    {
                        
                        uno::Reference< animations::XAnimate > xAnimateNode( xNode,
                                                                             uno::UNO_QUERY );

                        if( !xAnimateNode.is() )
                            break; 

                        
                        ShapeHashKey aTarget;
                        if( xTargetShape.is() )
                        {
                            
                            aTarget.mxRef = xTargetShape;
                            aTarget.mnParagraphIndex = nParagraphIndex;
                        }
                        else
                        {
                            
                            
                            if( (xAnimateNode->getTarget() >>= aTarget.mxRef) )
                            {
                                
                                
                                aTarget.mnParagraphIndex = -1;
                            }
                            else
                            {
                                
                                
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
                            break; 
                        }

                        
                        
                        
                        XShapeHash::const_iterator aIter;
                        if( (aIter=mrShapeHash.find( aTarget )) != mrShapeHash.end() )
                            break; 

                        
                        
                        
                        
                        sal_Bool bVisible( sal_False );
                        if( xAnimateNode->getAttributeName().equalsIgnoreAsciiCase("visibility") )
                        {

                            uno::Any aAny( xAnimateNode->getTo() );

                            
                            if( !(aAny >>= bVisible) )
                            {
                                
                                OUString aString;
                                if( (aAny >>= aString) )
                                {
                                    
                                    
                                    if( aString.equalsIgnoreAsciiCase("true") ||
                                        aString.equalsIgnoreAsciiCase("on") )
                                    {
                                        bVisible = sal_True;
                                    }
                                    if( aString.equalsIgnoreAsciiCase("false") ||
                                        aString.equalsIgnoreAsciiCase("off") )
                                    {
                                        bVisible = sal_False;
                                    }
                                }
                            }

                            /*if( bVisible )
                            {
                                
                                
                                
                                
                                */
                }
                            
                            
                mrShapeHash.insert(
                                    XShapeHash::value_type(
                                        aTarget,
                                        VectorOfNamedValues(
                                            1,
                                            beans::NamedValue(
                                                
                                                OUString("visibility"),
                                                uno::makeAny( !bVisible ) ) ) ) );
                            
                        
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

    
    uno::Sequence< animations::TargetProperties > SAL_CALL TargetPropertiesCreator::createInitialTargetProperties
        (
            const uno::Reference< animations::XAnimationNode >& xRootNode
        ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        
        
        XShapeHash aShapeHash( 101 );

        NodeFunctor aFunctor( aShapeHash );

        
        
        
        //
        
        
        aFunctor( xRootNode );

        
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

    
    OUString SAL_CALL TargetPropertiesCreator::getImplementationName() throw( uno::RuntimeException )
    {
        return OUString( IMPLEMENTATION_NAME );
    }

    sal_Bool SAL_CALL TargetPropertiesCreator::supportsService( const OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return cppu::supportsService(this, ServiceName);
    }

    uno::Sequence< OUString > SAL_CALL TargetPropertiesCreator::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = SERVICE_NAME;

        return aRet;
    }

    
    OUString SAL_CALL TargetPropertiesCreator::getServiceName(  ) throw (uno::RuntimeException)
    {
        return OUString( SERVICE_NAME );
    }

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
