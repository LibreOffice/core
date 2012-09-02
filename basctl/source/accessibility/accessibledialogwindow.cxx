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


#include <accessibledialogwindow.hxx>
#include <accessibledialogcontrolshape.hxx>
#include <baside3.hxx>
#include <dlged.hxx>
#include <dlgedmod.hxx>
#include <dlgedpage.hxx>
#include <dlgedview.hxx>
#include <dlgedobj.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

DBG_NAME( AccessibleDialogWindow )


// -----------------------------------------------------------------------------
// class ChildDescriptor
// -----------------------------------------------------------------------------

AccessibleDialogWindow::ChildDescriptor::ChildDescriptor( DlgEdObj* _pDlgEdObj )
    :pDlgEdObj( _pDlgEdObj )
    ,rxAccessible( 0 )
{
}

// -----------------------------------------------------------------------------

AccessibleDialogWindow::ChildDescriptor::~ChildDescriptor()
{
}

// -----------------------------------------------------------------------------

AccessibleDialogWindow::ChildDescriptor::ChildDescriptor( const ChildDescriptor& rDesc )
    :pDlgEdObj( rDesc.pDlgEdObj )
    ,rxAccessible( rDesc.rxAccessible )
{
}

// -----------------------------------------------------------------------------

AccessibleDialogWindow::ChildDescriptor& AccessibleDialogWindow::ChildDescriptor::operator=( const ChildDescriptor& rDesc )
{
    pDlgEdObj = rDesc.pDlgEdObj;
    rxAccessible = rDesc.rxAccessible;

    return *this;
}

// -----------------------------------------------------------------------------

bool AccessibleDialogWindow::ChildDescriptor::operator==( const ChildDescriptor& rDesc )
{
    bool bRet = false;
    if ( pDlgEdObj == rDesc.pDlgEdObj )
        bRet = true;

    return bRet;
}

// -----------------------------------------------------------------------------

bool AccessibleDialogWindow::ChildDescriptor::operator<( const ChildDescriptor& rDesc ) const
{
    bool bRet = false;
    if ( pDlgEdObj && rDesc.pDlgEdObj && pDlgEdObj->GetOrdNum() < rDesc.pDlgEdObj->GetOrdNum() )
        bRet = true;

    return bRet;
}

// -----------------------------------------------------------------------------
// class AccessibleDialogWindow
// -----------------------------------------------------------------------------

AccessibleDialogWindow::AccessibleDialogWindow (basctl::DialogWindow* pDialogWindow)
    :AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    ,m_pDialogWindow( pDialogWindow )
{
    DBG_CTOR( AccessibleDialogWindow, NULL );
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    if ( m_pDialogWindow )
    {
        SdrPage& rPage = m_pDialogWindow->GetPage();
        sal_uLong nCount = rPage.GetObjCount();

        for ( sal_uLong i = 0; i < nCount; ++i )
        {
            if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(rPage.GetObj(i)))
            {
                ChildDescriptor aDesc( pDlgEdObj );
                if ( IsChildVisible( aDesc ) )
                    m_aAccessibleChildren.push_back( aDesc );
            }
        }

        m_pDialogWindow->AddEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );

        StartListening(m_pDialogWindow->GetEditor());

        m_pDlgEdModel = &m_pDialogWindow->GetModel();
        StartListening(*m_pDlgEdModel);
    }
}

// -----------------------------------------------------------------------------

AccessibleDialogWindow::~AccessibleDialogWindow()
{
    DBG_DTOR( AccessibleDialogWindow, NULL );
    if ( m_pDialogWindow )
        m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );

    if ( m_pDlgEditor )
        EndListening( *m_pDlgEditor );

    if ( m_pDlgEdModel )
        EndListening( *m_pDlgEdModel );

    delete m_pExternalLock;
    m_pExternalLock = NULL;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::UpdateFocused()
{
    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i].rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetFocused( pShape->IsFocused() );
        }
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::UpdateSelected()
{
    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i].rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetSelected( pShape->IsSelected() );
        }
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::UpdateBounds()
{
    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i].rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetBounds( pShape->GetBounds() );
        }
    }
}

