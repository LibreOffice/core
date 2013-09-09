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

#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/AnimationFill.hpp>
#include <com/sun/star/animations/AnimationRestart.hpp>
#include <com/sun/star/animations/Timing.hpp>
#include <com/sun/star/animations/Event.hpp>
#include <com/sun/star/animations/AnimationEndSync.hpp>
#include <com/sun/star/animations/Command.hpp>
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/animations/AnimationValueType.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XCommand.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>
#include <com/sun/star/presentation/EffectCommands.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#include <vcl/vclenum.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <pptinanimations.hxx>
#include <pptatom.hxx>
#include "pptin.hxx"
#include <algorithm>

using ::std::map;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::lang::XMultiServiceFactory;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

namespace sd
{
extern Reference< XInterface > RandomAnimationNode_createInstance( sal_Int16 nPresetClass );
}

namespace ppt
{

const transition* transition::find( const OUString& rName )
{
    const transition* p = gTransitions;

    while( p->mpName )
    {
        if( rName.compareToAscii( p->mpName ) == 0 )
            return p;

        p++;
    }

    return NULL;
}

// ====================================================================



// ====================================================================

SvStream& operator>>(SvStream& rIn, AnimationNode& rNode )
{
    rIn >> rNode.mnU1;
    rIn >> rNode.mnRestart;
    rIn >> rNode.mnGroupType;
    rIn >> rNode.mnFill;
    rIn >> rNode.mnU3;
    rIn >> rNode.mnU4;
    rIn >> rNode.mnDuration;
    rIn >> rNode.mnNodeType;

    return rIn;
}

// ====================================================================

static bool convertMeasure( OUString& rString )
{
    bool bRet = false;

    const sal_Char* pSource[] = { "ppt_x", "ppt_y", "ppt_w", "ppt_h", NULL };
    const sal_Char* pDest[] = { "x", "y", "width", "height", NULL };
    sal_Int32 nIndex = 0;

    const sal_Char** ps = pSource;
    const sal_Char** pd = pDest;

    while( *ps )
    {
        const OUString aSearch( OUString::createFromAscii( *ps ) );
        while( (nIndex = rString.indexOf( aSearch, nIndex )) != -1  )
        {
            sal_Int32 nLength = aSearch.getLength();
            if( nIndex && (rString.getStr()[nIndex-1] == '#' ) )
            {
                nIndex--;
                nLength++;
            }

            const OUString aNew( OUString::createFromAscii( *pd ) );
            rString = rString.replaceAt( nIndex, nLength, aNew );
            nIndex += aNew.getLength();
            bRet = true;
        }
        ps++;
        pd++;
    }

    return bRet;
}


// ====================================================================

bool PropertySet::hasProperty( sal_Int32 nProperty ) const
{
    return maProperties.find( nProperty ) != maProperties.end();
}

// --------------------------------------------------------------------

Any PropertySet::getProperty( sal_Int32 nProperty ) const
{
    PropertySetMap_t::const_iterator aIter( maProperties.find( nProperty ) );
    if( aIter != maProperties.end() )
        return (*aIter).second;
    else
        return Any();
}

// ====================================================================

/** this adds an any to another any.
    if rNewValue is empty, rOldValue is returned.
    if rOldValue is empty, rNewValue is returned.
    if rOldValue contains a value, a sequence with rOldValue and rNewValue is returned.
    if rOldValue contains a sequence, a new sequence with the old sequence and rNewValue is returned.
*/
static Any addToSequence( const Any& rOldValue, const Any& rNewValue )
{
    if( !rNewValue.hasValue() )
    {
        return rOldValue;
    }
    else if( !rOldValue.hasValue() )
    {
        return rNewValue;
    }
    else
    {
        Sequence< Any > aNewSeq;
        if( rOldValue >>= aNewSeq )
        {
            sal_Int32 nSize = aNewSeq.getLength();
            aNewSeq.realloc(nSize+1);
            aNewSeq[nSize] = rNewValue;
        }
        else
        {
            aNewSeq.realloc(2);
            aNewSeq[0] = rOldValue;
            aNewSeq[1] = rNewValue;
        }
        return makeAny( aNewSeq );
    }
}

// ====================================================================

AnimationImporter::AnimationImporter( ImplSdPPTImport* pPPTImport, SvStream& rStCtrl )
: mpPPTImport( pPPTImport ), mrStCtrl( rStCtrl )
{
}

// --------------------------------------------------------------------

int AnimationImporter::import( const Reference< XDrawPage >& xPage, const DffRecordHeader& rProgTagContentHd )
{
    int nNodes = 0;

#ifdef DBG_ANIM_LOG
    static int ppt_anim_debug_stream_number = 1;
    OUString ppt_anim_debug_filename("ppt-animation-import-debug-output-");
    ppt_anim_debug_filename += OUString::number(ppt_anim_debug_stream_number++);
    ppt_anim_debug_filename += OUString(".xml");
    mpFile = fopen( OUStringToOString( ppt_anim_debug_filename, RTL_TEXTENCODING_UTF8).getStr() , "w+" );
#endif
    dump("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

    Reference< XAnimationNodeSupplier > xNodeSupplier( xPage, UNO_QUERY );
    if( xNodeSupplier.is() )
    {
        mxRootNode = xNodeSupplier->getAnimationNode();
        if( mxRootNode.is() )
        {
            Reference< XAnimationNode > xParent;

            boost::scoped_ptr<Atom> pAtom(Atom::import( rProgTagContentHd, mrStCtrl ));
            if( pAtom )
            {
                nNodes = importAnimationContainer( pAtom.get(), xParent );
            }

            processAfterEffectNodes();
        }
    }

#ifdef DBG_ANIM_LOG
    fclose( mpFile );
#endif

    return nNodes;
}

// --------------------------------------------------------------------

void AnimationImporter::processAfterEffectNodes()
{
    std::for_each( maAfterEffectNodes.begin(), maAfterEffectNodes.end(), sd::stl_process_after_effect_node_func );
}

// --------------------------------------------------------------------

Reference< XAnimationNode > AnimationImporter::createNode( const Atom* pAtom, const AnimationNode& rNode )
{
    const char* pServiceName = NULL;

    switch( rNode.mnGroupType )
    {
    case mso_Anim_GroupType_PAR:
        if( pAtom->hasChildAtom( DFF_msofbtAnimIteration ) )
            pServiceName = "com.sun.star.animations.IterateContainer";
        else
            pServiceName = "com.sun.star.animations.ParallelTimeContainer";
        break;
    case mso_Anim_GroupType_SEQ:
        pServiceName = "com.sun.star.animations.SequenceTimeContainer";
        break;
    case mso_Anim_GroupType_NODE:
    {
        switch( rNode.mnNodeType )
        {
        case mso_Anim_Behaviour_FILTER:
        case mso_Anim_Behaviour_ANIMATION:
            if( pAtom->hasChildAtom( DFF_msofbtAnimateSet ) )
                pServiceName = "com.sun.star.animations.AnimateSet";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateColor ) )
                pServiceName = "com.sun.star.animations.AnimateColor";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateScale ) )
                pServiceName = "com.sun.star.animations.AnimateTransform";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateRotation ) )
                pServiceName = "com.sun.star.animations.AnimateTransform";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateMotion ) )
                pServiceName = "com.sun.star.animations.AnimateMotion";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateFilter ) )
                pServiceName = "com.sun.star.animations.TransitionFilter";
            else if( pAtom->hasChildAtom( DFF_msofbtAnimCommand ) )
                pServiceName = "com.sun.star.animations.Command";
            else
                pServiceName = "com.sun.star.animations.Animate";
            break;
        }
        break;
    }
    case mso_Anim_GroupType_MEDIA:
        pServiceName = "com.sun.star.animations.Audio";
        break;

    default:
        pServiceName = "com.sun.star.animations.Animate";
        break;
    }

    Reference< XAnimationNode > xNode;
    if( pServiceName )
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        const OUString aServiceName( OUString::createFromAscii(pServiceName) );
        Reference< XInterface > xFac( xContext->getServiceManager()->createInstanceWithContext(aServiceName, xContext) );
        xNode.set(xFac , UNO_QUERY );
    }

    DBG_ASSERT( xNode.is(), "sd::AnimationImporter::createNode(), node creation failed!" );
    return xNode;
}

// --------------------------------------------------------------------

static bool is_random( const AnimationNode& rNode, const PropertySet& rSet, sal_Int32& rPresetClass )
{
    if( rNode.mnGroupType != mso_Anim_GroupType_PAR )
        return false;

    if( !rSet.hasProperty( DFF_ANIM_PRESET_ID ) || !rSet.hasProperty( DFF_ANIM_PRESET_CLASS ) )
        return false;

    sal_Int32 nPresetId = 0;
    if( !(rSet.getProperty( DFF_ANIM_PRESET_ID ) >>= nPresetId) || (nPresetId != 24) )
        return false;

    sal_Int32 nPresetClass = 0;
    if( !(rSet.getProperty( DFF_ANIM_PRESET_CLASS ) >>= nPresetClass) )
        return false;

    switch( nPresetClass )
    {
    case DFF_ANIM_PRESS_CLASS_ENTRANCE: rPresetClass = EffectPresetClass::ENTRANCE; return true;
    case DFF_ANIM_PRESS_CLASS_EXIT: rPresetClass = EffectPresetClass::EXIT; return true;
    }
    return false;
}


int AnimationImporter::importAnimationContainer( const Atom* pAtom, const Reference< XAnimationNode >& xParent )
{
    int nNodes = 0;
    if( pAtom->seekToContent() )
    {
        AnimationNode aNode;
        const Atom* pAnimationNodeAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimNode );
        if( pAnimationNodeAtom && pAnimationNodeAtom->seekToContent() )
            mrStCtrl >> aNode;

        PropertySet aSet;
        const Atom* pAnimationPropertySetAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimPropertySet );
        if( pAnimationPropertySetAtom )
            importPropertySetContainer( pAnimationPropertySetAtom, aSet );

        Reference< XAnimationNode > xNode;

        if( xParent.is() )
        {
            sal_Int32 nPresetClass;
            if( is_random( aNode, aSet, nPresetClass ) )
            {
                // create a random animation node with the given preset class
                xNode.set( sd::RandomAnimationNode_createInstance( (sal_Int16)nPresetClass ), UNO_QUERY );
            }

            if( !xNode.is() )
            {
                // create a node for the given atom
                xNode = createNode( pAtom, aNode );
            }
        }
        else
        {
            // if we have no parent we fill the root node
            xNode = mxRootNode;
        }

        // import if we have a node and its not random
        if( xNode.is() )
        {
            fillNode( xNode, aNode, aSet );

            switch( aNode.mnGroupType )
            {
            case mso_Anim_GroupType_PAR:
            {
                dump( "<par" );
                dump( aNode );
                dump( aSet );
                nNodes += importTimeContainer( pAtom, xNode );
                dump( "</par>\n" );

                // for iteration containers, map target from children to iteration
                Reference< XIterateContainer > xIter( xNode, UNO_QUERY );
                if( xIter.is() )
                {
                    double fDuration = 0.0;
                    Any aTarget, aEmpty;
                    Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
                    if( xEnumerationAccess.is() )
                    {
                        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
                        if( xEnumeration.is() )
                        {
                            while( xEnumeration->hasMoreElements() )
                            {
                                Reference< XAnimate > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                                if( xChildNode.is() )
                                {
                                    double fChildBegin = 0.0;
                                    double fChildDuration = 0.0;
                                    xChildNode->getBegin() >>= fChildBegin;
                                    xChildNode->getDuration() >>= fChildDuration;

                                    fChildDuration += fChildBegin;
                                    if( fChildDuration > fDuration )
                                        fDuration = fChildDuration;

                                    if( !aTarget.hasValue() )
                                        aTarget = xChildNode->getTarget();

                                    xChildNode->setTarget( aEmpty );
                                }
                            }
                        }
                    }

                    xIter->setTarget( aTarget );

                    double fIterateInterval = xIter->getIterateInterval() * fDuration / 100;
                    xIter->setIterateInterval( fIterateInterval );
                }
            }
            break;

            case mso_Anim_GroupType_SEQ:
            {
                dump( "<seq" );
                dump( aNode );
                dump( aSet );
                nNodes += importTimeContainer( pAtom, xNode );
                dump( "</seq>\n" );

                if( aSet.hasProperty( DFF_ANIM_NODE_TYPE ) )
                {
                    sal_Int32 nPPTNodeType = 0;
                    if( aSet.getProperty( DFF_ANIM_NODE_TYPE ) >>= nPPTNodeType )
                    {
                        switch(nPPTNodeType)
                        {
                        case DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE:
                            fixMainSequenceTiming( xNode );
                            break;
                        case DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ:
                            fixInteractiveSequenceTiming( xNode );
                            break;
                        }
                    }
                }
            }
            break;

            case mso_Anim_GroupType_NODE:
            {
#ifdef DBG_ANIM_LOG
                if( pAtom->hasChildAtom( DFF_msofbtAnimateSet ) )
                {
                    dump( "<set" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimateColor ) )
                {
                    dump( "<animateColor" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimateScale ) )
                {
                    dump( "<animateScale" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimateRotation ) )
                {
                    dump( "<animateRotation" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimateMotion ) )
                {
                    dump( "<animateMotion" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimate ) )
                {
                    dump( "<animate" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimateFilter ) )
                {
                    dump( "<animateFilter" );
                }
                else if( pAtom->hasChildAtom( DFF_msofbtAnimCommand ) )
                {
                    dump( "<command" );
                }
                else
                {
                    OSL_FAIL( "unknown node atom!" );
                    dump_atom_header( pAtom, true, false );
                    dump_atom( pAtom );
                    dump_atom_header( pAtom, false, false );
                    break;
                }
                dump( aNode );
                dump( aSet );
#endif
                int nANCNodes = importAnimationNodeContainer( pAtom, xNode );
                if( !convertAnimationNode( xNode, xParent ) )
                    xNode = 0;
                else
                    nNodes += nANCNodes;
                dump( "/>\n");

            }
            break;

            case mso_Anim_GroupType_MEDIA:
            {
                dump( "<audio" );
                dump( aNode );
                dump( aSet );
                nNodes += importAudioContainer( pAtom, xNode );
                dump( "</audio>\n" );
            }
            break;

            default:
                OSL_FAIL( "unknown group atom!" );

                dump_atom_header( pAtom, true, false );
                dump_atom( pAtom );
                dump_atom_header( pAtom, false, false );
                break;

            }
        }

        if( xParent.is() && xNode.is() )
        {
            Reference< XTimeContainer > xParentContainer( xParent, UNO_QUERY );
            DBG_ASSERT( xParentContainer.is(), "parent is no container, then why do I have a child here?" );
            if( xParentContainer.is() )
            {
                xParentContainer->appendChild( xNode );
            }
        }
    }

    return nNodes;
}

