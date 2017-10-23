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

#ifndef INCLUDED_CPPUHELPER_EXC_HLP_HXX
#define INCLUDED_CPPUHELPER_EXC_HLP_HXX

#include "com/sun/star/uno/Any.hxx"
#include "cppuhelper/cppuhelperdllapi.h"

namespace cppu
{

/** This function throws the exception given by rExc.  The given value has to
    be of typeclass EXCEPTION and must be derived from or of
    type com.sun.star.uno.Exception.

    @param rExc
           exception to be thrown.
*/
CPPUHELPER_DLLPUBLIC void SAL_CALL throwException( const css::uno::Any & rExc );

/** Use this function to get the dynamic type of a caught C++-UNO exception;
    completes the above function throwing exceptions generically.

    @code
    try
    {
        ...
    }
    catch (css::uno::RuntimeException &)
    {
        // you ought not handle RuntimeExceptions:
        throw;
    }
    catch (css::uno::Exception &)
    {
        css::uno::Any caught( ::cppu::getCaughtException() );
        ...
    }
    @endcode

    Restrictions:
    - use only for caught C++-UNO exceptions (UNOIDL defined)
    - only as first statement in a catch block!
    - don't do a C++ rethrow (throw;) after you have called this function
    - call getCaughtException() just once in your catch block!
      (function internally uses a C++ rethrow)

    @return
              caught UNO exception

    @attention Caution!
              This function is limited to the same C++ compiler runtime library.
              E.g. for MSVC, this means that the catch handler code (the one
              that calls getCaughtException()) needs to use the very same
              C++ runtime library, e.g. msvcrt.dll as cppuhelper, e.g.
              cppuhelper3MSC.dll and the bridge library, e.g. msci_uno.dll.
              This is the case if all of them are compiled with the same
              compiler version.
              Background: The msci_uno.dll gets a rethrown exception out
              of the internal msvcrt.dll thread local storage (tls).
              Thus you _must_ not use this function if your code needs to run
              in newer UDK versions without being recompiled, because those
              newer UDK (-> OOo versions) potentially use newer C++ runtime
              libraries which most often become incompatible!

              But this function ought to be usable for most OOo internal C++-UNO
              development, because the whole OOo code base is compiled using the
              same C++ compiler (and linking against one runtime library).
*/
CPPUHELPER_DLLPUBLIC css::uno::Any SAL_CALL getCaughtException();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
