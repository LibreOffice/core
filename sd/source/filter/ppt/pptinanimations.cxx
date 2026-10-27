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
#include <xmloff/SoundReference.hxx>
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
#include <com/sun/star/animations/XAnimationNode.hpp>
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
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <comphelper/processfactory.hxx>
#include <oox/helper/addtosequence.hxx>
#include <oox/ppt/pptfilterhelpers.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>

#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <animations.hxx>
#include <drawdoc.hxx>
#include "pptanimations.hxx"
#include "pptinanimations.hxx"
#include "pptatom.hxx"
#include "pptin.hxx"
#include <randomnode.hxx>

#include <algorithm>
#include <memory>

using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

namespace ppt
{

static SvStream& operator>>(SvStream& rIn, AnimationNode& rNode )
{
    rIn.ReadInt32( rNode.mnU1 );
    rIn.ReadInt32( rNode.mnRestart );
    rIn.ReadInt32( rNode.mnGroupType );
    rIn.ReadInt32( rNode.mnFill );
    rIn.ReadInt32( rNode.mnU3 );
    rIn.ReadInt32( rNode.mnU4 );
    rIn.ReadInt32( rNode.mnDuration );
    rIn.ReadInt32( rNode.mnNodeType );

    return rIn;
}

bool PropertySet::hasProperty( sal_Int32 nProperty ) const
{
    return maProperties.find( nProperty ) != maProperties.end();
}

Any PropertySet::getProperty( sal_Int32 nProperty ) const
{
    PropertySetMap_t::const_iterator aIter( maProperties.find( nProperty ) );
    if( aIter != maProperties.end() )
        return (*aIter).second;
    else
        return Any();
}

AnimationImporter::AnimationImporter( ImplSdPPTImport* pPPTImport, SvStream& rStCtrl )
: mpPPTImport( pPPTImport ), mrStCtrl( rStCtrl )
{
}

int AnimationImporter::import( const Reference< XDrawPage >& xPage, const DffRecordHeader& rProgTagContentHd )
{
    int nNodes = 0;

    Reference< XAnimationNodeSupplier > xNodeSupplier( xPage, UNO_QUERY );
    if( xNodeSupplier.is() )
    {
        mxRootNode = xNodeSupplier->getAnimationNode();
        if( mxRootNode.is() )
        {
            Reference< XAnimationNode > xParent;

            std::unique_ptr<Atom> pAtom(Atom::import( rProgTagContentHd, mrStCtrl ));
            if( pAtom )
            {
                nNodes = importAnimationContainer( pAtom.get(), xParent );
            }

            std::for_each( maAfterEffectNodes.begin(), maAfterEffectNodes.end(),
                           sd::stl_process_after_effect_node_func );
        }
    }

    return nNodes;
}

Reference< XAnimationNode > AnimationImporter::createNode( const Atom* pAtom, const AnimationNode& rNode )
{
    const char* pServiceName = nullptr;

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
        const Reference< XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
        const OUString aServiceName( OUString::createFromAscii(pServiceName) );
        Reference< XInterface > xFac( xContext->getServiceManager()->createInstanceWithContext(aServiceName, xContext) );
        xNode.set(xFac , UNO_QUERY );
    }

