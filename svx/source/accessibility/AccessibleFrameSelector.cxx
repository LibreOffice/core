/*************************************************************************
 *
 *  $RCSfile: AccessibleFrameSelector.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:39:11 $
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

#ifndef SVX_ACCESSIBLEFRAMESELECTOR_HXX
#include "AccessibleFrameSelector.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEY_HPP_
#include <com/sun/star/awt/Key.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HDL_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HDL_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HDL_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FOCUSCHANGEREASON_HPP_
#include <com/sun/star/awt/FocusChangeReason.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef SVX_FRMSEL_HXX
#include "frmsel.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef SVX_FRMSEL_HRC
#include "frmsel.hrc"
#endif

#ifndef MNEMONIC_CHAR
#define MNEMONIC_CHAR ((sal_Unicode)'~')
#endif

namespace svx {
namespace a11y {

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::awt::XFocusListener;

using namespace ::com::sun::star::accessibility;

namespace AwtKey                    = ::com::sun::star::awt::Key;
namespace AwtKeyModifier            = ::com::sun::star::awt::KeyModifier;
namespace AwtFocusChangeReason      = ::com::sun::star::awt::FocusChangeReason;

typedef ::com::sun::star::awt::Point        AwtPoint;
typedef ::com::sun::star::awt::Size         AwtSize;
typedef ::com::sun::star::awt::Rectangle    AwtRectangle;
typedef ::com::sun::star::awt::KeyEvent     AwtKeyEvent;
typedef ::com::sun::star::awt::FocusEvent   AwtFocusEvent;

// ============================================================================

AccFrameSelector::AccFrameSelector( FrameSelector& rFrameSel, FrameBorderType eBorder ) :
    Resource( SVX_RES( RID_SVXSTR_BORDER_CONTROL ) ),
    maNames( ResId( ARR_TEXTS ) ),
    maDescriptions( ResId(ARR_DESCRIPTIONS ) ),
    mpFrameSel( &rFrameSel ),
    maFocusListeners( maFocusMutex ),
    maPropertyListeners( maPropertyMutex ),
    meBorder( eBorder )
{
    FreeResource();
}

// ----------------------------------------------------------------------------

AccFrameSelector::~AccFrameSelector()
{
}

// ----------------------------------------------------------------------------

Reference< XAccessibleContext > AccFrameSelector::getAccessibleContext(  )
    throw (RuntimeException)
{
    return this;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getAccessibleChildCount(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return (meBorder == FRAMEBORDER_NONE) ? mpFrameSel->GetEnabledBorderCount() : 0;
}

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleChild( sal_Int32 i )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Reference< XAccessible > xRet;
    if( meBorder == FRAMEBORDER_NONE )
        xRet = mpFrameSel->GetChildAccessible( i );
    if( !xRet.is() )
        throw RuntimeException();
    return xRet;
}

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleParent(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Reference< XAccessible > xRet;
    if(meBorder == FRAMEBORDER_NONE)
        xRet = mpFrameSel->GetParent()->GetAccessible( sal_True );
    else
        xRet = mpFrameSel->CreateAccessible();
    return xRet;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getAccessibleIndexInParent(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();

    sal_Int32 nIdx = 0;
    if( meBorder == FRAMEBORDER_NONE )
    {
        Window* pTabPage = mpFrameSel->GetParent();
        sal_Int32 nChildren = pTabPage->GetChildCount();
        for( nIdx = 0; nIdx < nChildren; ++nIdx )
            if( pTabPage->GetChild( static_cast< USHORT >( nIdx ) ) == mpFrameSel )
                break;
    }
    else
        nIdx = mpFrameSel->GetEnabledBorderIndex( meBorder );

    if( nIdx < 0 )
        throw RuntimeException();
    return nIdx;
}

// ----------------------------------------------------------------------------

sal_Int16 AccFrameSelector::getAccessibleRole(  ) throw (RuntimeException)
{
    return AccessibleRole::OPTION_PANE;
}

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getAccessibleDescription(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return maDescriptions.GetString(meBorder);
}

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getAccessibleName(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return maNames.GetString(meBorder);
}

// ----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > AccFrameSelector::getAccessibleRelationSet(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FRAMEBORDER_NONE)
    {
        //add the label relation
        Window* pPrev = mpFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            AccessibleRelation aLabelRelation;
            aLabelRelation.RelationType = AccessibleRelationType::LABELED_BY;
            aLabelRelation.TargetSet.realloc(1);
            aLabelRelation.TargetSet.getArray()[0]  = pPrev->GetAccessible();
            pHelper->AddRelation(aLabelRelation);
        }
    }
    return xRet;
}

// ----------------------------------------------------------------------------

Reference< XAccessibleStateSet > AccFrameSelector::getAccessibleStateSet(  )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xRet = pStateSetHelper;

    if(!mpFrameSel)
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);
    else
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::EDITABLE,
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::MULTI_SELECTABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0};
        sal_Int16 nState = 0;
        while(aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if(mpFrameSel->IsEnabled())
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);

        sal_Bool bIsParent = meBorder == FRAMEBORDER_NONE;
        if(mpFrameSel->HasFocus() &&
            (bIsParent || mpFrameSel->IsBorderSelected(meBorder)))
        {
            pStateSetHelper->AddState(AccessibleStateType::ACTIVE);
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
            pStateSetHelper->AddState(AccessibleStateType::SELECTED);
        }
    }
    return xRet;
}

// ----------------------------------------------------------------------------

Locale AccFrameSelector::getLocale(  )
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    Locale aRet;
    SvxLanguageToLocale( aRet, Application::GetSettings().GetUILanguage() );
    return aRet;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::addPropertyChangeListener(
    const Reference< XPropertyChangeListener >& xListener )
        throw (RuntimeException)
{
    maPropertyListeners.addInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::removePropertyChangeListener( const Reference< XPropertyChangeListener >& xListener )
    throw (RuntimeException)
{
    maPropertyListeners.removeInterface( xListener );
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::containsPoint( const AwtPoint& aPt )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->ContainsClickPoint( Point( aPt.X, aPt.Y ) );
}

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleAtPoint(
    const AwtPoint& aPt )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

AwtRectangle AccFrameSelector::getBounds(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Size aSz;
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aSz = mpFrameSel->GetSizePixel();
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
            aSz = aSpot.GetSize();
    }
    AwtRectangle aRet;
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}

// ----------------------------------------------------------------------------

AwtPoint AccFrameSelector::getLocation(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    AwtPoint aRet(aPos.X(), aPos.Y());
    return aRet;
}

// ----------------------------------------------------------------------------

AwtPoint AccFrameSelector::getLocationOnScreen(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    aPos = mpFrameSel->OutputToAbsoluteScreenPixel( aPos );
    AwtPoint aRet(aPos.X(), aPos.Y());
    return aRet;
}

// ----------------------------------------------------------------------------

AwtSize AccFrameSelector::getSize(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    Size aSz;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aSz = mpFrameSel->GetSizePixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aSz = aSpot.GetSize();
    }
    AwtSize aRet(aSz.Width(), aSz.Height());
    return aRet;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isShowing(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isVisible(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isFocusTraversable(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::addFocusListener( const Reference< XFocusListener >& xListener ) throw (RuntimeException)
{
    maFocusListeners.addInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::removeFocusListener( const Reference< XFocusListener >& xListener ) throw (RuntimeException)
{
    maFocusListeners.removeInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::grabFocus(  ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    mpFrameSel->GrabFocus();
}

// ----------------------------------------------------------------------------

Any AccFrameSelector::getAccessibleKeyBinding(  ) throw (RuntimeException)
{
    Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FRAMEBORDER_NONE)
    {
        Window* pPrev = mpFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            String sText = pPrev->GetText();
            xub_StrLen nFound = sText.Search( MNEMONIC_CHAR );
            if(STRING_NOTFOUND != nFound && ++nFound < sText.Len())
            {
                sText.ToUpperAscii();
                sal_Unicode cChar = sText.GetChar(nFound);
                AwtKeyEvent aEvent;

                aEvent.KeyCode = 0;
                aEvent.KeyChar = cChar;
                aEvent.KeyFunc = 0;
                if(cChar >= 'A' && cChar <= 'Z')
                {
                     aEvent.KeyCode = AwtKey::A + cChar - 'A';
                }
                aEvent.Modifiers = AwtKeyModifier::MOD2;
                aRet <<= aEvent;
            }
        }
    }
    return aRet;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getForeground(  )
        throw (RuntimeException)
{
    Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return mpFrameSel->GetControlForeground().GetColor();
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getBackground(  )
        throw (RuntimeException)
{
    Any aRet;
    vos::OGuard aGuard(Application::GetSolarMutex());
    IsValid();
    return mpFrameSel->GetControlBackground().GetColor();
}

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getImplementationName(  ) throw (RuntimeException)
{
    return OUString::createFromAscii("AccFrameSelector");
}

// ----------------------------------------------------------------------------

const sal_Char sAccessible[]          = "Accessible";
const sal_Char sAccessibleContext[]   = "AccessibleContext";
const sal_Char sAccessibleComponent[] = "AccessibleComponent";

sal_Bool AccFrameSelector::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return  rServiceName.equalsAsciiL( sAccessible         , sizeof(sAccessible         )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleContext  , sizeof(sAccessibleContext  )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleComponent, sizeof(sAccessibleComponent)-1 );
}

// ----------------------------------------------------------------------------

Sequence< OUString > AccFrameSelector::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessible         ) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleContext  ) );
    pArray[2] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleComponent) );
    return aRet;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::IsValid() throw (RuntimeException)
{
    if(!mpFrameSel)
        throw RuntimeException();
}

// ----------------------------------------------------------------------------

void    AccFrameSelector::NotifyFocusListeners(sal_Bool bGetFocus)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    AwtFocusEvent aEvent;
    aEvent.FocusFlags = 0;
    if(bGetFocus)
    {
        USHORT nFocusFlags = mpFrameSel->GetGetFocusFlags();
        if(nFocusFlags&GETFOCUS_TAB)
            aEvent.FocusFlags |= AwtFocusChangeReason::TAB;
        if(nFocusFlags&GETFOCUS_CURSOR)
            aEvent.FocusFlags |= AwtFocusChangeReason::CURSOR;
        if(nFocusFlags&GETFOCUS_MNEMONIC)
            aEvent.FocusFlags |= AwtFocusChangeReason::MNEMONIC;
        if(nFocusFlags&GETFOCUS_FORWARD)
            aEvent.FocusFlags |= AwtFocusChangeReason::FORWARD;
        if(nFocusFlags&GETFOCUS_BACKWARD)
            aEvent.FocusFlags |= AwtFocusChangeReason::BACKWARD;
        if(nFocusFlags&GETFOCUS_AROUND)
            aEvent.FocusFlags |= AwtFocusChangeReason::AROUND;
        if(nFocusFlags&GETFOCUS_UNIQUEMNEMONIC)
            aEvent.FocusFlags |= AwtFocusChangeReason::UNIQUEMNEMONIC;
    //        if(nFocusFlags&GETFOCUS_INIT)
    //            aEvent.FocusFlags |= AwtFocusChangeReason::
    }
//    else
    //how can I find the current focus window?
//        aEvent.NextFocus = ;
    aEvent.Temporary = sal_False;

    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;

    ::cppu::OInterfaceIteratorHelper aIter( maFocusListeners );
    while( aIter.hasMoreElements() )
    {
        Reference < XFocusListener > xListener( aIter.next(), UNO_QUERY );
        if(bGetFocus)
            xListener->focusGained( aEvent );
        else
            xListener->focusLost( aEvent );
    }
}

// ----------------------------------------------------------------------------

void AccFrameSelector::Invalidate()
{
    mpFrameSel = 0;
    EventObject aEvent;
    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;
    maFocusListeners.disposeAndClear( aEvent );
    maPropertyListeners.disposeAndClear( aEvent );
}

// ============================================================================

} // namespace a11y
} // namespace svx

