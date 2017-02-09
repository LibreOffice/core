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

#ifndef INCLUDED_SC_INC_DOCOPTIO_HXX
#define INCLUDED_SC_INC_DOCOPTIO_HXX

#include <unotools/configitem.hxx>
#include <unotools/textsearch.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemprop.hxx>
#include "scdllapi.h"
#include "scmod.hxx"
#include "optutil.hxx"

#include <formula/grammar.hxx>

class SC_DLLPUBLIC ScDocOptions
{
    double     fIterEps;                // epsilon value dazu
    sal_uInt16 nIterCount;              ///< number
    sal_uInt16 nPrecStandardFormat;     ///< precision for standard format
    sal_uInt16 nDay;                    ///< Null date:
    sal_uInt16 nMonth;
    sal_uInt16 nYear;
    sal_uInt16 nYear2000;               ///< earlier 19YY is assumed, 20YY otherwise (if only YY of year is given)
    sal_uInt16 nTabDistance;            ///< distance of standard tabs
    mutable utl::SearchParam::SearchType eFormulaSearchType; ///< wildcards or regular expressions or normal search
    bool       bIsIgnoreCase;           ///< ignore case for comparisons?
    bool       bIsIter;                 ///< iterations for circular refs
    bool       bCalcAsShown;            ///< calculate as shown (wrt precision)
    bool       bMatchWholeCell;         ///< search criteria must match the whole cell
    bool       bDoAutoSpell;            ///< auto-spelling
    bool       bLookUpColRowNames;      ///< determine column-/row titles automagically
    mutable bool bFormulaRegexEnabled;    ///< regular expressions in formulas enabled, only when reading settings
    mutable bool bFormulaWildcardsEnabled;///< wildcards in formulas enabled, only when reading settings
    bool       bWriteCalcConfig;        ///< (subset of) Calc config will be written to user's profile

public:
                ScDocOptions();
                ScDocOptions( const ScDocOptions& rCpy );
                ~ScDocOptions();

    bool   IsLookUpColRowNames() const       { return bLookUpColRowNames; }
    void   SetLookUpColRowNames( bool bVal ) { bLookUpColRowNames = bVal; }
    bool   IsAutoSpell() const              { return bDoAutoSpell; }
    void   SetAutoSpell( bool bVal )        { bDoAutoSpell = bVal; }
    bool   IsMatchWholeCell() const         { return bMatchWholeCell; }
    void   SetMatchWholeCell( bool bVal )   { bMatchWholeCell = bVal; }
    bool   IsIgnoreCase() const             { return bIsIgnoreCase; }
    void   SetIgnoreCase( bool bVal )       { bIsIgnoreCase = bVal; }
    bool   IsIter() const                   { return bIsIter; }
    void   SetIter( bool bVal )             { bIsIter = bVal; }
    sal_uInt16 GetIterCount() const         { return nIterCount; }
    void   SetIterCount( sal_uInt16 nCount) { nIterCount = nCount; }
    double GetIterEps() const               { return fIterEps; }
    void   SetIterEps( double fEps )        { fIterEps = fEps; }

    void   GetDate( sal_uInt16& rD, sal_uInt16& rM, sal_Int16& rY ) const
                                        { rD = nDay; rM = nMonth; rY = nYear;}
    void   SetDate (sal_uInt16 nD, sal_uInt16 nM, sal_Int16 nY)
                                        { nDay = nD; nMonth = nM; nYear = nY; }
    sal_uInt16 GetTabDistance() const { return nTabDistance;}
    void   SetTabDistance( sal_uInt16 nTabDist ) {nTabDistance = nTabDist;}

    void        ResetDocOptions();

    inline bool                 operator==( const ScDocOptions& rOpt ) const;
    inline bool                 operator!=( const ScDocOptions& rOpt ) const;

    sal_uInt16  GetStdPrecision() const         { return nPrecStandardFormat; }
    void        SetStdPrecision( sal_uInt16 n ) { nPrecStandardFormat = n; }

