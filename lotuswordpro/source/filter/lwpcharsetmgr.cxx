/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/
#include "lwpcharsetmgr.hxx"
//Added for SS migration
#include <rtl/textenc.h>

LwpCharSetMgr* LwpCharSetMgr::Instance = nullptr;

LwpCharSetMgr* LwpCharSetMgr::GetInstance()
{
   if (Instance == nullptr)
       Instance = new LwpCharSetMgr;
   return Instance;
}

void LwpCharSetMgr::SetCodePageMap()
{//sal_uInt16 wordproCode,rtl_TextEncoding encoding, tmp hardcoding, 1-18
/*  m_CodePageMap[256] =
    m_CodePageMap[259] =
    m_CodePageMap[819] =
    m_CodePageMap[921] =*/
    m_CodePageMap[437]  = RTL_TEXTENCODING_ASCII_US;
    m_CodePageMap[850]  = RTL_TEXTENCODING_IBM_850;
    //m_CodePageMap[851]  = RTL_TEXTENCODING_IBM_851;
    m_CodePageMap[852]  = RTL_TEXTENCODING_IBM_852;
    m_CodePageMap[857]  = RTL_TEXTENCODING_IBM_857;
    m_CodePageMap[860]  = RTL_TEXTENCODING_IBM_860;
    m_CodePageMap[863]  = RTL_TEXTENCODING_IBM_863;
    m_CodePageMap[865]  = RTL_TEXTENCODING_IBM_865;
    m_CodePageMap[866]  = RTL_TEXTENCODING_IBM_866;
    m_CodePageMap[869]  = RTL_TEXTENCODING_IBM_869;
    m_CodePageMap[874]  = RTL_TEXTENCODING_MS_874;//thai
    m_CodePageMap[932]  = RTL_TEXTENCODING_MS_932; //japanese
    m_CodePageMap[936]  = RTL_TEXTENCODING_MS_936; //chinese simple
    m_CodePageMap[949]  = RTL_TEXTENCODING_MS_949;//korean
    m_CodePageMap[950]  = RTL_TEXTENCODING_BIG5;//chinese traditional
    m_CodePageMap[1250] = RTL_TEXTENCODING_MS_1250;//Central European
    m_CodePageMap[1252] = RTL_TEXTENCODING_MS_1252;
    m_CodePageMap[1251] = RTL_TEXTENCODING_MS_1251; //ukrainian belarusian macedonian russian
    m_CodePageMap[1253] = RTL_TEXTENCODING_MS_1253;//greek
    m_CodePageMap[1254] = RTL_TEXTENCODING_MS_1254;//Turkish
    m_CodePageMap[1255] = RTL_TEXTENCODING_MS_1255;//hebrew
    m_CodePageMap[1256] = RTL_TEXTENCODING_MS_1256;//arabic
    m_CodePageMap[1257] = RTL_TEXTENCODING_MS_1257;//estonian
}

rtl_TextEncoding LwpCharSetMgr::GetTextCharEncoding(sal_uInt16 wordproCode)
{
    std::map<sal_uInt16,rtl_TextEncoding>::iterator
                                             pos = m_CodePageMap.find(wordproCode);
    if (pos != m_CodePageMap.end())
        return m_CodePageMap[wordproCode];
    return GetTextCharEncoding();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
