/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleEditObject.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "AccessibleEditObject.hxx"
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#include "unoguard.hxx"
#include "AccessibleText.hxx"
#include "editsrc.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#include <rtl/uuid.h>
#include <tools/debug.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/editview.hxx>
#include <svx/editeng.hxx>
#include <svx/svdmodel.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleEditObject::ScAccessibleEditObject(
        const uno::Reference<XAccessible>& rxParent,
        EditView* pEditView, Window* pWin, const rtl::OUString& rName,
        const rtl::OUString& rDescription, EditObjectType eObjectType)
    :
    ScAccessibleContextBase(rxParent, AccessibleRole::PANEL),
    mpTextHelper(NULL),
    mpEditView(pEditView),
    mpWindow(pWin),
    meObjectType(eObjectType),
    mbHasFocus(sal_False)
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
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleEditObject::disposing()
{
    ScUnoGuard aGuard;
    if (mpTextHelper)
        DELETEZ(mpTextHelper);

    ScAccessibleContextBase::disposing();
}

void ScAccessibleEditObject::LostFocus()
{
    if (mpTextHelper)
        mpTextHelper->SetFocus(sal_False);
    mbHasFocus = sal_False;
}

void ScAccessibleEditObject::GotFocus()
{
    if (mpTextHelper)
        mpTextHelper->SetFocus(sal_True);
    mbHasFocus = sal_True;
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleEditObject::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
         ScUnoGuard aGuard;
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
    Rectangle aCellRect(GetBoundingBox());
    if (mpWindow)
    {
        Point aLoc = aCellRect.TopLeft();
        Rectangle aRect = mpWindow->GetWindowExtentsRelative(NULL);
        Point aLoc2 = aRect.TopLeft();
        aCellRect.SetPos(Point(aLoc.getX() + aLoc2.getX(), aLoc.getY() + aLoc2.getY()));
        //aCellRect.setX(aCellRect.getX() + aRect.getX());
        //aCellRect.setY(aCellRect.getY() + aRect.getY());
    }
    return aCellRect;
}

Rectangle ScAccessibleEditObject::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    Rectangle aCellRect;
    if (meObjectType == CellInEditMode)
    {
        if (mpEditView && mpWindow && mpEditView->GetEditEngine())
        {
            MapMode aMapMode(mpEditView->GetEditEngine()->GetRefMapMode());

            aCellRect = mpWindow->LogicToPixel( mpEditView->GetOutputArea() , aMapMode );
        }
    }
    else
    {
        if (mpWindow)
        {
            aCellRect = mpWindow->GetWindowExtentsRelative(mpWindow->GetAccessibleParentWindow());
        }
    }
    return aCellRect;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleEditObject::getAccessibleChildCount(void)
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleEditObject::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
//    DBG_ERRORFILE("Should never be called, because is set in the constructor.")
    return rtl::OUString();
}

::rtl::OUString SAL_CALL
    ScAccessibleEditObject::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    DBG_ERRORFILE("Should never be called, because is set in the constructor.");
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

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleEditObject::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
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
        ::std::auto_ptr < ScAccessibleTextData > pAccessibleTextData;
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

        ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleTextData));
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