    bool    IsCalcAsShown() const           { return bCalcAsShown; }
    void    SetCalcAsShown( bool bVal )     { bCalcAsShown = bVal; }

    void    SetYear2000( sal_uInt16 nVal )  { nYear2000 = nVal; }
    sal_uInt16  GetYear2000() const         { return nYear2000; }

    utl::SearchParam::SearchType GetFormulaSearchType() const
    {
        if (eFormulaSearchType == utl::SearchParam::SearchType::Unknown || (bFormulaRegexEnabled && bFormulaWildcardsEnabled))
            eFormulaSearchType = utl::SearchParam::ConvertToSearchType( bFormulaWildcardsEnabled, bFormulaRegexEnabled);
        return eFormulaSearchType;
    }

    void    SetFormulaRegexEnabled( bool bVal );
    bool    IsFormulaRegexEnabled() const       { return GetFormulaSearchType() == utl::SearchParam::SearchType::Regexp; }

    void    SetFormulaWildcardsEnabled( bool bVal );
    bool    IsFormulaWildcardsEnabled() const   { return GetFormulaSearchType() == utl::SearchParam::SearchType::Wildcard; }

    void    SetWriteCalcConfig( bool bVal ) { bWriteCalcConfig = bVal; }
    bool    IsWriteCalcConfig() const       { return bWriteCalcConfig; }
};

inline bool ScDocOptions::operator==( const ScDocOptions& rOpt ) const
{
    return (
                rOpt.bIsIgnoreCase          == bIsIgnoreCase
            &&  rOpt.bIsIter                == bIsIter
            &&  rOpt.nIterCount             == nIterCount
            &&  rOpt.fIterEps               == fIterEps
            &&  rOpt.nPrecStandardFormat    == nPrecStandardFormat
            &&  rOpt.nDay                   == nDay
            &&  rOpt.nMonth                 == nMonth
            &&  rOpt.nYear                  == nYear
            &&  rOpt.nYear2000              == nYear2000
            &&  rOpt.nTabDistance           == nTabDistance
            &&  rOpt.bCalcAsShown           == bCalcAsShown
            &&  rOpt.bMatchWholeCell        == bMatchWholeCell
            &&  rOpt.bDoAutoSpell           == bDoAutoSpell
            &&  rOpt.bLookUpColRowNames     == bLookUpColRowNames
            &&  rOpt.bFormulaRegexEnabled   == bFormulaRegexEnabled
            &&  rOpt.bFormulaWildcardsEnabled == bFormulaWildcardsEnabled
            &&  rOpt.eFormulaSearchType     == eFormulaSearchType
            &&  rOpt.bWriteCalcConfig       == bWriteCalcConfig
            );
}

inline bool ScDocOptions::operator!=( const ScDocOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

// Item for preferences dialog - calculation

class SC_DLLPUBLIC ScTpCalcItem : public SfxPoolItem
{
public:
                static SfxPoolItem* CreateDefault();
                ScTpCalcItem( sal_uInt16 nWhich,
                              const ScDocOptions& rOpt );
                ScTpCalcItem( const ScTpCalcItem& rItem );
                virtual ~ScTpCalcItem() override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScDocOptions& GetDocOptions() const { return theOptions; }

private:
    ScDocOptions theOptions;
};

//  Config Item containing document options

class ScDocCfg : public ScDocOptions
{
    ScLinkConfigItem    aCalcItem;
    ScLinkConfigItem    aLayoutItem;

    DECL_LINK( CalcCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK( LayoutCommitHdl, ScLinkConfigItem&, void );

    static css::uno::Sequence<OUString> GetCalcPropertyNames();
    static css::uno::Sequence<OUString> GetLayoutPropertyNames();

public:
            ScDocCfg();

    void    SetOptions( const ScDocOptions& rNew );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
