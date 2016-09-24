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

#include "ppt97animations.hxx"

#include <svx/svdobj.hxx>
#include "sdpage.hxx"
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <svx/unoapi.hxx>
#include "EffectMigration.hxx"
#include <CustomAnimationPreset.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>

using namespace ::com::sun::star;

void Ppt97AnimationInfoAtom::ReadStream( SvStream& rIn )
{
    rIn.ReadUInt32( nDimColor );
    rIn.ReadUInt32( nFlags );
    rIn.ReadUInt32( nSoundRef );
    rIn.ReadInt32( nDelayTime );
    rIn.ReadUInt16( nOrderID );
    rIn.ReadUInt16( nSlideCount );
    rIn.ReadUChar( nBuildType );
    rIn.ReadUChar( nFlyMethod );
    rIn.ReadUChar( nFlyDirection );
    rIn.ReadUChar( nAfterEffect );
    rIn.ReadUChar( nSubEffect );
    rIn.ReadUChar( nOLEVerb );
    rIn.ReadUChar( nUnknown1 );
    rIn.ReadUChar( nUnknown2 );
}

#define MEMBER_CONSTRUCTOR_LIST() \
    m_aAtom() \
    , m_aSoundFileUrl() \
    , m_bDirtyCache(true) \
    , m_aPresetId() \
    , m_aSubType() \
    , m_bHasSpecialDuration(false) \
    , m_fDurationInSeconds(0.001)

Ppt97Animation::Ppt97Animation( SvStream& rInputStream )
    : MEMBER_CONSTRUCTOR_LIST()
{
     m_aAtom.ReadStream( rInputStream );
}

Ppt97Animation::Ppt97Animation( const Ppt97Animation& rAnimation )
    : MEMBER_CONSTRUCTOR_LIST()
{
    *this = rAnimation;
}

Ppt97Animation& Ppt97Animation::operator= ( const Ppt97Animation& rAnimation )
{
    m_aAtom = rAnimation.m_aAtom;
    m_aSoundFileUrl = rAnimation.m_aSoundFileUrl;
    m_bDirtyCache = rAnimation.m_bDirtyCache;
    m_aPresetId = rAnimation.m_aPresetId;
    m_aSubType = rAnimation.m_aSubType;
    m_bHasSpecialDuration = rAnimation.m_bHasSpecialDuration;
    m_fDurationInSeconds = rAnimation.m_fDurationInSeconds;

    return *this;
}

Ppt97Animation::~Ppt97Animation()
{
}

bool Ppt97Animation::operator < ( const Ppt97Animation& rAnimation ) const
{
    return m_aAtom.nOrderID < rAnimation.m_aAtom.nOrderID;
}
bool Ppt97Animation::operator > ( const Ppt97Animation& rAnimation ) const
{
    return m_aAtom.nOrderID > rAnimation.m_aAtom.nOrderID;
}
bool Ppt97Animation::HasEffect() const
{
    return m_aAtom.nBuildType != 0;
}
bool Ppt97Animation::HasParagraphEffect() const
{
    return m_aAtom.nBuildType > 1;
}
sal_Int32 Ppt97Animation::GetParagraphLevel() const
{
    sal_Int32 nParagraphLevel = 0;
    if(m_aAtom.nBuildType>1)
        nParagraphLevel = m_aAtom.nBuildType-1;
    return nParagraphLevel;
}
bool Ppt97Animation::HasSoundEffect() const
{
    return m_aAtom.nSoundRef && m_aAtom.nFlags & 0x0010;
}
bool Ppt97Animation::HasStopPreviousSound() const
{
    return m_aAtom.nFlags & 0x0040;
}
bool Ppt97Animation::HasReverseOrder() const
{
    return m_aAtom.nFlags & 0x001;
}
bool Ppt97Animation::HasAnimateAssociatedShape() const
{
    return m_aAtom.nFlags & 0x004000;
}
bool Ppt97Animation::HasAfterEffect() const
{
    return m_aAtom.nAfterEffect != 0;
}
bool Ppt97Animation::HasAfterEffect_ChangeColor() const
{
    return m_aAtom.nAfterEffect == 1;
}
bool Ppt97Animation::HasAfterEffect_DimAtNextEffect() const
{
    return m_aAtom.nAfterEffect == 2;
}
#ifdef FUTURE
bool Ppt97Animation::HasAfterEffect_DimAfterEffect() const
{
    return m_aAtom.nAfterEffect == 3;
}
#endif
void Ppt97Animation::SetSoundFileUrl( const OUString& rSoundFileUrl )
{
    m_aSoundFileUrl = rSoundFileUrl;
}

