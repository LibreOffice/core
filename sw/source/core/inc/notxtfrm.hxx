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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_NOTXTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_NOTXTFRM_HXX

#include "cntfrm.hxx"
#include <node.hxx>

class SwNoTextNode;
class OutputDevice;
class SwBorderAttrs;
struct SwCursorMoveState;

class SwNoTextFrame: public SwContentFrame
{
private:
    friend void FrameFinit();
    const Size& GetSize() const;

    void Format ( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    void PaintPicture( vcl::RenderContext*, const SwRect& ) const;

    virtual void DestroyImpl() override;
    virtual ~SwNoTextFrame() override;

    // RotateFlyFrame3 add TransformableSwFrame
    std::unique_ptr< TransformableSwFrame >     mpTransformableSwFrame;

    // RotateFlyFrame3 - Support for inner frame of a SwGrfNode.
    // Only for local data extraction. To uniquely access information
    // for local transformation, use getFrameArea(Print)Transformation.
    friend double getLocalFrameRotation_from_SwNoTextFrame(const SwNoTextFrame& rNoTextFrame);
    double getLocalFrameRotation() const;

    void ClearCache();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    SwNoTextFrame( SwNoTextNode * const, SwFrame* );

    const SwContentNode *GetNode() const
        { return static_cast<SwContentNode const*>(GetDep()); }
          SwContentNode *GetNode()
        { return static_cast<SwContentNode      *>(GetDep()); }

    virtual bool LeftMargin(SwPaM *) const override;
    virtual bool RightMargin(SwPaM *, bool bAPI = false) const override;

    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual bool GetCharRect( SwRect &, const SwPosition&,
                              SwCursorMoveState* = nullptr, bool bAllowFarAway = true ) const override;
    virtual bool GetCursorOfst(SwPosition* pPos, Point& aPoint,
                     SwCursorMoveState* = nullptr, bool bTestBackground = false) const override;

    void GetGrfArea( SwRect &rRect, SwRect * ) const;

    bool IsTransparent() const;

    void StopAnimation( OutputDevice* = nullptr ) const;
    bool HasAnimation()  const;

    // RotateFlyFrame3 - Support for Transformations
    bool isTransformableSwFrame() const { return bool(mpTransformableSwFrame); }
    TransformableSwFrame* getTransformableSwFrame() { return mpTransformableSwFrame.get(); }
    const TransformableSwFrame* getTransformableSwFrame() const { return mpTransformableSwFrame.get(); }

    // RotateFlyFrame3 - Support for Transformations
    virtual basegfx::B2DHomMatrix getFrameAreaTransformation() const override;
    virtual basegfx::B2DHomMatrix getFramePrintAreaTransformation() const override;

    // RotateFlyFrame3 - Support for Transformations
    virtual void transform_translate(const Point& rOffset) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
