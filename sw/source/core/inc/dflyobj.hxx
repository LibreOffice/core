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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DFLYOBJ_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DFLYOBJ_HXX

#include <svx/svdovirt.hxx>

class SwFlyFrm;
class SwFrmFmt;
class SdrObjMacroHitRec;

const sal_uInt32 SWGInventor =  sal_uInt32('S')*0x00000001 +
                                sal_uInt32('W')*0x00000100 +
                                sal_uInt32('G')*0x00010000;

const sal_uInt16 SwFlyDrawObjIdentifier = 0x0001;

// DrawObjects for Flys
class SwFlyDrawObj : public SdrObject
{
private:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    TYPEINFO_OVERRIDE();

    SwFlyDrawObj();
    ~SwFlyDrawObj();

    // for instantiation of this class while loading (via factory)
    virtual sal_uInt32 GetObjInventor()     const;
    virtual sal_uInt16 GetObjIdentifier()   const;
};

// virtual objects for Flys
// Flys will always be shown with virtual objects. By doing that, they can be
// shown multiple times if needed (header/footer).
class SwVirtFlyDrawObj : public SdrVirtObj
{
private:
    SwFlyFrm *pFlyFrm;

protected:
    // AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
    // not used but something own (top left of new SnapRect minus top left
    // of original SnapRect)
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    // for paints triggered form ExecutePrimitive
    void wrap_DoPaintObject() const;

    // for simple access to inner and outer bounds
    basegfx::B2DRange getOuterBound() const;
    basegfx::B2DRange getInnerBound() const;

    TYPEINFO_OVERRIDE();

    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);
    ~SwVirtFlyDrawObj();

    // overloaded method of base class SdrVirtObj
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const;

    // we treat the size calculation completely on ourself here
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLastBoundRect() const;
    virtual       void       RecalcBoundRect();
    virtual       void       RecalcSnapRect();
    virtual const Rectangle& GetSnapRect()  const;
    virtual       void       SetSnapRect(const Rectangle& rRect);
    virtual       void       NbcSetSnapRect(const Rectangle& rRect);
    virtual const Rectangle& GetLogicRect() const;
    virtual       void       SetLogicRect(const Rectangle& rRect);
    virtual       void       NbcSetLogicRect(const Rectangle& rRect);
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual       void       NbcMove  (const Size& rSiz);
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact);
    virtual       void       Move  (const Size& rSiz);
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact, bool bUnsetRelative = true);

    const SwFrmFmt *GetFmt() const;
          SwFrmFmt *GetFmt();

    // methods to get pointers for the Fly
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // if an URL is attached to a graphic than this is a macro object
    virtual bool       HasMacro() const;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const;

    // FullDrag support
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
