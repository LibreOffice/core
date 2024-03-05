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
#include <string.h>

#include <standard/vclxaccessibletoolbox.hxx>
#include <standard/vclxaccessibletoolboxitem.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <o3tl/safeint.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/vclevent.hxx>
#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessiblewrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

namespace
{

    // = OToolBoxWindowItemContext

    /** XAccessibleContext implementation for a toolbox item which is represented by a VCL Window
    */
    class OToolBoxWindowItemContext final : public OAccessibleContextWrapper
    {
        sal_Int32 m_nIndexInParent;
    public:
        OToolBoxWindowItemContext(sal_Int32 _nIndexInParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxOwningAccessible,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParentAccessible
            ) : OAccessibleContextWrapper(
            _rxContext,
            _rxInnerAccessibleContext,
            _rxOwningAccessible,
            _rxParentAccessible     )
            ,m_nIndexInParent(_nIndexInParent)
        {
        }
        virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
    };


    sal_Int64 SAL_CALL OToolBoxWindowItemContext::getAccessibleIndexInParent(  )
    {
        return m_nIndexInParent;
    }


    // = OToolBoxWindowItem

    /** XAccessible implementation for a toolbox item which is represented by a VCL Window
    */
    class OToolBoxWindowItem : public OAccessibleWrapper
    {
    private:
        sal_Int32 m_nIndexInParent;

    public:
        OToolBoxWindowItem(sal_Int32 _nIndexInParent,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxInnerAccessible,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParentAccessible
            ) : OAccessibleWrapper(
            _rxContext,
            _rxInnerAccessible,
            _rxParentAccessible)
            ,m_nIndexInParent(_nIndexInParent)
        {
        }

    protected:
        // OAccessibleWrapper
        virtual rtl::Reference<OAccessibleContextWrapper> createAccessibleContext(
                const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerContext
            ) override;
    };

    rtl::Reference<OAccessibleContextWrapper> OToolBoxWindowItem::createAccessibleContext(
            const Reference< XAccessibleContext >& _rxInnerContext )
    {
        return new OToolBoxWindowItemContext( m_nIndexInParent, getComponentContext(), _rxInnerContext, this, getParent() );
    }
}

// VCLXAccessibleToolBox

VCLXAccessibleToolBox::VCLXAccessibleToolBox( VCLXWindow* pVCLXWindow ) :

    ImplInheritanceHelper( pVCLXWindow )

{
}

VCLXAccessibleToolBox::~VCLXAccessibleToolBox()
{
}

VCLXAccessibleToolBoxItem* VCLXAccessibleToolBox::GetItem_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VCLXAccessibleToolBoxItem* pItem = nullptr;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        // returns only toolbox buttons, not windows
        if ( aIter != m_aAccessibleChildren.end()  && aIter->second.is())
            pItem = aIter->second.get();
    }

    return pItem;
}

void VCLXAccessibleToolBox::UpdateFocus_Impl()
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if( !pToolBox )
        return;

    // submit events only if toolbox has the focus to avoid sending events due to mouse move
    bool bHasFocus = false;
    if ( pToolBox->HasFocus() )
        bHasFocus = true;
    else
    {
        // check for subtoolbar, i.e. check if our parent is a toolbar
        ToolBox* pToolBoxParent = dynamic_cast< ToolBox* >( pToolBox->GetParent() );
        // subtoolbars never get the focus as key input is just forwarded, so check if the parent toolbar has it
        if ( pToolBoxParent && pToolBoxParent->HasFocus() )
            bHasFocus = true;
    }

    if ( !bHasFocus )
        return;

    ToolBoxItemId nHighlightItemId = pToolBox->GetHighlightItemId();
    sal_uInt16 nFocusCount = 0;
    for ( const auto& [rPos, rxChild] : m_aAccessibleChildren )
    {
        ToolBoxItemId nItemId = pToolBox->GetItemId( rPos );

        if ( rxChild.is() )
        {
            VCLXAccessibleToolBoxItem* pItem = rxChild.get();
            if ( pItem->HasFocus() && nItemId != nHighlightItemId )
            {
                // reset the old focused item
                pItem->SetFocus( false );
                nFocusCount++;
            }
            if ( nItemId == nHighlightItemId )
            {
                // set the new focused item
                pItem->SetFocus( true );
                nFocusCount++;
            }
        }
        // both items changed?
        if ( nFocusCount > 1 )
            break;
    }
}

void VCLXAccessibleToolBox::ReleaseFocus_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox ) // #107124#, do not check for focus because this message is also handled in losefocus
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem = aIter->second.get();
            if ( pItem->HasFocus() )
                pItem->SetFocus( false );
        }
    }
}

