 /*************************************************************************
 *
 *  $RCSfile: acccell.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 16:09:49 $
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


#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _CRSRSH_HXX
#include "crsrsh.hxx"
#endif
#ifndef _VISCRS_HXX
#include "viscrs.hxx"
#endif
#ifndef _ACCFRMOBJSLIST_HXX
#include <accfrmobjslist.hxx>
#endif
#ifndef _FRMFMT_HXX
#include "frmfmt.hxx"
#endif
#ifndef _CELLATR_HXX
#include "cellatr.hxx"
#endif

#ifndef _ACCMAP_HXX
#include "accmap.hxx"
#endif
#ifndef _ACCCELL_HXX
#include <acccell.hxx>
#endif

#include <limits.h>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::rtl;

const sal_Char sServiceName[] = "com.sun.star.table.AccessibleCellView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleCellView";

sal_Bool SwAccessibleCell::IsSelected()
{
    sal_Bool bRet = sal_False;

    DBG_ASSERT( GetMap(), "no map?" );
    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
    if( pVSh->ISA( SwCrsrShell ) )
    {
        const SwCrsrShell *pCSh = static_cast< const SwCrsrShell * >( pVSh );
        if( pCSh->IsTableMode() )
        {
            const SwCellFrm *pCFrm =
                static_cast< const SwCellFrm * >( GetFrm() );
            SwTableBox *pBox =
                const_cast< SwTableBox *>( pCFrm->GetTabBox() ); //SVPtrArr!
            bRet = pCSh->GetTableCrsr()->GetBoxes().Seek_Entry( pBox );
        }
    }

    return bRet;
}

void SwAccessibleCell::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // SELECTABLE
    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
    if( pVSh->ISA( SwCrsrShell ) )
        rStateSet.AddState( AccessibleStateType::SELECTABLE );

    // SELECTED
    if( IsSelected() )
    {
        rStateSet.AddState( AccessibleStateType::SELECTED );
        ASSERT( bIsSelected, "bSelected out of sync" );
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
}

SwAccessibleCell::SwAccessibleCell(
        SwAccessibleMap *pMap,
        const SwCellFrm *pCellFrm   ) :
    SwAccessibleContext( pMap, AccessibleRole::TABLE_CELL, pCellFrm ),
    bIsSelected( sal_False )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sName( pCellFrm->GetTabBox()->GetName() );
    SetName( sName );

    bIsSelected = IsSelected();
}

sal_Bool SwAccessibleCell::_InvalidateMyCursorPos()
{
    sal_Bool bNew = IsSelected();
    sal_Bool bOld;
    {
        vos::OGuard aGuard( aMutex );
        bOld = bIsSelected;
        bIsSelected = bNew;
    }
    if( bNew )
    {
        // remember that object as the one that has the caret. This is
        // neccessary to notify that object if the cursor leaves it.
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    sal_Bool bChanged = bOld != bNew;
    if( bChanged )
        FireStateChangedEvent( AccessibleStateType::SELECTED, bNew );

    return bChanged;
}

sal_Bool SwAccessibleCell::_InvalidateChildrenCursorPos( const SwFrm *pFrm )
{
    sal_Bool bChanged = sal_False;

    const SwFrmOrObjSList aVisList( GetVisArea(), pFrm );
    SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        if( pLower )
        {
            if( rLower.IsAccessible( GetMap()->GetShell()->IsPreView() )  )
            {
                ::vos::ORef< SwAccessibleContext > xAccImpl(
                    GetMap()->GetContextImpl( pLower, sal_False ) );
                if( xAccImpl.isValid() )
                {
                    ASSERT( xAccImpl->GetFrm()->IsCellFrm(),
                             "table child is not a cell frame" )
                    bChanged |= static_cast< SwAccessibleCell *>(
                            xAccImpl.getBodyPtr() )->_InvalidateMyCursorPos();
                }
                else
                    bChanged = sal_True; // If the context is not know we
                                         // don't know whether the selection
                                         // changed or not.
            }
            else
            {
                // This is a box with sub rows.
                bChanged |= _InvalidateChildrenCursorPos( pLower );
            }
        }
        ++aIter;
    }

    return bChanged;
}

void SwAccessibleCell::_InvalidateCursorPos()
{

    const SwFrm *pParent = GetParent( GetFrm(), IsInPagePreview() );
    ASSERT( pParent->IsTabFrm(), "parent is not a tab frame" );
    const SwTabFrm *pTabFrm = static_cast< const SwTabFrm * >( pParent );
    if( pTabFrm->IsFollow() )
        pTabFrm = pTabFrm->FindMaster();

    while( pTabFrm )
    {
        sal_Bool bChanged = _InvalidateChildrenCursorPos( pTabFrm );
        if( bChanged )
        {
            ::vos::ORef< SwAccessibleContext > xAccImpl(
                GetMap()->GetContextImpl( pTabFrm, sal_False ) );
            if( xAccImpl.isValid() )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
                xAccImpl->FireAccessibleEvent( aEvent );
            }
        }

        pTabFrm = pTabFrm->GetFollow();
    }
}

sal_Bool SwAccessibleCell::HasCursor()
{
    vos::OGuard aGuard( aMutex );
    return bIsSelected;
}

SwAccessibleCell::~SwAccessibleCell()
{
}

OUString SAL_CALL SwAccessibleCell::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return GetName();
}

OUString SAL_CALL SwAccessibleCell::getImplementationName()
        throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleCell::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleCell::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

void SwAccessibleCell::Dispose( sal_Bool bRecursive )
{
    const SwFrm *pParent = GetParent( GetFrm(), IsInPagePreview() );
    ::vos::ORef< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, sal_False ) );
    if( xAccImpl.isValid() )
        xAccImpl->DisposeChild( GetFrm(), bRecursive );
    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleCell::InvalidatePosOrSize( const SwRect& rOldBox )
{
    const SwFrm *pParent = GetParent( GetFrm(), IsInPagePreview() );
    ::vos::ORef< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, sal_False ) );
    if( xAccImpl.isValid() )
        xAccImpl->InvalidateChildPosOrSize( GetFrm(), rOldBox );
    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}


// =====  XAccessibleInterface  ===========================================

Any SwAccessibleCell::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    if ( rType == ::getCppuType((Reference<XAccessibleValue> *)0) )
    {
        Reference<XAccessibleValue> xValue = this;
        Any aRet;
        aRet <<= xValue;
        return aRet;
    }
    else
    {
        return SwAccessibleContext::queryInterface( rType );
    }
}

//====== XTypeProvider ====================================================
Sequence< Type > SAL_CALL SwAccessibleCell::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< Reference< XAccessibleValue > * >( 0 ) );

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleCell::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

// =====  XAccessibleValue  ===============================================

SwFrmFmt* SwAccessibleCell::GetTblBoxFormat() const
{
    DBG_ASSERT( GetFrm() != NULL, "no frame?" );
    DBG_ASSERT( GetFrm()->IsCellFrm(), "no cell frame?" );

    const SwCellFrm* pCellFrm = static_cast<const SwCellFrm*>( GetFrm() );
    return pCellFrm->GetTabBox()->GetFrmFmt();
}


Any SwAccessibleCell::getCurrentValue( )
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleValue );

    Any aAny;
    aAny <<= GetTblBoxFormat()->GetTblBoxValue().GetValue();
    return aAny;
}

sal_Bool SwAccessibleCell::setCurrentValue( const Any& aNumber )
    throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleValue );

    double fValue;
    sal_Bool bValid = (aNumber >>= fValue);
    if( bValid )
    {
        SwTblBoxValue aValue( fValue );
        GetTblBoxFormat()->SetAttr( aValue );
    }
    return bValid;
}

Any SwAccessibleCell::getMaximumValue( )
    throw( RuntimeException )
{
    Any aAny;
    aAny <<= DBL_MAX;
    return aAny;
}

Any SwAccessibleCell::getMinimumValue(  )
    throw( RuntimeException )
{
    Any aAny;
    aAny <<= -DBL_MAX;
    return aAny;
}
