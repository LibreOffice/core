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

#include <accessibility/standard/vclxaccessibletoolbox.hxx>
#include <accessibility/standard/vclxaccessibletoolboxitem.hxx>
#include <toolkit/helper/convert.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/toolbox.hxx>
#include <comphelper/accessiblewrapper.hxx>
#include <comphelper/processfactory.hxx>

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
    class OToolBoxWindowItemContext : public OAccessibleContextWrapper
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
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    };


    sal_Int32 SAL_CALL OToolBoxWindowItemContext::getAccessibleIndexInParent(  ) throw (css::uno::RuntimeException, std::exception)
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
        inline sal_Int32    getIndexInParent() const                    { return m_nIndexInParent; }
        inline void         setIndexInParent( sal_Int32 _nNewIndex )    { m_nIndexInParent = _nNewIndex; }

        static  bool    isWindowItem( const Reference< XAccessible >& _rxAcc, OToolBoxWindowItem** /* [out] */ _ppImplementation = nullptr );

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
        virtual sal_Int64 SAL_CALL getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw (RuntimeException, std::exception) override;
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

        return nullptr != pImplementation;
    }

    Sequence< sal_Int8 > OToolBoxWindowItem::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId * pId = nullptr;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    sal_Int64 SAL_CALL OToolBoxWindowItem::getSomething( const Sequence< sal_Int8 >& _rId ) throw (RuntimeException, std::exception)
    {
        if  (   ( 16 == _rId.getLength() )
            &&  ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(),  _rId.getConstArray(), 16 ) )
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

VCLXAccessibleToolBoxItem* VCLXAccessibleToolBox::GetItem_Impl( sal_Int32 _nPos, bool _bMustHaveFocus )
{
    VCLXAccessibleToolBoxItem* pItem = nullptr;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox && ( !_bMustHaveFocus || pToolBox->HasFocus() ) )
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
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
        for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
              aIter != m_aAccessibleChildren.end(); ++aIter )
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)aIter->first );

            if ( aIter->second.is() )
            {
                VCLXAccessibleToolBoxItem* pItem =
                    static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
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

void VCLXAccessibleToolBox::ReleaseFocus_Impl( sal_Int32 _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox ) // #107124#, do not check for focus because this message is also handled in losefocus
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem->HasFocus() )
                pItem->SetFocus( false );
        }
    }
}

void VCLXAccessibleToolBox::UpdateChecked_Impl( sal_Int32 _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        sal_uInt16 nFocusId = pToolBox->GetItemId( (sal_uInt16)_nPos );
        VCLXAccessibleToolBoxItem* pFocusItem = nullptr;

        for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
              aIter != m_aAccessibleChildren.end(); ++aIter )
        {
                sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)aIter->first );

                VCLXAccessibleToolBoxItem* pItem =
                    static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
                pItem->SetChecked( pToolBox->IsItemChecked( nItemId ) );
                if ( nItemId == nFocusId )
                    pFocusItem = pItem;
        }
        //Solution:If the position is not a child item,the focus should not be called
        if ( pFocusItem && (sal_uInt16)_nPos != TOOLBOX_ITEM_NOTFOUND )
            pFocusItem->SetFocus( true );
    }
}

void VCLXAccessibleToolBox::UpdateIndeterminate_Impl( sal_Int32 _nPos )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)_nPos );

        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem )
                pItem->SetIndeterminate( pToolBox->GetItemState( nItemId ) == TRISTATE_INDET );
        }
    }
}