void VCLXAccessibleToolBox::UpdateChecked_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    ToolBoxItemId nFocusId = pToolBox->GetItemId( _nPos );
    VCLXAccessibleToolBoxItem* pFocusItem = nullptr;

    for ( const auto& [rPos, rxChild] : m_aAccessibleChildren )
    {
            ToolBoxItemId nItemId = pToolBox->GetItemId( rPos );

            VCLXAccessibleToolBoxItem* pItem = rxChild.get();
            pItem->SetChecked( pToolBox->IsItemChecked( nItemId ) );
            if ( nItemId == nFocusId )
                pFocusItem = pItem;
    }
    //Solution:If the position is not a child item,the focus should not be called
    if ( pFocusItem && _nPos != ToolBox::ITEM_NOTFOUND )
        pFocusItem->SetFocus( true );
}

void VCLXAccessibleToolBox::UpdateIndeterminate_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    ToolBoxItemId nItemId = pToolBox->GetItemId( _nPos );

    ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
    if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
    {
        VCLXAccessibleToolBoxItem* pItem = aIter->second.get();
        if ( pItem )
            pItem->SetIndeterminate( pToolBox->GetItemState( nItemId ) == TRISTATE_INDET );
    }
}

void VCLXAccessibleToolBox::implReleaseToolboxItem( ToolBoxItemsMap::iterator const & _rMapPos,
        bool _bNotifyRemoval )
{
    rtl::Reference<VCLXAccessibleToolBoxItem> xItemAcc(_rMapPos->second);
    if ( !xItemAcc.is() )
        return;

    if ( _bNotifyRemoval )
    {
        NotifyAccessibleEvent(AccessibleEventId::CHILD, Any(Reference<XAccessible>(xItemAcc)), Any());
    }

    xItemAcc->ReleaseToolBox();
    xItemAcc->dispose();
}

void VCLXAccessibleToolBox::UpdateItem_Impl( ToolBox::ImplToolItems::size_type _nPos)
{
    if ( _nPos < m_aAccessibleChildren.size() )
    {
        UpdateAllItems_Impl();
        return;
    }

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    // adjust the "index-in-parent"s
    ToolBoxItemsMap::iterator aIndexAdjust = m_aAccessibleChildren.upper_bound( _nPos );
        //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
    while ( m_aAccessibleChildren.end() != aIndexAdjust )
    {
        rtl::Reference<VCLXAccessibleToolBoxItem> xItem(aIndexAdjust->second);
        if (xItem.is())
        {
            sal_Int32 nIndex = xItem->getIndexInParent();
            nIndex++;
            xItem->setIndexInParent(nIndex);
        }

        ++aIndexAdjust;
    }

    // TODO: we should make this dependent on the existence of event listeners
    // with the current implementation, we always create accessible object
    Any aNewChild( getAccessibleChild( static_cast<sal_Int64>(_nPos) ) );
        //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
    NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewChild );
}

void VCLXAccessibleToolBox::UpdateAllItems_Impl()
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    // deregister the old items
    for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
            aIter != m_aAccessibleChildren.end(); ++aIter )
    {
        implReleaseToolboxItem( aIter, true );
    }
    m_aAccessibleChildren.clear();

    // register the new items
    ToolBox::ImplToolItems::size_type i, nCount = pToolBox->GetItemCount();
    for ( i = 0; i < nCount; ++i )
    {
        Any aNewValue;
        aNewValue <<= getAccessibleChild(i);
        NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewValue );
    }
}

void VCLXAccessibleToolBox::UpdateCustomPopupItemp_Impl( vcl::Window* pWindow, bool bOpen )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if( !(pWindow && pToolBox) )
        return;

    const ToolBoxItemId nDownItem = pToolBox->GetDownItemId();
    if ( !nDownItem )
        // No item is currently in down state.
        // Moreover, calling GetItemPos with 0 will find a separator if there is any.
        return;

    Reference< XAccessible > xChild( pWindow->GetAccessible() );
    if( xChild.is() )
    {
        Reference< XAccessible > xChildItem( getAccessibleChild(pToolBox->GetItemPos(nDownItem)));
        VCLXAccessibleToolBoxItem* pItem = static_cast< VCLXAccessibleToolBoxItem* >( xChildItem.get() );

        pItem->SetChild( xChild );
        pItem->NotifyChildEvent( xChild, bOpen );
    }
}

void VCLXAccessibleToolBox::UpdateItemName_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VCLXAccessibleToolBoxItem* pItem = GetItem_Impl( _nPos );
    if ( pItem )
        pItem->NameChanged();
}

