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
#include <com/sun/star/animations/EventTrigger.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/EffectPresetClass.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/animations/AnimationTransformType.hpp>
#include <com/sun/star/animations/AnimationCalcMode.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/animations/AnimationAdditiveMode.hpp>
#include <com/sun/star/animations/XAnimateSet.hpp>
#include <com/sun/star/animations/XAudio.hpp>
#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/animations/XAnimateColor.hpp>
#include <com/sun/star/animations/XAnimateMotion.hpp>
#include <com/sun/star/animations/XAnimateTransform.hpp>
#include <com/sun/star/animations/ValuePair.hpp>
#include <com/sun/star/animations/AnimationColorSpace.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/animations/XIterateContainer.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>

#include <oox/ppt/pptfilterhelpers.hxx>
#include "pptexanimations.hxx"
#include "pptexsoundcollection.hxx"
#include "../ppt/pptanimations.hxx"
#include <filter/msfilter/escherex.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <algorithm>

using ::com::sun::star::uno::Any;
using ::com::sun::star::util::XCloneable;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::presentation;

namespace ppt
{

static void ImplTranslateAttribute( OUString& rString, const TranslateMode eTranslateMode )
{
    if ( eTranslateMode == TRANSLATE_NONE )
        return;

    if ( ( eTranslateMode & TRANSLATE_VALUE ) || ( eTranslateMode & TRANSLATE_ATTRIBUTE ) )
    {
        const oox::ppt::ImplAttributeNameConversion* p = oox::ppt::getAttributeConversionList();
        while( p->mpAPIName )
        {
            if( rString.equalsAscii( p->mpAPIName ) )
                break;
            p++;
        }
        if( p->mpMSName )
        {
            if ( eTranslateMode & TRANSLATE_VALUE )
            {
                rString = "#";
                rString += OUString::createFromAscii( p->mpMSName );
            }
            else
                rString = OUString::createFromAscii( p->mpMSName );
        }
    }
    else if ( eTranslateMode & TRANSLATE_MEASURE )
    {
        const sal_Char* pDest[] = { "#ppt_x", "#ppt_y", "#ppt_w", "#ppt_h", nullptr };
        const sal_Char* pSource[] = { "x", "y", "width", "height", nullptr };
        sal_Int32 nIndex = 0;

        const sal_Char** ps = pSource;
        const sal_Char** pd = pDest;

        while( *ps )
        {
            const OUString aSearch( OUString::createFromAscii( *ps ) );
            while( (nIndex = rString.indexOf( aSearch, nIndex )) != -1  )
            {
                sal_Int32 nLength = aSearch.getLength();
                if( nIndex && ( rString[nIndex-1] == '#' ) )
                {
                    nIndex--;
                    nLength++;
                }

                const OUString aNew( OUString::createFromAscii( *pd ) );
                rString = rString.replaceAt( nIndex, nLength, aNew );
                nIndex += aNew.getLength();
            }
            ps++;
            pd++;
        }
    }
}

sal_uInt32 AnimationExporter::TranslatePresetSubType( const sal_uInt32 nPresetClass, const sal_uInt32 nPresetId, const OUString& rPresetSubType )
{
    sal_uInt32  nPresetSubType = 0;
    bool    bTranslated = false;

    if ( ( nPresetClass == sal_uInt32(EffectPresetClass::ENTRANCE) ) || ( nPresetClass == sal_uInt32(EffectPresetClass::EXIT) ) )
    {
        if ( nPresetId != 21 )
        {
            switch( nPresetId )
            {
                case 5 :
                {
                    if ( rPresetSubType == "downward" )
                    {
                        nPresetSubType = 5;
                        bTranslated = true;
                    }
                    else if ( rPresetSubType == "across" )
                    {
                        nPresetSubType = 10;
                        bTranslated = true;
                    }
                }
                break;
                case 17 :
                {
                    if ( rPresetSubType == "across" )
                    {
                        nPresetSubType = 10;
                        bTranslated = true;
                    }
                }
                break;
                case 18 :
                {
                    if ( rPresetSubType == "right-to-top" )
                    {
                        nPresetSubType = 3;
                        bTranslated = true;
                    }
                    else if ( rPresetSubType == "right-to-bottom" )
                    {
                        nPresetSubType = 6;
                        bTranslated = true;
                    }
                    else if ( rPresetSubType == "left-to-top" )
                    {
                        nPresetSubType = 9;
                        bTranslated = true;
                    }
                    else if ( rPresetSubType == "left-to-bottom" )
                    {
                        nPresetSubType = 12;
                        bTranslated = true;
                    }
                }
                break;
            }
        }
        if ( !bTranslated )
        {
            const oox::ppt::convert_subtype* p = oox::ppt::convert_subtype::getList();
            while( p->mpStrSubType )
            {
                if ( rPresetSubType.equalsAscii( p->mpStrSubType ) )
                {
                    nPresetSubType = p->mnID;
                    bTranslated = true;
                    break;
                }
                p++;
            }
        }
    }
    if ( !bTranslated )
        nPresetSubType = static_cast<sal_uInt32>(rPresetSubType.toInt32());
    return nPresetSubType;
}

const sal_Char* AnimationExporter::FindTransitionName( const sal_Int16 nType, const sal_Int16 nSubType, const bool bDirection )
{
    const sal_Char* pRet = nullptr;
    int             nFit = 0;

    const oox::ppt::transition* p = oox::ppt::transition::getList();
    while( p->mpName )
    {
        int nF = 0;
        if ( nType == p->mnType )
            nF += 4;
        if ( nSubType == p->mnSubType )
            nF += 2;
        if ( bDirection == p->mbDirection )
            nF += 1;
        if ( nF > nFit )
        {
            pRet = p->mpName;
            nFit = nF;
        }
        if ( nFit == 7 )    // maximum
            break;
        p++;
    }
    return pRet;
}

SvStream& WriteAnimationNode(SvStream& rOut, AnimationNode const & rNode )
{
    rOut.WriteInt32( rNode.mnU1 );
    rOut.WriteInt32( rNode.mnRestart );
    rOut.WriteInt32( rNode.mnGroupType );
    rOut.WriteInt32( rNode.mnFill );
    rOut.WriteInt32( rNode.mnU3 );
    rOut.WriteInt32( rNode.mnU4 );
    rOut.WriteInt32( rNode.mnDuration );
    rOut.WriteInt32( rNode.mnNodeType );

    return rOut;
}

AnimationExporter::AnimationExporter( const EscherSolverContainer& rSolverContainer, ppt::ExSoundCollection& rExSoundCollection ) :
    mrSolverContainer   ( rSolverContainer ),
    mrExSoundCollection ( rExSoundCollection ),
    mnCurrentGroup(0)
{
}

sal_Int16 AnimationExporter::GetFillMode( const Reference< XAnimationNode >& xNode, const sal_Int16 nFillDefault )
{
    sal_Int16 nFill = xNode->getFill();
    //#i119699 <Animation> The animation effect "Emphasis->FlashBulb" play incorrectly in Aoo saves a .ppt to another .ppt and plays the saved one.
    //#i119740 <Animation> The animation effect "Entrance->Flash Once" fails to play in Aoo while Aoo saves a .ppt to another .ppt and plays the saved one.
    if ((xNode->getType() == AnimationNodeType::ANIMATE)
        ||(xNode->getType() == AnimationNodeType::SET)
        ||(xNode->getType() == AnimationNodeType::TRANSITIONFILTER))
    {
        if ( nFill == AnimationFill::DEFAULT )
            return nFill;
    }

    if ( nFill == AnimationFill::DEFAULT )
    {
        nFill = nFillDefault;
    }
    if( nFill == AnimationFill::AUTO )
    {
        nFill = AnimationFill::REMOVE;
        bool bIsIndefiniteTiming = true;
        Any aAny = xNode->getDuration();
        if( aAny.hasValue() )
        {
            Timing eTiming;
            if( aAny >>= eTiming )
                bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
        }
        if ( bIsIndefiniteTiming )
        {
            aAny = xNode->getEnd();
            if( aAny.hasValue() )
            {
                Timing eTiming;
                if( aAny >>= eTiming )
                    bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
            }
            if ( bIsIndefiniteTiming )
            {
                if ( !xNode->getRepeatCount().hasValue() )
                {
                    aAny = xNode->getRepeatDuration();
                    if( aAny.hasValue() )
                    {
                        Timing eTiming;
                        if( aAny >>= eTiming )
                            bIsIndefiniteTiming = eTiming == Timing_INDEFINITE;
                    }
                    if ( bIsIndefiniteTiming )
                        nFill = AnimationFill::FREEZE;
                }
            }
        }
    }
    return nFill;
}

void AnimationExporter::doexport( const Reference< XDrawPage >& xPage, SvStream& rStrm )
{
    Reference< XAnimationNodeSupplier > xNodeSupplier( xPage, UNO_QUERY );
    if( xNodeSupplier.is() )
    {
        const Reference< XAnimationNode > xRootNode( xNodeSupplier->getAnimationNode() );
        if( xRootNode.is() )
        {
            processAfterEffectNodes( xRootNode );
            exportNode( rStrm, xRootNode, DFF_msofbtAnimGroup, 1, 0, false, AnimationFill::AUTO );
        }
    }
}

void AnimationExporter::processAfterEffectNodes( const Reference< XAnimationNode >& xRootNode )
{
    try
    {
        Reference< XEnumerationAccess > xEnumerationAccess( xRootNode, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            Reference< XAnimationNode > xNode( xEnumeration->nextElement(), UNO_QUERY_THROW );

            Reference< XEnumerationAccess > xEnumerationAccess2( xNode, UNO_QUERY );
            if ( xEnumerationAccess2.is() )
            {
                Reference< XEnumeration > xEnumeration2( xEnumerationAccess2->createEnumeration(), UNO_QUERY_THROW );
                while( xEnumeration2->hasMoreElements() )
                {
                    Reference< XAnimationNode > xChildNode( xEnumeration2->nextElement(), UNO_QUERY_THROW );

                    Reference< XEnumerationAccess > xEnumerationAccess3( xChildNode, UNO_QUERY_THROW );
                    Reference< XEnumeration > xEnumeration3( xEnumerationAccess3->createEnumeration(), UNO_QUERY_THROW );
                    while( xEnumeration3->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xChildNode2( xEnumeration3->nextElement(), UNO_QUERY_THROW );

                        Reference< XEnumerationAccess > xEnumerationAccess4( xChildNode2, UNO_QUERY_THROW );
                        Reference< XEnumeration > xEnumeration4( xEnumerationAccess4->createEnumeration(), UNO_QUERY_THROW );
                        while( xEnumeration4->hasMoreElements() )
                        {
                            Reference< XAnimationNode > xChildNode3( xEnumeration4->nextElement(), UNO_QUERY_THROW );

                            switch( xChildNode3->getType() )
                            {
                            // found an after effect
                            case AnimationNodeType::SET:
                            case AnimationNodeType::ANIMATECOLOR:
                                {
                                    Reference< XAnimationNode > xMaster;

                                    Sequence< NamedValue > aUserData( xChildNode3->getUserData() );
                                    sal_Int32 nLength = aUserData.getLength();
                                    const NamedValue* p = aUserData.getConstArray();

                                    while( nLength-- )
                                    {
                                        if ( p->Name == "master-element" )
                                        {
                                            p->Value >>= xMaster;
                                            break;
                                        }
                                        p++;
                                    }

                                    AfterEffectNodePtr pAfterEffectNode( new AfterEffectNode( xChildNode3, xMaster ) );
                                    maAfterEffectNodes.push_back( pAfterEffectNode );
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL( "(@CL)AnimationExporter::processAfterEffectNodes(), exception caught!" );
    }
}

bool AnimationExporter::isAfterEffectNode( const Reference< XAnimationNode >& xNode ) const
{
    return std::any_of(maAfterEffectNodes.begin(), maAfterEffectNodes.end(),
        [&xNode](const AfterEffectNodePtr& rxNode) { return rxNode->mxNode == xNode; });
}

bool AnimationExporter::hasAfterEffectNode( const Reference< XAnimationNode >& xNode, Reference< XAnimationNode >& xAfterEffectNode ) const
{
    auto aIter = std::find_if(maAfterEffectNodes.begin(), maAfterEffectNodes.end(),
        [&xNode](const AfterEffectNodePtr& rxNode) { return rxNode->mxMaster == xNode; });
    if (aIter != maAfterEffectNodes.end())
    {
        xAfterEffectNode = (*aIter)->mxNode;
        return true;
    }

    return false;
}

// check if this group only contain empty groups. this may happen when
// after effect nodes are not exported at theire original position
bool AnimationExporter::isEmptyNode( const Reference< XAnimationNode >& xNode ) const
{
    if( xNode.is() ) switch( xNode->getType() )
    {
    case AnimationNodeType::PAR :
    case AnimationNodeType::SEQ :
    case AnimationNodeType::ITERATE :
        {
            Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
            if( xEnumerationAccess.is() )
            {
                Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
                if( xEnumeration.is() )
                {
                    while( xEnumeration->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                        if( xChildNode.is() && !isEmptyNode( xChildNode ) )
                            return false;
                    }
                }
            }
        }
        break;

    case AnimationNodeType::SET :
    case AnimationNodeType::ANIMATECOLOR :
        return isAfterEffectNode( xNode );
    default:
        return false;
    }

    return true;
}

void AnimationExporter::exportNode( SvStream& rStrm, Reference< XAnimationNode > const & xNode_in, const sal_uInt16 nContainerRecType,
                                    const sal_uInt16 nInstance, const sal_Int32 nGroupLevel, const bool bTakeBackInteractiveSequenceTiming, const sal_Int16 nFDef )
{
    auto xNode = xNode_in;

    if( (nGroupLevel == 4) && isEmptyNode( xNode ) )
        return;

    if ( ( nContainerRecType == DFF_msofbtAnimGroup ) && ( nGroupLevel == 2 ) && isEmptyNode( xNode ) )
        return;

    if( nContainerRecType == DFF_msofbtAnimGroup )
        mnCurrentGroup++;

    bool bTakeBackInteractiveSequenceTimingForChild = false;
    sal_Int16 nFillDefault = GetFillMode( xNode, nFDef );

    Reference< XAnimationNode > xAudioNode;
    static sal_uInt32 nAudioGroup;

    {
        bool bSkipChildren = false;
        EscherExContainer aContainer( rStrm, nContainerRecType, nInstance );
        switch( xNode->getType() )
        {
            case AnimationNodeType::CUSTOM :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
            }
            break;

            case AnimationNodeType::PAR :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                sal_Int32 nFlags = nGroupLevel == 2 ? 0x10 : 0;
                if ( bTakeBackInteractiveSequenceTiming )
                    nFlags |= 0x40;
                exportAnimEvent( rStrm, xNode, nFlags );
                exportAnimValue( rStrm, xNode, nGroupLevel == 4 );
            }
            break;

            case AnimationNodeType::SEQ :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                sal_Int16 nNodeType = exportAnimPropertySet( rStrm, xNode );
                sal_Int32 nFlags = 12;
                if ( ( nGroupLevel == 1 ) && ( nNodeType == css::presentation::EffectNodeType::INTERACTIVE_SEQUENCE ) )
                {
                    nFlags |= 0x20;
                    bTakeBackInteractiveSequenceTimingForChild = true;
                }
                exportAnimAction( rStrm, xNode );
                exportAnimEvent( rStrm, xNode, nFlags );
                exportAnimValue( rStrm, xNode, false );
            }
            break;

            case AnimationNodeType::ITERATE :
            {
                {
                    EscherExAtom aAnimNodeExAtom( rStrm, DFF_msofbtAnimNode );
                    AnimationNode aAnim;
                    aAnim.mnGroupType = mso_Anim_GroupType_PAR;
                    aAnim.mnNodeType = 1;
                    // attribute Restart
                    switch( xNode->getRestart() )
                    {
                        default:
                        case AnimationRestart::DEFAULT : aAnim.mnRestart = 0; break;
                        case AnimationRestart::ALWAYS  : aAnim.mnRestart = 1; break;
                        case AnimationRestart::WHEN_NOT_ACTIVE : aAnim.mnRestart = 2; break;
                        case AnimationRestart::NEVER : aAnim.mnRestart = 3; break;
                    }
                    // attribute Fill
                    switch( xNode->getFill() )
                    {
                        default:
                        case AnimationFill::DEFAULT : aAnim.mnFill = 0; break;
                        case AnimationFill::REMOVE : aAnim.mnFill = 1; break;
                        case AnimationFill::FREEZE : aAnim.mnFill = 2; break;
                        case AnimationFill::HOLD : aAnim.mnFill = 3; break;
                        case AnimationFill::TRANSITION : aAnim.mnFill = 4; break;
                    }
                    WriteAnimationNode( rStrm, aAnim );
                }
                exportIterate( rStrm, xNode );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
            }
            break;

            case AnimationNodeType::ANIMATE :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
                exportAnimate( rStrm, xNode );
            }
            break;

            case AnimationNodeType::SET :
            {
                bool bIsAfterEffectNode( isAfterEffectNode( xNode ) );
                if( (nGroupLevel != 4) || !bIsAfterEffectNode )
                {
                    exportAnimNode( rStrm, xNode, nFillDefault );
                    exportAnimPropertySet( rStrm, xNode );
                    exportAnimateSet( rStrm, xNode, bIsAfterEffectNode ? AFTEREFFECT_SET : AFTEREFFECT_NONE );
                    exportAnimEvent( rStrm, xNode );
                    exportAnimValue( rStrm, xNode, false );
                }
                else
                {
                    bSkipChildren = true;
                }
            }
            break;

            case AnimationNodeType::ANIMATEMOTION :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimateMotion( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
            }
            break;

            case AnimationNodeType::ANIMATECOLOR :
            {
                bool bIsAfterEffectNode( isAfterEffectNode( xNode ) );
                if( (nGroupLevel != 4) || !bIsAfterEffectNode )
                {
                    if( bIsAfterEffectNode )
                        xNode = createAfterEffectNodeClone( xNode );

                    exportAnimNode( rStrm, xNode, nFillDefault );
                    exportAnimPropertySet( rStrm, xNode );
                    exportAnimateColor( rStrm, xNode, bIsAfterEffectNode ? AFTEREFFECT_COLOR : AFTEREFFECT_NONE );
                    exportAnimEvent( rStrm, xNode );
                    exportAnimValue( rStrm, xNode, false );
                }
                else
                {
                    bSkipChildren = true;
                }
            }
            break;

            case AnimationNodeType::ANIMATETRANSFORM :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimateTransform( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
            }
            break;

            case AnimationNodeType::TRANSITIONFILTER :
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );
                exportAnimEvent( rStrm, xNode );
                exportAnimValue( rStrm, xNode, false );
                exportTransitionFilter( rStrm, xNode );
            }
            break;

            case AnimationNodeType::AUDIO :     // #i58428#
            {
                exportAnimNode( rStrm, xNode, nFillDefault );
                exportAnimPropertySet( rStrm, xNode );

                Reference< XAudio > xAudio( xNode, UNO_QUERY );
                if( xAudio.is() )
                {
                    Any aAny( xAudio->getSource() );
                    OUString aURL;

                    if ( ( aAny >>= aURL)  &&  !aURL.isEmpty()  )
                    {
                        sal_Int32 nU1 = 2;
                        sal_Int32 nTrigger = 3;
                        sal_Int32 nU3 = nAudioGroup;
                        sal_Int32 nBegin = 0;
                        {
                            EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, 1 );
                            {
                                EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
                                rStrm.WriteInt32( nU1 ).WriteInt32( nTrigger ).WriteInt32( nU3 ).WriteInt32( nBegin );
                            }
                        }
                        nU1 = 1;
                        nTrigger = 0xb;
                        nU3 = 0;
                        {
                            EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, 2 );
                            {
                                EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
                                rStrm.WriteInt32( nU1 ).WriteInt32( nTrigger ).WriteInt32( nU3 ).WriteInt32( nBegin );
                            }
                        }
                        EscherExContainer aAnimateTargetElement( rStrm, DFF_msofbtAnimateTargetElement );
                        {
                            sal_uInt32 const nRefMode = 3;
                            sal_uInt32 const nRefType = 2;
                            sal_uInt32 nRefId = mrExSoundCollection.GetId( aURL );
                            sal_Int32 const begin = -1;
                            sal_Int32 const end = -1;

                            EscherExAtom aAnimReference( rStrm, DFF_msofbtAnimReference );
                            rStrm.WriteUInt32( nRefMode ).WriteUInt32( nRefType ).WriteUInt32( nRefId ).WriteInt32( begin ).WriteInt32( end );
                        }
                    }
                }
                exportAnimValue( rStrm, xNode, false );
            }
            break;
        }
        if( !bSkipChildren )
        {
            // export after effect node if one exists for this node
            Reference< XAnimationNode > xAfterEffectNode;
            if( hasAfterEffectNode( xNode, xAfterEffectNode ) )
            {
                exportNode( rStrm, xAfterEffectNode, DFF_msofbtAnimSubGoup, 1, nGroupLevel + 1, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );
            }

            Reference< XEnumerationAccess > xEnumerationAccess( xNode, UNO_QUERY );
            if( xEnumerationAccess.is() )
            {
                Reference< XEnumeration > xEnumeration( xEnumerationAccess->createEnumeration(), UNO_QUERY );
                if( xEnumeration.is() )
                {
                    while( xEnumeration->hasMoreElements() )
                    {
                        Reference< XAnimationNode > xChildNode( xEnumeration->nextElement(), UNO_QUERY );
                        if( xChildNode.is() )
                        {
                            if ( xChildNode->getType() == AnimationNodeType::AUDIO )
                            {
                                xAudioNode = xChildNode;
                                nAudioGroup = mnCurrentGroup;
                            }
                            else
                                exportNode( rStrm, xChildNode, DFF_msofbtAnimGroup, 1, nGroupLevel + 1, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );
                        }
                    }
                }
            }
        }
    }
    if ( xAudioNode.is() )
        exportNode( rStrm, xAudioNode, DFF_msofbtAnimGroup, 1, nGroupLevel, bTakeBackInteractiveSequenceTimingForChild, nFillDefault );

