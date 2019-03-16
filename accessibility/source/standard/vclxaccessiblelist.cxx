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

#include <standard/vclxaccessiblelist.hxx>
#include <standard/vclxaccessiblelistitem.hxx>
#include <helper/listboxhelper.hxx>

#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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
    /// @throws css::lang::IndexOutOfBoundsException
    void checkSelection_Impl( sal_Int32 _nIndex, const IComboListBoxHelper& _rListBox, bool bSelected )
    {
        sal_Int32 nCount = bSelected ? _rListBox.GetSelectedEntryCount()
                                     : _rListBox.GetEntryCount();
        if ( _nIndex < 0 || _nIndex >= nCount )
            throw css::lang::IndexOutOfBoundsException();
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
      m_nCurSelectedPos         ( LISTBOX_ENTRY_NOTFOUND ),
      m_xParent                 ( _xParent )
{
    // Because combo boxes and list boxes don't have a common interface for
    // methods with identical signature we have to write down twice the
    // same code.
    switch (m_aBoxType)
    {
        case COMBOBOX:
        {
            VclPtr< ComboBox > pBox = GetAs< ComboBox >();
            if ( pBox )
                m_pListBoxHelper.reset( new VCLListBoxHelper<ComboBox> (*pBox) );
            break;
        }

        case LISTBOX:
        {
            VclPtr< ListBox > pBox = GetAs< ListBox >();
            if ( pBox )
                m_pListBoxHelper.reset( new VCLListBoxHelper<ListBox> (*pBox) );
            break;
        }
    }
    UpdateVisibleLineCount();
    if(m_pListBoxHelper)
    {
        m_nCurSelectedPos=m_pListBoxHelper->GetSelectedEntryPos(0);
    }
    sal_uInt16 nCount = static_cast<sal_uInt16>(getAccessibleChildCount());
    m_aAccessibleChildren.reserve(nCount);
}


void VCLXAccessibleList::SetIndexInParent (sal_Int32 nIndex)
{
    m_nIndexInParent = nIndex;
}


void SAL_CALL VCLXAccessibleList::disposing()
{
    VCLXAccessibleComponent::disposing();

    // Dispose all items in the list.
    m_aAccessibleChildren.clear();

    m_pListBoxHelper.reset();
}


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

void VCLXAccessibleList::notifyVisibleStates(bool _bSetNew )
{
    m_bVisible = _bSetNew;
    Any aOldValue, aNewValue;
    (_bSetNew ? aNewValue : aOldValue ) <<= AccessibleStateType::VISIBLE;
    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    (_bSetNew ? aNewValue : aOldValue ) <<= AccessibleStateType::SHOWING;
    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );

    ListItems::iterator aIter = m_aAccessibleChildren.begin();
    UpdateVisibleLineCount();
    // adjust the index inside the VCLXAccessibleListItem
    for ( ; aIter != m_aAccessibleChildren.end(); )
    {
        Reference< XAccessible > xHold = *aIter;
        if (!xHold.is())
        {
            aIter = m_aAccessibleChildren.erase(aIter);
        }
        else
        {
            VCLXAccessibleListItem* pItem = static_cast<VCLXAccessibleListItem*>(xHold.get());
            const sal_Int32 nTopEntry = m_pListBoxHelper ? m_pListBoxHelper->GetTopEntry() : 0;
            const sal_Int32 nPos = static_cast<sal_Int32>(aIter - m_aAccessibleChildren.begin());
            bool bVisible = ( nPos>=nTopEntry && nPos<( nTopEntry + m_nVisibleLineCount ) );
            pItem->SetVisible( m_bVisible && bVisible );
            ++aIter;
        }

    }
}

void VCLXAccessibleList::UpdateSelection_Acc (const OUString& /*sTextOfSelectedItem*/, bool b_IsDropDownList)
{
    if ( m_aBoxType == COMBOBOX )
    {
        /* FIXME: is there something missing here? nIndex is unused. Looks like
         * copy-paste from VCLXAccessibleList::UpdateSelection() */
        // VclPtr< ComboBox > pBox = GetAs< ComboBox >();
        // if ( pBox )
        // {
        //     // Find the index of the selected item inside the VCL control...
        //     sal_Int32 nIndex = pBox->GetEntryPos(sTextOfSelectedItem);
        //     // ...and then find the associated accessibility object.
        //     if ( nIndex == LISTBOX_ENTRY_NOTFOUND )
        //         nIndex = 0;
               UpdateSelection_Impl_Acc(b_IsDropDownList);
        // }
    }
}


