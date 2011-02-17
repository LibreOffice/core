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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <tools/urlobj.hxx>
#include <svl/smplhint.hxx>
#include "svx/xtable.hxx"
#include <svx/svdopath.hxx>
#include <svl/urihelper.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>

#include "anminfo.hxx"
#include "glob.hxx"
#include "sdiocmpt.hxx"
#include "drawdoc.hxx"

// #90477#
#include <tools/tenccvt.hxx>

using namespace ::com::sun::star;

SdAnimationInfo::SdAnimationInfo(SdrObject& rObject)
               : SdrObjUserData(SdUDInventor, SD_ANIMATIONINFO_ID, 0),
                 mePresObjKind              (PRESOBJ_NONE),
                 meEffect                   (presentation::AnimationEffect_NONE),
                 meTextEffect               (presentation::AnimationEffect_NONE),
                 meSpeed                    (presentation::AnimationSpeed_SLOW),
                 mbActive                   (sal_True),
                 mbDimPrevious              (sal_False),
                 mbIsMovie                  (sal_False),
                 mbDimHide                  (sal_False),
                 mbSoundOn                  (sal_False),
                 mbPlayFull                 (sal_False),
                 mpPathObj                  (NULL),
                 meClickAction              (presentation::ClickAction_NONE),
                 meSecondEffect             (presentation::AnimationEffect_NONE),
                 meSecondSpeed              (presentation::AnimationSpeed_SLOW),
                 mbSecondSoundOn            (sal_False),
                 mbSecondPlayFull           (sal_False),
                 mnVerb                     (0),
                 mnPresOrder                (LIST_APPEND),
                 mrObject                   (rObject)
{
    maBlueScreen = RGB_Color(COL_LIGHTMAGENTA);
    maDimColor = RGB_Color(COL_LIGHTGRAY);
}

SdAnimationInfo::SdAnimationInfo(const SdAnimationInfo& rAnmInfo, SdrObject& rObject)
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
//               maBookmark                 (rAnmInfo.maBookmark),
                 mnVerb                     (rAnmInfo.mnVerb),
                 mnPresOrder                (LIST_APPEND),
                 mrObject                   (rObject)
{
    // can not be copied
    if(meEffect == presentation::AnimationEffect_PATH)
        meEffect =  presentation::AnimationEffect_NONE;
}


SdAnimationInfo::~SdAnimationInfo()
{
}

SdrObjUserData* SdAnimationInfo::Clone(SdrObject* pObject) const
{
    DBG_ASSERT( pObject, "SdAnimationInfo::Clone(), pObject must not be null!" );
    if( pObject == 0 )
        pObject = &mrObject;

    return new SdAnimationInfo(*this, *pObject );
}

void SdAnimationInfo::SetBookmark( const String& rBookmark )
{
    if( meClickAction == ::com::sun::star::presentation::ClickAction_BOOKMARK )
    {
        String sURL( '#' );
        sURL += rBookmark;
        SvxFieldItem aURLItem( SvxURLField( sURL, sURL ), EE_FEATURE_FIELD );
        mrObject.SetMergedItem( aURLItem );
    }
    else
    {
        SvxFieldItem aURLItem( SvxURLField( rBookmark, rBookmark ), EE_FEATURE_FIELD );
        mrObject.SetMergedItem( aURLItem );
    }
}

String SdAnimationInfo::GetBookmark()
{
    String sBookmark;

    const SvxFieldItem* pFldItem = dynamic_cast< const SvxFieldItem* >( &mrObject.GetMergedItem( EE_FEATURE_FIELD ) );
    if( pFldItem )
    {
        SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );
        if( pURLField )
            sBookmark = pURLField->GetURL();
    }

    if( (meClickAction == ::com::sun::star::presentation::ClickAction_BOOKMARK) && sBookmark.Len() && (sBookmark.GetChar(0) == '#') )
        sBookmark = sBookmark.Copy( 1 );

    return sBookmark;
}