double Ppt97Animation::GetDelayTimeInSeconds() const
{
    return m_aAtom.nDelayTime != 0X7FFFFFFF ? m_aAtom.nDelayTime/1000.0 : 0.0;
}

bool Ppt97Animation::GetSpecialDuration( double& rfDurationInSeconds ) const
{
    UpdateCacheData();
    if( m_bHasSpecialDuration )
        rfDurationInSeconds = m_fDurationInSeconds;
    return m_bHasSpecialDuration;
}

bool Ppt97Animation::GetSpecialTextIterationDelay( double& rfTextIterationDelay ) const
{
    bool bRet = false;
    switch(this->GetTextAnimationType())
    {
    case presentation::TextAnimationType::BY_LETTER:
            rfTextIterationDelay = 0.075;
            bRet = true;
            break;
    case presentation::TextAnimationType::BY_WORD:
            rfTextIterationDelay = 0.3;
            bRet = true;
            break;
    default:
        break;
    }
    return bRet;
}

void Ppt97Animation::SetDimColor( sal_Int32 nDimColor )
{
    m_aAtom.nDimColor = nDimColor;
}
void Ppt97Animation::SetAnimateAssociatedShape( bool bAnimate )
{
    if( !bAnimate )
    {
        //the appear effect cannot be animated without text
        if( this->GetPresetId() == "ooo-entrance-appear" )
            return;
        //the random effect may be the appear effect and than has the same problem
        if( this->GetPresetId() == "ooo-entrance-random" )
        {
            //this case is not 100% correct -> feel free to complete
            //i consider this case as seldom and not that problematic and a simple correct fix is not in sight
            SAL_INFO("sd", "you tried to deselect the animation of the form for random animation-> this has been refused");
            return;
        }

    }

    if(bAnimate)
        m_aAtom.nFlags = m_aAtom.nFlags | 0x004000;
    else if( HasAnimateAssociatedShape() )
    {
        m_aAtom.nFlags = m_aAtom.nFlags ^ 0x004000;
    }
}

sal_Int16 Ppt97Animation::GetEffectNodeType() const //see css::presentation::EffectNodeType
{
    sal_Int16 nRet = presentation::EffectNodeType::ON_CLICK;
    if( m_aAtom.nFlags & 0x04 )
    {
        nRet = presentation::EffectNodeType::AFTER_PREVIOUS;
    }
    return nRet;
}

sal_Int16 Ppt97Animation::GetTextAnimationType() const
{
    sal_Int16 nRet = presentation::TextAnimationType::BY_PARAGRAPH;
    switch( m_aAtom.nSubEffect )
    {
    case 0:
        break;
    case 2:
        nRet = presentation::TextAnimationType::BY_LETTER;
        break;
    default:
        nRet = presentation::TextAnimationType::BY_WORD;
        break;
    }
    return nRet;
}
OUString const & Ppt97Animation::GetPresetId() const
{
    UpdateCacheData();
    return m_aPresetId;
}
OUString const & Ppt97Animation::GetPresetSubType() const
{
    UpdateCacheData();
    return m_aSubType;
}

