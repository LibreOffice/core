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

// global includes
#include <tools/string.hxx>

//
// XRMResParser
//

class XRMResParser
{
private:
    ByteString sGID;
    ByteString sLID;

    sal_Bool bError;
    sal_Bool bText;

    bool sLocalized;

    ByteString sCurrentOpenTag;
    ByteString sCurrentCloseTag;
    ByteString sCurrentText;
    std::vector<ByteString> aLanguages;

protected:
    ByteString GetAttribute( const ByteString &rToken, const ByteString &rAttribute );
    void Error( const ByteString &rError );

    virtual void Output( const ByteString& rOutput )=0;
    virtual void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    )=0;
    virtual void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    )=0;

    ByteString GetGID() { return sGID; }
    ByteString GetLID() { return sLID; }

    void ConvertStringToDBFormat( ByteString &rString );
    void ConvertStringToXMLFormat( ByteString &rString );

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
    std::vector<ByteString> aLanguages;
protected:
    SvFileStream *pOutputStream;
public:
    XRMResOutputParser ( const ByteString &rOutputFile );
    virtual ~XRMResOutputParser();
};

//
// XRMResExport
//

class XRMResExport : public XRMResOutputParser
{
private:
    ResData *pResData;
    ByteString sPrj;
    ByteString sPath;
    std::vector<ByteString> aLanguages;

protected:
    void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    );
    void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    );
    void Output( const ByteString& rOutput );

public:
    XRMResExport(
        const ByteString &rOutputFile,
        const ByteString &rProject,
        const ByteString &rFilePath
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
    ByteString sFilename;
    ResData *pResData;
    std::vector<ByteString> aLanguages;

protected:
    void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    );
    void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    );
    void Output( const ByteString& rOutput );
public:
    XRMResMerge(
        const ByteString &rMergeSource,
        const ByteString &rOutputFile,
        ByteString &rFilename
    );
    virtual ~XRMResMerge();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