// -----------------------------------------------------------------------------

bool AccessibleDialogWindow::IsChildVisible( const ChildDescriptor& rDesc )
{
    bool bVisible = false;

    if ( m_pDialogWindow )
    {
        // first check, if the shape is in a visible layer
        SdrLayerAdmin& rLayerAdmin = m_pDialogWindow->GetModel().GetLayerAdmin();
        DlgEdObj* pDlgEdObj = rDesc.pDlgEdObj;
        if ( pDlgEdObj )
        {
            SdrLayerID nLayerId = pDlgEdObj->GetLayer();
            const SdrLayer* pSdrLayer = rLayerAdmin.GetLayerPerID( nLayerId );
            if ( pSdrLayer )
            {
                ::rtl::OUString aLayerName = pSdrLayer->GetName();
                SdrView& rView = m_pDialogWindow->GetView();
                if (rView.IsLayerVisible(aLayerName))
                {
                    // get the bounding box of the shape in logic units
                    Rectangle aRect = pDlgEdObj->GetSnapRect();

                    // transform coordinates relative to the parent
                    MapMode aMap = m_pDialogWindow->GetMapMode();
                    Point aOrg = aMap.GetOrigin();
                    aRect.Move( aOrg.X(), aOrg.Y() );

                    // convert logic units to pixel
                    aRect = m_pDialogWindow->LogicToPixel( aRect, MapMode(MAP_100TH_MM) );

                    // check, if the shape's bounding box intersects with the bounding box of its parent
                    Rectangle aParentRect( Point( 0, 0 ), m_pDialogWindow->GetSizePixel() );
                    if ( aParentRect.IsOver( aRect ) )
                        bVisible = true;
                }
            }
        }
    }

    return bVisible;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::InsertChild( const ChildDescriptor& rDesc )
{
    // check, if object is already in child list
    AccessibleChildren::iterator aIter = ::std::find( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end(), rDesc );

    // if not found, insert in child list
    if ( aIter == m_aAccessibleChildren.end() )
    {
        // insert entry in child list
        m_aAccessibleChildren.push_back( rDesc );

        // get the accessible of the inserted child
        Reference< XAccessible > xChild( getAccessibleChild( m_aAccessibleChildren.size() - 1 ) );

        // sort child list
        SortChildren();

        // send accessible child event
        if ( xChild.is() )
        {
            Any aOldValue, aNewValue;
            aNewValue <<= xChild;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
        }
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::RemoveChild( const ChildDescriptor& rDesc )
{
    // find object in child list
    AccessibleChildren::iterator aIter = ::std::find( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end(), rDesc );

    // if found, remove from child list
    if ( aIter != m_aAccessibleChildren.end() )
    {
        // get the accessible of the removed child
        Reference< XAccessible > xChild( aIter->rxAccessible );

        // remove entry from child list
        m_aAccessibleChildren.erase( aIter );

        // send accessible child event
        if ( xChild.is() )
        {
            Any aOldValue, aNewValue;
            aOldValue <<= xChild;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );

            Reference< XComponent > xComponent( xChild, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::UpdateChild( const ChildDescriptor& rDesc )
{
    if ( IsChildVisible( rDesc ) )
    {
        // if the object is not in the child list, insert child
        InsertChild( rDesc );
    }
    else
    {
        // if the object is in the child list, remove child
        RemoveChild( rDesc );
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::UpdateChildren()
{
    if ( m_pDialogWindow )
    {
        SdrPage& rPage = m_pDialogWindow->GetPage();
        for ( sal_uLong i = 0, nCount = rPage.GetObjCount(); i < nCount; ++i )
            if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(rPage.GetObj(i)))
                UpdateChild( ChildDescriptor( pDlgEdObj ) );
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::SortChildren()
{
    // sort child list
    ::std::sort( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end() );
}

// -----------------------------------------------------------------------------

IMPL_LINK( AccessibleDialogWindow, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_CHKTHIS( AccessibleDialogWindow, 0 );

    if (VclWindowEvent* pWinEvent = dynamic_cast<VclWindowEvent*>(pEvent))
    {
        DBG_ASSERT(pWinEvent->GetWindow(), "AccessibleDialogWindow::WindowEventListener: no window!");
        if (!pWinEvent->GetWindow()->IsAccessibilityEventsSuppressed() || pEvent->GetId() == VCLEVENT_OBJECT_DYING)
            ProcessWindowEvent(*pWinEvent);
    }
    else
        DBG_ASSERT(false, "AccessibleDialogWindow::WindowEventListener: unknown window event!");
    return 0;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    Any aOldValue, aNewValue;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_ENABLED:
        {
            aNewValue <<= AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_DISABLED:
        {
            aOldValue <<= AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            aNewValue <<= AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            aOldValue <<= AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_GETFOCUS:
        {
            aNewValue <<= AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            aOldValue <<= AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_SHOW:
        {
            aNewValue <<= AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            aOldValue <<= AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_RESIZE:
        {
            NotifyAccessibleEvent( AccessibleEventId::BOUNDRECT_CHANGED, aOldValue, aNewValue );
            UpdateChildren();
            UpdateBounds();
        }
        break;
        case VCLEVENT_OBJECT_DYING:
        {
            if ( m_pDialogWindow )
            {
                m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );
                m_pDialogWindow = NULL;

                if ( m_pDlgEditor )
                    EndListening( *m_pDlgEditor );
                m_pDlgEditor = NULL;

                if ( m_pDlgEdModel )
                    EndListening( *m_pDlgEdModel );
                m_pDlgEdModel = NULL;

                // dispose all children
                for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
                {
                    Reference< XComponent > xComponent( m_aAccessibleChildren[i].rxAccessible, UNO_QUERY );
                    if ( xComponent.is() )
                        xComponent->dispose();
                }
                m_aAccessibleChildren.clear();
            }
        }
        break;
        default:
        {
        }
        break;
    }
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    if ( m_pDialogWindow )
    {
        if ( m_pDialogWindow->IsEnabled() )
            rStateSet.AddState( AccessibleStateType::ENABLED );

        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        if ( m_pDialogWindow->HasFocus() )
            rStateSet.AddState( AccessibleStateType::FOCUSED );

        rStateSet.AddState( AccessibleStateType::VISIBLE );

        if ( m_pDialogWindow->IsVisible() )
            rStateSet.AddState( AccessibleStateType::SHOWING );

        rStateSet.AddState( AccessibleStateType::OPAQUE );

        rStateSet.AddState( AccessibleStateType::RESIZABLE );
    }
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle AccessibleDialogWindow::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds;
    if ( m_pDialogWindow )
        aBounds = AWTRectangle( Rectangle( m_pDialogWindow->GetPosPixel(), m_pDialogWindow->GetSizePixel() ) );

    return aBounds;
}

// -----------------------------------------------------------------------------
// SfxListener
// -----------------------------------------------------------------------------

void AccessibleDialogWindow::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (SdrHint const* pSdrHint = dynamic_cast<SdrHint const*>(&rHint))
    {
        switch ( pSdrHint->GetKind() )
        {
            case HINT_OBJINSERTED:
            {
                if (DlgEdObj const* pDlgEdObj = dynamic_cast<DlgEdObj const*>(pSdrHint->GetObject()))
                {
                    ChildDescriptor aDesc(const_cast<DlgEdObj*>(pDlgEdObj));
                    if ( IsChildVisible( aDesc ) )
                        InsertChild( aDesc );
                }
            }
            break;
            case HINT_OBJREMOVED:
            {
                if (DlgEdObj const* pDlgEdObj = dynamic_cast<DlgEdObj const*>(pSdrHint->GetObject()))
                    RemoveChild( ChildDescriptor(const_cast<DlgEdObj*>(pDlgEdObj)) );
            }
            break;
            default: ;
        }
    }
    else if (DlgEdHint const* pDlgEdHint = dynamic_cast<DlgEdHint const*>(&rHint))
    {
        switch (pDlgEdHint->GetKind())
        {
            case DlgEdHint::WINDOWSCROLLED:
            {
                UpdateChildren();
                UpdateBounds();
            }
            break;
            case DlgEdHint::LAYERCHANGED:
            {
                if (DlgEdObj* pDlgEdObj = pDlgEdHint->GetObject())
                    UpdateChild( ChildDescriptor( pDlgEdObj ) );
            }
            break;
            case DlgEdHint::OBJORDERCHANGED:
            {
                SortChildren();
            }
            break;
            case DlgEdHint::SELECTIONCHANGED:
            {
                UpdateFocused();
                UpdateSelected();
            }
            break;
            default: ;
        }
    }
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( AccessibleDialogWindow, AccessibleExtendedComponentHelper_BASE, AccessibleDialogWindow_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleDialogWindow, AccessibleExtendedComponentHelper_BASE, AccessibleDialogWindow_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void AccessibleDialogWindow::disposing()
{
    AccessibleExtendedComponentHelper_BASE::disposing();

    if ( m_pDialogWindow )
    {
        m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );
        m_pDialogWindow = NULL;

        if ( m_pDlgEditor )
            EndListening( *m_pDlgEditor );
        m_pDlgEditor = NULL;

        if ( m_pDlgEdModel )
            EndListening( *m_pDlgEdModel );
        m_pDlgEdModel = NULL;

        // dispose all children
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XComponent > xComponent( m_aAccessibleChildren[i].rxAccessible, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogWindow::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.basctl.AccessibleWindow" ));
}

// -----------------------------------------------------------------------------

sal_Bool AccessibleDialogWindow::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > AccessibleDialogWindow::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.AccessibleWindow" ));
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > AccessibleDialogWindow::getAccessibleContext(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogWindow::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogWindow::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild = m_aAccessibleChildren[i].rxAccessible;
    if ( !xChild.is() )
    {
        if ( m_pDialogWindow )
        {
            DlgEdObj* pDlgEdObj = m_aAccessibleChildren[i].pDlgEdObj;
            if ( pDlgEdObj )
            {
                xChild = new AccessibleDialogControlShape( m_pDialogWindow, pDlgEdObj );

                // insert into child list
                m_aAccessibleChildren[i].rxAccessible = xChild;
            }
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogWindow::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pDialogWindow )
    {
        Window* pParent = m_pDialogWindow->GetAccessibleParentWindow();
        if ( pParent )
            xParent = pParent->GetAccessible();
    }

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogWindow::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;
    if ( m_pDialogWindow )
    {
        Window* pParent = m_pDialogWindow->GetAccessibleParentWindow();
        if ( pParent )
        {
            for ( sal_uInt16 i = 0, nCount = pParent->GetAccessibleChildWindowCount(); i < nCount; ++i )
            {
                Window* pChild = pParent->GetAccessibleChildWindow( i );
                if ( pChild == static_cast< Window* >( m_pDialogWindow ) )
                {
                    nIndexInParent = i;
                    break;
                }
            }
        }
    }

    return nIndexInParent;
}

// -----------------------------------------------------------------------------

sal_Int16 AccessibleDialogWindow::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::PANEL;
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogWindow::getAccessibleDescription(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sDescription;
    if ( m_pDialogWindow )
        sDescription = m_pDialogWindow->GetAccessibleDescription();

    return sDescription;
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogWindow::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sName;
    if ( m_pDialogWindow )
        sName = m_pDialogWindow->GetAccessibleName();

    return sName;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > AccessibleDialogWindow::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > AccessibleDialogWindow::getAccessibleStateSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xSet = pStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        FillAccessibleStateSet( *pStateSetHelper );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }

    return xSet;
}

// -----------------------------------------------------------------------------

Locale AccessibleDialogWindow::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogWindow::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xChild;
    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xAcc = getAccessibleChild( i );
        if ( xAcc.is() )
        {
            Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
            if ( xComp.is() )
            {
                Rectangle aRect = VCLRectangle( xComp->getBounds() );
                Point aPos = VCLPoint( rPoint );
                if ( aRect.IsInside( aPos ) )
                {
                    xChild = xAcc;
                    break;
                }
            }
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::grabFocus(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GrabFocus();
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogWindow::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pDialogWindow )
    {
        if ( m_pDialogWindow->IsControlForeground() )
            nColor = m_pDialogWindow->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( m_pDialogWindow->IsControlFont() )
                aFont = m_pDialogWindow->GetControlFont();
            else
                aFont = m_pDialogWindow->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogWindow::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pDialogWindow )
    {
        if ( m_pDialogWindow->IsControlBackground() )
            nColor = m_pDialogWindow->GetControlBackground().GetColor();
        else
            nColor = m_pDialogWindow->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > AccessibleDialogWindow::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    if ( m_pDialogWindow )
    {
        Reference< awt::XDevice > xDev( m_pDialogWindow->GetComponentInterface(), UNO_QUERY );
        if ( xDev.is() )
        {
            Font aFont;
            if ( m_pDialogWindow->IsControlFont() )
                aFont = m_pDialogWindow->GetControlFont();
            else
                aFont = m_pDialogWindow->GetFont();
            VCLXFont* pVCLXFont = new VCLXFont;
            pVCLXFont->Init( *xDev.get(), aFont );
            xFont = pVCLXFont;
        }
    }

    return xFont;
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogWindow::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogWindow::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sText;
    if ( m_pDialogWindow )
        sText = m_pDialogWindow->GetQuickHelpText();

    return sText;
}

// -----------------------------------------------------------------------------
// XAccessibleSelection
// -----------------------------------------------------------------------------

void AccessibleDialogWindow::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if ( m_pDialogWindow )
    {
        if (DlgEdObj* pDlgEdObj = m_aAccessibleChildren[nChildIndex].pDlgEdObj)
        {
            SdrView& rView = m_pDialogWindow->GetView();
            if (SdrPageView* pPgView = rView.GetSdrPageView())
                rView.MarkObj(pDlgEdObj, pPgView);
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool AccessibleDialogWindow::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if (m_pDialogWindow)
        if (DlgEdObj* pDlgEdObj = m_aAccessibleChildren[nChildIndex].pDlgEdObj)
            return m_pDialogWindow->GetView().IsObjMarked(pDlgEdObj);
    return false;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::clearAccessibleSelection(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GetView().UnmarkAll();
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GetView().MarkAll();
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogWindow::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nRet = 0;

    for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
    {
        if ( isAccessibleChildSelected( i ) )
            ++nRet;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogWindow::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;

    for ( sal_Int32 i = 0, j = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
    {
        if ( isAccessibleChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
        {
            xChild = getAccessibleChild( i );
            break;
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

void AccessibleDialogWindow::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if ( m_pDialogWindow )
    {
        if (DlgEdObj* pDlgEdObj = m_aAccessibleChildren[nChildIndex].pDlgEdObj)
        {
            SdrView& rView = m_pDialogWindow->GetView();
            SdrPageView* pPgView = rView.GetSdrPageView();
            if (pPgView)
                rView.MarkObj( pDlgEdObj, pPgView, true );
        }
    }
}

// -----------------------------------------------------------------------------

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
