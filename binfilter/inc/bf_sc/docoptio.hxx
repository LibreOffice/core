/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DOCOPTIO_HXX
#define SC_DOCOPTIO_HXX



#include <bf_svtools/poolitem.hxx>

#include "optutil.hxx"
namespace binfilter {



class ScDocOptions
{
    double fIterEps;				// Epsilon-Wert dazu
    USHORT nIterCount;				// Anzahl
    USHORT nPrecStandardFormat;		// Nachkommastellen Standard
    USHORT nDay;					// Nulldatum:
    USHORT nMonth;
    USHORT nYear;
    USHORT nYear2000;				// bis zu welcher zweistelligen Jahreszahl 20xx angenommen wird
    USHORT nTabDistance;			// Abstand Standardtabulatoren
    BOOL   bIsIgnoreCase;			// Gross-/Kleinschr. bei Vergleichen
    BOOL   bIsIter;					// Iteration bei cirk. Ref
    BOOL   bCalcAsShown;			// berechnen wie angezeigt (Precision)
    BOOL   bMatchWholeCell;			// Suchkriterien muessen ganze Zelle matchen
    BOOL   bDoAutoSpell;			// Auto-Spelling
    BOOL   bLookUpColRowNames;		// Spalten-/Zeilenbeschriftungen automagisch suchen
    BOOL   bFormulaRegexEnabled;    // regular expressions in formulas enabled

public:
                ScDocOptions();
                ScDocOptions( const ScDocOptions& rCpy );
                ~ScDocOptions();

    BOOL   IsLookUpColRowNames() const	{ return bLookUpColRowNames; }
    void   SetLookUpColRowNames( BOOL bVal ) { bLookUpColRowNames = bVal; }
    BOOL   IsAutoSpell() const			{ return bDoAutoSpell; }
    void   SetAutoSpell( BOOL bVal )	{ bDoAutoSpell = bVal; }
    BOOL   IsMatchWholeCell() const		{ return bMatchWholeCell; }
    void   SetMatchWholeCell( BOOL bVal ){ bMatchWholeCell = bVal; }
    BOOL   IsIgnoreCase() const			{ return bIsIgnoreCase; }
    void   SetIgnoreCase( BOOL bVal )	{ bIsIgnoreCase = bVal; }
    BOOL   IsIter() const				{ return bIsIter; }
    void   SetIter( BOOL bVal )			{ bIsIter = bVal; }
    USHORT GetIterCount() const			{ return nIterCount; }
    void   SetIterCount( USHORT nCount) { nIterCount = nCount; }
    double GetIterEps() const			{ return fIterEps; }
    void   SetIterEps( double fEps )	{ fIterEps = fEps; }

    void   GetDate( USHORT& rD, USHORT& rM, USHORT& rY ) const
                                        { rD = nDay; rM = nMonth; rY = nYear;}
    void   SetDate (USHORT nD, USHORT nM, USHORT nY)
                                        { nDay = nD; nMonth = nM; nYear = nY; }
    USHORT GetTabDistance() const { return nTabDistance;}
    void   SetTabDistance( USHORT nTabDist ) {nTabDistance = nTabDist;}

    void		ResetDocOptions();
    inline void		CopyTo(ScDocOptions& rOpt);
    void		Load(SvStream& rStream);
    void		Save(SvStream& rStream, BOOL bConfig = FALSE) const;

    inline const ScDocOptions&	operator=( const ScDocOptions& rOpt );
    inline int					operator==( const ScDocOptions& rOpt ) const;
    inline int					operator!=( const ScDocOptions& rOpt ) const;

    USHORT	GetStdPrecision() const 	{ return nPrecStandardFormat; }
    void	SetStdPrecision( USHORT n ) { nPrecStandardFormat = n; }

    BOOL	IsCalcAsShown() const		{ return bCalcAsShown; }
    void	SetCalcAsShown( BOOL bVal )	{ bCalcAsShown = bVal; }

    void	SetYear2000( USHORT nVal )	{ nYear2000 = nVal; }
    USHORT	GetYear2000() const			{ return nYear2000; }

