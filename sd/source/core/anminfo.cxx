/*************************************************************************
 *
 *  $RCSfile: anminfo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 16:41:38 $
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#include "svx/xtable.hxx"
#ifndef _SVDSURO_HXX //autogen
#include <svx/svdsuro.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#include <svtools/urihelper.hxx>

#pragma hdrstop

#include "anminfo.hxx"
#include "glob.hxx"
#include "sdiocmpt.hxx"
#include "drawdoc.hxx"

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Konstruktor   (String aSecondSoundFile unbenutzt --> default ctor)
|*
\************************************************************************/

SdAnimationInfo::SdAnimationInfo(SdDrawDocument* pTheDoc)
               : SdrObjUserData(SdUDInventor, SD_ANIMATIONINFO_ID, 0),
                 pPolygon                   (NULL),
                 eEffect                    (presentation::AnimationEffect_NONE),
                 eTextEffect                (presentation::AnimationEffect_NONE),
                 eSpeed                     (presentation::AnimationSpeed_SLOW),
                 bActive                    (TRUE),
                 bDimPrevious               (FALSE),
                 bIsMovie                   (FALSE),
                 bDimHide                   (FALSE),
                 bSoundOn                   (FALSE),
                 bPlayFull                  (FALSE),
                 pPathSuro                  (NULL),
                 pPathObj                   (NULL),
                 eClickAction               (presentation::ClickAction_NONE),
                 eSecondEffect              (presentation::AnimationEffect_NONE),
                 eSecondSpeed               (presentation::AnimationSpeed_SLOW),
                 bSecondSoundOn             (FALSE),
                 bSecondPlayFull            (FALSE),
                 bInvisibleInPresentation   (FALSE),
                 nVerb                      (0),
                 pDoc                       (pTheDoc),
                 bShow                      (TRUE),
                 bIsShown                   (TRUE),
                 bDimmed                    (FALSE),
                 nPresOrder                 (LIST_APPEND)
{
    aBlueScreen = RGB_Color(COL_LIGHTMAGENTA);
    aDimColor = RGB_Color(COL_LIGHTGRAY);
}

/*************************************************************************
|*
|* Copy Konstruktor
|* einige Anteile koennen nicht kopiert werden, da sie Referenzen in ein
|* bestimmtes Model bilden
|*
\************************************************************************/