// --------------------------------------------------------------------
void AnimationImporter::fixMainSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    try
    {
        bool bFirst = true;
        Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
        while( xE->hasMoreElements() )
        {
            // click node
            Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );

            Event aEvent;
            aEvent.Trigger = EventTrigger::ON_NEXT;
            aEvent.Repeat = 0;
            xClickNode->setBegin( makeAny( aEvent ) );

            if( bFirst )
            {
                bFirst = false;
                Reference< XEnumerationAccess > xEA2( xClickNode, UNO_QUERY_THROW );
                Reference< XEnumeration > xE2( xEA2->createEnumeration(), UNO_QUERY_THROW );
                if( xE2->hasMoreElements() )
                {
                    // with node
                    xE2->nextElement() >>= xEA2;
                    if( xEA2.is() )
                        xE2.query( xEA2->createEnumeration() );
                    else
                        xE2.clear();

                    if( xE2.is() && xE2->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xEffectNode( xE2->nextElement(), UNO_QUERY_THROW );
                        const Sequence< NamedValue > aUserData( xEffectNode->getUserData() );
                        const NamedValue* p = aUserData.getConstArray();
                        sal_Int32 nLength = aUserData.getLength();
                        while( nLength-- )
                        {
                            if ( p->Name == "node-type" )
                            {
                                sal_Int16 nNodeType = 0;
                                p->Value >>= nNodeType;
                                if( nNodeType != ::com::sun::star::presentation::EffectNodeType::ON_CLICK )
                                {
                                    // first effect does not start on click, so correct
                                    // first click nodes begin to 0s
                                    xClickNode->setBegin( makeAny( (double)0.0 ) );
                                    break;
                                }
                            }
                            p++;
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::AnimationImporter::fixMainSequenceTiming(), exception caught!" );
    }
}

// --------------------------------------------------------------------

void AnimationImporter::fixInteractiveSequenceTiming( const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xNode )
{
    try
    {
        Any aBegin( xNode->getBegin() );
        Any aEmpty;
        xNode->setBegin( aEmpty );

        Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
        while( xE->hasMoreElements() )
        {
            // click node
            Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );
            xClickNode->setBegin( aBegin );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::AnimationImporter::fixInteractiveSequenceTiming(), exception caught!" );
    }
}

// --------------------------------------------------------------------

bool AnimationImporter::convertAnimationNode( const Reference< XAnimationNode >& xNode, const Reference< XAnimationNode >& xParent )
{
    Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
    if( !xAnimate.is() )
        return true;

    if( !xAnimate->getTarget().hasValue() )
        return false;

    const sal_Int16 nNodeType = xNode->getType();

    if( nNodeType == AnimationNodeType::TRANSITIONFILTER )
        return true;

    OUString aAttributeName( xAnimate->getAttributeName() );

    if( (nNodeType == AnimationNodeType::SET) && aAttributeName == "fill.on" )
        return false;

    const ImplAttributeNameConversion* p = gImplConversionList;

    MS_AttributeNames eAttribute = MS_UNKNOWN;

    if( (nNodeType == AnimationNodeType::ANIMATEMOTION) ||
        (nNodeType == AnimationNodeType::ANIMATETRANSFORM) )
    {
        OUString aEmpty;
        aAttributeName = aEmpty;
    }
    else
    {
        while( p->mpMSName )
        {
            if( aAttributeName.compareToAscii( p->mpMSName ) == 0 )
                break;

            p++;
        }

        DBG_ASSERT( p->mpMSName || aAttributeName.isEmpty(), "sd::AnimationImporter::convertAnimationNode(), unknown attribute!" );
#ifdef DBG_ANIM_LOG
        if( p->mpMSName == 0 ) dump( "<error text=\"sd::AnimationImporter::convertAnimationNode(), unknown attribute!\"/>\n" );
#endif

        eAttribute = p->meAttribute;

        if( p->mpAPIName )
            aAttributeName = OUString::createFromAscii( p->mpAPIName );
    }

    xAnimate->setAttributeName( aAttributeName );

    if( eAttribute != MS_UNKNOWN )
    {
        Any aAny( xAnimate->getFrom() );
        if( aAny.hasValue() )
        {
            if( convertAnimationValue( eAttribute, aAny ) )
                xAnimate->setFrom( aAny );
        }

        aAny = xAnimate->getBy();
        if( aAny.hasValue() )
        {
            if( convertAnimationValue( eAttribute, aAny ) )
                xAnimate->setBy( aAny );
        }

        aAny = xAnimate->getTo();
        if( aAny.hasValue() )
        {
            if( convertAnimationValue( eAttribute, aAny ) )
                xAnimate->setTo( aAny );
        }

        Sequence< Any > aValues( xAnimate->getValues() );
        sal_Int32 nValues = aValues.getLength();
        if( nValues )
        {
            Any* p2 = aValues.getArray();
            while( nValues-- )
                convertAnimationValue( eAttribute, *p2++ );

            xAnimate->setValues( aValues );
        }

        OUString aFormula( xAnimate->getFormula() );
        if( !aFormula.isEmpty() )
        {
            if( convertMeasure( aFormula ) )
                xAnimate->setFormula( aFormula );
        }
    }

    // check for after-affect
    Sequence< NamedValue > aUserData( xNode->getUserData() );
    NamedValue* pValue = aUserData.getArray();
    NamedValue* pLastValue = pValue;
    sal_Int32 nLength = aUserData.getLength(), nRemoved = 0;

    sal_Bool bAfterEffect = false;
    sal_Int32 nMasterRel = 0;
    for( ; nLength--; pValue++ )
    {
        if ( pValue->Name == "after-effect" )
        {
            pValue->Value >>= bAfterEffect;
            nRemoved++;
        }
        else if ( pValue->Name == "master-rel" )
        {
            pValue->Value >>= nMasterRel;
            nRemoved++;
        }
        else
        {
            if( nRemoved )
                *pLastValue = *pValue;
            pLastValue++;
        }
    }

    if( nRemoved )
    {
        aUserData.realloc( aUserData.getLength() - nRemoved );
        xNode->setUserData( aUserData );
    }

    // if its an after effect node, add it to the list for
    // later processing
    // after effect nodes are not inserted at their import
    // position, so return false in this case
    if( bAfterEffect )
    {
        if( nMasterRel != 2 )
        {
            Event aEvent;

            aEvent.Source <<= xParent;
            aEvent.Trigger = EventTrigger::END_EVENT;
            aEvent.Repeat = 0;

            xNode->setBegin( makeAny( aEvent ) );
        }

        // add to after effect nodes for later processing
        sd::AfterEffectNode aNode( xNode, xParent, nMasterRel == 2 );
        maAfterEffectNodes.push_back( aNode );
        return false;
    }

    return true;
}

static int lcl_gethex( int nChar )
{
    if( nChar >= '0' && nChar <= '9' )
        return nChar - '0';
    else if( nChar >= 'a' && nChar <= 'f' )
        return nChar - 'a' + 10;
    else if( nChar >= 'A' && nChar <= 'F' )
        return nChar - 'A' + 10;
    else
        return 0;
}

bool AnimationImporter::convertAnimationValue( MS_AttributeNames eAttribute, Any& rValue )
{
    bool bRet = false;
    switch( eAttribute )
    {
    case MS_PPT_X:
    case MS_PPT_Y:
    case MS_PPT_W:
    case MS_PPT_H:
    {
        OUString aString;

        if( rValue.getValueType() == ::getCppuType((const ValuePair*)0) )
        {
            ValuePair aValuePair;
            if( rValue >>= aValuePair )
            {
                if( aValuePair.First >>= aString )
                {
                    if( convertMeasure( aString ) )
                    {
                        aValuePair.First <<= aString;
                        bRet = true;
                    }
                }

                if( aValuePair.Second >>= aString )
                {
                    if( convertMeasure( aString ) )
                    {
                        aValuePair.Second <<= aString;
                        bRet = true;
                    }
                }
            }
        }
        else if( rValue.getValueType() == ::getCppuType((const OUString*)0) )
        {
            if( rValue >>= aString )
            {
                bRet = convertMeasure( aString );

                if( bRet )
                    rValue <<= aString;
            }
        }
    }
    break;

    case MS_XSHEAR:
    case MS_R:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString.toDouble();
            bRet = true;
        }
    }
    break;

    case MS_STYLEROTATION:
    {
        if( rValue.getValueType() == ::getCppuType((const OUString*)0) )
        {
            OUString aString;
            rValue >>= aString;
            rValue <<= (sal_Int16)aString.toDouble();
            bRet = true;
        }
        else if( rValue.getValueType() == ::getCppuType((const double*)0) )
        {
            double fValue = 0.0;
            rValue >>= fValue;
            rValue <<= (sal_Int16)fValue;
            bRet = true;
        }
    }
    break;

    case MS_FILLCOLOR:
    case MS_STROKECOLOR:
    case MS_STYLECOLOR:
    case MS_PPT_C:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            if( aString.getLength() >= 7 && aString[0] == '#' )
            {
                Color aColor;
                aColor.SetRed( (sal_uInt8)(lcl_gethex( aString[1] ) * 16 + lcl_gethex( aString[2] )) );
                aColor.SetGreen( (sal_uInt8)(lcl_gethex( aString[3] ) * 16 + lcl_gethex( aString[4] )) );
                aColor.SetBlue( (sal_uInt8)(lcl_gethex( aString[5] ) * 16 + lcl_gethex( aString[6] )) );
                rValue <<= (sal_Int32)aColor.GetColor();
                bRet = true;
            }
            else if( aString.matchAsciiL( "rgb(", 4, 0 ) )
            {
                aString = aString.copy( 4, aString.getLength() - 5 );
                Color aColor;
                sal_Int32 index = 0;
                aColor.SetRed( (sal_uInt8)aString.getToken( 0, (sal_Unicode)',', index ).toInt32() );
                aColor.SetGreen( (sal_uInt8)aString.getToken( 0, (sal_Unicode)',', index ).toInt32() );
                aColor.SetRed( (sal_uInt8)aString.getToken( 0, (sal_Unicode)',', index ).toInt32() );
                rValue <<= (sal_Int32)aColor.GetColor();
                bRet = true;
            }
            else if( aString.matchAsciiL( "hsl(", 4, 0 ) )
            {
                sal_Int32 index = 0;
                sal_Int32 nA = aString.getToken( 0, (sal_Unicode)',', index ).toInt32();
                sal_Int32 nB = aString.getToken( 0, (sal_Unicode)',', index ).toInt32();
                sal_Int32 nC = aString.getToken( 0, (sal_Unicode)',', index ).toInt32();
                dump( "hsl(%ld", nA );
                dump( ",%ld", nB );
                dump( ",%ld)", nC );
                Sequence< double > aHSL( 3 );
                aHSL[0] = nA * 360.0/255.0;
                aHSL[1] = nB / 255.0;
                aHSL[2] = nC / 255.0;
                rValue <<= aHSL;
                bRet = true;
            }
        }
    }
    break;

    case MS_FILLTYPE:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "solid" ? FillStyle_SOLID : FillStyle_NONE;
            bRet = true;
        }
    }
    break;

    case MS_STROKEON:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "true" ? ::com::sun::star::drawing::LineStyle_SOLID : ::com::sun::star::drawing::LineStyle_NONE;
            bRet = true;
        }
    }
    break;

    case MS_FONTWEIGHT:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "bold" ? com::sun::star::awt::FontWeight::BOLD : com::sun::star::awt::FontWeight::NORMAL;
            bRet = true;
        }
    }
    break;

    case MS_STYLEFONTSTYLE:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "italic" ? com::sun::star::awt::FontSlant_ITALIC : com::sun::star::awt::FontSlant_NONE;
            bRet = true;
        }
    }
    break;

    case MS_STYLEUNDERLINE:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "true" ? com::sun::star::awt::FontUnderline::SINGLE : com::sun::star::awt::FontUnderline::NONE;
            bRet = true;
        }
    }
    break;

    case MS_STYLEOPACITY:
    case MS_STYLEFONTSIZE:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= (float)aString.toDouble();
            bRet = true;
        }
    }
    break;

    case MS_STYLEVISIBILITY:
    {
        OUString aString;
        if( rValue >>= aString )
        {
            rValue <<= aString == "visible" ? sal_True : sal_False;
            bRet = true;
        }
    }
    break;
    default:
        break;
    }

    return bRet;
}

