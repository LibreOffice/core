/*************************************************************************
 *
 *  $RCSfile: odbcconfig.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2000-10-26 13:11:36 $
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
#define _DBAUI_ODBC_CONFIG_HXX_

#if defined(WIN) || defined(WNT) || defined (UNX)
#define HAVE_ODBC_SUPPORT
#endif
#if ( defined(WIN) || defined(WNT) ) && defined(HAVE_ODBC_SUPPORT)
#define HAVE_ODBC_ADMINISTRATION
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

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
#endif
    OOdbcLibWrapper(const sal_Char* _pLibPath);
    ~OOdbcLibWrapper();

    void*   loadSymbol(const sal_Char* _pFunctionName);

    /// load the lib
    sal_Bool    load();
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
    void*       m_pAllocHandle;
    void*       m_pFreeHandle;
    void*       m_pSetEnvAttr;
    void*       m_pDataSources;

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
class OOdbcManagement : public OOdbcLibWrapper
{
#ifdef HAVE_ODBC_SUPPORT
    // entry points for ODBC administration
    void*       m_pSQLManageDataSource;
    oslModule   m_pOdbcLib;

    OdbcTypesImpl*  m_pImpl;
        // needed because we can't have a member of type SQLHANDLE: this would require us to include the respective
        // ODBC file, which would lead to a lot of conflicts with other includes
#endif

public:
    OOdbcManagement();
    void        manageDataSources(void* _pParentSysWindowHandle);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_ODBC_CONFIG_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/24 12:48:44  fs
 *  initial checkin - wrapping (system) data source related ODBC functionality
 *
 *
 *  Revision 1.0 24.10.00 10:11:51  fs
 ************************************************************************/

