/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: motionpathtag.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 13:12:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    virtual ULONG GetMarkablePointCount() const;
    virtual ULONG GetMarkedPointCount() const;
    virtual BOOL MarkPoint(SdrHdl& rHdl, BOOL bUnmark=FALSE);
    virtual void CheckPossibilities();
    virtual BOOL MarkPoints(const Rectangle* pRect, BOOL bUnmark);

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // IPolyPolygonEditorController
    virtual void DeleteMarkedPoints();
    virtual BOOL IsDeleteMarkedPointsPossible() const;

    virtual void RipUpAtMarkedPoints();
    virtual bool IsRipUpAtMarkedPointsPossible() const;

    virtual BOOL IsSetMarkedSegmentsKindPossible() const;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    virtual BOOL IsSetMarkedPointsSmoothPossible() const;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    virtual void CloseMarkedObjects(BOOL bToggle, BOOL bOpen );
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

