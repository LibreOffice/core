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

#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <stdio.h>
#include <svx/charmap.hxx>
#include "charmapacc.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>
#include <osl/interlck.h>
#include <svx/dialmgr.hxx>
#include "accessibility.hrc"
#include <comphelper/types.hxx>

namespace svx
{
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;


SvxShowCharSetVirtualAcc::SvxShowCharSetVirtualAcc( SvxShowCharSet* pParent ) : OAccessibleComponentHelper(new VCLExternalSolarLock())
,mpParent( pParent )
,m_pTable(nullptr)
{
    osl_atomic_increment(&m_refCount);
    {
        lateInit(this);
    }
    osl_atomic_decrement(&m_refCount);
}


SvxShowCharSetVirtualAcc::~SvxShowCharSetVirtualAcc()
{
    ensureDisposed();
    delete getExternalLock();
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetVirtualAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetVirtualAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )

void SAL_CALL SvxShowCharSetVirtualAcc::fireEvent(
                    const sal_Int16 _nEventId,
                    const css::uno::Any& _rOldValue,
                    const css::uno::Any& _rNewValue
                )
{
    if ( m_pTable )
        m_pTable->fireEvent(_nEventId,_rOldValue,_rNewValue);
}

sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return ( mpParent->getScrollBar().IsVisible() ) ? 2 : 1;
}

uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    uno::Reference< css::accessibility::XAccessible >    xRet;
    const sal_uInt16 nItemId = sal::static_int_cast<sal_uInt16>(mpParent->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) ));

    if( sal_uInt16(-1) != nItemId )
    {
        if ( !m_pTable )
            m_pTable = new SvxShowCharSetAcc(this);
        xRet = m_pTable;
    }
    else if ( mpParent->getScrollBar().IsVisible() )
    {
        const Point aOutPos( mpParent->getScrollBar().GetPosPixel() );
        const Size  aScrollBar = mpParent->getScrollBar().GetOutputSizePixel();
        Rectangle aRect(aOutPos,aScrollBar);

        if ( aRect.IsInside(VCLPoint(aPoint)) )
            xRet = mpParent->getScrollBar().GetAccessible();
    }
    return xRet;
}

void SAL_CALL SvxShowCharSetVirtualAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    mpParent->GrabFocus();
}

Reference< XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    if ( mpParent->getScrollBar().IsVisible() && i == 0 )
        return mpParent->getScrollBar().GetAccessible();
    else if ( i == 1 )
    {
        if ( !m_xAcc.is() )
        {
            m_pTable = new SvxShowCharSetAcc(this);
            m_xAcc = m_pTable;
        }
    }
    else
        throw IndexOutOfBoundsException();
    return m_xAcc;
}

Reference< XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleParent(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    vcl::Window*                                         pParent = mpParent->GetParent();
    uno::Reference< css::accessibility::XAccessible >    xRet;

    if ( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}

css::awt::Rectangle SvxShowCharSetVirtualAcc::implGetBounds(  ) throw (RuntimeException)
{
    css::awt::Rectangle aBounds ( 0, 0, 0, 0 );
    vcl::Window* pWindow = mpParent;
    if ( pWindow )
    {
        Rectangle aRect = pWindow->GetWindowExtentsRelative( nullptr );
        aBounds = AWTRectangle( aRect );
        vcl::Window* pParent = pWindow->GetAccessibleParentWindow();
        if ( pParent )
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative( nullptr );
            css::awt::Point aParentScreenLoc = AWTPoint( aParentRect.TopLeft() );
            aBounds.X -= aParentScreenLoc.X;
            aBounds.Y -= aParentScreenLoc.Y;
        }
    }
    return aBounds;
}

sal_Int16 SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    return css::accessibility::AccessibleRole::SCROLL_PANE;
}

OUString SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleDescription(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHARACTER_SELECTION);
}

OUString SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleName(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHAR_SEL_DESC);
}

Reference< XAccessibleRelationSet > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleRelationSet(  ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessibleRelationSet >();
}

Reference< XAccessibleStateSet > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleStateSet(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        // SELECTABLE
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if ( mpParent->HasFocus() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );
        if ( mpParent->IsActive() )
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if ( mpParent->IsEnabled() )
        {
            pStateSet->AddState( AccessibleStateType::ENABLED );
            pStateSet->AddState( AccessibleStateType::SENSITIVE );
        }
        if ( mpParent->IsReallyVisible() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );
    }

    return pStateSet;
}

