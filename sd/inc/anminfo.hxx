/*************************************************************************
 *
 *  $RCSfile: anminfo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:27 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _SD_ANMDEF_HXX
#include "anmdef.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif

class Polygon;
class Point;
class SvStream;
class SdrObjSurrogate;
class SdrObject;
class SdrPathObj;
class SdDrawDocument;


class SdAnimationInfo : public SdrObjUserData, public SfxListener
{
private:

    SdDrawDocument*         pDoc;

public:

    Polygon*                pPolygon;       // fuer nichtlinearen Pfad (unbenutzt)
    Point                   aStart;         // Startpunkt eines linearen Pfades (unbenutzt)
    Point                   aEnd;           // Endpunkt eines linearen Pfades (unbenutzt)
    ::com::sun::star::presentation::AnimationEffect         eEffect;        // Animationseffekt
    ::com::sun::star::presentation::AnimationEffect         eTextEffect;    // Animationseffekt fuer Textinhalt
    ::com::sun::star::presentation::AnimationSpeed          eSpeed;         // Geschwindigkeit der Animation
    BOOL                    bActive;        // eingeschaltet ?
    BOOL                    bDimPrevious;   // Objekt abblenden
    BOOL                    bIsMovie;       // wenn Gruppenobjekt, dann Sequenz aus den
    BOOL                    bDimHide;       // verstecken statt abblenden
    Color                   aBlueScreen;    // identifiziert "Hintergrundpixel"
    Color                   aDimColor;      // zum Abblenden des Objekts
    String                  aSoundFile;     // Pfad zum Soundfile in MSDOS-Notation
    BOOL                    bSoundOn;       // Sound ein/aus
    BOOL                    bPlayFull;      // Sound ganz abspielen
    SdrObjSurrogate*        pPathSuro;      // Surrogat fuer pPathObj
    SdrPathObj*             pPathObj;       // das Pfadobjekt
    ::com::sun::star::presentation::ClickAction             eClickAction;   // Aktion bei Mausklick
    ::com::sun::star::presentation::AnimationEffect         eSecondEffect;  // fuer Objekt ausblenden
    ::com::sun::star::presentation::AnimationSpeed          eSecondSpeed;   // fuer Objekt ausblenden
    String                  aSecondSoundFile; // fuer Objekt ausblenden
    BOOL                    bSecondSoundOn; // fuer Objekt ausblenden
    BOOL                    bSecondPlayFull;// fuer Objekt ausblenden
    String                  aBookmark;      // Sprung zu Objekt/Seite
    USHORT                  nVerb;          // fuer OLE-Objekt
    BOOL                    bInvisibleInPresentation;
    BOOL                    bIsShown;       // in der Show gerade sichtbar, NICHT PERSISTENT!
    BOOL                    bShow;          // Befehl: mit 1. Effekt zeigen (TRUE)
                                            // oder mit 2. Effekt entfernen (FALSE)
                                            // NICHT PERSISTENT!
    BOOL                    bDimmed;        // in der Show abgeblendet (TRUE) oder
                                            // nicht (TRUE)
                                            // NICHT PERSISTENT!
    ULONG                   nPresOrder;

                            SdAnimationInfo(SdDrawDocument* pTheDoc);
                            SdAnimationInfo(const SdAnimationInfo& rAnmInfo);
    virtual                 ~SdAnimationInfo();

    virtual SdrObjUserData* Clone(SdrObject* pObj) const;

    virtual void            WriteData(SvStream& rOut);
    virtual void            ReadData(SvStream& rIn);

            // NULL loest die Verbindung zum Pfadobjekt
            void            SetPath(SdrPathObj* pPath = NULL);
    virtual void            SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void            AfterRead();
};

#endif // _SD_ANMINFO_HXX

