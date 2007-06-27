/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletabbarpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:33:34 $
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
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGE_HXX_
#include <accessibility/extended/accessibletabbarpage.hxx>
#endif

#ifndef _TABBAR_HXX
#include <svtools/tabbar.hxx>
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif


//.........................................................................
namespace accessibility
{
//.........................................................................

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;

    // -----------------------------------------------------------------------------
    // class AccessibleTabBarPage
    // -----------------------------------------------------------------------------

    AccessibleTabBarPage::AccessibleTabBarPage( TabBar* pTabBar, sal_uInt16 nPageId, const Reference< XAccessible >& rxParent )
        :AccessibleTabBarBase( pTabBar )
        ,m_nPageId( nPageId )
        ,m_xParent( rxParent )
    {
        m_bEnabled  = IsEnabled();
        m_bShowing  = IsShowing();
        m_bSelected = IsSelected();

        if ( m_pTabBar )
            m_sPageText = m_pTabBar->GetPageText( m_nPageId );
    }

    // -----------------------------------------------------------------------------

    AccessibleTabBarPage::~AccessibleTabBarPage()
    {
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPage::IsEnabled()
    {
        OExternalLockGuard aGuard( this );

        sal_Bool bEnabled = sal_False;
        if ( m_pTabBar )
            bEnabled = m_pTabBar->IsPageEnabled( m_nPageId );

        return bEnabled;
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPage::IsShowing()
    {
        sal_Bool bShowing = sal_False;

        if ( m_pTabBar && m_pTabBar->IsVisible() )
            bShowing = sal_True;

        return bShowing;
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPage::IsSelected()
    {
        sal_Bool bSelected = sal_False;

        if ( m_pTabBar && m_pTabBar->GetCurPageId() == m_nPageId )
            bSelected = sal_True;

        return bSelected;
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::SetEnabled( sal_Bool bEnabled )
    {
        if ( m_bEnabled != bEnabled )
        {
            Any aOldValue[2], aNewValue[2];
            if ( m_bEnabled )
            {
                aOldValue[0] <<= AccessibleStateType::SENSITIVE;
                aOldValue[1] <<= AccessibleStateType::ENABLED;
            }
            else
            {

                aNewValue[0] <<= AccessibleStateType::ENABLED;
                aNewValue[1] <<= AccessibleStateType::SENSITIVE;
            }
            m_bEnabled = bEnabled;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[0], aNewValue[0] );
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[1], aNewValue[1] );
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::SetShowing( sal_Bool bShowing )
    {
        if ( m_bShowing != bShowing )
        {
            Any aOldValue, aNewValue;
            if ( m_bShowing )
                aOldValue <<= AccessibleStateType::SHOWING;
            else
                aNewValue <<= AccessibleStateType::SHOWING;
            m_bShowing = bShowing;
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::SetSelected( sal_Bool bSelected )
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

    void AccessibleTabBarPage::SetPageText( const ::rtl::OUString& sPageText )
    {
        if ( !m_sPageText.equals( sPageText ) )
        {
            Any aOldValue, aNewValue;
            aOldValue <<= m_sPageText;
            aNewValue <<= sPageText;
            m_sPageText = sPageText;
            NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        if ( IsEnabled() )
        {
            rStateSet.AddState( AccessibleStateType::ENABLED );
            rStateSet.AddState( AccessibleStateType::SENSITIVE );
        }

        rStateSet.AddState( AccessibleStateType::VISIBLE );

        if ( IsShowing() )
            rStateSet.AddState( AccessibleStateType::SHOWING );

        rStateSet.AddState( AccessibleStateType::SELECTABLE );

        if ( IsSelected() )
            rStateSet.AddState( AccessibleStateType::SELECTED );
    }

    // -----------------------------------------------------------------------------
    // OCommonAccessibleComponent
    // -----------------------------------------------------------------------------

    awt::Rectangle AccessibleTabBarPage::implGetBounds() throw (RuntimeException)
    {
        awt::Rectangle aBounds;
        if ( m_pTabBar )
        {
            // get bounding rectangle relative to the AccessibleTabBar
            aBounds = AWTRectangle( m_pTabBar->GetPageRect( m_nPageId ) );

            // get position of the AccessibleTabBarPageList relative to the AccessibleTabBar
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentLoc = xParentComponent->getLocation();

                    // calculate bounding rectangle relative to the AccessibleTabBarPageList
                    aBounds.X -= aParentLoc.X;
                    aBounds.Y -= aParentLoc.Y;
                }
            }
        }

        return aBounds;
    }

    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabBarPage, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPage_BASE )

    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabBarPage, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPage_BASE )

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::disposing()
    {
        AccessibleTabBarBase::disposing();
        m_sPageText = ::rtl::OUString();
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBarPage::getImplementationName() throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( "com.sun.star.comp.svtools.AccessibleTabBarPage" );
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPage::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > AccessibleTabBarPage::getSupportedServiceNames() throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames(1);
        aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleTabBarPage" );
        return aNames;
    }

    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------

    Reference< XAccessibleContext > AccessibleTabBarPage::getAccessibleContext(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return this;
    }

    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPage::getAccessibleChildCount() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return 0;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPage::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( i < 0 || i >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        return Reference< XAccessible >();
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPage::getAccessibleParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_xParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPage::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nIndexInParent = -1;
        if ( m_pTabBar )
            nIndexInParent = m_pTabBar->GetPagePos( m_nPageId );

        return nIndexInParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 AccessibleTabBarPage::getAccessibleRole(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return AccessibleRole::PAGE_TAB;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBarPage::getAccessibleDescription( ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        ::rtl::OUString sDescription;
        if ( m_pTabBar )
            sDescription = m_pTabBar->GetHelpText( m_nPageId );

        return sDescription;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBarPage::getAccessibleName(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleRelationSet > AccessibleTabBarPage::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
        return xSet;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleStateSet > AccessibleTabBarPage::getAccessibleStateSet(  ) throw (RuntimeException)
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

    Locale AccessibleTabBarPage::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLocale();
    }

    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPage::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return Reference< XAccessible >();
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPage::grabFocus(  ) throw (RuntimeException)
    {
        // no focus
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPage::getForeground(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nColor = 0;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
            if ( xParentComp.is() )
                nColor = xParentComp->getForeground();
        }

        return nColor;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPage::getBackground(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nColor = 0;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
            if ( xParentComp.is() )
                nColor = xParentComp->getBackground();
        }

        return nColor;
    }

    // -----------------------------------------------------------------------------
    // XAccessibleExtendedComponent
    // -----------------------------------------------------------------------------

    Reference< awt::XFont > AccessibleTabBarPage::getFont(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< awt::XFont > xFont;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleExtendedComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
            if ( xParentComp.is() )
                xFont = xParentComp->getFont();
        }

        return xFont;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBarPage::getTitledBorderText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }

    // -----------------------------------------------------------------------------

    ::rtl::OUString AccessibleTabBarPage::getToolTipText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return ::rtl::OUString();
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace accessibility
//.........................................................................
