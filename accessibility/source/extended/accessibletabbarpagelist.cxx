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

#include <extended/accessibletabbarpagelist.hxx>
#include <svtools/tabbar.hxx>
#include <extended/accessibletabbarpage.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/convert.hxx>
#include <i18nlangtag/languagetag.hxx>


namespace accessibility
{


    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;


    // class AccessibleTabBarPageList


    AccessibleTabBarPageList::AccessibleTabBarPageList( TabBar* pTabBar, sal_Int32 nIndexInParent )
        :AccessibleTabBarBase( pTabBar )
        ,m_nIndexInParent( nIndexInParent )
    {
        if ( m_pTabBar )
            m_aAccessibleChildren.assign( m_pTabBar->GetPageCount(), Reference< XAccessible >() );
    }


    void AccessibleTabBarPageList::UpdateShowing( bool bShowing )
    {
        for (Reference<XAccessible>& xChild : m_aAccessibleChildren)
        {
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage )
                    pAccessibleTabBarPage->SetShowing( bShowing );
            }
        }
    }


    void AccessibleTabBarPageList::UpdateSelected( sal_Int32 i, bool bSelected )
    {
        NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

        if ( i >= 0 && i < static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
        {
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage )
                    pAccessibleTabBarPage->SetSelected( bSelected );
            }
        }
    }


    void AccessibleTabBarPageList::UpdatePageText( sal_Int32 i )
    {
        if ( i >= 0 && i < static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
        {
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage && m_pTabBar )
                {
                    OUString sPageText = m_pTabBar->GetPageText( m_pTabBar->GetPageId( static_cast<sal_uInt16>(i) ) );
                    pAccessibleTabBarPage->SetPageText( sPageText );
                }
            }
        }
    }


    void AccessibleTabBarPageList::InsertChild( sal_Int32 i )
    {
        if ( i >= 0 && i <= static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
        {
            // insert entry in child list
            m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + i, Reference< XAccessible >() );

            // send accessible child event
            Reference< XAccessible > xChild( getAccessibleChild( i ) );
            if ( xChild.is() )
            {
                Any aOldValue, aNewValue;
                aNewValue <<= xChild;
                NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
            }
        }
    }


    void AccessibleTabBarPageList::RemoveChild( sal_Int32 i )
    {
        if ( i >= 0 && i < static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
        {
            // get the accessible of the removed page
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );

            // remove entry in child list
            m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

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


    void AccessibleTabBarPageList::MoveChild( sal_Int32 i, sal_Int32 j )
    {
        if ( i >= 0 && i < static_cast<sal_Int32>(m_aAccessibleChildren.size()) &&
             j >= 0 && j <= static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
        {
            if ( i < j )
                --j;

            // get the accessible of the moved page
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );

            // remove entry in child list at old position
            m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

            // insert entry in child list at new position
            m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + j, xChild );
        }
    }


    void AccessibleTabBarPageList::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
         switch ( rVclWindowEvent.GetId() )
         {
            case VclEventId::WindowEnabled:
            {
                Any aNewValue;
                aNewValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), aNewValue );
                aNewValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), aNewValue );
            }
            break;
            case VclEventId::WindowDisabled:
            {
                Any aOldValue;
                aOldValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, Any() );
                aOldValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, Any() );
            }
            break;
            case VclEventId::WindowShow:
            {
                Any aOldValue, aNewValue;
                aNewValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                UpdateShowing( true );
            }
            break;
            case VclEventId::WindowHide:
            {
                Any aOldValue, aNewValue;
                aOldValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                UpdateShowing( false );
            }
            break;
            case VclEventId::TabbarPageSelected:
            {
                // do nothing
            }
            break;
            case VclEventId::TabbarPageActivated:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateSelected( nPagePos, true );
                }
            }
            break;
            case VclEventId::TabbarPageDeactivated:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateSelected( nPagePos, false );
                }
            }
            break;
            case VclEventId::TabbarPageInserted:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    InsertChild( nPagePos );
                }
            }
            break;
            case VclEventId::TabbarPageRemoved:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));

                    if ( nPageId == TabBar::PAGE_NOT_FOUND )
                    {
                        for ( sal_Int32 i = m_aAccessibleChildren.size() - 1; i >= 0; --i )
                            RemoveChild( i );
                    }
                    else
                    {
                        for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
                        {
                            Reference< XAccessible > xChild( getAccessibleChild( i ) );
                            if ( xChild.is() )
                            {
                                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                                if ( pAccessibleTabBarPage && pAccessibleTabBarPage->GetPageId() == nPageId )
                                {
                                    RemoveChild( i );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            break;
            case VclEventId::TabbarPageMoved:
            {
                Pair* pPair = static_cast<Pair*>(rVclWindowEvent.GetData());
                if ( pPair )
                    MoveChild( pPair->A(), pPair->B() );
            }
            break;
            case VclEventId::TabbarPageTextChanged:
            {
                sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                UpdatePageText( nPagePos );
            }
            break;
            default:
            {
                AccessibleTabBarBase::ProcessWindowEvent( rVclWindowEvent );
            }
            break;
        }
    }


    void AccessibleTabBarPageList::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        if ( m_pTabBar )
        {
            if ( m_pTabBar->IsEnabled() )
            {
                rStateSet.AddState( AccessibleStateType::ENABLED );
                rStateSet.AddState( AccessibleStateType::SENSITIVE );
            }

            rStateSet.AddState( AccessibleStateType::VISIBLE );

            if ( m_pTabBar->IsVisible() )
                rStateSet.AddState( AccessibleStateType::SHOWING );
        }
    }


    // OCommonAccessibleComponent


    awt::Rectangle AccessibleTabBarPageList::implGetBounds()
    {
        awt::Rectangle aBounds;
        if ( m_pTabBar )
            aBounds = AWTRectangle( m_pTabBar->GetPageArea() );

        return aBounds;
    }


    // XInterface


    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabBarPageList, OAccessibleExtendedComponentHelper, AccessibleTabBarPageList_BASE )


    // XTypeProvider


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabBarPageList, OAccessibleExtendedComponentHelper, AccessibleTabBarPageList_BASE )


    // XComponent


    void AccessibleTabBarPageList::disposing()
    {
        AccessibleTabBarBase::disposing();

        // dispose all children
        for (Reference<XAccessible>& i : m_aAccessibleChildren)
        {
            Reference< XComponent > xComponent( i, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }


    // XServiceInfo


    OUString AccessibleTabBarPageList::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTabBarPageList" );
    }


    sal_Bool AccessibleTabBarPageList::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }


    Sequence< OUString > AccessibleTabBarPageList::getSupportedServiceNames()
    {
        return { "com.sun.star.awt.AccessibleTabBarPageList" };
    }


    // XAccessible


    Reference< XAccessibleContext > AccessibleTabBarPageList::getAccessibleContext(  )
    {
        OExternalLockGuard aGuard( this );

        return this;
    }


    // XAccessibleContext


    sal_Int32 AccessibleTabBarPageList::getAccessibleChildCount()
    {
        OExternalLockGuard aGuard( this );

        return m_aAccessibleChildren.size();
    }


    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleChild( sal_Int32 i )
    {
        OExternalLockGuard aGuard( this );

        if ( i < 0 || i >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
        {
            if ( m_pTabBar )
            {
                sal_uInt16 nPageId = m_pTabBar->GetPageId( static_cast<sal_uInt16>(i) );

                xChild = new AccessibleTabBarPage( m_pTabBar, nPageId, this );

                // insert into child list
                m_aAccessibleChildren[i] = xChild;
            }
        }

        return xChild;
    }


    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleParent(  )
    {
        OExternalLockGuard aGuard( this );

        Reference< XAccessible > xParent;
        if ( m_pTabBar )
            xParent = m_pTabBar->GetAccessible();

        return xParent;
    }


    sal_Int32 AccessibleTabBarPageList::getAccessibleIndexInParent(  )
    {
        OExternalLockGuard aGuard( this );

        return m_nIndexInParent;
    }


    sal_Int16 AccessibleTabBarPageList::getAccessibleRole(  )
    {
        return AccessibleRole::PAGE_TAB_LIST;
    }


    OUString AccessibleTabBarPageList::getAccessibleDescription( )
    {
        return OUString();
    }


    OUString AccessibleTabBarPageList::getAccessibleName(  )
    {
        return OUString();
    }


    Reference< XAccessibleRelationSet > AccessibleTabBarPageList::getAccessibleRelationSet(  )
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
        return xSet;
    }


    Reference< XAccessibleStateSet > AccessibleTabBarPageList::getAccessibleStateSet(  )
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


    Locale AccessibleTabBarPageList::getLocale(  )
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLanguageTag().getLocale();
    }


    // XAccessibleComponent


    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleAtPoint( const awt::Point& rPoint )
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


    void AccessibleTabBarPageList::grabFocus(  )
    {
        // no focus
    }


    sal_Int32 AccessibleTabBarPageList::getForeground(  )
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


    sal_Int32 AccessibleTabBarPageList::getBackground(  )
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


    Reference< awt::XFont > AccessibleTabBarPageList::getFont(  )
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


    OUString AccessibleTabBarPageList::getTitledBorderText(  )
    {
        return OUString();
    }


    OUString AccessibleTabBarPageList::getToolTipText(  )
    {
        return OUString();
    }


    // XAccessibleSelection


    void AccessibleTabBarPageList::selectAccessibleChild( sal_Int32 nChildIndex )
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        if ( m_pTabBar )
        {
            m_pTabBar->SetCurPageId( m_pTabBar->GetPageId( static_cast<sal_uInt16>(nChildIndex) ) );
            m_pTabBar->Update();
            m_pTabBar->ActivatePage();
            m_pTabBar->Select();
        }
    }


    sal_Bool AccessibleTabBarPageList::isAccessibleChildSelected( sal_Int32 nChildIndex )
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        bool bSelected = false;
        if ( m_pTabBar && m_pTabBar->GetCurPageId() == m_pTabBar->GetPageId( static_cast<sal_uInt16>(nChildIndex) ) )
            bSelected = true;

        return bSelected;
    }


    void AccessibleTabBarPageList::clearAccessibleSelection(  )
    {
        // This method makes no sense in a TabBar, and so does nothing.
    }


    void AccessibleTabBarPageList::selectAllAccessibleChildren(  )
    {
        selectAccessibleChild( 0 );
    }


    sal_Int32 AccessibleTabBarPageList::getSelectedAccessibleChildCount(  )
    {
        return 1;
    }


    Reference< XAccessible > AccessibleTabBarPageList::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        OExternalLockGuard aGuard( this );

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;

        for ( sal_Int32 i = 0, j = 0, nCount = getAccessibleChildCount(); i < nCount; i++ )
        {
            if ( isAccessibleChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
            {
                xChild = getAccessibleChild( i );
                break;
            }
        }

        return xChild;
    }


    void AccessibleTabBarPageList::deselectAccessibleChild( sal_Int32 nChildIndex )
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        // This method makes no sense in a TabBar, and so does nothing.
    }


}   // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
