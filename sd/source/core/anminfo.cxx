/*************************************************************************
 *
 *  $RCSfile: anminfo.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:52:44 $
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

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

using namespace ::com::sun::star;

/*************************************************************************
|*
|* Konstruktor   (String aSecondSoundFile unbenutzt --> default ctor)
|*
\************************************************************************/

SdAnimationInfo::SdAnimationInfo(SdDrawDocument* pTheDoc)
               : SdrObjUserData(SdUDInventor, SD_ANIMATIONINFO_ID, 0),
//BFS02              pPolygon                   (NULL),
                 eEffect                    (presentation::AnimationEffect_NONE),
                 eTextEffect                (presentation::AnimationEffect_NONE),
                 eSpeed                     (presentation::AnimationSpeed_SLOW),
                 bActive                    (TRUE),
                 bDimPrevious               (FALSE),
                 bIsMovie                   (FALSE),
                 bDimHide                   (FALSE),
                 bSoundOn                   (FALSE),
                 bPlayFull                  (FALSE),
//BFS02              pPathSuro                  (NULL),
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
//BFS02              pPolygon                   (NULL),
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
//BFS02              pPathSuro                  (NULL),
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
//BFS02 if (pPolygon)
//BFS02     pPolygon = new Polygon(*(rAnmInfo.pPolygon));

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
//BFS02 delete pPathSuro;
//BFS02 delete pPolygon;
}

/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrObjUserData* SdAnimationInfo::Clone(SdrObject* pObj) const
{
    return new SdAnimationInfo(*this);
}


/*************************************************************************
|*
|* Daten in Stream schreiben
|*
\************************************************************************/

//BFS02void SdAnimationInfo::WriteData(SvStream& rOut)
//BFS02{
//BFS02 SdrObjUserData::WriteData(rOut);
//BFS02
//BFS02         // letzter Parameter ist die aktuelle Versionsnummer des Codes
//BFS02 SdIOCompat aIO(rOut, STREAM_WRITE, 9);
//BFS02
//BFS02 if(pPolygon)
//BFS02 {
//BFS02     rOut << (UINT16)1;  // es folgt ein Polygon
//BFS02     rOut << *pPolygon;
//BFS02 }
//BFS02 else
//BFS02     rOut << (UINT16)0;  // kein Polygon
//BFS02
//BFS02 rOut << aStart;
//BFS02 rOut << aEnd;
//BFS02 rOut << (UINT16)eEffect;
//BFS02 rOut << (UINT16)eSpeed;
//BFS02
//BFS02 rOut << (UINT16)bActive;
//BFS02 rOut << (UINT16)bDimPrevious;
//BFS02 rOut << (UINT16)bIsMovie;
//BFS02
//BFS02 rOut << aBlueScreen;
//BFS02 rOut << aDimColor;
//BFS02
//BFS02 // #90477# rtl_TextEncoding eSysEnc = ::GetStoreCharSet( gsl_getSystemTextEncoding() );
//BFS02 rtl_TextEncoding eSysEnc = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());
//BFS02
//BFS02 rOut << (INT16) eSysEnc;
//BFS02
//BFS02 rOut.WriteByteString( INetURLObject::AbsToRel( aSoundFile,
//BFS02                                                INetURLObject::WAS_ENCODED,
//BFS02                                                INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );
//BFS02
//BFS02 rOut << bSoundOn;
//BFS02 rOut << bPlayFull;
//BFS02
//BFS02
//BFS02 if (pPathObj && pPathObj->IsInserted())
//BFS02 {
//BFS02      rOut << (USHORT)1;
//BFS02      SdrObjSurrogate aSuro(pPathObj);
//BFS02      rOut << aSuro;
//BFS02 }
//BFS02 else
//BFS02     rOut << (USHORT)0;
//BFS02
//BFS02 rOut << (UINT16)eClickAction;
//BFS02 rOut << (UINT16)eSecondEffect;
//BFS02 rOut << (UINT16)eSecondSpeed;
//BFS02
//BFS02 if (eClickAction == presentation::ClickAction_DOCUMENT || eClickAction == presentation::ClickAction_PROGRAM  ||
//BFS02     eClickAction == presentation::ClickAction_VANISH   || eClickAction == presentation::ClickAction_SOUND)
//BFS02 {
//BFS02     rOut.WriteByteString( INetURLObject::AbsToRel( aBookmark,
//BFS02                                                    INetURLObject::WAS_ENCODED,
//BFS02                                                    INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );
//BFS02 }
//BFS02 else
//BFS02     rOut.WriteByteString( aBookmark, eSysEnc );
//BFS02
//BFS02 rOut.WriteByteString( INetURLObject::AbsToRel(aSecondSoundFile,
//BFS02                                               INetURLObject::WAS_ENCODED,
//BFS02                                               INetURLObject::DECODE_UNAMBIGUOUS), eSysEnc );
//BFS02 rOut << (UINT16)bInvisibleInPresentation;
//BFS02 rOut << (UINT16)nVerb;
//BFS02
//BFS02 rOut << bSecondSoundOn;
//BFS02 rOut << bSecondPlayFull;
//BFS02
//BFS02 rOut << bDimHide;
//BFS02 rOut << (UINT16)eTextEffect;
//BFS02 rOut << (UINT32)nPresOrder;
//BFS02}


