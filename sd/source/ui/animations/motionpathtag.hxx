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
#include <com/sun/star/drawing/XShape.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "smarttag.hxx"
#include "CustomAnimationEffect.hxx"

class SdrPathObj;

namespace sd {

class View;
class CustomAnimationPane;

/// Base class for all functions.
class MotionPathTag : public SmartTag, public IPolyPolygonEditorController, public SfxListener, public ::com::sun::star::util::XChangesListener
{
public:
    MotionPathTag( CustomAnimationPane& rPane, ::sd::View& rView, const CustomAnimationEffectPtr& pEffect );
    virtual ~MotionPathTag();

    SdrPathObj* getPathObj() const { return mpPathObj; }

    /// @return true if the SmartTag handled the event.
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& ) SAL_OVERRIDE;

    /// @return true if the SmartTag consumes this event.
    virtual bool KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

    // callbacks from sdr view
    virtual sal_uLong GetMarkablePointCount() const SAL_OVERRIDE;
    virtual sal_uLong GetMarkedPointCount() const SAL_OVERRIDE;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False) SAL_OVERRIDE;
    virtual void CheckPossibilities() SAL_OVERRIDE;
    virtual sal_Bool MarkPoints(const Rectangle* pRect, sal_Bool bUnmark) SAL_OVERRIDE;

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    // IPolyPolygonEditorController
    virtual void DeleteMarkedPoints() SAL_OVERRIDE;
    virtual bool IsDeleteMarkedPointsPossible() const SAL_OVERRIDE;

    virtual void RipUpAtMarkedPoints() SAL_OVERRIDE;
    virtual bool IsRipUpAtMarkedPointsPossible() const SAL_OVERRIDE;

    virtual bool IsSetMarkedSegmentsKindPossible() const SAL_OVERRIDE;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const SAL_OVERRIDE;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind) SAL_OVERRIDE;

    virtual bool IsSetMarkedPointsSmoothPossible() const SAL_OVERRIDE;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const SAL_OVERRIDE;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind) SAL_OVERRIDE;

    virtual void CloseMarkedObjects(bool bToggle, bool bOpen ) SAL_OVERRIDE;
    virtual bool IsOpenCloseMarkedObjectsPossible() const SAL_OVERRIDE;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const SAL_OVERRIDE;

    void MovePath( int nDX, int nDY );
    bool OnDelete();
    bool OnTabHandles( const KeyEvent& rKEvt );
    bool OnMarkHandle( const KeyEvent& rKEvt );
    bool OnMove( const KeyEvent& rKEvt );

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire(  ) throw () SAL_OVERRIDE;
    virtual void SAL_CALL release(  ) throw () SAL_OVERRIDE;

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList ) SAL_OVERRIDE;
    virtual bool getContext( SdrViewContext& rContext ) SAL_OVERRIDE;
    virtual void disposing() SAL_OVERRIDE;
    virtual void deselect() SAL_OVERRIDE;

    void updatePathAttributes();
    void selectionChanged();

private:
    CustomAnimationPane& mrPane;
    CustomAnimationEffectPtr mpEffect;
    ::basegfx::B2DPolyPolygon mxPolyPoly;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > mxOrigin;
    SdrPathObj* mpPathObj;
    ::com::sun::star::awt::Point maOriginPos;
    SdrMark* mpMark;
    OUString msLastPath;
    bool mbInUpdatePath;
};

} // end of namespace sd

#endif // INCLUDED_SD_SOURCE_UI_ANIMATIONS_MOTIONPATHTAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
