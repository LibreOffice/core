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
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <stdio.h>
#include <svx/charmap.hxx>
#include <charmapacc.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <toolkit/helper/convert.hxx>
#include <osl/interlck.h>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <comphelper/types.hxx>

namespace svx
{
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

SvxShowCharSetItem::SvxShowCharSetItem( SvxShowCharSet& rParent,SvxShowCharSetAcc*  _pParent,sal_uInt16 _nPos ) :
    mrParent( rParent )
    ,mnId( _nPos )
    ,m_pParent(_pParent)
{
}

SvxShowCharSetItem::~SvxShowCharSetItem()
{
    if ( m_xItem.is() )
    {
        m_xItem->ParentDestroyed();
        m_xItem.clear();
    }
}

uno::Reference< css::accessibility::XAccessible > SvxShowCharSetItem::GetAccessible()
{
    if( !m_xItem.is() )
    {
        m_xItem = new SvxShowCharSetItemAcc( this );
    }

    return m_xItem.get();
}

SvxShowCharSetAcc::SvxShowCharSetAcc(SvxShowCharSet* pParent)
    : m_pParent(pParent)
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
}

void SAL_CALL SvxShowCharSetAcc::disposing()
{
    OAccessibleSelectionHelper::disposing();
    for (auto& rChild : m_aChildren)
        ::comphelper::disposeComponent(rChild);

    m_aChildren.clear();
    m_pParent = nullptr;
}


IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )

bool SvxShowCharSetAcc::implIsSelected( sal_Int32 nAccessibleChildIndex )
{
    return m_pParent && m_pParent->IsSelected(
        sal::static_int_cast<sal_uInt16>(nAccessibleChildIndex));
}

        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
void SvxShowCharSetAcc::implSelect(sal_Int32 nAccessibleChildIndex, bool bSelect)
{
    if ( m_pParent )
    {
        if ( bSelect )
            m_pParent->SelectIndex(nAccessibleChildIndex, true);
        else
            m_pParent->DeSelect();
    }
}

css::awt::Rectangle SvxShowCharSetAcc::implGetBounds()
{
    awt::Rectangle aRet;

    if (m_pParent)
    {
        const Point   aOutPos;//( m_pParent->GetPosPixel() );
        Size          aOutSize( m_pParent->GetOutputSizePixel());

        aRet.X = aOutPos.X();
        aRet.Y = aOutPos.Y();
        aRet.Width = aOutSize.Width();
        aRet.Height = aOutSize.Height();
    }

    return aRet;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_pParent->getMaxCharCount();
}

uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleChild( sal_Int32 i )
{
    OExternalLockGuard aGuard( this );

    uno::Reference< css::accessibility::XAccessible >    xRet;
    SvxShowCharSetItem* pItem = m_pParent->ImplGetItem( static_cast< sal_uInt16 >( i ) );

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    pItem->m_pParent = this;
    xRet = pItem->GetAccessible();
    m_aChildren.push_back(xRet);

    return xRet;
}

uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleParent()
{
    OExternalLockGuard aGuard( this );

    if (m_pParent)
        return m_pParent->getAccessibleParent();
    return uno::Reference<css::accessibility::XAccessible>();
}

sal_Int16 SAL_CALL SvxShowCharSetAcc::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::TABLE;
}

OUString SAL_CALL SvxShowCharSetAcc::getAccessibleDescription()
{
    OExternalLockGuard aGuard( this );
    return SvxResId( RID_SVXSTR_CHARACTER_SELECTION );
}


OUString SAL_CALL SvxShowCharSetAcc::getAccessibleName()
{
    OExternalLockGuard aGuard( this );

    return SvxResId( RID_SVXSTR_CHAR_SEL_DESC );
}


uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL SvxShowCharSetAcc::getAccessibleRelationSet()
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetAcc::getAccessibleStateSet()
{
    OExternalLockGuard aGuard( this );

    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if (m_pParent)
    {
        // SELECTABLE
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if (m_pParent->HasFocus())
        {
            pStateSet->AddState( AccessibleStateType::FOCUSED );
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        }
        if (m_pParent->IsEnabled())
        {
            pStateSet->AddState( AccessibleStateType::ENABLED );
            pStateSet->AddState( AccessibleStateType::SENSITIVE );
        }
        if (m_pParent->IsVisible())
            pStateSet->AddState( AccessibleStateType::VISIBLE );

        pStateSet->AddState( AccessibleStateType::MANAGES_DESCENDANTS );
    }

    return pStateSet;
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    OExternalLockGuard aGuard( this );

    uno::Reference< css::accessibility::XAccessible >    xRet;
    const sal_uInt16 nItemId = sal::static_int_cast<sal_uInt16>(
        m_pParent->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) ));

    if( sal_uInt16(-1) != nItemId )
    {
        SvxShowCharSetItem* pItem = m_pParent->ImplGetItem( nItemId );
        xRet = pItem->GetAccessible();
    }
    return xRet;
}