    if( xNode->getType() == AnimationNodeType::ITERATE )
        aTarget = Any();
}

Reference< XAnimationNode > AnimationExporter::createAfterEffectNodeClone( const Reference< XAnimationNode >& xNode )
{
    try
    {
        Reference< css::util::XCloneable > xClonable( xNode, UNO_QUERY_THROW );
        Reference< XAnimationNode > xCloneNode( xClonable->createClone(), UNO_QUERY_THROW );

        Any aEmpty;
        xCloneNode->setBegin( aEmpty );

        return xCloneNode;
    }
    catch( Exception& )
    {
        OSL_FAIL("(@CL)sd::ppt::AnimationExporter::createAfterEffectNodeClone(), could not create clone!" );
    }
    return xNode;
}

bool AnimationExporter::GetNodeType( const Reference< XAnimationNode >& xNode, sal_Int16& nType )
{
    // trying to get the nodetype
    Sequence< NamedValue > aUserData = xNode->getUserData();
    if ( aUserData.getLength() )
    {
    const NamedValue* p = aUserData.getConstArray();
    sal_Int32 nLength = aUserData.getLength();
    while( nLength-- )
    {
        if ( p->Name == "node-type" )
        {
        if ( p->Value >>= nType )
            return true;
        }
    }
    }

    return false;
}

