/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hierarchyuri.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:29:52 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _HIERARCHYURI_HXX
#include "hierarchyuri.hxx"
#endif

using namespace hierarchy_ucp;

//=========================================================================

#define DEFAULT_DATA_SOURCE_SERVICE \
                    "com.sun.star.ucb.DefaultHierarchyDataSource"

//=========================================================================
//=========================================================================
//
// HierarchyUri Implementation.
//
//=========================================================================
//=========================================================================

void HierarchyUri::init() const
{
    // Already inited?
    if ( m_aUri.getLength() && !m_aPath.getLength() )
    {
        // Note: Maybe it's a re-init, setUri only resets m_aPath!
        m_aService = m_aParentUri = m_aName = rtl::OUString();

        // URI must match at least: <sheme>:
        if ( ( m_aUri.getLength() < HIERARCHY_URL_SCHEME_LENGTH + 1 ) )
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString::createFromAscii( "/" );
            return;
        }

        // Scheme is case insensitive.
        rtl::OUString aScheme
            = m_aUri.copy( 0, HIERARCHY_URL_SCHEME_LENGTH ).toAsciiLowerCase();
        if ( aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( HIERARCHY_URL_SCHEME ) ) )
        {
            m_aUri = m_aUri.replaceAt( 0, aScheme.getLength(), aScheme );

            sal_Int32 nPos = 0;

            // If the URI has no service specifier, insert default service.
            // This is for backward compatibility and for convenience.

            if ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 1 )
            {
                // root folder URI without path and service specifier.
                m_aUri += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "//" DEFAULT_DATA_SOURCE_SERVICE "/" ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos = m_aUri.getLength() - 1;
//              nPos = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else if ( ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri.getStr()[ HIERARCHY_URL_SCHEME_LENGTH + 1 ]
                                                    == sal_Unicode( '/' ) ) )
            {
                // root folder URI without service specifier.
                m_aUri += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "/" DEFAULT_DATA_SOURCE_SERVICE "/" ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos = m_aUri.getLength() - 1;
//              nPos = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else if ( ( m_aUri.getLength() > HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri.getStr()[ HIERARCHY_URL_SCHEME_LENGTH + 2 ]
                                                    != sal_Unicode( '/' ) ) )
            {
                // other (no root folder) URI without service specifier.
                m_aUri = m_aUri.replaceAt(
                            HIERARCHY_URL_SCHEME_LENGTH + 2,
                            0,
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                "/" DEFAULT_DATA_SOURCE_SERVICE "/" ) ) );
                m_aService
                    = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            DEFAULT_DATA_SOURCE_SERVICE ) );

                nPos = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else
            {
                // URI with service specifier.
                sal_Int32 nStart = HIERARCHY_URL_SCHEME_LENGTH + 3;

                // Here: - m_aUri has at least the form "<scheme>://"
                //       - nStart points to char after <scheme>://

                // Only <scheme>:// ?
                if ( nStart == m_aUri.getLength() )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString::createFromAscii( "/" );
                    return;
                }

                // Empty path segments?
                if ( m_aUri.indexOf(
                        rtl::OUString::createFromAscii( "//" ), nStart ) != -1 )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString::createFromAscii( "/" );
                    return;
                }

                sal_Int32 nEnd = m_aUri.indexOf( '/', nStart );

                // Only <scheme>:/// ?
                if ( nEnd == nStart )
                {
                    // error, but remember that we did a init().
                    m_aPath = rtl::OUString::createFromAscii( "/" );
                    return;
                }

                if ( nEnd == -1 )
                {
                    // Trailing slash missing.
                    nEnd = m_aUri.getLength();
                    m_aUri += rtl::OUString::createFromAscii( "/" );
                }

                m_aService = m_aUri.copy( nStart, nEnd - nStart );

                nPos = nEnd;
//              nPos = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }

            // Here: - m_aUri has at least the form "<scheme>://<service>/"
            //       - m_aService was set
            //       - m_aPath, m_aParentPath, m_aName not yet set
            //       - nPos points to slash after service specifier

            // Remove trailing slash, if not a root folder URI.
            sal_Int32 nEnd = m_aUri.lastIndexOf( '/' );
            if ( ( nEnd > nPos ) && ( nEnd == ( m_aUri.getLength() - 1 ) ) )
                m_aUri = m_aUri.copy( 0, nEnd );

            // Path (includes leading slash)
            m_aPath = m_aUri.copy( nPos );

            // parent URI + name
            sal_Int32 nLastSlash = m_aUri.lastIndexOf( '/' );
            if ( ( nLastSlash != -1 ) &&
                 ( nLastSlash != m_aUri.getLength() - 1 ) ) // root
            {
                m_aParentUri = m_aUri.copy( 0, nLastSlash );
                m_aName      = m_aUri.copy( nLastSlash + 1 );
            }

            // success
            m_bValid = true;
        }
        else
        {
            // error, but remember that we did a init().
            m_aPath = rtl::OUString::createFromAscii( "/" );
        }
    }
}

//=========================================================================
//
// URI encode/decode stuff.
//
//=========================================================================

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
    OSL_ASSERT(rBegin && rBegin < pEnd);
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
    OSL_ASSERT(nWeight >= 0 && nWeight < 16);
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
    OSL_ASSERT(nUCS4 < 0x80000000);
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
    bool bEscape = false;
    rtl_TextEncoding eTargetCharset = RTL_TEXTENCODING_ASCII_US;
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
                OSL_ASSERT(false);
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
    OSL_ASSERT(rBegin < pEnd);
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
            if (nUTF32 == sal::static_int_cast<sal_uInt32>(cEscapePrefix) && rBegin + 1 < pEnd
                && (nWeight1 = getHexWeight(rBegin[0])) >= 0
                && (nWeight2 = getHexWeight(rBegin[1])) >= 0)
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                switch (eCharset)
                {
                    default:
                        OSL_ASSERT(false);
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
            if (nUTF32 == sal::static_int_cast<sal_uInt32>(cEscapePrefix) && rBegin + 1 < pEnd
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

/*
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

//=========================================================================
// static
rtl::OUString HierarchyUri::decodeSegment( const rtl::OUString& rSource )
{
    return decode(rSource.getStr(), rSource.getStr() + rSource.getLength(),
                  '%', DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8);
}
*/

//============================================================================
// static
rtl::OUString HierarchyUri::encodeURL( const rtl::OUString& rSource )
{
    rtl::OUStringBuffer aResult;

    sal_Unicode const * pBegin = rSource.getStr();
    sal_Unicode const * pEnd = pBegin + rSource.getLength();
    sal_Unicode const * p = pBegin;
    while (p != pEnd && *p++ != ':');
    aResult.append(pBegin, p - pBegin);

    while (p != pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(p, pEnd, false, '%', WAS_ENCODED,
                                     RTL_TEXTENCODING_UTF8, eEscapeType);
        if (nUTF32 == '/' && eEscapeType == ESCAPE_NO)
            aResult.append(sal_Unicode(nUTF32));
        else
            appendUCS4(aResult, nUTF32, eEscapeType, false, PART_PCHAR, '%',
                       RTL_TEXTENCODING_UTF8, false);
    }

    return aResult.makeStringAndClear();
}

//=========================================================================
// static
rtl::OUString HierarchyUri::encodeSegment( const rtl::OUString& rSource )
{
    return encodeText(rSource.getStr(),
                      rSource.getStr() + rSource.getLength(), false,
                      PART_PCHAR, '%', ENCODE_ALL, RTL_TEXTENCODING_UTF8,
                      false);
}

