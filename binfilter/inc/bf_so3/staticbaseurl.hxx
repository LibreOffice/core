/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SO3_INC_BF_XMLOFF_STATICBASEURL_HXX
#define INCLUDED_SO3_INC_BF_XMLOFF_STATICBASEURL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif
#ifndef _URLOBJ_HXX
#include "tools/urlobj.hxx"
#endif

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

class ByteString;
class String;
 
namespace binfilter {

/** A collection of static functions from tools/inc/urlobj.hxx (rev. 1.26) and
    svtools/inc/urihelper.hxx (rev. 1.2) that have since been removed, but need
    to remain available in so3 and binfilter.
 */
namespace StaticBaseUrl {

String RelToAbs(
    String const & rTheRelURIRef, bool bIgnoreFragment = false,
    INetURLObject::EncodeMechanism eEncodeMechanism
    = INetURLObject::WAS_ENCODED,
    INetURLObject::DecodeMechanism eDecodeMechanism
    = INetURLObject::DECODE_TO_IURI,
    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
    INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

String AbsToRel(
    String const & rTheAbsURIRef,
    INetURLObject::EncodeMechanism eEncodeMechanism
    = INetURLObject::WAS_ENCODED,
    INetURLObject::DecodeMechanism eDecodeMechanism
    = INetURLObject::DECODE_TO_IURI,
    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
    INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

bool SetBaseURL(
    String const & rTheBaseURIRef,
    INetURLObject::EncodeMechanism eMechanism = INetURLObject::WAS_ENCODED,
    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

String GetBaseURL(
    INetURLObject::DecodeMechanism eMechanism = INetURLObject::DECODE_TO_IURI,
    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

String SmartRelToAbs(
    String const & rTheRelURIRef, bool bIgnoreFragment = false,
    INetURLObject::EncodeMechanism eEncodeMechanism
    = INetURLObject::WAS_ENCODED,
    INetURLObject::DecodeMechanism eDecodeMechanism
    = INetURLObject::DECODE_TO_IURI,
    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
    INetURLObject::FSysStyle eStyle = INetURLObject::FSYS_DETECT);

}

}

#endif
