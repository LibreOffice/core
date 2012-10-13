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
#ifndef _SFX_SRCHITEM_HXX
#define _SFX_SRCHITEM_HXX

#include "sal/config.h"
#include "svl/svldllapi.h"
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <unotools/configitem.hxx>
#include <rsc/rscsfx.hxx>
#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include <svl/srchdefs.hxx>

// defines ---------------------------------------------------------------

// commands
#define     SVX_SEARCHCMD_FIND          ((sal_uInt16)0)
#define     SVX_SEARCHCMD_FIND_ALL      ((sal_uInt16)1)
#define     SVX_SEARCHCMD_REPLACE       ((sal_uInt16)2)
#define     SVX_SEARCHCMD_REPLACE_ALL   ((sal_uInt16)3)

// search flags
#define     SVX_SEARCHIN_FORMULA        ((sal_uInt16)0)
#define     SVX_SEARCHIN_VALUE          ((sal_uInt16)1)
#define     SVX_SEARCHIN_NOTE           ((sal_uInt16)2)
#define     SVX_SEARCHAPP_WRITER        ((sal_uInt16)0)
#define     SVX_SEARCHAPP_CALC          ((sal_uInt16)1)
#define     SVX_SEARCHAPP_DRAW          ((sal_uInt16)2)
#define     SVX_SEARCHAPP_BASE          ((sal_uInt16)3)

// class SvxSearchItem ---------------------------------------------------

