/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    bool       bUTF8;
    bool       bHasInputList;

#if OSL_DEBUG_LEVEL > 2
    /// Debugmethod, prints the content of the map to stdout
    static void Dump(LangHashMap* rElem_in , const rtl::OString & sKey_in);

    /// Debugmethod, prints the content of the map to stdout
    static void Dump(XMLHashMap* rElem_in);
#endif

public:
    HelpParser( const rtl::OString &rHelpFile, bool bUTF8 , bool bHasInputList );
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
