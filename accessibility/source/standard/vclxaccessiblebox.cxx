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

#include <accessibility/standard/vclxaccessiblebox.hxx>
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#include <accessibility/standard/vclxaccessibleedit.hxx>
#include <accessibility/standard/vclxaccessiblelist.hxx>
#include <accessibility/helper/listboxhelper.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;

VCLXAccessibleBox::VCLXAccessibleBox (VCLXWindow* pVCLWindow, BoxType aType, bool bIsDropDownBox)
    : VCLXAccessibleComponent (pVCLWindow),
      m_aBoxType (aType),
      m_bIsDropDownBox (bIsDropDownBox),
      m_nIndexInParent (DEFAULT_INDEX_IN_PARENT)
{
    
    m_bHasListChild = true;

    
    if ((m_aBoxType==LISTBOX) && ! m_bIsDropDownBox)
        m_bHasTextChild = false;
    else
        m_bHasTextChild = true;
}

VCLXAccessibleBox::~VCLXAccessibleBox (void)
{
}

void VCLXAccessibleBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
       uno::Any aOldValue, aNewValue;
    uno::Reference<XAccessible> xAcc;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:
        case VCLEVENT_WINDOW_HIDE:
        {
            Window* pChildWindow = (Window *) rVclWindowEvent.GetData();
            
            
            
            if (m_aBoxType==COMBOBOX)
            {
                ComboBox* pComboBox = static_cast<ComboBox*>(GetWindow());
                if ( ( pComboBox != NULL ) && ( pChildWindow != NULL ) )
                    if (pChildWindow == pComboBox->GetSubEdit())
                    {
                        if (rVclWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW)
                        {
                            
                            getAccessibleChild (0);
                            aNewValue <<= m_xText;
                        }
                        else
                        {
                            
                            aOldValue <<= m_xText;
                            m_xText = NULL;
                        }
                        
                        NotifyAccessibleEvent (
                            AccessibleEventId::CHILD,
                            aOldValue, aNewValue);
                    }

            }
        }
        break;

        default:
            VCLXAccessibleComponent::ProcessWindowChildEvent (rVclWindowEvent);
    }
}

