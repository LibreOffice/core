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

#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <cppuhelper/weakref.hxx>
#include <vcl/window.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unomod.hxx>
#include <algorithm>
#include <map>
#include <list>
#include <accmap.hxx>
#include <acccontext.hxx>
#include <accdoc.hxx>
#include <accpreview.hxx>
#include <accpage.hxx>
#include <accpara.hxx>
#include <accheaderfooter.hxx>
#include <accfootnote.hxx>
#include <acctextframe.hxx>
#include <accgraphic.hxx>
#include <accembedded.hxx>
#include <acccell.hxx>
#include <acctable.hxx>
#include <fesh.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <hffrm.hxx>
#include <ftnfrm.hxx>
#include <cellfrm.hxx>
#include <tabfrm.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>
#include <ndtyp.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <svx/ShapeTypeHandler.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/implbase1.hxx>
#include <pagepreviewlayout.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>
#include <prevwpage.hxx>
#include <switerator.hxx>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::sw::access;

struct SwFrmFunc
{
    bool operator()( const SwFrm * p1,
                         const SwFrm * p2) const
    {
        return p1 < p2;
    }
};

typedef ::std::map < const SwFrm *, uno::WeakReference < XAccessible >, SwFrmFunc > _SwAccessibleContextMap_Impl;

class SwAccessibleContextMap_Impl: public _SwAccessibleContextMap_Impl
{
public:

#if OSL_DEBUG_LEVEL > 0
    bool mbLocked;
#endif

    SwAccessibleContextMap_Impl()
#if OSL_DEBUG_LEVEL > 0
        : mbLocked( false )
#endif
    {}

};

class SwDrawModellListener_Impl : public SfxListener,
    public ::cppu::WeakImplHelper1< document::XEventBroadcaster >
{
    mutable ::osl::Mutex maListenerMutex;
    ::cppu::OInterfaceContainerHelper maEventListeners;
    SdrModel *mpDrawModel;
protected:
    virtual ~SwDrawModellListener_Impl();

public:
    SwDrawModellListener_Impl( SdrModel *pDrawModel );

    virtual void SAL_CALL addEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException);

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    void Dispose();
};

SwDrawModellListener_Impl::SwDrawModellListener_Impl( SdrModel *pDrawModel ) :
    maEventListeners( maListenerMutex ),
    mpDrawModel( pDrawModel )
{
    StartListening( *mpDrawModel );
}

SwDrawModellListener_Impl::~SwDrawModellListener_Impl()
{
    EndListening( *mpDrawModel );
}

void SAL_CALL SwDrawModellListener_Impl::addEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException)
{
    maEventListeners.addInterface( xListener );
}

void SAL_CALL SwDrawModellListener_Impl::removeEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException)
{
    maEventListeners.removeInterface( xListener );
}

void SwDrawModellListener_Impl::Notify( SfxBroadcaster& /*rBC*/,
        const SfxHint& rHint )
{
    // do not broadcast notifications for writer fly frames, because there
    // are no shapes that need to know about them.
    const SdrHint *pSdrHint = PTR_CAST( SdrHint, &rHint );
    if ( !pSdrHint ||
         ( pSdrHint->GetObject() &&
           ( pSdrHint->GetObject()->ISA(SwFlyDrawObj) ||
             pSdrHint->GetObject()->ISA(SwVirtFlyDrawObj) ||
             IS_TYPE(SdrObject,pSdrHint->GetObject()) ) ) )
    {
        return;
    }

    OSL_ENSURE( mpDrawModel, "draw model listener is disposed" );
    if( !mpDrawModel )
        return;

    document::EventObject aEvent;
    if( !SvxUnoDrawMSFactory::createEvent( mpDrawModel, pSdrHint, aEvent ) )
        return;

    ::cppu::OInterfaceIteratorHelper aIter( maEventListeners );
    while( aIter.hasMoreElements() )
    {
        uno::Reference < document::XEventListener > xListener( aIter.next(),
                                                uno::UNO_QUERY );
        try
        {
            xListener->notifyEvent( aEvent );
        }
        catch( uno::RuntimeException const & r )
        {
            SAL_WARN( "sw.core", "Runtime exception caught while notifying shape.: " << r.Message );
        }
    }
}

void SwDrawModellListener_Impl::Dispose()
{
    mpDrawModel = 0;
}

struct SwShapeFunc
{
    bool operator()( const SdrObject * p1,
                         const SdrObject * p2) const
    {
        return p1 < p2;
    }
};
typedef ::std::map < const SdrObject *, uno::WeakReference < XAccessible >, SwShapeFunc > _SwAccessibleShapeMap_Impl;
typedef ::std::pair < const SdrObject *, ::rtl::Reference < ::accessibility::AccessibleShape > > SwAccessibleObjShape_Impl;

class SwAccessibleShapeMap_Impl: public _SwAccessibleShapeMap_Impl

{
    ::accessibility::AccessibleShapeTreeInfo maInfo;

public:

#if OSL_DEBUG_LEVEL > 0
    bool mbLocked;
#endif
    SwAccessibleShapeMap_Impl( SwAccessibleMap *pMap )
#if OSL_DEBUG_LEVEL > 0
        : mbLocked( false )
#endif
    {
        maInfo.SetSdrView( pMap->GetShell()->GetDrawView() );
        maInfo.SetWindow( pMap->GetShell()->GetWin() );
        maInfo.SetViewForwarder( pMap );
        uno::Reference < document::XEventBroadcaster > xModelBroadcaster =
            new SwDrawModellListener_Impl(
                    pMap->GetShell()->getIDocumentDrawModelAccess()->GetOrCreateDrawModel() );
        maInfo.SetControllerBroadcaster( xModelBroadcaster );
    }

    ~SwAccessibleShapeMap_Impl();

    const ::accessibility::AccessibleShapeTreeInfo& GetInfo() const { return maInfo; }

    SwAccessibleObjShape_Impl *Copy( size_t& rSize,
        const SwFEShell *pFESh = 0,
        SwAccessibleObjShape_Impl  **pSelShape = 0 ) const;
};

SwAccessibleShapeMap_Impl::~SwAccessibleShapeMap_Impl()
{
    uno::Reference < document::XEventBroadcaster > xBrd( maInfo.GetControllerBroadcaster() );
    if( xBrd.is() )
        static_cast < SwDrawModellListener_Impl * >( xBrd.get() )->Dispose();
}

SwAccessibleObjShape_Impl
    *SwAccessibleShapeMap_Impl::Copy(
            size_t& rSize, const SwFEShell *pFESh,
            SwAccessibleObjShape_Impl **pSelStart ) const
{
    SwAccessibleObjShape_Impl *pShapes = 0;
    SwAccessibleObjShape_Impl *pSelShape = 0;

    sal_uInt16 nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;
    rSize = size();

    if( rSize > 0 )
    {
        pShapes =
            new SwAccessibleObjShape_Impl[rSize];

        const_iterator aIter = begin();
        const_iterator aEndIter = end();

        SwAccessibleObjShape_Impl *pShape = pShapes;
        pSelShape = &(pShapes[rSize]);
        while( aIter != aEndIter )
        {
            const SdrObject *pObj = (*aIter).first;
            uno::Reference < XAccessible > xAcc( (*aIter).second );
            if( nSelShapes && pFESh->IsObjSelected( *pObj ) )
            {
                // selected objects are inserted from the back
                --pSelShape;
                pSelShape->first = pObj;
                pSelShape->second =
                    static_cast < ::accessibility::AccessibleShape* >(
                                                    xAcc.get() );
                --nSelShapes;
            }
            else
            {
                pShape->first = pObj;
                pShape->second =
                    static_cast < ::accessibility::AccessibleShape* >(
                                                    xAcc.get() );
                ++pShape;
            }
            ++aIter;
        }
        OSL_ENSURE( pSelShape == pShape, "copying shapes went wrong!" );
    }

    if( pSelStart )
        *pSelStart = pSelShape;

    return pShapes;
}

struct SwAccessibleEvent_Impl
{
public:
    enum EventType { CARET_OR_STATES,
                     INVALID_CONTENT,
                     POS_CHANGED,
                     CHILD_POS_CHANGED,
                     SHAPE_SELECTION,
                     DISPOSE,
                     INVALID_ATTR };

private:
    SwRect      maOldBox;                       // the old bounds for CHILD_POS_CHANGED
                                                // and POS_CHANGED
    uno::WeakReference < XAccessible > mxAcc;   // The object that fires the event
    SwAccessibleChild   maFrmOrObj;             // the child for CHILD_POS_CHANGED and
                                                // the same as xAcc for any other
                                                // event type
    EventType   meType;                         // The event type
    tAccessibleStates mnStates;                 // check states or update caret pos

