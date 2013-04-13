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
    OString sGID;
    OString sLID;

    sal_Bool bError;
    sal_Bool bText;

    OString sCurrentOpenTag;
    OString sCurrentCloseTag;
    OString sCurrentText;
protected:
    std::vector<OString> aLanguages;
    OString GetAttribute( const OString &rToken, const OString &rAttribute );
    void Error( const OString &rError );

    virtual void Output( const OString& rOutput )=0;
    virtual void WorkOnDesc(
        const OString &rOpenTag,
        OString &rText
    )=0;
    virtual void WorkOnText(
        const OString &rOpenTag,
        OString &rText
    )=0;
    virtual void EndOfText(
        const OString &rOpenTag,
        const OString &rCloseTag
    )=0;

    OString GetGID() { return sGID; }
    OString GetLID() { return sLID; }

public:
    XRMResParser();
    virtual ~XRMResParser();

    int Execute( int nToken, char * pToken );

    void SetError( sal_Bool bErr = sal_True ) { bError = bErr; }
    sal_Bool GetError() { return bError; }
};

//
// XRMResExport
//

class XRMResExport : public XRMResParser
{
private:
    ResData *pResData;
    OString sPath;
    PoOfstream pOutputStream;
protected:
    void WorkOnDesc(
        const OString &rOpenTag,
        OString &rText
    );
    void WorkOnText(
        const OString &rOpenTag,
        OString &rText
    );
    void EndOfText(
        const OString &rOpenTag,
        const OString &rCloseTag
    );
    void Output( const OString& rOutput );

public:
    XRMResExport(
        const OString &rOutputFile,
        const OString &rFilePath
    );
    virtual ~XRMResExport();
};

//
// class XRMResMerge
//

class XRMResMerge : public XRMResParser
{
private:
    MergeDataFile *pMergeDataFile;
    OString sFilename;
    ResData *pResData;
    std::ofstream pOutputStream;

protected:
    void WorkOnDesc(
        const OString &rOpenTag,
        OString &rText
    );
    void WorkOnText(
        const OString &rOpenTag,
        OString &rText
    );
    void EndOfText(
        const OString &rOpenTag,
        const OString &rCloseTag
    );
    void Output( const OString& rOutput );
public:
    XRMResMerge(
        const OString &rMergeSource,
        const OString &rOutputFile,
        const OString &rFilename
    );
    virtual ~XRMResMerge();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
