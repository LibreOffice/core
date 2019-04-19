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
#pragma once

inline const bool& TaskManager::MyProperty::IsNative() const
{
    return isNative;
}
inline const sal_Int32& TaskManager::MyProperty::getHandle() const
{
    return Handle;
}
inline const css::uno::Type& TaskManager::MyProperty::getType() const
{
    return Typ;
}
inline const css::uno::Any& TaskManager::MyProperty::getValue() const
{
    return Value;
}
inline const css::beans::PropertyState& TaskManager::MyProperty::getState() const
{
    return State;
}
inline const sal_Int16& TaskManager::MyProperty::getAttributes() const
{
    return Attributes;
}
inline void TaskManager::MyProperty::setValue( const css::uno::Any& theValue ) const
{
    const_cast<MyProperty*>(this)->Value = theValue;
}
inline void TaskManager::MyProperty::setState( const css::beans::PropertyState& theState ) const
{
    const_cast<MyProperty*>(this)->State = theState;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
