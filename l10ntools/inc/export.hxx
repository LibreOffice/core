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

#pragma once
#if 1

#include "sal/config.h"
#include "po.hxx"

#include <cstddef>
#include <fstream>

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

class PFormEntrys;
class MergeData;

typedef boost::unordered_map<rtl::OString, rtl::OString, rtl::OStringHash>
    OStringHashMap;

typedef boost::unordered_map<rtl::OString, bool, rtl::OStringHash>
    OStringBoolHashMap;

typedef boost::unordered_map<rtl::OString, PFormEntrys*, rtl::OStringHash>
    PFormEntrysHashMap;

typedef boost::unordered_map<rtl::OString, MergeData*, rtl::OStringHash>
    MergeDataHashMap;

#define SOURCE_LANGUAGE "en-US"
#define X_COMMENT "x-comment"
#define LIST_REFID  "LIST_REFID"

typedef OStringHashMap ExportListEntry;
typedef ::std::vector< ExportListEntry* > ExportListBase;

//
// class ExportList
//

class ExportList
{
private:
    ExportListBase maList;
    std::size_t nSourceLanguageListEntryCount;

public:
    ExportList() { nSourceLanguageListEntryCount = 0; }
    std::size_t GetSourceLanguageListEntryCount() { return nSourceLanguageListEntryCount; }
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
    sal_Bool SetId(const rtl::OString &rId, sal_uInt16 nLevel);

    sal_Int32 nWidth;
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

    rtl::OString sResTyp;
    rtl::OString sId;
    rtl::OString sGId;
    rtl::OString sHelpId;
    rtl::OString sFilename;

    OStringHashMap sText;
    sal_uInt16 nTextRefId;

    OStringHashMap sHelpText;
    sal_uInt16 nHelpTextRefId;

    OStringHashMap sQuickHelpText;
    sal_uInt16 nQuickHelpTextRefId;

    OStringHashMap sTitle;
    sal_uInt16 nTitleRefId;

    rtl::OString sTextTyp;

    ExportList  *pStringList;
    ExportList  *pUIEntries;
    ExportList  *pItemList;
    ExportList  *pFilterList;
    ExportList  *pPairedList;

    rtl::OString sPForm;

    ResData(const rtl::OString &rPF, const rtl::OString &rGId)
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
        sGId = sGId.replaceAll("\r", rtl::OString());
        sPForm = sPForm.replaceAll("\r", rtl::OString());
    }

    ResData(const rtl::OString &rPF, const rtl::OString &rGId , const rtl::OString &rFilename)
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
        sGId = sGId.replaceAll("\r", rtl::OString());
        sPForm = sPForm.replaceAll("\r", rtl::OString());
    }
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

//result type of handleArguments()
struct HandledArgs
{
    OString m_sPrj;
    OString m_sPrjRoot;
    OString m_sInputFile;
    OString m_sOutputFile;
    OString m_sMergeSrc;
    OString m_sLanguages;
    bool m_bMergeMode;
    HandledArgs()
        : m_sPrj( OString() )
        , m_sPrjRoot( OString() )
        , m_sInputFile( OString() )
        , m_sOutputFile( OString() )
        , m_sMergeSrc( OString() )
        , m_sLanguages( OString() )
        , m_bMergeMode( false )
    {}
};

class Export
{
private:
    WordTransformer *pWordTransformer;

    union
    {
        std::ofstream* mSimple;
        PoOfstream* mPo;

    } aOutput;

    ResStack aResStack;                 // stack for parsing recursive

    rtl::OString sActPForm;               // hold cur. system

    sal_Bool bDefine;                       // cur. res. in a define?
    sal_Bool bNextMustBeDefineEOL;          // define but no \ at lineend
    std::size_t nLevel; // res. recursiv? how deep?
    sal_uInt16 nList;                       // cur. res. is String- or FilterList
    rtl::OString m_sListLang;
    std::size_t nListIndex;
    std::size_t nListLevel;
    bool bSkipFile;
    rtl::OString sProject;
    rtl::OString sRoot;
    sal_Bool bMergeMode;
    rtl::OString sMergeSrc;
    rtl::OString sLastListLine;
    sal_Bool bError;                        // any errors while export?
    sal_Bool bReadOver;
    sal_Bool bDontWriteOutput;
    rtl::OString sLastTextTyp;
    static bool isInitialized;
    rtl::OString sFilename;


public:
    ParserQueue* pParseQueue; // public ?
    static rtl::OString sLanguages; // public ?

    static bool handleArguments(int argc, char * argv[], HandledArgs& o_aHandledArgs);
    static void writeUsage(const OString& rName, const OString& rFileType);
    static void writePoEntry(const OString& rExecutable, PoOfstream& rPoStream, const OString& rSourceFile,
                             const OString& rResType, const OString& rGroupId, const OString& rLocalId,
                             const OString& rHelpText, const OString& rText,
                             const PoEntry::TYPE eType = PoEntry::TTEXT);

    static void InitLanguages( bool bMergeMode = false );
    static void InitForcedLanguages( bool bMergeMode = false );
    static std::vector<rtl::OString> GetLanguages();

    static void SetLanguages( std::vector<rtl::OString> val );
    static void RemoveUTF8ByteOrderMarker( rtl::OString &rString );
    static bool hasUTF8ByteOrderMarker( const rtl::OString &rString );
    static rtl::OString QuoteHTML( rtl::OString const &rString );

    static rtl::OString UnquoteHTML( rtl::OString const &rString );

    static bool isSourceLanguage(const rtl::OString &rLanguage);
    static bool isAllowed(const rtl::OString &rLanguage);

