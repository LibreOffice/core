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
#include <vector>
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
#include <svx/SvxShapeTypes.hxx>
#include <svx/svdpage.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <pagepreviewlayout.hxx>
#include <dcontact.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdmark.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>
#include <prevwpage.hxx>
#include <calbck.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::sw::access;

struct SwFrameFunc
{
    bool operator()( const SwFrame * p1, const SwFrame * p2) const
    {
        return p1 < p2;
    }
};

class SwAccessibleContextMap_Impl
{
public:
    typedef const SwFrame *                                               key_type;
    typedef uno::WeakReference < XAccessible >                          mapped_type;
    typedef std::pair<const key_type,mapped_type>                       value_type;
    typedef SwFrameFunc                                                   key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator        iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_iterator  const_iterator;
private:
    std::map <key_type,mapped_type,key_compare> maMap;
public:

#if OSL_DEBUG_LEVEL > 0
    bool mbLocked;
#endif

    SwAccessibleContextMap_Impl()
#if OSL_DEBUG_LEVEL > 0
        : mbLocked( false )
#endif
    {}

    iterator begin() { return maMap.begin(); }
    iterator end() { return maMap.end(); }
    bool empty() const { return maMap.empty(); }
    void clear() { maMap.clear(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    std::pair<iterator,bool> insert(const value_type& value ) { return maMap.insert(value); }
    iterator erase(const_iterator pos) { return maMap.erase(pos); }
};

class SwDrawModellListener_Impl : public SfxListener,
    public ::cppu::WeakImplHelper< document::XEventBroadcaster >
{
    mutable ::osl::Mutex maListenerMutex;
    ::comphelper::OInterfaceContainerHelper2 maEventListeners;
    SdrModel *mpDrawModel;
protected:
    virtual ~SwDrawModellListener_Impl();

public:
    explicit SwDrawModellListener_Impl( SdrModel *pDrawModel );

    virtual void SAL_CALL addEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException, std::exception) override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
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
    Dispose();
}

void SAL_CALL SwDrawModellListener_Impl::addEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    maEventListeners.addInterface( xListener );
}

void SAL_CALL SwDrawModellListener_Impl::removeEventListener( const uno::Reference< document::XEventListener >& xListener ) throw (uno::RuntimeException, std::exception)
{
    maEventListeners.removeInterface( xListener );
}

void SwDrawModellListener_Impl::Notify( SfxBroadcaster& /*rBC*/,
        const SfxHint& rHint )
{
    // do not broadcast notifications for writer fly frames, because there
    // are no shapes that need to know about them.
    const SdrHint *pSdrHint = dynamic_cast<const SdrHint*>( &rHint );
    if ( !pSdrHint ||
         ( pSdrHint->GetObject() &&
           ( dynamic_cast< const SwFlyDrawObj* >(pSdrHint->GetObject()) !=  nullptr ||
              dynamic_cast< const SwVirtFlyDrawObj* >(pSdrHint->GetObject()) !=  nullptr ||
             typeid(SdrObject) == typeid(pSdrHint->GetObject()) ) ) )
    {
        return;
    }

    OSL_ENSURE( mpDrawModel, "draw model listener is disposed" );
    if( !mpDrawModel )
        return;

    document::EventObject aEvent;
    if( !SvxUnoDrawMSFactory::createEvent( mpDrawModel, pSdrHint, aEvent ) )
        return;

    ::comphelper::OInterfaceIteratorHelper2 aIter( maEventListeners );
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
            SAL_WARN("sw.a11y", "Runtime exception caught while notifying shape: " << r.Message);
        }
    }
}

void SwDrawModellListener_Impl::Dispose()
{
    if (mpDrawModel != nullptr) {
        EndListening( *mpDrawModel );
    }
    mpDrawModel = nullptr;
}

struct SwShapeFunc
{
    bool operator()( const SdrObject * p1, const SdrObject * p2) const
    {
        return p1 < p2;
    }
};
typedef ::std::pair < const SdrObject *, ::rtl::Reference < ::accessibility::AccessibleShape > > SwAccessibleObjShape_Impl;

class SwAccessibleShapeMap_Impl
{
public:

    typedef const SdrObject *                                           key_type;
    typedef uno::WeakReference<XAccessible>                             mapped_type;
    typedef std::pair<const key_type,mapped_type>                       value_type;
    typedef SwShapeFunc                                                 key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator        iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_iterator  const_iterator;

private:

    ::accessibility::AccessibleShapeTreeInfo    maInfo;
    std::map<key_type,mapped_type,SwShapeFunc>  maMap;

public:

    explicit SwAccessibleShapeMap_Impl( SwAccessibleMap *pMap )
        : maMap()
    {
        maInfo.SetSdrView( pMap->GetShell()->GetDrawView() );
        maInfo.SetWindow( pMap->GetShell()->GetWin() );
        maInfo.SetViewForwarder( pMap );
        uno::Reference < document::XEventBroadcaster > xModelBroadcaster =
            new SwDrawModellListener_Impl(
                    pMap->GetShell()->getIDocumentDrawModelAccess().GetOrCreateDrawModel() );
        maInfo.SetModelBroadcaster( xModelBroadcaster );
    }

    ~SwAccessibleShapeMap_Impl();

    const ::accessibility::AccessibleShapeTreeInfo& GetInfo() const { return maInfo; }