void AnimationExporter::exportAnimNode( SvStream& rStrm, const Reference< XAnimationNode >& xNode,
        const sal_Int16 nFillDefault )
{
    EscherExAtom    aAnimNodeExAtom( rStrm, DFF_msofbtAnimNode );
    AnimationNode   aAnim;

    // attribute Restart
    switch( xNode->getRestart() )
    {
        default:
        case AnimationRestart::DEFAULT : aAnim.mnRestart = 0; break;
        case AnimationRestart::ALWAYS  : aAnim.mnRestart = 1; break;
        case AnimationRestart::WHEN_NOT_ACTIVE : aAnim.mnRestart = 2; break;
        case AnimationRestart::NEVER : aAnim.mnRestart = 3; break;
    }

    switch( nFillDefault )
    {
        default:
        case AnimationFill::DEFAULT : aAnim.mnFill = 0; break;
        case AnimationFill::REMOVE : aAnim.mnFill = 1; break;
        case AnimationFill::FREEZE :
        case AnimationFill::HOLD :   aAnim.mnFill = 3; break;
        case AnimationFill::TRANSITION : aAnim.mnFill = 4; break;
    }
    // attribute Duration
    double fDuration = 0.0;
    css::animations::Timing eTiming;
    if ( xNode->getDuration() >>= eTiming )
    {
        if ( eTiming == Timing_INDEFINITE )
            aAnim.mnDuration = -1;
    }
    else if ( xNode->getDuration() >>= fDuration )
    {
        aAnim.mnDuration = static_cast<sal_Int32>( fDuration * 1000.0 );
    }
    else
        aAnim.mnDuration = -1;

    // NodeType, NodeGroup
    aAnim.mnNodeType = 1;
    aAnim.mnGroupType = mso_Anim_GroupType_SEQ;
    switch( xNode->getType() )
    {
        case AnimationNodeType::PAR :
            aAnim.mnGroupType = mso_Anim_GroupType_PAR;
            [[fallthrough]];
        case AnimationNodeType::SEQ :
        {
            sal_Int16 nType = 0;
            if( GetNodeType( xNode, nType ) )
            switch( nType )
            {
                case css::presentation::EffectNodeType::TIMING_ROOT : aAnim.mnNodeType = 0x12; break;
                case css::presentation::EffectNodeType::MAIN_SEQUENCE : aAnim.mnNodeType = 0x18; break;
            }
        }
        break;

        case AnimationNodeType::ANIMATE :
        case AnimationNodeType::SET :

        case AnimationNodeType::CUSTOM :
        case AnimationNodeType::ITERATE :
        case AnimationNodeType::ANIMATEMOTION :
        case AnimationNodeType::ANIMATECOLOR :
        case AnimationNodeType::ANIMATETRANSFORM :
        {
            aAnim.mnGroupType = mso_Anim_GroupType_NODE;
            aAnim.mnNodeType  = mso_Anim_Behaviour_ANIMATION;
        }
        break;

        case AnimationNodeType::AUDIO :
        {
            aAnim.mnGroupType = mso_Anim_GroupType_MEDIA;
            aAnim.mnNodeType  = mso_Anim_Behaviour_ANIMATION;
        }
        break;

        case AnimationNodeType::TRANSITIONFILTER :
        {
            aAnim.mnGroupType = mso_Anim_GroupType_NODE;
            aAnim.mnNodeType  = mso_Anim_Behaviour_FILTER;
        }
        break;
    }

    WriteAnimationNode( rStrm, aAnim );
}

void AnimationExporter::GetUserData( const Sequence< NamedValue >& rUserData, const Any ** pAny, std::size_t nLen )
{
    // storing user data into pAny, to allow direct access later
    memset( pAny, 0, nLen );
    if ( !rUserData.getLength() )
        return;

    const NamedValue* p = rUserData.getConstArray();
    sal_Int32 nLength = rUserData.getLength();
    while( nLength-- )
    {
        if ( p->Name == "node-type" )
        {
        pAny[ DFF_ANIM_NODE_TYPE ] = &(p->Value);
        }
        else if ( p->Name == "preset-class" )
        {
        pAny[ DFF_ANIM_PRESET_CLASS ] = &(p->Value);
        }
        else if ( p->Name == "preset-id" )
        {
        pAny[ DFF_ANIM_PRESET_ID ] = &(p->Value);
        }
        else if ( p->Name == "preset-sub-type" )
        {
        pAny[ DFF_ANIM_PRESET_SUB_TYPE ] = &(p->Value);
        }
        else if ( p->Name == "master-element" )
        {
        pAny[ DFF_ANIM_AFTEREFFECT ] = &(p->Value);
        }
        p++;
    }
}