void VCLXAccessibleBox::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_DROPDOWN_SELECT:
        case VCLEVENT_LISTBOX_SELECT:
        case VCLEVENT_LISTBOX_FOCUSITEMCHANGED:

        {
                
                VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
                if ( pList == NULL )
            {
                getAccessibleChild ( m_bHasTextChild ? 1 : 0 );
                pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            }
            if ( pList != NULL )
            {
                pList->ProcessWindowEvent (rVclWindowEvent, m_bIsDropDownBox);
                if(m_bIsDropDownBox)
                {
                    NotifyAccessibleEvent(AccessibleEventId::VALUE_CHANGED, Any(), Any());
                    Any aOldValue;
                    Any aNewValue;
                    aOldValue <<= AccessibleStateType::INDETERMINATE;
                    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue);

                }
            }
            break;
        }
        case VCLEVENT_DROPDOWN_OPEN:
        {
                 VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
                 if ( pList == NULL )
            {
                getAccessibleChild ( m_bHasTextChild ? 1 : 0 );
                pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            }
            if ( pList != NULL )
            {
                pList->ProcessWindowEvent (rVclWindowEvent);
                pList->HandleDropOpen();
            }
            break;
        }
        case VCLEVENT_DROPDOWN_CLOSE:
        {
                 VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
                 if ( pList == NULL )
            {
                getAccessibleChild ( m_bHasTextChild ? 1 : 0 );
                pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            }
            if ( pList != NULL )
            {
                pList->ProcessWindowEvent (rVclWindowEvent);
            }
            Window* pWindow = GetWindow();
            if( pWindow && (pWindow->HasFocus() || pWindow->HasChildPathFocus()) )
            {
                Any aOldValue, aNewValue;
                aNewValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
            break;
        }
        case VCLEVENT_COMBOBOX_SELECT:
        {
                 VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
                 if (pList != NULL)
                 {
                        Reference<XAccessibleText> xText (m_xText->getAccessibleContext(), UNO_QUERY);
                        if ( xText.is() )
                {
                    ::rtl::OUString sText = xText->getSelectedText();
                    if ( sText.isEmpty() )
                        sText = xText->getText();
                            pList->UpdateSelection_Acc (sText, m_bIsDropDownBox);
                    
                    if (m_bIsDropDownBox || ( !m_bIsDropDownBox && m_aBoxType==COMBOBOX))
                        NotifyAccessibleEvent(AccessibleEventId::VALUE_CHANGED, Any(), Any());

                    Any aOldValue;
                    Any aNewValue;
                    aOldValue <<= AccessibleStateType::INDETERMINATE;
                    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue);

                }
                    }
            break;
        }
        
        
        case VCLEVENT_LISTBOX_DOUBLECLICK:
        case VCLEVENT_LISTBOX_SCROLLED:
        
        case VCLEVENT_LISTBOX_ITEMADDED:
        case VCLEVENT_LISTBOX_ITEMREMOVED:
        case VCLEVENT_COMBOBOX_ITEMADDED:
        case VCLEVENT_COMBOBOX_ITEMREMOVED:
        case VCLEVENT_COMBOBOX_SCROLLED:
        {
            
            VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            if ( pList == NULL )
            {
                getAccessibleChild ( m_bHasTextChild ? 1 : 0 );
                pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            }
            if ( pList != NULL )
                pList->ProcessWindowEvent (rVclWindowEvent);
            break;
        }

        
        case VCLEVENT_COMBOBOX_DESELECT:
        {
            
            
            
            
            VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            if (pList != NULL && m_xText.is())
            {
                Reference<XAccessibleText> xText (m_xText->getAccessibleContext(), UNO_QUERY);
                if ( xText.is() )
                {
                    OUString sText = xText->getSelectedText();
                    if ( sText.isEmpty() )
                        sText = xText->getText();
                    pList->UpdateSelection (sText);
                }
            }
            break;
        }

        case VCLEVENT_EDIT_MODIFY:
        case VCLEVENT_EDIT_SELECTIONCHANGED:
     
            
            
            
            if (m_aBoxType==COMBOBOX)
            {
                if (m_xText.is())
                {
                    Reference<XAccessibleContext> xContext = m_xText->getAccessibleContext();
                    VCLXAccessibleEdit* pEdit = static_cast<VCLXAccessibleEdit*>(xContext.get());
                    if (pEdit != NULL)
                        pEdit->ProcessWindowEvent (rVclWindowEvent);
                }
            }
            break;
        /*
        
        case VCLEVENT_LISTBOX_STATEUPDATE:
        {
            
            if (m_bIsDropDownBox && m_aBoxType==LISTBOX)
            {
                sal_Int32 nSelectedEntryCount = 0;
                ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
                if (pListBox != NULL && pListBox->GetEntryCount() > 0)
                {
                    nSelectedEntryCount = pListBox->GetSelectEntryCount();
                    Any aOldValue;
                    Any aNewValue;
                    if ( nSelectedEntryCount == 0)
                        aNewValue <<= AccessibleStateType::INDETERMINATE;
                    else
                        aOldValue <<= AccessibleStateType::INDETERMINATE;
                    NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue);
                }
            }
            break;
        }
        */
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleBox, VCLXAccessibleComponent, VCLXAccessibleBox_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleBox, VCLXAccessibleComponent, VCLXAccessibleBox_BASE)



Reference< XAccessibleContext > SAL_CALL VCLXAccessibleBox::getAccessibleContext(  )
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return this;
}



sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    
    
    
    sal_Int32 nCount = 0;
    if (IsValid())
        nCount += (m_bHasTextChild?1:0) + (m_bHasListChild?1:0);
    else
    {
        
        m_bHasTextChild = false;
        m_xText = NULL;
        m_bHasListChild = false;
        m_xList = NULL;
    }

    return nCount;
}

Reference<XAccessible> SAL_CALL VCLXAccessibleBox::getAccessibleChild (sal_Int32 i)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if (i<0 || i>=getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    if (IsValid())
    {
        if (i==1 || ! m_bHasTextChild)
        {
            
            if ( ! m_xList.is())
            {
                VCLXAccessibleList* pList = new VCLXAccessibleList ( GetVCLXWindow(),
                    (m_aBoxType == LISTBOX ? VCLXAccessibleList::LISTBOX : VCLXAccessibleList::COMBOBOX),
                                                                    this);
                pList->SetIndexInParent (i);
                m_xList = pList;
            }
            xChild = m_xList;
        }
        else
        {
            
            if ( ! m_xText.is())
            {
                if (m_aBoxType==COMBOBOX)
                {
                    ComboBox* pComboBox = static_cast<ComboBox*>(GetWindow());
                    if (pComboBox!=NULL && pComboBox->GetSubEdit()!=NULL)
                    
                    {
                        pComboBox->GetSubEdit()->SetAccessibleName(getAccessibleName());
                        m_xText = pComboBox->GetSubEdit()->GetAccessible();
                    }
                }
                else if (m_bIsDropDownBox)
                    m_xText = new VCLXAccessibleTextField (GetVCLXWindow(),this);
            }
            xChild = m_xText;
        }
    }

    return xChild;
}

