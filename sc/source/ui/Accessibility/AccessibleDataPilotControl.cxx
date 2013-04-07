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

#include "AccessibleDataPilotControl.hxx"
#include "fieldwnd.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <tools/gen.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

class ScAccessibleDataPilotButton
    :   public ScAccessibleContextBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleDataPilotButton(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDPFieldControlBase* pFieldWindow,
        sal_Int32 nIndex);

    virtual void Init();

    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL disposing();

    void SetIndex(sal_Int32 nIndex) { mnIndex = nIndex; }
    void NameChanged();
    void SetFocused();
    void ResetFocused();
protected:
    virtual ~ScAccessibleDataPilotButton(void);
public:
    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void) throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);

private:
    ScDPFieldControlBase* mpFieldWindow;
    sal_Int32        mnIndex;
};

    //=====  internal  ========================================================
ScAccessibleDataPilotControl::ScAccessibleDataPilotControl(
        const uno::Reference<XAccessible>& rxParent,
        ScDPFieldControlBase* pFieldWindow)
        :
    ScAccessibleContextBase(rxParent, AccessibleRole::GROUP_BOX),
    mpFieldWindow(pFieldWindow)
{
    if (mpFieldWindow)
        maChildren.resize(mpFieldWindow->GetFieldCount());
}

ScAccessibleDataPilotControl::~ScAccessibleDataPilotControl(void)
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleDataPilotControl::Init()
{
}

void SAL_CALL ScAccessibleDataPilotControl::disposing()
{
    SolarMutexGuard aGuard;
    mpFieldWindow = NULL;

    ScAccessibleContextBase::disposing();
}

void ScAccessibleDataPilotControl::AddField(sal_Int32 nNewIndex)
{
    sal_Bool bAdded(sal_False);
    if (static_cast<size_t>(nNewIndex) == maChildren.size())
    {
        maChildren.push_back(AccessibleWeak());
        bAdded = sal_True;
    }
    else if (static_cast<size_t>(nNewIndex) < maChildren.size())
    {
        ::std::vector < AccessibleWeak >::iterator aItr = maChildren.begin() + nNewIndex;
        maChildren.insert(aItr, AccessibleWeak());

        ::std::vector < AccessibleWeak >::iterator aEndItr = maChildren.end();
        aItr = maChildren.begin() + nNewIndex + 1;
        uno::Reference< XAccessible > xTempAcc;
        sal_Int32 nIndex = nNewIndex + 1;
        while (aItr != aEndItr)
        {
            xTempAcc = aItr->xWeakAcc;
            if (xTempAcc.is() && aItr->pAcc)
                aItr->pAcc->SetIndex(nIndex);
            ++nIndex;
            ++aItr;
        }
        bAdded = sal_True;
    }
    else
    {
        OSL_FAIL("did not recognize a child count change");
    }

    if (bAdded)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.Source = uno::Reference< XAccessibleContext >(this);
        aEvent.NewValue <<= getAccessibleChild(nNewIndex);

        CommitChange(aEvent); // new child - event
    }
}

void ScAccessibleDataPilotControl::MoveField(sal_Int32 nOldIndex, sal_Int32 nNewIndex)
{
    RemoveField(nOldIndex);
    if (nNewIndex > nOldIndex)
        --nNewIndex;
    AddField(nNewIndex);
}

void ScAccessibleDataPilotControl::RemoveField(sal_Int32 nOldIndex)
{
    sal_Bool bRemoved(sal_False);
    uno::Reference< XAccessible > xTempAcc;
    ScAccessibleDataPilotButton* pField = NULL;
    if (static_cast<size_t>(nOldIndex) < maChildren.size())
    {
        xTempAcc = getAccessibleChild(nOldIndex);
        pField = maChildren[nOldIndex].pAcc;

        ::std::vector < AccessibleWeak >::iterator aItr = maChildren.begin() + nOldIndex;
        aItr = maChildren.erase(aItr);

        ::std::vector < AccessibleWeak >::iterator aEndItr = maChildren.end();
        uno::Reference< XAccessible > xItrAcc;
        while (aItr != aEndItr)
        {
            xItrAcc = aItr->xWeakAcc;
            if (xItrAcc.is() && aItr->pAcc)
                aItr->pAcc->SetIndex(nOldIndex);
            ++nOldIndex;
            ++aItr;
        }
        bRemoved = sal_True;
    }
    else
    {
        OSL_FAIL("did not recognize a child count change");
    }

    if (bRemoved)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.Source = uno::Reference< XAccessibleContext >(this);
        aEvent.NewValue <<= xTempAcc;

        CommitChange(aEvent); // gone child - event

        if (pField)
            pField->dispose();
    }
}

