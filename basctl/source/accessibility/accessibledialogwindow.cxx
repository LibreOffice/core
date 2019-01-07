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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

AccessibleDialogWindow::ChildDescriptor::ChildDescriptor( DlgEdObj* _pDlgEdObj )
    :pDlgEdObj( _pDlgEdObj )
{
}

bool AccessibleDialogWindow::ChildDescriptor::operator==( const ChildDescriptor& rDesc )
{
    bool bRet = false;
    if ( pDlgEdObj == rDesc.pDlgEdObj )
        bRet = true;

    return bRet;
}


bool AccessibleDialogWindow::ChildDescriptor::operator<( const ChildDescriptor& rDesc ) const
{
    bool bRet = false;
    if ( pDlgEdObj && rDesc.pDlgEdObj && pDlgEdObj->GetOrdNum() < rDesc.pDlgEdObj->GetOrdNum() )
        bRet = true;

    return bRet;
}


// class AccessibleDialogWindow


AccessibleDialogWindow::AccessibleDialogWindow (basctl::DialogWindow* pDialogWindow)
    : m_pDialogWindow(pDialogWindow)
    , m_pDlgEdModel(nullptr)
{
    if ( m_pDialogWindow )
    {
        SdrPage& rPage = m_pDialogWindow->GetPage();
        const size_t nCount = rPage.GetObjCount();

        for ( size_t i = 0; i < nCount; ++i )
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


AccessibleDialogWindow::~AccessibleDialogWindow()
{
    if ( m_pDialogWindow )
        m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );

    if ( m_pDlgEdModel )
        EndListening( *m_pDlgEdModel );
}


void AccessibleDialogWindow::UpdateFocused()
{
    for (ChildDescriptor & i : m_aAccessibleChildren)
    {
        Reference< XAccessible > xChild( i.rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetFocused( pShape->IsFocused() );
        }
    }
}


void AccessibleDialogWindow::UpdateSelected()
{
    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

    for (ChildDescriptor & i : m_aAccessibleChildren)
    {
        Reference< XAccessible > xChild( i.rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetSelected( pShape->IsSelected() );
        }
    }
}


void AccessibleDialogWindow::UpdateBounds()
{
    for (ChildDescriptor & i : m_aAccessibleChildren)
    {
        Reference< XAccessible > xChild( i.rxAccessible );
        if ( xChild.is() )
        {
            AccessibleDialogControlShape* pShape = static_cast< AccessibleDialogControlShape* >( xChild.get() );
            if ( pShape )
                pShape->SetBounds( pShape->GetBounds() );
        }
    }
}


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
                const OUString& aLayerName = pSdrLayer->GetName();
                SdrView& rView = m_pDialogWindow->GetView();
                if (rView.IsLayerVisible(aLayerName))
                {
                    // get the bounding box of the shape in logic units
                    tools::Rectangle aRect = pDlgEdObj->GetSnapRect();

                    // transform coordinates relative to the parent
                    MapMode aMap = m_pDialogWindow->GetMapMode();
                    Point aOrg = aMap.GetOrigin();
                    aRect.Move( aOrg.X(), aOrg.Y() );

                    // convert logic units to pixel
                    aRect = m_pDialogWindow->LogicToPixel( aRect, MapMode(MapUnit::Map100thMM) );

                    // check, if the shape's bounding box intersects with the bounding box of its parent
                    tools::Rectangle aParentRect( Point( 0, 0 ), m_pDialogWindow->GetSizePixel() );
                    if ( aParentRect.IsOver( aRect ) )
                        bVisible = true;
                }
            }
        }
    }

    return bVisible;
}


