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
    
    
    
    /** XAccessibleContext implementation for a toolbox item which is represented by a VCL Window
    */
    class OToolBoxWindowItemContext : public OAccessibleContextWrapper
    {
        sal_Int32 m_nIndexInParent;
    public:
        OToolBoxWindowItemContext(sal_Int32 _nIndexInParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerAccessibleContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxOwningAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
            ) : OAccessibleContextWrapper(
            _rxContext,
            _rxInnerAccessibleContext,
            _rxOwningAccessible,
            _rxParentAccessible     )
            ,m_nIndexInParent(_nIndexInParent)
        {
        }
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
    };

    
    sal_Int32 SAL_CALL OToolBoxWindowItemContext::getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return m_nIndexInParent;
    }

    
    
    
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

        static  sal_Bool    isWindowItem( const Reference< XAccessible >& _rxAcc, OToolBoxWindowItem** /* [out] */ _ppImplementation = NULL );

    public:
        OToolBoxWindowItem(sal_Int32 _nIndexInParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxInnerAccessible,
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParentAccessible
            ) : OAccessibleWrapper(
            _rxContext,
            _rxInnerAccessible,
            _rxParentAccessible)
            ,m_nIndexInParent(_nIndexInParent)
        {
        }

    protected:
        
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )

        
        virtual OAccessibleContextWrapper* createAccessibleContext(
                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >& _rxInnerContext
            );

        
        virtual sal_Int64 SAL_CALL getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw (RuntimeException);
        static Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };

    IMPLEMENT_FORWARD_XINTERFACE2( OToolBoxWindowItem, OAccessibleWrapper, OToolBoxWindowItem_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OToolBoxWindowItem, OAccessibleWrapper, OToolBoxWindowItem_Base )

    OAccessibleContextWrapper* OToolBoxWindowItem::createAccessibleContext(
            const Reference< XAccessibleContext >& _rxInnerContext )
    {
        return new OToolBoxWindowItemContext( m_nIndexInParent, getComponentContext(), _rxInnerContext, this, getParent() );
    }

    sal_Bool OToolBoxWindowItem::isWindowItem( const Reference< XAccessible >& _rxAcc, OToolBoxWindowItem** /* [out] */ _ppImplementation )
    {
        OToolBoxWindowItem* pImplementation = NULL;

        Reference< XUnoTunnel > xTunnel( _rxAcc, UNO_QUERY );
        if ( xTunnel.is() )
            pImplementation = reinterpret_cast< OToolBoxWindowItem* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

        if ( _ppImplementation )
            *_ppImplementation = pImplementation;

        return NULL != pImplementation;
    }

    Sequence< sal_Int8 > OToolBoxWindowItem::getUnoTunnelImplementationId()
    {
        static ::cppu::OImplementationId * pId = 0;
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

    sal_Int64 SAL_CALL OToolBoxWindowItem::getSomething( const Sequence< sal_Int8 >& _rId ) throw (RuntimeException)
    {
        if  (   ( 16 == _rId.getLength() )
            &&  ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(),  _rId.getConstArray(), 16 ) )
            )
            return reinterpret_cast< sal_Int64>( this );

        return 0;
    }
}



VCLXAccessibleToolBox::VCLXAccessibleToolBox( VCLXWindow* pVCLXWindow ) :

    VCLXAccessibleComponent( pVCLXWindow )

{
}

VCLXAccessibleToolBox::~VCLXAccessibleToolBox()
{
}

VCLXAccessibleToolBoxItem* VCLXAccessibleToolBox::GetItem_Impl( sal_Int32 _nPos, bool _bMustHaveFocus )
{
    VCLXAccessibleToolBoxItem* pItem = NULL;
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox && ( !_bMustHaveFocus || pToolBox->HasFocus() ) )
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        
        if ( aIter != m_aAccessibleChildren.end()  && aIter->second.is())
            pItem = static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
    }

    return pItem;
}

void VCLXAccessibleToolBox::UpdateFocus_Impl()
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if( !pToolBox )
        return;

    
    sal_Bool bHasFocus = sal_False;
    if ( pToolBox->HasFocus() )
        bHasFocus = sal_True;
    else
    {
        
        ToolBox* pToolBoxParent = dynamic_cast< ToolBox* >( pToolBox->GetParent() );
        
        if ( pToolBoxParent && pToolBoxParent->HasFocus() )
            bHasFocus = sal_True;
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
                    
                    pItem->SetFocus( sal_False );
                    nFocusCount++;
                }
                if ( nItemId == nHighlightItemId )
                {
                    
                    pItem->SetFocus( sal_True );
                    nFocusCount++;
                }
            }
        
        if ( nFocusCount > 1 )
            break;
        }
    }
}

void VCLXAccessibleToolBox::ReleaseFocus_Impl( sal_Int32 _nPos )
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox ) 
    {
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem->HasFocus() )
                pItem->SetFocus( sal_False );
        }
    }
}

