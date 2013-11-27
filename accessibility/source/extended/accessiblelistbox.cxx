/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/extended/accessiblelistbox.hxx>
#include <accessibility/extended/accessiblelistboxentry.hxx>
#include <svtools/svtreebx.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    DBG_NAME(AccessibleListBox)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleListBox::AccessibleListBox( SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rListBox.GetWindowPeer() ),
        m_xParent( _xParent )
    {
        DBG_CTOR( AccessibleListBox, NULL );
    }
    // -----------------------------------------------------------------------------
    AccessibleListBox::~AccessibleListBox()
    {
        DBG_DTOR( AccessibleListBox, NULL );
        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    // -----------------------------------------------------------------------------
    SvTreeListBox* AccessibleListBox::getListBox() const
    {
        return  static_cast< SvTreeListBox* >( const_cast<AccessibleListBox*>(this)->GetWindow() );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
                case  VCLEVENT_CHECKBOX_TOGGLE :
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

                    if ( getListBox()->GetCheckButtonState( pCurOpEntry->GetSvLBoxEntry() ) == SV_BUTTON_CHECKED )
                    {
                        pCurOpEntry->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, uno::Any(), aValue );
                    }
                    else
                    {
                        pCurOpEntry->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aValue,uno::Any() );
                    }
                    break;
                }

                case VCLEVENT_LISTBOX_SELECT :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
