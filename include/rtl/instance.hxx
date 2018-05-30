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

#ifndef INCLUDED_RTL_INSTANCE_HXX
#define INCLUDED_RTL_INSTANCE_HXX

#include "sal/config.h"

#include <cstddef>

#include "osl/doublecheckedlocking.h"
#include "osl/getglobalmutex.hxx"

namespace {

/** A non-broken version of the double-checked locking pattern.

    See
    <http://www.cs.umd.edu/~pugh/java/memoryModel/DoubleCheckedLocking.html>
    for a description of double-checked locking, why it is broken, and how it
    can be fixed.  Always use this template instead of spelling out the
    double-checked locking pattern explicitly, and only in those rare cases
    where that is not possible and you have to spell it out explicitly, at
    least call OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER() at the right
    places.  That way, all platform-dependent code to make double-checked
    locking work can be kept in one place.

    Usage scenarios:

    1  Static instance (most common case)

    Pattern:

      T * getInstance()
      {
          static T * pInstance = 0;
          if (!pInstance)
          {
              ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());
              if (!pInstance)
              {
                  static T aInstance;
                  pInstance = &aInstance;
              }
          }
          return pInstance;
      }

    Code:

      #include <rtl/instance.hxx>
      #include <osl/getglobalmutex.hxx>

      namespace {
          struct Init
          {
              T * operator()()
              {
                  static T aInstance;
                  return &aInstance;
              }
          };
      }

      T * getInstance()
      {
          return rtl_Instance< T, Init, ::osl::MutexGuard,
                               ::osl::GetGlobalMutex >::create(
              Init(), ::osl::GetGlobalMutex());
      }

    2  Dynamic instance

    Pattern:

      T * getInstance()
      {
          static T * pInstance = 0;
          if (!pInstance)
          {
              ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());
              if (!pInstance)
                  pInstance = new T;
          }
          return pInstance;
      }

    Code:

      #include <rtl/instance.hxx>
      #include <osl/getglobalmutex.hxx>

      namespace {
          struct Init
          {
              T * operator()()
              {
                  return new T;
              }
          };
      }

      T * getInstance()
      {
          return rtl_Instance< T, Init, ::osl::MutexGuard,
                               ::osl::GetGlobalMutex >::create(
              Init(), ::osl::GetGlobalMutex());
      }

    3  Other guard/mutex

    Pattern:

      T * getInstance()
      {
          static T * pInstance = 0;
          if (!pInstance)
          {
              SomeGuard aGuard(pSomeMutex);
              if (!pInstance)
              {
                  static T aInstance;
                  pInstance = &aInstance;
              }
          }
          return pInstance;
      }

    Code:

      #include <rtl/instance.hxx>

      namespace {
          struct InitInstance
          {
              T * operator()()
              {
                  static T aInstance;
                  return &aInstance;
              }
          };

          struct InitGuard
          {
              SomeMutex * operator()()
              {
                  return pSomeMutex;
              }
          };
      }

      T * getInstance()
      {
          return rtl_Instance< T, InitInstance,
                               SomeGuard, InitGuard >::create(
              InitInstance(), InitMutex());
      }

    4  Calculate extra data

    Pattern:

      T * getInstance()
      {
          static T * pInstance = 0;
          if (!pInstance)
          {
              Data aData(...);
              ::osl::MutexGuard aGuard(::osl::Mutex::getGlobalMutex());
              if (!pInstance)
              {
                  static T aInstance(aData);
                  pInstance = &aInstance;
              }
          }
          return pInstance;
      }

    Code:

      #include <rtl/instance.hxx>
      #include <osl/getglobalmutex.hxx>

      namespace {
          struct InitInstance
          {
              T * operator()()
              {
                  static T aInstance;
                  return &aInstance;
              }
          }

          struct InitData
          {
              Data const & operator()()
              {
                  return ...;
              }
          }
      }

      T * getInstance()
      {
          return rtl_Instance< T, InitInstance,
                               ::osl::MutexGuard, ::osl::GetGlobalMutex,
                               Data, InitData >::create(
              InitInstance(), ::osl::GetGlobalMutex(), InitData());
      }

    Some comments:

    For any instantiation of rtl_Instance, at most one call to a create method
    may occur in the program code:  Each occurrence of a create method within
    the program code is supposed to return a fresh object instance on the
    first call, and that same object instance on subsequent calls; but
    independent occurrences of create methods are supposed to return
    independent object instances.  Since there is a one-to-one correspondence
    between object instances and instantiations of rtl_Instance, the
    requirement should be clear.  One measure to enforce the requirement is
    that rtl_Instance lives in an unnamed namespace, so that instantiations of
    rtl_Instance in different translation units will definitely be different
    instantiations.  A drawback of that measure is that the name of the class
    needs a funny "hand coded" prefix "rtl_" instead of a proper namespace
    prefix like "::rtl::".

    A known problem with this template is when two occurrences of calls to
    create methods with identical template arguments appear in one translation
    unit.  Those two places will share a single object instance.  This can be
    avoided by using different Init structs (see the above code samples) in
    the two places.

    There is no need to make m_pInstance volatile, in order to avoid usage of
    stale copies of m_pInstance:  At the first check, a thread will see that
    m_pInstance contains either 0 or a valid pointer.  If it contains a valid
    pointer, it cannot be stale, and that pointer is used.  If it contains 0,
    acquiring the mutex will ensure that the second check sees a non-stale
    value in all cases.

    On some compilers, the create methods would not be inlined if they
    contained any static variables, so m_pInstance is made a class member
    instead (and the create methods are inlined).  But on MSC, the definition
    of the class member m_pInstance would cause compilation to fail with an
    internal compiler error.  Since MSC is able to inline methods containing
    static variables, m_pInstance is moved into the methods there.  Note that
    this only works well because for any instantiation of rtl_Instance at most
    one call to a create method should be present, anyway.
 */
template< typename Inst, typename InstCtor,
          typename Guard, typename GuardCtor,
          typename Data = int, typename DataCtor = int >
class rtl_Instance
{
public:
    static Inst * create(InstCtor aInstCtor, GuardCtor aGuardCtor)
    {
#if defined _MSC_VER
        static Inst * m_pInstance = 0;
#endif // _MSC_VER
        Inst * p = m_pInstance;
        if (!p)
        {
            Guard aGuard(aGuardCtor());
            p = m_pInstance;
            if (!p)
            {
                p = aInstCtor();
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                m_pInstance = p;
            }
        }
        else
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
        return p;
    }

