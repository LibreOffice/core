/*************************************************************************
 *
 *  $RCSfile: t_file.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:32 $
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

#define _T_FILE_CXX "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif

#ifdef _USE_NAMESPACE
using namespace store;
#endif

/*========================================================================
 *
 * __store_getProcessTextEncoding.
 *
 *======================================================================*/
inline rtl_TextEncoding __store_getProcessTextEncoding (void)
{
    rtl_TextEncoding eEncoding;
#if defined(SAL_OS2)
    eEncoding = RTL_TEXTENCODING_IBM850;
#elif defined(SAL_UNX)
    eEncoding = RTL_TEXTENCODING_ISO8859_1;
#elif defined(SAL_W32)
    eEncoding = RTL_TEXTENCODING_MS_1252;
#else
    eEncoding = RTL_TEXTENCODING_ASCII_US;
#endif
    return eEncoding;
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    if (argc < 2)
        return 0;

    NAMESPACE_VOS(ORef)<OFileLockBytes> xLockBytes (new OFileLockBytes());
    if (!xLockBytes.isValid())
        return 0;

    NAMESPACE_RTL(OUString) aFilename (
        argv[1], rtl_str_getLength(argv[1]),
        __store_getProcessTextEncoding());

    storeError eErrCode = xLockBytes->create (
        aFilename.pData, store_AccessReadWrite);
    if (eErrCode != store_E_None)
    {
        // Check reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        // Create.
        eErrCode = xLockBytes->create (
            aFilename.pData, store_AccessReadCreate);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    sal_uInt32 k = 0;
    eErrCode = xLockBytes->writeAt (
        0, argv[0], rtl_str_getLength(argv[0]), k);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = xLockBytes->setSize (1024);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = xLockBytes->writeAt (
        1024, argv[1], rtl_str_getLength(argv[1]), k);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = xLockBytes->setSize (2048);
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = xLockBytes->flush();
    if (eErrCode != store_E_None)
        return eErrCode;

    xLockBytes.unbind();
    return 0;
}

