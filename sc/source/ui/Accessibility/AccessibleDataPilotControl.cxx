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
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <vcl/keycodes.hxx>
#include <vcl/settings.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <tools/gen.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

class ScAccessibleDataPilotButton
    :   public ScAccessibleContextBase
    , public ::com::sun::star::accessibility::XAccessibleAction
{
public:
    //=====  internal  ========================================================
    ScAccessibleDataPilotButton(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDPFieldControlBase* pFieldWindow,
        sal_Int32 nIndex);

    virtual void Init() SAL_OVERRIDE;

    using ScAccessibleContextBase::disposing;
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    void SetIndex(sal_Int32 nIndex) { mnIndex = nIndex; }
    void NameChanged();
    void SetFocused();
    void ResetFocused();
protected:
    virtual ~ScAccessibleDataPilotButton(void);
public:
    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ///=====  XInterface  =====================================================
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;
    virtual void SAL_CALL release() throw () SAL_OVERRIDE;
    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isVisible(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getForeground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getBackground(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) SAL_OVERRIDE;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet >
        SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
        // call dispose to inform object which have a weak reference to this object
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
        aEvent.OldValue <<= xTempAcc;

        CommitChange(aEvent); // gone child - event

        if (pField)
            pField->dispose();
    }
}

void ScAccessibleDataPilotControl::FieldFocusChange(sal_Int32 nOldIndex, sal_Int32 nNewIndex)
{
    if (0 <= nOldIndex && static_cast<size_t>(nOldIndex) < maChildren.size())
    {
        uno::Reference < XAccessible > xTempAcc = maChildren[nOldIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nOldIndex].pAcc)
            maChildren[nOldIndex].pAcc->ResetFocused();
    }
    else
    {
        SAL_WARN( "sc", "ScAccessibleDataPilotControl::FieldFocusChange() old index out of bounds: " << nOldIndex);
    }

    if (0 <= nNewIndex && static_cast<size_t>(nNewIndex) < maChildren.size())
    {
        uno::Reference < XAccessible > xTempAcc = maChildren[nNewIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nNewIndex].pAcc)
            maChildren[nNewIndex].pAcc->SetFocused();
    }
    else
    {
        SAL_WARN( "sc", "ScAccessibleDataPilotControl::FieldFocusChange() new index out of bounds: " << nNewIndex);
    }
}

void ScAccessibleDataPilotControl::FieldNameChange(sal_Int32 nIndex)
{
    if (0 <= nIndex && static_cast<size_t>(nIndex) < maChildren.size())
    {
        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nIndex].pAcc)
            maChildren[nIndex].pAcc->ChangeName();
    }
    else
    {
        SAL_WARN( "sc", "ScAccessibleDataPilotControl::FieldNameChange() index out of bounds: " << nIndex);
    }
}

void ScAccessibleDataPilotControl::GotFocus()
{
    if (mpFieldWindow)
    {
        OSL_ENSURE(mpFieldWindow->GetFieldCount() == maChildren.size(), "did not recognize a child count change");

        if(maChildren.size()==0)
            return ;

        sal_Int32 nIndex(mpFieldWindow->GetSelectedField());
        if (0 <= nIndex && static_cast<size_t>(nIndex) < maChildren.size())
        {
            uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
            if (xTempAcc.is() && maChildren[nIndex].pAcc)
                maChildren[nIndex].pAcc->SetFocused();
        }
        else
        {
            SAL_WARN( "sc", "ScAccessibleDataPilotControl::GotFocus() field index out of bounds: " << nIndex);
        }
    }
}

void ScAccessibleDataPilotControl::LostFocus()
{
    if (mpFieldWindow)
    {
        OSL_ENSURE(mpFieldWindow->GetFieldCount() == maChildren.size(), "did not recognize a child count change");

        if(maChildren.size()==0)
            return ;

        sal_Int32 nIndex(mpFieldWindow->GetSelectedField());
        if (0 <= nIndex && static_cast<size_t>(nIndex) < maChildren.size())
        {
            uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
            if (xTempAcc.is() && maChildren[nIndex].pAcc)
                maChildren[nIndex].pAcc->ResetFocused();
        }
        else
        {
            // This may actually happen if the last field is dropped somewhere
            // and was already removed from the pane by dragging it away. This
            // is odd.. looks like all LostFocus() are called for the previous
            // field of the same original pane in the remove case?
            SAL_WARN_IF( nIndex != -1 || !maChildren.empty(), "sc",
                    "ScAccessibleDataPilotControl::LostFocus() field index out of bounds: " << nIndex);
        }
    }
}

    ///=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDataPilotControl::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException, std::exception)
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
        throw (uno::RuntimeException, std::exception)
{
    return sal_True;
}

