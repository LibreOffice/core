/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: t_print.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:21:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppunit.hxx"

#include <stdio.h>
#include <cppunit/cmdlinebits.hxx>
#include <rtl/tres.h>
#include <osl/mutex.hxx>

extern CmdLineBits st_nBits;

sal_Bool isBit( CmdLineBits _nBits, CmdLineBits _nFlag )
{
    return( ( _nBits & _nFlag ) == _nFlag );
}

// -----------------------------------------------------------------------------
// ----------------------------- own printf method -----------------------------
// -----------------------------------------------------------------------------

bool isVerbose()
{
    if ( isBit( st_nBits, rtl_tres_Flag_VERBOSE ) )
    {
        return true;
    }
    return false;
}

bool isQuiet()
{
    if ( isBit( st_nBits, rtl_tres_Flag_QUIET ) )
    {
        return true;
    }
    return false;
}

// ::osl::Mutex   m_aTPrintMutex;

void impl_t_print(const char* _pFormatStr, va_list &args)
{
    if (! isQuiet())
    {
        // ::osl::MutexGuard g(m_aTPrintMutex);
        printf("# ");
        vprintf(_pFormatStr, args);
    }
}

void t_print(const char* _pFormatStr, ...)
{
    va_list  args;
    va_start( args, _pFormatStr );
    impl_t_print(_pFormatStr, args);
    va_end( args );
}

void t_print(T_Print_Params _eType, const char* _pFormatStr, ...)
{
    if (_eType == T_VERBOSE && isVerbose())
    {
        // if T_VERBOSE and we should be verbose, print info
        va_list  args;
        va_start( args, _pFormatStr );
        impl_t_print(_pFormatStr, args);
        va_end( args );
    }
}