void VCLXAccessibleList::UpdateSelection_Impl_Acc(bool bHasDropDownList)
{
    uno::Any aOldValue, aNewValue;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        Reference< XAccessible > xNewAcc;
        if ( m_pListBoxHelper )
        {
            sal_Int32 i=0;
            m_nCurSelectedPos = LISTBOX_ENTRY_NOTFOUND;
            for ( auto& rChild : m_aAccessibleChildren )
            {
                Reference< XAccessible > xHold = rChild;
                if ( xHold.is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( xHold.get() );
                    // Retrieve the item's index from the list entry.
                    bool bNowSelected = m_pListBoxHelper->IsEntryPosSelected (i);
                    if (bNowSelected)
                        m_nCurSelectedPos = i;

                    if ( bNowSelected && !pItem->IsSelected() )
                    {
                        xNewAcc = rChild;
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
                ++i;
            }
            const sal_Int32 nCount = m_pListBoxHelper->GetEntryCount();
            if ( i < nCount ) // here we have to check the if any other listbox entry is selected
            {
                for (; i < nCount && !checkEntrySelected(i,aNewValue,xNewAcc) ;++i )
                    ;
            }
            if ( xNewAcc.is() && GetWindow()->HasFocus() )
            {
                if ( m_nLastSelectedPos != LISTBOX_ENTRY_NOTFOUND )
                    aOldValue <<= getAccessibleChild( m_nLastSelectedPos );
                aNewValue <<= xNewAcc;
            }
        }
    }

    if (m_aBoxType == COMBOBOX)
    {
        //VCLXAccessibleDropDownComboBox
        //when in list is dropped down, xText = NULL
        if (bHasDropDownList && m_pListBoxHelper && m_pListBoxHelper->IsInDropDown())
        {
            if ( aNewValue.hasValue() || aOldValue.hasValue() )
            {
                NotifyAccessibleEvent(
                    AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                    aOldValue,
                    aNewValue );

                NotifyListItem(aNewValue);
            }
        }
        else
        {
            //VCLXAccessibleComboBox
            NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, uno::Any(), uno::Any() );
        }
    }
    else if (m_aBoxType == LISTBOX)
    {
        if ( aNewValue.hasValue() || aOldValue.hasValue() )
        {
            NotifyAccessibleEvent(
                    AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                    aOldValue,
                    aNewValue );

            NotifyListItem(aNewValue);
        }
    }
}

void VCLXAccessibleList::NotifyListItem(css::uno::Any const & val)
{
    Reference< XAccessible > xCurItem;
    val >>= xCurItem;
    if (xCurItem.is())
    {
        VCLXAccessibleListItem* pCurItem = static_cast< VCLXAccessibleListItem* >(xCurItem.get());
        if (pCurItem)
        {
            pCurItem->NotifyAccessibleEvent(AccessibleEventId::SELECTION_CHANGED,Any(),Any());
        }
    }
}

void VCLXAccessibleList::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent,  bool b_IsDropDownList)
{
    switch ( rVclWindowEvent.GetId() )
      {
        case VclEventId::DropdownSelect:
        case VclEventId::ListboxSelect:
            if ( !m_bDisableProcessEvent )
                UpdateSelection_Impl_Acc(b_IsDropDownList);
            break;
        case VclEventId::WindowGetFocus:
            break;
        case VclEventId::ControlGetFocus:
            {
                VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
                if (m_aBoxType == COMBOBOX && b_IsDropDownList)
                {
                    //VCLXAccessibleDropDownComboBox
                }
                else if (m_aBoxType == LISTBOX && b_IsDropDownList)
                {
                }
                else if ( m_aBoxType == LISTBOX && !b_IsDropDownList)
                {
                    if ( m_pListBoxHelper )
                    {
                        uno::Any    aOldValue,
                                    aNewValue;
                        sal_Int32 nPos = m_nCurSelectedPos; //m_pListBoxHelper->GetSelectedEntryPos();

                        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                            nPos = m_pListBoxHelper->GetTopEntry();
                        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
                            aNewValue <<= CreateChild(nPos);
                        NotifyAccessibleEvent(  AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                                                aOldValue,
                                                aNewValue );
                    }
                }
            }
            break;
        default:
            break;
    }

}

