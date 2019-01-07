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


// Global header


#include <memory>
#include <utility>
#include <algorithm>
#include <deque>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/textdata.hxx>
#include <vcl/unohelp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>


// Project-local header


#include "AccessibleTextEventQueue.hxx"
#include <svx/AccessibleTextHelper.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/unotext.hxx>

#include <editeng/unoedhlp.hxx>
#include <editeng/unopracc.hxx>
#include <editeng/AccessibleParaManager.hxx>
#include <editeng/AccessibleEditableTextPara.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpntv.hxx>
#include <cell.hxx>
#include "../table/accessiblecell.hxx"
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{

// AccessibleTextHelper_Impl declaration

    template < typename first_type, typename second_type >
        static ::std::pair< first_type, second_type > makeSortedPair( first_type   first,
                                                                                 second_type    second  )
    {
        if( first > second )
            return ::std::make_pair( second, first );
        else
            return ::std::make_pair( first, second );
    }

    class AccessibleTextHelper_Impl : public SfxListener
    {
    public:
        typedef ::std::vector< sal_Int16 > VectorOfStates;

        // receive pointer to our frontend class and view window
        AccessibleTextHelper_Impl();
        virtual ~AccessibleTextHelper_Impl() override;

        // XAccessibleContext child handling methods
        sal_Int32 getAccessibleChildCount();
        uno::Reference< XAccessible > getAccessibleChild( sal_Int32 i );

        // XAccessibleEventBroadcaster child related methods
        void addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener );
        void removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener );

        // XAccessibleComponent child related methods
        uno::Reference< XAccessible > getAccessibleAtPoint( const awt::Point& aPoint );

        SvxEditSourceAdapter& GetEditSource() const;

        void SetEditSource( ::std::unique_ptr< SvxEditSource > && pEditSource );

        void SetEventSource( const uno::Reference< XAccessible >& rInterface )
        {
            mxFrontEnd = rInterface;
        }

        void SetOffset( const Point& );
        Point GetOffset() const
        {
            ::osl::MutexGuard aGuard( maMutex ); Point aPoint( maOffset );
            return aPoint;
        }

        void SetStartIndex( sal_Int32 nOffset );
        sal_Int32 GetStartIndex() const
        {
            // Strictly correct only with locked solar mutex, // but
            // here we rely on the fact that sal_Int32 access is
            // atomic
            return mnStartIndex;
        }

        void SetAdditionalChildStates( const VectorOfStates& rChildStates );

        void Dispose();

        // do NOT hold object mutex when calling this! Danger of deadlock
        void FireEvent( const sal_Int16 nEventId, const uno::Any& rNewValue = uno::Any(), const uno::Any& rOldValue = uno::Any() ) const;
        void FireEvent( const AccessibleEventObject& rEvent ) const;

        void SetFocus( bool bHaveFocus );
        bool HaveFocus()
        {
            // No locking of solar mutex here, since we rely on the fact
            // that sal_Bool access is atomic
            return mbThisHasFocus;
        }
        void SetChildFocus( sal_Int32 nChild, bool bHaveFocus );
        void SetShapeFocus( bool bHaveFocus );
        void ChangeChildFocus( sal_Int32 nNewChild );

#ifdef DBG_UTIL
        void CheckInvariants() const;
