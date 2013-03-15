/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _TREEMERGE_INCLUDED
#define _TREEMERGE_INCLUDED

#include <libxml/tree.h>
#include <rtl/string.hxx>
#include <vector>

/** Class for tree localization

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
    TreeParser(
        const OString& rInputFile, const OString& rLang );
    ~TreeParser();

    bool isInitialized() const { return m_bIsInitialized; }
    void Extract( const OString& rPOFile );
    void Merge(
        const OString &rMergeSrc, const OString &rDestinationFile,
        const OString &rXhpRoot );
};

#endif //_TREEMERGE_INCLUDED
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
