/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "ppt97animations.hxx"

// header for class SdrObject
#include <svx/svdobj.hxx>
// header for class SdPage
#include "sdpage.hxx"
// header for define DBG_ERROR
#include <tools/debug.hxx>
// header for define GetXShapeForSdrObject
#include <svx/unoapi.hxx>
#include "EffectMigration.hxx"
#include <CustomAnimationPreset.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/presentation/TextAnimationType.hpp>
#include <com/sun/star/presentation/EffectNodeType.hpp>
#include <com/sun/star/presentation/ShapeAnimationSubType.hpp>

using namespace ::com::sun::star;

//---------------------------------------------------------------------------------------

void Ppt97AnimationInfoAtom::ReadStream( SvStream& rIn )
{
    rIn >> nDimColor;
    rIn >> nFlags;
    rIn >> nSoundRef;
    rIn >> nDelayTime;
    rIn >> nOrderID;
    rIn >> nSlideCount;
    rIn >> nBuildType;
    rIn >> nFlyMethod;
    rIn >> nFlyDirection;
    rIn >> nAfterEffect;
    rIn >> nSubEffect;
    rIn >> nOLEVerb;
    rIn >> nUnknown1;
    rIn >> nUnknown2;
}

//---------------------------------------------------------------------------------------

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
sal_uInt32 Ppt97Animation::GetSoundRef() const
{
    return m_aAtom.nSoundRef;
}
void Ppt97Animation::SetSoundFileUrl( const ::rtl::OUString& rSoundFileUrl )
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

