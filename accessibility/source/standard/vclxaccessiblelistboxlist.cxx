/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessiblelistboxlist.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/standard/vclxaccessiblelistboxlist.hxx>
#include <accessibility/standard/vclxaccessiblelistitem.hxx>
#include <accessibility/helper/listboxhelper.hxx>

#include <algorithm>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/unohelp.hxx>

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <unotools/accessiblestatesethelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;


namespace
{
    void checkSelection_Impl( sal_Int32 _nIndex, const ListBox& _rListBox, sal_Bool bSelected )
        throw (::com::sun::star::lang::IndexOutOfBoundsException)
    {
        sal_Int32 nCount = bSelected ? (sal_Int32)_rListBox.GetSelectEntryCount()
                                     : (sal_Int32)_rListBox.GetEntryCount();
        if ( _nIndex < 0 || _nIndex >= nCount )
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }
}


VCLXAccessibleListBoxList::VCLXAccessibleListBoxList (VCLXWindow* pVCLWindow,
    BoxType aBoxType,const Reference< XAccessible >& _xParent)
    : VCLXAccessibleList (pVCLWindow, aBoxType, _xParent)
{
}




VCLXAccessibleListBoxList::~VCLXAccessibleListBoxList (void)
{
}




void VCLXAccessibleListBoxList::ProcessWindowEvent (const VclWindowEvent& rVclWindowEvent)
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_LISTBOX_SCROLLED:
            UpdateEntryRange_Impl();
            break;

        case VCLEVENT_LISTBOX_SELECT:
            if ( !m_bDisableProcessEvent )
                UpdateSelection_Impl();
            break;

        default:
            VCLXAccessibleList::ProcessWindowEvent (rVclWindowEvent);
    }
}

IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleListBoxList, VCLXAccessibleList, VCLXAccessibleListBoxList_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleListBoxList, VCLXAccessibleList, VCLXAccessibleListBoxList_BASE)



//=====  XServiceInfo  ========================================================

::rtl::OUString VCLXAccessibleListBoxList::getImplementationName (void)
    throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.toolkit.AccessibleListBoxList");
}




Sequence< ::rtl::OUString > VCLXAccessibleListBoxList::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames = VCLXAccessibleList::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = ::rtl::OUString::createFromAscii(
        "com.sun.star.accessibility.AccessibleListBoxList");
    return aNames;
}
// -----------------------------------------------------------------------------

void VCLXAccessibleListBoxList::UpdateSelection_Impl()
{
    uno::Any aOldValue, aNewValue;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        Reference< XAccessible > xNewAcc;

        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if ( pListBox )
        {
            USHORT nPos = 0;
            ListItems::iterator aEnd = m_aAccessibleChildren.end();
            for ( ListItems::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != aEnd; ++aIter,++nPos)
            {
                if ( aIter->is() )
                {
                    VCLXAccessibleListItem* pItem = static_cast< VCLXAccessibleListItem* >( aIter->get() );
                    // Retrieve the item's index from the list entry.
                    BOOL bNowSelected = pListBox->IsEntryPosSelected (nPos);

                    if ( bNowSelected && !pItem->IsSelected() )
                    {
                        xNewAcc = *aIter;
                        aNewValue <<= xNewAcc;
                        m_nLastSelectedPos = nPos;
                    }

                    pItem->SetSelected( bNowSelected );
                }
                else
                { // it could happen that a child was not created before
                    checkEntrySelected(pListBox,nPos,aNewValue,xNewAcc);
                }
            }

            USHORT nCount = pListBox->GetEntryCount();
            if ( nPos < nCount ) // here we have to check the if any other listbox entry is selected
            {
                for (; nPos < nCount && !checkEntrySelected(pListBox,nPos,aNewValue,xNewAcc) ;++nPos )
                    ;
            }
        }

        if ( xNewAcc.is() && pListBox->HasFocus() )
        {
            if ( m_nLastSelectedPos != LISTBOX_ENTRY_NOTFOUND )
                aOldValue <<= getAccessibleChild( (sal_Int32)m_nLastSelectedPos );
            aNewValue <<= xNewAcc;
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
void SAL_CALL VCLXAccessibleListBoxList::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if ( pListBox )
        {
            checkSelection_Impl( nChildIndex, *pListBox, sal_False );
            pListBox->SelectEntryPos( (USHORT)nChildIndex, TRUE );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            pListBox->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL VCLXAccessibleListBoxList::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bRet = sal_False;
    ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
    if ( pListBox )
    {
        checkSelection_Impl( nChildIndex, *pListBox, sal_False );
        bRet = pListBox->IsEntryPosSelected( (USHORT)nChildIndex );
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleListBoxList::clearAccessibleSelection(  ) throw (RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if ( pListBox )
        {
            pListBox->SetNoSelection();
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleListBoxList::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if ( pListBox )
        {
            USHORT nCount = pListBox->GetEntryCount();
            for ( USHORT i = 0; i < nCount; ++i )
                pListBox->SelectEntryPos( i, TRUE );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            pListBox->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleListBoxList::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nCount = 0;
    ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
    if ( pListBox )
        nCount = pListBox->GetSelectEntryCount();
    return nCount;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL VCLXAccessibleListBoxList::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
    if ( pListBox )
    {
        checkSelection_Impl( nSelectedChildIndex, *pListBox, sal_True );
        return getAccessibleChild( (sal_Int32)pListBox->GetSelectEntryPos( (USHORT)nSelectedChildIndex ) );
    }

    return NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleListBoxList::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    sal_Bool bNotify = sal_False;

    {
        vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        ListBox* pListBox = static_cast< ListBox* >( GetWindow() );
        if ( pListBox )
        {
            checkSelection_Impl( nSelectedChildIndex, *pListBox, sal_False );
            pListBox->SelectEntryPos( (USHORT)nSelectedChildIndex, FALSE );
            // call the select handler, don't handle events in this time
            m_bDisableProcessEvent = true;
            pListBox->Select();
            m_bDisableProcessEvent = false;
            bNotify = sal_True;
        }
    }

    if ( bNotify )
        UpdateSelection_Impl();
}
// -----------------------------------------------------------------------------