    SwAccessibleEvent_Impl& operator==( const SwAccessibleEvent_Impl& );

public:
    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrmOrObj )
        : mxAcc( pA ),
          maFrmOrObj( rFrmOrObj ),
          meType( eT ),
          mnStates( 0 )
    {}

    SwAccessibleEvent_Impl( EventType eT,
                            const SwAccessibleChild& rFrmOrObj )
        : maFrmOrObj( rFrmOrObj ),
          meType( eT ),
          mnStates( 0 )
    {
        OSL_ENSURE( SwAccessibleEvent_Impl::DISPOSE == meType,
                "wrong event constructor, DISPOSE only" );
    }

    SwAccessibleEvent_Impl( EventType eT )
        : meType( eT ),
          mnStates( 0 )
    {
        OSL_ENSURE( SwAccessibleEvent_Impl::SHAPE_SELECTION == meType,
                "wrong event constructor, SHAPE_SELECTION only" );
    }

    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrmOrObj,
                            const SwRect& rR )
        : maOldBox( rR ),
          mxAcc( pA ),
          maFrmOrObj( rFrmOrObj ),
          meType( eT ),
          mnStates( 0 )
    {
        OSL_ENSURE( SwAccessibleEvent_Impl::CHILD_POS_CHANGED == meType ||
                SwAccessibleEvent_Impl::POS_CHANGED == meType,
                "wrong event constructor, (CHILD_)POS_CHANGED only" );
    }

    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrmOrObj,
                            const tAccessibleStates _nStates )
        : mxAcc( pA ),
          maFrmOrObj( rFrmOrObj ),
          meType( eT ),
          mnStates( _nStates )
    {
        OSL_ENSURE( SwAccessibleEvent_Impl::CARET_OR_STATES == meType,
                "wrong event constructor, CARET_OR_STATES only" );
    }

    // <SetType(..)> only used in method <SwAccessibleMap::AppendEvent(..)>
    inline void SetType( EventType eT )
    {
        meType = eT;
    }
    inline EventType GetType() const
    {
        return meType;
    }

    inline ::rtl::Reference < SwAccessibleContext > GetContext() const
    {
        uno::Reference < XAccessible > xTmp( mxAcc );
        ::rtl::Reference < SwAccessibleContext > xAccImpl(
                            static_cast<SwAccessibleContext*>( xTmp.get() ) );

        return xAccImpl;
    }

    inline const SwRect& GetOldBox() const
    {
        return maOldBox;
    }
    // <SetOldBox(..)> only used in method <SwAccessibleMap::AppendEvent(..)>
    inline void SetOldBox( const SwRect& rOldBox )
    {
        maOldBox = rOldBox;
    }

    inline const SwAccessibleChild& GetFrmOrObj() const
    {
        return maFrmOrObj;
    }

    // <SetStates(..)> only used in method <SwAccessibleMap::AppendEvent(..)>
    inline void SetStates( tAccessibleStates _nStates )
    {
        mnStates |= _nStates;
    }

    inline bool IsUpdateCursorPos() const
    {
        return (mnStates & ACC_STATE_CARET) != 0;
    }
    inline bool IsInvalidateStates() const
    {
        return (mnStates & ACC_STATE_MASK) != 0;
    }
    inline bool IsInvalidateRelation() const
    {
        return (mnStates & ACC_STATE_RELATION_MASK) != 0;
    }
    inline bool IsInvalidateTextSelection() const
    {
        return ( mnStates & ACC_STATE_TEXT_SELECTION_CHANGED ) != 0;
    }

    inline bool IsInvalidateTextAttrs() const
    {
        return ( mnStates & ACC_STATE_TEXT_ATTRIBUTE_CHANGED ) != 0;
    }

    inline tAccessibleStates GetStates() const
    {
        return mnStates & ACC_STATE_MASK;
    }

    inline tAccessibleStates GetAllStates() const
    {
        return mnStates;
    }
};

typedef ::std::list < SwAccessibleEvent_Impl > _SwAccessibleEventList_Impl;

class SwAccessibleEventList_Impl: public _SwAccessibleEventList_Impl
{
    bool mbFiring;

public:
    SwAccessibleEventList_Impl()
        : mbFiring( false )
    {}

    inline void SetFiring()
    {
        mbFiring = true;
    }
    inline bool IsFiring() const
    {
        return mbFiring;
    }
};

// The shape list is filled if an accessible shape is destroyed. It
// simply keeps a reference to the accessible shape's XShape. These
// references are destroyed within the EndAction when firing events.
// There are two reason for this. First of all, a new accessible shape
// for the XShape might be created soon. It's then cheaper if the XShape
// still exists. The other reason are situations where an accessible shape
// is destroyed within an SwFrmFmt::Modify. In this case, destroying
// the XShape at the same time (indirectly by destroying the accessible
// shape) leads to an assert, because a client of the Modify is destroyed
// within a Modify call.

typedef ::std::list < uno::Reference < drawing::XShape > > _SwShapeList_Impl;

class SwShapeList_Impl: public _SwShapeList_Impl
{
public:
    SwShapeList_Impl() {}
};

struct SwAccessibleChildFunc
{
    bool operator()( const SwAccessibleChild& r1,
                         const SwAccessibleChild& r2 ) const
    {
        const void *p1 = r1.GetSwFrm()
                         ? static_cast < const void * >( r1.GetSwFrm())
                         : ( r1.GetDrawObject()
                             ? static_cast < const void * >( r1.GetDrawObject() )
                             : static_cast < const void * >( r1.GetWindow() ) );
        const void *p2 = r2.GetSwFrm()
                         ? static_cast < const void * >( r2.GetSwFrm())
                         : ( r2.GetDrawObject()
                             ? static_cast < const void * >( r2.GetDrawObject() )
                             : static_cast < const void * >( r2.GetWindow() ) );
        return p1 < p2;
    }
};

typedef ::std::map < SwAccessibleChild, SwAccessibleEventList_Impl::iterator,
                     SwAccessibleChildFunc > _SwAccessibleEventMap_Impl;

class SwAccessibleEventMap_Impl: public _SwAccessibleEventMap_Impl
{
};

struct SwAccessibleParaSelection
{
    xub_StrLen nStartOfSelection;
    xub_StrLen nEndOfSelection;

    SwAccessibleParaSelection( const xub_StrLen _nStartOfSelection,
                               const xub_StrLen _nEndOfSelection )
        : nStartOfSelection( _nStartOfSelection ),
          nEndOfSelection( _nEndOfSelection )
    {}
};

struct SwXAccWeakRefComp
{
    bool operator()( const uno::WeakReference<XAccessible>& _rXAccWeakRef1,
                         const uno::WeakReference<XAccessible>& _rXAccWeakRef2 ) const
    {
        return _rXAccWeakRef1.get() < _rXAccWeakRef2.get();
    }
};

typedef ::std::map< uno::WeakReference < XAccessible >,
                    SwAccessibleParaSelection,
                    SwXAccWeakRefComp > _SwAccessibleSelectedParas_Impl;

class SwAccessibleSelectedParas_Impl: public _SwAccessibleSelectedParas_Impl
{};

// helper class that stores preview data
class SwAccPreviewData
{
    typedef std::vector<Rectangle> Rectangles;
    Rectangles maPreviewRects;
    Rectangles maLogicRects;

    SwRect maVisArea;
    Fraction maScale;

    const SwPageFrm *mpSelPage;

    /** adjust logic page retangle to its visible part

        @param _iorLogicPgSwRect
        input/output parameter - reference to the logic page rectangle, which
        has to be adjusted.

        @param _rPrevwPgSwRect
        input parameter - constant reference to the corresponding preview page
        rectangle; needed to determine the visible part of the logic page rectangle.

        @param _rPrevwWinSize
        input paramter - constant reference to the preview window size in TWIP;
        needed to determine the visible part of the logic page rectangle
    */
    void AdjustLogicPgRectToVisibleArea( SwRect&         _iorLogicPgSwRect,
                                         const SwRect&   _rPrevwPgSwRect,
                                         const Size&     _rPrevwWinSize );

public:
    SwAccPreviewData();
    ~SwAccPreviewData();

    void Update( const SwAccessibleMap& rAccMap,
                 const std::vector<PrevwPage*>& _rPrevwPages,
                 const Fraction&  _rScale,
                 const SwPageFrm* _pSelectedPageFrm,
                 const Size&      _rPrevwWinSize );

    void InvalidateSelection( const SwPageFrm* _pSelectedPageFrm );

    const SwRect& GetVisArea() const;

    MapMode GetMapModeForPreview( ) const;

    /** Adjust the MapMode so that the preview page appears at the
     * proper position. rPoint identifies the page for which the
     * MapMode should be adjusted. If bFromPreview is true, rPoint is
     * a preview coordinate; else it's a document coordinate. */
    void AdjustMapMode( MapMode& rMapMode,
                        const Point& rPoint ) const;

    inline const SwPageFrm *GetSelPage() const { return mpSelPage; }

    void DisposePage(const SwPageFrm *pPageFrm );
};

SwAccPreviewData::SwAccPreviewData() :
    mpSelPage( 0 )
{
}

SwAccPreviewData::~SwAccPreviewData()
{
}

void SwAccPreviewData::Update( const SwAccessibleMap& rAccMap,
                               const std::vector<PrevwPage*>& _rPrevwPages,
                               const Fraction&  _rScale,
                               const SwPageFrm* _pSelectedPageFrm,
                               const Size&      _rPrevwWinSize )
{
    // store preview scaling, maximal preview page size and selected page
    maScale = _rScale;
    mpSelPage = _pSelectedPageFrm;

    // prepare loop on preview pages
    maPreviewRects.clear();
    maLogicRects.clear();
    SwAccessibleChild aPage;
    maVisArea.Clear();

    // loop on preview pages to calculate <maPreviewRects>, <maLogicRects> and
    // <maVisArea>
    for ( std::vector<PrevwPage*>::const_iterator aPageIter = _rPrevwPages.begin();
          aPageIter != _rPrevwPages.end();
          ++aPageIter )
    {
        aPage = (*aPageIter)->pPage;

        // add preview page rectangle to <maPreviewRects>
        Rectangle aPrevwPgRect( (*aPageIter)->aPrevwWinPos, (*aPageIter)->aPageSize );
        maPreviewRects.push_back( aPrevwPgRect );

        // add logic page rectangle to <maLogicRects>
        SwRect aLogicPgSwRect( aPage.GetBox( rAccMap ) );
        Rectangle aLogicPgRect( aLogicPgSwRect.SVRect() );
        maLogicRects.push_back( aLogicPgRect );
        // union visible area with visible part of logic page rectangle
        if ( (*aPageIter)->bVisible )
        {
            if ( !(*aPageIter)->pPage->IsEmptyPage() )
            {
                AdjustLogicPgRectToVisibleArea( aLogicPgSwRect,
                                                SwRect( aPrevwPgRect ),
                                                _rPrevwWinSize );
            }
            if ( maVisArea.IsEmpty() )
                maVisArea = aLogicPgSwRect;
            else
                maVisArea.Union( aLogicPgSwRect );
        }
    }
}

void SwAccPreviewData::InvalidateSelection( const SwPageFrm* _pSelectedPageFrm )
{
    mpSelPage = _pSelectedPageFrm;
    OSL_ENSURE( mpSelPage, "selected page not found" );
}

struct ContainsPredicate
{
    const Point& mrPoint;
    ContainsPredicate( const Point& rPoint ) : mrPoint(rPoint) {}
    bool operator() ( const Rectangle& rRect ) const
    {
        return rRect.IsInside( mrPoint ) ? true : false;
    }
};

const SwRect& SwAccPreviewData::GetVisArea() const
{
    return maVisArea;
}

void SwAccPreviewData::AdjustMapMode( MapMode& rMapMode,
                                      const Point& rPoint ) const
{
    // adjust scale
    rMapMode.SetScaleX( maScale );
    rMapMode.SetScaleY( maScale );

    // find proper rectangle
    Rectangles::const_iterator aBegin = maLogicRects.begin();
    Rectangles::const_iterator aEnd = maLogicRects.end();
    Rectangles::const_iterator aFound = ::std::find_if( aBegin, aEnd,
                                                 ContainsPredicate( rPoint ) );

    if( aFound != aEnd )
    {
        // found! set new origin
        Point aPoint = (maPreviewRects.begin() + (aFound - aBegin))->TopLeft();
        aPoint -= (maLogicRects.begin() + (aFound-aBegin))->TopLeft();
        rMapMode.SetOrigin( aPoint );
    }
    // else: don't adjust MapMode
}

