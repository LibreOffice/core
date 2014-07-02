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
    // Set up the flags that indicate which children this object has.
    m_bHasListChild = true;

    // A text field is not present for non drop down list boxes.
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
            // Just compare to the combo box text field.  All other children
            // are identical to this object in which case this object will
            // be removed in a short time.
            if (m_aBoxType==COMBOBOX)
            {
                ComboBox* pComboBox = static_cast<ComboBox*>(GetWindow());
                if ( ( pComboBox != NULL ) && ( pChildWindow != NULL ) )
                    if (pChildWindow == pComboBox->GetSubEdit())
                    {
                        if (rVclWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW)
                        {
                            // Instantiate text field.
                            getAccessibleChild (0);
                            aNewValue <<= m_xText;
                        }
                        else
                        {
                            // Release text field.
                            aOldValue <<= m_xText;
                            m_xText = NULL;
                        }
                        // Tell the listeners about the new/removed child.
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
                // Forward the call to the list child.
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
            if (pList != NULL && m_xText.is())
            {
                Reference<XAccessibleText> xText (m_xText->getAccessibleContext(), UNO_QUERY);
                if ( xText.is() )
                {
                    ::rtl::OUString sText = xText->getSelectedText();
                    if ( sText.isEmpty() )
                        sText = xText->getText();
                            pList->UpdateSelection_Acc (sText, m_bIsDropDownBox);
                    //if(m_bIsDropDownBox && !pList->IsInDropDown())
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
        //case VCLEVENT_DROPDOWN_OPEN:
        //case VCLEVENT_DROPDOWN_CLOSE:
        case VCLEVENT_LISTBOX_DOUBLECLICK:
        case VCLEVENT_LISTBOX_SCROLLED:
        //case VCLEVENT_LISTBOX_SELECT:
        case VCLEVENT_LISTBOX_ITEMADDED:
        case VCLEVENT_LISTBOX_ITEMREMOVED:
        case VCLEVENT_COMBOBOX_ITEMADDED:
        case VCLEVENT_COMBOBOX_ITEMREMOVED:
        case VCLEVENT_COMBOBOX_SCROLLED:
        {
            // Forward the call to the list child.
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

        //case VCLEVENT_COMBOBOX_SELECT:
        case VCLEVENT_COMBOBOX_DESELECT:
        {
            // Selection is handled by VCLXAccessibleList which operates on
            // the same VCL object as this box does.  In case of the
            // combobox, however, we have to help by providing the list with
            // the text of the currently selected item.
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
     // case VCLEVENT_EDIT_CARETCHANGED:
            // Modify/Selection events are handled by the combo box instead of
            // directly by the edit field (Why?).  Therefore, delegate this
            // call to the edit field.
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
        // MT: Not sending VCLEVENT_LISTBOX_STATEUPDATE, see comment in ListBox::SelectEntryPos
        case VCLEVENT_LISTBOX_STATEUPDATE:
        {
            // Need to update the INDETERMINATE state sometimes
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

//=====  XAccessible  =========================================================

Reference< XAccessibleContext > SAL_CALL VCLXAccessibleBox::getAccessibleContext(  )
    throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleChildCount (void)
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Usually a box has a text field and a list of items as its children.
    // Non drop down list boxes have no text field.  Additionally check
    // whether the object is valid.
    sal_Int32 nCount = 0;
    if (IsValid())
        nCount += (m_bHasTextChild?1:0) + (m_bHasListChild?1:0);
    else
    {
        // Object not valid anymore.  Release references to children.
        m_bHasTextChild = false;
        m_xText = NULL;
        m_bHasListChild = false;
        m_xList = NULL;
    }

    return nCount;
}

Reference<XAccessible> SAL_CALL VCLXAccessibleBox::getAccessibleChild (sal_Int32 i)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
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
            // List.
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
            // Text Field.
            if ( ! m_xText.is())
            {
                if (m_aBoxType==COMBOBOX)
                {
                    ComboBox* pComboBox = static_cast<ComboBox*>(GetWindow());
                    if (pComboBox!=NULL && pComboBox->GetSubEdit()!=NULL)
                    //Set the edit's acc name the same as parent
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

sal_Int16 SAL_CALL VCLXAccessibleBox::getAccessibleRole (void) throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    // Return the role <const>COMBO_BOX</const> for both VCL combo boxes and
    // VCL list boxes in DropDown-Mode else <const>PANEL</const>.
    // This way the Java bridge has not to handle both independently.
    //return m_bIsDropDownBox ? AccessibleRole::COMBO_BOX : AccessibleRole::PANEL;
    if (m_bIsDropDownBox || (!m_bIsDropDownBox && m_aBoxType == COMBOBOX ))
        return AccessibleRole::COMBO_BOX;
    else
        return AccessibleRole::PANEL;
}

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}

//=====  XAccessibleAction  ===================================================

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleActionCount (void)
    throw (RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex> aGuard (GetMutex());

    // There is one action for drop down boxes (toggle popup) and none for
    // the other boxes.
    return m_bIsDropDownBox ? 1 : 0;
}

sal_Bool SAL_CALL VCLXAccessibleBox::doAccessibleAction (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    bool bNotify = false;

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
                bNotify = true;
            }
        }
        else if (m_aBoxType == LISTBOX)
        {
            ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
            if (pListBox != NULL)
            {
                pListBox->ToggleDropDown();
                bNotify = true;
            }
        }
    }

    if (bNotify)
        NotifyAccessibleEvent (AccessibleEventId::ACTION_CHANGED, Any(), Any());

    return bNotify;
}

OUString SAL_CALL VCLXAccessibleBox::getAccessibleActionDescription (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if (nIndex<0 || nIndex>=getAccessibleActionCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    //Solution:When combo_box,it should not has action information.
    //return TK_RES_STRING( RID_STR_ACC_ACTION_TOGGLEPOPUP);
    if (m_aBoxType == LISTBOX)
        return ::rtl::OUString();
    return m_bIsDropDownBox?::rtl::OUString():TK_RES_STRING( RID_STR_ACC_ACTION_TOGGLEPOPUP);
}

Reference< XAccessibleKeyBinding > VCLXAccessibleBox::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Reference< XAccessibleKeyBinding > xRet;

    if (nIndex<0 || nIndex>=getAccessibleActionCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();

    // ... which key?
    return xRet;
}

//=====  XComponent  ==========================================================

void SAL_CALL VCLXAccessibleBox::disposing (void)
{
    VCLXAccessibleComponent::disposing();
}

// =====  XAccessibleValue  ===============================================
Any VCLXAccessibleBox::getCurrentValue( )
    throw( RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    Any aAny;
    if( m_xList.is() && m_xText.is())
    {
        // VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
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
    throw( RuntimeException, std::exception )
{
    SolarMutexGuard aSolarGuard;
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString  fValue;
    bool bValid = (aNumber >>= fValue);
    if( bValid )
    {

    }
    return bValid;

}

Any VCLXAccessibleBox::getMaximumValue( )
    throw( RuntimeException, std::exception )
{
    Any aAny;
    return aAny;
}

Any VCLXAccessibleBox::getMinimumValue(  )
    throw( RuntimeException, std::exception )
{
    Any aAny;
    return aAny;
}

// Set the INDETERMINATE state when there is no selected item for combobox
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