    SwAccessibleObjShape_Impl *Copy( size_t& rSize,
        const SwFEShell *pFESh = nullptr,
        SwAccessibleObjShape_Impl  **pSelShape = nullptr ) const;

#if OSL_DEBUG_LEVEL > 0
    iterator begin() { return maMap.begin(); }
#endif
    iterator end() { return maMap.end(); }
    const_iterator cbegin() const { return maMap.cbegin(); }
    const_iterator cend() const { return maMap.cend(); }
    bool empty() const { return maMap.empty(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    std::pair<iterator,bool> insert(const value_type& value ) { return maMap.insert(value); }
    iterator erase(const_iterator pos) { return maMap.erase(pos); }
};

SwAccessibleShapeMap_Impl::~SwAccessibleShapeMap_Impl()
{
    uno::Reference < document::XEventBroadcaster > xBrd( maInfo.GetModelBroadcaster() );
    if( xBrd.is() )
        static_cast < SwDrawModellListener_Impl * >( xBrd.get() )->Dispose();
}

SwAccessibleObjShape_Impl
    *SwAccessibleShapeMap_Impl::Copy(
            size_t& rSize, const SwFEShell *pFESh,
            SwAccessibleObjShape_Impl **pSelStart ) const
{
    SwAccessibleObjShape_Impl *pShapes = nullptr;
    SwAccessibleObjShape_Impl *pSelShape = nullptr;

    size_t nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;
    rSize = maMap.size();

    if( rSize > 0 )
    {
        pShapes = new SwAccessibleObjShape_Impl[rSize];

        const_iterator aIter = maMap.cbegin();
        const_iterator aEndIter = maMap.cend();

        SwAccessibleObjShape_Impl *pShape = pShapes;
        pSelShape = &(pShapes[rSize]);
        while( aIter != aEndIter )
        {
            const SdrObject *pObj = (*aIter).first;
            uno::Reference < XAccessible > xAcc( (*aIter).second );
            if( nSelShapes && pFESh && pFESh->IsObjSelected( *pObj ) )
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
    SwAccessibleChild   maFrameOrObj;             // the child for CHILD_POS_CHANGED and
                                                // the same as xAcc for any other
                                                // event type
    EventType   meType;                         // The event type
    AccessibleStates mnStates;                 // check states or update caret pos

public:
    const SwFrame* mpParentFrame;   // The object that fires the event
    bool IsNoXaccParentFrame() const
    {
        return CHILD_POS_CHANGED == meType && mpParentFrame != nullptr;
    }

public:
    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrameOrObj )
        : mxAcc( pA ),
          maFrameOrObj( rFrameOrObj ),
          meType( eT ),
          mnStates( AccessibleStates::NONE ),
          mpParentFrame( nullptr )
    {}

    SwAccessibleEvent_Impl( EventType eT,
                            const SwAccessibleChild& rFrameOrObj )
        : maFrameOrObj( rFrameOrObj ),
          meType( eT ),
          mnStates( AccessibleStates::NONE ),
          mpParentFrame( nullptr )
    {
        assert(SwAccessibleEvent_Impl::DISPOSE == meType &&
                "wrong event constructor, DISPOSE only");
    }

    explicit SwAccessibleEvent_Impl( EventType eT )
        : meType( eT ),
          mnStates( AccessibleStates::NONE ),
          mpParentFrame( nullptr )
    {
        assert(SwAccessibleEvent_Impl::SHAPE_SELECTION == meType &&
                "wrong event constructor, SHAPE_SELECTION only" );
    }

    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrameOrObj,
                            const SwRect& rR )
        : maOldBox( rR ),
          mxAcc( pA ),
          maFrameOrObj( rFrameOrObj ),
          meType( eT ),
          mnStates( AccessibleStates::NONE ),
          mpParentFrame( nullptr )
    {
        assert((SwAccessibleEvent_Impl::CHILD_POS_CHANGED == meType ||
                SwAccessibleEvent_Impl::POS_CHANGED == meType) &&
                "wrong event constructor, (CHILD_)POS_CHANGED only" );
    }

    SwAccessibleEvent_Impl( EventType eT,
                            SwAccessibleContext *pA,
                            const SwAccessibleChild& rFrameOrObj,
                            const AccessibleStates _nStates )
        : mxAcc( pA ),
          maFrameOrObj( rFrameOrObj ),
          meType( eT ),
          mnStates( _nStates ),
          mpParentFrame( nullptr )
    {
        assert( SwAccessibleEvent_Impl::CARET_OR_STATES == meType &&
                "wrong event constructor, CARET_OR_STATES only" );
    }

    SwAccessibleEvent_Impl( EventType eT, const SwFrame *pParentFrame,
                const SwAccessibleChild& rFrameOrObj, const SwRect& rR ) :
        maOldBox( rR ),
        maFrameOrObj( rFrameOrObj ),
        meType( eT ),
        mnStates( AccessibleStates::NONE ),
        mpParentFrame( pParentFrame )
    {
        assert( SwAccessibleEvent_Impl::CHILD_POS_CHANGED == meType &&
            "wrong event constructor, CHILD_POS_CHANGED only" );
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

    inline const SwAccessibleChild& GetFrameOrObj() const
    {
        return maFrameOrObj;
    }

    // <SetStates(..)> only used in method <SwAccessibleMap::AppendEvent(..)>
    inline void SetStates( AccessibleStates _nStates )
    {
        mnStates |= _nStates;
    }

    inline bool IsUpdateCursorPos() const
    {
        return bool(mnStates & AccessibleStates::CARET);
    }
    inline bool IsInvalidateStates() const
    {
        return bool(mnStates & (AccessibleStates::EDITABLE | AccessibleStates::OPAQUE));
    }
    inline bool IsInvalidateRelation() const
    {
        return bool(mnStates & (AccessibleStates::RELATION_FROM | AccessibleStates::RELATION_TO));
    }
    inline bool IsInvalidateTextSelection() const
    {
        return bool( mnStates & AccessibleStates::TEXT_SELECTION_CHANGED );
    }

    inline bool IsInvalidateTextAttrs() const
    {
        return bool( mnStates & AccessibleStates::TEXT_ATTRIBUTE_CHANGED );
    }

    inline AccessibleStates GetStates() const
    {
        return mnStates;
    }

    inline AccessibleStates GetAllStates() const
    {
        return mnStates;
    }
};

class SwAccessibleEventList_Impl: public ::std::list < SwAccessibleEvent_Impl >
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

    void MoveInvalidXAccToEnd();
};

void SwAccessibleEventList_Impl::MoveInvalidXAccToEnd()
{
    size_t nSize = size();
    if (nSize < 2 )
    {
        return;
    }
    SwAccessibleEventList_Impl lstEvent;
    iterator li = begin();
    for ( ;li != end();)
    {
        SwAccessibleEvent_Impl e = *li;
        if (e.IsNoXaccParentFrame())
        {
            iterator liNext = li;
            ++liNext;
            erase(li);
            li = liNext;
            lstEvent.insert(lstEvent.end(),e);
        }
        else
            ++li;
    }
    OSL_ENSURE(size() + lstEvent.size() == nSize ,"");
    insert(end(),lstEvent.begin(),lstEvent.end());
    OSL_ENSURE(size() == nSize ,"");
}

// The shape list is filled if an accessible shape is destroyed. It
// simply keeps a reference to the accessible shape's XShape. These
// references are destroyed within the EndAction when firing events.
// There are two reason for this. First of all, a new accessible shape
// for the XShape might be created soon. It's then cheaper if the XShape
// still exists. The other reason are situations where an accessible shape
// is destroyed within an SwFrameFormat::Modify. In this case, destroying
// the XShape at the same time (indirectly by destroying the accessible
// shape) leads to an assert, because a client of the Modify is destroyed
// within a Modify call.

class SwShapeList_Impl: public ::std::list < uno::Reference < drawing::XShape > >
{
public:
    SwShapeList_Impl() {}
};

struct SwAccessibleChildFunc
{
    bool operator()( const SwAccessibleChild& r1,
                         const SwAccessibleChild& r2 ) const
    {
        const void *p1 = r1.GetSwFrame()
                         ? static_cast < const void * >( r1.GetSwFrame())
                         : ( r1.GetDrawObject()
                             ? static_cast < const void * >( r1.GetDrawObject() )
                             : static_cast < const void * >( r1.GetWindow() ) );
        const void *p2 = r2.GetSwFrame()
                         ? static_cast < const void * >( r2.GetSwFrame())
                         : ( r2.GetDrawObject()
                             ? static_cast < const void * >( r2.GetDrawObject() )
                             : static_cast < const void * >( r2.GetWindow() ) );
        return p1 < p2;
    }
};

class SwAccessibleEventMap_Impl
{
public:
    typedef SwAccessibleChild                                           key_type;
    typedef SwAccessibleEventList_Impl::iterator                        mapped_type;
    typedef std::pair<const key_type,mapped_type>                       value_type;
    typedef SwAccessibleChildFunc                                       key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator        iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_iterator  const_iterator;
private:
    std::map <key_type,mapped_type,key_compare> maMap;
public:
#if OSL_DEBUG_LEVEL > 0
    iterator begin() { return maMap.begin(); }
#endif
    iterator end() { return maMap.end(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    std::pair<iterator,bool> insert(const value_type& value ) { return maMap.insert(value); }
    iterator erase(const_iterator pos) { return maMap.erase(pos); }
};

struct SwAccessibleParaSelection
{
    sal_Int32 nStartOfSelection;
    sal_Int32 nEndOfSelection;

    SwAccessibleParaSelection( const sal_Int32 _nStartOfSelection,
                               const sal_Int32 _nEndOfSelection )
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

class SwAccessibleSelectedParas_Impl
{
public:
    typedef uno::WeakReference < XAccessible >                          key_type;
    typedef SwAccessibleParaSelection                                   mapped_type;
    typedef std::pair<const key_type,mapped_type>                       value_type;
    typedef SwXAccWeakRefComp                                           key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator        iterator;
    typedef std::map<key_type,mapped_type,key_compare>::const_iterator  const_iterator;
private:
    std::map<key_type,mapped_type,key_compare> maMap;
public:
    iterator begin() { return maMap.begin(); }
    iterator end() { return maMap.end(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    std::pair<iterator,bool> insert(const value_type& value ) { return maMap.insert(value); }
    iterator erase(const_iterator pos) { return maMap.erase(pos); }
};

// helper class that stores preview data
class SwAccPreviewData
{
    typedef std::vector<Rectangle> Rectangles;
    Rectangles maPreviewRects;
    Rectangles maLogicRects;

    SwRect maVisArea;
    Fraction maScale;

    const SwPageFrame *mpSelPage;

    /** adjust logic page rectangle to its visible part

        @param _iorLogicPgSwRect
        input/output parameter - reference to the logic page rectangle, which
        has to be adjusted.

        @param _rPreviewPgSwRect
        input parameter - constant reference to the corresponding preview page
        rectangle; needed to determine the visible part of the logic page rectangle.

        @param _rPreviewWinSize
        input parameter - constant reference to the preview window size in TWIP;
        needed to determine the visible part of the logic page rectangle
    */
    static void AdjustLogicPgRectToVisibleArea( SwRect&         _iorLogicPgSwRect,
                                         const SwRect&   _rPreviewPgSwRect,
                                         const Size&     _rPreviewWinSize );

public:
    SwAccPreviewData();
    ~SwAccPreviewData();

    void Update( const SwAccessibleMap& rAccMap,
                 const std::vector<PreviewPage*>& _rPreviewPages,
                 const Fraction&  _rScale,
                 const SwPageFrame* _pSelectedPageFrame,
                 const Size&      _rPreviewWinSize );

    void InvalidateSelection( const SwPageFrame* _pSelectedPageFrame );

    const SwRect& GetVisArea() const { return maVisArea;}

    /** Adjust the MapMode so that the preview page appears at the
     * proper position. rPoint identifies the page for which the
     * MapMode should be adjusted. If bFromPreview is true, rPoint is
     * a preview coordinate; else it's a document coordinate. */
    void AdjustMapMode( MapMode& rMapMode,
                        const Point& rPoint ) const;

    inline const SwPageFrame *GetSelPage() const { return mpSelPage; }

    void DisposePage(const SwPageFrame *pPageFrame );
};

SwAccPreviewData::SwAccPreviewData() :
    mpSelPage( nullptr )
{
}

SwAccPreviewData::~SwAccPreviewData()
{
}

void SwAccPreviewData::Update( const SwAccessibleMap& rAccMap,
                               const std::vector<PreviewPage*>& _rPreviewPages,
                               const Fraction&  _rScale,
                               const SwPageFrame* _pSelectedPageFrame,
                               const Size&      _rPreviewWinSize )
{
    // store preview scaling, maximal preview page size and selected page
    maScale = _rScale;
    mpSelPage = _pSelectedPageFrame;

    // prepare loop on preview pages
    maPreviewRects.clear();
    maLogicRects.clear();
    SwAccessibleChild aPage;
    maVisArea.Clear();

    // loop on preview pages to calculate <maPreviewRects>, <maLogicRects> and
    // <maVisArea>
    for ( std::vector<PreviewPage*>::const_iterator aPageIter = _rPreviewPages.begin();
          aPageIter != _rPreviewPages.end();
          ++aPageIter )
    {
        aPage = (*aPageIter)->pPage;

        // add preview page rectangle to <maPreviewRects>
        Rectangle aPreviewPgRect( (*aPageIter)->aPreviewWinPos, (*aPageIter)->aPageSize );
        maPreviewRects.push_back( aPreviewPgRect );

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
                                                SwRect( aPreviewPgRect ),
                                                _rPreviewWinSize );
            }
            if ( maVisArea.IsEmpty() )
                maVisArea = aLogicPgSwRect;
            else
                maVisArea.Union( aLogicPgSwRect );
        }
    }
}

void SwAccPreviewData::InvalidateSelection( const SwPageFrame* _pSelectedPageFrame )
{
    mpSelPage = _pSelectedPageFrame;
    OSL_ENSURE( mpSelPage, "selected page not found" );
}

struct ContainsPredicate
{
    const Point& mrPoint;
    explicit ContainsPredicate( const Point& rPoint ) : mrPoint(rPoint) {}
    bool operator() ( const Rectangle& rRect ) const
    {
        return rRect.IsInside( mrPoint );
    }
};


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

void SwAccPreviewData::DisposePage(const SwPageFrame *pPageFrame )
{
    if( mpSelPage == pPageFrame )
        mpSelPage = nullptr;
}

// adjust logic page rectangle to its visible part
void SwAccPreviewData::AdjustLogicPgRectToVisibleArea(
                            SwRect&         _iorLogicPgSwRect,
                            const SwRect&   _rPreviewPgSwRect,
                            const Size&     _rPreviewWinSize )
{
    // determine preview window rectangle
    const SwRect aPreviewWinSwRect( Point( 0, 0 ), _rPreviewWinSize );
    // calculate visible preview page rectangle
    SwRect aVisPreviewPgSwRect( _rPreviewPgSwRect );
    aVisPreviewPgSwRect.Intersection( aPreviewWinSwRect );
    // adjust logic page rectangle
    SwTwips nTmpDiff;
    // left
    nTmpDiff = aVisPreviewPgSwRect.Left() - _rPreviewPgSwRect.Left();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Left( _iorLogicPgSwRect.Left() + nTmpDiff );
    // top
    nTmpDiff = aVisPreviewPgSwRect.Top() - _rPreviewPgSwRect.Top();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Top( _iorLogicPgSwRect.Top() + nTmpDiff );
    // right
    nTmpDiff = _rPreviewPgSwRect.Right() - aVisPreviewPgSwRect.Right();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Right( _iorLogicPgSwRect.Right() - nTmpDiff );
    // bottom
    nTmpDiff = _rPreviewPgSwRect.Bottom() - aVisPreviewPgSwRect.Bottom();
    if ( nTmpDiff > 0 )
        _iorLogicPgSwRect.Bottom( _iorLogicPgSwRect.Bottom() - nTmpDiff );
}

static bool AreInSameTable( const uno::Reference< XAccessible >& rAcc,
                                  const SwFrame *pFrame )
{
    bool bRet = false;

    if( pFrame && pFrame->IsCellFrame() && rAcc.is() )
    {
        // Is it in the same table? We check that
        // by comparing the last table frame in the
        // follow chain, because that's cheaper than
        // searching the first one.
        SwAccessibleContext *pAccImpl =
            static_cast< SwAccessibleContext *>( rAcc.get() );
        if( pAccImpl->GetFrame()->IsCellFrame() )
        {
            const SwTabFrame *pTabFrame1 = pAccImpl->GetFrame()->FindTabFrame();
            if (pTabFrame1)
            {
                while (pTabFrame1->GetFollow())
                    pTabFrame1 = pTabFrame1->GetFollow();
            }

            const SwTabFrame *pTabFrame2 = pFrame->FindTabFrame();
            if (pTabFrame2)
            {
                while (pTabFrame2->GetFollow())
                    pTabFrame2 = pTabFrame2->GetFollow();
            }

            bRet = (pTabFrame1 == pTabFrame2);
        }
    }

    return bRet;
}

void SwAccessibleMap::FireEvent( const SwAccessibleEvent_Impl& rEvent )
{
    ::rtl::Reference < SwAccessibleContext > xAccImpl( rEvent.GetContext() );
    if (!xAccImpl.is() && rEvent.mpParentFrame != nullptr)
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            mpFrameMap->find( rEvent.mpParentFrame );
        if( aIter != mpFrameMap->end() )
        {
            uno::Reference < XAccessible > xAcc( (*aIter).second );
            if (xAcc.is())
            {
                uno::Reference < XAccessibleContext >  xContext(xAcc,uno::UNO_QUERY);
                if (xContext.is() && xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH)
                {
                    xAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
                }
            }
        }
    }
    if( SwAccessibleEvent_Impl::SHAPE_SELECTION == rEvent.GetType() )
    {
        DoInvalidateShapeSelection();
    }
    else if( xAccImpl.is() && xAccImpl->GetFrame() )
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
            xAccImpl->InvalidateChildPosOrSize( rEvent.GetFrameOrObj(),
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
                if ( rEvent.GetAllStates() & AccessibleStates::RELATION_FROM )
                {
                    xAccImpl->InvalidateRelation(
                        AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED );
                }
                if ( rEvent.GetAllStates() & AccessibleStates::RELATION_TO )
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
                                        mpEventMap->find( rEvent.GetFrameOrObj() );
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
            SwAccessibleEventMap_Impl::value_type aEntry( rEvent.GetFrameOrObj(),
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
    OSL_ENSURE( pAccImpl->GetFrame(), "caret context is disposed" );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::CARET_OR_STATES,
                                       pAccImpl,
                                       SwAccessibleChild(pAccImpl->GetFrame()),
                                       AccessibleStates::CARET );
        AppendEvent( aEvent );
    }
    else
    {
        FireEvents();
        // While firing events the current frame might have
        // been disposed because it moved out of the vis area.
        // Setting the cursor for such frames is useless and even
        // causes asserts.
        if( pAccImpl->GetFrame() )
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

//This method should implement the following functions:
//1.find the shape objects and set the selected state.
//2.find the Swframe objects and set the selected state.
//3.find the paragraph objects and set the selected state.
void SwAccessibleMap::InvalidateShapeInParaSelection()
{
    SwAccessibleObjShape_Impl *pShapes = nullptr;
    SwAccessibleObjShape_Impl *pSelShape = nullptr;
    size_t nShapes = 0;

    const SwViewShell *pVSh = GetShell();
    const SwFEShell *pFESh = dynamic_cast<const SwFEShell*>( pVSh) !=  nullptr ?
                            static_cast< const SwFEShell * >( pVSh ) : nullptr;
    SwPaM* pCursor = pFESh ? pFESh->GetCursor( false /* ??? */ ) : nullptr;

    //const size_t nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;

    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    bool bIsSelAll =IsDocumentSelAll();

    if( mpShapeMap )
    {
        //Checked for shapes.
        SwAccessibleShapeMap_Impl::const_iterator aIter = mpShapeMap->cbegin();
        SwAccessibleShapeMap_Impl::const_iterator aEndIter = mpShapeMap->cend();
        ::rtl::Reference< SwAccessibleContext > xParentAccImpl;

        if( bIsSelAll)
        {
            while( aIter != aEndIter )
            {
                uno::Reference < XAccessible > xAcc( (*aIter).second );
                if( xAcc.is() )
                    (static_cast < ::accessibility::AccessibleShape* >(xAcc.get()))->SetState( AccessibleStateType::SELECTED );

                ++aIter;
            }
        }
        else
        {
            while( aIter != aEndIter )
            {
                SwAccessibleChild aFrame( (*aIter).first );

                const SwFrameFormat *pFrameFormat = (*aIter).first ? ::FindFrameFormat( (*aIter).first ) : nullptr;
                if( !pFrameFormat )
                {
                    ++aIter;
                    continue;
                }
                const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                const SwPosition *pPos = rAnchor.GetContentAnchor();

                if(rAnchor.GetAnchorId() == FLY_AT_PAGE)
                {
                    uno::Reference < XAccessible > xAcc( (*aIter).second );
                    if(xAcc.is())
                        (static_cast < ::accessibility::AccessibleShape* >(xAcc.get()))->ResetState( AccessibleStateType::SELECTED );

                    ++aIter;
                    continue;
                }

                if( !pPos )
                {
                    ++aIter;
                    continue;
                }
                if( pPos->nNode.GetNode().GetTextNode() )
                {
                    int nIndex = pPos->nContent.GetIndex();
                    bool bMarked = false;
                    if( pCursor != nullptr )
                    {
                        const SwTextNode* pNode = pPos->nNode.GetNode().GetTextNode();
                        sal_uLong nHere = pNode->GetIndex();

                        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
                        {
                            // ignore, if no mark
                            if( rTmpCursor.HasMark() )
                            {
                                bMarked = true;
                                // check whether nHere is 'inside' pCursor
                                SwPosition* pStart = rTmpCursor.Start();
                                sal_uLong nStartIndex = pStart->nNode.GetIndex();
                                SwPosition* pEnd = rTmpCursor.End();
                                sal_uLong nEndIndex = pEnd->nNode.GetIndex();
                                if( ( nHere >= nStartIndex ) && (nHere <= nEndIndex)  )
                                {
                                    if( rAnchor.GetAnchorId() == FLY_AS_CHAR )
                                    {
                                        if( ( ((nHere == nStartIndex) && (nIndex >= pStart->nContent.GetIndex())) || (nHere > nStartIndex) )
                                            &&( ((nHere == nEndIndex) && (nIndex < pEnd->nContent.GetIndex())) || (nHere < nEndIndex) ) )
                                        {
                                            uno::Reference < XAccessible > xAcc( (*aIter).second );
                                            if( xAcc.is() )
                                                static_cast < ::accessibility::AccessibleShape* >(xAcc.get())->SetState( AccessibleStateType::SELECTED );
                                        }
                                        else
                                        {
                                            uno::Reference < XAccessible > xAcc( (*aIter).second );
                                            if( xAcc.is() )
                                                static_cast < ::accessibility::AccessibleShape* >(xAcc.get())->ResetState( AccessibleStateType::SELECTED );
                                        }
                                    }
                                    else if( rAnchor.GetAnchorId() == FLY_AT_PARA )
                                    {
                                        if( ((nHere > nStartIndex) || pStart->nContent.GetIndex() ==0 )
                                            && (nHere < nEndIndex ) )
                                        {
                                            uno::Reference < XAccessible > xAcc( (*aIter).second );
                                            if( xAcc.is() )
                                                static_cast < ::accessibility::AccessibleShape* >(xAcc.get())->SetState( AccessibleStateType::SELECTED );
                                        }
                                        else
                                        {
                                            uno::Reference < XAccessible > xAcc( (*aIter).second );
                                            if(xAcc.is())
                                                (static_cast < ::accessibility::AccessibleShape* >(xAcc.get()))->ResetState( AccessibleStateType::SELECTED );
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if( !bMarked )
                    {
                        SwAccessibleObjShape_Impl  *pShape = pShapes;
                        size_t nNumShapes = nShapes;
                        while( nNumShapes )
                        {
                            if( pShape < pSelShape && (pShape->first==(*aIter).first) )
                            {
                                uno::Reference < XAccessible > xAcc( (*aIter).second );
                                if(xAcc.is())
                                    (static_cast < ::accessibility::AccessibleShape* >(xAcc.get()))->ResetState( AccessibleStateType::SELECTED );
                            }
                            --nNumShapes;
                            ++pShape;
                        }
                    }
                }

                ++aIter;
            }//while( aIter != aEndIter )
        }//else
    }

    delete[] pShapes;

    //Checked for FlyFrame
    if (mpFrameMap)
    {
        SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->begin();
        while( aIter != mpFrameMap->end() )
        {
            const SwFrame *pFrame = (*aIter).first;
            if(pFrame->IsFlyFrame())
            {
                uno::Reference < XAccessible > xAcc = (*aIter).second;

                if(xAcc.is())
                {
                    SwAccessibleFrameBase *pAccFrame = (static_cast< SwAccessibleFrameBase * >(xAcc.get()));
                    bool bFrameChanged = pAccFrame->SetSelectedState( true );
                    if (bFrameChanged)
                    {
                        const SwFlyFrame *pFlyFrame = static_cast< const SwFlyFrame * >( pFrame );
                        const SwFrameFormat *pFrameFormat = pFlyFrame->GetFormat();
                        if (pFrameFormat)
                        {
                            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                            if( rAnchor.GetAnchorId() == FLY_AS_CHAR )
                            {
                                uno::Reference< XAccessible > xAccParent = pAccFrame->getAccessibleParent();
                                if (xAccParent.is())
                                {
                                    uno::Reference< XAccessibleContext > xAccContext = xAccParent->getAccessibleContext();
                                    if(xAccContext.is() && xAccContext->getAccessibleRole() == AccessibleRole::PARAGRAPH)
                                    {
                                        SwAccessibleParagraph* pAccPara = static_cast< SwAccessibleParagraph *>(xAccContext.get());
                                        if(pAccFrame->IsSeletedInDoc())
                                        {
                                            m_setParaAdd.insert(pAccPara);
                                        }
                                        else if(m_setParaAdd.count(pAccPara) == 0)
                                        {
                                            m_setParaRemove.insert(pAccPara);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ++aIter;
        }
    }

    typedef std::vector< SwAccessibleContext* > VEC_PARA;
    VEC_PARA vecAdd;
    VEC_PARA vecRemove;
    //Checked for Paras.
    bool bMarkChanged = false;
    SwAccessibleContextMap_Impl mapTemp;
    if( pCursor != nullptr )
    {
        for(SwPaM& rTmpCursor : pCursor->GetRingContainer())
        {
            if( rTmpCursor.HasMark() )
            {
                SwNodeIndex nStartIndex( rTmpCursor.Start()->nNode );
                SwNodeIndex nEndIndex( rTmpCursor.End()->nNode );
                while(nStartIndex <= nEndIndex)
                {
                    SwFrame *pFrame = nullptr;
                    if(nStartIndex.GetNode().IsContentNode())
                    {
                        SwContentNode* pCNd = static_cast<SwContentNode*>(&(nStartIndex.GetNode()));
                        pFrame = SwIterator<SwFrame, SwContentNode>(*pCNd).First();
                    }
                    else if( nStartIndex.GetNode().IsTableNode() )
                    {
                        SwTableNode * pTable = static_cast<SwTableNode *>(&(nStartIndex.GetNode()));
                        SwTableFormat* pFormat = pTable->GetTable().GetFrameFormat();
                        pFrame = SwIterator<SwFrame, SwTableFormat>(*pFormat).First();
                    }

                    if( pFrame && mpFrameMap)
                    {
                        SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->find( pFrame );
                        if( aIter != mpFrameMap->end() )
                        {
                            uno::Reference < XAccessible > xAcc = (*aIter).second;
                            bool isChanged = false;
                            if( xAcc.is() )
                            {
                                isChanged = (static_cast< SwAccessibleContext * >(xAcc.get()))->SetSelectedState( true );
                            }
                            if(!isChanged)
                            {
                                SwAccessibleContextMap_Impl::iterator aEraseIter = mpSeletedFrameMap->find( pFrame );
                                if(aEraseIter != mpSeletedFrameMap->end())
                                    mpSeletedFrameMap->erase(aEraseIter);
                            }
                            else
                            {
                                bMarkChanged = true;
                                vecAdd.push_back(static_cast< SwAccessibleContext * >(xAcc.get()));
                            }

                            mapTemp.insert( SwAccessibleContextMap_Impl::value_type( pFrame, xAcc ) );
                        }
                    }
                    ++nStartIndex;
                }
            }
        }
    }
    if( !mpSeletedFrameMap )
        mpSeletedFrameMap = new SwAccessibleContextMap_Impl;
    if( !mpSeletedFrameMap->empty() )
    {
        SwAccessibleContextMap_Impl::iterator aIter = mpSeletedFrameMap->begin();
        while( aIter != mpSeletedFrameMap->end() )
        {
            uno::Reference < XAccessible > xAcc = (*aIter).second;
            if(xAcc.is())
                (static_cast< SwAccessibleContext * >(xAcc.get()))->SetSelectedState( false );
            ++aIter;
            vecRemove.push_back(static_cast< SwAccessibleContext * >(xAcc.get()));
        }
        bMarkChanged = true;
        mpSeletedFrameMap->clear();
    }

    if( !mapTemp.empty() )
    {
        SwAccessibleContextMap_Impl::iterator aIter = mapTemp.begin();
        while( aIter != mapTemp.end() )
        {
            mpSeletedFrameMap->insert( SwAccessibleContextMap_Impl::value_type( (*aIter).first, (*aIter).second ) );
            ++aIter;
        }
        mapTemp.clear();
    }
    if( bMarkChanged && mpFrameMap)
    {
        VEC_PARA::iterator vi = vecAdd.begin();
        for (; vi != vecAdd.end() ; ++vi)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
            SwAccessibleContext* pAccPara = *vi;
            if (pAccPara)
            {
                pAccPara->FireAccessibleEvent( aEvent );
            }
        }
        vi = vecRemove.begin();
        for (; vi != vecRemove.end() ; ++vi)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_REMOVE;
            SwAccessibleContext* pAccPara = *vi;
            if (pAccPara)
            {
                pAccPara->FireAccessibleEvent( aEvent );
            }
        }
    }
}

//Marge with DoInvalidateShapeFocus
void SwAccessibleMap::DoInvalidateShapeSelection(bool bInvalidateFocusMode /*=false*/)
{
    SwAccessibleObjShape_Impl *pShapes = nullptr;
    SwAccessibleObjShape_Impl *pSelShape = nullptr;
    size_t nShapes = 0;

    const SwViewShell *pVSh = GetShell();
    const SwFEShell *pFESh = dynamic_cast<const SwFEShell*>( pVSh) !=  nullptr ?
                            static_cast< const SwFEShell * >( pVSh ) : nullptr;
    const size_t nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;

    //when InvalidateFocus Call this function ,and the current selected shape count is not 1 ,
    //return
    if (bInvalidateFocusMode && nSelShapes != 1)
    {
        return;
    }
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        typedef std::vector< ::rtl::Reference < ::accessibility::AccessibleShape >  >  VEC_SHAPE;
        VEC_SHAPE vecxShapeAdd;
        VEC_SHAPE vecxShapeRemove;
        int nCountSelectedShape=0;

        vcl::Window *pWin = GetShell()->GetWin();
        bool bFocused = pWin && pWin->HasFocus();
        SwAccessibleObjShape_Impl *pShape = pShapes;
        int nShapeCount = nShapes;
        while( nShapeCount )
        {
            if (pShape->second.is() && IsInSameLevel(pShape->first, pFESh))
            {
                if( pShape < pSelShape )
                {
                    if(pShape->second->ResetState( AccessibleStateType::SELECTED ))
                    {
                        vecxShapeRemove.push_back(pShape->second);
                    }
                    pShape->second->ResetState( AccessibleStateType::FOCUSED );
                }
            }
            --nShapeCount;
            ++pShape;
        }

        VEC_SHAPE::iterator vi =vecxShapeRemove.begin();
        for (; vi != vecxShapeRemove.end(); ++vi)
        {
            ::accessibility::AccessibleShape *pAccShape = static_cast< ::accessibility::AccessibleShape * >(vi->get());
            if (pAccShape)
            {
                pAccShape->CommitChange(AccessibleEventId::SELECTION_CHANGED_REMOVE, uno::Any(), uno::Any());
            }
        }

        pShape = pShapes;

        while( nShapes )
        {
            if (pShape->second.is() && IsInSameLevel(pShape->first, pFESh))
            {
                if( pShape >= pSelShape )
                {
                    //first fire focus event
                    if( bFocused && 1 == nSelShapes )
                        pShape->second->SetState( AccessibleStateType::FOCUSED );
                    else
                        pShape->second->ResetState( AccessibleStateType::FOCUSED );

                    if(pShape->second->SetState( AccessibleStateType::SELECTED ))
                    {
                        vecxShapeAdd.push_back(pShape->second);
                    }
                    ++nCountSelectedShape;
                }
            }

            --nShapes;
            ++pShape;
        }

        const unsigned int SELECTION_WITH_NUM = 10;
        if (vecxShapeAdd.size() > SELECTION_WITH_NUM )
        {
             uno::Reference< XAccessible > xDoc = GetDocumentView( );
             SwAccessibleContext * pCont = static_cast<SwAccessibleContext *>(xDoc.get());
             if (pCont)
             {
                 AccessibleEventObject aEvent;
                 aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
                 pCont->FireAccessibleEvent(aEvent);
             }
        }
        else
        {
            short nEventID = AccessibleEventId::SELECTION_CHANGED_ADD;
            if (nCountSelectedShape <= 1 && vecxShapeAdd.size() == 1 )
            {
                nEventID = AccessibleEventId::SELECTION_CHANGED;
            }
            vi = vecxShapeAdd.begin();
            for (; vi != vecxShapeAdd.end(); ++vi)
            {
                ::accessibility::AccessibleShape *pAccShape = static_cast< ::accessibility::AccessibleShape * >(vi->get());
                if (pAccShape)
                {
                    pAccShape->CommitChange(nEventID, uno::Any(), uno::Any());
                }
            }
        }

        vi = vecxShapeAdd.begin();
        for (; vi != vecxShapeAdd.end(); ++vi)
        {
            ::accessibility::AccessibleShape *pAccShape = static_cast< ::accessibility::AccessibleShape * >(vi->get());
            if (pAccShape)
            {
                SdrObject *pObj = GetSdrObjectFromXShape(pAccShape->GetXShape());
                SwFrameFormat *pFrameFormat = pObj ? FindFrameFormat( pObj ) : nullptr;
                if (pFrameFormat)
                {
                    const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
                    if( rAnchor.GetAnchorId() == FLY_AS_CHAR )
                    {
                        uno::Reference< XAccessible > xPara = pAccShape->getAccessibleParent();
                        if (xPara.is())
                        {
                            uno::Reference< XAccessibleContext > xParaContext = xPara->getAccessibleContext();
                            if (xParaContext.is() && xParaContext->getAccessibleRole() == AccessibleRole::PARAGRAPH)
                            {
                                SwAccessibleParagraph* pAccPara = static_cast< SwAccessibleParagraph *>(xPara.get());
                                if (pAccPara)
                                {
                                    m_setParaAdd.insert(pAccPara);
                                }
                            }
                        }
                    }
                }
            }
        }
        vi = vecxShapeRemove.begin();
        for (; vi != vecxShapeRemove.end(); ++vi)
        {
            ::accessibility::AccessibleShape *pAccShape = static_cast< ::accessibility::AccessibleShape * >(vi->get());
            if (pAccShape)
            {
                uno::Reference< XAccessible > xPara = pAccShape->getAccessibleParent();
                uno::Reference< XAccessibleContext > xParaContext = xPara->getAccessibleContext();
                if (xParaContext.is() && xParaContext->getAccessibleRole() == AccessibleRole::PARAGRAPH)
                {
                    SwAccessibleParagraph* pAccPara = static_cast< SwAccessibleParagraph *>(xPara.get());
                    if (m_setParaAdd.count(pAccPara) == 0 )
                    {
                        m_setParaRemove.insert(pAccPara);
                    }
                }
            }
        }

        delete[] pShapes;
    }
}

//Marge with DoInvalidateShapeSelection
/*
void SwAccessibleMap::DoInvalidateShapeFocus()
{
    const SwViewShell *pVSh = GetShell();
    const SwFEShell *pFESh = dynamic_cast<const SwFEShell*>( pVSh) !=  nullptr ?
                            static_cast< const SwFEShell * >( pVSh ) : nullptr;
    const size_t nSelShapes = pFESh ? pFESh->IsObjSelected() : 0;

    if( nSelShapes != 1 )
        return;

    SwAccessibleObjShape_Impl *pShapes = nullptr;
    SwAccessibleObjShape_Impl *pSelShape = nullptr;
    size_t nShapes = 0;

    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
            pShapes = mpShapeMap->Copy( nShapes, pFESh, &pSelShape );
    }

    if( pShapes )
    {
        vcl::Window *pWin = GetShell()->GetWin();
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

*/

SwAccessibleMap::SwAccessibleMap( SwViewShell *pSh ) :
    mpFrameMap( nullptr ),
    mpShapeMap( nullptr ),
    mpShapes( nullptr ),
    mpEvents( nullptr ),
    mpEventMap( nullptr ),
    mpSelectedParas( nullptr ),
    mpVSh( pSh ),
    mpPreview( nullptr ),
    mbShapeSelected( false ),
    mpSeletedFrameMap(nullptr)
{
    pSh->GetLayout()->AddAccessibleShell();
}

SwAccessibleMap::~SwAccessibleMap()
{
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpFrameMap )
        {
            const SwRootFrame *pRootFrame = GetShell()->GetLayout();
            SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->find( pRootFrame );
            if( aIter != mpFrameMap->end() )
                xAcc = (*aIter).second;
            if( !xAcc.is() )
                xAcc = new SwAccessibleDocument( this );
        }
    }

    if(xAcc.is())
    {
        SwAccessibleDocument *pAcc = static_cast< SwAccessibleDocument * >( xAcc.get() );
        pAcc->Dispose( true );
    }
    if( mpFrameMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->begin();
        while( aIter != mpFrameMap->end() )
        {
            uno::Reference < XAccessible > xTmp = (*aIter).second;
            if( xTmp.is() )
            {
                SwAccessibleContext *pTmp = static_cast< SwAccessibleContext * >( xTmp.get() );
                pTmp->SetMap(nullptr);
            }
            ++aIter;
        }
    }
    {
        osl::MutexGuard aGuard( maMutex );
#if OSL_DEBUG_LEVEL > 0
        assert((!mpFrameMap || mpFrameMap->empty()) &&
                "Frame map should be empty after disposing the root frame");
        if( mpFrameMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->begin();
            while( aIter != mpFrameMap->end() )
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
        assert((!mpShapeMap || mpShapeMap->empty()) &&
                "Object map should be empty after disposing the root frame");
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
        delete mpFrameMap;
        mpFrameMap = nullptr;
        delete mpShapeMap;
        mpShapeMap = nullptr;
        delete mpShapes;
        mpShapes = nullptr;
        delete mpSelectedParas;
        mpSelectedParas = nullptr;
    }

    delete mpPreview;
    mpPreview = nullptr;

    {
        osl::MutexGuard aGuard( maEventMutex );
#if OSL_DEBUG_LEVEL > 0
        assert(!(mpEvents || mpEventMap));
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
        mpEventMap = nullptr;
        delete mpEvents;
        mpEvents = nullptr;
    }
    mpVSh->GetLayout()->RemoveAccessibleShell();
    delete mpSeletedFrameMap;
}

uno::Reference< XAccessible > SwAccessibleMap::GetDocumentView_(
    bool bPagePreview )
{
    uno::Reference < XAccessible > xAcc;
    bool bSetVisArea = false;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpFrameMap )
        {
            mpFrameMap = new SwAccessibleContextMap_Impl;
#if OSL_DEBUG_LEVEL > 0
            mpFrameMap->mbLocked = false;
#endif
        }

#if OSL_DEBUG_LEVEL > 0
        OSL_ENSURE( !mpFrameMap->mbLocked, "Map is locked" );
        mpFrameMap->mbLocked = true;
#endif

        const SwRootFrame *pRootFrame = GetShell()->GetLayout();
        SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->find( pRootFrame );
        if( aIter != mpFrameMap->end() )
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

            if( aIter != mpFrameMap->end() )
            {
                (*aIter).second = xAcc;
            }
            else
            {
                SwAccessibleContextMap_Impl::value_type aEntry( pRootFrame, xAcc );
                mpFrameMap->insert( aEntry );
            }
        }

#if OSL_DEBUG_LEVEL > 0
        mpFrameMap->mbLocked = false;
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
    return GetDocumentView_( false );
}

uno::Reference<XAccessible> SwAccessibleMap::GetDocumentPreview(
                                    const std::vector<PreviewPage*>& _rPreviewPages,
                                    const Fraction&  _rScale,
                                    const SwPageFrame* _pSelectedPageFrame,
                                    const Size&      _rPreviewWinSize )
{
    // create & update preview data object
    if( mpPreview == nullptr )
        mpPreview = new SwAccPreviewData();
    mpPreview->Update( *this, _rPreviewPages, _rScale, _pSelectedPageFrame, _rPreviewWinSize );

    uno::Reference<XAccessible> xAcc = GetDocumentView_( true );
    return xAcc;
}

uno::Reference< XAccessible> SwAccessibleMap::GetContext( const SwFrame *pFrame,
                                                     bool bCreate )
{
    uno::Reference < XAccessible > xAcc;
    uno::Reference < XAccessible > xOldCursorAcc;
    bool bOldShapeSelected = false;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpFrameMap && bCreate )
            mpFrameMap = new SwAccessibleContextMap_Impl;
        if( mpFrameMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->find( pFrame );
            if( aIter != mpFrameMap->end() )
                xAcc = (*aIter).second;

            if( !xAcc.is() && bCreate )
            {
                SwAccessibleContext *pAcc = nullptr;
                switch( pFrame->GetType() )
                {
                case SwFrameType::Txt:
                    pAcc = new SwAccessibleParagraph( this,
                                    static_cast< const SwTextFrame& >( *pFrame ) );
                    break;
                case SwFrameType::Header:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwHeaderFrame *>( pFrame ) );
                    break;
                case SwFrameType::Footer:
                    pAcc = new SwAccessibleHeaderFooter( this,
                                    static_cast< const SwFooterFrame *>( pFrame ) );
                    break;
                case SwFrameType::Ftn:
                    {
                        const SwFootnoteFrame *pFootnoteFrame =
                            static_cast < const SwFootnoteFrame * >( pFrame );
                        bool bIsEndnote =
                            SwAccessibleFootnote::IsEndnote( pFootnoteFrame );
                        pAcc = new SwAccessibleFootnote( this, bIsEndnote,
                                    /*(bIsEndnote ? mnEndnote++ : mnFootnote++),*/
                                    pFootnoteFrame );
                    }
                    break;
                case SwFrameType::Fly:
                    {
                        const SwFlyFrame *pFlyFrame =
                            static_cast < const SwFlyFrame * >( pFrame );
                        switch( SwAccessibleFrameBase::GetNodeType( pFlyFrame ) )
                        {
                        case ND_GRFNODE:
                            pAcc = new SwAccessibleGraphic( this, pFlyFrame );
                            break;
                        case ND_OLENODE:
                            pAcc = new SwAccessibleEmbeddedObject( this, pFlyFrame );
                            break;
                        default:
                            pAcc = new SwAccessibleTextFrame( this, *pFlyFrame );
                            break;
                        }
                    }
                    break;
                case SwFrameType::Cell:
                    pAcc = new SwAccessibleCell( this,
                                    static_cast< const SwCellFrame *>( pFrame ) );
                    break;
                case SwFrameType::Tab:
                    pAcc = new SwAccessibleTable( this,
                                    static_cast< const SwTabFrame *>( pFrame ) );
                    break;
                case SwFrameType::Page:
                    OSL_ENSURE( GetShell()->IsPreview(),
                                "accessible page frames only in PagePreview" );
                    pAcc = new SwAccessiblePage( this, pFrame );
                    break;
                default: break;
                }
                xAcc = pAcc;

                OSL_ENSURE( xAcc.is(), "unknown frame type" );
                if( xAcc.is() )
                {
                    if( aIter != mpFrameMap->end() )
                    {
                        (*aIter).second = xAcc;
                    }
                    else
                    {
                        SwAccessibleContextMap_Impl::value_type aEntry( pFrame, xAcc );
                        mpFrameMap->insert( aEntry );
                    }

                    if( pAcc->HasCursor() &&
                        !AreInSameTable( mxCursorContext, pFrame ) )
                    {
                        // If the new context has the focus, and if we know
                        // another context that had the focus, then the focus
                        // just moves from the old context to the new one. We
                        // then have to send a focus event and a caret event for
                        // the old context. We have to do that now,
                        // because after we have left this method, anyone might
                        // call getStates for the new context and will get a
                        // focused state then. Sending the focus changes event
                        // after that seems to be strange. However, we cannot
                        // send a focus event for the new context now, because
                        // no one except us knows it. In any case, we remember
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
            const SwFrame *pFrame,
            bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pFrame, bCreate ) );

    ::rtl::Reference < SwAccessibleContext > xAccImpl(
         static_cast< SwAccessibleContext * >( xAcc.get() ) );

    return xAccImpl;
}

uno::Reference< XAccessible> SwAccessibleMap::GetContext(
        const SdrObject *pObj,
        SwAccessibleContext *pParentImpl,
        bool bCreate )
{
    uno::Reference < XAccessible > xAcc;
    uno::Reference < XAccessible > xOldCursorAcc;

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpShapeMap && bCreate )
            mpShapeMap = new SwAccessibleShapeMap_Impl( this );
        if( mpShapeMap )
        {
            SwAccessibleShapeMap_Impl::iterator aIter = mpShapeMap->find( pObj );
            if( aIter != mpShapeMap->end() )
                xAcc = (*aIter).second;

            if( !xAcc.is() && bCreate )
            {
                rtl::Reference< ::accessibility::AccessibleShape> pAcc;
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
                xAcc = pAcc.get();

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
                if (xAcc.is())
                    AddGroupContext(pObj, xAcc);
            }
        }
    }

    // Invalidate focus for old object when map is not locked
    if( xOldCursorAcc.is() )
        InvalidateCursorPosition( xOldCursorAcc );

    return xAcc;
}

bool SwAccessibleMap::IsInSameLevel(const SdrObject* pObj, const SwFEShell* pFESh)
{
    if (pFESh)
        return pFESh->IsObjSameLevelWithMarked(pObj);
    return false;
}

void SwAccessibleMap::AddShapeContext(const SdrObject *pObj, uno::Reference < XAccessible > xAccShape)
{
    osl::MutexGuard aGuard( maMutex );

    if( mpShapeMap )
    {
        SwAccessibleShapeMap_Impl::value_type aEntry( pObj, xAccShape );
        mpShapeMap->insert( aEntry );
    }

}

//Added by yanjun for sym2_6407
void SwAccessibleMap::RemoveGroupContext(const SdrObject *pParentObj, css::uno::Reference < css::accessibility::XAccessible > xAccParent)
{
    osl::MutexGuard aGuard( maMutex );
    if (mpShapeMap && pParentObj && pParentObj->IsGroupObject() && xAccParent.is())
    {
        uno::Reference < XAccessibleContext > xContext = xAccParent->getAccessibleContext();
        if (xContext.is())
        {
            for (sal_Int32 i = 0; i < xContext->getAccessibleChildCount(); ++i)
            {
                uno::Reference < XAccessible > xChild = xContext->getAccessibleChild(i);
                if (xChild.is())
                {
                    uno::Reference < XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                    if (xChildContext.is())
                    {
                        if (xChildContext->getAccessibleRole() == AccessibleRole::SHAPE)
                        {
                            ::accessibility::AccessibleShape* pAccShape = static_cast < ::accessibility::AccessibleShape* >( xChild.get());
                            uno::Reference < drawing::XShape > xShape = pAccShape->GetXShape();
                            if (xShape.is())
                            {
                                SdrObject* pObj = GetSdrObjectFromXShape(xShape);
                                if (pObj)
                                    RemoveContext(pObj);
                            }
                        }
                    }
                }
            }
        }
    }
}
//End

void SwAccessibleMap::AddGroupContext(const SdrObject *pParentObj, uno::Reference < XAccessible > xAccParent)
{
    osl::MutexGuard aGuard( maMutex );
    if( mpShapeMap )
    {
        //here get all the sub list.
        if (pParentObj->IsGroupObject())
        {
            if (xAccParent.is())
            {
                uno::Reference < XAccessibleContext > xContext = xAccParent->getAccessibleContext();
                if (xContext.is())
                {
                    sal_Int32 nChildren = xContext->getAccessibleChildCount();
                    for(sal_Int32 i = 0; i<nChildren; i++)
                    {
                        uno::Reference < XAccessible > xChild = xContext->getAccessibleChild(i);
                        if (xChild.is())
                        {
                            uno::Reference < XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                            if (xChildContext.is())
                            {
                                short nRole = xChildContext->getAccessibleRole();
                                if (nRole == AccessibleRole::SHAPE)
                                {
                                    ::accessibility::AccessibleShape* pAccShape = static_cast < ::accessibility::AccessibleShape* >( xChild.get());
                                    uno::Reference < drawing::XShape > xShape = pAccShape->GetXShape();
                                    if (xShape.is())
                                    {
                                        SdrObject* pObj = GetSdrObjectFromXShape(xShape);
                                        AddShapeContext(pObj, xChild);
                                        AddGroupContext(pObj,xChild);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

::rtl::Reference < ::accessibility::AccessibleShape > SwAccessibleMap::GetContextImpl(
            const SdrObject *pObj,
            SwAccessibleContext *pParentImpl,
            bool bCreate )
{
    uno::Reference < XAccessible > xAcc( GetContext( pObj, pParentImpl, bCreate ) );

    ::rtl::Reference < ::accessibility::AccessibleShape > xAccImpl(
         static_cast< ::accessibility::AccessibleShape* >( xAcc.get() ) );

    return xAccImpl;
}

void SwAccessibleMap::RemoveContext( const SwFrame *pFrame )
{
    osl::MutexGuard aGuard( maMutex );

    if( mpFrameMap )
    {
        SwAccessibleContextMap_Impl::iterator aIter =
            mpFrameMap->find( pFrame );
        if( aIter != mpFrameMap->end() )
        {
            mpFrameMap->erase( aIter );

            // Remove reference to old caret object. Though mxCursorContext
            // is a weak reference and cleared automatically, clearing it
            // directly makes sure to not keep a non-functional object.
            uno::Reference < XAccessible > xOldAcc( mxCursorContext );
            if( xOldAcc.is() )
            {
                SwAccessibleContext *pOldAccImpl =
                    static_cast< SwAccessibleContext *>( xOldAcc.get() );
                OSL_ENSURE( pOldAccImpl->GetFrame(), "old caret context is disposed" );
                if( pOldAccImpl->GetFrame() == pFrame )
                {
                    xOldAcc.clear();    // get an empty ref
                    mxCursorContext = xOldAcc;
                }
            }

            if( mpFrameMap->empty() )
            {
                delete mpFrameMap;
                mpFrameMap = nullptr;
            }
        }
    }
}

void SwAccessibleMap::RemoveContext( const SdrObject *pObj )
{
    osl::MutexGuard aGuard( maMutex );

    if( mpShapeMap )
    {
        SwAccessibleShapeMap_Impl::iterator aIter = mpShapeMap->find( pObj );
        if( aIter != mpShapeMap->end() )
        {
            uno::Reference < XAccessible > xAcc( (*aIter).second );
            mpShapeMap->erase( aIter );
            RemoveGroupContext(pObj, xAcc);
            // The shape selection flag is not cleared, but one might do
            // so but has to make sure that the removed context is the one
            // that is selected.

            if( mpShapeMap && mpShapeMap->empty() )
            {
                delete mpShapeMap;
                mpShapeMap = nullptr;
            }
        }
    }
}

void SwAccessibleMap::Dispose( const SwFrame *pFrame,
                               const SdrObject *pObj,
                               vcl::Window* pWindow,
                               bool bRecursive )
{
    SwAccessibleChild aFrameOrObj( pFrame, pObj, pWindow );

    // Indeed, the following assert checks the frame's accessible flag,
    // because that's the one that is evaluated in the layout. The frame
    // might not be accessible anyway. That's the case for cell frames that
    // contain further cells.
    OSL_ENSURE( !aFrameOrObj.GetSwFrame() || aFrameOrObj.GetSwFrame()->IsAccessibleFrame(),
            "non accessible frame should be disposed" );

    if (aFrameOrObj.IsAccessible( GetShell()->IsPreview() )
            // fdo#87199 dispose the darn thing if it ever was accessible
        || (pFrame && mpFrameMap && mpFrameMap->find(pFrame) != mpFrameMap->end()))
    {
        ::rtl::Reference< SwAccessibleContext > xAccImpl;
        ::rtl::Reference< SwAccessibleContext > xParentAccImpl;
        ::rtl::Reference< ::accessibility::AccessibleShape > xShapeAccImpl;
        // get accessible context for frame
        {
            osl::MutexGuard aGuard( maMutex );

            // First of all look for an accessible context for a frame
            if( aFrameOrObj.GetSwFrame() && mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
                {
                    uno::Reference < XAccessible > xAcc( (*aIter).second );
                    xAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
                }
            }
            if( !xAccImpl.is() && mpFrameMap )
            {
                // If there is none, look if the parent is accessible.
                const SwFrame *pParent =
                        SwAccessibleFrame::GetParent( aFrameOrObj,
                                                      GetShell()->IsPreview());

                if( pParent )
                {
                    SwAccessibleContextMap_Impl::iterator aIter =
                        mpFrameMap->find( pParent );
                    if( aIter != mpFrameMap->end() )
                    {
                        uno::Reference < XAccessible > xAcc( (*aIter).second );
                        xParentAccImpl =
                            static_cast< SwAccessibleContext *>( xAcc.get() );
                    }
                }
            }
            if( !xParentAccImpl.is() && !aFrameOrObj.GetSwFrame() && mpShapeMap )
            {
                SwAccessibleShapeMap_Impl::iterator aIter =
                    mpShapeMap->find( aFrameOrObj.GetDrawObject() );
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
                // is deleted with a SwFrameFormat::Modify.
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
                    mpEventMap->find( aFrameOrObj );
                if( aIter != mpEventMap->end() )
                {
                    SwAccessibleEvent_Impl aEvent(
                            SwAccessibleEvent_Impl::DISPOSE, aFrameOrObj );
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

            xParentAccImpl->DisposeChild( aFrameOrObj, bRecursive );
        }
        else if( xShapeAccImpl.is() )
        {
            RemoveContext( aFrameOrObj.GetDrawObject() );
            xShapeAccImpl->dispose();
        }

        if( mpPreview && pFrame && pFrame->IsPageFrame() )
            mpPreview->DisposePage( static_cast< const SwPageFrame *>( pFrame ) );
    }
}

void SwAccessibleMap::InvalidatePosOrSize( const SwFrame *pFrame,
                                           const SdrObject *pObj,
                                           vcl::Window* pWindow,
                                           const SwRect& rOldBox )
{
    SwAccessibleChild aFrameOrObj( pFrame, pObj, pWindow );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        ::rtl::Reference< SwAccessibleContext > xAccImpl;
        ::rtl::Reference< SwAccessibleContext > xParentAccImpl;
        const SwFrame *pParent =nullptr;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                if( aFrameOrObj.GetSwFrame() )
                {
                    SwAccessibleContextMap_Impl::iterator aIter =
                        mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                    if( aIter != mpFrameMap->end() )
                    {
                        // If there is an accessible object already it is
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
                    pParent =
                        SwAccessibleFrame::GetParent( aFrameOrObj,
                                                      GetShell()->IsPreview());

                    if( pParent )
                    {
                        SwAccessibleContextMap_Impl::iterator aIter =
                            mpFrameMap->find( pParent );
                        if( aIter != mpFrameMap->end() )
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
                    aFrameOrObj, rOldBox );
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
                    xParentAccImpl.get(), aFrameOrObj, rOldBox );
                AppendEvent( aEvent );
            }
            else
            {
                FireEvents();
                xParentAccImpl->InvalidateChildPosOrSize( aFrameOrObj,
                                                          rOldBox );
            }
        }
        else if(pParent)
        {
/*
For child graphic and its parent paragraph,if split 2 graphic to 2 paragraph,
will delete one graphic swfrm and new create 1 graphic swfrm ,
then the new paragraph and the new graphic SwFrame will add .
but when add graphic SwFrame ,the accessible of the new Paragraph is not created yet.
so the new graphic accessible 'parent is NULL,
so run here: save the parent's SwFrame not the accessible object parent,
*/
            bool bIsValidFrame = false;
            bool bIsTextParent = false;
            if (aFrameOrObj.GetSwFrame())
            {
                if (SwFrameType::Fly == pFrame->GetType())
                {
                    bIsValidFrame =true;
                }
            }
            else if(pObj)
            {
                if (SwFrameType::Txt == pParent->GetType())
                {
                    bIsTextParent =true;
                }
            }
//          bool bIsVisibleChildrenOnly =aFrameOrObj.IsVisibleChildrenOnly() ;
//          bool bIsBoundAsChar =aFrameOrObj.IsBoundAsChar() ;//bIsVisibleChildrenOnly && bIsBoundAsChar &&
            if((bIsValidFrame || bIsTextParent) )
            {
                if( GetShell()->ActionPend() )
                {
                    SwAccessibleEvent_Impl aEvent(
                        SwAccessibleEvent_Impl::CHILD_POS_CHANGED,
                        pParent, aFrameOrObj, rOldBox );
                    AppendEvent( aEvent );
                }
                else
                {
                    OSL_ENSURE(false,"");
                }
            }
        }
    }
}

void SwAccessibleMap::InvalidateContent( const SwFrame *pFrame )
{
    SwAccessibleChild aFrameOrObj( pFrame );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
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
                    aFrameOrObj );
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

void SwAccessibleMap::InvalidateAttr( const SwTextFrame& rTextFrame )
{
    SwAccessibleChild aFrameOrObj( &rTextFrame );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                    mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
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
                                               pAccImpl, aFrameOrObj );
                aEvent.SetStates( AccessibleStates::TEXT_ATTRIBUTE_CHANGED );
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

void SwAccessibleMap::InvalidateCursorPosition( const SwFrame *pFrame )
{
    SwAccessibleChild aFrameOrObj( pFrame );
    bool bShapeSelected = false;
    const SwViewShell *pVSh = GetShell();
    if( dynamic_cast<const SwCursorShell*>( pVSh) !=  nullptr )
    {
        const SwCursorShell *pCSh = static_cast< const SwCursorShell * >( pVSh );
        if( pCSh->IsTableMode() )
        {
            while( aFrameOrObj.GetSwFrame() && !aFrameOrObj.GetSwFrame()->IsCellFrame() )
                aFrameOrObj = aFrameOrObj.GetSwFrame()->GetUpper();
        }
        else if( dynamic_cast<const SwFEShell*>( pVSh) !=  nullptr )
        {
            const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
            const SwFrame *pFlyFrame = pFESh->GetSelectedFlyFrame();
            if( pFlyFrame )
            {
                OSL_ENSURE( !pFrame || pFrame->FindFlyFrame() == pFlyFrame,
                        "cursor is not contained in fly frame" );
                aFrameOrObj = pFlyFrame;
            }
            else if( pFESh->IsObjSelected() > 0 )
            {
                bShapeSelected = true;
                aFrameOrObj = static_cast<const SwFrame *>( nullptr );
            }
        }
    }

    OSL_ENSURE( bShapeSelected || aFrameOrObj.IsAccessible(GetShell()->IsPreview()),
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

        if( aFrameOrObj.GetSwFrame() && mpFrameMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter =
                mpFrameMap->find( aFrameOrObj.GetSwFrame() );
            if( aIter != mpFrameMap->end() )
                xAcc = (*aIter).second;
            else
            {
                SwRect rcEmpty;
                const SwTabFrame* pTabFrame = aFrameOrObj.GetSwFrame()->FindTabFrame();
                if (pTabFrame)
                {
                    InvalidatePosOrSize(pTabFrame, nullptr, nullptr, rcEmpty);
                }
                else
                {
                    InvalidatePosOrSize(aFrameOrObj.GetSwFrame(), nullptr, nullptr, rcEmpty);
                }

                aIter = mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
                {
                    xAcc = (*aIter).second;
                }
            }

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
            // an unnecessary cursor invalidation cycle when creating
            // a new object for the current cell.
            if( aFrameOrObj.GetSwFrame()->IsCellFrame() )
            {
                if( xOldAcc.is() &&
                    AreInSameTable( xOldAcc, aFrameOrObj.GetSwFrame() ) )
                {
                    if( xAcc.is() )
                        xOldAcc = xAcc; // avoid extra invalidation
                    else
                        xAcc = xOldAcc; // make sure ate least one
                }
                if( !xAcc.is() )
                    xAcc = GetContext( aFrameOrObj.GetSwFrame() );
            }
        }
        else if (bShapeSelected)
        {
            const SwFEShell *pFESh = static_cast< const SwFEShell * >( pVSh );
            const SdrMarkList *pMarkList = pFESh->GetMarkList();
            if (pMarkList != nullptr && pMarkList->GetMarkCount() == 1)
            {
                SdrObject *pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();
                ::rtl::Reference < ::accessibility::AccessibleShape > pAccShapeImpl = GetContextImpl(pObj,nullptr,false);
                if (!pAccShapeImpl.is())
                {
                    while (pObj && pObj->GetUpGroup())
                    {
                        pObj = pObj->GetUpGroup();
                    }
                    if (pObj != nullptr)
                    {
                        const SwFrame *pParent = SwAccessibleFrame::GetParent( SwAccessibleChild(pObj), GetShell()->IsPreview() );
                        if( pParent )
                        {
                            ::rtl::Reference< SwAccessibleContext > xParentAccImpl = GetContextImpl(pParent,false);
                            if (!xParentAccImpl.is())
                            {
                                const SwTabFrame* pTabFrame = pParent->FindTabFrame();
                                if (pTabFrame)
                                {
                                    //The Table should not add in acc.because the "pParent" is not add to acc .
                                    uno::Reference< XAccessible>  xAccParentTab = GetContext(pTabFrame);//Should Create.

                                    const SwFrame *pParentRoot = SwAccessibleFrame::GetParent( SwAccessibleChild(pTabFrame), GetShell()->IsPreview() );
                                    if (pParentRoot)
                                    {
                                        ::rtl::Reference< SwAccessibleContext > xParentAccImplRoot = GetContextImpl(pParentRoot,false);
                                        if(xParentAccImplRoot.is())
                                        {
                                            AccessibleEventObject aEvent;
                                            aEvent.EventId = AccessibleEventId::CHILD;
                                            aEvent.NewValue <<= xAccParentTab;
                                            xParentAccImplRoot->FireAccessibleEvent( aEvent );
                                        }
                                    }

                                    //Get "pParent" acc again.
                                    xParentAccImpl = GetContextImpl(pParent,false);
                                }
                                else
                                {
                                    //directly create this acc para .
                                    xParentAccImpl = GetContextImpl(pParent);//Should Create.

                                    const SwFrame *pParentRoot = SwAccessibleFrame::GetParent( SwAccessibleChild(pParent), GetShell()->IsPreview() );

                                    ::rtl::Reference< SwAccessibleContext > xParentAccImplRoot = GetContextImpl(pParentRoot,false);
                                    if(xParentAccImplRoot.is())
                                    {
                                        AccessibleEventObject aEvent;
                                        aEvent.EventId = AccessibleEventId::CHILD;
                                        aEvent.NewValue <<= uno::Reference< XAccessible>(xParentAccImpl.get());
                                        xParentAccImplRoot->FireAccessibleEvent( aEvent );
                                    }
                                }
                            }
                            if (xParentAccImpl.is())
                            {
                                uno::Reference< XAccessible>  xAccShape =
                                    GetContext(pObj,xParentAccImpl.get());

                                AccessibleEventObject aEvent;
                                aEvent.EventId = AccessibleEventId::CHILD;
                                aEvent.NewValue <<= xAccShape;
                                xParentAccImpl->FireAccessibleEvent( aEvent );
                            }
                        }
                    }
                }
            }
        }
    }

    m_setParaAdd.clear();
    m_setParaRemove.clear();
    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( bOldShapeSelected || bShapeSelected )
        InvalidateShapeSelection();
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );

    InvalidateShapeInParaSelection();

    SET_PARA::iterator si = m_setParaRemove.begin();
    for (; si != m_setParaRemove.end() ; ++si)
    {
        SwAccessibleParagraph* pAccPara = *si;
        if(pAccPara && pAccPara->getSelectedAccessibleChildCount() == 0 && pAccPara->getSelectedText().getLength() == 0)
        {
            if(pAccPara->SetSelectedState(false))
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_REMOVE;
                pAccPara->FireAccessibleEvent( aEvent );
            }
        }
    }
    si = m_setParaAdd.begin();
    for (; si != m_setParaAdd.end() ; ++si)
    {
        SwAccessibleParagraph* pAccPara = *si;
        if(pAccPara && pAccPara->SetSelectedState(true))
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
            pAccPara->FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleMap::InvalidateFocus()
{
    if(GetShell()->IsPreview())
    {
        uno::Reference<XAccessible> xAcc = GetDocumentView_( true );
        if (xAcc.get())
        {
            SwAccessiblePreview *pAccPreview = static_cast<SwAccessiblePreview *>(xAcc.get());
            if (pAccPreview)
            {
                pAccPreview->InvalidateFocus();
                return ;
            }
        }
    }
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );

        xAcc = mxCursorContext;
    }

    if( xAcc.is() )
    {
        SwAccessibleContext *pAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
        pAccImpl->InvalidateFocus();
    }
    else
    {
        DoInvalidateShapeSelection(true);
    }
}

void SwAccessibleMap::SetCursorContext(
        const ::rtl::Reference < SwAccessibleContext >& rCursorContext )
{
    osl::MutexGuard aGuard( maMutex );
    uno::Reference < XAccessible > xAcc( rCursorContext.get() );
    mxCursorContext = xAcc;
}

void SwAccessibleMap::InvalidateEditableStates( const SwFrame* _pFrame )
{
    // Start with the frame or the first upper that is accessible
    SwAccessibleChild aFrameOrObj( _pFrame );
    while( aFrameOrObj.GetSwFrame() &&
            !aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
        aFrameOrObj = aFrameOrObj.GetSwFrame()->GetUpper();
    if( !aFrameOrObj.GetSwFrame() )
        aFrameOrObj = GetShell()->GetLayout();

    uno::Reference< XAccessible > xAcc( GetContext( aFrameOrObj.GetSwFrame() ) );
    SwAccessibleContext *pAccImpl = static_cast< SwAccessibleContext *>( xAcc.get() );
    if( GetShell()->ActionPend() )
    {
        SwAccessibleEvent_Impl aEvent( SwAccessibleEvent_Impl::CARET_OR_STATES,
                                       pAccImpl,
                                       SwAccessibleChild(pAccImpl->GetFrame()),
                                       AccessibleStates::EDITABLE );
        AppendEvent( aEvent );
    }
    else
    {
        FireEvents();
        pAccImpl->InvalidateStates( AccessibleStates::EDITABLE );
    }
}

void SwAccessibleMap::InvalidateRelationSet_( const SwFrame* pFrame,
                                              bool bFrom )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrameOrObj( pFrame );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
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
                                               pAccImpl, SwAccessibleChild(pFrame),
                                               ( bFrom
                                                 ? AccessibleStates::RELATION_FROM
                                                 : AccessibleStates::RELATION_TO ) );
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

void SwAccessibleMap::InvalidateRelationSet( const SwFrame* pMaster,
                                             const SwFrame* pFollow )
{
    InvalidateRelationSet_( pMaster, false );
    InvalidateRelationSet_( pFollow, true );
}

// invalidation of CONTENT_FLOW_FROM/_TO relation of a paragraph
void SwAccessibleMap::InvalidateParaFlowRelation( const SwTextFrame& _rTextFrame,
                                                  const bool _bFrom )
{
    InvalidateRelationSet_( &_rTextFrame, _bFrom );
}

// invalidation of text selection of a paragraph
void SwAccessibleMap::InvalidateParaTextSelection( const SwTextFrame& _rTextFrame )
{
    // first, see if this frame is accessible, and if so, get the respective
    SwAccessibleChild aFrameOrObj( &_rTextFrame );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
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
                    SwAccessibleChild( &_rTextFrame ),
                    AccessibleStates::TEXT_SELECTION_CHANGED );
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

sal_Int32 SwAccessibleMap::GetChildIndex( const SwFrame& rParentFrame,
                                          vcl::Window& rChild ) const
{
    sal_Int32 nIndex( -1 );

    SwAccessibleChild aFrameOrObj( &rParentFrame );
    if( aFrameOrObj.IsAccessible( GetShell()->IsPreview() ) )
    {
        uno::Reference < XAccessible > xAcc;
        {
            osl::MutexGuard aGuard( maMutex );

            if( mpFrameMap )
            {
                SwAccessibleContextMap_Impl::iterator aIter =
                                        mpFrameMap->find( aFrameOrObj.GetSwFrame() );
                if( aIter != mpFrameMap->end() )
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

void SwAccessibleMap::UpdatePreview( const std::vector<PreviewPage*>& _rPreviewPages,
                                     const Fraction&  _rScale,
                                     const SwPageFrame* _pSelectedPageFrame,
                                     const Size&      _rPreviewWinSize )
{
    assert(GetShell()->IsPreview() && "no preview?");
    assert(mpPreview != nullptr && "no preview data?");

    mpPreview->Update( *this, _rPreviewPages, _rScale, _pSelectedPageFrame, _rPreviewWinSize );

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

        const SwPageFrame *pSelPage = mpPreview->GetSelPage();
        if( pSelPage && mpFrameMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter =
                mpFrameMap->find( pSelPage );
            if( aIter != mpFrameMap->end() )
                xAcc = (*aIter).second;
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

void SwAccessibleMap::InvalidatePreviewSelection( sal_uInt16 nSelPage )
{
    OSL_ENSURE( GetShell()->IsPreview(), "no preview?" );
    OSL_ENSURE( mpPreview != nullptr, "no preview data?" );

    mpPreview->InvalidateSelection( GetShell()->GetLayout()->GetPageByPageNum( nSelPage ) );

    uno::Reference < XAccessible > xOldAcc;
    uno::Reference < XAccessible > xAcc;
    {
        osl::MutexGuard aGuard( maMutex );

        xOldAcc = mxCursorContext;

        const SwPageFrame *pSelPage = mpPreview->GetSelPage();
        if( pSelPage && mpFrameMap )
        {
            SwAccessibleContextMap_Impl::iterator aIter = mpFrameMap->find( pSelPage );
            if( aIter != mpFrameMap->end() )
                xAcc = (*aIter).second;
        }
    }

    if( xOldAcc.is() && xOldAcc != xAcc )
        InvalidateCursorPosition( xOldAcc );
    if( xAcc.is() )
        InvalidateCursorPosition( xAcc );
}

bool SwAccessibleMap::IsPageSelected( const SwPageFrame *pPageFrame ) const
{
    return mpPreview && mpPreview->GetSelPage() == pPageFrame;
}

void SwAccessibleMap::FireEvents()
{
    {
        osl::MutexGuard aGuard( maEventMutex );
        if( mpEvents )
        {
            mpEvents->SetFiring();
            mpEvents->MoveInvalidXAccToEnd();
            for( auto const& aEvent : *mpEvents )
                 FireEvent(aEvent);

            delete mpEventMap;
            mpEventMap = nullptr;

            delete mpEvents;
            mpEvents = nullptr;
        }
    }
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapes )
        {
            delete mpShapes;
            mpShapes = nullptr;
        }
    }

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
    vcl::Window *pWin = GetShell()->GetWin();
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

bool SwAccessibleMap::ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        const uno::Reference< drawing::XShape >& _rxShape,
        const long /*_nIndex*/,
        const ::accessibility::AccessibleShapeTreeInfo& /*_rShapeTreeInfo*/
    )   throw (uno::RuntimeException)
{
    const SdrObject *pObj = nullptr;
    {
        osl::MutexGuard aGuard( maMutex );
        if( mpShapeMap )
        {
            SwAccessibleShapeMap_Impl::const_iterator aIter = mpShapeMap->cbegin();
            SwAccessibleShapeMap_Impl::const_iterator aEndIter = mpShapeMap->cend();
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
        return false;

    uno::Reference < drawing::XShape > xShape( _rxShape );  // keep reference to shape, because
                                                            // we might be the only one that
                                                            // holds it.
    // Also get keep parent.
    uno::Reference < XAccessible > xParent( pCurrentChild->getAccessibleParent() );
    pCurrentChild = nullptr;  // will be released by dispose
    Dispose( nullptr, pObj, nullptr );

    {
        osl::MutexGuard aGuard( maMutex );

        if( !mpShapeMap )
            mpShapeMap = new SwAccessibleShapeMap_Impl( this );

        // create the new child
        ::accessibility::ShapeTypeHandler& rShapeTypeHandler =
                        ::accessibility::ShapeTypeHandler::Instance();
        ::accessibility::AccessibleShapeInfo aShapeInfo(
                                            xShape, xParent, this );
        rtl::Reference< ::accessibility::AccessibleShape> pReplacement(
            rShapeTypeHandler.CreateAccessibleObject (
                aShapeInfo, mpShapeMap->GetInfo() ));

        uno::Reference < XAccessible > xAcc( pReplacement.get() );
        if( xAcc.is() )
        {
            pReplacement->Init();

            SwAccessibleShapeMap_Impl::iterator aIter = mpShapeMap->find( pObj );
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
    InvalidatePosOrSize( nullptr, pObj, nullptr, aEmptyRect );

    return true;
}

//Get the accessible control shape from the model object, here model object is with XPropertySet type
::accessibility::AccessibleControlShape * SwAccessibleMap::GetAccControlShapeFromModel(css::beans::XPropertySet* pSet) throw (css::uno::RuntimeException)
{
    if( mpShapeMap )
    {
        SwAccessibleShapeMap_Impl::const_iterator aIter = mpShapeMap->cbegin();
        SwAccessibleShapeMap_Impl::const_iterator aEndIter = mpShapeMap->cend();
        while( aIter != aEndIter)
        {
            uno::Reference < XAccessible > xAcc( (*aIter).second );
            ::accessibility::AccessibleShape *pAccShape =
                static_cast < ::accessibility::AccessibleShape* >( xAcc.get() );
            if(pAccShape && ::accessibility::ShapeTypeHandler::Instance().GetTypeId (pAccShape->GetXShape()) == ::accessibility::DRAWING_CONTROL)
            {
                ::accessibility::AccessibleControlShape *pCtlAccShape = static_cast < ::accessibility::AccessibleControlShape* >(pAccShape);
                if (pCtlAccShape && pCtlAccShape->GetControlModel() == pSet)
                    return pCtlAccShape;
            }
            ++aIter;
        }
    }
    return nullptr;
}

css::uno::Reference< XAccessible >
    SwAccessibleMap::GetAccessibleCaption (const css::uno::Reference< css::drawing::XShape >&)
    throw (css::uno::RuntimeException)
{
    return nullptr;
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
    if( GetShell()->IsPreview() )
    {
        OSL_ENSURE( mpPreview != nullptr, "need preview data" );

        mpPreview->AdjustMapMode( aMapMode, _rPoint );
    }
    _orMapMode = aMapMode;
}

Size SwAccessibleMap::GetPreviewPageSize( sal_uInt16 _nPreviewPageNum ) const
{
    OSL_ENSURE( mpVSh->IsPreview(), "no page preview accessible." );
    OSL_ENSURE( mpVSh->IsPreview() && ( mpPreview != nullptr ),
                "missing accessible preview data at page preview" );
    if ( mpVSh->IsPreview() && ( mpPreview != nullptr ) )
    {
        return mpVSh->PagePreviewLayout()->GetPreviewPageSizeByPageNum( _nPreviewPageNum );
    }
    else
    {
        return Size( 0, 0 );
    }
}

/** method to build up a new data structure of the accessible paragraphs,
    which have a selection
    Important note: method has to be used inside a mutual exclusive section
*/
SwAccessibleSelectedParas_Impl* SwAccessibleMap::BuildSelectedParas()
{
    // no accessible contexts, no selection
    if ( !mpFrameMap )
    {
        return nullptr;
    }

    // get cursor as an instance of its base class <SwPaM>
    SwPaM* pCursor( nullptr );
    {
        SwCursorShell* pCursorShell = dynamic_cast<SwCursorShell*>(GetShell());
        if ( pCursorShell )
        {
            SwFEShell* pFEShell = dynamic_cast<SwFEShell*>(pCursorShell);
            if ( !pFEShell ||
                 ( !pFEShell->IsFrameSelected() &&
                   pFEShell->IsObjSelected() == 0 ) )
            {
                // get cursor without updating an existing table cursor.
                pCursor = pCursorShell->GetCursor( false );
            }
        }
    }
    // no cursor, no selection
    if ( !pCursor )
    {
        return nullptr;
    }

    SwAccessibleSelectedParas_Impl* pRetSelectedParas( nullptr );

    // loop on all cursors
    SwPaM* pRingStart = pCursor;
    do {

        // for a selection the cursor has to have a mark.
        // for safety reasons assure that point and mark are in text nodes
        if ( pCursor->HasMark() &&
             pCursor->GetPoint()->nNode.GetNode().IsTextNode() &&
             pCursor->GetMark()->nNode.GetNode().IsTextNode() )
        {
            SwPosition* pStartPos = pCursor->Start();
            SwPosition* pEndPos = pCursor->End();
            // loop on all text nodes inside the selection
            SwNodeIndex aIdx( pStartPos->nNode );
            for ( ; aIdx.GetIndex() <= pEndPos->nNode.GetIndex(); ++aIdx )
            {
                SwTextNode* pTextNode( aIdx.GetNode().GetTextNode() );
                if ( pTextNode )
                {
                    // loop on all text frames registered at the text node.
                    SwIterator<SwTextFrame,SwTextNode> aIter( *pTextNode );
                    for( SwTextFrame* pTextFrame = aIter.First(); pTextFrame; pTextFrame = aIter.Next() )
                        {
                            uno::WeakReference < XAccessible > xWeakAcc;
                            SwAccessibleContextMap_Impl::iterator aMapIter =
                                                    mpFrameMap->find( pTextFrame );
                            if( aMapIter != mpFrameMap->end() )
                            {
                                xWeakAcc = (*aMapIter).second;
                                SwAccessibleParaSelection aDataEntry(
                                    pTextNode == &(pStartPos->nNode.GetNode())
                                                ? pStartPos->nContent.GetIndex()
                                                : 0,
                                    pTextNode == &(pEndPos->nNode.GetNode())
                                                ? pEndPos->nContent.GetIndex()
                                                : -1 );
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
        pCursor = static_cast<SwPaM*>( pCursor->GetNext() );
    } while ( pCursor != pRingStart );

    return pRetSelectedParas;
}

void SwAccessibleMap::InvalidateTextSelectionOfAllParas()
{
    osl::MutexGuard aGuard( maMutex );

    // keep previously known selected paragraphs
    SwAccessibleSelectedParas_Impl* pPrevSelectedParas( mpSelectedParas );

    // determine currently selected paragraphs
    mpSelectedParas = BuildSelectedParas();

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
                    if ( xAccImpl.is() && xAccImpl->GetFrame() )
                    {
                        const SwTextFrame* pTextFrame(
                            dynamic_cast<const SwTextFrame*>(xAccImpl->GetFrame()) );
                        OSL_ENSURE( pTextFrame,
                                "<SwAccessibleMap::_SubmitTextSelectionChangedEvents()> - unexpected type of frame" );
                        if ( pTextFrame )
                        {
                            InvalidateParaTextSelection( *pTextFrame );
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
                if ( xAccImpl.is() && xAccImpl->GetFrame() )
                {
                    const SwTextFrame* pTextFrame(
                            dynamic_cast<const SwTextFrame*>(xAccImpl->GetFrame()) );
                    OSL_ENSURE( pTextFrame,
                            "<SwAccessibleMap::_SubmitTextSelectionChangedEvents()> - unexpected type of frame" );
                    if ( pTextFrame )
                    {
                        InvalidateParaTextSelection( *pTextFrame );
                    }
                }
            }
        }

        delete pPrevSelectedParas;
    }
}

const SwRect& SwAccessibleMap::GetVisArea() const
{
    OSL_ENSURE( !GetShell()->IsPreview() || (mpPreview != nullptr),
                "preview without preview data?" );

    return GetShell()->IsPreview()
           ? mpPreview->GetVisArea()
           : GetShell()->VisArea();
}

bool SwAccessibleMap::IsDocumentSelAll()
{
    return GetShell()->GetDoc()->IsPrepareSelAll();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
