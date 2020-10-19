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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>

#include <svx/dlgctrl.hxx>

#include <svxpixelctlaccessiblecontext.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SvxPixelCtlAccessible::SvxPixelCtlAccessible(SvxPixelCtl* pControl)
    : mpPixelCtl(pControl)
{
}

SvxPixelCtlAccessible::~SvxPixelCtlAccessible()
{
    ensureDisposed();
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxPixelCtlAccessible, OAccessibleSelectionHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxPixelCtlAccessible, OAccessibleSelectionHelper, OAccessibleHelper_Base )

uno::Reference< XAccessibleContext > SvxPixelCtlAccessible::getAccessibleContext(  )
{
    return this;
}

sal_Int32 SvxPixelCtlAccessible::getAccessibleChildCount(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return SvxPixelCtl::GetSquares();
}
uno::Reference< XAccessible > SvxPixelCtlAccessible::getAccessibleChild( sal_Int32 i )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if ( i < 0 || i >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();
    Reference< XAccessible > xChild;
    if (mpPixelCtl)
        xChild = CreateChild(i, mpPixelCtl->IndexToPoint(i));
    return xChild;
}

uno::Reference< XAccessible > SvxPixelCtlAccessible::getAccessibleParent(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if (mpPixelCtl)
        return mpPixelCtl->getAccessibleParent();
    return uno::Reference<css::accessibility::XAccessible>();
}

sal_Int16 SvxPixelCtlAccessible::getAccessibleRole(  )
{
    return AccessibleRole::LIST;
}

OUString SvxPixelCtlAccessible::getAccessibleDescription(  )
{

    ::osl::MutexGuard   aGuard( m_aMutex );
    return mpPixelCtl ? mpPixelCtl->GetAccessibleDescription() : "";
}

OUString SvxPixelCtlAccessible::getAccessibleName(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mpPixelCtl ? mpPixelCtl->GetAccessibleName() : "";
}

Reference< XAccessibleRelationSet > SAL_CALL SvxPixelCtlAccessible::getAccessibleRelationSet()
{
    if (mpPixelCtl)
        return mpPixelCtl->get_accessible_relation_set();
    return uno::Reference<css::accessibility::XAccessibleRelationSet>();
}

uno::Reference< XAccessibleStateSet > SvxPixelCtlAccessible::getAccessibleStateSet(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< XAccessibleStateSet > xRet = pStateSetHelper;

    if (mpPixelCtl)
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0
        };

        sal_Int16 nState = 0;
        while (aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if (mpPixelCtl->IsEnabled())
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);
        if (mpPixelCtl->HasFocus())
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
        pStateSetHelper->AddState(AccessibleStateType::MANAGES_DESCENDANTS);
    }

    return xRet;
}

uno::Reference<XAccessible > SAL_CALL SvxPixelCtlAccessible::getAccessibleAtPoint (
        const awt::Point& rPoint)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    Reference< XAccessible >    xRet;

    if (mpPixelCtl)
    {
        tools::Long nIndex = mpPixelCtl->PointToIndex(Point(rPoint.X, rPoint.Y));
        xRet = CreateChild(nIndex, mpPixelCtl->IndexToPoint(nIndex));
    }

    return xRet;
}

awt::Rectangle SvxPixelCtlAccessible::implGetBounds()
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    awt::Rectangle aRet;

    if (mpPixelCtl)
    {
        const Point   aOutPos;
        Size          aOutSize(mpPixelCtl->GetOutputSizePixel());

        aRet.X = aOutPos.X();
        aRet.Y = aOutPos.Y();
        aRet.Width = aOutSize.Width();
        aRet.Height = aOutSize.Height();
    }

    return aRet;
}

void SvxPixelCtlAccessible::grabFocus(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if (mpPixelCtl)
        mpPixelCtl->GrabFocus();
}

sal_Int32 SvxPixelCtlAccessible::getForeground(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxPixelCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetLabelTextColor());
}

sal_Int32 SvxPixelCtlAccessible::getBackground(  )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    //see SvxPixelCtl::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetDialogColor());
}

void SvxPixelCtlAccessible::implSelect(sal_Int32 nChildIndex, bool bSelect)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    if (!mpPixelCtl)
        return;

    tools::Long nIndex = mpPixelCtl->ShowPosition(mpPixelCtl->IndexToPoint(nChildIndex));
    NotifyChild(nIndex, bSelect, false);
}

bool SvxPixelCtlAccessible::implIsSelected(sal_Int32 nChildIndex)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if (!mpPixelCtl)
        return false;

    return mpPixelCtl->GetFocusPosIndex() == nChildIndex;
}

void SAL_CALL SvxPixelCtlAccessible::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OAccessibleSelectionHelper::disposing();
    m_xCurChild.clear();
    mpPixelCtl = nullptr;
}