void SwAccPreviewData::DisposePage(const SwPageFrm *pPageFrm )
{
    if( mpSelPage == pPageFrm )
        mpSelPage = 0;
}

// adjust logic page retangle to its visible part
void SwAccPreviewData::AdjustLogicPgRectToVisibleArea(
                            SwRect&         _iorLogicPgSwRect,
                            const SwRect&   _rPrevwPgSwRect,
                            const Size&     _rPrevwWinSize )
{
    // determine preview window rectangle
    const SwRect aPrevwWinSwRect( Point( 0, 0 ), _rPrevwWinSize );
    // calculate visible preview page rectangle
    SwRect aVisPrevwPgSwRect( _rPrevwPgSwRect );
    aVisPrevwPgSwRect.Intersection( aPrevwWinSwRect );
    // adjust logic page rectangle
    SwTwips nTmpDiff;
    // left
    nTmpDiff = aVisPrevwPgSwRect.Left() - _rPrevwPgSwRect.Left();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Left( _iorLogicPgSwRect.Left() + nTmpDiff );
    // top
    nTmpDiff = aVisPrevwPgSwRect.Top() - _rPrevwPgSwRect.Top();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Top( _iorLogicPgSwRect.Top() + nTmpDiff );
    // right
    nTmpDiff = _rPrevwPgSwRect.Right() - aVisPrevwPgSwRect.Right();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Right( _iorLogicPgSwRect.Right() - nTmpDiff );
    // bottom
    nTmpDiff = _rPrevwPgSwRect.Bottom() - aVisPrevwPgSwRect.Bottom();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Bottom( _iorLogicPgSwRect.Bottom() - nTmpDiff );
}

static bool AreInSameTable( const uno::Reference< XAccessible >& rAcc,
                                  const SwFrm *pFrm )
{
    bool bRet = false;

    if( pFrm && pFrm->IsCellFrm() && rAcc.is() )
    {
        // Is it in the same table? We check that
        // by comparing the last table frame in the
        // follow chain, because that's cheaper than
        // searching the first one.
        SwAccessibleContext *pAccImpl =
            static_cast< SwAccessibleContext *>( rAcc.get() );
        if( pAccImpl->GetFrm()->IsCellFrm() )
        {
            const SwTabFrm *pTabFrm1 = pAccImpl->GetFrm()->FindTabFrm();
            while( pTabFrm1->GetFollow() )
                   pTabFrm1 = pTabFrm1->GetFollow();

            const SwTabFrm *pTabFrm2 = pFrm->FindTabFrm();
            while( pTabFrm2->GetFollow() )
                   pTabFrm2 = pTabFrm2->GetFollow();

            bRet = (pTabFrm1 == pTabFrm2);
        }
    }

    return bRet;
}

void SwAccessibleMap::FireEvent( const SwAccessibleEvent_Impl& rEvent )
{
    ::rtl::Reference < SwAccessibleContext > xAccImpl( rEvent.GetContext() );
    if( SwAccessibleEvent_Impl::SHAPE_SELECTION == rEvent.GetType() )
    {
        DoInvalidateShapeSelection();
    }
    else if( xAccImpl.is() && xAccImpl->GetFrm() )
    {
        if ( rEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE &&
             rEvent.IsInvalidateTextAttrs() )
        {
            xAccImpl->InvalidateAttr();
        }
        switch( rEvent.GetType() )
        {
        case SwAccessibleEvent_Impl::INVALID_CONTENT:
            xAccImpl->InvalidateContent();
            break;
        case SwAccessibleEvent_Impl::POS_CHANGED:
            xAccImpl->InvalidatePosOrSize( rEvent.GetOldBox() );
            break;
        case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
            xAccImpl->InvalidateChildPosOrSize( rEvent.GetFrmOrObj(),
                                       rEvent.GetOldBox() );
            break;
        case SwAccessibleEvent_Impl::DISPOSE:
            OSL_ENSURE( xAccImpl.is(),
                    "dispose event has been stored" );
            break;
        case SwAccessibleEvent_Impl::INVALID_ATTR:
            // nothing to do here - handled above
            break;
        default:
            break;
        }
        if( SwAccessibleEvent_Impl::DISPOSE != rEvent.GetType() )
        {
            if( rEvent.IsUpdateCursorPos() )
                xAccImpl->InvalidateCursorPos();
            if( rEvent.IsInvalidateStates() )
                xAccImpl->InvalidateStates( rEvent.GetStates() );
            if( rEvent.IsInvalidateRelation() )
            {
                // both events CONTENT_FLOWS_FROM_RELATION_CHANGED and
                // CONTENT_FLOWS_TO_RELATION_CHANGED are possible
                if ( rEvent.GetAllStates() & ACC_STATE_RELATION_FROM )
                {
                    xAccImpl->InvalidateRelation(
                        AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED );
                }
                if ( rEvent.GetAllStates() & ACC_STATE_RELATION_TO )
                {
                    xAccImpl->InvalidateRelation(
                        AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED );
                }
            }

            if ( rEvent.IsInvalidateTextSelection() )
            {
                xAccImpl->InvalidateTextSelection();
            }
        }
    }
}

void SwAccessibleMap::AppendEvent( const SwAccessibleEvent_Impl& rEvent )
{
    osl::MutexGuard aGuard( maEventMutex );

    if( !mpEvents )
        mpEvents = new SwAccessibleEventList_Impl;
    if( !mpEventMap )
        mpEventMap = new SwAccessibleEventMap_Impl;

    if( mpEvents->IsFiring() )
    {
        // While events are fired new ones are generated. They have to be fired
        // now. This does not work for DISPOSE events!
        OSL_ENSURE( rEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                "dispose event while firing events" );
        FireEvent( rEvent );
    }
    else
    {

        SwAccessibleEventMap_Impl::iterator aIter =
                                        mpEventMap->find( rEvent.GetFrmOrObj() );
        if( aIter != mpEventMap->end() )
        {
            SwAccessibleEvent_Impl aEvent( *(*aIter).second );
            OSL_ENSURE( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            bool bAppendEvent = true;
            switch( rEvent.GetType() )
            {
            case SwAccessibleEvent_Impl::CARET_OR_STATES:
                // A CARET_OR_STATES event is added to any other
                // event only. It is broadcasted after any other event, so the
                // event should be put to the back.
                OSL_ENSURE( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                aEvent.SetStates( rEvent.GetAllStates() );
                break;
            case SwAccessibleEvent_Impl::INVALID_CONTENT:
                // An INVALID_CONTENT event overwrites a CARET_OR_STATES
                // event (but keeps its flags) and it is contained in a
                // POS_CHANGED event.
                // Therefore, the event's type has to be adapted and the event
                // has to be put at the end.
                //
                // fdo#56031 An INVALID_CONTENT event overwrites a INVALID_ATTR
                // event and overwrites its flags
                OSL_ENSURE( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                if( aEvent.GetType() == SwAccessibleEvent_Impl::CARET_OR_STATES )
                    aEvent.SetType( SwAccessibleEvent_Impl::INVALID_CONTENT );
                else if ( aEvent.GetType() == SwAccessibleEvent_Impl::INVALID_ATTR )
                {
                    aEvent.SetType( SwAccessibleEvent_Impl::INVALID_CONTENT );
                    aEvent.SetStates( rEvent.GetAllStates() );
                }

                break;
            case SwAccessibleEvent_Impl::POS_CHANGED:
                // A pos changed event overwrites CARET_STATES (keeping its
                // flags) as well as INVALID_CONTENT. The old box position
                // has to be stored however if the old event is not a
                // POS_CHANGED itself.
                OSL_ENSURE( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                if( aEvent.GetType() != SwAccessibleEvent_Impl::POS_CHANGED )
                    aEvent.SetOldBox( rEvent.GetOldBox() );
                aEvent.SetType( SwAccessibleEvent_Impl::POS_CHANGED );
                break;
            case SwAccessibleEvent_Impl::CHILD_POS_CHANGED:
                // CHILD_POS_CHANGED events can only follow CHILD_POS_CHANGED
                // events. The only action that needs to be done again is
                // to put the old event to the back. The new one cannot be used,
                // because we are interested in the old frame bounds.
                OSL_ENSURE( aEvent.GetType() == SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                break;
            case SwAccessibleEvent_Impl::SHAPE_SELECTION:
                OSL_ENSURE( aEvent.GetType() == SwAccessibleEvent_Impl::SHAPE_SELECTION,
                        "invalid event combination" );
                break;
            case SwAccessibleEvent_Impl::DISPOSE:
                // DISPOSE events overwrite all others. They are not stored
                // but executed immediately to avoid broadcasting of
                // defunctional objects. So what needs to be done here is to
                // remove all events for the frame in question.
                bAppendEvent = false;
                break;
            case SwAccessibleEvent_Impl::INVALID_ATTR:
                OSL_ENSURE( aEvent.GetType() == SwAccessibleEvent_Impl::INVALID_ATTR,
                        "invalid event combination" );
                break;
            }
            if( bAppendEvent )
            {
                mpEvents->erase( (*aIter).second );
                (*aIter).second = mpEvents->insert( mpEvents->end(), aEvent );
            }
            else
            {
                mpEvents->erase( (*aIter).second );
                mpEventMap->erase( aIter );
            }
        }
        else if( SwAccessibleEvent_Impl::DISPOSE != rEvent.GetType() )
        {
            SwAccessibleEventMap_Impl::value_type aEntry( rEvent.GetFrmOrObj(),
                    mpEvents->insert( mpEvents->end(), rEvent ) );
            mpEventMap->insert( aEntry );
        }
    }
}

void SwAccessibleMap::InvalidateCursorPosition(
        const uno::Reference< XAccessible >& rAcc )
{
    SwAccessibleContext *pAccImpl =
        static_cast< SwAccessibleContext *>( rAcc.get() );
    OSL_ENSURE( pAccImpl, "no caret context" );
    OSL_ENSURE( pAccImpl->GetFrm(), "caret context is disposed" );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::CARET_OR_STATES,
                                       pAccImpl,
                                       SwAccessibleChild(pAccImpl->GetFrm()),
                                       ACC_STATE_CARET );
        AppendEvent( aEvent );
    }
    else
    {
        FireEvents();
        // While firing events the current frame might have
        // been disposed because it moved out of the vis area.
        // Setting the cursor for such frames is useless and even
        // causes asserts.
        if( pAccImpl->GetFrm() )
            pAccImpl->InvalidateCursorPos();
    }
}

void SwAccessibleMap::InvalidateShapeSelection()
{
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent(
            SwAccessibleEvent_Impl::SHAPE_SELECTION );
        AppendEvent( aEvent );
    }
    else
    {
        FireEvents();
        DoInvalidateShapeSelection();
    }
}

void SwAccessibleMap::DoInvalidateShapeSelection()
{
    SwAccessibleObjShape_Impl *pShapes = 0;
    SwAccessibleObjShape_Impl *pSelShape = 0;
    size_t nShapes = 0;

    const ViewShell *pVSh = GetShell();
    const SwFEShell *pFESh = pVSh->ISA( SwFEShell ) ?
                            static_cast< const SwFEShell * >( pVSh ) : 0;
    sal_uInt16 nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;

    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        ::std::list< const SwFrm * > aParents;
        Window *pWin = GetShell()->GetWin();
        bool bFocused = pWin && pWin->HasFocus();
        SwAccessibleObjShape_Impl *pShape = pShapes;
        while( nShapes )
        {
            if( pShape->second.is() )
            {
                sal_Bool bChanged;
                if( pShape >= pSelShape )
                {
                    bChanged =
                        pShape->second->SetState( AccessibleStateType::SELECTED );
                    if( bFocused && 1 == nSelShapes )
                        pShape->second->SetState( AccessibleStateType::FOCUSED );
                    else
                        pShape->second->ResetState( AccessibleStateType::FOCUSED );
                }
                else
                {
                    bChanged =
                        pShape->second->ResetState( AccessibleStateType::SELECTED );
                    pShape->second->ResetState( AccessibleStateType::FOCUSED );
                }
                if( bChanged )
                {
                    const SwFrm* pParent = SwAccessibleFrame::GetParent(
                                                    SwAccessibleChild( pShape->first ),
                                                    GetShell()->IsPreView() );
                    aParents.push_back( pParent );
                }
            }

            --nShapes;
            ++pShape;
        }
        if( aParents.size() > 0 )
        {
            ::std::list< const SwFrm * >::const_iterator aIter = aParents.begin();
            ::std::list< const SwFrm * >::const_iterator aEndIter = aParents.end();
            while( aIter != aEndIter )
            {
                ::rtl::Reference< SwAccessibleContext > xParentAccImpl;
                {
                    osl::MutexGuard aGuard( maMutex );
                    if(  mpFrmMap )
                    {
                        SwAccessibleContextMap_Impl::const_iterator aMapIter =
                            mpFrmMap->find( *aIter );
                        if( aMapIter != mpFrmMap->end() )
                        {
                            uno::Reference < XAccessible > xAcc( (*aMapIter).second );
                            xParentAccImpl =
                                static_cast< SwAccessibleContext *>( xAcc.get() );
                        }
                    }
                }
                if( xParentAccImpl.is() )
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
                    xParentAccImpl->FireAccessibleEvent( aEvent );
                }

                ++aIter;
            }
        }

        delete[] pShapes;
    }
}

