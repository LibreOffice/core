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

#include <sal/config.h>
#include "po.hxx"

#include <cstddef>
#include <fstream>

#include <osl/file.hxx>
#include <osl/file.h>

#include <iterator>
#include <set>
#include <unordered_map>
#include <memory>
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

typedef std::unordered_map<OString, OString>
    OStringHashMap;

typedef std::unordered_map<OString, bool>
    OStringBoolHashMap;

#define SOURCE_LANGUAGE "en-US"
#define X_COMMENT "x-comment"


// class ResData


/// Purpose: holds mandatory data to export a single res
class ResData
{
public:
    ResData( const OString &rGId );
    ResData( const OString &rGId , const OString &rFilename );

    OString sResTyp;
    OString sId;
    OString sGId;
    OString sFilename;

    OStringHashMap sText;
};


// class Export


class ParserQueue;

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
    bool GetText( OString &rReturn, const OString &nLangIndex, bool bDel = false );

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
        typedef std::unordered_map<OString, std::unique_ptr<MergeData>> HashMap_t;

    public:
        MergeDataHashMap()
            : bFirstSearch(true)
            , aLastInsertion(m_aHashMap.end())
            , aLastFound(m_aHashMap.end())
            , aFirstInOrder(m_aHashMap.end())
        {
        }

        typedef HashMap_t::iterator iterator;
        typedef HashMap_t::const_iterator const_iterator;

        std::pair<iterator,bool> insert(const OString& rKey, std::unique_ptr<MergeData> pMergeData);
        iterator const & find(const OString& rKey);

        iterator end() {return m_aHashMap.end();}

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

    std::unique_ptr<MergeEntrys> pMergeEntrys;
    MergeDataHashMap::iterator m_aNextData;

public:
    MergeData();
    ~MergeData();
    MergeEntrys* GetMergeEntries() { return pMergeEntrys.get();}

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

        MergeEntrys *GetMergeEntrys( ResData *pResData );
        MergeEntrys *GetMergeEntrysCaseSensitive( ResData *pResData );

        static OString CreateKey(const OString& rTYP, const OString& rGID,
            const OString& rLID, const OString& rFilename, bool bCaseSensitive);
};


#endif // INCLUDED_L10NTOOLS_INC_EXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
