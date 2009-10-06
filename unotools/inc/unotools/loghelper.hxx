/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loghelper.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:20:29 $
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
#ifndef _SVT_LOGHELPER_HXX
#define _SVT_LOGHELPER_HXX

namespace css = ::com::sun::star;

class LogHelper
{
public:
    LogHelper();
    ~LogHelper();

    static void logIt(const css::uno::Exception&){}
};

inline void logIt(const css::uno::Exception& ex)
{
    ::rtl::OUStringBuffer sMsg(256);
    sMsg.appendAscii("Unexpected exception catched. Original message was:\n\""      );
    sMsg.append(ex.Message);
    sMsg.appendAscii("\"");
    OSL_ENSURE(sal_False, ::rtl::OUStringToOString(sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr());
}

#endif

