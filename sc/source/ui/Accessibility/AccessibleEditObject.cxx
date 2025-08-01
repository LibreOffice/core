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

#include <sal/config.h>

#include <memory>
#include <utility>

#include <AccessibleEditObject.hxx>
#include <AccessibleText.hxx>
#include <editsrc.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <sfx2/objsh.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <unonames.hxx>
#include <document.hxx>
#include <AccessibleDocument.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

using ::com::sun::star::lang::IndexOutOfBoundsException;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

ScAccessibleEditObject::ScAccessibleEditObject(const uno::Reference<XAccessible>& rxParent,
                                               EditView* pEditView, vcl::Window* pWin,
                                               const OUString& rName, const OUString& rDescription,
                                               EditObjectType eObjectType)
    : ImplInheritanceHelper(rxParent, AccessibleRole::TEXT_FRAME)
    , mpEditView(pEditView)
    , mpWindow(pWin)
    , mpTextWnd(nullptr)
    , meObjectType(eObjectType)
    , mbHasFocus(false)
    , m_pScDoc(nullptr)
{
    InitAcc(rxParent, pEditView, rName, rDescription);
}

ScAccessibleEditObject::ScAccessibleEditObject(EditObjectType eObjectType)
    : ImplInheritanceHelper(nullptr, AccessibleRole::TEXT_FRAME)
    , mpEditView(nullptr)
    , mpWindow(nullptr)
    , mpTextWnd(nullptr)
    , meObjectType(eObjectType)
    , mbHasFocus(false)
    , m_pScDoc(nullptr)
{
}

void ScAccessibleEditObject::InitAcc(
        const uno::Reference<XAccessible>& rxParent,
        EditView* pEditView,
        const OUString& rName,
        const OUString& rDescription)
{
    SetParent(rxParent);
    mpEditView = pEditView;

    CreateTextHelper();
    SetName(rName);
    SetDescription(rDescription);
    if( meObjectType == CellInEditMode)
    {
        const ScAccessibleDocument *pAccDoc = static_cast<ScAccessibleDocument*>(rxParent.get());
        if (pAccDoc)
        {
            m_pScDoc = pAccDoc->GetDocument();
            m_curCellAddress =pAccDoc->GetCurCellAddress();
        }
    }
}

ScAccessibleEditObject::~ScAccessibleEditObject()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call of dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleEditObject::disposing()
{
    SolarMutexGuard aGuard;
    mpTextHelper.reset();

    ScAccessibleContextBase::disposing();
}

void ScAccessibleEditObject::LostFocus()
{
    mbHasFocus = false;
    if (mpTextHelper)
        mpTextHelper->SetFocus(false);
    CommitFocusLost();
}

void ScAccessibleEditObject::GotFocus()
{
    mbHasFocus = true;
    CommitFocusGained();
    if (mpTextHelper)
        mpTextHelper->SetFocus();
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleEditObject::getAccessibleAtPoint(
        const awt::Point& rPoint )
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
        SolarMutexGuard aGuard;
        ensureAlive();

        CreateTextHelper();

        xRet = mpTextHelper->GetAt(rPoint);
    }

    return xRet;
}

AbsoluteScreenPixelRectangle ScAccessibleEditObject::GetBoundingBoxOnScreen()
{
    AbsoluteScreenPixelRectangle aScreenBounds;

    if ( mpWindow )
    {
        if ( meObjectType == CellInEditMode )
        {
            if (mpEditView)
            {
                MapMode aMapMode(mpEditView->getEditEngine().GetRefMapMode());
                tools::Rectangle aScreenBoundsLog = mpWindow->LogicToPixel( mpEditView->GetOutputArea(), aMapMode );
                Point aCellLoc = aScreenBoundsLog.TopLeft();
                AbsoluteScreenPixelRectangle aWindowRect = mpWindow->GetWindowExtentsAbsolute();
                AbsoluteScreenPixelPoint aWindowLoc = aWindowRect.TopLeft();
                AbsoluteScreenPixelPoint aPos( aCellLoc.getX() + aWindowLoc.getX(), aCellLoc.getY() + aWindowLoc.getY() );
                aScreenBounds = AbsoluteScreenPixelRectangle( aPos, aScreenBoundsLog.GetSize() );
            }
        }
        else
        {
            aScreenBounds = mpWindow->GetWindowExtentsAbsolute();
        }
    }

    return aScreenBounds;
}

