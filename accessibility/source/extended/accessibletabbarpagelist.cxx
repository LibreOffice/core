/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <accessibility/extended/accessibletabbarpagelist.hxx>
#include <svtools/tabbar.hxx>
#include <accessibility/extended/accessibletabbarpage.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/convert.hxx>


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
    // class AccessibleTabBarPageList
    // -----------------------------------------------------------------------------

    AccessibleTabBarPageList::AccessibleTabBarPageList( TabBar* pTabBar, sal_Int32 nIndexInParent )
        :AccessibleTabBarBase( pTabBar )
        ,m_nIndexInParent( nIndexInParent )
    {
        if ( m_pTabBar )
            m_aAccessibleChildren.assign( m_pTabBar->GetPageCount(), Reference< XAccessible >() );
    }

    // -----------------------------------------------------------------------------

    AccessibleTabBarPageList::~AccessibleTabBarPageList()
    {
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::UpdateEnabled( sal_Int32 i, sal_Bool bEnabled )
    {
        if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
        {
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage )
                    pAccessibleTabBarPage->SetEnabled( bEnabled );
            }
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::UpdateShowing( sal_Bool bShowing )
    {
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage )
                    pAccessibleTabBarPage->SetShowing( bShowing );
            }
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::UpdateSelected( sal_Int32 i, sal_Bool bSelected )
    {
        NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

        if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::UpdatePageText( sal_Int32 i )
    {
        if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
        {
            Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
            if ( xChild.is() )
            {
                AccessibleTabBarPage* pAccessibleTabBarPage = static_cast< AccessibleTabBarPage* >( xChild.get() );
                if ( pAccessibleTabBarPage )
                {
                    if ( m_pTabBar )
                    {
                        OUString sPageText = m_pTabBar->GetPageText( m_pTabBar->GetPageId( (sal_uInt16)i ) );
                        pAccessibleTabBarPage->SetPageText( sPageText );
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::InsertChild( sal_Int32 i )
    {
        if ( i >= 0 && i <= (sal_Int32)m_aAccessibleChildren.size() )
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::RemoveChild( sal_Int32 i )
    {
        if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::MoveChild( sal_Int32 i, sal_Int32 j )
    {
        if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() &&
             j >= 0 && j <= (sal_Int32)m_aAccessibleChildren.size() )
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
         switch ( rVclWindowEvent.GetId() )
         {
            case VCLEVENT_WINDOW_ENABLED:
            {
               Any aNewValue;
                aNewValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), aNewValue );
                aNewValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), aNewValue );
            }
            break;
            case VCLEVENT_WINDOW_DISABLED:
            {
               Any aOldValue;
                aOldValue <<= AccessibleStateType::ENABLED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, Any() );
                aOldValue <<= AccessibleStateType::SENSITIVE;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, Any() );
            }
            break;
            case VCLEVENT_WINDOW_SHOW:
            {
                Any aOldValue, aNewValue;
                aNewValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                UpdateShowing( sal_True );
            }
            break;
            case VCLEVENT_WINDOW_HIDE:
            {
                Any aOldValue, aNewValue;
                aOldValue <<= AccessibleStateType::SHOWING;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                UpdateShowing( sal_False );
            }
            break;
            case VCLEVENT_TABBAR_PAGEENABLED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateEnabled( nPagePos, sal_True );
                }
            }
            break;
            case VCLEVENT_TABBAR_PAGEDISABLED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateEnabled( nPagePos, sal_False );
                }
            }
            break;
            case VCLEVENT_TABBAR_PAGESELECTED:
            {
                // do nothing
            }
            break;
            case VCLEVENT_TABBAR_PAGEACTIVATED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateSelected( nPagePos, sal_True );
                }
            }
            break;
            case VCLEVENT_TABBAR_PAGEDEACTIVATED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    UpdateSelected( nPagePos, sal_False );
                }
            }
            break;
            case VCLEVENT_TABBAR_PAGEINSERTED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                    sal_uInt16 nPagePos = m_pTabBar->GetPagePos( nPageId );
                    InsertChild( nPagePos );
                }
            }
            break;
            case VCLEVENT_TABBAR_PAGEREMOVED:
            {
                if ( m_pTabBar )
                {
                    sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();

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
            case VCLEVENT_TABBAR_PAGEMOVED:
            {
                Pair* pPair = (Pair*) rVclWindowEvent.GetData();
                if ( pPair )
                    MoveChild( pPair->A(), pPair->B() );
            }
            break;
            case VCLEVENT_TABBAR_PAGETEXTCHANGED:
            {
                sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
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

    // -----------------------------------------------------------------------------

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

    // -----------------------------------------------------------------------------
    // OCommonAccessibleComponent
    // -----------------------------------------------------------------------------

    awt::Rectangle AccessibleTabBarPageList::implGetBounds() throw (RuntimeException)
    {
        awt::Rectangle aBounds;
        if ( m_pTabBar )
            aBounds = AWTRectangle( m_pTabBar->GetPageArea() );

        return aBounds;
    }

    // -----------------------------------------------------------------------------
    // XInterface
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabBarPageList, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPageList_BASE )

    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabBarPageList, AccessibleExtendedComponentHelper_BASE, AccessibleTabBarPageList_BASE )

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::disposing()
    {
        AccessibleTabBarBase::disposing();

        // dispose all children
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XComponent > xComponent( m_aAccessibleChildren[i], UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    OUString AccessibleTabBarPageList::getImplementationName() throw (RuntimeException)
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTabBarPageList" );
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPageList::supportsService( const OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< OUString > aNames( getSupportedServiceNames() );
        const OUString* pNames = aNames.getConstArray();
        const OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< OUString > AccessibleTabBarPageList::getSupportedServiceNames() throw (RuntimeException)
    {
        Sequence< OUString > aNames(1);
        aNames[0] = "com.sun.star.awt.AccessibleTabBarPageList";
        return aNames;
    }

    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------

    Reference< XAccessibleContext > AccessibleTabBarPageList::getAccessibleContext(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return this;
    }

    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPageList::getAccessibleChildCount() throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_aAccessibleChildren.size();
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( i < 0 || i >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
        {
            if ( m_pTabBar )
            {
                sal_uInt16 nPageId = m_pTabBar->GetPageId( (sal_uInt16)i );

                xChild = new AccessibleTabBarPage( m_pTabBar, nPageId, this );

                // insert into child list
                m_aAccessibleChildren[i] = xChild;
            }
        }

        return xChild;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< XAccessible > xParent;
        if ( m_pTabBar )
            xParent = m_pTabBar->GetAccessible();

        return xParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPageList::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return m_nIndexInParent;
    }

    // -----------------------------------------------------------------------------

    sal_Int16 AccessibleTabBarPageList::getAccessibleRole(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return AccessibleRole::PAGE_TAB_LIST;
    }

    // -----------------------------------------------------------------------------

    OUString AccessibleTabBarPageList::getAccessibleDescription( ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OUString();
    }

    // -----------------------------------------------------------------------------

    OUString AccessibleTabBarPageList::getAccessibleName(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OUString();
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleRelationSet > AccessibleTabBarPageList::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
        Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
        return xSet;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessibleStateSet > AccessibleTabBarPageList::getAccessibleStateSet(  ) throw (RuntimeException)
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

    Locale AccessibleTabBarPageList::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return Application::GetSettings().GetLocale();
    }

    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPageList::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        Reference< XAccessible > xChild;
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XAccessible > xAcc = getAccessibleChild( i );
            if ( xAcc.is() )
            {
                Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
                if ( xComp.is() )
                {
                    Rectangle aRect = VCLRectangle( xComp->getBounds() );
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::grabFocus(  ) throw (RuntimeException)
    {
        // no focus
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPageList::getForeground(  ) throw (RuntimeException)
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

    sal_Int32 AccessibleTabBarPageList::getBackground(  ) throw (RuntimeException)
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

    Reference< awt::XFont > AccessibleTabBarPageList::getFont(  ) throw (RuntimeException)
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

    OUString AccessibleTabBarPageList::getTitledBorderText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OUString();
    }

    // -----------------------------------------------------------------------------

    OUString AccessibleTabBarPageList::getToolTipText(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return OUString();
    }

    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        if ( m_pTabBar )
        {
            m_pTabBar->SetCurPageId( m_pTabBar->GetPageId( (sal_uInt16)nChildIndex ) );
            m_pTabBar->Update();
            m_pTabBar->ActivatePage();
            m_pTabBar->Select();
        }
    }

    // -----------------------------------------------------------------------------

    sal_Bool AccessibleTabBarPageList::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        sal_Bool bSelected = sal_False;
        if ( m_pTabBar && m_pTabBar->GetCurPageId() == m_pTabBar->GetPageId( (sal_uInt16)nChildIndex ) )
            bSelected = sal_True;

        return bSelected;
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        // This method makes no sense in a TabBar, and so does nothing.
    }

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        selectAccessibleChild( 0 );
    }

    // -----------------------------------------------------------------------------

    sal_Int32 AccessibleTabBarPageList::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        return 1;
    }

    // -----------------------------------------------------------------------------

    Reference< XAccessible > AccessibleTabBarPageList::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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

    // -----------------------------------------------------------------------------

    void AccessibleTabBarPageList::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );

        if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        // This method makes no sense in a TabBar, and so does nothing.
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace accessibility
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
