/*************************************************************************
 *
 *  $RCSfile: odbcconfig.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2000-10-26 13:47:46 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


#ifdef HAVE_ODBC_SUPPORT

#ifdef WIN
#define ODBC_LIBRARY    "ODBC.DLL"
#define ODBC_UI_LIBRARY "ODBCINST.DLL"
#endif
#if defined WNT
#define ODBC_LIBRARY    "ODBC32.DLL"
#define ODBC_UI_LIBRARY "ODBCCP32.DLL"
#endif
#ifdef UNX
#define ODBC_LIBRARY    "libodbc.so"
#define ODBC_UI_LIBRARY "libodbc.so"
#endif

#ifndef __SQLEXT_H
#include <odbc/sqlext.h>
#endif

#else

#define ODBC_LIBRARY    ""
#define ODBC_UI_LIBRARY ""

#endif  // HAVE_ODBC_SUPPORT

//.........................................................................
namespace dbaui
{
//.........................................................................

#ifdef HAVE_ODBC_SUPPORT
typedef SQLRETURN (SQL_API* TSQLManageDataSource) (SQLHWND hwndParent);
typedef SQLRETURN (SQL_API* TSQLAllocHandle) (SQLSMALLINT HandleType, SQLHANDLE InputHandle, SQLHANDLE* OutputHandlePtr);
typedef SQLRETURN (SQL_API* TSQLFreeHandle) (SQLSMALLINT HandleType, SQLHANDLE Handle);
typedef SQLRETURN (SQL_API* TSQLSetEnvAttr) (SQLHENV EnvironmentHandle, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength);
typedef SQLRETURN (SQL_API* TSQLDataSources) (SQLHENV EnvironmentHandle, SQLUSMALLINT   Direction, SQLCHAR* ServerName,
                                SQLSMALLINT BufferLength1, SQLSMALLINT* NameLength1Ptr, SQLCHAR* Description, SQLSMALLINT BufferLength2, SQLSMALLINT* NameLength2Ptr);

#define NSQLManageDataSource(a) (*(TSQLManageDataSource)(m_pSQLManageDataSource))(a)
#define NSQLAllocHandle(a,b,c) (*(TSQLAllocHandle)(m_pAllocHandle))(a,b,c)
#define NSQLFreeHandle(a,b) (*(TSQLFreeHandle)(m_pFreeHandle))(a,b)
#define NSQLSetEnvAttr(a,b,c,d) (*(TSQLSetEnvAttr)(m_pSetEnvAttr))(a,b,c,d)
#define NSQLDataSources(a,b,c,d,e,f,g,h) (*(TSQLDataSources)(m_pDataSources))(a,b,c,d,e,f,g,h)
#endif

//=========================================================================
//= OOdbcLibWrapper
//=========================================================================
//-------------------------------------------------------------------------
OOdbcLibWrapper::OOdbcLibWrapper(const sal_Char* _pLibPath)
    :m_sLibPath(::rtl::OUString::createFromAscii(_pLibPath))
    ,m_pOdbcLib(NULL)
{
}

//-------------------------------------------------------------------------
sal_Bool OOdbcLibWrapper::load()
{
#ifdef HAVE_ODBC_SUPPORT
    // load the module
    m_pOdbcLib = osl_loadModule(m_sLibPath.pData, SAL_LOADMODULE_NOW);
    return (NULL != m_pOdbcLib);
#endif
}

//-------------------------------------------------------------------------
void OOdbcLibWrapper::unload()
{
#ifdef HAVE_ODBC_SUPPORT
    if (isLoaded())
    {
        osl_unloadModule(m_pOdbcLib);
        m_pOdbcLib = NULL;
    }
#endif
}

//-------------------------------------------------------------------------
void* OOdbcLibWrapper::loadSymbol(const sal_Char* _pFunctionName)
{
    return osl_getSymbol(m_pOdbcLib, ::rtl::OUString::createFromAscii(_pFunctionName).pData);
}

//-------------------------------------------------------------------------
OOdbcLibWrapper::~OOdbcLibWrapper()
{
    unload();
}

//=========================================================================
//= OOdbcEnumeration
//=========================================================================
struct OdbcTypesImpl
{
#ifdef HAVE_ODBC_SUPPORT
    SQLHANDLE   hEnvironment;
    OdbcTypesImpl() : hEnvironment(NULL) { }
#else
    void*       pDummy;
#endif
};
//-------------------------------------------------------------------------
OOdbcEnumeration::OOdbcEnumeration()
#ifdef HAVE_ODBC_SUPPORT
    :OOdbcLibWrapper(ODBC_LIBRARY)
    ,m_pAllocHandle(NULL)
    ,m_pSetEnvAttr(NULL)
    ,m_pDataSources(NULL)
    ,m_pImpl(new OdbcTypesImpl)
#endif
{
    if (load())
    {
#ifdef HAVE_ODBC_SUPPORT
        // load the generic functions
        m_pAllocHandle = loadSymbol("SQLAllocHandle");
        m_pFreeHandle = loadSymbol("SQLFreeHandle");
        m_pSetEnvAttr = loadSymbol("SQLSetEnvAttr");
        m_pDataSources = loadSymbol("SQLDataSources");

        // all or nothing
        if (!m_pAllocHandle || !m_pSetEnvAttr || !m_pDataSources || !m_pFreeHandle)
        {
            unload();
            m_pAllocHandle = m_pFreeHandle = m_pSetEnvAttr = m_pDataSources = NULL;
        }
#endif
    }
}

//-------------------------------------------------------------------------
OOdbcEnumeration::~OOdbcEnumeration()
{
    freeEnv();
    delete m_pImpl;
}

//-------------------------------------------------------------------------
sal_Bool OOdbcEnumeration::allocEnv()
{
    OSL_ENSHURE(isLoaded(), "OOdbcEnumeration::allocEnv: not loaded!");
    if (!isLoaded())
        return sal_False;

#ifdef HAVE_ODBC_SUPPORT
    if (m_pImpl->hEnvironment)
        // nothing to do
        return sal_True;
    SQLRETURN nResult = NSQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_pImpl->hEnvironment);
    if (SQL_SUCCESS != nResult)
        // can't do anything without environment
        return sal_False;

    NSQLSetEnvAttr(m_pImpl->hEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
    return sal_True;
#else
    return sal_False;
#endif
}

//-------------------------------------------------------------------------
void OOdbcEnumeration::freeEnv()
{
#ifdef HAVE_ODBC_SUPPORT
    if (m_pImpl->hEnvironment)
        NSQLFreeHandle(SQL_HANDLE_ENV, m_pImpl->hEnvironment);
    m_pImpl->hEnvironment  =NULL;
#endif
}

//-------------------------------------------------------------------------
void OOdbcEnumeration::getDatasourceNames(StringBag& _rNames)
{
    OSL_ENSHURE(isLoaded(), "OOdbcManagement::getDatasourceNames: not loaded!");
    if (!isLoaded())
        return;

    if (!allocEnv())
    {
        OSL_ENSHURE(sal_False, "OOdbcManagement::getDatasourceNames: could not allocate an ODBC environment!");
        return;
    }

#ifdef HAVE_ODBC_SUPPORT
    // now that we have an environment collect the data source names
    UCHAR szDSN[SQL_MAX_DSN_LENGTH+1];
    SWORD pcbDSN;
    UCHAR szDescription[1024+1];
    SWORD pcbDescription;
    SQLRETURN nResult = SQL_SUCCESS;

    for (   nResult = NSQLDataSources(m_pImpl->hEnvironment, SQL_FETCH_FIRST, szDSN, sizeof(szDSN), &pcbDSN, szDescription, sizeof(szDescription), &pcbDescription);
            ;
            nResult = NSQLDataSources(m_pImpl->hEnvironment, SQL_FETCH_NEXT, szDSN, sizeof(szDSN), &pcbDSN, szDescription, sizeof(szDescription), &pcbDescription)
        )
    {
        if (nResult != SQL_SUCCESS)
            // no further error handling
            break;
        else
        {
            ::rtl::OUStringBuffer aCurrentDsn; aCurrentDsn.appendAscii(reinterpret_cast<const char*>(szDSN));
            // append the description (as comment)
            if (*szDescription)
            {
                aCurrentDsn.appendAscii(" (");
                aCurrentDsn.appendAscii(reinterpret_cast<const char*>(szDescription));
                aCurrentDsn.appendAscii(")");
            }
            _rNames.insert(aCurrentDsn.makeStringAndClear());
        }
    }
#endif
}

//=========================================================================
//= OOdbcManagement
//=========================================================================
//-------------------------------------------------------------------------
OOdbcManagement::OOdbcManagement()
    :OOdbcLibWrapper(ODBC_UI_LIBRARY)
#ifdef HAVE_ODBC_SUPPORT
    ,m_pSQLManageDataSource(NULL)
#endif
{
    if (load())
    {
#ifdef HAVE_ODBC_SUPPORT
        m_pSQLManageDataSource = loadSymbol("SQLManageDataSources");
        if (!m_pSQLManageDataSource)
            unload();
#endif // HAVE_ODBC_SUPPORT
    }
}

//-------------------------------------------------------------------------
void OOdbcManagement::manageDataSources(void* _pParentSysWindowHandle)
{
    OSL_ENSHURE(isLoaded(), "OOdbcManagement::manageDataSources: not loaded!");
    OSL_ENSHURE(_pParentSysWindowHandle, "OOdbcManagement::manageDataSources: invalid parent window!");
    if (!isLoaded())
        return;

#ifdef HAVE_ODBC_SUPPORT
    NSQLManageDataSource(_pParentSysWindowHandle);
#endif // HAVE_ODBC_SUPPORT
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/26 13:11:36  obo
 *  #65293# cant compile for linux
 *
 *  Revision 1.1  2000/10/24 12:48:36  fs
 *  initial checkin - wrapping (system) data source related ODBC functionality
 *
 *
 *  Revision 1.0 24.10.00 10:14:13  fs
 ************************************************************************/

