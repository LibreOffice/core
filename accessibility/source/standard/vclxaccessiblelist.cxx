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

#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <accessibility/standard/vclxaccessiblelistitem.hxx>
#include <accessibility/helper/listboxhelper.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <toolkit/helper/convert.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::accessibility;

namespace
{
    void checkSelection_Impl( sal_Int32 _nIndex, const IComboListBoxHelper& _rListBox, sal_Bool bSelected )
        throw (::com::sun::star::lang::IndexOutOfBoundsException)
    {
        sal_Int32 nCount = bSelected ? (sal_Int32)_rListBox.GetSelectEntryCount()
                                     : (sal_Int32)_rListBox.GetEntryCount();
        if ( _nIndex < 0 || _nIndex >= nCount )
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }
}

VCLXAccessibleList::VCLXAccessibleList (VCLXWindow* pVCLWindow, BoxType aBoxType,
                                        const Reference< XAccessible >& _xParent)
    : VCLXAccessibleComponent   (pVCLWindow),
      m_aBoxType                (aBoxType),
      m_nVisibleLineCount       (0),
      m_nIndexInParent          (DEFAULT_INDEX_IN_PARENT),
      m_nLastTopEntry           ( 0 ),
      m_nLastSelectedPos        ( LISTBOX_ENTRY_NOTFOUND ),
      m_bDisableProcessEvent    ( false ),
      m_bVisible                ( true ),
      m_xParent                 ( _xParent )
{
    // Because combo boxes and list boxes don't have a common interface for
    // methods with identical signature we have to write down twice the
    // same code.
    switch (m_aBoxType)
    {
        case COMBOBOX:
        {
            ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
            if ( pBox != NULL )
                m_pListBoxHelper = new VCLListBoxHelper<ComboBox> (*pBox);
            break;
        }

        case LISTBOX:
        {
            ListBox* pBox = static_cast<ListBox*>(GetWindow());
            if ( pBox != NULL )
                m_pListBoxHelper = new VCLListBoxHelper<ListBox> (*pBox);
            break;
        }
    }
    UpdateVisibleLineCount();

    sal_uInt16 nCount = static_cast<sal_uInt16>(getAccessibleChildCount());
    m_aAccessibleChildren.reserve(nCount);
}
// -----------------------------------------------------------------------------

