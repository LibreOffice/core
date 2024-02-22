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
#include <rtl/ustring.hxx>

class SvStream;

namespace com::sun::star {
    namespace document {
        class XDocumentProperties;
    }
}

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

class SFX2_DLLPUBLIC SfxFrameHTMLWriter
{
    SAL_DLLPRIVATE static void OutMeta( SvStream& rStrm,
                                const char *pIndent, std::u16string_view rName,
                                std::u16string_view rContent, bool bHTTPEquiv,
                                OUString *pNonConvertableChars = nullptr );
    SAL_DLLPRIVATE inline static void OutMeta( SvStream& rStrm,
                                const char *pIndent, const char *pName,
                                std::u16string_view rContent, bool bHTTPEquiv,
                                OUString *pNonConvertableChars = nullptr );

public:
    static void Out_DocInfo( SvStream& rStrm, const OUString& rBaseURL,
            const css::uno::Reference< css::document::XDocumentProperties>&,
            const char *pIndent,
            OUString *pNonConvertableChars = nullptr );

    static void Out_FrameDescriptor(
        SvStream&, const OUString& rBaseURL, const css::uno::Reference < css::beans::XPropertySet >& xSet);
};

inline void SfxFrameHTMLWriter::OutMeta( SvStream& rStrm,
                            const char *pIndent, const char *pName,
                            std::u16string_view rContent, bool bHTTPEquiv,
                            OUString *pNonConvertableChars )
{
    OUString sTmp = OUString::createFromAscii(pName);
    OutMeta( rStrm, pIndent, sTmp, rContent, bHTTPEquiv, pNonConvertableChars );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
