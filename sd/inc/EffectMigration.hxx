/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SD_EFFECT_MIGRATION_HXX
#define _SD_EFFECT_MIGRATION_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/FadeEffect.hpp>

class SdPage;
class SvxShape;
class SdAnimationInfo;
class SdrObject;

namespace sd {

/** this helper class gives various methods to convert effects from OOo 1.1
    and below to and from OOo 2.0 and above
*/
class EffectMigration
{
public:
    static ::com::sun::star::presentation::AnimationSpeed ConvertDuration( double fDuration );
    static double ConvertAnimationSpeed( ::com::sun::star::presentation::AnimationSpeed eSpeed );

    static bool ConvertPreset( const ::rtl::OUString& rPresetId, const ::rtl::OUString* pPresetSubType, ::com::sun::star::presentation::AnimationEffect& rEffect );
    static bool ConvertAnimationEffect( const ::com::sun::star::presentation::AnimationEffect& rEffect, ::rtl::OUString& rPresetId, ::rtl::OUString& rPresetSubType );

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
    static ::rtl::OUString GetSoundFile( SvxShape* pShape );
    static sal_Bool GetSoundOn( SvxShape* pShape );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
