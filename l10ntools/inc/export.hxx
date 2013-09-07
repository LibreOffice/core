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

#ifndef _EXPORT_HXX
#define _EXPORT_HXX

#include "sal/config.h"
#include "po.hxx"

#include <cstddef>
#include <fstream>

#include <osl/file.hxx>
#include <osl/file.h>

#include <boost/unordered_map.hpp>
#include <iterator>
#include <set>
#include <vector>
#include <queue>
#include <string>

#ifdef WNT
#include <direct.h>
#else
#include <unistd.h>
#endif

#define NO_TRANSLATE_ISO        "x-no-translate"

class MergeEntrys;

typedef boost::unordered_map<OString, OString, OStringHash>
    OStringHashMap;

typedef boost::unordered_map<OString, bool, OStringHash>
    OStringBoolHashMap;

#define SOURCE_LANGUAGE "en-US"
#define X_COMMENT "x-comment"
#define LIST_REFID  "LIST_REFID"

//
// class ExportList
//

typedef OStringHashMap ExportListEntry;


/// Container for parse different types of string lists
class ExportList
{
private:
    std::vector< ExportListEntry* > maList;
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
// class ResData
//

#define ID_LEVEL_NULL       0x0000
#define ID_LEVEL_AUTOID     0x0001
#define ID_LEVEL_TEXT       0x0002
#define ID_LEVEL_FIELDNAME  0x0003
#define ID_LEVEL_ACCESSPATH 0x0004
#define ID_LEVEL_IDENTIFIER 0x0005
#define ID_LEVEL_LISTINDEX  0x0006


/// Purpose: holds mandatory data to export a single res (used with ResStack)
class ResData
{
public:
    ResData( const OString &rGId );
    ResData( const OString &rGId , const OString &rFilename );
    ~ResData();
    sal_Bool SetId(const OString &rId, sal_uInt16 nLevel);

    sal_uInt16 nIdLevel;
    sal_Bool bChild;
    sal_Bool bChildWithText;

    sal_Bool bText;
    sal_Bool bHelpText;
    sal_Bool bQuickHelpText;
    sal_Bool bTitle;
    sal_Bool bList;

    OString sResTyp;
    OString sId;
    OString sGId;
    OString sHelpId;
    OString sFilename;

    OStringHashMap sText;

    OStringHashMap sHelpText;

    OStringHashMap sQuickHelpText;

    OStringHashMap sTitle;

    OString sTextTyp;

    ExportList  *pStringList;
    ExportList  *pUIEntries;
    ExportList  *pItemList;
    ExportList  *pFilterList;
    ExportList  *pPairedList;
};


//
// class Export
//

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
class ParserQueue;

/// Purpose: syntax check and export of *.src, called from lexer
class Export
{
private:
    union
    {
        std::ofstream* mSimple;
        PoOfstream* mPo;

    } aOutput;

    ResStack aResStack;                 ///< stack for parsing recursive

    sal_Bool bDefine;                       // cur. res. in a define?
    sal_Bool bNextMustBeDefineEOL;          ///< define but no \ at lineend
    std::size_t nLevel; // res. recursiv? how deep?
    sal_uInt16 nList;                       ///< cur. res. is List
    OString m_sListLang;
    std::size_t nListIndex;
    std::size_t nListLevel;
    bool bSkipFile;
    sal_Bool bMergeMode;
    OString sMergeSrc;
    sal_Bool bError;                        // any errors while export?
    sal_Bool bReadOver;
    sal_Bool bDontWriteOutput;
    OString sLastTextTyp;
    OString sFilename;
    OString sLanguages;

    std::vector<OString> aLanguages;

    sal_Bool WriteData( ResData *pResData, sal_Bool bCreateNew = sal_False ); ///< called befor dest. cur ResData
    sal_Bool WriteExportList( ResData *pResData, ExportList *pExportList,
                        const OString &rTyp, sal_Bool bCreateNew = sal_False );

    OString MergePairedList( OString const & sLine , OString const & sText );

    OString FullId();                    ///< creates cur. GID

    OString GetPairedListID(const OString & rText);
    OString GetPairedListString(const OString& rText);
    OString StripList(const OString& rText);

    void InsertListEntry(const OString &rText, const OString &rLine);
    void CleanValue( OString &rValue );
    OString GetText(const OString &rSource, int nToken);

    /**
      Get all MergeEntrys for the ExportList identified by pResData
      Check whether list can merge and load all needed MergeEntry from DataBase.
    */
    bool GetAllMergeEntrysOfList(ResData *pResData, std::vector<MergeEntrys*>& o_vMergeEntrys, ExportList*& o_pList);

    void ResData2Output( MergeEntrys *pEntry, sal_uInt16 nType, const OString& rTextType );
    void MergeRest( ResData *pResData, sal_uInt16 nMode = MERGE_MODE_NORMAL );
    void ConvertMergeContent( OString &rText );
    void ConvertExportContent( OString &rText );

    void WriteToMerged(const OString &rText , bool bSDFContent);
    void SetChildWithText();

    void CutComment( OString &rText );