    DBG_ASSERT( xNode.is(), "sd::AnimationImporter::createNode(), node creation failed!" );
    return xNode;
}

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
                xNode.set( sd::RandomAnimationNode_createInstance( static_cast<sal_Int16>(nPresetClass) ), UNO_QUERY );
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

        // import if we have a node and it's not random
        if( xNode.is() )
        {
            fillNode( xNode, aNode, aSet );

            switch( aNode.mnGroupType )
            {
            case mso_Anim_GroupType_PAR:
            {
                nNodes += importTimeContainer( pAtom, xNode );

                // for iteration containers, map target from children to iteration
                Reference< XIterateContainer > xIter( xNode, UNO_QUERY );
                if( xIter.is() )
                {
                    double fDuration = 0.0;
                    Any aTarget, aEmpty;
                    Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
                    if( xEnumerationAccess.is() )
                    {
                        Reference< XEnumeration > xEnumeration = xEnumerationAccess->createEnumeration();
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
                nNodes += importTimeContainer( pAtom, xNode );

                if( aSet.hasProperty( DFF_ANIM_NODE_TYPE ) )
                {
                    sal_Int32 nPPTNodeType = 0;
                    if( aSet.getProperty( DFF_ANIM_NODE_TYPE ) >>= nPPTNodeType )
                    {
                        switch(nPPTNodeType)
                        {
                        case DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE:
                            oox::ppt::fixMainSequenceTiming( xNode );
                            break;
                        case DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ:
                            oox::ppt::fixInteractiveSequenceTiming( xNode );
                            break;
                        }
                    }
                }
            }
            break;

            case mso_Anim_GroupType_NODE:
            {
                int nANCNodes = importAnimationNodeContainer( pAtom, xNode );
                if( !convertAnimationNode( xNode, xParent ) )
                    xNode = nullptr;
                else
                    nNodes += nANCNodes;

            }
            break;

            case mso_Anim_GroupType_MEDIA:
            {
                nNodes += importAudioContainer( pAtom, xNode );
            }
            break;

            default:
                OSL_FAIL( "unknown group atom!" );
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

    const oox::ppt::ImplAttributeNameConversion* p = oox::ppt::getAttributeConversionList();

    oox::ppt::AnimationAttributeEnum eAttribute = oox::ppt::AnimationAttributeEnum::UNKNOWN;

    if( (nNodeType == AnimationNodeType::ANIMATEMOTION) ||
        (nNodeType == AnimationNodeType::ANIMATETRANSFORM) )
    {
        aAttributeName.clear();
    }
    else
    {
        while( p->mpMSName )
        {
            if( aAttributeName.equalsAscii( p->mpMSName ) )
                break;

            p++;
        }

        DBG_ASSERT( p->mpMSName || aAttributeName.isEmpty(), "sd::AnimationImporter::convertAnimationNode(), unknown attribute!" );

        eAttribute = p->meAttribute;

        if( p->mpAPIName )
            aAttributeName = OUString::createFromAscii( p->mpAPIName );
    }

    xAnimate->setAttributeName( aAttributeName );

    if(eAttribute != oox::ppt::AnimationAttributeEnum::UNKNOWN)
    {
        Any aAny( xAnimate->getFrom() );
        if( aAny.hasValue() )
        {
            if(oox::ppt::convertAnimationValue(eAttribute, aAny))
                xAnimate->setFrom( aAny );
        }

        aAny = xAnimate->getBy();
        if( aAny.hasValue() )
        {
            if(oox::ppt::convertAnimationValue(eAttribute, aAny))
                xAnimate->setBy( aAny );
        }

        aAny = xAnimate->getTo();
        if( aAny.hasValue() )
        {
            if(oox::ppt::convertAnimationValue(eAttribute, aAny))
                xAnimate->setTo( aAny );
        }

        Sequence< Any > aValues( xAnimate->getValues() );
        if( aValues.hasElements() )
        {
            for( Any& rValue : asNonConstRange(aValues) )
                oox::ppt::convertAnimationValue(eAttribute, rValue);

            xAnimate->setValues( aValues );
        }

        OUString aFormula( xAnimate->getFormula() );
        if( !aFormula.isEmpty() )
        {
            if(oox::ppt::convertMeasure(aFormula))
                xAnimate->setFormula( aFormula );
        }
    }

    // check for after-effect
    Sequence< NamedValue > aUserData( xNode->getUserData() );
    NamedValue* pLastValue = aUserData.getArray();
    sal_Int32 nRemoved = 0;

    bool bAfterEffect = false;
    sal_Int32 nMasterRel = 0;
    for (const NamedValue& rValue : aUserData)
    {
        if ( rValue.Name == "after-effect" )
        {
            rValue.Value >>= bAfterEffect;
            nRemoved++;
        }
        else if ( rValue.Name == "master-rel" )
        {
            rValue.Value >>= nMasterRel;
            nRemoved++;
        }
        else
        {
            if( nRemoved )
                *pLastValue = rValue;
            pLastValue++;
        }
    }

    if( nRemoved )
    {
        aUserData.realloc( aUserData.getLength() - nRemoved );
        xNode->setUserData( aUserData );
    }

    // if it's an after effect node, add it to the list for
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

            xNode->setBegin( Any( aEvent ) );
        }

        // add to after effect nodes for later processing
        sd::AfterEffectNode aNode( xNode, xParent, nMasterRel == 2 );
        maAfterEffectNodes.push_back( aNode );
        return false;
    }

    return true;
}

void AnimationImporter::fillNode( Reference< XAnimationNode > const & xNode, const AnimationNode& rNode, const PropertySet& rSet )
{
    bool bAfterEffect = false;

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
            aDuration <<= rNode.mnDuration / 1000.0;
        }
        else if( rNode.mnDuration < 0 )
        {
            aDuration <<= Timing_INDEFINITE;
        }
        xNode->setDuration( aDuration );
    }

    // TODO: DFF_ANIM_PATH_EDIT_MODE

    // set user data
    Sequence< NamedValue > aUserData;

    // attribute Type
    if( rSet.hasProperty( DFF_ANIM_NODE_TYPE ) )
    {
        sal_Int32 nPPTNodeType = 0;
        if( rSet.getProperty( DFF_ANIM_NODE_TYPE ) >>= nPPTNodeType )
        {
            sal_Int16 nNodeType = css::presentation::EffectNodeType::DEFAULT;
            switch( nPPTNodeType )
            {
                case DFF_ANIM_NODE_TYPE_CLICK_PARALLEL: [[fallthrough]];
                case DFF_ANIM_NODE_TYPE_ON_CLICK:       nNodeType = css::presentation::EffectNodeType::ON_CLICK;   break;
                case DFF_ANIM_NODE_TYPE_WITH_GROUP:     [[fallthrough]];
                case DFF_ANIM_NODE_TYPE_WITH_PREVIOUS:  nNodeType = css::presentation::EffectNodeType::WITH_PREVIOUS; break;
                case DFF_ANIM_NODE_TYPE_AFTER_GROUP:    [[fallthrough]];
                case DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS: nNodeType = css::presentation::EffectNodeType::AFTER_PREVIOUS; break;
                case DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE:  nNodeType = css::presentation::EffectNodeType::MAIN_SEQUENCE; break;
                case DFF_ANIM_NODE_TYPE_TIMING_ROOT:    nNodeType = css::presentation::EffectNodeType::TIMING_ROOT; break;
                case DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ:nNodeType = css::presentation::EffectNodeType::INTERACTIVE_SEQUENCE; break;
            }

            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "node-type";
            pUserData[nSize].Value <<= nNodeType;
        }
    }

    if( rSet.hasProperty( DFF_ANIM_GROUP_ID ) )
    {
        sal_Int32 nGroupId;
        if( rSet.getProperty( DFF_ANIM_GROUP_ID ) >>= nGroupId )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "group-id";
            pUserData[nSize].Value <<= nGroupId;
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
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "preset-class";
            pUserData[nSize].Value <<= nEffectPresetClass;
        }
    }

    if( rSet.hasProperty( DFF_ANIM_PRESET_ID ) )
    {
        if( rSet.getProperty( DFF_ANIM_PRESET_ID ) >>= nPresetId )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "preset-id";

            const oox::ppt::preset_mapping* p = oox::ppt::preset_mapping::getList();
            while( p->mpStrPresetId && ((p->mnPresetClass != nEffectPresetClass) || (p->mnPresetId != nPresetId )) )
                p++;

            if( p->mpStrPresetId )
            {
                pUserData[nSize].Value <<= OUString::createFromAscii( p->mpStrPresetId );
            }
            else
            {
                OUStringBuffer sBuffer( "ppt_" );
                switch( nEffectPresetClass )
                {
                case EffectPresetClass::ENTRANCE: sBuffer.append( "entrance_" ); break;
                case EffectPresetClass::EXIT: sBuffer.append( "exit_" ); break;
                case EffectPresetClass::EMPHASIS: sBuffer.append( "emphasis_" ); break;
                case EffectPresetClass::MOTIONPATH: sBuffer.append( "motionpath_" ); break;
                case EffectPresetClass::OLEACTION: sBuffer.append( "oleaction_" ); break;
                case EffectPresetClass::MEDIACALL: sBuffer.append( "mediacall_" ); break;
                }
                sBuffer.append( nPresetId );

                pUserData[nSize].Value <<= sBuffer.makeStringAndClear();
            }
        }
    }

    if( rSet.hasProperty( DFF_ANIM_PRESET_SUB_TYPE ) )
    {
        sal_Int32 nPresetSubType = 0;
        if( rSet.getProperty( DFF_ANIM_PRESET_SUB_TYPE ) >>= nPresetSubType )
        {
            if( nPresetSubType )
            {
                sal_Int32 nSize = aUserData.getLength();
                aUserData.realloc(nSize+1);
                auto pUserData = aUserData.getArray();
                pUserData[nSize].Name = "preset-sub-type";
                pUserData[nSize].Value <<= oox::ppt::getConvertedSubType( nEffectPresetClass, nPresetId, nPresetSubType );
            }
        }
    }

    if( rSet.hasProperty( DFF_ANIM_AFTEREFFECT ) )
    {
        if( rSet.getProperty( DFF_ANIM_AFTEREFFECT ) >>= bAfterEffect )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "after-effect";
            pUserData[nSize].Value <<= bAfterEffect;
        }
    }

    if( bAfterEffect && rSet.hasProperty( DFF_ANIM_MASTERREL ) )
    {
        sal_Int32 nMasterRel = 2;
        if( rSet.getProperty( DFF_ANIM_MASTERREL ) >>= nMasterRel )
        {
            sal_Int32 nSize = aUserData.getLength();
            aUserData.realloc(nSize+1);
            auto pUserData = aUserData.getArray();
            pUserData[nSize].Name = "master-rel";
            pUserData[nSize].Value <<= nMasterRel;
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
                    fromIndex = aString.indexOf( ';', fromIndex );
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
                    const std::u16string_view aToken( o3tl::getToken(aString, 0, ';', nIndex ) );

                    size_t nPos = aToken.find( ',' );
                    if( nPos != std::u16string_view::npos )
                    {
                        pValues->Time = o3tl::toDouble(aToken.substr( 0, nPos ));
                        pValues->Progress = o3tl::toDouble(aToken.substr( nPos+1 ));
                    }
                    pValues++;
                }

                xAnim->setTimeFilter( aTimeFilter );
            }
        }
    }

