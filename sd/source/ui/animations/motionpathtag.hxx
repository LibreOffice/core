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

#ifndef _SD_MOTIONPATHTAG_HXX_
#define _SD_MOTIONPATHTAG_HXX_

#include <com/sun/star/util/XChangesListener.hpp>
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

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class MotionPathTag : public SmartTag, public IPolyPolygonEditorController, public SfxListener, public ::com::sun::star::util::XChangesListener
{
public:
    MotionPathTag( CustomAnimationPane& rPane, ::sd::View& rView, const CustomAnimationEffectPtr& pEffect );
    virtual ~MotionPathTag();

    SdrPathObj* getPathObj() const { return mpPathObj; }

    /** returns true if the SmartTag handled the event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt );

    // callbacks from sdr view
    virtual sal_uLong GetMarkablePointCount() const;
    virtual sal_uLong GetMarkedPointCount() const;
    virtual sal_Bool MarkPoint(SdrHdl& rHdl, sal_Bool bUnmark=sal_False);
    virtual void CheckPossibilities();
    virtual sal_Bool MarkPoints(const Rectangle* pRect, sal_Bool bUnmark);

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // IPolyPolygonEditorController
    virtual void DeleteMarkedPoints();
    virtual sal_Bool IsDeleteMarkedPointsPossible() const;

    virtual void RipUpAtMarkedPoints();
    virtual bool IsRipUpAtMarkedPointsPossible() const;

    virtual sal_Bool IsSetMarkedSegmentsKindPossible() const;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    virtual sal_Bool IsSetMarkedPointsSmoothPossible() const;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    virtual void CloseMarkedObjects(sal_Bool bToggle, sal_Bool bOpen );
    virtual bool IsOpenCloseMarkedObjectsPossible() const;
    virtual SdrObjClosedKind GetMarkedObjectsClosedState() const;

    void MovePath( int nDX, int nDY );
    bool OnDelete();
    bool OnTabHandles( const KeyEvent& rKEvt );
    bool OnMarkHandle( const KeyEvent& rKEvt );
    bool OnMove( const KeyEvent& rKEvt );

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual bool getContext( SdrViewContext& rContext );
    virtual void disposing();
    virtual void deselect();

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
    rtl::OUString msLastPath;
    bool mbInUpdatePath;
};

} // end of namespace sd

#endif      // _SD_MOTIONPATHTAG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
