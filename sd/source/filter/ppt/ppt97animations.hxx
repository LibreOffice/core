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

#ifndef INCLUDED_SD_SOURCE_FILTER_PPT_PPT97ANIMATIONS_HXX
#define INCLUDED_SD_SOURCE_FILTER_PPT_PPT97ANIMATIONS_HXX

#include <rtl/ustring.hxx>
#include <sal/types.h>

class SdrObject;
class Ppt97Animation;
class SvStream;

/// helper class for reading PPT AnimationInfoAtom
class Ppt97AnimationInfoAtom
{
    friend class Ppt97Animation;

//-- member
    sal_uInt32          nDimColor;
    sal_uInt32          nFlags;         ///< 0x0004: time instead of click
    sal_uInt32          nSoundRef;
    sal_Int32           nDelayTime;     ///< 1/1000 sec
    sal_uInt16          nOrderID;
    sal_uInt16          nSlideCount;
    sal_uInt8           nBuildType;
    sal_uInt8           nFlyMethod;
    sal_uInt8           nFlyDirection;
    sal_uInt8           nAfterEffect;   ///< nAfterEffect: 0: none; 1: change color; 2: dim on next effect; 3: dim after effect;
    sal_uInt8           nSubEffect;
    sal_uInt8           nOLEVerb;

    // unknown, because whole size needs to be 28
    sal_uInt8           nUnknown1;
    sal_uInt8           nUnknown2;

//-- methods
    void ReadStream( SvStream& rIn );
/*
    nFlags:
    decimal / hexadecimal / binary
    1040 0x00000410     10000010000 mouseclick
   17428 0x00004414 100010000010100 after previous 0 sec (animate form)
   17412 0x00004404 100010000000100 after previous 0 sec
    1088 0x00000440     10001000000 stop previous sound and mouseclick
    1044 0x00000414     10000010100 play sound automatic
    1041 0x00000411     10000010001
                    |   |   | | | |
                    |   |   | | | reverse order
                    |   |   | | after previous
                    |   |   | sound
                    |   |   stop previous sound
                    |   ?
                    animate form

    nAfterEffect:
    1: color
    0: nothing
    3: hide after animation
    2: hide at next mouse click
*/
};

/** this is a helping class for import of PPT 97 animations
        1. use the constructor Ppt97Animation( SvStream& rIn ) to import information from the stream
        2. use the set methods to modify and complete the data
        3. use the method createAndSetCustomAnimationEffect( ) to create an effect in sd model
    */
class Ppt97Animation
{

public: //public methods
    explicit Ppt97Animation( SvStream& rIn );

    Ppt97Animation( const Ppt97Animation& rAnimation );
    Ppt97Animation& operator= ( const Ppt97Animation& rAnimation );
    bool operator < ( const Ppt97Animation& rAnimation ) const;//later is greater
    bool operator > ( const Ppt97Animation& rAnimation ) const;//later is greater
    ~Ppt97Animation();

    //get methods
    bool HasEffect() const;
    bool HasParagraphEffect() const;
    bool HasSoundEffect() const;
    sal_Int32 GetDimColor() const { return static_cast<sal_Int32>(m_aAtom.nDimColor);}
    sal_uInt32 GetSoundRef() const { return m_aAtom.nSoundRef;}
    /// @return true if the shape should be animated in addition to the text
    bool HasAnimateAssociatedShape() const;

    //set methods
    void SetDimColor( sal_Int32 nDimColor );
    void SetSoundFileUrl( const OUString& rSoundFileUrl );
    void SetAnimateAssociatedShape( bool bAnimate ); //true if the shape should be animated in addition to the text

    //action methods
    /** this method creates a CustomAnimationEffect for the given SdrObject
    from internal data and stores the created effect at the draw model
    */
    void createAndSetCustomAnimationEffect( SdrObject* pObj );

private: //private methods

    //read methods
    OUString const & GetPresetId() const;
    OUString const & GetPresetSubType() const;
    bool HasAfterEffect() const;
    bool HasAfterEffect_ChangeColor() const;
    bool HasAfterEffect_DimAtNextEffect() const;
    bool HasStopPreviousSound() const;

    /// @return true if the text paragraphs should be animated in reverse order
    bool HasReverseOrder() const;

    ///paragraph level that is animated ( that paragraph and higher levels )
    sal_Int32 GetParagraphLevel() const;

    ///@see css::presentation::TextAnimationType
    sal_Int16 GetTextAnimationType() const;

    ///@see css::presentation::EffectNodeType
    sal_Int16 GetEffectNodeType() const;

    /// @return -1 for start on mouseclick or >= 0 for a delay in seconds for automatic start
    double GetDelayTimeInSeconds() const;
    bool GetSpecialDuration( double& rfDurationInSeconds ) const;
    bool GetSpecialTextIterationDelay( double& rfTextIterationDelay ) const;

    void UpdateCacheData() const;
    void ClearCacheData() const;

private: //private member
    //input information:
    Ppt97AnimationInfoAtom  m_aAtom;         ///< pure input from stream
    OUString         m_aSoundFileUrl; ///< this needs to be set in addition from outside as this class has not the knowledge to translate the sound bits to a file url

    //cached generated output information:
    mutable bool            m_bDirtyCache;
    mutable OUString m_aPresetId; // m_aPresetId and m_aSubType match to the values in sd/xml/effects.xml
    mutable OUString m_aSubType;
    mutable bool            m_bHasSpecialDuration;
    mutable double          m_fDurationInSeconds;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
