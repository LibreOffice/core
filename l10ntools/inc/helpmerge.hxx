/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
