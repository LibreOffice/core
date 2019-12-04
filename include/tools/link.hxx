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

#include <sal/macros.h>
#include <sal/types.h>

#define DECL_LINK(Member, ArgType, RetType) \
    static RetType LinkStub##Member(void *, ArgType); \
    RetType Member(ArgType)

#define DECL_STATIC_LINK(Class, Member, ArgType, RetType) \
    static RetType LinkStub##Member(void *, ArgType); \
    static RetType Member(Class *, ArgType)

#define DECL_DLLPRIVATE_LINK(Member, ArgType, RetType) \
    SAL_DLLPRIVATE static RetType LinkStub##Member(void *, ArgType); \
    SAL_DLLPRIVATE RetType Member(ArgType)

#define DECL_DLLPRIVATE_STATIC_LINK(Class, Member, ArgType, RetType) \
    SAL_DLLPRIVATE static RetType LinkStub##Member(void *, ArgType); \
    SAL_DLLPRIVATE static RetType Member(Class *, ArgType)

#define IMPL_LINK(Class, Member, ArgType, ArgName, RetType) \
    RetType Class::LinkStub##Member(void * instance, ArgType data) { \
        return static_cast<Class *>(instance)->Member(data); \
    } \
    RetType Class::Member(ArgType ArgName)

#define IMPL_LINK_NOARG(Class, Member, ArgType, RetType) \
    RetType Class::LinkStub##Member(void * instance, ArgType data) { \
        return static_cast<Class *>(instance)->Member(data); \
    } \
    RetType Class::Member(SAL_UNUSED_PARAMETER ArgType)

#define IMPL_STATIC_LINK( \
        Class, Member, ArgType, ArgName, RetType) \
    RetType Class::LinkStub##Member(void * instance, ArgType data) { \
        return Member(static_cast<Class *>(instance), data); \
    } \
    RetType Class::Member(SAL_UNUSED_PARAMETER Class *, ArgType ArgName)

#define IMPL_STATIC_LINK_NOARG( \
        Class, Member, ArgType, RetType) \
    RetType Class::LinkStub##Member(void * instance, ArgType data) { \
        return Member(static_cast<Class *>(instance), data); \
    } \
    RetType Class::Member( \
        SAL_UNUSED_PARAMETER Class *, SAL_UNUSED_PARAMETER ArgType)

#ifdef DBG_UTIL
#define LINK(Instance, Class, Member) ::tools::detail::makeLink( \
    ::tools::detail::castTo<Class *>(Instance), &Class::LinkStub##Member, __FILE__, __LINE__, SAL_STRINGIFY(Class::LinkStub##Member))
#else
#define LINK(Instance, Class, Member) ::tools::detail::makeLink( \
    ::tools::detail::castTo<Class *>(Instance), &Class::LinkStub##Member)
#endif

template<typename Arg, typename Ret>
class SAL_WARN_UNUSED Link {
public:
    typedef Ret Stub(void *, Arg);

#ifdef DBG_UTIL
    Link()
        : function_(nullptr)
        , instance_(nullptr)
        , file_("unknown")
        , line_(0)
        , target_("unknown")
    {
    }

    Link(void* instance, Stub* function, const char* const file = "unknown", const int line = 0,
         const char* const target = "unknown")
        : function_(function)
        , instance_(instance)
        , file_(file)
        , line_(line)
        , target_(target)
    {
    }
#else
    Link(): function_(nullptr), instance_(nullptr) {}

    Link(void * instance, Stub * function):
        function_(function), instance_(instance) {}
#endif

    Ret Call(Arg data) const
    { return function_ == nullptr ? Ret() : (*function_)(instance_, data); }

    bool IsSet() const { return function_ != nullptr; }

    bool operator !() const { return !IsSet(); }

    bool operator <(Link const & other) const {
        char* ptr1 = reinterpret_cast<char*>(function_);
        char* ptr2 = reinterpret_cast<char*>(other.function_);
        if (ptr1 < ptr2)
            return true;
        else if (ptr1 > ptr2)
            return false;
        else
            return instance_ < other.instance_;
    };

    bool operator ==(Link const & other) const
    { return function_ == other.function_ && instance_ == other.instance_; };

    void *GetInstance() const { return instance_; }

#ifdef DBG_UTIL
    const char* getSourceFilename() const { return file_; }
    int getSourceLineNumber() const { return line_; }
    const char* getTargetName() const { return target_; }
#endif

private:
    Stub * function_;
    void * instance_;

#ifdef DBG_UTIL
    /// Support tracing link source and target.
    /// When debugging async events, it's often critical
    /// to find out not only where a link leads (i.e. the target
    /// function), but also where it was created (file:line).
    const char* file_;
    int line_;
    const char* target_;
#endif
};

// Class used to indicate that the Call() parameter is not in use:
class LinkParamNone { LinkParamNone() = delete; };

namespace tools { namespace detail {

// Avoids loplugin:redundantcast in LINK macro, in the common case that Instance
// is already of type Class * (instead of a derived type):
template<typename To, typename From> To castTo(From from)
{ return static_cast<To>(from); }

#ifdef DBG_UTIL
template<typename Arg, typename Ret>
Link<Arg, Ret> makeLink(void * instance, Ret (* function)(void *, Arg), const char* file, int line, const char* target) {
    return Link<Arg, Ret>(instance, function, file, line, target);
}
#else
template<typename Arg, typename Ret>
Link<Arg, Ret> makeLink(void * instance, Ret (* function)(void *, Arg)) {
    return Link<Arg, Ret>(instance, function);
}
#endif

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
