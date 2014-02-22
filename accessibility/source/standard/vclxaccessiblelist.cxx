/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <accessibility/standard/vclxaccessiblelistitem.hxx>
#include <accessibility/helper/listboxhelper.hxx>

#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
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
      m_pListBoxHelper          (0),
      m_nVisibleLineCount       (0),
      m_nIndexInParent          (DEFAULT_INDEX_IN_PARENT),
      m_nLastTopEntry           ( 0 ),
      m_nLastSelectedPos        ( LISTBOX_ENTRY_NOTFOUND ),
      m_bDisableProcessEvent    ( false ),
      m_bVisible                ( true ),
    m_nCurSelectedPos       ( LISTBOX_ENTRY_NOTFOUND ),
      m_xParent                 ( _xParent )
{
    
    
    
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
    if(m_pListBoxHelper)
    {
        m_nCurSelectedPos=m_pListBoxHelper->GetSelectEntryPos();
    }
    sal_uInt16 nCount = static_cast<sal_uInt16>(getAccessibleChildCount());
    m_aAccessibleChildren.reserve(nCount);
}


VCLXAccessibleList::~VCLXAccessibleList (void)
{
    delete m_pListBoxHelper;
}


void VCLXAccessibleList::SetIndexInParent (sal_Int32 nIndex)
{
    m_nIndexInParent = nIndex;
}


void SAL_CALL VCLXAccessibleList::disposing (void)
{
    VCLXAccessibleComponent::disposing();

    
    clearItems();

    delete m_pListBoxHelper;
    m_pListBoxHelper = NULL;
}


void VCLXAccessibleList::clearItems()
{
    
    ListItems().swap(m_aAccessibleChildren); 
}


void VCLXAccessibleList::FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet)
{
    SolarMutexGuard aSolarGuard;

    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
    
    if (    m_pListBoxHelper
        && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN
        && !m_pListBoxHelper->IsInDropDown() )
    {
        rStateSet.RemoveState (AccessibleStateType::VISIBLE);
        rStateSet.RemoveState (AccessibleStateType::SHOWING);
        m_bVisible = false;
    }

    
    
    
    if ( m_pListBoxHelper )
    {
        if ( m_pListBoxHelper->IsMultiSelectionEnabled() )
            rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE);
        rStateSet.AddState (AccessibleStateType::FOCUSABLE);
        
        rStateSet.AddState (AccessibleStateType::MANAGES_DESCENDANTS);
    }
}

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

void VCLXAccessibleList::UpdateSelection_Acc (::rtl::OUString sTextOfSelectedItem, bool b_IsDropDownList)
{
    if ( m_aBoxType == COMBOBOX )
    {
        ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
        if ( pBox != NULL )
        {
        
        sal_uInt16 nIndex = pBox->GetEntryPos(sTextOfSelectedItem);
        
        if ( nIndex == LISTBOX_ENTRY_NOTFOUND )
            nIndex = 0;
        UpdateSelection_Impl_Acc(b_IsDropDownList);
        }
    }
}


