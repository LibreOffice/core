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

#include "SidebarTxtControlAcc.hxx"

#include "SidebarTxtControl.hxx"

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
        virtual ~SidebarTextEditSource() override;

        virtual std::unique_ptr<SvxEditSource> Clone() const override;

        virtual SvxTextForwarder* GetTextForwarder() override;
        virtual SvxViewForwarder* GetViewForwarder() override;
        virtual SvxEditViewForwarder* GetEditViewForwarder( bool bCreate = false ) override;

        virtual void UpdateData() override;

        virtual SfxBroadcaster& GetBroadcaster() const override;
        DECL_LINK( NotifyHdl, EENotify&, void );

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

std::unique_ptr<SvxEditSource> SidebarTextEditSource::Clone() const
{
    return std::unique_ptr<SvxEditSource>(new SidebarTextEditSource( mrSidebarTextControl ));
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
    return * const_cast< SidebarTextEditSource* > (this);
}

IMPL_LINK(SidebarTextEditSource, NotifyHdl, EENotify&, rNotify, void)
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

        virtual sal_Int32 SAL_CALL
                getAccessibleChildCount() override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleChild( sal_Int32 i ) override;

        virtual void SAL_CALL
                addAccessibleEventListener (
                    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) override;
        virtual void SAL_CALL
                removeAccessibleEventListener (
                    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener) override;

    protected:
        virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;

    private:
        std::unique_ptr<::accessibility::AccessibleTextHelper> mpAccessibleTextHelper;

        ::osl::Mutex maMutex;

        void defunc();
};

SidebarTextControlAccessibleContext::SidebarTextControlAccessibleContext( SidebarTextControl& rSidebarTextControl )
    : VCLXAccessibleComponent( rSidebarTextControl.GetWindowPeer() )
    , maMutex()
{
    mpAccessibleTextHelper.reset(new ::accessibility::AccessibleTextHelper( o3tl::make_unique<SidebarTextEditSource>(rSidebarTextControl) ));
    mpAccessibleTextHelper->SetEventSource( rSidebarTextControl.GetWindowPeer() );
}

void SidebarTextControlAccessibleContext::defunc()
{
    mpAccessibleTextHelper.reset();
}

sal_Int32 SAL_CALL SidebarTextControlAccessibleContext::getAccessibleChildCount()
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
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->AddEventListener(xListener);
    }
}

void SAL_CALL SidebarTextControlAccessibleContext::removeAccessibleEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
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
            case VclEventId::ObjectDying:
            {
                defunc();
            }
            break;
            case VclEventId::WindowGetFocus:
            case VclEventId::ControlGetFocus:
            {
                mpAccessibleTextHelper->SetFocus();
            }
            break;
            case VclEventId::WindowLoseFocus:
            case VclEventId::ControlLoseFocus:
            {
                mpAccessibleTextHelper->SetFocus( false );
            }
            break;
            default: break;
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
