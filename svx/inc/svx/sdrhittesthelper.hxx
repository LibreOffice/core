/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SDRHITTESTHELPER_HXX
#define _SDRHITTESTHELPER_HXX

#include "svx/svxdllapi.h"
#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

/////////////////////////////////////////////////////////////////////
// #i101872# new Object HitTest as View-tooling

class SdrObject;
class SdrView;
class SetOfByte;
class SdrObjList;
namespace sdr { namespace contact { class ViewObjectContact; }}
namespace basegfx { class B2DPoint; }

/////////////////////////////////////////////////////////////////////
// Wrappers for classic Sdr* Mode/View classes

SVX_DLLPUBLIC SdrObject* SdrObjectPrimitiveHit(
    const SdrObject& rObject,
    const basegfx::B2DPoint& rPnt,
    double fTol,
    const SdrView& rSdrView,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields);

SVX_DLLPUBLIC SdrObject* SdrObjListPrimitiveHit(
    const SdrObjList& rList,
    const basegfx::B2DPoint& rPnt,
    double fTol,
    const SdrView& rSdrView,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields);

/////////////////////////////////////////////////////////////////////
// the pure HitTest based on a VOC

SVX_DLLPUBLIC bool ViewObjectContactPrimitiveHit(
    const sdr::contact::ViewObjectContact& rVOC,
    const basegfx::B2DPoint& rHitPosition,
    double fLogicHitTolerance,
    bool bTextOnly,
    drawinglayer::primitive2d::Primitive2DSequence* pRecordFields);

/////////////////////////////////////////////////////////////////////

#endif //_SDRHITTESTHELPER_HXX

/////////////////////////////////////////////////////////////////////
// eof
