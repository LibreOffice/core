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

#ifndef _CFG_MERGE_HXX
#define _CFG_MERGE_HXX

#include <tools/string.hxx>
#include <boost/unordered_map.hpp>
#include <vector>

typedef boost::unordered_map<ByteString , ByteString , hashByteString,equalByteString>
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

typedef ::std::vector< CfgStackData* > CfgStackList;

class CfgStack
{
private:
    CfgStackList maList;

public:
    CfgStack() {}
    ~CfgStack();

    size_t Push( CfgStackData *pStackData );
    CfgStackData *Push( const ByteString &rTag, const ByteString &rId );
    CfgStackData *Pop()
        {
            if ( maList.empty() ) return NULL;
            CfgStackData* temp = maList.back();
            maList.pop_back();
            return temp;
        }

    CfgStackData *GetStackData( size_t nPos = LIST_APPEND );

    ByteString GetAccessPath( size_t nPos = LIST_APPEND );

    size_t size() const { return maList.size(); }
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

    BOOL bLocalize;

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

BOOL IsTokenClosed( const ByteString &rToken );

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

    BOOL bGerman;
    ByteString sFilename;
    BOOL bEnglish;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
