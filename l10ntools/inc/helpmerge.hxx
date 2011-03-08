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
// local includes
#include "export.hxx"
#include "xmlparse.hxx"
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <memory> /* auto_ptr */
#include "tools/isofallback.hxx"

#define MOVEFILE_REPLACE_EXISTING 0x01

/// This Class is responsible for extracting/merging OpenOffice XML Helpfiles
class HelpParser
{
private:
    ByteString sHelpFile;
    bool       bUTF8;
    bool       bHasInputList;

/// Copy fallback language String (ENUS,DE) into position of the numeric language iso code
/// @PRECOND 0 < langIdx_in < MAX_IDX
    static void FillInFallbacks( LangHashMap& rElem_out, ByteString sLangIdx_in );

/// Debugmethod, prints the content of the map to stdout
    static  void Dump(  LangHashMap* rElem_in , const ByteString sKey_in );

/// Debugmethod, prints the content of the map to stdout
    static  void Dump(  XMLHashMap* rElem_in ) ;



public:
    HelpParser( const ByteString &rHelpFile, bool bUTF8 , bool bHasInputList );
    ~HelpParser(){};

/// Method creates/append a SDF file with the content of a parsed XML file
/// @PRECOND rHelpFile is valid
    static bool CreateSDF( const ByteString &rSDFFile_in, const ByteString &rPrj_in, const ByteString &rRoot_in,
                           const ByteString &sHelpFile, XMLFile *pXmlFile, const ByteString &rGsi1 );

    static  void parse_languages( std::vector<ByteString>& aLanguages , MergeDataFile& aMergeDataFile );

/// Method merges the String from the SDFfile into XMLfile. Both Strings must
/// point to existing files.
    bool Merge( const ByteString &rSDFFile_in, const ByteString &rDestinationFile_in , ByteString& sLanguage , MergeDataFile& aMergeDataFile );
    bool Merge( const ByteString &rSDFFile, const ByteString &rPathX , const ByteString &rPathY , bool bISO
        , const std::vector<ByteString>& aLanguages , MergeDataFile& aMergeDataFile , bool bCreateDir );

private:
    static ByteString makeAbsolutePath( const ByteString& sHelpFile , const ByteString& rRoot_in );

    ByteString GetOutpath( const ByteString& rPathX , const ByteString& sCur , const ByteString& rPathY );
    bool MergeSingleFile( XMLFile* file , MergeDataFile& aMergeDataFile , const ByteString& sLanguage , ByteString sPath );

    void Process( LangHashMap* aLangHM , const ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile );
    void ProcessHelp( LangHashMap* aLangHM , const ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile );
    void MakeDir( const ByteString& sPath );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
