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

#ifndef _EXPORT_HXX
#define _EXPORT_HXX

#ifndef L10NTOOLS_DIRECTORY_HXX
#define L10NTOOLS_DIRECTORY_HXX
#include <l10ntools/directory.hxx>
#endif

#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <osl/file.hxx>
#include <osl/file.h>

#include <boost/unordered_map.hpp>
#include <iterator> /* std::iterator*/
#include <set>      /* std::set*/
#include <vector>   /* std::vector*/
#include <queue>
#include <string>

#include <unistd.h>
#ifdef WNT
#include <direct.h>
#endif

#define NO_TRANSLATE_ISO        "x-no-translate"

#define JAPANESE_ISO "ja"


struct eqstr{
  sal_Bool operator()(const char* s1, const char* s2) const{
    return strcmp(s1,s2)==0;
  }
};

struct equalByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
    }
};
struct lessByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
    }
};

struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
        return rtl_str_hashCode_WithLength(rName.GetBuffer(), rName.Len() );
    }
};

class PFormEntrys;
class MergeData;
typedef std::set<ByteString , lessByteString > ByteStringSet;

typedef boost::unordered_map<ByteString , ByteString , hashByteString,equalByteString>
                                ByteStringHashMap;

typedef boost::unordered_map<ByteString , bool , hashByteString,equalByteString>
                                ByteStringBoolHashMap;

typedef boost::unordered_map<ByteString , PFormEntrys* , hashByteString,equalByteString>
                                PFormEntrysHashMap;

typedef boost::unordered_map<ByteString , MergeData* , hashByteString,equalByteString>
                                MergeDataHashMap;

#define SOURCE_LANGUAGE ByteString("en-US")
#define LIST_REFID  "LIST_REFID"

typedef ByteStringHashMap ExportListEntry;
typedef ::std::vector< ExportListEntry* > ExportListBase;

//
// class ExportList
//

class ExportList
{
private:
    ExportListBase maList;
    sal_uLong nSourceLanguageListEntryCount;

public:
    ExportList() { nSourceLanguageListEntryCount = 0; }
    sal_uLong GetSourceLanguageListEntryCount() { return nSourceLanguageListEntryCount; }
    void NewSourceLanguageListEntry() { nSourceLanguageListEntryCount++; }
    size_t size() const { return maList.size(); }
    void push_back( ExportListEntry* item ) { maList.push_back( item ); }

    ExportListEntry* operator [] ( size_t i )
        {
            return ( i < maList.size() ) ? maList[ i ] : NULL;
        }
};

#define REFID_NONE 0xFFFF

//
// struct ResData
//

/******************************************************************************
* Purpose: holds mandatory data to export a single res (used with ResStack)
******************************************************************************/

#define ID_LEVEL_NULL       0x0000
#define ID_LEVEL_AUTOID     0x0001
#define ID_LEVEL_TEXT       0x0002
#define ID_LEVEL_FIELDNAME  0x0003
#define ID_LEVEL_ACCESSPATH 0x0004
#define ID_LEVEL_IDENTIFIER 0x0005
#define ID_LEVEL_LISTINDEX  0x0006

class ResData
{
public:
    ~ResData();
    sal_Bool SetId( const ByteString &rId, sal_uInt16 nLevel );

    sal_uInt16 nWidth;
    sal_uInt16 nChildIndex;
    sal_uInt16 nIdLevel;
    sal_Bool bChild;
    sal_Bool bChildWithText;

    sal_Bool bText;
    sal_Bool bHelpText;
    sal_Bool bQuickHelpText;
    sal_Bool bTitle;
    sal_Bool bList;

    sal_Bool bRestMerged;

    ByteString sResTyp;
    ByteString sId;
    ByteString sGId;
    ByteString sHelpId;
    ByteString sFilename;

    ByteStringHashMap sText;
    sal_uInt16 nTextRefId;

    ByteStringHashMap sHelpText;
    sal_uInt16 nHelpTextRefId;

    ByteStringHashMap sQuickHelpText;
    sal_uInt16 nQuickHelpTextRefId;

    ByteStringHashMap sTitle;
    sal_uInt16 nTitleRefId;

    ByteString sTextTyp;
    ByteStringHashMap aFallbackData;
    ByteStringHashMap aMergedLanguages;

    ExportList  *pStringList;
    ExportList  *pUIEntries;
    ExportList  *pItemList;
    ExportList  *pFilterList;
    ExportList  *pPairedList;

    ByteString sPForm;

