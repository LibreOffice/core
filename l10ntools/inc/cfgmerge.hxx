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

#ifndef INCLUDED_L10NTOOLS_INC_CFGMERGE_HXX
#define INCLUDED_L10NTOOLS_INC_CFGMERGE_HXX

#include "sal/config.h"

#include <fstream>
#include <unordered_map>
#include <vector>
#include "po.hxx"

typedef std::unordered_map<OString, OString, OStringHash> OStringHashMap;



// class CfgStackData


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


// class CfgStack


class CfgStack
{
private:
    std::vector< CfgStackData* > maList;

public:
    CfgStack() {}
    ~CfgStack();

    CfgStackData *Push(const OString &rTag, const OString &rId);
    void Pop()
    {
        if (!maList.empty())
        {
            delete maList.back();
            maList.pop_back();
        }
    }

    CfgStackData *GetStackData();

    OString GetAccessPath( size_t nPos );

    size_t size() const { return maList.size(); }
};

/// Parser for *.xcu files
class CfgParser
{
protected:
    OString sCurrentResTyp;
    OString sCurrentIsoLang;
    OString sCurrentText;

    OString sLastWhitespace;

    CfgStack aStack;
    CfgStackData *pStackData;

    bool bLocalize;

    virtual void WorkOnText(
        OString &rText,
        const OString &rLangIndex )=0;

    virtual void WorkOnResourceEnd()=0;

    virtual void Output(const OString & rOutput)=0;

    static void Error(const OString &rError);

private:
    int ExecuteAnalyzedToken( int nToken, char *pToken );
    void AddText(
        OString &rText,
        const OString &rIsoLang,
        const OString &rResTyp );

    static bool IsTokenClosed(const OString &rToken);

public:
    CfgParser();
    virtual ~CfgParser();

    int Execute( int nToken, char * pToken );
};

/// Export strings from *.xcu files
class CfgExport : public CfgParser
{
private:
    OString sPath;
    PoOfstream pOutputStream;

protected:
    virtual void WorkOnText(
        OString &rText,
        const OString &rIsoLang
        ) override;

    void WorkOnResourceEnd() override;
    void Output(const OString& rOutput) override;
public:
    CfgExport(
        const OString &rOutputFile,
        const OString &rFilePath
    );
    virtual ~CfgExport();
};

/// Merge strings to *.xcu files
class CfgMerge : public CfgParser
{
private:
    MergeDataFile *pMergeDataFile;
    std::vector<OString> aLanguages;
    std::unique_ptr<ResData> pResData;

    OString sFilename;
    bool bEnglish;

    std::ofstream pOutputStream;

protected:
    virtual void WorkOnText(OString &rText, const OString &rLangIndex) override;

    void WorkOnResourceEnd() override;

    void Output(const OString& rOutput) override;
public:
    CfgMerge(
        const OString &rMergeSource, const OString &rOutputFile,
        const OString &rFilename, const OString &rLanguage );
    virtual ~CfgMerge();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
