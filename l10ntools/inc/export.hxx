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

#ifndef INCLUDED_L10NTOOLS_INC_EXPORT_HXX
#define INCLUDED_L10NTOOLS_INC_EXPORT_HXX

#include "sal/config.h"
#include "po.hxx"

#include <cstddef>
#include <fstream>

#include <osl/file.hxx>
#include <osl/file.h>

#include <iterator>
#include <set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <string>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define NO_TRANSLATE_ISO        "x-no-translate"

class MergeEntrys;

typedef std::unordered_map<OString, OString, OStringHash>
    OStringHashMap;

typedef std::unordered_map<OString, bool, OStringHash>
    OStringBoolHashMap;

#define SOURCE_LANGUAGE "en-US"
#define X_COMMENT "x-comment"


// class ResData


enum class IdLevel { Null=0, Text=2, Identifier=5 };

typedef std::vector< OString > ExportList;

/// Purpose: holds mandatory data to export a single res (used with ResStack)
class ResData
{
public:
    ResData( const OString &rGId );
    ResData( const OString &rGId , const OString &rFilename );
    bool SetId(const OString &rId, IdLevel nLevel);

    IdLevel nIdLevel;
    bool bChild;
    bool bChildWithText;

    bool bText;
    bool bQuickHelpText;
    bool bTitle;

    OString sResTyp;
    OString sId;
    OString sGId;
    OString sFilename;

    OStringHashMap sText;

    OStringHashMap sQuickHelpText;

    OStringHashMap sTitle;

    OString sTextTyp;

    ExportList  m_aList;
};


// class Export


enum class ExportListType {
    NONE, String, Filter, Item, Paired
};

#define STRING_TYP_TEXT             0x0010
#define STRING_TYP_QUICKHELPTEXT    0x0040
#define STRING_TYP_TITLE            0x0080


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

    bool bDefine;                       // cur. res. in a define?
    bool bNextMustBeDefineEOL;          ///< define but no \ at lineend
    std::size_t nLevel; // res. recursive? how deep?
    ExportListType nList;                       ///< cur. res. is List
    std::size_t nListIndex;
    std::size_t nListLevel;
    bool bMergeMode;
    OString sMergeSrc;
    bool bError;                        // any errors while export?
    bool bReadOver;
    OString sFilename;

    std::vector<OString> aLanguages;

    ParserQueue* pParseQueue;

    void WriteData( ResData *pResData, bool bCreateNew = false ); ///< called before dest. cur ResData
    void WriteExportList( ResData *pResData, ExportList& rExportList, const ExportListType nTyp );

    OString FullId();                    ///< creates cur. GID

    static OString GetPairedListID(const OString & rText);
    static OString GetPairedListString(const OString& rText);
    static OString StripList(const OString& rText);

    void InsertListEntry(const OString &rLine);
    static void CleanValue( OString &rValue );
    static OString GetText(const OString &rSource, int nToken);

    void ResData2Output( MergeEntrys *pEntry, sal_uInt16 nType, const OString& rTextType );
    void MergeRest( ResData *pResData );
    static void ConvertMergeContent( OString &rText );
    static void ConvertExportContent( OString &rText );

    void WriteToMerged(const OString &rText , bool bSDFContent);
    void SetChildWithText();

    static void CutComment( OString &rText );

    void WriteUTF8ByteOrderMarkToOutput() { *aOutput.mSimple << '\xEF' << '\xBB' << '\xBF'; }

public:
    Export( const OString &rOutput );
    Export(const OString &rMergeSource, const OString &rOutput, bool bUTF8BOM);
    ~Export();

    void Init();
    void Execute( int nToken, const char * pToken ); ///< called from lexer

    void SetError() { bError = true; }
    bool GetError() { return bError; }
    ParserQueue* GetParseQueue() { return pParseQueue; }
};


// class MergeEntrys


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
    bool GetText( OString &rReturn, sal_uInt16 nTyp, const OString &nLangIndex, bool bDel = false );

    /**
      Generate QTZ string with ResData
      For executable which works one language and without PO files.
    */
    static OString GetQTZText(const ResData& rResData, const OString& rOrigText);

};


// class MergeDataHashMap


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
        typedef std::unordered_map<OString, MergeData*, OStringHash> HashMap_t;

    public:
        MergeDataHashMap()
            : bFirstSearch(true)
            , aLastInsertion(m_aHashMap.end())
            , aLastFound(m_aHashMap.end())
            , aFirstInOrder(m_aHashMap.end())
        {
        }

        ~MergeDataHashMap()
        {
        }

        typedef HashMap_t::iterator iterator;
        typedef HashMap_t::const_iterator const_iterator;

        std::pair<iterator,bool> insert(const OString& rKey, MergeData* pMergeData);
        iterator const & find(const OString& rKey);

        iterator begin() {return m_aHashMap.begin();}
        iterator end() {return m_aHashMap.end();}

        const_iterator begin() const {return m_aHashMap.begin();}
        const_iterator end() const {return m_aHashMap.end();}

    private:
        bool bFirstSearch;
        HashMap_t m_aHashMap;
        iterator aLastInsertion;
        iterator aLastFound;
        iterator aFirstInOrder;
};


// class MergeData


/// Purpose: holds information of data to merge (one resource)
class MergeData
{
    friend class MergeDataHashMap;

public:
    OString sGID;
    OString sLID;
    MergeEntrys* pMergeEntrys;
private:
    MergeDataHashMap::iterator m_aNextData;
public:
    MergeData( const OString &rGID, const OString &rLID );
    ~MergeData();
    MergeEntrys* GetMergeEntries() { return pMergeEntrys;}

};


// class MergeDataFile


/// Purpose: holds information of data to merge, read from PO file
class MergeDataFile
{
    private:
        MergeDataHashMap aMap;
        std::set<OString> aLanguageSet;

        MergeData *GetMergeData( ResData *pResData , bool bCaseSensitive = false );
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

    Export& aExport;
    bool bStart;

    inline void Pop( std::queue<QueueEntry>& aQueue );

};
#endif // INCLUDED_L10NTOOLS_INC_EXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
