/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SD_ANMINFO_HXX
#define _SD_ANMINFO_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "pres.hxx"
#include "anmdef.hxx"
#include <svx/svdobj.hxx>
#include <tools/color.hxx>
#include <sddllapi.h>

class Polygon;
class Point;
class SvStream;
class SdrObjSurrogate;
class SdrObject;
class SdrPathObj;
class SdDrawDocument;

class SdAnimationInfo : public SdrObjUserData
{
public:
    PresObjKind             mePresObjKind;

    /* deprecated animation infos */
    ::com::sun::star::presentation::AnimationEffect         meEffect;       // Animationseffekt
    ::com::sun::star::presentation::AnimationEffect         meTextEffect;   // Animationseffekt fuer Textinhalt
    ::com::sun::star::presentation::AnimationSpeed          meSpeed;            // Geschwindigkeit der Animation
    bool                    mbActive;       // eingeschaltet ?
    bool                    mbDimPrevious;  // Objekt abblenden
    bool                    mbIsMovie;      // wenn Gruppenobjekt, dann Sequenz aus den
    bool                    mbDimHide;      // verstecken statt abblenden
    Color                   maBlueScreen;   // identifiziert "Hintergrundpixel"
    Color                   maDimColor;     // zum Abblenden des Objekts
    String                  maSoundFile;        // Pfad zum Soundfile in MSDOS-Notation
    bool                    mbSoundOn;      // Sound ein/aus
    bool                    mbPlayFull;     // Sound ganz abspielen
    SdrPathObj*             mpPathObj;      // das Pfadobjekt
    ::com::sun::star::presentation::ClickAction             meClickAction;  // Aktion bei Mausklick
    ::com::sun::star::presentation::AnimationEffect         meSecondEffect; // fuer Objekt ausblenden
    ::com::sun::star::presentation::AnimationSpeed          meSecondSpeed;  // fuer Objekt ausblenden
    String                  maSecondSoundFile; // fuer Objekt ausblenden
    bool                    mbSecondSoundOn;    // fuer Objekt ausblenden
    bool                    mbSecondPlayFull;// fuer Objekt ausblenden
//  String                  maBookmark;     // Sprung zu Objekt/Seite
    sal_uInt16                  mnVerb;         // fuer OLE-Objekt
    sal_uLong                   mnPresOrder;
    SdrObject&              mrObject;

    SD_DLLPUBLIC void                    SetBookmark( const String& rBookmark );
    SD_DLLPUBLIC String                  GetBookmark();
public:
                            SdAnimationInfo(SdrObject& rObject);
                            SdAnimationInfo(const SdAnimationInfo& rAnmInfo, SdrObject& rObject);
    virtual                 ~SdAnimationInfo();

    virtual SdrObjUserData* Clone(SdrObject* pObject) const;
};

#endif // _SD_ANMINFO_HXX

