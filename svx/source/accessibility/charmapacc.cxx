/*************************************************************************
 *
 *  $RCSfile: charmapacc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SVX_CHARMAP_CXX_

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <stdio.h>
#include "charmap.hxx"
#include "charmapacc.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#include <toolkit/helper/externallock.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#include "accessibility.hrc"
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

namespace svx
{
    using namespace comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::drafts::com::sun::star;
    using namespace ::drafts::com::sun::star::accessibility;

// ----------------
// - SvxShowCharSetVirtualAcc -
// ----------------
SvxShowCharSetVirtualAcc::SvxShowCharSetVirtualAcc( SvxShowCharSet* pParent ) : OAccessibleComponentHelper(new VCLExternalSolarLock())
,mpParent( pParent )
,m_pTable(NULL)
{
    osl_incrementInterlockedCount(&m_refCount);
    lateInit(this);
    osl_decrementInterlockedCount(&m_refCount);
}

// -----------------------------------------------------------------------------

SvxShowCharSetVirtualAcc::~SvxShowCharSetVirtualAcc()
{
    ensureDisposed();
    delete getExternalLock();
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetVirtualAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetVirtualAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )

void SAL_CALL SvxShowCharSetVirtualAcc::fireEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                )
{
    if ( m_pTable )
        m_pTable->fireEvent(_nEventId,_rOldValue,_rNewValue);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleChildCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return ( mpParent->getScrollBar()->IsVisible() ) ? 2 : 1;
}
// -----------------------------------------------------------------------------
uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleAt( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    uno::Reference< accessibility::XAccessible >    xRet;
    const USHORT nItemId = mpParent->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) );

    if( USHORT(-1) != nItemId )
    {
        if ( !m_pTable )
            m_pTable = new SvxShowCharSetAcc(this);
        xRet = m_pTable;
    }
    else if ( mpParent->getScrollBar()->IsVisible() )
    {
        const Point aOutPos( mpParent->getScrollBar()->GetPosPixel() );
        const Size  aScrollBar = mpParent->getScrollBar()->GetOutputSizePixel();
        Rectangle aRect(aOutPos,aScrollBar);

        if ( aRect.IsInside(VCLPoint(aPoint)) )
            xRet = mpParent->getScrollBar()->GetAccessible();
    }
    return xRet;
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}
// -----------------------------------------------------------------------------
void SAL_CALL SvxShowCharSetVirtualAcc::grabFocus()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    mpParent->GrabFocus();
}


// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    if ( mpParent->getScrollBar()->IsVisible() && i == 0 )
        return mpParent->getScrollBar()->GetAccessible();
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
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    Window*                                         pParent = mpParent->GetParent();
    uno::Reference< accessibility::XAccessible >    xRet;

    if ( pParent )
        xRet = pParent->GetAccessible();

    return xRet;
}
// -----------------------------------------------------------------------------
::com::sun::star::awt::Rectangle SAL_CALL SvxShowCharSetVirtualAcc::implGetBounds(  ) throw (RuntimeException)
{
    const Point   aOutPos( mpParent->GetPosPixel() );
    Size          aOutSize( mpParent->GetOutputSizePixel() );
    if ( mpParent->getScrollBar()->IsVisible() )
    {
        const Size aScrollBar = mpParent->getScrollBar()->GetOutputSizePixel();
        aOutSize.Width() -= aScrollBar.Width();
        aOutSize.Height() -= aScrollBar.Height();
    }

    awt::Rectangle aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}
// -----------------------------------------------------------------------------
sal_Int16 SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleRole(  ) throw (RuntimeException)
{
    return accessibility::AccessibleRole::SCROLLPANE;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleDescription(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHARACTER_SELECTION);
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHAR_SEL_DESC);
}
// -----------------------------------------------------------------------------
Reference< XAccessibleRelationSet > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    return Reference< XAccessibleRelationSet >();
}
// -----------------------------------------------------------------------------
Reference< XAccessibleStateSet > SAL_CALL SvxShowCharSetVirtualAcc::getAccessibleStateSet(  ) throw (RuntimeException)
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
            pStateSet->AddState( AccessibleStateType::ENABLED );
        if ( mpParent->IsReallyVisible() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );
    }

    return pStateSet;
}
// -----------------------------------------------------------------------------
void SAL_CALL SvxShowCharSetVirtualAcc::disposing()
{
    OAccessibleContextHelper::disposing();
    if ( m_pTable )
        m_pTable->dispose();
    m_pTable = NULL;
}
// -----------------------------------------------------------------------------
// ----------------
// - SvxShowCharSetItem -
// ----------------

SvxShowCharSetItem::SvxShowCharSetItem( SvxShowCharSet& rParent,SvxShowCharSetAcc*  _pParent,USHORT _nPos ) :
    mrParent( rParent )
    ,mnId( _nPos )
    ,m_pParent(_pParent)
    ,m_pItem(NULL)
{
}
// -----------------------------------------------------------------------

SvxShowCharSetItem::~SvxShowCharSetItem()
{
    if ( m_xAcc.is() )
    {
        m_pItem->ParentDestroyed();
        ClearAccessible();
    }
}

// -----------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SvxShowCharSetItem::GetAccessible()
{
    if( !m_xAcc.is() )
    {
        m_pItem = new SvxShowCharSetItemAcc( this );
        m_xAcc = m_pItem;
    }

    return m_xAcc;
}

// -----------------------------------------------------------------------

void SvxShowCharSetItem::ClearAccessible()
{
    if ( m_xAcc.is() )
    {
        m_pItem = NULL;
        m_xAcc  = NULL;
    }
}


// ---------------
// - SvxShowCharSetAcc -
// ---------------

SvxShowCharSetAcc::SvxShowCharSetAcc( SvxShowCharSetVirtualAcc* _pParent ) : OAccessibleSelectionHelper(new VCLExternalSolarLock())
  ,m_pParent( _pParent )
{
    osl_incrementInterlockedCount(&m_refCount);
    lateInit(this);
    osl_decrementInterlockedCount(&m_refCount);
}

// -----------------------------------------------------------------------------

SvxShowCharSetAcc::~SvxShowCharSetAcc()
{
    ensureDisposed();
    delete getExternalLock();
}
// -----------------------------------------------------------------------------
void SAL_CALL SvxShowCharSetAcc::disposing()
{
    OAccessibleSelectionHelper::disposing();
    ::std::vector< Reference< XAccessible > >::iterator aIter = m_aChildren.begin();
    ::std::vector< Reference< XAccessible > >::iterator aEnd  = m_aChildren.end();
    for (;aIter != aEnd ; ++aIter)
        ::comphelper::disposeComponent(*aIter);

    m_aChildren.clear();
    m_pParent = NULL;
}

// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetAcc, OAccessibleSelectionHelper, OAccessibleHelper_Base )
// -----------------------------------------------------------------------
sal_Bool SvxShowCharSetAcc::implIsSelected( sal_Int32 nAccessibleChildIndex ) throw (RuntimeException)
{
    return m_pParent && m_pParent->getCharSetControl()->IsSelected(nAccessibleChildIndex);
}
// -----------------------------------------------------------------------------
        // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
void SvxShowCharSetAcc::implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect ) throw (IndexOutOfBoundsException, RuntimeException)
{
    if ( m_pParent )
    {
        if ( bSelect )
            m_pParent->getCharSetControl()->SelectIndex(nAccessibleChildIndex,sal_True);
        else
            m_pParent->getCharSetControl()->DeSelect();
    }
}
// -----------------------------------------------------------------------------
::com::sun::star::awt::Rectangle SAL_CALL SvxShowCharSetAcc::implGetBounds(  ) throw (RuntimeException)
{
    const Point   aOutPos( m_pParent->getCharSetControl()->GetPosPixel() );
    Size          aOutSize( m_pParent->getCharSetControl()->GetOutputSizePixel());
    if ( m_pParent->getCharSetControl()->getScrollBar()->IsVisible() )
    {
        const Size aScrollBar = m_pParent->getCharSetControl()->getScrollBar()->GetOutputSizePixel();
        aOutSize.Width() -= aScrollBar.Width();
        aOutSize.Height() -= aScrollBar.Height();
    }

    awt::Rectangle aRet;

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->getMaxCharCount();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    uno::Reference< accessibility::XAccessible >    xRet;
    SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem( static_cast< USHORT >( i ) );

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

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent;
}

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL SvxShowCharSetAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    return accessibility::AccessibleRole::TABLE;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL SvxShowCharSetAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    return SVX_RESSTR( RID_SVXSTR_CHARACTER_SELECTION );
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL SvxShowCharSetAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return SVX_RESSTR( RID_SVXSTR_CHAR_SEL_DESC );
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL SvxShowCharSetAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
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
            pStateSet->AddState( AccessibleStateType::ENABLED );
        if ( m_pParent->getCharSetControl()->IsReallyVisible() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );

        pStateSet->AddState( AccessibleStateType::MANAGES_DESCENDANT );
    }

    return pStateSet;
}
// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleAt( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    uno::Reference< accessibility::XAccessible >    xRet;
    const USHORT nItemId = m_pParent->getCharSetControl()->PixelToMapIndex( Point( aPoint.X, aPoint.Y ) );

    if( USHORT(-1) != nItemId )
    {
        SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem( nItemId );
        xRet = pItem->GetAccessible();
    }
    return xRet;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void SAL_CALL SvxShowCharSetAcc::grabFocus()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    m_pParent->getCharSetControl()->GrabFocus();
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL SvxShowCharSetAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowCount(  ) throw (RuntimeException)
{
    return ((getAccessibleChildCount()-1) / COLUMN_COUNT) + 1;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnCount(  ) throw (RuntimeException)
{
    return COLUMN_COUNT;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvxShowCharSetAcc::getAccessibleRowDescription( sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvxShowCharSetAcc::getAccessibleColumnDescription( sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return 1;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return 1;
}
// -----------------------------------------------------------------------------
Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleRowHeaders(  ) throw (RuntimeException)
{
    return Reference< XAccessibleTable >();
}
// -----------------------------------------------------------------------------
Reference< XAccessibleTable > SAL_CALL SvxShowCharSetAcc::getAccessibleColumnHeaders(  ) throw (RuntimeException)
{
    return Reference< XAccessibleTable >();
}
// -----------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleRows(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    Sequence< sal_Int32 > aSel(1);
    aSel[0] = m_pParent->getCharSetControl()->GetRowPos(m_pParent->getCharSetControl()->GetSelectIndexId());
    return aSel;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int32 > SAL_CALL SvxShowCharSetAcc::getSelectedAccessibleColumns(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    Sequence< sal_Int32 > aSel(1);
    aSel[0] = m_pParent->getCharSetControl()->GetColumnPos(m_pParent->getCharSetControl()->GetSelectIndexId());
    return aSel;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleRowSelected( sal_Int32 nRow ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetRowPos(m_pParent->getCharSetControl()->GetSelectIndexId()) == nRow;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleColumnSelected( sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetColumnPos(m_pParent->getCharSetControl()->GetSelectIndexId()) == nColumn;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    ::svx::SvxShowCharSetItem* pItem = m_pParent->getCharSetControl()->ImplGetItem( getAccessibleIndex(nRow,nColumn) );
    if ( !pItem  )
        throw IndexOutOfBoundsException();
    return pItem->GetAccessible();
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleCaption(  ) throw (RuntimeException)
{
    return Reference< XAccessible >();
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL SvxShowCharSetAcc::getAccessibleSummary(  ) throw (RuntimeException)
{
    return Reference< XAccessible >();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL SvxShowCharSetAcc::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetSelectIndexId() == getAccessibleIndex(nRow,nColumn);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return (nRow*COLUMN_COUNT) + nColumn;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleRow( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetRowPos(nChildIndex);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getAccessibleColumn( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return m_pParent->getCharSetControl()->GetColumnPos(nChildIndex);
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ----------------
// - SvxShowCharSetItemAcc -
// ----------------

SvxShowCharSetItemAcc::SvxShowCharSetItemAcc( SvxShowCharSetItem* pParent ) : OAccessibleComponentHelper(new VCLExternalSolarLock())
,mpParent( pParent )
{
    OSL_ENSURE(pParent,"NO parent supplied!");
    osl_incrementInterlockedCount(&m_refCount);
    lateInit(this);
    osl_decrementInterlockedCount(&m_refCount);
}

// -----------------------------------------------------------------------------

SvxShowCharSetItemAcc::~SvxShowCharSetItemAcc()
{
    ensureDisposed();
    delete getExternalLock();
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxShowCharSetItemAcc, OAccessibleComponentHelper, OAccessibleHelper_Base_2 )
// -----------------------------------------------------------------------------

void SvxShowCharSetItemAcc::ParentDestroyed()
{
    const ::osl::MutexGuard aGuard( GetMutex() );
    mpParent = NULL;
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvxShowCharSetItemAcc::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    return 0;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    throw lang::IndexOutOfBoundsException();
    return uno::Reference< accessibility::XAccessible >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleParent()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    return mpParent->m_pParent;
}

// -----------------------------------------------------------------------------

sal_Int16 SAL_CALL SvxShowCharSetItemAcc::getAccessibleRole()
    throw (uno::RuntimeException)
{
    return accessibility::AccessibleRole::LABEL;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    String sDescription = SVX_RESSTR( RID_SVXSTR_CHARACTER_CODE );

    sal_Unicode c = mpParent->maText.GetChar(0);
    char buf[16] = "0x0000";
    sal_Unicode c_Shifted = c;
    for( int i = 0; i < 4; ++i )
    {
        char h = c_Shifted & 0x0F;
        buf[5-i] = (h > 9) ? (h - 10 + 'A') : (h + '0');
        c_Shifted >>= 4;
    }
    if( c < 256 )
        snprintf( buf+6, 10, " (%d)", c );
    sDescription.AppendAscii(buf);

    return sDescription;
}

// -----------------------------------------------------------------------------

::rtl::OUString SAL_CALL SvxShowCharSetItemAcc::getAccessibleName()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();
    String aRet;

    if( mpParent )
    {
        aRet = mpParent->maText;

        if( !aRet.Len() )
            aRet = getAccessibleDescription();
    }

    return aRet;
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleRelationSet > SAL_CALL SvxShowCharSetItemAcc::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessibleRelationSet >();
}

// -----------------------------------------------------------------------------

uno::Reference< accessibility::XAccessibleStateSet > SAL_CALL SvxShowCharSetItemAcc::getAccessibleStateSet()
    throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );
    ensureAlive();

    ::utl::AccessibleStateSetHelper*    pStateSet = new ::utl::AccessibleStateSetHelper;

    if( mpParent )
    {
        // SELECTABLE
        pStateSet->AddState( accessibility::AccessibleStateType::SELECTABLE );
        pStateSet->AddState( accessibility::AccessibleStateType::FOCUSABLE );

        // SELECTED
        if( mpParent->mrParent.GetSelectIndexId() == mpParent->mnId )
        {
            pStateSet->AddState( accessibility::AccessibleStateType::SELECTED );
               pStateSet->AddState( accessibility::AccessibleStateType::FOCUSED );
        }
        if ( mpParent->mnId >= mpParent->mrParent.FirstInView() && mpParent->mnId <= mpParent->mrParent.LastInView() )
            pStateSet->AddState( AccessibleStateType::VISIBLE );
        pStateSet->AddState( AccessibleStateType::TRANSIENT );
    }

    return pStateSet;
}

// -----------------------------------------------------------------------------
void SAL_CALL SvxShowCharSetItemAcc::grabFocus()
    throw (uno::RuntimeException)
{
    // nothing to do
}

// -----------------------------------------------------------------------------

uno::Any SAL_CALL SvxShowCharSetItemAcc::getAccessibleKeyBinding()
    throw (uno::RuntimeException)
{
    return uno::Any();
}
// -----------------------------------------------------------------------------
awt::Rectangle SAL_CALL SvxShowCharSetItemAcc::implGetBounds(  ) throw (RuntimeException)
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
// -----------------------------------------------------------------------------
uno::Reference< accessibility::XAccessible > SAL_CALL SvxShowCharSetItemAcc::getAccessibleAt( const awt::Point& aPoint )
    throw (uno::RuntimeException)
{
    return uno::Reference< accessibility::XAccessible >();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( mpParent )
    {
        if ( mpParent->IsControlForeground() )
            nColor = mpParent->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( mpParent->IsControlFont() )
                aFont = mpParent->GetControlFont();
            else
                aFont = mpParent->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetVirtualAcc::getBackground(  ) throw (RuntimeException)
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
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pParent )
        nColor = m_pParent->getForeground();
    return nColor;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShowCharSetAcc::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this  );
    sal_Int32 nColor = 0;
    if ( m_pParent )
        nColor = m_pParent->getBackground();
    return nColor;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
} // namespace svx
// -----------------------------------------------------------------------------


