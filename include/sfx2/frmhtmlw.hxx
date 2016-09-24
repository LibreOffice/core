/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_FRMHTMLW_HXX
#define INCLUDED_SFX2_FRMHTMLW_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>

#include <sfx2/frmdescr.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <svtools/parhtml.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmlkywd.hxx>

class SfxFrame;
class SvStream;

namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class SFX2_DLLPUBLIC SfxFrameHTMLWriter
{
    SAL_DLLPRIVATE static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const OUString& rName,
                                const OUString& rContent, bool bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                                OUString *pNonConvertableChars = nullptr );
    SAL_DLLPRIVATE inline static void OutMeta( SvStream& rStrm,
                                const sal_Char *pIndent, const sal_Char *pName,
                                const OUString& rContent, bool bHTTPEquiv,
                                rtl_TextEncoding eDestEnc,
                                OUString *pNonConvertableChars = nullptr );

public:
    static void Out_DocInfo( SvStream& rStrm, const OUString& rBaseURL,
            const css::uno::Reference< css::document::XDocumentProperties>&,
            const sal_Char *pIndent,
            rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
            OUString *pNonConvertableChars = nullptr );

    static void Out_FrameDescriptor(
        SvStream&, const OUString& rBaseURL, const css::uno::Reference < css::beans::XPropertySet >& xSet,
        rtl_TextEncoding eDestEnc = RTL_TEXTENCODING_MS_1252,
        OUString *pNonConvertableChars = nullptr );
};

inline void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                            const sal_Char *pIndent, const sal_Char *pName,
                            const OUString& rContent, bool bHTTPEquiv,
                            rtl_TextEncoding eDestEnc,
                            OUString *pNonConvertableChars )
{
    OUString sTmp = OUString::createFromAscii(pName);
    OutMeta( rStrm, pIndent, sTmp, rContent, bHTTPEquiv, eDestEnc, pNonConvertableChars );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