    static Inst * create(InstCtor aInstCtor, GuardCtor aGuardCtor,
                                DataCtor aDataCtor)
    {
#if defined _MSC_VER
        static Inst * m_pInstance = 0;
#endif // _MSC_VER
        Inst * p = m_pInstance;
        if (!p)
        {
            Data aData(aDataCtor());
            Guard aGuard(aGuardCtor());
            p = m_pInstance;
            if (!p)
            {
                p = aInstCtor(aData);
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                m_pInstance = p;
            }
        }
        else
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
        return p;
    }

    static Inst * create(InstCtor aInstCtor, GuardCtor aGuardCtor,
                                const Data &rData)
    {
#if defined _MSC_VER
        static Inst * m_pInstance = 0;
#endif // _MSC_VER
        Inst * p = m_pInstance;
        if (!p)
        {
            Guard aGuard(aGuardCtor());
            p = m_pInstance;
            if (!p)
            {
                p = aInstCtor(rData);
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                m_pInstance = p;
            }
        }
        else
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
        return p;
    }

private:
#if !defined _MSC_VER
    static Inst * m_pInstance;
#endif // _MSC_VER
};

#if !defined _MSC_VER
template< typename Inst, typename InstCtor,
          typename Guard, typename GuardCtor,
          typename Data, typename DataCtor >
Inst *
rtl_Instance< Inst, InstCtor, Guard, GuardCtor, Data, DataCtor >::m_pInstance
= NULL;
#endif // _MSC_VER

}

namespace rtl {

/** Helper base class for a late-initialized (default-constructed)
    static variable, implementing the double-checked locking pattern correctly.

    @derive
    Derive from this class (common practice), e.g.
    <pre>
    struct MyStatic : public rtl::Static<MyType, MyStatic> {};
    ...
    MyType & rStatic = MyStatic::get();
    ...
    </pre>

    @tparam T
              variable's type
    @tparam Unique
              Implementation trick to make the inner static holder unique,
              using the outer class
              (the one that derives from this base class)
*/
#if defined LIBO_INTERNAL_ONLY
template<typename T, typename Unique>
class Static {
public:
    /** Gets the static.  Mutual exclusion is implied by a functional
        -fthreadsafe-statics

        @return
                static variable
    */
    static T & get() {
        static T instance;
        return instance;
    }
};
#else
template<typename T, typename Unique>
class Static {
public:
    /** Gets the static.  Mutual exclusion is performed using the
        osl global mutex.

        @return
                static variable
    */
    static T & get() {
        return *rtl_Instance<
            T, StaticInstance,
            ::osl::MutexGuard, ::osl::GetGlobalMutex >::create(
                StaticInstance(), ::osl::GetGlobalMutex() );
    }
private:
    struct StaticInstance {
        T * operator () () {
            static T instance;
            return &instance;
        }
    };
};
#endif

/** Helper base class for a late-initialized (default-constructed)
    static variable, implementing the double-checked locking pattern correctly.

    @derive
    Derive from this class (common practice), e.g.
    <pre>
    struct MyStatic : public rtl::Static<MyType, MyStatic> {};
    ...
    MyType & rStatic = MyStatic::get();
    ...
    </pre>

    @tparam T
              variable's type
    @tparam Unique
              Implementation trick to make the inner static holder unique,
              using the outer class
              (the one that derives from this base class)
*/
#if defined LIBO_INTERNAL_ONLY
template<typename T, typename Data, typename Unique>
class StaticWithArg {
public:
    /** Gets the static.  Mutual exclusion is implied by a functional
        -fthreadsafe-statics

        @return
                static variable
    */
    static T & get(const Data& rData) {
        static T instance(rData);
        return instance;
    }

