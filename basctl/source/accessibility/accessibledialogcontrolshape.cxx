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

#include <accessibledialogcontrolshape.hxx>
#include <baside3.hxx>
#include <dlgeddef.hxx>
#include <dlgedview.hxx>
#include <dlgedobj.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;



// class AccessibleDialogControlShape


AccessibleDialogControlShape::AccessibleDialogControlShape (DialogWindow* pDialogWindow, DlgEdObj* pDlgEdObj)
    :AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    ,m_pDialogWindow( pDialogWindow )
    ,m_pDlgEdObj( pDlgEdObj )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    if ( m_pDlgEdObj )
        m_xControlModel.set( m_pDlgEdObj->GetUnoControlModel(), UNO_QUERY );

    if ( m_xControlModel.is() )
        m_xControlModel->addPropertyChangeListener( OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );

    m_bFocused = IsFocused();
    m_bSelected = IsSelected();
    m_aBounds = GetBounds();
}



AccessibleDialogControlShape::~AccessibleDialogControlShape()
{
    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );

    delete m_pExternalLock;
    m_pExternalLock = nullptr;
}



bool AccessibleDialogControlShape::IsFocused()
{
    bool bFocused = false;
    if ( m_pDialogWindow )
    {
        SdrView& rView = m_pDialogWindow->GetView();
        if (rView.IsObjMarked(m_pDlgEdObj) && rView.GetMarkedObjectList().GetMarkCount() == 1)
            bFocused = true;
    }

    return bFocused;
}



bool AccessibleDialogControlShape::IsSelected()
{
    if ( m_pDialogWindow )
        return m_pDialogWindow->GetView().IsObjMarked(m_pDlgEdObj);
    return false;
}



void AccessibleDialogControlShape::SetFocused( bool bFocused )
{
    if ( m_bFocused != bFocused )
    {
        Any aOldValue, aNewValue;
        if ( m_bFocused )
            aOldValue <<= AccessibleStateType::FOCUSED;
        else
            aNewValue <<= AccessibleStateType::FOCUSED;
        m_bFocused = bFocused;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}



void AccessibleDialogControlShape::SetSelected( bool bSelected )
{
    if ( m_bSelected != bSelected )
    {
        Any aOldValue, aNewValue;
        if ( m_bSelected )
            aOldValue <<= AccessibleStateType::SELECTED;
        else
            aNewValue <<= AccessibleStateType::SELECTED;
        m_bSelected = bSelected;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}



awt::Rectangle AccessibleDialogControlShape::GetBounds()
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( m_pDlgEdObj )
    {
        // get the bounding box of the shape in logic units
        Rectangle aRect = m_pDlgEdObj->GetSnapRect();

        if ( m_pDialogWindow )
        {
            // transform coordinates relative to the parent
            MapMode aMap = m_pDialogWindow->GetMapMode();
            Point aOrg = aMap.GetOrigin();
            aRect.Move( aOrg.X(), aOrg.Y() );

            // convert logic units to pixel
            aRect = m_pDialogWindow->LogicToPixel( aRect, MapMode(MAP_100TH_MM) );

            // clip the shape's bounding box with the bounding box of its parent
            Rectangle aParentRect( Point( 0, 0 ), m_pDialogWindow->GetSizePixel() );
            aRect = aRect.GetIntersection( aParentRect );
            aBounds = AWTRectangle( aRect );
        }
    }

    return aBounds;
}



void AccessibleDialogControlShape::SetBounds( const awt::Rectangle& aBounds )
{
    if ( m_aBounds.X != aBounds.X || m_aBounds.Y != aBounds.Y || m_aBounds.Width != aBounds.Width || m_aBounds.Height != aBounds.Height )
    {
        m_aBounds = aBounds;
        NotifyAccessibleEvent( AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any() );
    }
}



vcl::Window* AccessibleDialogControlShape::GetWindow() const
{
    vcl::Window* pWindow = nullptr;
    if ( m_pDlgEdObj )
    {
        Reference< awt::XControl > xControl( m_pDlgEdObj->GetControl(), UNO_QUERY );
        if ( xControl.is() )
            pWindow = VCLUnoHelper::GetWindow( xControl->getPeer() );
    }

    return pWindow;
}



OUString AccessibleDialogControlShape::GetModelStringProperty( const sal_Char* pPropertyName )
{
    OUString sReturn;

    try
    {
        if ( m_xControlModel.is() )
        {
            OUString sPropertyName( OUString::createFromAscii( pPropertyName ) );
            Reference< XPropertySetInfo > xInfo = m_xControlModel->getPropertySetInfo();
            if ( xInfo.is() && xInfo->hasPropertyByName( sPropertyName ) )
                m_xControlModel->getPropertyValue( sPropertyName ) >>= sReturn;
        }
    }
    catch ( const Exception& )
    {
        OSL_FAIL( "AccessibleDialogControlShape::GetModelStringProperty: caught an exception!" );
    }

    return sReturn;
}



void AccessibleDialogControlShape::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    rStateSet.AddState( AccessibleStateType::ENABLED );

    rStateSet.AddState( AccessibleStateType::VISIBLE );

    rStateSet.AddState( AccessibleStateType::SHOWING );

    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    if ( IsFocused() )
        rStateSet.AddState( AccessibleStateType::FOCUSED );

    rStateSet.AddState( AccessibleStateType::SELECTABLE );

    if ( IsSelected() )
        rStateSet.AddState( AccessibleStateType::SELECTED );

    rStateSet.AddState( AccessibleStateType::RESIZABLE );
}