void SwAccessibleMap::DoInvalidateShapeFocus()
{
    const ViewShell *pVSh = GetShell();
    const SwFEShell *pFESh = pVSh->ISA( SwFEShell ) ?
                            static_cast< const SwFEShell * >( pVSh ) : 0;
    sal_uInt16 nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;

    if( nSelShapes != 1 )
        return;

    SwAccessibleObjShape_Impl *pShapes = 0;
    SwAccessibleObjShape_Impl *pSelShape = 0;
    size_t nShapes = 0;

    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        Window *pWin = GetShell()->GetWin();
        bool bFocused = pWin && pWin->HasFocus();
        SwAccessibleObjShape_Impl  *pShape = pShapes;
        while( nShapes )
        {
            if( pShape->second.is() )
            {
                if( bFocused && pShape >= pSelShape )
                    pShape->second->SetState( AccessibleStateType::FOCUSED );
                else
                    pShape->second->ResetState( AccessibleStateType::FOCUSED );
            }

            --nShapes;
            ++pShape;
        }

        delete[] pShapes;
    }
}

SwAccessibleMap::SwAccessibleMap( ViewShell *pSh ) :
    mpFrmMap( 0  ),
    mpShapeMap( 0  ),
    mpShapes( 0  ),
    mpEvents( 0  ),
    mpEventMap( 0  ),
    mpSelectedParas( 0 ),
    mpVSh( pSh ),
        mpPreview( 0 ),
    mnPara( 1 ),
    mnFootnote( 1 ),
    mnEndnote( 1 ),
    mbShapeSelected( false )
{
    pSh->GetLayout()->AddAccessibleShell();
}

SwAccessibleMap::~SwAccessibleMap()
{
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpFrmMap )
        {
            const SwRootFrm *pRootFrm = GetShell()->GetLayout();
            SwAccessibleContextMap_Impl::iterator aIter = mpFrmMap->find( pRootFrm );
            if( aIter != mpFrmMap->end() )
                xAcc = (*aIter).second;
            if( !xAcc.is() )
                xAcc = new SwAccessibleDocument( this );
        }
    }

    SwAccessibleDocument *pAcc =
        static_cast< SwAccessibleDocument * >( xAcc.get() );
    pAcc->Dispose( sal_True );

    {
        osl::MutexGuard aGuard( maMutex );
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !mpFrmMap || mpFrmMap->empty(),
                "Frame map should be empty after disposing the root frame" );
        if( mpFrmMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpFrmMap->begin();
            while( aIter != mpFrmMap->end() )
            {
                uno::Reference < XAccessible > xTmp = (*aIter).second;
                if( xTmp.is() )
                {
                    SwAccessibleContext *pTmp =
                        static_cast< SwAccessibleContext * >( xTmp.get() );
                    (void) pTmp;
                }
                ++aIter;
            }
        }
        OSL_ENSURE( !mpShapeMap || mpShapeMap->empty(),
                "Object map should be empty after disposing the root frame" );
        if( mpShapeMap )
        {
            SwAccessibleShapeMap_Impl::iterator aIter = mpShapeMap->begin();
            while( aIter != mpShapeMap->end() )
            {
                uno::Reference < XAccessible > xTmp = (*aIter).second;
                if( xTmp.is() )
                {
                    ::accessibility::AccessibleShape *pTmp =
                        static_cast< ::accessibility::AccessibleShape* >( xTmp.get() );
                    (void) pTmp;
                }
                ++aIter;
            }
        }
#endif
        delete mpFrmMap;
        mpFrmMap = 0;
        delete mpShapeMap;
        mpShapeMap = 0;
        delete mpShapes;
        mpShapes = 0;
        delete mpSelectedParas;
        mpSelectedParas = 0;
    }

    delete mpPreview;
    mpPreview = NULL;

    {
        osl::MutexGuard aGuard( maEventMutex );
#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !(mpEvents || mpEventMap), "pending events" );
        if( mpEvents )
        {
            SwAccessibleEventList_Impl::iterator aIter = mpEvents->begin();
            while( aIter != mpEvents->end() )
            {
                ++aIter;
            }
        }
        if( mpEventMap )
        {
            SwAccessibleEventMap_Impl::iterator aIter = mpEventMap->begin();
            while( aIter != mpEventMap->end() )
            {
                ++aIter;
            }
        }
#endif
        delete mpEventMap;
        mpEventMap = 0;
        delete mpEvents;
        mpEvents = 0;
    }
    mpVSh->GetLayout()->RemoveAccessibleShell();
}

uno::Reference< XAccessible > SwAccessibleMap::_GetDocumentView(
    bool bPagePreview )
{
    uno::Reference < XAccessible > xAcc;
    bool bSetVisArea = false;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpFrmMap )
        {
            mpFrmMap = new SwAccessibleContextMap_Impl;
#if OSL_DEBUG_LEVEL > 0
            mpFrmMap->mbLocked = false;
#endif
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !mpFrmMap->mbLocked, "Map is locked" );
        mpFrmMap->mbLocked = true;
#endif

        const SwRootFrm *pRootFrm = GetShell()->GetLayout();
        SwAccessibleContextMap_Impl::iterator aIter = mpFrmMap->find( pRootFrm );
        if( aIter != mpFrmMap->end() )
            xAcc = (*aIter).second;
        if( xAcc.is() )
        {
            bSetVisArea = true; // Set VisArea when map mutex is not locked
        }
        else
        {
            if( bPagePreview )
                xAcc = new SwAccessiblePreview( this );
            else
                xAcc = new SwAccessibleDocument( this );

            if( aIter != mpFrmMap->end() )
            {
                (*aIter).second = xAcc;
            }
            else
            {
                SwAccessibleContextMap_Impl::value_type aEntry( pRootFrm, xAcc );
                mpFrmMap->insert( aEntry );
            }
        }

#if OSL_DEBUG_LEVEL > 0
        mpFrmMap->mbLocked = false;
#endif
    }

    if( bSetVisArea )
    {
        SwAccessibleDocumentBase *pAcc =
            static_cast< SwAccessibleDocumentBase * >( xAcc.get() );
        pAcc->SetVisArea();
    }

    return xAcc;
}

uno::Reference< XAccessible > SwAccessibleMap::GetDocumentView( )
{
    return _GetDocumentView( false );
}

uno::Reference<XAccessible> SwAccessibleMap::GetDocumentPreview(
                                    const std::vector<PrevwPage*>& _rPrevwPages,
                                    const Fraction&  _rScale,
                                    const SwPageFrm* _pSelectedPageFrm,
                                    const Size&      _rPrevwWinSize )
{
    // create & update preview data object
    if( mpPreview == NULL )
        mpPreview = new SwAccPreviewData();
    mpPreview->Update( *this, _rPrevwPages, _rScale, _pSelectedPageFrm, _rPrevwWinSize );

    uno::Reference<XAccessible> xAcc = _GetDocumentView( true );
    return xAcc;
}

