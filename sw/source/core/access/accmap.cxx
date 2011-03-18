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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <vos/ref.hxx>
#include <cppuhelper/weakref.hxx>
#include <vcl/window.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unomod.hxx>
#include <tools/debug.hxx>

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

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using namespace ::sw::access;

struct SwFrmFunc
{
    sal_Bool operator()( const SwFrm * p1,
                         const SwFrm * p2) const
    {
        return p1 < p2;
    }
};

typedef ::std::map < const SwFrm *, uno::WeakReference < XAccessible >, SwFrmFunc > _SwAccessibleContextMap_Impl;

class SwAccessibleContextMap_Impl: public _SwAccessibleContextMap_Impl
{
public:

#ifdef DBG_UTIL
    sal_Bool mbLocked;
#endif

    SwAccessibleContextMap_Impl()
#ifdef DBG_UTIL
        : mbLocked( sal_False )
#endif
    {}

};

//------------------------------------------------------------------------------
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
    // OD 01.07.2003 #110554# - correct condition in order not to broadcast
    // notifications for writer fly frames.
    // OD 01.07.2003 #110554# - do not broadcast notifications for plane
    // <SdrObject>objects
    const SdrHint *pSdrHint = PTR_CAST( SdrHint, &rHint );
    if ( !pSdrHint ||
         ( pSdrHint->GetObject() &&
           ( pSdrHint->GetObject()->ISA(SwFlyDrawObj) ||
             pSdrHint->GetObject()->ISA(SwVirtFlyDrawObj) ||
             IS_TYPE(SdrObject,pSdrHint->GetObject()) ) ) )
    {
        return;
    }

    ASSERT( mpDrawModel, "draw model listener is disposed" );
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
            (void)r;
#if OSL_DEBUG_LEVEL > 1
            ByteString aError( "Runtime exception caught while notifying shape.:\n" );
            aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aError.GetBuffer() );
#endif
        }
    }
}

void SwDrawModellListener_Impl::Dispose()
{
    mpDrawModel = 0;
}

//------------------------------------------------------------------------------
struct SwShapeFunc
{
    sal_Bool operator()( const SdrObject * p1,
                         const SdrObject * p2) const
    {
        return p1 < p2;
    }
};
typedef ::std::map < const SdrObject *, uno::WeakReference < XAccessible >, SwShapeFunc > _SwAccessibleShapeMap_Impl;
typedef ::std::pair < const SdrObject *, ::vos::ORef < ::accessibility::AccessibleShape > > SwAccessibleObjShape_Impl;

class SwAccessibleShapeMap_Impl: public _SwAccessibleShapeMap_Impl

{
    ::accessibility::AccessibleShapeTreeInfo maInfo;

public:

#ifdef DBG_UTIL
    sal_Bool mbLocked;
#endif
    SwAccessibleShapeMap_Impl( SwAccessibleMap *pMap )
#ifdef DBG_UTIL
        : mbLocked( sal_False )
#endif
    {
        maInfo.SetSdrView( pMap->GetShell()->GetDrawView() );
        maInfo.SetWindow( pMap->GetShell()->GetWin() );
        maInfo.SetViewForwarder( pMap );
        // --> OD 2005-08-08 #i52858# - method name changed
        uno::Reference < document::XEventBroadcaster > xModelBroadcaster =
            new SwDrawModellListener_Impl(
                    pMap->GetShell()->getIDocumentDrawModelAccess()->GetOrCreateDrawModel() );
        // <--
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
        ASSERT( pSelShape == pShape, "copying shapes went wrong!" );
    }

    if( pSelStart )
        *pSelStart = pSelShape;

    return pShapes;
}

