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

#ifndef _SD_ANMINFO_HXX
#define _SD_ANMINFO_HXX

#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include "pres.hxx"
#include "anmdef.hxx"
#include <svx/svdobj.hxx>
#include <tools/color.hxx>
#include <sddllapi.h>

class Polygon;
class Point;
class SvStream;
class SdrObjSurrogate;
class SdrObject;
class SdrPathObj;
class SdDrawDocument;

class SdAnimationInfo : public SdrObjUserData
{
public:
    PresObjKind             mePresObjKind;

    /* deprecated animation infos */
    ::com::sun::star::presentation::AnimationEffect         meEffect;       ///< Animation effect
    ::com::sun::star::presentation::AnimationEffect         meTextEffect;   ///< Animation effect for text
    ::com::sun::star::presentation::AnimationSpeed          meSpeed;            ///< Speed ​​of the animation
    sal_Bool                    mbActive;       ///< turned on?
    sal_Bool                    mbDimPrevious;  ///< Object Dim
    sal_Bool                    mbIsMovie;      ///< wenn Gruppenobjekt, dann Sequenz aus den
    sal_Bool                    mbDimHide;      ///< hide rather than dim
    Color                   maBlueScreen;   ///< identifies "background pixels"
    Color                   maDimColor;     ///< zum Abblenden des Objekts
    String                  maSoundFile;        ///< Path to the sound file in MS DOS notation
    sal_Bool                    mbSoundOn;      ///< Sound on / off
    sal_Bool                    mbPlayFull;     ///< Play sound quite
    SdrPathObj*             mpPathObj;      ///< The path object
    ::com::sun::star::presentation::ClickAction             meClickAction;  ///< Action at mouse click
    ::com::sun::star::presentation::AnimationEffect         meSecondEffect; ///< for  Hidden  object
    ::com::sun::star::presentation::AnimationSpeed          meSecondSpeed;  ///< for  Hidden  object
    String                  maSecondSoundFile; ///< for  Hidden  object
    sal_Bool                    mbSecondSoundOn;    ///< for  Hidden  object
    sal_Bool                    mbSecondPlayFull;///< ffor  Hidden  object
    sal_uInt16                  mnVerb;         ///< for OLE object
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