    void    SetFormulaRegexEnabled( BOOL bVal ) { bFormulaRegexEnabled = bVal; }
    BOOL    IsFormulaRegexEnabled() const       { return bFormulaRegexEnabled; }
};


inline void ScDocOptions::CopyTo(ScDocOptions& rOpt)
{
    rOpt.bIsIgnoreCase			= bIsIgnoreCase;
    rOpt.bIsIter 				= bIsIter;
    rOpt.nIterCount 			= nIterCount;
    rOpt.fIterEps 				= fIterEps;
    rOpt.nPrecStandardFormat 	= nPrecStandardFormat;
    rOpt.nDay 					= nDay;
    rOpt.nMonth 				= nMonth;
    rOpt.nYear2000				= nYear2000;
    rOpt.nYear 					= nYear;
    rOpt.nTabDistance			= nTabDistance;
    rOpt.bCalcAsShown			= bCalcAsShown;
    rOpt.bMatchWholeCell		= bMatchWholeCell;
    rOpt.bDoAutoSpell			= bDoAutoSpell;
    rOpt.bLookUpColRowNames		= bLookUpColRowNames;
    rOpt.bFormulaRegexEnabled   = bFormulaRegexEnabled;
}

inline const ScDocOptions& ScDocOptions::operator=( const ScDocOptions& rCpy )
{
    bIsIgnoreCase		= rCpy.bIsIgnoreCase;
    bIsIter				= rCpy.bIsIter;
    nIterCount			= rCpy.nIterCount;
    fIterEps			= rCpy.fIterEps;
    nPrecStandardFormat = rCpy.nPrecStandardFormat;
    nDay				= rCpy.nDay;
    nMonth				= rCpy.nMonth;
    nYear				= rCpy.nYear;
    nYear2000			= rCpy.nYear2000;
    nTabDistance 		= rCpy.nTabDistance;
    bCalcAsShown		= rCpy.bCalcAsShown;
    bMatchWholeCell		= rCpy.bMatchWholeCell;
    bDoAutoSpell		= rCpy.bDoAutoSpell;
    bLookUpColRowNames	= rCpy.bLookUpColRowNames;
    bFormulaRegexEnabled= rCpy.bFormulaRegexEnabled;

    return *this;
}

inline int ScDocOptions::operator==( const ScDocOptions& rOpt ) const
{
    return (
                rOpt.bIsIgnoreCase			== bIsIgnoreCase
            &&	rOpt.bIsIter 				== bIsIter
            &&	rOpt.nIterCount 			== nIterCount
            &&	rOpt.fIterEps 				== fIterEps
            &&	rOpt.nPrecStandardFormat 	== nPrecStandardFormat
            &&	rOpt.nDay 					== nDay
            &&	rOpt.nMonth 				== nMonth
            &&	rOpt.nYear 					== nYear
            &&	rOpt.nYear2000				== nYear2000
            &&  rOpt.nTabDistance 			== nTabDistance
            &&	rOpt.bCalcAsShown			== bCalcAsShown
            &&	rOpt.bMatchWholeCell		== bMatchWholeCell
            &&	rOpt.bDoAutoSpell			== bDoAutoSpell
            &&	rOpt.bLookUpColRowNames		== bLookUpColRowNames
            &&  rOpt.bFormulaRegexEnabled   == bFormulaRegexEnabled
            );
}

inline int ScDocOptions::operator!=( const ScDocOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

//==================================================================
// Item fuer Einstellungsdialog - Berechnen
//==================================================================


//==================================================================
//	Config Item containing document options
//==================================================================

class ScDocCfg : public ScDocOptions
{
    ScLinkConfigItem	aCalcItem;
    ScLinkConfigItem	aLayoutItem;

    DECL_LINK( CalcCommitHdl, void* );
    DECL_LINK( LayoutCommitHdl, void* );

    ::com::sun::star::uno::Sequence<rtl::OUString> GetCalcPropertyNames();
    ::com::sun::star::uno::Sequence<rtl::OUString> GetLayoutPropertyNames();

public:
            ScDocCfg();

};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
