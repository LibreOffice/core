/*************************************************************************
 *
 *  $RCSfile: cmdargs.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2001-08-30 11:51:36 $
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

using namespace ::rtl;

OUString *g_pCommandArgs = 0;
sal_Int32 g_nCommandArgCount = -1;

struct rtl_CmdArgs_ArgHolder
{
    ~rtl_CmdArgs_ArgHolder();
};

rtl_CmdArgs_ArgHolder::~rtl_CmdArgs_ArgHolder()
{
    delete [] g_pCommandArgs;
}

rtl_CmdArgs_ArgHolder MyHolder;

void impl_rtl_initCommandArgs()
{
    ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
    if( !g_pCommandArgs )
    {
        sal_Int32 nCount = osl_getCommandArgCount();
        ::rtl::OUString * p = new ::rtl::OUString[nCount];
        sal_Int32 i = 0, i2 = 0;
        for( ; i < nCount ; i ++ )
        {
            ::rtl::OUString data;
            osl_getCommandArg( i, &(data.pData) );
            if( ('-' == data.pData->buffer[0] || '/' == data.pData->buffer[0] ) &&
                 'e' == data.pData->buffer[1] &&
                 'n' == data.pData->buffer[2] &&
                 'v' == data.pData->buffer[3] &&
                 ':' == data.pData->buffer[4] &&
                rtl_ustr_indexOfChar( &(data.pData->buffer[5]), '=' ) >= 0 )
            {
                // ignore
            }
            else
            {
                p[i2] = data;
                i2 ++;
            }
        }
        g_nCommandArgCount = i2;
        g_pCommandArgs = p;
    }
}



extern "C"
{
    oslProcessError SAL_CALL rtl_getAppCommandArg(sal_uInt32 nArg, rtl_uString **strCommandArg)
    {
        if( !g_pCommandArgs )
            impl_rtl_initCommandArgs();

        oslProcessError err = osl_Process_E_None;
        if( nArg < g_nCommandArgCount )
        {
            OUString expandedArg = expandMacros(NULL, g_pCommandArgs[nArg]);

            rtl_uString_assign( strCommandArg, expandedArg.pData );
         }
        else
        {
            err = osl_Process_E_NotFound;
        }
        return err;
    }

    sal_uInt32 SAL_CALL rtl_getAppCommandArgCount()
    {
        if( !g_pCommandArgs )
            impl_rtl_initCommandArgs();
        return g_nCommandArgCount;
    }
}
