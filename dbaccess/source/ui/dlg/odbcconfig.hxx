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

#ifndef _DBAUI_ODBC_CONFIG_HXX_
#define _DBAUI_ODBC_CONFIG_HXX_

#include "commontypes.hxx"

#if defined(WNT) || (defined (UNX) && !defined(ANDROID) && !defined(IOS))
#define HAVE_ODBC_SUPPORT
#endif

#if defined(WNT) && defined(HAVE_ODBC_SUPPORT)
#define HAVE_ODBC_ADMINISTRATION
#endif

#include <tools/link.hxx>
#include <osl/module.h>

#include <memory>

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OOdbcLibWrapper
//=========================================================================
/** base for helper classes wrapping functionality from an ODBC library
*/
class OOdbcLibWrapper
{
    oslModule           m_pOdbcLib;     // the library handle
    ::rtl::OUString     m_sLibPath;     // the path to the library

public:
#ifdef HAVE_ODBC_SUPPORT
    sal_Bool    isLoaded() const { return NULL != m_pOdbcLib; }
#else
    sal_Bool    isLoaded() const { return sal_False; }
#endif
    ::rtl::OUString getLibraryName() const { return m_sLibPath; }

protected:
#ifndef HAVE_ODBC_SUPPORT
    OOdbcLibWrapper() : m_pOdbcLib(NULL) { }
#else
    OOdbcLibWrapper();
#endif
    ~OOdbcLibWrapper();

    oslGenericFunction  loadSymbol(const sal_Char* _pFunctionName);

    /// load the lib
    sal_Bool    load(const sal_Char* _pLibPath);
    /// unload the lib
    void        unload();
};

//=========================================================================
//= OOdbcEnumeration
//=========================================================================
struct OdbcTypesImpl;
class OOdbcEnumeration : public OOdbcLibWrapper
{
#ifdef HAVE_ODBC_SUPPORT
    // entry points for ODBC administration
    oslGenericFunction  m_pAllocHandle;
    oslGenericFunction  m_pFreeHandle;
    oslGenericFunction  m_pSetEnvAttr;
    oslGenericFunction  m_pDataSources;

#endif
    OdbcTypesImpl*  m_pImpl;
        // needed because we can't have a member of type SQLHANDLE: this would require us to include the respective
        // ODBC file, which would lead to a lot of conflicts with other includes

public:
    OOdbcEnumeration();
    ~OOdbcEnumeration();

    void        getDatasourceNames(StringBag& _rNames);

protected:
    /// ensure that an ODBC environment is allocated
    sal_Bool    allocEnv();
    /// free any allocated ODBC environment
    void        freeEnv();
};

//=========================================================================
//= OOdbcManagement
//=========================================================================
#ifdef HAVE_ODBC_ADMINISTRATION
class ProcessTerminationWait;
class OOdbcManagement
{
    ::std::auto_ptr< ProcessTerminationWait >   m_pProcessWait;
    Link                                        m_aAsyncFinishCallback;

public:
    OOdbcManagement( const Link& _rAsyncFinishCallback );
    ~OOdbcManagement();

    bool    manageDataSources_async();
    bool    isRunning() const;
};
#endif

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_ODBC_CONFIG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
