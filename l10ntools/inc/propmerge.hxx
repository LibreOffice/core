/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _PROPMERGE_INCLUDED
#define _PROPMERGE_INCLUDED

#include <rtl/string.hxx>
#include <vector>

class PropParser
{
private:
    std::vector<OString> m_vLines;
    OString m_sSource;
    OString m_sLang;
    bool m_bIsInitialized;

public:
    PropParser(
        const OString& rInputFile, const OString& rLang,
        const bool bMergeMode );
    ~PropParser();

    bool isInitialized() const { return m_bIsInitialized; }
    void Extract(
        const OString& rSDFFile, const OString& rPrj, const OString& rRoot );
    void Merge( const OString &rSDFFile, const OString &rDestinationFile );
};

#endif //_PROPMERGE_INCLUDED
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
