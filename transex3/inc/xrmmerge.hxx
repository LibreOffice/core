/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xrmmerge.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:58:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    BOOL bError;
    BOOL bText;

    ByteString sLocalized;

    ByteString sCurrentOpenTag;
    ByteString sCurrentCloseTag;
    ByteString sCurrentText;
    std::vector<ByteString> aLanguages;

protected:
    ByteString GetAttribute( const ByteString &rToken, const ByteString &rAttribute );
    void Error( const ByteString &rError );

    virtual void Output( const ByteString& rOutput );
    virtual void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    );
    virtual void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    );

    ByteString GetGID() { return sGID; }
    ByteString GetLID() { return sLID; }

    void ConvertStringToDBFormat( ByteString &rString );
    void ConvertStringToXMLFormat( ByteString &rString );

public:
    XRMResParser();
    virtual ~XRMResParser();

    int Execute( int nToken, char * pToken );

    void SetError( BOOL bErr = TRUE ) { bError = bErr; }
    BOOL GetError() { return bError; }
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
    virtual void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    );
    virtual void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    );

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
    ResData *pResData;
    std::vector<ByteString> aLanguages;

protected:
    virtual void WorkOnText(
        const ByteString &rOpenTag,
        ByteString &rText
    );
    virtual void EndOfText(
        const ByteString &rOpenTag,
        const ByteString &rCloseTag
    );
    virtual void Output( const ByteString& rOutput );
public:
    XRMResMerge(
        const ByteString &rMergeSource,
        const ByteString &rOutputFile,
        BOOL bErrorLog
    );
    virtual ~XRMResMerge();
};