class SVL_DLLPUBLIC SvxSearchItem :
        public SfxPoolItem,
        public utl::ConfigItem
{
    com::sun::star::util::SearchOptions aSearchOpt;

    SfxStyleFamily  eFamily;            // Vorlagen-Familie

    sal_uInt16      nCommand;           // Kommando (Suchen, Alle Suchen, Ersetzen, Alle Ersetzen)

    // Calc-Spezifische Daten
    sal_uInt16      nCellType;          // Suche in Formeln/Werten/Notizen
    sal_uInt16      nAppFlag;           // Fuer welche Applikation ist der Dialog ueberhaupt
    bool            bRowDirection;      // Suchrichtung Zeilenweise/Spaltenweise
    bool            bAllTables;         // in alle Tabellen suchen
    bool            bSearchFiltered;      // search filtered cells.

    // Writer-spezifisch
    bool            bNotes;

    bool            bBackward;          // Suche Rueckwaerts
    bool            bPattern;           // Suche nach Vorlagen
    bool            bContent;           // Suche im Inhalt
    bool            bAsianOptions;      // use asian options?

public:
    TYPEINFO();

    SvxSearchItem( const sal_uInt16 nId );
    SvxSearchItem( const SvxSearchItem& rItem );
    virtual ~SvxSearchItem();

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    virtual int              operator == ( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    // ConfigItem
    virtual void            Notify( const com::sun::star::uno::Sequence< rtl::OUString > &rPropertyNames );
    virtual void            Commit();

            sal_uInt16      GetCommand() const { return nCommand; }
            void            SetCommand(sal_uInt16 nNewCommand) { nCommand = nNewCommand; }

    inline  const String    GetSearchString() const;
    inline  void            SetSearchString(const String& rNewString);

    inline  const String    GetReplaceString() const;
    inline  void            SetReplaceString(const String& rNewString);

    inline  bool            GetWordOnly() const;
            void            SetWordOnly(bool bNewWordOnly);

    inline  bool            GetExact() const;
            void            SetExact(bool bNewExact);

            bool            GetBackward() const { return bBackward; }
            void            SetBackward(bool bNewBackward) { bBackward = bNewBackward; }

    inline  bool            GetSelection() const;
            void            SetSelection(bool bNewSelection);

    inline  bool            GetRegExp() const;
            void            SetRegExp( bool bVal );

            bool            GetPattern() const { return bPattern; }
            void            SetPattern(bool bNewPattern) { bPattern = bNewPattern; }

            bool            IsContent() const { return bContent; }
            void            SetContent( bool bNew ) { bContent = bNew; }

            SfxStyleFamily  GetFamily() const { return eFamily; }
            void            SetFamily( SfxStyleFamily eNewFamily )
                                { eFamily = eNewFamily; }

            bool            GetRowDirection() const { return bRowDirection; }
            void            SetRowDirection(bool bNewRowDirection) { bRowDirection = bNewRowDirection; }

            bool            IsAllTables() const { return bAllTables; }
            void            SetAllTables(bool bNew) { bAllTables = bNew; }

            bool            IsSearchFiltered() const { return bSearchFiltered; }
            void            SetSearchFiltered(bool b) { bSearchFiltered = b; }

            sal_uInt16      GetCellType() const { return nCellType; }
            void            SetCellType(sal_uInt16 nNewCellType) { nCellType = nNewCellType; }

            bool            GetNotes() const { return bNotes; }
            void            SetNotes(bool bNew) { bNotes = bNew; }

            sal_uInt16      GetAppFlag() const { return nAppFlag; }
            void            SetAppFlag(sal_uInt16 nNewAppFlag) { nAppFlag = nNewAppFlag; }

    inline  bool            IsLevenshtein() const;
            void            SetLevenshtein( bool bVal );

    inline  bool            IsLEVRelaxed() const;
            void            SetLEVRelaxed(bool bSet);

    inline  sal_uInt16      GetLEVOther() const;
    inline  void            SetLEVOther(sal_uInt16 nSet);

    inline  sal_uInt16      GetLEVShorter() const;
    inline  void            SetLEVShorter(sal_uInt16 nSet);

    inline  sal_uInt16      GetLEVLonger() const;
    inline  void            SetLEVLonger(sal_uInt16 nSet);

    inline const com::sun::star::util::SearchOptions &
                GetSearchOptions() const;
    inline void SetSearchOptions( const com::sun::star::util::SearchOptions &rOpt );

    inline  sal_Int32       GetTransliterationFlags() const;
            void            SetTransliterationFlags( sal_Int32 nFlags );

    inline  bool            IsMatchFullHalfWidthForms() const;
    void                    SetMatchFullHalfWidthForms( bool bVal );

    inline  bool            IsUseAsianOptions() const           { return bAsianOptions; }
    inline  void            SetUseAsianOptions( bool bVal ) { bAsianOptions = bVal; }
};

const String SvxSearchItem::GetSearchString() const
{
    return aSearchOpt.searchString;
}

void SvxSearchItem::SetSearchString(const String& rNewString)
{
    aSearchOpt.searchString = rNewString;
}

const String SvxSearchItem::GetReplaceString() const
{
    return aSearchOpt.replaceString;
}

void SvxSearchItem::SetReplaceString(const String& rNewString)
{
    aSearchOpt.replaceString = rNewString;
}

bool SvxSearchItem::GetWordOnly() const
{
    return 0 != (aSearchOpt.searchFlag &
                        com::sun::star::util::SearchFlags::NORM_WORD_ONLY);
}

bool SvxSearchItem::GetExact() const
{
    return 0 == (aSearchOpt.transliterateFlags &
                        com::sun::star::i18n::TransliterationModules_IGNORE_CASE);
}

bool SvxSearchItem::GetSelection() const
{
    return 0 != (aSearchOpt.searchFlag &
                        com::sun::star::util::SearchFlags::REG_NOT_BEGINOFLINE);
}

bool SvxSearchItem::GetRegExp() const
{
    return aSearchOpt.algorithmType == com::sun::star::util::SearchAlgorithms_REGEXP ;
}

bool SvxSearchItem::IsLEVRelaxed() const
{
    return 0 != (aSearchOpt.searchFlag &
                        com::sun::star::util::SearchFlags::LEV_RELAXED);
}

sal_uInt16 SvxSearchItem::GetLEVOther() const
{
    return (sal_Int16) aSearchOpt.changedChars;
}

void SvxSearchItem::SetLEVOther( sal_uInt16 nVal )
{
    aSearchOpt.changedChars = nVal;
}

sal_uInt16 SvxSearchItem::GetLEVShorter() const
{
    return (sal_Int16) aSearchOpt.insertedChars;
}

void SvxSearchItem::SetLEVShorter( sal_uInt16 nVal )
{
    aSearchOpt.insertedChars = nVal;
}

sal_uInt16 SvxSearchItem::GetLEVLonger() const
{
    return (sal_Int16) aSearchOpt.deletedChars;
}

void SvxSearchItem::SetLEVLonger( sal_uInt16 nVal )
{
    aSearchOpt.deletedChars = nVal;
}

bool SvxSearchItem::IsLevenshtein() const
{
    return aSearchOpt.algorithmType == com::sun::star::util::SearchAlgorithms_APPROXIMATE;
}

const com::sun::star::util::SearchOptions & SvxSearchItem::GetSearchOptions() const
{
    return aSearchOpt;
}

void SvxSearchItem::SetSearchOptions( const com::sun::star::util::SearchOptions &rOpt )
{
    aSearchOpt = rOpt;
}

sal_Int32 SvxSearchItem::GetTransliterationFlags() const
{
    return aSearchOpt.transliterateFlags;
}

bool SvxSearchItem::IsMatchFullHalfWidthForms() const
{
    return 0 != (aSearchOpt.transliterateFlags &
                        com::sun::star::i18n::TransliterationModules_IGNORE_WIDTH);
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
