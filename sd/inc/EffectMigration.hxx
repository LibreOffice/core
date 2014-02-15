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

#ifndef INCLUDED_SD_INC_EFFECTMIGRATION_HXX
#define INCLUDED_SD_INC_EFFECTMIGRATION_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>

class SdPage;
class SvxShape;
class SdAnimationInfo;
class SdrObject;
class SdrPathObj;
class SdrObjGroup;

namespace sd {

/** this helper class gives various methods to convert effects from OOo 1.1
    and below to and from OOo 2.0 and above
*/
class EffectMigration
{
public:
    static ::com::sun::star::presentation::AnimationSpeed ConvertDuration( double fDuration );
    static double ConvertAnimationSpeed( ::com::sun::star::presentation::AnimationSpeed eSpeed );

    static bool ConvertPreset( const OUString& rPresetId, const OUString* pPresetSubType, ::com::sun::star::presentation::AnimationEffect& rEffect );
    static bool ConvertAnimationEffect( const ::com::sun::star::presentation::AnimationEffect& rEffect, OUString& rPresetId, OUString& rPresetSubType );

public:
    void AddEffectFromAnimationInfo( SdrObject* pObject, SdAnimationInfo* pInfo );

public:
    static void SetFadeEffect( SdPage* pPage, ::com::sun::star::presentation::FadeEffect eNewEffect);
    static ::com::sun::star::presentation::FadeEffect GetFadeEffect( const SdPage* pPage );

public:
    static void SetAnimationEffect( SvxShape* pShape, ::com::sun::star::presentation::AnimationEffect eValue );
    static ::com::sun::star::presentation::AnimationEffect GetAnimationEffect( SvxShape* pShape );
    static void SetTextAnimationEffect( SvxShape* pShape, ::com::sun::star::presentation::AnimationEffect eValue );
    static ::com::sun::star::presentation::AnimationEffect GetTextAnimationEffect( SvxShape* pShape );
    static void SetAnimationSpeed( SvxShape* pShape, ::com::sun::star::presentation::AnimationSpeed eSpeed );
    static ::com::sun::star::presentation::AnimationSpeed GetAnimationSpeed( SvxShape* pShape );
    static void SetDimColor( SvxShape* pShape, sal_Int32 nColor );
    static sal_Int32 GetDimColor( SvxShape* pShape );
    static void SetDimHide( SvxShape* pShape, sal_Bool bDimHide );
    static sal_Bool GetDimHide( SvxShape* pShape );
    static void SetDimPrevious( SvxShape* pShape, sal_Bool bDimPrevious );
    static sal_Bool GetDimPrevious( SvxShape* pShape );
    static void SetPresentationOrder( SvxShape* pShape, sal_Int32 nNewPos );
    static sal_Int32 GetPresentationOrder( SvxShape* pShape );
    static void UpdateSoundEffect( SvxShape* pShape, SdAnimationInfo* pInfo );
    static OUString GetSoundFile( SvxShape* pShape );
    static sal_Bool GetSoundOn( SvxShape* pShape );

    static void SetAnimationPath( SvxShape* pShape, SdrPathObj* pPathObj );
    static void CreateAnimatedGroup(SdrObjGroup& rGroupObj, SdPage& rPage);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
