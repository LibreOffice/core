/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_L10NTOOLS_INC_PROPMERGE_HXX
#define INCLUDED_L10NTOOLS_INC_PROPMERGE_HXX

#include <rtl/string.hxx>
#include <vector>

/**
  Class for localization of *.properties files

  Parse *.properties files, extract translatable strings
  and merge translated strings.
*/

class PropParser
{
private:
    std::vector<OString> m_vLines;
    OString m_sSource;
    OString m_sLang;
    bool m_bIsInitialized;

public:
    PropParser(
        OString sInputFile, OString sLang,
        const bool bMergeMode );
    ~PropParser();

    bool isInitialized() const { return m_bIsInitialized; }
    void Extract( const OString& rPOFile );
    void Merge( const OString &rMergeSrc, const OString &rDestinationFile );
};

#endif // INCLUDED_L10NTOOLS_INC_PROPMERGE_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
