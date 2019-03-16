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

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/toolbox.hxx>
#include <vcl/vclevent.hxx>
#include <comphelper/accessiblewrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/typeprovider.hxx>

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
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    };


    sal_Int32 SAL_CALL OToolBoxWindowItemContext::getAccessibleIndexInParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nIndexInParent;
    }


    // = OToolBoxWindowItem

    typedef ::cppu::ImplHelper1 <   XUnoTunnel
                                >   OToolBoxWindowItem_Base;

    /** XAccessible implementation for a toolbox item which is represented by a VCL Window
    */
    class OToolBoxWindowItem
            :public OAccessibleWrapper
            ,public OToolBoxWindowItem_Base
    {
    private:
        sal_Int32 m_nIndexInParent;

    public:
        sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
        void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

        static  bool    isWindowItem( const Reference< XAccessible >& _rxAcc, OToolBoxWindowItem** /* [out] */ _ppImplementation );

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
        // XInterface
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        // OAccessibleWrapper
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const css::uno::Reference< css::accessibility::XAccessibleContext >& _rxInnerContext
            ) override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const Sequence< sal_Int8 >& aIdentifier ) override;
        static Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };

    IMPLEMENT_FORWARD_XINTERFACE2( OToolBoxWindowItem, OAccessibleWrapper, OToolBoxWindowItem_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OToolBoxWindowItem, OAccessibleWrapper, OToolBoxWindowItem_Base )

    OAccessibleContextWrapper* OToolBoxWindowItem::createAccessibleContext(
            const Reference< XAccessibleContext >& _rxInnerContext )
    {
        return new OToolBoxWindowItemContext( m_nIndexInParent, getComponentContext(), _rxInnerContext, this, getParent() );
    }

    bool OToolBoxWindowItem::isWindowItem( const Reference< XAccessible >& _rxAcc, OToolBoxWindowItem** /* [out] */ _ppImplementation )
    {
        OToolBoxWindowItem* pImplementation = nullptr;

        Reference< XUnoTunnel > xTunnel( _rxAcc, UNO_QUERY );
        if ( xTunnel.is() )
            pImplementation = reinterpret_cast< OToolBoxWindowItem* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

        if ( _ppImplementation )
            *_ppImplementation = pImplementation;

        return pImplementation != nullptr;
    }

    Sequence< sal_Int8 > OToolBoxWindowItem::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId implId;

        return implId.getImplementationId();
    }

    sal_Int64 SAL_CALL OToolBoxWindowItem::getSomething( const Sequence< sal_Int8 >& _rId )
    {
        if  (   ( _rId.getLength() == 16 )
            &&  ( memcmp( getUnoTunnelImplementationId().getConstArray(),  _rId.getConstArray(), 16 ) == 0 )
            )
            return reinterpret_cast< sal_Int64>( this );

        return 0;
    }
}

// VCLXAccessibleToolBox

VCLXAccessibleToolBox::VCLXAccessibleToolBox( VCLXWindow* pVCLXWindow ) :

    VCLXAccessibleComponent( pVCLXWindow )

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
            pItem = static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
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

    if ( bHasFocus )
    {
        sal_uInt16 nHighlightItemId = pToolBox->GetHighlightItemId();
        sal_uInt16 nFocusCount = 0;
        for ( const auto& [rPos, rxChild] : m_aAccessibleChildren )
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( rPos );

            if ( rxChild.is() )
            {
                VCLXAccessibleToolBoxItem* pItem =
                    static_cast< VCLXAccessibleToolBoxItem* >( rxChild.get() );
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
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem->HasFocus() )
                pItem->SetFocus( false );
        }
    }
}

void VCLXAccessibleToolBox::UpdateChecked_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        sal_uInt16 nFocusId = pToolBox->GetItemId( _nPos );
        VCLXAccessibleToolBoxItem* pFocusItem = nullptr;

        for ( const auto& [rPos, rxChild] : m_aAccessibleChildren )
        {
                sal_uInt16 nItemId = pToolBox->GetItemId( rPos );

                VCLXAccessibleToolBoxItem* pItem =
                    static_cast< VCLXAccessibleToolBoxItem* >( rxChild.get() );
                pItem->SetChecked( pToolBox->IsItemChecked( nItemId ) );
                if ( nItemId == nFocusId )
                    pFocusItem = pItem;
        }
        //Solution:If the position is not a child item,the focus should not be called
        if ( pFocusItem && _nPos != ToolBox::ITEM_NOTFOUND )
            pFocusItem->SetFocus( true );
    }
}