uno::Reference< XAccessible> SwAccessibleMap::GetContext( const SwFrm *pFrm,
                                                     sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc;
    uno::Reference < XAccessible > xOldCursorAcc;
    bool bOldShapeSelected = false;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpFrmMap && bCreate )
            mpFrmMap = new SwAccessibleContextMap_Impl;
        if( mpFrmMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpFrmMap->find( pFrm );
            if( aIter != mpFrmMap->end() )
                xAcc = (*aIter).second;

            if( !xAcc.is() && bCreate )
            {
                SwAccessibleContext *pAcc = 0;
                switch( pFrm->GetType() )
                {
                case FRM_TXT:
                    mnPara++;
                    pAcc = new SwAccessibleParagraph( *this,
                                    static_cast< const SwTxtFrm& >( *pFrm ) );
                    break;
                case FRM_HEADER:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwHeaderFrm *>( pFrm ) );
                    break;
                case FRM_FOOTER:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwFooterFrm *>( pFrm ) );
                    break;
                case FRM_FTN:
                    {
                        const SwFtnFrm *pFtnFrm =
                            static_cast < const SwFtnFrm * >( pFrm );
                        sal_Bool bIsEndnote =
                            SwAccessibleFootnote::IsEndnote( pFtnFrm );
                        pAcc = new SwAccessibleFootnote( this, bIsEndnote,
                                    (bIsEndnote ? mnEndnote++ : mnFootnote++),
                                    pFtnFrm );
                    }
                    break;
                case FRM_FLY:
                    {
                        const SwFlyFrm *pFlyFrm =
                            static_cast < const SwFlyFrm * >( pFrm );
                        switch( SwAccessibleFrameBase::GetNodeType( pFlyFrm ) )
                        {
                        case ND_GRFNODE:
                            pAcc = new SwAccessibleGraphic( this, pFlyFrm );
                            break;
                        case ND_OLENODE:
                            pAcc = new SwAccessibleEmbeddedObject( this, pFlyFrm );
                            break;
                        default:
                            pAcc = new SwAccessibleTextFrame( this, pFlyFrm );
                            break;
                        }
                    }
                    break;
                case FRM_CELL:
                    pAcc = new SwAccessibleCell( this,
                                    static_cast< const SwCellFrm *>( pFrm ) );
                    break;
                case FRM_TAB:
                    pAcc = new SwAccessibleTable( this,
                                    static_cast< const SwTabFrm *>( pFrm ) );
                    break;
                case FRM_PAGE:
                    OSL_ENSURE( GetShell()->IsPreView(),
                                "accessible page frames only in PagePreview" );
                    pAcc = new SwAccessiblePage( this, pFrm );
                    break;
                }
                xAcc = pAcc;

                OSL_ENSURE( xAcc.is(), "unknown frame type" );
                if( xAcc.is() )
                {
                    if( aIter != mpFrmMap->end() )
                    {
                        (*aIter).second = xAcc;
                    }
                    else
                    {
                        SwAccessibleContextMap_Impl::value_type aEntry( pFrm, xAcc );
                        mpFrmMap->insert( aEntry );
                    }

                    if( pAcc->HasCursor() &&
                        !AreInSameTable( mxCursorContext, pFrm ) )
                    {
                        // If the new context has the focus, and if we know
                        // another context that had the focus, then the focus
                        // just moves from the old context to the new one. We
                        // then have to send a focus event and a caret event for
                        // the old context. We have to to that now,
                        // because after we have left this method, anyone might
                        // call getStates for the new context and will get a
                        // focused state then. Sending the focus changes event
                        // after that seems to be strange. However, we cannot
                        // send a focus event for the new context now, because
                        // noone except us knows it. In any case, we remember
                        // the new context as the one that has the focus
                        // currently.

                        xOldCursorAcc = mxCursorContext;
                        mxCursorContext = xAcc;

                        bOldShapeSelected = mbShapeSelected;
                        mbShapeSelected = false;
                    }
                }
            }
        }
    }

    // Invalidate focus for old object when map is not locked
    if( xOldCursorAcc.is() )
        InvalidateCursorPosition( xOldCursorAcc );
    if( bOldShapeSelected )
        InvalidateShapeSelection();

    return xAcc;
}

::rtl::Reference < SwAccessibleContext > SwAccessibleMap::GetContextImpl(
            const SwFrm *pFrm,
            sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pFrm, bCreate ) );

    ::rtl::Reference < SwAccessibleContext > xAccImpl(
         static_cast< SwAccessibleContext * >( xAcc.get() ) );

    return xAccImpl;
}

uno::Reference< XAccessible> SwAccessibleMap::GetContext(
        const SdrObject *pObj,
        SwAccessibleContext *pParentImpl,
        sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc;
    uno::Reference < XAccessible > xOldCursorAcc;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpShapeMap && bCreate )
            mpShapeMap = new SwAccessibleShapeMap_Impl( this );
        if( mpShapeMap )
        {
            SwAccessibleShapeMap_Impl::iterator aIter =
                   mpShapeMap->find( pObj );
            if( aIter != mpShapeMap->end() )
                xAcc = (*aIter).second;

            if( !xAcc.is() && bCreate )
            {
                ::accessibility::AccessibleShape *pAcc = 0;
                uno::Reference < drawing::XShape > xShape(
                    const_cast< SdrObject * >( pObj )->getUnoShape(),
                    uno::UNO_QUERY );
                if( xShape.is() )
                {
                    ::accessibility::ShapeTypeHandler& rShapeTypeHandler =
                                ::accessibility::ShapeTypeHandler::Instance();
                    uno::Reference < XAccessible > xParent( pParentImpl );
                    ::accessibility::AccessibleShapeInfo aShapeInfo(
                            xShape, xParent, this );

                    pAcc = rShapeTypeHandler.CreateAccessibleObject(
                                aShapeInfo, mpShapeMap->GetInfo() );
                }
                xAcc = pAcc;

                OSL_ENSURE( xAcc.is(), "unknown shape type" );
                if( xAcc.is() )
                {
                    pAcc->Init();
                    if( aIter != mpShapeMap->end() )
                    {
                        (*aIter).second = xAcc;
                    }
                    else
                    {
                        SwAccessibleShapeMap_Impl::value_type aEntry( pObj,
                                                                      xAcc );
                        mpShapeMap->insert( aEntry );
                    }
                    // TODO: focus!!!
                }
            }
        }
    }

    // Invalidate focus for old object when map is not locked
    if( xOldCursorAcc.is() )
        InvalidateCursorPosition( xOldCursorAcc );

    return xAcc;
}

::rtl::Reference < ::accessibility::AccessibleShape > SwAccessibleMap::GetContextImpl(
            const SdrObject *pObj,
            SwAccessibleContext *pParentImpl,
            sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pObj, pParentImpl, bCreate ) );

    ::rtl::Reference < ::accessibility::AccessibleShape > xAccImpl(
         static_cast< ::accessibility::AccessibleShape* >( xAcc.get() ) );

    return xAccImpl;
}

void SwAccessibleMap::RemoveContext( const SwFrm *pFrm )
{
    osl::MutexGuard aGuard( maMutex );

    if( mpFrmMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            mpFrmMap->find( pFrm );
        if( aIter != mpFrmMap->end() )
        {
            mpFrmMap->erase( aIter );

            // Remove reference to old caret object. Though mxCursorContext
            // is a weak reference and cleared automatically, clearing it
            // directly makes sure to not keep a non-functional object.
            uno::Reference < XAccessible > xOldAcc( mxCursorContext );
            if( xOldAcc.is() )
            {
                SwAccessibleContext *pOldAccImpl =
                    static_cast< SwAccessibleContext *>( xOldAcc.get() );
                OSL_ENSURE( pOldAccImpl->GetFrm(), "old caret context is disposed" );
                if( pOldAccImpl->GetFrm() == pFrm )
                {
                    xOldAcc.clear();    // get an empty ref
                    mxCursorContext = xOldAcc;
                }
            }

            if( mpFrmMap->empty() )
            {
                delete mpFrmMap;
                mpFrmMap = 0;
            }
        }
    }
}

void SwAccessibleMap::RemoveContext( const SdrObject *pObj )
{
    osl::MutexGuard aGuard( maMutex );

    if( mpShapeMap )
    {
        SwAccessibleShapeMap_Impl::iterator aIter =
            mpShapeMap->find( pObj );
        if( aIter != mpShapeMap->end() )
        {
            mpShapeMap->erase( aIter );

            // The shape selection flag is not cleared, but one might do
            // so but has to make sure that the removed context is the one
            // that is selected.

            if( mpShapeMap->empty() )
            {
                delete mpShapeMap;
                mpShapeMap = 0;
            }
        }
    }
}