void VCLXAccessibleToolBox::implReleaseToolboxItem( ToolBoxItemsMap::iterator& _rMapPos,
        bool _bNotifyRemoval, bool _bDispose )
{
    Reference< XAccessible > xItemAcc( _rMapPos->second );
    if ( !xItemAcc.is() )
        return;

    if ( _bNotifyRemoval )
    {
        NotifyAccessibleEvent( AccessibleEventId::CHILD, makeAny( xItemAcc ), Any() );
    }

    OToolBoxWindowItem* pWindowItem = nullptr;
    if ( !OToolBoxWindowItem::isWindowItem( xItemAcc, &pWindowItem ) )
    {
        static_cast< VCLXAccessibleToolBoxItem* >( xItemAcc.get() )->ReleaseToolBox();
        if ( _bDispose )
            ::comphelper::disposeComponent( xItemAcc );
    }
    else
    {
        if ( _bDispose )
        {
            if ( pWindowItem )
            {
                Reference< XAccessibleContext > xContext( pWindowItem->getContextNoCreate() );
                ::comphelper::disposeComponent( xContext );
            }
        }
    }
}

void VCLXAccessibleToolBox::UpdateItem_Impl( sal_Int32 _nPos, bool _bItemAdded )
{
    if ( _nPos < sal_Int32( m_aAccessibleChildren.size() ) )
    {
        UpdateAllItems_Impl();
        return;
    }

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        if ( !_bItemAdded )
        {   // the item was removed
            // -> destroy the old item
            ToolBoxItemsMap::iterator aItemPos = m_aAccessibleChildren.find( _nPos );
            if ( m_aAccessibleChildren.end() != aItemPos )
            {
                implReleaseToolboxItem( aItemPos, true, true );
                m_aAccessibleChildren.erase( aItemPos );
            }
        }

        // adjust the "index-in-parent"s
        ToolBoxItemsMap::iterator aIndexAdjust = m_aAccessibleChildren.upper_bound( _nPos );
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
                    nIndex += (_bItemAdded ? +1 : -1);
                    pItem->setIndexInParent( nIndex );
                }
            }
            else
            {
                if ( pWindowItem )
                {
                    sal_Int32 nIndex = pWindowItem->getIndexInParent( );
                    nIndex += (_bItemAdded ? +1 : -1);
                    pWindowItem->setIndexInParent( nIndex );
                }
            }

            ++aIndexAdjust;
        }

        if ( _bItemAdded )
        {
            // TODO: we should make this dependent on the existence of event listeners
            // with the current implementation, we always create accessible object
            Any aNewChild = makeAny( getAccessibleChild( (sal_Int32)_nPos ) );
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewChild );
        }
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
            implReleaseToolboxItem( aIter, true, true );
        }
        m_aAccessibleChildren.clear();

        // register the new items
        sal_uInt16 i, nCount = pToolBox->GetItemCount();
        for ( i = 0; i < nCount; ++i )
        {
            Any aNewValue;
            aNewValue <<= getAccessibleChild( (sal_Int32)i );
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
            // Items with ItemId == 0 are not allowed in ToolBox, which means that currently no item is in down state.
            // Moreover, running GetItemPos with 0 could find a separator item if there is any.
            return;

        Reference< XAccessible > xChild( pWindow->GetAccessible() );
        if( xChild.is() )
        {
            Reference< XAccessible > xChildItem( getAccessibleChild( static_cast< sal_Int32 >( pToolBox->GetItemPos( nDownItem ) ) ) );
            VCLXAccessibleToolBoxItem* pItem = static_cast< VCLXAccessibleToolBoxItem* >( xChildItem.get() );

            pItem->SetChild( xChild );
            pItem->NotifyChildEvent( xChild, bOpen );
        }
    }
}

void VCLXAccessibleToolBox::UpdateItemName_Impl( sal_Int32 _nPos )
{
    VCLXAccessibleToolBoxItem* pItem = GetItem_Impl( _nPos, false );
    if ( pItem )
        pItem->NameChanged();
}

void VCLXAccessibleToolBox::UpdateItemEnabled_Impl( sal_Int32 _nPos )
{
    VCLXAccessibleToolBoxItem* pItem = GetItem_Impl( _nPos, false );
    if ( pItem )
        pItem->ToggleEnableState();
}