//                    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    OSL_ASSERT(0 && "Debug: Treelist shouldn't use VCLEVENT_LISTBOX_SELECT");
                }
                case VCLEVENT_LISTBOX_TREESELECT:
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
                case VCLEVENT_LISTBOX_TREEFOCUS:
                    {
                        SvTreeListBox* pBox = getListBox();
                        sal_Bool bNeedFocus = sal_False;
                        if (pBox)
                        {
                            Window* pParent = ((Window*)pBox)->GetParent();
                            if (pParent && pParent->GetType() == WINDOW_FLOATINGWINDOW)
                            {
                                // MT: ImplGetAppSVData shouldn't be exported from VCL.
                                // In which scenario is this needed?
                                // If needed, we need to find an other solution
                                /*
                                ImplSVData* pSVData = ImplGetAppSVData();
                                if (pSVData && pSVData->maWinData.mpFirstFloat == (FloatingWindow*)pParent)
                                    bNeedFocus = sal_True;
                                */
                            }
                        }
                        if( pBox && (pBox->HasFocus() || bNeedFocus) )
                        {
                            uno::Any aOldValue, aNewValue;
                            SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
                            if ( pEntry )
                            {
                                AccessibleListBoxEntry* pEntryFocus =static_cast< AccessibleListBoxEntry* >(m_xFocusedChild.get());
                                if (pEntryFocus && pEntryFocus->GetSvLBoxEntry() == pEntry)
                                {
                                    aOldValue <<= uno::Any();
                                    aNewValue <<= m_xFocusedChild;
                                    NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                                    return ;
                                }

                                aOldValue <<= m_xFocusedChild;

                                MAP_ENTRY::iterator mi = m_mapEntry.find(pEntry);
                                if(mi != m_mapEntry.end())
                                {
                                    OSL_ASSERT(mi->second.get() != NULL);
                                    m_xFocusedChild = mi->second;
                                }
                                else
                                {
                                    AccessibleListBoxEntry *pEntNew = new AccessibleListBoxEntry( *getListBox(), pEntry, NULL );
                                    m_xFocusedChild = pEntNew;
                                    m_mapEntry.insert(MAP_ENTRY::value_type(pEntry,pEntNew));
                                }

                                aNewValue <<= m_xFocusedChild;
                                NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                            }
                            else
                            {
                                aOldValue <<= uno::Any();
                                aNewValue <<= AccessibleStateType::FOCUSED;
                                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                            }
                        }
                    }
                    break;
                case VCLEVENT_LISTBOX_ITEMREMOVED:
                    {
                        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
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

                // --> OD 2009-04-01 #i92103#
                case VCLEVENT_ITEM_EXPANDED :
                case VCLEVENT_ITEM_COLLAPSED :
                {
                    SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
                    if ( pEntry )
                    {
                        AccessibleListBoxEntry* pAccListBoxEntry =
                            new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                        Reference< XAccessible > xChild = pAccListBoxEntry;
                        const short nAccEvent =
                                ( rVclWindowEvent.GetId() == VCLEVENT_ITEM_EXPANDED )
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
                // <--
                default:
                    VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            }
        }
    }

    AccessibleListBoxEntry* AccessibleListBox::GetCurEventEntry( const VclWindowEvent& rVclWindowEvent )
    {
        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
        if ( !pEntry )
            pEntry = getListBox()->GetCurEntry();

        AccessibleListBoxEntry* pEntryFocus =static_cast< AccessibleListBoxEntry* >(m_xFocusedChild.get());
        if (pEntryFocus && pEntry && pEntry != pEntryFocus->GetSvLBoxEntry())
        {
            AccessibleListBoxEntry *pAccCurOptionEntry =NULL;
            MAP_ENTRY::iterator mi = m_mapEntry.find(pEntry);
            if (mi != m_mapEntry.end())
            {
                pAccCurOptionEntry= static_cast< AccessibleListBoxEntry* >(mi->second.get());
            }
            else
            {
                pAccCurOptionEntry =new AccessibleListBoxEntry( *getListBox(), pEntry, NULL );
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
        return NULL;
    }

    void AccessibleListBox::RemoveChildEntries(SvLBoxEntry* pEntry)
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

        SvTreeListBox* pBox = getListBox();
        SvLBoxEntry* pEntryChild = pBox->FirstChild(pEntry);
        while (pEntryChild)
        {
            RemoveChildEntries(pEntryChild);
            pEntryChild = pBox->NextSibling(pEntryChild);
        }
    }

    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_HIDE:
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

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_mapEntry.clear();
        VCLXAccessibleComponent::disposing();
        m_xParent = NULL;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AccessibleListBox::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for ( ; pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported )
            ;

        return pSupported != pEnd;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > AccessibleListBox::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aSupported(3);
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleContext") );
        aSupported[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleComponent") );
        aSupported[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.AccessibleTreeListBox") );
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString AccessibleListBox::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.svtools.AccessibleTreeListBox") );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleListBox::getAccessibleContext(  ) throw (RuntimeException)
    {
        ensureAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = 0;
        SvTreeListBox* pSvTreeListBox = getListBox();
        if ( pSvTreeListBox )
            nCount = pSvTreeListBox->GetLevelChildCount( NULL );

        return nCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntry(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        // Solution: Set the parameter of the parent to null to let entry determine the parent by itself
        //return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
        return new AccessibleListBoxEntry( *getListBox(), pEntry, NULL );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getRoleType()
    {
        sal_Int32 nCase = 0;
        SvLBoxEntry* pEntry = getListBox()->GetEntry(0);
        if ( pEntry )
        {
            if( pEntry->HasChildsOnDemand() || getListBox()->GetChildCount(pEntry) > 0  )
            {
                nCase = 1;
                return nCase;
            }
        }

        sal_Bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if( !(getListBox()->GetTreeFlags() & TREEFLAG_CHKBTN) )
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
    sal_Int16 SAL_CALL AccessibleListBox::getAccessibleRole(  ) throw (RuntimeException)
    {
        if(getListBox())
        {
            short nType = getListBox()->GetAllEntriesAccessibleRoleType();
            if( nType == TREEBOX_ALLITEM_ACCROLE_TYPE_TREE)
                    return AccessibleRole::TREE;
            else if( nType == TREEBOX_ALLITEM_ACCROLE_TYPE_LIST)
                    return AccessibleRole::LIST;
        }

        //o is: return AccessibleRole::TREE;
        sal_Bool bHasButtons = (getListBox()->GetStyle() & WB_HASBUTTONS)!=0;
        if(!bHasButtons && (getListBox()->GetTreeFlags() & TREEFLAG_CHKBTN))
            return AccessibleRole::LIST;
        else
            if(getRoleType() == 0)
                return AccessibleRole::LIST;
            else
            return AccessibleRole::TREE;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleDescription();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getAccessibleName(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleName();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_False );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_True );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();


//      sal_Int32 i, nSelCount = 0, nCount = 0;
//      nCount = getListBox()->GetLevelChildCount( NULL );
//      for ( i = 0; i < nCount; ++i )
//      {
//          SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
//          if ( getListBox()->IsSelected( pEntry ) )
//              ++nSelCount;
//      }
//      return nSelCount;

        int nTestCount =  getListBox()->GetSelectionCount();
        return nTestCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                // Solution: Set the parameter of the parent to null to let entry determine the parent by itself
                //xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, NULL );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_False );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( getListBox() && isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            if ( getListBox()->GetSelectionMode() == MULTIPLE_SELECTION )
                rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
        }
    }


//........................................................................
}// namespace accessibility
//........................................................................

