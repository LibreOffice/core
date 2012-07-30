/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SD_PPT_97_ANIMATIONS_HXX
#define _SD_PPT_97_ANIMATIONS_HXX

// header for class SvStream
#include <tools/stream.hxx>

class SdrObject;
class Ppt97Animation;

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
        1. use the constructor Ppt97Animation( SvStream& rIn ) to import informations from the stream
        2. use the set methods to modify and complete the data
        3. use the method createAndSetCustomAnimationEffect( ) to create an effect in sd model
    */
class Ppt97Animation
{

public: //public methods
    Ppt97Animation( SvStream& rIn );

    Ppt97Animation( const Ppt97Animation& rAnimation );
    Ppt97Animation& operator= ( const Ppt97Animation& rAnimation );
    bool operator < ( const Ppt97Animation& rAnimation ) const;//later is greater
    bool operator > ( const Ppt97Animation& rAnimation ) const;//later is greater
    ~Ppt97Animation();

    //get methods
    bool HasEffect() const;
    bool HasParagraphEffect() const;
    bool HasSoundEffect() const;
    sal_Int32 GetDimColor() const;
    sal_uInt32 GetSoundRef() const;
    /// @return true if the shape should be animated in addition to the text
    bool HasAnimateAssociatedShape() const;

    //set methods
    void SetDimColor( sal_Int32 nDimColor );
    void SetSoundFileUrl( const ::rtl::OUString& rSoundFileUrl );
    void SetAnimateAssociatedShape( bool bAnimate ); //true if the shape should be animated in addition to the text

    //action methods
    /** this method creates a CustomAnimationEffect for the given SdrObject
    from internal data and stores the created effect at the draw model
    */
    void createAndSetCustomAnimationEffect( SdrObject* pObj );

private: //private methods

    //read methods
    ::rtl::OUString GetPresetId() const;
    ::rtl::OUString GetPresetSubType() const;
    bool HasAfterEffect() const;
    bool HasAfterEffect_ChangeColor() const;
    bool HasAfterEffect_DimAtNextEffect() const;
    bool HasStopPreviousSound() const;

    /// @return true if the text paragraphs should be animated in reverse order
    bool HasReverseOrder() const;

    ///paragraph level that is animated ( that paragraph and higher levels )
    sal_Int32 GetParagraphLevel() const;

    ///@see com::sun::star::presentation::TextAnimationType
    sal_Int16 GetTextAnimationType() const;

    ///@see com::sun::star::presentation::EffectNodeType
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
    ::rtl::OUString         m_aSoundFileUrl; ///< this needs to be set in addition from outside as this class has not the knowledge to translate the sound bits to a file url

    //cached generated output information:
    mutable bool            m_bDirtyCache;
    mutable ::rtl::OUString m_aPresetId; // m_aPresetId and m_aSubType match to the values in sd/xml/effects.xml
    mutable ::rtl::OUString m_aSubType;
    mutable bool            m_bHasSpecialDuration;
    mutable double          m_fDurationInSeconds;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