    /** Gets the static.  Mutual exclusion is implied by a functional
        -fthreadsafe-statics

        @return
                static variable
    */
    static T & get(Data& rData) {
        static T instance(rData);
        return instance;
    }
};
#else
template<typename T, typename Data, typename Unique>
class StaticWithArg {
public:
    /** Gets the static.  Mutual exclusion is performed using the
        osl global mutex.

        @return
                static variable
    */
    static T & get(const Data& rData) {
        return *rtl_Instance<
            T, StaticInstanceWithArg,
            ::osl::MutexGuard, ::osl::GetGlobalMutex,
            Data >::create( StaticInstanceWithArg(),
                                      ::osl::GetGlobalMutex(),
                                      rData );
    }

    /** Gets the static.  Mutual exclusion is performed using the
        osl global mutex.

        @return
                static variable
    */
    static T & get(Data& rData) {
        return *rtl_Instance<
            T, StaticInstanceWithArg,
            ::osl::MutexGuard, ::osl::GetGlobalMutex,
            Data >::create( StaticInstanceWithArg(),
                                      ::osl::GetGlobalMutex(),
                                      rData );
    }
private:
    struct StaticInstanceWithArg {
        T * operator () (const Data& rData) {
            static T instance(rData);
            return &instance;
        }

        T * operator () (Data& rData) {
            static T instance(rData);
            return &instance;
         }
    };
};
#endif

/** Helper class for a late-initialized static aggregate, e.g. an array,
    implementing the double-checked locking pattern correctly.

    @tparam T
              aggregate's element type
    @tparam InitAggregate
              initializer functor class
*/
#if defined LIBO_INTERNAL_ONLY
template<typename T, typename InitAggregate>
class StaticAggregate {
public:
    /** Gets the static aggregate, late-initializing.
        Mutual exclusion is implied by a functional
        -fthreadsafe-statics

        @return
                aggregate
    */
    static T * get() {
        static T *instance = InitAggregate()();
        return instance;
    }
};
#else
template<typename T, typename InitAggregate>
class StaticAggregate {
public:
    /** Gets the static aggregate, late-initializing.
        Mutual exclusion is performed using the osl global mutex.

        @return
                aggregate
    */
    static T * get() {
        return rtl_Instance<
            T, InitAggregate,
            ::osl::MutexGuard, ::osl::GetGlobalMutex >::create(
                InitAggregate(), ::osl::GetGlobalMutex() );
    }
};
#endif
/** Helper base class for a late-initialized static variable,
    implementing the double-checked locking pattern correctly.

    @derive
    Derive from this class (common practice),
    providing an initializer functor class, e.g.
    <pre>
    struct MyStatic : public rtl::StaticWithInit<MyType, MyStatic> {
        MyType operator () () {
            ...
            return MyType( ... );
        }
    };
    ...
    MyType & rStatic = MyStatic::get();
    ...
    </pre>

    @tparam T
              variable's type
    @tparam InitData
              initializer functor class
    @tparam Unique
              Implementation trick to make the inner static holder unique,
              using the outer class
              (the one that derives from this base class).
              Default is InitData (common practice).
    @tparam Data
              Initializer functor's return type.
              Default is T (common practice).
*/
#if defined LIBO_INTERNAL_ONLY
template<typename T, typename InitData,
         typename Unique = InitData, typename Data = T>
class StaticWithInit {
public:
    /** Gets the static.  Mutual exclusion is implied by a functional
        -fthreadsafe-statics

        @return
                static variable
    */
    static T & get() {
        static T instance = InitData()();
        return instance;
    }
};
#else
template<typename T, typename InitData,
         typename Unique = InitData, typename Data = T>
class StaticWithInit {
public:
    /** Gets the static.  Mutual exclusion is performed using the
        osl global mutex.

        @return
                static variable
    */
    static T & get() {
        return *rtl_Instance<
            T, StaticInstanceWithInit,
            ::osl::MutexGuard, ::osl::GetGlobalMutex,
            Data, InitData >::create( StaticInstanceWithInit(),
                                      ::osl::GetGlobalMutex(),
                                      InitData() );
    }
private:
    struct StaticInstanceWithInit {
        T * operator () ( Data d ) {
            static T instance(d);
            return &instance;
        }
    };
};
#endif
} // namespace rtl

#endif // INCLUDED_RTL_INSTANCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
