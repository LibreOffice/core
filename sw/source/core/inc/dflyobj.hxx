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
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() SAL_OVERRIDE;

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    SwFlyDrawObj();
    ~SwFlyDrawObj();

    // for instantiation of this class while loading (via factory)
    virtual sal_uInt32 GetObjInventor()     const SAL_OVERRIDE;
    virtual sal_uInt16 GetObjIdentifier()   const SAL_OVERRIDE;
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
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

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
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const SAL_OVERRIDE;

    // we treat the size calculation completely on ourself here
    virtual const Rectangle& GetCurrentBoundRect() const SAL_OVERRIDE;
    virtual const Rectangle& GetLastBoundRect() const SAL_OVERRIDE;
    virtual       void       RecalcBoundRect() SAL_OVERRIDE;
    virtual       void       RecalcSnapRect() SAL_OVERRIDE;
    virtual const Rectangle& GetSnapRect()  const SAL_OVERRIDE;
    virtual       void       SetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual       void       NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual const Rectangle& GetLogicRect() const SAL_OVERRIDE;
    virtual       void       SetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual       void       NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
    virtual       void       NbcMove  (const Size& rSiz) SAL_OVERRIDE;
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact) SAL_OVERRIDE;
    virtual       void       Move  (const Size& rSiz) SAL_OVERRIDE;
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact, bool bUnsetRelative = true) SAL_OVERRIDE;

    const SwFrmFmt *GetFmt() const;
          SwFrmFmt *GetFmt();

    // methods to get pointers for the Fly
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // if an URL is attached to a graphic than this is a macro object
    virtual bool       HasMacro() const SAL_OVERRIDE;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const SAL_OVERRIDE;

    // FullDrag support
    virtual bool supportsFullDrag() const SAL_OVERRIDE;
    virtual SdrObject* getFullDragClone() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