void SAL_CALL SvxShowCharSetVirtualAcc::disposing()
{
    OAccessibleContextHelper::disposing();
    if ( m_pTable )
        m_pTable->dispose();
    m_pTable = nullptr;
}


SvxShowCharSetItem::SvxShowCharSetItem( SvxShowCharSet& rParent,SvxShowCharSetAcc*  _pParent,sal_uInt16 _nPos ) :
    mrParent( rParent )
    ,mnId( _nPos )
    ,m_pItem(nullptr)
    ,m_pParent(_pParent)
{
}


SvxShowCharSetItem::~SvxShowCharSetItem()
{
    if ( m_xAcc.is() )
    {
        m_pItem->ParentDestroyed();
        ClearAccessible();
    }
}


uno::Reference< css::accessibility::XAccessible > const & SvxShowCharSetItem::GetAccessible()
{
    if( !m_xAcc.is() )
    {
        m_pItem = new SvxShowCharSetItemAcc( this );
        m_xAcc = m_pItem;
    }

    return m_xAcc;
}


void SvxShowCharSetItem::ClearAccessible()
{
    if ( m_xAcc.is() )
    {
        m_pItem = nullptr;
        m_xAcc  = nullptr;
    }
}


SvxShowCharSetAcc::SvxShowCharSetAcc( SvxShowCharSetVirtualAcc* _pParent ) : OAccessibleSelectionHelper(new VCLExternalSolarLock())
  ,m_pParent( _pParent )
{
    osl_atomic_increment(&m_refCount);
    {
        lateInit(this);
    }
    osl_atomic_decrement(&m_refCount);
}


SvxShowCharSetAcc::~SvxShowCharSetAcc()
{
    ensureDisposed();
    delete getExternalLock();
}

void SAL_CALL SvxShowCharSetAcc::disposing()
{
    OAccessibleSelectionHelper::disposing();
    ::std::vector< Reference< XAccessible > >::const_iterator aEnd  = m_aChildren.end();
    for (::std::vector< Reference< XAccessible > >::iterator aIter = m_aChildren.begin();aIter != aEnd ; ++aIter)
        ::comphelper::disposeComponent(*aIter);

    m_aChildren.clear();
    m_pParent = nullptr;
}


IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )

bool SvxShowCharSetAcc::implIsSelected( sal_Int32 nAccessibleChildIndex ) throw (RuntimeException)
{
    return m_pParent && m_pParent->getCharSetControl()->IsSelected(
        sal::static_int_cast<sal_uInt16>(nAccessibleChildIndex));
}

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
void SvxShowCharSetAcc::implSelect(sal_Int32 nAccessibleChildIndex, bool bSelect)
    throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if ( m_pParent )
    {
        if ( bSelect )
            m_pParent->getCharSetControl()->SelectIndex(nAccessibleChildIndex, true);
        else
            m_pParent->getCharSetControl()->DeSelect();
    }
}

css::awt::Rectangle SvxShowCharSetAcc::implGetBounds(  ) throw (RuntimeException)
{
    const Point   aOutPos;//( m_pParent->getCharSetControl()->GetPosPixel() );
    Size          aOutSize( m_pParent->getCharSetControl()->GetOutputSizePixel());
    if ( m_pParent->getCharSetControl()->getScrollBar().IsVisible() )
    {
        const Size aScrollBar = m_pParent->getCharSetControl()->getScrollBar().GetOutputSizePixel();
        aOutSize.Width() -= aScrollBar.Width();
    }

    awt::Rectangle aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->getMaxCharCount();
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    uno::Reference< css::accessibility::XAccessible >    xRet;
    SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem( static_cast< sal_uInt16 >( i ) );

    if( pItem )
    {
        pItem->m_pParent = this;
        xRet = pItem->GetAccessible();
        m_aChildren.push_back(xRet);
    }
    else
        throw lang::IndexOutOfBoundsException();

    return xRet;
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent;
}


sal_Int16 SAL_CALL SvxShowCharSetAcc::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    return css::accessibility::AccessibleRole::TABLE;
}


OUString SAL_CALL SvxShowCharSetAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHARACTER_SELECTION );
}