void ScAccessibleDataPilotControl::FieldFocusChange(sal_Int32 nOldIndex, sal_Int32 nNewIndex)
{
    OSL_ENSURE(static_cast<size_t>(nOldIndex) < maChildren.size() &&
                static_cast<size_t>(nNewIndex) < maChildren.size(), "did not recognize a child count change");

    uno::Reference < XAccessible > xTempAcc = maChildren[nOldIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nOldIndex].pAcc)
        maChildren[nOldIndex].pAcc->ResetFocused();

    xTempAcc = maChildren[nNewIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nNewIndex].pAcc)
        maChildren[nNewIndex].pAcc->SetFocused();
}

void ScAccessibleDataPilotControl::FieldNameChange(sal_Int32 nIndex)
{
    OSL_ENSURE(static_cast<size_t>(nIndex) < maChildren.size(), "did not recognize a child count change");

    uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nIndex].pAcc)
        maChildren[nIndex].pAcc->ChangeName();
}

void ScAccessibleDataPilotControl::GotFocus()
{
    if (mpFieldWindow)
    {
        OSL_ENSURE(mpFieldWindow->GetFieldCount() == maChildren.size(), "did not recognize a child count change");

        sal_Int32 nIndex(mpFieldWindow->GetSelectedField());
        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nIndex].pAcc)
            maChildren[nIndex].pAcc->SetFocused();
    }
}

void ScAccessibleDataPilotControl::LostFocus()
{
    if (mpFieldWindow)
    {
        OSL_ENSURE(mpFieldWindow->GetFieldCount() == maChildren.size(), "did not recognize a child count change");

        sal_Int32 nIndex(mpFieldWindow->GetSelectedField());
        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nIndex].pAcc)
            maChildren[nIndex].pAcc->ResetFocused();
    }
}

    ///=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDataPilotControl::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xAcc;
    if (containsPoint(rPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();
        if (mpFieldWindow)
        {
            Point aAbsPoint(VCLPoint(rPoint));
            Point aControlEdge(GetBoundingBoxOnScreen().TopLeft());
            Point aRelPoint(aAbsPoint - aControlEdge);
            size_t nChildIndex = mpFieldWindow->GetFieldIndex(aRelPoint );
            if( nChildIndex != PIVOTFIELD_INVALID )
                xAcc = getAccessibleChild(static_cast< long >( nChildIndex ));
        }
    }
    return xAcc;
}

sal_Bool SAL_CALL ScAccessibleDataPilotControl::isVisible(  )
        throw (uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL ScAccessibleDataPilotControl::grabFocus(  )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        mpFieldWindow->GrabFocus();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getForeground(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpFieldWindow)
    {
        nColor = mpFieldWindow->GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getBackground(  )
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpFieldWindow)
    {
        const StyleSettings& rStyleSett = mpFieldWindow->GetSettings().GetStyleSettings();
        nColor = (mpFieldWindow->GetFieldType() == PIVOTFIELDTYPE_SELECT) ? rStyleSett.GetFaceColor().GetColor() : rStyleSett.GetWindowColor().GetColor();
    }
    return nColor;
}

    ///=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getAccessibleChildCount(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetFieldCount();
    else
        return 0;
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotControl::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    uno::Reference<XAccessible> xAcc;
    if (mpFieldWindow)
    {
        if (nIndex < 0 || static_cast< size_t >( nIndex ) >= mpFieldWindow->GetFieldCount())
            throw lang::IndexOutOfBoundsException();

        OSL_ENSURE(mpFieldWindow->GetFieldCount() == maChildren.size(), "did not recognize a child count change");

        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (!xTempAcc.is())
        {
            maChildren[nIndex].pAcc = new ScAccessibleDataPilotButton(this, mpFieldWindow, nIndex);
            xTempAcc = maChildren[nIndex].pAcc;
            maChildren[nIndex].xWeakAcc = xTempAcc;
        }

        xAcc = xTempAcc;
    }
    return xAcc;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDataPilotControl::getAccessibleStateSet(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();

    if (IsDefunc())
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::OPAQUE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }

    return pStateSet;
}

    ///=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleDataPilotControl::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return OUString("ScAccessibleDataPilotControl");
}

    ///=====  XTypeProvider  ===================================================