sal_uInt32 AnimationExporter::GetPresetID( const OUString& rPreset, sal_uInt32 nAPIPresetClass, bool& bPresetId )
{
    sal_uInt32 nPresetId = 0;
    bPresetId = false;

    if ( rPreset.match("ppt_", 0) )
    {
    sal_Int32 nLast = rPreset.lastIndexOf( '_' );
    if ( ( nLast != -1 ) && ( ( nLast + 1 ) < rPreset.getLength() ) )
    {
        OUString aNumber( rPreset.copy( nLast + 1 ) );
        nPresetId = aNumber.toInt32();
        bPresetId = true;
    }
    }
    else
    {
        const oox::ppt::preset_mapping* p = oox::ppt::preset_mapping::getList();
        while( p->mpStrPresetId && ((p->mnPresetClass != static_cast<sal_Int32>(nAPIPresetClass)) || !rPreset.equalsAscii( p->mpStrPresetId )) )
            p++;

        if( p->mpStrPresetId )
        {
            nPresetId = p->mnPresetId;
            bPresetId = true;
        }
    }

    return nPresetId;
}

sal_Int16 AnimationExporter::exportAnimPropertySet( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    sal_Int16 nNodeType = css::presentation::EffectNodeType::DEFAULT;

    EscherExContainer aAnimPropertySet( rStrm, DFF_msofbtAnimPropertySet );

    Reference< XAnimationNode > xMaster;

    Any aMasterRel, aOverride, aRunTimeContext;

    // storing user data into pAny, to allow direct access later
    const Sequence< NamedValue > aUserData = xNode->getUserData();
    const css::uno::Any* pAny[ DFF_ANIM_PROPERTY_ID_COUNT ];
    GetUserData( aUserData, pAny, sizeof( pAny ) );

    if( pAny[ DFF_ANIM_AFTEREFFECT ] )
        ( *pAny[ DFF_ANIM_AFTEREFFECT ] ) >>= xMaster;

    // calculate master-rel
    if( xMaster.is() )
    {
        sal_Int32 nMasterRel = 2;
        if( xNode.is() && xMaster.is() && (xNode->getParent() == xMaster->getParent() ) )
            nMasterRel = 0;

        aMasterRel <<= nMasterRel;

        pAny[ DFF_ANIM_MASTERREL ] = &aMasterRel;

        aOverride <<= sal_Int32(1);
        pAny[ DFF_ANIM_OVERRIDE ] = &aOverride;

        aRunTimeContext <<= sal_Int32(1);
        pAny[ DFF_ANIM_RUNTIMECONTEXT ] = &aRunTimeContext;
    }

    // the order is important
    if ( pAny[ DFF_ANIM_NODE_TYPE ] )
    {
        if ( *pAny[ DFF_ANIM_NODE_TYPE ] >>= nNodeType )
        {
            sal_uInt32 nPPTNodeType = DFF_ANIM_NODE_TYPE_ON_CLICK;
            switch( nNodeType )
            {
                case css::presentation::EffectNodeType::ON_CLICK : nPPTNodeType = DFF_ANIM_NODE_TYPE_ON_CLICK; break;
                case css::presentation::EffectNodeType::WITH_PREVIOUS : nPPTNodeType = DFF_ANIM_NODE_TYPE_WITH_PREVIOUS; break;
                case css::presentation::EffectNodeType::AFTER_PREVIOUS : nPPTNodeType = DFF_ANIM_NODE_TYPE_AFTER_PREVIOUS; break;
                case css::presentation::EffectNodeType::MAIN_SEQUENCE : nPPTNodeType = DFF_ANIM_NODE_TYPE_MAIN_SEQUENCE; break;
                case css::presentation::EffectNodeType::TIMING_ROOT : nPPTNodeType = DFF_ANIM_NODE_TYPE_TIMING_ROOT; break;
                case css::presentation::EffectNodeType::INTERACTIVE_SEQUENCE: nPPTNodeType = DFF_ANIM_NODE_TYPE_INTERACTIVE_SEQ; break;
            }
            exportAnimPropertyuInt32( rStrm, DFF_ANIM_NODE_TYPE, nPPTNodeType );
        }
    }
    sal_uInt32 nPresetId = 0;
    sal_uInt32 nPresetSubType = 0;
    sal_uInt32 nAPIPresetClass = EffectPresetClass::CUSTOM;
    sal_uInt32 nPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
    bool bPresetClass, bPresetId, bPresetSubType;
    bPresetId = bPresetClass = bPresetSubType = false;

    if ( pAny[ DFF_ANIM_PRESET_CLASS ] )
    {
        if ( *pAny[ DFF_ANIM_PRESET_CLASS ] >>= nAPIPresetClass )
        {
            sal_uInt8 nPPTPresetClass;
            switch( nAPIPresetClass )
            {
                case EffectPresetClass::ENTRANCE : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_ENTRANCE; break;
                case EffectPresetClass::EXIT : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_EXIT; break;
                case EffectPresetClass::EMPHASIS : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_EMPHASIS; break;
                case EffectPresetClass::MOTIONPATH : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_MOTIONPATH; break;
                case EffectPresetClass::OLEACTION : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_OLE_ACTION; break;
                case EffectPresetClass::MEDIACALL : nPPTPresetClass = DFF_ANIM_PRESS_CLASS_MEDIACALL; break;
                default :
                    nPPTPresetClass = DFF_ANIM_PRESS_CLASS_USER_DEFINED;
            }
            nPresetClass = nPPTPresetClass;
            bPresetClass = true;
        }
    }
    if ( pAny[ DFF_ANIM_PRESET_ID ] )
    {
        OUString sPreset;
        if ( *pAny[ DFF_ANIM_PRESET_ID ] >>= sPreset )
            nPresetId = GetPresetID( sPreset, nAPIPresetClass, bPresetId );
    }

    if ( pAny[ DFF_ANIM_PRESET_SUB_TYPE ] )
    {
        OUString sPresetSubType;
        if ( *pAny[ DFF_ANIM_PRESET_SUB_TYPE ] >>= sPresetSubType )
        {
            nPresetSubType = TranslatePresetSubType( nPresetClass, nPresetId, sPresetSubType );
            bPresetSubType = true;
        }
    }
    if ( bPresetId )
        exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_ID, nPresetId );
    if ( bPresetSubType )
        exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_SUB_TYPE, nPresetSubType );
    if ( bPresetClass )
        exportAnimPropertyuInt32( rStrm, DFF_ANIM_PRESET_CLASS, nPresetClass );

    if ( pAny[ DFF_ANIM_ID ] )
    {
        // TODO DFF_ANIM_ID
    }

    if ( pAny[ DFF_ANIM_AFTEREFFECT ] )
    {
        bool bAfterEffect = false;
        if ( *pAny[ DFF_ANIM_AFTEREFFECT ] >>= bAfterEffect )
            exportAnimPropertyByte( rStrm, DFF_ANIM_AFTEREFFECT, int(bAfterEffect) );
    }

    if ( pAny[ DFF_ANIM_RUNTIMECONTEXT ] )
    {
        sal_Int32 nRunTimeContext = 0;
        if ( *pAny[ DFF_ANIM_RUNTIMECONTEXT ] >>= nRunTimeContext )
            exportAnimPropertyuInt32( rStrm, DFF_ANIM_RUNTIMECONTEXT, nRunTimeContext );
    }
    if ( pAny[ DFF_ANIM_PATH_EDIT_MODE ] )
    {
        // TODO DFF_ANIM_ID
    }

    if( !xMaster.is() )
    {
        Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
        if( xColor.is() )
        {

            bool bDirection = !xColor->getDirection();
            exportAnimPropertyuInt32( rStrm, DFF_ANIM_DIRECTION, bDirection ? 1 : 0 );
        }
    }

    if ( pAny[ DFF_ANIM_OVERRIDE ] )
    {
        sal_Int32 nOverride = 0;
        if ( *pAny[ DFF_ANIM_OVERRIDE ] >>= nOverride )
            exportAnimPropertyuInt32( rStrm, DFF_ANIM_OVERRIDE, nOverride );
    }

    if ( pAny[ DFF_ANIM_MASTERREL ] )
    {
        sal_Int32 nMasterRel = 0;
        if ( *pAny[ DFF_ANIM_MASTERREL ] >>= nMasterRel )
            exportAnimPropertyuInt32( rStrm, DFF_ANIM_MASTERREL, nMasterRel );
    }

/* todo
    Reference< XAudio > xAudio( xNode, UNO_QUERY );
    if( xAudio.is() )
    {
        sal_Int16 nEndAfterSlide = 0;
        nEndAfterSlide = xAudio->getEndAfterSlide();
        exportAnimPropertyuInt32( rStrm, DFF_ANIM_ENDAFTERSLIDE, nEndAfterSlide, TRANSLATE_NONE );
    }
*/
    Reference< XAnimate > xAnim( xNode, UNO_QUERY );
    if( xAnim.is() )
    {
        // TODO: DFF_ANIM_TIMEFILTER
    }
    if ( pAny[ DFF_ANIM_EVENT_FILTER ] )
    {
        // TODO DFF_ANIM_EVENT_FILTER
    }
    if ( pAny[ DFF_ANIM_VOLUME ] )
    {
        // TODO DFF_ANIM_VOLUME
    }
    return nNodeType;
}