void VCLXAccessibleToolBox::UpdateChecked_Impl( sal_Int32 _nPos )
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
    {
        sal_uInt16 nFocusId = pToolBox->GetItemId( (sal_uInt16)_nPos );
        VCLXAccessibleToolBoxItem* pFocusItem = NULL;

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
        
        if ( pFocusItem && (sal_uInt16)_nPos != TOOLBOX_ITEM_NOTFOUND )
            pFocusItem->SetFocus( sal_True );
    }
}

void VCLXAccessibleToolBox::UpdateIndeterminate_Impl( sal_Int32 _nPos )
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
    {
        sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)_nPos );

        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find( _nPos );
        if ( aIter != m_aAccessibleChildren.end() && aIter->second.is() )
        {
            VCLXAccessibleToolBoxItem* pItem =
                static_cast< VCLXAccessibleToolBoxItem* >( aIter->second.get() );
            if ( pItem )
                pItem->SetIndeterminate( pToolBox->GetItemState( nItemId ) == STATE_DONTKNOW );
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

    OToolBoxWindowItem* pWindowItem = NULL;
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

void VCLXAccessibleToolBox::UpdateItem_Impl( sal_Int32 _nPos, sal_Bool _bItemAdded )
{
    if ( _nPos < sal_Int32( m_aAccessibleChildren.size() ) )
    {
        UpdateAllItems_Impl();
        return;
    }

    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
    {
        if ( !_bItemAdded )
        {   
            
            ToolBoxItemsMap::iterator aItemPos = m_aAccessibleChildren.find( _nPos );
            if ( m_aAccessibleChildren.end() != aItemPos )
            {
                implReleaseToolboxItem( aItemPos, true, true );
                m_aAccessibleChildren.erase( aItemPos );
            }
        }

        
        ToolBoxItemsMap::iterator aIndexAdjust = m_aAccessibleChildren.upper_bound( _nPos );
        while ( m_aAccessibleChildren.end() != aIndexAdjust )
        {
            Reference< XAccessible > xItemAcc( aIndexAdjust->second );

            OToolBoxWindowItem* pWindowItem = NULL;
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
            
            
            Any aNewChild = makeAny( getAccessibleChild( (sal_Int32)_nPos ) );
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewChild );
        }
    }
}

void VCLXAccessibleToolBox::UpdateAllItems_Impl()
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
    {
        
        for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
                aIter != m_aAccessibleChildren.end(); ++aIter )
        {
            implReleaseToolboxItem( aIter, true, true );
        }
        m_aAccessibleChildren.clear();

        
        sal_uInt16 i, nCount = pToolBox->GetItemCount();
        for ( i = 0; i < nCount; ++i )
        {
            Any aNewValue;
            aNewValue <<= getAccessibleChild( (sal_Int32)i );;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), aNewValue );
        }
    }
}

void VCLXAccessibleToolBox::UpdateCustomPopupItemp_Impl( Window* pWindow, bool bOpen )
{
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if( pWindow && pToolBox )
    {
        Reference< XAccessible > xChild( pWindow->GetAccessible() );
        if( xChild.is() )
        {
            Reference< XAccessible > xChildItem( getAccessibleChild( static_cast< sal_Int32 >( pToolBox->GetItemPos( pToolBox->GetDownItemId() ) ) ) );
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
    Window* pChildWindow = (Window *) rVclWindowEvent.GetData();
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
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
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( !pToolBox )
        return;

    sal_Int32 nIndex = pToolBox->GetItemPos( pToolBox->GetCurItemId() );
    if ( nIndex == SAL_MAX_UINT16 )
        return; 

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

    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
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
    
    Reference< XAccessibleContext > xTemp = this;

    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TOOLBOX_CLICK:
        case VCLEVENT_TOOLBOX_SELECT:
        {
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData() );
                UpdateIndeterminate_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData() );
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
        
            break;
        
        /*
        case VCLEVENT_TOOLBOX_ITEMUPDATED:
        {
            if ( rVclWindowEvent.GetData() )
            {
                UpdateChecked_Impl( TOOLBOX_ITEM_NOTFOUND );
                UpdateIndeterminate_Impl( (sal_Int32)rVclWindowEvent.GetData() );
            }
        break;
        }
        */
        case VCLEVENT_TOOLBOX_HIGHLIGHT:
            UpdateFocus_Impl();
            break;

        case VCLEVENT_TOOLBOX_HIGHLIGHTOFF:
            ReleaseFocus_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData() );
            break;

        case VCLEVENT_TOOLBOX_ITEMADDED :
            UpdateItem_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData(), sal_True );
            break;

        case VCLEVENT_TOOLBOX_ITEMREMOVED :
        case VCLEVENT_TOOLBOX_ALLITEMSCHANGED :
        {
            UpdateAllItems_Impl();
            break;
        }

        case VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED:
        {
            sal_Int32 nPos = (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData();
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
            UpdateItemName_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData() );
            break;

        case VCLEVENT_TOOLBOX_ITEMENABLED :
        case VCLEVENT_TOOLBOX_ITEMDISABLED :
        {
            UpdateItemEnabled_Impl( (sal_Int32)(sal_IntPtr)rVclWindowEvent.GetData() );
            break;
        }

        case VCLEVENT_DROPDOWN_OPEN:
        case VCLEVENT_DROPDOWN_CLOSE:
        {
            UpdateCustomPopupItemp_Impl( static_cast< Window* >( rVclWindowEvent.GetData() ), rVclWindowEvent.GetId() == VCLEVENT_DROPDOWN_OPEN );
            break;
        }

        case VCLEVENT_OBJECT_DYING :
        {
            
            ToolBox* pBox = static_cast< ToolBox* >( GetWindow() );
            if ( pBox && pBox->GetParent() &&
                 pBox->GetParent()->GetType() == WINDOW_TOOLBOX )
            {
                VCLXAccessibleToolBox* pParent = static_cast< VCLXAccessibleToolBox* >(
                    pBox->GetParent()->GetAccessible()->getAccessibleContext().get() );
                if ( pParent )
                    pParent->ReleaseSubToolBox( pBox );
            }

            
            for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
                  aIter != m_aAccessibleChildren.end(); ++aIter )
            {
                implReleaseToolboxItem( aIter, false, true );
            }
            m_aAccessibleChildren.clear();

            
        }

        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

void VCLXAccessibleToolBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:  
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


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleToolBox, VCLXAccessibleComponent, VCLXAccessibleToolBox_BASE )


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleToolBox, VCLXAccessibleComponent, VCLXAccessibleToolBox_BASE )


void SAL_CALL VCLXAccessibleToolBox::disposing()
{
    VCLXAccessibleComponent::disposing();

    
    for ( ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.begin();
          aIter != m_aAccessibleChildren.end(); ++aIter )
    {
        implReleaseToolboxItem( aIter, false, true );
    }
    m_aAccessibleChildren.clear();
}


OUString VCLXAccessibleToolBox::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleToolBox" );
}

Sequence< OUString > VCLXAccessibleToolBox::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames = VCLXAccessibleComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleToolBox";
    return aNames;
}


sal_Int32 SAL_CALL VCLXAccessibleToolBox::getAccessibleChildCount(  ) throw (RuntimeException)
{
    comphelper::OExternalLockGuard aGuard( this );

    sal_Int32 nCount = 0;
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
        nCount = pToolBox->GetItemCount();

    return nCount;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    comphelper::OExternalLockGuard aGuard( this );

    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pToolBox )
    {
        Reference< XAccessible > xChild;
        
        ToolBoxItemsMap::iterator aIter = m_aAccessibleChildren.find(i);
        if ( m_aAccessibleChildren.end() == aIter )
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( (sal_uInt16)i );
            sal_uInt16 nHighlightItemId = pToolBox->GetHighlightItemId();
            Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
            
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
                pChild->SetFocus( sal_True );
            if ( pToolBox->IsItemChecked( nItemId ) )
                pChild->SetChecked( sal_True );
            if ( pToolBox->GetItemState( nItemId ) == STATE_DONTKNOW )
                pChild->SetIndeterminate( true );
            m_aAccessibleChildren.insert( ToolBoxItemsMap::value_type( i, xChild ) );
        }
        else
        {
            
            xChild = aIter->second;
        }
        return xChild;
    }

    return NULL;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleToolBox::getAccessibleAtPoint( const awt::Point& _rPoint ) throw (RuntimeException)
{
    comphelper::OExternalLockGuard aGuard( this );

    Reference< XAccessible > xAccessible;
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
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
    Window* pChildWindow = (Window *) rVclWindowEvent.GetData();
    ToolBox* pToolBox = static_cast< ToolBox* >( GetWindow() );
    if ( pChildWindow && pToolBox )
    {
        sal_uInt16 nCount = pToolBox->GetItemCount();
        for (sal_uInt16 i = 0 ; i < nCount && !xReturn.is() ; ++i)
        {
            sal_uInt16 nItemId = pToolBox->GetItemId( i );
            Window* pItemWindow = pToolBox->GetItemWindow( nItemId );
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


void VCLXAccessibleToolBox::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    ToolBox * pToolBox = static_cast < ToolBox * > ( GetWindow() );
    sal_uInt16 nPos = static_cast < sal_uInt16 > (nChildIndex);
    pToolBox->ChangeHighlight( nPos );
}

sal_Bool VCLXAccessibleToolBox::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    ToolBox * pToolBox = static_cast < ToolBox * > ( GetWindow() );
    sal_uInt16 nPos = static_cast < sal_uInt16 > (nChildIndex);
    if ( pToolBox != NULL && pToolBox->GetHighlightItemId() == pToolBox->GetItemId( nPos ) )
        return sal_True;
    else
        return sal_False;
}

void VCLXAccessibleToolBox::clearAccessibleSelection(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ToolBox * pToolBox = static_cast < ToolBox * > ( GetWindow() );
    pToolBox -> LoseFocus();
}

void VCLXAccessibleToolBox::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    
}

sal_Int32 VCLXAccessibleToolBox::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    sal_Int32 nRet = 0;
    for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; i++ )
    {
        if ( isAccessibleChildSelected( i ) )
        {
            nRet = 1;
            break; 
        }
    }
    return nRet;
}

Reference< XAccessible > VCLXAccessibleToolBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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

void VCLXAccessibleToolBox::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    clearAccessibleSelection(); 
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