void VCLXAccessibleList::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    // Create a reference to this object to prevent an early release of the
    // listbox (VclEventId::ObjectDying).
    Reference< XAccessible > xTemp = this;

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::DropdownOpen:
            notifyVisibleStates(true);
            break;
        case VclEventId::DropdownClose:
            notifyVisibleStates(false);
            break;
        case VclEventId::ListboxScrolled:
            UpdateEntryRange_Impl();
            break;

        // The selection events VclEventId::ComboboxSelect and
        // VclEventId::ComboboxDeselect are not handled here because here we
        // have no access to the edit field.  Its text is necessary to
        // identify the currently selected item.

        case VclEventId::ObjectDying:
        {
            dispose();

            VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            break;
        }

        case VclEventId::ListboxItemRemoved:
        case VclEventId::ComboboxItemRemoved:
        case VclEventId::ListboxItemAdded:
        case VclEventId::ComboboxItemAdded:
            HandleChangedItemList();
            break;
        case VclEventId::ControlGetFocus:
            {
                VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
                // Added by IBM Symphony Acc team to handle the list item focus when List control get focus
                bool b_IsDropDownList = true;
                if (m_pListBoxHelper)
                    b_IsDropDownList = ((m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN);
                if ( m_aBoxType == LISTBOX && !b_IsDropDownList )
                {
                    if ( m_pListBoxHelper )
                    {
                        uno::Any    aOldValue,
                                    aNewValue;
                        sal_Int32 nPos = m_nCurSelectedPos;

                        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
                            nPos = m_pListBoxHelper->GetTopEntry();
                        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
                            aNewValue <<= CreateChild(nPos);
                        NotifyAccessibleEvent(  AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                                                aOldValue,
                                                aNewValue );
                    }
                }
            }
            break;

        default:
            VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
    }
}

 void VCLXAccessibleList::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    VclPtr< ListBox > pBox = GetAs< ListBox >();
    if( m_aBoxType == LISTBOX  )
    {
        if (m_pListBoxHelper && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) != WB_DROPDOWN)
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence { pBox->GetAccessible() };
            rRelationSet.AddRelation( com::sun::star::accessibility::AccessibleRelation( com::sun::star::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
    }
    else
    {
        VCLXAccessibleComponent::FillAccessibleRelationSet(rRelationSet);
    }
}


/** To find out which item is currently selected and to update the SELECTED
    state of the associated accessibility objects accordingly we exploit the
    fact that the
*/
void VCLXAccessibleList::UpdateSelection (const OUString& sTextOfSelectedItem)
{
    if ( m_aBoxType == COMBOBOX )
    {
        VclPtr< ComboBox > pBox = GetAs< ComboBox >();
        if ( pBox )
        {
            // Find the index of the selected item inside the VCL control...
            sal_Int32 nIndex = pBox->GetEntryPos(sTextOfSelectedItem);
            // ...and then find the associated accessibility object.
            if ( nIndex == LISTBOX_ENTRY_NOTFOUND )
                nIndex = 0;
            UpdateSelection_Impl(nIndex);
        }
    }
}


