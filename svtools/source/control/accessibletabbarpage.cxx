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

#include "accessibletabbarpage.hxx"

#include <svtools/tabbar.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/unohelp.hxx>
#include <i18nlangtag/languagetag.hxx>


namespace accessibility
{


    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;




    AccessibleTabBarPage::AccessibleTabBarPage( TabBar* pTabBar, sal_uInt16 nPageId, const Reference< XAccessible >& rxParent )
        :ImplInheritanceHelper( pTabBar )
        ,m_nPageId( nPageId )
        ,m_xParent( rxParent )
    {
        m_bShowing  = IsShowing();
        m_bSelected = IsSelected();

        if ( m_pTabBar )
            m_sPageText = m_pTabBar->GetPageText( m_nPageId );
    }


    bool AccessibleTabBarPage::IsEnabled()
    {
        OExternalLockGuard aGuard( this );

        bool bEnabled = false;
        if ( m_pTabBar )
            bEnabled = m_pTabBar->IsPageEnabled( m_nPageId );

        return bEnabled;
    }


    bool AccessibleTabBarPage::IsShowing() const
    {
        bool bShowing = false;

        if ( m_pTabBar && m_pTabBar->IsVisible() )
            bShowing = true;

        return bShowing;
    }


    bool AccessibleTabBarPage::IsSelected() const
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
        if ( m_sPageText != sPageText )
        {
            Any aOldValue, aNewValue;
            aOldValue <<= m_sPageText;
            aNewValue <<= sPageText;
            m_sPageText = sPageText;
            NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
        }
    }


    void AccessibleTabBarPage::FillAccessibleStateSet( sal_Int64& rStateSet )
    {
        if ( IsEnabled() )
        {
            rStateSet |=  AccessibleStateType::ENABLED;
            rStateSet |=  AccessibleStateType::SENSITIVE;
        }

        rStateSet |=  AccessibleStateType::VISIBLE;

        if ( IsShowing() )
            rStateSet |=  AccessibleStateType::SHOWING;

        rStateSet |=  AccessibleStateType::SELECTABLE;

        if ( IsSelected() )
            rStateSet |=  AccessibleStateType::SELECTED;
    }


    // OAccessibleComponentHelper


    awt::Rectangle AccessibleTabBarPage::implGetBounds()
    {
        awt::Rectangle aBounds;
        if ( m_pTabBar )
        {
            // get bounding rectangle relative to the AccessibleTabBar
            aBounds = vcl::unohelper::ConvertToAWTRect(m_pTabBar->GetPageRect(m_nPageId));

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


    // XComponent


    void AccessibleTabBarPage::disposing()
    {
        AccessibleTabBarBase::disposing();
        m_sPageText.clear();
    }


    // XServiceInfo


    OUString AccessibleTabBarPage::getImplementationName()
    {
        return u"com.sun.star.comp.svtools.AccessibleTabBarPage"_ustr;
    }


    sal_Bool AccessibleTabBarPage::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }


    Sequence< OUString > AccessibleTabBarPage::getSupportedServiceNames()
    {
        return { u"com.sun.star.awt.AccessibleTabBarPage"_ustr };
    }


    // XAccessible


    Reference< XAccessibleContext > AccessibleTabBarPage::getAccessibleContext(  )
    {
        OExternalLockGuard aGuard( this );

        return this;
    }


    // XAccessibleContext


    sal_Int64 AccessibleTabBarPage::getAccessibleChildCount()
    {
        return 0;
    }


    Reference< XAccessible > AccessibleTabBarPage::getAccessibleChild( sal_Int64 )
    {
        OExternalLockGuard aGuard( this );

        throw IndexOutOfBoundsException();
    }


    Reference< XAccessible > AccessibleTabBarPage::getAccessibleParent(  )
    {
        OExternalLockGuard aGuard( this );

        return m_xParent;
    }


    sal_Int64 AccessibleTabBarPage::getAccessibleIndexInParent(  )
    {
        OExternalLockGuard aGuard( this );

        sal_Int64 nIndexInParent = -1;
        if ( m_pTabBar )
            nIndexInParent = m_pTabBar->GetPagePos( m_nPageId );

        return nIndexInParent;
    }

    sal_Int16 AccessibleTabBarPage::getAccessibleRole(  )
    {
        return AccessibleRole::PAGE_TAB;
    }

    OUString AccessibleTabBarPage::getAccessibleDescription( )
    {
        OExternalLockGuard aGuard( this );

        if (m_pTabBar)
            return m_pTabBar->GetHelpText(m_nPageId);

        return OUString();
    }

    OUString AccessibleTabBarPage::getAccessibleName(  )
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }

    Reference< XAccessibleRelationSet > AccessibleTabBarPage::getAccessibleRelationSet(  )
    {
        OExternalLockGuard aGuard( this );

        return new utl::AccessibleRelationSetHelper;
    }

    sal_Int64 AccessibleTabBarPage::getAccessibleStateSet(  )
    {
        OExternalLockGuard aGuard( this );

        sal_Int64 nStateSet = 0;

        if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
        {
            FillAccessibleStateSet( nStateSet );
        }
        else
        {
            nStateSet |= AccessibleStateType::DEFUNC;
        }

        return nStateSet;
    }


    Locale AccessibleTabBarPage::getLocale(  )
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLanguageTag().getLocale();
    }


    // XAccessibleComponent


    Reference< XAccessible > AccessibleTabBarPage::getAccessibleAtPoint( const awt::Point& )
    {
        return Reference< XAccessible >();
    }


    void AccessibleTabBarPage::grabFocus(  )
    {
        // no focus
    }


    sal_Int32 AccessibleTabBarPage::getForeground(  )
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


    sal_Int32 AccessibleTabBarPage::getBackground(  )
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

    OUString AccessibleTabBarPage::getTitledBorderText(  )
    {
        OExternalLockGuard aGuard( this );

        return m_sPageText;
    }


    OUString AccessibleTabBarPage::getToolTipText(  )
    {
        return OUString();
    }


}   // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