VCLXAccessibleList::~VCLXAccessibleList (void)
{
    delete m_pListBoxHelper;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::SetIndexInParent (sal_Int32 nIndex)
{
    m_nIndexInParent = nIndex;
}
// -----------------------------------------------------------------------------

void SAL_CALL VCLXAccessibleList::disposing (void)
{
    VCLXAccessibleComponent::disposing();

    // Dispose all items in the list.
    clearItems();

    delete m_pListBoxHelper;
    m_pListBoxHelper = NULL;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::clearItems()
{
    // Clear the list itself and delete all the rest.
    ListItems().swap(m_aAccessibleChildren); // clear and minimize
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet)
{
    SolarMutexGuard aSolarGuard;

    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
    // check if our list should be visible
    if (    m_pListBoxHelper
        && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN
        && !m_pListBoxHelper->IsInDropDown() )
    {
        rStateSet.RemoveState (AccessibleStateType::VISIBLE);
        rStateSet.RemoveState (AccessibleStateType::SHOWING);
        m_bVisible = false;
    }

    // Both the combo box and list box are handled identical in the
    // following but for some reason they don't have a common interface for
    // the methods used.
    if ( m_pListBoxHelper )
    {
        if ( m_pListBoxHelper->IsMultiSelectionEnabled() )
            rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE);
        rStateSet.AddState (AccessibleStateType::FOCUSABLE);
        // All children are transient.
        rStateSet.AddState (AccessibleStateType::MANAGES_DESCENDANTS);
    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleList::notifyVisibleStates(sal_Bool _bSetNew )
{
    m_bVisible = _bSetNew ? true : false;
    Any aOldValue, aNewValue;
    (_bSetNew ? aNewValue : aOldValue ) <<= AccessibleStateType::VISIBLE;
    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    (_bSetNew ? aNewValue : aOldValue ) <<= AccessibleStateType::SHOWING;
    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );

    ListItems::iterator aIter = m_aAccessibleChildren.begin();
    ListItems::iterator aEnd = m_aAccessibleChildren.end();
    UpdateVisibleLineCount();
    // adjust the index inside the VCLXAccessibleListItem
    for (;aIter != aEnd ; ++aIter)
    {
        Reference< XAccessible > xHold = *aIter;
        VCLXAccessibleListItem* pItem = static_cast<VCLXAccessibleListItem*>(xHold.get());
        if ( pItem )
        {
            sal_uInt16 nTopEntry = 0;
            if ( m_pListBoxHelper )
                nTopEntry = m_pListBoxHelper->GetTopEntry();
            sal_uInt16 nPos = (sal_uInt16)(aIter - m_aAccessibleChildren.begin());
            sal_Bool bVisible = ( nPos>=nTopEntry && nPos<( nTopEntry + m_nVisibleLineCount ) );
            pItem->SetVisible( m_bVisible && bVisible );
        }

    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleList::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    // Create a reference to this object to prevent an early release of the
    // listbox (VCLEVENT_OBJECT_DYING).
    Reference< XAccessible > xTemp = this;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_DROPDOWN_OPEN:
            notifyVisibleStates(sal_True);
            break;
        case VCLEVENT_DROPDOWN_CLOSE:
            notifyVisibleStates(sal_False);
            break;
        case VCLEVENT_LISTBOX_SCROLLED:
        case VCLEVENT_COMBOBOX_SCROLLED:
            UpdateEntryRange_Impl();
            break;

        case VCLEVENT_LISTBOX_SELECT:
            if ( !m_bDisableProcessEvent )
                UpdateSelection_Impl();
            break;
        // The selection events VCLEVENT_COMBOBOX_SELECT and
        // VCLEVENT_COMBOBOX_DESELECT are not handled here because here we
        // have no access to the edit field.  Its text is necessary to
        // identify the currently selected item.

        case VCLEVENT_OBJECT_DYING:
        {
            dispose();

            VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            break;
        }

        case VCLEVENT_LISTBOX_ITEMREMOVED:
        case VCLEVENT_COMBOBOX_ITEMREMOVED:
            HandleChangedItemList (false, reinterpret_cast<sal_IntPtr>(
                rVclWindowEvent.GetData()));
            break;

        case VCLEVENT_LISTBOX_ITEMADDED:
        case VCLEVENT_COMBOBOX_ITEMADDED:
            HandleChangedItemList (true, reinterpret_cast<sal_IntPtr>(
                rVclWindowEvent.GetData()));
            break;
        case VCLEVENT_CONTROL_GETFOCUS:
            VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            if ( m_pListBoxHelper )
            {
                uno::Any    aOldValue,
                            aNewValue;
                sal_uInt16 nPos = m_pListBoxHelper->GetSelectEntryPos();
                if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                    nPos = m_pListBoxHelper->GetTopEntry();
                if ( nPos != LISTBOX_ENTRY_NOTFOUND )
                    aNewValue <<= CreateChild(nPos);

                NotifyAccessibleEvent(  AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                                        aOldValue,
                                        aNewValue );
            }
            break;

        default:
            VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
    }
}
// -----------------------------------------------------------------------------

/** To find out which item is currently selected and to update the SELECTED
    state of the associated accessibility objects accordingly we exploit the
    fact that the
*/
void VCLXAccessibleList::UpdateSelection (OUString sTextOfSelectedItem)
{
    if ( m_aBoxType == COMBOBOX )
    {
        ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
        if ( pBox != NULL )
        {
            // Find the index of the selected item inside the VCL control...
            sal_uInt16 nIndex = pBox->GetEntryPos(sTextOfSelectedItem);
            // ...and then find the associated accessibility object.
            if ( nIndex == LISTBOX_ENTRY_NOTFOUND )
                nIndex = 0;
            UpdateSelection_Impl(nIndex);
        }
    }
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::adjustEntriesIndexInParent(ListItems::iterator& _aBegin,::std::mem_fun_t<bool,VCLXAccessibleListItem>& _rMemFun)
{
    ListItems::iterator aIter = _aBegin;
    ListItems::iterator aEnd = m_aAccessibleChildren.end();
    // adjust the index inside the VCLXAccessibleListItem
    for (;aIter != aEnd ; ++aIter)
    {
        Reference< XAccessible > xHold = *aIter;
        VCLXAccessibleListItem* pItem = static_cast<VCLXAccessibleListItem*>(xHold.get());
        if ( pItem )
            _rMemFun(pItem);
    }
}
// -----------------------------------------------------------------------------

Reference<XAccessible> VCLXAccessibleList::CreateChild (sal_Int32 i)
{
    Reference<XAccessible> xChild;

    sal_uInt16 nPos = static_cast<sal_uInt16>(i);
    if ( nPos >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(nPos + 1);

        // insert into the container
        xChild = new VCLXAccessibleListItem(m_pListBoxHelper, i, this);
        m_aAccessibleChildren[nPos] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[nPos];
        // check if position is empty and can be used else we have to adjust all entries behind this
        if ( xChild.is() )
        {
            ListItems::iterator aIter = m_aAccessibleChildren.begin() + nPos;
            ::std::mem_fun_t<bool, VCLXAccessibleListItem> aTemp(&VCLXAccessibleListItem::IncrementIndexInParent);
            adjustEntriesIndexInParent( aIter, aTemp);
        }
        else
        {
            xChild = new VCLXAccessibleListItem(m_pListBoxHelper, i, this);
            m_aAccessibleChildren[nPos] = xChild;
        }
    }

    if ( xChild.is() )
    {
        // Just add the SELECTED state.
        bool bNowSelected = false;
        if ( m_pListBoxHelper )
            bNowSelected = m_pListBoxHelper->IsEntryPosSelected ((sal_uInt16)i);
        VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >(xChild.get());
        pItem->SetSelected( bNowSelected );

        // Set the child's VISIBLE state.
        UpdateVisibleLineCount();
        sal_uInt16 nTopEntry = 0;
        if ( m_pListBoxHelper )
            nTopEntry = m_pListBoxHelper->GetTopEntry();
        bool bVisible = ( nPos>=nTopEntry && nPos<( nTopEntry + m_nVisibleLineCount ) );
        pItem->SetVisible( m_bVisible && bVisible );
    }

    return xChild;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::HandleChangedItemList (bool bItemInserted, sal_Int32 nIndex)
{
    if ( !bItemInserted )
    {
        if ( nIndex == -1 ) // special handling here
        {
            clearItems();
        }
        else
        {
            if ( nIndex >= 0 && static_cast<sal_uInt16>(nIndex) < m_aAccessibleChildren.size() )
            {
                ListItems::iterator aIter = m_aAccessibleChildren.erase(m_aAccessibleChildren.begin()+nIndex);
            ::std::mem_fun_t<bool, VCLXAccessibleListItem> aTemp(&VCLXAccessibleListItem::DecrementIndexInParent);
                adjustEntriesIndexInParent( aIter, aTemp );
            }
        }
    }
    else
        getAccessibleChild(nIndex);

    NotifyAccessibleEvent (
        AccessibleEventId::INVALIDATE_ALL_CHILDREN,
        Any(), Any());
}
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)

//=====  XAccessible  =========================================================

Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleList::getAccessibleContext (void)
    throw (RuntimeException)
{
    return this;
}
// -----------------------------------------------------------------------------

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL VCLXAccessibleList::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nCount = 0;
    if ( m_pListBoxHelper )
        nCount = m_pListBoxHelper->GetEntryCount();

    return nCount;
}
// -----------------------------------------------------------------------------

Reference<XAccessible> SAL_CALL VCLXAccessibleList::getAccessibleChild (sal_Int32 i)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    // search for the child
    if ( static_cast<sal_uInt16>(i) >= m_aAccessibleChildren.size() )
        xChild = CreateChild (i);
    else
    {
        xChild = m_aAccessibleChildren[i];
        if ( !xChild.is() )
            xChild = CreateChild (i);
    }
    OSL_ENSURE( xChild.is(), "VCLXAccessibleList::getAccessibleChild: returning empty child!" );
    return xChild;
}
// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL VCLXAccessibleList::getAccessibleParent(  )
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}
// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL VCLXAccessibleList::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}
// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL VCLXAccessibleList::getAccessibleRole (void)
    throw (RuntimeException)
{
    return AccessibleRole::LIST;
}
// -----------------------------------------------------------------------------

