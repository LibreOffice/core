/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: anminfo.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:30:40 $
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