OUString SAL_CALL SvxShowCharSetAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SVX_RESSTR( RID_SVXSTR_CHAR_SEL_DESC );
}


uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL SvxShowCharSetAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetAcc::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( m_pParent->getCharSetControl() )
    {
        // SELECTABLE
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if ( m_pParent->getCharSetControl()->HasFocus() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );
        if ( m_pParent->getCharSetControl()->IsActive() )
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if ( m_pParent->getCharSetControl()->IsEnabled() )
        {
            pStateSet->AddState( AccessibleStateType::ENABLED );
            pStateSet->AddState( AccessibleStateType::SENSITIVE );
        }
        if ( m_pParent->getCharSetControl()->IsReallyVisible() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );

        pStateSet->AddState( AccessibleStateType::MANAGES_DESCENDANTS );
    }

    return pStateSet;
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    uno::Reference< css::accessibility::XAccessible >    xRet;
    const sal_uInt16 nItemId = sal::static_int_cast<sal_uInt16>(
        m_pParent->getCharSetControl()->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) ));

    if( sal_uInt16(-1) != nItemId )
    {
        SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem( nItemId );
        xRet = pItem->GetAccessible();
    }
    return xRet;
}

void SAL_CALL SvxShowCharSetAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    m_pParent->getCharSetControl()->GrabFocus();
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowCount(  ) throw (RuntimeException, std::exception)
{
    return ((getAccessibleChildCount()-1) / COLUMN_COUNT) + 1;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnCount(  ) throw (RuntimeException, std::exception)
{
    return COLUMN_COUNT;
}

OUString SAL_CALL SvxShowCharSetAcc::getAccessibleRowDescription( sal_Int32 /*nRow*/ ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return OUString();
}

OUString SAL_CALL SvxShowCharSetAcc::getAccessibleColumnDescription( sal_Int32 /*nColumn*/ ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return OUString();
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return 1;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return 1;
}

Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleRowHeaders(  ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessibleTable >();
}

Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleColumnHeaders(  ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessibleTable >();
}

Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleRows(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    Sequence< sal_Int32 > aSel(1);
    aSel[0] = SvxShowCharSet::GetRowPos(m_pParent->getCharSetControl()->GetSelectIndexId());
    return aSel;
}

Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleColumns(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    Sequence< sal_Int32 > aSel(1);
    aSel[0] = SvxShowCharSet::GetColumnPos(m_pParent->getCharSetControl()->GetSelectIndexId());
    return aSel;
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleRowSelected( sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetRowPos(m_pParent->getCharSetControl()->GetSelectIndexId()) == nRow;
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleColumnSelected( sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetColumnPos(m_pParent->getCharSetControl()->GetSelectIndexId()) == nColumn;
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    svx::SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem(
        sal::static_int_cast<sal_uInt16>(getAccessibleIndex(nRow,nColumn) ));
    if ( !pItem  )
        throw IndexOutOfBoundsException();
    return pItem->GetAccessible();
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCaption(  ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessible >();
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleSummary(  ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessible >();
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetSelectIndexId() == getAccessibleIndex(nRow,nColumn);
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return (nRow*COLUMN_COUNT) + nColumn;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRow( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetRowPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumn( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetColumnPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}


SvxShowCharSetItemAcc::SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent ) : OAccessibleComponentHelper(new VCLExternalSolarLock())
,mpParent( pParent )
{
    OSL_ENSURE(pParent,"NO parent supplied!");
    osl_atomic_increment(&m_refCount);
    { // #b6211265 #
        lateInit(this);
    }
    osl_atomic_decrement(&m_refCount);
}


SvxShowCharSetItemAcc::~SvxShowCharSetItemAcc()
{
    ensureDisposed();
    delete getExternalLock();
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )


void SvxShowCharSetItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( GetMutex() );
    mpParent = nullptr;
}


sal_Int32 SAL_CALL SvxShowCharSetItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException, std::exception)
{
    return 0;
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleChild( sal_Int32 /*i*/ )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return mpParent->m_pParent;
}


sal_Int16 SAL_CALL SvxShowCharSetItemAcc::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    return css::accessibility::AccessibleRole::TABLE_CELL;
}


OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    OUString sDescription;

    const OUString aCharStr( mpParent->maText);
    sal_Int32 nStrIndex = 0;
    const sal_UCS4 c = aCharStr.iterateCodePoints( &nStrIndex );
    const int tmp_len = (c < 0x10000) ? 4 : 6;
    char buf[16] = "0x0000";
    sal_UCS4 c_Shifted = c;
    for( int i = 0; i < tmp_len; ++i )
    {
        char h = (char)(c_Shifted & 0x0F);
        buf[tmp_len+1-i] = (h > 9) ? (h - 10 + 'A') : (h + '0');
        c_Shifted >>= 4;
    }
    if( c < 256 )
        snprintf( buf+6, 10, " (%" SAL_PRIuUINT32 ")", c );

    sDescription = SVX_RESSTR( RID_SVXSTR_CHARACTER_CODE )
                 + " "
                 + OUString(buf, strlen(buf), RTL_TEXTENCODING_ASCII_US);

    return sDescription;
}


OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    OUString aRet;

    if( mpParent )
    {
        aRet = mpParent->maText;

        if (aRet.isEmpty())
            aRet = getAccessibleDescription();
    }

    return aRet;
}


uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL SvxShowCharSetItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetItemAcc::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        if (mpParent->mrParent.IsEnabled())
        {
            pStateSet->AddState( css::accessibility::AccessibleStateType::ENABLED );
            // SELECTABLE
            pStateSet->AddState( css::accessibility::AccessibleStateType::SELECTABLE );
            pStateSet->AddState( css::accessibility::AccessibleStateType::FOCUSABLE );
        }

        // SELECTED
        if( mpParent->mrParent.GetSelectIndexId() == mpParent->mnId )
        {
            pStateSet->AddState( css::accessibility::AccessibleStateType::SELECTED );
               pStateSet->AddState( css::accessibility::AccessibleStateType::FOCUSED );
        }
        if ( mpParent->mnId >= mpParent->mrParent.FirstInView() && mpParent->mnId <= mpParent->mrParent.LastInView() )
        {
            pStateSet->AddState( AccessibleStateType::VISIBLE );
            pStateSet->AddState( AccessibleStateType::SHOWING );
        }
        pStateSet->AddState( AccessibleStateType::TRANSIENT );
    }

    return pStateSet;
}