// --------------------------------------------------------------------

static OUString getConvertedSubType( sal_Int16 nPresetClass, sal_Int32 nPresetId, sal_Int32 nPresetSubType )
{
    const sal_Char* pStr = 0;

    if( (nPresetClass == EffectPresetClass::ENTRANCE) || (nPresetClass == EffectPresetClass::EXIT) )
    {
        // skip wheel effect
        if( nPresetId != 21 )
        {
            if( nPresetId == 5 )
            {
                // checkerboard
                switch( nPresetSubType )
                {
                case  5: pStr = "downward"; break;
                case 10: pStr = "across"; break;
                }
            }
            else if( nPresetId == 17 )
            {
                // stretch
                if( nPresetSubType == 10 )
                    pStr = "across";
            }
            else if( nPresetId == 18 )
            {
                // strips
                switch( nPresetSubType )
                {
                case 3: pStr = "right-to-top"; break;
                case 6: pStr = "right-to-bottom"; break;
                case 9: pStr = "left-to-top"; break;
                case 12: pStr = "left-to-bottom"; break;
                }
            }

            if( pStr == 0 )
            {
                const convert_subtype* p = gConvertArray;

                while( p->mpStrSubType )
                {
                    if( p->mnID == nPresetSubType )
                    {
                        pStr = p->mpStrSubType;
                        break;
                    }
                    p++;
                }
            }
        }
    }

    if( pStr )
        return OUString::createFromAscii( pStr );
    else
        return OUString::number( nPresetSubType );
}

// --------------------------------------------------------------------

void AnimationImporter::fillNode( Reference< XAnimationNode >& xNode, const AnimationNode& rNode, const PropertySet& rSet )
{
    sal_Bool bAfterEffect = false;

    // attribute Restart
    if( rNode.mnRestart )
    {
        sal_Int16 nRestart = AnimationRestart::DEFAULT;
        switch( rNode.mnRestart )
        {
        case 1: nRestart = AnimationRestart::ALWAYS; break;
        case 2: nRestart = AnimationRestart::WHEN_NOT_ACTIVE; break;
        case 3: nRestart = AnimationRestart::NEVER; break;
        }
        xNode->setRestart( nRestart );
    }

    // attribute Fill
    if( rNode.mnFill )
    {
        sal_Int16 nFill = AnimationFill::DEFAULT;
        switch( rNode.mnFill )
        {
        case 1: nFill = AnimationFill::REMOVE; break;
        case 2: nFill = AnimationFill::FREEZE; break;
        case 3: nFill = AnimationFill::HOLD; break;
        case 4: nFill = AnimationFill::TRANSITION; break;
        }
        xNode->setFill( nFill );
    }

    // attribute Duration
    if( rNode.mnDuration )
    {
        Any aDuration;
        if( rNode.mnDuration > 0 )
        {
            aDuration <<= (double)(rNode.mnDuration / 1000.0);
        }
        else if( rNode.mnDuration < 0 )
        {
            aDuration <<= Timing_INDEFINITE;
        }
        xNode->setDuration( aDuration );
    }

    // TODO: DFF_ANIM_PATH_EDIT_MODE
    if( rSet.hasProperty( DFF_ANIM_PATH_EDIT_MODE ) )
    {
        sal_Int32 nPathEditMode ;
        if( rSet.getProperty( DFF_ANIM_PATH_EDIT_MODE ) >>= nPathEditMode )
        {
        }
    }

    // set user data
    Sequence< NamedValue > aUserData;

    // attribute Type
    if( rSet.hasProperty( DFF_ANIM_NODE_TYPE ) )
    {
        sal_Int32 nPPTNodeType = 0;
        if( rSet.getProperty( DFF_ANIM_NODE_TYPE ) >>= nPPTNodeType )
        {
            sal_Int16 nNodeType = ::com::sun::star::presentation::EffectNodeType::DEFAULT;
            switch( nPPTNodeType )
            {
                case DFF_ANIM_NODE_TYPE_ON_CLICK:       nNodeType = ::com::sun::star::presentation::EffectNodeType::ON_CLICK;   break;
                case DFF_ANIM_NODE_TYPE_WITH_PREVIOUS:  nNodeType = ::com::sun::star::presentation::EffectNodeType::WITH_PREVIOUS; break;
                case DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS: nNodeType = ::com::sun::star::presentation::EffectNodeType::AFTER_PREVIOUS; break;
                case DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE:  nNodeType = ::com::sun::star::presentation::EffectNodeType::MAIN_SEQUENCE; break;
                case DFF_ANIM_NODE_TYPE_TIMING_ROOT:    nNodeType = ::com::sun::star::presentation::EffectNodeType::TIMING_ROOT; break;
                case DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ:nNodeType = ::com::sun::star::presentation::EffectNodeType::INTERACTIVE_SEQUENCE; break;
            }

            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "node-type";
            aUserData[nSize].Value <<= nNodeType;
        }
    }

    if( rSet.hasProperty( DFF_ANIM_GROUP_ID ) )
    {
        sal_Int32 nGroupId;
        if( rSet.getProperty( DFF_ANIM_GROUP_ID ) >>= nGroupId )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "group-id";
            aUserData[nSize].Value <<= nGroupId;
        }
    }

    sal_Int16 nEffectPresetClass = EffectPresetClass::CUSTOM;
    sal_Int32 nPresetId = 0;

    if( rSet.hasProperty( DFF_ANIM_PRESET_CLASS ) )
    {
        sal_Int32 nPresetClass = 0;
        if ( rSet.getProperty( DFF_ANIM_PRESET_CLASS ) >>= nPresetClass )
        {
            switch( nPresetClass )
            {
            case DFF_ANIM_PRESS_CLASS_ENTRANCE:     nEffectPresetClass = EffectPresetClass::ENTRANCE; break;
            case DFF_ANIM_PRESS_CLASS_EXIT:         nEffectPresetClass = EffectPresetClass::EXIT; break;
            case DFF_ANIM_PRESS_CLASS_EMPHASIS:     nEffectPresetClass = EffectPresetClass::EMPHASIS; break;
            case DFF_ANIM_PRESS_CLASS_MOTIONPATH:   nEffectPresetClass = EffectPresetClass::MOTIONPATH; break;
            case DFF_ANIM_PRESS_CLASS_OLE_ACTION:   nEffectPresetClass = EffectPresetClass::OLEACTION; break;
            case DFF_ANIM_PRESS_CLASS_MEDIACALL:    nEffectPresetClass = EffectPresetClass::MEDIACALL; break;
            }
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "preset-class";
            aUserData[nSize].Value <<= nEffectPresetClass;
        }
    }

    if( rSet.hasProperty( DFF_ANIM_PRESET_ID ) )
    {
        if( rSet.getProperty( DFF_ANIM_PRESET_ID ) >>= nPresetId )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "preset-id";

            const preset_maping* p = gPresetMaping;
            while( p->mpStrPresetId && ((p->mnPresetClass != nEffectPresetClass) || (p->mnPresetId != nPresetId )) )
                p++;

            if( p->mpStrPresetId )
            {
                aUserData[nSize].Value <<= OUString::createFromAscii( p->mpStrPresetId );
            }
            else
            {
                OUStringBuffer sBuffer;
                sBuffer.appendAscii( "ppt_" );
                switch( nEffectPresetClass )
                {
                case EffectPresetClass::ENTRANCE: sBuffer.appendAscii( "entrance_" ); break;
                case EffectPresetClass::EXIT: sBuffer.appendAscii( "exit_" ); break;
                case EffectPresetClass::EMPHASIS: sBuffer.appendAscii( "emphasis_" ); break;
                case EffectPresetClass::MOTIONPATH: sBuffer.appendAscii( "motionpath_" ); break;
                case EffectPresetClass::OLEACTION: sBuffer.appendAscii( "oleaction_" ); break;
                case EffectPresetClass::MEDIACALL: sBuffer.appendAscii( "mediacall_" ); break;
                }
                sBuffer.append( nPresetId );

                aUserData[nSize].Value <<= sBuffer.makeStringAndClear();
            }
        }
    }

    if( rSet.hasProperty( DFF_ANIM_PRESET_SUB_TYPE ) )
    {
        sal_Int32 nPresetSubType = 0;
        if( (rSet.getProperty( DFF_ANIM_PRESET_SUB_TYPE ) >>= nPresetSubType) )
        {
            if( nPresetSubType )
            {
                sal_Int32 nSize = aUserData.getLength();
                aUserData.realloc(nSize+1);
                aUserData[nSize].Name = "preset-sub-type";
                aUserData[nSize].Value <<= getConvertedSubType( nEffectPresetClass, nPresetId, nPresetSubType );
            }
        }
    }

    if( rSet.hasProperty( DFF_ANIM_AFTEREFFECT ) )
    {
        if( rSet.getProperty( DFF_ANIM_AFTEREFFECT ) >>= bAfterEffect )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "after-effect";
            aUserData[nSize].Value <<= bAfterEffect;
        }
    }

    if( bAfterEffect && rSet.hasProperty( DFF_ANIM_MASTERREL ) )
    {
        sal_Int32 nMasterRel = 2;
        if( rSet.getProperty( DFF_ANIM_MASTERREL ) >>= nMasterRel )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            aUserData[nSize].Name = "master-rel";
            aUserData[nSize].Value <<= nMasterRel;
        }
    }

    xNode->setUserData( aUserData );

    // TODO: DFF_ANIM_ID
    if( rSet.hasProperty( DFF_ANIM_ID ) )
    {
        OUString aString;
        rSet.getProperty( DFF_ANIM_ID ) >>= aString;
        //if( !aString.isEmpty() )
        //{
        //}
    }

    // TODO: DFF_ANIM_EVENT_FILTER
    if( rSet.hasProperty( DFF_ANIM_EVENT_FILTER ) )
    {
        OUString aString;
        rSet.getProperty( DFF_ANIM_EVENT_FILTER ) >>= aString;
        //if( !aString.isEmpty() )
        //{
        //}
    }

    // DFF_ANIM_TIMEFILTER
    if( rSet.hasProperty( DFF_ANIM_TIMEFILTER ) )
    {
        Reference< XAnimate > xAnim( xNode, UNO_QUERY );
        if( xAnim.is() )
        {
            OUString aString;
            rSet.getProperty( DFF_ANIM_TIMEFILTER ) >>= aString;
            if( !aString.isEmpty() )
            {
                sal_Int32 nElements = 1; // a non empty string has at least one value

                sal_Int32 fromIndex = 0;
                while(true)
                {
                    fromIndex = aString.indexOf( (sal_Unicode)';', fromIndex );
                    if( fromIndex == -1 )
                        break;

                    fromIndex++;
                    nElements++;
                }

                Sequence< TimeFilterPair > aTimeFilter( nElements );

                TimeFilterPair* pValues = aTimeFilter.getArray();
                sal_Int32 nIndex = 0;
                while( (nElements--) && (nIndex >= 0) )
                {
                    const OUString aToken( aString.getToken( 0, ';', nIndex ) );

                    sal_Int32 nPos = aToken.indexOf( ',' );
                    if( nPos >= 0 )
                    {
                        pValues->Time = aToken.copy( 0, nPos ).toDouble();
                        pValues->Progress = aToken.copy( nPos+1, aToken.getLength() - nPos - 1 ).toDouble();
                    }
                    pValues++;
                }

                xAnim->setTimeFilter( aTimeFilter );
            }
        }
    }


// TODO: DFF_ANIM_ENDAFTERSLIDE / DFF_ANIM_VOLUME handling. git history has sample code
    Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
    if( xColor.is() )
    {
        if( rSet.hasProperty( DFF_ANIM_DIRECTION ) )
        {
            sal_Bool bDirection = sal_False;
            if( rSet.getProperty( DFF_ANIM_DIRECTION ) >>= bDirection )
                xColor->setDirection( (sal_Bool)!bDirection );
        }

        if( rSet.hasProperty( DFF_ANIM_COLORSPACE ) )
        {
            sal_Int32 nColorSpace = 0;
            rSet.getProperty( DFF_ANIM_COLORSPACE ) >>= nColorSpace;
            xColor->setColorInterpolation( (nColorSpace == 0) ? AnimationColorSpace::RGB : AnimationColorSpace::HSL );
        }
    }
}

// --------------------------------------------------------------------

