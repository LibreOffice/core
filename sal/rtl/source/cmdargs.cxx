/*************************************************************************
 *
 *  $RCSfile: cmdargs.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:14:59 $
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

#include <osl/mutex.hxx>
#include <rtl/process.h>
#include <rtl/ustring.hxx>

#include "macro.hxx"

rtl_uString ** g_ppCommandArgs = 0;
sal_uInt32     g_nCommandArgCount = 0;

struct rtl_CmdArgs_ArgHolder
{
    ~rtl_CmdArgs_ArgHolder();
};

rtl_CmdArgs_ArgHolder::~rtl_CmdArgs_ArgHolder()
{
    while (g_nCommandArgCount > 0)
        rtl_uString_release (g_ppCommandArgs[--g_nCommandArgCount]);

    rtl_freeMemory (g_ppCommandArgs);
    g_ppCommandArgs = 0;
}

static void impl_rtl_initCommandArgs()
{
    osl::MutexGuard guard( osl::Mutex::getGlobalMutex() );
    static rtl_CmdArgs_ArgHolder MyHolder;
    if (!g_ppCommandArgs)
    {
        sal_Int32 i, n = osl_getCommandArgCount();

        g_ppCommandArgs =
            (rtl_uString**)rtl_allocateZeroMemory (n * sizeof(rtl_uString*));
        for (i = 0; i < n; i++)
        {
            rtl_uString * pArg = 0;
            osl_getCommandArg (i, &pArg);
            if (('-' == pArg->buffer[0] || '/' == pArg->buffer[0]) &&
                 'e' == pArg->buffer[1] &&
                 'n' == pArg->buffer[2] &&
                 'v' == pArg->buffer[3] &&
                 ':' == pArg->buffer[4] &&
                rtl_ustr_indexOfChar (&(pArg->buffer[5]), '=') >= 0 )
            {
                // ignore.
                rtl_uString_release (pArg);
            }
            else
            {
                // assign.
                g_ppCommandArgs[g_nCommandArgCount++] = pArg;
            }
        }
    }
}

oslProcessError SAL_CALL rtl_getAppCommandArg (
    sal_uInt32 nArg, rtl_uString **ppCommandArg)
{
    if (!g_ppCommandArgs)
        impl_rtl_initCommandArgs();

    oslProcessError result = osl_Process_E_NotFound;
    if( nArg < g_nCommandArgCount )
    {
        rtl::OUString expandedArg (expandMacros(NULL, g_ppCommandArgs[nArg]));

        rtl_uString_assign( ppCommandArg, expandedArg.pData );
        result = osl_Process_E_None;
    }
    return (result);
}

sal_uInt32 SAL_CALL rtl_getAppCommandArgCount (void)
{
    if (!g_ppCommandArgs)
        impl_rtl_initCommandArgs();
    return g_nCommandArgCount;
}
