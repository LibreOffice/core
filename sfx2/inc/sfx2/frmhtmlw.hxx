/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

