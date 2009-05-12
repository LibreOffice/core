/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anminfo.cxx,v $
 * $Revision: 1.16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/urlobj.hxx>
#include <svtools/smplhint.hxx>
#include "svx/xtable.hxx"
#include <svx/svdopath.hxx>
#include <svtools/urihelper.hxx>


#include "anminfo.hxx"
#include "glob.hxx"
#include "sdiocmpt.hxx"
#include "drawdoc.hxx"

// #90477#
#include <tools/tenccvt.hxx>

using namespace ::com::sun::star;

SdAnimationInfo::SdAnimationInfo()
               : SdrObjUserData(SdUDInventor, SD_ANIMATIONINFO_ID, 0),
                 mePresObjKind              (PRESOBJ_NONE),
                 meEffect                   (presentation::AnimationEffect_NONE),
                 meTextEffect               (presentation::AnimationEffect_NONE),
                 meSpeed                    (presentation::AnimationSpeed_SLOW),
                 mbActive                   (TRUE),
                 mbDimPrevious              (FALSE),
                 mbIsMovie                  (FALSE),
                 mbDimHide                  (FALSE),
                 mbSoundOn                  (FALSE),
                 mbPlayFull                 (FALSE),
                 mpPathObj                  (NULL),
                 meClickAction              (presentation::ClickAction_NONE),
                 meSecondEffect             (presentation::AnimationEffect_NONE),
                 meSecondSpeed              (presentation::AnimationSpeed_SLOW),
                 mbSecondSoundOn            (FALSE),
                 mbSecondPlayFull           (FALSE),
                 mnVerb                     (0),
                 mnPresOrder                (LIST_APPEND)
{
    maBlueScreen = RGB_Color(COL_LIGHTMAGENTA);
    maDimColor = RGB_Color(COL_LIGHTGRAY);
}

SdAnimationInfo::SdAnimationInfo(const SdAnimationInfo& rAnmInfo)
               : SdrObjUserData             (rAnmInfo),
                    mePresObjKind               (PRESOBJ_NONE),
                 meEffect                   (rAnmInfo.meEffect),
                 meTextEffect               (rAnmInfo.meTextEffect),
                 meSpeed                    (rAnmInfo.meSpeed),
                 mbActive                   (rAnmInfo.mbActive),
                 mbDimPrevious              (rAnmInfo.mbDimPrevious),
                 mbIsMovie                  (rAnmInfo.mbIsMovie),
                 mbDimHide                  (rAnmInfo.mbDimHide),
                 maBlueScreen               (rAnmInfo.maBlueScreen),
                 maDimColor                 (rAnmInfo.maDimColor),
                 maSoundFile                (rAnmInfo.maSoundFile),
                 mbSoundOn                  (rAnmInfo.mbSoundOn),
                 mbPlayFull                 (rAnmInfo.mbPlayFull),
                 mpPathObj                  (NULL),
                 meClickAction              (rAnmInfo.meClickAction),
                 meSecondEffect             (rAnmInfo.meSecondEffect),
                 meSecondSpeed              (rAnmInfo.meSecondSpeed),
                 maSecondSoundFile          (rAnmInfo.maSecondSoundFile),
                 mbSecondSoundOn            (rAnmInfo.mbSecondSoundOn),
                 mbSecondPlayFull           (rAnmInfo.mbSecondPlayFull),
                 maBookmark                 (rAnmInfo.maBookmark),
                 mnVerb                     (rAnmInfo.mnVerb),
                 mnPresOrder                (LIST_APPEND)
{
    // can not be copied
    if(meEffect == presentation::AnimationEffect_PATH)
        meEffect =  presentation::AnimationEffect_NONE;
}


SdAnimationInfo::~SdAnimationInfo()
{
}

SdrObjUserData* SdAnimationInfo::Clone(SdrObject*) const
{
    return new SdAnimationInfo(*this);
}