bool AnimationExporter::exportAnimProperty( SvStream& rStrm, const sal_uInt16 nPropertyId, const css::uno::Any& rAny, const TranslateMode eTranslateMode )
{
    bool bRet = false;
    if ( rAny.hasValue() )
    {
        switch( rAny.getValueType().getTypeClass() )
        {
            case css::uno::TypeClass_UNSIGNED_SHORT :
            case css::uno::TypeClass_SHORT :
            case css::uno::TypeClass_UNSIGNED_LONG :
            case css::uno::TypeClass_LONG :
            {
                sal_Int32 nVal = 0;
                if ( rAny >>= nVal )
                {
                    exportAnimPropertyuInt32( rStrm, nPropertyId, nVal );
                    bRet = true;
                }
            }
            break;

            case css::uno::TypeClass_DOUBLE :
            {
                double fVal = 0.0;
                if ( rAny >>= fVal )
                {
                    exportAnimPropertyFloat( rStrm, nPropertyId, fVal );
                    bRet = true;
                }
            }
            break;
            case css::uno::TypeClass_FLOAT :
            {
                float fVal = 0.0;
                if ( rAny >>= fVal )
                {
                    if ( eTranslateMode & TRANSLATE_NUMBER_TO_STRING )
                    {
                        OUString aNumber( OUString::number( fVal ) );
                        exportAnimPropertyString( rStrm, nPropertyId, aNumber, eTranslateMode );
                    }
                    else
                    {
                        exportAnimPropertyFloat( rStrm, nPropertyId, fVal );
                        bRet = true;
                    }
                }
            }
            break;
            case css::uno::TypeClass_STRING :
            {
                OUString aStr;
                if ( rAny >>= aStr )
                {
                    exportAnimPropertyString( rStrm, nPropertyId, aStr, eTranslateMode );
                    bRet = true;
                }
            }
            break;
            default:
                break;
        }
    }
    return bRet;
}
void AnimationExporter::exportAnimPropertyString( SvStream& rStrm, const sal_uInt16 nPropertyId, const OUString& rVal, const TranslateMode eTranslateMode )
{
    EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
    rStrm.WriteUChar( DFF_ANIM_PROP_TYPE_UNISTRING );
    OUString aStr( rVal );
    if ( eTranslateMode != TRANSLATE_NONE )
        ImplTranslateAttribute( aStr, eTranslateMode );
    writeZString( rStrm, aStr );
}

void AnimationExporter::exportAnimPropertyFloat( SvStream& rStrm, const sal_uInt16 nPropertyId, const double& rVal )
{
    EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
    float fFloat = static_cast<float>(rVal);
    rStrm.WriteUChar( DFF_ANIM_PROP_TYPE_FLOAT )
         .WriteFloat( fFloat );
}

void AnimationExporter::exportAnimPropertyuInt32( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt32 nVal )
{
    EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
    rStrm.WriteUChar( DFF_ANIM_PROP_TYPE_INT32 )
         .WriteUInt32( nVal );
}

void AnimationExporter::exportAnimPropertyByte( SvStream& rStrm, const sal_uInt16 nPropertyId, const sal_uInt8 nVal )
{
    EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAttributeValue, nPropertyId );
    rStrm.WriteUChar( DFF_ANIM_PROP_TYPE_BYTE )
         .WriteUChar( nVal );
}

void AnimationExporter::writeZString( SvStream& rStrm, const OUString& rVal )
{
    sal_Int32 i;
    for ( i = 0; i < rVal.getLength(); i++ )
        rStrm.WriteUInt16( rVal[ i ] );
    rStrm.WriteUInt16( 0 );
}

void AnimationExporter::exportAnimAction( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    EscherExAtom aExAtom( rStrm, DFF_msofbtAnimAction );

    sal_Int32 const nConcurrent = 1;
    sal_Int32 const nNextAction = 1;
    sal_Int32 nEndSync = 0;
    sal_Int32 const nU4 = 0;
    sal_Int32 const nU5 = 3;

    sal_Int16 nAnimationEndSync = 0;
    if ( xNode->getEndSync() >>= nAnimationEndSync )
    {
        if ( nAnimationEndSync == AnimationEndSync::ALL )
            nEndSync = 1;
    }
    rStrm.WriteInt32( nConcurrent )
         .WriteInt32( nNextAction )
         .WriteInt32( nEndSync )
         .WriteInt32( nU4 )
         .WriteInt32( nU5 );

}

// nFlags Bit 6 = fixInteractiveSequenceTiming (for child)
// nFlags Bit 5 = fixInteractiveSequenceTiming (for root)
// nFlags Bit 4 = first node of main sequence -> begin event next has to be replaced to indefinite
void AnimationExporter::exportAnimEvent( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const sal_Int32 nFlags )
{
    sal_uInt16 i;
    for ( i = 0; i < 4; i++ )
    {
        sal_Int32 nU1 = 0;
        sal_Int32 nTrigger = 0;
        sal_Int32 nU3 = 0;
        sal_Int32 nBegin = 0;

        bool bCreateEvent = false;
        Any aSource;

        switch( i )
        {
            case 0 :
            case 1 :
            {
                Any aAny;
                Event aEvent;
                css::animations::Timing eTiming;
                if ( i == 0 )
                {
                    if ( nFlags & 0x20 )
                    {
                        // taking the first child
                        Reference< XEnumerationAccess > xEA( xNode, UNO_QUERY_THROW );
                        Reference< XEnumeration > xE( xEA->createEnumeration(), UNO_QUERY_THROW );
                        if ( xE->hasMoreElements() )
                        {
                            Reference< XAnimationNode > xClickNode( xE->nextElement(), UNO_QUERY );
                            aAny = xClickNode->getBegin();
                        }
                    }
                    else if ( nFlags & 0x40 )
                    {
                        // begin has to be replaced with void, so don't do anything
                    }
                    else
                    {
                        aAny = xNode->getBegin();
                        if ( nFlags & 0x10 )    // replace ON_NEXT with INDEFINITE
                        {
                            if ( ( aAny >>= aEvent ) && ( aEvent.Trigger == EventTrigger::ON_NEXT ) )
                            {
                                eTiming = Timing_INDEFINITE;
                                aAny <<= eTiming;
                            }
                        }
                    }
                }
                else
                    aAny = xNode->getEnd();

                double fTiming = 0.0;
                if ( aAny >>= aEvent )
                {
                    bCreateEvent = true;
                    switch( aEvent.Trigger )
                    {
                        case EventTrigger::NONE : nTrigger = 0; break;
                        case EventTrigger::ON_BEGIN : nTrigger = 1; break;
                        case EventTrigger::ON_END : nTrigger = 2; break;
                        case EventTrigger::BEGIN_EVENT : nTrigger = 3; break;
                        case EventTrigger::END_EVENT : nTrigger = 4; nU1 = 2; nU3 = mnCurrentGroup; break;
                        case EventTrigger::ON_CLICK : nTrigger = 5; break;
                        case EventTrigger::ON_DBL_CLICK : nTrigger = 6; break;
                        case EventTrigger::ON_MOUSE_ENTER : nTrigger = 7; break;
                        case EventTrigger::ON_MOUSE_LEAVE : nTrigger = 8; break;
                        case EventTrigger::ON_NEXT : nTrigger = 9; break;
                        case EventTrigger::ON_PREV : nTrigger = 10; break;
                        case EventTrigger::ON_STOP_AUDIO : nTrigger = 11; break;
                    }
                    if ( aEvent.Offset.hasValue() )
                    {
                        if ( aEvent.Offset >>= eTiming )
                        {
                            if ( eTiming == Timing_INDEFINITE )
                                nBegin = -1;
                        }
                        else if ( aEvent.Offset >>= fTiming )
                            nBegin = static_cast<sal_Int32>( fTiming * 1000.0 );
                    }
                    aSource = aEvent.Source;
                }
                else if ( aAny >>= eTiming )
                {
                    bCreateEvent = true;
                    if ( eTiming == Timing_INDEFINITE )
                        nBegin = -1;
                }
                else if ( aAny >>= fTiming )
                {
                    bCreateEvent = true;
                    nBegin = static_cast<sal_Int32>( fTiming * 1000.0 );
                }
            }
            break;

            case 2 :
            {
                if ( nFlags & ( 1 << i ) )
                {
                    bCreateEvent = true;
                    nU1 = 1;
                    nTrigger = 9;
                }
            }
            break;
            case 3 :
            {
                if ( nFlags & ( 1 << i ) )
                {
                    bCreateEvent = true;
                    nU1 = 1;
                    nTrigger = 10;
                }
            }
            break;
        };
        if ( bCreateEvent )
        {
            EscherExContainer aAnimEvent( rStrm, DFF_msofbtAnimEvent, i + 1 );
            {
                EscherExAtom aAnimTrigger( rStrm, DFF_msofbtAnimTrigger );
                rStrm.WriteInt32( nU1 )
                     .WriteInt32( nTrigger )
                     .WriteInt32( nU3 )
                     .WriteInt32( nBegin );
            }
            exportAnimateTargetElement( rStrm, aSource, ( nFlags & ( 1 << i ) ) != 0 );
        }
    }
}