    void Dump();
    void addFallbackData( ByteString& sId , const ByteString& sText );
    bool getFallbackData( ByteString& sId , ByteString& sText);

    void addMergedLanguage( ByteString& sLang );
    bool isMerged( ByteString& sLang );
    ResData( const ByteString &rPF, const ByteString &rGId )
            :
            nWidth( 0 ),
            nChildIndex( 0 ),
            nIdLevel( ID_LEVEL_NULL ),
            bChild( sal_False ),
            bChildWithText( sal_False ),
            bText( sal_False ),
            bHelpText( sal_False ),
            bQuickHelpText( sal_False ),
            bTitle( sal_False ),
            bList( sal_False ),
            bRestMerged( sal_False ),
            sGId( rGId ),
            nTextRefId( REFID_NONE ),
            nHelpTextRefId( REFID_NONE ),
            nQuickHelpTextRefId( REFID_NONE ),
            nTitleRefId( REFID_NONE ),
            sTextTyp( "Text" ),
            pStringList( NULL ),
            pUIEntries( NULL ),
            pItemList( NULL ),
            pFilterList( NULL ),
            pPairedList( NULL ),
            sPForm( rPF )
    {
        sGId.EraseAllChars( '\r' );
        sPForm.EraseAllChars( '\r' );
    };
    ResData( const ByteString &rPF, const ByteString &rGId , const ByteString &rFilename )
            :
            nChildIndex( 0 ),
            nIdLevel( ID_LEVEL_NULL ),
            bChild( sal_False ),
            bChildWithText( sal_False ),
            bText( sal_False ),
            bHelpText( sal_False ),
            bQuickHelpText( sal_False ),
            bTitle( sal_False ),
            bList( sal_False ),
            bRestMerged( sal_False ),
            sGId( rGId ),
            sFilename( rFilename ),
            nTextRefId( REFID_NONE ),
            nHelpTextRefId( REFID_NONE ),
            nQuickHelpTextRefId( REFID_NONE ),
            nTitleRefId( REFID_NONE ),
            sTextTyp( "Text" ),
            pStringList( NULL ),
            pUIEntries( NULL ),
            pItemList( NULL ),
            pFilterList( NULL ),
            pPairedList( NULL ),
            sPForm( rPF )

    {
        sGId.EraseAllChars( '\r' );
        sPForm.EraseAllChars( '\r' );
    };


};


//
// class Export
//

/******************************************************************************
* Purpose: syntax check and export of *.src, called from lexer
******************************************************************************/

#define LIST_NON                    0x0000
#define LIST_STRING                 0x0001
#define LIST_FILTER                 0x0002
#define LIST_ITEM                   0x0004
#define LIST_PAIRED                 0x0005
#define LIST_UIENTRIES              0x0008
#define STRING_TYP_TEXT             0x0010
#define STRING_TYP_HELPTEXT         0x0020
#define STRING_TYP_QUICKHELPTEXT    0x0040
#define STRING_TYP_TITLE            0x0080

#define MERGE_MODE_NORMAL           0x0000
#define MERGE_MODE_LIST             0x0001

typedef ::std::vector< ResData* > ResStack;
// forwards
class WordTransformer;
class ParserQueue;

class Export
{
private:
    WordTransformer *pWordTransformer;

    CharSet aCharSet;                   // used charset in src

    SvFileStream aOutput;

    ResStack aResStack;                 // stack for parsing recursive

    ByteString sActPForm;               // hold cur. system

    sal_Bool bDefine;                       // cur. res. in a define?
    sal_Bool bNextMustBeDefineEOL;          // define but no \ at lineend
    sal_uLong nLevel;                       // res. recursiv? how deep?
    sal_uInt16 nList;                       // cur. res. is String- or FilterList
    ByteString nListLang;
    sal_uLong nListIndex;
    sal_uLong nListLevel;
    bool bSkipFile;
    ByteString sProject;
    ByteString sRoot;
    sal_Bool bEnableExport;
    sal_Bool bMergeMode;
    ByteString sMergeSrc;
    ByteString sLastListLine;
    sal_Bool bError;                        // any errors while export?
    sal_Bool bReadOver;
    sal_Bool bDontWriteOutput;
    ByteString sLastTextTyp;
    static bool isInitialized;
    ByteString sFilename;


public:
    ParserQueue* pParseQueue; // public ?
    static ByteString sLanguages; // public ?
    static ByteString sForcedLanguages; // public ?


