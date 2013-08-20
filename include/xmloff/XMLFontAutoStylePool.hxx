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

#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>
#include <xmloff/uniref.hxx>
#include <set>

class XMLFontAutoStylePool_Impl;
typedef std::set<OUString> XMLFontAutoStylePoolNames_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLFontAutoStylePool : public UniRefBase
{
    SvXMLExport& rExport;

    XMLFontAutoStylePool_Impl *pPool;
    XMLFontAutoStylePoolNames_Impl m_aNames;
    bool tryToEmbedFonts;

    OUString embedFontFile( const OUString& fontUrl );

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLFontAutoStylePool( SvXMLExport& rExport, bool tryToEmbedFonts = false );
    ~XMLFontAutoStylePool();

    OUString Add(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    OUString Find(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc )const;

    void exportXML();
};

#endif  //  _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
