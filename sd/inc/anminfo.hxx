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
    sal_Bool                    mbActive;       // eingeschaltet ?
    sal_Bool                    mbDimPrevious;  // Objekt abblenden
    sal_Bool                    mbIsMovie;      // wenn Gruppenobjekt, dann Sequenz aus den
    sal_Bool                    mbDimHide;      // verstecken statt abblenden
    Color                   maBlueScreen;   // identifiziert "Hintergrundpixel"
    Color                   maDimColor;     // zum Abblenden des Objekts
    String                  maSoundFile;        // Pfad zum Soundfile in MSDOS-Notation
    sal_Bool                    mbSoundOn;      // Sound ein/aus
    sal_Bool                    mbPlayFull;     // Sound ganz abspielen
    SdrPathObj*             mpPathObj;      // das Pfadobjekt
    ::com::sun::star::presentation::ClickAction             meClickAction;  // Aktion bei Mausklick
    ::com::sun::star::presentation::AnimationEffect         meSecondEffect; // fuer Objekt ausblenden
    ::com::sun::star::presentation::AnimationSpeed          meSecondSpeed;  // fuer Objekt ausblenden
    String                  maSecondSoundFile; // fuer Objekt ausblenden
    sal_Bool                    mbSecondSoundOn;    // fuer Objekt ausblenden
    sal_Bool                    mbSecondPlayFull;// fuer Objekt ausblenden
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

