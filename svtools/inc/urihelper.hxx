/*************************************************************************
 *
 *  $RCSfile: urihelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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

#ifndef SVTOOLS_URIHELPER_HXX
#define SVTOOLS_URIHELPER_HXX

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno {
    class RuntimeException;
} } } }
class ByteString;
class CharClass;
class UniString;

//============================================================================
namespace URIHelper {

UniString
SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
             ByteString const & rTheRelURIRef,
             Link const & rMaybeFileHdl = Link(),
             bool bCheckFileExists = true,
             bool bIgnoreFragment = false,
             INetURLObject::EncodeMechanism eEncodeMechanism
                 = INetURLObject::WAS_ENCODED,
             INetURLObject::DecodeMechanism eDecodeMechanism
                 = INetURLObject::DECODE_TO_IURI,
             rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
             bool bRelativeNonURIs = false,
             INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

UniString
SmartRel2Abs(INetURLObject const & rTheBaseURIRef,
             UniString const & rTheRelURIRef,
             Link const & rMaybeFileHdl = Link(),
             bool bCheckFileExists = true,
             bool bIgnoreFragment = false,
             INetURLObject::EncodeMechanism eEncodeMechanism
                 = INetURLObject::WAS_ENCODED,
             INetURLObject::DecodeMechanism eDecodeMechanism
                 = INetURLObject::DECODE_TO_IURI,
             rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
             bool bRelativeNonURIs = false,
             INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

//============================================================================
void SetMaybeFileHdl(Link const & rTheMaybeFileHdl);

//============================================================================
Link GetMaybeFileHdl();

//============================================================================
inline UniString
SmartRelToAbs(ByteString const & rTheRelURIRef,
              bool bIgnoreFragment = false,
              INetURLObject::EncodeMechanism eEncodeMechanism
                  = INetURLObject::WAS_ENCODED,
              INetURLObject::DecodeMechanism eDecodeMechanism
                  = INetURLObject::DECODE_TO_IURI,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT)
{
    return SmartRel2Abs(INetURLObject(INetURLObject::GetBaseURL()),
                        rTheRelURIRef, GetMaybeFileHdl(), true,
                        bIgnoreFragment, eEncodeMechanism, eDecodeMechanism,
                        eCharset, false, eStyle);
}

inline UniString
SmartRelToAbs(UniString const & rTheRelURIRef,
              bool bIgnoreFragment = false,
              INetURLObject::EncodeMechanism eEncodeMechanism
                  = INetURLObject::WAS_ENCODED,
              INetURLObject::DecodeMechanism eDecodeMechanism
                  = INetURLObject::DECODE_TO_IURI,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT)
{
    return SmartRel2Abs(INetURLObject(INetURLObject::GetBaseURL()),
                        rTheRelURIRef, GetMaybeFileHdl(), true,
                        bIgnoreFragment, eEncodeMechanism, eDecodeMechanism,
                        eCharset, false, eStyle);
}

//============================================================================
UniString
FindFirstURLInText(UniString const & rText,
                   xub_StrLen & rBegin,
                   xub_StrLen & rEnd,
                   CharClass const & rCharClass,
                   INetURLObject::EncodeMechanism eMechanism
                       = INetURLObject::WAS_ENCODED,
                   rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                   INetURLObject::FSysStyle eStyle
                       = INetURLObject::FSYS_DETECT);

//============================================================================
/** Remove any password component from both absolute and relative URLs.

    @ATT  The current implementation will not remove a password from a
    relative URL that has an authority component (e.g., the password is not
    removed from the relative ftp URL <//user:password@domain/path>).  But
    since our functions to translate between absolute and relative URLs never
    produce relative URLs with authority components, this is no real problem.

    @ATT  For relative URLs (or anything not recognized as an absolute URI),
    the current implementation will return the input unmodified, not applying
    any translations implied by the encode/decode parameters.

    @param rURI  An absolute or relative URI reference.

    @param eEncodeMechanism  See the general discussion for INetURLObject set-
    methods.

    @param eDecodeMechanism  See the general discussion for INetURLObject get-
    methods.

    @param eCharset  See the general discussion for INetURLObject get- and
    set-methods.

    @return  The input URI with any password component removed.
 */
UniString
removePassword(UniString const & rURI,
               INetURLObject::EncodeMechanism eEncodeMechanism
                   = INetURLObject::WAS_ENCODED,
               INetURLObject::DecodeMechanism eDecodeMechanism
                   = INetURLObject::DECODE_TO_IURI,
               rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

//============================================================================
/** Query the notational conventions used in the file system provided by some
    file content provider.

    @param rFileUrl  This file URL determines which file content provider is
    used to query the desired information.  (The UCB's usual mapping from URLs
    to content providers is used.)

    @param bAddConvenienceStyles  If true, the return value contains not only
    the style bit corresponding to the queried content provider's conventions,
    but may also contain additional style bits that make using this function
    more convenient in certain situations.  Currently, the effect is that
    FSYS_UNX is extended with FSYS_VOS, and both FSYS_DOS and FSYS_MAC are
    extended with FSYS_VOS and FSYS_UNX (i.e., the---unambiguous---detection
    of VOS style and Unix style file system paths is always enabled); also, in
    case the content provider's conventions cannot be determined, FSYS_DETECT
    is returned instead of FSysStyle(0).

    @return  The style bit corresponding to the queried content provider's
    conventions, or FSysStyle(0) if these cannot be determined.
 */
INetURLObject::FSysStyle queryFSysStyle(UniString const & rFileUrl,
                                        bool bAddConvenienceStyles = true)
    throw (com::sun::star::uno::RuntimeException);

}

#endif // SVTOOLS_URIHELPER_HXX
