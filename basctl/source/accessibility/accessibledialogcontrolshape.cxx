/*************************************************************************
 *
 *  $RCSfile: accessibledialogcontrolshape.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:54:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BASCTL_ACCESSIBLEDIALOGCONTROLSHAPE_HXX_
#include <accessibledialogcontrolshape.hxx>
#endif

#ifndef _BASIDE3_HXX
#include <baside3.hxx>
#endif
#ifndef _BASCTL_DLGEDVIEW_HXX
#include <dlgedview.hxx>
#endif
#ifndef _BASCTL_DLGEDOBJ_HXX
#include <dlgedobj.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXFONT_HXX_
#include <toolkit/awt/vclxfont.hxx>
#endif
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#include <toolkit/helper/externallock.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class AccessibleDialogControlShape
// -----------------------------------------------------------------------------

AccessibleDialogControlShape::AccessibleDialogControlShape( DialogWindow* pDialogWindow, DlgEdObj* pDlgEdObj )
    :AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    ,m_pDialogWindow( pDialogWindow )
    ,m_pDlgEdObj( pDlgEdObj )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    if ( m_pDlgEdObj )
        m_xControlModel = Reference< XPropertySet >( m_pDlgEdObj->GetUnoControlModel(), UNO_QUERY );

    if ( m_xControlModel.is() )
        m_xControlModel->addPropertyChangeListener( ::rtl::OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );

    m_bFocused = IsFocused();
    m_bSelected = IsSelected();
    m_aBounds = GetBounds();
}

// -----------------------------------------------------------------------------

AccessibleDialogControlShape::~AccessibleDialogControlShape()
{
    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( ::rtl::OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );

    delete m_pExternalLock;
    m_pExternalLock = NULL;
}

// -----------------------------------------------------------------------------

sal_Bool AccessibleDialogControlShape::IsFocused()
{
    sal_Bool bFocused = sal_False;
    if ( m_pDialogWindow )
    {
        SdrView* pSdrView = m_pDialogWindow->GetView();
        if ( pSdrView && pSdrView->IsObjMarked( m_pDlgEdObj ) && pSdrView->GetMarkedObjectList().GetMarkCount() == 1 )
            bFocused = sal_True;
    }

    return bFocused;
}

// -----------------------------------------------------------------------------

sal_Bool AccessibleDialogControlShape::IsSelected()
{
    sal_Bool bSelected = sal_False;
    if ( m_pDialogWindow )
    {
        SdrView* pSdrView = m_pDialogWindow->GetView();
        if ( pSdrView )
            bSelected = pSdrView->IsObjMarked( m_pDlgEdObj );
    }

    return bSelected;
}

// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::SetFocused( sal_Bool bFocused )
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

// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::SetSelected( sal_Bool bSelected )
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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::SetBounds( const awt::Rectangle& aBounds )
{
    if ( m_aBounds.X != aBounds.X || m_aBounds.Y != aBounds.Y || m_aBounds.Width != aBounds.Width || m_aBounds.Height != aBounds.Height )
    {
        m_aBounds = aBounds;
        NotifyAccessibleEvent( AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any() );
    }
}

// -----------------------------------------------------------------------------

Window* AccessibleDialogControlShape::GetWindow() const
{
    Window* pWindow = NULL;
    if ( m_pDlgEdObj )
    {
        Reference< awt::XControl > xControl( m_pDlgEdObj->GetUnoControl(m_pDialogWindow), UNO_QUERY );
        if ( xControl.is() )
            pWindow = VCLUnoHelper::GetWindow( xControl->getPeer() );
    }

    return pWindow;
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::GetModelStringProperty( const sal_Char* pPropertyName )
{
    ::rtl::OUString sReturn;

    try
    {
        if ( m_xControlModel.is() )
        {
            ::rtl::OUString sPropertyName( ::rtl::OUString::createFromAscii( pPropertyName ) );
            Reference< XPropertySetInfo > xInfo = m_xControlModel->getPropertySetInfo();
            if ( xInfo.is() && xInfo->hasPropertyByName( sPropertyName ) )
                m_xControlModel->getPropertyValue( sPropertyName ) >>= sReturn;
        }
    }
    catch ( const Exception& )
    {
        OSL_ENSURE( sal_False, "AccessibleDialogControlShape::GetModelStringProperty: caught an exception!" );
    }

    return sReturn;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle AccessibleDialogControlShape::implGetBounds() throw (RuntimeException)
{
    return GetBounds();
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( AccessibleDialogControlShape, AccessibleExtendedComponentHelper_BASE, AccessibleDialogControlShape_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleDialogControlShape, AccessibleExtendedComponentHelper_BASE, AccessibleDialogControlShape_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::disposing()
{
    AccessibleExtendedComponentHelper_BASE::disposing();

    m_pDialogWindow = NULL;
    m_pDlgEdObj = NULL;

    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( ::rtl::OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );
    m_xControlModel.clear();
}

// -----------------------------------------------------------------------------
// XEventListener
// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::disposing( const lang::EventObject& rSource ) throw (RuntimeException)
{
    if ( m_xControlModel.is() )
        m_xControlModel->removePropertyChangeListener( ::rtl::OUString(), static_cast< beans::XPropertyChangeListener* >( this ) );
    m_xControlModel.clear();
}

// -----------------------------------------------------------------------------
// XPropertyChangeListener
// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::propertyChange( const beans::PropertyChangeEvent& rEvent ) throw (RuntimeException)
{
    if ( rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) )
    {
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, rEvent.OldValue, rEvent.NewValue );
    }
    else if ( rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ||
              rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ||
              rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ||
              rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) )
    {
        SetBounds( GetBounds() );
    }
    else if ( rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BackgroundColor" ) ) ||
              rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextColor" ) ) ||
              rEvent.PropertyName == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextLineColor" ) ) )
    {
        NotifyAccessibleEvent( AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any() );
    }
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.basctl.AccessibleShape" );
}

// -----------------------------------------------------------------------------

sal_Bool AccessibleDialogControlShape::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > AccessibleDialogControlShape::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.drawing.AccessibleShape" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > AccessibleDialogControlShape::getAccessibleContext(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogControlShape::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogControlShape::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogControlShape::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pDialogWindow )
        xParent = m_pDialogWindow->GetAccessible();

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogControlShape::getAccessibleIndexInParent(  ) throw (RuntimeException)
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
                    if ( xChildContext == (XAccessibleContext*)this )
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

// -----------------------------------------------------------------------------

sal_Int16 AccessibleDialogControlShape::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::SHAPE;
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::getAccessibleDescription( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetModelStringProperty( "HelpText" );
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetModelStringProperty( "Name" );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > AccessibleDialogControlShape::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > AccessibleDialogControlShape::getAccessibleStateSet(  ) throw (RuntimeException)
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

Locale AccessibleDialogControlShape::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > AccessibleDialogControlShape::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

void AccessibleDialogControlShape::grabFocus(  ) throw (RuntimeException)
{
    // no focus for shapes
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogControlShape::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlForeground() )
            nColor = pWindow->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 AccessibleDialogControlShape::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlBackground() )
            nColor = pWindow->GetControlBackground().GetColor();
        else
            nColor = pWindow->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > AccessibleDialogControlShape::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Reference< awt::XDevice > xDev( pWindow->GetComponentInterface(), UNO_QUERY );
        if ( xDev.is() )
        {
            Font aFont;
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

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------

::rtl::OUString AccessibleDialogControlShape::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        sText = pWindow->GetQuickHelpText();

    return sText;
}

// -----------------------------------------------------------------------------