//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL VCLXAccessibleList::contains( const awt::Point& rPoint ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bInside = sal_False;

    Window* pListBox = GetWindow();
    if ( pListBox )
    {
        Rectangle aRect( Point(0,0), pListBox->GetSizePixel() );
        bInside = aRect.IsInside( VCLPoint( rPoint ) );
    }

    return bInside;
}
// -----------------------------------------------------------------------------

Reference< XAccessible > SAL_CALL VCLXAccessibleList::getAccessibleAt( const awt::Point& rPoint )
    throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Reference< XAccessible > xChild;
    if ( m_pListBoxHelper )
    {
        UpdateVisibleLineCount();
        if ( contains( rPoint ) && m_nVisibleLineCount > 0 )
        {
            Point aPos = VCLPoint( rPoint );
            sal_uInt16 nEndPos = m_pListBoxHelper->GetTopEntry() + (sal_uInt16)m_nVisibleLineCount;
            for ( sal_uInt16 i = m_pListBoxHelper->GetTopEntry(); i < nEndPos; ++i )
            {
                if ( m_pListBoxHelper->GetBoundingRectangle(i).IsInside( aPos ) )
                {
                    xChild = getAccessibleChild(i);
                    break;
                }
            }
        }
    }

    return xChild;
}
// -----------------------------------------------------------------------------

