/*************************************************************************
 *
 *  $RCSfile: NeonUri.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kso $ $Date: 2001-02-20 15:06:26 $
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

#include "NeonUri.hxx"
#include "DAVException.hxx"

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

using namespace rtl;
using namespace webdav_ucp;

//============================================================================
namespace webdav_ucp_impl {

inline bool isUSASCII(sal_uInt32 nChar)
{
    return nChar <= 0x7F;
}

inline bool isVisible(sal_uInt32 nChar)
{
    return nChar >= '!' && nChar <= '~';
}

inline bool isDigit(sal_uInt32 nChar)
{
    return nChar >= '0' && nChar <= '9';
}

inline int getHexWeight(sal_uInt32 nChar)
{
    return isDigit(nChar) ? int(nChar - '0') :
           nChar >= 'A' && nChar <= 'F' ? int(nChar - 'A' + 10) :
           nChar >= 'a' && nChar <= 'f' ? int(nChar - 'a' + 10) : -1;
}

inline bool isHighSurrogate(sal_uInt32 nUTF16)
{
    return nUTF16 >= 0xD800 && nUTF16 <= 0xDBFF;
}

inline bool isLowSurrogate(sal_uInt32 nUTF16)
{
    return nUTF16 >= 0xDC00 && nUTF16 <= 0xDFFF;
}

inline sal_uInt32 getUTF32Character(sal_Unicode const *& rBegin,
                                    sal_Unicode const * pEnd)
{
    VOS_ASSERT(rBegin && rBegin < pEnd);
    if (rBegin + 1 < pEnd && rBegin[0] >= 0xD800 && rBegin[0] <= 0xDBFF
        && rBegin[1] >= 0xDC00 && rBegin[1] <= 0xDFFF)
    {
        sal_uInt32 nUTF32 = sal_uInt32(*rBegin++ & 0x3FF) << 10;
        return (nUTF32 | (*rBegin++ & 0x3FF)) + 0x10000;
    }
    else
        return *rBegin++;
}

sal_uInt32 getHexDigit(int nWeight)
{
    VOS_ASSERT(nWeight >= 0 && nWeight < 16);
    static sal_Char const aDigits[16]
        = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
    return aDigits[nWeight];
}

enum EncodeMechanism
{
    ENCODE_ALL,
    WAS_ENCODED,
    NOT_CANONIC
};

enum DecodeMechanism
{
    NO_DECODE,
    DECODE_TO_IURI,
    DECODE_WITH_CHARSET
};

enum Part
{
    PART_OBSOLETE_NORMAL = 0x001, // Obsolete, do not use!
    PART_OBSOLETE_FILE = 0x002, // Obsolete, do not use!
    PART_OBSOLETE_PARAM = 0x004, // Obsolete, do not use!
    PART_USER_PASSWORD = 0x008,
    PART_IMAP_ACHAR = 0x010,
    PART_VIM = 0x020,
    PART_HOST_EXTRA = 0x040,
    PART_FPATH = 0x080,
    PART_AUTHORITY = 0x100,
    PART_PATH_SEGMENTS_EXTRA = 0x200,
    PART_REL_SEGMENT_EXTRA = 0x400,
    PART_URIC = 0x800,
    PART_HTTP_PATH = 0x1000,
    PART_FILE_SEGMENT_EXTRA = 0x2000, // Obsolete, do not use!
    PART_MESSAGE_ID = 0x4000,
    PART_MESSAGE_ID_PATH = 0x8000,
    PART_MAILTO = 0x10000,
    PART_PATH_BEFORE_QUERY = 0x20000,
    PART_PCHAR = 0x40000,
    PART_FRAGMENT = 0x80000, // Obsolete, do not use!
    PART_VISIBLE = 0x100000,
    max_part = 0x80000000
        // Do not use!  Only there to allow compatible changes in the future.
};

enum EscapeType
{
    ESCAPE_NO,
    ESCAPE_OCTET,
    ESCAPE_UTF32
};

inline void appendEscape(rtl::OUStringBuffer & rTheText,
                         sal_Char cEscapePrefix, sal_uInt32 nOctet)
{
    rTheText.append(sal_Unicode(cEscapePrefix));
    rTheText.append(sal_Unicode(getHexDigit(int(nOctet >> 4))));
    rTheText.append(sal_Unicode(getHexDigit(int(nOctet & 15))));
}

inline bool mustEncode(sal_uInt32 nUTF32, Part ePart)
{
    enum
    {
        pA = PART_OBSOLETE_NORMAL,
        pB = PART_OBSOLETE_FILE,
        pC = PART_OBSOLETE_PARAM,
        pD = PART_USER_PASSWORD,
        pE = PART_IMAP_ACHAR,
        pF = PART_VIM,
        pG = PART_HOST_EXTRA,
        pH = PART_FPATH,
        pI = PART_AUTHORITY,
        pJ = PART_PATH_SEGMENTS_EXTRA,
        pK = PART_REL_SEGMENT_EXTRA,
        pL = PART_URIC,
        pM = PART_HTTP_PATH,
        pN = PART_FILE_SEGMENT_EXTRA,
        pO = PART_MESSAGE_ID,
        pP = PART_MESSAGE_ID_PATH,
        pQ = PART_MAILTO,
        pR = PART_PATH_BEFORE_QUERY,
        pS = PART_PCHAR,
        pT = PART_FRAGMENT,
        pU = PART_VISIBLE
    };
    static sal_uInt32 const aMap[128]
        = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*   */ 0,
    /* ! */       pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* " */                                                             pU,
    /* # */                                                             pU,
    /* $ */          pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* % */                                                             pU,
    /* & */ pA+pB+pC+pD+pE      +pH+pI+pJ+pK+pL+pM+pN+pO+pP   +pR+pS+pT+pU,
    /* ' */          pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* ( */          pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* ) */          pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* * */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* + */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* , */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* - */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* . */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* / */ pA+pB+pC            +pH   +pJ   +pL+pM      +pP+pQ+pR   +pT+pU,
    /* 0 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 1 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 2 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 3 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 4 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 5 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 6 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 7 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 8 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* 9 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* : */    pB+pC            +pH+pI+pJ   +pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* ; */       pC+pD            +pI+pJ+pK+pL+pM   +pO+pP+pQ+pR   +pT+pU,
    /* < */       pC                                 +pO+pP            +pU,
    /* = */ pA+pB+pC+pD+pE      +pH+pI+pJ+pK+pL+pM+pN         +pR+pS+pT+pU,
    /* > */       pC                                 +pO+pP            +pU,
    /* ? */       pC                        +pL                     +pT+pU,
    /* @ */       pC            +pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* A */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* B */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* C */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* D */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* E */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* F */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* G */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* H */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* I */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* J */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* K */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* L */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* M */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* N */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* O */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* P */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* Q */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* R */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* S */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* T */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* U */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* V */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* W */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* X */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* Y */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* Z */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* [ */                                  pL                        +pU,
    /* \ */    pB                                                      +pU,
    /* ] */                                  pL                        +pU,
    /* ^ */                                                             pU,
    /* _ */ pA+pB+pC+pD+pE   +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* ` */                                                            +pU,
    /* a */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* b */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* c */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* d */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* e */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* f */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* g */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* h */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* i */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* j */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* k */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* l */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* m */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* n */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* o */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* p */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* q */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* r */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* s */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* t */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* u */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* v */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* w */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* x */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* y */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* z */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /* { */                                                            +pU,
    /* | */    pB+pC                              +pN               +pT+pU,
    /* } */                                                            +pU,
    /* ~ */ pA+pB+pC+pD+pE      +pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU,
    /*   */ 0 };
    return !isUSASCII(nUTF32) || !(aMap[nUTF32] & ePart);
}

void appendUCS4Escape(rtl::OUStringBuffer & rTheText, sal_Char cEscapePrefix,
                      sal_uInt32 nUCS4)
{
    VOS_ASSERT(nUCS4 < 0x80000000);
    if (nUCS4 < 0x80)
        appendEscape(rTheText, cEscapePrefix, nUCS4);
    else if (nUCS4 < 0x800)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 | 0xC0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x10000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 | 0xE0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x200000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 | 0xF0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x4000000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 24 | 0xF8);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 30 | 0xFC);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 24 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
}

void appendUCS4(rtl::OUStringBuffer & rTheText, sal_uInt32 nUCS4,
                EscapeType eEscapeType, bool bOctets, Part ePart,
                sal_Char cEscapePrefix, rtl_TextEncoding eCharset,
                bool bKeepVisibleEscapes)
{
    bool bEscape;
    rtl_TextEncoding eTargetCharset;
    switch (eEscapeType)
    {
        case ESCAPE_NO:
            if (mustEncode(nUCS4, ePart))
            {
                bEscape = true;
                eTargetCharset = bOctets ? RTL_TEXTENCODING_ISO_8859_1 :
                                           RTL_TEXTENCODING_UTF8;
            }
            else
                bEscape = false;
            break;

        case ESCAPE_OCTET:
            bEscape = true;
            eTargetCharset = RTL_TEXTENCODING_ISO_8859_1;
            break;

        case ESCAPE_UTF32:
            if (mustEncode(nUCS4, ePart))
            {
                bEscape = true;
                eTargetCharset = eCharset;
            }
            else if (bKeepVisibleEscapes && isVisible(nUCS4))
            {
                bEscape = true;
                eTargetCharset = RTL_TEXTENCODING_ASCII_US;
            }
            else
                bEscape = false;
            break;
    }
    if (bEscape)
        switch (eTargetCharset)
        {
            default:
                VOS_ASSERT(false);
            case RTL_TEXTENCODING_ASCII_US:
            case RTL_TEXTENCODING_ISO_8859_1:
                appendEscape(rTheText, cEscapePrefix, nUCS4);
                break;

            case RTL_TEXTENCODING_UTF8:
                appendUCS4Escape(rTheText, cEscapePrefix, nUCS4);
                break;
        }
    else
        rTheText.append(sal_Unicode(nUCS4));
}

sal_uInt32 getUTF32(sal_Unicode const *& rBegin, sal_Unicode const * pEnd,
                    bool bOctets, sal_Char cEscapePrefix,
                    EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
                    EscapeType & rEscapeType)
{
    VOS_ASSERT(rBegin < pEnd);
    sal_uInt32 nUTF32 = bOctets ? *rBegin++ : getUTF32Character(rBegin, pEnd);
    switch (eMechanism)
    {
        case ENCODE_ALL:
            rEscapeType = ESCAPE_NO;
            break;

        case WAS_ENCODED:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && (nWeight1 = getHexWeight(rBegin[0])) >= 0
                && (nWeight2 = getHexWeight(rBegin[1])) >= 0)
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                switch (eCharset)
                {
                    default:
                        VOS_ASSERT(false);
                    case RTL_TEXTENCODING_ASCII_US:
                        rEscapeType
                            = isUSASCII(nUTF32) ? ESCAPE_UTF32 : ESCAPE_OCTET;
                        break;

                    case RTL_TEXTENCODING_ISO_8859_1:
                        rEscapeType = ESCAPE_UTF32;
                        break;

                    case RTL_TEXTENCODING_UTF8:
                        if (isUSASCII(nUTF32))
                            rEscapeType = ESCAPE_UTF32;
                        else
                        {
                            if (nUTF32 >= 0xC0 && nUTF32 <= 0xF4)
                            {
                                sal_uInt32 nEncoded;
                                int nShift;
                                sal_uInt32 nMin;
                                if (nUTF32 <= 0xDF)
                                {
                                    nEncoded = (nUTF32 & 0x1F) << 6;
                                    nShift = 0;
                                    nMin = 0x80;
                                }
                                else if (nUTF32 <= 0xEF)
                                {
                                    nEncoded = (nUTF32 & 0x0F) << 12;
                                    nShift = 6;
                                    nMin = 0x800;
                                }
                                else
                                {
                                    nEncoded = (nUTF32 & 0x07) << 18;
                                    nShift = 12;
                                    nMin = 0x10000;
                                }
                                sal_Unicode const * p = rBegin;
                                bool bUTF8 = true;
                                for (;;)
                                {
                                    if (p + 2 >= pEnd || p[0] != cEscapePrefix
                                        || (nWeight1 = getHexWeight(p[1])) < 0
                                        || (nWeight2 = getHexWeight(p[2])) < 0
                                        || nWeight1 < 8)
                                    {
                                        bUTF8 = false;
                                        break;
                                    }
                                    p += 3;
                                    nEncoded
                                        |= ((nWeight1 & 3) << 4 | nWeight2)
                                               << nShift;
                                    if (nShift == 0)
                                        break;
                                    nShift -= 6;
                                }
                                if (bUTF8 && nEncoded >= nMin
                                    && !isHighSurrogate(nEncoded)
                                    && !isLowSurrogate(nEncoded)
                                    && nEncoded <= 0x10FFFF)
                                {
                                    rBegin = p;
                                    nUTF32 = nEncoded;
                                    rEscapeType = ESCAPE_UTF32;
                                    break;
                                }
                            }
                            rEscapeType = ESCAPE_OCTET;
                        }
                        break;
                }
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }

        case NOT_CANONIC:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && ((nWeight1 = getHexWeight(rBegin[0])) >= 0)
                && ((nWeight2 = getHexWeight(rBegin[1])) >= 0))
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                rEscapeType = ESCAPE_OCTET;
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }
    }
    return nUTF32;
}

static rtl::OUString encodeText(sal_Unicode const * pBegin,
                                sal_Unicode const * pEnd, bool bOctets,
                                Part ePart, sal_Char cEscapePrefix,
                                EncodeMechanism eMechanism,
                                rtl_TextEncoding eCharset,
                                bool bKeepVisibleEscapes)
{
    rtl::OUStringBuffer aResult;
    while (pBegin != pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, bOctets, cEscapePrefix,
                                     eMechanism, eCharset, eEscapeType);
        appendUCS4(aResult, nUTF32, eEscapeType, bOctets, ePart,
                   cEscapePrefix, eCharset, bKeepVisibleEscapes);
    }
    return aResult.makeStringAndClear();
}

static rtl::OUString decode(sal_Unicode const * pBegin,
                            sal_Unicode const * pEnd, sal_Char cEscapePrefix,
                            DecodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    switch (eMechanism)
    {
        case NO_DECODE:
            return rtl::OUString(pBegin, pEnd - pBegin);

        case DECODE_TO_IURI:
            eCharset = RTL_TEXTENCODING_UTF8;
            break;
    }
    rtl::OUStringBuffer aResult;
    while (pBegin < pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, false, cEscapePrefix,
                                     WAS_ENCODED, eCharset, eEscapeType);
        switch (eEscapeType)
        {
            case ESCAPE_NO:
                aResult.append(sal_Unicode(nUTF32));
                break;

            case ESCAPE_OCTET:
                appendEscape(aResult, cEscapePrefix, nUTF32);
                break;

            case ESCAPE_UTF32:
                if (eMechanism == DECODE_TO_IURI && isUSASCII(nUTF32))
                    appendEscape(aResult, cEscapePrefix, nUTF32);
                else
                    aResult.append(sal_Unicode(nUTF32));
                break;
        }
    }
    return aResult.makeStringAndClear();
}

}

uri NeonUri::sUriDefaults = { "http", NULL, DEFAULT_HTTP_PORT, NULL };

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonUri::NeonUri( const OUString & inUri )
{
    if ( inUri.getLength() <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    OString theInputUri (
        inUri.getStr(), inUri.getLength(), RTL_TEXTENCODING_UTF8);

    uri theUri;
    if ( uri_parse( theInputUri.getStr(), &theUri, &sUriDefaults ) != 0 )
    {
        uri_free( &theUri );
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    mScheme     = OStringToOUString( theUri.scheme, RTL_TEXTENCODING_UTF8 );
    mHostName   = OStringToOUString( theUri.host, RTL_TEXTENCODING_UTF8 );
    mPort       = theUri.port;
    mPath       = OStringToOUString( theUri.path, RTL_TEXTENCODING_UTF8 );

    uri_free( &theUri );

    calculateURI ();
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonUri::~NeonUri( )
{
}

void NeonUri::calculateURI ()
{
    mURI = mScheme;
    mURI += OUString::createFromAscii ("://");
    mURI += mHostName;
    mURI += OUString::createFromAscii (":");
    mURI += OUString::valueOf (mPort);
    mURI += mPath;
}

::rtl::OUString NeonUri::GetPathBaseName () const
{
    sal_Int32 nPos = mPath.lastIndexOf ('/');
    sal_Int32 nTrail = 0;
    if (nPos == mPath.getLength () - 1)
    {
        // Trailing slash found. Skip.
        nTrail = 1;
        nPos = mPath.lastIndexOf ('/', nPos);
    }
    if (nPos != -1)
        return mPath.copy (nPos + 1, mPath.getLength () - nPos - 1 - nTrail);
    else
        return OUString::createFromAscii ("/");
}

::rtl::OUString NeonUri::GetPathBaseNameUnescaped () const
{
    OUString aName = GetPathBaseName();
    return webdav_ucp_impl::decode( aName.getStr(),
                                    aName.getStr() + aName.getLength(),
                                    '%',
                                    webdav_ucp_impl::DECODE_WITH_CHARSET,
                                    RTL_TEXTENCODING_UTF8 );
}

::rtl::OUString NeonUri::GetPathDirName () const
{
    sal_Int32 nPos = mPath.lastIndexOf ('/');
    if (nPos == mPath.getLength () - 1)
    {
        // Trailing slash found. Skip.
        nPos = mPath.lastIndexOf ('/', nPos);
    }
    if (nPos != -1)
        return mPath.copy (0, nPos + 1);
    else
        return OUString::createFromAscii ("/");
}

void NeonUri::AppendPath (const OUString& path)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += OUString::createFromAscii ("/");

    mPath += path;
    calculateURI ();
};

// static
OUString NeonUri::escapeSegment( const OUString& segment )
{
    return webdav_ucp_impl::encodeText( segment.getStr(),
                                          segment.getStr() + segment.getLength(),
                                        false,
                                          webdav_ucp_impl::PART_PCHAR,
                                        '%',
                                        webdav_ucp_impl::ENCODE_ALL,
                                        RTL_TEXTENCODING_UTF8,
                                          false);
}