sal_Int32 SvxShowCharSetItemAcc::getAccessibleActionCount() throw (RuntimeException, std::exception)
{
    return 1;
}


sal_Bool SvxShowCharSetItemAcc::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if( nIndex == 0 )
    {
        mpParent->mrParent.OutputIndex( mpParent->mnId );
        return true;
    }
    throw IndexOutOfBoundsException();
}


OUString SvxShowCharSetItemAcc::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if( nIndex == 0 )
        return OUString( "press" );
    throw IndexOutOfBoundsException();
}


Reference< css::accessibility::XAccessibleKeyBinding > SvxShowCharSetItemAcc::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    if( nIndex == 0 )
        return Reference< css::accessibility::XAccessibleKeyBinding >();
    throw IndexOutOfBoundsException();
}


void SAL_CALL SvxShowCharSetItemAcc::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    // nothing to do
}

awt::Rectangle SvxShowCharSetItemAcc::implGetBounds(  ) throw (RuntimeException)
{
    awt::Rectangle      aRet;

    if( mpParent )
    {
        Rectangle   aRect( mpParent->maRect );
        Point       aOrigin;
        Rectangle   aParentRect( aOrigin, mpParent->mrParent.GetOutputSizePixel() );

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleAtPoint( const awt::Point& /*aPoint*/ )
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< css::accessibility::XAccessible >();
}

sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getForeground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( mpParent )
    {
        if ( mpParent->IsControlForeground() )
            nColor = mpParent->GetControlForeground().GetColor();
        else
        {
            vcl::Font aFont;
            if ( mpParent->IsControlFont() )
                aFont = mpParent->GetControlFont();
            else
                aFont = mpParent->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}

sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getBackground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this  );
    sal_Int32 nColor = 0;
    if ( mpParent )
    {
        if ( mpParent->IsControlBackground() )
            nColor = mpParent->GetControlBackground().GetColor();
        else
            nColor = mpParent->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getForeground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pParent )
        nColor = m_pParent->getForeground();
    return nColor;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getBackground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this  );
    sal_Int32 nColor = 0;
    if ( m_pParent )
        nColor = m_pParent->getBackground();
    return nColor;
}


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
