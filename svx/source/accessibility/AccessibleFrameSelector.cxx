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

#include <AccessibleFrameSelector.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/frmsel.hxx>
#include <svx/dialmgr.hxx>

#include <frmsel.hrc>

namespace svx::a11y {

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;

using namespace ::com::sun::star::accessibility;


AccFrameSelector::AccFrameSelector(FrameSelector& rFrameSel)
    : mpFrameSel(&rFrameSel)
{
}

AccFrameSelector::~AccFrameSelector()
{
}

IMPLEMENT_FORWARD_XINTERFACE2( AccFrameSelector, OAccessibleComponentHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccFrameSelector, OAccessibleComponentHelper, OAccessibleHelper_Base )

Reference< XAccessibleContext > AccFrameSelector::getAccessibleContext(  )
{
    return this;
}

sal_Int32 AccFrameSelector::getAccessibleChildCount(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetEnabledBorderCount();
}

Reference< XAccessible > AccFrameSelector::getAccessibleChild( sal_Int32 i )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet = mpFrameSel->GetChildAccessible( i );
    if( !xRet.is() )
        throw RuntimeException();
    return xRet;
}

Reference< XAccessible > AccFrameSelector::getAccessibleParent(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet = mpFrameSel->getAccessibleParent();
    return xRet;
}

sal_Int16 AccFrameSelector::getAccessibleRole(  )
{
    return AccessibleRole::OPTION_PANE;
}

OUString AccFrameSelector::getAccessibleDescription(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_DESCRIPTIONS[0].first);
}

OUString AccFrameSelector::getAccessibleName(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_TEXTS[0].first);
}

Reference< XAccessibleRelationSet > AccFrameSelector::getAccessibleRelationSet(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->get_accessible_relation_set();
}

Reference< XAccessibleStateSet > AccFrameSelector::getAccessibleStateSet(  )
{
    SolarMutexGuard aGuard;
    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper = new utl::AccessibleStateSetHelper;

    if(!mpFrameSel)
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);
    else
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::EDITABLE,
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::MULTI_SELECTABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0};
        sal_Int16 nState = 0;
        while(aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if(mpFrameSel->IsEnabled())
        {
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);
            pStateSetHelper->AddState(AccessibleStateType::SENSITIVE);
        }

        if (mpFrameSel->HasFocus())
        {
            pStateSetHelper->AddState(AccessibleStateType::ACTIVE);
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
            pStateSetHelper->AddState(AccessibleStateType::SELECTED);
        }
    }
    return pStateSetHelper;
}

Reference< XAccessible > AccFrameSelector::getAccessibleAtPoint(
    const css::awt::Point& aPt )
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

void AccFrameSelector::grabFocus(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    mpFrameSel->GrabFocus();
}

sal_Int32 AccFrameSelector::getForeground(  )
{
    SolarMutexGuard aGuard;

    //see FrameSelector::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetLabelTextColor());
}

sal_Int32 AccFrameSelector::getBackground(  )
{
    SolarMutexGuard aGuard;

    //see FrameSelector::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetDialogColor());
}

css::awt::Rectangle AccFrameSelector::implGetBounds()
{
    SolarMutexGuard aGuard;
    IsValid();

    css::awt::Rectangle aRet;

    const Point   aOutPos;
    Size          aOutSize(mpFrameSel->GetOutputSizePixel());

    aRet.X = aOutPos.X();
    aRet.Y = aOutPos.Y();
    aRet.Width = aOutSize.Width();
    aRet.Height = aOutSize.Height();

    return aRet;
}

void AccFrameSelector::IsValid()
{
    if(!mpFrameSel)
        throw RuntimeException();
}

void AccFrameSelector::Invalidate()
{
    mpFrameSel = nullptr;
}

AccFrameSelectorChild::AccFrameSelectorChild(FrameSelector& rFrameSel, FrameBorderType eBorder)
    : mpFrameSel(&rFrameSel)
    , meBorder(eBorder)
{
}

AccFrameSelectorChild::~AccFrameSelectorChild()
{
}

IMPLEMENT_FORWARD_XINTERFACE2( AccFrameSelectorChild, OAccessibleComponentHelper, OAccessibleHelper_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccFrameSelectorChild, OAccessibleComponentHelper, OAccessibleHelper_Base )

Reference< XAccessibleContext > AccFrameSelectorChild::getAccessibleContext(  )
{
    return this;
}

sal_Int32 AccFrameSelectorChild::getAccessibleChildCount(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return 0;
}

Reference< XAccessible > AccFrameSelectorChild::getAccessibleChild( sal_Int32 )
{
    throw RuntimeException();
}

Reference< XAccessible > AccFrameSelectorChild::getAccessibleParent(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet = mpFrameSel->CreateAccessible();
    return xRet;
}

sal_Int16 AccFrameSelectorChild::getAccessibleRole(  )
{
    return AccessibleRole::CHECK_BOX;
}

OUString AccFrameSelectorChild::getAccessibleDescription(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_DESCRIPTIONS[static_cast<sal_uInt32>(meBorder)].first);
}

OUString AccFrameSelectorChild::getAccessibleName(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_TEXTS[static_cast<sal_uInt32>(meBorder)].first);
}

Reference< XAccessibleRelationSet > AccFrameSelectorChild::getAccessibleRelationSet(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessibleRelationSet > xRet = new utl::AccessibleRelationSetHelper;
    return xRet;
}

Reference< XAccessibleStateSet > AccFrameSelectorChild::getAccessibleStateSet(  )
{
    SolarMutexGuard aGuard;
    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper = new utl::AccessibleStateSetHelper;

    if(!mpFrameSel)
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);
    else
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::EDITABLE,
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::MULTI_SELECTABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0};
        sal_Int16 nState = 0;
        while(aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if(mpFrameSel->IsEnabled())
        {
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);
            pStateSetHelper->AddState(AccessibleStateType::SENSITIVE);
        }

        if (mpFrameSel->HasFocus() && mpFrameSel->IsBorderSelected(meBorder))
        {
            pStateSetHelper->AddState(AccessibleStateType::ACTIVE);
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
            pStateSetHelper->AddState(AccessibleStateType::SELECTED);
        }
    }
    return pStateSetHelper;
}

Reference< XAccessible > AccFrameSelectorChild::getAccessibleAtPoint(
    const css::awt::Point& aPt )
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

css::awt::Rectangle AccFrameSelectorChild::implGetBounds(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    const tools::Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
    Point aPos = aSpot.TopLeft();
    Size aSz = aSpot.GetSize();
    css::awt::Rectangle aRet;
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}

void AccFrameSelectorChild::grabFocus(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    mpFrameSel->GrabFocus();
}

sal_Int32 AccFrameSelectorChild::getForeground(  )
{
    SolarMutexGuard aGuard;

    //see FrameSelector::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetLabelTextColor());
}

sal_Int32 AccFrameSelectorChild::getBackground(  )
{
    SolarMutexGuard aGuard;

    //see FrameSelector::Paint
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    return sal_Int32(rStyles.GetDialogColor());
}

void AccFrameSelectorChild::IsValid()
{
    if(!mpFrameSel)
        throw RuntimeException();
}

void AccFrameSelectorChild::Invalidate()
{
    mpFrameSel = nullptr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
