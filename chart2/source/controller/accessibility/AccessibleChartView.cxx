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
#include "precompiled_chart2.hxx"

#include "AccessibleChartView.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "servicenames.hxx"
#include "macros.hxx"
#include "ObjectHierarchy.hxx"
#include "ObjectIdentifier.hxx"
#include "ResId.hxx"
#include "Strings.hrc"
#include "AccessibleViewForwarder.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <tools/string.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
// for SolarMutex
#include <vcl/svapp.hxx>

// header for typedef MutexGuard
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using osl::MutexGuard;

//.............................................................................
namespace chart
{
//.............................................................................

AccessibleChartView::AccessibleChartView(
    const Reference< uno::XComponentContext >& xContext, SdrView* pView ) :
        impl::AccessibleChartView_Base(
            AccessibleElementInfo(), // empty for now
            true, // has children
            true  // always transparent
            ),
        m_xContext( xContext ),
        m_pSdrView( pView ),
        m_pViewForwarder( NULL )
{
    AddState( AccessibleStateType::OPAQUE );
}

AccessibleChartView::~AccessibleChartView()
{
    delete m_pViewForwarder;
}


awt::Rectangle AccessibleChartView::GetWindowPosSize() const
{
    Reference< awt::XWindow > xWindow( GetInfo().m_xWindow );
    if( ! xWindow.is())
        return awt::Rectangle();

    // this should do, but it doesn't => HACK
//     return xWindow->getPosSize();

    awt::Rectangle aBBox( xWindow->getPosSize() );

    Window* pWindow( VCLUnoHelper::GetWindow( GetInfo().m_xWindow ));
    if( pWindow )
    {
        // /-- solar
        SolarMutexGuard aSolarGuard;
        Point aVCLPoint( pWindow->OutputToAbsoluteScreenPixel( Point( 0, 0 ) ));
        aBBox.X = aVCLPoint.getX();
        aBBox.Y = aVCLPoint.getY();
        // \-- solar
    }

    return aBBox;
}

awt::Point AccessibleChartView::GetUpperLeftOnScreen() const
{
    awt::Point aParentPosition;

    awt::Rectangle aBBox( GetWindowPosSize() );
    aParentPosition.X = aBBox.X;
    aParentPosition.Y = aBBox.Y;

    return aParentPosition;
}

// ________ XAccessibleContext ________
OUString SAL_CALL AccessibleChartView::getAccessibleName()
    throw (uno::RuntimeException)
{
    return OUString( String( SchResId( STR_OBJECT_DIAGRAM )));
}

OUString SAL_CALL AccessibleChartView::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    return getAccessibleName();
}

Reference< XAccessible > SAL_CALL AccessibleChartView::getAccessibleParent()
    throw (uno::RuntimeException)
{
    return Reference< XAccessible >( m_xParent );
}

sal_Int32 SAL_CALL AccessibleChartView::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    // the document is always the only child of the window
    return 0;
}

sal_Int16 SAL_CALL AccessibleChartView::getAccessibleRole()
    throw (uno::RuntimeException)
{
    return AccessibleRole::DOCUMENT;
}

// ________ XAccessibleComponent ________
awt::Rectangle SAL_CALL AccessibleChartView::getBounds()
    throw (uno::RuntimeException)
{
    awt::Rectangle aResult( GetWindowPosSize());
    Reference< XAccessible > xParent( m_xParent );
    if( xParent.is())
    {
        Reference< XAccessibleComponent > xContext( xParent->getAccessibleContext(), uno::UNO_QUERY );
        if( xContext.is())
        {
            awt::Point aParentPosition = xContext->getLocationOnScreen();
            aResult.X -= aParentPosition.X;
            aResult.Y -= aParentPosition.Y;
        }
    }
    return aResult;
}

