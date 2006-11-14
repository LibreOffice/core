/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anminfo.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:20:48 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#include "svx/xtable.hxx"
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif
#include <svtools/urihelper.hxx>


#include "anminfo.hxx"
#include "glob.hxx"
#include "sdiocmpt.hxx"
#include "drawdoc.hxx"

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

using namespace ::com::sun::star;

SdAnimationInfo::SdAnimationInfo()
               : SdrObjUserData(SdUDInventor, SD_ANIMATIONINFO_ID, 0),
                 mePresObjKind              (PRESOBJ_NONE),
                 eEffect                    (presentation::AnimationEffect_NONE),
                 eTextEffect                (presentation::AnimationEffect_NONE),
                 eSpeed                     (presentation::AnimationSpeed_SLOW),
                 bActive                    (TRUE),
                 bDimPrevious               (FALSE),
                 bIsMovie                   (FALSE),
                 bDimHide                   (FALSE),
                 bSoundOn                   (FALSE),
                 bPlayFull                  (FALSE),
                 pPathObj                   (NULL),
                 eClickAction               (presentation::ClickAction_NONE),
                 eSecondEffect              (presentation::AnimationEffect_NONE),
                 eSecondSpeed               (presentation::AnimationSpeed_SLOW),
                 bSecondSoundOn             (FALSE),
                 bSecondPlayFull            (FALSE),
                 nVerb                      (0),
                 nPresOrder                 (LIST_APPEND)
{
    aBlueScreen = RGB_Color(COL_LIGHTMAGENTA);
    aDimColor = RGB_Color(COL_LIGHTGRAY);
}

SdAnimationInfo::SdAnimationInfo(const SdAnimationInfo& rAnmInfo)
               : SdrObjUserData             (rAnmInfo),
                    mePresObjKind               (PRESOBJ_NONE),
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
                 eClickAction               (rAnmInfo.eClickAction),
                 eSecondEffect              (rAnmInfo.eSecondEffect),
                 eSecondSpeed               (rAnmInfo.eSecondSpeed),
                 bSecondSoundOn             (rAnmInfo.bSecondSoundOn),
                 bSecondPlayFull            (rAnmInfo.bSecondPlayFull),
                 nVerb                      (rAnmInfo.nVerb),
                 aBookmark                  (rAnmInfo.aBookmark),
                 aSecondSoundFile           (rAnmInfo.aSecondSoundFile),
                 nPresOrder                 (LIST_APPEND)
{
    // can not be copied
    if (eEffect == presentation::AnimationEffect_PATH)
        eEffect =  presentation::AnimationEffect_NONE;
}


SdAnimationInfo::~SdAnimationInfo()
{
}

SdrObjUserData* SdAnimationInfo::Clone(SdrObject* pObj) const
{
    return new SdAnimationInfo(*this);
}
