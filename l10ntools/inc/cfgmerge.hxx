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



#ifndef _CFG_MERGE_HXX
#define _CFG_MERGE_HXX

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <hash_map>

typedef std::hash_map<ByteString , ByteString , hashByteString,equalByteString>
                                ByteStringHashMap;


//
// class CfgStackData
//

class CfgStackData
{
friend class CfgParser;
friend class CfgExport;
friend class CfgMerge;
private:
    ByteString sTagType;
    ByteString sIdentifier;

    ByteString sResTyp;

    ByteString sTextTag;
    ByteString sEndTextTag;

    ByteStringHashMap sText;
public:
    CfgStackData( const ByteString &rTag, const ByteString &rId )
            : sTagType( rTag ), sIdentifier( rId ) {};

    ByteString &GetTagType() { return sTagType; }
    ByteString &GetIdentifier() { return sIdentifier; }

};

//
// class CfgStack
//

DECLARE_LIST( CfgStackList, CfgStackData * )

class CfgStack : public CfgStackList
{
public:
    CfgStack() : CfgStackList( 10, 10 ) {}
    ~CfgStack();

    sal_uLong Push( CfgStackData *pStackData );
    CfgStackData *Push( const ByteString &rTag, const ByteString &rId );
    CfgStackData *Pop() { return Remove( Count() - 1 ); }

    CfgStackData *GetStackData( sal_uLong nPos = LIST_APPEND );

    ByteString GetAccessPath( sal_uLong nPos = LIST_APPEND );
};

//
// class CfgParser
//

class CfgParser
{
protected:
    ByteString sCurrentResTyp;
    ByteString sCurrentIsoLang;
    ByteString sCurrentText;

    ByteString sLastWhitespace;

    CfgStack aStack;
    CfgStackData *pStackData;

    sal_Bool bLocalize;

    virtual void WorkOnText(
        ByteString &rText,
        const ByteString &nLangIndex )=0;

    virtual void WorkOnRessourceEnd()=0;

    virtual void Output( const ByteString& rOutput )=0;

    void Error( const ByteString &rError );

private:
    int ExecuteAnalyzedToken( int nToken, char *pToken );
    std::vector<ByteString> aLanguages;
    void AddText(
        ByteString &rText,
        const ByteString &rIsoLang,
        const ByteString &rResTyp );

sal_Bool IsTokenClosed( const ByteString &rToken );

public:
    CfgParser();
    virtual ~CfgParser();

    int Execute( int nToken, char * pToken );
};

//
// class CfgOutputParser
//

class CfgOutputParser : public CfgParser
{
protected:
    SvFileStream *pOutputStream;
public:
    CfgOutputParser ( const ByteString &rOutputFile );
    virtual ~CfgOutputParser();
};

//
// class CfgExport
//

class CfgExport : public CfgOutputParser
{
private:
    ByteString sPrj;
    ByteString sPath;
    std::vector<ByteString> aLanguages;
protected:
    void WorkOnText(
        ByteString &rText,
        const ByteString &rIsoLang
        );

    void WorkOnRessourceEnd();
    void Output( const ByteString& rOutput );
public:
    CfgExport(
        const ByteString &rOutputFile,
        const ByteString &rProject,
        const ByteString &rFilePath
    );
    ~CfgExport();
};

//
// class CfgMerge
//

class CfgMerge : public CfgOutputParser
{
private:
    MergeDataFile *pMergeDataFile;
    std::vector<ByteString> aLanguages;
    ResData *pResData;

    sal_Bool bGerman;
    ByteString sFilename;
    sal_Bool bEnglish;

protected:
    void WorkOnText(
        ByteString &rText,
        const ByteString &nLangIndex );

    void WorkOnRessourceEnd();

    void Output( const ByteString& rOutput );
public:
    CfgMerge(
        const ByteString &rMergeSource,
        const ByteString &rOutputFile,
        ByteString &rFilename
    );
    ~CfgMerge();
};

#endif
