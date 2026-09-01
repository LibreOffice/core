/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <stdio.h>
#include <svx/charmap.hxx>
#include <charmapacc.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <o3tl/temporary.hxx>
#include <osl/interlck.h>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <comphelper/accessiblecontexthelper.hxx>

namespace svx
{
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;
    using namespace ::cpo::uno;

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
        m_xItem->dispose();
        m_xItem.clear();
    }
}

const rtl::Reference<SvxShowCharSetItemAcc> & SvxShowCharSetItem::GetAccessible()
{
    if( !m_xItem.is() )
    {
        m_xItem = new SvxShowCharSetItemAcc( this );
    }

    return m_xItem;
}

SvxShowCharSetAcc::SvxShowCharSetAcc(SvxShowCharSet* pParent)
    : m_pParent(pParent)
{
}

SvxShowCharSetAcc::~SvxShowCharSetAcc()
{
    ensureDisposed();
}

void SvxShowCharSetAcc::disposing()
{
    OAccessibleSelectionHelper::disposing();
    for (auto& rxChild : m_aChildren)
        rxChild->dispose();

    m_aChildren.clear();
    m_pParent = nullptr;
}

bool SvxShowCharSetAcc::implIsSelected( sal_Int64 nAccessibleChildIndex )
{
    if (!m_pParent)
        return false;

    if (nAccessibleChildIndex < 0 || nAccessibleChildIndex >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    return m_pParent->IsSelected(sal::static_int_cast<sal_uInt16>(nAccessibleChildIndex));
}

// select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
void SvxShowCharSetAcc::implSelect(sal_Int64 nAccessibleChildIndex, bool bSelect)
{
    if (!m_pParent)
        return;

    if (nAccessibleChildIndex < 0 || nAccessibleChildIndex >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    if (bSelect)
        m_pParent->SelectIndex(nAccessibleChildIndex, true);
    else
        m_pParent->DeSelect();
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

sal_Int64 SvxShowCharSetAcc::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_pParent->getMaxCharCount();
}

uno::Reference< css::accessibility::XAccessible > SvxShowCharSetAcc::getAccessibleChild( sal_Int64 i )
{
    OExternalLockGuard aGuard( this );

    rtl::Reference< SvxShowCharSetItemAcc >  xRet;
    SvxShowCharSetItem* pItem = m_pParent->ImplGetItem( static_cast< sal_uInt16 >( i ) );

    if( !pItem )
        throw lang::IndexOutOfBoundsException();

    pItem->m_pParent = this;
    xRet = pItem->GetAccessible();
    m_aChildren.push_back(xRet);

    return xRet;
}

uno::Reference< css::accessibility::XAccessible > SvxShowCharSetAcc::getAccessibleParent()
{
    OExternalLockGuard aGuard( this );

    if (m_pParent)
        return m_pParent->getAccessibleParent();
    return uno::Reference<css::accessibility::XAccessible>();
}

sal_Int16 SvxShowCharSetAcc::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::TABLE;
}

OUString SvxShowCharSetAcc::getAccessibleDescription()
{
    OExternalLockGuard aGuard( this );
    return SvxResId( RID_SVXSTR_CHARACTER_SELECTION );
}


OUString SvxShowCharSetAcc::getAccessibleName()
{
    OExternalLockGuard aGuard( this );

    return SvxResId( RID_SVXSTR_CHAR_SEL_DESC );
}


uno::Reference< css::accessibility::XAccessibleRelationSet > SvxShowCharSetAcc::getAccessibleRelationSet()
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


sal_Int64 SvxShowCharSetAcc::getAccessibleStateSet()
{
    OExternalLockGuard aGuard( this );

    sal_Int64 nStateSet = 0;

    if (m_pParent)
    {
        // SELECTABLE
        nStateSet |= AccessibleStateType::FOCUSABLE;
        if (m_pParent->HasFocus())
        {
            nStateSet |= AccessibleStateType::FOCUSED;
            nStateSet |= AccessibleStateType::ACTIVE;
        }
        if (m_pParent->IsEnabled())
        {
            nStateSet |= AccessibleStateType::ENABLED;
            nStateSet |= AccessibleStateType::SENSITIVE;
        }
        if (m_pParent->IsVisible())
            nStateSet |= AccessibleStateType::VISIBLE;

        nStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
    }

    return nStateSet;
}


uno::Reference< css::accessibility::XAccessible > SvxShowCharSetAcc::getAccessibleAtPoint( const awt::Point& aPoint )
{
    OExternalLockGuard aGuard( this );

    const sal_uInt16 nItemId = sal::static_int_cast<sal_uInt16>(
        m_pParent->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) ));

    if( sal_uInt16(-1) == nItemId )
        return nullptr;
    SvxShowCharSetItem* pItem = m_pParent->ImplGetItem( nItemId );
    return pItem->GetAccessible();
}

