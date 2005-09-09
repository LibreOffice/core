/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgmerge.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:54:49 $
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

/*struct equalByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
    }
};

struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
                std::hash< const char* > myHash;
                return myHash( rName.GetBuffer() );
    }
};
*/
typedef std::hash_map<ByteString , ByteString , hashByteString,equalByteString>
                                ByteStringHashMap;

//=============================================

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

    //ByteString sText[ LANGUAGES ];
    ByteStringHashMap sText;
public:
    CfgStackData( const ByteString &rTag, const ByteString &rId )
            : sTagType( rTag ), sIdentifier( rId ) {};

    ByteString &GetTagType() { return sTagType; }
    ByteString &GetIdentifier() { return sIdentifier; }

    void FillInFallbacks();
};

//
// class CfgStack
//

DECLARE_LIST( CfgStackList, CfgStackData * );

class CfgStack : public CfgStackList
{
public:
    CfgStack() : CfgStackList( 10, 10 ) {}
    ~CfgStack();

    ULONG Push( CfgStackData *pStackData ) { Insert( pStackData, LIST_APPEND ); return Count() - 1; }
    CfgStackData *Push( const ByteString &rTag, const ByteString &rId )
        { CfgStackData *pD = new CfgStackData( rTag, rId ); Insert( pD, LIST_APPEND ); return pD; }
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
        //USHORT nLangIndex,
        ByteString nLangIndex,
        const ByteString &rResTyp );

    virtual void WorkOnRessourceEnd();

    virtual void Output( const ByteString& rOutput );

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
    virtual void WorkOnText(
        ByteString &rText,
        USHORT nLangIndex,
        const ByteString &rResTyp );

    virtual void WorkOnRessourceEnd();

public:
    CfgExport(
        const ByteString &rOutputFile,
        const ByteString &rProject,
        const ByteString &rFilePath
    );
    virtual ~CfgExport();
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
    BOOL bEnglish;

protected:
    virtual void WorkOnText(
        ByteString &rText,
        //USHORT nLangIndex,
        ByteString nLangIndex,
        const ByteString &rResTyp );

    virtual void WorkOnRessourceEnd();

    virtual void Output( const ByteString& rOutput );
public:
    CfgMerge(
        const ByteString &rMergeSource,
        const ByteString &rOutputFile,
        BOOL bErrorLog
    );
    virtual ~CfgMerge();
};

#endif