int AnimationImporter::importTimeContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    int nNodes = 0;

    DBG_ASSERT( pAtom && xNode.is(), "invalid call to ppt::AnimationImporter::importTimeContainer()!");
    if( pAtom && xNode.is() )
    {
        importAnimationEvents( pAtom, xNode );
        importAnimationValues( pAtom, xNode );
        importAnimationActions( pAtom, xNode );

        dump(">\n");

        // import sub containers
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            switch( pChildAtom->getType() )
            {
                case DFF_msofbtAnimNode:
                case DFF_msofbtAnimEvent:
                case DFF_msofbtAnimValue:
                case DFF_msofbtAnimAction:
                case DFF_msofbtAnimPropertySet:
                    break;

                case DFF_msofbtAnimSubGoup :
                {
                    if( pChildAtom->hasChildAtom( DFF_msofbtAnimCommand ) )
                    {
                        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                        Reference< XAnimationNode > xChildNode( Command::create(xContext), UNO_QUERY_THROW );
                        nNodes += importAnimationNodeContainer( pChildAtom, xChildNode );
                        Reference< XTimeContainer > xParentContainer( xNode, UNO_QUERY );
                        if( xParentContainer.is() && xChildNode.is() )
                            xParentContainer->appendChild( xChildNode );
                    }
                    else
                    {
                        nNodes += importAnimationContainer( pChildAtom, xNode );
                    }
                }
                break;
                case DFF_msofbtAnimGroup :
                {
                    nNodes += importAnimationContainer( pChildAtom, xNode );
                }
                break;
                case DFF_msofbtAnimIteration:
                {
                    if( pChildAtom->seekToContent() )
                    {
                        float fInterval;
                        sal_Int32 nTextUnitEffect, nU1, nU2, nU3;

                        mrStCtrl >> fInterval >> nTextUnitEffect >> nU1 >> nU2 >> nU3;

                        Reference< XIterateContainer > xIter( xNode, UNO_QUERY );
                        if( xIter.is() )
                        {
                            sal_Int16 nIterateType = TextAnimationType::BY_PARAGRAPH;
                            switch( nTextUnitEffect )
                            {
                            case 1: nIterateType = TextAnimationType::BY_WORD; break;
                            case 2: nIterateType = TextAnimationType::BY_LETTER; break;
                            }
                            xIter->setIterateType( nIterateType );
                            xIter->setIterateInterval( (double)fInterval );
                        }

                        nNodes++;

                        dump( "<iterate" );
                        dump( " iterateType=\"%s\"", (nTextUnitEffect == 0) ? "byElement" : (nTextUnitEffect == 1) ? "byWord" : "byLetter" );
                        dump( " iterateInterval=\"%g\"", fInterval );
                        dump( " u1=\"%ld\"", nU1 );
                        dump( " u2=\"%ld\"", nU2 );
                        dump( " u3=\"%ld\"/>\n", nU3 );
                    }
                }
                break;

                case 0xf136:
                {
#ifdef DBG_ANIM_LOG
                    sal_uInt32 nU1, nU2;
                    mrStCtrl >> nU1 >> nU2;

                    fprintf( mpFile, "<unknown_0xf136 nU1=\"%ld\" nU2=\"%ld\"/>\n", nU1, nU2 );
#endif
                }
                break;

                default:
                {
                    dump_atom_header( pChildAtom, true, false );
                    dump_atom( pChildAtom );
                    dump_atom_header( pChildAtom, false, false );
                }
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }

    return nNodes;
}

// --------------------------------------------------------------------

