/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <comphelper/servicehelper.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <svx/svdmodel.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleEditObject::ScAccessibleEditObject(
        const uno::Reference<XAccessible>& rxParent,
        EditView* pEditView, Window* pWin, const rtl::OUString& rName,
        const rtl::OUString& rDescription, EditObjectType eObjectType)
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
}

ScAccessibleEditObject::~ScAccessibleEditObject()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
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
    mbHasFocus = sal_True;
    CommitFocusGained();
    if (mpTextHelper)
        mpTextHelper->SetFocus(sal_True);
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleEditObject::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
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
        throw (uno::RuntimeException)
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
        throw (uno::RuntimeException)
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
                    throw (uno::RuntimeException)
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
        lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleEditObject::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
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

::rtl::OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
//    OSL_FAIL("Should never be called, because is set in the constructor.")
    return rtl::OUString();
}

::rtl::OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    OSL_FAIL("Should never be called, because is set in the constructor.");
    return rtl::OUString();
}

    ///=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL
    ScAccessibleEditObject::addEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->AddEventListener(xListener);

    ScAccessibleContextBase::addEventListener(xListener);
}

void SAL_CALL
    ScAccessibleEditObject::removeEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException)
{
    if (!mpTextHelper)
        CreateTextHelper();

    mpTextHelper->RemoveEventListener(xListener);

    ScAccessibleContextBase::removeEventListener(xListener);
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleEditObject::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleEditObject"));
}

//=====  XTypeProvider  =======================================================

namespace
{
    class theScAccessibleEditObjectImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleEditObjectImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleEditObject::getImplementationId(void)
    throw (uno::RuntimeException)
{
    return theScAccessibleEditObjectImplementationId::get().getSeq();
}

    //====  internal  =========================================================

sal_Bool ScAccessibleEditObject::IsDefunc(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