void SwAccessibleMap::Dispose( const SwFrm *pFrm,
                               const SdrObject *pObj,
                               Window* pWindow,
                               sal_Bool bRecursive )
{
    SwAccessibleChild aFrmOrObj( pFrm, pObj, pWindow );

    // Indeed, the following assert checks the frame's accessible flag,
    // because that's the one that is evaluated in the layout. The frame
    // might not be accessible anyway. That's the case for cell frames that
    // contain further cells.
    OSL_ENSURE( !aFrmOrObj.GetSwFrm() || aFrmOrObj.GetSwFrm()->IsAccessibleFrm(),
            "non accessible frame should be disposed" );

    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        ::rtl::Reference< SwAccessibleContext > xAccImpl;
        ::rtl::Reference< SwAccessibleContext > xParentAccImpl;
        ::rtl::Reference< ::accessibility::AccessibleShape > xShapeAccImpl;
        // get accessible context for frame
        {
            osl::MutexGuard aGuard( maMutex );

            // First of all look for an accessible context for a frame
            if( aFrmOrObj.GetSwFrm() && mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                {
                    uno::Reference < XAccessible > xAcc( (*aIter).second );
                    xAccImpl =
                        static_cast< SwAccessibleContext *>( xAcc.get() );
                }
            }
            if( !xAccImpl.is() && mpFrmMap )
            {
                // If there is none, look if the parent is accessible.
                const SwFrm *pParent =
                        SwAccessibleFrame::GetParent( aFrmOrObj,
                                                      GetShell()->IsPreView());

                if( pParent )
                {
                    SwAccessibleContextMap_Impl::iterator aIter =
                        mpFrmMap->find( pParent );
                    if( aIter != mpFrmMap->end() )
                    {
                        uno::Reference < XAccessible > xAcc( (*aIter).second );
                        xParentAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );
                    }
                }
            }
            if( !xParentAccImpl.is() && !aFrmOrObj.GetSwFrm() &&
                mpShapeMap )
            {
                SwAccessibleShapeMap_Impl::iterator aIter =
                    mpShapeMap->find( aFrmOrObj.GetDrawObject() );
                if( aIter != mpShapeMap->end() )
                {
                    uno::Reference < XAccessible > xAcc( (*aIter).second );
                    xShapeAccImpl =
                        static_cast< ::accessibility::AccessibleShape *>( xAcc.get() );
                }
            }
            if( pObj && GetShell()->ActionPend() &&
                (xParentAccImpl.is() || xShapeAccImpl.is()) )
            {
                // Keep a reference to the XShape to avoid that it
                // is deleted with a SwFrmFmt::Modify.
                uno::Reference < drawing::XShape > xShape(
                    const_cast< SdrObject * >( pObj )->getUnoShape(),
                    uno::UNO_QUERY );
                if( xShape.is() )
                {
                    if( !mpShapes )
                        mpShapes = new SwShapeList_Impl;
                    mpShapes->push_back( xShape );
                }
            }
        }

        // remove events stored for the frame
        {
            osl::MutexGuard aGuard( maEventMutex );
            if( mpEvents )
            {
                SwAccessibleEventMap_Impl::iterator aIter =
                    mpEventMap->find( aFrmOrObj );
                if( aIter != mpEventMap->end() )
                {
                    SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::DISPOSE, aFrmOrObj );
                    AppendEvent( aEvent );
                }
            }
        }

        // If the frame is accessible and there is a context for it, dispose
        // the frame. If the frame is no context for it but disposing should
        // take place recursive, the frame's children have to be disposed
        // anyway, so we have to create the context then.
        if( xAccImpl.is() )
        {
            xAccImpl->Dispose( bRecursive );
        }
        else if( xParentAccImpl.is() )
        {
            // If the frame is a cell frame, the table must be notified.
            // If we are in an action, a table model change event will
            // be broadcasted at the end of the action to give the table
            // a chance to generate a single table change event.

            xParentAccImpl->DisposeChild( aFrmOrObj, bRecursive );
        }
        else if( xShapeAccImpl.is() )
        {
            RemoveContext( aFrmOrObj.GetDrawObject() );
            xShapeAccImpl->dispose();
        }

        if( mpPreview && pFrm && pFrm->IsPageFrm() )
            mpPreview->DisposePage( static_cast< const SwPageFrm *>( pFrm ) );
    }
}

void SwAccessibleMap::InvalidatePosOrSize( const SwFrm *pFrm,
                                           const SdrObject *pObj,
                                           Window* pWindow,
                                           const SwRect& rOldBox )
{
    SwAccessibleChild aFrmOrObj( pFrm, pObj, pWindow );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        ::rtl::Reference< SwAccessibleContext > xAccImpl;
        ::rtl::Reference< SwAccessibleContext > xParentAccImpl;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                if( aFrmOrObj.GetSwFrm() )
                {
                    SwAccessibleContextMap_Impl::iterator aIter =
                        mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                    if( aIter != mpFrmMap->end() )
                    {
                        // If there is an accesible object already it is
                        // notified directly.
                        uno::Reference < XAccessible > xAcc( (*aIter).second );
                        xAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );
                    }
                }
                if( !xAccImpl.is() )
                {
                    // Otherwise we look if the parent is accessible.
                    // If not, there is nothing to do.
                    const SwFrm *pParent =
                        SwAccessibleFrame::GetParent( aFrmOrObj,
                                                      GetShell()->IsPreView());

                    if( pParent )
                    {
                        SwAccessibleContextMap_Impl::iterator aIter =
                            mpFrmMap->find( pParent );
                        if( aIter != mpFrmMap->end() )
                        {
                            uno::Reference < XAccessible > xAcc( (*aIter).second );
                            xParentAccImpl =
                                static_cast< SwAccessibleContext *>( xAcc.get() );
                        }
                    }
                }
            }
        }

        if( xAccImpl.is() )
        {
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::POS_CHANGED, xAccImpl.get(),
                    aFrmOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                xAccImpl->InvalidatePosOrSize( rOldBox );
            }
        }
        else if( xParentAccImpl.is() )
        {
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    xParentAccImpl.get(), aFrmOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                xParentAccImpl->InvalidateChildPosOrSize( aFrmOrObj,
                                                          rOldBox );
            }
        }
    }
}

void SwAccessibleMap::InvalidateContent( const SwFrm *pFrm )
{
    SwAccessibleChild aFrmOrObj( pFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                    xAcc = (*aIter).second;
            }
        }

        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::INVALID_CONTENT, pAccImpl,
                    aFrmOrObj );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                pAccImpl->InvalidateContent();
            }
        }
    }
}

void SwAccessibleMap::InvalidateAttr( const SwTxtFrm& rTxtFrm )
{
    SwAccessibleChild aFrmOrObj( &rTxtFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                    xAcc = (*aIter).second;
            }
        }

        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::INVALID_ATTR,
                                               pAccImpl, aFrmOrObj );
                aEvent.SetStates( ACC_STATE_TEXT_ATTRIBUTE_CHANGED );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                pAccImpl->InvalidateAttr();
            }
        }
    }
}

void SwAccessibleMap::InvalidateCursorPosition( const SwFrm *pFrm )
{
    SwAccessibleChild aFrmOrObj( pFrm );
    bool bShapeSelected = false;
    const ViewShell *pVSh = GetShell();
    if( pVSh->ISA( SwCrsrShell ) )
    {
        const SwCrsrShell *pCSh = static_cast< const SwCrsrShell * >( pVSh );
        if( pCSh->IsTableMode() )
        {
            while( aFrmOrObj.GetSwFrm() && !aFrmOrObj.GetSwFrm()->IsCellFrm() )
                aFrmOrObj = aFrmOrObj.GetSwFrm()->GetUpper();
        }
        else if( pVSh->ISA( SwFEShell ) )
        {
            const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
            const SwFrm *pFlyFrm = pFESh->GetCurrFlyFrm();
            if( pFlyFrm )
            {
                OSL_ENSURE( !pFrm || pFrm->FindFlyFrm() == pFlyFrm,
                        "cursor is not contained in fly frame" );
                aFrmOrObj = pFlyFrm;
            }
            else if( pFESh->IsObjSelected() > 0 )
            {
                bShapeSelected = true;
                aFrmOrObj = static_cast<const SwFrm *>( 0 );
            }
        }
    }

    OSL_ENSURE( bShapeSelected || aFrmOrObj.IsAccessible(GetShell()->IsPreView()),
            "frame is not accessible" );

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    bool bOldShapeSelected = false;

    {
        osl::MutexGuard aGuard( maMutex );

        xOldAcc = mxCursorContext;
        mxCursorContext = xAcc; // clear reference

        bOldShapeSelected = mbShapeSelected;
        mbShapeSelected = bShapeSelected;

        if( aFrmOrObj.GetSwFrm() && mpFrmMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter =
                mpFrmMap->find( aFrmOrObj.GetSwFrm() );
            if( aIter != mpFrmMap->end() )
                xAcc = (*aIter).second;

            // For cells, some extra thoughts are necessary,
            // because invalidating the cursor for one cell
            // invalidates the cursor for all cells of the same
            // table. For this reason, we don't want to
            // invalidate the cursor for the old cursor object
            // and the new one if they are within the same table,
            // because this would result in doing the work twice.
            // Moreover, we have to make sure to invalidate the
            // cursor even if the current cell has no accessible object.
            // If the old cursor objects exists and is in the same
            // table, its the best choice, because using it avoids
            // an unnessarary cursor invalidation cycle when creating
            // a new object for the current cell.
            if( aFrmOrObj.GetSwFrm()->IsCellFrm() )
            {
                if( xOldAcc.is() &&
                    AreInSameTable( xOldAcc, aFrmOrObj.GetSwFrm() ) )
                {
                    if( xAcc.is() )
                        xOldAcc = xAcc; // avoid extra invalidation
                    else
                        xAcc = xOldAcc; // make sure ate least one
                }
                if( !xAcc.is() )
                    xAcc = GetContext( aFrmOrObj.GetSwFrm(), sal_True );
            }
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( bOldShapeSelected || bShapeSelected )
        InvalidateShapeSelection();
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

void SwAccessibleMap::InvalidateFocus()
{
    uno::Reference < XAccessible > xAcc;
    bool bShapeSelected;
    {
        osl::MutexGuard aGuard( maMutex );

        xAcc = mxCursorContext;
        bShapeSelected = mbShapeSelected;
    }

    if( xAcc.is() )
    {
        SwAccessibleContext *pAccImpl =
            static_cast< SwAccessibleContext *>( xAcc.get() );
        pAccImpl->InvalidateFocus();
    }
    else if( bShapeSelected  )
    {
        DoInvalidateShapeFocus();
    }
}

void SwAccessibleMap::SetCursorContext(
        const ::rtl::Reference < SwAccessibleContext >& rCursorContext )
{
    osl::MutexGuard aGuard( maMutex );
    uno::Reference < XAccessible > xAcc( rCursorContext.get() );
    mxCursorContext = xAcc;
}

void SwAccessibleMap::InvalidateStates( tAccessibleStates _nStates,
                                        const SwFrm* _pFrm )
{
    // Start with the frame or the first upper that is accessible
    SwAccessibleChild aFrmOrObj( _pFrm );
    while( aFrmOrObj.GetSwFrm() &&
            !aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
        aFrmOrObj = aFrmOrObj.GetSwFrm()->GetUpper();
    if( !aFrmOrObj.GetSwFrm() )
        aFrmOrObj = GetShell()->GetLayout();

    uno::Reference< XAccessible > xAcc( GetContext( aFrmOrObj.GetSwFrm(), sal_True ) );
    SwAccessibleContext *pAccImpl =
        static_cast< SwAccessibleContext *>( xAcc.get() );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::CARET_OR_STATES,
                                       pAccImpl,
                                       SwAccessibleChild(pAccImpl->GetFrm()),
                                       _nStates );
        AppendEvent( aEvent );
    }
    else
    {
        FireEvents();
        pAccImpl->InvalidateStates( _nStates );
    }
}

void SwAccessibleMap::_InvalidateRelationSet( const SwFrm* pFrm,
                                              bool bFrom )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrmOrObj( pFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                {
                    xAcc = (*aIter).second;
                }
            }
        }

        // deliver event directly, or queue event
        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::CARET_OR_STATES,
                                               pAccImpl, SwAccessibleChild(pFrm),
                                               ( bFrom
                                                 ? ACC_STATE_RELATION_FROM
                                                 : ACC_STATE_RELATION_TO ) );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                pAccImpl->InvalidateRelation( bFrom
                        ? AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED
                        : AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED );
            }
        }
    }
}