    static bool skipProject( ByteString sPrj ) ;
    static void InitLanguages( bool bMergeMode = false );
    static void InitForcedLanguages( bool bMergeMode = false );
    static std::vector<ByteString> GetLanguages();
    static std::vector<ByteString> GetForcedLanguages();

    static void SetLanguages( std::vector<ByteString> val );
    static void RemoveUTF8ByteOrderMarker( ByteString &rString );
    static bool hasUTF8ByteOrderMarker( const ByteString &rString );
    static void RemoveUTF8ByteOrderMarkerFromFile( const ByteString &rFilename );
    static bool fileHasUTF8ByteOrderMarker( const ByteString &rString );
    static ByteString GetIsoLangByIndex( sal_uInt16 nIndex );
    static void QuotHTML( ByteString &rString );
    static bool CopyFile( const ByteString& source , const ByteString& dest );

    static void QuotHTMLXRM( ByteString &rString );
    static void UnquotHTML( ByteString &rString );

    static const char* GetEnv( const char *pVar );
    static int getCurrentDirectory( rtl::OUString& base_fqurl , rtl::OUString& base );

    static bool isSourceLanguage( const ByteString &sLanguage );
    static bool isAllowed( const ByteString &sLanguage );

    static bool LanguageAllowed( const ByteString &nLanguage );
    static void Languages( std::vector<ByteString>::const_iterator& begin , std::vector<ByteString>::const_iterator& end );
    static void getRandomName( const ByteString& sPrefix , ByteString& sRandStr , const ByteString& sPostfix  );
    static void getRandomName( ByteString& sRandStr );
    static void getCurrentDir( std::string& dir );

    static void replaceEncoding( ByteString& rString );

    static ByteString GetFallbackLanguage( const ByteString nLanguage );
    static void FillInFallbacks( ResData *pResData );
    static void FillInListFallbacks( ExportList *pList, const ByteString &nSource, const ByteString &nFallback );
    static ByteString GetTimeStamp();
    static sal_Bool ConvertLineEnds( ByteString sSource, ByteString sDestination );
    static ByteString GetNativeFile( ByteString sSource );
    static DirEntry GetTempFile();

    static void DumpExportList( ByteString& sListName , ExportList& aList );
    static ByteString DumpMap( ByteString& sMapName , ByteStringHashMap& aMap );

private:
    static std::vector<ByteString> aLanguages;
    static std::vector<ByteString> aForcedLanguages;

    sal_Bool ListExists( ResData *pResData, sal_uInt16 nLst );

    sal_Bool WriteData( ResData *pResData, sal_Bool bCreateNew = sal_False );// called befor dest. cur ResData
    sal_Bool WriteExportList( ResData *pResData, ExportList *pExportList,
                        const ByteString &rTyp, sal_Bool bCreateNew = sal_False );

    ByteString MergePairedList( ByteString& sLine , ByteString& sText );

    ByteString FullId();                    // creates cur. GID

    bool PairedListFallback( ByteString& sText , ResData& aResData );

    ByteString GetPairedListID      ( const ByteString& sText );
    ByteString GetPairedListString  ( const ByteString& sText );
    ByteString StripList    ( const ByteString& sText );

    void UnmergeUTF8( ByteString& sOrig );
    void InsertListEntry( const ByteString &rText, const ByteString &rLine );
    void CleanValue( ByteString &rValue );
    ByteString GetText( const ByteString &rSource, int nToken );

    sal_Bool PrepareTextToMerge( ByteString &rText, sal_uInt16 nTyp,
        ByteString &nLangIndex, ResData *pResData );

    void MergeRest( ResData *pResData, sal_uInt16 nMode = MERGE_MODE_NORMAL );
    void ConvertMergeContent( ByteString &rText );

      void WriteToMerged( const ByteString &rText , bool bSDFContent );
    void SetChildWithText();

    void CutComment( ByteString &rText );

public:
    Export( const ByteString &rOutput, sal_Bool bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot , const ByteString& rFile );
    Export( const ByteString &rOutput, sal_Bool bWrite,
            const ByteString &rPrj, const ByteString &rPrjRoot,
            const ByteString &rMergeSource , const ByteString& rFile );
    ~Export();

    void Init();
    int Execute( int nToken, const char * pToken ); // called from lexer
    void SetError() { bError = sal_True; }
    sal_Bool GetError() { return bError; }
};


//
// class PFormEntrys
//

/******************************************************************************
* Purpose: holds information of data to merge (one pform)
******************************************************************************/

