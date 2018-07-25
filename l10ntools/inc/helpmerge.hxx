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

#ifndef INCLUDED_L10NTOOLS_INC_HELPMERGE_HXX
#define INCLUDED_L10NTOOLS_INC_HELPMERGE_HXX

#include "xmlparse.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

/// This Class is responsible for extracting/merging OpenOffice XML Helpfiles
class HelpParser
{
private:
    OString sHelpFile;

#if OSL_DEBUG_LEVEL > 2
    /// Debugmethod, prints the content of the map to stdout
    static void Dump(LangHashMap* rElem_in , const OString & sKey_in);

    /// Debugmethod, prints the content of the map to stdout
    static void Dump(XMLHashMap* rElem_in);
#endif

public:
    HelpParser( const OString &rHelpFile );

/// Method append a PO file with the content of a parsed XML file
/// @PRECOND rHelpFile is valid
    static bool CreatePO( const OString &rPOFile_in, const OString &sHelpFile,
                          XMLFile *pXmlFile, const OString &rGsi1 );

/// Method merges the String into XMLfile, which must point to an existing file.
    bool Merge( const OString &rDestinationFile_in ,
        const OString& sLanguage , MergeDataFile* pMergeDataFile );

private:
    void MergeSingleFile( XMLFile* file , MergeDataFile* pMergeDataFile , const OString& sLanguage , OString const & sPath );

    static void ProcessHelp( LangHashMap* aLangHM , const OString& sCur , ResData *pResData , MergeDataFile* pMergeDataFile );
};

#endif // INCLUDED_L10NTOOLS_INC_HELPMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
