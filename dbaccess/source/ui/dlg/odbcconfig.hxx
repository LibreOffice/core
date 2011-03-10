/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_ODBC_CONFIG_HXX_
#define _DBAUI_ODBC_CONFIG_HXX_

#include "commontypes.hxx"

#if defined(WNT) || defined (UNX) || defined (OS2)
#define HAVE_ODBC_SUPPORT
#endif

#if ( defined(WNT) || defined (OS2) ) && defined(HAVE_ODBC_SUPPORT)
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