class PFormEntrys : public ByteString
{
friend class MergeDataFile;
private:
    ByteString sHelpText; // empty string
    ByteStringHashMap sText;
    ByteStringBoolHashMap bTextFirst;
    ByteStringHashMap sQuickHelpText;
    ByteStringBoolHashMap bQuickHelpTextFirst;
    ByteStringHashMap sTitle;
    ByteStringBoolHashMap bTitleFirst;

public:
    PFormEntrys( const ByteString &rPForm ) : ByteString( rPForm ) {};
    ByteString Dump();
    void InsertEntry(
                    const ByteString &nId ,
                    const ByteString &rText,
                    const ByteString &rQuickHelpText,
                    const ByteString &rTitle
                    )
        {

            sText[ nId ] = rText;
            bTextFirst[ nId ] = true;
            sQuickHelpText[ nId ] = rQuickHelpText;
            bQuickHelpTextFirst[ nId ] = true;
            sTitle[ nId ] = rTitle;
            bTitleFirst[ nId ] = true;
        }
     sal_Bool GetText( ByteString &rReturn, sal_uInt16 nTyp, const ByteString &nLangIndex, sal_Bool bDel = sal_False );
     sal_Bool GetTransex3Text( ByteString &rReturn, sal_uInt16 nTyp, const ByteString &nLangIndex, sal_Bool bDel = sal_False );

};

//
// class MergeData
//

/******************************************************************************
* Purpose: holds information of data to merge (one ressource)
******************************************************************************/

class MergeDataFile;

class MergeData
{
friend class MergeDataFile;
private:
    ByteString sTyp;
    ByteString sGID;
    ByteString sLID;
    ByteString sFilename;
    PFormEntrysHashMap aMap;
public:
    MergeData( const ByteString &rTyp, const ByteString &rGID, const ByteString &rLID , const ByteString &rFilename )
            : sTyp( rTyp ), sGID( rGID ), sLID( rLID ) , sFilename( rFilename ) {};
    ~MergeData();
    PFormEntrys* InsertEntry( const ByteString &rPForm );
    PFormEntrys* GetPFormEntrys( ResData *pResData );

    void Insert( const ByteString& rPFO , PFormEntrys* pfEntrys );
    PFormEntrys* GetPFObject( const ByteString& rPFO );

    ByteString Dump();
    sal_Bool operator==( ResData *pData );
};

//
// class MergeDataFile
//

/******************************************************************************
* Purpose: holds information of data to merge
******************************************************************************/

class MergeDataFile
{
    private:
        sal_Bool bErrorLog;
        ByteString sErrorLog;
        SvFileStream aErrLog;
    MergeDataHashMap aMap;
        std::set<ByteString> aLanguageSet;

        MergeData *GetMergeData( ResData *pResData , bool bCaseSensitve = false );
        void InsertEntry( const ByteString &rTYP, const ByteString &rGID, const ByteString &rLID,
            const ByteString &rPFO,
            const ByteString &nLang, const ByteString &rTEXT,
            const ByteString &rQHTEXT, const ByteString &rTITLE,
            const ByteString &sFilename, bool bCaseSensitive
            );
        ByteString Dump();
        void WriteError( const ByteString &rLine );

    public:
        MergeDataFile( const ByteString &rFileName, const ByteString& rFile , sal_Bool bErrLog, CharSet aCharSet, bool bCaseSensitive = false );
        ~MergeDataFile();


        std::vector<ByteString> GetLanguages();

        PFormEntrys *GetPFormEntrys( ResData *pResData );
        PFormEntrys *GetPFormEntrysCaseSensitive( ResData *pResData );

        static ByteString CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID , const ByteString& rFilename , bool bCaseSensitive = false );
};


class QueueEntry
{
public:
    QueueEntry( int nTypVal , ByteString sLineVal ): nTyp( nTypVal ) , sLine( sLineVal ){};
    int nTyp;
    ByteString sLine;
};

class ParserQueue
{
public:

    ParserQueue( Export& aExportObj );
    ~ParserQueue();

    inline void Push( const QueueEntry& aEntry );
    bool bCurrentIsM;  // public ?
    bool bNextIsM;   // public ?
    bool bLastWasM;   // public ?
    bool bMflag;   // public ?

    void Close();
private:
    // Future / Next
    std::queue<QueueEntry>* aQueueNext;
    // Current
    std::queue<QueueEntry>* aQueueCur;
    // Ref
    std::queue<QueueEntry>* aQref;

    Export& aExport;
    bool bStart;
    bool bStartNext;

    inline void Pop( std::queue<QueueEntry>& aQueue );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
