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

#include "AccessibleEditObject.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <comphelper/servicehelper.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>

#include "unonames.hxx"
#include "document.hxx"
#include "AccessibleDocument.hxx"
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleEditObject::ScAccessibleEditObject(
        const uno::Reference<XAccessible>& rxParent,
        EditView* pEditView, Window* pWin, const OUString& rName,
        const OUString& rDescription, EditObjectType eObjectType)
    :
    ScAccessibleContextBase(rxParent, AccessibleRole::TEXT_FRAME),
    mpTextHelper(NULL),
    mpEditView(pEditView),
    mpWindow(pWin),
    meObjectType(eObjectType),
    mbHasFocus(false)
{
    CreateTextHelper();
    SetName(rName);
    SetDescription(rDescription);
    if( meObjectType == CellInEditMode)
    {
        const ScAccessibleDocument *pAccDoc = const_cast<ScAccessibleDocument*>(static_cast<ScAccessibleDocument*>(rxParent.get())) ;
        if (pAccDoc)
        {
            m_pScDoc = pAccDoc->GetDocument();
            m_curCellAddress =pAccDoc->GetCurCellAddress();
        }
        else
        {
            m_pScDoc=NULL;
        }
    }
    else
        m_pScDoc=NULL;
}

ScAccessibleEditObject::~ScAccessibleEditObject()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleEditObject::disposing()
{
    SolarMutexGuard aGuard;
    if (mpTextHelper)
        DELETEZ(mpTextHelper);

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
        mpTextHelper->SetFocus(sal_True);
}

//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    ScAccessibleEditObject::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::uno::Any aReturn = ScAccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast< ::com::sun::star::accessibility::XAccessibleSelection* >(this)
            );
    return aReturn;
}
void SAL_CALL
    ScAccessibleEditObject::acquire (void)
    throw ()
{
    ScAccessibleContextBase::acquire ();
}
void SAL_CALL
    ScAccessibleEditObject::release (void)
    throw ()
{
    ScAccessibleContextBase::release ();
}
    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleEditObject::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException, std::exception)
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
         SolarMutexGuard aGuard;
        IsObjectValid();

        if(!mpTextHelper)
            CreateTextHelper();

        xRet = mpTextHelper->GetAt(rPoint);
    }

    return xRet;
}

Rectangle ScAccessibleEditObject::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException, std::exception)
{
    Rectangle aScreenBounds;

    if ( mpWindow )
    {
        if ( meObjectType == CellInEditMode )
        {
            if ( mpEditView && mpEditView->GetEditEngine() )
            {
                MapMode aMapMode( mpEditView->GetEditEngine()->GetRefMapMode() );
                aScreenBounds = mpWindow->LogicToPixel( mpEditView->GetOutputArea(), aMapMode );
                Point aCellLoc = aScreenBounds.TopLeft();
                Rectangle aWindowRect = mpWindow->GetWindowExtentsRelative( NULL );
                Point aWindowLoc = aWindowRect.TopLeft();
                Point aPos( aCellLoc.getX() + aWindowLoc.getX(), aCellLoc.getY() + aWindowLoc.getY() );
                aScreenBounds.SetPos( aPos );
            }
        }
        else
        {
            aScreenBounds = mpWindow->GetWindowExtentsRelative( NULL );
        }
    }

    return aScreenBounds;
}

Rectangle ScAccessibleEditObject::GetBoundingBox(void) const
        throw (uno::RuntimeException, std::exception)
{
    Rectangle aBounds( GetBoundingBoxOnScreen() );

    if ( mpWindow )
    {
        uno::Reference< XAccessible > xThis( mpWindow->GetAccessible() );
        if ( xThis.is() )
        {
            uno::Reference< XAccessibleContext > xContext( xThis->getAccessibleContext() );
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
        }
    }

    return aBounds;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleEditObject::getAccessibleChildCount(void)
                    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleEditObject::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleEditObject::getAccessibleStateSet(void)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        // all states are const, because this object exists only in one state
        pStateSet->AddState(AccessibleStateType::EDITABLE);
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::SENSITIVE);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
        pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
//    OSL_FAIL("Should never be called, because is set in the constructor.")
    return OUString();
}

OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleName(void)
    throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("Should never be called, because is set in the constructor.");
    return OUString();
}

    ///=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
    ScAccessibleEditObject::addAccessibleEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException, std::exception)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->AddEventListener(xListener);

    ScAccessibleContextBase::addAccessibleEventListener(xListener);
}

void SAL_CALL
    ScAccessibleEditObject::removeAccessibleEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException, std::exception)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->RemoveEventListener(xListener);

    ScAccessibleContextBase::removeAccessibleEventListener(xListener);
}

    //=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleEditObject::getImplementationName(void)
        throw (uno::RuntimeException, std::exception)
{
    return OUString("ScAccessibleEditObject");
}

//=====  XTypeProvider  =======================================================

