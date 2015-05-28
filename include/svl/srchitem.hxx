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
#ifndef INCLUDED_SVL_SRCHITEM_HXX
#define INCLUDED_SVL_SRCHITEM_HXX

#include <sal/config.h>
#include <svl/svldllapi.h>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <unotools/configitem.hxx>
#include <rsc/rscsfx.hxx>
#include <svl/poolitem.hxx>
#include <svl/srchdefs.hxx>

// defines ---------------------------------------------------------------

// commands
enum class SvxSearchCmd
{
    FIND          = 0,
    FIND_ALL      = 1,
    REPLACE       = 2,
    REPLACE_ALL   = 3,
};

// search flags
enum class SvxSearchCellType
{
    FORMULA        = 0,
    VALUE          = 1,
    NOTE           = 2,
};

enum class SvxSearchApp
{
    WRITER        = 0,
    CALC          = 1,
    DRAW          = 2,
    BASE          = 3,
};

// class SvxSearchItem ---------------------------------------------------

class SVL_DLLPUBLIC SvxSearchItem :
        public SfxPoolItem,
        public utl::ConfigItem
{
    com::sun::star::util::SearchOptions aSearchOpt;

    SfxStyleFamily  eFamily;            // style family

    SvxSearchCmd    nCommand;           // command (Search, Search all, Replace, Replace all)

    // Calc-specific
    SvxSearchCellType nCellType;          // Search in Formulas/Values/Notes
    SvxSearchApp    nAppFlag;           // application which the dialog is for
    bool            bRowDirection;      // search direction: row-wise/column-wise
    bool            bAllTables;         // search in all sheets
    bool            bSearchFiltered;      // search filtered cells.

    // Writer-specific
    bool            bNotes;

    bool            bBackward;          // search backwards
    bool            bPattern;           // search for styles
    bool            bContent;           // search in content
    bool            bAsianOptions;      // use asian options?

    // Start search at this point (absolute twips).
    sal_Int32       m_nStartPointX;
    sal_Int32       m_nStartPointY;

    virtual void    ImplCommit() SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    explicit SvxSearchItem( const sal_uInt16 nId );
    SvxSearchItem( const SvxSearchItem& rItem );
    virtual ~SvxSearchItem();

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;
    virtual bool             operator == ( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    // ConfigItem
    virtual void            Notify( const com::sun::star::uno::Sequence< OUString > &rPropertyNames ) SAL_OVERRIDE;

            SvxSearchCmd    GetCommand() const { return nCommand; }
            void            SetCommand(SvxSearchCmd nNewCommand) { nCommand = nNewCommand; }

    inline  const OUString  GetSearchString() const;
    inline  void            SetSearchString(const OUString& rNewString);

    inline  const OUString  GetReplaceString() const;
    inline  void            SetReplaceString(const OUString& rNewString);

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

            SvxSearchCellType GetCellType() const { return nCellType; }
            void            SetCellType(SvxSearchCellType nNewCellType) { nCellType = nNewCellType; }

            bool            GetNotes() const { return bNotes; }
            void            SetNotes(bool bNew) { bNotes = bNew; }

            SvxSearchApp    GetAppFlag() const { return nAppFlag; }
            void            SetAppFlag(SvxSearchApp nNewAppFlag) { nAppFlag = nNewAppFlag; }

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

    sal_Int32 GetStartPointX() const;
    sal_Int32 GetStartPointY() const;
    /// Either x or y start point is set.
    bool HasStartPoint() const;
};

const OUString SvxSearchItem::GetSearchString() const
{
    return aSearchOpt.searchString;
}

void SvxSearchItem::SetSearchString(const OUString& rNewString)
{
    aSearchOpt.searchString = rNewString;
}

const OUString SvxSearchItem::GetReplaceString() const
{
    return aSearchOpt.replaceString;
}

void SvxSearchItem::SetReplaceString(const OUString& rNewString)
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
