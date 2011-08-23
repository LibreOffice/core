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

#ifndef _SD_ANMINFO_HXX
#define _SD_ANMINFO_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>

#include <bf_svx/svdobj.hxx>
#include <tools/color.hxx>
class Polygon;
class Point;
class SvStream;
namespace binfilter {

class SdrObjSurrogate;
class SdrObject;
class SdrPathObj;
class SdDrawDocument;

class SdAnimationInfo : public SdrObjUserData, public SfxListener
{
private:

    SdDrawDocument*			pDoc;

public:

    Polygon*				pPolygon; 		// fuer nichtlinearen Pfad (unbenutzt)
    Point					aStart;			// Startpunkt eines linearen Pfades (unbenutzt)
    Point					aEnd; 			// Endpunkt eines linearen Pfades (unbenutzt)
    ::com::sun::star::presentation::AnimationEffect			eEffect;		// Animationseffekt
    ::com::sun::star::presentation::AnimationEffect			eTextEffect;	// Animationseffekt fuer Textinhalt
    ::com::sun::star::presentation::AnimationSpeed			eSpeed;			// Geschwindigkeit der Animation
    BOOL					bActive;		// eingeschaltet ?
    BOOL					bDimPrevious;	// Objekt abblenden
    BOOL					bIsMovie; 		// wenn Gruppenobjekt, dann Sequenz aus den
    BOOL					bDimHide; 		// verstecken statt abblenden
    Color					aBlueScreen;	// identifiziert "Hintergrundpixel"
    Color					aDimColor;		// zum Abblenden des Objekts
    String					aSoundFile;		// Pfad zum Soundfile in MSDOS-Notation
    BOOL					bSoundOn; 		// Sound ein/aus
    BOOL					bPlayFull;		// Sound ganz abspielen
    SdrObjSurrogate*		pPathSuro;		// Surrogat fuer pPathObj
    SdrPathObj* 			pPathObj; 		// das Pfadobjekt
    ::com::sun::star::presentation::ClickAction 			eClickAction; 	// Aktion bei Mausklick
    ::com::sun::star::presentation::AnimationEffect			eSecondEffect;	// fuer Objekt ausblenden
    ::com::sun::star::presentation::AnimationSpeed			eSecondSpeed; 	// fuer Objekt ausblenden
    String					aSecondSoundFile; // fuer Objekt ausblenden
    BOOL					bSecondSoundOn;	// fuer Objekt ausblenden
    BOOL					bSecondPlayFull;// fuer Objekt ausblenden
    String					aBookmark;		// Sprung zu Objekt/Seite
    USHORT					nVerb;			// fuer OLE-Objekt
    BOOL					bInvisibleInPresentation;
    BOOL					bIsShown; 		// in der Show gerade sichtbar, NICHT PERSISTENT!
    BOOL					bShow;			// Befehl: mit 1. Effekt zeigen (TRUE)
                                            // oder mit 2. Effekt entfernen (FALSE)
                                            // NICHT PERSISTENT!
    BOOL					bDimmed;		// in der Show abgeblendet (TRUE) oder
                                            // nicht (TRUE)
                                            // NICHT PERSISTENT!
    ULONG					nPresOrder;

                            SdAnimationInfo(SdDrawDocument* pTheDoc);
                            SdAnimationInfo(const SdAnimationInfo& rAnmInfo);
    virtual					~SdAnimationInfo();

    virtual SdrObjUserData* Clone(SdrObject* pObj) const {DBG_BF_ASSERT(0, "STRIP"); return NULL;} //STRIP001 	virtual SdrObjUserData*	Clone(SdrObject* pObj) const;

    virtual void			WriteData(SvStream& rOut);
    virtual void			ReadData(SvStream& rIn);

            // NULL loest die Verbindung zum Pfadobjekt
    virtual void			AfterRead();
};

} //namespace binfilter
#endif // _SD_ANMINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
