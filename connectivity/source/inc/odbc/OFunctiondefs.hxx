/*************************************************************************
 *
 *  $RCSfile: OFunctiondefs.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:27 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//--------------------------------------------------------------------------
#ifndef _CONNECTIVITY_OFUNCTIONDEFS_HXX_
#define _CONNECTIVITY_OFUNCTIONDEFS_HXX_

#if defined(WIN) || defined(WNT)

//  #include "odbc3imp.hxx"
#include <tools/prewin.h>
#include <odbc/sqlext.h>
#include <tools/postwin.h>




#ifndef SQL_C_BOOKMARK
#define SQL_C_BOOKMARK   SQL_C_ULONG                     /* BOOKMARK         */
#endif

#ifndef SQL_OPT_TRACE_OFF
#define SQL_OPT_TRACE_OFF               0UL
#endif

#define SDB_ODBC_CHAR UCHAR

#endif

//--------------------------------------------------------------------------

#ifdef MAC

#include <mac_start.h>

#define sal_Bool    SQLBOOL
#define PFUNC   SQLPFUNC

#ifdef __powerc

#ifndef _ODBCCFM_H
#include <ODBCCfm.h>
#endif

#else

#ifndef __ODBCSHAREDLIBRARY__
  #include <odbc/ODBCSharedLibrary.h>
#endif

#endif

#ifndef __SQLEXT
  #include <odbc/sqlext.h>
#endif

#include <mac_end.h>

// Schnell wieder weg mit dem ganzen Zeug

#undef sal_Bool
#undef PFUNC

#ifdef __powerc
    // Herkoemmliche Version (DLL-Bindung automatisch beim Start des Programms).
    // odbcdefs.hxx biegt die in den Quellen benutzten NSQL-Methoden auf die
    // herkoemmlichen SQL...-Aufrufe um.
    //  #include "odbc3defs.hxx"
#else
    // Stub-Version: dynamische Bindung an die DLL zur Laufzeit.
    // odbcstub definiert die in den Quellen benutzten NSQL...-Methoden
    // als indirekte Funktionsaufrufe.
    // odbcimp zieht sich selbst preos2, odbc und postos2 an.
    //  #include "odbc3imp.hxx"
#endif

#define SDB_ODBC_CHAR UCHAR

#endif

//--------------------------------------------------------------------------

#ifdef OS2

#ifdef ODBCIMP

// Stub-Version: dynamische Bindung an die DLL zur Laufzeit.
// odbcstub definiert die in den Quellen benutzten NSQL...-Methoden
// als indirekte Funktionsaufrufe.
// odbcimp zieht sich selbst preos2, odbc und postos2 an.
//  #include "odbc3imp.hxx"

#else

// Zur Zeit verwenden wir die ODBC-DLL von Watcom-SQL direkt (ueber die
// mitgelieferte Lib).

#ifndef ODBC_OS2
#define ODBC_OS2
#endif

#include <svpm.h>
#include <odbc.h>
#define SQL_API __syscall
#ifndef SQL_MAX_MESSAGE_LENGTH
#define SQL_MAX_MESSAGE_LENGTH MAX_MESSAGE_LENGTH
#endif
#ifndef SQL_MAX_DSN_LENGTH
#define SQL_MAX_DSN_LENGTH MAX_DSN_LENGTH
#endif
#ifndef SQL_AUTOCOMMIT_ON
#define SQL_AUTOCOMMIT_ON 1UL
#endif
#ifndef SQL_AUTOCOMMIT_OFF
#define SQL_AUTOCOMMIT_OFF 0UL
#endif

#define SQL_FETCH_PRIOR SQL_FETCH_PREV
#define SQL_NO_TOTAL (-4)

//  #include "odbc3defs.hxx"

#endif

// In der ODBC.H von Watcom werden Strings als char * erwartet
// (nicht, wie sonst bei ODBC ueblich, als UCHAR *).
#if defined( ICC ) || defined( WTC )
#define SDB_ODBC_CHAR unsigned char
#else
#define SDB_ODBC_CHAR char
#endif

#endif

//--------------------------------------------------------------------------

#ifdef UNX

#ifdef ODBCIMP
// Stub-Version: dynamische Bindung an die shared library zur Laufzeit.
// odbcstub definiert die in den Quellen benutzten NSQL...-Methoden
// als indirekte Funktionsaufrufe.
// odbcimp zieht sich selbst preos2, odbc und postos2 an.
//  #include "odbc3imp.hxx"

#else

// Zur Zeit verwenden wir die ODBC-shared library von Q+E direkt (ueber die
// mitgelieferte Lib).

#ifndef ODBC_UNX
#define ODBC_UNX
#endif
#include <odbc/qeodbc.h>
#define CALLBACK
#define EXPORT
#include <odbc/sqlext.h>
#undef sal_Bool // Ist in qeodbc.h definiert, wird aber von solar.h noch einmal
            // definiert.

//  #include "odbc3defs.hxx"

#endif // ODBCIMP

#define SDB_ODBC_CHAR UCHAR
#define SQL_WCHAR           (-8)
#define SQL_WVARCHAR        (-9)
#define SQL_WLONGVARCHAR    (-10)
#define SQL_C_WCHAR         SQL_WCHAR


#endif // UNX
#endif // _CONNECTIVITY_OFUNCTIONDEFS_HXX_

