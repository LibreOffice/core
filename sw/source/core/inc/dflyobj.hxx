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
#include <svx/svdobj.hxx>

namespace drawinglayer::geometry { class ViewInformation2D; }

class SwFlyFrame;
class SwFrameFormat;

const sal_uInt16 SwFlyDrawObjIdentifier = 0x0001;

// DrawObjects for Flys
class SwFlyDrawObj : public SdrObject
{
private:
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when RecalcBoundRect() is used
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    // protected destructor
    virtual ~SwFlyDrawObj() override;

public:
    SwFlyDrawObj(SdrModel& rSdrModel);

    // for instantiation of this class while loading (via factory)
    virtual SdrInventor GetObjInventor()     const override;
    virtual sal_uInt16  GetObjIdentifier()   const override;
};

// virtual objects for Flys
// Flys will always be shown with virtual objects. By doing that, they can be
// shown multiple times if needed (header/footer).
class SwVirtFlyDrawObj : public SdrVirtObj
{
private:
    SwFlyFrame *m_pFlyFrame;

    // RotGrfFlyFrame: Helper to access the rotation angle (in 10th degrees, left-handed)
    // of a GraphicFrame
    sal_uInt16 getPossibleRotationFromFraphicFrame(Size& rSize) const;

protected:
    // AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
    // not used but something own (top left of new SnapRect minus top left
    // of original SnapRect)
    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

    // protected destructor
    virtual ~SwVirtFlyDrawObj() override;

public:
    // for paints triggered form ExecutePrimitive
    void wrap_DoPaintObject(
            drawinglayer::geometry::ViewInformation2D const&) const;

    // for simple access to inner and outer bounds
    basegfx::B2DRange getOuterBound() const;
    basegfx::B2DRange getInnerBound() const;

    // RotGrfFlyFrame: Check if this is a SwGrfNode
    bool ContainsSwGrfNode() const;

    SwVirtFlyDrawObj(
        SdrModel& rSdrModel,
        SdrObject& rNew,
        SwFlyFrame* pFly);

    // override method of base class SdrVirtObj
    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const override;

    // we treat the size calculation completely on ourself here
    virtual const tools::Rectangle& GetCurrentBoundRect() const override;
    virtual const tools::Rectangle& GetLastBoundRect() const override;
    virtual       void       RecalcBoundRect() override;
    virtual       void       RecalcSnapRect() override;
    virtual const tools::Rectangle& GetSnapRect()  const override;
    virtual       void       SetSnapRect(const tools::Rectangle& rRect) override;
    virtual       void       NbcSetSnapRect(const tools::Rectangle& rRect) override;
    virtual const tools::Rectangle& GetLogicRect() const override;
    virtual       void       SetLogicRect(const tools::Rectangle& rRect) override;
    virtual       void       NbcSetLogicRect(const tools::Rectangle& rRect) override;
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual       void       NbcMove  (const Size& rSiz) override;
    virtual       void       NbcResize(const Point& rRef, const Fraction& xFact,
                                       const Fraction& yFact) override;
    virtual       void       NbcCrop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact) override;
    virtual       void       Move  (const Size& rSiz) override;
    virtual       void       Resize(const Point& rRef, const Fraction& xFact,
                                    const Fraction& yFact, bool bUnsetRelative = true) override;
    virtual       void       Crop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact) override;
    virtual       void       addCropHandles(SdrHdlList& rTarget) const override;
    virtual       void       Rotate(const Point& rRef, long nAngle, double sn, double cs) override;

    // FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    const SwFrameFormat *GetFormat() const;
          SwFrameFormat *GetFormat();

    // methods to get pointers for the Fly
          SwFlyFrame* GetFlyFrame()         { return m_pFlyFrame; }
    const SwFlyFrame* GetFlyFrame() const   { return m_pFlyFrame; }

    void SetRect() const;

    // if an URL is attached to a graphic than this is a macro object
    virtual bool       HasMacro() const override;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const override;
    virtual PointerStyle GetMacroPointer     (const SdrObjMacroHitRec& rRec) const override;

    // RotGrfFlyFrame: If true, this SdrObject supports only limited rotation.
    virtual bool HasLimitedRotation() const override;

    virtual bool IsTextBox() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
