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

#include <AccessibleContextBase.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svl/hint.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <utility>
#include <vcl/unohelp.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

ScAccessibleContextBase::ScAccessibleContextBase(
                                                 uno::Reference<XAccessible> xParent,
                                                 const sal_Int16 aRole)
                                                 :
    mxParent(std::move(xParent)),
    maRole(aRole)
{
}

ScAccessibleContextBase::~ScAccessibleContextBase()
{
    if (!IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call of dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void ScAccessibleContextBase::Init()
{
    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xKeepAlive(this);

    msName = createAccessibleName();
    msDescription = createAccessibleDescription();
}

void SAL_CALL ScAccessibleContextBase::disposing()
{
    SolarMutexGuard aGuard;

    // hold reference to make sure that the destructor is not called
    uno::Reference< XAccessibleContext > xKeepAlive(this);

    OAccessibleComponentHelper::disposing();

    mxParent.clear();
}


//=====  SfxListener  =====================================================

void ScAccessibleContextBase::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        // it seems the Broadcaster is dying, since the view is dying
        dispose();
    }
}

//=====  XAccessible  =========================================================

uno::Reference< XAccessibleContext> SAL_CALL
    ScAccessibleContextBase::getAccessibleContext()
{
    return this;
}

// OAccessibleComponentHelper

awt::Rectangle ScAccessibleContextBase::implGetBounds(  )
{
    return vcl::unohelper::ConvertToAWTRect(GetBoundingBox());
}

//=====  XAccessibleComponent  ================================================

awt::Point SAL_CALL ScAccessibleContextBase::getLocationOnScreen(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    return vcl::unohelper::ConvertToAWTPoint(GetBoundingBoxOnScreen().TopLeft());
}

bool ScAccessibleContextBase::isShowing(  )
{
    SolarMutexGuard aGuard;
    ensureAlive();
    bool bShowing(false);
    if (mxParent.is())
    {
        uno::Reference<XAccessibleComponent> xParentComponent (mxParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            tools::Rectangle aParentBounds(
                vcl::unohelper::ConvertToVCLRect(xParentComponent->getBounds()));
            tools::Rectangle aBounds(vcl::unohelper::ConvertToVCLRect(getBounds()));
            bShowing = aBounds.Overlaps(aParentBounds);
        }
    }
    return bShowing;
}

bool ScAccessibleContextBase::isVisible()
{
    return true;
}

void SAL_CALL ScAccessibleContextBase::grabFocus(  )
{
    OSL_FAIL("not implemented");
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getForeground(  )
{
    return sal_Int32(COL_BLACK);
}

sal_Int32 SAL_CALL ScAccessibleContextBase::getBackground(  )
{
    return sal_Int32(COL_WHITE);
}

//=====  XAccessibleContext  ==================================================

uno::Reference<XAccessible> SAL_CALL
       ScAccessibleContextBase::getAccessibleParent()
{
    return mxParent;
}

sal_Int16 SAL_CALL
    ScAccessibleContextBase::getAccessibleRole()
{
    return maRole;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleDescription()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    if (msDescription.isEmpty())
    {
        OUString sDescription(createAccessibleDescription());

        if (msDescription != sDescription)
        {
            const OUString sOldDescription = msDescription;
            msDescription = sDescription;

            CommitChange(AccessibleEventId::DESCRIPTION_CHANGED, uno::Any(sOldDescription),
                         uno::Any(sDescription));
        }
    }
    return msDescription;
}

OUString SAL_CALL
       ScAccessibleContextBase::getAccessibleName()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    if (msName.isEmpty())
    {
        OUString sName(createAccessibleName());
        OSL_ENSURE(!sName.isEmpty(), "We should give always a name.");

        if (msName != sName)
        {
            const OUString sOldName = msName;
            msName = sName;

            CommitChange(AccessibleEventId::NAME_CHANGED, uno::Any(sOldName), uno::Any(sName));
        }
    }
    return msName;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleContextBase::getAccessibleRelationSet()
{
    return new utl::AccessibleRelationSetHelper();
}

sal_Int64 SAL_CALL ScAccessibleContextBase::getAccessibleStateSet()
{
    return 0;
}

lang::Locale SAL_CALL
       ScAccessibleContextBase::getLocale()
{
    SolarMutexGuard aGuard;
    ensureAlive();
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xParentContext (
            mxParent->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale ();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException ();
}

// XServiceInfo
OUString SAL_CALL ScAccessibleContextBase::getImplementationName()
{
    return u"ScAccessibleContextBase"_ustr;
}

sal_Bool SAL_CALL ScAccessibleContextBase::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       ScAccessibleContextBase::getSupportedServiceNames()
{
    return {u"com.sun.star.accessibility.Accessible"_ustr,
            u"com.sun.star.accessibility.AccessibleContext"_ustr};
}

//=====  internal  ============================================================

OUString
    ScAccessibleContextBase::createAccessibleDescription()
{
    OSL_FAIL("should be implemented in the abbreviated class");
    return OUString();
}

OUString ScAccessibleContextBase::createAccessibleName()
{
    OSL_FAIL("should be implemented in the abbreviated class");
    return OUString();
}

void ScAccessibleContextBase::CommitChange(const sal_Int16 nEventId, const css::uno::Any& rOldValue,
                                           const css::uno::Any& rNewValue, sal_Int32 nIndexHint)
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue, nIndexHint);
}

void ScAccessibleContextBase::CommitFocusGained()
{
    CommitChange(AccessibleEventId::STATE_CHANGED, uno::Any(),
                 uno::Any(AccessibleStateType::FOCUSED));
}

void ScAccessibleContextBase::CommitFocusLost()
{
    CommitChange(AccessibleEventId::STATE_CHANGED, uno::Any(AccessibleStateType::FOCUSED),
                 uno::Any());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