// TODO: DFF_ANIM_ENDAFTERSLIDE / DFF_ANIM_VOLUME handling. git history has sample code
    Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
    if( !xColor.is() )
        return;

    if( rSet.hasProperty( DFF_ANIM_DIRECTION ) )
    {
        bool bDirection = false;
        if( rSet.getProperty( DFF_ANIM_DIRECTION ) >>= bDirection )
            xColor->setDirection( !bDirection );
    }

    if( rSet.hasProperty( DFF_ANIM_COLORSPACE ) )
    {
        sal_Int32 nColorSpace = 0;
        rSet.getProperty( DFF_ANIM_COLORSPACE ) >>= nColorSpace;
        xColor->setColorInterpolation( (nColorSpace == 0) ? AnimationColorSpace::RGB : AnimationColorSpace::HSL );
    }
}

int AnimationImporter::importTimeContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    int nNodes = 0;

    DBG_ASSERT( pAtom && xNode.is(), "invalid call to ppt::AnimationImporter::importTimeContainer()!");
    if( pAtom && xNode.is() )
    {
        importAnimationEvents( pAtom, xNode );
        importAnimationValues( pAtom, xNode );
        importAnimationActions( pAtom, xNode );

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
                        const Reference< XComponentContext >& xContext = ::comphelper::getProcessComponentContext();
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
                        float fInterval(0.0);
                        sal_Int32 nTextUnitEffect(0), nU1(0), nU2(0), nU3(0);

                        mrStCtrl.ReadFloat( fInterval ).ReadInt32( nTextUnitEffect ).ReadInt32( nU1 ).ReadInt32( nU2 ).ReadInt32( nU3 );

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
                            xIter->setIterateInterval( static_cast<double>(fInterval) );
                        }

                        nNodes++;
                    }
                }
                break;
            }

            pChildAtom = Atom::findNextChildAtom( pChildAtom );
        }
    }

    return nNodes;
}

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
                }
                break;
            }

            pChildAtom = Atom::findNextChildAtom( pChildAtom );
        }
    }

    return nNodes;
}