//------------------------------------------------------------------------------
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
    SwRect      maOldBox;               // the old bounds for CHILD_POS_CHANGED
                                        // and POS_CHANGED
    uno::WeakReference < XAccessible > mxAcc;   // The object that fires the event
    SwAccessibleChild   maFrmOrObj;             // the child for CHILD_POS_CHANGED and
                                        // the same as xAcc for any other
                                        // event type
    EventType   meType;                 // The event type
    // --> OD 2005-12-12 #i27301# - use new type definition for <mnStates>
    tAccessibleStates mnStates;         // check states or update caret pos
    // <--

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
        ASSERT( SwAccessibleEvent_Impl::DISPOSE == meType,
                "wrong event constructor, DISPOSE only" );
    }

    SwAccessibleEvent_Impl( EventType eT )
        : meType( eT ),
          mnStates( 0 )
    {
        ASSERT( SwAccessibleEvent_Impl::SHAPE_SELECTION == meType,
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
        ASSERT( SwAccessibleEvent_Impl::CHILD_POS_CHANGED == meType ||
                SwAccessibleEvent_Impl::POS_CHANGED == meType,
                "wrong event constructor, (CHILD_)POS_CHANGED only" );
    }

    // --> OD 2005-12-12 #i27301# - use new type definition for parameter <_nStates>
    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrmOrObj,
                            const tAccessibleStates _nStates )
        : mxAcc( pA ),
          maFrmOrObj( rFrmOrObj ),
          meType( eT ),
          mnStates( _nStates )
    {
        ASSERT( SwAccessibleEvent_Impl::CARET_OR_STATES == meType,
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

    inline ::vos::ORef < SwAccessibleContext > GetContext() const
    {
        uno::Reference < XAccessible > xTmp( mxAcc );
        ::vos::ORef < SwAccessibleContext > xAccImpl(
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
    // --> OD 2005-12-12 #i27301# - use new type definition for parameter <_nStates>
    inline void SetStates( tAccessibleStates _nStates )
    {
        mnStates |= _nStates;
    }
    // <--

    inline sal_Bool IsUpdateCursorPos() const
    {
        return (mnStates & ACC_STATE_CARET) != 0;
    }
    inline sal_Bool IsInvalidateStates() const
    {
        return (mnStates & ACC_STATE_MASK) != 0;
    }
    inline sal_Bool IsInvalidateRelation() const
    {
        return (mnStates & ACC_STATE_RELATION_MASK) != 0;
    }
    // --> OD 2005-12-12 #i27301# - new event TEXT_SELECTION_CHANGED
    inline sal_Bool IsInvalidateTextSelection() const
    {
        return ( mnStates & ACC_STATE_TEXT_SELECTION_CHANGED ) != 0;
    }
    // <--
    // --> OD 2009-01-07 #i88069# - new event TEXT_ATTRIBUTE_CHANGED
    inline sal_Bool IsInvalidateTextAttrs() const
    {
        return ( mnStates & ACC_STATE_TEXT_ATTRIBUTE_CHANGED ) != 0;
    }
    // <--
    // --> OD 2005-12-12 #i27301# - use new type definition <tAccessibleStates>
    // for return value
    inline tAccessibleStates GetStates() const
    {
        return mnStates & ACC_STATE_MASK;
    }
    // <--
    // --> OD 2005-12-12 #i27301# - use new type definition <tAccessibleStates>
    // for return value
    inline tAccessibleStates GetAllStates() const
    {
        return mnStates;
    }
    // <--
};

//------------------------------------------------------------------------------
typedef ::std::list < SwAccessibleEvent_Impl > _SwAccessibleEventList_Impl;

class SwAccessibleEventList_Impl: public _SwAccessibleEventList_Impl
{
    sal_Bool mbFiring;

public:

    SwAccessibleEventList_Impl()
        : mbFiring( sal_False )
    {}

    inline void SetFiring()
    {
        mbFiring = sal_True;
    }
    inline sal_Bool IsFiring() const
    {
        return mbFiring;
    }
};

//------------------------------------------------------------------------------
// The shape list is filled if an accessible shape is destroyed. It
// simply keeps a reference to the accessible shape's XShape. These
// references are destroyed within the EndAction when firing events,
// There are twp reason for this. First of all, a new accessible shape
// for the XShape might be created soon. It's then cheaper if the XShape
// still exists. The other reason are situations where an accessible shape
// is destroyed within an SwFrmFmt::Modify. In this case, destryoing
// the XShape at the same time (indirectly by destroying the accessible
// shape) leads to an assert, because a client of the Modify is destroyed
// within a Modify call.

typedef ::std::list < uno::Reference < drawing::XShape > > _SwShapeList_Impl;

class SwShapeList_Impl: public _SwShapeList_Impl
{
public:

    SwShapeList_Impl() {}
};


//------------------------------------------------------------------------------
struct SwAccessibleChildFunc
{
    sal_Bool operator()( const SwAccessibleChild& r1,
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

//------------------------------------------------------------------------------
// --> OD 2005-12-13 #i27301# - map containing the accessible paragraph, which
// have a selection. Needed to keep this information to submit corresponding
// TEXT_SELECTION_CHANGED events.
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
    sal_Bool operator()( const uno::WeakReference<XAccessible>& _rXAccWeakRef1,
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
// <--

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

        OD 17.01.2003 #103492#

        @author OD

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

    // OD 14.01.2003 #103492# - complete re-factoring of method due to new
    // page/print preview functionality.
    void Update( const SwAccessibleMap& rAccMap,
                 const std::vector<PrevwPage*>& _rPrevwPages,
                 const Fraction&  _rScale,
                 const SwPageFrm* _pSelectedPageFrm,
                 const Size&      _rPrevwWinSize );

    // OD 14.01.2003 #103492# - complete re-factoring of method due to new
    // page/print preview functionality.
    void InvalidateSelection( const SwPageFrm* _pSelectedPageFrm );

    const SwRect& GetVisArea() const;

    MapMode GetMapModeForPreview( ) const;

    /** Adjust the MapMode so that the preview page appears at the
     * proper position. rPoint identifies the page for which the
     * MapMode should be adjusted. If bFromPreview is true, rPoint is
     * a preview coordinate; else it's a document coordinate. */
    // OD 17.01.2003 #103492# - delete unused 3rd parameter.
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

// OD 13.01.2003 #103492# - complete re-factoring of method due to new page/print
// preview functionality.
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

// OD 16.01.2003 #103492# - complete re-factoring of method due to new page/print
// preview functionality.
void SwAccPreviewData::InvalidateSelection( const SwPageFrm* _pSelectedPageFrm )
{
    mpSelPage = _pSelectedPageFrm;
    ASSERT( mpSelPage, "selected page not found" );
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

/** adjust logic page retangle to its visible part

    OD 17.01.2003 #103492#

    @author OD
*/
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

//------------------------------------------------------------------------------
static sal_Bool AreInSameTable( const uno::Reference< XAccessible >& rAcc,
                                  const SwFrm *pFrm )
{
    sal_Bool bRet = sal_False;

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
    ::vos::ORef < SwAccessibleContext > xAccImpl( rEvent.GetContext() );
    if( SwAccessibleEvent_Impl::SHAPE_SELECTION == rEvent.GetType() )
    {
        DoInvalidateShapeSelection();
    }
    else if( xAccImpl.isValid() && xAccImpl->GetFrm() )
    {
        // --> OD 2009-01-07 #i88069#
        if ( rEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE &&
             rEvent.IsInvalidateTextAttrs() )
        {
            xAccImpl->InvalidateAttr();
        }
        // <--
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
            ASSERT( xAccImpl.isValid(),
                    "dispose event has been stored" );
            break;
        // --> OD 2009-01-06 #i88069#
        case SwAccessibleEvent_Impl::INVALID_ATTR:
            // nothing to do here - handled above
            break;
        // <--
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
                // --> OD 2005-12-01 #i27138#
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
                // <--
            }
            // --> OD 2005-12-12 #i27301# - submit event TEXT_SELECTION_CHANGED
            if ( rEvent.IsInvalidateTextSelection() )
            {
                xAccImpl->InvalidateTextSelection();
            }
            // <--
        }
    }
}

void SwAccessibleMap::AppendEvent( const SwAccessibleEvent_Impl& rEvent )
{
    vos::OGuard aGuard( maEventMutex );

    if( !mpEvents )
        mpEvents = new SwAccessibleEventList_Impl;
    if( !mpEventMap )
        mpEventMap = new SwAccessibleEventMap_Impl;

    if( mpEvents->IsFiring() )
    {
        // While events are fired new ones are generated. They have to be fired
        // now. This does not work for DISPOSE events!
        ASSERT( rEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
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
            ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::DISPOSE,
                    "dispose events should not be stored" );
            sal_Bool bAppendEvent = sal_True;
            switch( rEvent.GetType() )
            {
            case SwAccessibleEvent_Impl::CARET_OR_STATES:
                // A CARET_OR_STATES event is added to any other
                // event only. It is broadcasted after any other event, so the
                // event should be put to the back.
                ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                aEvent.SetStates( rEvent.GetAllStates() );
                break;
            case SwAccessibleEvent_Impl::INVALID_CONTENT:
                // An INVALID_CONTENT event overwrites a CARET_OR_STATES
                // event (but keeps its flags) and it is contained in a
                // POS_CHANGED event.
                // Therefor, the event's type has to be adapted and the event
                // has to be put at the end.
                ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                if( aEvent.GetType() == SwAccessibleEvent_Impl::CARET_OR_STATES )
                    aEvent.SetType( SwAccessibleEvent_Impl::INVALID_CONTENT );
                break;
            case SwAccessibleEvent_Impl::POS_CHANGED:
                // A pos changed event overwrites CARET_STATES (keeping its
                // flags) as well as INVALID_CONTENT. The old box position
                // has to be stored however if the old event is not a
                // POS_CHANGED itself.
                ASSERT( aEvent.GetType() != SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
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
                ASSERT( aEvent.GetType() == SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        "invalid event combination" );
                break;
            case SwAccessibleEvent_Impl::SHAPE_SELECTION:
                ASSERT( aEvent.GetType() == SwAccessibleEvent_Impl::SHAPE_SELECTION,
                        "invalid event combination" );
                break;
            case SwAccessibleEvent_Impl::DISPOSE:
                // DISPOSE events overwrite all others. They are not stored
                // but executed immediatly to avoid broadcasting of
                // defunctional objects. So what needs to be done here is to
                // remove all events for the frame in question.
                bAppendEvent = sal_False;
                break;
            // --> OD 2009-01-06 #i88069#
            case SwAccessibleEvent_Impl::INVALID_ATTR:
                ASSERT( aEvent.GetType() == SwAccessibleEvent_Impl::INVALID_ATTR,
                        "invalid event combination" );
                break;
            // <--
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
    ASSERT( pAccImpl, "no caret context" );
    ASSERT( pAccImpl->GetFrm(), "caret context is disposed" );
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
        vos::OGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        ::std::list< const SwFrm * > aParents;
        Window *pWin = GetShell()->GetWin();
        sal_Bool bFocused = pWin && pWin->HasFocus();
        SwAccessibleObjShape_Impl *pShape = pShapes;
        while( nShapes )
        {
            if( pShape->second.isValid() )
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
                ::vos::ORef< SwAccessibleContext > xParentAccImpl;
                {
                    vos::OGuard aGuard( maMutex );
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
                if( xParentAccImpl.isValid() )
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
        vos::OGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        Window *pWin = GetShell()->GetWin();
        sal_Bool bFocused = pWin && pWin->HasFocus();
        SwAccessibleObjShape_Impl  *pShape = pShapes;
        while( nShapes )
        {
            if( pShape->second.isValid() )
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
    // --> OD 2005-12-13 #i27301#
    mpSelectedParas( 0 ),
    // <--
    mpVSh( pSh ),
        mpPreview( 0 ),
    mnPara( 1 ),
    mnFootnote( 1 ),
    mnEndnote( 1 ),
    mbShapeSelected( sal_False )
{
    pSh->GetLayout()->AddAccessibleShell();
}

SwAccessibleMap::~SwAccessibleMap()
{
    uno::Reference < XAccessible > xAcc;
    {
        vos::OGuard aGuard( maMutex );
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
        vos::OGuard aGuard( maMutex );
#ifdef DBG_UTIL
        ASSERT( !mpFrmMap || mpFrmMap->empty(),
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
        ASSERT( !mpShapeMap || mpShapeMap->empty(),
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
        // --> OD 2005-12-13 #i27301#
        delete mpSelectedParas;
        mpSelectedParas = 0;
        // <--
    }

    delete mpPreview;
    mpPreview = NULL;

    {
        vos::OGuard aGuard( maEventMutex );
#ifdef DBG_UTIL
        ASSERT( !(mpEvents || mpEventMap), "pending events" );
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
    sal_Bool bPagePreview )
{
    uno::Reference < XAccessible > xAcc;
    sal_Bool bSetVisArea = sal_False;

    {
        vos::OGuard aGuard( maMutex );

        if( !mpFrmMap )
        {
            mpFrmMap = new SwAccessibleContextMap_Impl;
#ifdef DBG_UTIL
            mpFrmMap->mbLocked = sal_False;
#endif
        }

#ifdef DBG_UTIL
        ASSERT( !mpFrmMap->mbLocked, "Map is locked" );
        mpFrmMap->mbLocked = sal_True;
#endif

        const SwRootFrm *pRootFrm = GetShell()->GetLayout();
        SwAccessibleContextMap_Impl::iterator aIter = mpFrmMap->find( pRootFrm );
        if( aIter != mpFrmMap->end() )
            xAcc = (*aIter).second;
        if( xAcc.is() )
        {
            bSetVisArea = sal_True; // Set VisArea when map mutex is not
                                    // locked
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

#ifdef DBG_UTIL
        mpFrmMap->mbLocked = sal_False;
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
    return _GetDocumentView( sal_False );
}

// OD 14.01.2003 #103492# - complete re-factoring of method due to new page/print
// preview functionality.
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

    uno::Reference<XAccessible> xAcc = _GetDocumentView( sal_True );
    return xAcc;
}

uno::Reference< XAccessible> SwAccessibleMap::GetContext( const SwFrm *pFrm,
                                                     sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc;
    uno::Reference < XAccessible > xOldCursorAcc;
    sal_Bool bOldShapeSelected = sal_False;

    {
        vos::OGuard aGuard( maMutex );

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
                    DBG_ASSERT( GetShell()->IsPreView(),
                                "accessible page frames only in PagePreview" );
                    pAcc = new SwAccessiblePage( this, pFrm );
                    break;
                }
                xAcc = pAcc;

                ASSERT( xAcc.is(), "unknown frame type" );
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
                        // have to send a focus event and a caret event for
                        // the old context then. We have to to that know,
                        // because after we have left this method, anyone might
                        // call getStates for the new context and will get a
                        // focused state then. Sending the focus changes event
                        // after that seems to be strange. However, we cannot
                        // send a focus event fo the new context now, because
                        // noone except us knows it. In any case, we remeber
                        // the new context as the one that has the focus
                        // currently.

                        xOldCursorAcc = mxCursorContext;
                        mxCursorContext = xAcc;

                        bOldShapeSelected = mbShapeSelected;
                        mbShapeSelected = sal_False;
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

::vos::ORef < SwAccessibleContext > SwAccessibleMap::GetContextImpl(
            const SwFrm *pFrm,
            sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pFrm, bCreate ) );

    ::vos::ORef < SwAccessibleContext > xAccImpl(
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
        vos::OGuard aGuard( maMutex );

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

                ASSERT( xAcc.is(), "unknown shape type" );
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

::vos::ORef < ::accessibility::AccessibleShape > SwAccessibleMap::GetContextImpl(
            const SdrObject *pObj,
            SwAccessibleContext *pParentImpl,
            sal_Bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pObj, pParentImpl, bCreate ) );

    ::vos::ORef < ::accessibility::AccessibleShape > xAccImpl(
         static_cast< ::accessibility::AccessibleShape* >( xAcc.get() ) );

    return xAccImpl;
}


void SwAccessibleMap::RemoveContext( const SwFrm *pFrm )
{
    vos::OGuard aGuard( maMutex );

    if( mpFrmMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            mpFrmMap->find( pFrm );
        if( aIter != mpFrmMap->end() )
        {
            mpFrmMap->erase( aIter );

            // Remove reference to old caret object. Though mxCursorContext
            // is a weak reference and cleared automatically, clearing it
            // directly makes sure to not keep a defunctional object.
            uno::Reference < XAccessible > xOldAcc( mxCursorContext );
            if( xOldAcc.is() )
            {
                SwAccessibleContext *pOldAccImpl =
                    static_cast< SwAccessibleContext *>( xOldAcc.get() );
                ASSERT( pOldAccImpl->GetFrm(), "old caret context is disposed" );
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
    vos::OGuard aGuard( maMutex );

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
    ASSERT( !aFrmOrObj.GetSwFrm() || aFrmOrObj.GetSwFrm()->IsAccessibleFrm(),
            "non accessible frame should be disposed" );

    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        ::vos::ORef< SwAccessibleContext > xAccImpl;
        ::vos::ORef< SwAccessibleContext > xParentAccImpl;
        ::vos::ORef< ::accessibility::AccessibleShape > xShapeAccImpl;
        // get accessible context for frame
        {
            vos::OGuard aGuard( maMutex );

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
            if( !xAccImpl.isValid() && mpFrmMap )
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
            if( !xParentAccImpl.isValid() && !aFrmOrObj.GetSwFrm() &&
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
                (xParentAccImpl.isValid() || xShapeAccImpl.isValid()) )
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
            vos::OGuard aGuard( maEventMutex );
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
        if( xAccImpl.isValid() )
        {
            xAccImpl->Dispose( bRecursive );
        }
        else if( xParentAccImpl.isValid() )
        {
            // If the frame is a cell frame, the table must be notified.
            // If we are in an action, a table model change event will
            // be broadcasted at the end of the action to give the table
            // a chance to generate a single table change event.

            xParentAccImpl->DisposeChild( aFrmOrObj, bRecursive );
        }
        else if( xShapeAccImpl.isValid() )
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
        ::vos::ORef< SwAccessibleContext > xAccImpl;
        ::vos::ORef< SwAccessibleContext > xParentAccImpl;
        {
            vos::OGuard aGuard( maMutex );

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
                if( !xAccImpl.isValid() )
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

        if( xAccImpl.isValid() )
        {
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::POS_CHANGED, xAccImpl.getBodyPtr(),
                    aFrmOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                xAccImpl->InvalidatePosOrSize( rOldBox );
            }
        }
        else if( xParentAccImpl.isValid() )
        {
            if( GetShell()->ActionPend() )
            {
                SwAccessibleEvent_Impl aEvent(
                    SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                    xParentAccImpl.getBodyPtr(), aFrmOrObj, rOldBox );
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
            vos::OGuard aGuard( maMutex );

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

// --> OD 2009-01-06 #i88069#
void SwAccessibleMap::InvalidateAttr( const SwTxtFrm& rTxtFrm )
{
    SwAccessibleChild aFrmOrObj( &rTxtFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            vos::OGuard aGuard( maMutex );

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
// <--

void SwAccessibleMap::InvalidateCursorPosition( const SwFrm *pFrm )
{
    SwAccessibleChild aFrmOrObj( pFrm );
    sal_Bool bShapeSelected = sal_False;
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
            sal_uInt16 nObjCount;
            const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
            const SwFrm *pFlyFrm = pFESh->GetCurrFlyFrm();
            if( pFlyFrm )
            {
                ASSERT( !pFrm || pFrm->FindFlyFrm() == pFlyFrm,
                        "cursor is not contained in fly frame" );
                aFrmOrObj = pFlyFrm;
            }
            else if( (nObjCount = pFESh->IsObjSelected()) > 0 )
            {
                bShapeSelected = sal_True;
                aFrmOrObj = static_cast<const SwFrm *>( 0 );
            }
        }
    }

    ASSERT( bShapeSelected || aFrmOrObj.IsAccessible(GetShell()->IsPreView()),
            "frame is not accessible" );

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    sal_Bool bOldShapeSelected = sal_False;

    {
        vos::OGuard aGuard( maMutex );

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
    sal_Bool bShapeSelected;
    {
        vos::OGuard aGuard( maMutex );

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
        const ::vos::ORef < SwAccessibleContext >& rCursorContext )
{
    vos::OGuard aGuard( maMutex );
    uno::Reference < XAccessible > xAcc( rCursorContext.getBodyPtr() );
    mxCursorContext = xAcc;
}

// --> OD 2005-12-12 #i27301# - use new type definition for <_nStates>
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
// <--

void SwAccessibleMap::_InvalidateRelationSet( const SwFrm* pFrm,
                                              sal_Bool bFrom )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrmOrObj( pFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            vos::OGuard aGuard( maMutex );

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
    _InvalidateRelationSet( pMaster, sal_False );
    _InvalidateRelationSet( pFollow, sal_True );
}

/** invalidation CONTENT_FLOW_FROM/_TO relation of a paragraph

    OD 2005-12-01 #i27138#

    @author OD
*/
void SwAccessibleMap::InvalidateParaFlowRelation( const SwTxtFrm& _rTxtFrm,
                                                  const bool _bFrom )
{
    _InvalidateRelationSet( &_rTxtFrm, _bFrom );
}

/** invalidation of text selection of a paragraph

    OD 2005-12-12 #i27301#

    @author OD
*/
void SwAccessibleMap::InvalidateParaTextSelection( const SwTxtFrm& _rTxtFrm )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrmOrObj( &_rTxtFrm );
    if( aFrmOrObj.IsAccessible( GetShell()->IsPreView() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            vos::OGuard aGuard( maMutex );

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
            vos::OGuard aGuard( maMutex );

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


// OD 15.01.2003 #103492# - complete re-factoring of method due to new page/print
// preview functionality.
void SwAccessibleMap::UpdatePreview( const std::vector<PrevwPage*>& _rPrevwPages,
                                     const Fraction&  _rScale,
                                     const SwPageFrm* _pSelectedPageFrm,
                                     const Size&      _rPrevwWinSize )
{
    DBG_ASSERT( GetShell()->IsPreView(), "no preview?" );
    DBG_ASSERT( mpPreview != NULL, "no preview data?" );

    // OD 15.01.2003 #103492# - adjustments for changed method signature
    mpPreview->Update( *this, _rPrevwPages, _rScale, _pSelectedPageFrm, _rPrevwWinSize );

    // propagate change of VisArea through the document's
    // accessibility tree; this will also send appropriate scroll
    // events
    SwAccessibleContext* pDoc =
        GetContextImpl( GetShell()->GetLayout() ).getBodyPtr();
    static_cast<SwAccessibleDocumentBase*>( pDoc )->SetVisArea();

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    {
        vos::OGuard aGuard( maMutex );

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
    DBG_ASSERT( GetShell()->IsPreView(), "no preview?" );
    DBG_ASSERT( mpPreview != NULL, "no preview data?" );

    // OD 16.01.2003 #103492# - changed metthod call due to method signature change.
    mpPreview->InvalidateSelection( GetShell()->GetLayout()->GetPageByPageNum( nSelPage ) );

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    {
        vos::OGuard aGuard( maMutex );

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


sal_Bool SwAccessibleMap::IsPageSelected( const SwPageFrm *pPageFrm ) const
{
    return mpPreview && mpPreview->GetSelPage() == pPageFrm;
}


void SwAccessibleMap::FireEvents()
{
    {
        vos::OGuard aGuard( maEventMutex );
        if( mpEvents )
        {
            mpEvents->SetFiring();
            SwAccessibleEventList_Impl::iterator aIter = mpEvents->begin();
            while( aIter != mpEvents->end() )
            {
                FireEvent( *aIter );
                ++aIter;
            }

            delete mpEventMap;
            mpEventMap = 0;

            delete mpEvents;
            mpEvents = 0;
        }
    }
    {
        vos::OGuard aGuard( maMutex );
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

// Convert a MM100 value realtive to the document root into a pixel value
// realtive to the screen!
Point SwAccessibleMap::LogicToPixel( const Point& rPoint ) const
{
    MapMode aSrc( MAP_100TH_MM );
    MapMode aDest( MAP_TWIP );

    Point aPoint = rPoint;

    aPoint = OutputDevice::LogicToLogic( aPoint, aSrc, aDest );
    Window *pWin = GetShell()->GetWin();
    if( pWin )
    {
        // OD 16.01.2003 #103492# - get mapping mode for LogicToPixel conversion
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
        // OD 16.01.2003 #103492# - get mapping mode for LogicToPixel conversion
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
        // OD 16.01.2003 #103492# - get mapping mode for PixelToLogic conversion
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
        // OD 16.01.2003 #103492# - get mapping mode for PixelToLogic conversion
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
        vos::OGuard aGuard( maMutex );
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

    uno::Reference < drawing::XShape > xShape( _rxShape ); //keep reference to shape, because
                                             // we might be the only one that
                                             // hold it.
    // Also get keep parent.
    uno::Reference < XAccessible > xParent( pCurrentChild->getAccessibleParent() );
    pCurrentChild = 0;  // well be realease by dispose
    Dispose( 0, pObj, 0 );

    {
        vos::OGuard aGuard( maMutex );

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
        // OD 15.01.2003 #103492# - replace <PreviewAdjust(..)> by <GetMapMode(..)>
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
    rRect.nLeft = lcl_CorrectCoarseValue(rRect.nLeft, rSource.nLeft,
                                         rInGrid.nLeft, false);
    rRect.nTop = lcl_CorrectCoarseValue(rRect.nTop, rSource.nTop,
                                        rInGrid.nTop, false);
    rRect.nRight = lcl_CorrectCoarseValue(rRect.nRight, rSource.nRight,
                                          rInGrid.nRight, true);
    rRect.nBottom = lcl_CorrectCoarseValue(rRect.nBottom, rSource.nBottom,
                                           rInGrid.nBottom, true);
}

Rectangle SwAccessibleMap::CoreToPixel( const Rectangle& rRect ) const
{
    Rectangle aRect;
    if( GetShell()->GetWin() )
    {
        // OD 15.01.2003 #103492# - replace <PreviewAdjust(..)> by <GetMapMode(..)>
        MapMode aMapMode;
        GetMapMode( rRect.TopLeft(), aMapMode );
        aRect = GetShell()->GetWin()->LogicToPixel( rRect, aMapMode );

        Rectangle aTmpRect = GetShell()->GetWin()->PixelToLogic( aRect, aMapMode );
        lcl_CorrectRectangle(aRect, rRect, aTmpRect);
    }

    return aRect;
}

/** get mapping mode for LogicToPixel and PixelToLogic conversions

    OD 15.01.2003 #103492#
    Replacement method <PreviewAdjust(..)> by new method <GetMapMode>.
    Method returns mapping mode of current output device and adjusts it,
    if the shell is in page/print preview.
    Necessary, because <PreviewAdjust(..)> changes mapping mode at current
    output device for mapping logic document positions to page preview window
    positions and vice versa and doesn't take care to recover its changes.

    @author OD
*/
void SwAccessibleMap::GetMapMode( const Point& _rPoint,
                                  MapMode&     _orMapMode ) const
{
    MapMode aMapMode = GetShell()->GetWin()->GetMapMode();
    if( GetShell()->IsPreView() )
    {
        DBG_ASSERT( mpPreview != NULL, "need preview data" );

        mpPreview->AdjustMapMode( aMapMode, _rPoint );
    }
    _orMapMode = aMapMode;
}

/** get size of a dedicated preview page

    OD 15.01.2003 #103492#

    @author OD
*/
Size SwAccessibleMap::GetPreViewPageSize( sal_uInt16 _nPrevwPageNum ) const
{
    DBG_ASSERT( mpVSh->IsPreView(), "no page preview accessible." );
    DBG_ASSERT( mpVSh->IsPreView() && ( mpPreview != NULL ),
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

    OD 2005-12-13 #i27301#
    Important note: method has to used inside a mutual exclusive section

    @author OD
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
        // for savety reasons assure that point and mark are in text nodes
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

/** invalidation of text selection of all paragraphs

    OD 2005-12-13 #i27301#

    @author OD
*/
void SwAccessibleMap::InvalidateTextSelectionOfAllParas()
{
    vos::OGuard aGuard( maMutex );

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
                    ::vos::ORef < SwAccessibleContext > xAccImpl(
                                static_cast<SwAccessibleContext*>( xAcc.get() ) );
                    if ( xAccImpl.isValid() && xAccImpl->GetFrm() )
                    {
                        const SwTxtFrm* pTxtFrm(
                            dynamic_cast<const SwTxtFrm*>(xAccImpl->GetFrm()) );
                        ASSERT( pTxtFrm,
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
                ::vos::ORef < SwAccessibleContext > xAccImpl(
                            static_cast<SwAccessibleContext*>( xAcc.get() ) );
                if ( xAccImpl.isValid() && xAccImpl->GetFrm() )
                {
                    const SwTxtFrm* pTxtFrm(
                            dynamic_cast<const SwTxtFrm*>(xAccImpl->GetFrm()) );
                    ASSERT( pTxtFrm,
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
    DBG_ASSERT( !GetShell()->IsPreView() || (mpPreview != NULL),
                "preview without preview data?" );

    return GetShell()->IsPreView()
           ? mpPreview->GetVisArea()
           : GetShell()->VisArea();
}

