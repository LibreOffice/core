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
    virtual sal_uInt32 GetMarkablePointCount() const;
    virtual sal_uInt32 GetMarkedPointCount() const;
    virtual bool MarkPoint(SdrHdl& rHdl, bool bUnmark = false);
    virtual void CheckPossibilities();
    virtual bool MarkPoints(const basegfx::B2DRange* pRange, bool bUnmark);

    const CustomAnimationEffectPtr& getEffect() const { return mpEffect; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // IPolyPolygonEditorController
    virtual void DeleteMarkedPoints();
    virtual bool IsDeleteMarkedPointsPossible() const;

    virtual void RipUpAtMarkedPoints();
    virtual bool IsRipUpAtMarkedPointsPossible() const;

    virtual bool IsSetMarkedSegmentsKindPossible() const;
    virtual SdrPathSegmentKind GetMarkedSegmentsKind() const;
    virtual void SetMarkedSegmentsKind(SdrPathSegmentKind eKind);

    virtual bool IsSetMarkedPointsSmoothPossible() const;
    virtual SdrPathSmoothKind GetMarkedPointsSmooth() const;
    virtual void SetMarkedPointsSmooth(SdrPathSmoothKind eKind);

    virtual void CloseMarkedObjects(bool bToggle, bool bOpen );
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
    sdr::selection::Indices maSelectedPoints;
    rtl::OUString msLastPath;
    bool mbInUpdatePath;
};

} // end of namespace sd

#endif      // _SD_MOTIONPATHTAG_HXX_

