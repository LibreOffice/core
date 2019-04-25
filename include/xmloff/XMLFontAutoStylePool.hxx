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

#pragma once

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <tools/fontenum.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <memory>

class XMLFontAutoStylePool_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLFontAutoStylePool : public salhelper::SimpleReferenceObject
{
private:
    SvXMLExport& rExport;

    std::unique_ptr<XMLFontAutoStylePool_Impl> m_pFontAutoStylePool;
    std::set<OUString> m_aNames;
    bool const m_bTryToEmbedFonts;
    std::unordered_map<OString, OUString> m_aEmbeddedFontFiles;

    OUString embedFontFile(OUString const & rFileUrl, OUString const & rFamilyName);

    std::unordered_set<OUString> getUsedFontList();

protected:
    bool m_bEmbedUsedOnly;
    bool m_bEmbedLatinScript;
    bool m_bEmbedAsianScript;
    bool m_bEmbedComplexScript;

    SvXMLExport& GetExport() { return rExport; }

public:
    XMLFontAutoStylePool( SvXMLExport& rExport, bool tryToEmbedFonts = false );
    virtual ~XMLFontAutoStylePool() override;

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

    void setEmbedOnlyUsedFonts(bool bEmbedUsedOnly)
    {
        m_bEmbedUsedOnly = bEmbedUsedOnly;
    }
    void setEmbedFontScripts(bool bEmbedLatinScript, bool bEmbedAsianScript, bool bEmbedComplexScript)
    {
        m_bEmbedLatinScript = bEmbedLatinScript;
        m_bEmbedAsianScript = bEmbedAsianScript;
        m_bEmbedComplexScript = bEmbedComplexScript;
    }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
