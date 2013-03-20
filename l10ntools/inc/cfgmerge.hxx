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

#pragma once
#if 1

#include "sal/config.h"

#include <fstream>
#include <vector>

#include "boost/unordered_map.hpp"
#include "po.hxx"

typedef boost::unordered_map<rtl::OString, rtl::OString, rtl::OStringHash> OStringHashMap;


//
// class CfgStackData
//

class CfgStackData
{
friend class CfgParser;
friend class CfgExport;
friend class CfgMerge;
private:
    rtl::OString sTagType;
    rtl::OString sIdentifier;

    rtl::OString sResTyp;

    rtl::OString sTextTag;
    rtl::OString sEndTextTag;

    OStringHashMap sText;
public:
    CfgStackData(const rtl::OString &rTag, const rtl::OString &rId)
        : sTagType( rTag ), sIdentifier( rId )
    {}

    const rtl::OString &GetTagType() { return sTagType; }
    const rtl::OString &GetIdentifier() { return sIdentifier; }

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

    CfgStackData *Push(const rtl::OString &rTag, const rtl::OString &rId);
    CfgStackData *Pop()
    {
        if (maList.empty())
            return NULL;
        CfgStackData* temp = maList.back();
        maList.pop_back();
        return temp;
    }

    CfgStackData *GetStackData();

    rtl::OString GetAccessPath( size_t nPos );

    size_t size() const { return maList.size(); }
};

//
// class CfgParser
//

class CfgParser
{
protected:
    rtl::OString sCurrentResTyp;
    rtl::OString sCurrentIsoLang;
    rtl::OString sCurrentText;

    rtl::OString sLastWhitespace;

    CfgStack aStack;
    CfgStackData *pStackData;

    sal_Bool bLocalize;

    virtual void WorkOnText(
        rtl::OString &rText,
        const rtl::OString &rLangIndex )=0;

    virtual void WorkOnResourceEnd()=0;

    virtual void Output(const rtl::OString & rOutput)=0;

    void Error(const rtl::OString &rError);

private:
    int ExecuteAnalyzedToken( int nToken, char *pToken );
    std::vector<rtl::OString> aLanguages;
    void AddText(
        rtl::OString &rText,
        const rtl::OString &rIsoLang,
        const rtl::OString &rResTyp );

    sal_Bool IsTokenClosed(const rtl::OString &rToken);

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
    rtl::OString sPrj;
    rtl::OString sPath;
    std::vector<rtl::OString> aLanguages;

    PoOfstream pOutputStream;

protected:
    virtual void WorkOnText(
        rtl::OString &rText,
        const rtl::OString &rIsoLang
        );

    void WorkOnResourceEnd();
    void Output(const rtl::OString& rOutput);
public:
    CfgExport(
        const rtl::OString &rOutputFile,
        const rtl::OString &rProject,
        const rtl::OString &rFilePath
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
    std::vector<rtl::OString> aLanguages;
    ResData *pResData;

    rtl::OString sFilename;
    sal_Bool bEnglish;

    std::ofstream pOutputStream;

protected:
    virtual void WorkOnText(rtl::OString &rText, const rtl::OString &rLangIndex);

    void WorkOnResourceEnd();

    void Output(const rtl::OString& rOutput);
public:
    CfgMerge(const rtl::OString &rMergeSource,
        const rtl::OString &rOutputFile, const rtl::OString &rFilename);
    ~CfgMerge();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
