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

#include <extended/accessiblelistbox.hxx>
#include <extended/accessiblelistboxentry.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/treelistentry.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>


namespace accessibility
{


    // class AccessibleListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;


    // Ctor() and Dtor()

    AccessibleListBox::AccessibleListBox( SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rListBox.GetWindowPeer() ),
        m_xParent( _xParent )
    {
    }

    AccessibleListBox::~AccessibleListBox()
    {
        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleListBox, VCLXAccessibleComponent, ImplHelper2)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleListBox, VCLXAccessibleComponent, ImplHelper2)

    void AccessibleListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
            case  VclEventId::CheckboxToggle :
                {
                    if ( !getListBox() || !getListBox()->HasFocus() )
                    {
                        return;
                    }
                    AccessibleListBoxEntry* pCurOpEntry = GetCurEventEntry(rVclWindowEvent);
                    if(!pCurOpEntry)
                    {
                        return ;
                    }
                    uno::Any aValue;
                    aValue <<= AccessibleStateType::CHECKED;

                    if ( getListBox()->GetCheckButtonState( pCurOpEntry->GetSvLBoxEntry() ) == SvButtonState::Checked )
                    {
                        pCurOpEntry->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, uno::Any(), aValue );
                    }
                    else
                    {
                        pCurOpEntry->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aValue,uno::Any() );
                    }
                    break;
                }

            case VclEventId::ListboxSelect :
                {
                    OSL_FAIL("Debug: Treelist shouldn't use VclEventId::ListboxSelect");
                    break;
                }

            case VclEventId::ListboxTreeSelect:
                {
                    if ( getListBox() && getListBox()->HasFocus() )
                    {
                        AccessibleListBoxEntry* pEntry =static_cast< AccessibleListBoxEntry* >(m_xFocusedChild.get());
                        if (pEntry)
                        {
                            pEntry->NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                        }
                    }
                }
                break;
            case VclEventId::ListboxTreeFocus:
                {
                    VclPtr<SvTreeListBox> pBox = getListBox();
                    bool bNeedFocus = false;
                    if (pBox)
                    {
                        vcl::Window* pParent = static_cast<vcl::Window*>(pBox)->GetParent();
                        if (pParent && pParent->GetType() == WINDOW_FLOATINGWINDOW)
                        {
                            // MT: ImplGetAppSVData shouldn't be exported from VCL.
                            // In which scenario is this needed?
                            // If needed, we need to find an other solution
                            /*
                              ImplSVData* pSVData = ImplGetAppSVData();
                              if (pSVData && pSVData->maWinData.mpFirstFloat == (FloatingWindow*)pParent)
                              bNeedFocus = true;
                            */
                        }
                    }
                    if( pBox && (pBox->HasFocus() || bNeedFocus) )
                    {
                        uno::Any aNewValue;
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            AccessibleListBoxEntry* pEntryFocus =static_cast< AccessibleListBoxEntry* >(m_xFocusedChild.get());
                            if (pEntryFocus && pEntryFocus->GetSvLBoxEntry() == pEntry)
                            {
                                aNewValue <<= m_xFocusedChild;
                                NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, uno::Any(), aNewValue );
                                return ;
                            }

                            uno::Any aOldValue;
                            aOldValue <<= m_xFocusedChild;

                            MAP_ENTRY::iterator mi = m_mapEntry.find(pEntry);
                            if(mi != m_mapEntry.end())
                            {
                                OSL_ASSERT(mi->second.get() != nullptr);
                                m_xFocusedChild = mi->second;
                            }
                            else
                            {
                                AccessibleListBoxEntry *pEntNew = new AccessibleListBoxEntry( *getListBox(), pEntry, nullptr );
                                m_xFocusedChild = pEntNew;
                                m_mapEntry.insert(MAP_ENTRY::value_type(pEntry,pEntNew));
                            }

                            aNewValue <<= m_xFocusedChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                        else
                        {
                            aNewValue <<= AccessibleStateType::FOCUSED;
                            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, uno::Any(), aNewValue );
                        }
                    }
                }
                break;
            case VclEventId::ListboxItemRemoved:
                {
                    SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                    if ( pEntry )
                    {
                        RemoveChildEntries(pEntry);
                    }
                    else
                    {
                        // NULL means Clear()
                        MAP_ENTRY::iterator mi = m_mapEntry.begin();
                        for ( ; mi != m_mapEntry.end() ; ++mi)
                        {
                            uno::Any aNewValue;
                            uno::Any aOldValue;
                            aOldValue <<= mi->second;
                            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
                        }
                        m_mapEntry.clear();
                    }
                }
                break;

                // #i92103#
            case VclEventId::ItemExpanded :
            case VclEventId::ItemCollapsed :
                {
                    SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                    if ( pEntry )
                    {
                        AccessibleListBoxEntry* pAccListBoxEntry =
                            new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                        Reference< XAccessible > xChild = pAccListBoxEntry;
                        const short nAccEvent =
                                ( rVclWindowEvent.GetId() == VclEventId::ItemExpanded )
                                ? AccessibleEventId::LISTBOX_ENTRY_EXPANDED
                                : AccessibleEventId::LISTBOX_ENTRY_COLLAPSED;
                        uno::Any aListBoxEntry;
                        aListBoxEntry <<= xChild;
                        NotifyAccessibleEvent( nAccEvent, Any(), aListBoxEntry );
                        if ( getListBox() && getListBox()->HasFocus() )
                        {
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), aListBoxEntry );
                        }
                    }
                }
                break;
            default:
                VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            }
        }
    }

    AccessibleListBoxEntry* AccessibleListBox::GetCurEventEntry( const VclWindowEvent& rVclWindowEvent )
    {
        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
        if ( !pEntry )
            pEntry = getListBox()->GetCurEntry();

        AccessibleListBoxEntry* pEntryFocus =static_cast< AccessibleListBoxEntry* >(m_xFocusedChild.get());
        if (pEntryFocus && pEntry && pEntry != pEntryFocus->GetSvLBoxEntry())
        {
            AccessibleListBoxEntry *pAccCurOptionEntry =nullptr;
            MAP_ENTRY::iterator mi = m_mapEntry.find(pEntry);
            if (mi != m_mapEntry.end())
            {
                pAccCurOptionEntry= static_cast< AccessibleListBoxEntry* >(mi->second.get());
            }
            else
            {
                pAccCurOptionEntry =new AccessibleListBoxEntry( *getListBox(), pEntry, nullptr );
                std::pair<MAP_ENTRY::iterator, bool> pairMi =  m_mapEntry.insert(MAP_ENTRY::value_type(pAccCurOptionEntry->GetSvLBoxEntry(),pAccCurOptionEntry));
                mi = pairMi.first;
            }

            uno::Any aNewValue;
            aNewValue <<= mi->second;//xAcc
            NotifyAccessibleEvent( AccessibleEventId::CHILD, uno::Any(), aNewValue );//Add

            return pAccCurOptionEntry;
        }
        else
        {
            return pEntryFocus;
        }
    }

    void AccessibleListBox::RemoveChildEntries(SvTreeListEntry* pEntry)
    {
        MAP_ENTRY::iterator mi = m_mapEntry.find(pEntry);
        if ( mi != m_mapEntry.end() )
        {
            uno::Any aNewValue;
            uno::Any aOldValue;
            aOldValue <<= mi->second;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );

            m_mapEntry.erase(mi);
        }

        VclPtr<SvTreeListBox> pBox = getListBox();
        SvTreeListEntry* pEntryChild = pBox->FirstChild(pEntry);
        while (pEntryChild)
        {
            RemoveChildEntries(pEntryChild);
            pEntryChild = SvTreeListBox::NextSibling(pEntryChild);
        }
    }


    void AccessibleListBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VclEventId::WindowShow:
            case VclEventId::WindowHide:
            {
            }
            break;
            default:
            {
                VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );
            }
            break;
        }
    }


    // XComponent

    void SAL_CALL AccessibleListBox::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_mapEntry.clear();
        VCLXAccessibleComponent::disposing();
        m_xParent = nullptr;
    }

    // XServiceInfo

    OUString SAL_CALL AccessibleListBox::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTreeListBox" );
    }

    Sequence< OUString > SAL_CALL AccessibleListBox::getSupportedServiceNames()
    {
        return {"com.sun.star.accessibility.AccessibleContext",
                "com.sun.star.accessibility.AccessibleComponent",
                "com.sun.star.awt.AccessibleTreeListBox"};
    }

    sal_Bool SAL_CALL AccessibleListBox::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    // XAccessible

    Reference< XAccessibleContext > SAL_CALL AccessibleListBox::getAccessibleContext(  )
    {
        ensureAlive();
        return this;
    }

    // XAccessibleContext

    sal_Int32 SAL_CALL AccessibleListBox::getAccessibleChildCount(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = 0;
        VclPtr<SvTreeListBox> pSvTreeListBox = getListBox();
        if ( pSvTreeListBox )
            nCount = pSvTreeListBox->GetLevelChildCount( nullptr );

        return nCount;
    }

    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleChild( sal_Int32 i )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        SvTreeListEntry* pEntry = getListBox()->GetEntry(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        // Solution: Set the parameter of the parent to null to let entry determine the parent by itself
        //return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
        return new AccessibleListBoxEntry( *getListBox(), pEntry, nullptr );
    }

    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }

    sal_Int32 AccessibleListBox::GetRoleType()
    {
        sal_Int32 nCase = 0;
        SvTreeListEntry* pEntry = getListBox()->GetEntry(0);
        if ( pEntry )
        {
            if( pEntry->HasChildrenOnDemand() || getListBox()->GetChildCount(pEntry) > 0  )
            {
                nCase = 1;
                return nCase;
            }
        }

        bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if( !(getListBox()->GetTreeFlags() & SvTreeFlags::CHKBTN) )
        {
            if( bHasButtons )
                nCase = 1;
        }
        else
        {
            if( bHasButtons )
                nCase = 2;
             else
                nCase = 3;
        }
        return nCase;
    }

    sal_Int16 SAL_CALL AccessibleListBox::getAccessibleRole()
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        if(getListBox())
        {
            SvTreeAccRoleType nType = getListBox()->GetAllEntriesAccessibleRoleType();
            if( nType == SvTreeAccRoleType::TREE)
                    return AccessibleRole::TREE;
            else if( nType == SvTreeAccRoleType::LIST)
                    return AccessibleRole::LIST;
        }

        //o is: return AccessibleRole::TREE;
        bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if(!bHasButtons && (getListBox()->GetTreeFlags() & SvTreeFlags::CHKBTN))
            return AccessibleRole::LIST;
        else
            if (GetRoleType() == 0)
                return AccessibleRole::LIST;
            else
            return AccessibleRole::TREE;
    }

    OUString SAL_CALL AccessibleListBox::getAccessibleDescription(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleDescription();
    }

    OUString SAL_CALL AccessibleListBox::getAccessibleName(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleName();
    }

    // XAccessibleSelection

    void SAL_CALL AccessibleListBox::selectAccessibleChild( sal_Int32 nChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry );
    }

    sal_Bool SAL_CALL AccessibleListBox::isAccessibleChildSelected( sal_Int32 nChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }

    void SAL_CALL AccessibleListBox::clearAccessibleSelection(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = getListBox()->GetLevelChildCount( nullptr );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, false );
        }
    }

    void SAL_CALL AccessibleListBox::selectAllAccessibleChildren(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = getListBox()->GetLevelChildCount( nullptr );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry );
        }
    }

    sal_Int32 SAL_CALL AccessibleListBox::getSelectedAccessibleChildCount(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        return getListBox()->GetSelectionCount();
    }

    Reference< XAccessible > SAL_CALL AccessibleListBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 nSelCount= 0;
        sal_Int32 nCount = getListBox()->GetLevelChildCount( nullptr );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                // Solution: Set the parameter of the parent to null to let entry determine the parent by itself
                //xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, nullptr );
                break;
            }
        }

        return xChild;
    }

    void SAL_CALL AccessibleListBox::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, false );
    }

    void AccessibleListBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( getListBox() && isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            if ( getListBox()->GetSelectionMode() == SelectionMode::Multiple )
                rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
        }
    }

    VclPtr< SvTreeListBox > AccessibleListBox::getListBox() const
    {
        return GetAs< SvTreeListBox >();
    }

}// namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