void SAL_CALL ScAccessibleDataPilotControl::grabFocus(  )
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        mpFieldWindow->GrabFocus();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getForeground(  )
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return static_cast<sal_Int32>(maChildren.size());
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotControl::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    uno::Reference<XAccessible> xAcc;
    if (mpFieldWindow)
    {
        if (nIndex < 0 || static_cast< size_t >( nIndex ) >= maChildren.size())
            throw lang::IndexOutOfBoundsException();

        OSL_ENSURE( mpFieldWindow->GetFieldCount() == maChildren.size()         // all except ...
                ||  mpFieldWindow->GetFieldCount() == maChildren.size() + 1,    // in CommitChange during RemoveField
                "did not recognize a child count change");

        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (!xTempAcc.is())
        {
            if (static_cast< size_t >( nIndex ) >= mpFieldWindow->GetFieldCount())
                throw lang::IndexOutOfBoundsException();

            maChildren[nIndex].pAcc = new ScAccessibleDataPilotButton(this, mpFieldWindow, nIndex);
            xTempAcc = maChildren[nIndex].pAcc;
            maChildren[nIndex].xWeakAcc = xTempAcc;
        }

        xAcc = xTempAcc;
    }
    return xAcc;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDataPilotControl::getAccessibleStateSet(void)
        throw (uno::RuntimeException, std::exception)
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
        throw (uno::RuntimeException, std::exception)
{
    return OUString("ScAccessibleDataPilotControl");
}

    ///=====  XTypeProvider  ===================================================

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotControl::getImplementationId(void)
        throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
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
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetName();

    return OUString();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException, std::exception)
{
    if (mpFieldWindow)
        return mpFieldWindow->GetWindowExtentsRelative(NULL);
    else
        return Rectangle();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBox(void) const
        throw (uno::RuntimeException, std::exception)
{
    if (mpFieldWindow)
        return mpFieldWindow->GetWindowExtentsRelative(mpFieldWindow->GetAccessibleParentWindow());
    else
        return Rectangle();
}




ScAccessibleDataPilotButton::ScAccessibleDataPilotButton(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDPFieldControlBase* pFieldWindow,
        sal_Int32 nIndex)
    : ScAccessibleContextBase(rxParent, AccessibleRole::BUTTON_MENU),
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
        // call dispose to inform object which have a weak reference to this object
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
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return NULL;
}

sal_Bool SAL_CALL ScAccessibleDataPilotButton::isVisible(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return sal_True;
}

void SAL_CALL ScAccessibleDataPilotButton::grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
    {
        mpFieldWindow->GrabFocusAndSelect(getAccessibleIndexInParent());
    }
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getForeground(  )
throw (uno::RuntimeException, std::exception)
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
throw (uno::RuntimeException, std::exception)
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
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return 0;
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotButton::getAccessibleChild(sal_Int32 /* nIndex */)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException, std::exception)
{
    throw lang::IndexOutOfBoundsException();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDataPilotButton::getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
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

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet >
    SAL_CALL ScAccessibleDataPilotButton::getAccessibleRelationSet( ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    uno::Reference< com::sun::star::accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;
    if(mxParent.is())
    {
        uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
        aSequence[0] = mxParent;
        pRelationSetHelper->AddRelation( com::sun::star::accessibility::AccessibleRelation(
            com::sun::star::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
    }

    return xSet;

}

///=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleDataPilotButton::getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString("ScAccessibleDataPilotButton");
}

    ///=====  XTypeProvider  ===================================================

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotButton::getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard g;

    if (mpFieldWindow)
        return mpFieldWindow->GetHelpText();
    return OUString();
}

OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (mpFieldWindow)
        return mpFieldWindow->GetFieldText(getAccessibleIndexInParent());

    return OUString();
}

Rectangle ScAccessibleDataPilotButton::GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException, std::exception)
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
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if (mpFieldWindow)
        return Rectangle (mpFieldWindow->GetFieldPosition(const_cast<ScAccessibleDataPilotButton*> (this)->getAccessibleIndexInParent()), mpFieldWindow->GetFieldSize());
    else
        return Rectangle();
}


// XAccessibleAction

sal_Int32 ScAccessibleDataPilotButton::getAccessibleActionCount( ) throw (uno::RuntimeException, std::exception)
{
    return 1;
}

sal_Bool ScAccessibleDataPilotButton::doAccessibleAction ( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw lang::IndexOutOfBoundsException();
    return sal_True;
}

OUString ScAccessibleDataPilotButton::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw lang::IndexOutOfBoundsException();
    return OUString("press");
}

::com::sun::star::uno::Reference< XAccessibleKeyBinding > ScAccessibleDataPilotButton::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard g;

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw lang::IndexOutOfBoundsException();
      comphelper::OAccessibleKeyBindingHelper* pKeyBindingHelper = new comphelper::OAccessibleKeyBindingHelper();
    ::com::sun::star::uno::Reference< XAccessibleKeyBinding > xKeyBinding = pKeyBindingHelper;
    ScDPFieldControlBase* pWindow = mpFieldWindow;
    if ( pWindow )
    {
        awt::KeyStroke aKeyStroke;
        aKeyStroke.Modifiers = 0;
        aKeyStroke.KeyCode = KEY_SPACE;
        pKeyBindingHelper->AddKeyBinding( aKeyStroke );
    }
    return xKeyBinding;
}

uno::Any SAL_CALL ScAccessibleDataPilotButton::queryInterface( uno::Type const & rType )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Any aAny (ScAccessibleContextBase::queryInterface(rType));
    if(!aAny.hasValue())
    {
      aAny = ::cppu::queryInterface (rType,
            static_cast<XAccessibleAction*>(this)
            );
    }
    return aAny;
}

void SAL_CALL ScAccessibleDataPilotButton::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleDataPilotButton::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
