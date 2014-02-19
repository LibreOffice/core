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


//------------------------------------------------------------------------
//
// Global header
//
//------------------------------------------------------------------------

#include <limits.h>
#include <memory>
#include <algorithm>
#include <deque>
#include <osl/mutex.hxx>
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

//------------------------------------------------------------------------
//
// Project-local header
//
//------------------------------------------------------------------------
#include "AccessibleTextEventQueue.hxx"
#include <svx/AccessibleTextHelper.hxx>
#include <svx/unoshape.hxx>
#include "editeng/unolingu.hxx"
#include <editeng/unotext.hxx>

#include "editeng/unoedhlp.hxx"
#include "editeng/unopracc.hxx"
#include "editeng/AccessibleParaManager.hxx"
#include "editeng/AccessibleEditableTextPara.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdpntv.hxx>
#include "../table/cell.hxx"
#include "../table/accessiblecell.hxx"
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{

//------------------------------------------------------------------------
//
// AccessibleTextHelper_Impl declaration
//
//------------------------------------------------------------------------

    DBG_NAME( AccessibleTextHelper_Impl )

    template < typename first_type, typename second_type >
        ::std::pair< first_type, second_type > makeSortedPair( first_type   first,
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
        ~AccessibleTextHelper_Impl();

        // XAccessibleContext child handling methods
        sal_Int32 SAL_CALL getAccessibleChildCount() SAL_THROW((uno::RuntimeException));
        uno::Reference< XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException));

        // XAccessibleEventBroadcaster child related methods
        void SAL_CALL addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException));
        void SAL_CALL removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException));

        // XAccessibleComponent child related methods
        uno::Reference< XAccessible > SAL_CALL getAccessibleAtPoint( const awt::Point& aPoint ) SAL_THROW((uno::RuntimeException));

        SvxEditSourceAdapter& GetEditSource() const SAL_THROW((uno::RuntimeException));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        void SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException));
        SAL_WNODEPRECATED_DECLARATIONS_POP

        void SetEventSource( const uno::Reference< XAccessible >& rInterface )
        {
            DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );
            mxFrontEnd = rInterface;
        }
        uno::Reference< XAccessible > GetEventSource() const
        {
            DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );
            return mxFrontEnd;
        }

        void SetOffset( const Point& );
        Point GetOffset() const
        {
            DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );
            ::osl::MutexGuard aGuard( maMutex ); Point aPoint( maOffset );
            return aPoint;
        }

        void SetStartIndex( sal_Int32 nOffset );
        sal_Int32 GetStartIndex() const
        {
            DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );
            // Strictly correct only with locked solar mutex, // but
            // here we rely on the fact that sal_Int32 access is
            // atomic
            return mnStartIndex;
        }

        void SetAdditionalChildStates( const VectorOfStates& rChildStates );

        sal_Bool IsSelected() const;

        void Dispose();

        // do NOT hold object mutex when calling this! Danger of deadlock
        void FireEvent( const sal_Int16 nEventId, const uno::Any& rNewValue = uno::Any(), const uno::Any& rOldValue = uno::Any() ) const;
        void FireEvent( const AccessibleEventObject& rEvent ) const;

        void SetFocus( sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        sal_Bool HaveFocus() SAL_THROW((::com::sun::star::uno::RuntimeException));
        void SetChildFocus( sal_Int32 nChild, sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        void SetShapeFocus( sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException));
        void ChangeChildFocus( sal_Int32 nNewChild ) SAL_THROW((::com::sun::star::uno::RuntimeException));

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
        void LostPropertyEvent( const uno::Any& rOldValue, const sal_Int16 nEventId ) const { FireEvent( nEventId, uno::Any(), rOldValue ); }

        // shutdown usage of current edit source on myself and the children.
        void ShutdownEditSource() SAL_THROW((uno::RuntimeException));

        void ParagraphsMoved( sal_Int32 nFirst, sal_Int32 nMiddle, sal_Int32 nLast );

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

        int getNotifierClientId() const { return mnNotifierClientId; }

        // lock solar mutex before
        SvxTextForwarder& GetTextForwarder() const SAL_THROW((uno::RuntimeException));
        // lock solar mutex before
        SvxViewForwarder& GetViewForwarder() const SAL_THROW((uno::RuntimeException));
        // lock solar mutex before
        SvxEditViewForwarder& GetEditViewForwarder( sal_Bool bCreate = sal_False ) const SAL_THROW((uno::RuntimeException));

        // are we in edit mode?
        sal_Bool IsActive() const SAL_THROW((uno::RuntimeException));

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

        // number of not-yet-closed event frames (BEGIN/END sequences) (guarded by solar mutex)
        sal_Int32 maEventOpenFrames;

        // Queued events from Notify() (guarded by solar mutex)
        AccessibleTextEventQueue maEventQueue;

        // spin lock to prevent notify in notify (guarded by solar mutex)
        bool mbInNotify;

        // whether the object or it's children has the focus set (guarded by solar mutex)
        sal_Bool mbGroupHasFocus;

        // whether we (this object) has the focus set (guarded by solar mutex)
        sal_Bool mbThisHasFocus;

        mutable ::osl::Mutex maMutex;

        /// our current offset to the containing shape/cell (guarded by maMutex)
        Point maOffset;

        /// client Id from AccessibleEventNotifier
        int mnNotifierClientId;
    };

    //------------------------------------------------------------------------
    //
    // AccessibleTextHelper_Impl implementation
    //
    //------------------------------------------------------------------------

    AccessibleTextHelper_Impl::AccessibleTextHelper_Impl() :
        mxFrontEnd( NULL ),
        maLastSelection( EE_PARA_NOT_FOUND,EE_INDEX_NOT_FOUND,EE_PARA_NOT_FOUND,EE_INDEX_NOT_FOUND ),
        mnFirstVisibleChild( -1 ),
        mnLastVisibleChild( -2 ),
        mnStartIndex( 0 ),
        maEventOpenFrames( 0 ),
        mbInNotify( false ),
        mbGroupHasFocus( sal_False ),
        mbThisHasFocus( sal_False ),
        maOffset(0,0),
        // well, that's strictly exception safe, though not really
        // robust. We rely on the fact that this member is constructed
        // last, and that the constructor body is empty, thus no
        // chance for exceptions once the Id is fetched. Nevertheless,
        // normally should employ RAII here...
        mnNotifierClientId(::comphelper::AccessibleEventNotifier::registerClient())
    {
        DBG_CTOR( AccessibleTextHelper_Impl, NULL );

#ifdef DBG_UTIL
        OSL_TRACE( "AccessibleTextHelper_Impl received ID: %d", mnNotifierClientId );
#endif
    }

    AccessibleTextHelper_Impl::~AccessibleTextHelper_Impl()
    {
        DBG_DTOR( AccessibleTextHelper_Impl, NULL );

        SolarMutexGuard aGuard;

        try
        {
            // call Dispose here, too, since we've some resources not
            // automatically freed otherwise
            Dispose();
        }
        catch( const uno::Exception& ) {}
    }

    SvxTextForwarder& AccessibleTextHelper_Impl::GetTextForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxTextForwarder* pTextForwarder = maEditSource.GetTextForwarder();

        if( !pTextForwarder )
            throw uno::RuntimeException("Unable to fetch text forwarder, model might be dead", mxFrontEnd);

        if( pTextForwarder->IsValid() )
            return *pTextForwarder;
        else
            throw uno::RuntimeException("Text forwarder is invalid, model might be dead", mxFrontEnd);
    }

    SvxViewForwarder& AccessibleTextHelper_Impl::GetViewForwarder() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxViewForwarder* pViewForwarder = maEditSource.GetViewForwarder();

        if( !pViewForwarder )
            throw uno::RuntimeException("Unable to fetch view forwarder, model might be dead", mxFrontEnd);

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
            throw uno::RuntimeException("View forwarder is invalid, model might be dead", mxFrontEnd);
    }

    SvxEditViewForwarder& AccessibleTextHelper_Impl::GetEditViewForwarder( sal_Bool bCreate ) const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( !maEditSource.IsValid() )
            throw uno::RuntimeException("Unknown edit source", mxFrontEnd);

        SvxEditViewForwarder* pViewForwarder = maEditSource.GetEditViewForwarder( bCreate );

        if( !pViewForwarder )
        {
            if( bCreate )
                throw uno::RuntimeException("Unable to fetch edit view forwarder, model might be dead", mxFrontEnd);
            else
                throw uno::RuntimeException("No edit view forwarder, object not in edit mode", mxFrontEnd);
        }

        if( pViewForwarder->IsValid() )
            return *pViewForwarder;
        else
        {
            if( bCreate )
                throw uno::RuntimeException("View forwarder is invalid, model might be dead", mxFrontEnd);
            else
                throw uno::RuntimeException("View forwarder is invalid, object not in edit mode", mxFrontEnd);
        }
    }

    SvxEditSourceAdapter& AccessibleTextHelper_Impl::GetEditSource() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( maEditSource.IsValid() )
            return maEditSource;
        else
            throw uno::RuntimeException("AccessibleTextHelper_Impl::GetEditSource: no edit source", mxFrontEnd );
    }

    sal_Bool AccessibleTextHelper_Impl::IsSelected() const
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        sal_Bool bRet = sal_False;

        try
        {
            ESelection aSelection;
            bRet = GetEditViewForwarder().GetSelection( aSelection );
        }
        catch( const uno::Exception& ) {}

        return bRet;
    }

    // functor for sending child events (no stand-alone function, they are maybe not inlined)
    class AccessibleTextHelper_OffsetChildIndex : public ::std::unary_function< ::accessibility::AccessibleEditableTextPara&, void >
    {
    public:
        AccessibleTextHelper_OffsetChildIndex( sal_Int32 nDifference ) : mnDifference(nDifference) {}
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.SetIndexInParent( rPara.GetIndexInParent() + mnDifference );
        }

    private:
        const sal_Int32 mnDifference;
    };

    void AccessibleTextHelper_Impl::SetStartIndex( sal_Int32 nOffset )
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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

    void AccessibleTextHelper_Impl::SetChildFocus( sal_Int32 nChild, sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( bHaveFocus )
        {
            if( mbThisHasFocus )
                SetShapeFocus( sal_False );

            maParaManager.SetFocus( nChild );

            // we just received the focus, also send caret event then
            UpdateSelection();

            OSL_TRACE("AccessibleTextHelper_Impl::SetChildFocus(): Paragraph %d received focus", nChild );
        }
        else
        {
            maParaManager.SetFocus( -1 );

            OSL_TRACE("AccessibleTextHelper_Impl::SetChildFocus(): Paragraph %d lost focus", nChild );

            if( mbGroupHasFocus )
                SetShapeFocus( sal_True );
        }
    }

    void AccessibleTextHelper_Impl::ChangeChildFocus( sal_Int32 nNewChild ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( mbThisHasFocus )
            SetShapeFocus( sal_False );

        mbGroupHasFocus = sal_True;
        maParaManager.SetFocus( nNewChild );

        OSL_TRACE("AccessibleTextHelper_Impl::ChangeChildFocus(): Paragraph %d received focus", nNewChild );
    }

    void AccessibleTextHelper_Impl::SetShapeFocus( sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        sal_Bool bOldFocus( mbThisHasFocus );

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
                OSL_TRACE("AccessibleTextHelper_Impl::SetShapeFocus(): Parent object received focus" );
            }
            else
            {
                // The focus state should be reset directly on table.
                //LostPropertyEvent( uno::makeAny(AccessibleStateType::FOCUSED), AccessibleEventId::STATE_CHANGED );
                if( mxFrontEnd.is() )
                {
                    AccessibleCell* pAccessibleCell = dynamic_cast< AccessibleCell* > ( mxFrontEnd.get() );
                    if ( !pAccessibleCell )
                            LostPropertyEvent( uno::makeAny(AccessibleStateType::FOCUSED), AccessibleEventId::STATE_CHANGED );
                    else
                    {
                        AccessibleTableShape* pAccTable = pAccessibleCell->GetParentTable();
                        if (pAccTable)
                            pAccTable->ResetStateDirectly(AccessibleStateType::FOCUSED);
                    }
                }
                OSL_TRACE("AccessibleTextHelper_Impl::SetShapeFocus(): Parent object lost focus" );
            }
        }
    }

    void AccessibleTextHelper_Impl::SetFocus( sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        sal_Bool bOldFocus( mbGroupHasFocus );

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

        OSL_TRACE("AccessibleTextHelper_Impl::SetFocus: focus changed, Object %p, state: %s", this, bHaveFocus ? "focused" : "not focused");
    }

    sal_Bool AccessibleTextHelper_Impl::HaveFocus() SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // No locking of solar mutex here, since we rely on the fact
        // that sal_Bool access is atomic
        return mbThisHasFocus;
    }

    sal_Bool AccessibleTextHelper_Impl::IsActive() const SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        try
        {
            SvxEditSource& rEditSource = GetEditSource();
            SvxEditViewForwarder* pViewForwarder = rEditSource.GetEditViewForwarder();

            if( !pViewForwarder )
                return sal_False;

            if( mxFrontEnd.is() )
            {
                AccessibleCell* pAccessibleCell = dynamic_cast< AccessibleCell* > ( mxFrontEnd.get() );
                if ( pAccessibleCell )
                {
                    sdr::table::CellRef xCell = pAccessibleCell->getCellRef();
                    if ( xCell.is() )
                        return xCell->IsTextEditActive();
                }
            }
            if( pViewForwarder->IsValid() )
                return sal_True;
            else
                return sal_False;
        }
        catch( const uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    void AccessibleTextHelper_Impl::UpdateSelection()
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        try
        {
            ESelection aSelection;
            if( GetEditViewForwarder().GetSelection( aSelection ) )
            {
                if( !maLastSelection.IsEqual( aSelection ) &&
                    aSelection.nEndPara < maParaManager.GetNum() )
                {
                    // #103998# Not that important, changed from assertion to trace
                    if( mbThisHasFocus )
                    {
                        OSL_TRACE("AccessibleTextHelper_Impl::UpdateSelection(): Parent has focus!");
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
                                                         uno::makeAny(static_cast<sal_Int32>(maLastSelection.nEndPos)) );
                            }

                            ChangeChildFocus( aSelection.nEndPara );

                            OSL_TRACE("AccessibleTextHelper_Impl::UpdateSelection(): focus changed, Object: %p, Paragraph: %d, Last paragraph: %d",
                                       this, aSelection.nEndPara, maLastSelection.nEndPara);
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
                            aOldCursor <<= static_cast<sal_Int32>(maLastSelection.nEndPos);
                        }
                        else
                        {
                            aOldCursor <<= static_cast<sal_Int32>(-1);
                        }

                        maParaManager.FireEvent( aSelection.nEndPara,
                                                 aSelection.nEndPara+1,
                                                 AccessibleEventId::CARET_CHANGED,
                                                 uno::makeAny(static_cast<sal_Int32>(aSelection.nEndPos)),
                                                 aOldCursor );
                    }

                    OSL_TRACE("AccessibleTextHelper_Impl::UpdateSelection(): caret changed, Object: %p, New pos: %d, Old pos: %d, New para: %d, Old para: %d",
                               this, aSelection.nEndPos, maLastSelection.nEndPos, aSelection.nEndPara, maLastSelection.nEndPara);

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

    void AccessibleTextHelper_Impl::ShutdownEditSource() SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        maEditSource.SetEditSource( ::std::auto_ptr< SvxEditSource >(NULL) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void AccessibleTextHelper_Impl::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // This should only be called with solar mutex locked, i.e. from the main office thread

        // shutdown old edit source
        ShutdownEditSource();

        // set new edit source
        maEditSource.SetEditSource( pEditSource );

        // init child vector to the current child count
        if( maEditSource.IsValid() )
        {
            maParaManager.SetNum( GetTextForwarder().GetParagraphCount() );

            // listen on new edit source
            StartListening( maEditSource.GetBroadcaster() );

            UpdateVisibleChildren();
        }
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void AccessibleTextHelper_Impl::SetOffset( const Point& rPoint )
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        try
        {
            SvxTextForwarder& rCacheTF = GetTextForwarder();
            SvxViewForwarder& rCacheVF = GetViewForwarder();

            Rectangle aViewArea = rCacheVF.GetVisArea();

            if( IsActive() )
            {
                // maybe the edit view scrolls, adapt aViewArea
                Rectangle aEditViewArea = GetEditViewForwarder().GetVisArea();
                aViewArea += aEditViewArea.TopLeft();

                // now determine intersection
                aViewArea.Intersection( aEditViewArea );
            }

            Rectangle aTmpBB, aParaBB;
            sal_Bool bFirstChild = sal_True;
            sal_Int32 nCurrPara;
            sal_Int32 nParas=rCacheTF.GetParagraphCount();

            mnFirstVisibleChild = -1;
            mnLastVisibleChild = -2;

            for( nCurrPara=0; nCurrPara<nParas; ++nCurrPara )
            {
                DBG_ASSERT(nCurrPara >= 0 && nCurrPara <= USHRT_MAX,
                           "AccessibleTextHelper_Impl::UpdateVisibleChildren: index value overflow");

                aTmpBB = rCacheTF.GetParaBounds( nCurrPara );

                // convert to screen coordinates
                aParaBB = ::accessibility::AccessibleEditableTextPara::LogicToPixel( aTmpBB, rCacheTF.GetMapMode(), rCacheVF );
                // at least partially visible
                if( bFirstChild )
                {
                    bFirstChild = sal_False;
                    mnFirstVisibleChild = nCurrPara;
                }

                mnLastVisibleChild = nCurrPara;

                // child not yet created?
                ::accessibility::AccessibleParaManager::WeakChild aChild( maParaManager.GetChild(nCurrPara) );
                if( aChild.second.Width == 0 &&
                    aChild.second.Height == 0 &&
                    mxFrontEnd.is() &&
                    bBroadcastEvents )
                {
                    GotPropertyEvent( uno::makeAny( maParaManager.CreateChild( nCurrPara - mnFirstVisibleChild,
                                                                               mxFrontEnd, GetEditSource(), nCurrPara ).first ),
                                      AccessibleEventId::CHILD );
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
    class AccessibleTextHelper_UpdateChildBounds : public ::std::unary_function< const ::accessibility::AccessibleParaManager::WeakChild&,
        ::accessibility::AccessibleParaManager::WeakChild >
    {
    public:
        AccessibleTextHelper_UpdateChildBounds( AccessibleTextHelper_Impl& rImpl ) : mrImpl(rImpl) {}
        ::accessibility::AccessibleParaManager::WeakChild operator()( const ::accessibility::AccessibleParaManager::WeakChild& rChild )
        {
            // retrieve hard reference from weak one
            ::accessibility::AccessibleParaManager::WeakPara::HardRefType aHardRef( rChild.first.get() );

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

    private:
        AccessibleTextHelper_Impl&  mrImpl;
    };

    void AccessibleTextHelper_Impl::UpdateBoundRect()
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // send BOUNDRECT_CHANGED to affected children
        AccessibleTextHelper_UpdateChildBounds aFunctor( *this );
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
    class AccessibleTextHelper_LostChildEvent : public ::std::unary_function< const ::accessibility::AccessibleParaManager::WeakChild&, void >
    {
    public:
        AccessibleTextHelper_LostChildEvent( AccessibleTextHelper_Impl& rImpl ) : mrImpl(rImpl) {}
        void operator()( const ::accessibility::AccessibleParaManager::WeakChild& rPara )
        {
            // retrieve hard reference from weak one
            ::accessibility::AccessibleParaManager::WeakPara::HardRefType aHardRef( rPara.first.get() );

            if( aHardRef.is() )
                mrImpl.FireEvent(AccessibleEventId::CHILD, uno::Any(), uno::makeAny( aHardRef.getRef() ) );
        }

    private:
        AccessibleTextHelper_Impl&  mrImpl;
    };

    void AccessibleTextHelper_Impl::ParagraphsMoved( sal_Int32 nFirst, sal_Int32 nMiddle, sal_Int32 nLast )
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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
            // use INVALIDATE_CHILDREN
            AccessibleTextHelper_LostChildEvent aFunctor( *this );

            ::std::for_each( begin, end, aFunctor );

            maParaManager.Release(nFirst, nLast+1);
            // should be no need for UpdateBoundRect, since all affected children are cleared.
        }
    }

    // functor for sending child events (no stand-alone function, they are maybe not inlined)
    class AccessibleTextHelper_ChildrenTextChanged : public ::std::unary_function< ::accessibility::AccessibleEditableTextPara&, void >
    {
    public:
        void operator()( ::accessibility::AccessibleEditableTextPara& rPara )
        {
            rPara.TextChanged();
        }
    };

    /** functor processing queue events

        Reacts on TEXT_HINT_PARAINSERTED/REMOVED events and stores
        their content
     */
    class AccessibleTextHelper_QueueFunctor : public ::std::unary_function< const SfxHint*, void >
    {
    public:
        AccessibleTextHelper_QueueFunctor() :
            mnParasChanged( 0 ),
            mnParaIndex(-1),
            mnHintId(-1)
        {}
        void operator()( const SfxHint* pEvent )
        {
            if( pEvent &&
                mnParasChanged != -1 )
            {
                // determine hint type
                const TextHint* pTextHint = PTR_CAST( TextHint, pEvent );
                const SvxEditSourceHint* pEditSourceHint = PTR_CAST( SvxEditSourceHint, pEvent );

                if( !pEditSourceHint && pTextHint &&
                    (pTextHint->GetId() == TEXT_HINT_PARAINSERTED ||
                     pTextHint->GetId() == TEXT_HINT_PARAREMOVED ) )
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
        int GetHintId() { return mnHintId; }

    private:
        /** number of paragraphs changed during queue processing. -1 for
            "every paragraph changed"
        */
        sal_Int32 mnParasChanged;
        /// index of paragraph added/removed last
        sal_Int32 mnParaIndex;
        /// TextHint ID (removed/inserted) of last interesting event
        int mnHintId;
    };

    void AccessibleTextHelper_Impl::ProcessQueue()
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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

            if( aFunctor.GetHintId() == TEXT_HINT_PARAINSERTED )
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
            else if( aFunctor.GetHintId() == TEXT_HINT_PARAREMOVED )
            {
                ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator begin = maParaManager.begin();
                ::std::advance( begin, aFunctor.GetParaIndex() );
                ::accessibility::AccessibleParaManager::VectorOfChildren::const_iterator end = begin;
                ::std::advance( end, 1 );

                // #i61812# remember para to be removed for later notification
                // AFTER the new state is applied (that after the para got removed)
                ::uno::Reference< XAccessible > xPara;
                ::accessibility::AccessibleParaManager::WeakPara::HardRefType aHardRef( begin->first.get() );
                if( aHardRef.is() )
                    xPara = ::uno::Reference< XAccessible >( aHardRef.getRef(), ::uno::UNO_QUERY );

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
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr< SfxHint > pHint( maEventQueue.PopFront() );
            SAL_WNODEPRECATED_DECLARATIONS_POP
            if( pHint.get() )
            {
                const SfxHint& rHint = *(pHint.get());

                // determine hint type
                const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
                const SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
                const TextHint* pTextHint = PTR_CAST( TextHint, &rHint );
                const SvxViewHint* pViewHint = PTR_CAST( SvxViewHint, &rHint );
                const SvxEditSourceHint* pEditSourceHint = PTR_CAST( SvxEditSourceHint, &rHint );

                try
                {
                    const sal_Int32 nParas = GetTextForwarder().GetParagraphCount();

                    if( pEditSourceHint )
                    {
                        switch( pEditSourceHint->GetId() )
                        {
                            case EDITSOURCE_HINT_PARASMOVED:
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

                            case EDITSOURCE_HINT_SELECTIONCHANGED:
                                // notify listeners
                                try
                                {
                                    UpdateSelection();
                                }
                                // maybe we're not in edit mode (this is not an error)
                                catch( const uno::Exception& ) {}
                                break;
                        }
                    }
                    else if( pTextHint )
                    {
                        switch( pTextHint->GetId() )
                        {
                            case TEXT_HINT_MODIFIED:
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

                            case TEXT_HINT_PARAINSERTED:
                                // already happened above
                                break;

                            case TEXT_HINT_PARAREMOVED:
                                // already happened above
                                break;

                            case TEXT_HINT_TEXTHEIGHTCHANGED:
                                // visibility changed, done below
                                break;

                            case TEXT_HINT_VIEWSCROLLED:
                                // visibility changed, done below
                                break;
                        }

                        // in all cases, check visibility afterwards.
                        UpdateVisibleChildren();
                        UpdateBoundRect();
                    }
                    else if( pViewHint )
                    {
                        switch( pViewHint->GetHintType() )
                        {
                            case SvxViewHint::SVX_HINT_VIEWCHANGED:
                                // just check visibility
                                UpdateVisibleChildren();
                                UpdateBoundRect();
                                break;
                        }
                    }
                    else if( pSdrHint )
                    {
                        switch( pSdrHint->GetKind() )
                        {
                            case HINT_BEGEDIT:
                            {
                                if(!IsActive())
                                {
                                    break;
                                }
                                // change children state
                                maParaManager.SetActive();

                                // per definition, edit mode text has the focus
                                SetFocus( sal_True );
                                break;
                            }

                            case HINT_ENDEDIT:
                            {
                                // focused child now looses focus
                                ESelection aSelection;
                                if( GetEditViewForwarder().GetSelection( aSelection ) )
                                    SetChildFocus( aSelection.nEndPara, sal_False );

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
                    else if( pSimpleHint )
                    {
                        switch( pSimpleHint->GetId() )
                        {
                            case SFX_HINT_DYING:
                                // edit source is dying under us, become defunc then
                                try
                                {
                                    // make edit source inaccessible
                                    // Note: cannot destroy it here, since we're called from there!
                                    ShutdownEditSource();
                                }
                                catch( const uno::Exception& ) {}

                                break;
                        }
                    }
                }
                catch( const uno::Exception& )
                {
#ifdef DBG_UTIL
                    OSL_TRACE("AccessibleTextHelper_Impl::ProcessQueue: Unhandled exception.");
#endif
                }
            }
        }
    }

    void AccessibleTextHelper_Impl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        // precondition: not in a recursion
        if( mbInNotify )
            return;

        mbInNotify = true;

        // determine hint type
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
        const SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
        const TextHint* pTextHint = PTR_CAST( TextHint, &rHint );
        const SvxViewHint* pViewHint = PTR_CAST( SvxViewHint, &rHint );
        const SvxEditSourceHint* pEditSourceHint = PTR_CAST( SvxEditSourceHint, &rHint );

        try
        {
            // Process notification event
            if( pEditSourceHint )
            {
                maEventQueue.Append( *pEditSourceHint );
                // EditEngine should emit TEXT_SELECTION_CHANGED events (#i27299#)
                if( maEventOpenFrames == 0 )
                    ProcessQueue();
            }
            else if( pTextHint )
            {
                switch( pTextHint->GetId() )
                {
                    case TEXT_HINT_BLOCKNOTIFICATION_END:
                    case TEXT_HINT_INPUT_END:
                        --maEventOpenFrames;

                        if( maEventOpenFrames == 0 )
                        {
                            // #103483#
                            /* All information should have arrived
                             * now, process queue. As stated in the
                             * above bug, we can often avoid throwing
                             * away all paragraphs by looking forward
                             * in the event queue (searching for
                             * PARAINSERT/REMOVE events). Furthermore,
                             * processing the event queue only at the
                             * end of an interaction cycle, ensures
                             * that the EditEngine state and the
                             * AccessibleText state are the same
                             * (well, mostly. If there are _multiple_
                             * interaction cycles in the EE queues, it
                             * can still happen that EE state is
                             * different. That's so to say broken by
                             * design with that delayed EE event
                             * concept).
                             */
                            ProcessQueue();
                        }
                        break;

                    case TEXT_HINT_BLOCKNOTIFICATION_START:
                    case TEXT_HINT_INPUT_START:
                        ++maEventOpenFrames;
                        // no FALLTHROUGH reason: event will not be processed,
                        // thus appending the event isn't necessary. (#i27299#)
                        break;
                    default:
                        maEventQueue.Append( *pTextHint );
                        // EditEngine should emit TEXT_SELECTION_CHANGED events (#i27299#)
                        if( maEventOpenFrames == 0 )
                            ProcessQueue();
                        break;
                }
            }
            else if( pViewHint )
            {
                maEventQueue.Append( *pViewHint );

                // process visibility right away, if not within an
                // open EE notification frame. Otherwise, event
                // processing would be delayed until next EE
                // notification sequence.
                if( maEventOpenFrames == 0 )
                    ProcessQueue();
            }
            else if( pSdrHint )
            {
                maEventQueue.Append( *pSdrHint );

                // process drawing layer events right away, if not
                // within an open EE notification frame. Otherwise,
                // event processing would be delayed until next EE
                // notification sequence.
                if( maEventOpenFrames == 0 )
                    ProcessQueue();
            }
            // it's VITAL to keep the SfxSimpleHint last! It's the base of some classes above!
            else if( pSimpleHint )
            {
                // handle this event _at once_, because after that, objects are invalid
                switch( pSimpleHint->GetId() )
                {
                    case SFX_HINT_DYING:
                        // edit source is dying under us, become defunc then
                        maEventQueue.Clear();
                        try
                        {
                            // make edit source inaccessible
                            // Note: cannot destroy it here, since we're called from there!
                            ShutdownEditSource();
                        }
                        catch( const uno::Exception& ) {}

                        break;
                }
            }
        }
        catch( const uno::Exception& )
        {
#ifdef DBG_UTIL
            OSL_TRACE("AccessibleTextHelper_Impl::Notify: Unhandled exception.");
#endif
            mbInNotify = false;
        }

        mbInNotify = false;
    }

    void AccessibleTextHelper_Impl::Dispose()
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( getNotifierClientId() != -1 )
        {
            try
            {
                // #106234# Unregister from EventNotifier
                ::comphelper::AccessibleEventNotifier::revokeClient( getNotifierClientId() );
#ifdef DBG_UTIL
                OSL_TRACE( "AccessibleTextHelper_Impl disposed ID: %d", mnNotifierClientId );
#endif
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
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        maEditSource.SetEditSource( ::std::auto_ptr< SvxEditSource >(NULL) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        mxFrontEnd = NULL;
    }

    void AccessibleTextHelper_Impl::FireEvent( const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue ) const
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // #102261# Call global queue for focus events
        if( rEvent.EventId == AccessibleStateType::FOCUSED )
            vcl::unohelper::NotifyAccessibleStateEventGlobally( rEvent );

        // #106234# Delegate to EventNotifier
        ::comphelper::AccessibleEventNotifier::addEvent( getNotifierClientId(),
                                                         rEvent );
    }

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleTextHelper_Impl::getAccessibleChildCount() SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        return mnLastVisibleChild - mnFirstVisibleChild + 1;
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleTextHelper_Impl::getAccessibleChild( sal_Int32 i ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

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
            return NULL;
    }

    void SAL_CALL AccessibleTextHelper_Impl::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::addEventListener( getNotifierClientId(), xListener );
    }

    void SAL_CALL AccessibleTextHelper_Impl::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        if( getNotifierClientId() != -1 )
            ::comphelper::AccessibleEventNotifier::removeEventListener( getNotifierClientId(), xListener );
    }

    uno::Reference< XAccessible > SAL_CALL AccessibleTextHelper_Impl::getAccessibleAtPoint( const awt::Point& _aPoint ) SAL_THROW((uno::RuntimeException))
    {
        DBG_CHKTHIS( AccessibleTextHelper_Impl, NULL );

        // make given position relative
        if( !mxFrontEnd.is() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::getAccessibleAt: frontend invalid", mxFrontEnd );

        uno::Reference< XAccessibleContext > xFrontEndContext = mxFrontEnd->getAccessibleContext();

        if( !xFrontEndContext.is() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::getAccessibleAt: frontend invalid", mxFrontEnd );

        uno::Reference< XAccessibleComponent > xFrontEndComponent( xFrontEndContext, uno::UNO_QUERY );

        if( !xFrontEndComponent.is() )
            throw uno::RuntimeException("AccessibleTextHelper_Impl::getAccessibleAt: frontend is no XAccessibleComponent", mxFrontEnd );

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
            DBG_ASSERT(nChild >= 0 && nChild <= USHRT_MAX,
                       "AccessibleTextHelper_Impl::getAccessibleAt: index value overflow");

            Rectangle aParaBounds( rCacheTF.GetParaBounds( nChild ) );

            if( aParaBounds.IsInside( aLogPoint ) )
                return getAccessibleChild( nChild - mnFirstVisibleChild + GetStartIndex() );
        }

        // found none
        return NULL;
    }

    //------------------------------------------------------------------------
    //
    // AccessibleTextHelper implementation (simply forwards to impl)
    //
    //------------------------------------------------------------------------

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AccessibleTextHelper::AccessibleTextHelper( ::std::auto_ptr< SvxEditSource > pEditSource ) :
        mpImpl( new AccessibleTextHelper_Impl() )
    {
        SolarMutexGuard aGuard;

        SetEditSource( pEditSource );
    }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH

    AccessibleTextHelper::~AccessibleTextHelper()
    {
    }

    const SvxEditSource& AccessibleTextHelper::GetEditSource() const SAL_THROW((uno::RuntimeException))
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

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    void AccessibleTextHelper::SetEditSource( ::std::auto_ptr< SvxEditSource > pEditSource ) SAL_THROW((uno::RuntimeException))
    {
#ifdef DBG_UTIL
        // precondition: solar mutex locked
        DBG_TESTSOLARMUTEX();

        mpImpl->CheckInvariants();
#endif

        mpImpl->SetEditSource( pEditSource );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

    uno::Reference< XAccessible > AccessibleTextHelper::GetEventSource() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        uno::Reference< XAccessible > xRet( mpImpl->GetEventSource() );

        mpImpl->CheckInvariants();

        return xRet;
#else
        return mpImpl->GetEventSource();
#endif
    }

    void AccessibleTextHelper::SetFocus( sal_Bool bHaveFocus ) SAL_THROW((::com::sun::star::uno::RuntimeException))
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

    sal_Bool AccessibleTextHelper::HaveFocus() SAL_THROW((::com::sun::star::uno::RuntimeException))
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        sal_Bool bRet( mpImpl->HaveFocus() );

        mpImpl->CheckInvariants();

        return bRet;
#else
        return mpImpl->HaveFocus();
#endif
    }

    void AccessibleTextHelper::FireEvent( const sal_Int16 nEventId, const uno::Any& rNewValue, const uno::Any& rOldValue ) const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->FireEvent( nEventId, rNewValue, rOldValue );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif
    }

    void AccessibleTextHelper::FireEvent( const AccessibleEventObject& rEvent ) const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
#endif

        mpImpl->FireEvent( rEvent );

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();
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

    Point AccessibleTextHelper::GetOffset() const
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        Point aPoint( mpImpl->GetOffset() );

        mpImpl->CheckInvariants();

        return aPoint;
#else
        return mpImpl->GetOffset();
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

    void AccessibleTextHelper::UpdateChildren() SAL_THROW((::com::sun::star::uno::RuntimeException))
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

    sal_Bool AccessibleTextHelper::IsSelected() const
    {
        SolarMutexGuard aGuard;

#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        sal_Bool aRet = mpImpl->IsSelected();

        mpImpl->CheckInvariants();

        return aRet;
#else
        return mpImpl->IsSelected();
#endif
    }

    // XAccessibleContext
    sal_Int32 AccessibleTextHelper::GetChildCount() SAL_THROW((uno::RuntimeException))
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

    uno::Reference< XAccessible > AccessibleTextHelper::GetChild( sal_Int32 i ) SAL_THROW((lang::IndexOutOfBoundsException, uno::RuntimeException))
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

    void AccessibleTextHelper::AddEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException))
    {
#ifdef DBG_UTIL
        mpImpl->CheckInvariants();

        mpImpl->addAccessibleEventListener( xListener );

        mpImpl->CheckInvariants();
#else
        mpImpl->addAccessibleEventListener( xListener );
#endif
    }

    void AccessibleTextHelper::RemoveEventListener( const uno::Reference< XAccessibleEventListener >& xListener ) SAL_THROW((uno::RuntimeException))
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
    uno::Reference< XAccessible > AccessibleTextHelper::GetAt( const awt::Point& aPoint ) SAL_THROW((uno::RuntimeException))
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

//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
