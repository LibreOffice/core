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
#include "pcrcommon.hxx"
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
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>


namespace pcr
{
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::inspection::XPropertyControlContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::inspection::XPropertyControl;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::uno::UNO_QUERY;

    namespace PropertyControlType = ::com::sun::star::inspection::PropertyControlType;

    namespace {

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

    }

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
        OBrowserListBox* m_pContext;
        NotificationMode m_eMode;

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
        bool impl_isDisposed_nothrow() const { return m_pContext == nullptr; }

        /** notifies the given event originating from the given control
        @throws DisposedException
        @param  _rxControl
        @param  _eType
        */
        void impl_notify_throw( const Reference< XPropertyControl >& _rxControl, ControlEventType _eType );
    };

    PropertyControlContext_Impl::PropertyControlContext_Impl( OBrowserListBox& _rContextImpl )
        : m_pContext( &_rContextImpl )
        , m_eMode( eAsynchronously )
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

    OBrowserListBox::OBrowserListBox(weld::Builder& rBuilder, weld::Container* pContainer)
        : m_xScrolledWindow(rBuilder.weld_scrolled_window("scrolledwindow"))
        , m_xLinesPlayground(rBuilder.weld_container("playground"))
        , m_xSizeGroup(rBuilder.create_size_group())
        , m_xHelpWindow(new InspectorHelpWindow(rBuilder))
        , m_pInitialControlParent(pContainer)
        , m_pLineListener(nullptr)
        , m_pControlObserver( nullptr )
        , m_nTheNameSize(0)
        , m_nRowHeight(0)
        , m_pControlContextImpl( new PropertyControlContext_Impl( *this ) )
    {
        m_xScrolledWindow->set_size_request(-1, m_xScrolledWindow->get_text_height() * 20);
    }

    OBrowserListBox::~OBrowserListBox()
    {
        OSL_ENSURE( !IsModified(), "OBrowserListBox::~OBrowserListBox: still modified - should have been committed before!" );

        // doing the commit here, while we, as well as our owner, as well as some other components,
        // are already "half dead" (means within their dtor) is potentially dangerous.
        // By definition, CommitModified has to be called (if necessary) before destruction
        m_pControlContextImpl->dispose();
        m_pControlContextImpl.clear();

        Clear();
    }

    bool OBrowserListBox::IsModified() const
    {
        bool bModified = false;

        if (m_xScrolledWindow->get_visible() && m_xActiveControl.is())
            bModified = m_xActiveControl->isModified();

        return bModified;
    }

    void OBrowserListBox::CommitModified( )
    {
        if ( !(IsModified() && m_xActiveControl.is()) )
            return;

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
        m_xHelpWindow->Show( _bEnable );
    }

    bool OBrowserListBox::HasHelpSection() const
    {
        return m_xHelpWindow->IsVisible();
    }

    void OBrowserListBox::SetHelpText( const OUString& _rHelpText )
    {
        OSL_ENSURE( HasHelpSection(), "OBrowserListBox::SetHelpText: help section not visible!" );
        m_xHelpWindow->SetText( _rHelpText );
    }

    void OBrowserListBox::UpdatePlayGround()
    {
        for (auto& line : m_aLines)
            line.pLine->SetTitleWidth(m_nTheNameSize);
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

    sal_uInt16 OBrowserListBox::GetPropertyPos( std::u16string_view _rEntryName ) const
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
        return bool(_out_rpLine);
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

    void OBrowserListBox::InsertEntry(const OLineDescriptor& rPropertyData, sal_uInt16 _nPos)
    {
        // create a new line
        BrowserLinePointer pBrowserLine = std::make_shared<OBrowserLine>(rPropertyData.sName, m_xLinesPlayground.get(),
                                                         m_xSizeGroup.get(), m_pInitialControlParent);

        // check that the name is unique
        for (auto const& line : m_aLines)
        {
            if (line.aName == rPropertyData.sName)
            {
                // already have another line for this name!
                assert(false);
            }
        }

        ListBoxLine aNewLine( rPropertyData.sName, pBrowserLine, rPropertyData.xPropertyHandler );
        ListBoxLines::size_type nInsertPos = _nPos;
        if ( _nPos >= m_aLines.size() )
        {
            nInsertPos = m_aLines.size();
            m_aLines.push_back( aNewLine );
        }
        else
            m_aLines.insert( m_aLines.begin() + _nPos, aNewLine );

        pBrowserLine->SetTitleWidth(m_nTheNameSize);

        // initialize the entry
        ChangeEntry(rPropertyData, nInsertPos);

        m_nRowHeight = std::max(m_nRowHeight, pBrowserLine->GetRowHeight() + 6); // 6 is spacing of the "playground" in browserpage.ui
        m_xScrolledWindow->vadjustment_set_step_increment(m_nRowHeight);
    }

    void OBrowserListBox::ShowEntry(sal_uInt16 nPos)
    {
        if (nPos == 0)
        {
            // special case the simple entry 0 situation
            m_xScrolledWindow->vadjustment_set_value(0);
            return;
        }

        if (nPos >= m_aLines.size())
            return;

        unsigned const nWinHeight = m_xScrolledWindow->vadjustment_get_page_size();

        auto nThumbPos = m_xScrolledWindow->vadjustment_get_value();
        int const nWinTop = nThumbPos;
        int const nWinBottom = nWinTop + nWinHeight;

        auto nCtrlPosY = nPos * m_nRowHeight;

        int const nSelectedItemTop = nCtrlPosY;
        int const nSelectedItemBottom = nCtrlPosY + m_nRowHeight;
        bool const shouldScrollDown = nSelectedItemBottom >= nWinBottom;
        bool const shouldScrollUp = nSelectedItemTop <= nWinTop;
        bool const isNeedToScroll = shouldScrollDown || shouldScrollUp;

        if (!isNeedToScroll)
            return;

        if (shouldScrollDown)
        {
            int nOffset = nSelectedItemBottom - nWinBottom;
            nThumbPos += nOffset;
        }
        else
        {
            int nOffset = nWinTop - nSelectedItemTop;
            nThumbPos -= nOffset;
            if(nThumbPos < 0)
                nThumbPos = 0;
        }
        m_xScrolledWindow->vadjustment_set_value(nThumbPos);
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

        m_aLines.erase( it );

        return true;
    }

    void OBrowserListBox::ChangeEntry( const OLineDescriptor& rPropertyData, ListBoxLines::size_type nPos )
    {
        OSL_PRECOND( rPropertyData.Control.is(), "OBrowserListBox::ChangeEntry: invalid control!" );
        if ( !rPropertyData.Control.is() )
            return;

        if ( nPos == EDITOR_LIST_REPLACE_EXISTING )
            nPos = GetPropertyPos( rPropertyData.sName );

        if ( nPos >= m_aLines.size() )
            return;

        // the current line and control
        ListBoxLine& rLine = m_aLines[nPos];

        // the old control and some data about it
        Reference< XPropertyControl > xControl = rLine.pLine->getControl();

        // clean up the old control
        lcl_implDisposeControl_nothrow( xControl );

        // set the new control at the line
        rLine.pLine->setControl( rPropertyData.Control );
        xControl = rLine.pLine->getControl();

        if ( xControl.is() )
            xControl->setControlContext( m_pControlContextImpl );

        // the initial property value
        if ( rPropertyData.bUnknownValue )
            xControl->setValue( Any() );
        else
            impl_setControlAsPropertyValue( rLine, rPropertyData.aValue );

        rLine.pLine->SetTitle(rPropertyData.DisplayName);
        rLine.xHandler = rPropertyData.xPropertyHandler;

        if ( rPropertyData.HasPrimaryButton )
        {
            if ( !rPropertyData.PrimaryButtonImageURL.isEmpty() )
                rLine.pLine->ShowBrowseButton( rPropertyData.PrimaryButtonImageURL, true );
            else if ( rPropertyData.PrimaryButtonImage.is() )
                rLine.pLine->ShowBrowseButton( rPropertyData.PrimaryButtonImage, true );
            else
                rLine.pLine->ShowBrowseButton( true );

            if ( rPropertyData.HasSecondaryButton )
            {
                if ( !rPropertyData.SecondaryButtonImageURL.isEmpty() )
                    rLine.pLine->ShowBrowseButton( rPropertyData.SecondaryButtonImageURL, false );
                else if ( rPropertyData.SecondaryButtonImage.is() )
                    rLine.pLine->ShowBrowseButton( rPropertyData.SecondaryButtonImage, false );
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

        DBG_ASSERT( ( rPropertyData.IndentLevel == 0 ) || ( rPropertyData.IndentLevel == 1 ),
            "OBrowserListBox::ChangeEntry: unsupported indent level!" );
        rLine.pLine->IndentTitle( rPropertyData.IndentLevel > 0 );

        rLine.pLine->SetComponentHelpIds(
            HelpIdUrl::getHelpId( rPropertyData.HelpURL )
        );

        if ( rPropertyData.bReadOnly )
        {
            rLine.pLine->SetReadOnly( true );

            // user controls (i.e. the ones not provided by the usual
            // XPropertyControlFactory) have no chance to know that they should be read-only,
            // since XPropertyHandler::describePropertyLine does not transport this
            // information.
            // So, we manually switch this to read-only.
            if ( xControl.is() && ( xControl->getControlType() == PropertyControlType::Unknown ) )
            {
                weld::Widget* pWindow = rLine.pLine->getControlWindow();
                weld::Entry* pControlWindowAsEdit = dynamic_cast<weld::Entry*>(pWindow);
                if (pControlWindowAsEdit)
                    pControlWindowAsEdit->set_editable(false);
                else
                    pWindow->set_sensitive(false);
            }
        }

        sal_uInt16 nTextWidth = m_xLinesPlayground->get_pixel_size(rPropertyData.DisplayName).Width();
        if (m_nTheNameSize< nTextWidth)
        {
            m_nTheNameSize = nTextWidth;
            UpdatePlayGround();
        }
    }
} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