void AccessibleDialogWindow::InsertChild( const ChildDescriptor& rDesc )
{
    // check, if object is already in child list
    AccessibleChildren::iterator aIter = std::find( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end(), rDesc );

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


void AccessibleDialogWindow::RemoveChild( const ChildDescriptor& rDesc )
{
    // find object in child list
    AccessibleChildren::iterator aIter = std::find( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end(), rDesc );

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


void AccessibleDialogWindow::UpdateChildren()
{
    if ( m_pDialogWindow )
    {
        SdrPage& rPage = m_pDialogWindow->GetPage();
        for ( size_t i = 0, nCount = rPage.GetObjCount(); i < nCount; ++i )
            if (DlgEdObj* pDlgEdObj = dynamic_cast<DlgEdObj*>(rPage.GetObj(i)))
                UpdateChild( ChildDescriptor( pDlgEdObj ) );
    }
}


void AccessibleDialogWindow::SortChildren()
{
    // sort child list
    std::sort( m_aAccessibleChildren.begin(), m_aAccessibleChildren.end() );
}


IMPL_LINK( AccessibleDialogWindow, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    DBG_ASSERT(rEvent.GetWindow(), "AccessibleDialogWindow::WindowEventListener: no window!");
    if (!rEvent.GetWindow()->IsAccessibilityEventsSuppressed() || rEvent.GetId() == VclEventId::ObjectDying)
        ProcessWindowEvent(rEvent);
}


void AccessibleDialogWindow::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    Any aOldValue, aNewValue;

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::WindowEnabled:
        {
            aNewValue <<= AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowDisabled:
        {
            aOldValue <<= AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowActivate:
        {
            aNewValue <<= AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowDeactivate:
        {
            aOldValue <<= AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowGetFocus:
        {
            aNewValue <<= AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowLoseFocus:
        {
            aOldValue <<= AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowShow:
        {
            aNewValue <<= AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowHide:
        {
            aOldValue <<= AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VclEventId::WindowResize:
        {
            NotifyAccessibleEvent( AccessibleEventId::BOUNDRECT_CHANGED, aOldValue, aNewValue );
            UpdateChildren();
            UpdateBounds();
        }
        break;
        case VclEventId::ObjectDying:
        {
            if ( m_pDialogWindow )
            {
                m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );
                m_pDialogWindow = nullptr;

                if ( m_pDlgEdModel )
                    EndListening( *m_pDlgEdModel );
                m_pDlgEdModel = nullptr;

                // dispose all children
                for (ChildDescriptor & i : m_aAccessibleChildren)
                {
                    Reference< XComponent > xComponent( i.rxAccessible, UNO_QUERY );
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


// OCommonAccessibleComponent


awt::Rectangle AccessibleDialogWindow::implGetBounds()
{
    awt::Rectangle aBounds;
    if ( m_pDialogWindow )
        aBounds = AWTRectangle( tools::Rectangle( m_pDialogWindow->GetPosPixel(), m_pDialogWindow->GetSizePixel() ) );

    return aBounds;
}


// SfxListener


void AccessibleDialogWindow::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (SdrHint const* pSdrHint = dynamic_cast<SdrHint const*>(&rHint))
    {
        switch ( pSdrHint->GetKind() )
        {
            case SdrHintKind::ObjectInserted:
            {
                if (DlgEdObj const* pDlgEdObj = dynamic_cast<DlgEdObj const*>(pSdrHint->GetObject()))
                {
                    ChildDescriptor aDesc(const_cast<DlgEdObj*>(pDlgEdObj));
                    if ( IsChildVisible( aDesc ) )
                        InsertChild( aDesc );
                }
            }
            break;
            case SdrHintKind::ObjectRemoved:
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


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( AccessibleDialogWindow, OAccessibleExtendedComponentHelper, AccessibleDialogWindow_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleDialogWindow, OAccessibleExtendedComponentHelper, AccessibleDialogWindow_BASE )


// XComponent


void AccessibleDialogWindow::disposing()
{
    OAccessibleExtendedComponentHelper::disposing();

    if ( m_pDialogWindow )
    {
        m_pDialogWindow->RemoveEventListener( LINK( this, AccessibleDialogWindow, WindowEventListener ) );
        m_pDialogWindow = nullptr;

        if ( m_pDlgEdModel )
            EndListening( *m_pDlgEdModel );
        m_pDlgEdModel = nullptr;

        // dispose all children
        for (ChildDescriptor & i : m_aAccessibleChildren)
        {
            Reference< XComponent > xComponent( i.rxAccessible, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }
}

// XServiceInfo
OUString AccessibleDialogWindow::getImplementationName()
{
    return OUString( "com.sun.star.comp.basctl.AccessibleWindow" );
}

sal_Bool AccessibleDialogWindow::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > AccessibleDialogWindow::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleWindow" };
}

// XAccessible
Reference< XAccessibleContext > AccessibleDialogWindow::getAccessibleContext(  )
{
    return this;
}

// XAccessibleContext
sal_Int32 AccessibleDialogWindow::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}


Reference< XAccessible > AccessibleDialogWindow::getAccessibleChild( sal_Int32 i )
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


Reference< XAccessible > AccessibleDialogWindow::getAccessibleParent(  )
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pDialogWindow )
    {
        vcl::Window* pParent = m_pDialogWindow->GetAccessibleParentWindow();
        if ( pParent )
            xParent = pParent->GetAccessible();
    }

    return xParent;
}


sal_Int32 AccessibleDialogWindow::getAccessibleIndexInParent(  )
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;
    if ( m_pDialogWindow )
    {
        vcl::Window* pParent = m_pDialogWindow->GetAccessibleParentWindow();
        if ( pParent )
        {
            for ( sal_uInt16 i = 0, nCount = pParent->GetAccessibleChildWindowCount(); i < nCount; ++i )
            {
                vcl::Window* pChild = pParent->GetAccessibleChildWindow( i );
                if ( pChild == static_cast< vcl::Window* >( m_pDialogWindow ) )
                {
                    nIndexInParent = i;
                    break;
                }
            }
        }
    }

    return nIndexInParent;
}


sal_Int16 AccessibleDialogWindow::getAccessibleRole(  )
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::PANEL;
}


OUString AccessibleDialogWindow::getAccessibleDescription(  )
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pDialogWindow )
        sDescription = m_pDialogWindow->GetAccessibleDescription();

    return sDescription;
}


OUString AccessibleDialogWindow::getAccessibleName(  )
{
    OExternalLockGuard aGuard( this );

    OUString sName;
    if ( m_pDialogWindow )
        sName = m_pDialogWindow->GetAccessibleName();

    return sName;
}


Reference< XAccessibleRelationSet > AccessibleDialogWindow::getAccessibleRelationSet(  )
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}


Reference< XAccessibleStateSet > AccessibleDialogWindow::getAccessibleStateSet(  )
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


Locale AccessibleDialogWindow::getLocale(  )
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}


// XAccessibleComponent


Reference< XAccessible > AccessibleDialogWindow::getAccessibleAtPoint( const awt::Point& rPoint )
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xChild;
    for ( size_t i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xAcc = getAccessibleChild( i );
        if ( xAcc.is() )
        {
            Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
            if ( xComp.is() )
            {
                tools::Rectangle aRect = VCLRectangle( xComp->getBounds() );
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


void AccessibleDialogWindow::grabFocus(  )
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GrabFocus();
}


sal_Int32 AccessibleDialogWindow::getForeground(  )
{
    OExternalLockGuard aGuard( this );

    Color nColor;
    if ( m_pDialogWindow )
    {
        if ( m_pDialogWindow->IsControlForeground() )
            nColor = m_pDialogWindow->GetControlForeground();
        else
        {
            vcl::Font aFont;
            if ( m_pDialogWindow->IsControlFont() )
                aFont = m_pDialogWindow->GetControlFont();
            else
                aFont = m_pDialogWindow->GetFont();
            nColor = aFont.GetColor();
        }
    }

    return sal_Int32(nColor);
}


sal_Int32 AccessibleDialogWindow::getBackground(  )
{
    OExternalLockGuard aGuard( this );

    Color nColor;
    if ( m_pDialogWindow )
    {
        if ( m_pDialogWindow->IsControlBackground() )
            nColor = m_pDialogWindow->GetControlBackground();
        else
            nColor = m_pDialogWindow->GetBackground().GetColor();
    }

    return sal_Int32(nColor);
}


// XAccessibleExtendedComponent


Reference< awt::XFont > AccessibleDialogWindow::getFont(  )
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    if ( m_pDialogWindow )
    {
        Reference< awt::XDevice > xDev( m_pDialogWindow->GetComponentInterface(), UNO_QUERY );
        if ( xDev.is() )
        {
            vcl::Font aFont;
            if ( m_pDialogWindow->IsControlFont() )
                aFont = m_pDialogWindow->GetControlFont();
            else
                aFont = m_pDialogWindow->GetFont();
            VCLXFont* pVCLXFont = new VCLXFont;
            pVCLXFont->Init( *xDev, aFont );
            xFont = pVCLXFont;
        }
    }

    return xFont;
}


OUString AccessibleDialogWindow::getTitledBorderText(  )
{
    OExternalLockGuard aGuard( this );

    return OUString();
}


OUString AccessibleDialogWindow::getToolTipText(  )
{
    OExternalLockGuard aGuard( this );

    OUString sText;
    if ( m_pDialogWindow )
        sText = m_pDialogWindow->GetQuickHelpText();

    return sText;
}


// XAccessibleSelection


void AccessibleDialogWindow::selectAccessibleChild( sal_Int32 nChildIndex )
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


sal_Bool AccessibleDialogWindow::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if (m_pDialogWindow)
        if (DlgEdObj* pDlgEdObj = m_aAccessibleChildren[nChildIndex].pDlgEdObj)
            return m_pDialogWindow->GetView().IsObjMarked(pDlgEdObj);
    return false;
}


void AccessibleDialogWindow::clearAccessibleSelection()
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GetView().UnmarkAll();
}


void AccessibleDialogWindow::selectAllAccessibleChildren(  )
{
    OExternalLockGuard aGuard( this );

    if ( m_pDialogWindow )
        m_pDialogWindow->GetView().MarkAll();
}


sal_Int32 AccessibleDialogWindow::getSelectedAccessibleChildCount(  )
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


Reference< XAccessible > AccessibleDialogWindow::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
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


void AccessibleDialogWindow::deselectAccessibleChild( sal_Int32 nChildIndex )
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


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