int AnimationImporter::importAnimationNodeContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    int nNodes = 0;

    DBG_ASSERT( pAtom && xNode.is(), "invalid call to ppt::AnimationImporter::importAnimationNodeContainer()!");
    if( pAtom && xNode.is() )
    {
        importAnimationEvents( pAtom, xNode );
        importAnimationValues( pAtom, xNode );
        importAnimationActions( pAtom, xNode );

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            nNodes ++;
            switch( pChildAtom->getType() )
            {
                case DFF_msofbtAnimNode:
                case DFF_msofbtAnimEvent:
                case DFF_msofbtAnimValue:
                case DFF_msofbtAnimAction:
                case DFF_msofbtAnimPropertySet:
                    break;

                case DFF_msofbtAnimateFilter:
                    importAnimateFilterContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimateSet:
                    importAnimateSetContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimate:
                    importAnimateContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimateScale:
                    importAnimateScaleContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimateColor:
                    importAnimateColorContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimateRotation:
                    importAnimateRotationContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimateMotion:
                    importAnimateMotionContainer( pChildAtom, xNode );
                    break;

                case DFF_msofbtAnimCommand:
                    importCommandContainer( pChildAtom, xNode );
                    break;

                default:
                {
                    nNodes --;
                    dump_atom_header( pChildAtom, true, false );
                    dump_atom( pChildAtom );
                    dump_atom_header( pChildAtom, false, false );
                }
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }

    return nNodes;
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateFilterContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XTransitionFilter > xFilter( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateFilter && xFilter.is(), "invalid call to ppt::AnimationImporter::importAnimateFilterContainer()!");
    if( pAtom && xFilter.is() )
    {
        sal_uInt32 nBits = 0;

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateFilterData:
            {
                sal_uInt32 transition;
                mrStCtrl >> nBits;
                mrStCtrl >> transition;

                if( nBits & 1 )
                    xFilter->setMode( transition == 0 );

                dump( " transition=\"%s\"", (transition == 0) ? "in" : "out" );
            }
            break;

            case DFF_msofbtAnimAttributeValue:
            {
                if( (nBits & 2 ) && ( pChildAtom->getInstance() == 1 )  )
                {
                    Any aAny;
                    if ( importAttributeValue( pChildAtom, aAny ) )
                    {
                        OUString filter;
                        aAny >>= filter;

                        dump( " filter=\"%s\"", filter );

                        const transition* pTransition = transition::find( filter );
                        if( pTransition )
                        {
                            xFilter->setTransition( pTransition->mnType );
                            xFilter->setSubtype( pTransition->mnSubType );
                            xFilter->setDirection( pTransition->mbDirection );
                        }
                        else
                        {
                            OSL_FAIL( "unknown transition!" );
                        }
                    }
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;

            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateAttributeTargetContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateTarget, "invalid call to ppt::AnimationImporter::importAnimateAttributeTargetContainer()!");

    Any aTarget;

    Reference< XAnimate > xAnimate( xNode, UNO_QUERY );

    bool bWrongContext = false;

    if( pAtom )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimPropertySet:
            {
                PropertySet aSet;
                importPropertySetContainer( pChildAtom, aSet );
                if( aSet.hasProperty( DFF_ANIM_RUNTIMECONTEXT ) )
                {
                    OUString aContext;
                    if( aSet.getProperty( DFF_ANIM_RUNTIMECONTEXT ) >>= aContext )
                    {
                        if( aContext != "PPT" )
                            bWrongContext = true;
                    }
                }

                dump( aSet );
            }
            break;

            case DFF_msofbtAnimateTargetSettings:
            {
                if( xAnimate.is() )
                {
                    sal_uInt32 nBits;
                    sal_uInt32 nAdditive;
                    sal_uInt32 nAccumulate;
                    sal_uInt32 nTransformType;

                    mrStCtrl >> nBits >> nAdditive >> nAccumulate >> nTransformType;

                    // nBits %0001: additive, %0010: accumulate, %0100: attributeName, %1000: transformtype
                    // nAdditive 0 = base, 1 = sum, 2 = replace, 3 = multiply, 4 = none
                    // nAccumulate 0 = none, 1 = always
                    // nTransformType 0: "property" else "image"

                    if( nBits & 3 )
                    {
                        if( xAnimate.is() )
                        {
                            if( nBits & 1 )
                            {
                                sal_Int16 nTemp = AnimationAdditiveMode::BASE;
                                switch( nAdditive )
                                {
                                case 1: nTemp = AnimationAdditiveMode::SUM; break;
                                case 2: nTemp = AnimationAdditiveMode::REPLACE; break;
                                case 3: nTemp = AnimationAdditiveMode::MULTIPLY; break;
                                case 4: nTemp = AnimationAdditiveMode::NONE; break;
                                }
                                xAnimate->setAdditive( nTemp );
                            }

                            if( nBits & 2 )
                            {
                                xAnimate->setAccumulate( (nAccumulate == 0) ? sal_True : sal_False );
                            }
                        }
                    }
#ifdef DBG_ANIM_LOG
                    if( nBits & 1 )
                        fprintf( mpFile, " additive=\"%s\"", (nAdditive == 0) ? "base" : (nAdditive == 2) ? "replace" : (nAdditive == 1) ? "sum" : (nAdditive == 3 ) ? "multiply" : (nAdditive == 4) ? "none" : "unknown" );

                    if( nBits & 2 )
                        fprintf( mpFile, " accumulate=\"%s\"", (nAccumulate == 0) ? "none" : "always" );

                    if( nBits & 8 )
                        fprintf( mpFile, " transformType=\"%s\"", (nTransformType == 0) ? "property" : "image" );
#endif
                }
            }
            break;

            case DFF_msofbtAnimateAttributeNames:
            {
                if( xAnimate.is() )
                {
                    OUString aAttributeName;
                    importAttributeNamesContainer( pChildAtom, aAttributeName );
                    if( xAnimate.is() )
                        xAnimate->setAttributeName( aAttributeName );
                    dump( " attributeName=\"%s\"", aAttributeName );
                }
            }
            break;

            case DFF_msofbtAnimateTargetElement:
            {
                sal_Int16 nSubType;
                importTargetElementContainer( pChildAtom, aTarget, nSubType );
                if( xAnimate.is() )
                    xAnimate->setSubItem( nSubType );

                dump( " target=\"" );
                dump_target( aTarget );
                dump( "\"" );
            }
            break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }

    if( bWrongContext )
        aTarget.clear();

    if( xAnimate.is() )
        xAnimate->setTarget( aTarget );
    else
    {
        Reference< XCommand > xCommand( xNode, UNO_QUERY );
        if( xCommand.is() )
            xCommand->setTarget( aTarget );
    }
}

// --------------------------------------------------------------------

sal_Int16 AnimationImporter::implGetColorSpace( sal_Int32 nMode, sal_Int32 /*nA*/, sal_Int32 /*nB*/, sal_Int32 /*nC*/ )
{
    switch( nMode )
    {
    case 2: // index
        // FALLTHROUGH intended
    default:
        // FALLTHROUGH intended
    case 0: // rgb
        return AnimationColorSpace::RGB;

    case 1: // hsl
        return AnimationColorSpace::HSL;
    }
}

// --------------------------------------------------------------------

Any AnimationImporter::implGetColorAny( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC )
{
    switch( nMode )
    {
    case 0: // rgb
        {
            dump( "rgb(%ld", nA );
            dump( ",%ld", nB );
            dump( ",%ld)", nC );
            Color aColor( (sal_uInt8)nA, (sal_uInt8)nB, (sal_uInt8)nC );
            return makeAny( (sal_Int32)aColor.GetRGBColor() );
        }
    case 1: // hsl
        {
            dump( "hsl(%ld", nA );
            dump( ",%ld", nB );
            dump( ",%ld)", nC );
            Sequence< double > aHSL( 3 );
            aHSL[0] = nA * 360.0/255.0;
            aHSL[1] = nB / 255.0;
            aHSL[2] = nC / 255.0;
            return makeAny( aHSL );
        }

    case 2: // index
        {
            Color aColor;
            mpPPTImport->GetColorFromPalette((sal_uInt16)nA, aColor );
            dump( "index(%ld", nA );
            dump( " [%ld", (sal_Int32)aColor.GetRed() );
            dump( ",%ld", (sal_Int32)aColor.GetGreen() );
            dump( ",%ld])", (sal_Int32)aColor.GetBlue() );
            return makeAny( (sal_Int32)aColor.GetRGBColor() );
        }

    default:
        {
            dump( "unknown_%ld(", nMode );
            dump( "%ld", nA );
            dump( ",%ld", nB );
            dump( ",%ld)", nC );
            OSL_FAIL( "ppt::implGetColorAny(), unhandled color type" );

            Any aAny;
            return aAny;
        }
    }
}

void AnimationImporter::importAnimateColorContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateColor > xColor( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateColor && xColor.is(), "invalid call to ppt::AnimationImporter::importAnimateColorContainer()!");
    if( pAtom && xColor.is() )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateColorData:
            {
                sal_uInt32 nBits;
                sal_Int32 nByMode, nByA, nByB, nByC;
                sal_Int32 nFromMode, nFromA, nFromB, nFromC;
                sal_Int32 nToMode, nToA, nToB, nToC;
                mrStCtrl >> nBits;
                mrStCtrl >> nByMode >> nByA >> nByB >> nByC;
                mrStCtrl >> nFromMode >> nFromA >> nFromB >> nFromC;
                mrStCtrl >> nToMode >> nToA >> nToB >> nToC;

                if( nBits & 1 )
                {
                    dump( " by=\"" );
                    xColor->setBy( implGetColorAny( nByMode, nByA, nByB, nByC ) );
                    xColor->setColorInterpolation( implGetColorSpace( nByMode, nByA, nByB, nByC ) );
                    dump( "\"");
                }

                if( nBits & 2 )
                {
                    dump( " from=\"" );
                    xColor->setFrom( implGetColorAny( nFromMode, nFromA, nFromB, nFromC ) );
                    xColor->setColorInterpolation( implGetColorSpace( nFromMode, nFromA, nFromB, nFromC ) );
                    dump( "\"");
                }

                if( nBits & 4 )
                {
                    dump( " to=\"" );
                    xColor->setTo( implGetColorAny( nToMode, nToA, nToB, nToC ) );
                    xColor->setColorInterpolation( implGetColorSpace( nToMode, nToA, nToB, nToC ) );
                    dump( "\"");
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateSetContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateSet > xSet( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateSet && xSet.is(), "invalid call to ppt::AnimationImporter::importAnimateSetContainer()!");
    if( pAtom && xSet.is() )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateSetData:
            {
                sal_Int32 nU1, nU2;
                mrStCtrl >> nU1 >> nU2;

                dump( " set_1=\"%ld\"", nU1 ),
                dump( " set_2=\"%ld\"", nU2 );
            }
            break;

            case DFF_msofbtAnimAttributeValue:
            {
                Any aTo;
                if ( importAttributeValue( pChildAtom, aTo ) )
                {
                    xSet->setTo( aTo );

                    dump( " value=\"" );
                    dump( aTo );
                    dump( "\"" );
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimate > xAnim( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimate && xAnim.is(), "invalid call to ppt::AnimationImporter::importAnimateContainer()!");
    if( pAtom && xAnim.is() )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateData:
            {
                sal_uInt32 nCalcmode, nBits, nValueType;
                mrStCtrl >> nCalcmode >> nBits >> nValueType;

                if( nBits & 0x08 )
                {
                    sal_Int16 n = (nCalcmode == 1) ? AnimationCalcMode::LINEAR : /* (nCalcmode == 2) ? AnimationCalcMode::FORMULA : */ AnimationCalcMode::DISCRETE;
                    xAnim->setCalcMode( n );
                    dump( " calcmode=\"%s\"", (nCalcmode == 0) ? "discrete" : (nCalcmode == 1) ? "linear" : (nCalcmode == 2) ? "formula" : "unknown" );
                }

                if( nBits & 0x30 )
                {
                    sal_Int16 n = (nValueType == 1) ? AnimationValueType::NUMBER : (nValueType == 2 ) ? AnimationValueType::COLOR : AnimationValueType::STRING;
                    xAnim->setValueType( n );
                    dump( " valueType=\"%s\"", (nValueType == 0) ? "string" : (nValueType == 1) ? "number" : (nValueType == 2) ? "color" : "unknown" );
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            case DFF_msofbtAnimKeyPoints:
                importAnimateKeyPoints( pChildAtom, xNode );
                break;

            case DFF_msofbtAnimAttributeValue:
                {
                    Any a;
                    if ( importAttributeValue( pChildAtom, a ) )
                    {
                        switch( pChildAtom->getInstance() )
                        {
                        case 1: xAnim->setBy( a ); dump( " by=\"" ); break;
                        case 2: xAnim->setFrom( a ); dump( " from=\"" ); break;
                        case 3: xAnim->setTo( a ); dump( " to=\"" ); break;
                        default:
                            dump( " unknown_value=\"" );
                        }

                        dump( a );
                        dump( "\"" );
                    }
                }
                break;
            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateMotionContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateMotion > xMotion( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateMotion && xMotion.is(), "invalid call to ppt::AnimationImporter::importAnimateMotionContainer()!");
    if( pAtom && xMotion.is() )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateMotionData:
            {
                sal_uInt32 nBits, nOrigin;
                float fByX, fByY, fFromX, fFromY, fToX, fToY;

                mrStCtrl >> nBits >> fByX >> fByY >> fFromX >> fFromY >> fToX >> fToY >> nOrigin;

#ifdef DBG_ANIM_LOG
                if( nBits & 1 )
                    fprintf( mpFile, " by=\"%g,%g\"", (double)fByX, (double)fByY );

                if( nBits & 2 )
                    fprintf( mpFile, " from=\"%g,%g\"", (double)fFromX, (double)fFromY );

                if( nBits & 4 )
                    fprintf( mpFile, " to=\"%g,%g\"", (double)fToX, (double)fToY );

                if( nBits & 8 )
                    fprintf( mpFile, " origin=\"%s\"", (nOrigin == 1) ? "parent" : (nOrigin == 2) ? "layout" : "unknown" );

#endif
            }
            break;

            case DFF_msofbtAnimAttributeValue:
            {
                Any aPath;
                if ( importAttributeValue( pChildAtom, aPath ) )
                {
                    OUString aStr;
                    if ( aPath >>= aStr )
                    {
                        aStr = aStr.replace( 'E', ' ' );
                        aStr = aStr.trim();
                        aPath <<= aStr;
                        xMotion->setPath( aPath );
                        dump( " path=\"" );
                        dump( aPath );
                        dump( "\"" );
                    }
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importCommandContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XCommand > xCommand( xNode, UNO_QUERY );
    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimCommand && xCommand.is(), "invalid call to ppt::AnimationImporter::importCommandContainer()!");
    if( pAtom && xCommand.is() )
    {
        sal_Int32 nBits = 0, nType = 0;
        Any aValue;

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtCommandData:
            {
                sal_Int32 nCommandType;
                // looks like U1 is a bitset, bit 1 enables the type and bit 2 enables
                // a propertyvalue that follows
                mrStCtrl >> nBits;
                mrStCtrl >> nCommandType;

                if( nBits & 1 )
                {
                    dump( " type=\"%s\"", (nCommandType == 0) ? "event" : ( nCommandType == 1) ? "call" : "verb" );
                }
            }
            break;

            case DFF_msofbtAnimAttributeValue:
            {
                if ( importAttributeValue( pChildAtom, aValue ) )
                {
                    if( nBits & 2 )
                    {
                        dump( " cmd=\"" );
                        dump( aValue );
                        dump( "\"" );
                    }
                }
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }

        if( nBits & 3 )
        {
            OUString aParam;
            aValue >>= aParam;

            sal_Int16 nCommand = EffectCommands::CUSTOM;

            NamedValue aParamValue;

            switch( nType )
            {
            case 0: // event
            case 1: // call
                if ( aParam == "onstopaudio" )
                {
                    nCommand = EffectCommands::STOPAUDIO;
                }
                else if ( aParam == "play" )
                {
                    nCommand = EffectCommands::PLAY;
                }
                else if( aParam.startsWith( "playFrom" ) )
                {
                    const OUString aMediaTime( aParam.copy( 9, aParam.getLength() - 10 ) );
                    rtl_math_ConversionStatus eStatus;
                    double fMediaTime = ::rtl::math::stringToDouble( aMediaTime, (sal_Unicode)('.'), (sal_Unicode)(','), &eStatus, NULL );
                    if( eStatus == rtl_math_ConversionStatus_Ok )
                    {
                        aParamValue.Name = "MediaTime";
                        aParamValue.Value <<= fMediaTime;
                    }
                    nCommand = EffectCommands::PLAY;
                }
                else if ( aParam == "togglePause" )
                {
                    nCommand = EffectCommands::TOGGLEPAUSE;
                }
                else if ( aParam == "stop" )
                {
                    nCommand = EffectCommands::STOP;
                }
                break;
            case 2: // verb
                {
                    aParamValue.Name = "Verb";
                    aParamValue.Value <<= aParam.toInt32();

                    nCommand = EffectCommands::VERB;
                }
                break;
            }

            xCommand->setCommand( nCommand );
            if( nCommand == EffectCommands::CUSTOM )
            {
                OSL_FAIL("sd::AnimationImporter::importCommandContainer(), unknown command!");
                aParamValue.Name = "UserDefined";
                aParamValue.Value <<= aParam;
            }

            if( aParamValue.Value.hasValue() )
            {
                Sequence< NamedValue > aParamSeq( &aParamValue, 1 );
                xCommand->setParameter( makeAny( aParamSeq ) );
            }
        }
    }
}

// --------------------------------------------------------------------

int AnimationImporter::importAudioContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    int nNodes = 0;

    Reference< XAudio > xAudio( xNode, UNO_QUERY );
    DBG_ASSERT( pAtom && xAudio.is() &&
                 ( (pAtom->getType() == DFF_msofbtAnimGroup) ||
                   (pAtom->getType() == DFF_msofbtAnimSubGoup) ), "invalid call to ppt::AnimationImporter::importAudioContainer()!");
    if( pAtom && xAudio.is() )
    {
        importAnimationEvents( pAtom, xNode );
        importAnimationValues( pAtom, xNode );
        importAnimationActions( pAtom, xNode );

        dump(">\n");

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimNode:
            case DFF_msofbtAnimEvent:
            case DFF_msofbtAnimValue:
            case DFF_msofbtAnimAction:
            case DFF_msofbtAnimPropertySet:
                break;

            case DFF_msofbtAnimAttributeValue:
            {
                Any aValue;
                if ( importAttributeValue( pChildAtom, aValue ) )
                {
                    nNodes ++;
                    dump( " value=\"" );
                    dump( aValue );
                    dump( "\"" );
                }
            }
            break;

            case DFF_msofbtAnimateTargetElement:
            {
                sal_Int16 nSubType;
                Any aSource;
                importTargetElementContainer( pChildAtom, aSource, nSubType );
                if( xAudio.is() ) {
                    xAudio->setSource( aSource );
                    nNodes ++;
                }
            }
            break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }

        // TODO: What to do with them?
        Any aEmpty;
        xAudio->setBegin( aEmpty );
        xAudio->setEnd( aEmpty );
    }

    return nNodes;
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateScaleContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateScale && xTransform.is(), "invalid call to ppt::AnimationImporter::importAnimateScaleContainer()!");
    if( pAtom && xTransform.is() )
    {
        xTransform->setTransformType( AnimationTransformType::SCALE );

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateScaleData:
            {
                sal_uInt32 nBits, nZoomContents;
                float fByX, fByY, fFromX, fFromY, fToX, fToY;

                // nBits %001: by, %010: from, %100: to, %1000: zoomContents(bool)
                mrStCtrl >> nBits >> fByX >> fByY >> fFromX >> fFromY >> fToX >> fToY >> nZoomContents;

                ValuePair aPair;
                // 'from' value
                if( nBits & 2 )
                {
                    aPair.First <<= (double)fFromX / 100.0;
                    aPair.Second <<= (double)fFromY / 100.0;
                    xTransform->setFrom( makeAny( aPair ) );
                }

                // 'to' value
                if( nBits & 4 )
                {
                    aPair.First <<= (double)fToX / 100.0;
                    aPair.Second <<= (double)fToY / 100.0;
                    xTransform->setTo( makeAny( aPair ) );
                }

                // 'by' value
                if( nBits & 1 )
                {
                    aPair.First <<= (double)fByX / 100.0;
                    aPair.Second <<= (double)fByY / 100.0;

                    if( nBits & 2 )
                    {
                        // 'from' value given, import normally
                        xTransform->setBy( makeAny( aPair ) );
                    }
                    else
                    {
                        // mapping 'by' to 'to', if no 'from' is
                        // given. This is due to a non-conformity in
                        // PPT, which exports animateScale effects
                        // with a sole 'by' value, but with the
                        // semantics of a sole 'to' animation
                        xTransform->setTo( makeAny( aPair ) );
                    }
                }


#ifdef DBG_ANIM_LOG
                if( nBits & 1 )
                    fprintf( mpFile, " by=\"%g,%g\"", (double)fByX, (double)fByY );

                if( nBits & 2 )
                    fprintf( mpFile, " from=\"%g,%g\"", (double)fFromX, (double)fFromY );

                if( nBits & 4 )
                    fprintf( mpFile, " to=\"%g,%g\"", (double)fToX, (double)fToY );

                if( nBits & 8 )
                    fprintf( mpFile, " zoomContents=\"%s\"", nZoomContents ? "true" : "false" );
#endif
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateRotationContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateRotation && xTransform.is(), "invalid call to ppt::AnimationImporter::importAnimateRotationContainer()!");
    if( pAtom && xTransform.is() )
    {
        xTransform->setTransformType( AnimationTransformType::ROTATE );

        const Atom* pChildAtom = pAtom->findFirstChildAtom();

        while( pChildAtom )
        {
            if( !pChildAtom->isContainer() )
            {
                if( !pChildAtom->seekToContent() )
                    break;
            }

            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimateRotationData:
            {
                sal_uInt32 nBits, nU1;
                float fBy, fFrom, fTo;

                // nBits %001: by, %010: from, %100: to, %1000: zoomContents(bool)
                mrStCtrl >> nBits >> fBy >> fFrom >> fTo >> nU1;

                if( nBits & 1 )
                    xTransform->setBy( makeAny( (double) fBy ) );

                if( nBits & 2 )
                    xTransform->setFrom( makeAny( (double) fFrom ) );

                if( nBits & 4 )
                    xTransform->setTo( makeAny( (double) fTo ) );

#ifdef DBG_ANIM_LOG
                if( nBits & 1 )
                    fprintf( mpFile, " by=\"%g\"", (double)fBy );

                if( nBits & 2 )
                    fprintf( mpFile, " from=\"%g\"", (double)fFrom );

                if( nBits & 4 )
                    fprintf( mpFile, " to=\"%g\"", (double)fTo );

                if( nU1 )
                    fprintf( mpFile, " rotation_1=\"%ld\"", nU1 );
#endif
            }
            break;

            case DFF_msofbtAnimateTarget:
                importAnimateAttributeTargetContainer( pChildAtom, xNode );
                break;

            default:
                dump( " unknown_atom=\"%ld\"", (sal_Int32)pChildAtom->getType() );
                break;
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}
// --------------------------------------------------------------------

bool AnimationImporter::importAttributeNamesContainer( const Atom* pAtom, OUString& rAttributeNames )
{
    OUStringBuffer aNames;

    DBG_ASSERT( pAtom && (pAtom->getType() == DFF_msofbtAnimateAttributeNames), "invalid call to ppt::AnimationImporter::importAttributeName()!" );
    if( pAtom )
    {
        const Atom* pAttributeValueAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimAttributeValue );

        while( pAttributeValueAtom )
        {
            Any aAny;
            if ( importAttributeValue( pAttributeValueAtom, aAny ) )
            {
                OUString aName;
                if( aAny >>= aName )
                {
                    if( !aNames.isEmpty() )
                        aNames.append( (sal_Unicode)';' );

                    aNames.append( aName );
                }
            }
            else
            {
                OSL_FAIL( "error during ppt::AnimationImporter::importAttributeName()!" );
            }

            pAttributeValueAtom = pAtom->findNextChildAtom( DFF_msofbtAnimAttributeValue, pAttributeValueAtom );
        }
    }

    rAttributeNames = aNames.makeStringAndClear();
    return true;
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimationValues( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( pAtom, "invalid call to ppt::AnimationImporter::importAnimationValues()!" );

    if( pAtom )
    {
        const Atom* pValueAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimValue );

        while( pValueAtom && pValueAtom->seekToContent() )
        {
            sal_uInt32 nType;
            mrStCtrl >> nType;
            switch( nType )
            {
            case 0:
            {
                float fRepeat;
                mrStCtrl >> fRepeat;
                xNode->setRepeatCount( (fRepeat < ((float)3.40282346638528860e+38)) ? makeAny( (double)fRepeat ) : makeAny( Timing_INDEFINITE ) );

#ifdef DBG_ANIM_LOG
                if( (fRepeat < ((float)3.40282346638528860e+38)) )
                {
                    dump( " repeat=\"%g\"", (double)fRepeat );
                }
                else
                {
                    dump( " repeat=\"indefinite\"" );
                }
#endif
            }
            break;

            case 3:
            {
                float faccelerate;
                mrStCtrl >> faccelerate;
                xNode->setAcceleration( faccelerate );
                dump( " accelerate=\"%g\"", (double)faccelerate );
            }
            break;

            case 4:
            {
                float fdecelerate;
                mrStCtrl >> fdecelerate;
                xNode->setDecelerate( fdecelerate );
                dump( " decelerate=\"%g\"", (double)fdecelerate );
            }
            break;

            case 5:
            {
                sal_Int32 nAutoreverse;
                mrStCtrl >> nAutoreverse;
                xNode->setAutoReverse( nAutoreverse != 0 );
                dump( " autoreverse=\"%#lx\"", nAutoreverse );
            }
            break;

            default:
            {
                sal_uInt32 nUnknown;
                mrStCtrl >> nUnknown;
#ifdef DBG_ANIM_LOG
                fprintf(mpFile, " attribute_%d=\"%#lx\"", nType, nUnknown );
#endif
            }
            break;
            }

            pValueAtom = pAtom->findNextChildAtom( DFF_msofbtAnimValue, pValueAtom );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimateKeyPoints( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimate > xAnim( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimKeyPoints && xAnim.is(), "invalid call to ppt::AnimationImporter::importAnimateKeyPoints()!" );

    if( pAtom && xAnim.is() )
    {
        // first count keytimes
        const Atom* pIter = NULL;
        int nKeyTimes = 0;

        while( (pIter = pAtom->findNextChildAtom( DFF_msofbtAnimKeyTime,  pIter )) != 0 )
            nKeyTimes++;

        Sequence< double > aKeyTimes( nKeyTimes );
        Sequence< Any > aValues( nKeyTimes );
        OUString aFormula;

        pIter = pAtom->findFirstChildAtom(DFF_msofbtAnimKeyTime);
        int nKeyTime;
        sal_Int32 nTemp;
        for( nKeyTime = 0; (nKeyTime < nKeyTimes) && pIter; nKeyTime++ )
        {
            if( pIter->seekToContent() )
            {
                mrStCtrl >> nTemp;
                double fTemp = (double)nTemp / 1000.0;
                aKeyTimes[nKeyTime] = fTemp;

                const Atom* pValue = pAtom->findNextChildAtom(pIter);
                if( pValue && pValue->getType() == DFF_msofbtAnimAttributeValue )
                {
                    Any aValue1, aValue2;
                    if( importAttributeValue( pValue, aValue1 ) )
                    {
                        pValue = pAtom->findNextChildAtom(pValue);
                        if( pValue && pValue->getType() == DFF_msofbtAnimAttributeValue )
                            importAttributeValue( pValue, aValue2 );

                        bool bCouldBeFormula = false;
                        bool bHasValue = aValue2.hasValue();
                        if( bHasValue )
                        {
                            if( aValue2.getValueType() == ::getCppuType((const OUString*)0) )
                            {
                                OUString aTest;
                                aValue2 >>= aTest;
                                bHasValue = !aTest.isEmpty();
                                bCouldBeFormula = true;
                            }
                        }

                        if( bHasValue && bCouldBeFormula && (aValue1.getValueType() == ::getCppuType((const double*)0)) )
                        {
                            aValue2 >>= aFormula;
                            bHasValue = false;
                        }

                        if( bHasValue )
                        {
                            aValues[nKeyTime] = makeAny( ValuePair( aValue1, aValue2 ) );
                        }
                        else
                        {
                            aValues[nKeyTime] = aValue1;
                        }
                    }
                }
            }
            pIter = pAtom->findNextChildAtom(DFF_msofbtAnimKeyTime, pIter);
        }

#ifdef DBG_ANIM_LOG
        dump( " keyTimes=\"" );
        for( int i=0; i<nKeyTimes; ++i )
            dump( "%f;", aKeyTimes[i] );

        if( !aFormula.isEmpty() )
        {
            dump( "formula=\"%s", aFormula );
        }

        dump( "\" values=\"" );
        double nVal;
        OUString aStr;
        for( int i=0; i<nKeyTimes; ++i )
        {
            if( i != 0 )
                dump( ";" );

            if( aValues[i] >>= aStr )
                dump( "%s",
                      OUStringToOString( aStr,
                                                RTL_TEXTENCODING_ASCII_US ).getStr() );
            else if( aValues[i] >>= nVal )
                dump( "%f", nVal );
            else
            {
                ValuePair aValuePair;

                if( aValues[i] >>= aValuePair )
                {
                    if( aValuePair.First >>= aStr )
                        dump( "%s",
                              OUStringToOString( aStr,
                                                        RTL_TEXTENCODING_ASCII_US ).getStr() );
                    else if( aValuePair.First >>= nVal )
                        dump( "%f", nVal );
                    else
                        dump( "%X", (sal_Int64)&aValuePair.First );

                    if( aValuePair.Second >>= aStr )
                        dump( ",%s",
                              OUStringToOString( aStr,
                                                        RTL_TEXTENCODING_ASCII_US ).getStr() );
                    else if( aValuePair.Second >>= nVal )
                        dump( ",%f", nVal );
                    else
                        dump( ",%X", (sal_Int64)&aValuePair.Second );
                }
            }
        }
        dump( "\"" );
#endif

        xAnim->setKeyTimes( aKeyTimes );
        xAnim->setValues( aValues );
        xAnim->setFormula( aFormula );
    }
}

// --------------------------------------------------------------------

bool AnimationImporter::importAttributeValue( const Atom* pAtom, Any& rAny )
{
    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimAttributeValue, "invalid call to ppt::AnimationImporter::importAttributeValue()!" );

    bool bOk = false;

    if( pAtom && pAtom->seekToContent() )
    {
        sal_uInt32 nRecLen = pAtom->getLength();
        if ( nRecLen >= 1 )
        {
            sal_Int8 nType;
            mrStCtrl >> nType;
            switch( nType )
            {
                case DFF_ANIM_PROP_TYPE_BYTE :
                {
                    if ( nRecLen == 2 )
                    {
                        sal_uInt8 nByte;
                        mrStCtrl >> nByte;
                        rAny <<= nByte;

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_INT32 :
                {
                    if ( nRecLen == 5 )
                    {
                        sal_uInt32 nInt32;
                        mrStCtrl >> nInt32;
                        rAny <<= nInt32;

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_FLOAT:
                {
                    if( nRecLen == 5 )
                    {
                        float fFloat;
                        mrStCtrl >> fFloat;
                        rAny <<= (double)fFloat;

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_UNISTRING :
                {
                    if ( ( nRecLen & 1 ) && ( nRecLen > 1 ) )
                    {
                        OUString aOUString = mpPPTImport->MSDFFReadZString( mrStCtrl, nRecLen - 1, sal_True );
                        rAny <<= aOUString;

                        bOk = true;
                    }
                }
                break;
            }
        }
    }

    DBG_ASSERT( bOk, "invalid value inside ppt::AnimationImporter::importAttributeValue()!" );
    return bOk;
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimationEvents( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( xNode.is() && pAtom, "invalid call to ppt::AnimationImporter::importAnimationEvents()!" );

    Any aBegin, aEnd, aNext, aPrev;

    const Atom* pEventAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimEvent );
    while( pEventAtom )
    {
        Any* pEvents = NULL;

        switch( pEventAtom->getInstance() )
        {
        case 1: pEvents = &aBegin; break;
        case 2: pEvents = &aEnd; break;
        case 3: pEvents = &aNext; break;
        case 4: pEvents = &aPrev; break;
        }

        if( pEvents )
        {
            Event aEvent;
            aEvent.Trigger = EventTrigger::NONE;
            aEvent.Repeat = 0;

            const Atom* pChildAtom = pEventAtom->findFirstChildAtom();

            while( pChildAtom && pChildAtom->seekToContent() )
            {
                switch( pChildAtom->getType() )
                {
                case DFF_msofbtAnimTrigger:
                {
                    sal_Int32 nU1, nTrigger, nU3, nBegin;
                    mrStCtrl >> nU1;
                    mrStCtrl >> nTrigger;
                    mrStCtrl >> nU3;
                    mrStCtrl >> nBegin;

                    switch( nTrigger )
                    {
                    case 0: aEvent.Trigger = EventTrigger::NONE; break;
                    case 1: aEvent.Trigger = EventTrigger::ON_BEGIN; break;
                    case 2: aEvent.Trigger = EventTrigger::ON_END; break;
                    case 3: aEvent.Trigger = EventTrigger::BEGIN_EVENT; break;
                    case 4: aEvent.Trigger = EventTrigger::END_EVENT; break;
                    case 5: aEvent.Trigger = EventTrigger::ON_CLICK; break;
                    case 6: aEvent.Trigger = EventTrigger::ON_DBL_CLICK; break;
                    case 7: aEvent.Trigger = EventTrigger::ON_MOUSE_ENTER; break;
                    case 8: aEvent.Trigger = EventTrigger::ON_MOUSE_LEAVE; break;
                    case 9: aEvent.Trigger = EventTrigger::ON_NEXT; break;
                    case 10: aEvent.Trigger = EventTrigger::ON_PREV; break;
                    case 11: aEvent.Trigger = EventTrigger::ON_STOP_AUDIO; break;
                    }

                    if( (nBegin != 0) || (aEvent.Trigger == EventTrigger::NONE) )
                        aEvent.Offset = (nBegin == -1) ? makeAny( Timing_INDEFINITE ) : makeAny( (double)(nBegin / 1000.0) );
                }
                break;
                case DFF_msofbtAnimateTargetElement:
                {
                    sal_Int16 nSubType;
                    importTargetElementContainer( pChildAtom, aEvent.Source, nSubType );
                }
                break;
                default:
                {
                    OSL_FAIL("unknown atom inside ppt::AnimationImporter::importAnimationEvents()!");
                }
                }

                pChildAtom = pEventAtom->findNextChildAtom( pChildAtom );
            }

            *pEvents = addToSequence( *pEvents, (aEvent.Trigger == EventTrigger::NONE) ? aEvent.Offset : makeAny( aEvent ) );
        }

        pEventAtom = pAtom->findNextChildAtom( DFF_msofbtAnimEvent, pEventAtom );
    }

    xNode->setBegin( aBegin );
    xNode->setEnd( aEnd );
    // TODO: xNode->setNext( aNext );
    // TODO: xNode->setPrev( aNext );

#ifdef DBG_ANIM_LOG
    if( aBegin.hasValue() )
    {
        dump( " begin=\"" );
        dump( aBegin );
        dump( "\"" );
    }

    if( aEnd.hasValue() )
    {
        dump( " end=\"" );
        dump( aEnd );
        dump( "\"" );
    }

    if( aNext.hasValue() )
    {
        dump( " next=\"" );
        dump( aNext );
        dump( "\"" );
    }

    if( aPrev.hasValue() )
    {
        dump( " prev=\"" );
        dump( aPrev );
        dump( "\"" );
    }
#endif
}

// --------------------------------------------------------------------

void AnimationImporter::importAnimationActions( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( pAtom && xNode.is(), "invalid call to ppt::AnimationImporter::importAnimationActions()!");

    if( pAtom )
    {
        const Atom* pActionAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimAction );

        if( pActionAtom && pActionAtom->seekToContent() )
        {
            sal_Int32 nConcurrent, nNextAction, nEndSync, nU4, nU5;
            mrStCtrl >> nConcurrent;
            mrStCtrl >> nNextAction;
            mrStCtrl >> nEndSync;
            mrStCtrl >> nU4;
            mrStCtrl >> nU5;

            if( nEndSync == 1 )
                xNode->setEndSync( makeAny( AnimationEndSync::ALL ) );

    #ifdef DBG_ANIM_LOG
            dump( " concurrent=\"%s\"", nConcurrent == 0 ? "disabled" : (nConcurrent == 1 ? "enabled" : "unknown") );

            dump( " nextAction=\"%s\"", nNextAction == 0 ? "none" : (nNextAction == 1 ? "seek" : "unknown") );

            if( nEndSync != 0 )
            {
                dump( " endSync=\"%s\"", nEndSync == 1 ? "all" : "unknown" );
            }

            dump( " action_4=\"%#lx\"", nU4 );
            dump( " action_5=\"%#lx\"", nU5 );
    #endif
        }
    }
}

// --------------------------------------------------------------------

sal_Int32 AnimationImporter::importTargetElementContainer( const Atom* pAtom, Any& rTarget, sal_Int16& rSubType )
{
    rSubType = ShapeAnimationSubType::AS_WHOLE;
    sal_Int32 nRefMode = -1;

    DBG_ASSERT( pAtom && (pAtom->getType() == DFF_msofbtAnimateTargetElement), "invalid call to ppt::AnimationImporter::importTargetElementContainer()!" );
    if( pAtom )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();
        while( pChildAtom && pChildAtom->seekToContent() )
        {
            switch( pChildAtom->getType() )
            {
            case DFF_msofbtAnimReference:
            {
                sal_Int32 nRefType,nRefId;
                sal_Int32 begin,end;
                mrStCtrl >> nRefMode;
                mrStCtrl >> nRefType;
                mrStCtrl >> nRefId;
                mrStCtrl >> begin;
                mrStCtrl >> end;

                switch( nRefType )
                {
                case 1: // shape
                {
                    SdrObject* pSdrObject = mpPPTImport->getShapeForId( nRefId );
                    if( pSdrObject == NULL )
                        break;

                    rTarget <<= pSdrObject->getUnoShape();

                    switch( nRefMode )
                    {
                    case 6: rSubType = ShapeAnimationSubType::ONLY_BACKGROUND; break;
                    case 8: rSubType = ShapeAnimationSubType::ONLY_TEXT; break;
                    case 2: // one paragraph
                    {
                        if( ((begin == -1) && (end == -1)) || !pSdrObject->ISA( SdrTextObj )  )
                            break;

                        SdrTextObj* pTextObj = static_cast< SdrTextObj* >( pSdrObject );

                        const OutlinerParaObject* pOPO = pTextObj->GetOutlinerParaObject();
                        if( pOPO == NULL )
                            break;

                        const EditTextObject& rEditTextObject = pOPO->GetTextObject();

                        const sal_Int32 nParaCount = rEditTextObject.GetParagraphCount();

                        sal_Int32 nPara = 0;

                        while( (nPara < nParaCount) && (begin > 0) )
                        {
                            sal_Int32 nParaLength = rEditTextObject.GetText( nPara ).getLength() + 1;
                            begin -= nParaLength;
                            end -= nParaLength;
                            nPara++;
                        }

                        if( nPara < nParaCount )
                        {
                            ParagraphTarget aParaTarget;
                            rTarget >>= aParaTarget.Shape;
                            /* FIXME: Paragraph should be sal_Int32 as well */
                            aParaTarget.Paragraph = static_cast<sal_Int16>(nPara);
                            rTarget = makeAny( aParaTarget );

                            rSubType = ShapeAnimationSubType::ONLY_TEXT;
                            dump( " paragraph %d,", (sal_Int32)nPara);
                            dump( " %d characters", (sal_Int32)end );
                        }
                    }
                    }
                }
                break;

                case 2: // sound
                    {
                        OUString aSoundURL( ((ImplSdPPTImport*)mpPPTImport)->ReadSound( nRefId ) );
                        rTarget <<= aSoundURL;
                        dump( " srcRef=\"%s\"", aSoundURL );
                    }
                    break;
                case 3: // audio object
                case 4: // video object
                    {
                        SdrObject* pSdrObject = mpPPTImport->getShapeForId( nRefId );
                        if( pSdrObject == NULL )
                            break;

                        rTarget <<= pSdrObject->getUnoShape();
                    }
                    break;
                default:
                    OSL_FAIL("unknown reference type");
                }

            }
            break;
            case 0x2b01:
            {
                sal_Int32 nU1;
                mrStCtrl >> nU1;
            }
            break;
            default:
                OSL_FAIL("unknown atom inside ppt::AnimationImporter::importTargetElementContainer()!");
                break;
            }

        pChildAtom = pAtom->findNextChildAtom( pChildAtom );

        }
    }

    return nRefMode;
}

// --------------------------------------------------------------------

void AnimationImporter::importPropertySetContainer( const Atom* pAtom, PropertySet& rSet )
{
    DBG_ASSERT( pAtom && (pAtom->getType() == DFF_msofbtAnimPropertySet), "invalid call to ppt::AnimationImporter::importPropertySetContainer()!" );

    if( pAtom )
    {
        const Atom* pChildAtom = pAtom->findFirstChildAtom();
        while( pChildAtom )
        {
            if( pChildAtom->getType() == DFF_msofbtAnimAttributeValue )
            {
                Any aAny;
                importAttributeValue( pChildAtom, aAny );
                rSet.maProperties[ pChildAtom->getInstance() ] = aAny;
            }
            else
            {
                OSL_FAIL("unknown atom inside ppt::AnimationImporter::importPropertySetContainer()!");
            }

            pChildAtom = pAtom->findNextChildAtom( pChildAtom );
        }
    }
}

// ====================================================================

#ifdef DBG_ANIM_LOG
void AnimationImporter::dump_atom_header( const Atom* pAtom, bool bOpen, bool bAppend )
{
    if( pAtom )
    {
        const char* pTitle;

        switch( pAtom->getType() )
        {
        case DFF_msofbtAnimEvent: pTitle = "AnimEvent"; break;
        case DFF_msofbtAnimTrigger: pTitle = "AnimTrigger"; break;
        case DFF_msofbtAnimateMotion:   pTitle = "AnimateMotion"; break;
        case DFF_msofbtAnimPropertySet: pTitle = "AnimPropertySet"; break;
        case DFF_msofbtAnimateAttributeNames: pTitle = "AnimAttributeName"; break;
        case DFF_msofbtAnimAttributeValue: pTitle = "AnimAttributeValue"; break;
        case DFF_msofbtAnimGroup: pTitle = "AnimGroup"; break;
        case DFF_msofbtAnimNode: pTitle = "AnimNode"; break;
        case DFF_msofbtAnimValue: pTitle = "AnimValue"; break;
        case DFF_msofbtAnimateFilter: pTitle = "animateFilter"; break;
        case DFF_msofbtAnimate: pTitle = "animate"; break;
        case DFF_msofbtAnimateSet: pTitle = "set"; break;
        case DFF_msofbtAnimKeyTime: pTitle = "AnimKeyTime"; break;
        case DFF_msofbtAnimKeyPoints: pTitle = "AnimKeyPoints"; break;
        case DFF_msofbtAnimReference: pTitle = "AnimReference"; break;
        case DFF_msofbtAnimateTargetElement: pTitle = "AnimTargetElementContainer"; break;
        case DFF_msofbtAnimAction: pTitle = "AnimAction"; break;
        case DFF_msofbtAnimCommand: pTitle = "AnimCommand"; break;
        case DFF_msofbtAnimateTarget: pTitle = "TransformationTarget"; break;
        case DFF_msofbtAnimateTargetSettings: pTitle = "TransformationTargetSettings"; break;
        case DFF_msofbtAnimIteration: pTitle = "iterate"; break;
        case DFF_msofbtAnimateColorData: pTitle = "colorData"; break;
        case DFF_msofbtAnimateScaleData: pTitle = "scaleData"; break;
        case DFF_msofbtAnimateSetData: pTitle = "setData"; break;

        default:
            {
                static char buffer[128];
                sprintf( buffer, "unknown_%#x", pAtom->getType() );
                pTitle = buffer;
            }
        }

        if( bOpen )
        {
            fprintf(mpFile, "<%s", pTitle );

            fprintf(mpFile, " instance=\"%hu\"%s",
                        pAtom->getInstance(),
                        bAppend ? "" : ">\n");
        }
        else
        {
            if( bAppend )
                fprintf(mpFile,"/>\n");
            else
                fprintf(mpFile, "</%s>\n", pTitle );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::dump( sal_uInt32 nLen, bool bNewLine )
{
    char * faul = "0123456789abcdef";

    sal_uInt32 i = 0;
    int b = 0;
    sal_Int8 nData;

    for( i = 0; i < nLen; i++ )
    {
        mrStCtrl >> nData;

        fprintf( mpFile, "%c%c ", faul[ (nData >> 4) & 0x0f ], faul[ nData & 0x0f ] );

        b++;
        if( bNewLine && (b == 32) )
        {
            fprintf(mpFile,"\n");
            b = 0;
        }
    }
    if( (b != 0) && bNewLine )
        fprintf(mpFile,"\n");
}

// --------------------------------------------------------------------

void AnimationImporter::dump_atom( const Atom* pAtom, bool bNewLine )
{
    if( pAtom )
    {
        if( pAtom->isContainer() )
        {
            const Atom* pChildAtom = pAtom->findFirstChildAtom();
            while( pChildAtom )
            {
                if( pChildAtom->getType() == DFF_msofbtAnimAttributeValue )
                {
                    fprintf(mpFile, "<attributeValue instance=\"%hu\"", pChildAtom->getInstance() );

                    Any aValue;
                    if( importAttributeValue( pChildAtom, aValue ) )
                    {
                        sal_Int32 nInt;
                        OUString aString;
                        double fDouble;

                        if( aValue >>= nInt )
                        {
                            fprintf(mpFile, " value=\"%ld\"", nInt );
                        }
                        else if( aValue >>= aString )
                        {
                            fprintf(mpFile, " value=\"%s\"",
                                OUStringToOString(aString,
                                    RTL_TEXTENCODING_UTF8).getStr());
                        }
                        else if( aValue >>= fDouble )
                        {
                            fprintf(mpFile, " value=\"%g\"", fDouble );
                        }
                    }
                    else
                    {
                        if( pChildAtom->seekToContent() )
                        {
                            fprintf(mpFile, " value=\""  );
                            dump_atom( pChildAtom, false );
                            fprintf(mpFile, "\"");
                        }
                    }

                    fprintf(mpFile, "/>\n" );
                }
                else
                {
                    dump_atom_header( pChildAtom, true, pChildAtom->getType() == DFF_msofbtAnimAttributeValue );
                    dump_atom( pChildAtom );
                    dump_atom_header( pChildAtom, false, pChildAtom->getType() == DFF_msofbtAnimAttributeValue );
                }

                pChildAtom = pAtom->findNextChildAtom(pChildAtom);
            }
        }
        else if( pAtom->seekToContent() )
        {
            dump( pAtom->getLength(), bNewLine );
        }
    }
}

// --------------------------------------------------------------------

void AnimationImporter::dump_anim_group( const Atom* pAtom, const AnimationNode& rNode, const PropertySet& rSet, bool bOpen )
{
    fprintf( mpFile, bOpen ? "<" : "</" );

    switch( rNode.mnGroupType )
    {
    case mso_Anim_GroupType_PAR:
        fprintf( mpFile, "par" );
        break;
    case mso_Anim_GroupType_SEQ:
        fprintf( mpFile, "seq" );
        break;
    case mso_Anim_GroupType_NODE:
        switch( rNode.mnNodeType )
        {
        case mso_Anim_Behaviour_FILTER:
            fprintf( mpFile, "animateFilter" );
            break;
        case mso_Anim_Behaviour_ANIMATION:
            if( pAtom->hasChildAtom( DFF_msofbtAnimateSet ) )
                fprintf( mpFile, "set" );
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateColor ) )
                fprintf( mpFile, "animateColor" );
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateScale ) )
                fprintf( mpFile, "animateScale" );
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateRotation ) )
                fprintf( mpFile, "animateRotation" );
            else if( pAtom->hasChildAtom( DFF_msofbtAnimateMotion ) )
                fprintf( mpFile, "animateMotion" );
            else if( pAtom->hasChildAtom( DFF_msofbtAnimCommand ) )
                fprintf( mpFile, "command" );
            else
                fprintf( mpFile, "animation" );
            break;
        default:
            {
                fprintf( mpFile, "unknown_node_%#lx", rNode.mnNodeType );
            }
            break;
        }
        break;
    case mso_Anim_GroupType_MEDIA:
        fprintf( mpFile, "media" );
        break;
    default:
        fprintf( mpFile, "unknown_group_%#lx", rNode.mnGroupType );
        break;
    }

    if( bOpen )
    {
        dump( rNode );
        dump( rSet );
    }

    fprintf(mpFile,">\n");
}

void AnimationImporter::dump( const AnimationNode& rNode )
{
    // dump animation node
    if( rNode.mnRestart != 0 )
    {
        fprintf(mpFile," restart=\"%s\"",
            rNode.mnRestart == 1 ? "always" : (rNode.mnRestart == 2 ? "whenOff" : (rNode.mnRestart == 3 ? "never" : "unknown")) );
    }

    if( rNode.mnFill )
    {
        fprintf(mpFile," fill=\"%s\"",
            rNode.mnFill == 1 ? "remove" : (rNode.mnFill == 3 ? "hold" : (rNode.mnFill == 2 ? "freeze" : "unknown")) );
    }

    if( rNode.mnDuration > 0 )
    {
        double fSeconds = rNode.mnDuration;
        fSeconds /= 1000.0;
        fprintf(mpFile, " dur=\"%g\"", fSeconds);
    }
    else if( rNode.mnDuration < 0 )
    {
        fprintf(mpFile, " dur=\"indefinite\"" );
    }

    if( rNode.mnU1 ) fprintf(mpFile," u1=\"%#lx\"", rNode.mnU1);
    if( rNode.mnU3 ) fprintf(mpFile," u3=\"%#lx\"", rNode.mnU3);
    if( rNode.mnU4 ) fprintf(mpFile," u4=\"%#lx\"", rNode.mnU4);
}

void AnimationImporter::dump( Any& rAny )
{
    Sequence< Any > aSeq;
    sal_Int32 nInt;
    double fDouble;
    OUString aString;
    sal_Bool bBool;
    Event aEvent;
    Timing aTiming;

    if( rAny >>= aSeq )
    {
        const sal_Int32 nSize = aSeq.getLength();
        sal_Int32 nIndex = 0;
        while( nIndex < nSize )
        {
            dump( aSeq[nIndex++] );
            if(nIndex < nSize)
                fprintf( mpFile, "," );
        }
    }
    else if( rAny >>= aString )
    {
        fprintf( mpFile, "%s", OUStringToOString(aString,
            RTL_TEXTENCODING_UTF8).getStr() );
    }
    else if( rAny >>= nInt )
    {
        fprintf( mpFile, "%ld", nInt );
    }
    else if( rAny >>= bBool )
    {
        fprintf( mpFile, "%s", bBool ? "true" : "false" );
    }
    else if( rAny >>= fDouble )
    {
        fprintf( mpFile, "%g", fDouble );
    }
    else if( rAny >>= aTiming )
    {
        fprintf( mpFile, "%s", aTiming == (Timing_INDEFINITE) ? "indefinite" : "media" );
    }
    else if( rAny >>= aEvent )
    {
        static const char* triggers[] =
        {
            "none","onbegin","onend","begin",
            "end","onclick","ondoubleclick","onmouseenter",
            "onmouseleave","onpptnext","onpptprev","onstopaudio"
        };

        if( aEvent.Trigger != EventTrigger::NONE )
        {
            if( aEvent.Source.hasValue() )
            {
                dump_target( aEvent.Source );
                dump( "." );
            }

            dump( triggers[ aEvent.Trigger ] );
        }

        if( aEvent.Offset.hasValue() )
        {
            double fOffset;
            if( aEvent.Offset >>= fOffset )
                fprintf( mpFile, "%g", fOffset );
            else
                dump( "indefinite" );
        }
    }
}

void AnimationImporter::dump( const PropertySet& rSet )
{
    // dump property set

    map< sal_Int32, Any >::const_iterator aIter( rSet.maProperties.begin() );
    const map< sal_Int32, Any >::const_iterator aEnd( rSet.maProperties.end() );
    while( aIter != aEnd )
    {
        bool bKnown = false;

        const sal_Int32 nInstance = (*aIter).first;
        Any aAny( (*aIter).second );

        switch ( nInstance )
        {
        case DFF_ANIM_COLORSPACE:
        {
            sal_Int32 nColorSpace;
            if( aAny >>= nColorSpace )
            {
                fprintf( mpFile, " colorSpace=\"%s\"", (nColorSpace == 0) ? "rgb" : (nColorSpace == 1) ? "hsl" : "unknown" );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_DIRECTION:
        {
            sal_Bool bDirection;
            if( aAny >>= bDirection )
            {
                fprintf( mpFile, " direction=\"%s\"", bDirection ? "cclockwise" : "clockwise"  );
                bKnown = true;
            }
            else
            {
                sal_Int32 nMasterRel;
                if( aAny >>= nMasterRel )
                {
                    fprintf( mpFile, " direction=\"%s\"", nMasterRel == 0 ? "sameClick" : ( nMasterRel == 2 ? "nextClick" : "lastClick" )  );
                    bKnown = true;
                }
            }
        }
        break;

        case DFF_ANIM_OVERRIDE:     // TODO
        {
            sal_Int32 nOverride;
            if( aAny >>= nOverride )
            {
                fprintf( mpFile, " override=\"%s\"", (nOverride == 1) ? "childStyle" : (nOverride == 0) ? "normal" : "unknown" );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_PATH_EDIT_MODE:
        {
            sal_Bool bPathEditMode;
            if( aAny >>= bPathEditMode )
            {
                fprintf( mpFile, " pptPathEditMode=\"%s\"", bPathEditMode ? "relative" : "fixed" );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_PRESET_ID :
        {
            sal_Int32 nPresetId ;
            if( aAny >>= nPresetId )
            {
                fprintf(mpFile, " presetid=\"%ld\"", nPresetId );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_PRESET_SUB_TYPE :
        {
            sal_Int32 nPointsType ;
            if( aAny >>= nPointsType )
            {
                fprintf(mpFile, " presetSubType=\"%ld\"", nPointsType );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_PRESET_CLASS :
        {
            sal_Int32 nPresetClass;
            if ( aAny >>= nPresetClass )
            {
                const char* pMode;
                switch( nPresetClass )
                {
                case DFF_ANIM_PRESS_CLASS_USER_DEFINED:     pMode = "userdefined"; break;
                case DFF_ANIM_PRESS_CLASS_ENTRANCE:         pMode = "entrance"; break;
                case DFF_ANIM_PRESS_CLASS_EXIT:             pMode = "exit"; break;
                case DFF_ANIM_PRESS_CLASS_EMPHASIS:         pMode = "emphasis"; break;
                case DFF_ANIM_PRESS_CLASS_MOTIONPATH:       pMode = "motionpath"; break;
                case DFF_ANIM_PRESS_CLASS_OLE_ACTION:       pMode = "oleaction"; break;
                case DFF_ANIM_PRESS_CLASS_MEDIACALL:        pMode = "mediacall"; break;
                default:
                {
                    static char buffer[128];
                    sprintf( buffer, "%ld", nPresetClass );
                    pMode = buffer;
                }
                break;
                }

                fprintf(mpFile, " class=\"%s\"", pMode);
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_NODE_TYPE :
        {
            sal_Int32 nNodeType;
            if ( aAny >>= nNodeType )
            {
                const char* pNode;
                switch( nNodeType )
                {
                    case DFF_ANIM_NODE_TYPE_ON_CLICK:       pNode = "onclick";  break;
                    case DFF_ANIM_NODE_TYPE_WITH_PREVIOUS:  pNode = "withprevious"; break;
                    case DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS: pNode = "afterprevious"; break;
                    case DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE:  pNode = "mainsequence"; break;
                    case DFF_ANIM_NODE_TYPE_TIMING_ROOT:    pNode = "timingroot"; break;
                    case DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ:pNode = "interactivesequence"; break;
                    default :
                    {
                        static char buffer[128];
                        sprintf( buffer, "%ld", nNodeType );
                        pNode = buffer;
                    }
                    break;
                }

                fprintf(mpFile, " nodeType=\"%s\"", pNode);
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_GROUP_ID:
        {
            sal_Int32 nGroupId;
            if ( aAny >>= nGroupId )
            {
                fprintf( mpFile, " groupId=\"%ld\"", nGroupId );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_ID:
        {
            OUString aString;
            if( aAny >>= aString )
            {
                fprintf( mpFile, " id=\"%s\"",
                    OUStringToOString(aString,
                        RTL_TEXTENCODING_UTF8).getStr() );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_EVENT_FILTER:
        {
            OUString aString;
            if( aAny >>= aString )
            {
                fprintf( mpFile, " eventFilter=\"%s\"",
                    OUStringToOString(aString,
                        RTL_TEXTENCODING_UTF8).getStr() );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_ENDAFTERSLIDE:
        {
            sal_Int32 nEndAfterSlide;
            if( aAny >>= nEndAfterSlide )
            {
                fprintf(mpFile, " endAfterSlide=\"%ld\"", nEndAfterSlide );
            bKnown = true;
            }
        }

        case DFF_ANIM_TIMEFILTER:
        {
            OUString aString;
            if( aAny >>= aString )
            {
                fprintf( mpFile, " timeFilter=\"%s\"",
                    OUStringToOString(aString,
                        RTL_TEXTENCODING_UTF8).getStr() );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_RUNTIMECONTEXT:
        {
            OUString aString;
            if( aAny >>= aString )
            {
                fprintf( mpFile, " runtimeContext=\"%s\"",
                    OUStringToOString(aString,
                        RTL_TEXTENCODING_UTF8).getStr() );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_VOLUME:
        {
            double fVolume(0.0);
            if( aAny >>= fVolume )
            {
                fprintf( mpFile, " volume=\"%g%%\"", (double)(fVolume * 100.0) );
                bKnown = true;
            }
        }
        break;

        case DFF_ANIM_AFTEREFFECT:
        {
            sal_Bool bAfterEffect;
            if( aAny >>= bAfterEffect )
            {
                fprintf( mpFile, "afterEffect=\"%s\"", bAfterEffect ? "true" : "false" );
                bKnown = true;
            }
        }
        break;

        }


        if( !bKnown )
        {
            fprintf( mpFile, " unknown_%lu=\"", nInstance );
            dump( aAny );
            fprintf( mpFile, "\"" );
        }

        ++aIter;
    }
}

void AnimationImporter::dump_target( Any& rAny )
{
    Any aSource, aSourceData;
    Sequence< Any > aSeq;
    if( rAny >>= aSeq )
    {
        if( aSeq.getLength() >= 1 ) aSource = aSeq[0];
        if( aSeq.getLength() >= 2 ) aSourceData = aSeq[1];
    }
    else
    {
        aSource = rAny;
    }

    Reference< XShape > xShape;
    aSource >>= xShape;
    if( xShape.is() )
    {
        OUString aStr( xShape->getShapeType() );
        dump( aStr );

        if( aSourceData.hasValue() )
        {
            dump( "(" );
            dump( aSourceData );
            dump( ")" );
        }
    }
}

void AnimationImporter::dump( const char * pText )
{
    fprintf( mpFile, "%s", pText );
}

void AnimationImporter::dump( const OUString& rString )
{
    fprintf( mpFile, OUStringToOString(rString,
        RTL_TEXTENCODING_UTF8).getStr() );
}

void AnimationImporter::dump( const char * pText, sal_Int64 nInt )
{
    fprintf( mpFile, pText, nInt );
}

void AnimationImporter::dump( const char * pText, sal_Int32 nInt )
{
    fprintf( mpFile, pText, nInt );
}

void AnimationImporter::dump( const char * pText, double fDouble )
{
    fprintf( mpFile, pText, fDouble );
}

void AnimationImporter::dump( const char * pText, const char * pText2 )
{
    fprintf( mpFile, pText, pText2 );
}

void AnimationImporter::dump( const char * pText, const OUString& rString )
{
    fprintf( mpFile, pText, OUStringToOString(rString,
        RTL_TEXTENCODING_UTF8).getStr() );
}

#else

void AnimationImporter::dump_atom_header( const Atom* , bool , bool  )
{
}

void AnimationImporter::dump_atom( const Atom* , bool  )
{
}

void AnimationImporter::dump_target( ::com::sun::star::uno::Any&  )
{
}

void AnimationImporter::dump( ::com::sun::star::uno::Any&  )
{
}

void AnimationImporter::dump( const PropertySet&  )
{
}

void AnimationImporter::dump( const AnimationNode&  )
{
}

void AnimationImporter::dump( const char *  )
{
}

void AnimationImporter::dump( const char * , sal_Int32  )
{
}

void AnimationImporter::dump( const char * , double  )
{
}

void AnimationImporter::dump( const char * , const char *  )
{
}

void AnimationImporter::dump( const char * , const OUString&  )
{
}

#endif

} // namespace ppt;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
