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

#include <osl/mutex.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
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

#include <cfloat>
#include <limits.h>

#include <ndtxt.hxx>
#include <editeng/brushitem.hxx>
#include <swatrset.hxx>
#include <frmatr.hxx>
#include "acctable.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace sw::access;

const sal_Char sServiceName[] = "com.sun.star.table.AccessibleCellView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleCellView";

bool SwAccessibleCell::IsSelected()
{
    bool bRet = false;

    assert(GetMap());
    const SwViewShell *pVSh = GetMap()->GetShell();
    assert(pVSh);
    if( dynamic_cast<const SwCursorShell*>( pVSh) !=  nullptr )
    {
        const SwCursorShell *pCSh = static_cast< const SwCursorShell * >( pVSh );
        if( pCSh->IsTableMode() )
        {
            const SwCellFrame *pCFrame =
                static_cast< const SwCellFrame * >( GetFrame() );
            SwTableBox *pBox =
                const_cast< SwTableBox *>( pCFrame->GetTabBox() );
            SwSelBoxes const& rBoxes(pCSh->GetTableCursor()->GetSelectedBoxes());
            bRet = rBoxes.find(pBox) != rBoxes.end();
        }
    }

    return bRet;
}

void SwAccessibleCell::GetStates( ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // SELECTABLE
    const SwViewShell *pVSh = GetMap()->GetShell();
    assert(pVSh);
    if( dynamic_cast<const SwCursorShell*>( pVSh) !=  nullptr )
        rStateSet.AddState( AccessibleStateType::SELECTABLE );
    //Add resizable state to table cell.
    rStateSet.AddState( AccessibleStateType::RESIZABLE );

    // SELECTED
    if( IsSelected() )
    {
        rStateSet.AddState( AccessibleStateType::SELECTED );
        assert(bIsSelected && "bSelected out of sync");
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }
}

SwAccessibleCell::SwAccessibleCell( SwAccessibleMap *pInitMap,
                                    const SwCellFrame *pCellFrame )
    : SwAccessibleContext( pInitMap, AccessibleRole::TABLE_CELL, pCellFrame )
    , aSelectionHelper( *this )
    , bIsSelected( false )
{
    SolarMutexGuard aGuard;
    OUString sBoxName( pCellFrame->GetTabBox()->GetName() );
    SetName( sBoxName );

    bIsSelected = IsSelected();

    css::uno::Reference<css::accessibility::XAccessible> xTableReference(
        getAccessibleParent());
    css::uno::Reference<css::accessibility::XAccessibleContext> xContextTable(
        xTableReference, css::uno::UNO_QUERY);
    SAL_WARN_IF(
        (!xContextTable.is()
         || xContextTable->getAccessibleRole() != AccessibleRole::TABLE),
        "sw.core", "bad accessible context");
    m_pAccTable = static_cast<SwAccessibleTable *>(xTableReference.get());
}

bool SwAccessibleCell::_InvalidateMyCursorPos()
{
    bool bNew = IsSelected();
    bool bOld;
    {
        osl::MutexGuard aGuard( m_Mutex );
        bOld = bIsSelected;
        bIsSelected = bNew;
    }
    if( bNew )
    {
        // remember that object as the one that has the caret. This is
        // necessary to notify that object if the cursor leaves it.
        ::rtl::Reference < SwAccessibleContext > xThis( this );
        GetMap()->SetCursorContext( xThis );
    }

    bool bChanged = bOld != bNew;
    if( bChanged )
    {
        FireStateChangedEvent( AccessibleStateType::SELECTED, bNew );
        if (m_pAccTable.is())
        {
            m_pAccTable->AddSelectionCell(this,bNew);
        }
    }
    return bChanged;
}

bool SwAccessibleCell::_InvalidateChildrenCursorPos( const SwFrame *pFrame )
{
    bool bChanged = false;

    const SwAccessibleChildSList aVisList( GetVisArea(), *pFrame, *GetMap() );
    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrame *pLower = rLower.GetSwFrame();
        if( pLower )
        {
            if( rLower.IsAccessible( GetMap()->GetShell()->IsPreview() )  )
            {
                ::rtl::Reference< SwAccessibleContext > xAccImpl(
                    GetMap()->GetContextImpl( pLower, false ) );
                if( xAccImpl.is() )
                {
                    assert(xAccImpl->GetFrame()->IsCellFrame());
                    bChanged = static_cast< SwAccessibleCell *>(
                            xAccImpl.get() )->_InvalidateMyCursorPos();
                }
                else
                    bChanged = true; // If the context is not know we
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
    if (IsSelected())
    {
        const SwAccessibleChild aChild( GetChild( *(GetMap()), 0 ) );
        if( aChild.IsValid()  && aChild.GetSwFrame() )
        {
            ::rtl::Reference < SwAccessibleContext > xChildImpl( GetMap()->GetContextImpl( aChild.GetSwFrame())  );
            if (xChildImpl.is())
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::STATE_CHANGED;
                aEvent.NewValue <<= AccessibleStateType::FOCUSED;
                xChildImpl->FireAccessibleEvent( aEvent );
            }
        }
    }

    const SwFrame *pParent = GetParent( SwAccessibleChild(GetFrame()), IsInPagePreview() );
    assert(pParent->IsTabFrame());
    const SwTabFrame *pTabFrame = static_cast< const SwTabFrame * >( pParent );
    if( pTabFrame->IsFollow() )
        pTabFrame = pTabFrame->FindMaster();

    while( pTabFrame )
    {
        _InvalidateChildrenCursorPos( pTabFrame );
        pTabFrame = pTabFrame->GetFollow();
    }
    if (m_pAccTable.is())
    {
        m_pAccTable->FireSelectionEvent();
    }
}

bool SwAccessibleCell::HasCursor()
{
    osl::MutexGuard aGuard( m_Mutex );
    return bIsSelected;
}

SwAccessibleCell::~SwAccessibleCell()
{
}

OUString SAL_CALL SwAccessibleCell::getAccessibleDescription()
        throw (uno::RuntimeException, std::exception)
{
    return GetName();
}

OUString SAL_CALL SwAccessibleCell::getImplementationName()
        throw( uno::RuntimeException, std::exception )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleCell::supportsService(const OUString& sTestServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sTestServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleCell::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = sServiceName;
    pArray[1] = sAccessibleServiceName;
    return aRet;
}

void SwAccessibleCell::Dispose( bool bRecursive )
{
    const SwFrame *pParent = GetParent( SwAccessibleChild(GetFrame()), IsInPagePreview() );
    ::rtl::Reference< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, false ) );
    if( xAccImpl.is() )
        xAccImpl->DisposeChild( SwAccessibleChild(GetFrame()), bRecursive );
    SwAccessibleContext::Dispose( bRecursive );
}

