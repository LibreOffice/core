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

#ifndef INCLUDED_SD_INC_ANMINFO_HXX
#define INCLUDED_SD_INC_ANMINFO_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "pres.hxx"
#include <svx/svdobj.hxx>
#include <tools/color.hxx>
#include <sddllapi.h>

class SdrObject;
class SdrPathObj;

class SD_DLLPUBLIC SdAnimationInfo : public SdrObjUserData
{
public:
    PresObjKind             mePresObjKind;

    /* deprecated animation infos */
    ::com::sun::star::presentation::AnimationEffect meEffect;     ///< Animation effect
    ::com::sun::star::presentation::AnimationEffect meTextEffect; ///< Animation effect for text content
    ::com::sun::star::presentation::AnimationSpeed  meSpeed;      ///< Speed of the animation
    bool                    mbActive;       ///< turned on?
    bool                    mbDimPrevious;  ///< Object fade out
    bool                    mbIsMovie;      ///< if group object than it is a sequence of them.
    bool                    mbDimHide;      ///< hide rather than dim
    Color                       maBlueScreen;   ///< identifies "background pixels"
    Color                       maDimColor;     ///< for fading the object
    OUString                    maSoundFile;    ///< Path to the sound file in MS DOS notation
    bool                    mbSoundOn;      ///< Sound on / off
    bool                    mbPlayFull;     ///< play sound completely.
    SdrPathObj*                 mpPathObj;      ///< The path object
    ::com::sun::star::presentation::ClickAction     meClickAction;  ///< Action at mouse click
    ::com::sun::star::presentation::AnimationEffect meSecondEffect; ///< for object fading.
    ::com::sun::star::presentation::AnimationSpeed  meSecondSpeed;  ///< for object fading.
    OUString                    maSecondSoundFile; ///< for object fading.
    bool                    mbSecondSoundOn;   ///< for object fading.
    bool                    mbSecondPlayFull;  ///< for object fading.
    sal_uInt16                  mnVerb;            ///< for OLE object
    sal_uLong                   mnPresOrder;
    SdrObject&                  mrObject;

    void           SetBookmark( const OUString& rBookmark );
    OUString       GetBookmark();
public:
                            SAL_DLLPRIVATE SdAnimationInfo(SdrObject& rObject);
                            SAL_DLLPRIVATE SdAnimationInfo(const SdAnimationInfo& rAnmInfo, SdrObject& rObject);
    SAL_DLLPRIVATE virtual                 ~SdAnimationInfo();

    SAL_DLLPRIVATE virtual SdrObjUserData* Clone(SdrObject* pObject) const override;
};

#endif // INCLUDED_SD_INC_ANMINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