// OCommonAccessibleComponent
awt::Rectangle AccessibleDialogControlShape::implGetBounds() throw (RuntimeException, std::exception)
{
    return GetBounds();
}

// XInterface
IMPLEMENT_FORWARD_XINTERFACE2( AccessibleDialogControlShape, AccessibleExtendedComponentHelper_BASE, AccessibleDialogControlShape_BASE )

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleDialogControlShape, AccessibleExtendedComponentHelper_BASE, AccessibleDialogControlShape_BASE )

// XComponent
void AccessibleDialogControlShape::disposing()
{
    AccessibleExtendedComponentHelper_BASE::disposing();

    m_pDialogWindow = nullptr;
    m_pDlgEdObj = nullptr;

    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );
    m_xControlModel.clear();
}


// XEventListener


void AccessibleDialogControlShape::disposing( const lang::EventObject& ) throw (RuntimeException, std::exception)
{
    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );
    m_xControlModel.clear();
}


// XPropertyChangeListener


void AccessibleDialogControlShape::propertyChange( const beans::PropertyChangeEvent& rEvent ) throw (RuntimeException, std::exception)
{
    if ( rEvent.PropertyName == DLGED_PROP_NAME )
    {
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, rEvent.OldValue, rEvent.NewValue );
    }
    else if ( rEvent.PropertyName == DLGED_PROP_POSITIONX ||
              rEvent.PropertyName == DLGED_PROP_POSITIONY ||
              rEvent.PropertyName == DLGED_PROP_WIDTH ||
              rEvent.PropertyName == DLGED_PROP_HEIGHT )
    {
        SetBounds( GetBounds() );
    }
    else if ( rEvent.PropertyName == DLGED_PROP_BACKGROUNDCOLOR ||
              rEvent.PropertyName == DLGED_PROP_TEXTCOLOR ||
              rEvent.PropertyName == DLGED_PROP_TEXTLINECOLOR )
    {
        NotifyAccessibleEvent( AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any() );
    }
}

// XServiceInfo
OUString AccessibleDialogControlShape::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.basctl.AccessibleShape" );
}

sal_Bool AccessibleDialogControlShape::supportsService( const OUString& rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > AccessibleDialogControlShape::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence<OUString> aNames { "com.sun.star.drawing.AccessibleShape" };
    return aNames;
}

// XAccessible
Reference< XAccessibleContext > AccessibleDialogControlShape::getAccessibleContext(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// XAccessibleContext
sal_Int32 AccessibleDialogControlShape::getAccessibleChildCount() throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return 0;
}



Reference< XAccessible > AccessibleDialogControlShape::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}



Reference< XAccessible > AccessibleDialogControlShape::getAccessibleParent(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pDialogWindow )
        xParent = m_pDialogWindow->GetAccessible();

    return xParent;
}



sal_Int32 AccessibleDialogControlShape::getAccessibleIndexInParent(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
        if ( xParentContext.is() )
        {
            for ( sal_Int32 i = 0, nCount = xParentContext->getAccessibleChildCount(); i < nCount; ++i )
            {
                Reference< XAccessible > xChild( xParentContext->getAccessibleChild( i ) );
                if ( xChild.is() )
                {
                    Reference< XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                    if ( xChildContext == static_cast<XAccessibleContext*>(this) )
                    {
                        nIndexInParent = i;
                        break;
                    }
                }
            }
        }
    }

    return nIndexInParent;
}



sal_Int16 AccessibleDialogControlShape::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::SHAPE;
}



OUString AccessibleDialogControlShape::getAccessibleDescription(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return GetModelStringProperty( "HelpText" );
}



OUString AccessibleDialogControlShape::getAccessibleName(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return GetModelStringProperty( "Name" );
}



Reference< XAccessibleRelationSet > AccessibleDialogControlShape::getAccessibleRelationSet(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}



Reference< XAccessibleStateSet > AccessibleDialogControlShape::getAccessibleStateSet(  ) throw (RuntimeException, std::exception)
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



Locale AccessibleDialogControlShape::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}


// XAccessibleComponent


Reference< XAccessible > AccessibleDialogControlShape::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}



void AccessibleDialogControlShape::grabFocus(  ) throw (RuntimeException, std::exception)
{
    // no focus for shapes
}



sal_Int32 AccessibleDialogControlShape::getForeground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlForeground() )
            nColor = pWindow->GetControlForeground().GetColor();
        else
        {
            vcl::Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}



sal_Int32 AccessibleDialogControlShape::getBackground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlBackground() )
            nColor = pWindow->GetControlBackground().GetColor();
        else
            nColor = pWindow->GetBackground().GetColor().GetColor();
    }

    return nColor;
}


// XAccessibleExtendedComponent


Reference< awt::XFont > AccessibleDialogControlShape::getFont(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Reference< awt::XDevice > xDev( pWindow->GetComponentInterface(), UNO_QUERY );
        if ( xDev.is() )
        {
            vcl::Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            VCLXFont* pVCLXFont = new VCLXFont;
            pVCLXFont->Init( *xDev.get(), aFont );
            xFont = pVCLXFont;
        }
    }

    return xFont;
}



OUString AccessibleDialogControlShape::getTitledBorderText(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}



OUString AccessibleDialogControlShape::getToolTipText(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    OUString sText;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        sText = pWindow->GetQuickHelpText();

    return sText;
}



} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
