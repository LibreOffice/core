/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleChartElement.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:14:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "AccessibleChartElement.hxx"
#include "CharacterProperties.hxx"
#include "ObjectIdentifier.hxx"
#include "ObjectNameProvider.hxx"
#include "servicenames.hxx"
#include "macros.hxx"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

// for SolarMutex
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

// #ifndef _RTL_UUID_H_
// #include <rtl/uuid.h>
// #endif
// #ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
// #include <cppuhelper/queryinterface.hxx>
// #endif
// #ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
// #include <toolkit/helper/vclunohelper.hxx>
// #endif
// #ifndef _SV_WINDOW_HXX
// #include <vcl/window.hxx>
// #endif

// #ifndef _SVX_ACCESSILE_TEXT_HELPER_HXX_
// #include <svx/AccessibleTextHelper.hxx>
// #endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::UNO_QUERY;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::osl::MutexGuard;
using ::osl::ClearableMutexGuard;
using ::osl::ResettableMutexGuard;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;

namespace chart
{

AccessibleChartElement::AccessibleChartElement(
    const AccessibleElementInfo & rAccInfo,
    bool bMayHaveChildren,
    bool bAlwaysTransparent /* default: false */ ) :
        impl::AccessibleChartElement_Base( rAccInfo, bMayHaveChildren, bAlwaysTransparent ),
        m_bHasText( false )
{
    AddState( AccessibleStateType::TRANSIENT );
}

AccessibleChartElement::~AccessibleChartElement()
{
    OSL_ASSERT( CheckDisposeState( false /* don't throw exceptions */ ) );
}

// ________ protected ________

bool AccessibleChartElement::ImplUpdateChildren()
{
    bool bResult = false;
    Reference< chart2::XTitle > xTitle(
        ObjectIdentifier::getObjectPropertySet(
            GetInfo().m_aCID, Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument )),
        uno::UNO_QUERY );
    m_bHasText = xTitle.is();

    if( m_bHasText )
    {
        InitTextEdit();
        bResult = true;
    }
    else
        bResult = AccessibleBase::ImplUpdateChildren();

    return bResult;
}

void AccessibleChartElement::InitTextEdit()
{
    if( ! m_xTextHelper.is())
    {
        // get hard reference
        Reference< view::XSelectionSupplier > xSelSupp( GetInfo().m_xSelectionSupplier );
        // get factory from selection supplier (controller)
        Reference< lang::XMultiServiceFactory > xFact( xSelSupp, uno::UNO_QUERY );
        if( xFact.is())
        {
            m_xTextHelper.set(
                xFact->createInstance( CHART_ACCESSIBLE_TEXT_SERVICE_NAME ), uno::UNO_QUERY );
        }
    }

    if( m_xTextHelper.is())
        try
        {
            Reference< lang::XInitialization > xInit( m_xTextHelper, uno::UNO_QUERY_THROW );
            Sequence< uno::Any > aArgs( 3 );
            aArgs[0] <<= GetInfo().m_aCID;
            aArgs[1] <<= Reference< XAccessible >( this );
            aArgs[2] <<= Reference< awt::XWindow >( GetInfo().m_xWindow );
            xInit->initialize( aArgs );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
}
//     OSL_ASSERT( m_pTextHelper == 0 );

//     // /-- solar
//     ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
//     Window* pWindow( VCLUnoHelper::GetWindow( GetInfo().m_xWindow ));
//     if( pWindow )
//     {
//         // we need ChartController::m_pDrawViewWrapper here
//         SdrView * pView = 0;
//         if( pView )
//         {
//             SdrObject * pTextObj = m_pDrawViewWrapper->getTextEditObject();
//             if( pTextObj )
//             {
//                 SvxEditSource * pEditSource = new SvxEditSource( pTextObj, pView, pWindow );
//                 m_pTextHelper = new ::accessibility::AccessibleTextHelper(
//                     ::std::auto_ptr< SvxEditSource >( pEditSource ));
//                 if( m_pTextHelper )
//                     m_pTextHelper->SetEventSource( this );
//             }
//         }
//     }
//     // \-- solar
// }

// ____________________________________
// ____________________________________
//
//             Interfaces
// ____________________________________
// ____________________________________

// ________ AccessibleBase::XAccessibleContext ________
Reference< XAccessible > AccessibleChartElement::ImplGetAccessibleChildById( sal_Int32 i ) const
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    Reference< XAccessible > xResult;

    if( m_bHasText )
    {
        xResult.set( m_xTextHelper->getAccessibleChild( i ));
        // /-- solar
//         ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
//         if( m_pTextHelper )
//             xResult.set( m_pTextHelper->GetChild( i ) );
        // \-- solar
    }
    else
        xResult.set( AccessibleBase::ImplGetAccessibleChildById( i ));

    return xResult;
}