#endif

        // checks all children for visibility, throws away invisible ones
        void UpdateVisibleChildren( bool bBroadcastEvents=true );

        // check all children for changes in position and size
        void UpdateBoundRect();

        // calls SetSelection on the forwarder and updates maLastSelection
        // cache.
        void UpdateSelection();

    private:

        // Process event queue
        void ProcessQueue();

        // syntactic sugar for FireEvent
        void GotPropertyEvent( const uno::Any& rNewValue, const sal_Int16 nEventId ) const { FireEvent( nEventId, rNewValue ); }

        // shutdown usage of current edit source on myself and the children.
        void ShutdownEditSource();

        void ParagraphsMoved( sal_Int32 nFirst, sal_Int32 nMiddle, sal_Int32 nLast );

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

        int getNotifierClientId() const { return mnNotifierClientId; }

        // lock solar mutex before
        SvxTextForwarder& GetTextForwarder() const;
        // lock solar mutex before
        SvxViewForwarder& GetViewForwarder() const;
        // lock solar mutex before
        SvxEditViewForwarder& GetEditViewForwarder() const;

        // are we in edit mode?
        bool IsActive() const;

        // our frontend class (the one implementing the actual
        // interface). That's not necessarily the one containing the impl
        // pointer!
        uno::Reference< XAccessible > mxFrontEnd;

        // a wrapper for the text forwarders (guarded by solar mutex)
        mutable SvxEditSourceAdapter maEditSource;

        // store last selection (to correctly report selection changes, guarded by solar mutex)
        ESelection maLastSelection;

        // cache range of visible children (guarded by solar mutex)
        sal_Int32 mnFirstVisibleChild;
        sal_Int32 mnLastVisibleChild;

        // offset to add to all our children (unguarded, relying on
        // the fact that sal_Int32 access is atomic)
        sal_Int32 mnStartIndex;

        // the object handling our children (guarded by solar mutex)
        ::accessibility::AccessibleParaManager maParaManager;

        // Queued events from Notify() (guarded by solar mutex)
        AccessibleTextEventQueue maEventQueue;

        // spin lock to prevent notify in notify (guarded by solar mutex)
        bool mbInNotify;

        // whether the object or its children has the focus set (guarded by solar mutex)
        bool mbGroupHasFocus;

        // whether we (this object) has the focus set (guarded by solar mutex)
        bool mbThisHasFocus;

        mutable ::osl::Mutex maMutex;

        /// our current offset to the containing shape/cell (guarded by maMutex)
        Point maOffset;

        /// client Id from AccessibleEventNotifier
        int mnNotifierClientId;
    };

    AccessibleTextHelper_Impl::AccessibleTextHelper_Impl() :
        maLastSelection( EE_PARA_NOT_FOUND,EE_INDEX_NOT_FOUND,EE_PARA_NOT_FOUND,EE_INDEX_NOT_FOUND ),
        mnFirstVisibleChild( -1 ),
        mnLastVisibleChild( -2 ),
        mnStartIndex( 0 ),
        mbInNotify( false ),
        mbGroupHasFocus( false ),
        mbThisHasFocus( false ),
        maOffset(0,0),
        // well, that's strictly exception safe, though not really
        // robust. We rely on the fact that this member is constructed
        // last, and that the constructor body is empty, thus no
        // chance for exceptions once the Id is fetched. Nevertheless,
        // normally should employ RAII here...
        mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient())
    {
        SAL_INFO("svx", "received ID: " << mnNotifierClientId );
    }

    AccessibleTextHelper_Impl::~AccessibleTextHelper_Impl()
    {
        SolarMutexGuard aGuard;

        try
        {
            // call Dispose here, too, since we've some resources not
            // automatically freed otherwise
            Dispose();
        }
        catch( const uno::Exception& ) {}
    }

    SvxTextForwarder& AccessibleTextHelper_Impl::GetTextForwarder() const
    {
        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxTextForwarder* pTextForwarder = maEditSource.GetTextForwarder();

        if( !pTextForwarder )
            throw uno::RuntimeException("Unable to fetch text forwarder, model might be dead", mxFrontEnd);

        if( !pTextForwarder->IsValid() )
            throw uno::RuntimeException("Text forwarder is invalid, model might be dead", mxFrontEnd);

        return *pTextForwarder;
    }

    SvxViewForwarder& AccessibleTextHelper_Impl::GetViewForwarder() const
    {
        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxViewForwarder* pViewForwarder = maEditSource.GetViewForwarder();

        if( !pViewForwarder )
            throw uno::RuntimeException("Unable to fetch view forwarder, model might be dead", mxFrontEnd);

        if( !pViewForwarder->IsValid() )
            throw uno::RuntimeException("View forwarder is invalid, model might be dead", mxFrontEnd);

        return *pViewForwarder;
    }

    SvxEditViewForwarder& AccessibleTextHelper_Impl::GetEditViewForwarder() const
    {
        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxEditViewForwarder* pViewForwarder = maEditSource.GetEditViewForwarder();

        if( !pViewForwarder )
        {
            throw uno::RuntimeException("No edit view forwarder, object not in edit mode", mxFrontEnd);
        }

        if( !pViewForwarder->IsValid() )
        {
            throw uno::RuntimeException("View forwarder is invalid, object not in edit mode", mxFrontEnd);
        }

        return *pViewForwarder;
    }

    SvxEditSourceAdapter& AccessibleTextHelper_Impl::GetEditSource() const
    {
        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::GetEditSource: no edit source", mxFrontEnd );
        return maEditSource;
    }

    // functor for sending child events (no stand-alone function, they are maybe not inlined)
    class AccessibleTextHelper_OffsetChildIndex
    {
    public:
        explicit AccessibleTextHelper_OffsetChildIndex( sal_Int32 nDifference ) : mnDifference(nDifference) {}
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.SetIndexInParent( rPara.GetIndexInParent() + mnDifference );
        }

    private:
        const sal_Int32 mnDifference;
    };

    void AccessibleTextHelper_Impl::SetStartIndex( sal_Int32 nOffset )
    {
        sal_Int32 nOldOffset( mnStartIndex );

        mnStartIndex = nOffset;

        if( nOldOffset != nOffset )
        {
            // update children
            AccessibleTextHelper_OffsetChildIndex aFunctor( nOffset - nOldOffset );

            ::std::for_each( maParaManager.begin(), maParaManager.end(),
                             AccessibleParaManager::WeakChildAdapter< AccessibleTextHelper_OffsetChildIndex > (aFunctor) );
        }
    }

    void AccessibleTextHelper_Impl::SetAdditionalChildStates( const VectorOfStates& rChildStates )
    {
        maParaManager.SetAdditionalChildStates( rChildStates );
    }

    void AccessibleTextHelper_Impl::SetChildFocus( sal_Int32 nChild, bool bHaveFocus )
    {
        if( bHaveFocus )
        {
            if( mbThisHasFocus )
                SetShapeFocus( false );

            maParaManager.SetFocus( nChild );

            // we just received the focus, also send caret event then
            UpdateSelection();

            SAL_INFO("svx", "Paragraph " << nChild << " received focus");
        }
        else
        {
            maParaManager.SetFocus( -1 );

            SAL_INFO("svx", "Paragraph " << nChild << " lost focus");

            if( mbGroupHasFocus )
                SetShapeFocus( true );
        }
    }

    void AccessibleTextHelper_Impl::ChangeChildFocus( sal_Int32 nNewChild )
    {
        if( mbThisHasFocus )
            SetShapeFocus( false );

        mbGroupHasFocus = true;
        maParaManager.SetFocus( nNewChild );

        SAL_INFO("svx", "Paragraph " << nNewChild << " received focus");
    }

    void AccessibleTextHelper_Impl::SetShapeFocus( bool bHaveFocus )
    {
        bool bOldFocus( mbThisHasFocus );

        mbThisHasFocus = bHaveFocus;

        if( bOldFocus != bHaveFocus )
        {
            if( bHaveFocus )
            {
                if( mxFrontEnd.is() )
                {
                    AccessibleCell* pAccessibleCell = dynamic_cast< AccessibleCell* > ( mxFrontEnd.get() );
                    if ( !pAccessibleCell )
                        GotPropertyEvent( uno::makeAny(AccessibleStateType::FOCUSED), AccessibleEventId::STATE_CHANGED );
                    else    // the focus event on cell should be fired on table directly
                    {
                        AccessibleTableShape* pAccTable = pAccessibleCell->GetParentTable();
                        if (pAccTable)
                            pAccTable->SetStateDirectly(AccessibleStateType::FOCUSED);
                    }
                }
                SAL_INFO("svx", "Parent object received focus" );
            }
            else
            {
                // The focus state should be reset directly on table.
                //LostPropertyEvent( uno::makeAny(AccessibleStateType::FOCUSED), AccessibleEventId::STATE_CHANGED );
                if( mxFrontEnd.is() )
                {
                    AccessibleCell* pAccessibleCell = dynamic_cast< AccessibleCell* > ( mxFrontEnd.get() );
                    if ( !pAccessibleCell )
                        FireEvent( AccessibleEventId::STATE_CHANGED, uno::Any(), uno::makeAny(AccessibleStateType::FOCUSED) );
                    else
                    {
                        AccessibleTableShape* pAccTable = pAccessibleCell->GetParentTable();
                        if (pAccTable)
                            pAccTable->ResetStateDirectly(AccessibleStateType::FOCUSED);
                    }
                }
                SAL_INFO("svx", "Parent object lost focus" );
            }
        }
    }

    void AccessibleTextHelper_Impl::SetFocus( bool bHaveFocus )
    {
        bool bOldFocus( mbGroupHasFocus );

        mbGroupHasFocus = bHaveFocus;

        if( IsActive() )
        {
            try
            {
                // find the one with the cursor and get/set focus accordingly
                ESelection aSelection;
                if( GetEditViewForwarder().GetSelection( aSelection ) )
                    SetChildFocus( aSelection.nEndPara, bHaveFocus );
            }
            catch( const uno::Exception& ) {}
        }
        else if( bOldFocus != bHaveFocus )
        {
            SetShapeFocus( bHaveFocus );
        }

        SAL_INFO("svx", "focus changed, Object " << this << ", state: " << (bHaveFocus ? "focused" : "not focused") );
    }

    bool AccessibleTextHelper_Impl::IsActive() const
    {
        try
        {
            SvxEditSource& rEditSource = GetEditSource();
            SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

            if( !pViewForwarder )
                return false;

            if( mxFrontEnd.is() )
            {
                AccessibleCell* pAccessibleCell = dynamic_cast< AccessibleCell* > ( mxFrontEnd.get() );
                if ( pAccessibleCell )
                {
                    sdr::table::CellRef xCell = pAccessibleCell->getCellRef();
                    if ( xCell.is() )
                        return xCell->IsActiveCell();
                }
            }
            return pViewForwarder->IsValid();
        }
        catch( const uno::RuntimeException& )
        {
            return false;
        }
    }

    void AccessibleTextHelper_Impl::UpdateSelection()
    {
        try
        {
            ESelection aSelection;
            if( GetEditViewForwarder().GetSelection( aSelection ) )
            {
                if( maLastSelection != aSelection &&
                    aSelection.nEndPara < maParaManager.GetNum() )
                {
                    // #103998# Not that important, changed from assertion to trace
                    if( mbThisHasFocus )
                    {
                        SAL_INFO("svx", "Parent has focus!");
                    }

                    sal_Int32 nMaxValidParaIndex( GetTextForwarder().GetParagraphCount() - 1 );

                    // notify all affected paragraphs (TODO: may be suboptimal,
                    // since some paragraphs might stay selected)
                    if( maLastSelection.nStartPara != EE_PARA_NOT_FOUND )
                    {
                        // Did the caret move from one paragraph to another?
                        // #100530# no caret events if not focused.
                        if( mbGroupHasFocus &&
                            maLastSelection.nEndPara != aSelection.nEndPara )
                        {
                            if( maLastSelection.nEndPara < maParaManager.GetNum() )
                            {
                                maParaManager.FireEvent( ::std::min( maLastSelection.nEndPara, nMaxValidParaIndex ),
                                                         ::std::min( maLastSelection.nEndPara, nMaxValidParaIndex )+1,
                                                         AccessibleEventId::CARET_CHANGED,
                                                         uno::makeAny(static_cast<sal_Int32>(-1)),
                                                         uno::makeAny(maLastSelection.nEndPos) );
                            }

                            ChangeChildFocus( aSelection.nEndPara );

                            SAL_INFO(
                                "svx",
                                "focus changed, Object: " << this
                                    << ", Paragraph: " << aSelection.nEndPara
                                    << ", Last paragraph: "
                                    << maLastSelection.nEndPara);
                        }
                    }

                    // #100530# no caret events if not focused.
                    if( mbGroupHasFocus )
                    {
                        uno::Any aOldCursor;

                        // #i13705# The old cursor can only contain valid
                        // values if it's the same paragraph!
                        if( maLastSelection.nStartPara != EE_PARA_NOT_FOUND &&
                            maLastSelection.nEndPara == aSelection.nEndPara )
                        {
                            aOldCursor <<= maLastSelection.nEndPos;
                        }
                        else
                        {
                            aOldCursor <<= static_cast<sal_Int32>(-1);
                        }

                        maParaManager.FireEvent( aSelection.nEndPara,
                                                 aSelection.nEndPara+1,
                                                 AccessibleEventId::CARET_CHANGED,
                                                 uno::makeAny(aSelection.nEndPos),
                                                 aOldCursor );
                    }

                    SAL_INFO(
                        "svx",
                        "caret changed, Object: " << this << ", New pos: "
                            << aSelection.nEndPos << ", Old pos: "
                            << maLastSelection.nEndPos << ", New para: "
                            << aSelection.nEndPara << ", Old para: "
                            << maLastSelection.nEndPara);

                    // #108947# Sort new range before calling FireEvent
                    ::std::pair<sal_Int32, sal_Int32> sortedSelection(
                        makeSortedPair(::std::min( aSelection.nStartPara, nMaxValidParaIndex ),
                                       ::std::min( aSelection.nEndPara, nMaxValidParaIndex ) ) );

                    // #108947# Sort last range before calling FireEvent
                    ::std::pair<sal_Int32, sal_Int32> sortedLastSelection(
                        makeSortedPair(::std::min( maLastSelection.nStartPara, nMaxValidParaIndex ),
                                       ::std::min( maLastSelection.nEndPara, nMaxValidParaIndex ) ) );

                    // event TEXT_SELECTION_CHANGED has to be submitted. (#i27299#)
                    const sal_Int16 nTextSelChgEventId =
                                    AccessibleEventId::TEXT_SELECTION_CHANGED;
                    // #107037# notify selection change
                    if( maLastSelection.nStartPara == EE_PARA_NOT_FOUND )
                    {
                        // last selection is undefined
                        // use method <ESelection::HasRange()> (#i27299#)
                        if ( aSelection.HasRange() )
                        {
                            // selection was undefined, now is on
                            maParaManager.FireEvent( sortedSelection.first,
                                                     sortedSelection.second+1,
                                                     nTextSelChgEventId );
                        }
                    }
                    else
                    {
                        // last selection is valid
                        // use method <ESelection::HasRange()> (#i27299#)
                        if ( maLastSelection.HasRange() &&
                             !aSelection.HasRange() )
                        {
                            // selection was on, now is empty
                            maParaManager.FireEvent( sortedLastSelection.first,
                                                     sortedLastSelection.second+1,
                                                     nTextSelChgEventId );
                        }
                        // use method <ESelection::HasRange()> (#i27299#)
                        else if( !maLastSelection.HasRange() &&
                                 aSelection.HasRange() )
                        {
                            // selection was empty, now is on
                            maParaManager.FireEvent( sortedSelection.first,
                                                     sortedSelection.second+1,
                                                     nTextSelChgEventId );
                        }
                        // no event TEXT_SELECTION_CHANGED event, if new and
                        // last selection are empty. (#i27299#)
                        else if ( maLastSelection.HasRange() &&
                                  aSelection.HasRange() )
                        {
                            // use sorted last and new selection
                            ESelection aTmpLastSel( maLastSelection );
                            aTmpLastSel.Adjust();
                            ESelection aTmpSel( aSelection );
                            aTmpSel.Adjust();
                            // first submit event for new and changed selection
                            sal_Int32 nPara = aTmpSel.nStartPara;
                            for ( ; nPara <= aTmpSel.nEndPara; ++nPara )
                            {
                                if ( nPara < aTmpLastSel.nStartPara ||
                                     nPara > aTmpLastSel.nEndPara )
                                {
                                    // new selection on paragraph <nPara>
                                    maParaManager.FireEvent( nPara,
                                                             nTextSelChgEventId );
                                }
                                else
                                {
                                    // check for changed selection on paragraph <nPara>
                                    const sal_Int32 nParaStartPos =
                                            nPara == aTmpSel.nStartPara
                                            ? aTmpSel.nStartPos : 0;
                                    const sal_Int32 nParaEndPos =
                                            nPara == aTmpSel.nEndPara
                                            ? aTmpSel.nEndPos : -1;
                                    const sal_Int32 nLastParaStartPos =
                                            nPara == aTmpLastSel.nStartPara
                                            ? aTmpLastSel.nStartPos : 0;
                                    const sal_Int32 nLastParaEndPos =
                                            nPara == aTmpLastSel.nEndPara
                                            ? aTmpLastSel.nEndPos : -1;
                                    if ( nParaStartPos != nLastParaStartPos ||
                                         nParaEndPos != nLastParaEndPos )
                                    {
                                        maParaManager.FireEvent(
                                                    nPara, nTextSelChgEventId );
                                    }
                                }
                            }
                            // second submit event for 'old' selections
                            nPara = aTmpLastSel.nStartPara;
                            for ( ; nPara <= aTmpLastSel.nEndPara; ++nPara )
                            {
                                if ( nPara < aTmpSel.nStartPara ||
                                     nPara > aTmpSel.nEndPara )
                                {
                                    maParaManager.FireEvent( nPara,
                                                             nTextSelChgEventId );
                                }
                            }
                        }
                    }

                    maLastSelection = aSelection;
                }
            }
        }
        // no selection? no update actions
        catch( const uno::RuntimeException& ) {}
    }

    void AccessibleTextHelper_Impl::ShutdownEditSource()
    {
        // This should only be called with solar mutex locked, i.e. from the main office thread

        // This here is somewhat clumsy: As soon as our children have
        // a NULL EditSource (maParaManager.SetEditSource()), they
        // enter the disposed state and cannot be reanimated. Thus, it
        // is unavoidable and a hard requirement to let go and create
        // from scratch each and every child.

        // invalidate children
        maParaManager.Dispose();
        maParaManager.SetNum(0);

        // lost all children
        if( mxFrontEnd.is() )
            FireEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN);

        // quit listen on stale edit source
        if( maEditSource.IsValid() )
            EndListening( maEditSource.GetBroadcaster() );

        maEditSource.SetEditSource( ::std::unique_ptr< SvxEditSource >() );
    }

    void AccessibleTextHelper_Impl::SetEditSource( ::std::unique_ptr< SvxEditSource > && pEditSource )
    {
        // This should only be called with solar mutex locked, i.e. from the main office thread

        // shutdown old edit source
        ShutdownEditSource();

        // set new edit source
        maEditSource.SetEditSource( std::move(pEditSource) );

        // init child vector to the current child count
        if( maEditSource.IsValid() )
        {
            maParaManager.SetNum( GetTextForwarder().GetParagraphCount() );

            // listen on new edit source
            StartListening( maEditSource.GetBroadcaster() );

            UpdateVisibleChildren();
        }
    }

    void AccessibleTextHelper_Impl::SetOffset( const Point& rPoint )
    {
        // guard against non-atomic access to maOffset data structure
        {
            ::osl::MutexGuard aGuard( maMutex );
            maOffset = rPoint;
        }

        maParaManager.SetEEOffset( rPoint );

        // in all cases, check visibility afterwards.
        UpdateVisibleChildren();
        UpdateBoundRect();
    }

    void AccessibleTextHelper_Impl::UpdateVisibleChildren( bool bBroadcastEvents )
    {
        try
        {
            SvxTextForwarder& rCacheTF = GetTextForwarder();
            sal_Int32 nParas=rCacheTF.GetParagraphCount();

            mnFirstVisibleChild = -1;
            mnLastVisibleChild = -2;

            for( sal_Int32 nCurrPara=0; nCurrPara<nParas; ++nCurrPara )
            {
                if (nCurrPara == 0)
                    mnFirstVisibleChild = nCurrPara;
                mnLastVisibleChild = nCurrPara;
                if (mxFrontEnd.is() && bBroadcastEvents)
                {
                    // child not yet created?
                    ::accessibility::AccessibleParaManager::WeakChild aChild( maParaManager.GetChild(nCurrPara) );
                    if( aChild.second.Width == 0 &&
                        aChild.second.Height == 0 )
                    {
                        GotPropertyEvent( uno::makeAny( maParaManager.CreateChild( nCurrPara - mnFirstVisibleChild,
                                                                                   mxFrontEnd, GetEditSource(), nCurrPara ).first ),
                                          AccessibleEventId::CHILD );
                    }
                }
            }
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL("AccessibleTextHelper_Impl::UpdateVisibleChildren error while determining visible children");

            // something failed - currently no children
            mnFirstVisibleChild = -1;
            mnLastVisibleChild = -2;
            maParaManager.SetNum(0);

            // lost all children
            if( bBroadcastEvents )
                FireEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN);
        }
    }

    // functor for checking changes in paragraph bounding boxes (no stand-alone function, maybe not inlined)
    class AccessibleTextHelper_UpdateChildBounds
    {
    public:
        explicit AccessibleTextHelper_UpdateChildBounds() {}
        ::accessibility::AccessibleParaManager::WeakChild operator()( const ::accessibility::AccessibleParaManager::WeakChild& rChild )
        {
            // retrieve hard reference from weak one
            auto aHardRef( rChild.first.get() );

            if( aHardRef.is() )
            {
                awt::Rectangle          aNewRect = aHardRef->getBounds();
                const awt::Rectangle&   aOldRect = rChild.second;

                if( aNewRect.X != aOldRect.X ||
                    aNewRect.Y != aOldRect.Y ||
                    aNewRect.Width != aOldRect.Width ||
                    aNewRect.Height != aOldRect.Height )
                {
                    // visible data changed
                    aHardRef->FireEvent( AccessibleEventId::BOUNDRECT_CHANGED );

                    // update internal bounds
                    return ::accessibility::AccessibleParaManager::WeakChild( rChild.first, aNewRect );
                }
            }

            // identity transform
            return rChild;
        }
    };

    void AccessibleTextHelper_Impl::UpdateBoundRect()
    {
        // send BOUNDRECT_CHANGED to affected children
        AccessibleTextHelper_UpdateChildBounds aFunctor;
        ::std::transform( maParaManager.begin(), maParaManager.end(), maParaManager.begin(), aFunctor );
    }

