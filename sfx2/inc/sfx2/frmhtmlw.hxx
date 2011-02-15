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
#ifndef _FRMHTMLW_HXX
#define _FRMHTMLW_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

#include <sfx2/frmdescr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/stream.hxx>
#include <svtools/parhtml.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>

class SfxFrameSetObjectShell;
class SfxFrame;

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class SFX2_DLLPUBLIC SfxFrameHTMLWriter
{
    SAL_DLLPRIVATE static const sal_Char sNewLine[];
    SAL_DLLPRIVATE static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const String& rName,
                                const String& rContent, sal_Bool bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars = 0 );
    SAL_DLLPRIVATE inline static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const sal_Char *pName,
                                const String& rContent, sal_Bool bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars = 0 );

public:
    static void Out_DocInfo( SvStream& rStrm, const String& rBaseURL,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            const sal_Char *pIndent,
            rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
            String *pNonConvertableChars = 0 );

    static void Out_FrameDescriptor(
        SvStream&, const String& rBaseURL, const com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet >& xSet,
        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
        String *pNonConvertableChars = 0 );

    String CreateURL( SfxFrame* pFrame );

};

inline void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                            const sal_Char *pIndent, const sal_Char *pName,
                            const String& rContent, sal_Bool bHTTPEquiv,
                            rtl_TextEncoding eDestEnc,
                            String *pNonConvertableChars )
{
    String sTmp = String::CreateFromAscii( pName );
    OutMeta( rStrm, pIndent, sTmp, rContent, bHTTPEquiv, eDestEnc, pNonConvertableChars );
}

#endif

