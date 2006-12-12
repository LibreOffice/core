/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anminfo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:22:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SD_ANMINFO_HXX
#define _SD_ANMINFO_HXX

#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONEFFECT_HPP_
#include <com/sun/star/presentation/AnimationEffect.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _SD_ANMDEF_HXX
#include "anmdef.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

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
    BOOL                    mbActive;       // eingeschaltet ?
    BOOL                    mbDimPrevious;  // Objekt abblenden
    BOOL                    mbIsMovie;      // wenn Gruppenobjekt, dann Sequenz aus den
    BOOL                    mbDimHide;      // verstecken statt abblenden
    Color                   maBlueScreen;   // identifiziert "Hintergrundpixel"
    Color                   maDimColor;     // zum Abblenden des Objekts
    String                  maSoundFile;        // Pfad zum Soundfile in MSDOS-Notation
    BOOL                    mbSoundOn;      // Sound ein/aus
    BOOL                    mbPlayFull;     // Sound ganz abspielen
    SdrPathObj*             mpPathObj;      // das Pfadobjekt
    ::com::sun::star::presentation::ClickAction             meClickAction;  // Aktion bei Mausklick
    ::com::sun::star::presentation::AnimationEffect         meSecondEffect; // fuer Objekt ausblenden
    ::com::sun::star::presentation::AnimationSpeed          meSecondSpeed;  // fuer Objekt ausblenden
    String                  maSecondSoundFile; // fuer Objekt ausblenden
    BOOL                    mbSecondSoundOn;    // fuer Objekt ausblenden
    BOOL                    mbSecondPlayFull;// fuer Objekt ausblenden
    String                  maBookmark;     // Sprung zu Objekt/Seite
    USHORT                  mnVerb;         // fuer OLE-Objekt
    ULONG                   mnPresOrder;

public:
                            SdAnimationInfo();
                            SdAnimationInfo(const SdAnimationInfo& rAnmInfo);
    virtual                 ~SdAnimationInfo();

    virtual SdrObjUserData* Clone(SdrObject* pObj) const;
};

#endif // _SD_ANMINFO_HXX