void SvxPixelCtlAccessible::NotifyChild(tools::Long nIndex,bool bSelect ,bool bCheck)
{
    DBG_ASSERT( !(!bSelect && !bCheck),"" );//non is false

    SvxPixelCtlAccessibleChild *pChild= nullptr;

    if (m_xCurChild.is())
    {
        pChild= static_cast<SvxPixelCtlAccessibleChild*>(m_xCurChild.get());
        DBG_ASSERT(pChild,"Child Must be Valid");
        if (pChild->getAccessibleIndexInParent() == nIndex )
        {
            if (bSelect)
            {
                pChild->SelectChild(true);
            }
            if (bCheck)
            {
                pChild->ChangePixelColorOrBG(mpPixelCtl->GetBitmapPixel(sal_uInt16(nIndex)) != 0);
                pChild->CheckChild();
            }
            return ;
        }
    }
    uno::Reference <XAccessible> xNewChild =CreateChild(nIndex, mpPixelCtl->IndexToPoint(nIndex));
    SvxPixelCtlAccessibleChild *pNewChild= static_cast<SvxPixelCtlAccessibleChild*>(xNewChild.get());
    DBG_ASSERT(pNewChild,"Child Must be Valid");

    Any aNewValue,aOldValue;
    aNewValue<<= xNewChild;
    NotifyAccessibleEvent(AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue);

    if (bSelect)
    {
        if (pChild)
        {
            pChild->SelectChild(false);
        }
        pNewChild->SelectChild(true);
    }
    if (bCheck)
    {
        pNewChild->CheckChild();
    }
    m_xCurChild= xNewChild;
}

uno::Reference<XAccessible> SvxPixelCtlAccessible::CreateChild (tools::Long nIndex,Point mPoint)
{
    bool bPixelColorOrBG = mpPixelCtl->GetBitmapPixel(sal_uInt16(nIndex)) != 0;
    Size size(mpPixelCtl->GetWidth() / SvxPixelCtl::GetLineCount(), mpPixelCtl->GetHeight() / SvxPixelCtl::GetLineCount());
    uno::Reference<XAccessible> xChild = new SvxPixelCtlAccessibleChild(*mpPixelCtl,
                bPixelColorOrBG,
                tools::Rectangle(mPoint,size),
                this,
                nIndex);

    return xChild;
}

void SvxPixelCtlAccessibleChild::CheckChild()
{
    Any aChecked;
    aChecked <<= AccessibleStateType::CHECKED;

    if (m_bPixelColorOrBG)//Current Child State
    {
        NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(), aChecked);
    }
    else
    {
        NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aChecked, Any());
    }
}

void SvxPixelCtlAccessibleChild::SelectChild( bool bSelect)
{
    Any aSelected;
    aSelected <<= AccessibleStateType::SELECTED;

    if (bSelect)
    {
        NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(), aSelected);
    }
    else
    {
        NotifyAccessibleEvent(AccessibleEventId::STATE_CHANGED, aSelected, Any());
    }
}

SvxPixelCtlAccessibleChild::SvxPixelCtlAccessibleChild( SvxPixelCtl& rWindow, bool bPixelColorOrBG,
    const tools::Rectangle& rBoundingBox, const rtl::Reference<SvxPixelCtlAccessible>& rxParent,
    tools::Long nIndexInParent)
    : mrParentWindow( rWindow )
    , mxParent(rxParent)
    , m_bPixelColorOrBG(bPixelColorOrBG)
    , maBoundingBox( rBoundingBox )
    , mnIndexInParent( nIndexInParent )
{
}

SvxPixelCtlAccessibleChild::~SvxPixelCtlAccessibleChild()
{
    ensureDisposed();
}

IMPLEMENT_FORWARD_XINTERFACE2( SvxPixelCtlAccessibleChild, OAccessibleComponentHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( SvxPixelCtlAccessibleChild, OAccessibleComponentHelper, OAccessibleHelper_Base )

// XAccessible
uno::Reference< XAccessibleContext> SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleContext()
{
    return this;
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleAtPoint( const awt::Point& )
{
    return uno::Reference< XAccessible >();
}

void SAL_CALL SvxPixelCtlAccessibleChild::grabFocus()
{
}

sal_Int32 SvxPixelCtlAccessibleChild::getForeground()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mxParent.is() ? mxParent->getForeground() : -1;
}

sal_Int32 SvxPixelCtlAccessibleChild::getBackground()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mxParent.is() ? mxParent->getBackground() : -1;
}

// XAccessibleContext
sal_Int32 SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleChildCount()
{
    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleChild( sal_Int32 )
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleParent()
{
    return mxParent.get();
}

sal_Int16 SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleRole()
{
    return AccessibleRole::CHECK_BOX;
}

OUString SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleDescription()
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    return  GetName();
}

OUString SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleName()
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return  GetName();
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleRelationSet()
{
    return uno::Reference< XAccessibleRelationSet >();
}

uno::Reference< XAccessibleStateSet > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleStateSet()
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if (!rBHelper.bDisposed)
    {

        pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
        pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );

        tools::Long nIndex = mrParentWindow.GetFocusPosIndex();
        if ( nIndex == mnIndexInParent)
        {
            pStateSetHelper->AddState( AccessibleStateType::SELECTED );
        }
        if (mrParentWindow.GetBitmapPixel(sal_uInt16(mnIndexInParent)))
        {
            pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        }
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

void SAL_CALL SvxPixelCtlAccessibleChild::disposing()
{
    OAccessibleComponentHelper::disposing();
    mxParent.clear();
}

awt::Rectangle SvxPixelCtlAccessibleChild::implGetBounds()
{
    // no guard necessary, because no one changes maBoundingBox after creating it
    return AWTRectangle(maBoundingBox);
}

OUString SvxPixelCtlAccessibleChild::GetName() const
{
    sal_Int32 nXIndex = mnIndexInParent % SvxPixelCtl::GetLineCount();
    sal_Int32 nYIndex = mnIndexInParent / SvxPixelCtl::GetLineCount();

    OUString str = "("
                 + OUString::number(nXIndex)
                 + ","
                 + OUString::number(nYIndex)
                 + ")";
    return str;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