sal_Int16 SAL_CALL VCLXAccessibleBox::getAccessibleRole (void) throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    
    
    
    
    if (m_bIsDropDownBox || (!m_bIsDropDownBox && m_aBoxType == COMBOBOX ))
        return AccessibleRole::COMBO_BOX;
    else
        return AccessibleRole::PANEL;
}

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}



sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleActionCount (void)
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex> aGuard (GetMutex());

    
    
    return m_bIsDropDownBox ? 1 : 0;
}

sal_Bool SAL_CALL VCLXAccessibleBox::doAccessibleAction (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        SolarMutexGuard aSolarGuard;
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if (nIndex<0 || nIndex>=getAccessibleActionCount())
            throw ::com::sun::star::lang::IndexOutOfBoundsException(
                ("VCLXAccessibleBox::doAccessibleAction: index "
                 + OUString::number(nIndex) + " not among 0.."
                 + OUString::number(getAccessibleActionCount())),
                static_cast<OWeakObject*>(this));

        if (m_aBoxType == COMBOBOX)
        {
            ComboBox* pComboBox = static_cast< ComboBox* >( GetWindow() );
            if (pComboBox != NULL)
            {
                pComboBox->ToggleDropDown();
                bNotify = sal_True;
            }
        }
        else if (m_aBoxType == LISTBOX)
        {
            ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
            if (pListBox != NULL)
            {
                pListBox->ToggleDropDown();
                bNotify = sal_True;
            }
        }
    }

    if (bNotify)
        NotifyAccessibleEvent (AccessibleEventId::ACTION_CHANGED, Any(), Any());

    return bNotify;
}

OUString SAL_CALL VCLXAccessibleBox::getAccessibleActionDescription (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if (nIndex<0 || nIndex>=getAccessibleActionCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    
    
    if (m_aBoxType == LISTBOX)
        return ::rtl::OUString();
    return m_bIsDropDownBox?::rtl::OUString():TK_RES_STRING( RID_STR_ACC_ACTION_TOGGLEPOPUP);
}

Reference< XAccessibleKeyBinding > VCLXAccessibleBox::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Reference< XAccessibleKeyBinding > xRet;

    if (nIndex<0 || nIndex>=getAccessibleActionCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    
    return xRet;
}



void SAL_CALL VCLXAccessibleBox::disposing (void)
{
    VCLXAccessibleComponent::disposing();
}


Any VCLXAccessibleBox::getCurrentValue( )
    throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Any aAny;
    if( m_xList.is() && m_xText.is())
    {
        
        Reference<XAccessibleText> xText (m_xText->getAccessibleContext(), UNO_QUERY);
        if ( xText.is() )
        {
            ::rtl::OUString sText = xText->getText();
            aAny <<= sText;
        }
    }
    if (m_aBoxType == LISTBOX && m_bIsDropDownBox  && m_xList.is() )
    {

        VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
        if(pList->IsInDropDown())
        {
            if(pList->getSelectedAccessibleChildCount()>0)
            {
                Reference<XAccessibleContext> xName (pList->getSelectedAccessibleChild((sal_Int32)(0)), UNO_QUERY);
                if(xName.is())
                {
                    aAny <<= xName->getAccessibleName();
                }
            }
        }
    }

    return aAny;
}

sal_Bool VCLXAccessibleBox::setCurrentValue( const Any& aNumber )
    throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString  fValue;
    sal_Bool bValid = (aNumber >>= fValue);
    if( bValid )
    {

    }
    return bValid;

}

Any VCLXAccessibleBox::getMaximumValue( )
    throw( RuntimeException )
{
    Any aAny;
    return aAny;
}

Any VCLXAccessibleBox::getMinimumValue(  )
    throw( RuntimeException )
{
    Any aAny;
    return aAny;
}


void VCLXAccessibleBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet(rStateSet);
    if (m_aBoxType == COMBOBOX )
    {
        ::rtl::OUString sText;
        sal_Int32 nEntryCount = 0;
        ComboBox* pComboBox = static_cast<ComboBox*>(GetWindow());
        if (pComboBox != NULL)
        {
            Edit* pSubEdit = pComboBox->GetSubEdit();
            if ( pSubEdit)
                sText = pSubEdit->GetText();
            nEntryCount = pComboBox->GetEntryCount();
        }
        if ( sText.isEmpty() && nEntryCount > 0 )
            rStateSet.AddState(AccessibleStateType::INDETERMINATE);
    }
    else if (m_aBoxType == LISTBOX && m_bIsDropDownBox == true)
    {
        sal_Int32 nSelectedEntryCount = 0;
        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if (pListBox != NULL && pListBox->GetEntryCount() > 0)
        {
                nSelectedEntryCount = pListBox->GetSelectEntryCount();
            if ( nSelectedEntryCount == 0)
                rStateSet.AddState(AccessibleStateType::INDETERMINATE);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
