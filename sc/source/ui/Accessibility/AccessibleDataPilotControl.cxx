/*************************************************************************
 *
 *  $RCSfile: AccessibleDataPilotControl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:10:52 $
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

#ifndef _SC_ACCESSIBLEDATAPILOTCONTROL_HXX
#include "AccessibleDataPilotControl.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_FIELDWND_HXX
#include "fieldwnd.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
        ScDPFieldWindow* pDPFieldWindow,
        sal_Int32 nIndex);

    virtual void Init();
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
    virtual ::rtl::OUString SAL_CALL
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
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);

private:
    ScDPFieldWindow* mpDPFieldWindow;
    sal_Int32        mnIndex;
};

    //=====  internal  ========================================================
ScAccessibleDataPilotControl::ScAccessibleDataPilotControl(
        const uno::Reference<XAccessible>& rxParent,
        ScDPFieldWindow* pDPFieldWindow)
        :
    ScAccessibleContextBase(rxParent, AccessibleRole::GROUP_BOX),
    mpDPFieldWindow(pDPFieldWindow)
{
    if (mpDPFieldWindow)
        maChildren.resize(mpDPFieldWindow->GetFieldCount());
}

ScAccessibleDataPilotControl::~ScAccessibleDataPilotControl(void)
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleDataPilotControl::Init()
{
}

void SAL_CALL ScAccessibleDataPilotControl::disposing()
{
    ScUnoGuard aGuard;
    mpDPFieldWindow = NULL;

    ScAccessibleContextBase::disposing();
}

void ScAccessibleDataPilotControl::AddField(sal_Int32 nNewIndex)
{
    sal_Bool bAdded(sal_False);
    if (static_cast<sal_uInt32>(nNewIndex) == maChildren.size())
    {
        maChildren.push_back(AccessibleWeak());
        bAdded = sal_True;
    }
    else if (static_cast<sal_uInt32>(nNewIndex) < maChildren.size())
    {
        ::std::vector < AccessibleWeak >::iterator aItr = maChildren.begin() + nNewIndex;
        maChildren.insert(aItr, AccessibleWeak());

        ::std::vector < AccessibleWeak >::iterator aEndItr = maChildren.end();
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
        DBG_ERRORFILE("did not recognize a child count change");

    if (bAdded)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.Source = uno::Reference< XAccessible >(this);
        aEvent.NewValue <<= getAccessibleChild(nNewIndex);

        CommitChange(aEvent); // new child - event
    }
}

void ScAccessibleDataPilotControl::RemoveField(sal_Int32 nOldIndex)
{
    sal_Bool bRemoved(sal_False);
    uno::Reference< XAccessible > xTempAcc;
    ScAccessibleDataPilotButton* pField = NULL;
    if (static_cast<sal_uInt32>(nOldIndex) < maChildren.size())
    {
        xTempAcc = getAccessibleChild(nOldIndex);
        pField = maChildren[nOldIndex].pAcc;

        ::std::vector < AccessibleWeak >::iterator aItr = maChildren.begin() + nOldIndex;
        aItr = maChildren.erase(aItr);

        ::std::vector < AccessibleWeak >::iterator aEndItr = maChildren.end();
        uno::Reference< XAccessible > xTempAcc;
        while (aItr != aEndItr)
        {
            xTempAcc = aItr->xWeakAcc;
            if (xTempAcc.is() && aItr->pAcc)
                aItr->pAcc->SetIndex(nOldIndex);
            ++nOldIndex;
            ++aItr;
        }
        bRemoved = sal_True;
    }
    else
        DBG_ERRORFILE("did not recognize a child count change");

    if (bRemoved)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.Source = uno::Reference< XAccessible >(this);
        aEvent.NewValue <<= xTempAcc;

        CommitChange(aEvent); // gone child - event

        if (pField)
            pField->dispose();
    }
}

void ScAccessibleDataPilotControl::FieldFocusChange(sal_Int32 nOldIndex, sal_Int32 nNewIndex)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nOldIndex) < maChildren.size() &&
                static_cast<sal_uInt32>(nNewIndex) < maChildren.size(), "did not recognize a child count change");

    uno::Reference < XAccessible > xTempAcc = maChildren[nOldIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nOldIndex].pAcc)
        maChildren[nOldIndex].pAcc->ResetFocused();

    xTempAcc = maChildren[nNewIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nNewIndex].pAcc)
        maChildren[nNewIndex].pAcc->SetFocused();
}

void ScAccessibleDataPilotControl::FieldNameChange(sal_Int32 nIndex)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nIndex) < maChildren.size(), "did not recognize a child count change");

    uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
    if (xTempAcc.is() && maChildren[nIndex].pAcc)
        maChildren[nIndex].pAcc->ChangeName();
}

void ScAccessibleDataPilotControl::GotFocus()
{
    if (mpDPFieldWindow)
    {
        DBG_ASSERT(static_cast<sal_uInt32>(mpDPFieldWindow->GetFieldCount()) == maChildren.size(), "did not recognize a child count change");

        sal_Int32 nIndex(mpDPFieldWindow->GetSelectedField());
        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (xTempAcc.is() && maChildren[nIndex].pAcc)
            maChildren[nIndex].pAcc->SetFocused();
    }
}

void ScAccessibleDataPilotControl::LostFocus()
{
    if (mpDPFieldWindow)
    {
        DBG_ASSERT(static_cast<sal_uInt32>(mpDPFieldWindow->GetFieldCount()) == maChildren.size(), "did not recognize a child count change");

        sal_Int32 nIndex(mpDPFieldWindow->GetSelectedField());
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
        ScUnoGuard aGuard;
        IsObjectValid();
        if (mpDPFieldWindow)
        {
            Point aAbsPoint(VCLPoint(rPoint));
            Point aControlEdge(GetBoundingBoxOnScreen().TopLeft());
            Point aRelPoint(aAbsPoint - aControlEdge);
            sal_Int32 nChildIndex(0);
            if (mpDPFieldWindow->GetFieldIndex(aRelPoint, nChildIndex))
                xAcc = getAccessibleChild(nChildIndex);
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
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
        mpDPFieldWindow->GrabFocus();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getForeground(  )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDPFieldWindow)
    {
        nColor = mpDPFieldWindow->GetSettings().GetStyleSettings().GetWindowTextColor().GetColor();
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getBackground(  )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDPFieldWindow)
    {
        if (mpDPFieldWindow->GetType() == TYPE_SELECT)
        {
            nColor = mpDPFieldWindow->GetSettings().GetStyleSettings().GetFaceColor().GetColor();
        }
        else
        {
            nColor = mpDPFieldWindow->GetSettings().GetStyleSettings().GetWindowColor().GetColor();
        }
    }
    return nColor;
}

    ///=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessibleDataPilotControl::getAccessibleChildCount(void)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetFieldCount();
    else
        return 0;
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotControl::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    uno::Reference<XAccessible> xAcc;
    if (mpDPFieldWindow)
    {
        if (nIndex < 0 || nIndex >= mpDPFieldWindow->GetFieldCount())
            throw lang::IndexOutOfBoundsException();

        DBG_ASSERT(static_cast<sal_uInt32>(mpDPFieldWindow->GetFieldCount()) == maChildren.size(), "did not recognize a child count change");

        uno::Reference < XAccessible > xTempAcc = maChildren[nIndex].xWeakAcc;
        if (!xTempAcc.is())
        {
            maChildren[nIndex].pAcc = new ScAccessibleDataPilotButton(this, mpDPFieldWindow, nIndex);
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
    ScUnoGuard aGuard;
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

::rtl::OUString SAL_CALL ScAccessibleDataPilotControl::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDataPilotControl"));
}

    ///=====  XTypeProvider  ===================================================

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotControl::getImplementationId(void)
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

    //=====  internal  ========================================================

::rtl::OUString SAL_CALL ScAccessibleDataPilotControl::createAccessibleDescription(void)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetDescription();

    return rtl::OUString();
}

::rtl::OUString SAL_CALL ScAccessibleDataPilotControl::createAccessibleName(void)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetName();

    return rtl::OUString();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetWindowExtentsRelative(NULL);
    else
        return Rectangle();
}

Rectangle ScAccessibleDataPilotControl::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetWindowExtentsRelative(mpDPFieldWindow->GetAccessibleParentWindow());
    else
        return Rectangle();
}


//===============================================================================

ScAccessibleDataPilotButton::ScAccessibleDataPilotButton(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScDPFieldWindow* pDPFieldWindow,
        sal_Int32 nIndex)
    : ScAccessibleContextBase(rxParent, AccessibleRole::PUSH_BUTTON),
    mpDPFieldWindow(pDPFieldWindow),
    mnIndex(nIndex)
{
}

ScAccessibleDataPilotButton::~ScAccessibleDataPilotButton(void)
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleDataPilotButton::Init()
{
}

void SAL_CALL ScAccessibleDataPilotButton::disposing()
{
    ScUnoGuard aGuard;
    mpDPFieldWindow = NULL;

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
        const ::com::sun::star::awt::Point& rPoint )
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
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
    {
        mpDPFieldWindow->GrabFocusWithSel(getAccessibleIndexInParent());
    }
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getForeground(  )
throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDPFieldWindow)
    {
        nColor = mpDPFieldWindow->GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getBackground(  )
throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDPFieldWindow)
    {
        nColor = mpDPFieldWindow->GetSettings().GetStyleSettings().GetFaceColor().GetColor();
    }
    return nColor;
}

    ///=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getAccessibleChildCount(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

uno::Reference< XAccessible> SAL_CALL ScAccessibleDataPilotButton::getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException)
{
    throw lang::IndexOutOfBoundsException();
}

sal_Int32 SAL_CALL ScAccessibleDataPilotButton::getAccessibleIndexInParent(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDataPilotButton::getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();

    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();

    if (IsDefunc())
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::OPAQUE);
        pStateSet->AddState(AccessibleStateType::FOCUSABLE);
        if (mpDPFieldWindow && (mpDPFieldWindow->GetSelectedField() == mnIndex))
            pStateSet->AddState(AccessibleStateType::FOCUSED);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }

    return pStateSet;
}

    ///=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleDataPilotButton::getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDataPilotButton"));
}

    ///=====  XTypeProvider  ===================================================

uno::Sequence<sal_Int8> SAL_CALL ScAccessibleDataPilotButton::getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException)
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

::rtl::OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return rtl::OUString();
}

::rtl::OUString SAL_CALL ScAccessibleDataPilotButton::createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    if (mpDPFieldWindow)
        return mpDPFieldWindow->GetFieldText(getAccessibleIndexInParent());

    return rtl::OUString();
}

Rectangle ScAccessibleDataPilotButton::GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException)
{
    Rectangle aRect(GetBoundingBox());

    if (mpDPFieldWindow)
    {
        Point aParentPos(mpDPFieldWindow->GetWindowExtentsRelative(NULL).TopLeft());
        aRect.Move(aParentPos.getX(), aParentPos.getY());
    }

    return aRect;
}

Rectangle ScAccessibleDataPilotButton::GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException)
{
    if (mpDPFieldWindow)
        return Rectangle (mpDPFieldWindow->GetFieldPosition(const_cast<ScAccessibleDataPilotButton*> (this)->getAccessibleIndexInParent()), mpDPFieldWindow->GetFieldSize());
    else
        return Rectangle();
}