void Ppt97Animation::ClearCacheData() const
{
    m_aPresetId.clear();
    m_aSubType.clear();
    m_bHasSpecialDuration = false;
    m_fDurationInSeconds = 0.001;
}
void Ppt97Animation::UpdateCacheData() const
{
    if( !m_bDirtyCache )
        return;

    ClearCacheData();

    if( !HasEffect() )
    {
        m_bDirtyCache = false;
        return;
    }

    switch( m_aAtom.nFlyMethod )
    {
        case 0x0:
            m_aPresetId = "ooo-entrance-appear";                  // --- appear ---
        break;
        case 0x01:
            m_aPresetId = "ooo-entrance-random";                  // --- random ---
        break;
        case 0x02:                                                                                  // --- blinds effect ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-venetian-blinds";
                    m_aSubType = "horizontal";                    // horizontal
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-venetian-blinds";
                    m_aSubType = "vertical";                      // vertical
                break;
            }
        }
        break;
        case 0x03:                                                                                  // --- (hor/ver) shifted appear ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-checkerboard";
                    m_aSubType = "across";                        // vertical ???
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-checkerboard";
                    m_aSubType = "downward";                      // horizontal ???
                break;
            }
        }
        break;
        case 0x05:
            m_aPresetId = "ooo-entrance-dissolve-in";
        break;
        case 0x08:                                                                                  // --- (hor/ver) lines ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-random-bars";
                    m_aSubType = "vertical";                      // horizontal ???
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-random-bars";
                    m_aSubType = "horizontal";                      // vertical ???
                break;
            }
        }
        break;
        case 0x09:                                                                                  // --- diagonal ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x4:
                    m_aPresetId = "ooo-entrance-diagonal-squares";
                    m_aSubType = "left-to-top";                     // to left top
                break;
                case 0x5:
                    m_aPresetId = "ooo-entrance-diagonal-squares";
                    m_aSubType = "right-to-top";                    // to right top
                break;
                case 0x6:
                    m_aPresetId = "ooo-entrance-diagonal-squares";
                    m_aSubType = "left-to-bottom";                  // to left bottom
                break;
                case 0x7:
                    m_aPresetId = "ooo-entrance-diagonal-squares";
                    m_aSubType = "right-to-bottom";                 // to right bottom
                break;
            }
        }
        break;
        case 0x0a:                                                                                  // --- roll/wipe ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-wipe";
                    m_aSubType = "from-right";                      // from right
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-wipe";
                    m_aSubType = "from-bottom";                     // from bottom
                break;
                case 0x2:
                    m_aPresetId = "ooo-entrance-wipe";
                    m_aSubType = "from-left";                       // from left
                break;
                case 0x3:
                    m_aPresetId = "ooo-entrance-wipe";
                    m_aSubType = "from-top";                        // from top
                break;
            }
        }
        break;
        case 0x0b:                                                                                  //--- fade in ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-box";
                    m_aSubType = "out";                             // from center
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-box";
                    m_aSubType = "in";                              // to center
                break;
            }
        }
        break;
        case 0x0c:                                                                                  // --- text effects ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-left";

                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-top";
                break;
                case 0x2:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-right";
                break;
                case 0x3:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-bottom";
                break;
                case 0x4:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-top-left";
                break;
                case 0x5:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-top-right";
                break;
                case 0x6:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-bottom-left";
                break;
                case 0x7:
                    m_aPresetId = "ooo-entrance-fly-in";
                    m_aSubType = "from-bottom-right";
                break;
                case 0x8:                                                                               // -- short text effects --
                    m_aPresetId = "ooo-entrance-peek-in";
                    m_aSubType = "from-left";
                break;
                case 0x9:
                    m_aPresetId = "ooo-entrance-peek-in";
                    m_aSubType = "from-bottom";
                break;
                case 0xa:
                    m_aPresetId = "ooo-entrance-peek-in";
                    m_aSubType = "from-right";
                break;
                case 0xb:
                    m_aPresetId = "ooo-entrance-peek-in";
                    m_aSubType = "from-top";
                break;
                case 0xc:                                                                               // -- slow text effects --
                {
                    m_aPresetId = "ooo-entrance-fly-in-slow";
                    m_aSubType = "from-left";
                }
                break;
                case 0xd:
                {
                    m_aPresetId = "ooo-entrance-fly-in-slow";
                    m_aSubType = "from-top";
                }
                break;
                case 0xe:
                {
                    m_aPresetId = "ooo-entrance-fly-in-slow";
                    m_aSubType = "from-right";
                }
                break;
                case 0xf:
                {
                    m_aPresetId = "ooo-entrance-fly-in-slow";
                    m_aSubType = "from-bottom";
                }
                break;
                case 0x10:                                                                              // --- zoom ---
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "in";
                break;
                case 0x11:
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "in-slightly";
                break;
                case 0x12:
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "out";
                break;
                case 0x13:
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "out-slightly";
                break;
                case 0x14:
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "in-from-screen-center";
                break;
                case 0x15:
                    m_aPresetId = "ooo-entrance-zoom";
                    m_aSubType = "out-from-screen-center";
                break;
                case 0x16:                                                                              // --- stretch ---
                    m_aPresetId = "ooo-entrance-stretchy";
                    m_aSubType = "across";
                break;
                case 0x17:
                    m_aPresetId = "ooo-entrance-stretchy";
                    m_aSubType = "from-left";
                break;
                case 0x18:
                    m_aPresetId = "ooo-entrance-stretchy";
                    m_aSubType = "from-top";
                break;
                case 0x19:
                    m_aPresetId = "ooo-entrance-stretchy";
                    m_aSubType = "from-right";
                break;
                case 0x1a:
                    m_aPresetId = "ooo-entrance-stretchy";
                    m_aSubType = "from-bottom";
                break;
                case 0x1b:                                                                              // --- rotate ---
                    m_aPresetId = "ooo-entrance-swivel";
                    m_aSubType = "vertical";
                break;
                case 0x1c:                                                                              // --- spirale ---
                    m_aPresetId = "ooo-entrance-spiral-in";
                break;
            }
        }
        break;
        case 0x0d:                                                                                      // --- open/close ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    m_aPresetId = "ooo-entrance-split";
                    m_aSubType = "horizontal-out";                      //horizontal open
                break;
                case 0x1:
                    m_aPresetId = "ooo-entrance-split";
                    m_aSubType = "horizontal-in";                       //horizontal close
                break;
                case 0x2:
                    m_aPresetId = "ooo-entrance-split";
                    m_aSubType = "vertical-out";                        // vertical open
                break;
                case 0x3:
                    m_aPresetId = "ooo-entrance-split";
                    m_aSubType = "vertical-in";                         // vertical close
                break;
            }
        }
        break;
        case 0x0e:                                                                                      // --- blink ---
        {
            m_aPresetId = "ooo-entrance-flash-once";
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0: //fast
                    m_fDurationInSeconds = 0.075;
                    m_bHasSpecialDuration = true;
                break;
                case 0x1: //medium
                    m_fDurationInSeconds = 0.5;
                    m_bHasSpecialDuration = true;
                break;
                case 0x2: //slow
                    m_fDurationInSeconds = 1.0;
                    m_bHasSpecialDuration = true;
                break;
            }
        }
        break;
        default:
        {
            m_aPresetId = "ooo-entrance-appear";
            OSL_FAIL("no effect mapped");
        }
        break;
    }
    m_bDirtyCache = false;
}

