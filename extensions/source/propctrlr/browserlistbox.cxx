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

#include "browserlistbox.hxx"
#include <strings.hrc>
#include "proplinelistener.hxx"
#include "propcontrolobserver.hxx"
#include "linedescriptor.hxx"
#include "inspectorhelpwindow.hxx"

#include <sal/log.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/asyncnotification.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>


namespace pcr
{


    #define FRAME_OFFSET 4
        // TODO: find out what this is really for ... and check if it does make sense in the new
        // browser environment
    #define LAYOUT_HELP_WINDOW_DISTANCE_APPFONT 3

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::uno::UNO_QUERY;

    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;

    enum ControlEventType
    {
        FOCUS_GAINED,
        VALUE_CHANGED,
        ACTIVATE_NEXT
    };

    struct ControlEvent : public ::comphelper::AnyEvent
    {
        Reference< XPropertyControl >   xControl;
        ControlEventType                eType;

        ControlEvent( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType )
            :xControl( _rxControl )
            ,eType( _eType )
        {
        }
    };

    class SharedNotifier
    {
    private:
        static ::osl::Mutex&                                        getMutex();
        static ::rtl::Reference< ::comphelper::AsyncEventNotifier > s_pNotifier;

    public:
        SharedNotifier(const SharedNotifier&) = delete;
        SharedNotifier& operator=(const SharedNotifier&) = delete;
        static const ::rtl::Reference< ::comphelper::AsyncEventNotifier >&
            getNotifier();
    };


    ::rtl::Reference< ::comphelper::AsyncEventNotifier > SharedNotifier::s_pNotifier;


    ::osl::Mutex& SharedNotifier::getMutex()
    {
        static ::osl::Mutex s_aMutex;
        return s_aMutex;
    }


