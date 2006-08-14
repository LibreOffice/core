/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgmerge.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:07:26 $
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

#ifndef _CFG_MERGE_HXX
#define _CFG_MERGE_HXX

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <hash_map>

typedef std::hash_map<ByteString , ByteString , hashByteString,equalByteString>
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

DECLARE_LIST( CfgStackList, CfgStackData * )

class CfgStack : public CfgStackList
{
public:
    CfgStack() : CfgStackList( 10, 10 ) {}
    ~CfgStack();

    ULONG Push( CfgStackData *pStackData );
    CfgStackData *Push( const ByteString &rTag, const ByteString &rId );
    CfgStackData *Pop() { return Remove( Count() - 1 ); }

    CfgStackData *GetStackData( ULONG nPos = LIST_APPEND );

    ByteString GetAccessPath( ULONG nPos = LIST_APPEND );
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
