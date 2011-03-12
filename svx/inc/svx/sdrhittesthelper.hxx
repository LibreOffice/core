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

#ifndef _SDRHITTESTHELPER_HXX
#define _SDRHITTESTHELPER_HXX

#include "svx/svxdllapi.h"
#include <tools/string.hxx>
#include <tools/gen.hxx>

/////////////////////////////////////////////////////////////////////
// #i101872# new Object HitTest as View-tooling

class SdrObject;
class SdrPageView;
class SetOfByte;
class SdrObjList;
namespace sdr { namespace contact { class ViewObjectContact; }}
namespace basegfx { class B2DPoint; }

/////////////////////////////////////////////////////////////////////
// Wrappers for classic Sdr* Mode/View classes

SVX_DLLPUBLIC SdrObject* SdrObjectPrimitiveHit(
    const SdrObject& rObject,
    const Point& rPnt,
    sal_uInt16 nTol,
    const SdrPageView& rSdrPageView,
    const SetOfByte* pVisiLayer,
    bool bTextOnly);

SVX_DLLPUBLIC SdrObject* SdrObjListPrimitiveHit(
    const SdrObjList& rList,
    const Point& rPnt,
    sal_uInt16 nTol,
    const SdrPageView& rSdrPageView,
    const SetOfByte* pVisiLayer,
    bool bTextOnly);

/////////////////////////////////////////////////////////////////////
// the pure HitTest based on a VOC

SVX_DLLPUBLIC bool ViewObjectContactPrimitiveHit(
    const sdr::contact::ViewObjectContact& rVOC,
    const basegfx::B2DPoint& rHitPosition,
    double fLogicHitTolerance,
    bool bTextOnly);

/////////////////////////////////////////////////////////////////////

#endif //_SDRHITTESTHELPER_HXX

/////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
