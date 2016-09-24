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

#ifndef INCLUDED_SVX_SDRHITTESTHELPER_HXX
#define INCLUDED_SVX_SDRHITTESTHELPER_HXX

#include <svx/svxdllapi.h>
#include <tools/gen.hxx>


// #i101872# new Object HitTest as View-tooling

class SdrObject;
class SdrPageView;
class SetOfByte;
class SdrObjList;
namespace sdr { namespace contact { class ViewObjectContact; }}
namespace basegfx { class B2DPoint; }


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


// the pure HitTest based on a VOC

SVX_DLLPUBLIC bool ViewObjectContactPrimitiveHit(
    const sdr::contact::ViewObjectContact& rVOC,
    const basegfx::B2DPoint& rHitPosition,
    double fLogicHitTolerance,
    bool bTextOnly);


#endif // INCLUDED_SVX_SDRHITTESTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
