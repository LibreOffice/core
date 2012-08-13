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