void VCLXAccessibleToolBox::UpdateItemEnabled_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VCLXAccessibleToolBoxItem* pItem = GetItem_Impl( _nPos );
    if ( pItem )
        pItem->ToggleEnableState();
}

void VCLXAccessibleToolBox::HandleSubToolBarEvent( const VclWindowEvent& rVclWindowEvent )
{
    vcl::Window* pChildWindow = static_cast<vcl::Window *>(rVclWindowEvent.GetData());
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !(pChildWindow
        && pToolBox
        && pToolBox == pChildWindow->GetParent()
        && pChildWindow->GetType() == WindowType::TOOLBOX) )
        return;

    const ToolBoxItemId nCurItemId( pToolBox->GetCurItemId() );
    if ( !nCurItemId )
        // No item is currently active (might happen when opening the overflow popup).
        // Moreover, calling GetItemPos with 0 will find a separator if there is any.
        return;

    ToolBox::ImplToolItems::size_type nIndex = pToolBox->GetItemPos( nCurItemId );
    Reference< XAccessible > xItem = getAccessibleChild( nIndex );
    if ( xItem.is() )
    {
        Reference< XAccessible > xChild = pChildWindow->GetAccessible();
        VCLXAccessibleToolBoxItem* pItem =
            static_cast< VCLXAccessibleToolBoxItem* >( xItem.get() );
        pItem->SetChild( xChild );
        pItem->NotifyChildEvent( xChild, true/*_bShow*/ );
    }
}

void VCLXAccessibleToolBox::ReleaseSubToolBox( ToolBox* _pSubToolBox )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    ToolBox::ImplToolItems::size_type nIndex = pToolBox->GetItemPos( pToolBox->GetCurItemId() );
    if ( nIndex == ToolBox::ITEM_NOTFOUND )
        return; // not found

    Reference< XAccessible > xItem = getAccessibleChild( nIndex );
    if ( !xItem.is() )
        return;

    Reference< XAccessible > xChild = _pSubToolBox->GetAccessible();
    VCLXAccessibleToolBoxItem* pItem =
        static_cast< VCLXAccessibleToolBoxItem* >( xItem.get() );
    if ( pItem->GetChild() == xChild )
    {
        pItem->SetChild( Reference< XAccessible >() );
        pItem->NotifyChildEvent( xChild, false );
    }
}

void VCLXAccessibleToolBox::FillAccessibleStateSet( sal_Int64& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        rStateSet |= AccessibleStateType::FOCUSABLE;
        if ( pToolBox->IsHorizontal() )
            rStateSet |= AccessibleStateType::HORIZONTAL;
        else
            rStateSet |= AccessibleStateType::VERTICAL;
    }
}

void VCLXAccessibleToolBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    // to prevent an early release of the toolbox (VclEventId::ObjectDying)
    Reference< XAccessibleContext > xHoldAlive = this;

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ToolboxClick:
        case VclEventId::ToolboxSelect:
        {
            VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
                UpdateIndeterminate_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            }
            else if( pToolBox->GetItemPos(pToolBox->GetCurItemId()) != ToolBox::ITEM_NOTFOUND )
            {
                UpdateChecked_Impl( pToolBox->GetItemPos(pToolBox->GetCurItemId()) );
                UpdateIndeterminate_Impl( pToolBox->GetItemPos(pToolBox->GetCurItemId()) );
            }
            break;
        }
        case VclEventId::ToolboxDoubleClick:
        case VclEventId::ToolboxActivate:
        case VclEventId::ToolboxDeactivate:
        //case VclEventId::ToolboxSelect:
            break;

        case VclEventId::ToolboxItemUpdated:
        {
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( ToolBox::ITEM_NOTFOUND );
                UpdateIndeterminate_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            }
            break;
        }

        case VclEventId::ToolboxHighlight:
            UpdateFocus_Impl();
            break;

        case VclEventId::ToolboxHighlightOff:
            ReleaseFocus_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            break;

        case VclEventId::ToolboxItemAdded :
            UpdateItem_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            break;

        case VclEventId::ToolboxItemRemoved :
        case VclEventId::ToolboxAllItemsChanged :
        {
            UpdateAllItems_Impl();
            break;
        }

        case VclEventId::ToolboxItemWindowChanged:
        {
            auto nPos = static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
            ToolBoxItemsMap::iterator aAccessiblePos( m_aAccessibleChildren.find( nPos ) );
                //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
            if ( m_aAccessibleChildren.end() != aAccessiblePos )
            {
                implReleaseToolboxItem( aAccessiblePos, false );
                m_aAccessibleChildren.erase (aAccessiblePos);
            }

            Any aNewValue;
            aNewValue <<= getAccessibleChild(nPos);
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewValue );
            break;
        }
        case VclEventId::ToolboxItemTextChanged :
            UpdateItemName_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            break;

        case VclEventId::ToolboxItemEnabled :
        case VclEventId::ToolboxItemDisabled :
        {
            UpdateItemEnabled_Impl( static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData())) );
            break;
        }

        case VclEventId::DropdownOpen:
        case VclEventId::DropdownClose:
        {
            UpdateCustomPopupItemp_Impl( static_cast< vcl::Window* >( rVclWindowEvent.GetData() ), rVclWindowEvent.GetId() == VclEventId::DropdownOpen );
            break;
        }

        case VclEventId::ObjectDying :
        {
            // if this toolbox is a subtoolbox, we have to release it from its parent
            VclPtr< vcl::Window > pWin = GetAs< vcl::Window >();
            if ( pWin && pWin->GetParent() &&
                 pWin->GetParent()->GetType() == WindowType::TOOLBOX )
            {
                auto pParentAccContext = pWin->GetParent()->GetAccessible()->getAccessibleContext();
                VCLXAccessibleToolBox* pParent = static_cast< VCLXAccessibleToolBox* >( pParentAccContext.get() );
                if ( pParent )
                    pParent->ReleaseSubToolBox(static_cast<ToolBox *>(pWin.get()));
            }

            // dispose all items
            for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != m_aAccessibleChildren.end(); ++aIter )
            {
                implReleaseToolboxItem( aIter, false );
            }
            m_aAccessibleChildren.clear();

            [[fallthrough]]; // call base class
        }

        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

void VCLXAccessibleToolBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::WindowShow:  // send create on show for direct accessible children
        {
            Reference< XAccessible > xReturn = GetItemWindowAccessible(rVclWindowEvent);
            if ( xReturn.is() )
                NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), Any(xReturn) );
            else
                HandleSubToolBarEvent( rVclWindowEvent );
        }
        break;

        default:
           VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );

    }
}

// XComponent
void SAL_CALL VCLXAccessibleToolBox::disposing()
{
    VCLXAccessibleComponent::disposing();

    // release the items
    for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
          aIter != m_aAccessibleChildren.end(); ++aIter )
    {
        implReleaseToolboxItem( aIter, false );
    }
    m_aAccessibleChildren.clear();
}

// XServiceInfo
OUString VCLXAccessibleToolBox::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleToolBox"_ustr;
}

Sequence< OUString > VCLXAccessibleToolBox::getSupportedServiceNames()
{
    return comphelper::concatSequences(VCLXAccessibleComponent::getSupportedServiceNames(),
                                       Sequence<OUString>{u"com.sun.star.accessibility.AccessibleToolBox"_ustr});
}

// XAccessibleContext
sal_Int64 SAL_CALL VCLXAccessibleToolBox::getAccessibleChildCount(  )
{
    comphelper::OExternalLockGuard aGuard( this );
    return implGetAccessibleChildCount();
}

 sal_Int64 VCLXAccessibleToolBox::implGetAccessibleChildCount(  )
 {
    sal_Int64 nCount = 0;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
        nCount = pToolBox->GetItemCount();

    return nCount;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleChild( sal_Int64 i )
{
    comphelper::OExternalLockGuard aGuard( this );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || i < 0 || o3tl::make_unsigned(i) >= pToolBox->GetItemCount() )
        throw IndexOutOfBoundsException();

    rtl::Reference< VCLXAccessibleToolBoxItem > xChild;
    // search for the child
    ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find(i);
    if ( m_aAccessibleChildren.end() == aIter )
    {
        ToolBoxItemId nItemId = pToolBox->GetItemId( i );
        ToolBoxItemId nHighlightItemId = pToolBox->GetHighlightItemId();
        vcl::Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
        // not found -> create a new child
        xChild = new VCLXAccessibleToolBoxItem( pToolBox, i );
        if ( pItemWindow )
        {
            Reference< XAccessible> xParent = xChild;
            auto const xInnerAcc(pItemWindow->GetAccessible());
            if (xInnerAcc) // else child is being disposed - avoid crashing
            {
                rtl::Reference<OToolBoxWindowItem> xChild2(new OToolBoxWindowItem(0,
                    ::comphelper::getProcessComponentContext(), xInnerAcc, xParent));
                pItemWindow->SetAccessible(xChild2);
                xChild->SetChild( xChild2 );
            }
        }
        if ( nHighlightItemId > ToolBoxItemId(0) && nItemId == nHighlightItemId )
            xChild->SetFocus( true );
        if ( pToolBox->IsItemChecked( nItemId ) )
            xChild->SetChecked( true );
        if ( pToolBox->GetItemState( nItemId ) == TRISTATE_INDET )
            xChild->SetIndeterminate( true );
        m_aAccessibleChildren.emplace( i, xChild );
    }
    else
    {
        // found it
        xChild = aIter->second;
    }
    return xChild;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleAtPoint( const awt::Point& _rPoint )
{
    comphelper::OExternalLockGuard aGuard( this );

    Reference< XAccessible > xAccessible;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        ToolBox::ImplToolItems::size_type nItemPos = pToolBox->GetItemPos( VCLPoint( _rPoint ) );
        if ( nItemPos != ToolBox::ITEM_NOTFOUND )
            xAccessible = getAccessibleChild( nItemPos );
    }

    return xAccessible;
}