SdAnimationInfo::SdAnimationInfo(const SdAnimationInfo& rAnmInfo)
               : SdrObjUserData             (rAnmInfo),
                 pPolygon                   (NULL),
                 aStart                     (rAnmInfo.aStart),
                 aEnd                       (rAnmInfo.aEnd),
                 eEffect                    (rAnmInfo.eEffect),
                 eTextEffect                (rAnmInfo.eTextEffect),
                 eSpeed                     (rAnmInfo.eSpeed),
                 bActive                    (rAnmInfo.bActive),
                 bDimPrevious               (rAnmInfo.bDimPrevious),
                 bIsMovie                   (rAnmInfo.bIsMovie),
                 bDimHide                   (rAnmInfo.bDimHide),
                 aBlueScreen                (rAnmInfo.aBlueScreen),
                 aDimColor                  (rAnmInfo.aDimColor),
                 aSoundFile                 (rAnmInfo.aSoundFile),
                 bSoundOn                   (rAnmInfo.bSoundOn),
                 bPlayFull                  (rAnmInfo.bPlayFull),
                 pPathObj                   (NULL),
                 pPathSuro                  (NULL),
                 eClickAction               (rAnmInfo.eClickAction),
                 eSecondEffect              (rAnmInfo.eSecondEffect),
                 eSecondSpeed               (rAnmInfo.eSecondSpeed),
                 bSecondSoundOn             (rAnmInfo.bSecondSoundOn),
                 bSecondPlayFull            (rAnmInfo.bSecondPlayFull),
                 bInvisibleInPresentation   (rAnmInfo.bInvisibleInPresentation),
                 nVerb                      (rAnmInfo.nVerb),
                 aBookmark                  (rAnmInfo.aBookmark),
                 aSecondSoundFile           (rAnmInfo.aSecondSoundFile),
                 pDoc                       (NULL),
                 bShow                      (rAnmInfo.bShow),
                 bIsShown                   (rAnmInfo.bIsShown),
                 bDimmed                    (rAnmInfo.bDimmed),
                 nPresOrder                 (LIST_APPEND)
{
    if (pPolygon)
        pPolygon = new Polygon(*(rAnmInfo.pPolygon));

    // kann nicht uebertragen werden
    if (eEffect == presentation::AnimationEffect_PATH)
        eEffect =  presentation::AnimationEffect_NONE;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdAnimationInfo::~SdAnimationInfo()
{
    delete pPathSuro;
    delete pPolygon;
}

/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrObjUserData* __EXPORT SdAnimationInfo::Clone(SdrObject* pObj) const
{
    return new SdAnimationInfo(*this);
}


/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

void SdAnimationInfo::WriteData(SvStream& rOut)
{
    SdrObjUserData::WriteData(rOut);

            // letzter Parameter ist die aktuelle Versionsnummer des Codes
    SdIOCompat aIO(rOut, STREAM_WRITE, 9);

    if(pPolygon)
    {
        rOut << (UINT16)1;  // es folgt ein Polygon
        rOut << *pPolygon;
    }
    else
        rOut << (UINT16)0;  // kein Polygon

    rOut << aStart;
    rOut << aEnd;
    rOut << (UINT16)eEffect;
    rOut << (UINT16)eSpeed;

    rOut << (UINT16)bActive;
    rOut << (UINT16)bDimPrevious;
    rOut << (UINT16)bIsMovie;

    rOut << aBlueScreen;
    rOut << aDimColor;

    rtl_TextEncoding eSysEnc = ::GetStoreCharSet( gsl_getSystemTextEncoding() );
    rOut << (INT16) eSysEnc;
    rOut.WriteByteString( INetURLObject::AbsToRel( aSoundFile,
                                                   INetURLObject::WAS_ENCODED,
                                                   INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );

    rOut << bSoundOn;
    rOut << bPlayFull;


    if (pPathObj && pPathObj->IsInserted())
    {
         rOut << (USHORT)1;
         SdrObjSurrogate aSuro(pPathObj);
         rOut << aSuro;
    }
    else
        rOut << (USHORT)0;

    rOut << (UINT16)eClickAction;
    rOut << (UINT16)eSecondEffect;
    rOut << (UINT16)eSecondSpeed;

    if (eClickAction == presentation::ClickAction_DOCUMENT || eClickAction == presentation::ClickAction_PROGRAM  ||
        eClickAction == presentation::ClickAction_VANISH   || eClickAction == presentation::ClickAction_SOUND)
    {
        rOut.WriteByteString( INetURLObject::AbsToRel( aBookmark,
                                                       INetURLObject::WAS_ENCODED,
                                                       INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );
    }
    else
        rOut.WriteByteString( aBookmark, eSysEnc );

    rOut.WriteByteString( INetURLObject::AbsToRel(aSecondSoundFile,
                                                  INetURLObject::WAS_ENCODED,
                                                  INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );
    rOut << (UINT16)bInvisibleInPresentation;
    rOut << (UINT16)nVerb;

    rOut << bSecondSoundOn;
    rOut << bSecondPlayFull;

    rOut << bDimHide;
    rOut << (UINT16)eTextEffect;
    rOut << (UINT32)nPresOrder;
}


/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

void SdAnimationInfo::ReadData(SvStream& rIn)
{
    SdrObjUserData::ReadData(rIn);

    SdIOCompat  aIO(rIn, STREAM_READ);
    UINT32      nTemp32;
    UINT16      nTemp;

    rIn >> nTemp;
    if (nTemp)
    {
        pPolygon = new Polygon;
        rIn >> *pPolygon;
    }

    rIn >> aStart;
    rIn >> aEnd;

    rIn >> nTemp; eEffect = (presentation::AnimationEffect)nTemp;
    rIn >> nTemp; eSpeed = (presentation::AnimationSpeed)nTemp;

    rIn >> nTemp; bActive = (BOOL)nTemp;
    rIn >> nTemp; bDimPrevious = (BOOL)nTemp;
    rIn >> nTemp; bIsMovie = (BOOL)nTemp;

    rIn >> aBlueScreen;
    rIn >> aDimColor;

    rtl_TextEncoding eTextEnc;

    // ab hier werden Daten der Versionen > 0 eingelesen
    if (aIO.GetVersion() > 0)
    {
        INT16 nCharSet;
        rIn >> nCharSet;
        eTextEnc = (rtl_TextEncoding)nCharSet;

        String aSoundFileRel;
        rIn.ReadByteString( aSoundFileRel, eTextEnc );
        INetURLObject aURLObj(::URIHelper::SmartRelToAbs( aSoundFileRel, FALSE,
                                                          INetURLObject::WAS_ENCODED,
                                                          INetURLObject::DECODE_UNAMBIGUOUS ));
        aSoundFile = aURLObj.GetMainURL();
    }

    // ab hier werden Daten der Versionen > 1 eingelesen
    if (aIO.GetVersion() > 1)
        rIn >> bSoundOn;

    // ab hier werden Daten der Versionen > 2 eingelesen
    if (aIO.GetVersion() > 2)
        rIn >> bPlayFull;

    // ab hier werden Daten der Versionen > 3 eingelesen
    if (aIO.GetVersion() > 3)
    {
        USHORT nFlag;
        rIn >> nFlag;
        if (nFlag == 1)
        {
            DBG_ASSERT(pDoc, "kein Doc");
            pPathSuro = new SdrObjSurrogate(*pDoc, rIn);
        }
    }

    // ab hier werden Daten der Versionen > 4 eingelesen
    if (aIO.GetVersion() > 4)
    {
        rIn >> nTemp; eClickAction  = (presentation::ClickAction)nTemp;
        rIn >> nTemp; eSecondEffect = (presentation::AnimationEffect)nTemp;
        rIn >> nTemp; eSecondSpeed  = (presentation::AnimationSpeed)nTemp;

        if (eClickAction == presentation::ClickAction_DOCUMENT ||
            eClickAction == presentation::ClickAction_PROGRAM  ||
            eClickAction == presentation::ClickAction_VANISH   ||
            eClickAction == presentation::ClickAction_SOUND)
        {
            String aBookmarkRel;
            rIn.ReadByteString( aBookmarkRel, eTextEnc );
            INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aBookmarkRel, FALSE,
                                                             INetURLObject::WAS_ENCODED,
                                                             INetURLObject::DECODE_UNAMBIGUOUS));
            aBookmark = aURLObj.GetMainURL();
        }
        else
            rIn.ReadByteString( aBookmark, eTextEnc );

        String aSecondSoundFileRel;
        rIn.ReadByteString( aSecondSoundFileRel, eTextEnc );
        INetURLObject aURLObjSound(::URIHelper::SmartRelToAbs(aSecondSoundFileRel, FALSE,
                                                              INetURLObject::WAS_ENCODED,
                                                              INetURLObject::DECODE_UNAMBIGUOUS));
        aSecondSoundFile = aURLObjSound.GetMainURL();

        rIn >> nTemp; bInvisibleInPresentation = (BOOL)nTemp;
        rIn >> nTemp; nVerb = (USHORT)nTemp;
    }

    // ab hier werden Daten der Versionen > 5 eingelesen
    if (aIO.GetVersion() > 5)
    {
        rIn >> bSecondSoundOn;
        rIn >> bSecondPlayFull;
    }

    // ab hier werden Daten der Versionen > 6 eingelesen
    if (aIO.GetVersion() > 6)
        rIn >> bDimHide;

    // ab hier werden Daten der Versionen > 7 eingelesen
    if (aIO.GetVersion() > 7)
    {
        rIn >> nTemp;
        eTextEffect = (presentation::AnimationEffect)nTemp;
    }

    if (aIO.GetVersion() > 8)
    {
        rIn >> nTemp32;
        nPresOrder = nTemp32;
    }
}

/*************************************************************************
|*
|* SetPath, Verbindung zum Pfadobjekt herstellen bzw. loesen
|*
\************************************************************************/

void SdAnimationInfo::SetPath(SdrPathObj* pPath)
{
    // alte Verbindung loesen, wenn eine besteht und die neue eine andere ist
    if (pPathObj != NULL && pPathObj != pPath)
    {
        // alte Verbindung loesen
        if (pDoc)
            EndListening(*pDoc);
        pPathObj->RemoveListener(*this);
        pPathObj = NULL;
    }

    // ggfs. neue Verbindung herstellen, wenn es nicht die alte ist
    if (pPathObj != pPath && pPath != NULL)
    {
        if (pDoc == NULL)               // durch copy ctor entstanden
            pDoc = (SdDrawDocument*)pPath->GetModel();
        pPathObj = pPath;
        pPathObj->AddListener(*this);   // DYING
        StartListening(*pDoc);          // OBJ_INSERTED, OBJ_REMOVED
    }
}

/*************************************************************************
|*
|* Notify, Aenderungen am Pfadobjekt
|*
\************************************************************************/

void __EXPORT SdAnimationInfo::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrHint* pSdrHint = PTR_CAST(SdrHint,&rHint);
    if (pSdrHint)
    {
        SdrHintKind eKind = pSdrHint->GetKind();
        if (eKind == HINT_OBJREMOVED && pSdrHint->GetObject() == pPathObj)
            eEffect = presentation::AnimationEffect_NONE;
        else if (eKind == HINT_OBJINSERTED && pSdrHint->GetObject() == pPathObj)
            eEffect = presentation::AnimationEffect_PATH;
    }

    SfxSimpleHint* pSimpleHint = PTR_CAST(SfxSimpleHint, &rHint);
    if (pSimpleHint)
    {
        ULONG nId = pSimpleHint->GetId();
        if (nId == SFX_HINT_DYING)
        {
            eEffect = presentation::AnimationEffect_NONE;
        }
    }
}

/*************************************************************************
|*
|* AfterRead, ggfs. Surrogat aufloesen
|*
\************************************************************************/

void __EXPORT SdAnimationInfo::AfterRead()
{
    if (pPathSuro)
    {
        pPathObj = (SdrPathObj*)pPathSuro->GetObject();
        DBG_ASSERT(pPathObj, "Surrogat kann nicht aufgeloest werden");
    }
}