void SvxShowCharSetAcc::grabFocus()
{
    OExternalLockGuard aGuard( this );

    m_pParent->GrabFocus();
}

sal_Int32 SvxShowCharSetAcc::getAccessibleRowCount(  )
{
    return ((getAccessibleChildCount()-1) / COLUMN_COUNT) + 1;
}

sal_Int32 SvxShowCharSetAcc::getAccessibleColumnCount(  )
{
    return COLUMN_COUNT;
}

OUString SvxShowCharSetAcc::getAccessibleRowDescription( sal_Int32 /*nRow*/ )
{
    return OUString();
}

OUString SvxShowCharSetAcc::getAccessibleColumnDescription( sal_Int32 /*nColumn*/ )
{
    return OUString();
}

sal_Int32 SvxShowCharSetAcc::getAccessibleRowExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
{
    return 1;
}

sal_Int32 SvxShowCharSetAcc::getAccessibleColumnExtentAt( sal_Int32 /*nRow*/, sal_Int32 /*nColumn*/ )
{
    return 1;
}

Reference< XAccessibleTable > SvxShowCharSetAcc::getAccessibleRowHeaders(  )
{
    return Reference< XAccessibleTable >();
}

Reference< XAccessibleTable > SvxShowCharSetAcc::getAccessibleColumnHeaders(  )
{
    return Reference< XAccessibleTable >();
}

Sequence< sal_Int32 > SvxShowCharSetAcc::getSelectedAccessibleRows(  )
{
    OExternalLockGuard aGuard( this );

    return { SvxShowCharSet::GetRowPos(m_pParent->GetSelectIndexId()) };
}

Sequence< sal_Int32 > SvxShowCharSetAcc::getSelectedAccessibleColumns(  )
{
    OExternalLockGuard aGuard( this );

    return { SvxShowCharSet::GetColumnPos(m_pParent->GetSelectIndexId()) };
}

bool SvxShowCharSetAcc::isAccessibleRowSelected( sal_Int32 nRow )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetRowPos(m_pParent->GetSelectIndexId()) == nRow;
}

bool SvxShowCharSetAcc::isAccessibleColumnSelected( sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SvxShowCharSet::GetColumnPos(m_pParent->GetSelectIndexId()) == nColumn;
}

Reference< XAccessible > SvxShowCharSetAcc::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );

    svx::SvxShowCharSetItem* pItem = m_pParent->ImplGetItem(
        sal::static_int_cast<sal_uInt16>(getAccessibleIndex(nRow,nColumn) ));
    if ( !pItem  )
        throw IndexOutOfBoundsException();
    return pItem->GetAccessible();
}

Reference< XAccessible > SvxShowCharSetAcc::getAccessibleCaption(  )
{
    return Reference< XAccessible >();
}

Reference< XAccessible > SvxShowCharSetAcc::getAccessibleSummary(  )
{
    return Reference< XAccessible >();
}

bool SvxShowCharSetAcc::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
{
    OExternalLockGuard aGuard( this );

    return m_pParent->GetSelectIndexId() == getAccessibleIndex(nRow,nColumn);
}

sal_Int64 SvxShowCharSetAcc::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
{
    return (static_cast<sal_Int64>(nRow) * COLUMN_COUNT) + nColumn;
}

sal_Int32 SvxShowCharSetAcc::getAccessibleRow( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetRowPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}

sal_Int32 SvxShowCharSetAcc::getAccessibleColumn( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    return SvxShowCharSet::GetColumnPos(sal::static_int_cast<sal_uInt16>(nChildIndex));
}