void AnimationImporter::importAnimateFilterContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XTransitionFilter > xFilter( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateFilter && xFilter.is(), "invalid call to ppt::AnimationImporter::importAnimateFilterContainer()!");
    if( !(pAtom && xFilter.is()) )
        return;

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
            sal_uInt32 transition(0);
            mrStCtrl.ReadUInt32( nBits );
            mrStCtrl.ReadUInt32( transition );

            if( nBits & 1 )
                xFilter->setMode( transition == 0 );
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

                    const oox::ppt::transition* pTransition = oox::ppt::transition::find( filter );
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
            break;

        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

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
            }
            break;

            case DFF_msofbtAnimateTargetSettings:
            {
                if( xAnimate.is() )
                {
                    sal_uInt32 nBits(0);
                    sal_uInt32 nAdditive(0);
                    sal_uInt32 nAccumulate(0);
                    sal_uInt32 nTransformType(0);

                    mrStCtrl.ReadUInt32( nBits ).ReadUInt32( nAdditive ).ReadUInt32( nAccumulate ).ReadUInt32( nTransformType );

                    // nBits %0001: additive, %0010: accumulate, %0100: attributeName, %1000: transformtype
                    // nAdditive 0 = base, 1 = sum, 2 = replace, 3 = multiply, 4 = none
                    // nAccumulate 0 = none, 1 = always
                    // nTransformType 0: "property" else "image"

                    if( nBits & 3 && xAnimate.is() )
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
                            xAnimate->setAccumulate( nAccumulate == 0 );
                        }
                    }
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
                }
            }
            break;

            case DFF_msofbtAnimateTargetElement:
            {
                sal_Int16 nSubType;
                importTargetElementContainer( pChildAtom, aTarget, nSubType );
                if( xAnimate.is() )
                    xAnimate->setSubItem( nSubType );
            }
            break;

            default:
                break;
            }

            pChildAtom = Atom::findNextChildAtom( pChildAtom );
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

sal_Int16 AnimationImporter::implGetColorSpace( sal_Int32 nMode, sal_Int32 /*nA*/, sal_Int32 /*nB*/, sal_Int32 /*nC*/ )
{
    switch( nMode )
    {
    case 2: // index
    default:
    case 0: // rgb
        return AnimationColorSpace::RGB;

    case 1: // hsl
        return AnimationColorSpace::HSL;
    }
}