#ifdef DBG_UTIL
    void AccessibleTextHelper_Impl::CheckInvariants() const
    {
        if( mnFirstVisibleChild >= 0 &&
            mnFirstVisibleChild > mnLastVisibleChild )
        {
            OSL_FAIL( "AccessibleTextHelper: range invalid" );
        }
    }
#endif

    // functor for sending child events (no stand-alone function, they are maybe not inlined)
    class AccessibleTextHelper_LostChildEvent
    {
    public:
        explicit AccessibleTextHelper_LostChildEvent( AccessibleTextHelper_Impl& rImpl ) : mrImpl(rImpl) {}
        void operator()( const ::accessibility::AccessibleParaManager::WeakChild& rPara )
        {
            // retrieve hard reference from weak one
            auto aHardRef( rPara.first.get() );

            if( aHardRef.is() )
                mrImpl.FireEvent(AccessibleEventId::CHILD, uno::Any(), uno::makeAny<css::uno::Reference<css::accessibility::XAccessible>>(aHardRef.get()) );
        }

    private:
        AccessibleTextHelper_Impl&  mrImpl;
    };

    void AccessibleTextHelper_Impl::ParagraphsMoved( sal_Int32 nFirst, sal_Int32 nMiddle, sal_Int32 nLast )
    {
        const sal_Int32 nParas = GetTextForwarder().GetParagraphCount();

        /* rotate paragraphs
         * =================
         *
         * Three cases:
         *
         * 1.
         *   ... nParagraph ... nParam1 ... nParam2 ...
         *       |______________[xxxxxxxxxxx]
         *              becomes
         *       [xxxxxxxxxxx]|______________
         *
         * tail is 0
         *
         * 2.
         *   ... nParam1 ... nParagraph ... nParam2 ...
         *       [xxxxxxxxxxx|xxxxxxxxxxxxxx]____________
         *              becomes
         *       ____________[xxxxxxxxxxx|xxxxxxxxxxxxxx]
         *
         * tail is nParagraph - nParam1
         *
         * 3.
         *   ... nParam1 ... nParam2 ... nParagraph ...
         *       [xxxxxxxxxxx]___________|____________
         *              becomes
         *       ___________|____________[xxxxxxxxxxx]
         *
         * tail is nParam2 - nParam1
         */

        // sort nParagraph, nParam1 and nParam2 in ascending order, calc range
        if( nMiddle < nFirst )
        {
            ::std::swap(nFirst, nMiddle);
        }
        else if( nMiddle < nLast )
        {
            nLast = nLast + nMiddle - nFirst;
        }
        else
        {
            ::std::swap(nMiddle, nLast);
            nLast = nLast + nMiddle - nFirst;
        }

        if( nFirst < nParas && nMiddle < nParas && nLast < nParas )
        {
            // since we have no "paragraph index
            // changed" event on UAA, remove
            // [first,last] and insert again later (in
            // UpdateVisibleChildren)

            // maParaManager.Rotate( nFirst, nMiddle, nLast );

            // send CHILD_EVENT to affected children
            ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator begin = maParaManager.begin();
            ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator end = begin;

            ::std::advance( begin, nFirst );
            ::std::advance( end, nLast+1 );

            // TODO: maybe optimize here in the following way.  If the
            // number of removed children exceeds a certain threshold,
            // use InvalidateFlags::Children
            AccessibleTextHelper_LostChildEvent aFunctor( *this );

            ::std::for_each( begin, end, aFunctor );

            maParaManager.Release(nFirst, nLast+1);
            // should be no need for UpdateBoundRect, since all affected children are cleared.
        }
    }

    // functor for sending child events (no stand-alone function, they are maybe not inlined)
    class AccessibleTextHelper_ChildrenTextChanged
    {
    public:
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.TextChanged();
        }
    };

    /** functor processing queue events

        Reacts on SfxHintId::TextParaInserted/REMOVED events and stores
        their content
     */
    class AccessibleTextHelper_QueueFunctor
    {
    public:
        AccessibleTextHelper_QueueFunctor() :
            mnParasChanged( 0 ),
            mnParaIndex(-1),
            mnHintId(SfxHintId::NONE)
        {}
        void operator()( const SfxHint* pEvent )
        {
            if( pEvent &&
                mnParasChanged != -1 )
            {
                // determine hint type
                const TextHint* pTextHint = dynamic_cast<const TextHint*>( pEvent );
                const SvxEditSourceHint* pEditSourceHint = dynamic_cast<const SvxEditSourceHint*>( pEvent );

                if( !pEditSourceHint && pTextHint &&
                    (pTextHint->GetId() == SfxHintId::TextParaInserted ||
                     pTextHint->GetId() == SfxHintId::TextParaRemoved ) )
                {
                    if( pTextHint->GetValue() == EE_PARA_ALL )
                    {
                        mnParasChanged = -1;
                    }
                    else
                    {
                        mnHintId = pTextHint->GetId();
                        mnParaIndex = pTextHint->GetValue();
                        ++mnParasChanged;
                    }
                }
            }
        }

        /** Query number of paragraphs changed during queue processing.

            @return number of changed paragraphs, -1 for
            "every paragraph changed"
        */
        sal_Int32 GetNumberOfParasChanged() { return mnParasChanged; }
        /** Query index of last added/removed paragraph

            @return index of lastly added paragraphs, -1 for none
            added so far.
        */
        sal_Int32 GetParaIndex() { return mnParaIndex; }
        /** Query hint id of last interesting event

            @return hint id of last interesting event (REMOVED/INSERTED).
        */
        SfxHintId GetHintId() { return mnHintId; }

    private:
        /** number of paragraphs changed during queue processing. -1 for
            "every paragraph changed"
        */
        sal_Int32 mnParasChanged;
        /// index of paragraph added/removed last
        sal_Int32 mnParaIndex;
        /// TextHint ID (removed/inserted) of last interesting event
        SfxHintId mnHintId;
    };

    void AccessibleTextHelper_Impl::ProcessQueue()
    {
        // inspect queue for paragraph insert/remove events. If there
        // is exactly _one_ of those in the queue, and the number of
        // paragraphs has changed by exactly one, use that event to
        // determine a priori which paragraph was added/removed. This
        // is necessary, since I must sync right here with the
        // EditEngine state (number of paragraphs etc.), since I'm
        // potentially sending listener events right away.
        AccessibleTextHelper_QueueFunctor aFunctor;
        maEventQueue.ForEach( aFunctor );

        const sal_Int32 nNewParas( GetTextForwarder().GetParagraphCount() );
        const sal_Int32 nCurrParas( maParaManager.GetNum() );

        // whether every paragraph already is updated (no need to
        // repeat that later on, e.g. for PARA_MOVED events)
        bool            bEverythingUpdated( false );

        if( labs( nNewParas - nCurrParas ) == 1 &&
            aFunctor.GetNumberOfParasChanged() == 1 )
        {
            // #103483# Exactly one paragraph added/removed. This is
            // the normal case, optimize event handling here.

            if( aFunctor.GetHintId() == SfxHintId::TextParaInserted )
            {
                // update num of paras
                maParaManager.SetNum( nNewParas );

                // release everything from the insertion position until the end
                maParaManager.Release(aFunctor.GetParaIndex(), nCurrParas);

                // TODO: Clarify whether this behaviour _really_ saves
                // anybody anything!
                // update children, _don't_ broadcast
                UpdateVisibleChildren( false );
                UpdateBoundRect();

                // send insert event
                // #109864# Enforce creation of this paragraph
                try
                {
                    GotPropertyEvent( uno::makeAny( getAccessibleChild( aFunctor.GetParaIndex() -
                                                                        mnFirstVisibleChild + GetStartIndex() ) ),
                                      AccessibleEventId::CHILD );
                }
                catch( const uno::Exception& )
                {
                    OSL_FAIL("AccessibleTextHelper_Impl::ProcessQueue: could not create new paragraph");
                }
            }
            else if( aFunctor.GetHintId() == SfxHintId::TextParaRemoved )
            {
                ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator begin = maParaManager.begin();
                ::std::advance( begin, aFunctor.GetParaIndex() );
                ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator end = begin;
                ::std::advance( end, 1 );

                // #i61812# remember para to be removed for later notification
                // AFTER the new state is applied (that after the para got removed)
                ::uno::Reference< XAccessible > xPara(begin->first.get().get());

                // release everything from the remove position until the end
                maParaManager.Release(aFunctor.GetParaIndex(), nCurrParas);

                // update num of paras
                maParaManager.SetNum( nNewParas );

                // TODO: Clarify whether this behaviour _really_ saves
                // anybody anything!
                // update children, _don't_ broadcast
                UpdateVisibleChildren( false );
                UpdateBoundRect();

                // #i61812# notification for removed para
                if (xPara.is())
                    FireEvent(AccessibleEventId::CHILD, uno::Any(), uno::makeAny( xPara) );
            }
#ifdef DBG_UTIL
            else
                OSL_FAIL("AccessibleTextHelper_Impl::ProcessQueue() invalid hint id");
#endif
        }
        else if( nNewParas != nCurrParas )
        {
            // release all paras
            maParaManager.Release(0, nCurrParas);

            // update num of paras
            maParaManager.SetNum( nNewParas );

            // #109864# create from scratch, don't broadcast
            UpdateVisibleChildren( false );
            UpdateBoundRect();

            // number of paragraphs somehow changed - but we have no
            // chance determining how. Thus, throw away everything and
            // create from scratch.
            // (child events should be broadcast after the changes are done...)
            FireEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN);

            // no need for further updates later on
            bEverythingUpdated = true;
        }

        while( !maEventQueue.IsEmpty() )
        {
            ::std::unique_ptr< SfxHint > pHint( maEventQueue.PopFront() );
            if (pHint)
            {
                const SfxHint& rHint = *pHint;

                // determine hint type
                const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint );
                const TextHint* pTextHint = dynamic_cast<const TextHint*>( &rHint );
                const SvxViewChangedHint* pViewHint = dynamic_cast<const SvxViewChangedHint*>( &rHint );
                const SvxEditSourceHint* pEditSourceHint = dynamic_cast<const SvxEditSourceHint*>( &rHint );

                try
                {
                    const sal_Int32 nParas = GetTextForwarder().GetParagraphCount();

                    if( pEditSourceHint )
                    {
                        switch( pEditSourceHint->GetId() )
                        {
                            case SfxHintId::EditSourceParasMoved:
                            {
                                DBG_ASSERT( pEditSourceHint->GetStartValue() < GetTextForwarder().GetParagraphCount() &&
                                            pEditSourceHint->GetEndValue() < GetTextForwarder().GetParagraphCount(),
                                            "AccessibleTextHelper_Impl::NotifyHdl: Invalid notification");

                                if( !bEverythingUpdated )
                                {
                                    ParagraphsMoved(pEditSourceHint->GetStartValue(),
                                                    pEditSourceHint->GetValue(),
                                                    pEditSourceHint->GetEndValue());

                                    // in all cases, check visibility afterwards.
                                    UpdateVisibleChildren();
                                }
                                break;
                            }

                            case SfxHintId::EditSourceSelectionChanged:
                                // notify listeners
                                try
                                {
                                    UpdateSelection();
                                }
                                // maybe we're not in edit mode (this is not an error)
                                catch( const uno::Exception& ) {}
                                break;
                            default: break;
                        }
                    }
                    else if( pTextHint )
                    {
                        switch( pTextHint->GetId() )
                        {
                            case SfxHintId::TextModified:
                            {
                                // notify listeners
                                sal_Int32 nPara( pTextHint->GetValue() );

                                // #108900# Delegate change event to children
                                AccessibleTextHelper_ChildrenTextChanged aNotifyChildrenFunctor;

                                if( nPara == EE_PARA_ALL )
                                {
                                    // #108900# Call every child
                                    ::std::for_each( maParaManager.begin(), maParaManager.end(),
                                                     AccessibleParaManager::WeakChildAdapter< AccessibleTextHelper_ChildrenTextChanged > (aNotifyChildrenFunctor) );
                                }
                                else
                                    if( nPara < nParas )
                                    {
                                        // #108900# Call child at index nPara
                                        ::std::for_each( maParaManager.begin()+nPara, maParaManager.begin()+nPara+1,
                                                         AccessibleParaManager::WeakChildAdapter< AccessibleTextHelper_ChildrenTextChanged > (aNotifyChildrenFunctor) );
                                    }
                                break;
                            }

                            case SfxHintId::TextParaInserted:
                                // already happened above
                                break;

                            case SfxHintId::TextParaRemoved:
                                // already happened above
                                break;

                            case SfxHintId::TextHeightChanged:
                                // visibility changed, done below
                                break;

                            case SfxHintId::TextViewScrolled:
                                // visibility changed, done below
                                break;
                            default: break;
                        }

                        // in all cases, check visibility afterwards.
                        UpdateVisibleChildren();
                        UpdateBoundRect();
                    }
                    else if( pViewHint )
                    {
                        // just check visibility
                        UpdateVisibleChildren();
                        UpdateBoundRect();
                    }
                    else if( pSdrHint )
                    {
                        switch( pSdrHint->GetKind() )
                        {
                            case SdrHintKind::BeginEdit:
                            {
                                if(!IsActive())
                                {
                                    break;
                                }
                                // change children state
                                maParaManager.SetActive();

                                // per definition, edit mode text has the focus
                                SetFocus( true );
                                break;
                            }

                            case SdrHintKind::EndEdit:
                            {
                                // focused child now loses focus
                                ESelection aSelection;
                                if( GetEditViewForwarder().GetSelection( aSelection ) )
                                    SetChildFocus( aSelection.nEndPara, false );

                                // change children state
                                maParaManager.SetActive( false );

                                maLastSelection = ESelection( EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND,
                                                              EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    // it's VITAL to keep the SfxSimpleHint last! It's the base of some classes above!
                    else if( rHint.GetId() == SfxHintId::Dying)
                    {
                        // edit source is dying under us, become defunc then
                        try
                        {
                            // make edit source inaccessible
                            // Note: cannot destroy it here, since we're called from there!
                            ShutdownEditSource();
                        }
                        catch( const uno::Exception& ) {}
                    }
                }
                catch( const uno::Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("svx");
                }
            }
        }
    }

    void AccessibleTextHelper_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        // precondition: not in a recursion
        if( mbInNotify )
            return;

        mbInNotify = true;

        try
        {
            // Process notification event, arranged in order of likelihood of
            // occurrence to avoid unnecessary dynamic_cast. Note that
            // SvxEditSourceHint is derived from TextHint, so has to be checked
            // before that.
            if( const SvxViewChangedHint* pViewHint = dynamic_cast<const SvxViewChangedHint*>( &rHint ) )
            {
                // process visibility right away, if not within an
                // open EE notification frame. Otherwise, event
                // processing would be delayed until next EE
                // notification sequence.
                maEventQueue.Append( *pViewHint );
            }
            else if( const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint ) )
            {
                // process drawing layer events right away, if not
                // within an open EE notification frame. Otherwise,
                // event processing would be delayed until next EE
                // notification sequence.
                maEventQueue.Append( *pSdrHint );
            }
            else if( const SvxEditSourceHint* pEditSourceHint = dynamic_cast<const SvxEditSourceHint*>( &rHint ) )
            {
                // EditEngine should emit TEXT_SELECTION_CHANGED events (#i27299#)
                maEventQueue.Append( *pEditSourceHint );
            }
            else if( const TextHint* pTextHint = dynamic_cast<const TextHint*>( &rHint ) )
            {
                // EditEngine should emit TEXT_SELECTION_CHANGED events (#i27299#)
                if(pTextHint->GetId() == SfxHintId::TextProcessNotifications)
                    ProcessQueue();
                else
                    maEventQueue.Append( *pTextHint );
            }
            // it's VITAL to keep the SfxHint last! It's the base of the classes above!
            else if( rHint.GetId() == SfxHintId::Dying )
            {
                // handle this event _at once_, because after that, objects are invalid
                // edit source is dying under us, become defunc then
                maEventQueue.Clear();
                try
                {
                    // make edit source inaccessible
                    // Note: cannot destroy it here, since we're called from there!
                    ShutdownEditSource();
                }
                catch( const uno::Exception& ) {}
            }
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
            mbInNotify = false;
        }

        mbInNotify = false;
    }

    void AccessibleTextHelper_Impl::Dispose()
    {
        if( getNotifierClientId() != -1 )
        {
            try
            {
                // #106234# Unregister from EventNotifier
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
                SAL_INFO("svx", "disposed ID: " << mnNotifierClientId );
            }
            catch( const uno::Exception& ) {}

            mnNotifierClientId = -1;
        }

        try
        {
            // dispose children
            maParaManager.Dispose();
        }
        catch( const uno::Exception& ) {}

        // quit listen on stale edit source
        if( maEditSource.IsValid() )
            EndListening( maEditSource.GetBroadcaster() );

        // clear references
        maEditSource.SetEditSource( ::std::unique_ptr< SvxEditSource >() );
        mxFrontEnd = nullptr;
    }

    void AccessibleTextHelper_Impl::FireEvent( const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue ) const
    {
        // -- object locked --
        ::osl::ClearableMutexGuard aGuard( maMutex );

        AccessibleEventObject aEvent;

        DBG_ASSERT(mxFrontEnd.is(), "AccessibleTextHelper::FireEvent: no event source set" );

        if( mxFrontEnd.is() )
            aEvent = AccessibleEventObject(mxFrontEnd->getAccessibleContext(), nEventId, rNewValue, rOldValue);
        else
            aEvent = AccessibleEventObject(uno::Reference< uno::XInterface >(), nEventId, rNewValue, rOldValue);

        // no locking necessary, FireEvent internally copies listeners
        // if someone removes/adds in between Further locking,
        // actually, might lead to deadlocks, since we're calling out
        // of this object
        aGuard.clear();
        // -- until here --

        FireEvent(aEvent);
    }

    void AccessibleTextHelper_Impl::FireEvent( const AccessibleEventObject& rEvent ) const
    {
        // #102261# Call global queue for focus events
        if( rEvent.EventId == AccessibleStateType::FOCUSED )
            vcl::unohelper::NotifyAccessibleStateEventGlobally( rEvent );

        // #106234# Delegate to EventNotifier
        ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                         rEvent );
    }

    // XAccessibleContext
    sal_Int32 AccessibleTextHelper_Impl::getAccessibleChildCount()
    {
        return mnLastVisibleChild - mnFirstVisibleChild + 1;
    }

    uno::Reference< XAccessible > AccessibleTextHelper_Impl::getAccessibleChild( sal_Int32 i )
    {
        i -= GetStartIndex();

        if( 0 > i || i >= getAccessibleChildCount() ||
            GetTextForwarder().GetParagraphCount() <= i )
        {
            throw lang::IndexOutOfBoundsException("Invalid child index", mxFrontEnd);
        }

        DBG_ASSERT(mxFrontEnd.is(), "AccessibleTextHelper_Impl::UpdateVisibleChildren: no frontend set");

        if( mxFrontEnd.is() )
            return maParaManager.CreateChild( i, mxFrontEnd, GetEditSource(), mnFirstVisibleChild + i ).first;
        else
            return nullptr;
    }

    void AccessibleTextHelper_Impl::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {
        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void AccessibleTextHelper_Impl::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {
        if( getNotifierClientId() != -1 )
        {
            const sal_Int32 nListenerCount = ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
            if ( !nListenerCount )
            {
                // no listeners anymore
                // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case somebody calls
                // NotifyAccessibleEvent, again
                ::comphelper::AccessibleEventNotifier::TClientId nId( getNotifierClientId() );
                mnNotifierClientId = -1;
                ::comphelper::AccessibleEventNotifier::revokeClient( nId );
            }
        }
    }

    uno::Reference< XAccessible > AccessibleTextHelper_Impl::getAccessibleAtPoint( const awt::Point& _aPoint )
    {
        // make given position relative
        if( !mxFrontEnd.is() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::getAccessibleAt: frontend invalid", mxFrontEnd );

        uno::Reference< XAccessibleContext > xFrontEndContext = mxFrontEnd->getAccessibleContext();

        if( !xFrontEndContext.is() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::getAccessibleAt: frontend invalid", mxFrontEnd );

        uno::Reference< XAccessibleComponent > xFrontEndComponent( xFrontEndContext, uno::UNO_QUERY_THROW );

        // #103862# No longer need to make given position relative
        Point aPoint( _aPoint.X, _aPoint.Y );

        // respect EditEngine offset to surrounding shape/cell
        aPoint -= GetOffset();

        // convert to EditEngine coordinate system
        SvxTextForwarder& rCacheTF = GetTextForwarder();
        Point aLogPoint( GetViewForwarder().PixelToLogic( aPoint, rCacheTF.GetMapMode() ) );

        // iterate over all visible children (including those not yet created)
        sal_Int32 nChild;
        for( nChild=mnFirstVisibleChild; nChild <= mnLastVisibleChild; ++nChild )
        {
            DBG_ASSERT(nChild >= 0,
                       "AccessibleTextHelper_Impl::getAccessibleAt: index value overflow");

            tools::Rectangle aParaBounds( rCacheTF.GetParaBounds( nChild ) );

            if( aParaBounds.IsInside( aLogPoint ) )
                return getAccessibleChild( nChild - mnFirstVisibleChild + GetStartIndex() );
        }

        // found none
        return nullptr;
    }


    // AccessibleTextHelper implementation (simply forwards to impl)

    AccessibleTextHelper::AccessibleTextHelper( ::std::unique_ptr< SvxEditSource > && pEditSource ) :
        mpImpl( new AccessibleTextHelper_Impl() )
    {
        SolarMutexGuard aGuard;

        SetEditSource( std::move(pEditSource) );
    }

    AccessibleTextHelper::~AccessibleTextHelper()
    {
    }

    const SvxEditSource& AccessibleTextHelper::GetEditSource() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        const SvxEditSource& aEditSource = mpImpl->GetEditSource();

        mpImpl->CheckInvariants();

        return aEditSource;
#else
        return mpImpl->GetEditSource();
#endif
    }

    void AccessibleTextHelper::SetEditSource( ::std::unique_ptr< SvxEditSource > && pEditSource )
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->SetEditSource( std::move(pEditSource) );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    void AccessibleTextHelper::SetEventSource( const uno::Reference< XAccessible >& rInterface )
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->SetEventSource( rInterface );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    void AccessibleTextHelper::SetFocus( bool bHaveFocus )
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->SetFocus( bHaveFocus );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    bool AccessibleTextHelper::HaveFocus()
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        bool bRet( mpImpl->HaveFocus() );

        mpImpl->CheckInvariants();

        return bRet;
#else
        return mpImpl->HaveFocus();
#endif
    }

    void AccessibleTextHelper::SetOffset( const Point& rPoint )
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->SetOffset( rPoint );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    void AccessibleTextHelper::SetStartIndex( sal_Int32 nOffset )
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->SetStartIndex( nOffset );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    sal_Int32 AccessibleTextHelper::GetStartIndex() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        sal_Int32 nOffset = mpImpl->GetStartIndex();

        mpImpl->CheckInvariants();

        return nOffset;
#else
        return mpImpl->GetStartIndex();
#endif
    }

    void AccessibleTextHelper::SetAdditionalChildStates( const VectorOfStates& rChildStates )
    {
        mpImpl->SetAdditionalChildStates( rChildStates );
    }

    void AccessibleTextHelper::UpdateChildren()
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->UpdateVisibleChildren();
        mpImpl->UpdateBoundRect();

        mpImpl->UpdateSelection();

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    void AccessibleTextHelper::Dispose()
    {
        // As Dispose calls ShutdownEditSource, which in turn
        // deregisters as listener on the edit source, have to lock
        // here
        SolarMutexGuard aGuard;

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->Dispose();

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    // XAccessibleContext
    sal_Int32 AccessibleTextHelper::GetChildCount()
    {
        SolarMutexGuard aGuard;

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        sal_Int32 nRet = mpImpl->getAccessibleChildCount();

        mpImpl->CheckInvariants();

        return nRet;
#else
        return mpImpl->getAccessibleChildCount();
#endif
    }

    uno::Reference< XAccessible > AccessibleTextHelper::GetChild( sal_Int32 i )
    {
        SolarMutexGuard aGuard;

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        uno::Reference< XAccessible > xRet = mpImpl->getAccessibleChild( i );

        mpImpl->CheckInvariants();

        return xRet;
#else
        return mpImpl->getAccessibleChild( i );
#endif
    }

    void AccessibleTextHelper::AddEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        mpImpl->addAccessibleEventListener( xListener );

        mpImpl->CheckInvariants();
#else
        mpImpl->addAccessibleEventListener( xListener );
#endif
    }

    void AccessibleTextHelper::RemoveEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        mpImpl->removeAccessibleEventListener( xListener );

        mpImpl->CheckInvariants();
#else
        mpImpl->removeAccessibleEventListener( xListener );
#endif
    }

    // XAccessibleComponent
    uno::Reference< XAccessible > AccessibleTextHelper::GetAt( const awt::Point& aPoint )
    {
        SolarMutexGuard aGuard;

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        uno::Reference< XAccessible > xChild = mpImpl->getAccessibleAtPoint( aPoint );

        mpImpl->CheckInvariants();

        return xChild;
#else
        return mpImpl->getAccessibleAtPoint( aPoint );
#endif
    }

} // end of namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
