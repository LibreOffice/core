/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "AccessibleFocusManager.hxx"
#include "AccessibleObject.hxx"

std::shared_ptr<AccessibleFocusManager> AccessibleFocusManager::mpInstance;

std::shared_ptr<AccessibleFocusManager> const & AccessibleFocusManager::Instance()
{
    if ( ! mpInstance)
    {
        mpInstance.reset(new AccessibleFocusManager());
    }
    return mpInstance;
}

AccessibleFocusManager::AccessibleFocusManager()
{
}

AccessibleFocusManager::~AccessibleFocusManager()
{
    // copy member to stack, then drop it - otherwise will get use-after-free
    // from AccessibleObject::disposing(), it will call ~Reference *twice*
    auto const temp(std::move(maFocusableObjects));
    (void) temp;
    m_isInDtor = true;
}

void AccessibleFocusManager::AddFocusableObject (
    const ::rtl::Reference<AccessibleObject>& rpObject)
{
    OSL_ASSERT(rpObject.is());
    OSL_ASSERT(::std::find(maFocusableObjects.begin(),maFocusableObjects.end(), rpObject)==maFocusableObjects.end());

    maFocusableObjects.push_back(rpObject);
}

void AccessibleFocusManager::RemoveFocusableObject (
    const ::rtl::Reference<AccessibleObject>& rpObject)
{
    ::std::vector<rtl::Reference<AccessibleObject> >::iterator iObject (
        ::std::find(maFocusableObjects.begin(),maFocusableObjects.end(), rpObject));

    if (iObject != maFocusableObjects.end())
        maFocusableObjects.erase(iObject);
    else
    {
        OSL_ASSERT(m_isInDtor); // in dtor, was removed already
    }
}

void AccessibleFocusManager::FocusObject (
    const ::rtl::Reference<AccessibleObject>& rpObject)
{
    // Remove the focus of any of the other focusable objects.
    for (auto& rxObject : maFocusableObjects)
    {
        if (rxObject!=rpObject)
            rxObject->SetIsFocused(false);
    }

    if (rpObject.is())
        rpObject->SetIsFocused(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