Any AnimationImporter::implGetColorAny( sal_Int32 nMode, sal_Int32  nA, sal_Int32 nB, sal_Int32 nC )
{
    switch( nMode )
    {
    case 0: // rgb
        {
            Color aColor( static_cast<sal_uInt8>(nA), static_cast<sal_uInt8>(nB), static_cast<sal_uInt8>(nC) );
            return Any( static_cast<sal_Int32>(aColor.GetRGBColor()) );
        }
    case 1: // hsl
        {
            Sequence< double > aHSL{ nA * 360.0/255.0,
                                     nB / 255.0,
                                     nC / 255.0 };
            return Any( aHSL );
        }

    case 2: // index
        {
            Color aColor;
            mpPPTImport->GetColorFromPalette(static_cast<sal_uInt16>(nA), aColor );
            return Any( static_cast<sal_Int32>(aColor.GetRGBColor()) );
        }

    default:
        {
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
    if( !(pAtom && xColor.is()) )
        return;

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
            mrStCtrl.ReadUInt32( nBits );
            mrStCtrl.ReadInt32( nByMode ).ReadInt32( nByA ).ReadInt32( nByB ).ReadInt32( nByC );
            mrStCtrl.ReadInt32( nFromMode ).ReadInt32( nFromA ).ReadInt32( nFromB ).ReadInt32( nFromC );
            mrStCtrl.ReadInt32( nToMode ).ReadInt32( nToA ).ReadInt32( nToB ).ReadInt32( nToC );

            if (!mrStCtrl.good())
            {
                SAL_WARN("filter.ms", "DFF_msofbtAnimateColorData: short read");
                break;
            }

            if( nBits & 1 )
            {
                xColor->setBy( implGetColorAny( nByMode, nByA, nByB, nByC ) );
                xColor->setColorInterpolation( implGetColorSpace( nByMode, nByA, nByB, nByC ) );
            }

            if( nBits & 2 )
            {
                xColor->setFrom( implGetColorAny( nFromMode, nFromA, nFromB, nFromC ) );
                xColor->setColorInterpolation( implGetColorSpace( nFromMode, nFromA, nFromB, nFromC ) );
            }

            if( nBits & 4 )
            {
                xColor->setTo( implGetColorAny( nToMode, nToA, nToB, nToC ) );
                xColor->setColorInterpolation( implGetColorSpace( nToMode, nToA, nToB, nToC ) );
            }
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importAnimateSetContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateSet > xSet( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateSet && xSet.is(), "invalid call to ppt::AnimationImporter::importAnimateSetContainer()!");
    if( !(pAtom && xSet.is()) )
        return;

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
            mrStCtrl.ReadInt32( nU1 ).ReadInt32( nU2 );
        }
        break;

        case DFF_msofbtAnimAttributeValue:
        {
            Any aTo;
            if ( importAttributeValue( pChildAtom, aTo ) )
            {
                xSet->setTo( aTo );
            }
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importAnimateContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimate > xAnim( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimate && xAnim.is(), "invalid call to ppt::AnimationImporter::importAnimateContainer()!");
    if( !(pAtom && xAnim.is()) )
        return;

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
            sal_uInt32 nCalcmode(0), nBits(0), nValueType(0);
            mrStCtrl.ReadUInt32( nCalcmode ).ReadUInt32( nBits ).ReadUInt32( nValueType );

            if( nBits & 0x08 )
            {
                sal_Int16 n = (nCalcmode == 1) ? AnimationCalcMode::LINEAR : /* (nCalcmode == 2) ? AnimationCalcMode::FORMULA : */ AnimationCalcMode::DISCRETE;
                xAnim->setCalcMode( n );
            }

            if( nBits & 0x30 )
            {
                sal_Int16 n = (nValueType == 1) ? AnimationValueType::NUMBER : (nValueType == 2 ) ? AnimationValueType::COLOR : AnimationValueType::STRING;
                xAnim->setValueType( n );
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
                    case 1: xAnim->setBy( a ); break;
                    case 2: xAnim->setFrom( a ); break;
                    case 3: xAnim->setTo( a ); break;
                    }
                }
            }
            break;
        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importAnimateMotionContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateMotion > xMotion( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateMotion && xMotion.is(), "invalid call to ppt::AnimationImporter::importAnimateMotionContainer()!");
    if( !(pAtom && xMotion.is()) )
        return;

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

            mrStCtrl.ReadUInt32( nBits ).ReadFloat( fByX ).ReadFloat( fByY ).ReadFloat( fFromX ).ReadFloat( fFromY ).ReadFloat( fToX ).ReadFloat( fToY ).ReadUInt32( nOrigin );
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
                    // E can appear inside a number, so we only check for its presence at the end
                    aStr = aStr.trim();
                    if (aStr.endsWith("E"))
                        aStr = aStr.copy(0, aStr.getLength() - 1);
                    aStr = aStr.trim();
                    aPath <<= aStr;
                    xMotion->setPath( aPath );
                }
            }
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importCommandContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XCommand > xCommand( xNode, UNO_QUERY );
    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimCommand && xCommand.is(), "invalid call to ppt::AnimationImporter::importCommandContainer()!");
    if( !(pAtom && xCommand.is()) )
        return;

    sal_Int32 nBits = 0;
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
            mrStCtrl.ReadInt32( nBits );
            mrStCtrl.ReadInt32( nCommandType );
        }
        break;

        case DFF_msofbtAnimAttributeValue:
        {
            importAttributeValue( pChildAtom, aValue );
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }

    if( !(nBits & 3) )
        return;

    OUString aParam;
    aValue >>= aParam;

    sal_Int16 nCommand = EffectCommands::CUSTOM;

    NamedValue aParamValue;

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
        const std::u16string_view aMediaTime( aParam.subView( 9, aParam.getLength() - 10 ) );
        rtl_math_ConversionStatus eStatus;
        double fMediaTime = ::rtl::math::stringToDouble( aMediaTime, u'.', u',', &eStatus );
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
        xCommand->setParameter( Any( aParamSeq ) );
    }
}

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
                }
            }
            break;

            case DFF_msofbtAnimateTargetElement:
            {
                sal_Int16 nSubType;
                Any aSource;
                importTargetElementContainer( pChildAtom, aSource, nSubType );
                if( xAudio.is() ) {
                    // a sound source is a URL, a media object source is a shape
                    OUString aSoundURL;
                    if( aSource >>= aSoundURL )
                    {
                        // Embedded in the .ppt, not a link: allow it.
                        xAudio->setSource( xmloff::makeSoundSource( aSoundURL, /*bAllowed*/true ) );
                    }
                    else
                        xAudio->setSource( aSource );
                    nNodes ++;
                }
            }
            break;

            default:
                break;
            }

            pChildAtom = Atom::findNextChildAtom( pChildAtom );
        }

        // TODO: What to do with them?
        Any aEmpty;
        xAudio->setBegin( aEmpty );
        xAudio->setEnd( aEmpty );
    }

    return nNodes;
}

