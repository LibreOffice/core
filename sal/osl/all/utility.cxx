/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: utility.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 08:45:16 $
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
#include "precompiled_sal.hxx"


#include "rtl/ustring.hxx"
#include "osl/time.h"
#include <stdio.h>


/*
 *   mfe : maybe it would be wishful to include initialization
 *         of the global timer in dllmain or _init directly.
 *         But noneoftheless this (should) work too.
 */
namespace osl
{

class OGlobalTimer
{

public:

    OGlobalTimer() {
        getTime();
    }

    sal_uInt32 getTime()
    {
        return osl_getGlobalTimer();
    }


};

static OGlobalTimer aGlobalTimer;

}


extern "C"
{
void debug_ustring(rtl_uString* ustr)
{
    sal_Char* psz=0;
    rtl_String* str=0;

    if ( ustr != 0 )
    {
        rtl_uString2String( &str,
                            rtl_uString_getStr(ustr),
                            rtl_uString_getLength(ustr),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        psz = rtl_string_getStr(str);
    }

    fprintf(stderr,"'%s'\n",psz);

    if ( str != 0 )
    {
        rtl_string_release(str);
    }

    return;
}

}

void debug_oustring(rtl::OUString& ustr)
{

    debug_ustring(ustr.pData);

    return;
}