void VCLXAccessibleToolBox::HandleSubToolBarEvent( const VclWindowEvent& rVclWindowEvent, bool _bShow )
{
    vcl::Window* pChildWindow = static_cast<vcl::Window *>(rVclWindowEvent.GetData());
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pChildWindow
        && pToolBox
        && pToolBox == pChildWindow->GetParent()
        && pChildWindow->GetType() == WINDOW_TOOLBOX )
    {
        sal_Int32 nIndex = pToolBox->GetItemPos( pToolBox->GetCurItemId() );
        Reference< XAccessible > xItem = getAccessibleChild( nIndex );
        if ( xItem.is() )
        {
            Reference< XAccessible > xChild = pChildWindow->GetAccessible();
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( xItem.get() );
            pItem->SetChild( xChild );
            pItem->NotifyChildEvent( xChild, _bShow );
        }
    }
}

void VCLXAccessibleToolBox::ReleaseSubToolBox( ToolBox* _pSubToolBox )
{
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( !pToolBox )
        return;

    sal_Int32 nIndex = pToolBox->GetItemPos( pToolBox->GetCurItemId() );
    if ( nIndex == SAL_MAX_UINT16 )
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
    // to prevent an early release of the toolbox (VCLEVENT_OBJECT_DYING)
    Reference< XAccessibleContext > xTemp = this;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TOOLBOX_CLICK:
        case VCLEVENT_TOOLBOX_SELECT:
        {
            VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
                UpdateIndeterminate_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
            }
            else if( pToolBox->GetItemPos(pToolBox->GetCurItemId()) != TOOLBOX_ITEM_NOTFOUND )
            {
                UpdateChecked_Impl( pToolBox->GetItemPos(pToolBox->GetCurItemId()) );
                        UpdateIndeterminate_Impl( pToolBox->GetItemPos(pToolBox->GetCurItemId()) );
            }
            break;
        }
        case VCLEVENT_TOOLBOX_DOUBLECLICK:
        case VCLEVENT_TOOLBOX_ACTIVATE:
        case VCLEVENT_TOOLBOX_DEACTIVATE:
        //case VCLEVENT_TOOLBOX_SELECT:
            break;

        case VCLEVENT_TOOLBOX_ITEMUPDATED:
        {
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( TOOLBOX_ITEM_NOTFOUND );
                UpdateIndeterminate_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
            }
        break;
        }

        case VCLEVENT_TOOLBOX_HIGHLIGHT:
            UpdateFocus_Impl();
            break;

        case VCLEVENT_TOOLBOX_HIGHLIGHTOFF:
            ReleaseFocus_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
            break;

        case VCLEVENT_TOOLBOX_ITEMADDED :
            UpdateItem_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()), true );
            break;

        case VCLEVENT_TOOLBOX_ITEMREMOVED :
        case VCLEVENT_TOOLBOX_ALLITEMSCHANGED :
        {
            UpdateAllItems_Impl();
            break;
        }

        case VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED:
        {
            sal_Int32 nPos = (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
            ToolBoxItemsMap::iterator aAccessiblePos( m_aAccessibleChildren.find( nPos ) );
            if ( m_aAccessibleChildren.end() != aAccessiblePos )
            {
                implReleaseToolboxItem( aAccessiblePos, false, true );
                m_aAccessibleChildren.erase (aAccessiblePos);
            }

            Any aNewValue;
            aNewValue <<= getAccessibleChild(nPos);
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewValue );
            break;
        }
        case VCLEVENT_TOOLBOX_ITEMTEXTCHANGED :
            UpdateItemName_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
            break;

        case VCLEVENT_TOOLBOX_ITEMENABLED :
        case VCLEVENT_TOOLBOX_ITEMDISABLED :
        {
            UpdateItemEnabled_Impl( (sal_Int32)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()) );
            break;
        }

        case VCLEVENT_DROPDOWN_OPEN:
        case VCLEVENT_DROPDOWN_CLOSE:
        {
            UpdateCustomPopupItemp_Impl( static_cast< vcl::Window* >( rVclWindowEvent.GetData() ), rVclWindowEvent.GetId() == VCLEVENT_DROPDOWN_OPEN );
            break;
        }

        case VCLEVENT_OBJECT_DYING :
        {
            // if this toolbox is a subtoolbox, we have to release it from its parent
            VclPtr< vcl::Window > pWin = GetAs< vcl::Window >();
            if ( pWin && pWin->GetParent() &&
                 pWin->GetParent()->GetType() == WINDOW_TOOLBOX )
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
                implReleaseToolboxItem( aIter, false, true );
            }
            m_aAccessibleChildren.clear();

            SAL_FALLTHROUGH; // call base class
        }

        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

void VCLXAccessibleToolBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
        {
            Reference< XAccessible > xReturn = GetItemWindowAccessible(rVclWindowEvent);
            if ( xReturn.is() )
                NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), makeAny(xReturn) );
            else
                HandleSubToolBarEvent( rVclWindowEvent, true );
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
        implReleaseToolboxItem( aIter, false, true );
    }
    m_aAccessibleChildren.clear();
}

// XServiceInfo
OUString VCLXAccessibleToolBox::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleToolBox" );
}

Sequence< OUString > VCLXAccessibleToolBox::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames = VCLXAccessibleComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleToolBox";
    return aNames;
}

// XAccessibleContext
sal_Int32 SAL_CALL VCLXAccessibleToolBox::getAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    comphelper::OExternalLockGuard aGuard( this );

    sal_Int32 nCount = 0;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
        nCount = pToolBox->GetItemCount();

    return nCount;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    comphelper::OExternalLockGuard aGuard( this );

    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        Reference< XAccessible > xChild;
        // search for the child
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find(i);
        if ( m_aAccessibleChildren.end() == aIter )
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)i );
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
            m_aAccessibleChildren.insert( ToolBoxItemsMap::value_type( i, xChild ) );
        }
        else
        {
            // found it
            xChild = aIter->second;
        }
        return xChild;
    }

    return nullptr;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleAtPoint( const awt::Point& _rPoint ) throw (RuntimeException, std::exception)
{
    comphelper::OExternalLockGuard aGuard( this );

    Reference< XAccessible > xAccessible;
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    if ( pToolBox )
    {
        sal_uInt16 nItemPos = pToolBox->GetItemPos( VCLPoint( _rPoint ) );
        if ( nItemPos != TOOLBOX_ITEM_NOTFOUND )
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
        sal_uInt16 nCount = pToolBox->GetItemCount();
        for (sal_uInt16 i = 0 ; i < nCount && !xReturn.is() ; ++i)
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( i );
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
void VCLXAccessibleToolBox::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    sal_uInt16 nPos = static_cast < sal_uInt16 > (nChildIndex);
    pToolBox->ChangeHighlight( nPos );
}

sal_Bool VCLXAccessibleToolBox::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    sal_uInt16 nPos = static_cast < sal_uInt16 > (nChildIndex);
    if ( pToolBox && pToolBox->GetHighlightItemId() == pToolBox->GetItemId( nPos ) )
        return true;
    else
        return false;
}

void VCLXAccessibleToolBox::clearAccessibleSelection(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    VclPtr< ToolBox > pToolBox = GetAs< ToolBox >();
    pToolBox -> LoseFocus();
}

void VCLXAccessibleToolBox::selectAllAccessibleChildren(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    // intentionally empty. makes no sense for a toolbox
}

sal_Int32 VCLXAccessibleToolBox::getSelectedAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    sal_Int32 nRet = 0;
    for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; i++ )
    {
        if ( isAccessibleChildSelected( i ) )
        {
            nRet = 1;
            break; // a toolbox can only have (n)one selected child
        }
    }
    return nRet;
}

Reference< XAccessible > VCLXAccessibleToolBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    Reference< XAccessible > xChild;
    for ( sal_Int32 i = 0, j = 0, nCount = getAccessibleChildCount(); i < nCount; i++ )
    {
        if ( isAccessibleChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
        {
            xChild = getAccessibleChild( i );
            break;
        }
    }
    return xChild;
}

void VCLXAccessibleToolBox::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    clearAccessibleSelection(); // a toolbox can only have (n)one selected child
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
