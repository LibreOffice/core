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

#include <map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <editeng/ESelection.hxx>

#include "address.hxx"

class SvXMLNamespaceMap;
enum class XmlStyleFamily;

struct ScStreamEntry
{
    sal_Int64   mnStartOffset;
    sal_Int64   mnEndOffset;

                ScStreamEntry() :
                    mnStartOffset(-1),
                    mnEndOffset(-1)
                {
                }

                ScStreamEntry( sal_Int64 nStart, sal_Int64 nEnd ) :
                    mnStartOffset(nStart),
                    mnEndOffset(nEnd)
                {
                }
};

struct ScCellStyleEntry
{
    OUString   maName;
    ScAddress       maCellPos;

                ScCellStyleEntry( OUString aName, const ScAddress& rPos ) :
                    maName(std::move(aName)),
                    maCellPos(rPos)
                {
                }
};

struct ScNoteStyleEntry
{
    OUString   maStyleName;
    OUString   maTextStyle;
    ScAddress       maCellPos;

                ScNoteStyleEntry( OUString aStyle, OUString aText, const ScAddress& rPos ) :
                    maStyleName(std::move(aStyle)),
                    maTextStyle(std::move(aText)),
                    maCellPos(rPos)
                {
                }
};

struct ScTextStyleEntry
{
    OUString   maName;
    ScAddress       maCellPos;
    ESelection      maSelection;

                ScTextStyleEntry( OUString aName, const ScAddress& rPos, const ESelection& rSel ) :
                    maName(std::move(aName)),
                    maCellPos(rPos),
                    maSelection(rSel)
                {
                }
};

struct ScLoadedNamespaceEntry
{
    OUString   maPrefix;
    OUString   maName;
    sal_uInt16      mnKey;

                ScLoadedNamespaceEntry( OUString aPrefix, OUString aName, sal_uInt16 nKey ) :
                    maPrefix(std::move(aPrefix)),
                    maName(std::move(aName)),
                    mnKey(nKey)
                {
                }
};

class ScSheetSaveData
{
    std::unordered_set<OUString>  maInitialPrefixes;
    std::vector<ScLoadedNamespaceEntry>              maLoadedNamespaces;

    std::vector<ScCellStyleEntry> maCellStyles;
    std::vector<ScCellStyleEntry> maColumnStyles;
    std::vector<ScCellStyleEntry> maRowStyles;
    std::vector<ScCellStyleEntry> maTableStyles;
    std::vector<ScNoteStyleEntry> maNoteStyles;
    std::vector<ScTextStyleEntry> maNoteParaStyles;
    std::vector<ScTextStyleEntry> maNoteTextStyles;
    std::vector<ScTextStyleEntry> maTextStyles;
    std::vector<bool>          maBlocked;
    std::vector<ScStreamEntry> maStreamEntries;
    std::vector<ScStreamEntry> maSaveEntries;
    SCTAB   mnStartTab;
    sal_Int64   mnStartOffset;

    ScNoteStyleEntry    maPreviousNote;

    bool                mbInSupportedSave;

public:
                ScSheetSaveData();
                ~ScSheetSaveData();

    void        AddCellStyle( const OUString& rName, const ScAddress& rCellPos );
    void        AddColumnStyle( const OUString& rName, const ScAddress& rCellPos );
    void        AddRowStyle( const OUString& rName, const ScAddress& rCellPos );
    void        AddTableStyle( const OUString& rName, const ScAddress& rCellPos );

    void        HandleNoteStyles( const OUString& rStyleName, const OUString& rTextName, const ScAddress& rCellPos );
    void        AddNoteContentStyle( XmlStyleFamily nFamily, const OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection );

    void        AddTextStyle( const OUString& rName, const ScAddress& rCellPos, const ESelection& rSelection );

    void        BlockSheet( SCTAB nTab );
    bool        IsSheetBlocked( SCTAB nTab ) const;

    void        AddStreamPos( SCTAB nTab, sal_Int64 nStartOffset, sal_Int64 nEndOffset );
    void        GetStreamPos( SCTAB nTab, sal_Int64& rStartOffset, sal_Int64& rEndOffset ) const;
    bool        HasStreamPos( SCTAB nTab ) const;

    void        StartStreamPos( SCTAB nTab, sal_Int64 nStartOffset );
    void        EndStreamPos( sal_Int64 nEndOffset );

    bool        HasStartPos() const { return mnStartTab >= 0; }

    void        ResetSaveEntries();
    void        AddSavePos( SCTAB nTab, sal_Int64 nStartOffset, sal_Int64 nEndOffset );
    void        UseSaveEntries();

    void        StoreInitialNamespaces( const SvXMLNamespaceMap& rNamespaces );
    void        StoreLoadedNamespaces( const SvXMLNamespaceMap& rNamespaces );
    bool        AddLoadedNamespaces( SvXMLNamespaceMap& rNamespaces ) const;

    const std::vector<ScCellStyleEntry>& GetCellStyles() const   { return maCellStyles; }
    const std::vector<ScCellStyleEntry>& GetColumnStyles() const { return maColumnStyles; }
    const std::vector<ScCellStyleEntry>& GetRowStyles() const    { return maRowStyles; }
    const std::vector<ScCellStyleEntry>& GetTableStyles() const  { return maTableStyles; }
    const std::vector<ScNoteStyleEntry>& GetNoteStyles() const   { return maNoteStyles; }
    const std::vector<ScTextStyleEntry>& GetNoteParaStyles() const { return maNoteParaStyles; }
    const std::vector<ScTextStyleEntry>& GetNoteTextStyles() const { return maNoteTextStyles; }
    const std::vector<ScTextStyleEntry>& GetTextStyles() const   { return maTextStyles; }

    bool        IsInSupportedSave() const { return mbInSupportedSave;}
    void        SetInSupportedSave( bool bSet );
};

struct ScFormatSaveData
{
    std::map<sal_uInt64, OUString> maIDToName;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
