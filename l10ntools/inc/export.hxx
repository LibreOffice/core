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

// class MergeDataFile


/// Purpose: holds information of data to merge, read from PO file
class MergeDataFile
{
    private:
        std::unordered_map<OString, std::unique_ptr<MergeEntrys>> aMap;
        std::set<OString> aLanguageSet;

        MergeEntrys *GetMergeData( ResData *pResData , bool bCaseSensitive = false );
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