awt::Point SAL_CALL AccessibleChartView::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    awt::Rectangle aBounds( getBounds());
    awt::Point aResult;
    Reference< XAccessible > xParent( m_xParent );
    if( xParent.is())
    {
        Reference< XAccessibleComponent > xAccComp(
            xParent->getAccessibleContext(), uno::UNO_QUERY );
        aResult = xAccComp->getLocationOnScreen();
        aResult.X += aBounds.X;
        aResult.Y += aBounds.Y;
    }
    return aResult;
}

//-----------------------------------------------------------------
// lang::XInitialization
//-----------------------------------------------------------------

void SAL_CALL AccessibleChartView::initialize( const Sequence< Any >& rArguments )
                throw (uno::Exception, uno::RuntimeException)
{
    //0: view::XSelectionSupplier offers notifications for selection changes and access to the selection itself
    //1: frame::XModel representing the chart model - offers access to object data
    //2: lang::XInterface representing the normal chart view - offers access to some extra object data

    //all arguments are only valid until next initialization
    bool bChanged = false;
    bool bOldInvalid = false;
    bool bNewInvalid = false;

    Reference< view::XSelectionSupplier > xSelectionSupplier;
    Reference< frame::XModel > xChartModel;
    Reference< uno::XInterface > xChartView;
    Reference< XAccessible > xParent;
    Reference< awt::XWindow > xWindow;
    {
        MutexGuard aGuard( GetMutex());
        xSelectionSupplier.set( m_xSelectionSupplier );
        xChartModel.set( m_xChartModel );
        xChartView.set( m_xChartView );
        xParent.set( m_xParent );
        xWindow.set( m_xWindow );
    }

    if( !xSelectionSupplier.is() || !xChartModel.is() || !xChartView.is() )
    {
        bOldInvalid = true;
    }

    if( rArguments.getLength() > 1 )
    {
        Reference< frame::XModel > xNewChartModel;
        rArguments[1] >>= xNewChartModel;
        if( xNewChartModel != xChartModel )
        {
            xChartModel = xNewChartModel;
            bChanged = true;
        }
    }
    else if( xChartModel.is() )
    {
        bChanged = true;
        xChartModel = 0;
    }

    if( rArguments.getLength() > 2 )
    {
        Reference< uno::XInterface > xNewChartView;
        rArguments[2] >>= xNewChartView;
        if( xNewChartView != xChartView )
        {
            xChartView = xNewChartView;
            bChanged = true;
        }
    }
    else if( xChartView.is() )
    {
        bChanged = true;
        xChartView = 0;
    }

    if( rArguments.getLength() > 3 )
    {
        Reference< XAccessible > xNewParent;
        rArguments[3] >>= xNewParent;
        if( xNewParent != xParent )
        {
            xParent = xNewParent;
            bChanged = true;
        }
    }

    if( rArguments.getLength() > 4 )
    {
        Reference< awt::XWindow > xNewWindow;
        rArguments[4] >>= xNewWindow;
        if( xNewWindow != xWindow )
        {
            xWindow.set( xNewWindow );
            bChanged = true;
        }
    }

    if( rArguments.getLength() > 0 && xChartModel.is() && xChartView.is() )
    {
        Reference< view::XSelectionSupplier > xNewSelectionSupplier;
        rArguments[0] >>= xNewSelectionSupplier;
        if(xSelectionSupplier!=xNewSelectionSupplier)
        {
            bChanged = true;
            if(xSelectionSupplier.is())
                xSelectionSupplier->removeSelectionChangeListener(this);
            if(xNewSelectionSupplier.is())
                xNewSelectionSupplier->addSelectionChangeListener(this);
            xSelectionSupplier = xNewSelectionSupplier;
        }
    }
    else if( xSelectionSupplier.is() )
    {
        bChanged = true;
        xSelectionSupplier->removeSelectionChangeListener(this);
        xSelectionSupplier = 0;
    }

    if( !xSelectionSupplier.is() || !xChartModel.is() || !xChartView.is() )
    {
        if(xSelectionSupplier.is())
            xSelectionSupplier->removeSelectionChangeListener(this);
        xSelectionSupplier = 0;
        xChartModel.clear();
        xChartView.clear();
        xParent.clear();
        xWindow.clear();

        bNewInvalid = true;
    }

    {
        MutexGuard aGuard( GetMutex());
        m_xSelectionSupplier = WeakReference< view::XSelectionSupplier >(xSelectionSupplier);
        m_xChartModel = WeakReference< frame::XModel >(xChartModel);
        m_xChartView = WeakReference< uno::XInterface >(xChartView);
        m_xParent = WeakReference< XAccessible >(xParent);
        m_xWindow = WeakReference< awt::XWindow >(xWindow);
    }

    if( bOldInvalid && bNewInvalid )
        bChanged = false;

    if( bChanged )
    {
        {
            //before notification we prepare for creation of new context
            //the old context will be deleted after notification than
            MutexGuard aGuard( GetMutex());
            Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY );
            if( xChartDoc.is())
                m_spObjectHierarchy.reset( new ObjectHierarchy( xChartDoc, getExplicitValueProvider() ));
            else
                m_spObjectHierarchy.reset();
        }

        {
            AccessibleElementInfo aAccInfo;
            aAccInfo.m_aOID = ObjectIdentifier( C2U( "ROOT" ) );
            aAccInfo.m_xChartDocument = uno::WeakReference< chart2::XChartDocument >(
                uno::Reference< chart2::XChartDocument >( m_xChartModel.get(), uno::UNO_QUERY ));
            aAccInfo.m_xSelectionSupplier = m_xSelectionSupplier;
            aAccInfo.m_xView = m_xChartView;
            aAccInfo.m_xWindow = m_xWindow;
            aAccInfo.m_pParent = 0;
            aAccInfo.m_spObjectHierarchy = m_spObjectHierarchy;
            aAccInfo.m_pSdrView = m_pSdrView;
            Window* pWindow = VCLUnoHelper::GetWindow( m_xWindow );
            if ( m_pViewForwarder )
            {
                delete m_pViewForwarder;
            }
            m_pViewForwarder = new AccessibleViewForwarder( this, pWindow );
            aAccInfo.m_pViewForwarder = m_pViewForwarder;
            // broadcasts an INVALIDATE_ALL_CHILDREN event globally
            SetInfo( aAccInfo );
        }
    }
}

