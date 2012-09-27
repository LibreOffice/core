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

#include "export.hxx"
#include "xmlparse.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <memory> /* auto_ptr */

#define MOVEFILE_REPLACE_EXISTING 0x01

/// This Class is responsible for extracting/merging OpenOffice XML Helpfiles
class HelpParser
{
private:
    rtl::OString sHelpFile;

#if OSL_DEBUG_LEVEL > 2
    /// Debugmethod, prints the content of the map to stdout
    static void Dump(LangHashMap* rElem_in , const rtl::OString & sKey_in);

    /// Debugmethod, prints the content of the map to stdout
    static void Dump(XMLHashMap* rElem_in);
#endif

public:
    HelpParser( const rtl::OString &rHelpFile );
    ~HelpParser(){};

/// Method creates/append a SDF file with the content of a parsed XML file
/// @PRECOND rHelpFile is valid
    static bool CreateSDF( const rtl::OString &rSDFFile_in, const rtl::OString &rPrj_in, const rtl::OString &rRoot_in,
                           const rtl::OString &sHelpFile, XMLFile *pXmlFile, const rtl::OString &rGsi1 );

    static  void parse_languages( std::vector<rtl::OString>& aLanguages , MergeDataFile& aMergeDataFile );

/// Method merges the String from the SDFfile into XMLfile. Both Strings must
/// point to existing files.
    bool Merge( const rtl::OString &rSDFFile_in, const rtl::OString &rDestinationFile_in ,
        const rtl::OString& sLanguage , MergeDataFile& aMergeDataFile );
    bool Merge( const rtl::OString &rSDFFile, const rtl::OString &rPathX , const rtl::OString &rPathY , bool bISO
        , const std::vector<rtl::OString>& aLanguages , MergeDataFile& aMergeDataFile , bool bCreateDir );

private:
    rtl::OString GetOutpath( const rtl::OString& rPathX , const rtl::OString& sCur , const rtl::OString& rPathY );
    bool MergeSingleFile( XMLFile* file , MergeDataFile& aMergeDataFile , const rtl::OString& sLanguage , rtl::OString const & sPath );

    void ProcessHelp( LangHashMap* aLangHM , const rtl::OString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile );
    void MakeDir(const rtl::OString& rPath);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