void Ppt97Animation::createAndSetCustomAnimationEffect( SdrObject* pObj )
{

    if( !this->HasEffect() )
        return;
    if( !pObj || !pObj->GetPage() )
    {
        OSL_FAIL("no valid SdrObject or page found for ppt import");
        return;
    }

    uno::Reference< drawing::XShape > xShape = GetXShapeForSdrObject( pObj );
    if( !xShape.is() )
    {
        OSL_FAIL("no XShape interface found for ppt import");
        return;
    }
    ::sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();
    if( !pMainSequence.get() )
    {
        OSL_FAIL("no MainSequence found for ppt import");
        return;
    }

    const ::sd::CustomAnimationPresets& rPresets( ::sd::CustomAnimationPresets::getCustomAnimationPresets() );
    ::sd::CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( this->GetPresetId() ) );
    if( !pPreset.get() )
    {
        OSL_FAIL("no suitable preset found for ppt import");
        return;
    }

    //--------------start doing something

    //1. ------ create an effect from the presets ------
    ::sd::CustomAnimationEffectPtr pEffect( new ::sd::CustomAnimationEffect( pPreset->create( this->GetPresetSubType() ) ) );
    if( !pEffect.get() )
    {
        DBG_ASSERT(pEffect.get(),"no suitable effect found");
        return;
    }

    //2. ------ adapt the created effect ------

    // set the shape targeted by this effect
    pEffect->setTarget( makeAny( xShape ) );

    pEffect->setBegin( this->GetDelayTimeInSeconds() );

    // some effects need a different duration than that of the mapped preset effect
    double fDurationInSeconds = 1.0; //in seconds
    if( this->GetSpecialDuration( fDurationInSeconds ) )
        pEffect->setDuration( fDurationInSeconds );

    // set after effect
    if( this->HasAfterEffect() )
    {
        pEffect->setHasAfterEffect( true );
        if( this->HasAfterEffect_ChangeColor() )
            pEffect->setDimColor( uno::makeAny( this->GetDimColor() ) );
        else
            pEffect->setAfterEffectOnNext( this->HasAfterEffect_DimAtNextEffect() );
    }

    // set sound effect
    if( this->HasSoundEffect() )
        pEffect->createAudio( uno::makeAny( m_aSoundFileUrl ) );

    // text iteration
    pEffect->setIterateType( this->GetTextAnimationType() );

    // some effects need a different delay between text iteration than that of the mapped preset effect
    double fTextIterationDelay = 1.0;
    if( this->GetSpecialTextIterationDelay( fTextIterationDelay ) )
        pEffect->setIterateInterval( fTextIterationDelay );

    // is the effect started on click or after the last effect (Another possible value is EffectNodeType::WITH_PREVIOUS )
    pEffect->setNodeType( this->GetEffectNodeType() );

    //set stop sound effect
    if( this->HasStopPreviousSound() )
        pEffect->setStopAudio();

    // append the effect to the main sequence
    if( !this->HasParagraphEffect() )
    {
        // TODO: !this->HasAnimateAssociatedShape() can possibly have this set to ONLY_TEXT - see i#42737
        pEffect->setTargetSubItem( presentation::ShapeAnimationSubType::AS_WHOLE );
    }

    //3. ------ put the created effect to the model and do some last changes fro paragraph effects ------
    pMainSequence->append( pEffect );
    if( this->HasParagraphEffect() )
    {
        sal_Int32 nParagraphLevel = this->GetParagraphLevel();
        double fDelaySeconds = this->GetDelayTimeInSeconds();
        bool bAnimateAssociatedShape = this->HasAnimateAssociatedShape();//or only text
        bool bTextReverse = this->HasReverseOrder();

        // now create effects for each paragraph
        ::sd::CustomAnimationTextGroupPtr pGroup = pMainSequence->
            createTextGroup( pEffect, nParagraphLevel, fDelaySeconds, bAnimateAssociatedShape, bTextReverse );

        if( pGroup )
        {
            const ::sd::EffectSequence& rEffects = pGroup->getEffects();
            ::sd::EffectSequence::const_iterator aIter = rEffects.begin();

            ::sd::CustomAnimationEffectPtr pLastEffect;
            sal_Int32 nIndex = 0;
            for( ; aIter != rEffects.end(); ++aIter )
            {
                ::sd::CustomAnimationEffectPtr pGroupEffect(*aIter);

                ////todo? if( nIndex > 1 && pLastEffect && this->HasSoundEffect() )
                ////          pLastEffect->setStopAudio();
                if( nIndex < 2  )
                {
                    pGroupEffect->setNodeType( this->GetEffectNodeType() );
                }
                else if( nIndex > 0 )
                {
                    bool bAtParagraphBegin = false;
                    if(!bTextReverse)
                        bAtParagraphBegin = pGroupEffect->getParaDepth() < nParagraphLevel;
                    else
                        bAtParagraphBegin = !pLastEffect || pLastEffect->getParaDepth() < nParagraphLevel;
                    if( bAtParagraphBegin )
                        pGroupEffect->setNodeType( this->GetEffectNodeType() );
                    else if( this->GetTextAnimationType() == presentation::TextAnimationType::BY_PARAGRAPH )
                        pGroupEffect->setNodeType( presentation::EffectNodeType::WITH_PREVIOUS );
                    else
                        pGroupEffect->setNodeType( presentation::EffectNodeType::AFTER_PREVIOUS );
                }
                pLastEffect = pGroupEffect;
                nIndex++;
            }
        }
    }
    pMainSequence->rebuild();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