//===== XServiceInfo ==========================================================

OUString VCLXAccessibleList::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleList" );
}
// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleList::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< OUString > aNames = VCLXAccessibleComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleList";
    return aNames;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::UpdateVisibleLineCount()
{
    if ( m_pListBoxHelper )
    {
        if ( (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN )
            m_nVisibleLineCount = m_pListBoxHelper->GetDisplayLineCount();
        else
        {
            sal_uInt16 nCols = 0,
                nLines = 0;
            m_pListBoxHelper->GetMaxVisColumnsAndLines (nCols, nLines);
            m_nVisibleLineCount = nLines;
        }
    }
}

// -----------------------------------------------------------------------------
void VCLXAccessibleList::UpdateEntryRange_Impl()
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nTop = m_nLastTopEntry;

    if ( m_pListBoxHelper )
        nTop = m_pListBoxHelper->GetTopEntry();
    if ( nTop != m_nLastTopEntry )
    {
        UpdateVisibleLineCount();
        sal_Int32 nBegin = Min( m_nLastTopEntry, nTop );
        sal_Int32 nEnd = Max( m_nLastTopEntry + m_nVisibleLineCount, nTop + m_nVisibleLineCount );
        for (sal_uInt16 i = static_cast<sal_uInt16>(nBegin); (i <= static_cast<sal_uInt16>(nEnd)); ++i)
        {
            sal_Bool bVisible = ( i >= nTop && i < ( nTop + m_nVisibleLineCount ) );
            Reference< XAccessible > xHold;
            if ( i < m_aAccessibleChildren.size() )
                xHold = m_aAccessibleChildren[i];
            else if ( bVisible )
                xHold = CreateChild(i);

            if ( xHold.is() )
                static_cast< VCLXAccessibleListItem* >( xHold.get() )->SetVisible( m_bVisible && bVisible );
        }
    }

    m_nLastTopEntry = nTop;
}
// -----------------------------------------------------------------------------
sal_Bool VCLXAccessibleList::checkEntrySelected(sal_uInt16 _nPos,Any& _rNewValue,Reference< XAccessible >& _rxNewAcc)
{
    OSL_ENSURE(m_pListBoxHelper,"Helper is not valid!");
    sal_Bool bNowSelected = sal_False;
    if ( m_pListBoxHelper )
    {
        bNowSelected = m_pListBoxHelper->IsEntryPosSelected (_nPos);
        if ( bNowSelected )
        {
            _rxNewAcc = CreateChild(_nPos);
            _rNewValue <<= _rxNewAcc;
        }
    }
    return bNowSelected;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleList::UpdateSelection_Impl(sal_uInt16)
{
    uno::Any aOldValue, aNewValue;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        Reference< XAccessible > xNewAcc;

        if ( m_pListBoxHelper )
        {
            sal_uInt16 i=0;
            for ( ListItems::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != m_aAccessibleChildren.end(); ++aIter,++i)
            {
                Reference< XAccessible > xHold = *aIter;
                if ( xHold.is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( xHold.get() );
                    // Retrieve the item's index from the list entry.
                    sal_Bool bNowSelected = m_pListBoxHelper->IsEntryPosSelected (i);

                    if ( bNowSelected && !pItem->IsSelected() )
                    {
                        xNewAcc = *aIter;
                        aNewValue <<= xNewAcc;
                    }
                    else if ( pItem->IsSelected() )
                        m_nLastSelectedPos = i;

                    pItem->SetSelected( bNowSelected );
                }
                else
                { // it could happen that a child was not created before
                    checkEntrySelected(i,aNewValue,xNewAcc);
                }
            }
            sal_uInt16 nCount = m_pListBoxHelper->GetEntryCount();
            if ( i < nCount ) // here we have to check the if any other listbox entry is selected
            {
                for (; i < nCount && !checkEntrySelected(i,aNewValue,xNewAcc) ;++i )
                    ;
            }
            if ( xNewAcc.is() && GetWindow()->HasFocus() )
            {
                if ( m_nLastSelectedPos != LISTBOX_ENTRY_NOTFOUND )
                    aOldValue <<= getAccessibleChild( (sal_Int32)m_nLastSelectedPos );
                aNewValue <<= xNewAcc;
            }
        }
    }

    if ( aNewValue.hasValue() || aOldValue.hasValue() )
        NotifyAccessibleEvent(
            AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
            aOldValue,
            aNewValue );

    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
}

