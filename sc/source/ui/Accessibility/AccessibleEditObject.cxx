/*************************************************************************
 *
 *  $RCSfile: AccessibleEditObject.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:11:35 $
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

#ifndef _SC_ACCESSIBLEEDITOBJECT_HXX
#include "AccessibleEditObject.hxx"
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif

#ifndef _SC_ACCESSIBLETEXT_HXX
#include "AccessibleText.hxx"
#endif
#ifndef SC_EDITSRC_HXX
#include "editsrc.hxx"
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_ACCESSILE_TEXT_HELPER_HXX_
#include <svx/AccessibleTextHelper.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

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
        Point aLoc;
        aCellRect.getLocation(aLoc);
        Rectangle aRect = mpWindow->GetWindowExtentsRelative(NULL);
        Point aLoc2;
        aRect.getLocation(aLoc2);
        aCellRect.setLocation(Point(aLoc.getX() + aLoc2.getX(), aLoc.getY() + aLoc2.getY()));
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
        if (meObjectType == CellInEditMode || meObjectType == EditControl)
        {
            ::std::auto_ptr < ScAccessibleTextData > pAccessibleTextData
                (new ScAccessibleEditObjectTextData(mpEditView, mpWindow));
            ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleTextData));

            mpTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource );
            mpTextHelper->SetEventSource(this);
        }
        else
        {
            ::std::auto_ptr < ScAccessibleTextData > pAccessibleTextData
                (new ScAccessibleEditLineTextData(NULL, mpWindow));
            ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleTextData));

            mpTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource );
            mpTextHelper->SetEventSource(this);
        }
        mpTextHelper->SetFocus(mbHasFocus);
    }
}