    static void Languages( std::vector<rtl::OString>::const_iterator& begin , std::vector<rtl::OString>::const_iterator& end );

private:
    static std::vector<rtl::OString> aLanguages;
    static std::vector<rtl::OString> aForcedLanguages;

    sal_Bool WriteData( ResData *pResData, sal_Bool bCreateNew = sal_False );// called befor dest. cur ResData
    sal_Bool WriteExportList( ResData *pResData, ExportList *pExportList,
                        const rtl::OString &rTyp, sal_Bool bCreateNew = sal_False );

    rtl::OString MergePairedList( rtl::OString const & sLine , rtl::OString const & sText );

    rtl::OString FullId();                    // creates cur. GID

    rtl::OString GetPairedListID(const rtl::OString & rText);
    rtl::OString GetPairedListString(const rtl::OString& rText);
    rtl::OString StripList(const rtl::OString& rText);

    void InsertListEntry(const rtl::OString &rText, const rtl::OString &rLine);
    void CleanValue( rtl::OString &rValue );
    rtl::OString GetText(const rtl::OString &rSource, int nToken);

    sal_Bool PrepareTextToMerge(rtl::OString &rText, sal_uInt16 nTyp,
        rtl::OString &rLangIndex, ResData *pResData);
    void ResData2Output( PFormEntrys *pEntry, sal_uInt16 nType, const rtl::OString& rTextType );
    void MergeRest( ResData *pResData, sal_uInt16 nMode = MERGE_MODE_NORMAL );
    void ConvertMergeContent( rtl::OString &rText );

    void WriteToMerged(const rtl::OString &rText , bool bSDFContent);
    void SetChildWithText();

    void CutComment( rtl::OString &rText );

public:
    Export(const rtl::OString &rOutput);
    Export(const rtl::OString &rMergeSource, const rtl::OString &rOutput);
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

class PFormEntrys
{
friend class MergeDataFile;
private:
    rtl::OString data_; //TODO
    rtl::OString sHelpText; // empty string
    OStringHashMap sText;
    OStringBoolHashMap bTextFirst;
    OStringHashMap sQuickHelpText;
    OStringBoolHashMap bQuickHelpTextFirst;
    OStringHashMap sTitle;
    OStringBoolHashMap bTitleFirst;

public:
    PFormEntrys( const rtl::OString &rPForm ) : data_( rPForm ) {};
    void InsertEntry(const rtl::OString &rId, const rtl::OString &rText,
        const rtl::OString &rQuickHelpText, const rtl::OString &rTitle)
    {

        sText[ rId ] = rText;
        bTextFirst[ rId ] = true;
        sQuickHelpText[ rId ] = rQuickHelpText;
        bQuickHelpTextFirst[ rId ] = true;
        sTitle[ rId ] = rTitle;
        bTitleFirst[ rId ] = true;
    }
    sal_Bool GetText( rtl::OString &rReturn, sal_uInt16 nTyp, const rtl::OString &nLangIndex, sal_Bool bDel = sal_False );
    sal_Bool GetTransex3Text( rtl::OString &rReturn, sal_uInt16 nTyp, const rtl::OString &nLangIndex, sal_Bool bDel = sal_False );

};

//
// class MergeData
//

/******************************************************************************
* Purpose: holds information of data to merge (one resource)
******************************************************************************/

class MergeDataFile;

class MergeData
{
public:
    rtl::OString sTyp;
    rtl::OString sGID;
    rtl::OString sLID;
    rtl::OString sFilename;
    PFormEntrysHashMap aMap;
public:
    MergeData( const rtl::OString &rTyp, const rtl::OString &rGID, const rtl::OString &rLID , const rtl::OString &rFilename )
            : sTyp( rTyp ), sGID( rGID ), sLID( rLID ) , sFilename( rFilename ) {};
    ~MergeData();
    PFormEntrys* GetPFormEntries();

    void Insert( PFormEntrys* pfEntrys );
    PFormEntrys* GetPFObject( const rtl::OString &rPFO );

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
        rtl::OString sErrorLog;
        MergeDataHashMap aMap;
        std::set<rtl::OString> aLanguageSet;

        MergeData *GetMergeData( ResData *pResData , bool bCaseSensitve = false );
        void InsertEntry(const rtl::OString &rTYP, const rtl::OString &rGID,
            const rtl::OString &rLID, const rtl::OString &rPFO,
            const rtl::OString &nLang, const rtl::OString &rTEXT,
            const rtl::OString &rQHTEXT, const rtl::OString &rTITLE,
            const rtl::OString &sFilename, bool bCaseSensitive);
    public:
        explicit MergeDataFile(
            const rtl::OString &rFileName, const rtl::OString& rFile,
            bool bCaseSensitive, bool bWithQtz = true );
        ~MergeDataFile();


        std::vector<rtl::OString> GetLanguages() const;
        const MergeDataHashMap& getMap() const { return aMap; }

        PFormEntrys *GetPFormEntrys( ResData *pResData );
        PFormEntrys *GetPFormEntrysCaseSensitive( ResData *pResData );

        static rtl::OString CreateKey(const rtl::OString& rTYP, const rtl::OString& rGID,
            const rtl::OString& rLID, const rtl::OString& rFilename , bool bCaseSensitive = false);
};


class QueueEntry
{
public:
    QueueEntry(int nTypVal, const rtl::OString &rLineVal)
        : nTyp(nTypVal), sLine(rLineVal)
    {
    }
    int nTyp;
    rtl::OString sLine;
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

    inline void Pop( std::queue<QueueEntry>& aQueue );

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
