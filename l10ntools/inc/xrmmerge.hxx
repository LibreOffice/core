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