void AnimationImporter::importAnimateScaleContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateScale && xTransform.is(), "invalid call to ppt::AnimationImporter::importAnimateScaleContainer()!");
    if( !(pAtom && xTransform.is()) )
        return;

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
            sal_uInt32 nBits(0), nZoomContents(0);
            float fByX(0.0), fByY(0.0), fFromX(0.0), fFromY(0.0), fToX(0.0), fToY(0.0);

            // nBits %001: by, %010: from, %100: to, %1000: zoomContents(bool)
            mrStCtrl.ReadUInt32( nBits ).ReadFloat( fByX ).ReadFloat( fByY ).ReadFloat( fFromX ).ReadFloat( fFromY ).ReadFloat( fToX ).ReadFloat( fToY ).ReadUInt32( nZoomContents );

            ValuePair aPair;
            // 'from' value
            if( nBits & 2 )
            {
                aPair.First <<= static_cast<double>(fFromX) / 100.0;
                aPair.Second <<= static_cast<double>(fFromY) / 100.0;
                xTransform->setFrom( Any( aPair ) );
            }

            // 'to' value
            if( nBits & 4 )
            {
                aPair.First <<= static_cast<double>(fToX) / 100.0;
                aPair.Second <<= static_cast<double>(fToY) / 100.0;
                xTransform->setTo( Any( aPair ) );
            }

            // 'by' value
            if( nBits & 1 )
            {
                aPair.First <<= static_cast<double>(fByX) / 100.0;
                aPair.Second <<= static_cast<double>(fByY) / 100.0;

                if( nBits & 2 )
                {
                    // 'from' value given, import normally
                    xTransform->setBy( Any( aPair ) );
                }
                else
                {
                    // mapping 'by' to 'to', if no 'from' is
                    // given. This is due to a non-conformity in
                    // PPT, which exports animateScale effects
                    // with a sole 'by' value, but with the
                    // semantics of a sole 'to' animation
                    xTransform->setTo( Any( aPair ) );
                }
            }
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importAnimateRotationContainer( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimateRotation && xTransform.is(), "invalid call to ppt::AnimationImporter::importAnimateRotationContainer()!");
    if( !(pAtom && xTransform.is()) )
        return;

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
            sal_uInt32 nBits(0), nU1(0);
            float fBy(0.0), fFrom(0.0), fTo(0.0);

            // nBits %001: by, %010: from, %100: to, %1000: zoomContents(bool)
            mrStCtrl.ReadUInt32( nBits ).ReadFloat( fBy ).ReadFloat( fFrom ).ReadFloat( fTo ).ReadUInt32( nU1 );

            if( nBits & 1 )
                xTransform->setBy( Any( static_cast<double>(fBy) ) );

            if( nBits & 2 )
                xTransform->setFrom( Any( static_cast<double>(fFrom) ) );

            if( nBits & 4 )
                xTransform->setTo( Any( static_cast<double>(fTo) ) );
        }
        break;

        case DFF_msofbtAnimateTarget:
            importAnimateAttributeTargetContainer( pChildAtom, xNode );
            break;

        default:
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

void AnimationImporter::importAttributeNamesContainer( const Atom* pAtom, OUString& rAttributeNames )
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
                        aNames.append( ';' );

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
}

void AnimationImporter::importAnimationValues( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( pAtom, "invalid call to ppt::AnimationImporter::importAnimationValues()!" );

    if( !pAtom )
        return;

    const Atom* pValueAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimValue );

    while( pValueAtom && pValueAtom->seekToContent() )
    {
        sal_uInt32 nType(0);
        mrStCtrl.ReadUInt32( nType );
        switch( nType )
        {
        case 0:
        {
            float fRepeat(0.0);
            mrStCtrl.ReadFloat( fRepeat );
            xNode->setRepeatCount( (fRepeat < (float(3.40282346638528860e+38))) ? Any( static_cast<double>(fRepeat) ) : Any( Timing_INDEFINITE ) );
        }
        break;

        case 3:
        {
            float faccelerate(0.0);
            mrStCtrl.ReadFloat( faccelerate );
            xNode->setAcceleration( faccelerate );
        }
        break;

        case 4:
        {
            float fdecelerate(0.0);
            mrStCtrl.ReadFloat( fdecelerate );
            xNode->setDecelerate( fdecelerate );
        }
        break;

        case 5:
        {
            sal_Int32 nAutoreverse(0);
            mrStCtrl.ReadInt32( nAutoreverse );
            xNode->setAutoReverse( nAutoreverse != 0 );
        }
        break;

        default:
        {
            sal_uInt32 nUnknown;
            mrStCtrl.ReadUInt32( nUnknown );
        }
        break;
        }

        pValueAtom = pAtom->findNextChildAtom( DFF_msofbtAnimValue, pValueAtom );
    }
}

