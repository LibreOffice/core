/*************************************************************************
 *
 *  $RCSfile: instance.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:55:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if !defined INCLUDED_RTL_INSTANCE_HXX
#define INCLUDED_RTL_INSTANCE_HXX

#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include "osl/doublecheckedlocking.h"
#endif

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

      #include "rtl/instance.hxx"
      #include "osl/getglobalmutex.hxx"

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

      #include "rtl/instance.hxx"
      #include "osl/getglobalmutex.hxx"

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

      #include "rtl/instance.hxx"

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

      #include "rtl/instance.hxx"
      #include "osl/getglobalmutex.hxx"

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
                               ::osl::Mutex, ::osl::GetGlobalMutex,
                               Data, InitData >::create(
              InitInstance(), ::osl::GetGlobalMutex(), InitData());
      }

    Some comments:

    For any instantiation of rtl_Instance, at most one call to a create method
    may occur in the program code:  Each occurance of a create method within
    the program code is supposed to return a fresh object instance on the
    first call, and that same object instance on subsequent calls; but
    independent occurances of create methods are supposed to return
    independent object instances.  Since there is a one-to-one correspondence
    between object instances and instantiations of rtl_Instance, the
    requirement should be clear.  One measure to enforce the requirement is
    that rtl_Instance lives in an unnamed namespace, so that instantiations of
    rtl_Instance in different translation units will definitely be different
    instantiations.  A drawback of that measure is that the name of the class
    needs a funny "hand coded" prefix "rtl_" instead of a proper namespace
    prefix like "::rtl::".

    A known problem with this template is when two occurences of calls to
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
    static inline Inst * create(InstCtor aInstCtor, GuardCtor aGuardCtor)
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
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        return p;
    }

    static inline Inst * create(InstCtor aInstCtor, GuardCtor aGuardCtor,
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
                p = aInstCtor(data);
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                m_pInstance = p;
            }
        }
        else
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
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
= 0;
#endif // _MSC_VER

}

#endif // INCLUDED_RTL_INSTANCE_HXX
