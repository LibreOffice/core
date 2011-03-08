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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "browserlistbox.hxx"
#include "propresid.hrc"
#include "proplinelistener.hxx"
#include "propcontrolobserver.hxx"
#include "linedescriptor.hxx"
#include "inspectorhelpwindow.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
/** === end UNO includes === **/
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/asyncnotification.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    #define FRAME_OFFSET 4
        // TODO: find out what this is really for ... and check if it does make sense in the new
        // browser environment
    #define LAYOUT_HELP_WINDOW_DISTANCE_APPFONT 3

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::graphic::XGraphic;
    /** === end UNO using === **/
    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;

    //==================================================================
    //= ControlEvent
    //==================================================================
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

    //==================================================================
    //= SharedNotifier
    //==================================================================
    class SharedNotifier
    {
    private:
        static ::osl::Mutex&                                        getMutex();
        static ::rtl::Reference< ::comphelper::AsyncEventNotifier > s_pNotifier;

    public:
        static const ::rtl::Reference< ::comphelper::AsyncEventNotifier >&
            getNotifier();

    private:
        SharedNotifier();                                   // never implemented
        SharedNotifier( const SharedNotifier& );            // never implemented
        SharedNotifier& operator=( const SharedNotifier& ); // never implemented
    };

    //------------------------------------------------------------------
    ::rtl::Reference< ::comphelper::AsyncEventNotifier > SharedNotifier::s_pNotifier;

    //------------------------------------------------------------------
    ::osl::Mutex& SharedNotifier::getMutex()
    {
        static ::osl::Mutex s_aMutex;
        return s_aMutex;
    }

    //------------------------------------------------------------------
    const ::rtl::Reference< ::comphelper::AsyncEventNotifier >& SharedNotifier::getNotifier()
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( !s_pNotifier.is() )
        {
            s_pNotifier.set( new ::comphelper::AsyncEventNotifier );
            s_pNotifier->create();
        }
        return s_pNotifier;
    }

    //==================================================================
    //= PropertyControlContext_Impl
    //==================================================================
    /** implementation for of <type scope="com::sun::star::inspection">XPropertyControlContext</type>
        which forwards all events to a non-UNO version of this interface
    */
    typedef ::cppu::WeakImplHelper1< XPropertyControlContext > PropertyControlContext_Impl_Base;
    class PropertyControlContext_Impl   :public PropertyControlContext_Impl_Base
                                        ,public ::comphelper::IEventProcessor
    {
    public:
        enum NotifcationMode
        {
            eSynchronously,
            eAsynchronously
        };

    private:
        IControlContext*    m_pContext;
        NotifcationMode     m_eMode;

    public:
        /** creates an instance
            @param _rContextImpl
                the instance to delegate events to
        */
        PropertyControlContext_Impl( IControlContext& _rContextImpl );

        /** disposes the context.

            When you call this method, all subsequent callbacks to the
            <type scope="com::sun::star::inspection">XPropertyControlContext</type> methods
            will throw a <type scope="com::sun::star::lang">DisposedException</type>.
        */
        void SAL_CALL dispose();

        /** sets the notification mode, so that notifications recieved from the controls are
            forwarded to our IControlContext either synchronously or asynchronously
            @param  _eMode
                the new notification mode
        */
        void setNotificationMode( NotifcationMode _eMode );

        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

    protected:
        ~PropertyControlContext_Impl();

        // XPropertyControlObserver
        virtual void SAL_CALL focusGained( const Reference< XPropertyControl >& Control ) throw (RuntimeException);
        virtual void SAL_CALL valueChanged( const Reference< XPropertyControl >& Control ) throw (RuntimeException);
        // XPropertyControlContext
        virtual void SAL_CALL activateNextControl( const Reference< XPropertyControl >& CurrentControl ) throw (RuntimeException);

        // IEventProcessor
        virtual void processEvent( const ::comphelper::AnyEvent& _rEvent );

    private:
        /** processes the given event, i.e. notifies it to our IControlContext
            @param  _rEvent
                the event no notify
            @precond
                our mutex (well, the SolarMutex) is locked
        */
        void impl_processEvent_throw( const ::comphelper::AnyEvent& _rEvent );

        /** checks whether we're alive

            @throws DisposedException
                if the instance is already disposed
        */
        void impl_checkAlive_throw() const;

        /** checks whether the instance is already disposed
        */
        bool impl_isDisposed_nothrow() const { return m_pContext == NULL; }

        /** notifies the given event originating from the given control
        @throws DisposedException
        @param  _rxControl
        @param  _eType
        */
        void impl_notify_throw( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType );
    };

    //--------------------------------------------------------------------
    PropertyControlContext_Impl::PropertyControlContext_Impl( IControlContext& _rContextImpl )
        :m_pContext( &_rContextImpl )
        ,m_eMode( eAsynchronously )
    {
    }

    //--------------------------------------------------------------------
    PropertyControlContext_Impl::~PropertyControlContext_Impl()
    {
        if ( !impl_isDisposed_nothrow() )
            dispose();
    }

    //--------------------------------------------------------------------
    void PropertyControlContext_Impl::impl_checkAlive_throw() const
    {
        if ( impl_isDisposed_nothrow() )
            throw DisposedException( ::rtl::OUString(), *const_cast< PropertyControlContext_Impl* >( this ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::dispose()
    {
        SolarMutexGuard aGuard;
        if ( impl_isDisposed_nothrow() )
            return;

        SharedNotifier::getNotifier()->removeEventsForProcessor( this );
        m_pContext = NULL;
    }

    //--------------------------------------------------------------------
    void PropertyControlContext_Impl::setNotificationMode( NotifcationMode _eMode )
    {
        SolarMutexGuard aGuard;
        m_eMode = _eMode;
    }

    //--------------------------------------------------------------------
    void PropertyControlContext_Impl::impl_notify_throw( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType )
    {
        ::comphelper::AnyEventRef pEvent;

        {
            SolarMutexGuard aGuard;
            impl_checkAlive_throw();
            pEvent = new ControlEvent( _rxControl, _eType );

            if ( m_eMode == eSynchronously )
            {
                impl_processEvent_throw( *pEvent );
                return;
            }
        }

        SharedNotifier::getNotifier()->addEvent( pEvent, this );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::focusGained( const Reference< XPropertyControl >& Control ) throw (RuntimeException)
    {
        OSL_TRACE( "PropertyControlContext_Impl: FOCUS_GAINED" );
        impl_notify_throw( Control, FOCUS_GAINED );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::valueChanged( const Reference< XPropertyControl >& Control ) throw (RuntimeException)
    {
        OSL_TRACE( "PropertyControlContext_Impl: VALUE_CHANGED" );
        impl_notify_throw( Control, VALUE_CHANGED );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::activateNextControl( const Reference< XPropertyControl >& CurrentControl ) throw (RuntimeException)
    {
        OSL_TRACE( "PropertyControlContext_Impl: ACTIVATE_NEXT" );
        impl_notify_throw( CurrentControl, ACTIVATE_NEXT );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::acquire() throw()
    {
        PropertyControlContext_Impl_Base::acquire();
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyControlContext_Impl::release() throw()
    {
        PropertyControlContext_Impl_Base::release();
    }

    //--------------------------------------------------------------------
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void PropertyControlContext_Impl::impl_processEvent_throw( const ::comphelper::AnyEvent& _rEvent )
    {
        const ControlEvent& rControlEvent = static_cast< const ControlEvent& >( _rEvent );
        switch ( rControlEvent.eType )
        {
        case FOCUS_GAINED:
            OSL_TRACE( "PropertyControlContext_Impl::processEvent: FOCUS_GAINED" );
            m_pContext->focusGained( rControlEvent.xControl );
            break;
        case VALUE_CHANGED:
            OSL_TRACE( "PropertyControlContext_Impl::processEvent: VALUE_CHANGED" );
            m_pContext->valueChanged( rControlEvent.xControl );
            break;
        case ACTIVATE_NEXT:
            OSL_TRACE( "PropertyControlContext_Impl::processEvent: ACTIVATE_NEXT" );
            m_pContext->activateNextControl( rControlEvent.xControl );
            break;
        }
    }

    //==================================================================
    //= OBrowserListBox
    //==================================================================
    DBG_NAME(OBrowserListBox)
    //------------------------------------------------------------------
    OBrowserListBox::OBrowserListBox( Window* pParent, WinBits nWinStyle)
            :Control(pParent, nWinStyle| WB_CLIPCHILDREN)
            ,m_aLinesPlayground(this,WB_DIALOGCONTROL | WB_CLIPCHILDREN)
            ,m_aVScroll(this,WB_VSCROLL|WB_REPEAT|WB_DRAG)
            ,m_pHelpWindow( new InspectorHelpWindow( this ) )
            ,m_pLineListener(NULL)
            ,m_pControlObserver( NULL )
            ,m_nYOffset(0)
            ,m_nCurrentPreferredHelpHeight(0)
            ,m_nTheNameSize(0)
            ,m_bIsActive(sal_False)
            ,m_bUpdate(sal_True)
            ,m_pControlContextImpl( new PropertyControlContext_Impl( *this ) )
    {
        DBG_CTOR(OBrowserListBox,NULL);

        ListBox aListBox(this,WB_DROPDOWN);
        aListBox.SetPosSizePixel(Point(0,0),Size(100,100));
        m_nRowHeight = (sal_uInt16)aListBox.GetSizePixel().Height()+2;
        SetBackground( pParent->GetBackground() );
        m_aLinesPlayground.SetBackground( GetBackground() );

        m_aLinesPlayground.SetPosPixel(Point(0,0));
        m_aLinesPlayground.SetPaintTransparent(sal_True);
        m_aLinesPlayground.Show();
        m_aVScroll.Hide();
        m_aVScroll.SetScrollHdl(LINK(this, OBrowserListBox, ScrollHdl));
    }

    //------------------------------------------------------------------
    OBrowserListBox::~OBrowserListBox()
    {
        OSL_ENSURE( !IsModified(), "OBrowserListBox::~OBrowserListBox: still modified - should have been committed before!" );
            // doing the commit here, while we, as well as our owner, as well as some other components,
            // are already "half dead" (means within their dtor) is potentially dangerous.
            // By definition, CommitModified has to be called (if necessary) before destruction

        m_pControlContextImpl->dispose();
        m_pControlContextImpl.clear();

        Hide();
        Clear();

        DBG_DTOR(OBrowserListBox,NULL);
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserListBox::IsModified( ) const
    {
        sal_Bool bModified = sal_False;

        if ( m_bIsActive && m_xActiveControl.is() )
            bModified = m_xActiveControl->isModified();

        return bModified;
    }

    //------------------------------------------------------------------
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
                DBG_UNHANDLED_EXCEPTION();
            }
            m_pControlContextImpl->setNotificationMode( PropertyControlContext_Impl::eAsynchronously );
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ActivateListBox(sal_Bool _bActive)
    {
        m_bIsActive = _bActive;
        if (m_bIsActive)
        {
            // TODO: what's the sense of this?
            m_aVScroll.SetThumbPos(100);
            MoveThumbTo(0);
            Resize();
        }
    }

    //------------------------------------------------------------------
    long OBrowserListBox::impl_getPrefererredHelpHeight()
    {
        return HasHelpSection() ? m_pHelpWindow->GetOptimalHeightPixel() : 0;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Resize()
    {
        Rectangle aPlayground( Point( 0, 0 ), GetOutputSizePixel() );
        Size aHelpWindowDistance( LogicToPixel( Size( 0, LAYOUT_HELP_WINDOW_DISTANCE_APPFONT ), MAP_APPFONT ) );

        long nHelpWindowHeight = m_nCurrentPreferredHelpHeight = impl_getPrefererredHelpHeight();
        bool bPositionHelpWindow = ( nHelpWindowHeight != 0 );

        Rectangle aLinesArea( aPlayground );
        if ( bPositionHelpWindow )
        {
            aLinesArea.Bottom() -= nHelpWindowHeight;
            aLinesArea.Bottom() -= aHelpWindowDistance.Height();
        }
        m_aLinesPlayground.SetPosSizePixel( aLinesArea.TopLeft(), aLinesArea.GetSize() );

        UpdateVScroll();

        sal_Bool bNeedScrollbar = m_aOrderedLines.size() > (sal_uInt32)CalcVisibleLines();
        if ( !bNeedScrollbar )
        {
            if ( m_aVScroll.IsVisible() )
                m_aVScroll.Hide();
            // scroll to top
            m_nYOffset = 0;
            m_aVScroll.SetThumbPos( 0 );
        }
        else
        {
            Size aVScrollSize( m_aVScroll.GetSizePixel() );

            // adjust the playground's width
            aLinesArea.Right() -= aVScrollSize.Width();
            m_aLinesPlayground.SetPosSizePixel( aLinesArea.TopLeft(), aLinesArea.GetSize() );

            // position the scrollbar
            aVScrollSize.Height() = aLinesArea.GetHeight();
            Point aVScrollPos( aLinesArea.GetWidth(), 0 );
            m_aVScroll.SetPosSizePixel( aVScrollPos, aVScrollSize );
        }

        for ( sal_uInt16 i = 0; i < m_aOrderedLines.size(); ++i )
            m_aOutOfDateLines.insert( i );

        // repaint
        EnablePaint(sal_False);
        UpdatePlayGround();
        EnablePaint(sal_True);

        // show the scrollbar
        if ( bNeedScrollbar )
            m_aVScroll.Show();

        // position the help window
        if ( bPositionHelpWindow )
        {
            Rectangle aHelpArea( aPlayground );
            aHelpArea.Top() = aLinesArea.Bottom() + aHelpWindowDistance.Height();
            m_pHelpWindow->SetPosSizePixel( aHelpArea.TopLeft(), aHelpArea.GetSize() );
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetListener( IPropertyLineListener* _pListener )
    {
        m_pLineListener = _pListener;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetObserver( IPropertyControlObserver* _pObserver )
    {
        m_pControlObserver = _pObserver;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::EnableHelpSection( bool _bEnable )
    {
        m_pHelpWindow->Show( _bEnable );
        Resize();
    }

    //------------------------------------------------------------------
    bool OBrowserListBox::HasHelpSection() const
    {
        return m_pHelpWindow->IsVisible();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetHelpText( const ::rtl::OUString& _rHelpText )
    {
        OSL_ENSURE( HasHelpSection(), "OBrowserListBox::SetHelpText: help section not visible!" );
        m_pHelpWindow->SetText( _rHelpText );
        if ( m_nCurrentPreferredHelpHeight != impl_getPrefererredHelpHeight() )
            Resize();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetHelpLineLimites( sal_Int32 _nMinLines, sal_Int32 _nMaxLines )
    {
        m_pHelpWindow->SetLimits( _nMinLines, _nMaxLines );
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::CalcVisibleLines()
    {
        Size aSize(m_aLinesPlayground.GetOutputSizePixel());
        sal_uInt16 nResult = 0;
        if (0 != m_nRowHeight)
            nResult = (sal_uInt16) aSize.Height()/m_nRowHeight;

        return nResult;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdateVScroll()
    {
        sal_uInt16 nLines = CalcVisibleLines();
        m_aVScroll.SetPageSize(nLines-1);
        m_aVScroll.SetVisibleSize(nLines-1);

        size_t nCount = m_aLines.size();
        if (nCount>0)
        {
            m_aVScroll.SetRange(Range(0,nCount-1));
            m_nYOffset = -m_aVScroll.GetThumbPos()*m_nRowHeight;
        }
        else
        {
            m_aVScroll.SetRange(Range(0,0));
            m_nYOffset = 0;
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::PositionLine( sal_uInt16 _nIndex )
    {
        Size aSize(m_aLinesPlayground.GetOutputSizePixel());
        Point aPos(0, m_nYOffset);

        aSize.Height() = m_nRowHeight;

        aPos.Y() += _nIndex * m_nRowHeight;

        if ( _nIndex < m_aOrderedLines.size() )
        {
            m_aOrderedLines[ _nIndex ]->second.pLine->SetPosSizePixel( aPos, aSize );

            m_aOrderedLines[ _nIndex ]->second.pLine->SetTitleWidth( m_nTheNameSize + 2 * FRAME_OFFSET );

            // show the line if necessary
            if ( !m_aOrderedLines[ _nIndex ]->second.pLine->IsVisible() )
                m_aOrderedLines[ _nIndex ]->second.pLine->Show();
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdatePosNSize()
    {
        for  (  ::std::set< sal_uInt16 >::const_iterator aLoop = m_aOutOfDateLines.begin();
                aLoop != m_aOutOfDateLines.end();
                ++aLoop
             )
        {
            DBG_ASSERT( *aLoop < m_aOrderedLines.size(), "OBrowserListBox::UpdatePosNSize: invalid line index!" );
            if ( *aLoop < m_aOrderedLines.size() )
                PositionLine( *aLoop );
        }
        m_aOutOfDateLines.clear();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdatePlayGround()
    {
        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();
        sal_Int32 nLines = CalcVisibleLines();

        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + nLines);
        if (nEnd >= m_aOrderedLines.size())
            nEnd = (sal_uInt16)m_aOrderedLines.size()-1;

        if ( !m_aOrderedLines.empty() )
        {
            for ( sal_uInt16 i = (sal_uInt16)nThumbPos; i <= nEnd; ++i )
                m_aOutOfDateLines.insert( i );
            UpdatePosNSize();
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::UpdateAll()
    {
        Resize();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::DisableUpdate()
    {
        m_bUpdate = sal_False;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::EnableUpdate()
    {
        m_bUpdate = sal_True;
        UpdateAll();
    }

    //------------------------------------------------------------------
    void OBrowserListBox::SetPropertyValue(const ::rtl::OUString& _rEntryName, const Any& _rValue, bool _bUnknownValue )
    {
        ListBoxLines::iterator line = m_aLines.find( _rEntryName );
        if ( line != m_aLines.end() )
        {
            if ( _bUnknownValue )
            {
                Reference< XPropertyControl > xControl( line->second.pLine->getControl() );
                OSL_ENSURE( xControl.is(), "OBrowserListBox::SetPropertyValue: illegal control!" );
                if ( xControl.is() )
                    xControl->setValue( Any() );
            }
            else
                impl_setControlAsPropertyValue( line->second, _rValue );
        }
    }

    //------------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::GetPropertyPos( const ::rtl::OUString& _rEntryName ) const
    {
        sal_uInt16 nRet = LISTBOX_ENTRY_NOTFOUND;
        for ( OrderedListBoxLines::const_iterator linePos = m_aOrderedLines.begin();
              linePos != m_aOrderedLines.end();
              ++linePos
            )
        {
            if ( (*linePos)->first == _rEntryName )
            {
                nRet = (sal_uInt16)( linePos - m_aOrderedLines.begin() );
                break;
            }
        }

        return nRet;
    }

    //------------------------------------------------------------------------
    bool OBrowserListBox::impl_getBrowserLineForName( const ::rtl::OUString& _rEntryName, BrowserLinePointer& _out_rpLine ) const
    {
        ListBoxLines::const_iterator line = m_aLines.find( _rEntryName );
        if ( line != m_aLines.end() )
            _out_rpLine = line->second.pLine;
        else
            _out_rpLine.reset();
        return ( NULL != _out_rpLine.get() );
    }

    //------------------------------------------------------------------------
    void OBrowserListBox::EnablePropertyControls( const ::rtl::OUString& _rEntryName, sal_Int16 _nControls, bool _bEnable )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            pLine->EnablePropertyControls( _nControls, _bEnable );
    }

    //------------------------------------------------------------------------
    void OBrowserListBox::EnablePropertyLine( const ::rtl::OUString& _rEntryName, bool _bEnable )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            pLine->EnablePropertyLine( _bEnable );
    }

    //------------------------------------------------------------------------
    Reference< XPropertyControl > OBrowserListBox::GetPropertyControl( const ::rtl::OUString& _rEntryName )
    {
        BrowserLinePointer pLine;
        if ( impl_getBrowserLineForName( _rEntryName, pLine ) )
            return pLine->getControl();
        return NULL;
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::InsertEntry(const OLineDescriptor& _rPropertyData, sal_uInt16 _nPos)
    {
        // create a new line
        BrowserLinePointer pBrowserLine( new OBrowserLine( _rPropertyData.sName, &m_aLinesPlayground ) );

        ListBoxLine aNewLine( pBrowserLine, _rPropertyData.xPropertyHandler );
        ::std::pair< ListBoxLines::iterator, bool > insertPoint =
            m_aLines.insert( ListBoxLines::value_type( _rPropertyData.sName, aNewLine ) );
        OSL_ENSURE( insertPoint.second, "OBrowserListBox::InsertEntry: already have another line for this name!" );

        sal_uInt16 nInsertPos = _nPos;
        if ( nInsertPos > m_aOrderedLines.size() )
            nInsertPos = EDITOR_LIST_APPEND;
        if ( EDITOR_LIST_APPEND == nInsertPos )
        {
            nInsertPos = (sal_uInt16)m_aOrderedLines.size();
            m_aOrderedLines.push_back( insertPoint.first );
        }
        else
            m_aOrderedLines.insert( m_aOrderedLines.begin() + nInsertPos, insertPoint.first );

        pBrowserLine->SetTitleWidth(m_nTheNameSize);
        if (m_bUpdate)
        {
            UpdateVScroll();
            Invalidate();
        }

        // initialize the entry
        ChangeEntry(_rPropertyData, nInsertPos);

        // update the positions of possibly affected lines
        sal_uInt16 nUpdatePos = nInsertPos;
        while ( nUpdatePos < m_aOrderedLines.size() )
            m_aOutOfDateLines.insert( nUpdatePos++ );
        UpdatePosNSize( );

        return nInsertPos;
    }

    //------------------------------------------------------------------
    sal_Int32 OBrowserListBox::GetMinimumWidth()
    {
        return m_nTheNameSize + 2 * FRAME_OFFSET + (m_nRowHeight - 4) * 8;
    }

    //------------------------------------------------------------------
    sal_Int32 OBrowserListBox::GetMinimumHeight()
    {
        // assume that we want to display 5 rows, at least
        sal_Int32 nMinHeight = m_nRowHeight * 5;

        if ( HasHelpSection() )
        {
            Size aHelpWindowDistance( LogicToPixel( Size( 0, LAYOUT_HELP_WINDOW_DISTANCE_APPFONT ), MAP_APPFONT ) );
            nMinHeight += aHelpWindowDistance.Height();

            nMinHeight += m_pHelpWindow->GetMinimalHeightPixel();
        }

        return nMinHeight;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ShowEntry(sal_uInt16 _nPos)
    {
        if ( _nPos < m_aOrderedLines.size() )
        {
            sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();

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

    //------------------------------------------------------------------
    void OBrowserListBox::MoveThumbTo(sal_Int32 _nNewThumbPos)
    {
        // disable painting to prevent flicker
        m_aLinesPlayground.EnablePaint(sal_False);

        sal_Int32 nDelta = _nNewThumbPos - m_aVScroll.GetThumbPos();
        // adjust the scrollbar
        m_aVScroll.SetThumbPos(_nNewThumbPos);
        sal_Int32 nThumbPos = _nNewThumbPos;

        m_nYOffset = -m_aVScroll.GetThumbPos() * m_nRowHeight;

        sal_Int32 nLines = CalcVisibleLines();
        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + nLines);

        m_aLinesPlayground.Scroll(0, -nDelta * m_nRowHeight, SCROLL_CHILDREN);

        if (1 == nDelta)
        {
            // TODO: what's the sense of this two PositionLines? Why not just one call?
            PositionLine(nEnd-1);
            PositionLine(nEnd);
        }
        else if (-1 == nDelta)
        {
            PositionLine((sal_uInt16)nThumbPos);
        }
        else if (0 != nDelta)
        {
            UpdatePlayGround();
        }

        m_aLinesPlayground.EnablePaint(sal_True);
        m_aLinesPlayground.Invalidate(INVALIDATE_CHILDREN);
    }

    //------------------------------------------------------------------
    IMPL_LINK(OBrowserListBox, ScrollHdl, ScrollBar*, _pScrollBar )
    {
        DBG_ASSERT(_pScrollBar == &m_aVScroll, "OBrowserListBox::ScrollHdl: where does this come from?");
        (void)_pScrollBar;

        // disable painting to prevent flicker
        m_aLinesPlayground.EnablePaint(sal_False);

        sal_Int32 nThumbPos = m_aVScroll.GetThumbPos();

        sal_Int32 nDelta = m_aVScroll.GetDelta();
        m_nYOffset = -nThumbPos * m_nRowHeight;

        sal_uInt16 nEnd = (sal_uInt16)(nThumbPos + CalcVisibleLines());

        m_aLinesPlayground.Scroll(0, -nDelta * m_nRowHeight, SCROLL_CHILDREN);

        if (1 == nDelta)
        {
            PositionLine(nEnd-1);
            PositionLine(nEnd);
        }
        else if (nDelta==-1)
        {
            PositionLine((sal_uInt16)nThumbPos);
        }
        else if (nDelta!=0 || m_aVScroll.GetType() == SCROLL_DONTKNOW)
        {
            UpdatePlayGround();
        }

        m_aLinesPlayground.EnablePaint(sal_True);
        return 0;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::buttonClicked( OBrowserLine* _pLine, sal_Bool _bPrimary )
    {
        DBG_ASSERT( _pLine, "OBrowserListBox::buttonClicked: invalid browser line!" );
        if ( _pLine && m_pLineListener )
        {
            m_pLineListener->Clicked( _pLine->GetEntryName(), _bPrimary );
        }
    }

    //------------------------------------------------------------------
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
    #ifdef DBG_UTIL
                if ( !_rLine.xHandler.is() )
                {
                    ::rtl::OString sMessage( "OBrowserListBox::impl_setControlAsPropertyValue: no handler -> no conversion (property: '" );
                    ::rtl::OUString sPropertyName( _rLine.pLine->GetEntryName() );
                    sMessage += ::rtl::OString( sPropertyName.getStr(), sPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += ::rtl::OString( "')!" );
                    OSL_FAIL( sMessage.getStr() );
                }
    #endif
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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------
    Any OBrowserListBox::impl_getControlAsPropertyValue( const ListBoxLine& _rLine ) const
    {
        Reference< XPropertyControl > xControl( _rLine.pLine->getControl() );
        Any aPropertyValue;
        try
        {
        #ifdef DBG_UTIL
            if ( !_rLine.xHandler.is() )
            {
                ::rtl::OString sMessage( "OBrowserListBox::impl_getControlAsPropertyValue: no handler -> no conversion (property: '" );
                ::rtl::OUString sPropertyName( _rLine.pLine->GetEntryName() );
                sMessage += ::rtl::OString( sPropertyName.getStr(), sPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US );
                sMessage += ::rtl::OString( "')!" );
                OSL_FAIL( sMessage.getStr() );
            }
        #endif
            if ( _rLine.xHandler.is() )
                aPropertyValue = _rLine.xHandler->convertToPropertyValue( _rLine.pLine->GetEntryName(), xControl->getValue() );
            else
                aPropertyValue = xControl->getValue();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aPropertyValue;
    }

    //------------------------------------------------------------------
    sal_uInt16 OBrowserListBox::impl_getControlPos( const Reference< XPropertyControl >& _rxControl ) const
    {
        for (   OrderedListBoxLines::const_iterator search = m_aOrderedLines.begin();
                search != m_aOrderedLines.end();
                ++search
            )
            if ( (*search)->second.pLine->getControl().get() == _rxControl.get() )
                return sal_uInt16( search - m_aOrderedLines.begin() );
        OSL_FAIL( "OBrowserListBox::impl_getControlPos: invalid control - not part of any of our lines!" );
        return (sal_uInt16)-1;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OBrowserListBox::focusGained( const Reference< XPropertyControl >& _rxControl ) throw (RuntimeException)
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

    //--------------------------------------------------------------------
    void SAL_CALL OBrowserListBox::valueChanged( const Reference< XPropertyControl >& _rxControl ) throw (RuntimeException)
    {
        DBG_TESTSOLARMUTEX();

        DBG_ASSERT( _rxControl.is(), "OBrowserListBox::valueChanged: invalid event source!" );
        if ( !_rxControl.is() )
            return;

        if ( m_pControlObserver )
            m_pControlObserver->valueChanged( _rxControl );

        if ( m_pLineListener )
        {
            const ListBoxLine& rLine = impl_getControlLine( _rxControl );
            m_pLineListener->Commit(
                rLine.pLine->GetEntryName(),
                impl_getControlAsPropertyValue( rLine )
            );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OBrowserListBox::activateNextControl( const Reference< XPropertyControl >& _rxCurrentControl ) throw (RuntimeException)
    {
        DBG_TESTSOLARMUTEX();

        sal_uInt16 nLine = impl_getControlPos( _rxCurrentControl );

        // cycle forwards, 'til we've the next control which can grab the focus
        ++nLine;
        while ( (size_t)nLine < m_aOrderedLines.size() )
        {
            if ( m_aOrderedLines[nLine]->second.pLine->GrabFocus() )
                break;
            ++nLine;
        }

        if  (   ( (size_t)nLine >= m_aOrderedLines.size() )
            &&  ( m_aOrderedLines.size() > 0 )
            )
            // wrap around
            m_aOrderedLines[0]->second.pLine->GrabFocus();
    }

    //------------------------------------------------------------------
    namespace
    {
        //..............................................................
        void lcl_implDisposeControl_nothrow( const Reference< XPropertyControl >& _rxControl )
        {
            if ( !_rxControl.is() )
                return;
            try
            {
                _rxControl->setControlContext( NULL );
                Reference< XComponent > xControlComponent( _rxControl, UNO_QUERY );
                if ( xControlComponent.is() )
                    xControlComponent->dispose();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //------------------------------------------------------------------
    void OBrowserListBox::Clear()
    {
        for (   ListBoxLines::iterator loop = m_aLines.begin();
                loop != m_aLines.end();
                ++loop
            )
        {
            // hide the line
            loop->second.pLine->Hide();
            // reset the listener
            lcl_implDisposeControl_nothrow( loop->second.pLine->getControl() );
        }

        clearContainer( m_aLines );
        clearContainer( m_aOrderedLines );
    }

    //------------------------------------------------------------------
    sal_Bool OBrowserListBox::RemoveEntry( const ::rtl::OUString& _rName )
    {
        sal_uInt16 nPos = GetPropertyPos( _rName );
        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
            return sal_False;

        OrderedListBoxLines::iterator orderedPos = m_aOrderedLines.begin() + nPos;
        BrowserLinePointer pLine = (*orderedPos)->second.pLine;
        pLine->Hide();
        lcl_implDisposeControl_nothrow( pLine->getControl() );

        m_aLines.erase( *orderedPos );
        m_aOrderedLines.erase( orderedPos );
        m_aOutOfDateLines.erase( (sal_uInt16)m_aOrderedLines.size() );
            // this index *may* have been out of date, which is obsoleted now by m_aOrderedLines shrinking

        // update the positions of possibly affected lines
        while ( nPos < m_aOrderedLines.size() )
            m_aOutOfDateLines.insert( nPos++ );
        UpdatePosNSize( );

        return sal_True;
    }

    //------------------------------------------------------------------
    void OBrowserListBox::ChangeEntry( const OLineDescriptor& _rPropertyData, sal_uInt16 nPos )
    {
        OSL_PRECOND( _rPropertyData.Control.is(), "OBrowserListBox::ChangeEntry: invalid control!" );
        if ( !_rPropertyData.Control.is() )
            return;

        if ( nPos == EDITOR_LIST_REPLACE_EXISTING )
            nPos = GetPropertyPos( _rPropertyData.sName );

        if ( nPos < m_aOrderedLines.size() )
        {
            Window* pRefWindow = NULL;
            if ( nPos > 0 )
                pRefWindow = m_aOrderedLines[nPos-1]->second.pLine->GetRefWindow();

            // the current line and control
            ListBoxLine& rLine = m_aOrderedLines[nPos]->second;

            // the old control and some data about it
            Reference< XPropertyControl > xControl = rLine.pLine->getControl();
            Window* pControlWindow = rLine.pLine->getControlWindow();
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

            sal_uInt16 nTextWidth = (sal_uInt16)m_aLinesPlayground.GetTextWidth(_rPropertyData.DisplayName);
            if (m_nTheNameSize< nTextWidth)
                m_nTheNameSize = nTextWidth;

            if ( _rPropertyData.HasPrimaryButton )
            {
                if ( _rPropertyData.PrimaryButtonImageURL.getLength() )
                    rLine.pLine->ShowBrowseButton( _rPropertyData.PrimaryButtonImageURL, true );
                else if ( _rPropertyData.PrimaryButtonImage.is() )
                    rLine.pLine->ShowBrowseButton( Image( _rPropertyData.PrimaryButtonImage ), true );
                else
                    rLine.pLine->ShowBrowseButton( true );

                if ( _rPropertyData.HasSecondaryButton )
                {
                    if ( _rPropertyData.SecondaryButtonImageURL.getLength() )
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
                rLine.pLine->SetTabOrder( pRefWindow, WINDOW_ZORDER_BEHIND );
            else
                rLine.pLine->SetTabOrder( pRefWindow, WINDOW_ZORDER_FIRST );

            m_aOutOfDateLines.insert( nPos );
            rLine.pLine->SetComponentHelpIds(
                HelpIdUrl::getHelpId( _rPropertyData.HelpURL ),
                _rPropertyData.PrimaryButtonId,
                _rPropertyData.SecondaryButtonId
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
                    Edit* pControlWindowAsEdit = dynamic_cast< Edit* >( rLine.pLine->getControlWindow() );
                    if ( pControlWindowAsEdit )
                        pControlWindowAsEdit->SetReadOnly( TRUE );
                    else
                        pControlWindowAsEdit->Enable( FALSE );
                }
            }
        }
    }

    //------------------------------------------------------------------
    long OBrowserListBox::PreNotify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
            if  (   ( pKeyEvent->GetKeyCode().GetModifier() != 0 )
                ||  (   ( pKeyEvent->GetKeyCode().GetCode() != KEY_PAGEUP )
                    &&  ( pKeyEvent->GetKeyCode().GetCode() != KEY_PAGEDOWN )
                    )
                )
                break;

            long nScrollOffset = 0;
            if ( m_aVScroll.IsVisible() )
            {
                if ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEUP )
                    nScrollOffset = -m_aVScroll.GetPageSize();
                else if ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEDOWN )
                    nScrollOffset = m_aVScroll.GetPageSize();
            }

            if ( nScrollOffset )
            {
                long nNewThumbPos = m_aVScroll.GetThumbPos() + nScrollOffset;
                nNewThumbPos = ::std::max( nNewThumbPos, m_aVScroll.GetRangeMin() );
                nNewThumbPos = ::std::min( nNewThumbPos, m_aVScroll.GetRangeMax() );
                m_aVScroll.DoScroll( nNewThumbPos );
                nNewThumbPos = m_aVScroll.GetThumbPos();

                sal_uInt16 nFocusControlPos = 0;
                sal_uInt16 nActiveControlPos = impl_getControlPos( m_xActiveControl );
                if ( nActiveControlPos < nNewThumbPos )
                    nFocusControlPos = (sal_uInt16)nNewThumbPos;
                else if ( nActiveControlPos >= nNewThumbPos + CalcVisibleLines() )
                    nFocusControlPos = (sal_uInt16)nNewThumbPos + CalcVisibleLines() - 1;
                if ( nFocusControlPos )
                {
                    if ( nFocusControlPos < m_aOrderedLines.size() )
                    {
                        m_aOrderedLines[ nFocusControlPos ]->second.pLine->GrabFocus();
                    }
                    else
                        OSL_ENSURE( false, "OBrowserListBox::PreNotify: internal error, invalid focus control position!" );
                }
            }

            return 1L;
            // handled this. In particular, we also consume PageUp/Down events if we do not use them for scrolling,
            // otherwise they would be used to scroll the document view, which does not sound like it is desired by
            // the user.
        }
        }
        return Control::PreNotify( _rNEvt );
    }

    //------------------------------------------------------------------
    long OBrowserListBox::Notify( NotifyEvent& _rNEvt )
    {
        switch ( _rNEvt.GetType() )
        {
        case EVENT_COMMAND:
        {
            const CommandEvent* pCommand = _rNEvt.GetCommandEvent();
            if  (   ( COMMAND_WHEEL == pCommand->GetCommand() )
                ||  ( COMMAND_STARTAUTOSCROLL == pCommand->GetCommand() )
                ||  ( COMMAND_AUTOSCROLL == pCommand->GetCommand() )
                )
            {
                // interested in scroll events if we have a scrollbar
                if ( m_aVScroll.IsVisible() )
                {
                    HandleScrollCommand( *pCommand, NULL, &m_aVScroll );
                }
            }
        }
        break;
        }

        return Control::Notify( _rNEvt );
    }

//............................................................................
} // namespace pcr
//............................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