tools::Rectangle ScAccessibleEditObject::GetBoundingBox()
{
    tools::Rectangle aBounds( GetBoundingBoxOnScreen() );

    if ( mpWindow )
    {
        rtl::Reference<comphelper::OAccessible> pThis = mpWindow->GetAccessible();
        if (pThis.is())
        {
            uno::Reference<XAccessible> xParent = pThis->getAccessibleParent();
            if ( xParent.is() )
            {
                uno::Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), uno::UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    Point aScreenLoc = aBounds.TopLeft();
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();
                    Point aPos( aScreenLoc.getX() - aParentScreenLoc.X, aScreenLoc.getY() - aParentScreenLoc.Y );
                    aBounds.SetPos( aPos );
                }
            }
        }
    }

    return aBounds;
}

    //=====  XAccessibleContext  ==============================================

sal_Int64 SAL_CALL
    ScAccessibleEditObject::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleEditObject::getAccessibleChild(sal_Int64 nIndex)
{
    SolarMutexGuard aGuard;
    ensureAlive();
    CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

sal_Int64 SAL_CALL ScAccessibleEditObject::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nParentStates = 0;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        nParentStates = xParentContext->getAccessibleStateSet();
    }
    sal_Int64 nStateSet = 0;
    if (IsDefunc(nParentStates))
        nStateSet |= AccessibleStateType::DEFUNC;
    else
    {
        // all states are const, because this object exists only in one state
        nStateSet |= AccessibleStateType::EDITABLE;
        nStateSet |= AccessibleStateType::ENABLED;
        nStateSet |= AccessibleStateType::SENSITIVE;
        nStateSet |= AccessibleStateType::MULTI_LINE;
        nStateSet |= AccessibleStateType::MULTI_SELECTABLE;
        nStateSet |= AccessibleStateType::SHOWING;
        nStateSet |= AccessibleStateType::VISIBLE;
    }
    return nStateSet;
}

OUString
    ScAccessibleEditObject::createAccessibleDescription()
{
//    OSL_FAIL("Should never be called, because is set in the constructor.")
    return OUString();
}

OUString
    ScAccessibleEditObject::createAccessibleName()
{
    OSL_FAIL("Should never be called, because is set in the constructor.");
    return OUString();
}

    ///=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
    ScAccessibleEditObject::addAccessibleEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
{
    CreateTextHelper();

    mpTextHelper->AddEventListener(xListener);

    ScAccessibleContextBase::addAccessibleEventListener(xListener);
}

void SAL_CALL
    ScAccessibleEditObject::removeAccessibleEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
{
    CreateTextHelper();

    mpTextHelper->RemoveEventListener(xListener);

    ScAccessibleContextBase::removeAccessibleEventListener(xListener);
}

bool ScAccessibleEditObject::IsDefunc(sal_Int64 nParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || !getAccessibleParent().is() ||
         (nParentStates & AccessibleStateType::DEFUNC);
}

OutputDevice* ScAccessibleEditObject::GetOutputDeviceForView()
{
    return mpWindow->GetOutDev();
}

