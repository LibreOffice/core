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

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <svx/svdovirt.hxx>

class SwFlyFrm;
class SwFrameFormat;
class SdrObjMacroHitRec;

const sal_uInt32 SWGInventor =  sal_uInt32('S')*0x00000001 +
                                sal_uInt32('W')*0x00000100 +
                                sal_uInt32('G')*0x00010000;

const sal_uInt16 SwFlyDrawObjIdentifier = 0x0001;

// DrawObjects for Flys
class SwFlyDrawObj : public SdrObject
{
private:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

public:

    SwFlyDrawObj();
    virtual ~SwFlyDrawObj();

    // for instantiation of this class while loading (via factory)
    virtual sal_uInt32 GetObjInventor()     const override;
    virtual sal_uInt16 GetObjIdentifier()   const override;
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
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

public:
    // for paints triggered form ExecutePrimitive
    void wrap_DoPaintObject(
            drawinglayer::geometry::ViewInformation2D const&) const;

    // for simple access to inner and outer bounds
    basegfx::B2DRange getOuterBound() const;
    basegfx::B2DRange getInnerBound() const;


    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);
    virtual ~SwVirtFlyDrawObj();

    // override method of base class SdrVirtObj
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const override;

    // we treat the size calculation completely on ourself here
    virtual const Rectangle& GetCurrentBoundRect() const override;
    virtual const Rectangle& GetLastBoundRect() const override;
    virtual       void       RecalcBoundRect() override;
    virtual       void       RecalcSnapRect() override;
    virtual const Rectangle& GetSnapRect()  const override;
    virtual       void       SetSnapRect(const Rectangle& rRect) override;
    virtual       void       NbcSetSnapRect(const Rectangle& rRect) override;
    virtual const Rectangle& GetLogicRect() const override;
    virtual       void       SetLogicRect(const Rectangle& rRect) override;
    virtual       void       NbcSetLogicRect(const Rectangle& rRect) override;
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual       void       NbcMove  (const Size& rSiz) override;
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact) override;
    virtual       void       NbcCrop(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual       void       Move  (const Size& rSiz) override;
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact, bool bUnsetRelative = true) override;
    virtual       void       Crop(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual       void       addCropHandles(SdrHdlList& rTarget) const override;

    const SwFrameFormat *GetFormat() const;
          SwFrameFormat *GetFormat();

    // methods to get pointers for the Fly
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // if an URL is attached to a graphic than this is a macro object
    virtual bool       HasMacro() const override;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const override;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const override;

    // FullDrag support
    virtual bool supportsFullDrag() const override;
    virtual SdrObject* getFullDragClone() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