Any AnimationExporter::convertAnimateValue( const Any& rSourceValue, const OUString& rAttributeName )
{
    OUString aDest;
    if ( rAttributeName == "X"
            || rAttributeName == "Y"
            || rAttributeName == "Width"
            || rAttributeName == "Height"
        )
    {
        OUString aStr;
        if ( rSourceValue >>= aStr )
        {
            ImplTranslateAttribute( aStr, TRANSLATE_MEASURE );
            aDest += aStr;
        }
    }
    else if ( rAttributeName == "Rotate"         // "r" or "style.rotation" ?
            || rAttributeName == "Opacity"
            || rAttributeName == "CharHeight"
            || rAttributeName == "SkewX"
        )
    {
        double fNumber = 0.0;
        if ( rSourceValue >>= fNumber )
            aDest += OUString::number( fNumber );
    }
    else if ( rAttributeName == "Color"
            || rAttributeName == "FillColor"     // "Fillcolor" or "FillColor" ?
            || rAttributeName == "LineColor"
            || rAttributeName == "CharColor"
        )
    {
        sal_Int32 nColor = 0;
        Sequence< double > aHSL( 3 );
        OUString aP( "," );
        if ( rSourceValue >>= aHSL )
        {
            aDest += "hsl("
                  +  OUString::number( static_cast<sal_Int32>( aHSL[ 0 ] / ( 360.0 / 255 ) ) )
                  +  aP
                  +  OUString::number( static_cast<sal_Int32>( aHSL[ 1 ] * 255.0 ) )
                  +  aP
                  +  OUString::number( static_cast<sal_Int32>( aHSL[ 2 ] * 255.0 ) )
                  +  ")";
        }
        else if ( rSourceValue >>= nColor )
        {
            aDest += "rgb("
                  +  OUString::number( static_cast<sal_Int8>(nColor) )
                  +  aP
                  +  OUString::number( static_cast<sal_Int8>( nColor >> 8 ) )
                  +  aP
                  +  OUString::number( static_cast<sal_Int8>( nColor >> 16 ) )
                  +  ")";
        }
    }
    else if ( rAttributeName == "FillStyle" )
    {
        css::drawing::FillStyle eFillStyle;
        if ( rSourceValue >>= eFillStyle )
        {
            if ( eFillStyle == css::drawing::FillStyle_NONE )
                aDest += "none";    // ?
            else
                aDest += "solid";
        }
    }
    else if (rAttributeName == "FillOn")
    {
        bool bFillOn;
        if ( rSourceValue >>= bFillOn )
        {
            if ( bFillOn )
                aDest += "true";
            else
                aDest += "false";
        }
    }
    else if ( rAttributeName == "LineStyle" )
    {
        css::drawing::LineStyle eLineStyle;
        if ( rSourceValue >>= eLineStyle )
        {
            if ( eLineStyle == css::drawing::LineStyle_NONE )
                aDest += "false";
            else
                aDest += "true";
        }
    }
    else if ( rAttributeName == "CharWeight" )
    {
        float fFontWeight = 0.0;
        if ( rSourceValue >>= fFontWeight )
        {
            if ( fFontWeight == css::awt::FontWeight::BOLD )
                aDest += "bold";
            else
                aDest += "normal";
        }
    }
    else if ( rAttributeName == "CharUnderline" )
    {
        sal_Int16 nFontUnderline = 0;
        if ( rSourceValue >>= nFontUnderline )
        {
            if ( nFontUnderline == css::awt::FontUnderline::NONE )
                aDest += "false";
            else
                aDest += "true";
        }
    }
    else if ( rAttributeName == "CharPosture" )
    {
        css::awt::FontSlant eFontSlant;
        if ( rSourceValue >>= eFontSlant )
        {
            if ( eFontSlant == css::awt::FontSlant_ITALIC )
                aDest += "italic";
            else
                aDest += "normal";  // ?
        }
    }
    else if ( rAttributeName == "Visibility" )
    {
        bool bVisible = true;
        if ( rSourceValue >>= bVisible )
        {
            if ( bVisible )
                aDest += "visible";
            else
                aDest += "hidden";
        }
    }
    Any aRet;
    if ( !aDest.isEmpty() )
        aRet <<= aDest;
    else
        aRet = rSourceValue;
    return aRet;
}

void AnimationExporter::exportAnimateSet( SvStream& rStrm, const Reference< XAnimationNode >& xNode, int nAfterEffectType )
{
    Reference< XAnimateSet > xSet( xNode, UNO_QUERY );
    if( !xSet.is() )
        return;

    EscherExContainer aAnimateSet( rStrm, DFF_msofbtAnimateSet, 0 );
    {
        EscherExAtom aAnimateSetData( rStrm, DFF_msofbtAnimateSetData );
        sal_uInt32 const nId1 = 1;            // ??
        sal_uInt32 const nId2 = 1;            // ??
        rStrm.WriteUInt32( nId1 ).WriteUInt32( nId2 );
    }
    Any aConvertedValue( convertAnimateValue( xSet->getTo(), xSet->getAttributeName() ) );
    if ( aConvertedValue.hasValue() )
        exportAnimProperty( rStrm, 1, aConvertedValue, TRANSLATE_NONE );
    exportAnimateTarget( rStrm, xNode, 0, nAfterEffectType );
}

sal_uInt32 AnimationExporter::GetValueTypeForAttributeName( const OUString& rAttributeName )
{
    sal_uInt32 nValueType = 0;

    struct Entry
    {
        const sal_Char* pName;
        sal_uInt8 nType;
    };
    static const Entry lcl_attributeMap[] =
    {
        { "charcolor", 2 },
        { "charfontname", 0 },
        { "charheight", 1 },
        { "charposture", 0 },
        // TODO(Q1): This should prolly be changed in PPT import
        // { "charrotation", ATTRIBUTE_CHAR_ROTATION },
        { "charrotation", 1 },
        { "charunderline", 0 },
        { "charweight", 0 },
        { "color", 2 },
        { "dimcolor", 2 },
        { "fillcolor", 2 },
        { "fillstyle", 0 },
        { "height", 1 },
        { "linecolor", 2 },
        { "linestyle", 0 },
        { "opacity", 0 },
        { "rotate", 1 },
        { "skewx", 1 },
        { "skewy", 1 },
        { "visibility", 1 },
        { "width", 1 },
        { "x", 1 },
        { "y", 1 },
        { nullptr, 0 }
    };
    const Entry* pPtr = &lcl_attributeMap[ 0 ];
    while( pPtr->pName )
    {
        if ( rAttributeName.equalsIgnoreAsciiCaseAscii( pPtr->pName ) )
        {
            nValueType = pPtr->nType;
            break;
        }
        pPtr++;
    }
    DBG_ASSERT( pPtr->pName, "GetValueTypeForAttributeName, unknown property value!" );
    return nValueType;
}

void AnimationExporter::exportAnimate( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
    if ( !xAnimate.is() )
        return;

    Any aBy  ( xAnimate->getBy() );
    Any aFrom( xAnimate->getFrom() );
    Any aTo  ( xAnimate->getTo() );

    EscherExContainer aContainer( rStrm, DFF_msofbtAnimate, 0 );
    {
        EscherExAtom    aAnimateData( rStrm, DFF_msofbtAnimateData );
        sal_uInt32 nBits = 0x38;
        sal_Int16 nTmp = xAnimate->getCalcMode();
        sal_uInt32 nCalcMode = /* (nTmp == AnimationCalcMode::FORMULA) ? 2 : */ (nTmp == AnimationCalcMode::LINEAR) ? 1 : 0;
        sal_uInt32 nValueType = GetValueTypeForAttributeName( xAnimate->getAttributeName() );

        if ( aBy.hasValue() )
            nBits |= 1;
        if ( aFrom.hasValue() )
            nBits |= 2;
        if ( aTo.hasValue() )
            nBits |= 4;

        rStrm.WriteUInt32( nCalcMode )
             .WriteUInt32( nBits )
             .WriteUInt32( nValueType );
    }
    if ( aBy.hasValue() )
        exportAnimProperty( rStrm, 1, aBy, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );
    if ( aFrom.hasValue() )
        exportAnimProperty( rStrm, 2, aFrom, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );
    if ( aTo.hasValue() )
        exportAnimProperty( rStrm, 3, aTo, TRANSLATE_NUMBER_TO_STRING | TRANSLATE_MEASURE );

    exportAnimateKeyPoints( rStrm, xAnimate );
    exportAnimateTarget( rStrm, xNode );
}