void SwAccessibleCell::InvalidatePosOrSize( const SwRect& rOldBox )
{
    const SwFrame *pParent = GetParent( SwAccessibleChild(GetFrame()), IsInPagePreview() );
    ::rtl::Reference< SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pParent, false ) );
    if( xAccImpl.is() )
        xAccImpl->InvalidateChildPosOrSize( SwAccessibleChild(GetFrame()), rOldBox );
    SwAccessibleContext::InvalidatePosOrSize( rOldBox );
}

// XAccessibleInterface

uno::Any SwAccessibleCell::queryInterface( const uno::Type& rType )
    throw( uno::RuntimeException, std::exception )
{
    if (rType == cppu::UnoType<XAccessibleExtendedAttributes>::get())
    {
        uno::Any aR;
        aR <<= uno::Reference<XAccessibleExtendedAttributes>(this);
        return aR;
    }

    if (rType == cppu::UnoType<XAccessibleSelection>::get())
    {
        uno::Any aR;
        aR <<= uno::Reference<XAccessibleSelection>(this);
        return aR;
    }
    if ( rType == ::cppu::UnoType<XAccessibleValue>::get() )
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

// XTypeProvider
uno::Sequence< uno::Type > SAL_CALL SwAccessibleCell::getTypes()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleContext::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::cppu::UnoType<XAccessibleValue>::get();

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleCell::getImplementationId()
        throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XAccessibleValue

SwFrameFormat* SwAccessibleCell::GetTableBoxFormat() const
{
    assert(GetFrame());
    assert(GetFrame()->IsCellFrame());

    const SwCellFrame* pCellFrame = static_cast<const SwCellFrame*>( GetFrame() );
    return pCellFrame->GetTabBox()->GetFrameFormat();
}

//Implement TableCell currentValue
uno::Any SwAccessibleCell::getCurrentValue( )
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleValue );

    return uno::Any( GetTableBoxFormat()->GetTableBoxValue().GetValue() );
}

sal_Bool SwAccessibleCell::setCurrentValue( const uno::Any& aNumber )
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleValue );

    double fValue = 0;
    bool bValid = (aNumber >>= fValue);
    if( bValid )
    {
        SwTableBoxValue aValue( fValue );
        GetTableBoxFormat()->SetFormatAttr( aValue );
    }
    return bValid;
}

uno::Any SwAccessibleCell::getMaximumValue( )
    throw( uno::RuntimeException, std::exception )
{
    return uno::Any(DBL_MAX);
}

uno::Any SwAccessibleCell::getMinimumValue(  )
    throw( uno::RuntimeException, std::exception )
{
    return uno::Any(-DBL_MAX);
}

static OUString ReplaceOneChar(const OUString& oldOUString, const OUString& replacedChar, const OUString& replaceStr)
{
    int iReplace = oldOUString.lastIndexOf(replacedChar);
    OUString aRet = oldOUString;
    while(iReplace > -1)
    {
        aRet = aRet.replaceAt(iReplace,1, replaceStr);
        iReplace = aRet.lastIndexOf(replacedChar,iReplace);
    }
    return aRet;
}

static OUString ReplaceFourChar(const OUString& oldOUString)
{
    OUString aRet = ReplaceOneChar(oldOUString,"\\","\\\\");
    aRet = ReplaceOneChar(aRet,";","\\;");
    aRet = ReplaceOneChar(aRet,"=","\\=");
    aRet = ReplaceOneChar(aRet,",","\\,");
    aRet = ReplaceOneChar(aRet,":","\\:");
    return aRet;
}

css::uno::Any SAL_CALL SwAccessibleCell::getExtendedAttributes()
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    css::uno::Any strRet;
    SwFrameFormat *pFrameFormat = GetTableBoxFormat();
    assert(pFrameFormat);

    const SwTableBoxFormula& tbl_formula = pFrameFormat->GetTableBoxFormula();

    OUString strFormula = ReplaceFourChar(tbl_formula.GetFormula());
    OUString strFor("Formula:");
    strFor += strFormula;
    strFor += ";" ;
    strRet <<= strFor;

    return strRet;
}

sal_Int32 SAL_CALL SwAccessibleCell::getBackground()
        throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    const SvxBrushItem &rBack = GetFrame()->GetAttrSet()->GetBackground();
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

// XAccessibleSelection
void SwAccessibleCell::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleCell::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleCell::clearAccessibleSelection(  )
    throw ( uno::RuntimeException, std::exception )
{
}

void SwAccessibleCell::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException, std::exception )
{
    aSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleCell::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException, std::exception )
{
    return aSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleCell::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

void SwAccessibleCell::deselectAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception )
{
    aSelectionHelper.deselectAccessibleChild(nSelectedChildIndex);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