void VCLXAccessibleToolBox::UpdateIndeterminate_Impl( ToolBox::ImplToolItems::size_type _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        sal_uInt16 nItemId = pToolBox->GetItemId( _nPos );

        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem )
                pItem->SetIndeterminate( pToolBox->GetItemState( nItemId ) == TRISTATE_INDET );
        }
    }
}

void VCLXAccessibleToolBox::implReleaseToolboxItem( ToolBoxItemsMap::iterator const & _rMapPos,
        bool _bNotifyRemoval )
{
    Reference< XAccessible > xItemAcc( _rMapPos->second );
    if ( !xItemAcc.is() )
        return;

    if ( _bNotifyRemoval )
    {
        NotifyAccessibleEvent( AccessibleEventId::CHILD, Any( xItemAcc ), Any() );
    }

    OToolBoxWindowItem* pWindowItem = nullptr;
    if ( !OToolBoxWindowItem::isWindowItem( xItemAcc, &pWindowItem ) )
    {
        static_cast< VCLXAccessibleToolBoxItem* >( xItemAcc.get() )->ReleaseToolBox();
        ::comphelper::disposeComponent( xItemAcc );
    }
    else
    {
        if ( pWindowItem )
        {
            Reference< XAccessibleContext > xContext( pWindowItem->getContextNoCreate() );
            ::comphelper::disposeComponent( xContext );
        }
    }
}

void VCLXAccessibleToolBox::UpdateItem_Impl( ToolBox::ImplToolItems::size_type _nPos)
{
    if ( _nPos < m_aAccessibleChildren.size() )
    {
        UpdateAllItems_Impl();
        return;
    }

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        // adjust the "index-in-parent"s
        ToolBoxItemsMap::iterator aIndexAdjust = m_aAccessibleChildren.upper_bound( _nPos );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        while ( m_aAccessibleChildren.end() != aIndexAdjust )
        {
            Reference< XAccessible > xItemAcc( aIndexAdjust->second );

            OToolBoxWindowItem* pWindowItem = nullptr;
            if ( !OToolBoxWindowItem::isWindowItem( xItemAcc, &pWindowItem ) )
            {
                VCLXAccessibleToolBoxItem* pItem = static_cast< VCLXAccessibleToolBoxItem* >( xItemAcc.get() );
                if ( pItem )
                {
                    sal_Int32 nIndex = pItem->getIndexInParent( );
                    nIndex++;
                    pItem->setIndexInParent( nIndex );
                }
            }
            else
            {
                if ( pWindowItem )
                {
                    sal_Int32 nIndex = pWindowItem->getIndexInParent( );
                    nIndex++;
                    pWindowItem->setIndexInParent( nIndex );
                }
            }

            ++aIndexAdjust;
        }

        // TODO: we should make this dependent on the existence of event listeners
        // with the current implementation, we always create accessible object
        Any aNewChild = Any( getAccessibleChild( static_cast<sal_Int32>(_nPos) ) );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewChild );
    }
}

void VCLXAccessibleToolBox::UpdateAllItems_Impl()
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
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
            aNewValue <<= getAccessibleChild( static_cast<sal_Int32>(i) );
                //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewValue );
        }
    }
}

void VCLXAccessibleToolBox::UpdateCustomPopupItemp_Impl( vcl::Window* pWindow, bool bOpen )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if( pWindow && pToolBox )
    {
        const sal_uInt16 nDownItem = pToolBox->GetDownItemId();
        if ( !nDownItem )
            // No item is currently in down state.
            // Moreover, calling GetItemPos with 0 will find a separator if there is any.
            return;

        Reference< XAccessible > xChild( pWindow->GetAccessible() );
        if( xChild.is() )
        {
            Reference< XAccessible > xChildItem( getAccessibleChild( static_cast< sal_Int32 >( pToolBox->GetItemPos( nDownItem ) ) ) );
                //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
            VCLXAccessibleToolBoxItem* pItem = static_cast< VCLXAccessibleToolBoxItem* >( xChildItem.get() );

            pItem->SetChild( xChild );
            pItem->NotifyChildEvent( xChild, bOpen );
        }
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
    if ( pChildWindow
        && pToolBox
        && pToolBox == pChildWindow->GetParent()
        && pChildWindow->GetType() == WindowType::TOOLBOX )
    {
        const sal_uInt16 nCurItemId( pToolBox->GetCurItemId() );
        if ( !nCurItemId )
            // No item is currently active (might happen when opening the overflow popup).
            // Moreover, calling GetItemPos with 0 will find a separator if there is any.
            return;

        ToolBox::ImplToolItems::size_type nIndex = pToolBox->GetItemPos( nCurItemId );
        Reference< XAccessible > xItem = getAccessibleChild( nIndex );
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
        if ( xItem.is() )
        {
            Reference< XAccessible > xChild = pChildWindow->GetAccessible();
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( xItem.get() );
            pItem->SetChild( xChild );
            pItem->NotifyChildEvent( xChild, true/*_bShow*/ );
        }
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
        //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
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

void VCLXAccessibleToolBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        if ( pToolBox->IsHorizontal() )
            rStateSet.AddState( AccessibleStateType::HORIZONTAL );
        else
            rStateSet.AddState( AccessibleStateType::VERTICAL );
    }
}

void VCLXAccessibleToolBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    // to prevent an early release of the toolbox (VclEventId::ObjectDying)
    Reference< XAccessibleContext > xTemp = this;

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
                //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
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
                VCLXAccessibleToolBox* pParent = static_cast< VCLXAccessibleToolBox* >(
                    pWin->GetParent()->GetAccessible()->getAccessibleContext().get() );
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

// XInterface
IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleToolBox, VCLXAccessibleComponent, VCLXAccessibleToolBox_BASE )

// XTypeProvider
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleToolBox, VCLXAccessibleComponent, VCLXAccessibleToolBox_BASE )

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
    return OUString( "com.sun.star.comp.toolkit.AccessibleToolBox" );
}

Sequence< OUString > VCLXAccessibleToolBox::getSupportedServiceNames()
{
    Sequence< OUString > aNames = VCLXAccessibleComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleToolBox";
    return aNames;
}

// XAccessibleContext
sal_Int32 SAL_CALL VCLXAccessibleToolBox::getAccessibleChildCount(  )
{
    comphelper::OExternalLockGuard aGuard( this );
    return implGetAccessibleChildCount();
}

 sal_Int32 VCLXAccessibleToolBox::implGetAccessibleChildCount(  )
 {
    sal_Int32 nCount = 0;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
        nCount = pToolBox->GetItemCount();
            //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!

    return nCount;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleChild( sal_Int32 i )
{
    comphelper::OExternalLockGuard aGuard( this );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || i < 0 || static_cast<size_t>(i) >= pToolBox->GetItemCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    // search for the child
    ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find(i);
    if ( m_aAccessibleChildren.end() == aIter )
    {
        sal_uInt16 nItemId = pToolBox->GetItemId( i );
        sal_uInt16 nHighlightItemId = pToolBox->GetHighlightItemId();
        vcl::Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
        // not found -> create a new child
        VCLXAccessibleToolBoxItem* pChild = new VCLXAccessibleToolBoxItem( pToolBox, i );
        Reference< XAccessible> xParent = pChild;
        if ( pItemWindow )
        {
            xChild = new OToolBoxWindowItem(0,::comphelper::getProcessComponentContext(),pItemWindow->GetAccessible(),xParent);
            pItemWindow->SetAccessible(xChild);
            pChild->SetChild( xChild );
        }
        xChild = pChild;
        if ( nHighlightItemId > 0 && nItemId == nHighlightItemId )
            pChild->SetFocus( true );
        if ( pToolBox->IsItemChecked( nItemId ) )
            pChild->SetChecked( true );
        if ( pToolBox->GetItemState( nItemId ) == TRISTATE_INDET )
            pChild->SetIndeterminate( true );
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
                //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
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
            sal_uInt16 nItemId = pToolBox->GetItemId( i );
            vcl::Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
            if ( pItemWindow == pChildWindow )
                xReturn = getAccessibleChild(i);
                    //TODO: ToolBox::ImplToolItems::size_type -> sal_Int32!
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
void VCLXAccessibleToolBox::selectAccessibleChild( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || nChildIndex < 0 || static_cast<size_t> (nChildIndex) >= pToolBox->GetItemCount() )
        throw IndexOutOfBoundsException();

    pToolBox->ChangeHighlight( nChildIndex );
}

sal_Bool VCLXAccessibleToolBox::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( (!pToolBox) || nChildIndex < 0 || static_cast<size_t>(nChildIndex) >= pToolBox->GetItemCount() )
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

sal_Int32 VCLXAccessibleToolBox::getSelectedAccessibleChildCount(  )
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nRet = 0;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if (pToolBox)
    {
        sal_uInt16 nHighlightItemId = pToolBox->GetHighlightItemId();
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

Reference< XAccessible > VCLXAccessibleToolBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    OExternalLockGuard aGuard( this );
    if ( nSelectedChildIndex != 0 )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if (pToolBox)
    {
        sal_uInt16 nHighlightItemId = pToolBox->GetHighlightItemId();
        for ( sal_Int32 i = 0, nCount = pToolBox->GetItemCount(); i < nCount; i++ )
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

void VCLXAccessibleToolBox::deselectAccessibleChild( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= implGetAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    clearAccessibleSelection(); // a toolbox can only have (n)one selected child
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