void SwAccessibleMap::InvalidateRelationSet( const SwFrm* pMaster,
                                             const SwFrm* pFollow )
{
    _InvalidateRelationSet( pMaster, false );
    _InvalidateRelationSet( pFollow, true );
}

// invalidation of CONTENT_FLOW_FROM/_TO relation of a paragraph
void SwAccessibleMap::InvalidateParaFlowRelation( const SwTxtFrm& _rTxtFrm,
                                                  const bool _bFrom )
{
    _InvalidateRelationSet( &_rTxtFrm, _bFrom );
}

// invalidation of text selection of a paragraph
void SwAccessibleMap::InvalidateParaTextSelection( const SwTxtFrm& _rTxtFrm )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrmOrObj( &_rTxtFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                {
                    xAcc = (*aIter).second;
                }
            }
        }

        // deliver event directly, or queue event
        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::CARET_OR_STATES,
                    pAccImpl,
                    SwAccessibleChild( &_rTxtFrm ),
                    ACC_STATE_TEXT_SELECTION_CHANGED );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                pAccImpl->InvalidateTextSelection();
            }
        }
    }
}

sal_Int32 SwAccessibleMap::GetChildIndex( const SwFrm& rParentFrm,
                                          Window& rChild ) const
{
    sal_Int32 nIndex( -1 );

    SwAccessibleChild aFrmOrObj( &rParentFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrmMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrmMap->find( aFrmOrObj.GetSwFrm() );
                if( aIter != mpFrmMap->end() )
                {
                    xAcc = (*aIter).second;
                }
            }
        }

        if( xAcc.is() )
        {
            SwAccessibleContext *pAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );

            nIndex = pAccImpl->GetChildIndex( const_cast<SwAccessibleMap&>(*this),
                                              SwAccessibleChild( &rChild ) );
        }
    }

    return nIndex;
}

void SwAccessibleMap::UpdatePreview( const std::vector<PrevwPage*>& _rPrevwPages,
                                     const Fraction&  _rScale,
                                     const SwPageFrm* _pSelectedPageFrm,
                                     const Size&      _rPrevwWinSize )
{
    OSL_ENSURE( GetShell()->IsPreView(), "no preview?" );
    OSL_ENSURE( mpPreview != NULL, "no preview data?" );

    mpPreview->Update( *this, _rPrevwPages, _rScale, _pSelectedPageFrm, _rPrevwWinSize );

    // propagate change of VisArea through the document's
    // accessibility tree; this will also send appropriate scroll
    // events
    SwAccessibleContext* pDoc =
        GetContextImpl( GetShell()->GetLayout() ).get();
    static_cast<SwAccessibleDocumentBase*>( pDoc )->SetVisArea();

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );

        xOldAcc = mxCursorContext;

        const SwPageFrm *pSelPage = mpPreview->GetSelPage();
        if( pSelPage && mpFrmMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter =
                mpFrmMap->find( pSelPage );
            if( aIter != mpFrmMap->end() )
                xAcc = (*aIter).second;
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

void SwAccessibleMap::InvalidatePreViewSelection( sal_uInt16 nSelPage )
{
    OSL_ENSURE( GetShell()->IsPreView(), "no preview?" );
    OSL_ENSURE( mpPreview != NULL, "no preview data?" );

    mpPreview->InvalidateSelection( GetShell()->GetLayout()->GetPageByPageNum( nSelPage ) );

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );

        xOldAcc = mxCursorContext;

        const SwPageFrm *pSelPage = mpPreview->GetSelPage();
        if( pSelPage && mpFrmMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter =
                mpFrmMap->find( pSelPage );
            if( aIter != mpFrmMap->end() )
                xAcc = (*aIter).second;
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

bool SwAccessibleMap::IsPageSelected( const SwPageFrm *pPageFrm ) const
{
    return mpPreview && mpPreview->GetSelPage() == pPageFrm;
}

void SwAccessibleMap::FireEvents()
{
    {
        osl::MutexGuard aGuard( maEventMutex );
        if( mpEvents )
        {
            mpEvents->SetFiring();
            ::std::for_each(mpEvents->begin(), mpEvents->end(),
                            boost::bind(&SwAccessibleMap::FireEvent, this, _1));

            delete mpEventMap;
            mpEventMap = 0;

            delete mpEvents;
            mpEvents = 0;
        }
    }
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapes )
        {
            delete mpShapes;
            mpShapes = 0;
        }
    }

}

sal_Bool SwAccessibleMap::IsValid() const
{
    return sal_True;
}

Rectangle SwAccessibleMap::GetVisibleArea() const
{
    MapMode aSrc( MAP_TWIP );
    MapMode aDest( MAP_100TH_MM );
    return OutputDevice::LogicToLogic( GetVisArea().SVRect(), aSrc, aDest );
}

// Convert a MM100 value relative to the document root into a pixel value
// relative to the screen!
Point SwAccessibleMap::LogicToPixel( const Point& rPoint ) const
{
    MapMode aSrc( MAP_100TH_MM );
    MapMode aDest( MAP_TWIP );

    Point aPoint = rPoint;

    aPoint = OutputDevice::LogicToLogic( aPoint, aSrc, aDest );
    Window *pWin = GetShell()->GetWin();
    if( pWin )
    {
        MapMode aMapMode;
        GetMapMode( aPoint, aMapMode );
        aPoint = pWin->LogicToPixel( aPoint, aMapMode );
        aPoint = pWin->OutputToAbsoluteScreenPixel( aPoint );
    }

    return aPoint;
}

Size SwAccessibleMap::LogicToPixel( const Size& rSize ) const
{
    MapMode aSrc( MAP_100TH_MM );
    MapMode aDest( MAP_TWIP );
    Size aSize( OutputDevice::LogicToLogic( rSize, aSrc, aDest ) );
    if( GetShell()->GetWin() )
    {
        MapMode aMapMode;
        GetMapMode( Point(0,0), aMapMode );
        aSize = GetShell()->GetWin()->LogicToPixel( aSize, aMapMode );
    }

    return aSize;
}

Point SwAccessibleMap::PixelToLogic( const Point& rPoint ) const
{
    Point aPoint;
    Window *pWin = GetShell()->GetWin();
    if( pWin )
    {
        aPoint = pWin->ScreenToOutputPixel( rPoint );
        MapMode aMapMode;
        GetMapMode( aPoint, aMapMode );
        aPoint = pWin->PixelToLogic( aPoint, aMapMode );
        MapMode aSrc( MAP_TWIP );
        MapMode aDest( MAP_100TH_MM );
        aPoint = OutputDevice::LogicToLogic( aPoint, aSrc, aDest );
    }

    return aPoint;
}

Size SwAccessibleMap::PixelToLogic( const Size& rSize ) const
{
    Size aSize;
    if( GetShell()->GetWin() )
    {
        MapMode aMapMode;
        GetMapMode( Point(0,0), aMapMode );
        aSize = GetShell()->GetWin()->PixelToLogic( rSize, aMapMode );
        MapMode aSrc( MAP_TWIP );
        MapMode aDest( MAP_100TH_MM );
        aSize = OutputDevice::LogicToLogic( aSize, aSrc, aDest );
    }

    return aSize;
}

sal_Bool SwAccessibleMap::ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        const uno::Reference< drawing::XShape >& _rxShape,
        const long /*_nIndex*/,
        const ::accessibility::AccessibleShapeTreeInfo& /*_rShapeTreeInfo*/
    )   throw (uno::RuntimeException)
{
    const SdrObject *pObj = 0;
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
        {
            SwAccessibleShapeMap_Impl::const_iterator aIter = mpShapeMap->begin();
            SwAccessibleShapeMap_Impl::const_iterator aEndIter = mpShapeMap->end();
            while( aIter != aEndIter && !pObj )
            {
                uno::Reference < XAccessible > xAcc( (*aIter).second );
                ::accessibility::AccessibleShape *pAccShape =
                    static_cast < ::accessibility::AccessibleShape* >( xAcc.get() );
                if( pAccShape == pCurrentChild )
                {
                    pObj = (*aIter).first;
                }
                ++aIter;
            }
        }
    }
    if( !pObj )
        return sal_False;

    uno::Reference < drawing::XShape > xShape( _rxShape );  // keep reference to shape, because
                                                            // we might be the only one that
                                                            // holds it.
    // Also get keep parent.
    uno::Reference < XAccessible > xParent( pCurrentChild->getAccessibleParent() );
    pCurrentChild = 0;  // will be released by dispose
    Dispose( 0, pObj, 0 );

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpShapeMap )
            mpShapeMap = new SwAccessibleShapeMap_Impl( this );

        // create the new child
        ::accessibility::ShapeTypeHandler& rShapeTypeHandler =
                        ::accessibility::ShapeTypeHandler::Instance();
        ::accessibility::AccessibleShapeInfo aShapeInfo(
                                            xShape, xParent, this );
        ::accessibility::AccessibleShape* pReplacement =
            rShapeTypeHandler.CreateAccessibleObject (
                aShapeInfo, mpShapeMap->GetInfo() );

        uno::Reference < XAccessible > xAcc( pReplacement );
        if( xAcc.is() )
        {
            pReplacement->Init();

            SwAccessibleShapeMap_Impl::iterator aIter =
                mpShapeMap->find( pObj );
            if( aIter != mpShapeMap->end() )
            {
                (*aIter).second = xAcc;
            }
            else
            {
                SwAccessibleShapeMap_Impl::value_type aEntry( pObj, xAcc );
                mpShapeMap->insert( aEntry );
            }
        }
    }

    SwRect aEmptyRect;
    InvalidatePosOrSize( 0, pObj, 0, aEmptyRect );

    return sal_True;
}

Point SwAccessibleMap::PixelToCore( const Point& rPoint ) const
{
    Point aPoint;
    if( GetShell()->GetWin() )
    {
        MapMode aMapMode;
        GetMapMode( rPoint, aMapMode );
        aPoint = GetShell()->GetWin()->PixelToLogic( rPoint, aMapMode );
    }
    return aPoint;
}