ExplicitValueProvider* AccessibleChartView::getExplicitValueProvider()
{
    return ExplicitValueProvider::getExplicitValueProvider(m_xChartView);
}

//-------------------------------------------------------------------------
// view::XSelectionChangeListener
//-------------------------------------------------------------------------

void SAL_CALL AccessibleChartView::selectionChanged( const lang::EventObject& /*rEvent*/ )
                throw (uno::RuntimeException)
{
    Reference< view::XSelectionSupplier > xSelectionSupplier;
    {
        MutexGuard aGuard( GetMutex());
        xSelectionSupplier = Reference< view::XSelectionSupplier >(m_xSelectionSupplier);
    }

    if( xSelectionSupplier.is() )
    {
        ObjectIdentifier aSelectedOID( xSelectionSupplier->getSelection() );
        if ( m_aCurrentSelectionOID.isValid() )
        {
            NotifyEvent( LOST_SELECTION, m_aCurrentSelectionOID );
        }
        if( aSelectedOID.isValid() )
        {
            NotifyEvent( GOT_SELECTION, aSelectedOID );
        }
        m_aCurrentSelectionOID = aSelectedOID;
    }
}

//-------------------------------------------------------------------------
// lang::XComponent::dispose()
//-------------------------------------------------------------------------
void SAL_CALL AccessibleChartView::disposing()
{
    AccessibleBase::disposing();
}

//-------------------------------------------------------------------------
// XEventListener
//-------------------------------------------------------------------------
void SAL_CALL AccessibleChartView::disposing( const lang::EventObject& /*Source*/ )
    throw (uno::RuntimeException)
{
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