/*************************************************************************
|*
|* Daten aus Stream lesen
|*
\************************************************************************/

//BFS02void SdAnimationInfo::ReadData(SvStream& rIn)
//BFS02{
//BFS02 SdrObjUserData::ReadData(rIn);
//BFS02
//BFS02 SdIOCompat  aIO(rIn, STREAM_READ);
//BFS02 UINT32      nTemp32;
//BFS02 UINT16      nTemp;
//BFS02
//BFS02 rIn >> nTemp;
//BFS02 if (nTemp)
//BFS02 {
//BFS02     pPolygon = new Polygon;
//BFS02     rIn >> *pPolygon;
//BFS02 }
//BFS02
//BFS02 rIn >> aStart;
//BFS02 rIn >> aEnd;
//BFS02
//BFS02 rIn >> nTemp; eEffect = (presentation::AnimationEffect)nTemp;
//BFS02 rIn >> nTemp; eSpeed = (presentation::AnimationSpeed)nTemp;
//BFS02
//BFS02 rIn >> nTemp; bActive = (BOOL)nTemp;
//BFS02 rIn >> nTemp; bDimPrevious = (BOOL)nTemp;
//BFS02 rIn >> nTemp; bIsMovie = (BOOL)nTemp;
//BFS02
//BFS02 rIn >> aBlueScreen;
//BFS02 rIn >> aDimColor;
//BFS02
//BFS02 rtl_TextEncoding eTextEnc;
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 0 eingelesen
//BFS02 if (aIO.GetVersion() > 0)
//BFS02 {
//BFS02     INT16 nCharSet;
//BFS02     rIn >> nCharSet;
//BFS02
//BFS02     // #unicode# eTextEnc = (rtl_TextEncoding)nCharSet;
//BFS02     eTextEnc = (rtl_TextEncoding)GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet, (sal_uInt16)rIn.GetVersion());
//BFS02
//BFS02     String aSoundFileRel;
//BFS02     rIn.ReadByteString( aSoundFileRel, eTextEnc );
//BFS02     if( aSoundFileRel.Len() )
//BFS02     {
//BFS02         INetURLObject aURLObj(::URIHelper::SmartRel2Abs( INetURLObject(INetURLObject::GetBaseURL()), aSoundFileRel, ::URIHelper::GetMaybeFileHdl(), false, false, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8, false, INetURLObject::FSYS_DETECT ));
//BFS02         aSoundFile = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
//BFS02     }
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 1 eingelesen
//BFS02 if (aIO.GetVersion() > 1)
//BFS02     rIn >> bSoundOn;
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 2 eingelesen
//BFS02 if (aIO.GetVersion() > 2)
//BFS02     rIn >> bPlayFull;
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 3 eingelesen
//BFS02 if (aIO.GetVersion() > 3)
//BFS02 {
//BFS02     USHORT nFlag;
//BFS02     rIn >> nFlag;
//BFS02     if (nFlag == 1)
//BFS02     {
//BFS02         DBG_ASSERT(pDoc, "kein Doc");
//BFS02         pPathSuro = new SdrObjSurrogate(*pDoc, rIn);
//BFS02     }
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 4 eingelesen
//BFS02 if (aIO.GetVersion() > 4)
//BFS02 {
//BFS02     rIn >> nTemp; eClickAction  = (presentation::ClickAction)nTemp;
//BFS02     rIn >> nTemp; eSecondEffect = (presentation::AnimationEffect)nTemp;
//BFS02     rIn >> nTemp; eSecondSpeed  = (presentation::AnimationSpeed)nTemp;
//BFS02
//BFS02     if (eClickAction == presentation::ClickAction_DOCUMENT ||
//BFS02         eClickAction == presentation::ClickAction_PROGRAM  ||
//BFS02         eClickAction == presentation::ClickAction_VANISH   ||
//BFS02         eClickAction == presentation::ClickAction_SOUND)
//BFS02     {
//BFS02         String aBookmarkRel;
//BFS02         rIn.ReadByteString( aBookmarkRel, eTextEnc );
//BFS02         INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aBookmarkRel, FALSE,
//BFS02                                                          INetURLObject::WAS_ENCODED,
//BFS02                                                          INetURLObject::DECODE_UNAMBIGUOUS));
//BFS02         aBookmark = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
//BFS02     }
//BFS02     else
//BFS02         rIn.ReadByteString( aBookmark, eTextEnc );
//BFS02
//BFS02     String aSecondSoundFileRel;
//BFS02     rIn.ReadByteString( aSecondSoundFileRel, eTextEnc );
//BFS02     if( aSecondSoundFileRel.Len() )
//BFS02     {
//BFS02         INetURLObject aURLObj(::URIHelper::SmartRel2Abs( INetURLObject(INetURLObject::GetBaseURL()), aSecondSoundFileRel, ::URIHelper::GetMaybeFileHdl(), false, false, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_UTF8, false, INetURLObject::FSYS_DETECT ));
//BFS02         aSecondSoundFile = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
//BFS02     }
//BFS02
//BFS02     rIn >> nTemp; bInvisibleInPresentation = (BOOL)nTemp;
//BFS02     rIn >> nTemp; nVerb = (USHORT)nTemp;
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 5 eingelesen
//BFS02 if (aIO.GetVersion() > 5)
//BFS02 {
//BFS02     rIn >> bSecondSoundOn;
//BFS02     rIn >> bSecondPlayFull;
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 6 eingelesen
//BFS02 if (aIO.GetVersion() > 6)
//BFS02     rIn >> bDimHide;
//BFS02
//BFS02 // ab hier werden Daten der Versionen > 7 eingelesen
//BFS02 if (aIO.GetVersion() > 7)
//BFS02 {
//BFS02     rIn >> nTemp;
//BFS02     eTextEffect = (presentation::AnimationEffect)nTemp;
//BFS02 }
//BFS02
//BFS02 if (aIO.GetVersion() > 8)
//BFS02 {
//BFS02     rIn >> nTemp32;
//BFS02     nPresOrder = nTemp32;
//BFS02 }
//BFS02}

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

void SdAnimationInfo::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
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

//BFS02void SdAnimationInfo::AfterRead()
//BFS02{
//BFS02 if (pPathSuro)
//BFS02 {
//BFS02     pPathObj = (SdrPathObj*)pPathSuro->GetObject();
//BFS02     DBG_ASSERT(pPathObj, "Surrogat kann nicht aufgeloest werden");
//BFS02 }
//BFS02}



