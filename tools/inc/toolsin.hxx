/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolsin.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 22:08:53 $
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

#ifndef _TOOLSIN_HXX
#define _TOOLSIN_HXX

#ifndef _TOOLS_H
#include <tools/tools.h>
#endif

struct ImplRegisterLanguageData;
struct ImplRegisterFormatData;
struct Impl1ByteUnicodeTabData;
struct Impl1ByteConvertTabData;

// --------------
// - TOOLSNDATA -
// --------------

struct TOOLSINDATA
{
    ImplRegisterLanguageData*   mpFirstLangData;
    ImplRegisterFormatData*     mpFirstFormatData;
    Impl1ByteUnicodeTabData*    mpFirstUniTabData;
    Impl1ByteConvertTabData*    mpFirstConvertTabData;
};

TOOLSINDATA* ImplGetToolsInData();

#endif // _TOOLSIN_HXX
