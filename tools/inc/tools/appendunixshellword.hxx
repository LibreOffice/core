/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appendunixshellword.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:26:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2008 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX
#define INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX

#include "sal/config.h"

#if defined UNX

#include "tools/toolsdllapi.h"

namespace rtl {
    class OString;
    class OStringBuffer;
}

namespace tools {

// append arbitrary bytes as a properly quoted Unix-style shell word
//
// @param accumulator
// the string buffer to which the word is appended (without any surrounding
// whitespace); must not be null
//
// @param text
// the text to add
TOOLS_DLLPUBLIC void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text);

}

#endif

#endif
