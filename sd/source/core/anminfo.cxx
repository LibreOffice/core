/*************************************************************************
 *
 *  $RCSfile: anminfo.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-11 12:09:08 $
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