    void WriteUTF8ByteOrderMarkToOutput() { *aOutput.mSimple << '\xEF' << '\xBB' << '\xBF'; }

public:
    Export( const OString &rOutput );
    Export(const OString &rMergeSource, const OString &rOutput, const OString &rLanguage, bool bUTF8BOM);
    ~Export();

    void Init();
    int Execute( int nToken, const char * pToken ); ///< called from lexer
    void SetError() { bError = sal_True; }
    sal_Bool GetError() { return bError; }
    ParserQueue* pParseQueue; // public!!
};


//
// class MergeEntrys
//

/// Purpose: holds information of data to merge
class MergeEntrys
{
friend class MergeDataFile;
private:
    OStringHashMap sText;
    OStringBoolHashMap bTextFirst;
    OStringHashMap sQuickHelpText;
    OStringBoolHashMap bQuickHelpTextFirst;
    OStringHashMap sTitle;
    OStringBoolHashMap bTitleFirst;

public:
    MergeEntrys(){};
    void InsertEntry(const OString &rId, const OString &rText,
        const OString &rQuickHelpText, const OString &rTitle)
    {

        sText[ rId ] = rText;
        bTextFirst[ rId ] = true;
        sQuickHelpText[ rId ] = rQuickHelpText;
        bQuickHelpTextFirst[ rId ] = true;
        sTitle[ rId ] = rTitle;
        bTitleFirst[ rId ] = true;
    }
    sal_Bool GetText( OString &rReturn, sal_uInt16 nTyp, const OString &nLangIndex, sal_Bool bDel = sal_False );

    /**
      Generate QTZ string with ResData
      For executable which works one language and without PO files.
    */
    static OString GetQTZText(const ResData& rResData, const OString& rOrigText);

};

//
// class MergeDataHashMap
//

class MergeData;

/** Container for MergeData

  This class is an HashMap with a hidden insertion
  order. The class can used just like a simple
  HashMap, but good to know that it's use is
  more effective if the accessing(find) order
  match with the insertion order.

  In the most case, this match is good.
  (e.g. reading PO files of different languages,
  executables merging)
*/
class MergeDataHashMap
{
    private:
        typedef boost::unordered_map<OString, MergeData*, OStringHash> HashMap_t;

    public:
        MergeDataHashMap():bFirstSearch(true){};
        ~MergeDataHashMap(){};

        typedef HashMap_t::iterator iterator;
        typedef HashMap_t::const_iterator const_iterator;

        std::pair<iterator,bool> insert(const OString& rKey, MergeData* pMergeData);
        iterator find(const OString& rKey);

        iterator begin() {return m_aHashMap.begin();}
        iterator end() {return m_aHashMap.end();}

        const_iterator begin() const {return m_aHashMap.begin();}
        const_iterator end() const {return m_aHashMap.end();}

    private:
        bool bFirstSearch;
        iterator aLastInsertion;
        iterator aLastFound;
        iterator aFirstInOrder;
        HashMap_t m_aHashMap;
};

//
// class MergeData
//

/// Purpose: holds information of data to merge (one resource)
class MergeData
{
    friend class MergeDataHashMap;

public:
    OString sTyp;
    OString sGID;
    OString sLID;
    OString sFilename;
    MergeEntrys* pMergeEntrys;
private:
    MergeDataHashMap::iterator m_aNextData;
public:
    MergeData( const OString &rTyp, const OString &rGID, const OString &rLID , const OString &rFilename );
    ~MergeData();
    MergeEntrys* GetMergeEntries();

    sal_Bool operator==( ResData *pData );
};

//
// class MergeDataFile
//

/// Purpose: holds information of data to merge, read from PO file
class MergeDataFile
{
    private:
        MergeDataHashMap aMap;
        std::set<OString> aLanguageSet;

        MergeData *GetMergeData( ResData *pResData , bool bCaseSensitve = false );
        void InsertEntry(const OString &rTYP, const OString &rGID,
            const OString &rLID, const OString &nLang,
            const OString &rTEXT, const OString &rQHTEXT,
            const OString &rTITLE, const OString &sFilename,
            bool bFirstLang, bool bCaseSensitive);
    public:
        explicit MergeDataFile(
            const OString &rFileName, const OString& rFile,
            bool bCaseSensitive, bool bWithQtz = true );
        ~MergeDataFile();


        std::vector<OString> GetLanguages() const;
        const MergeDataHashMap& getMap() const { return aMap; }

        MergeEntrys *GetMergeEntrys( ResData *pResData );
        MergeEntrys *GetMergeEntrysCaseSensitive( ResData *pResData );

        static OString CreateKey(const OString& rTYP, const OString& rGID,
            const OString& rLID, const OString& rFilename , bool bCaseSensitive = false);
};


class QueueEntry
{
public:
    QueueEntry(int nTypVal, const OString &rLineVal)
        : nTyp(nTypVal), sLine(rLineVal)
    {
    }
    int nTyp;
    OString sLine;
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
    std::queue<QueueEntry>* aQueueNext;
    std::queue<QueueEntry>* aQueueCur;
    std::queue<QueueEntry>* aQref;

    Export& aExport;
    bool bStart;

    inline void Pop( std::queue<QueueEntry>& aQueue );

};
#endif // _EXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
