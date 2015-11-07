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
    css::util::SearchOptions m_aSearchOpt;

    SfxStyleFamily  m_eFamily;            // style family

    SvxSearchCmd    m_nCommand;           // command (Search, Search all, Replace, Replace all)

    // Calc-specific
    SvxSearchCellType m_nCellType;          // Search in Formulas/Values/Notes
    SvxSearchApp    m_nAppFlag;           // application which the dialog is for
    bool            m_bRowDirection;      // search direction: row-wise/column-wise
    bool            m_bAllTables;         // search in all sheets
    bool            m_bSearchFiltered;      // search filtered cells.
    bool            m_bSearchFormatted;     // search formatted display strings

    // Writer-specific
    bool            m_bNotes;

    bool            m_bBackward;          // search backwards
    bool            m_bPattern;           // search for styles
    bool            m_bContent;           // search in content
    bool            m_bAsianOptions;      // use asian options?

    // Start search at this point (absolute twips).
    sal_Int32       m_nStartPointX;
    sal_Int32       m_nStartPointY;

    virtual void    ImplCommit() override;

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxSearchItem( const sal_uInt16 nId );
    SvxSearchItem( const SvxSearchItem& rItem );
    virtual ~SvxSearchItem();

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool             operator == ( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    // ConfigItem
    virtual void            Notify( const css::uno::Sequence< OUString > &rPropertyNames ) override;

            SvxSearchCmd    GetCommand() const { return m_nCommand; }
            void            SetCommand(SvxSearchCmd nNewCommand) { m_nCommand = nNewCommand; }

    inline  const OUString  GetSearchString() const;
    inline  void            SetSearchString(const OUString& rNewString);

    inline  const OUString  GetReplaceString() const;
    inline  void            SetReplaceString(const OUString& rNewString);

    inline  bool            GetWordOnly() const;
            void            SetWordOnly(bool bNewWordOnly);

    inline  bool            GetExact() const;
            void            SetExact(bool bNewExact);

            bool            GetBackward() const { return m_bBackward; }
            void            SetBackward(bool bNewBackward) { m_bBackward = bNewBackward; }

    inline  bool            GetSelection() const;
            void            SetSelection(bool bNewSelection);

    inline  bool            GetRegExp() const;
            void            SetRegExp( bool bVal );

            bool            GetPattern() const { return m_bPattern; }
            void            SetPattern(bool bNewPattern) { m_bPattern = bNewPattern; }

            SfxStyleFamily  GetFamily() const { return m_eFamily; }
            void            SetFamily( SfxStyleFamily eNewFamily )
                                { m_eFamily = eNewFamily; }

            bool            GetRowDirection() const { return m_bRowDirection; }
            void            SetRowDirection(bool bNewRowDirection) { m_bRowDirection = bNewRowDirection; }

            bool            IsAllTables() const { return m_bAllTables; }
            void            SetAllTables(bool bNew) { m_bAllTables = bNew; }

            bool            IsSearchFiltered() const { return m_bSearchFiltered; }
            void            SetSearchFiltered(bool b) { m_bSearchFiltered = b; }

            bool            IsSearchFormatted() const { return m_bSearchFormatted; }
            void            SetSearchFormatted(bool b) { m_bSearchFormatted = b; }

            SvxSearchCellType GetCellType() const { return m_nCellType; }
            void            SetCellType(SvxSearchCellType nNewCellType) { m_nCellType = nNewCellType; }

            bool            GetNotes() const { return m_bNotes; }
            void            SetNotes(bool bNew) { m_bNotes = bNew; }

            SvxSearchApp    GetAppFlag() const { return m_nAppFlag; }
            void            SetAppFlag(SvxSearchApp nNewAppFlag) { m_nAppFlag = nNewAppFlag; }

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

    inline const css::util::SearchOptions &
                            GetSearchOptions() const;
    inline void             SetSearchOptions( const css::util::SearchOptions &rOpt );

    inline  sal_Int32       GetTransliterationFlags() const;
            void            SetTransliterationFlags( sal_Int32 nFlags );

    inline  bool            IsMatchFullHalfWidthForms() const;
    void                    SetMatchFullHalfWidthForms( bool bVal );

    inline  bool            IsUseAsianOptions() const           { return m_bAsianOptions; }
    inline  void            SetUseAsianOptions( bool bVal ) { m_bAsianOptions = bVal; }

    sal_Int32 GetStartPointX() const;
    sal_Int32 GetStartPointY() const;
    /// Either x or y start point is set.
    bool HasStartPoint() const;
};

const OUString SvxSearchItem::GetSearchString() const
{
    return m_aSearchOpt.searchString;
}

void SvxSearchItem::SetSearchString(const OUString& rNewString)
{
    m_aSearchOpt.searchString = rNewString;
}

const OUString SvxSearchItem::GetReplaceString() const
{
    return m_aSearchOpt.replaceString;
}

void SvxSearchItem::SetReplaceString(const OUString& rNewString)
{
    m_aSearchOpt.replaceString = rNewString;
}

bool SvxSearchItem::GetWordOnly() const
{
    return 0 != (m_aSearchOpt.searchFlag &
                        css::util::SearchFlags::NORM_WORD_ONLY);
}

bool SvxSearchItem::GetExact() const
{
    return 0 == (m_aSearchOpt.transliterateFlags & css::i18n::TransliterationModules_IGNORE_CASE);
}

bool SvxSearchItem::GetSelection() const
{
    return 0 != (m_aSearchOpt.searchFlag & css::util::SearchFlags::REG_NOT_BEGINOFLINE);
}

bool SvxSearchItem::GetRegExp() const
{
    return m_aSearchOpt.algorithmType == css::util::SearchAlgorithms_REGEXP ;
}

bool SvxSearchItem::IsLEVRelaxed() const
{
    return 0 != (m_aSearchOpt.searchFlag & css::util::SearchFlags::LEV_RELAXED);
}

sal_uInt16 SvxSearchItem::GetLEVOther() const
{
    return (sal_Int16) m_aSearchOpt.changedChars;
}

void SvxSearchItem::SetLEVOther( sal_uInt16 nVal )
{
    m_aSearchOpt.changedChars = nVal;
}

sal_uInt16 SvxSearchItem::GetLEVShorter() const
{
    return (sal_Int16) m_aSearchOpt.insertedChars;
}

void SvxSearchItem::SetLEVShorter( sal_uInt16 nVal )
{
    m_aSearchOpt.insertedChars = nVal;
}

sal_uInt16 SvxSearchItem::GetLEVLonger() const
{
    return (sal_Int16) m_aSearchOpt.deletedChars;
}

void SvxSearchItem::SetLEVLonger( sal_uInt16 nVal )
{
    m_aSearchOpt.deletedChars = nVal;
}

bool SvxSearchItem::IsLevenshtein() const
{
    return m_aSearchOpt.algorithmType == css::util::SearchAlgorithms_APPROXIMATE;
}

const css::util::SearchOptions & SvxSearchItem::GetSearchOptions() const
{
    return m_aSearchOpt;
}

void SvxSearchItem::SetSearchOptions( const css::util::SearchOptions &rOpt )
{
    m_aSearchOpt = rOpt;
}

sal_Int32 SvxSearchItem::GetTransliterationFlags() const
{
    return m_aSearchOpt.transliterateFlags;
}

bool SvxSearchItem::IsMatchFullHalfWidthForms() const
{
    return 0 != (m_aSearchOpt.transliterateFlags & css::i18n::TransliterationModules_IGNORE_WIDTH);
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
