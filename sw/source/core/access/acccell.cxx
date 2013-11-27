/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <vos/mutex.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/uuid.h>
#include <vcl/svapp.hxx>
#include <cellfrm.hxx>
#include <tabfrm.hxx>
#include <swtable.hxx>
#include "crsrsh.hxx"
#include "viscrs.hxx"
#include <accfrmobj.hxx>
#include <accfrmobjslist.hxx>
#include "frmfmt.hxx"
#include "cellatr.hxx"
#include "accmap.hxx"
#include <acccell.hxx>

#ifndef _STLP_CFLOAT
#include <cfloat>
#endif

#include <limits.h>

//IAccessibility2 Implementation 2009-----
#include <ndtxt.hxx>
#include <editeng/brshitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include "acctable.hxx"
//-----IAccessibility2 Implementation 2009

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;
using namespace sw::access;

const sal_Char sServiceName[] = "com.sun.star.table.AccessibleCellView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleCellView";

sal_Bool SwAccessibleCell::IsSelected()
{
    sal_Bool bRet = sal_False;

    DBG_ASSERT( GetMap(), "no map?" );
    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
    const SwCrsrShell *pCSh = dynamic_cast< const SwCrsrShell * >( pVSh );

    if( pCSh )
    {
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

void SwAccessibleCell::GetStates( ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // SELECTABLE
    const ViewShell *pVSh = GetMap()->GetShell();
    DBG_ASSERT( pVSh, "no shell?" );
    if( dynamic_cast< const SwCrsrShell* >(pVSh) )
        rStateSet.AddState( AccessibleStateType::SELECTABLE );
    //IAccessibility2 Implementation 2009-----
    //Solution:Add resizable state to table cell.
    rStateSet.AddState( AccessibleStateType::RESIZABLE );
    //-----IAccessibility2 Implementation 2009

    // SELECTED
    if( IsSelected() )
    {
        rStateSet.AddState( AccessibleStateType::SELECTED );
        ASSERT( bIsSelected, "bSelected out of sync" );
        ::vos::ORef < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
}

SwAccessibleCell::SwAccessibleCell( SwAccessibleMap *pInitMap,
                                    const SwCellFrm *pCellFrm )
    : SwAccessibleContext( pInitMap, AccessibleRole::TABLE_CELL, pCellFrm )
    , aSelectionHelper( *this )
    , bIsSelected( sal_False )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    OUString sBoxName( pCellFrm->GetTabBox()->GetName() );
    SetName( sBoxName );

    bIsSelected = IsSelected();

    //IAccessibility2 Implementation 2009-----
    //Need not assign the pointer of accessible table object to m_pAccTable,
    //for it already done in SwAccessibleCell::GetTable(); Former codes:
    //m_pAccTable= GetTable();
    GetTable();
    //-----IAccessibility2 Implementation 2009
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
    //IAccessibility2 Implementation 2009-----
    if( bChanged )
    {
        FireStateChangedEvent( AccessibleStateType::SELECTED, bNew );
        if (m_pAccTable)
        {
            m_pAccTable->AddSelectionCell(this,bNew);
        }
    }
    //-----IAccessibility2 Implementation 2009
    return bChanged;
}

sal_Bool SwAccessibleCell::_InvalidateChildrenCursorPos( const SwFrm *pFrm )
{
    sal_Bool bChanged = sal_False;

    const SwAccessibleChildSList aVisList( GetVisArea(), *pFrm, *GetMap() );
    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
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
                    //IAccessibility2 Implementation 2009-----
                    bChanged = static_cast< SwAccessibleCell *>(
                            xAccImpl.getBodyPtr() )->_InvalidateMyCursorPos();
                    //-----IAccessibility2 Implementation 2009
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
    //IAccessibility2 Implementation 2009-----
    if (IsSelected())
    {
        const SwAccessibleChild aChild( GetChild( *(GetMap()), 0 ) );
        if( aChild.IsValid()  && aChild.GetSwFrm() )
        {
            ::vos::ORef < SwAccessibleContext > xChildImpl( GetMap()->GetContextImpl( aChild.GetSwFrm())  );
            if(xChildImpl.isValid())
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::STATE_CHANGED;
                aEvent.NewValue<<=AccessibleStateType::FOCUSED;
                xChildImpl->FireAccessibleEvent( aEvent );
            }
        }
    }

    const SwFrm *pParent = GetParent( SwAccessibleChild(GetFrm()), IsInPagePreview() );
    ASSERT( pParent->IsTabFrm(), "parent is not a tab frame" );
    const SwTabFrm *pTabFrm = static_cast< const SwTabFrm * >( pParent );
    if( pTabFrm->IsFollow() )
        pTabFrm = pTabFrm->FindMaster();

    while( pTabFrm )
    {
                _InvalidateChildrenCursorPos( pTabFrm );
/*
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
*/
        pTabFrm = pTabFrm->GetFollow();
    }
    if (m_pAccTable)
    {
        m_pAccTable->FireSelectionEvent();
    }
    //-----IAccessibility2 Implementation 2009
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
        throw (uno::RuntimeException)
{
    return GetName();
}

OUString SAL_CALL SwAccessibleCell::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleCell::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleCell::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

void SwAccessibleCell::Dispose( sal_Bool bRecursive )
{
    const SwFrm *pParent = GetParent( SwAccessibleChild(GetFrm()), IsInPagePreview() );
    ::vos::ORef< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, sal_False ) );
    if( xAccImpl.isValid() )
        xAccImpl->DisposeChild( SwAccessibleChild(GetFrm()), bRecursive );
    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleCell::InvalidatePosOrSize( const SwRect& rOldBox )
{
    const SwFrm *pParent = GetParent( SwAccessibleChild(GetFrm()), IsInPagePreview() );
    ::vos::ORef< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, sal_False ) );
    if( xAccImpl.isValid() )
        xAccImpl->InvalidateChildPosOrSize( SwAccessibleChild(GetFrm()), rOldBox );
    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}


