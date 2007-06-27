/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblebox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:36:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBOX_HXX
#include <accessibility/standard/vclxaccessiblebox.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLETEXTFIELD_HXX
#include <accessibility/standard/vclxaccessibletextfield.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEEDIT_HXX
#include <accessibility/standard/vclxaccessibleedit.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELIST_HXX
#include <accessibility/standard/vclxaccessiblelist.hxx>
#endif
#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLELISTBOXLIST_HXX
#include <accessibility/standard/vclxaccessiblelistboxlist.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX
#include <accessibility/helper/listboxhelper.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_ACCESSIBLESTRINGS_HRC_
#include <accessibility/helper/accessiblestrings.hrc>
#endif

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

void VCLXAccessibleBox::SetIndexInParent (sal_Int32 _nIndex)
{
    m_nIndexInParent = _nIndex;
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
        case VCLEVENT_DROPDOWN_OPEN:
        case VCLEVENT_DROPDOWN_CLOSE:
        case VCLEVENT_LISTBOX_DOUBLECLICK:
        case VCLEVENT_LISTBOX_SCROLLED:
        case VCLEVENT_LISTBOX_SELECT:
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

        case VCLEVENT_COMBOBOX_SELECT:
        case VCLEVENT_COMBOBOX_DESELECT:
        {
            // Selection is handled by VCLXAccessibleList which operates on
            // the same VCL object as this box does.  In case of the
            // combobox, however, we have to help the list with providing
            // the text of the currently selected item.
            VCLXAccessibleList* pList = static_cast<VCLXAccessibleList*>(m_xList.get());
            if (pList != NULL)
            {
                Reference<XAccessibleText> xText (m_xText->getAccessibleContext(), UNO_QUERY);
                if ( xText.is() )
                {
                    ::rtl::OUString sText = xText->getSelectedText();
                    if ( !sText.getLength() )
                        sText = xText->getText();
                    pList->UpdateSelection (sText);
                }
            }
            break;
        }

        case VCLEVENT_EDIT_MODIFY:
        case VCLEVENT_EDIT_SELECTIONCHANGED:
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

        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleBox, VCLXAccessibleComponent, VCLXAccessibleBox_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleBox, VCLXAccessibleComponent, VCLXAccessibleBox_BASE)

//=====  XAccessible  =========================================================

Reference< XAccessibleContext > SAL_CALL VCLXAccessibleBox::getAccessibleContext(  )
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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
                        m_xText = pComboBox->GetSubEdit()->GetAccessible();
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

    // Return the role <const>COMBO_BOX</const> for both VCL combo boxes and
    // VCL list boxes in DropDown-Mode else <const>PANEL</const>.
    // This way the Java bridge has not to handle both independently.
    return m_bIsDropDownBox ? AccessibleRole::COMBO_BOX : AccessibleRole::PANEL;
}

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleIndexInParent (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (m_nIndexInParent != DEFAULT_INDEX_IN_PARENT)
        return m_nIndexInParent;
    else
        return VCLXAccessibleComponent::getAccessibleIndexInParent();
}

//=====  XAccessibleAction  ===================================================

sal_Int32 SAL_CALL VCLXAccessibleBox::getAccessibleActionCount (void)
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex> aGuard (GetMutex());

    // There is one action for drop down boxes (toggle popup) and none for
    // the other boxes.
    return m_bIsDropDownBox ? 1 : 0;
}

sal_Bool SAL_CALL VCLXAccessibleBox::doAccessibleAction (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        if (nIndex<0 || nIndex>=getAccessibleActionCount())
            throw ::com::sun::star::lang::IndexOutOfBoundsException();

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

::rtl::OUString SAL_CALL VCLXAccessibleBox::getAccessibleActionDescription (sal_Int32 nIndex)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    if (nIndex<0 || nIndex>=getAccessibleActionCount())
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    return TK_RES_STRING( RID_STR_ACC_ACTION_TOGGLEPOPUP);
}

Reference< XAccessibleKeyBinding > VCLXAccessibleBox::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
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