    const ::rtl::Reference< ::comphelper::AsyncEventNotifier >& SharedNotifier::getNotifier()
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( !s_pNotifier.is() )
        {
            s_pNotifier.set(
                new ::comphelper::AsyncEventNotifier("browserlistbox"));
            s_pNotifier->launch();
                //TODO: a protocol is missing how to join with the launched
                // thread before exit(3), to ensure the thread is no longer
                // relying on any infrastructure while that infrastructure is
                // being shut down in atexit handlers
        }
        return s_pNotifier;
    }


    /** implementation for of <type scope="css::inspection">XPropertyControlContext</type>
        which forwards all events to a non-UNO version of this interface
    */
    typedef ::cppu::WeakImplHelper< XPropertyControlContext > PropertyControlContext_Impl_Base;
    class PropertyControlContext_Impl   :public PropertyControlContext_Impl_Base
                                        ,public ::comphelper::IEventProcessor
    {
    public:
        enum NotificationMode
        {
            eSynchronously,
            eAsynchronously
        };

    private:
        VclPtr<OBrowserListBox>     m_pContext;
        NotificationMode            m_eMode;

    public:
        /** creates an instance
            @param _rContextImpl
                the instance to delegate events to
        */
        explicit PropertyControlContext_Impl( OBrowserListBox& _rContextImpl );

        /** disposes the context.

            When you call this method, all subsequent callbacks to the
            <type scope="css::inspection">XPropertyControlContext</type> methods
            will throw a <type scope="css::lang">DisposedException</type>.
        */
        void dispose();

        /** sets the notification mode, so that notifications received from the controls are
            forwarded to our OBrowserListBox either synchronously or asynchronously
            @param  _eMode
                the new notification mode
        */
        void setNotificationMode( NotificationMode _eMode );

        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

    protected:
        virtual ~PropertyControlContext_Impl() override;

        // XPropertyControlObserver
        virtual void SAL_CALL focusGained( const Reference< XPropertyControl >& Control ) override;
        virtual void SAL_CALL valueChanged( const Reference< XPropertyControl >& Control ) override;
        // XPropertyControlContext
        virtual void SAL_CALL activateNextControl( const Reference< XPropertyControl >& CurrentControl ) override;

        // IEventProcessor
        virtual void processEvent( const ::comphelper::AnyEvent& _rEvent ) override;

    private:
        /** processes the given event, i.e. notifies it to our OBrowserListBox
            @param  _rEvent
                the event no notify
            @precond
                our mutex (well, the SolarMutex) is locked
        */
        void impl_processEvent_throw( const ::comphelper::AnyEvent& _rEvent );

        /** checks whether the instance is already disposed
        */
        bool impl_isDisposed_nothrow() const { return m_pContext.get() == nullptr; }

        /** notifies the given event originating from the given control
        @throws DisposedException
        @param  _rxControl
        @param  _eType
        */
        void impl_notify_throw( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType );
    };


    PropertyControlContext_Impl::PropertyControlContext_Impl( OBrowserListBox& _rContextImpl )
        :m_pContext( &_rContextImpl )
        ,m_eMode( eAsynchronously )
    {
    }


    PropertyControlContext_Impl::~PropertyControlContext_Impl()
    {
        if ( !impl_isDisposed_nothrow() )
            dispose();
    }


    void PropertyControlContext_Impl::dispose()
    {
        SolarMutexGuard aGuard;
        if ( impl_isDisposed_nothrow() )
            return;

        SharedNotifier::getNotifier()->removeEventsForProcessor( this );
        m_pContext = nullptr;
    }


    void PropertyControlContext_Impl::setNotificationMode( NotificationMode _eMode )
    {
        SolarMutexGuard aGuard;
        m_eMode = _eMode;
    }


    void PropertyControlContext_Impl::impl_notify_throw( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType )
    {
        ::comphelper::AnyEventRef pEvent;

        {
            SolarMutexGuard aGuard;
            if ( impl_isDisposed_nothrow() )
                 throw DisposedException( OUString(), *this );
            pEvent = new ControlEvent( _rxControl, _eType );

            if ( m_eMode == eSynchronously )
            {
                impl_processEvent_throw( *pEvent );
                return;
            }
        }

        SharedNotifier::getNotifier()->addEvent( pEvent, this );
    }


    void SAL_CALL PropertyControlContext_Impl::focusGained( const Reference< XPropertyControl >& Control )
    {
        impl_notify_throw( Control, FOCUS_GAINED );
    }


    void SAL_CALL PropertyControlContext_Impl::valueChanged( const Reference< XPropertyControl >& Control )
    {
        impl_notify_throw( Control, VALUE_CHANGED );
    }


    void SAL_CALL PropertyControlContext_Impl::activateNextControl( const Reference< XPropertyControl >& CurrentControl )
    {
        impl_notify_throw( CurrentControl, ACTIVATE_NEXT );
    }


    void SAL_CALL PropertyControlContext_Impl::acquire() throw()
    {
        PropertyControlContext_Impl_Base::acquire();
    }


    void SAL_CALL PropertyControlContext_Impl::release() throw()
    {
        PropertyControlContext_Impl_Base::release();
    }


    void PropertyControlContext_Impl::processEvent( const ::comphelper::AnyEvent& _rEvent )
    {
        SolarMutexGuard aGuard;
        if ( impl_isDisposed_nothrow() )
            return;

        try
        {
            impl_processEvent_throw( _rEvent );
        }
        catch( const Exception& )
        {
            // can't handle otherwise, since our caller (the notification thread) does not allow
            // for exceptions (it could itself abort only)
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    void PropertyControlContext_Impl::impl_processEvent_throw( const ::comphelper::AnyEvent& _rEvent )
    {
        const ControlEvent& rControlEvent = static_cast< const ControlEvent& >( _rEvent );
        switch ( rControlEvent.eType )
        {
        case FOCUS_GAINED:
            m_pContext->focusGained( rControlEvent.xControl );
            break;
        case VALUE_CHANGED:
            m_pContext->valueChanged( rControlEvent.xControl );
            break;
        case ACTIVATE_NEXT:
            m_pContext->activateNextControl( rControlEvent.xControl );
            break;
        }
    }

    OBrowserListBox::OBrowserListBox( vcl::Window* pParent)
            :Control(pParent, WB_DIALOGCONTROL | WB_CLIPCHILDREN)
            ,m_aLinesPlayground(VclPtr<vcl::Window>::Create(this,WB_DIALOGCONTROL | WB_CLIPCHILDREN))
            ,m_aVScroll(VclPtr<ScrollBar>::Create(this,WB_VSCROLL|WB_REPEAT|WB_DRAG))
            ,m_pHelpWindow( VclPtr<InspectorHelpWindow>::Create( this ) )
            ,m_pLineListener(nullptr)
            ,m_pControlObserver( nullptr )
            ,m_nYOffset(0)
            ,m_nCurrentPreferredHelpHeight(0)
            ,m_nTheNameSize(0)
            ,m_bIsActive(false)
            ,m_bUpdate(true)
            ,m_pControlContextImpl( new PropertyControlContext_Impl( *this ) )
    {
        ScopedVclPtrInstance<ListBox> aListBox(this, WB_DROPDOWN);
        ScopedVclPtrInstance<Edit> aEditBox(this);
        m_nRowHeight = std::max(aListBox->get_preferred_size().Height(),
                                aEditBox->get_preferred_size().Height());
        m_nRowHeight += 2;
        SetBackground( pParent->GetBackground() );
        m_aLinesPlayground->SetBackground( GetBackground() );

        m_aLinesPlayground->SetPosPixel(Point(0,0));
        m_aLinesPlayground->SetPaintTransparent(true);
        m_aLinesPlayground->Show();
        m_aVScroll->Hide();
        m_aVScroll->SetScrollHdl(LINK(this, OBrowserListBox, ScrollHdl));
    }

    OBrowserListBox::~OBrowserListBox()
    {
        disposeOnce();
    }

    void OBrowserListBox::dispose()
    {
        OSL_ENSURE( !IsModified(), "OBrowserListBox::~OBrowserListBox: still modified - should have been committed before!" );
            // doing the commit here, while we, as well as our owner, as well as some other components,
            // are already "half dead" (means within their dtor) is potentially dangerous.
            // By definition, CommitModified has to be called (if necessary) before destruction

        m_pControlContextImpl->dispose();
        m_pControlContextImpl.clear();

        Hide();
        Clear();
        m_aLinesPlayground.disposeAndClear();
        m_aVScroll.disposeAndClear();
        m_pHelpWindow.disposeAndClear();
        Control::dispose();
    }


    bool OBrowserListBox::IsModified( ) const
    {
        bool bModified = false;

        if ( m_bIsActive && m_xActiveControl.is() )
            bModified = m_xActiveControl->isModified();

        return bModified;
    }


    void OBrowserListBox::CommitModified( )
    {
        if ( IsModified() && m_xActiveControl.is() )
        {
            // for the time of this commit, notify all events synchronously
            // #i63814#
            m_pControlContextImpl->setNotificationMode( PropertyControlContext_Impl::eSynchronously );
            try
            {
                m_xActiveControl->notifyModifiedValue();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
            m_pControlContextImpl->setNotificationMode( PropertyControlContext_Impl::eAsynchronously );
        }
    }


    void OBrowserListBox::ActivateListBox(bool _bActive)
    {
        m_bIsActive = _bActive;
        if (m_bIsActive)
        {
            // TODO: what's the sense of this?
            m_aVScroll->SetThumbPos(100);
            MoveThumbTo(0);
            Resize();
        }
    }


    long OBrowserListBox::impl_getPrefererredHelpHeight()
    {
        return HasHelpSection() ? m_pHelpWindow->GetOptimalHeightPixel() : 0;
    }


    void OBrowserListBox::Resize()
    {
        tools::Rectangle aPlayground( Point( 0, 0 ), GetOutputSizePixel() );
        Size aHelpWindowDistance( LogicToPixel(Size(0, LAYOUT_HELP_WINDOW_DISTANCE_APPFONT), MapMode(MapUnit::MapAppFont)) );

        long nHelpWindowHeight = m_nCurrentPreferredHelpHeight = impl_getPrefererredHelpHeight();
        bool bPositionHelpWindow = ( nHelpWindowHeight != 0 );

        tools::Rectangle aLinesArea( aPlayground );
        if ( bPositionHelpWindow )
        {
            aLinesArea.AdjustBottom( -nHelpWindowHeight );
            aLinesArea.AdjustBottom( -(aHelpWindowDistance.Height()) );
        }
        m_aLinesPlayground->SetPosSizePixel( aLinesArea.TopLeft(), aLinesArea.GetSize() );

        UpdateVScroll();

        bool bNeedScrollbar = m_aLines.size() > static_cast<sal_uInt32>(CalcVisibleLines());
        if ( !bNeedScrollbar )
        {
            if ( m_aVScroll->IsVisible() )
                m_aVScroll->Hide();
            // scroll to top
            m_nYOffset = 0;
            m_aVScroll->SetThumbPos( 0 );
        }
        else
        {
            Size aVScrollSize( m_aVScroll->GetSizePixel() );

            // adjust the playground's width
            aLinesArea.AdjustRight( -(aVScrollSize.Width()) );
            m_aLinesPlayground->SetPosSizePixel( aLinesArea.TopLeft(), aLinesArea.GetSize() );

            // position the scrollbar
            aVScrollSize.setHeight( aLinesArea.GetHeight() );
            Point aVScrollPos( aLinesArea.GetWidth(), 0 );
            m_aVScroll->SetPosSizePixel( aVScrollPos, aVScrollSize );
        }

        for ( ListBoxLines::size_type i = 0; i < m_aLines.size(); ++i )
            m_aOutOfDateLines.insert( i );

        // repaint
        EnablePaint(false);
        UpdatePlayGround();
        EnablePaint(true);

        // show the scrollbar
        if ( bNeedScrollbar )
            m_aVScroll->Show();

        // position the help window
        if ( bPositionHelpWindow )
        {
            tools::Rectangle aHelpArea( aPlayground );
            aHelpArea.SetTop( aLinesArea.Bottom() + aHelpWindowDistance.Height() );
            m_pHelpWindow->SetPosSizePixel( aHelpArea.TopLeft(), aHelpArea.GetSize() );
        }
    }


    void OBrowserListBox::SetListener( IPropertyLineListener* _pListener )
    {
        m_pLineListener = _pListener;
    }


    void OBrowserListBox::SetObserver( IPropertyControlObserver* _pObserver )
    {
        m_pControlObserver = _pObserver;
    }


    void OBrowserListBox::EnableHelpSection( bool _bEnable )
    {
        m_pHelpWindow->Show( _bEnable );
        Resize();
    }


    bool OBrowserListBox::HasHelpSection() const
    {
        return m_pHelpWindow->IsVisible();
    }


    void OBrowserListBox::SetHelpText( const OUString& _rHelpText )
    {
        OSL_ENSURE( HasHelpSection(), "OBrowserListBox::SetHelpText: help section not visible!" );
        m_pHelpWindow->SetText( _rHelpText );
        if ( m_nCurrentPreferredHelpHeight != impl_getPrefererredHelpHeight() )
            Resize();
    }


    void OBrowserListBox::SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines )
    {
        m_pHelpWindow->SetLimits( _nMinLines, _nMaxLines );
    }


    sal_uInt16 OBrowserListBox::CalcVisibleLines()
    {
        Size aSize(m_aLinesPlayground->GetOutputSizePixel());
        sal_uInt16 nResult = 0;
        if (0 != m_nRowHeight)
            nResult = static_cast<sal_uInt16>(aSize.Height())/m_nRowHeight;

        return nResult;
    }


    void OBrowserListBox::UpdateVScroll()
    {
        sal_uInt16 nLines = CalcVisibleLines();
        m_aVScroll->SetPageSize(nLines-1);
        m_aVScroll->SetVisibleSize(nLines-1);

        size_t nCount = m_aLines.size();
        if (nCount>0)
        {
            m_aVScroll->SetRange(Range(0,nCount-1));
            m_nYOffset = -m_aVScroll->GetThumbPos()*m_nRowHeight;
        }
        else
        {
            m_aVScroll->SetRange(Range(0,0));
            m_nYOffset = 0;
        }
    }


    void OBrowserListBox::PositionLine( ListBoxLines::size_type _nIndex )
    {
        Size aSize(m_aLinesPlayground->GetOutputSizePixel());
        Point aPos(0, m_nYOffset);

        aSize.setHeight( m_nRowHeight );

        aPos.AdjustY(_nIndex * m_nRowHeight );

        if ( _nIndex < m_aLines.size() )
        {
            BrowserLinePointer pLine = m_aLines[ _nIndex ].pLine;

            pLine->SetPosSizePixel( aPos, aSize );
            pLine->SetTitleWidth( m_nTheNameSize + 2 * FRAME_OFFSET );

            // show the line if necessary
            if ( !pLine->IsVisible() )
                pLine->Show();
        }
    }


    void OBrowserListBox::UpdatePosNSize()
    {
        for ( auto const & aLoop: m_aOutOfDateLines )
        {
            DBG_ASSERT( aLoop < m_aLines.size(), "OBrowserListBox::UpdatePosNSize: invalid line index!" );
            if ( aLoop < m_aLines.size() )
                PositionLine( aLoop );
        }
        m_aOutOfDateLines.clear();
    }


    void OBrowserListBox::UpdatePlayGround()
    {
        sal_Int32 nThumbPos = m_aVScroll->GetThumbPos();
        sal_Int32 nLines = CalcVisibleLines();

        ListBoxLines::size_type nEnd = nThumbPos + nLines;
        if (nEnd >= m_aLines.size())
            nEnd = m_aLines.size()-1;

        if ( !m_aLines.empty() )
        {
            for ( ListBoxLines::size_type i = nThumbPos; i <= nEnd; ++i )
                m_aOutOfDateLines.insert( i );
            UpdatePosNSize();
        }
    }


    void OBrowserListBox::DisableUpdate()
    {
        m_bUpdate = false;
    }


    void OBrowserListBox::EnableUpdate()
    {
        m_bUpdate = true;
        Resize();
    }


    void OBrowserListBox::SetPropertyValue(const OUString& _rEntryName, const Any& _rValue, bool _bUnknownValue )
    {
        ListBoxLines::iterator line = std::find_if(m_aLines.begin(), m_aLines.end(),
            [&_rEntryName](const ListBoxLine& rLine) { return rLine.aName == _rEntryName; });

        if ( line != m_aLines.end() )
        {
            if ( _bUnknownValue )
            {
                Reference< XPropertyControl > xControl( line->pLine->getControl() );
                OSL_ENSURE( xControl.is(), "OBrowserListBox::SetPropertyValue: illegal control!" );
                if ( xControl.is() )
                    xControl->setValue( Any() );
            }
            else
                impl_setControlAsPropertyValue( *line, _rValue );
        }
    }


    sal_uInt16 OBrowserListBox::GetPropertyPos( const OUString& _rEntryName ) const
    {
        sal_uInt16 nPos = 0;
        for (auto const& line : m_aLines)
        {
            if ( line.aName == _rEntryName )
            {
                return nPos;
            }
            ++nPos;
        }

        return EDITOR_LIST_ENTRY_NOTFOUND;
    }


    bool OBrowserListBox::impl_getBrowserLineForName( const OUString& _rEntryName, BrowserLinePointer& _out_rpLine ) const
    {
        ListBoxLines::const_iterator line = std::find_if(m_aLines.begin(), m_aLines.end(),
            [&_rEntryName](const ListBoxLine& rLine) { return rLine.aName == _rEntryName; });

        if ( line != m_aLines.end() )
            _out_rpLine = line->pLine;
        else
            _out_rpLine.reset();
        return ( nullptr != _out_rpLine.get() );
    }


    void OBrowserListBox::EnablePropertyControls( const OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            pLine->EnablePropertyControls( _nControls, _bEnable );
    }


    void OBrowserListBox::EnablePropertyLine( const OUString& _rEntryName, bool _bEnable )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            pLine->EnablePropertyLine( _bEnable );
    }


    Reference< XPropertyControl > OBrowserListBox::GetPropertyControl( const OUString& _rEntryName )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            return pLine->getControl();
        return nullptr;
    }


    void OBrowserListBox::InsertEntry(const OLineDescriptor& _rPropertyData, sal_uInt16 _nPos)
    {
        // create a new line
        BrowserLinePointer pBrowserLine( new OBrowserLine( _rPropertyData.sName, m_aLinesPlayground.get() ) );

        // check that the name is unique
        for (auto const& line : m_aLines)
        {
            if (line.aName == _rPropertyData.sName)
            {
                // already have another line for this name!
                assert(false);
            }
        }

        ListBoxLine aNewLine( _rPropertyData.sName, pBrowserLine, _rPropertyData.xPropertyHandler );
        ListBoxLines::size_type nInsertPos = _nPos;
        if ( _nPos >= m_aLines.size() )
        {
            nInsertPos = m_aLines.size();
            m_aLines.push_back( aNewLine );
        }
        else
            m_aLines.insert( m_aLines.begin() + _nPos, aNewLine );

        pBrowserLine->SetTitleWidth(m_nTheNameSize);
        if (m_bUpdate)
        {
            UpdateVScroll();
            Invalidate();
        }

        // initialize the entry
        ChangeEntry(_rPropertyData, nInsertPos);

        // update the positions of possibly affected lines
        ListBoxLines::size_type nUpdatePos = nInsertPos;
        while ( nUpdatePos < m_aLines.size() )
            m_aOutOfDateLines.insert( nUpdatePos++ );
        UpdatePosNSize( );
    }


    sal_Int32 OBrowserListBox::GetMinimumWidth()
    {
        return m_nTheNameSize + 2 * FRAME_OFFSET + (m_nRowHeight - 4) * 8;
    }


    sal_Int32 OBrowserListBox::GetMinimumHeight()
    {
        // assume that we want to display 5 rows, at least
        sal_Int32 nMinHeight = m_nRowHeight * 5;

        if ( HasHelpSection() )
        {
            Size aHelpWindowDistance( LogicToPixel(Size(0, LAYOUT_HELP_WINDOW_DISTANCE_APPFONT), MapMode(MapUnit::MapAppFont)) );
            nMinHeight += aHelpWindowDistance.Height();

            nMinHeight += m_pHelpWindow->GetMinimalHeightPixel();
        }

        return nMinHeight;
    }


    void OBrowserListBox::ShowEntry(sal_uInt16 _nPos)
    {
        if ( _nPos < m_aLines.size() )
        {
            sal_Int32 nThumbPos = m_aVScroll->GetThumbPos();

            if (_nPos < nThumbPos)
                MoveThumbTo(_nPos);
            else
            {
                sal_Int32 nLines = CalcVisibleLines();
                if (_nPos >= nThumbPos + nLines)
                    MoveThumbTo(_nPos - nLines + 1);
            }
        }

    }


    void OBrowserListBox::MoveThumbTo(sal_Int32 _nNewThumbPos)
    {
        // disable painting to prevent flicker
        m_aLinesPlayground->EnablePaint(false);

        sal_Int32 nDelta = _nNewThumbPos - m_aVScroll->GetThumbPos();
        // adjust the scrollbar
        m_aVScroll->SetThumbPos(_nNewThumbPos);
        sal_Int32 nThumbPos = _nNewThumbPos;

        m_nYOffset = -m_aVScroll->GetThumbPos() * m_nRowHeight;

        sal_Int32 nLines = CalcVisibleLines();
        ListBoxLines::size_type nEnd = nThumbPos + nLines;

        m_aLinesPlayground->Scroll(0, -nDelta * m_nRowHeight, ScrollFlags::Children);

        if (1 == nDelta)
        {
            // TODO: what's the sense of this two PositionLines? Why not just one call?
            PositionLine(nEnd-1);
            PositionLine(nEnd);
        }
        else if (-1 == nDelta)
        {
            PositionLine(nThumbPos);
        }
        else if (0 != nDelta)
        {
            UpdatePlayGround();
        }

        m_aLinesPlayground->EnablePaint(true);
        m_aLinesPlayground->Invalidate(InvalidateFlags::Children);
    }


    IMPL_LINK(OBrowserListBox, ScrollHdl, ScrollBar*, _pScrollBar, void )
    {
        DBG_ASSERT(_pScrollBar == m_aVScroll.get(), "OBrowserListBox::ScrollHdl: where does this come from?");

        // disable painting to prevent flicker
        m_aLinesPlayground->EnablePaint(false);

        sal_Int32 nThumbPos = m_aVScroll->GetThumbPos();

        sal_Int32 nDelta = m_aVScroll->GetDelta();
        m_nYOffset = -nThumbPos * m_nRowHeight;

        ListBoxLines::size_type nEnd = nThumbPos + CalcVisibleLines();

        m_aLinesPlayground->Scroll(0, -nDelta * m_nRowHeight, ScrollFlags::Children);

        if (1 == nDelta)
        {
            PositionLine(nEnd-1);
            PositionLine(nEnd);
        }
        else if (nDelta==-1)
        {
            PositionLine(nThumbPos);
        }
        else if (nDelta!=0 || m_aVScroll->GetType() == ScrollType::DontKnow)
        {
            UpdatePlayGround();
        }

        m_aLinesPlayground->EnablePaint(true);
    }


    void OBrowserListBox::buttonClicked( OBrowserLine* _pLine, bool _bPrimary )
    {
        DBG_ASSERT( _pLine, "OBrowserListBox::buttonClicked: invalid browser line!" );
        if ( _pLine && m_pLineListener )
        {
            m_pLineListener->Clicked( _pLine->GetEntryName(), _bPrimary );
        }
    }


    void OBrowserListBox::impl_setControlAsPropertyValue( const ListBoxLine& _rLine, const Any& _rPropertyValue )
    {
        Reference< XPropertyControl > xControl( _rLine.pLine->getControl() );
        try
        {
            if ( _rPropertyValue.getValueType().equals( _rLine.pLine->getControl()->getValueType() ) )
            {
                xControl->setValue( _rPropertyValue );
            }
            else
            {
                SAL_WARN_IF( !_rLine.xHandler.is(), "extensions.propctrlr",
                        "OBrowserListBox::impl_setControlAsPropertyValue: no handler -> no conversion (property: '"
                        << _rLine.pLine->GetEntryName() << "')!" );
                if ( _rLine.xHandler.is() )
                {
                    Any aControlValue = _rLine.xHandler->convertToControlValue(
                        _rLine.pLine->GetEntryName(), _rPropertyValue, xControl->getValueType() );
                    xControl->setValue( aControlValue );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
    }


    Any OBrowserListBox::impl_getControlAsPropertyValue( const ListBoxLine& _rLine )
    {
        Reference< XPropertyControl > xControl( _rLine.pLine->getControl() );
        Any aPropertyValue;
        try
        {
            SAL_WARN_IF( !_rLine.xHandler.is(), "extensions.propctrlr",
                    "OBrowserListBox::impl_getControlAsPropertyValue: no handler -> no conversion (property: '"
                    << _rLine.pLine->GetEntryName() << "')!" );
            if ( _rLine.xHandler.is() )
                aPropertyValue = _rLine.xHandler->convertToPropertyValue( _rLine.pLine->GetEntryName(), xControl->getValue() );
            else
                aPropertyValue = xControl->getValue();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
        return aPropertyValue;
    }


    sal_uInt16 OBrowserListBox::impl_getControlPos( const Reference< XPropertyControl >& _rxControl ) const
    {
        sal_uInt16 nPos = 0;
        for (auto const& search : m_aLines)
        {
            if ( search.pLine->getControl().get() == _rxControl.get() )
                return nPos;
            ++nPos;
        }
        OSL_FAIL( "OBrowserListBox::impl_getControlPos: invalid control - not part of any of our lines!" );
        return sal_uInt16(-1);
    }


    void OBrowserListBox::focusGained( const Reference< XPropertyControl >& _rxControl )
    {
        DBG_TESTSOLARMUTEX();

        DBG_ASSERT( _rxControl.is(), "OBrowserListBox::focusGained: invalid event source!" );
        if ( !_rxControl.is() )
            return;

        if ( m_pControlObserver )
            m_pControlObserver->focusGained( _rxControl );

        m_xActiveControl = _rxControl;
        ShowEntry( impl_getControlPos( m_xActiveControl ) );
    }


    void OBrowserListBox::valueChanged( const Reference< XPropertyControl >& _rxControl )
    {
        DBG_TESTSOLARMUTEX();

        DBG_ASSERT( _rxControl.is(), "OBrowserListBox::valueChanged: invalid event source!" );
        if ( !_rxControl.is() )
            return;

        if ( m_pControlObserver )
            m_pControlObserver->valueChanged( _rxControl );

        if ( m_pLineListener )
        {
            const ListBoxLine& rLine = m_aLines[ impl_getControlPos( _rxControl ) ];
            m_pLineListener->Commit(
                rLine.pLine->GetEntryName(),
                impl_getControlAsPropertyValue( rLine )
            );
        }
    }


    void OBrowserListBox::activateNextControl( const Reference< XPropertyControl >& _rxCurrentControl )
    {
        DBG_TESTSOLARMUTEX();

        sal_uInt16 nLine = impl_getControlPos( _rxCurrentControl );

        // cycle forwards, 'til we've the next control which can grab the focus
        ++nLine;
        while ( static_cast< size_t >( nLine ) < m_aLines.size() )
        {
            if ( m_aLines[nLine].pLine->GrabFocus() )
                break;
            ++nLine;
        }

        // wrap around?
        if ( ( static_cast< size_t >( nLine ) >= m_aLines.size() ) && ( !m_aLines.empty() ) )
            m_aLines[0].pLine->GrabFocus();
    }


    namespace
    {

        void lcl_implDisposeControl_nothrow( const Reference< XPropertyControl >& _rxControl )
        {
            if ( !_rxControl.is() )
                return;
            try
            {
                _rxControl->setControlContext( nullptr );
                Reference< XComponent > xControlComponent( _rxControl, UNO_QUERY );
                if ( xControlComponent.is() )
                    xControlComponent->dispose();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
            }
        }
    }


    void OBrowserListBox::Clear()
    {
        for (auto const& line : m_aLines)
        {
            // hide the line
            line.pLine->Hide();
            // reset the listener
            lcl_implDisposeControl_nothrow( line.pLine->getControl() );
        }

        clearContainer( m_aLines );
    }


    bool OBrowserListBox::RemoveEntry( const OUString& _rName )
    {
        ListBoxLines::iterator it = std::find_if(m_aLines.begin(), m_aLines.end(),
            [&_rName](const ListBoxLine& rLine) { return rLine.aName == _rName; });

        if ( it == m_aLines.end() )
            return false;

        ListBoxLines::size_type nPos = static_cast<ListBoxLines::size_type>(std::distance(m_aLines.begin(), it));
        m_aLines.erase( it );
        m_aOutOfDateLines.erase( m_aLines.size() );

        // update the positions of possibly affected lines
        while ( nPos < m_aLines.size() )
            m_aOutOfDateLines.insert( nPos++ );
        UpdatePosNSize( );

        return true;
    }


    void OBrowserListBox::ChangeEntry( const OLineDescriptor& _rPropertyData, ListBoxLines::size_type nPos )
    {
        OSL_PRECOND( _rPropertyData.Control.is(), "OBrowserListBox::ChangeEntry: invalid control!" );
        if ( !_rPropertyData.Control.is() )
            return;

        if ( nPos == EDITOR_LIST_REPLACE_EXISTING )
            nPos = GetPropertyPos( _rPropertyData.sName );

        if ( nPos < m_aLines.size() )
        {
            vcl::Window* pRefWindow = nullptr;
            if ( nPos > 0 )
                pRefWindow = m_aLines[nPos-1].pLine->GetRefWindow();

            // the current line and control
            ListBoxLine& rLine = m_aLines[nPos];

            // the old control and some data about it
            Reference< XPropertyControl > xControl = rLine.pLine->getControl();
            vcl::Window* pControlWindow = rLine.pLine->getControlWindow();
            Point aControlPos;
            if ( pControlWindow )
                aControlPos = pControlWindow->GetPosPixel();

            // clean up the old control
            lcl_implDisposeControl_nothrow( xControl );

            // set the new control at the line
            rLine.pLine->setControl( _rPropertyData.Control );
            xControl = rLine.pLine->getControl();

            if ( xControl.is() )
                xControl->setControlContext( m_pControlContextImpl.get() );

            // the initial property value
            if ( _rPropertyData.bUnknownValue )
                xControl->setValue( Any() );
            else
                impl_setControlAsPropertyValue( rLine, _rPropertyData.aValue );

            rLine.pLine->SetTitle(_rPropertyData.DisplayName);
            rLine.xHandler = _rPropertyData.xPropertyHandler;

            sal_uInt16 nTextWidth = static_cast<sal_uInt16>(m_aLinesPlayground->GetTextWidth(_rPropertyData.DisplayName));
            if (m_nTheNameSize< nTextWidth)
                m_nTheNameSize = nTextWidth;

            if ( _rPropertyData.HasPrimaryButton )
            {
                if ( !_rPropertyData.PrimaryButtonImageURL.isEmpty() )
                    rLine.pLine->ShowBrowseButton( _rPropertyData.PrimaryButtonImageURL, true );
                else if ( _rPropertyData.PrimaryButtonImage.is() )
                    rLine.pLine->ShowBrowseButton( Image( _rPropertyData.PrimaryButtonImage ), true );
                else
                    rLine.pLine->ShowBrowseButton( true );

                if ( _rPropertyData.HasSecondaryButton )
                {
                    if ( !_rPropertyData.SecondaryButtonImageURL.isEmpty() )
                        rLine.pLine->ShowBrowseButton( _rPropertyData.SecondaryButtonImageURL, false );
                    else if ( _rPropertyData.SecondaryButtonImage.is() )
                        rLine.pLine->ShowBrowseButton( Image( _rPropertyData.SecondaryButtonImage ), false );
                    else
                        rLine.pLine->ShowBrowseButton( false );
                }
                else
                    rLine.pLine->HideBrowseButton( false );

                rLine.pLine->SetClickListener( this );
            }
            else
            {
                rLine.pLine->HideBrowseButton( true );
                rLine.pLine->HideBrowseButton( false );
            }

            DBG_ASSERT( ( _rPropertyData.IndentLevel == 0 ) || ( _rPropertyData.IndentLevel == 1 ),
                "OBrowserListBox::ChangeEntry: unsupported indent level!" );
            rLine.pLine->IndentTitle( _rPropertyData.IndentLevel > 0 );

            if ( nPos > 0 )
                rLine.pLine->SetTabOrder( pRefWindow, ZOrderFlags::Behind );
            else
                rLine.pLine->SetTabOrder( pRefWindow, ZOrderFlags::First );

            m_aOutOfDateLines.insert( nPos );
            rLine.pLine->SetComponentHelpIds(
                HelpIdUrl::getHelpId( _rPropertyData.HelpURL )
            );

            if ( _rPropertyData.bReadOnly )
            {
                rLine.pLine->SetReadOnly( true );

                // user controls (i.e. the ones not provided by the usual
                // XPropertyControlFactory) have no chance to know that they should be read-only,
                // since XPropertyHandler::describePropertyLine does not transport this
                // information.
                // So, we manually switch this to read-only.
                if ( xControl.is() && ( xControl->getControlType() == PropertyControlType::Unknown ) )
                {
                    vcl::Window *pWindow = rLine.pLine->getControlWindow();
                    Edit* pControlWindowAsEdit = dynamic_cast<Edit*>(pWindow);
                    if (pControlWindowAsEdit)
                        pControlWindowAsEdit->SetReadOnly();
                    else
                        pWindow->Enable(false);
                }
            }
        }
    }


    bool OBrowserListBox::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
            case MouseNotifyEvent::KEYINPUT:
            {
                const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
                if  (   ( pKeyEvent->GetKeyCode().GetModifier() != 0 )
                    ||  (   ( pKeyEvent->GetKeyCode().GetCode() != KEY_PAGEUP )
                        &&  ( pKeyEvent->GetKeyCode().GetCode() != KEY_PAGEDOWN )
                        )
                    )
                    break;

                long nScrollOffset = 0;
                if ( m_aVScroll->IsVisible() )
                {
                    if ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEUP )
                        nScrollOffset = -m_aVScroll->GetPageSize();
                    else if ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEDOWN )
                        nScrollOffset = m_aVScroll->GetPageSize();
                }

                if ( nScrollOffset )
                {
                    long nNewThumbPos = m_aVScroll->GetThumbPos() + nScrollOffset;
                    nNewThumbPos = std::max( nNewThumbPos, m_aVScroll->GetRangeMin() );
                    nNewThumbPos = std::min( nNewThumbPos, m_aVScroll->GetRangeMax() );
                    m_aVScroll->DoScroll( nNewThumbPos );
                    nNewThumbPos = m_aVScroll->GetThumbPos();

                    sal_uInt16 nFocusControlPos = 0;
                    sal_uInt16 nActiveControlPos = impl_getControlPos( m_xActiveControl );
                    if ( nActiveControlPos < nNewThumbPos )
                        nFocusControlPos = static_cast<sal_uInt16>(nNewThumbPos);
                    else if ( nActiveControlPos >= nNewThumbPos + CalcVisibleLines() )
                        nFocusControlPos = static_cast<sal_uInt16>(nNewThumbPos) + CalcVisibleLines() - 1;
                    if ( nFocusControlPos )
                    {
                        if ( nFocusControlPos < m_aLines.size() )
                        {
                            m_aLines[ nFocusControlPos ].pLine->GrabFocus();
                        }
                        else
                            OSL_FAIL( "OBrowserListBox::PreNotify: internal error, invalid focus control position!" );
                    }
                }

                return true;
                // handled this. In particular, we also consume PageUp/Down events if we do not use them for scrolling,
                // otherwise they would be used to scroll the document view, which does not sound like it is desired by
                // the user.
            }
            default:
                break;
        }
        return Control::PreNotify( _rNEvt );
    }

    bool OBrowserListBox::EventNotify( NotifyEvent& _rNEvt )
    {
        if ( _rNEvt.GetType() == MouseNotifyEvent::COMMAND)
        {
            const CommandEvent* pCommand = _rNEvt.GetCommandEvent();
            if  (   ( CommandEventId::Wheel == pCommand->GetCommand() )
                ||  ( CommandEventId::StartAutoScroll == pCommand->GetCommand() )
                ||  ( CommandEventId::AutoScroll == pCommand->GetCommand() )
                )
            {
                // interested in scroll events if we have a scrollbar
                if ( m_aVScroll->IsVisible() )
                {
                    HandleScrollCommand( *pCommand, nullptr, m_aVScroll.get() );
                }
            }
        }
        return Control::EventNotify(_rNEvt);
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
