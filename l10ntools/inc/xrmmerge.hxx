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

#include "sal/config.h"

#include <fstream>

//
// XRMResParser
//

class XRMResParser
{
private:
    rtl::OString sGID;
    rtl::OString sLID;

    sal_Bool bError;
    sal_Bool bText;

    rtl::OString sCurrentOpenTag;
    rtl::OString sCurrentCloseTag;
    rtl::OString sCurrentText;
    std::vector<rtl::OString> aLanguages;

protected:
    rtl::OString GetAttribute( const rtl::OString &rToken, const rtl::OString &rAttribute );
    void Error( const rtl::OString &rError );

    virtual void Output( const rtl::OString& rOutput )=0;
    virtual void WorkOnDesc(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    )=0;
    virtual void WorkOnText(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    )=0;
    virtual void EndOfText(
        const rtl::OString &rOpenTag,
        const rtl::OString &rCloseTag
    )=0;

    rtl::OString GetGID() { return sGID; }
    rtl::OString GetLID() { return sLID; }

    void ConvertStringToDBFormat( rtl::OString &rString );
    void ConvertStringToXMLFormat( rtl::OString &rString );

public:
    XRMResParser();
    virtual ~XRMResParser();

    int Execute( int nToken, char * pToken );

    void SetError( sal_Bool bErr = sal_True ) { bError = bErr; }
    sal_Bool GetError() { return bError; }
};

//
// class XRMResOutputParser
//

class XRMResOutputParser : public XRMResParser
{
private:
    std::vector<rtl::OString> aLanguages;
protected:
    std::ofstream pOutputStream;
public:
    XRMResOutputParser ( const rtl::OString &rOutputFile );
    virtual ~XRMResOutputParser();
};

//
// XRMResExport
//

class XRMResExport : public XRMResOutputParser
{
private:
    ResData *pResData;
    rtl::OString sPrj;
    rtl::OString sPath;
    std::vector<rtl::OString> aLanguages;

protected:
    void WorkOnDesc(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    );
    void WorkOnText(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    );
    void EndOfText(
        const rtl::OString &rOpenTag,
        const rtl::OString &rCloseTag
    );
    void Output( const rtl::OString& rOutput );

public:
    XRMResExport(
        const rtl::OString &rOutputFile,
        const rtl::OString &rProject,
        const rtl::OString &rFilePath
    );
    virtual ~XRMResExport();
};

//
// class XRMResMerge
//

class XRMResMerge : public XRMResOutputParser
{
private:
    MergeDataFile *pMergeDataFile;
    rtl::OString sFilename;
    ResData *pResData;
    std::vector<rtl::OString> aLanguages;

protected:
    void WorkOnDesc(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    );
    void WorkOnText(
        const rtl::OString &rOpenTag,
        rtl::OString &rText
    );
    void EndOfText(
        const rtl::OString &rOpenTag,
        const rtl::OString &rCloseTag
    );
    void Output( const rtl::OString& rOutput );
public:
    XRMResMerge(
        const rtl::OString &rMergeSource,
        const rtl::OString &rOutputFile,
        const rtl::OString &rFilename
    );
    virtual ~XRMResMerge();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