void ScAccessibleEditObject::CreateTextHelper()
{
    if (mpTextHelper)
        return;

    ::std::unique_ptr < ScAccessibleTextData > pAccessibleTextData;
    if (meObjectType == CellInEditMode || meObjectType == EditControl)
    {
        pAccessibleTextData.reset
            (new ScAccessibleEditObjectTextData(mpEditView, GetOutputDeviceForView()));
    }
    else
    {
        pAccessibleTextData.reset
            (new ScAccessibleEditLineTextData(nullptr, GetOutputDeviceForView(), mpTextWnd));
    }

    std::unique_ptr<ScAccessibilityEditSource> pEditSrc =
        std::make_unique<ScAccessibilityEditSource>(std::move(pAccessibleTextData));

    mpTextHelper = std::make_unique<::accessibility::AccessibleTextHelper>(std::move(pEditSrc));
    mpTextHelper->SetEventSource(this);

    const ScInputHandler* pInputHdl = ScModule::get()->GetInputHdl();
    if ( pInputHdl && pInputHdl->IsEditMode() )
    {
        mpTextHelper->SetFocus();
    }
    else
    {
        mpTextHelper->SetFocus(mbHasFocus);
    }

    // #i54814# activate cell in edit mode
    if( meObjectType == CellInEditMode )
    {
        // do not activate cell object, if top edit line is active
        if( pInputHdl && !pInputHdl->IsTopMode() )
        {
            SdrHint aHint( SdrHintKind::BeginEdit );
            mpTextHelper->GetEditSource().GetBroadcaster().Broadcast( aHint );
        }
    }
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getForeground(  )
{
    return GetFgBgColor(SC_UNONAME_CCOLOR);
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getBackground(  )
{
    return GetFgBgColor(SC_UNONAME_CELLBACK);
}

sal_Int32 ScAccessibleEditObject::GetFgBgColor( const OUString &strPropColor)
{
    SolarMutexGuard aGuard;
    sal_Int32 nColor(0);
    if (m_pScDoc)
    {
        ScDocShell* pObjSh = m_pScDoc->GetDocumentShell();
        if ( pObjSh )
        {
            ScModelObj* pSpreadDoc = pObjSh->GetModel();
            if ( pSpreadDoc )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = pSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(m_curCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(m_curCellAddress.Col(), m_curCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(strPropColor);
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }
    return nColor;
}
//=====  XAccessibleSelection  ============================================

void SAL_CALL ScAccessibleEditObject::selectAccessibleChild( sal_Int64 )
{
}

sal_Bool SAL_CALL ScAccessibleEditObject::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
        xContext = xAcc->getAccessibleContext();
    if( xContext.is() )
    {
        if( xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
        {
            uno::Reference< css::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return true;
            }
        }
    }
    return false;
}

void SAL_CALL ScAccessibleEditObject::clearAccessibleSelection(  )
{
}

void SAL_CALL ScAccessibleEditObject::selectAllAccessibleChildren(  )
{
}

sal_Int64 SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChildCount()
{
    sal_Int64 nCount = 0;
    sal_Int64 TotalCount = getAccessibleChildCount();
    for( sal_Int64 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;
    return nCount;
}

uno::Reference<XAccessible> SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    if ( nSelectedChildIndex < 0 || nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    for (sal_Int64 i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return uno::Reference<XAccessible>();
}

void SAL_CALL ScAccessibleEditObject::deselectAccessibleChild(sal_Int64)
{
}

uno::Reference< XAccessibleRelationSet > ScAccessibleEditObject::getAccessibleRelationSet(  )
{
    SolarMutexGuard aGuard;
    vcl::Window* pWindow = mpWindow;
    rtl::Reference<utl::AccessibleRelationSetHelper> rRelationSet = new utl::AccessibleRelationSetHelper;
    if ( pWindow )
    {
        vcl::Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence<uno::Reference<css::accessibility::XAccessible>> aSequence { pLabeledBy->GetAccessible() };
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType_LABELED_BY, aSequence ) );
        }
        vcl::Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != pWindow )
        {
            uno::Sequence< uno::Reference<css::accessibility::XAccessible> > aSequence { pMemberOf->GetAccessible() };
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType_MEMBER_OF, aSequence ) );
        }
        return rRelationSet;
    }
    return uno::Reference< XAccessibleRelationSet >();
}

AbsoluteScreenPixelRectangle ScAccessibleEditControlObject::GetBoundingBoxOnScreen()
{
    AbsoluteScreenPixelRectangle aScreenBounds;

    if (m_pController && m_pController->GetDrawingArea())
    {
        aScreenBounds = AbsoluteScreenPixelRectangle(m_pController->GetDrawingArea()->get_accessible_location_on_screen(),
                                         m_pController->GetOutputSizePixel());
    }

    return aScreenBounds;
}

tools::Rectangle ScAccessibleEditControlObject::GetBoundingBox()
{
    tools::Rectangle aBounds( GetBoundingBoxOnScreen() );

    uno::Reference<XAccessibleContext> xContext = getAccessibleContext();
    if ( xContext.is() )
    {
        uno::Reference< XAccessible > xParent( xContext->getAccessibleParent() );
        if ( xParent.is() )
        {
            uno::Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), uno::UNO_QUERY );
            if ( xParentComponent.is() )
            {
                Point aScreenLoc = aBounds.TopLeft();
                awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();
                Point aPos( aScreenLoc.getX() - aParentScreenLoc.X, aScreenLoc.getY() - aParentScreenLoc.Y );
                aBounds.SetPos( aPos );
            }
        }
    }

    return aBounds;
}

void SAL_CALL ScAccessibleEditControlObject::disposing()
{
    ScAccessibleEditObject::disposing();
    m_pController = nullptr;
}

uno::Reference< XAccessibleRelationSet > ScAccessibleEditControlObject::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    if (!m_pController || !m_pController->GetDrawingArea())
        return uno::Reference< XAccessibleRelationSet >();
    return m_pController->GetDrawingArea()->get_accessible_relation_set();
}

OutputDevice* ScAccessibleEditControlObject::GetOutputDeviceForView()
{
    if (!m_pController || !m_pController->GetDrawingArea())
        return nullptr;
    return &m_pController->GetDrawingArea()->get_ref_device();
}

ScAccessibleEditLineObject::ScAccessibleEditLineObject(ScTextWnd* pTextWnd)
    : ScAccessibleEditControlObject(pTextWnd, ScAccessibleEditObject::EditLine)
{
    // tdf#141769 set this early so its always available, even before the on-demand
    // editview is created
    mpTextWnd = pTextWnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
