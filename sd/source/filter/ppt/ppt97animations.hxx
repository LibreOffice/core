/*************************************************************************
 *
 *  $RCSfile: ppt97animations.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-29 14:18:31 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2005 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_PPT_97_ANIMATIONS_HXX
#define _SD_PPT_97_ANIMATIONS_HXX

// header for class SvStream
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

class SdrObject;
class Ppt97Animation;

// helper class for reading PPT AnimationInfoAtom
class Ppt97AnimationInfoAtom
{
    friend class Ppt97Animation;

//-- member
    UINT32          nDimColor;
    UINT32          nFlags;         // 0x0004: time instead of click
    UINT32          nSoundRef;
    INT32           nDelayTime;     // 1/1000 sec
    UINT16          nOrderID;
    UINT16          nSlideCount;
    UINT8           nBuildType;
    UINT8           nFlyMethod;
    UINT8           nFlyDirection;
    UINT8           nAfterEffect; //nAfterEffect: 0: none; 1: change color; 2: dim on next effect; 3: dim after effect;
    UINT8           nSubEffect;
    UINT8           nOLEVerb;

    // unknown, because whole size needs to be 28
    UINT8           nUnknown1;
    UINT8           nUnknown2;

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

class Ppt97Animation
{
    /** this is a helping class for import of PPT 97 animations
        1. use the constructor Ppt97Animation( SvStream& rIn ) to import informations from the stream
        2. use the set methods to modify and complete the data
        3. use the method createAndSetCustomAnimationEffect( ) to create an effect in sd model
    */

public: //public methods
    Ppt97Animation( SvStream& rIn );

    Ppt97Animation();
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
    UINT32 GetSoundRef() const;
    bool HasAnimateAssociatedShape() const; //true if the shape should be animated in addition to the text

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
    bool HasAfterEffect_DimAfterEffect() const;
    bool HasStopPreviousSound() const;
    bool HasReverseOrder() const; //true if the text paragraphs should be animated in reverse order
    sal_Int32 GetParagraphLevel() const; //paragraph level that is animated ( that paragraph and higher levels )
    sal_Int16 GetTextAnimationType() const; //see com::sun::star::presentation::TextAnimationType
    sal_Int16 GetEffectNodeType() const; //see com::sun::star::presentation::EffectNodeType
    double GetDelayTimeInSeconds() const;//-1 for start on mouseclick or >= 0 for a delay in seconds for automatic start
    bool GetSpecialDuration( double& rfDurationInSeconds ) const;
    bool GetSpecialTextIterationDelay( double& rfTextIterationDelay ) const;

    void UpdateCacheData() const;
    void ClearCacheData() const;

private: //private member
    //input information:
    Ppt97AnimationInfoAtom  m_aAtom;//pure input from stream
    ::rtl::OUString         m_aSoundFileUrl;//this needs to be set in addition from outside as this class has not the knowledge to translate the sound bits to a file url/

    //cached generated output information:
    mutable bool            m_bDirtyCache;
    mutable ::rtl::OUString m_aPresetId; // m_aPresetId and m_aSubType match to the values in sd/xml/effects.xml
    mutable ::rtl::OUString m_aSubType;
    mutable bool            m_bHasSpecialDuration;
    mutable double          m_fDurationInSeconds;
};

#endif