sal_Int32 Ppt97Animation::GetDimColor() const
{
    return static_cast<sal_Int32>(m_aAtom.nDimColor);
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
        if( this->GetPresetId().equals( ::rtl::OUString::createFromAscii("ooo-entrance-appear") ) )
            return;
        //the random effect may be the appear effect and than has the same problem
        if( this->GetPresetId().equals( ::rtl::OUString::createFromAscii("ooo-entrance-random") ) )
        {
            //this case is not 100% correct -> feel free to complete
            //i consider this case as seldom and not that problematic and a simple correct fix is not in sight
            DBG_WARNING("you tried to deselect the animation of the form for random animation-> this has been refused");
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

sal_Int16 Ppt97Animation::GetEffectNodeType() const //see com::sun::star::presentation::EffectNodeType
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
::rtl::OUString Ppt97Animation::GetPresetId() const
{
    UpdateCacheData();
    return m_aPresetId;
}
::rtl::OUString Ppt97Animation::GetPresetSubType() const
{
    UpdateCacheData();
    return m_aSubType;
}

void Ppt97Animation::ClearCacheData() const
{
    m_aPresetId = m_aSubType = rtl::OUString();
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
            //eRetval = ::com::sun::star::presentation::AnimationEffect_APPEAR;
            m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-appear");                  // --- appear ---
        break;
        case 0x01:
            //eRetval = ::com::sun::star::presentation::AnimationEffect_RANDOM;
            m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-random");                  // --- random ---
        break;
        case 0x02:                                                                                  // --- blinds effect ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_STRIPES;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-venetian-blinds");
                    m_aSubType = ::rtl::OUString::createFromAscii("horizontal");                    // horizontal
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRIPES;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-venetian-blinds");
                    m_aSubType = ::rtl::OUString::createFromAscii("vertical");                      // vertical
                break;
            }
        }
        break;
        case 0x03:                                                                                  // --- (hor/ver) shifted appear ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_CHECKERBOARD;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-checkerboard");
                    m_aSubType = ::rtl::OUString::createFromAscii("across");                        // vertical ???
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_CHECKERBOARD;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-checkerboard");
                    m_aSubType = ::rtl::OUString::createFromAscii("downward");                      // horizontal ???
                break;
            }
        }
        break;
        case 0x05:
            //eRetval = ::com::sun::star::presentation::AnimationEffect_DISSOLVE;                   // --- dissolve ----
            m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-dissolve-in");
        break;
        case 0x08:                                                                                  // --- (hor/ver) lines ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_LINES;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-random-bars");
                    m_aSubType = ::rtl::OUString::createFromAscii("vertical");                      // horizontal ???
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_VERTICAL_LINES;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-random-bars");
                    m_aSubType = rtl::OUString::createFromAscii("horizontal");                      // vertical ???
                break;
            }
        }
        break;
        case 0x09:                                                                                  // --- diagonal ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x4:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERRIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-diagonal-squares");
                    m_aSubType = rtl::OUString::createFromAscii("left-to-top");                     // to left top
                break;
                case 0x5:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LOWERLEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-diagonal-squares");
                    m_aSubType = rtl::OUString::createFromAscii("right-to-top");                    // to right top
                break;
                case 0x6:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERRIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-diagonal-squares");
                    m_aSubType = rtl::OUString::createFromAscii("left-to-bottom");                  // to left bottom
                break;
                case 0x7:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_UPPERLEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-diagonal-squares");
                    m_aSubType = rtl::OUString::createFromAscii("right-to-bottom");                 // to right bottom
                break;
            }
        }
        break;
        case 0x0a:                                                                                  // --- roll/wipe ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_RIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-wipe");
                    m_aSubType = rtl::OUString::createFromAscii("from-right");                      // from right
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_BOTTOM;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-wipe");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom");                     // from bottom
                break;
                case 0x2:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_LEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-wipe");
                    m_aSubType = rtl::OUString::createFromAscii("from-left");                       // from left
                break;
                case 0x3:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_TOP;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-wipe");
                    m_aSubType = rtl::OUString::createFromAscii("from-top");                        // from top
                break;
            }
        }
        break;
        case 0x0b:                                                                                  //--- fade in ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-box");
                    m_aSubType = rtl::OUString::createFromAscii("out");                             // from center
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-box");
                    m_aSubType = rtl::OUString::createFromAscii("in");                              // to center
                break;
            }
        }
        break;
        case 0x0c:                                                                                  // --- text effects ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-left");

                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-top");
                break;
                case 0x2:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-right");
                break;
                case 0x3:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom");
                break;
                case 0x4:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERLEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-top-left");
                break;
                case 0x5:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_UPPERRIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-top-right");
                break;
                case 0x6:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERLEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom-left");
                break;
                case 0x7:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LOWERRIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom-right");
                break;
                case 0x8:                                                                               // -- short text effects --
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_LEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-peek-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-left");
                break;
                case 0x9:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_BOTTOM;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-peek-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom");
                break;
                case 0xa:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_RIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-peek-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-right");
                break;
                case 0xb:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_SHORT_FROM_TOP;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-peek-in");
                    m_aSubType = rtl::OUString::createFromAscii("from-top");
                break;
                case 0xc:                                                                               // -- slow text effects --
                {
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_LEFT;
                    //rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in-slow");
                    m_aSubType = rtl::OUString::createFromAscii("from-left");
                }
                break;
                case 0xd:
                {
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_TOP;
                    //rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in-slow");
                    m_aSubType = rtl::OUString::createFromAscii("from-top");
                }
                break;
                case 0xe:
                {
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_RIGHT;
                    //rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in-slow");
                    m_aSubType = rtl::OUString::createFromAscii("from-right");
                }
                break;
                case 0xf:
                {
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_MOVE_FROM_BOTTOM;
                    //rSpeed = ::com::sun::star::presentation::AnimationSpeed_SLOW;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-fly-in-slow");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom");
                }
                break;
                case 0x10:                                                                              // --- zoom ---
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("in");
                break;
                case 0x11:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("in-slightly");
                break;
                case 0x12:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("out");
                break;
                case 0x13:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("out-slightly");
                break;
                case 0x14:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_FROM_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("in-from-screen-center");
                break;
                case 0x15:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_FADE_TO_CENTER;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-zoom");
                    m_aSubType = rtl::OUString::createFromAscii("out-from-screen-center");
                break;
                case 0x16:                                                                              // --- stretch ---
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_STRETCH;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-stretchy");
                    m_aSubType = rtl::OUString::createFromAscii("across");
                break;
                case 0x17:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_LEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-stretchy");
                    m_aSubType = rtl::OUString::createFromAscii("from-left");
                break;
                case 0x18:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_TOP;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-stretchy");
                    m_aSubType = rtl::OUString::createFromAscii("from-top");
                break;
                case 0x19:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_RIGHT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-stretchy");
                    m_aSubType = rtl::OUString::createFromAscii("from-right");
                break;
                case 0x1a:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_STRETCH_FROM_BOTTOM;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-stretchy");
                    m_aSubType = rtl::OUString::createFromAscii("from-bottom");
                break;
                case 0x1b:                                                                              // --- rotate ---
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_HORIZONTAL_ROTATE;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-swivel");
                    m_aSubType = rtl::OUString::createFromAscii("vertical");
                break;
                case 0x1c:                                                                              // --- spirale ---
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_SPIRALOUT_LEFT;
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-spiral-in");
                break;
            }
        }
        break;
        case 0x0d:                                                                                      // --- open/close ---
        {
            switch ( m_aAtom.nFlyDirection )
            {
                case 0x0:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_OPEN_VERTICAL   ;       // ???
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-split");
                    m_aSubType = rtl::OUString::createFromAscii("horizontal-out");                      //horizontal open
                break;
                case 0x1:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_CLOSE_VERTICAL;         // ???
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-split");
                    m_aSubType = rtl::OUString::createFromAscii("horizontal-in");                       //horizontal close
                break;
                case 0x2:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_OPEN_HORIZONTAL;        // ???
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-split");
                    m_aSubType = rtl::OUString::createFromAscii("vertical-out");                        // vertical open
                break;
                case 0x3:
                    //eRetval = ::com::sun::star::presentation::AnimationEffect_CLOSE_HORIZONTAL;       // ???
                    m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-split");
                    m_aSubType = rtl::OUString::createFromAscii("vertical-in");                         // vertical close
                break;
            }
        }
        break;
        case 0x0e:                                                                                      // --- blink ---
        {
            m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-flash-once");
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
            m_aPresetId = ::rtl::OUString::createFromAscii("ooo-entrance-appear");
            DBG_ERROR("no effect mapped");
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
        DBG_ERROR("no valid SdrObject or page found for ppt import");
        return;
    }

    uno::Reference< drawing::XShape > xShape = GetXShapeForSdrObject( pObj );
    if( !xShape.is() )
    {
        DBG_ERROR("no XShape interface found for ppt import");
        return;
    }
    ::sd::MainSequencePtr pMainSequence = static_cast<SdPage*>(pObj->GetPage())->getMainSequence();
    if( !pMainSequence.get() )
    {
        DBG_ERROR("no MainSequence found for ppt import");
        return;
    }

    const ::sd::CustomAnimationPresets& rPresets( ::sd::CustomAnimationPresets::getCustomAnimationPresets() );
    ::sd::CustomAnimationPresetPtr pPreset( rPresets.getEffectDescriptor( this->GetPresetId() ) );
    if( !pPreset.get() )
    {
        DBG_ERROR("no suiteable preset found for ppt import");
        return;
    }

    //--------------start doing something

    //1. ------ create an effect from the presets ------
    ::sd::CustomAnimationEffectPtr pEffect( new ::sd::CustomAnimationEffect( pPreset->create( this->GetPresetSubType() ) ) );
    if( !pEffect.get() )
    {
        DBG_ASSERT(pEffect.get(),"no suiteable effect found");
        return;
    }

    //2. ------ adapt the created effect ------

    // set the shape targeted by this effect
    pEffect->setTarget( makeAny( xShape ) );

    pEffect->setBegin( this->GetDelayTimeInSeconds() );

    // some effects need a different duration than that of the mapped preset effect
    double fDurationInSeconds = 1.0;//in secunden
    if( this->GetSpecialDuration( fDurationInSeconds ) )
        pEffect->setDuration( fDurationInSeconds );

    // set after effect
    if( this->HasAfterEffect() )
    {
        pEffect->setHasAfterEffect( sal_True );
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
        if( this->HasAnimateAssociatedShape() )
            pEffect->setTargetSubItem( presentation::ShapeAnimationSubType::AS_WHOLE );
        else
            pEffect->setTargetSubItem( presentation::ShapeAnimationSubType::AS_WHOLE ); //todo: set ONLY_TEXT again if that is fixed
            //pEffect->setTargetSubItem( presentation::ShapeAnimationSubType::ONLY_TEXT );
    }

    //3. ------ put the created effect to the model and do some last changes fro paragraph effects ------
    pMainSequence->append( pEffect );
    if( this->HasParagraphEffect() )
    {
        sal_Int32 nParagraphLevel = this->GetParagraphLevel();
        double fDelaySeconds = this->GetDelayTimeInSeconds();
        sal_Bool bAnimateAssociatedShape = this->HasAnimateAssociatedShape();//or only text
        sal_Bool bTextReverse = this->HasReverseOrder();

        // now create effects for each paragraph
        ::sd::CustomAnimationTextGroupPtr pGroup = pMainSequence->
            createTextGroup( pEffect, nParagraphLevel, fDelaySeconds, bAnimateAssociatedShape, bTextReverse );

        if( pGroup )
        {
            const ::sd::EffectSequence& rEffects = pGroup->getEffects();
            ::sd::EffectSequence::const_iterator aIter = rEffects.begin();

            ::sd::CustomAnimationEffectPtr pLastEffect;
            sal_Int32 nIndex = 0;
            for( ; aIter != rEffects.end(); aIter++ )
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