void SAL_CALL SvxShowCharSetAcc::grabFocus()
{
    OExternalLockGuard aGuard( this );

    m_pParent->GrabFocus();
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowCount(  )
{
    return ((getAccessibleChildCount()-1) / COLUMN_COUNT) + 1;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnCount(  )
{
    return COLUMN_COUNT;
}

OUString SAL_CALL SvxShowCharSetAcc::getAccessibleRowDescription( sal_Int32 /*nRow*/ )
{
    return OUString();
}

OUString SAL_CALL SvxShowCharSetAcc::getAccessibleColumnDescription( sal_Int32 /*nColumn*/ )
{
    return OUString();
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
{
    return 1;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
{
    return 1;
}

Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleRowHeaders(  )
{
    return Reference< XAccessibleTable >();
}

Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleColumnHeaders(  )
{
    return Reference< XAccessibleTable >();
}

Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleRows(  )
{
    OExternalLockGuard aGuard( this );

    Sequence< sal_Int32 > aSel(1);
    aSel[0] = SvxShowCharSet::GetRowPos(m_pParent->GetSelectIndexId());
    return aSel;
}

Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleColumns(  )
{
    OExternalLockGuard aGuard( this );

    Sequence< sal_Int32 > aSel(1);
    aSel[0] = SvxShowCharSet::GetColumnPos(m_pParent->GetSelectIndexId());
    return aSel;
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleRowSelected( sal_Int32 nRow )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetRowPos(m_pParent->GetSelectIndexId()) == nRow;
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleColumnSelected( sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetColumnPos(m_pParent->GetSelectIndexId()) == nColumn;
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );

    svx::SvxShowCharSetItem* pItem = m_pParent->ImplGetItem(
        sal::static_int_cast<sal_uInt16>(getAccessibleIndex(nRow,nColumn) ));
    if ( !pItem  )
        throw IndexOutOfBoundsException();
    return pItem->GetAccessible();
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCaption(  )
{
    return Reference< XAccessible >();
}

Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleSummary(  )
{
    return Reference< XAccessible >();
}

sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );

    return m_pParent->GetSelectIndexId() == getAccessibleIndex(nRow,nColumn);
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
{
    return (nRow*COLUMN_COUNT) + nColumn;
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRow( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetRowPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumn( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetColumnPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}


SvxShowCharSetItemAcc::SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent ) : mpParent( pParent )
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
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_3 )

void SvxShowCharSetItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( GetMutex() );
    mpParent = nullptr;
}

sal_Int32 SAL_CALL SvxShowCharSetItemAcc::getAccessibleChildCount()
{
    return 0;
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleChild( sal_Int32 /*i*/ )
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleParent()
{
    OExternalLockGuard aGuard( this );

    return mpParent->m_pParent;
}


sal_Int16 SAL_CALL SvxShowCharSetItemAcc::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::TABLE_CELL;
}


OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleDescription()
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;

    const OUString aCharStr( mpParent->maText);
    sal_Int32 nStrIndex = 0;
    const sal_UCS4 c = aCharStr.iterateCodePoints( &nStrIndex );
    const int tmp_len = (c < 0x10000) ? 4 : 6;
    char buf[16] = "0x0000";
    sal_UCS4 c_Shifted = c;
    for( int i = 0; i < tmp_len; ++i )
    {
        char h = static_cast<char>(c_Shifted & 0x0F);
        buf[tmp_len+1-i] = (h > 9) ? (h - 10 + 'A') : (h + '0');
        c_Shifted >>= 4;
    }
    if( c < 256 )
        snprintf( buf+6, 10, " (%" SAL_PRIuUINT32 ")", c );

    sDescription = SvxResId( RID_SVXSTR_CHARACTER_CODE )
                 + " "
                 + OUString(buf, strlen(buf), RTL_TEXTENCODING_ASCII_US);

    return sDescription;
}


OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleName()
{
    OExternalLockGuard aGuard( this );

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
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetItemAcc::getAccessibleStateSet()
{
    OExternalLockGuard aGuard( this );

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


sal_Int32 SvxShowCharSetItemAcc::getAccessibleActionCount()
{
    return 1;
}


sal_Bool SvxShowCharSetItemAcc::doAccessibleAction ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if( nIndex == 0 )
    {
        mpParent->mrParent.OutputIndex( mpParent->mnId );
        return true;
    }
    throw IndexOutOfBoundsException();
}


OUString SvxShowCharSetItemAcc::getAccessibleActionDescription ( sal_Int32 nIndex )
{
    if( nIndex == 0 )
        return OUString( "press" );
    throw IndexOutOfBoundsException();
}


Reference< css::accessibility::XAccessibleKeyBinding > SvxShowCharSetItemAcc::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    if( nIndex == 0 )
        return Reference< css::accessibility::XAccessibleKeyBinding >();
    throw IndexOutOfBoundsException();
}


void SAL_CALL SvxShowCharSetItemAcc::grabFocus()
{
    // nothing to do
}

awt::Rectangle SvxShowCharSetItemAcc::implGetBounds(  )
{
    awt::Rectangle      aRet;

    if( mpParent )
    {
        tools::Rectangle   aRect( mpParent->maRect );
        tools::Rectangle   aParentRect(Point(), mpParent->mrParent.GetOutputSizePixel());

        aRect.Intersection( aParentRect );

        aRet.X = aRect.Left();
        aRet.Y = aRect.Top();
        aRet.Width = aRect.GetWidth();
        aRet.Height = aRect.GetHeight();
    }

    return aRet;
}

uno::Reference< css::accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleAtPoint( const awt::Point& /*aPoint*/ )
{
    return uno::Reference< css::accessibility::XAccessible >();
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getForeground(  )
{
    OExternalLockGuard aGuard( this );

    //see SvxShowCharSet::InitSettings
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return static_cast<sal_Int32>(rStyleSettings.GetDialogTextColor());
}

sal_Int32 SAL_CALL SvxShowCharSetAcc::getBackground(  )
{
    OExternalLockGuard aGuard( this  );

    //see SvxShowCharSet::InitSettings
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return static_cast<sal_Int32>(rStyleSettings.GetWindowColor());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