void VCLXAccessibleList::UpdateSelection_Impl_Acc(bool b_IsDropDownList)
{
    uno::Any aOldValue, aNewValue;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
            Reference< XAccessible > xNewAcc;
        if ( m_pListBoxHelper )
        {
            sal_uInt16 i=0;
            m_nCurSelectedPos = LISTBOX_ENTRY_NOTFOUND;
            for ( ListItems::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != m_aAccessibleChildren.end(); ++aIter,++i)
            {
                Reference< XAccessible > xHold = *aIter;
                if ( xHold.is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( xHold.get() );
                    
                    sal_Bool bNowSelected = m_pListBoxHelper->IsEntryPosSelected (i);
                    if (bNowSelected)
                        m_nCurSelectedPos = i;

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
                { 
                    checkEntrySelected(i,aNewValue,xNewAcc);
                }
            }
            sal_uInt16 nCount = m_pListBoxHelper->GetEntryCount();
            if ( i < nCount ) 
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
    if (m_aBoxType == COMBOBOX && b_IsDropDownList)
    {
        
        
        if (m_pListBoxHelper->IsInDropDown())
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
    else if (m_aBoxType == COMBOBOX && !b_IsDropDownList)
    {
        
        NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, uno::Any(), uno::Any() );
    }
    else if (m_aBoxType == LISTBOX && b_IsDropDownList)
    {
        
        
        if (m_pListBoxHelper && m_pListBoxHelper->IsInDropDown())
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
    else if (m_aBoxType == LISTBOX && !b_IsDropDownList)
    {
        if ( aNewValue.hasValue())
        {
            NotifyListItem(aNewValue);
        }
    }
}

void VCLXAccessibleList::NotifyListItem(::com::sun::star::uno::Any& val)
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

void VCLXAccessibleList::UpdateFocus_Impl_Acc (sal_uInt16 nPos ,bool b_IsDropDownList)
{
    if (!(m_aBoxType == LISTBOX && !b_IsDropDownList))
    {
        return ;
    }
    Reference<XAccessible> xChild= CreateChild(nPos);
    if ( !xChild.is() )
    {
        return ;
    }
    m_nCurSelectedPos = nPos;
    uno::Any aOldValue, aNewValue;
    aNewValue <<= xChild;

    NotifyAccessibleEvent(
            AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
            aOldValue,
            aNewValue );
}


void VCLXAccessibleList::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent,  bool b_IsDropDownList)
{
    switch ( rVclWindowEvent.GetId() )
      {
        case VCLEVENT_DROPDOWN_SELECT:
        case VCLEVENT_LISTBOX_SELECT:
            if ( !m_bDisableProcessEvent )
                UpdateSelection_Impl_Acc(b_IsDropDownList);
            break;
        case VCLEVENT_LISTBOX_FOCUSITEMCHANGED:
            if ( !m_bDisableProcessEvent )
                UpdateFocus_Impl_Acc((sal_uInt16)reinterpret_cast<sal_uIntPtr>(rVclWindowEvent.GetData()),b_IsDropDownList);
            break;
        case VCLEVENT_WINDOW_GETFOCUS:
            break;
        case VCLEVENT_CONTROL_GETFOCUS:
            {
                VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
                if (m_aBoxType == COMBOBOX && b_IsDropDownList)
                {
                    
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
                        sal_uInt16 nPos = m_nCurSelectedPos; 

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
            {
                VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
                
                sal_Bool b_IsDropDownList = sal_True;
                if (m_pListBoxHelper)
                    b_IsDropDownList = ((m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN);
                if ( m_aBoxType == LISTBOX && !b_IsDropDownList )
                {
                    if ( m_pListBoxHelper )
                    {
                        uno::Any    aOldValue,
                                    aNewValue;
                        sal_uInt16 nPos = m_nCurSelectedPos;

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
    ListBox* pBox = static_cast<ListBox*>(GetWindow());
    if( m_aBoxType == LISTBOX  )
    {
        if (m_pListBoxHelper && (m_pListBoxHelper->GetStyle() & WB_DROPDOWN ) != WB_DROPDOWN)
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pBox->GetAccessible();
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
void VCLXAccessibleList::UpdateSelection (OUString sTextOfSelectedItem)
{
    if ( m_aBoxType == COMBOBOX )
    {
        ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
        if ( pBox != NULL )
        {
            
            sal_uInt16 nIndex = pBox->GetEntryPos(sTextOfSelectedItem);
            
            if ( nIndex == LISTBOX_ENTRY_NOTFOUND )
                nIndex = 0;
            UpdateSelection_Impl(nIndex);
        }
    }
}



Reference<XAccessible> VCLXAccessibleList::CreateChild (sal_Int32 i)
{
    Reference<XAccessible> xChild;

    sal_uInt16 nPos = static_cast<sal_uInt16>(i);
    if ( nPos >= m_aAccessibleChildren.size() )
    {
        m_aAccessibleChildren.resize(nPos + 1);

        
        xChild = new VCLXAccessibleListItem(m_pListBoxHelper, i, this);
        m_aAccessibleChildren[nPos] = xChild;
    }
    else
    {
        xChild = m_aAccessibleChildren[nPos];
        
        if (!xChild.is())
        {
            xChild = new VCLXAccessibleListItem(m_pListBoxHelper, i, this);
            m_aAccessibleChildren[nPos] = xChild;
        }
    }

    if ( xChild.is() )
    {
        
        bool bNowSelected = false;
        if ( m_pListBoxHelper )
            bNowSelected = m_pListBoxHelper->IsEntryPosSelected ((sal_uInt16)i);
        if (bNowSelected)
            m_nCurSelectedPos = sal_uInt16(i);
        VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >(xChild.get());
        pItem->SetSelected( bNowSelected );

        
        UpdateVisibleLineCount();
        sal_uInt16 nTopEntry = 0;
        if ( m_pListBoxHelper )
            nTopEntry = m_pListBoxHelper->GetTopEntry();
        bool bVisible = ( nPos>=nTopEntry && nPos<( nTopEntry + m_nVisibleLineCount ) );
        pItem->SetVisible( m_bVisible && bVisible );
    }

    return xChild;
}


void VCLXAccessibleList::HandleChangedItemList (bool /*bItemInserted*/, sal_Int32 /*nIndex*/)
{
    clearItems();
    NotifyAccessibleEvent (
        AccessibleEventId::INVALIDATE_ALL_CHILDREN,
        Any(), Any());
}


IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleList, VCLXAccessibleComponent, VCLXAccessibleList_BASE)



Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleList::getAccessibleContext (void)
    throw (RuntimeException)
{
    return this;
}




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


Reference<XAccessible> SAL_CALL VCLXAccessibleList::getAccessibleChild (sal_Int32 i)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    
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
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}


sal_Int32 SAL_CALL VCLXAccessibleList::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}


sal_Int16 SAL_CALL VCLXAccessibleList::getAccessibleRole (void)
    throw (RuntimeException)
{
    return AccessibleRole::LIST;
}




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




OUString VCLXAccessibleList::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleList" );
}


Sequence< OUString > VCLXAccessibleList::getSupportedServiceNames (void)
    throw (RuntimeException)
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
            m_nCurSelectedPos = LISTBOX_ENTRY_NOTFOUND;
            for ( ListItems::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != m_aAccessibleChildren.end(); ++aIter,++i)
            {
                Reference< XAccessible > xHold = *aIter;
                if ( xHold.is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( xHold.get() );
                    
                    sal_Bool bNowSelected = m_pListBoxHelper->IsEntryPosSelected (i);
                    if (bNowSelected)
                        m_nCurSelectedPos = i;

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
                { 
                    checkEntrySelected(i,aNewValue,xNewAcc);
                }
            }
            sal_uInt16 nCount = m_pListBoxHelper->GetEntryCount();
            if ( i < nCount ) 
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
            if (m_pListBoxHelper->IsInDropDown())
            {
                if ( aNewValue.hasValue() || aOldValue.hasValue() )
                    NotifyAccessibleEvent(
                            AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                            aOldValue,
                            aNewValue );
                
                
            }
        }
    }
}




void SAL_CALL VCLXAccessibleList::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nChildIndex,*m_pListBoxHelper,sal_False);

            m_pListBoxHelper->SelectEntryPos( (sal_uInt16)nChildIndex, true );
            
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

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
                m_pListBoxHelper->SelectEntryPos( i, true );
            
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

sal_Int32 SAL_CALL VCLXAccessibleList::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nCount = 0;
    if ( m_pListBoxHelper )
           nCount = m_pListBoxHelper->GetSelectEntryCount();
    return nCount;
}

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

void SAL_CALL VCLXAccessibleList::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if ( m_pListBoxHelper )
        {
            checkSelection_Impl(nSelectedChildIndex,*m_pListBoxHelper,sal_False);

            m_pListBoxHelper->SelectEntryPos( (sal_uInt16)nSelectedChildIndex, false );
            
            m_bDisableProcessEvent = true;
            m_pListBoxHelper->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}

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
        
        aBounds = VCLXAccessibleComponent::implGetBounds();
        aBounds.X = 0;
        aBounds.Y = 0;
        if ( m_aBoxType == COMBOBOX )
        {
            ComboBox* pBox = static_cast<ComboBox*>(GetWindow());
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
                aPos.Y += pBox->GetSubEdit()->GetSizePixel().Height();
            }
        }
    }
    return aPos;
}



sal_Bool VCLXAccessibleList::IsInDropDown()
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