namespace
{
    class theScAccessibleDataPilotControlImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleDataPilotControlImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotControl::getImplementationId(void)
        throw (uno::RuntimeException)
{
    return theScAccessibleDataPilotControlImplementationId::get().getSeq();
}

    //=====  internal  ========================================================

OUString SAL_CALL ScAccessibleDataPilotControl::createAccessibleDescription(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetDescription();

    return OUString();
}

OUString SAL_CALL ScAccessibleDataPilotControl::createAccessibleName(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetName();

    return OUString();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    if (mpFieldWindow)
        return mpFieldWindow->GetWindowExtentsRelative(NULL);
    else
        return Rectangle();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    if (mpFieldWindow)
        return mpFieldWindow->GetWindowExtentsRelative(mpFieldWindow->GetAccessibleParentWindow());
    else
        return Rectangle();
}


//===============================================================================

ScAccessibleDataPilotButton::ScAccessibleDataPilotButton(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDPFieldControlBase* pFieldWindow,
        sal_Int32 nIndex)
    : ScAccessibleContextBase(rxParent, AccessibleRole::PUSH_BUTTON),
    mpFieldWindow(pFieldWindow),
    mnIndex(nIndex)
{
}

ScAccessibleDataPilotButton::~ScAccessibleDataPilotButton(void)
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleDataPilotButton::Init()
{
}

void SAL_CALL ScAccessibleDataPilotButton::disposing()
{
    SolarMutexGuard aGuard;
    mpFieldWindow = NULL;

    ScAccessibleContextBase::disposing();
}

void ScAccessibleDataPilotButton::SetFocused()
{
    CommitFocusGained();
}

void ScAccessibleDataPilotButton::ResetFocused()
{
    CommitFocusLost();
}

    ///=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDataPilotButton::getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& /* rPoint */ )
        throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}

sal_Bool SAL_CALL ScAccessibleDataPilotButton::isVisible(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

void SAL_CALL ScAccessibleDataPilotButton::grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
    {
        mpFieldWindow->GrabFocusAndSelect(getAccessibleIndexInParent());
    }
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getForeground(  )
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpFieldWindow)
    {
        nColor = mpFieldWindow->GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getBackground(  )
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpFieldWindow)
    {
        nColor = mpFieldWindow->GetSettings().GetStyleSettings().GetFaceColor().GetColor();
    }
    return nColor;
}

    ///=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getAccessibleChildCount(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotButton::getAccessibleChild(sal_Int32 /* nIndex */)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException)
{
    throw lang::IndexOutOfBoundsException();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDataPilotButton::getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();

    if (IsDefunc())
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::OPAQUE);
        pStateSet->AddState(AccessibleStateType::FOCUSABLE);
        if (mpFieldWindow && (sal::static_int_cast<sal_Int32>(mpFieldWindow->GetSelectedField()) == mnIndex))
            pStateSet->AddState(AccessibleStateType::FOCUSED);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }

    return pStateSet;
}

    ///=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleDataPilotButton::getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return OUString("ScAccessibleDataPilotButton");
}

    ///=====  XTypeProvider  ===================================================

namespace
{
    class theScAccessibleDataPilotButtonImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleDataPilotButtonImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotButton::getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return theScAccessibleDataPilotButtonImplementationId::get().getSeq();
}

OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return OUString();
}

OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetFieldText(getAccessibleIndexInParent());

    return OUString();
}

Rectangle ScAccessibleDataPilotButton::GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException)
{
    Rectangle aRect(GetBoundingBox());

    if (mpFieldWindow)
    {
        Point aParentPos(mpFieldWindow->GetWindowExtentsRelative(NULL).TopLeft());
        aRect.Move(aParentPos.getX(), aParentPos.getY());
    }

    return aRect;
}

Rectangle ScAccessibleDataPilotButton::GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException)
{
    if (mpFieldWindow)
        return Rectangle (mpFieldWindow->GetFieldPosition(const_cast<ScAccessibleDataPilotButton*> (this)->getAccessibleIndexInParent()), mpFieldWindow->GetFieldSize());
    else
        return Rectangle();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
