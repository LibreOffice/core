/*************************************************************************
 *
 *  $RCSfile: logfile.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-07-06 09:36:09 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _RTL_LOGFILE_HXX_
#define _RTL_LOGFILE_HXX_

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _RTL_LOGFILE_H_
#include <rtl/logfile.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

namespace rtl
{
    /** wrapper class to keep a logging context.

        Should not be used directly, instead use the RTL_LOGFILE_CONTEXT -macros.
    */
    class Logfile
    {
    public:
        inline Logfile( const sal_Char *name );
        inline ~Logfile();
        inline const sal_Char *getName();
    private:
        ::rtl::OString m_sName;
    };

    inline Logfile::Logfile( const sal_Char *name )
        : m_sName( name )
    {
        rtl_logfile_trace( "%06lu %lu { %s\n",
                           osl_getGlobalTimer(),
                           osl_getThreadIdentifier( 0 ),
                           name );
    }

    inline Logfile::~Logfile()
    {
        rtl_logfile_trace( "%06lu %lu } %s\n",
                           osl_getGlobalTimer(),
                           osl_getThreadIdentifier(0),
                           m_sName.pData->buffer );
    }

    inline const sal_Char * Logfile::getName()
    {
        return m_sName.getStr();
    }
}

#ifdef PROFILE
#define RTL_LOGFILE_CONTEXT( instance, name ) ::rtl::Logfile instance( name )
#define RTL_LOGFILE_CONTEXT_TRACE( instance, message ) \
        rtl_logfile_trace( "%06lu %lu | %s : %s\n", \
                           osl_getGlobalTimer(),  \
                           osl_getThreadIdentifier( 0 ), \
                           instance.getName(), \
                           message )
#define RTL_LOGFILE_CONTEXT_TRACE1( instance , frmt, arg1 ) \
        rtl_logfile_trace( "%06lu %lu | %s : ", \
                           osl_getGlobalTimer(),  \
                           osl_getThreadIdentifier( 0 ), \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 ); \
        rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_CONTEXT_TRACE2( instance , frmt, arg1 , arg2 ) \
        rtl_logfile_trace( "%06lu %lu | %s : ", \
                           osl_getGlobalTimer(),  \
                           osl_getThreadIdentifier( 0 ), \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 , arg2 ); \
        rtl_logfile_trace( "\n" )
#define RTL_LOGFILE_CONTEXT_TRACE3( instance , frmt, arg1 , arg2 , arg3 ) \
        rtl_logfile_trace( "%06lu %lu | %s : ", \
                           osl_getGlobalTimer(),  \
                           osl_getThreadIdentifier( 0 ), \
                           instance.getName() ); \
        rtl_logfile_trace( frmt , arg1 , arg2 , arg3 ); \
        rtl_logfile_trace( "\n" )

#else
#define RTL_LOGFILE_CONTEXT( instance,name )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE( instance, message )  ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE1( instance, frmt, arg1 ) ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE2( instance, frmt, arg1, arg2 ) ((void)0)
#define RTL_LOGFILE_CONTEXT_TRACE3( instance, frmt, arg1, arg2 , arg3 ) ((void)0)
#endif

#endif
