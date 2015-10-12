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

#include <sal/config.h>

#include <memory>

#include <SidebarTxtControlAcc.hxx>

#include <SidebarTxtControl.hxx>

#include <svl/SfxBroadcaster.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>
#include <editeng/unoedhlp.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/outliner.hxx>
#include <o3tl/make_unique.hxx>

namespace sw { namespace sidebarwindows {

// declaration and implementation of <SvxEditSource>
// for <::accessibility::AccessibleTextHelper> instance
class SidebarTextEditSource : public SvxEditSource,
                              public SfxBroadcaster
{
    public:
        explicit SidebarTextEditSource(SidebarTextControl& rSidebarTextControl);
        virtual ~SidebarTextEditSource();

        virtual SvxEditSource* Clone() const override;

        virtual SvxTextForwarder* GetTextForwarder() override;
        virtual SvxViewForwarder* GetViewForwarder() override;
        virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate = false ) override;

        virtual void UpdateData() override;

        virtual SfxBroadcaster& GetBroadcaster() const override;
        DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

    private:
        SidebarTextControl& mrSidebarTextControl;
        SvxOutlinerForwarder mTextForwarder;
        SvxDrawOutlinerViewForwarder mViewForwarder;
};

SidebarTextEditSource::SidebarTextEditSource( SidebarTextControl& rSidebarTextControl )
    : SvxEditSource()
    , mrSidebarTextControl( rSidebarTextControl )
    , mTextForwarder( *(rSidebarTextControl.GetTextView()->GetOutliner()), false )
    , mViewForwarder( *(rSidebarTextControl.GetTextView()) )
{
    if ( mrSidebarTextControl.GetTextView() )
    {
        mrSidebarTextControl.GetTextView()->GetOutliner()->SetNotifyHdl( LINK(this, SidebarTextEditSource, NotifyHdl) );
    }
}

SidebarTextEditSource::~SidebarTextEditSource()
{
    if ( mrSidebarTextControl.GetTextView() )
    {
        mrSidebarTextControl.GetTextView()->GetOutliner()->SetNotifyHdl( Link<EENotify&,void>() );
    }
}

SvxEditSource* SidebarTextEditSource::Clone() const
{
    return new SidebarTextEditSource( mrSidebarTextControl );
}

SvxTextForwarder* SidebarTextEditSource::GetTextForwarder()
{
    return &mTextForwarder;
}

SvxViewForwarder* SidebarTextEditSource::GetViewForwarder()
{
    return &mViewForwarder;
}

SvxEditViewForwarder* SidebarTextEditSource::GetEditViewForwarder( bool /*bCreate*/ )
{
    return &mViewForwarder;
}

void SidebarTextEditSource::UpdateData()
{
    // nothing to do
}

SfxBroadcaster& SidebarTextEditSource::GetBroadcaster() const
{
    return *( const_cast< SidebarTextEditSource* > (this) );
}

IMPL_LINK_TYPED(SidebarTextEditSource, NotifyHdl, EENotify&, rNotify, void)
{
    std::unique_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( &rNotify ) );

    if( aHint.get() )
    {
        Broadcast( *aHint.get() );
    }
}

// declaration and implementation of accessible context for <SidebarTextControl> instance
class SidebarTextControlAccessibleContext : public VCLXAccessibleComponent
{
    public:
        explicit SidebarTextControlAccessibleContext( SidebarTextControl& rSidebarTextControl );
        virtual ~SidebarTextControlAccessibleContext();

        virtual sal_Int32 SAL_CALL
                getAccessibleChildCount()
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleChild( sal_Int32 i )
                throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
                addAccessibleEventListener (
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL
                removeAccessibleEventListener (
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
                throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    protected:
        virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    private:
        SidebarTextControl& mrSidebarTextControl;
        ::accessibility::AccessibleTextHelper* mpAccessibleTextHelper;

        ::osl::Mutex maMutex;

        void defunc();
};

SidebarTextControlAccessibleContext::SidebarTextControlAccessibleContext( SidebarTextControl& rSidebarTextControl )
    : VCLXAccessibleComponent( rSidebarTextControl.GetWindowPeer() )
    , mrSidebarTextControl( rSidebarTextControl )
    , mpAccessibleTextHelper( 0 )
    , maMutex()
{
    mpAccessibleTextHelper = new ::accessibility::AccessibleTextHelper( o3tl::make_unique<SidebarTextEditSource>(mrSidebarTextControl) );
    mpAccessibleTextHelper->SetEventSource( mrSidebarTextControl.GetWindowPeer() );
}

SidebarTextControlAccessibleContext::~SidebarTextControlAccessibleContext()
{
    defunc();
}

void SidebarTextControlAccessibleContext::defunc()
{
    delete mpAccessibleTextHelper;
    mpAccessibleTextHelper = 0;
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getAccessibleChildCount()
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    sal_Int32 nChildCount( 0 );

    if ( mpAccessibleTextHelper )
    {
        nChildCount = mpAccessibleTextHelper->GetChildCount();
    }

    return nChildCount;
}

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL SidebarTextControlAccessibleContext::getAccessibleChild( sal_Int32 i )
    throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( maMutex );

    css::uno::Reference< css::accessibility::XAccessible > xChild;

    if ( mpAccessibleTextHelper )
    {
        xChild = mpAccessibleTextHelper->GetChild( i );
    }

    return xChild;
}

void SAL_CALL SidebarTextControlAccessibleContext::addAccessibleEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->AddEventListener(xListener);
    }
}

void SAL_CALL SidebarTextControlAccessibleContext::removeAccessibleEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->RemoveEventListener(xListener);
    }
}

void SidebarTextControlAccessibleContext::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    if ( mpAccessibleTextHelper )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_OBJECT_DYING:
            {
                defunc();
            }
            break;
            case VCLEVENT_WINDOW_GETFOCUS:
            case VCLEVENT_CONTROL_GETFOCUS:
            {
                mpAccessibleTextHelper->SetFocus();
            }
            break;
            case VCLEVENT_WINDOW_LOSEFOCUS:
            case VCLEVENT_CONTROL_LOSEFOCUS:
            {
                mpAccessibleTextHelper->SetFocus( false );
            }
            break;
        }
    }

    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

// implementation of accessible for <SidebarTextControl> instance
SidebarTextControlAccessible::SidebarTextControlAccessible( SidebarTextControl& rSidebarTextControl )
    : VCLXWindow()
    , mrSidebarTextControl( rSidebarTextControl )
{
    SetWindow( &mrSidebarTextControl );
}

SidebarTextControlAccessible::~SidebarTextControlAccessible()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > SidebarTextControlAccessible::CreateAccessibleContext()
{
    SidebarTextControlAccessibleContext* pAccContext(
                        new SidebarTextControlAccessibleContext( mrSidebarTextControl ) );
    css::uno::Reference< css::accessibility::XAccessibleContext > xAcc( pAccContext );
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