sal_Int32 AccessibleChartElement::ImplGetAccessibleChildCount() const
    throw (RuntimeException)
{
    if( m_bHasText )
    {
        if( m_xTextHelper.is())
            return m_xTextHelper->getAccessibleChildCount();
        return 0;
    }

    return AccessibleBase::ImplGetAccessibleChildCount();
}

// ________ XServiceInfo ________
OUString SAL_CALL AccessibleChartElement::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "AccessibleChartElement" ));
}

// ________ AccessibleChartElement::XAccessibleContext (overloaded) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleName()
    throw (::com::sun::star::uno::RuntimeException)
{
    return ObjectNameProvider::getNameForCID(
        GetInfo().m_aCID, GetInfo().m_xChartDocument );
}

// ________ AccessibleChartElement::XAccessibleContext (overloaded) ________
OUString SAL_CALL AccessibleChartElement::getAccessibleDescription()
    throw (::com::sun::star::uno::RuntimeException)
{
    return getToolTipText();
}

// ________ AccessibleChartElement::XAccessibleExtendedComponent ________
Reference< awt::XFont > SAL_CALL AccessibleChartElement::getFont()
    throw (uno::RuntimeException)
{
    CheckDisposeState();

    Reference< awt::XFont > xFont;
    Reference< awt::XDevice > xDevice( Reference< awt::XWindow >( GetInfo().m_xWindow ), uno::UNO_QUERY );

    if( xDevice.is())
    {
        Reference< beans::XMultiPropertySet > xObjProp(
            ObjectIdentifier::getObjectPropertySet(
                GetInfo().m_aCID, Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument )), uno::UNO_QUERY );
        awt::FontDescriptor aDescr(
            CharacterProperties::createFontDescriptorFromPropertySet( xObjProp ));
        xFont = xDevice->getFont( aDescr );
    }

    return xFont;
}

OUString SAL_CALL AccessibleChartElement::getTitledBorderText()
    throw (uno::RuntimeException)
{
    return OUString();
}

OUString SAL_CALL AccessibleChartElement::getToolTipText()
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposeState();

    return ObjectNameProvider::getHelpText(
        GetInfo().m_aCID, Reference< chart2::XChartDocument >( GetInfo().m_xChartDocument ));
}

// ________ XAccessibleComponent ________
sal_Bool SAL_CALL AccessibleChartElement::containsPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    return AccessibleBase::containsPoint( aPoint );
}

Reference< accessibility::XAccessible > SAL_CALL AccessibleChartElement::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    return AccessibleBase::getAccessibleAtPoint( aPoint );
}

awt::Rectangle SAL_CALL AccessibleChartElement::getBounds()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getBounds();
}

awt::Point SAL_CALL AccessibleChartElement::getLocation()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getLocation();
}

awt::Point SAL_CALL AccessibleChartElement::getLocationOnScreen()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getLocationOnScreen();
}

awt::Size SAL_CALL AccessibleChartElement::getSize()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getSize();
}

void SAL_CALL AccessibleChartElement::grabFocus()
    throw (uno::RuntimeException)
{
    return AccessibleBase::grabFocus();
}

sal_Int32 SAL_CALL AccessibleChartElement::getForeground()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getForeground();
}

sal_Int32 SAL_CALL AccessibleChartElement::getBackground()
    throw (uno::RuntimeException)
{
    return AccessibleBase::getBackground();
}


} // namespace chart