void AnimationImporter::importAnimateKeyPoints( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimate > xAnim( xNode, UNO_QUERY );

    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimKeyPoints && xAnim.is(), "invalid call to ppt::AnimationImporter::importAnimateKeyPoints()!" );

    if( !(pAtom && xAnim.is()) )
        return;

    // first count keytimes
    const Atom* pIter = nullptr;
    int nKeyTimes = 0;

    while( (pIter = pAtom->findNextChildAtom( DFF_msofbtAnimKeyTime,  pIter )) != nullptr )
        nKeyTimes++;

    Sequence< double > aKeyTimes( nKeyTimes );
    auto aKeyTimesRange = asNonConstRange(aKeyTimes);
    Sequence< Any > aValues( nKeyTimes );
    auto aValuesRange = asNonConstRange(aValues);
    OUString aFormula;

    pIter = pAtom->findFirstChildAtom(DFF_msofbtAnimKeyTime);
    bool bToNormalize = false;
    for( int nKeyTime = 0; (nKeyTime < nKeyTimes) && pIter; nKeyTime++ )
    {
        if( pIter->seekToContent() )
        {
            sal_Int32 nTemp(0);
            mrStCtrl.ReadInt32(nTemp);
            double fTemp = static_cast<double>(nTemp) / 1000.0;
            aKeyTimesRange[nKeyTime] = fTemp;
            if( fTemp == -1 )
                bToNormalize = true;

            const Atom* pValue = Atom::findNextChildAtom(pIter);
            if( pValue && pValue->getType() == DFF_msofbtAnimAttributeValue )
            {
                Any aValue1, aValue2;
                if( importAttributeValue( pValue, aValue1 ) )
                {
                    pValue = Atom::findNextChildAtom(pValue);
                    if( pValue && pValue->getType() == DFF_msofbtAnimAttributeValue )
                    {
                        // Any occurrence of the formula becomes the formula of the whole list.
                        if (importAttributeValue(pValue, aValue2) && aFormula.isEmpty())
                            aValue2 >>= aFormula;
                    }
                    aValuesRange[nKeyTime] = std::move(aValue1);
                }
            }
        }
        pIter = pAtom->findNextChildAtom(DFF_msofbtAnimKeyTime, pIter);
    }

    if( bToNormalize && nKeyTimes >= 2 )
    {
        // if TimeAnimationValueList contains time -1000, key points must be evenly distributed between 0 and 1 ([MS-PPT] 2.8.31)
        for( int nKeyTime = 0; nKeyTime < nKeyTimes; ++nKeyTime )
        {
            aKeyTimesRange[nKeyTime] = static_cast<double>(nKeyTime) / static_cast<double>(nKeyTimes - 1);
        }
    }

    if (aValues.getLength() != aKeyTimes.getLength())
        throw css::io::WrongFormatException();

    xAnim->setKeyTimes( aKeyTimes );
    xAnim->setValues( aValues );
    xAnim->setFormula( aFormula );
}

