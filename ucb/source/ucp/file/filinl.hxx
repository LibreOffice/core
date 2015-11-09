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
#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_FILINL_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_FILINL_HXX

inline const bool& SAL_CALL shell::MyProperty::IsNative() const
{
    return isNative;
}
inline const sal_Int32& SAL_CALL shell::MyProperty::getHandle() const
{
    return Handle;
}
inline const css::uno::Type& SAL_CALL shell::MyProperty::getType() const
{
    return Typ;
}
inline const css::uno::Any& SAL_CALL shell::MyProperty::getValue() const
{
    return Value;
}
inline const css::beans::PropertyState& SAL_CALL shell::MyProperty::getState() const
{
    return State;
}
inline const sal_Int16& SAL_CALL shell::MyProperty::getAttributes() const
{
    return Attributes;
}
inline void SAL_CALL shell::MyProperty::setValue( const css::uno::Any& __Value ) const
{
    const_cast<MyProperty*>(this)->Value = __Value;
}
inline void SAL_CALL shell::MyProperty::setState( const css::beans::PropertyState& __State ) const
{
    const_cast<MyProperty*>(this)->State = __State;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