Reference<XAccessible> VCLXAccessibleList::CreateChild (sal_Int32 nPos)
{
    Reference<XAccessible> xChild;

    if ( static_cast<size_t>(nPos) >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(nPos + 1);

        // insert into the container
        xChild = new VCLXAccessibleListItem(nPos, this);
        m_aAccessibleChildren[nPos] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[nPos];
        // check if position is empty and can be used else we have to adjust all entries behind this
        if (!xChild.is())
        {
            xChild = new VCLXAccessibleListItem(nPos, this);
            m_aAccessibleChildren[nPos] = xChild;
        }
    }

    if ( xChild.is() )
    {
        // Just add the SELECTED state.
        bool bNowSelected = false;
        if ( m_pListBoxHelper )
            bNowSelected = m_pListBoxHelper->IsEntryPosSelected(nPos);
        if (bNowSelected)
            m_nCurSelectedPos = nPos;
        VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >(xChild.get());
        pItem->SetSelected( bNowSelected );

        // Set the child's VISIBLE state.
        UpdateVisibleLineCount();
        const sal_Int32 nTopEntry = m_pListBoxHelper ? m_pListBoxHelper->GetTopEntry() : 0;
        bool bVisible = ( nPos>=nTopEntry && nPos<( nTopEntry + m_nVisibleLineCount ) );
        pItem->SetVisible( m_bVisible && bVisible );
    }

    return xChild;
}


void VCLXAccessibleList::HandleChangedItemList()
{
    m_aAccessibleChildren.clear();
    NotifyAccessibleEvent (
        AccessibleEventId::INVALIDATE_ALL_CHILDREN,
        Any(), Any());
}


IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)

// XAccessible

Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleList::getAccessibleContext()
{
    return this;
}


// XAccessibleContext

sal_Int32 SAL_CALL VCLXAccessibleList::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return implGetAccessibleChildCount();
}

sal_Int32 VCLXAccessibleList::implGetAccessibleChildCount()
{
    sal_Int32 nCount = 0;
    if ( m_pListBoxHelper )
        nCount = m_pListBoxHelper->GetEntryCount();

    return nCount;
}

Reference<XAccessible> SAL_CALL VCLXAccessibleList::getAccessibleChild (sal_Int32 i)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    // search for the child
    if ( i >= static_cast<sal_Int32>(m_aAccessibleChildren.size()) )
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

Reference< XAccessible > SAL_CALL VCLXAccessibleList::getAccessibleParent(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}

sal_Int32 SAL_CALL VCLXAccessibleList::getAccessibleIndexInParent()
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}

sal_Int16 SAL_CALL VCLXAccessibleList::getAccessibleRole()
{
    return AccessibleRole::LIST;
}

// XServiceInfo
OUString VCLXAccessibleList::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleList" );
}

