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

#ifndef INCLUDED_SD_SOURCE_UI_ANIMATIONS_MOTIONPATHTAG_HXX
#define INCLUDED_SD_SOURCE_UI_ANIMATIONS_MOTIONPATHTAG_HXX

#include <com/sun/star/util/XChangesListener.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <smarttag.hxx>
#include "CustomAnimationList.hxx"

namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
class SdrPathObj;

namespace sd {

class View;
class CustomAnimationPane;

/// Base class for all functions.
class MotionPathTag final : public SmartTag, public IPolyPolygonEditorController, public SfxListener, public css::util::XChangesListener
{
public:
    MotionPathTag( CustomAnimationPane& rPane, ::sd::View& rView, const CustomAnimationEffectPtr& pEffect );
    virtual ~MotionPathTag() override;

    SdrPathObj* getPathObj() const { return mpPathObj; }

    /// @return true if the SmartTag handled the event.
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& ) override;

    /// @return true if the SmartTag consumes this event.
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    // callbacks from sdr view
    virtual sal_Int32 GetMarkablePointCount() const override;
    virtual sal_Int32 GetMarkedPointCount() const override;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark) override;
    virtual void CheckPossibilities() override;
    virtual bool MarkPoints(const ::tools::Rectangle* pRect, bool bUnmark) override;

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // IPolyPolygonEditorController
    virtual void DeleteMarkedPoints() override;
    virtual bool IsDeleteMarkedPointsPossible() const override;

    virtual void RipUpAtMarkedPoints() override;
    virtual bool IsRipUpAtMarkedPointsPossible() const override;

    virtual bool IsSetMarkedSegmentsKindPossible() const override;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const override;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) override;

    virtual bool IsSetMarkedPointsSmoothPossible() const override;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const override;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) override;

    virtual bool IsOpenCloseMarkedObjectsPossible() const override;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const override;

    void MovePath( int nDX, int nDY );
    bool OnDelete();
    bool OnTabHandles( const KeyEvent& rKEvt );
    bool OnMarkHandle( const KeyEvent& rKEvt );
    bool OnMove( const KeyEvent& rKEvt );

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

private:
    virtual void addCustomHandles( SdrHdlList& rHandlerList ) override;
    virtual bool getContext( SdrViewContext& rContext ) override;
    virtual void disposing() override;
    virtual void deselect() override;

    void updatePathAttributes();
    void selectionChanged();

    CustomAnimationPane& mrPane;
    CustomAnimationEffectPtr mpEffect;
    ::basegfx::B2DPolyPolygon mxPolyPoly;
    css::uno::Reference< css::drawing::XShape > mxOrigin;
    SdrPathObj* mpPathObj;
    css::awt::Point maOriginPos;
    std::unique_ptr<SdrMark> mpMark;
    OUString msLastPath;
    bool mbInUpdatePath;
};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_MOTIONPATHTAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
