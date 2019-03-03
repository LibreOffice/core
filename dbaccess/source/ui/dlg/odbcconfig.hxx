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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_ODBCCONFIG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_ODBCCONFIG_HXX

#include <commontypes.hxx>

#if defined(_WIN32) || (defined (UNX) && !defined(ANDROID) && !defined(IOS))
#define HAVE_ODBC_SUPPORT
#endif

#if defined(_WIN32) && defined(HAVE_ODBC_SUPPORT)
#define HAVE_ODBC_ADMINISTRATION
#endif

#include <tools/link.hxx>
#include <osl/module.h>

#include <memory>
#include <set>

namespace dbaui
{

// OOdbcEnumeration
struct OdbcTypesImpl;
class OOdbcEnumeration final
{
    oslModule        m_pOdbcLib;     // the library handle
    OUString         m_sLibPath;     // the path to the library

#ifdef HAVE_ODBC_SUPPORT
    // entry points for ODBC administration
    oslGenericFunction  m_pAllocHandle;
    oslGenericFunction  m_pFreeHandle;
    oslGenericFunction  m_pSetEnvAttr;
    oslGenericFunction  m_pDataSources;

#endif
    std::unique_ptr<OdbcTypesImpl>  m_pImpl;
        // needed because we can't have a member of type SQLHANDLE: this would require us to include the respective
        // ODBC file, which would lead to a lot of conflicts with other includes

public:
    OOdbcEnumeration();
    ~OOdbcEnumeration();

#ifdef HAVE_ODBC_SUPPORT
    bool        isLoaded() const { return nullptr != m_pOdbcLib; }
#else
    bool        isLoaded() const { return false; }
#endif
    const OUString& getLibraryName() const { return m_sLibPath; }

    void        getDatasourceNames(std::set<OUString>& _rNames);

private:
    oslGenericFunction  loadSymbol(const sal_Char* _pFunctionName);

    /// load the lib
    bool        load(const sal_Char* _pLibPath);
    /// unload the lib
    void        unload();
    /// ensure that an ODBC environment is allocated
    bool        allocEnv();
    /// free any allocated ODBC environment
    void        freeEnv();
};

// OOdbcManagement
#ifdef HAVE_ODBC_ADMINISTRATION
class ProcessTerminationWait;
class OOdbcManagement
{
    std::unique_ptr< ProcessTerminationWait >   m_pProcessWait;
    Link<void*,void>                            m_aAsyncFinishCallback;

public:
    explicit OOdbcManagement( const Link<void*,void>& _rAsyncFinishCallback );
    ~OOdbcManagement();

    bool    manageDataSources_async();
    bool    isRunning() const;
    void    disableCallback();
    void    receivedCallback();
};
#endif

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ODBCCONFIG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
