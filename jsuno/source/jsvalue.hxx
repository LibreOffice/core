/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <quickjs.h>
#include <cassert>

namespace jsuno
{
class ValueRef
{
public:
    ValueRef(JSRuntime* rt, JSValue val = JS_UNINITIALIZED)
        : rt_(rt)
        , val_(val)
    {
    }

    ValueRef(JSContext* ctx, JSValue val = JS_UNINITIALIZED)
        : ValueRef(JS_GetRuntime(ctx), val)
    {
    }

    ValueRef(ValueRef&& ref)
        : rt_(ref.rt_)
        , val_(ref.val_)
    {
        ref.val_ = JS_UNINITIALIZED;
    }

    ~ValueRef() { JS_FreeValueRT(rt_, val_); }

    ValueRef& operator=(ValueRef&& ref)
    {
        assert(rt_ == ref.rt_);
        JS_FreeValueRT(rt_, val_);
        val_ = ref.val_;
        ref.val_ = JS_UNINITIALIZED;
        return *this;
    }

    ValueRef& operator=(JSValue val)
    {
        JS_FreeValueRT(rt_, val_);
        val_ = val;
        return *this;
    }

    operator JSValueConst() const { return val_; }

    JSValue dup() const { return JS_DupValueRT(rt_, val_); }

    JSValue release()
    {
        auto const val = val_;
        val_ = JS_UNINITIALIZED;
        return val;
    }

    JSValue* ptr() { return &val_; }

private:
    ValueRef(ValueRef const&) = delete;
    void operator=(ValueRef const&) = delete;

    JSRuntime* rt_;
    JSValue val_;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