namespace
{
    class theScAccessibleEditObjectImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleEditObjectImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleEditObject::getImplementationId(void)
    throw (uno::RuntimeException, std::exception)
{
    return theScAccessibleEditObjectImplementationId::get().getSeq();
}

    //====  internal  =========================================================

bool ScAccessibleEditObject::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessibleEditObject::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr < ScAccessibleTextData > pAccessibleTextData;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if (meObjectType == CellInEditMode || meObjectType == EditControl)
        {
            pAccessibleTextData.reset
                (new ScAccessibleEditObjectTextData(mpEditView, mpWindow));
        }
        else
        {
            pAccessibleTextData.reset
                (new ScAccessibleEditLineTextData(NULL, mpWindow));
        }

        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleTextData));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        mpTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource );
        mpTextHelper->SetEventSource(this);
        mpTextHelper->SetFocus(mbHasFocus);

        // #i54814# activate cell in edit mode
        if( meObjectType == CellInEditMode )
        {
            // do not activate cell object, if top edit line is active
            const ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
            if( pInputHdl && !pInputHdl->IsTopMode() )
            {
                SdrHint aHint( HINT_BEGEDIT );
                mpTextHelper->GetEditSource().GetBroadcaster().Broadcast( aHint );
            }
        }
    }
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getForeground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return GetFgBgColor(OUString(SC_UNONAME_CCOLOR));
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getBackground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return GetFgBgColor(OUString(SC_UNONAME_CELLBACK));
}

sal_Int32 ScAccessibleEditObject::GetFgBgColor( const OUString &strPropColor)
{
    SolarMutexGuard aGuard;
    sal_Int32 nColor(0);
    if (m_pScDoc)
    {
        SfxObjectShell* pObjSh = m_pScDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
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

void SAL_CALL ScAccessibleEditObject::selectAccessibleChild( sal_Int32 )
throw ( IndexOutOfBoundsException, RuntimeException, std::exception )
{
}

sal_Bool SAL_CALL ScAccessibleEditObject::isAccessibleChildSelected( sal_Int32 nChildIndex )
throw ( IndexOutOfBoundsException,
       RuntimeException, std::exception )
{
    uno::Reference<XAccessible> xAcc = getAccessibleChild( nChildIndex );
    uno::Reference<XAccessibleContext> xContext;
    if( xAcc.is() )
        xContext = xAcc->getAccessibleContext();
    if( xContext.is() )
    {
        if( xContext->getAccessibleRole() == AccessibleRole::PARAGRAPH )
        {
            uno::Reference< ::com::sun::star::accessibility::XAccessibleText >
                xText(xAcc, uno::UNO_QUERY);
            if( xText.is() )
            {
                if( xText->getSelectionStart() >= 0 ) return sal_True;
            }
        }
    }
    return sal_False;
}

void SAL_CALL ScAccessibleEditObject::clearAccessibleSelection(  )
throw ( RuntimeException, std::exception )
{
}

void SAL_CALL ScAccessibleEditObject::selectAllAccessibleChildren(  )
throw ( RuntimeException, std::exception )
{
}

sal_Int32 SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChildCount()
throw ( RuntimeException, std::exception )
{
    sal_Int32 nCount = 0;
    sal_Int32 TotalCount = getAccessibleChildCount();
    for( sal_Int32 i = 0; i < TotalCount; i++ )
        if( isAccessibleChildSelected(i) ) nCount++;
    return nCount;
}

uno::Reference<XAccessible> SAL_CALL ScAccessibleEditObject::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
throw ( IndexOutOfBoundsException, RuntimeException, std::exception)
{
    if ( nSelectedChildIndex > getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();
    sal_Int32 i1, i2;
    for( i1 = 0, i2 = 0; i1 < getAccessibleChildCount(); i1++ )
        if( isAccessibleChildSelected(i1) )
        {
            if( i2 == nSelectedChildIndex )
                return getAccessibleChild( i1 );
            i2++;
        }
    return uno::Reference<XAccessible>();
}

void SAL_CALL ScAccessibleEditObject::deselectAccessibleChild(
                                                            sal_Int32 )
                                                            throw ( IndexOutOfBoundsException,
                                                            RuntimeException, std::exception )
{
}

uno::Reference< XAccessibleRelationSet > ScAccessibleEditObject::getAccessibleRelationSet(  )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    Window* pWindow = mpWindow;
    utl::AccessibleRelationSetHelper* rRelationSet = new utl::AccessibleRelationSetHelper;
    uno::Reference< XAccessibleRelationSet > rSet = rRelationSet;
    if ( pWindow )
    {
        Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabeledBy->GetAccessible();
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::LABELED_BY, aSequence ) );
        }
        Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pMemberOf->GetAccessible();
            rRelationSet->AddRelation( AccessibleRelation( AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
        return rSet;
    }
    return uno::Reference< XAccessibleRelationSet >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
