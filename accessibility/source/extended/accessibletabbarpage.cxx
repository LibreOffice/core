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

#include <accessibility/extended/accessibletabbarpage.hxx>
#include <svtools/tabbar.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/convert.hxx>



namespace accessibility
{


    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;


    // class AccessibleTabBarPage


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



    AccessibleTabBarPage::~AccessibleTabBarPage()
    {
    }



    bool AccessibleTabBarPage::IsEnabled()
    {
        OExternalLockGuard aGuard( this );

        bool bEnabled = false;
        if ( m_pTabBar )
            bEnabled = m_pTabBar->IsPageEnabled( m_nPageId );

        return bEnabled;
    }



    bool AccessibleTabBarPage::IsShowing()
    {
        bool bShowing = false;

        if ( m_pTabBar && m_pTabBar->IsVisible() )
            bShowing = true;

        return bShowing;
    }



    bool AccessibleTabBarPage::IsSelected()
    {
        bool bSelected = false;

        if ( m_pTabBar && m_pTabBar->GetCurPageId() == m_nPageId )
            bSelected = true;

        return bSelected;
    }


    void AccessibleTabBarPage::SetShowing( bool bShowing )
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



    void AccessibleTabBarPage::SetSelected( bool bSelected )
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



    void AccessibleTabBarPage::SetPageText( const OUString& sPageText )
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


    // OCommonAccessibleComponent


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


    // XInterface


    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabBarPage, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPage_BASE )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabBarPage, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPage_BASE )


    // XComponent


    void AccessibleTabBarPage::disposing()
    {
        AccessibleTabBarBase::disposing();
        m_sPageText.clear();
    }


    // XServiceInfo


    OUString AccessibleTabBarPage::getImplementationName() throw (RuntimeException, std::exception)
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTabBarPage" );
    }



    sal_Bool AccessibleTabBarPage::supportsService( const OUString& rServiceName ) throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, rServiceName);
    }



    Sequence< OUString > AccessibleTabBarPage::getSupportedServiceNames() throw (RuntimeException, std::exception)
    {
        Sequence< OUString > aNames { "com.sun.star.awt.AccessibleTabBarPage" };
        return aNames;
    }


    // XAccessible


    Reference< XAccessibleContext > AccessibleTabBarPage::getAccessibleContext(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return this;
    }


    // XAccessibleContext


    sal_Int32 AccessibleTabBarPage::getAccessibleChildCount() throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return 0;
    }



    Reference< XAccessible > AccessibleTabBarPage::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        if ( i < 0 || i >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        return Reference< XAccessible >();
    }



    Reference< XAccessible > AccessibleTabBarPage::getAccessibleParent(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return m_xParent;
    }



    sal_Int32 AccessibleTabBarPage::getAccessibleIndexInParent(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        sal_Int32 nIndexInParent = -1;
        if ( m_pTabBar )
            nIndexInParent = m_pTabBar->GetPagePos( m_nPageId );

        return nIndexInParent;
    }



    sal_Int16 AccessibleTabBarPage::getAccessibleRole(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return AccessibleRole::PAGE_TAB;
    }



    OUString AccessibleTabBarPage::getAccessibleDescription( ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        OUString sDescription;
        if ( m_pTabBar )
            sDescription = m_pTabBar->GetHelpText( m_nPageId );

        return sDescription;
    }



    OUString AccessibleTabBarPage::getAccessibleName(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }



    Reference< XAccessibleRelationSet > AccessibleTabBarPage::getAccessibleRelationSet(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
        return xSet;
    }



    Reference< XAccessibleStateSet > AccessibleTabBarPage::getAccessibleStateSet(  ) throw (RuntimeException, std::exception)
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



    Locale AccessibleTabBarPage::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLanguageTag().getLocale();
    }


    // XAccessibleComponent


    Reference< XAccessible > AccessibleTabBarPage::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return Reference< XAccessible >();
    }



    void AccessibleTabBarPage::grabFocus(  ) throw (RuntimeException, std::exception)
    {
        // no focus
    }



    sal_Int32 AccessibleTabBarPage::getForeground(  ) throw (RuntimeException, std::exception)
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



    sal_Int32 AccessibleTabBarPage::getBackground(  ) throw (RuntimeException, std::exception)
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


    // XAccessibleExtendedComponent


    Reference< awt::XFont > AccessibleTabBarPage::getFont(  ) throw (RuntimeException, std::exception)
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



    OUString AccessibleTabBarPage::getTitledBorderText(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }



    OUString AccessibleTabBarPage::getToolTipText(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );

        return OUString();
    }




}   // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
