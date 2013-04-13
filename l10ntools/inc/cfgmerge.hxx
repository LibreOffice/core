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

#ifndef _CFG_MERGE_HXX
#define _CFG_MERGE_HXX

#include "sal/config.h"

#include <fstream>
#include <vector>

#include "boost/unordered_map.hpp"
#include "po.hxx"

typedef boost::unordered_map<OString, OString, OStringHash> OStringHashMap;


//
// class CfgStackData
//

class CfgStackData
{
friend class CfgParser;
friend class CfgExport;
friend class CfgMerge;
private:
    OString sTagType;
    OString sIdentifier;

    OString sResTyp;

    OString sTextTag;
    OString sEndTextTag;

    OStringHashMap sText;
public:
    CfgStackData(const OString &rTag, const OString &rId)
        : sTagType( rTag ), sIdentifier( rId )
    {}

    const OString &GetTagType() { return sTagType; }
    const OString &GetIdentifier() { return sIdentifier; }

};

//
// class CfgStack
//

typedef std::vector< CfgStackData* > CfgStackList;

class CfgStack
{
private:
    CfgStackList maList;

public:
    CfgStack() {}
    ~CfgStack();

    CfgStackData *Push(const OString &rTag, const OString &rId);
    CfgStackData *Pop()
    {
        if (maList.empty())
            return NULL;
        CfgStackData* temp = maList.back();
        maList.pop_back();
        return temp;
    }

    CfgStackData *GetStackData();

    OString GetAccessPath( size_t nPos );

    size_t size() const { return maList.size(); }
};

//
// class CfgParser
//

class CfgParser
{
protected:
    OString sCurrentResTyp;
    OString sCurrentIsoLang;
    OString sCurrentText;

    OString sLastWhitespace;

    CfgStack aStack;
    CfgStackData *pStackData;

    sal_Bool bLocalize;

    virtual void WorkOnText(
        OString &rText,
        const OString &rLangIndex )=0;

    virtual void WorkOnResourceEnd()=0;

    virtual void Output(const OString & rOutput)=0;

    void Error(const OString &rError);

private:
    int ExecuteAnalyzedToken( int nToken, char *pToken );
    std::vector<OString> aLanguages;
    void AddText(
        OString &rText,
        const OString &rIsoLang,
        const OString &rResTyp );

    sal_Bool IsTokenClosed(const OString &rToken);

public:
    CfgParser();
    virtual ~CfgParser();

    int Execute( int nToken, char * pToken );
};

//
// class CfgExport
//

class CfgExport : public CfgParser
{
private:
    OString sPath;
    std::vector<OString> aLanguages;

    PoOfstream pOutputStream;

protected:
    virtual void WorkOnText(
        OString &rText,
        const OString &rIsoLang
        );

    void WorkOnResourceEnd();
    void Output(const OString& rOutput);
public:
    CfgExport(
        const OString &rOutputFile,
        const OString &rFilePath
    );
    ~CfgExport();
};

//
// class CfgMerge
//

class CfgMerge : public CfgParser
{
private:
    MergeDataFile *pMergeDataFile;
    std::vector<OString> aLanguages;
    ResData *pResData;

    OString sFilename;
    sal_Bool bEnglish;

    std::ofstream pOutputStream;

protected:
    virtual void WorkOnText(OString &rText, const OString &rLangIndex);

    void WorkOnResourceEnd();

    void Output(const OString& rOutput);
public:
    CfgMerge(
        const OString &rMergeSource, const OString &rOutputFile,
        const OString &rFilename, const OString &rLanguage );
    ~CfgMerge();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
