/*************************************************************************
 *
 *  $RCSfile: vclxaccessiblecomponent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2002-02-14 17:48:20 $
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


#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/window.hxx>

#include <unotools/accessiblestatesethelper.hxx>

//  ----------------------------------------------------
//  class VCLXAccessibleComponent
//  ----------------------------------------------------
VCLXAccessibleComponent::VCLXAccessibleComponent( VCLXWindow* pVCLXindow )
    : cppu::WeakComponentImplHelper3<
        ::drafts::com::sun::star::accessibility::XAccessible,
        ::drafts::com::sun::star::accessibility::XAccessibleContext,
        ::drafts::com::sun::star::accessibility::XAccessibleComponent> ( GetMutex() )
{
    mpVCLXindow = pVCLXindow;
    mxWindow = pVCLXindow;
}

VCLXAccessibleComponent::~VCLXAccessibleComponent()
{
}

void VCLXAccessibleComponent::disposing()
{
    mxWindow.clear();
}

Window* VCLXAccessibleComponent::GetWindow() const
{
    return GetVCLXWindow() ? GetVCLXWindow()->GetWindow() : NULL;
}

// ::drafts::com::sun::star::accessibility::XAccessible
::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > VCLXAccessibleComponent::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext > xAcc( (::cppu::OWeakObject*)this, ::com::sun::star::uno::UNO_QUERY );
    return xAcc;
}

void VCLXAccessibleComponent::FillAccessibleStateSet( AccessibleStateSetHelper& rStateSet )
{
}


// ::drafts::com::sun::star::accessibility::XAccessibleContext
sal_Int32 VCLXAccessibleComponent::getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nChildren = 0;
    if ( GetWindow() )
        nChildren = GetWindow()->GetAccessibleChildWindowCount();

    return nChildren;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        if ( i >= getAccessibleChildCount() )
            throw ::com::sun::star::lang::IndexOutOfBoundsException();


        Window* pChild = GetWindow()->GetAccessibleChildWindow( (USHORT)i );
        if ( pChild )
            xAcc = pChild->GetAccessible();
    }

    return xAcc;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
            xAcc = GetWindow()->GetParent()->GetAccessible();
    }

    return xAcc;
}

sal_Int32 VCLXAccessibleComponent::getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nIndex = 0;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
        {
            for ( USHORT n = pParent->GetChildCount(); n; )
            {
                Window* pChild = pParent->GetChild( --n );
                if ( pChild == GetWindow() )
                {
                    nIndex = n;
                    break;
                }
            }
        }
    }
    return nIndex;
}

sal_Int16 VCLXAccessibleComponent::getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // MT: Move to VCL Window, so that everybody can overwrite the role!

    using namespace ::drafts::com::sun::star;

    sal_Int16 nRole = 0;
    if ( GetWindow() )
    {
        switch ( GetWindow()->GetType() )
        {
            case WINDOW_MESSBOX:    // MT: Would be nice to have special roles!
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX: nRole = accessibility::AccessibleRole::OPTIONPANE; break;

            case WINDOW_MODELESSDIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_SYSTEMDIALOG:
            case WINDOW_PRINTERSETUPDIALOG:
            case WINDOW_PRINTDIALOG:
            case WINDOW_TABDIALOG:
            case WINDOW_BUTTONDIALOG:
            case WINDOW_DIALOG: nRole = accessibility::AccessibleRole::DIALOG; break;

            case WINDOW_PUSHBUTTON:
            case WINDOW_OKBUTTON:
            case WINDOW_CANCELBUTTON:
            case WINDOW_HELPBUTTON:
            case WINDOW_IMAGEBUTTON:
            case WINDOW_MENUBUTTON:
            case WINDOW_MOREBUTTON:
            case WINDOW_SPINBUTTON:
            case WINDOW_BUTTON: nRole = accessibility::AccessibleRole::PUSHBUTTON; break;

            case WINDOW_PATHDIALOG: nRole = accessibility::AccessibleRole::DIRECTORYPANE; break;
            case WINDOW_FILEDIALOG: nRole = accessibility::AccessibleRole::FILECHOOSER; break;
            case WINDOW_COLORDIALOG: nRole = accessibility::AccessibleRole::COLORCHOOSER; break;
            case WINDOW_FONTDIALOG: nRole = accessibility::AccessibleRole::FONTCHOOSER; break;

            case WINDOW_IMAGERADIOBUTTON:
            case WINDOW_RADIOBUTTON: nRole = accessibility::AccessibleRole::RADIOBUTTON; break;
            case WINDOW_TRISTATEBOX:
            case WINDOW_CHECKBOX: nRole = accessibility::AccessibleRole::CHECKBOX; break;

            case WINDOW_MULTILINEEDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_EDIT: nRole = ( GetWindow()->GetStyle() & WB_PASSWORD ) ? (accessibility::AccessibleRole::PASSWORDTEXT) : (accessibility::AccessibleRole::TEXT); break;

            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_LONGCURRENCYBOX:
            case WINDOW_COMBOBOX: nRole = accessibility::AccessibleRole::COMBOBOX; break;

            case WINDOW_LISTBOX:
            case WINDOW_MULTILISTBOX: nRole = accessibility::AccessibleRole::LIST; break;

            case WINDOW_FIXEDTEXT: nRole = accessibility::AccessibleRole::LABEL; break;
            case WINDOW_FIXEDBORDER:
            case WINDOW_FIXEDLINE: nRole = accessibility::AccessibleRole::SEPARATOR; break;
            case WINDOW_FIXEDBITMAP:
            case WINDOW_FIXEDIMAGE: nRole = accessibility::AccessibleRole::ICON; break;
            case WINDOW_GROUPBOX: nRole = accessibility::AccessibleRole::GROUPBOX; break;
            case WINDOW_SCROLLBAR: nRole = accessibility::AccessibleRole::SCROLLBAR; break;

            case WINDOW_SLIDER:
            case WINDOW_SPLITTER:
            case WINDOW_SPLITWINDOW: nRole = accessibility::AccessibleRole::SPLITPANE; break;

            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD: nRole = accessibility::AccessibleRole::DATEEDITOR; break;

            case WINDOW_SPINFIELD: nRole = accessibility::AccessibleRole::SPINBOX; break;

            case WINDOW_TOOLBOX: nRole = accessibility::AccessibleRole::TOOLBAR; break;
            case WINDOW_STATUSBAR: nRole = accessibility::AccessibleRole::STATUSBAR; break;

            case WINDOW_TABPAGE: nRole = accessibility::AccessibleRole::PAGETAB; break;
            case WINDOW_TABCONTROL: nRole = accessibility::AccessibleRole::PAGETABLIST; break;

            case WINDOW_DOCKINGWINDOW:
            case WINDOW_SYSWINDOW:
            case WINDOW_FLOATINGWINDOW: nRole = accessibility::AccessibleRole::LAYEREDPANE; break;

            case WINDOW_WORKWINDOW: nRole = accessibility::AccessibleRole::FRAME; break;


            case WINDOW_SCROLLBARBOX: nRole = accessibility::AccessibleRole::FILLER; break;

            case WINDOW_WINDOW:
            case WINDOW_CONTROL:
            case WINDOW_BORDERWINDOW:
            case WINDOW_SYSTEMCHILDWINDOW:
            default: nRole = accessibility::AccessibleRole::WINDOW;

        }
    }
    return nRole;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aDescription;
    if ( GetWindow() )
    {
//      aDescription = GetWindow()->GetText();
    }
    return aDescription;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aName;
    if ( GetWindow() )
    {
        aName = GetWindow()->GetText();
#ifdef DEBUG
        aName += String( RTL_CONSTASCII_USTRINGPARAM( " (Type = " ) );
        aName += String::CreateFromInt32( GetWindow()->GetType() );
        aName += String( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
#endif
    }
    return aName;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > VCLXAccessibleComponent::getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return NULL;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > VCLXAccessibleComponent::getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

//  AccessibleStateSetHelper* pStateSetHelper = new AccessibleStateSetHelper;
//  ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > xSet = pStateSetHelper;
//  ...
//  FillStateSet( *pStateSetHelper );
    return NULL;
}

::com::sun::star::lang::Locale VCLXAccessibleComponent::getLocale() throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::lang::Locale aLocale;
    if ( GetWindow() )
        aLocale = GetWindow()->GetSettings().GetUILocale();

    return aLocale;
}

void VCLXAccessibleComponent::addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
}

void VCLXAccessibleComponent::removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
}

// ::drafts::com::sun::star::accessibility::XAccessibleComponent
sal_Bool VCLXAccessibleComponent::contains( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return sal_False;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleAt( const ::com::sun::star::awt::Point& rPoint ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > xAcc;

    if ( GetWindow() )
    {
        Window* pWindow = GetWindow()->FindWindow( VCLPoint( rPoint ) );
        if ( !pWindow )
            pWindow = GetWindow();

        xAcc = pWindow->GetAccessible();

    }
    return xAcc;
}

::com::sun::star::awt::Size VCLXAccessibleComponent::getSize(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Size aSize;
    if ( GetWindow() )
        aSize = AWTSize( GetWindow()->GetSizePixel() );
    return aSize;

}

::com::sun::star::awt::Rectangle VCLXAccessibleComponent::getBounds() throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Rectangle aBounds;
    if ( mxWindow.is() )
        aBounds = mxWindow->getPosSize();
    return aBounds;

}

::com::sun::star::awt::Point VCLXAccessibleComponent::getLocation() throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::Point aPos;
    if ( GetWindow() )
    {
        Point aVclPos = GetWindow()->GetPosPixel();
        aPos.X = aVclPos.X();
        aPos.Y = aVclPos.Y();
    }

    return aPos;
}

::com::sun::star::awt::Point VCLXAccessibleComponent::getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::awt::Point aPos;
    if ( GetWindow() )
    {
        Rectangle aRect = GetWindow()->GetWindowExtentsRelative( NULL );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }

    return aPos;
}

sal_Bool VCLXAccessibleComponent::isShowing(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bShowing = sal_False;
    if ( GetWindow() && GetWindow()->IsVisible() )
        bShowing = GetWindow()->IsReallyVisible();  // I hope IsReallyVisible is doing everything I need?

    return bShowing;
}

sal_Bool VCLXAccessibleComponent::isVisible(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bVisible = sal_False;
    if ( GetWindow() )
        bVisible = GetWindow()->IsVisible();

    return bVisible;
}

sal_Bool VCLXAccessibleComponent::isFocusTraversable(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return FALSE;
}

void VCLXAccessibleComponent::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() )
        mxWindow->addFocusListener( xListener );
}

void VCLXAccessibleComponent::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() )
        mxWindow->removeFocusListener( xListener );
}

void VCLXAccessibleComponent::grabFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() && isFocusTraversable() )
        mxWindow->setFocus();
}

::com::sun::star::uno::Any VCLXAccessibleComponent::getAccessibleKeyBinding(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return ::com::sun::star::uno::Any();
}