SvxShowCharSetItemAcc::SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent ) : mpParent( pParent )
{
    OSL_ENSURE(pParent,"NO parent supplied!");
}


SvxShowCharSetItemAcc::~SvxShowCharSetItemAcc()
{
    ensureDisposed();
}

void SvxShowCharSetItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( GetMutex() );
    mpParent = nullptr;
}

sal_Int64 SvxShowCharSetItemAcc::getAccessibleChildCount()
{
    return 0;
}


uno::Reference< css::accessibility::XAccessible > SvxShowCharSetItemAcc::getAccessibleChild( sal_Int64 /*i*/ )
{
    throw lang::IndexOutOfBoundsException();
}


uno::Reference< css::accessibility::XAccessible > SvxShowCharSetItemAcc::getAccessibleParent()
{
    OExternalLockGuard aGuard( this );

    return mpParent->m_pParent;
}


sal_Int16 SvxShowCharSetItemAcc::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::TABLE_CELL;
}


OUString SvxShowCharSetItemAcc::getAccessibleDescription()
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;

    const OUString aCharStr( mpParent->maText);
    const sal_UCS4 c = aCharStr.iterateCodePoints( &o3tl::temporary(sal_Int32(0)) );
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


OUString SvxShowCharSetItemAcc::getAccessibleName()
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


uno::Reference< css::accessibility::XAccessibleRelationSet > SvxShowCharSetItemAcc::getAccessibleRelationSet()
{
    return uno::Reference< css::accessibility::XAccessibleRelationSet >();
}


sal_Int64 SvxShowCharSetItemAcc::getAccessibleStateSet()
{
    OExternalLockGuard aGuard( this );

    sal_Int64 nStateSet = 0;

    if( mpParent )
    {
        if (mpParent->mrParent.IsEnabled())
        {
            nStateSet |= css::accessibility::AccessibleStateType::ENABLED;
            // SELECTABLE
            nStateSet |= css::accessibility::AccessibleStateType::SELECTABLE;
            nStateSet |= css::accessibility::AccessibleStateType::FOCUSABLE;
        }

        // SELECTED
        if( mpParent->mrParent.GetSelectIndexId() == mpParent->mnId )
        {
            nStateSet |= css::accessibility::AccessibleStateType::SELECTED;
            if (mpParent->mrParent.HasChildFocus())
                nStateSet |= css::accessibility::AccessibleStateType::FOCUSED;
        }
        if ( mpParent->mnId >= mpParent->mrParent.FirstInView() && mpParent->mnId <= mpParent->mrParent.LastInView() )
        {
            nStateSet |= AccessibleStateType::VISIBLE;
            nStateSet |= AccessibleStateType::SHOWING;
        }
        nStateSet |= AccessibleStateType::TRANSIENT;
    }

    return nStateSet;
}


sal_Int32 SvxShowCharSetItemAcc::getAccessibleActionCount()
{
    return 1;
}


bool SvxShowCharSetItemAcc::doAccessibleAction ( sal_Int32 nIndex )
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
        return u"press"_ustr;
    throw IndexOutOfBoundsException();
}


Reference< css::accessibility::XAccessibleKeyBinding > SvxShowCharSetItemAcc::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    if( nIndex == 0 )
        return Reference< css::accessibility::XAccessibleKeyBinding >();
    throw IndexOutOfBoundsException();
}


void SvxShowCharSetItemAcc::grabFocus()
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

uno::Reference< css::accessibility::XAccessible > SvxShowCharSetItemAcc::getAccessibleAtPoint( const awt::Point& /*aPoint*/ )
{
    return uno::Reference< css::accessibility::XAccessible >();
}

sal_Int32 SvxShowCharSetAcc::getForeground(  )
{
    OExternalLockGuard aGuard( this );

    //see SvxShowCharSet::InitSettings
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return static_cast<sal_Int32>(rStyleSettings.GetDialogTextColor());
}

sal_Int32 SvxShowCharSetAcc::getBackground(  )
{
    OExternalLockGuard aGuard( this  );

    //see SvxShowCharSet::InitSettings
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    return static_cast<sal_Int32>(rStyleSettings.GetWindowColor());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