Sequence< OUString > VCLXAccessibleList::getSupportedServiceNames()
{
    Sequence< OUString > aNames = VCLXAccessibleComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleList";
    return aNames;
}

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
        sal_Int32 nBegin = std::min( m_nLastTopEntry, nTop );
        sal_Int32 nEnd = std::max( m_nLastTopEntry + m_nVisibleLineCount, nTop + m_nVisibleLineCount );
        for (sal_uInt16 i = static_cast<sal_uInt16>(nBegin); (i <= static_cast<sal_uInt16>(nEnd)); ++i)
        {
            bool bVisible = ( i >= nTop && i < ( nTop + m_nVisibleLineCount ) );
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

bool VCLXAccessibleList::checkEntrySelected(sal_Int32 _nPos,Any& _rNewValue,Reference< XAccessible >& _rxNewAcc)
{
    OSL_ENSURE(m_pListBoxHelper,"Helper is not valid!");
    bool bNowSelected = false;
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


void VCLXAccessibleList::UpdateSelection_Impl(sal_Int32)
{
    uno::Any aOldValue, aNewValue;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        Reference< XAccessible > xNewAcc;

        if ( m_pListBoxHelper )
        {
            sal_Int32 i=0;
            m_nCurSelectedPos = LISTBOX_ENTRY_NOTFOUND;
            for ( auto& rChild : m_aAccessibleChildren )
            {
                Reference< XAccessible > xHold = rChild;
                if ( xHold.is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( xHold.get() );
                    // Retrieve the item's index from the list entry.
                    bool bNowSelected = m_pListBoxHelper->IsEntryPosSelected (i);
                    if (bNowSelected)
                        m_nCurSelectedPos = i;

                    if ( bNowSelected && !pItem->IsSelected() )
                    {
                        xNewAcc = rChild;
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
                ++i;
            }
            const sal_Int32 nCount = m_pListBoxHelper->GetEntryCount();
            if ( i < nCount ) // here we have to check the if any other listbox entry is selected
            {
                for (; i < nCount && !checkEntrySelected(i,aNewValue,xNewAcc) ;++i )
                    ;
            }
            if ( xNewAcc.is() && GetWindow()->HasFocus() )
            {
                if ( m_nLastSelectedPos != LISTBOX_ENTRY_NOTFOUND )
                    aOldValue <<= getAccessibleChild( m_nLastSelectedPos );
                aNewValue <<= xNewAcc;
            }
            if (m_pListBoxHelper->IsInDropDown())
            {
                if ( aNewValue.hasValue() || aOldValue.hasValue() )
                    NotifyAccessibleEvent(
                            AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                            aOldValue,
                            aNewValue );
                //the SELECTION_CHANGED is not necessary
                //NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
            }
        }
    }
}


// XAccessibleSelection

void SAL_CALL VCLXAccessibleList::selectAccessibleChild( sal_Int32 nChildIndex )
{
    bool bNotify = false;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nChildIndex,*m_pListBoxHelper,false);

            m_pListBoxHelper->SelectEntryPos( static_cast<sal_uInt16>(nChildIndex) );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = true;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

sal_Bool SAL_CALL VCLXAccessibleList::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    bool bRet = false;
    if ( m_pListBoxHelper )
    {
        checkSelection_Impl(nChildIndex,*m_pListBoxHelper,false);

        bRet = m_pListBoxHelper->IsEntryPosSelected( static_cast<sal_uInt16>(nChildIndex) );
    }
    return bRet;
}

void SAL_CALL VCLXAccessibleList::clearAccessibleSelection(  )
{
    bool bNotify = false;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            m_pListBoxHelper->SetNoSelection();
            bNotify = true;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

void SAL_CALL VCLXAccessibleList::selectAllAccessibleChildren(  )
{
    bool bNotify = false;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            const sal_Int32 nCount = m_pListBoxHelper->GetEntryCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
                m_pListBoxHelper->SelectEntryPos( i );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = true;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

sal_Int32 SAL_CALL VCLXAccessibleList::getSelectedAccessibleChildCount(  )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nCount = 0;
    if ( m_pListBoxHelper )
           nCount = m_pListBoxHelper->GetSelectedEntryCount();
    return nCount;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleList::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( m_pListBoxHelper )
    {
        checkSelection_Impl(nSelectedChildIndex,*m_pListBoxHelper,true);
        return getAccessibleChild( m_pListBoxHelper->GetSelectedEntryPos( static_cast<sal_uInt16>(nSelectedChildIndex) ) );
    }

    return nullptr;
}

void SAL_CALL VCLXAccessibleList::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    bool bNotify = false;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nSelectedChildIndex,*m_pListBoxHelper,false);

            m_pListBoxHelper->SelectEntryPos( static_cast<sal_uInt16>(nSelectedChildIndex), false );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = true;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

awt::Rectangle VCLXAccessibleList::implGetBounds()
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
            VclPtr< ComboBox > pBox = GetAs< ComboBox >();
            if ( pBox )
            {
                Size aSize = pBox->GetSubEdit()->GetSizePixel();
                aBounds.Y += aSize.Height();
                aBounds.Height -= aSize.Height();
            }
        }
    }
    return aBounds;
}


awt::Point VCLXAccessibleList::getLocationOnScreen(  )
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
            VclPtr< ComboBox > pBox = GetAs< ComboBox >();
            if ( pBox )
            {
                aPos.Y += pBox->GetSubEdit()->GetSizePixel().Height();
            }
        }
    }
    return aPos;
}


bool VCLXAccessibleList::IsInDropDown()
{
    return m_pListBoxHelper->IsInDropDown();
}


void VCLXAccessibleList::HandleDropOpen()
{
    if ( !m_bDisableProcessEvent )
        UpdateSelection_Impl();
    if (m_nCurSelectedPos != LISTBOX_ENTRY_NOTFOUND &&
        m_nLastSelectedPos != LISTBOX_ENTRY_NOTFOUND)
    {
        Reference< XAccessible > xChild = getAccessibleChild(m_nCurSelectedPos);
        if(xChild.is())
        {
            uno::Any aNewValue;
            aNewValue <<= xChild;
            NotifyAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, uno::Any(), aNewValue );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