void AnimationExporter::exportAnimateTarget( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const sal_uInt32 nForceAttributeNames, int nAfterEffectType )
{
    EscherExContainer aAnimateTarget( rStrm, DFF_msofbtAnimateTarget, 0 );
    Reference< XAnimate > xAnimate( xNode, UNO_QUERY );
    if ( !xAnimate.is() )
        return;

    {
        EscherExAtom aAnimateTargetSettings( rStrm, DFF_msofbtAnimateTargetSettings, 0 );
        // nBits %0001: additive, %0010: accumulate, %0100: attributeName, %1000: transformtype
        // nAdditive 0 = base, 1 = sum, 2 = replace, 3 = multiply, 4 = none
        // nAccumulate 0 = none, 1 = always
        // nTransformType 0: "property" else "image"
        sal_uInt32 nBits = 0;
        sal_uInt32 nAdditive = 0;
        sal_uInt32 nAccumulate = 0;
        sal_uInt32 const nTransformType = 0;
        if ( xAnimate.is() )
        {
            if ( !xAnimate->getAttributeName().isEmpty() )
                nBits |= 4;     // what is attributeName ?, maybe this is set if a DFF_msofbtAnimateAttributeNames is written
            sal_Int16 nAdditiveMode = xAnimate->getAdditive();
            if ( nAdditiveMode != AnimationAdditiveMode::BASE )
            {
                nBits |= 1;
                switch( nAdditiveMode )
                {
                    case AnimationAdditiveMode::SUM : nAdditive = 1; break;
                    case AnimationAdditiveMode::REPLACE : nAdditive = 2; break;
                    case AnimationAdditiveMode::MULTIPLY : nAdditive = 3; break;
                    case AnimationAdditiveMode::NONE : nAdditive = 4; break;
                }
            }
            if ( xAnimate->getAccumulate() )
            {
                nBits  |= 2;
                nAccumulate = 1;
            }
        }
        rStrm.WriteUInt32( nBits )
           .WriteUInt32( nAdditive )
           .WriteUInt32( nAccumulate )
           .WriteUInt32( nTransformType );
    }
    if ( !xAnimate->getAttributeName().isEmpty() || nForceAttributeNames )
    {
        EscherExContainer aAnimateAttributeNames( rStrm, DFF_msofbtAnimateAttributeNames, 1 );
        OUString aAttributeName( xAnimate->getAttributeName() );
        if ( nForceAttributeNames )
        {
            if( nForceAttributeNames == 1 )
            {
                aAttributeName = "r";
            }
        }
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken( aAttributeName.getToken( 0, ';', nIndex ) );
            exportAnimPropertyString( rStrm, 0, aToken, TRANSLATE_ATTRIBUTE );
        }
        while ( nIndex >= 0 );
    }

    if( nAfterEffectType != AFTEREFFECT_NONE )
    {
        EscherExContainer aAnimPropertySet( rStrm, DFF_msofbtAnimPropertySet );
        exportAnimPropertyuInt32( rStrm, 6, 1 );
        if( nAfterEffectType == AFTEREFFECT_COLOR )
        {
            exportAnimPropertyuInt32( rStrm, 4, 0 );
            exportAnimPropertyuInt32( rStrm, 5, 0 );
        }
    }
    exportAnimateTargetElement( rStrm, aTarget.hasValue() ? aTarget : xAnimate->getTarget(), false );
}

Reference< XShape > AnimationExporter::getTargetElementShape( const Any& rAny, sal_Int32& rBegin, sal_Int32& rEnd, bool& rParagraphTarget )
{
    Reference< XShape > xShape;
    rAny >>= xShape;

    rParagraphTarget = false;

    if( !xShape.is() )
    {
    ParagraphTarget aParaTarget;
    if( rAny >>= aParaTarget )
        xShape = aParaTarget.Shape;
    if ( xShape.is() )
    {
        // now calculating the character range for the paragraph
        sal_Int16 nParagraph = aParaTarget.Paragraph;
        Reference< XSimpleText > xText( xShape, UNO_QUERY );
        if ( xText.is() )
        {
        rParagraphTarget = true;
        Reference< XEnumerationAccess > xTextParagraphEnumerationAccess( xText, UNO_QUERY );
        if ( xTextParagraphEnumerationAccess.is() )
        {
            Reference< XEnumeration > xTextParagraphEnumeration( xTextParagraphEnumerationAccess->createEnumeration() );
            if ( xTextParagraphEnumeration.is() )
            {
            sal_Int16 nCurrentParagraph;
            rBegin = rEnd = nCurrentParagraph = 0;
            while ( xTextParagraphEnumeration->hasMoreElements() )
            {
                Reference< XTextRange > xTextRange( xTextParagraphEnumeration->nextElement(), UNO_QUERY );
                if ( xTextRange.is() )
                {
                OUString aParaText( xTextRange->getString() );
                sal_Int32 nLength = aParaText.getLength() + 1;
                rEnd += nLength;
                if ( nCurrentParagraph == nParagraph )
                    break;
                nCurrentParagraph++;
                rBegin += nLength;
                }
            }
            }
        }
        }
    }
    }

    return xShape;
}

void AnimationExporter::exportAnimateTargetElement( SvStream& rStrm, const Any& rAny, const bool bCreate2b01Atom )
{
    sal_uInt32 nRefMode = 0;    // nRefMode == 2 -> Paragraph
    sal_Int32 begin = -1;
    sal_Int32 end = -1;
    bool bParagraphTarget;

    Reference< XShape > xShape = getTargetElementShape(rAny, begin, end, bParagraphTarget);

    if( bParagraphTarget )
        nRefMode = 2;

    if ( !(xShape.is() || bCreate2b01Atom) )
        return;

    EscherExContainer aAnimateTargetElement( rStrm, DFF_msofbtAnimateTargetElement );
    if ( xShape.is() )
    {
        EscherExAtom aAnimReference( rStrm, DFF_msofbtAnimReference );

        sal_uInt32 const nRefType = 1;    // TODO: nRefType == 2 -> Sound;
        sal_uInt32 nRefId = mrSolverContainer.GetShapeId( xShape );

        rStrm.WriteUInt32( nRefMode )
             .WriteUInt32( nRefType )
             .WriteUInt32( nRefId )
             .WriteInt32( begin )
             .WriteInt32( end );
    }
    if ( bCreate2b01Atom )
    {
        EscherExAtom a2b01Atom( rStrm, 0x2b01 );
        rStrm.WriteUInt32( 1 );     // ?
    }
}

void AnimationExporter::exportAnimateKeyPoints( SvStream& rStrm, const Reference< XAnimate >& xAnimate )
{
    Sequence< double > aKeyTimes( xAnimate->getKeyTimes() );
    Sequence< Any > aValues( xAnimate->getValues() );
    OUString aFormula( xAnimate->getFormula() );
    if ( !aKeyTimes.getLength() )
        return;

    EscherExContainer aAnimKeyPoints( rStrm, DFF_msofbtAnimKeyPoints );
    sal_Int32 i;
    for ( i = 0; i < aKeyTimes.getLength(); i++ )
    {
        {
            EscherExAtom aAnimKeyTime( rStrm, DFF_msofbtAnimKeyTime );
            sal_Int32 nKeyTime = static_cast<sal_Int32>( aKeyTimes[ i ] * 1000.0 );
            rStrm.WriteInt32( nKeyTime );
        }
        Any aAny[ 2 ];
        if ( aValues[ i ].hasValue() )
        {
            ValuePair aPair;
            if ( aValues[ i ] >>= aPair )
            {
                aAny[ 0 ] = convertAnimateValue( aPair.First, xAnimate->getAttributeName() );
                aAny[ 1 ] = convertAnimateValue( aPair.Second, xAnimate->getAttributeName() );
            }
            else
            {
                aAny[ 0 ] = convertAnimateValue( aValues[ i ], xAnimate->getAttributeName() );
            }
            if ( !i && !aFormula.isEmpty() )
            {
                ImplTranslateAttribute( aFormula, TRANSLATE_MEASURE );
                aAny[ 1 ] <<= aFormula;
            }
            exportAnimProperty( rStrm, 0, aAny[ 0 ], TRANSLATE_NONE );
            exportAnimProperty( rStrm, 1, aAny[ 1 ], TRANSLATE_NONE );
        }
    }
}

void AnimationExporter::exportAnimValue( SvStream& rStrm, const Reference< XAnimationNode >& xNode, const bool bExportAlways )
{
    Any aAny;
    // repeat count (0)
    double fRepeat = 0.0;
    float fRepeatCount = 0.0;
    css::animations::Timing eTiming;
    aAny = xNode->getRepeatCount();
    if ( aAny >>= eTiming )
    {
        if ( eTiming == Timing_INDEFINITE )
            fRepeatCount = (float(3.40282346638528860e+38));
    }
    else if ( aAny >>= fRepeat )
        fRepeatCount = static_cast<float>(fRepeat);
    if ( fRepeatCount != 0.0 )
    {
        EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
        sal_uInt32 const nType = 0;
        rStrm.WriteUInt32( nType )
             .WriteFloat( fRepeatCount );
    }
    // accelerate (3)
    float fAccelerate = static_cast<float>(xNode->getAcceleration());
    if ( bExportAlways || ( fAccelerate != 0.0 ) )
    {
        EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
        sal_uInt32 const nType = 3;
        rStrm.WriteUInt32( nType )
             .WriteFloat( fAccelerate );
    }

    // decelerate (4)
    float fDecelerate = static_cast<float>(xNode->getDecelerate());
    if ( bExportAlways || ( fDecelerate != 0.0 ) )
    {
        EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
        sal_uInt32 const nType = 4;
        rStrm.WriteUInt32( nType )
             .WriteFloat( fDecelerate );
    }

    // autoreverse (5)
    bool bAutoReverse = xNode->getAutoReverse();
    if ( bExportAlways || bAutoReverse )
    {
        EscherExAtom aExAtom( rStrm, DFF_msofbtAnimValue );
        sal_uInt32 const nType = 5;
        sal_uInt32 nVal  = bAutoReverse ? 1 : 0;
        rStrm.WriteUInt32( nType )
             .WriteUInt32( nVal );
    }
}

void AnimationExporter::exportTransitionFilter( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    Reference< XTransitionFilter > xFilter( xNode, UNO_QUERY );
    if ( !xFilter.is() )
        return;

    EscherExContainer aAnimateFilter( rStrm, DFF_msofbtAnimateFilter );
    {
        EscherExAtom aAnimateFilterData( rStrm, DFF_msofbtAnimateFilterData );
        sal_uInt32 const nBits = 3;       // bit 0 -> use AnimAttributeValue
                                          // bit 1 -> use nTransition

        sal_uInt32 nTransition = xFilter->getMode() ? 0 : 1;
        rStrm.WriteUInt32( nBits )
             .WriteUInt32( nTransition );
    }
    const sal_Char* pFilter = FindTransitionName( xFilter->getTransition(), xFilter->getSubtype(), xFilter->getDirection() );
    if ( pFilter )
    {
        const OUString aStr( OUString::createFromAscii( pFilter ) );
        exportAnimPropertyString( rStrm, 1, aStr, TRANSLATE_NONE );
    }
    exportAnimateTarget( rStrm, xNode );
}

