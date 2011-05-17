/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include "precompiled_sw.hxx"

#include <SidebarTxtControlAcc.hxx>

#include <SidebarTxtControl.hxx>

#include <svl/brdcst.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/unoforou.hxx>
#include <editeng/unoviwou.hxx>
#include <editeng/unoedhlp.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/outliner.hxx>


namespace css = ::com::sun::star;

namespace sw { namespace sidebarwindows {

// =============================================================================
// declaration and implementation of <SvxEditSource>
// for <::accessibiliy::AccessibleTextHelper> instance
// =============================================================================
class SidebarTextEditSource : public SvxEditSource,
                              public SfxBroadcaster
{
    public:
        SidebarTextEditSource( SidebarTxtControl& rSidebarTxtControl );
        virtual ~SidebarTextEditSource();

        virtual SvxEditSource* Clone() const;

        virtual SvxTextForwarder* GetTextForwarder();
        virtual SvxViewForwarder* GetViewForwarder();
        virtual SvxEditViewForwarder* GetEditViewForwarder( sal_Bool bCreate = sal_False );

        virtual void UpdateData();

        virtual SfxBroadcaster& GetBroadcaster() const;
        DECL_LINK( NotifyHdl, EENotify* );

    private:
        SidebarTxtControl& mrSidebarTxtControl;
        SvxOutlinerForwarder mTextForwarder;
        SvxDrawOutlinerViewForwarder mViewForwarder;
};

SidebarTextEditSource::SidebarTextEditSource( SidebarTxtControl& rSidebarTxtControl )
    : SvxEditSource()
    , mrSidebarTxtControl( rSidebarTxtControl )
    , mTextForwarder( *(rSidebarTxtControl.GetTextView()->GetOutliner()), sal_False )
    , mViewForwarder( *(rSidebarTxtControl.GetTextView()) )
{
    if ( mrSidebarTxtControl.GetTextView() )
    {
        mrSidebarTxtControl.GetTextView()->GetOutliner()->SetNotifyHdl( LINK(this, SidebarTextEditSource, NotifyHdl) );
    }
}

SidebarTextEditSource::~SidebarTextEditSource()
{
    if ( mrSidebarTxtControl.GetTextView() )
    {
        mrSidebarTxtControl.GetTextView()->GetOutliner()->SetNotifyHdl( Link() );
    }
}

SvxEditSource* SidebarTextEditSource::Clone() const
{
    return new SidebarTextEditSource( mrSidebarTxtControl );
}

SvxTextForwarder* SidebarTextEditSource::GetTextForwarder()
{
    return &mTextForwarder;
}

SvxViewForwarder* SidebarTextEditSource::GetViewForwarder()
{
    return &mViewForwarder;
}

SvxEditViewForwarder* SidebarTextEditSource::GetEditViewForwarder( sal_Bool /*bCreate*/ )
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

IMPL_LINK(SidebarTextEditSource, NotifyHdl, EENotify*, pNotify)
{
    if ( pNotify )
    {
        ::std::auto_ptr< SfxHint > aHint( SvxEditSourceHelper::EENotification2Hint( pNotify ) );

        if( aHint.get() )
        {
            Broadcast( *aHint.get() );
        }
    }

    return 0;
}


// =============================================================================
// declaration and implementation of accessible context for <SidebarTxtControl> instance
// =============================================================================
class SidebarTxtControlAccessibleContext : public VCLXAccessibleComponent
{
    public:
        explicit SidebarTxtControlAccessibleContext( SidebarTxtControl& rSidebarTxtControl );
        virtual ~SidebarTxtControlAccessibleContext();

        virtual sal_Int32 SAL_CALL
                getAccessibleChildCount()
                throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                getAccessibleChild( sal_Int32 i )
                throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

        using WeakAggComponentImplHelperBase::addEventListener;
        using WeakAggComponentImplHelperBase::removeEventListener;

        virtual void SAL_CALL
                addEventListener (
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
                throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL
                removeEventListener (
                    const ::com::sun::star::uno::Reference<
                        ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
                throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

    private:
        SidebarTxtControl& mrSidebarTxtControl;
        ::accessibility::AccessibleTextHelper* mpAccessibleTextHelper;

        ::osl::Mutex maMutex;

        void defunc();
};

SidebarTxtControlAccessibleContext::SidebarTxtControlAccessibleContext( SidebarTxtControl& rSidebarTxtControl )
    : VCLXAccessibleComponent( rSidebarTxtControl.GetWindowPeer() )
    , mrSidebarTxtControl( rSidebarTxtControl )
    , mpAccessibleTextHelper( 0 )
    , maMutex()
{
    ::std::auto_ptr<SvxEditSource> pEditSource(
                        new SidebarTextEditSource( mrSidebarTxtControl ) );
    mpAccessibleTextHelper = new ::accessibility::AccessibleTextHelper( pEditSource );
    mpAccessibleTextHelper->SetEventSource( mrSidebarTxtControl.GetWindowPeer() );
}

SidebarTxtControlAccessibleContext::~SidebarTxtControlAccessibleContext()
{
    defunc();
}

void SidebarTxtControlAccessibleContext::defunc()
{
    delete mpAccessibleTextHelper;
    mpAccessibleTextHelper = 0;
}

sal_Int32 SAL_CALL SidebarTxtControlAccessibleContext::getAccessibleChildCount()
    throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard( maMutex );

    sal_Int32 nChildCount( 0 );

    if ( mpAccessibleTextHelper )
    {
        nChildCount = mpAccessibleTextHelper->GetChildCount();
    }

    return nChildCount;
}

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL SidebarTxtControlAccessibleContext::getAccessibleChild( sal_Int32 i )
    throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException )
{
    osl::MutexGuard aGuard( maMutex );

    css::uno::Reference< css::accessibility::XAccessible > xChild;

    if ( mpAccessibleTextHelper )
    {
        xChild = mpAccessibleTextHelper->GetChild( i );
    }

    return xChild;
}

void SAL_CALL SidebarTxtControlAccessibleContext::addEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->AddEventListener(xListener);
    }
}

void SAL_CALL SidebarTxtControlAccessibleContext::removeEventListener (
    const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener)
    throw (css::uno::RuntimeException)
{
    osl::MutexGuard aGuard( maMutex );

    if ( mpAccessibleTextHelper )
    {
        mpAccessibleTextHelper->RemoveEventListener(xListener);
    }
}

void SidebarTxtControlAccessibleContext::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
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
                mpAccessibleTextHelper->SetFocus( sal_True );
            }
            break;
            case VCLEVENT_WINDOW_LOSEFOCUS:
            case VCLEVENT_CONTROL_LOSEFOCUS:
            {
                mpAccessibleTextHelper->SetFocus( sal_False );
            }
            break;
        }
    }

    VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
}

// =============================================================================
// implementaion of accessible for <SidebarTxtControl> instance
// =============================================================================
SidebarTxtControlAccessible::SidebarTxtControlAccessible( SidebarTxtControl& rSidebarTxtControl )
    : VCLXWindow()
    , mrSidebarTxtControl( rSidebarTxtControl )
{
    SetWindow( &mrSidebarTxtControl );
}

SidebarTxtControlAccessible::~SidebarTxtControlAccessible()
{
}

css::uno::Reference< css::accessibility::XAccessibleContext > SidebarTxtControlAccessible::CreateAccessibleContext()
{
    SidebarTxtControlAccessibleContext* pAccContext(
                        new SidebarTxtControlAccessibleContext( mrSidebarTxtControl ) );
    css::uno::Reference< css::accessibility::XAccessibleContext > xAcc( pAccContext );
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
