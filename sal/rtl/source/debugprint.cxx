/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debugprint.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 16:20:25 $
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

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

const sal_Char *dbg_dump(const rtl::OString &rStr)
{
    static rtl::OStringBuffer aStr;

    aStr = rtl::OStringBuffer(rStr);
    aStr.append(static_cast<char>(0));
    return aStr.getStr();
}

const sal_Char *dbg_dump(const rtl::OUString &rStr)
{
    return dbg_dump(rtl::OUStringToOString(rStr, RTL_TEXTENCODING_UTF8));
}

const sal_Char *dbg_dump(rtl_String *pStr)
{
    return dbg_dump(rtl::OString(pStr));
}

const sal_Char *dbg_dump(rtl_uString *pStr)
{
    return dbg_dump(rtl::OUString(pStr));
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
