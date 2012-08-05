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

#ifndef _SD_ANMINFO_HXX
#define _SD_ANMINFO_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "pres.hxx"
#include <svx/svdobj.hxx>
#include <tools/color.hxx>
#include <sddllapi.h>

class SdrObject;
class SdrPathObj;

class SdAnimationInfo : public SdrObjUserData
{
public:
    PresObjKind             mePresObjKind;

    /* deprecated animation infos */
    ::com::sun::star::presentation::AnimationEffect meEffect;     ///< Animation effect
    ::com::sun::star::presentation::AnimationEffect meTextEffect; ///< Animation effect for text content
    ::com::sun::star::presentation::AnimationSpeed  meSpeed;      ///< Speed of the animation
    sal_Bool                    mbActive;       ///< turned on?
    sal_Bool                    mbDimPrevious;  ///< Object fade out
    sal_Bool                    mbIsMovie;      ///< if group object than it is a sequence of them.
    sal_Bool                    mbDimHide;      ///< hide rather than dim
    Color                       maBlueScreen;   ///< identifies "background pixels"
    Color                       maDimColor;     ///< for fading the object
    String                      maSoundFile;    ///< Path to the sound file in MS DOS notation
    sal_Bool                    mbSoundOn;      ///< Sound on / off
    sal_Bool                    mbPlayFull;     ///< play sound completely.
    SdrPathObj*                 mpPathObj;      ///< The path object
    ::com::sun::star::presentation::ClickAction     meClickAction;  ///< Action at mouse click
    ::com::sun::star::presentation::AnimationEffect meSecondEffect; ///< for object fading.
    ::com::sun::star::presentation::AnimationSpeed  meSecondSpeed;  ///< for object fading.
    String                      maSecondSoundFile; ///< for object fading.
    sal_Bool                    mbSecondSoundOn;   ///< for object fading.
    sal_Bool                    mbSecondPlayFull;  ///< for object fading.
    sal_uInt16                  mnVerb;            ///< for OLE object
    sal_uLong                   mnPresOrder;
    SdrObject&              mrObject;

    SD_DLLPUBLIC void                    SetBookmark( const String& rBookmark );
    SD_DLLPUBLIC String                  GetBookmark();
public:
                            SdAnimationInfo(SdrObject& rObject);
                            SdAnimationInfo(const SdAnimationInfo& rAnmInfo, SdrObject& rObject);
    virtual                 ~SdAnimationInfo();

    virtual SdrObjUserData* Clone(SdrObject* pObject) const;
};

#endif // _SD_ANMINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