void AnimationExporter::exportAnimateMotion( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateMotion > xMotion( xNode, UNO_QUERY );
    if ( !xMotion.is() )
        return;

    EscherExContainer aAnimateMotion( rStrm, DFF_msofbtAnimateMotion );
    {
        {   //SJ: Ignored from import filter
            EscherExAtom aAnimateMotionData( rStrm, DFF_msofbtAnimateMotionData );
            sal_uInt32 const nBits = 0x98;
            sal_uInt32 const nOrigin = 0x2;
            float const fByX = 100.0; // nBits&1
            float const fByY = 100.0; // nBits&1
            float const fFromX = 0.0; // nBits&2
            float const fFromY = 0.0; // nBits&2
            float const fToX = 100.0; // nBits&4
            float const fToY = 100.0; // nBits&4
            rStrm.WriteUInt32( nBits ).WriteFloat( fByX ).WriteFloat( fByY ).WriteFloat( fFromX ).WriteFloat( fFromY ).WriteFloat( fToX ).WriteFloat( fToY ).WriteUInt32( nOrigin );
        }

        OUString aStr;
        if ( xMotion->getPath() >>= aStr )
        {
            if ( !aStr.isEmpty() )
                exportAnimPropertyString( rStrm, 1, aStr, TRANSLATE_NONE );
        }
        exportAnimateTarget( rStrm, xNode );
    }
}

void AnimationExporter::exportAnimateTransform( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    Reference< XAnimateTransform > xTransform( xNode, UNO_QUERY );
    if ( !xTransform.is() )
        return;

    if ( xTransform->getTransformType() ==  AnimationTransformType::SCALE )
    {
        EscherExContainer aAnimateScale( rStrm, DFF_msofbtAnimateScale );
        {
            EscherExAtom aAnimateScaleData( rStrm, DFF_msofbtAnimateScaleData );
            sal_uInt32 nBits = 0;
            sal_uInt32 const nZoomContents = 1;
            float fByX = 100.0;
            float fByY = 100.0;
            float fFromX = 0.0;
            float fFromY = 0.0;
            float fToX = 100.0;
            float fToY = 100.0;

            double fX = 0.0, fY = 0.0;
            ValuePair aPair;
            if ( xTransform->getBy() >>= aPair )
            {
                if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
                {
                    nBits |= 1;
                    fByX = static_cast<float>( fX * 100 );
                    fByY = static_cast<float>( fY * 100 );
                }
            }
            if ( xTransform->getFrom() >>= aPair )
            {
                if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
                {
                    nBits |= 2;
                    fFromX = static_cast<float>( fX * 100 );
                    fFromY = static_cast<float>( fY * 100 );
                }
            }
            if( xTransform->getTo() >>= aPair )
            {
                if ( ( aPair.First >>= fX ) && ( aPair.Second >>= fY ) )
                {
                    nBits |= 4;
                    fToX = static_cast<float>( fX * 100 );
                    fToY = static_cast<float>( fY * 100 );
                }
            }

            // TODO: ZoomContents:
            //if( nBits & 8 )
            //( fprintf( mpFile, " zoomContents=\"%s\"", nZoomContents ? "true" : "false" );

            rStrm.WriteUInt32( nBits ).WriteFloat( fByX ).WriteFloat( fByY ).WriteFloat( fFromX ).WriteFloat( fFromY ).WriteFloat( fToX ).WriteFloat( fToY ).WriteUInt32( nZoomContents );
        }
        exportAnimateTarget( rStrm, xNode );
    }
    else if ( xTransform->getTransformType() ==  AnimationTransformType::ROTATE )
    {
        EscherExContainer aAnimateRotation( rStrm, DFF_msofbtAnimateRotation );
        {
            EscherExAtom aAnimateRotationData( rStrm, DFF_msofbtAnimateRotationData );
            sal_uInt32 nBits = 0;
            sal_uInt32 const nU1 = 0;
            float fBy = 360.0;
            float fFrom = 0.0;
            float fTo = 360.0;

            double fVal = 0.0;
            if ( xTransform->getBy() >>= fVal )
            {
                nBits |= 1;
                fBy = static_cast<float>(fVal);
            }
            if ( xTransform->getFrom() >>= fVal )
            {
                nBits |= 2;
                fFrom = static_cast<float>(fVal);
            }
            if ( xTransform->getTo() >>= fVal )
            {
                nBits |= 4;
                fTo = static_cast<float>(fVal);
            }
            rStrm.WriteUInt32( nBits ).WriteFloat( fBy ).WriteFloat( fFrom ).WriteFloat( fTo ).WriteUInt32( nU1 );
        }
        exportAnimateTarget( rStrm, xNode, 1 );
    }
}

bool AnimationExporter::getColorAny( const Any& rAny, const sal_Int16 nColorSpace, sal_Int32& rMode, sal_Int32& rA, sal_Int32& rB, sal_Int32& rC )
{
    bool bIsColor = true;

    rMode = 0;
    if ( nColorSpace == AnimationColorSpace::HSL )
        rMode = 1;

    sal_Int32 nColor = 0;
    Sequence< double > aHSL( 3 );
    if ( rAny >>= nColor )      // RGB color
    {
        rA = static_cast<sal_uInt8>( nColor >> 16 );
        rB = static_cast<sal_uInt8>( nColor >> 8 );
        rC = static_cast<sal_uInt8>(nColor);
    }
    else if ( rAny >>= aHSL )   // HSL
    {
        rA = static_cast<sal_Int32>( aHSL[ 0 ] * 255.0 / 360.0 );
        rB = static_cast<sal_Int32>( aHSL[ 1 ] * 255.0 );
        rC = static_cast<sal_Int32>( aHSL[ 2 ] * 255.0 );
    }
    else
        bIsColor = false;
    return bIsColor;
}

void AnimationExporter::exportAnimateColor( SvStream& rStrm, const Reference< XAnimationNode >& xNode, int nAfterEffectType )
{
    Reference< XAnimateColor > xColor( xNode, UNO_QUERY );
    if ( !xColor.is() )
        return;

    EscherExContainer aAnimateColor( rStrm, DFF_msofbtAnimateColor );
    {
        EscherExAtom aAnimateColorData( rStrm, DFF_msofbtAnimateColorData );
        sal_uInt32 nBits = 8;

        sal_Int32 nByMode, nByA, nByB, nByC;
        nByMode = nByA = nByB = nByC = 0;

        sal_Int32 nFromMode, nFromA, nFromB, nFromC;
        nFromMode = nFromA = nFromB = nFromC = 0;

        sal_Int32 nToMode, nToA, nToB, nToC;
        nToMode = nToA = nToB = nToC = 0;

        sal_Int16 nColorSpace = xColor->getColorInterpolation();

        Any aAny( xColor->getBy() );
        if ( aAny.hasValue() )
        {
            if ( getColorAny( aAny, nColorSpace, nByMode, nByA, nByB, nByC ) )
                nBits |= 0x11;
        }
        aAny = xColor->getFrom();
        if ( aAny.hasValue() )
        {
            if ( getColorAny( aAny, nColorSpace, nFromMode, nFromA, nFromB, nFromC ) )
                nBits |= 0x12;
        }
        aAny = xColor->getTo();
        if ( aAny.hasValue() )
        {
            if ( getColorAny( aAny, nColorSpace, nToMode, nToA, nToB, nToC ) )
                nBits |= 0x14;
        }
        rStrm  .WriteUInt32( nBits )
               .WriteInt32( nByMode ).WriteInt32( nByA ).WriteInt32( nByB ).WriteInt32( nByC )
               .WriteInt32( nFromMode ).WriteInt32( nFromA ).WriteInt32( nFromB ).WriteInt32( nFromC )
               .WriteInt32( nToMode ).WriteInt32( nToA ).WriteInt32( nToB ).WriteInt32( nToC );
    }
    exportAnimateTarget( rStrm, xNode, 0, nAfterEffectType );
}

void AnimationExporter::exportIterate( SvStream& rStrm, const Reference< XAnimationNode >& xNode )
{
    Reference< XIterateContainer > xIterate( xNode, UNO_QUERY );
    if ( !xIterate.is() )
        return;

    EscherExAtom aAnimIteration( rStrm, DFF_msofbtAnimIteration );

    float       fInterval = 10.0;
    sal_Int32   nTextUnitEffect = 0;
    sal_Int32 const nU1 = 1;
    sal_Int32 const nU2 = 1;
    sal_Int32 const nU3 = 0xe;

    sal_Int16 nIterateType = xIterate->getIterateType();
    switch( nIterateType )
    {
        case TextAnimationType::BY_WORD : nTextUnitEffect = 1; break;
        case TextAnimationType::BY_LETTER : nTextUnitEffect = 2; break;
    }

    fInterval = static_cast<float>(xIterate->getIterateInterval());

    // convert interval from absolute to percentage
    double fDuration = 0.0;

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
                }
            }
        }
    }

    if( fDuration )
        fInterval = static_cast<float>(100.0 * fInterval / fDuration);

    rStrm.WriteFloat( fInterval ).WriteInt32( nTextUnitEffect ).WriteInt32( nU1 ).WriteInt32( nU2 ).WriteInt32( nU3 );
    aTarget = xIterate->getTarget();
}

} // namespace ppt;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