// =====  XAccessibleInterface  ===========================================

uno::Any SwAccessibleCell::queryInterface( const uno::Type& rType )
    throw( uno::RuntimeException )
{
    //IAccessibility2 Implementation 2009-----
    if (rType == ::getCppuType((const uno::Reference<XAccessibleExtendedAttributes>*)0))
    {
        uno::Any aR;
        aR <<= uno::Reference<XAccessibleExtendedAttributes>(this);
        return aR;
    }

    if (rType == ::getCppuType((const uno::Reference<XAccessibleSelection>*)0))
    {
        uno::Any aR;
        aR <<= uno::Reference<XAccessibleSelection>(this);
        return aR;
    }
    //-----IAccessibility2 Implementation 2009
    if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleValue > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleValue> xValue = this;
        uno::Any aRet;
        aRet <<= xValue;
        return aRet;
    }
    else
    {
        return SwAccessibleContext::queryInterface( rType );
    }
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleCell::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleValue > * >( 0 ) );

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleCell::getImplementationId()
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
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

//IAccessibility2 Implementation 2009-----
//Implement TableCell currentValue
uno::Any SwAccessibleCell::getCurrentValue( )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleValue );

    uno::Any aAny;

    const SwCellFrm* pCellFrm = static_cast<const SwCellFrm*>( GetFrm() );
    const SwStartNode *pSttNd = pCellFrm->GetTabBox()->GetSttNd();
    if( pSttNd )
    {
        ::rtl::OUString strRet;
        SwNodeIndex aCntntIdx( *pSttNd, 0 );
        SwCntntNode* pCNd=NULL;
        for(int nIndex = 0 ;
            0 != ( pCNd = pSttNd->GetNodes().GoNext( &aCntntIdx ) ) &&
            aCntntIdx.GetIndex() < pSttNd->EndOfSectionIndex();
            ++nIndex )
        {
            if(pCNd && pCNd->IsTxtNode())
            {
                if (0 != nIndex)
                {
                    strRet += ::rtl::OUString::createFromAscii(" ");
                }
                strRet +=((SwTxtNode*)pCNd)->GetTxt();
            }
        }
        aAny <<= strRet;
    }
    return aAny;
}
//-----IAccessibility2 Implementation 2009

sal_Bool SwAccessibleCell::setCurrentValue( const uno::Any& aNumber )
    throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    CHECK_FOR_DEFUNC( XAccessibleValue );

    double fValue = 0;
    sal_Bool bValid = (aNumber >>= fValue);
    if( bValid )
    {
        SwTblBoxValue aValue( fValue );
        GetTblBoxFormat()->SetFmtAttr( aValue );
    }
    return bValid;
}

