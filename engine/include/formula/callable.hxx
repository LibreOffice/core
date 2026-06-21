/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <formula/formuladllapi.h>
#include <formula/opcode.hxx>
#include <formula/types.hxx>
#include <osl/interlck.h>
#include <rtl/ustring.hxx>

namespace formula
{
/// represents a function that can be called to produce a result
class FORMULA_DLLPUBLIC FormulaCallable
{
private:
    mutable oslInterlockedCount mnRefCnt;

protected:
    FormulaCallable()
        : mnRefCnt(0)
    {
    }

    FormulaCallable(const FormulaCallable&)
        : mnRefCnt(0)
    {
    }

public:
    void IncRef() const { osl_atomic_increment(&mnRefCnt); }
    void DecRef() const
    {
        if (!osl_atomic_decrement(&mnRefCnt))
            const_cast<FormulaCallable*>(this)->Delete();
    }
    oslInterlockedCount GetRef() const { return mnRefCnt; }
    void Delete() { delete this; }
    void DeleteIfZeroRef()
    {
        if (mnRefCnt == 0)
            delete this;
    }

    virtual ~FormulaCallable() {}
    virtual OpCode GetOpCode() const { return ocNone; }
};

inline void intrusive_ptr_add_ref(const FormulaCallable* p) { p->IncRef(); }
inline void intrusive_ptr_release(const FormulaCallable* p) { p->DecRef(); }

/// a callable built-in function
class FORMULA_DLLPUBLIC FormulaBuiltInFunction : public FormulaCallable
{
private:
    OpCode meOpCode;

    /// no copy constructor; copies are not allowed!
    FormulaBuiltInFunction(OpCode eOpCode)
        : FormulaCallable()
        , meOpCode(eOpCode)
    {
    }

public:
    /// use Get instead of constructing new objects
    static FormulaCallableRef Get(OpCode eOpCode);

    virtual OpCode GetOpCode() const override { return meOpCode; }
};

/// a callable external function
class FORMULA_DLLPUBLIC FormulaExternalFunction : public FormulaCallable
{
private:
    OUString maName;

public:
    FormulaExternalFunction(const OUString& aName)
        : FormulaCallable()
        , maName(aName)
    {
    }
    FormulaExternalFunction(const FormulaExternalFunction& r)
        : FormulaCallable(r)
        , maName(r.maName)
    {
    }

    virtual OpCode GetOpCode() const override { return ocExternal; }
    const OUString& GetName() const { return maName; }
    FormulaCallable* Clone() const { return new FormulaExternalFunction(*this); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
