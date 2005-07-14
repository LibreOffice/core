/*************************************************************************
 *
 *  $RCSfile: CustomAnimationCloner.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:43:10 $
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

#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODE_HPP_
#include <com/sun/star/animations/XAnimationNode.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_EVENT_HPP_
#include <com/sun/star/animations/Event.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAnimateColor_HPP_
#include <com/sun/star/animations/XAnimateColor.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAnimateSet_HPP_
#include <com/sun/star/animations/XAnimateSet.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XCOMMAND_HPP_
#include <com/sun/star/animations/XCommand.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAnimateMotion_HPP_
#include <com/sun/star/animations/XAnimateMotion.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAnimateTransform_HPP_
#include <com/sun/star/animations/XAnimateTransform.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XTransitionFilter_HPP_
#include <com/sun/star/animations/XTransitionFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XITERATECONTAINER_HPP_
#include <com/sun/star/animations/XIterateContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XAUDIO_HPP_
#include <com/sun/star/animations/XAudio.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_ANIMATIONNODETYPE_HPP_
#include <com/sun/star/animations/AnimationNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_VALUEPAIR_HPP_
#include <com/sun/star/animations/ValuePair.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_EFFECTNODETYPE_HPP_
#include <com/sun/star/presentation/EffectNodeType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_PARAGRAPHTARGET_HPP_
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#include <map>

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"

#ifndef INCLUDED_ANIMATIONS_ANIMATIONNODEHELPER_HXX
#include <animations/animationnodehelper.hxx>
#endif

// header for class SdrObjListIter
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#include "sdpage.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::container;

using ::rtl::OUString;
using ::rtl::OString;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::beans::NamedValue;

namespace sd
{
    class CustomAnimationClonerImpl
    {
    public:
        CustomAnimationClonerImpl();
        Reference< XAnimationNode > Clone( const Reference< XAnimationNode >& xSourceNode, const SdPage* pSource = 0, const SdPage* pTarget = 0 );

    private:
        void transformNode( const Reference< XAnimationNode >& xNode );
        Any transformValue( const Any& rValue );

        Reference< XShape > getClonedShape( const Reference< XShape >& xSource ) const;
        Reference< XAnimationNode > getClonedNode( const Reference< XAnimationNode >& xSource ) const;

        mutable ::std::map< Reference< XShape >, Reference< XShape > > maShapeMap;
        std::vector< Reference< XAnimationNode > > maSourceNodeVector;
        std::vector< Reference< XAnimationNode > > maCloneNodeVector;
    };

    CustomAnimationClonerImpl::CustomAnimationClonerImpl()
    {
    }

    Reference< XAnimationNode > Clone( const Reference< XAnimationNode >& xSourceNode, const SdPage* pSource, const SdPage* pTarget )
    {
        CustomAnimationClonerImpl aCloner;
        return aCloner.Clone( xSourceNode, pSource, pTarget );
    }

    Reference< XAnimationNode > CustomAnimationClonerImpl::Clone( const Reference< XAnimationNode >& xSourceNode, const SdPage* pSource, const SdPage* pTarget )
    {
        try
        {
            // clone animation hierarchie
            Reference< ::com::sun::star::util::XCloneable > xClonable( xSourceNode, UNO_QUERY_THROW );
            Reference< XAnimationNode > xCloneNode( xClonable->createClone(), UNO_QUERY_THROW );

            // create a dictionary to map source to cloned shapes
            if( pSource && pTarget )
            {
                SdrObjListIter aSourceIter( *pSource, IM_DEEPWITHGROUPS );
                SdrObjListIter aTargetIter( *pTarget, IM_DEEPWITHGROUPS );

                while( aSourceIter.IsMore() && aTargetIter.IsMore() )
                {
                    SdrObject* pSource = aSourceIter.Next();
                    SdrObject* pTarget = aTargetIter.Next();

                    if( pSource && pTarget)
                    {
                        Reference< XShape > xSource( pSource->getUnoShape(), UNO_QUERY );
                        Reference< XShape > xTarget( pTarget->getUnoShape(), UNO_QUERY );
                        if( xSource.is() && xTarget.is() )
                        {
                            maShapeMap[xSource] = xTarget;
                        }
                    }
                }
            }

            // create a dictionary to map source to cloned nodes
            ::anim::create_deep_vector( xSourceNode, maSourceNodeVector );
            ::anim::create_deep_vector( xCloneNode, maCloneNodeVector );

            transformNode( xCloneNode );

            return xCloneNode;
        }
        catch( Exception& e )
        {
            (void)e;
            DBG_ERROR(
                (OString("sd::CustomAnimationClonerImpl::Clone(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );

            Reference< XAnimationNode > xEmpty;
            return xEmpty;
        }
    }

    void CustomAnimationClonerImpl::transformNode( const Reference< XAnimationNode >& xNode )
    {
        try
        {
            xNode->setBegin( transformValue( xNode->getBegin() ) );
            xNode->setEnd( transformValue( xNode->getEnd() ) );

            sal_Int16 nNodeType( xNode->getType() );
            switch( nNodeType )
            {
            case AnimationNodeType::ITERATE:
            {
                Reference< XIterateContainer > xIter( xNode, UNO_QUERY_THROW );
                xIter->setTarget( transformValue( xIter->getTarget() ) );
            }
            // its intended that here is no break!
            case AnimationNodeType::PAR:
            case AnimationNodeType::SEQ:
            {
                Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY_THROW );
                Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
                while( xEnumeration->hasMoreElements() )
                {
                    Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY_THROW );
                    transformNode( xChildNode );
                }
            }
            break;

            case AnimationNodeType::ANIMATE:
            case AnimationNodeType::SET:
            case AnimationNodeType::ANIMATEMOTION:
            case AnimationNodeType::ANIMATECOLOR:
            case AnimationNodeType::ANIMATETRANSFORM:
            case AnimationNodeType::TRANSITIONFILTER:
            {
                Reference< XAnimate > xAnimate( xNode, UNO_QUERY_THROW );
                xAnimate->setTarget( transformValue( xAnimate->getTarget() ) );
            }
            break;

            case AnimationNodeType::COMMAND:
            {
                Reference< XCommand > xCommand( xNode, UNO_QUERY_THROW );
                xCommand->setTarget( transformValue( xCommand->getTarget() ) );
            }
            break;

            case AnimationNodeType::AUDIO:
            {
                Reference< XAudio > xAudio( xNode, UNO_QUERY_THROW );
                xAudio->setSource( transformValue( xAudio->getSource() ) );
            }
            break;
            }

            Sequence< NamedValue > aUserData( xNode->getUserData() );
            if( aUserData.hasElements() )
            {
                NamedValue* pValue = aUserData.getArray();
                const sal_Int32 nLength = aUserData.getLength();
                sal_Int32 nElement;
                for( nElement = 0; nElement < nLength; nElement++, pValue++ )
                {
                    pValue->Value = transformValue( pValue->Value );
                }

                xNode->setUserData( aUserData );
            }
        }
        catch( Exception& e )
        {
            (void)e;
            DBG_ERROR(
                (OString("sd::CustomAnimationClonerImpl::transformNode(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
    }

    Any CustomAnimationClonerImpl::transformValue( const Any& rValue )
    {
        if( rValue.hasValue() ) try
        {
            if( rValue.getValueType() == ::getCppuType((const ValuePair*)0) )
            {
                ValuePair aValuePair;
                rValue >>= aValuePair;

                aValuePair.First = transformValue( aValuePair.First );
                aValuePair.Second = transformValue( aValuePair.Second );

                return makeAny( aValuePair );
            }
            else if( rValue.getValueType() == ::getCppuType((Sequence<Any>*)0) )
            {
                Sequence<Any> aSequence;
                rValue >>= aSequence;

                const sal_Int32 nLength = aSequence.getLength();
                sal_Int32 nElement;
                Any* pAny = aSequence.getArray();

                for( nElement = 0; nElement < nLength; nElement++, pAny++ )
                    *pAny = transformValue( *pAny );

                return makeAny( aSequence );
            }
            else if( rValue.getValueTypeClass() == TypeClass_INTERFACE )
            {
                Reference< XShape > xShape;
                rValue >>= xShape;
                if( xShape.is() )
                {
                    return makeAny( getClonedShape( xShape ) );
                }
                else
                {
                    Reference< XAnimationNode > xNode;
                    rValue >>= xNode;
                    if( xNode.is() )
                        return makeAny( getClonedNode( xNode ) );
                }
            }
            else if( rValue.getValueType() == ::getCppuType((const ParagraphTarget*)0) )
            {
                ParagraphTarget aParaTarget;
                rValue >>= aParaTarget;

                aParaTarget.Shape = getClonedShape( aParaTarget.Shape );

                return makeAny( aParaTarget );
            }
            else if( rValue.getValueType() == ::getCppuType((const Event*)0) )
            {
                Event aEvent;
                rValue >>= aEvent;

                aEvent.Source = transformValue( aEvent.Source );

                return makeAny( aEvent );
            }
        }
        catch( Exception& e )
        {
            (void)e;
            DBG_ERROR(
                (OString("sd::CustomAnimationClonerImpl::transformValue(), "
                        "exception caught: ") +
                rtl::OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }

        return rValue;
    }

    Reference< XShape > CustomAnimationClonerImpl::getClonedShape( const Reference< XShape >& xSource ) const
    {
        if( xSource.is() )
        {
            if( maShapeMap.find(xSource) != maShapeMap.end() )
            {
                return maShapeMap[xSource];
            }

            DBG_ASSERT( maShapeMap.empty(), "sd::CustomAnimationClonerImpl::getClonedShape() failed!" );
        }
        return xSource;
    }

    Reference< XAnimationNode > CustomAnimationClonerImpl::getClonedNode( const Reference< XAnimationNode >& xSource ) const
    {
        sal_Int32 nNode, nNodeCount = maSourceNodeVector.size();

        for( nNode = 0; nNode < nNodeCount; nNode++ )
        {
            if( maSourceNodeVector[nNode] == xSource )
                return maCloneNodeVector[nNode];
        }

        DBG_ERROR( "sd::CustomAnimationClonerImpl::getClonedNode() failed!" );
        return xSource;
    }
}
