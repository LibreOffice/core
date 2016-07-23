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

class SdDrawDocument;
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
    static css::presentation::AnimationSpeed ConvertDuration( double fDuration );
    static double ConvertAnimationSpeed( css::presentation::AnimationSpeed eSpeed );

    static bool ConvertPreset( const OUString& rPresetId, const OUString* pPresetSubType, css::presentation::AnimationEffect& rEffect );
    static bool ConvertAnimationEffect( const css::presentation::AnimationEffect& rEffect, OUString& rPresetId, OUString& rPresetSubType );

    static void SetFadeEffect( SdPage* pPage, css::presentation::FadeEffect eNewEffect);
    static css::presentation::FadeEffect GetFadeEffect( const SdPage* pPage );

    static void SetAnimationEffect( SvxShape* pShape, css::presentation::AnimationEffect eValue );
    static css::presentation::AnimationEffect GetAnimationEffect( SvxShape* pShape );
    static void SetTextAnimationEffect( SvxShape* pShape, css::presentation::AnimationEffect eValue );
    static css::presentation::AnimationEffect GetTextAnimationEffect( SvxShape* pShape );
    static void SetAnimationSpeed( SvxShape* pShape, css::presentation::AnimationSpeed eSpeed );
    static css::presentation::AnimationSpeed GetAnimationSpeed( SvxShape* pShape );
    static void SetDimColor( SvxShape* pShape, sal_Int32 nColor );
    static sal_Int32 GetDimColor( SvxShape* pShape );
    static void SetDimHide( SvxShape* pShape, bool bDimHide );
    static bool GetDimHide( SvxShape* pShape );
    static void SetDimPrevious( SvxShape* pShape, bool bDimPrevious );
    static bool GetDimPrevious( SvxShape* pShape );
    static void SetPresentationOrder( SvxShape* pShape, sal_Int32 nNewPos );
    static sal_Int32 GetPresentationOrder( SvxShape* pShape );
    static void UpdateSoundEffect( SvxShape* pShape, SdAnimationInfo* pInfo );
    static OUString GetSoundFile( SvxShape* pShape );
    static bool GetSoundOn( SvxShape* pShape );

    static void SetAnimationPath( SvxShape* pShape, SdrPathObj* pPathObj );
    static void CreateAnimatedGroup(SdrObjGroup& rGroupObj, SdPage& rPage);
    static void DocumentLoaded(SdDrawDocument & rDoc);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