// -----------------------------------------------------------------------------
// XAccessibleSelection
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleList::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nChildIndex,*m_pListBoxHelper,sal_False);

            m_pListBoxHelper->SelectEntryPos( (sal_uInt16)nChildIndex, sal_True );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL VCLXAccessibleList::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bRet = sal_False;
    if ( m_pListBoxHelper )
    {
        checkSelection_Impl(nChildIndex,*m_pListBoxHelper,sal_False);

        bRet = m_pListBoxHelper->IsEntryPosSelected( (sal_uInt16)nChildIndex );
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleList::clearAccessibleSelection(  ) throw (RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            m_pListBoxHelper->SetNoSelection();
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleList::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            sal_uInt16 nCount = m_pListBoxHelper->GetEntryCount();
            for ( sal_uInt16 i = 0; i < nCount; ++i )
                m_pListBoxHelper->SelectEntryPos( i, sal_True );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleList::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nCount = 0;
    if ( m_pListBoxHelper )
           nCount = m_pListBoxHelper->GetSelectEntryCount();
    return nCount;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL VCLXAccessibleList::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( m_pListBoxHelper )
    {
        checkSelection_Impl(nSelectedChildIndex,*m_pListBoxHelper,sal_True);
        return getAccessibleChild( (sal_Int32)m_pListBoxHelper->GetSelectEntryPos( (sal_uInt16)nSelectedChildIndex ) );
    }

    return NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleList::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nSelectedChildIndex,*m_pListBoxHelper,sal_False);

            m_pListBoxHelper->SelectEntryPos( (sal_uInt16)nSelectedChildIndex, sal_False );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
awt::Rectangle VCLXAccessibleList::implGetBounds() throw (uno::RuntimeException)
{
    awt::Rectangle aBounds ( 0, 0, 0, 0 );
    if ( m_pListBoxHelper
        && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN )
    {
        if ( m_pListBoxHelper->IsInDropDown() )
            aBounds = AWTRectangle(m_pListBoxHelper->GetDropDownPosSizePixel());
    }
    else
    {
        // a list has the same bounds as his parent but starts at (0,0)
        aBounds = VCLXAccessibleComponent::implGetBounds();
        aBounds.X = 0;
        aBounds.Y = 0;
        if ( m_aBoxType == COMBOBOX )
        {
            ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
            if ( pBox )
            {
                Size aSize = pBox->GetSubEdit()->GetSizePixel();
                aBounds.X += aSize.Height();
                aBounds.Y += aSize.Width();
                aBounds.Height -= aSize.Height();
                aBounds.Width  -= aSize.Width();
            }
        }
    }
    return aBounds;
}
// -----------------------------------------------------------------------------

awt::Point VCLXAccessibleList::getLocationOnScreen(  ) throw (uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    awt::Point aPos;
    if ( m_pListBoxHelper
        && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN )
    {
        if ( m_pListBoxHelper->IsInDropDown() )
            aPos = AWTPoint(m_pListBoxHelper->GetDropDownPosSizePixel().TopLeft());
    }
    else
    {
        aPos = VCLXAccessibleComponent::getLocationOnScreen();
        if ( m_aBoxType == COMBOBOX )
        {
            ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
            if ( pBox )
            {
                aPos.X += pBox->GetSubEdit()->GetSizePixel().Height();
                aPos.Y += pBox->GetSubEdit()->GetSizePixel().Width();
            }
        }
    }
    return aPos;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
