/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_L10NTOOLS_INC_TREEMERGE_HXX
#define INCLUDED_L10NTOOLS_INC_TREEMERGE_HXX

#include <libxml/tree.h>
#include <rtl/string.hxx>

/**
  Class for localization of *.tree files

  Parse *.tree files, extract translatable strings,
  merge translated strings and update reference and title
  of referred help files.
*/
class TreeParser
{
private:
    xmlDocPtr m_pSource;
    OString m_sLang;
    bool m_bIsInitialized;

public:
    /// Parse tree file
    TreeParser( const OString& rInputFile, OString sLang );
    ~TreeParser();

    bool isInitialized() const { return m_bIsInitialized; }
    /// Export strings
    void Extract( const OString& rPOFile );
    /// Merge strings to tree file and update reference to help files(xhp)
    void Merge(
        const OString &rMergeSrc, const OString &rDestinationFile,
        const OString &rXhpRoot );
};

#endif // INCLUDED_L10NTOOLS_INC_TREEMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