static inline long lcl_CorrectCoarseValue(long aCoarseValue, long aFineValue,
                                          long aRefValue, bool bToLower)
{
    long aResult = aCoarseValue;

    if (bToLower)
    {
        if (aFineValue < aRefValue)
            aResult -= 1;
    }
    else
    {
        if (aFineValue > aRefValue)
            aResult += 1;
    }

    return aResult;
}

static inline void lcl_CorrectRectangle(Rectangle & rRect,
                                        const Rectangle & rSource,
                                        const Rectangle & rInGrid)
{
    rRect.Left() = lcl_CorrectCoarseValue(rRect.Left(), rSource.Left(),
                                          rInGrid.Left(), false);
    rRect.Top() = lcl_CorrectCoarseValue(rRect.Top(), rSource.Top(),
                                         rInGrid.Top(), false);
    rRect.Right() = lcl_CorrectCoarseValue(rRect.Right(), rSource.Right(),
                                           rInGrid.Right(), true);
    rRect.Bottom() = lcl_CorrectCoarseValue(rRect.Bottom(), rSource.Bottom(),
                                            rInGrid.Bottom(), true);
}

Rectangle SwAccessibleMap::CoreToPixel( const Rectangle& rRect ) const
{
    Rectangle aRect;
    if( GetShell()->GetWin() )
    {
        MapMode aMapMode;
        GetMapMode( rRect.TopLeft(), aMapMode );
        aRect = GetShell()->GetWin()->LogicToPixel( rRect, aMapMode );

        Rectangle aTmpRect = GetShell()->GetWin()->PixelToLogic( aRect, aMapMode );
        lcl_CorrectRectangle(aRect, rRect, aTmpRect);
    }

    return aRect;
}

/** get mapping mode for LogicToPixel and PixelToLogic conversions

    Method returns mapping mode of current output device and adjusts it,
    if the shell is in page/print preview.
    Necessary, because <PreviewAdjust(..)> changes mapping mode at current
    output device for mapping logic document positions to page preview window
    positions and vice versa and doesn't take care to recover its changes.
*/
void SwAccessibleMap::GetMapMode( const Point& _rPoint,
                                  MapMode&     _orMapMode ) const
{
    MapMode aMapMode = GetShell()->GetWin()->GetMapMode();
    if( GetShell()->IsPreView() )
    {
        OSL_ENSURE( mpPreview != NULL, "need preview data" );

        mpPreview->AdjustMapMode( aMapMode, _rPoint );
    }
    _orMapMode = aMapMode;
}

Size SwAccessibleMap::GetPreViewPageSize( sal_uInt16 _nPrevwPageNum ) const
{
    OSL_ENSURE( mpVSh->IsPreView(), "no page preview accessible." );
    OSL_ENSURE( mpVSh->IsPreView() && ( mpPreview != NULL ),
                "missing accessible preview data at page preview" );
    if ( mpVSh->IsPreView() && ( mpPreview != NULL ) )
    {
        return mpVSh->PagePreviewLayout()->GetPrevwPageSizeByPageNum( _nPrevwPageNum );
    }
    else
    {
        return Size( 0, 0 );
    }
}

/** method to build up a new data structure of the accessible pararaphs,
    which have a selection
    Important note: method has to be used inside a mutual exclusive section
*/
SwAccessibleSelectedParas_Impl* SwAccessibleMap::_BuildSelectedParas()
{
    // no accessible contexts, no selection
    if ( !mpFrmMap )
    {
        return 0L;
    }

    // get cursor as an instance of its base class <SwPaM>
    SwPaM* pCrsr( 0L );
    {
        SwCrsrShell* pCrsrShell = dynamic_cast<SwCrsrShell*>(GetShell());
        if ( pCrsrShell )
        {
            SwFEShell* pFEShell = dynamic_cast<SwFEShell*>(pCrsrShell);
            if ( !pFEShell ||
                 ( !pFEShell->IsFrmSelected() &&
                   pFEShell->IsObjSelected() == 0 ) )
            {
                // get cursor without updating an existing table cursor.
                pCrsr = pCrsrShell->GetCrsr( sal_False );
            }
        }
    }
    // no cursor, no selection
    if ( !pCrsr )
    {
        return 0L;
    }

    SwAccessibleSelectedParas_Impl* pRetSelectedParas( 0L );

    // loop on all cursors
    SwPaM* pRingStart = pCrsr;
    do {

        // for a selection the cursor has to have a mark.
        // for safety reasons assure that point and mark are in text nodes
        if ( pCrsr->HasMark() &&
             pCrsr->GetPoint()->nNode.GetNode().IsTxtNode() &&
             pCrsr->GetMark()->nNode.GetNode().IsTxtNode() )
        {
            SwPosition* pStartPos = pCrsr->Start();
            SwPosition* pEndPos = pCrsr->End();
            // loop on all text nodes inside the selection
            SwNodeIndex aIdx( pStartPos->nNode );
            for ( ; aIdx.GetIndex() <= pEndPos->nNode.GetIndex(); ++aIdx )
            {
                SwTxtNode* pTxtNode( aIdx.GetNode().GetTxtNode() );
                if ( pTxtNode )
                {
                    // loop on all text frames registered at the text node.
                    SwIterator<SwTxtFrm,SwTxtNode> aIter( *pTxtNode );
                    for( SwTxtFrm* pTxtFrm = aIter.First(); pTxtFrm; pTxtFrm = aIter.Next() )
                        {
                            uno::WeakReference < XAccessible > xWeakAcc;
                            SwAccessibleContextMap_Impl::iterator aMapIter =
                                                    mpFrmMap->find( pTxtFrm );
                            if( aMapIter != mpFrmMap->end() )
                            {
                                xWeakAcc = (*aMapIter).second;
                                SwAccessibleParaSelection aDataEntry(
                                    pTxtNode == &(pStartPos->nNode.GetNode())
                                                ? pStartPos->nContent.GetIndex()
                                                : 0,
                                    pTxtNode == &(pEndPos->nNode.GetNode())
                                                ? pEndPos->nContent.GetIndex()
                                                : STRING_LEN );
                                SwAccessibleSelectedParas_Impl::value_type
                                                aEntry( xWeakAcc, aDataEntry );
                                if ( !pRetSelectedParas )
                                {
                                    pRetSelectedParas =
                                            new SwAccessibleSelectedParas_Impl;
                                }
                                pRetSelectedParas->insert( aEntry );
                            }
                        }
                    }
                }
            }

        // prepare next turn: get next cursor in ring
        pCrsr = static_cast<SwPaM*>( pCrsr->GetNext() );
    } while ( pCrsr != pRingStart );

    return pRetSelectedParas;
}

void SwAccessibleMap::InvalidateTextSelectionOfAllParas()
{
    osl::MutexGuard aGuard( maMutex );

    // keep previously known selected paragraphs
    SwAccessibleSelectedParas_Impl* pPrevSelectedParas( mpSelectedParas );

    // determine currently selected paragraphs
    mpSelectedParas = _BuildSelectedParas();

    // compare currently selected paragraphs with the previously selected
    // paragraphs and submit corresponding TEXT_SELECTION_CHANGED events.
    // first, search for new and changed selections.
    // on the run remove selections from previously known ones, if they are
    // also in the current ones.
    if ( mpSelectedParas )
    {
        SwAccessibleSelectedParas_Impl::iterator aIter = mpSelectedParas->begin();
        for ( ; aIter != mpSelectedParas->end(); ++aIter )
        {
            bool bSubmitEvent( false );
            if ( !pPrevSelectedParas )
            {
                // new selection
                bSubmitEvent = true;
            }
            else
            {
                SwAccessibleSelectedParas_Impl::iterator aPrevSelected =
                                        pPrevSelectedParas->find( (*aIter).first );
                if ( aPrevSelected != pPrevSelectedParas->end() )
                {
                    // check, if selection has changed
                    if ( (*aIter).second.nStartOfSelection !=
                                    (*aPrevSelected).second.nStartOfSelection ||
                         (*aIter).second.nEndOfSelection !=
                                    (*aPrevSelected).second.nEndOfSelection )
                    {
                        // changed selection
                        bSubmitEvent = true;
                    }
                    pPrevSelectedParas->erase( aPrevSelected );
                }
                else
                {
                    // new selection
                    bSubmitEvent = true;
                }
            }

            if ( bSubmitEvent )
            {
                uno::Reference < XAccessible > xAcc( (*aIter).first );
                if ( xAcc.is() )
                {
                    ::rtl::Reference < SwAccessibleContext > xAccImpl(
                                static_cast<SwAccessibleContext*>( xAcc.get() ) );
                    if ( xAccImpl.is() && xAccImpl->GetFrm() )
                    {
                        const SwTxtFrm* pTxtFrm(
                            dynamic_cast<const SwTxtFrm*>(xAccImpl->GetFrm()) );
                        OSL_ENSURE( pTxtFrm,
                                "<SwAccessibleMap::_SubmitTextSelectionChangedEvents()> - unexcepted type of frame" );
                        if ( pTxtFrm )
                        {
                            InvalidateParaTextSelection( *pTxtFrm );
                        }
                    }
                }
            }
        }
    }

    // second, handle previous selections - after the first step the data
    // structure of the previously known only contains the 'old' selections
    if ( pPrevSelectedParas )
    {
        SwAccessibleSelectedParas_Impl::iterator aIter = pPrevSelectedParas->begin();
        for ( ; aIter != pPrevSelectedParas->end(); ++aIter )
        {
            uno::Reference < XAccessible > xAcc( (*aIter).first );
            if ( xAcc.is() )
            {
                ::rtl::Reference < SwAccessibleContext > xAccImpl(
                            static_cast<SwAccessibleContext*>( xAcc.get() ) );
                if ( xAccImpl.is() && xAccImpl->GetFrm() )
                {
                    const SwTxtFrm* pTxtFrm(
                            dynamic_cast<const SwTxtFrm*>(xAccImpl->GetFrm()) );
                    OSL_ENSURE( pTxtFrm,
                            "<SwAccessibleMap::_SubmitTextSelectionChangedEvents()> - unexcepted type of frame" );
                    if ( pTxtFrm )
                    {
                        InvalidateParaTextSelection( *pTxtFrm );
                    }
                }
            }
        }

        delete pPrevSelectedParas;
    }
}

const SwRect& SwAccessibleMap::GetVisArea() const
{
    OSL_ENSURE( !GetShell()->IsPreView() || (mpPreview != NULL),
                "preview without preview data?" );

    return GetShell()->IsPreView()
           ? mpPreview->GetVisArea()
           : GetShell()->VisArea();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
