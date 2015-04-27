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

#ifndef INCLUDED_TOOLS_LINK_HXX
#define INCLUDED_TOOLS_LINK_HXX

#include <sal/config.h>

#include <sal/types.h>
#include <tools/toolsdllapi.h>

#define DECL_LINK(Member, ArgType) \
    static sal_IntPtr LinkStub##Member(void *, void *); \
    sal_IntPtr Member(ArgType)

#define DECL_STATIC_LINK(Class, Member, ArgType) \
    static sal_IntPtr LinkStub##Member(void *, void *); \
    static sal_IntPtr Member(Class *, ArgType)

#define DECL_DLLPRIVATE_LINK(Member, ArgType) \
    SAL_DLLPRIVATE static sal_IntPtr LinkStub##Member(void *, void *); \
    SAL_DLLPRIVATE sal_IntPtr Member(ArgType)

#define DECL_DLLPRIVATE_STATIC_LINK(Class, Member, ArgType) \
    SAL_DLLPRIVATE static sal_IntPtr LinkStub##Member(void *, void *); \
    SAL_DLLPRIVATE static sal_IntPtr Member(Class *, ArgType)

#define IMPL_LINK(Class, Member, ArgType, ArgName) \
    sal_IntPtr Class::LinkStub##Member(void * instance, void * data) { \
        return static_cast<Class *>(instance)->Member( \
            static_cast<ArgType>(data)); \
    } \
    sal_IntPtr Class::Member(ArgType ArgName)

#define IMPL_LINK_NOARG(Class, Member) \
    sal_IntPtr Class::LinkStub##Member(void * instance, void * data) { \
        return static_cast<Class *>(instance)->Member(data); \
    } \
    sal_IntPtr Class::Member(SAL_UNUSED_PARAMETER void *)

#define IMPL_STATIC_LINK(Class, Member, ArgType, ArgName) \
    sal_IntPtr Class::LinkStub##Member(void * instance, void * data) { \
        return Member( \
            static_cast<Class *>(instance), static_cast<ArgType>(data)); \
    } \
    sal_IntPtr Class::Member(Class * pThis, ArgType ArgName)

#define IMPL_STATIC_LINK_NOINSTANCE(Class, Member, ArgType, ArgName) \
    sal_IntPtr Class::LinkStub##Member(void * instance, void * data) { \
        return Member( \
            static_cast<Class *>(instance), static_cast<ArgType>(data)); \
    } \
    sal_IntPtr Class::Member(SAL_UNUSED_PARAMETER Class *, ArgType ArgName)

#define IMPL_STATIC_LINK_NOINSTANCE_NOARG(Class, Member) \
    sal_IntPtr Class::LinkStub##Member(void * instance, void * data) { \
        return Member(static_cast<Class *>(instance), data); \
    } \
    sal_IntPtr Class::Member( \
        SAL_UNUSED_PARAMETER Class *, SAL_UNUSED_PARAMETER void *)

#define LINK(Instance, Class, Member) \
    Link(static_cast<Class *>(Instance), &Class::LinkStub##Member)

#define EMPTYARG

class TOOLS_DLLPUBLIC Link {
public:
    typedef sal_IntPtr Stub(void *, void *);

    Link(): function_(nullptr), instance_(nullptr) {}

    Link(void * instance, Stub * function):
        function_(function), instance_(instance) {}

    sal_IntPtr Call(void * data) const
    { return function_ == nullptr ? 0 : (*function_)(instance_, data); }

    bool IsSet() const { return function_ != nullptr; }

    bool operator !() const { return !IsSet(); }

    bool operator <(Link const & other) const {
        return reinterpret_cast<sal_uIntPtr>(function_)
            < reinterpret_cast<sal_uIntPtr>(other.function_);
    };

    bool operator ==(Link const & other) const
    { return function_ == other.function_ && instance_ == other.instance_; };

    bool operator !=(Link const & other) const { return !operator ==(other); };

private:
    Stub * function_;
    void * instance_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