uno::Any SwAccessibleCell::getMaximumValue( )
    throw( uno::RuntimeException )
{
    uno::Any aAny;
    aAny <<= DBL_MAX;
    return aAny;
}

uno::Any SwAccessibleCell::getMinimumValue(  )
    throw( uno::RuntimeException )
{
    uno::Any aAny;
    aAny <<= -DBL_MAX;
    return aAny;
}

//IAccessibility2 Implementation 2009-----
::rtl::OUString ReplaceOneChar(::rtl::OUString oldOUString, ::rtl::OUString replacedChar, ::rtl::OUString replaceStr)
{
    int iReplace = -1;
    iReplace = oldOUString.lastIndexOf(replacedChar);
    if (iReplace > -1)
    {
        for(;iReplace>-1;)
        {
            oldOUString = oldOUString.replaceAt(iReplace,1, replaceStr);
            iReplace=oldOUString.lastIndexOf(replacedChar,iReplace);
        }
    }
    return oldOUString;
}
::rtl::OUString ReplaceFourChar(::rtl::OUString oldOUString)
{
    oldOUString = ReplaceOneChar(oldOUString,OUString::createFromAscii("\\"),OUString::createFromAscii("\\\\"));
    oldOUString = ReplaceOneChar(oldOUString,::rtl::OUString::createFromAscii(";"),::rtl::OUString::createFromAscii("\\;"));
    oldOUString = ReplaceOneChar(oldOUString,::rtl::OUString::createFromAscii("="),::rtl::OUString::createFromAscii("\\="));
    oldOUString = ReplaceOneChar(oldOUString,::rtl::OUString::createFromAscii(","),::rtl::OUString::createFromAscii("\\,"));
    oldOUString = ReplaceOneChar(oldOUString,::rtl::OUString::createFromAscii(":"),::rtl::OUString::createFromAscii("\\:"));
    return oldOUString;
}

::com::sun::star::uno::Any SAL_CALL SwAccessibleCell::getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any strRet;
    SwFrmFmt *pFrmFmt = GetTblBoxFormat();
    DBG_ASSERT(pFrmFmt,"Must be Valid");

    const SwTblBoxFormula& tbl_formula = pFrmFmt->GetTblBoxFormula();

    ::rtl::OUString strFormula = ReplaceFourChar(tbl_formula.GetFormula());
    ::rtl::OUString strFor = ::rtl::OUString::createFromAscii("Formula:");
    strFor += strFormula;
    strFor += ::rtl::OUString::createFromAscii(";") ;
    strRet <<= strFor;

    return strRet;
}

sal_Int32 SAL_CALL SwAccessibleCell::getBackground()
        throw (::com::sun::star::uno::RuntimeException)
{
    const SvxBrushItem &rBack = GetFrm()->GetAttrSet()->GetBackground();
    sal_uInt32 crBack = rBack.GetColor().GetColor();

    if (COL_AUTO == crBack)
    {
        uno::Reference<XAccessible> xAccDoc = getAccessibleParent();
        if (xAccDoc.is())
        {
            uno::Reference<XAccessibleComponent> xCompoentDoc(xAccDoc, uno::UNO_QUERY);
            if (xCompoentDoc.is())
            {
                crBack = (sal_uInt32)xCompoentDoc->getBackground();
            }
        }
    }
    return crBack;
}

//=====  XAccessibleSelection  ============================================
void SwAccessibleCell::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleCell::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleCell::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    aSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleCell::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleCell::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    return aSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleCell::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

void SwAccessibleCell::deselectAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    aSelectionHelper.deselectAccessibleChild(nSelectedChildIndex);
}

SwAccessibleTable *SwAccessibleCell::GetTable()
{
    if (!m_pAccTable)
    {
        if (!xTableReference.is())
        {
            xTableReference = getAccessibleParent();
        #ifdef OSL_DEBUG_LEVEL
            uno::Reference<XAccessibleContext> xContextTable(xTableReference, uno::UNO_QUERY);
            OSL_ASSERT(xContextTable.is() && xContextTable->getAccessibleRole() == AccessibleRole::TABLE);
        #endif
            //SwAccessibleTable aTable = *(static_cast<SwAccessibleTable *>(xTable.get()));
        }
        m_pAccTable = static_cast<SwAccessibleTable *>(xTableReference.get());
    }
    return m_pAccTable;
}
//-----IAccessibility2 Implementation 2009
