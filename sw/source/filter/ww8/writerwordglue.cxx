/*************************************************************************
 *
 *  $RCSfile: writerwordglue.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:15:32 $
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
 *  Contributor(s): cmc@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML

#ifndef SW_MS_MSFILTER_HXX
#include <msfilter.hxx>
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif

#include <algorithm>        //std::find_if
#include <functional>       //std::unary_function

#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx> //lA0Width...
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>   //rtl_getBestWindowsCharsetFromTextEncoding
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // ASSERT()
#endif

namespace sw
{
    namespace types
    {

        class closeenough : public std::unary_function<long, bool>
        {
        private:
            long mnValue;
            long mnWriggleRoom;
        public:
            closeenough(long nValue, long nWriggleRoom)
                : mnValue(nValue), mnWriggleRoom(nWriggleRoom) {}
            bool operator() (long nTest) const
            {
                return (
                        (mnValue - nTest < mnWriggleRoom) &&
                        (mnValue - nTest > -mnWriggleRoom)
                       );
            }
        };

        long SnapPageDimension(long nSize) throw()
        {
            static const long aSizes[] =
            {
                lA0Width, lA0Height, lA1Width, lA2Width, lA3Width, lA4Width,
                lA5Width, lB4Width, lB4Height, lB5Width, lB6Width, lC4Width,
                lC4Height, lC5Width, lC6Width, lC65Width, lC65Height, lDLWidth,
                lDLHeight, lJISB4Width, lJISB4Height, lJISB5Width, lJISB6Width,
                lLetterWidth, lLetterHeight, lLegalHeight, lTabloidWidth,
                lTabloidHeight, lDiaWidth, lDiaHeight, lScreenWidth,
                lScreenHeight, lAWidth, lAHeight, lBHeight, lCHeight, lDHeight,
                lEHeight, lExeWidth, lExeHeight, lLegal2Width, lLegal2Height,
                lCom675Width, lCom675Height, lCom9Width, lCom9Height,
                lCom10Width, lCom10Height, lCom11Width, lCom11Height,
                lCom12Width, lMonarchHeight, lKai16Width, lKai16Height,
                lKai32Width, lKai32BigWidth, lKai32BigHeight
            };

            const long nWriggleRoom = 5;
            const long *pEnd = aSizes + sizeof(aSizes) / sizeof(aSizes[0]);
            const long *pEntry =
                std::find_if(aSizes, pEnd, closeenough(nSize, nWriggleRoom));

            if (pEntry != pEnd)
                nSize = *pEntry;

            return nSize;
        }

        sal_uInt8 rtl_TextEncodingToWinCharset(rtl_TextEncoding eTextEncoding)
        {
            sal_uInt8 nRet =
                rtl_getBestWindowsCharsetFromTextEncoding(eTextEncoding);
            if (eTextEncoding == RTL_TEXTENCODING_UCS2)
            {
                ASSERT(nRet != 0x80, "This method may be redundant");
                nRet = 0x80;
            }
            else if (eTextEncoding == RTL_TEXTENCODING_DONTKNOW)
            {
                ASSERT(nRet != 0x80, "This method may be redundant");
                nRet = 0x80;
            }
            return nRet;
        }

        rtl_TextEncoding rtl_TextEncodingToWinCharsetAndBack(
            rtl_TextEncoding eTextEncoding)
        {
            return rtl_getTextEncodingFromWindowsCharset(
                    rtl_TextEncodingToWinCharset(eTextEncoding));
        }
    }

}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
