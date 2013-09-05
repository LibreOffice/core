/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

void SdAnimationInfo::SetBookmark( const OUString& rBookmark )
{
    if( meClickAction == ::com::sun::star::presentation::ClickAction_BOOKMARK )
    {
        OUString sURL("#");
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

OUString SdAnimationInfo::GetBookmark()
{
    OUString sBookmark;

    const SvxFieldItem* pFldItem = dynamic_cast< const SvxFieldItem* >( &mrObject.GetMergedItem( EE_FEATURE_FIELD ) );
    if( pFldItem )
    {
        SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );
        if( pURLField )
            sBookmark = pURLField->GetURL();
    }

    if( (meClickAction == ::com::sun::star::presentation::ClickAction_BOOKMARK) && !sBookmark.isEmpty() && (sBookmark[0] == '#') )
        sBookmark = sBookmark.copy( 1 );

    return sBookmark;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