bool AnimationImporter::importAttributeValue( const Atom* pAtom, Any& rAny )
{
    DBG_ASSERT( pAtom && pAtom->getType() == DFF_msofbtAnimAttributeValue, "invalid call to ppt::AnimationImporter::importAttributeValue()!" );

    bool bOk = false;

    if( pAtom && pAtom->seekToContent() )
    {
        sal_uInt32 nRecLen = pAtom->getLength();
        if ( nRecLen >= 1 )
        {
            sal_Int8 nType(0);
            mrStCtrl.ReadSChar( nType );
            switch( nType )
            {
                case DFF_ANIM_PROP_TYPE_BYTE :
                {
                    if ( nRecLen == 2 )
                    {
                        sal_uInt8 nByte(0);
                        mrStCtrl.ReadUChar( nByte );
                        rAny <<= nByte;

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_INT32 :
                {
                    if ( nRecLen == 5 )
                    {
                        sal_uInt32 nInt32(0);
                        mrStCtrl.ReadUInt32( nInt32 );
                        rAny <<= nInt32;

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_FLOAT:
                {
                    if( nRecLen == 5 )
                    {
                        float fFloat(0.0);
                        mrStCtrl.ReadFloat( fFloat );
                        rAny <<= static_cast<double>(fFloat);

                        bOk = true;
                    }
                }
                break;

                case DFF_ANIM_PROP_TYPE_UNISTRING :
                {
                    if ( ( nRecLen & 1 ) && ( nRecLen > 1 ) )
                    {
                        OUString aOUString = SvxMSDffManager::MSDFFReadZString( mrStCtrl, nRecLen - 1, true );
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

void AnimationImporter::importAnimationEvents( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( xNode.is() && pAtom, "invalid call to ppt::AnimationImporter::importAnimationEvents()!" );

    Any aBegin, aEnd, aNext, aPrev;

    const Atom* pEventAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimEvent );
    while( pEventAtom )
    {
        Any* pEvents = nullptr;

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
                    sal_Int32 nU1(0), nTrigger(0), nU3(0), nBegin(0);
                    mrStCtrl.ReadInt32( nU1 );
                    mrStCtrl.ReadInt32( nTrigger );
                    mrStCtrl.ReadInt32( nU3 );
                    mrStCtrl.ReadInt32( nBegin );

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
                        aEvent.Offset = (nBegin == -1) ? Any( Timing_INDEFINITE ) : Any( nBegin / 1000.0 );
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

                pChildAtom = Atom::findNextChildAtom( pChildAtom );
            }

            *pEvents = oox::addToSequence( *pEvents, (aEvent.Trigger == EventTrigger::NONE) ? aEvent.Offset : Any( aEvent ) );
        }

        pEventAtom = pAtom->findNextChildAtom( DFF_msofbtAnimEvent, pEventAtom );
    }

    xNode->setBegin( aBegin );
    xNode->setEnd( aEnd );
    // TODO: xNode->setNext( aNext );
    // TODO: xNode->setPrev( aNext );
}

void AnimationImporter::importAnimationActions( const Atom* pAtom, const Reference< XAnimationNode >& xNode )
{
    DBG_ASSERT( pAtom && xNode.is(), "invalid call to ppt::AnimationImporter::importAnimationActions()!");

    if( !pAtom )
        return;

    const Atom* pActionAtom = pAtom->findFirstChildAtom( DFF_msofbtAnimAction );

    if( !(pActionAtom && pActionAtom->seekToContent()) )
        return;

    sal_Int32 nConcurrent(0), nNextAction(0), nEndSync(0), nU4(0), nU5(0);
    mrStCtrl.ReadInt32( nConcurrent );
    mrStCtrl.ReadInt32( nNextAction );
    mrStCtrl.ReadInt32( nEndSync );
    mrStCtrl.ReadInt32( nU4 );
    mrStCtrl.ReadInt32( nU5 );

    if( nEndSync == 1 )
        xNode->setEndSync( Any( AnimationEndSync::ALL ) );
}

void AnimationImporter::importTargetElementContainer( const Atom* pAtom, Any& rTarget, sal_Int16& rSubType )
{
    rSubType = ShapeAnimationSubType::AS_WHOLE;
    sal_Int32 nRefMode = -1;

    DBG_ASSERT( pAtom && (pAtom->getType() == DFF_msofbtAnimateTargetElement), "invalid call to ppt::AnimationImporter::importTargetElementContainer()!" );
    if( !pAtom )
        return;

    const Atom* pChildAtom = pAtom->findFirstChildAtom();
    while( pChildAtom && pChildAtom->seekToContent() )
    {
        switch( pChildAtom->getType() )
        {
        case DFF_msofbtAnimReference:
        {
            sal_Int32 nRefType(0), nRefId(0);
            sal_Int32 begin(0), end(0);
            mrStCtrl.ReadInt32( nRefMode );
            mrStCtrl.ReadInt32( nRefType );
            mrStCtrl.ReadInt32( nRefId );
            mrStCtrl.ReadInt32( begin );
            mrStCtrl.ReadInt32( end );

            switch( nRefType )
            {
            case 1: // shape
            {
                SdrObject* pSdrObject = mpPPTImport->getShapeForId( nRefId );
                if( pSdrObject == nullptr )
                    break;

                rTarget <<= pSdrObject->getUnoShape();

                switch( nRefMode )
                {
                case 6: rSubType = ShapeAnimationSubType::ONLY_BACKGROUND; break;
                case 8: rSubType = ShapeAnimationSubType::ONLY_TEXT; break;
                case 2: // one paragraph
                {
                    if((begin == -1) && (end == -1))
                        break;

                    SdrTextObj* pTextObj = DynCastSdrTextObj( pSdrObject );
                    if(!pTextObj)
                        break;

                    const OutlinerParaObject* pOPO = pTextObj->GetOutlinerParaObject();
                    if( pOPO == nullptr )
                        break;

                    const EditTextObject& rEditTextObject = pOPO->GetTextObject();

                    const sal_Int32 nParaCount = rEditTextObject.GetParagraphCount();

                    sal_Int32 nPara = 0;

                    while( (nPara < nParaCount) && (begin > 0) )
                    {
                        sal_Int32 nParaLength = rEditTextObject.GetTextLen( nPara ) + 1;
                        begin -= nParaLength;
                        nPara++;
                    }

                    if( nPara < nParaCount )
                    {
                        ParagraphTarget aParaTarget;
                        rTarget >>= aParaTarget.Shape;
                        /* FIXME: Paragraph should be sal_Int32 as well */
                        aParaTarget.Paragraph = static_cast<sal_Int16>(nPara);
                        rTarget <<= aParaTarget;

                        rSubType = ShapeAnimationSubType::ONLY_TEXT;
                    }
                }
                }
            }
            break;

            case 2: // sound
                {
                    OUString aSoundURL( mpPPTImport->ReadSound( nRefId ) );
                    rTarget <<= aSoundURL;
                }
                break;
            case 3: // audio object
            case 4: // video object
                {
                    SdrObject* pSdrObject = mpPPTImport->getShapeForId( nRefId );
                    if( pSdrObject == nullptr )
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
            mrStCtrl.ReadInt32( nU1 );
        }
        break;
        default:
            OSL_FAIL("unknown atom inside ppt::AnimationImporter::importTargetElementContainer()!");
            break;
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );

    }
}

void AnimationImporter::importPropertySetContainer( const Atom* pAtom, PropertySet& rSet )
{
    DBG_ASSERT( pAtom && (pAtom->getType() == DFF_msofbtAnimPropertySet), "invalid call to ppt::AnimationImporter::importPropertySetContainer()!" );

    if( !pAtom )
        return;

    const Atom* pChildAtom = pAtom->findFirstChildAtom();
    while( pChildAtom )
    {
        if( pChildAtom->getType() == DFF_msofbtAnimAttributeValue )
        {
            Any aAny;
            (void)importAttributeValue( pChildAtom, aAny );
            rSet.maProperties[ pChildAtom->getInstance() ] = std::move(aAny);
        }
        else
        {
            OSL_FAIL("unknown atom inside ppt::AnimationImporter::importPropertySetContainer()!");
        }

        pChildAtom = Atom::findNextChildAtom( pChildAtom );
    }
}

} // namespace ppt;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