Reference< XAccessible > VCLXAccessibleToolBox::GetItemWindowAccessible( const VclWindowEvent& rVclWindowEvent )
{
    Reference< XAccessible > xReturn;
    vcl::Window* pChildWindow = static_cast<vcl::Window *>(rVclWindowEvent.GetData());
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pChildWindow && pToolBox )
    {
        ToolBox::ImplToolItems::size_type nCount = pToolBox->GetItemCount();
        for (ToolBox::ImplToolItems::size_type i = 0 ; i < nCount && !xReturn.is() ; ++i)
        {
            ToolBoxItemId nItemId = pToolBox->GetItemId( i );
            vcl::Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
            if ( pItemWindow == pChildWindow )
                xReturn = getAccessibleChild(i);
        }
    }
    return xReturn;
}

Reference< XAccessible > VCLXAccessibleToolBox::GetChildAccessible( const VclWindowEvent& rVclWindowEvent )
{
    Reference< XAccessible > xReturn = GetItemWindowAccessible(rVclWindowEvent);

    if ( !xReturn.is() )
        xReturn = VCLXAccessibleComponent::GetChildAccessible(rVclWindowEvent);
    return xReturn;
}

// XAccessibleSelection
void VCLXAccessibleToolBox::selectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || nChildIndex < 0 || o3tl::make_unsigned(nChildIndex) >= pToolBox->GetItemCount() )
        throw IndexOutOfBoundsException();

    pToolBox->ChangeHighlight( nChildIndex );
}

sal_Bool VCLXAccessibleToolBox::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || nChildIndex < 0 || o3tl::make_unsigned(nChildIndex) >= pToolBox->GetItemCount() )
        throw IndexOutOfBoundsException();

    if ( pToolBox->GetHighlightItemId() == pToolBox->GetItemId( nChildIndex ) )
        return true;
    else
        return false;
}

void VCLXAccessibleToolBox::clearAccessibleSelection(  )
{
    OExternalLockGuard aGuard( this );
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    pToolBox -> LoseFocus();
}

void VCLXAccessibleToolBox::selectAllAccessibleChildren(  )
{
    OExternalLockGuard aGuard( this );
    // intentionally empty. makes no sense for a toolbox
}

sal_Int64 VCLXAccessibleToolBox::getSelectedAccessibleChildCount(  )
{
    OExternalLockGuard aGuard( this );

    sal_Int64 nRet = 0;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if (pToolBox)
    {
        ToolBoxItemId nHighlightItemId = pToolBox->GetHighlightItemId();
        for ( size_t i = 0, nCount = pToolBox->GetItemCount(); i < nCount; i++ )
        {
            if ( nHighlightItemId == pToolBox->GetItemId( i ) )
            {
                nRet = 1;
                break; // a toolbox can only have (n)one selected child
            }
        }
    }

    return nRet;
}

Reference< XAccessible > VCLXAccessibleToolBox::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    OExternalLockGuard aGuard( this );
    if ( nSelectedChildIndex != 0 )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if (pToolBox)
    {
        ToolBoxItemId nHighlightItemId = pToolBox->GetHighlightItemId();
        for (ToolBox::ImplToolItems::size_type i = 0, nCount = pToolBox->GetItemCount(); i < nCount; i++ )
        {
            if ( nHighlightItemId == pToolBox->GetItemId( i ) )
            {
                xChild = getAccessibleChild( i );
                break;
            }
        }
    }

    if (!xChild)
        throw IndexOutOfBoundsException();

    return xChild;
}

void VCLXAccessibleToolBox::deselectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= implGetAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    clearAccessibleSelection(); // a toolbox can only have (n)one selected child
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
