/*************************************************************************
 *
 *  $RCSfile: docoptio.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-02 19:12:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/svapp.hxx>
#include <svtools/zforlist.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "cfgids.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------------

#define SC_VERSION ((USHORT)251)

TYPEINIT1(ScTpCalcItem, SfxPoolItem);

//------------------------------------------------------------------------

//! these functions should be moved to some header file
inline long TwipsToHMM(long nTwips) { return (nTwips * 127 + 36) / 72; }
inline long HMMToTwips(long nHMM)   { return (nHMM * 72 + 63) / 127; }

inline long TwipsToEvenHMM(long nTwips) { return ( (nTwips * 127 + 72) / 144 ) * 2; }

//------------------------------------------------------------------------

USHORT lcl_GetDefaultTabDist()
{
    if ( ScOptionsUtil::IsMetricSystem() )
        return 709;                 // 1,25 cm
    else
        return 720;                 // 1/2"
}

//========================================================================
//      ScDocOptions - Dokument-Optionen
//========================================================================

ScDocOptions::ScDocOptions()
{
    ResetDocOptions();
}

//------------------------------------------------------------------------

ScDocOptions::ScDocOptions( const ScDocOptions& rCpy )
        :   bIsIgnoreCase( rCpy.bIsIgnoreCase ),
            bIsIter( rCpy.bIsIter ),
            nIterCount( rCpy.nIterCount ),
            fIterEps( rCpy.fIterEps ),
            nPrecStandardFormat( rCpy.nPrecStandardFormat ),
            nDay( rCpy.nDay ),
            nMonth( rCpy.nMonth ),
            nYear( rCpy.nYear ),
            nYear2000( rCpy.nYear2000 ),
            nTabDistance( rCpy.nTabDistance ),
            bCalcAsShown( rCpy.bCalcAsShown ),
            bMatchWholeCell( rCpy.bMatchWholeCell ),
            bDoAutoSpell( rCpy.bDoAutoSpell ),
            bLookUpColRowNames( rCpy.bLookUpColRowNames )
{
}

//------------------------------------------------------------------------

ScDocOptions::~ScDocOptions()
{
}

//------------------------------------------------------------------------

void ScDocOptions::Save(SvStream& rStream, BOOL bConfig) const
{
    ScWriteHeader aHdr( rStream, 28 );

    rStream << bIsIgnoreCase;
    rStream << bIsIter;
    rStream << nIterCount;
    rStream << fIterEps;
    rStream << nPrecStandardFormat;
    rStream << nDay;
    rStream << nMonth;
    rStream << nYear;
    rStream << nTabDistance;
    rStream << bCalcAsShown;
    rStream << bMatchWholeCell;
    rStream << bDoAutoSpell;
    rStream << bLookUpColRowNames;

    if ( bConfig || rStream.GetVersion() > SOFFICE_FILEFORMAT_40 )      // nicht bei 4.0 Export
    {
        if ( !bConfig && 1901 <= nYear2000 && nYear2000 <= 1999 )
        {   // fuer SO5 auf altes Format zweistellig abbilden
            rStream << (USHORT) (nYear2000 - 1901);
        }
        else
        {   // neues Format vierstellig, beliebiges Jahrhundert
            // erzeugt in SO5 vor src513e ein Warning beim Laden
            rStream << (USHORT) 29;     // Dummy, alter SO5 Default
            rStream << nYear2000;       // echter Wert
        }
    }
}

void ScDocOptions::Load(SvStream& rStream)
{
    ScReadHeader aHdr( rStream );

    rStream >> bIsIgnoreCase;
    rStream >> bIsIter;
    rStream >> nIterCount;
    rStream >> fIterEps;
    rStream >> nPrecStandardFormat;
    rStream >> nDay;
    rStream >> nMonth;
    rStream >> nYear;
    if ( aHdr.BytesLeft() )
        rStream >> nTabDistance;
    else
        nTabDistance = lcl_GetDefaultTabDist();
    if ( aHdr.BytesLeft() )
        rStream >> bCalcAsShown;
    else
        bCalcAsShown = FALSE;
    if ( aHdr.BytesLeft() )
        rStream >> bMatchWholeCell;
    else
        bMatchWholeCell = FALSE;
    if ( aHdr.BytesLeft() )
        rStream >> bDoAutoSpell;
    else
        bDoAutoSpell = FALSE;
    if ( aHdr.BytesLeft() )
        rStream >> bLookUpColRowNames;
    else
        bLookUpColRowNames = TRUE;
    if ( aHdr.BytesLeft() )
    {
        rStream >> nYear2000;       // SO5 ab 24.06.98
        // SO51 ab src513e
        if ( aHdr.BytesLeft() )
            rStream >> nYear2000;   // der echte Wert
        else
            nYear2000 += 1901;      // altes zweistelliges auf neues vierstelliges
    }
    else
        nYear2000 = 18 + 1901;      // alter Wert vor SO5
}

void ScDocOptions::ResetDocOptions()
{
    bIsIgnoreCase       = FALSE;
    bIsIter             = FALSE;
    nIterCount          = 100;
    fIterEps            = 1.0E-3;
    nPrecStandardFormat = 2;
    nDay                = 30;
    nMonth              = 12;
    nYear               = 1899;
    nYear2000           = SvNumberFormatter::GetYear2000Default();
    nTabDistance        = lcl_GetDefaultTabDist();
    bCalcAsShown        = FALSE;
    bMatchWholeCell     = TRUE;
    bDoAutoSpell        = FALSE;
    bLookUpColRowNames  = TRUE;
}

//========================================================================
//      ScTpCalcItem - Daten fuer die CalcOptions-TabPage
//========================================================================

ScTpCalcItem::ScTpCalcItem( USHORT nWhich ) : SfxPoolItem( nWhich )
{
}

//------------------------------------------------------------------------

ScTpCalcItem::ScTpCalcItem( USHORT nWhich, const ScDocOptions& rOpt )
    :   SfxPoolItem ( nWhich ),
        theOptions  ( rOpt )
{
}

//------------------------------------------------------------------------

ScTpCalcItem::ScTpCalcItem( const ScTpCalcItem& rItem )
    :   SfxPoolItem ( rItem ),
        theOptions  ( rItem.theOptions )
{
}

//------------------------------------------------------------------------

__EXPORT ScTpCalcItem::~ScTpCalcItem()
{
}

//------------------------------------------------------------------------

String __EXPORT ScTpCalcItem::GetValueText() const
{
    return String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("ScTpCalcItem") );
}

//------------------------------------------------------------------------

int __EXPORT ScTpCalcItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpCalcItem& rPItem = (const ScTpCalcItem&)rItem;

    return ( theOptions == rPItem.theOptions );
}

//------------------------------------------------------------------------

SfxPoolItem* __EXPORT ScTpCalcItem::Clone( SfxItemPool * ) const
{
    return new ScTpCalcItem( *this );
}

//==================================================================
//  Config Item containing document options
//==================================================================

#define CFGPATH_CALC        "Office.Calc/Calculate"

#define SCCALCOPT_ITER_ITER         0
#define SCCALCOPT_ITER_STEPS        1
#define SCCALCOPT_ITER_MINCHG       2
#define SCCALCOPT_DATE_DAY          3
#define SCCALCOPT_DATE_MONTH        4
#define SCCALCOPT_DATE_YEAR         5
#define SCCALCOPT_DECIMALS          6
#define SCCALCOPT_CASESENSITIVE     7
#define SCCALCOPT_PRECISION         8
#define SCCALCOPT_SEARCHCRIT        9
#define SCCALCOPT_FINDLABEL         10
#define SCCALCOPT_COUNT             11

#define CFGPATH_DOCLAYOUT   "Office.Calc/Layout/Other"

#define SCDOCLAYOUTOPT_TABSTOP      0
#define SCDOCLAYOUTOPT_COUNT        1


Sequence<OUString> ScDocCfg::GetCalcPropertyNames()
{
    static const char* aPropNames[] =
    {
        "IterativeReference/Iteration",     // SCCALCOPT_ITER_ITER
        "IterativeReference/Steps",         // SCCALCOPT_ITER_STEPS
        "IterativeReference/MinimumChange", // SCCALCOPT_ITER_MINCHG
        "Other/Date/DD",                    // SCCALCOPT_DATE_DAY
        "Other/Date/MM",                    // SCCALCOPT_DATE_MONTH
        "Other/Date/YY",                    // SCCALCOPT_DATE_YEAR
        "Other/DecimalPlaces",              // SCCALCOPT_DECIMALS
        "Other/CaseSensitive",              // SCCALCOPT_CASESENSITIVE
        "Other/Precision",                  // SCCALCOPT_PRECISION
        "Other/SearchCriteria",             // SCCALCOPT_SEARCHCRIT
        "Other/FindLabel"                   // SCCALCOPT_FINDLABEL
    };
    Sequence<OUString> aNames(SCCALCOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCCALCOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScDocCfg::GetLayoutPropertyNames()
{
    static const char* aPropNames[] =
    {
        "TabStop/NonMetric"         // SCDOCLAYOUTOPT_TABSTOP
    };
    Sequence<OUString> aNames(SCDOCLAYOUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCDOCLAYOUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    //  adjust for metric system
    if (ScOptionsUtil::IsMetricSystem())
        pNames[SCDOCLAYOUTOPT_TABSTOP] = OUString::createFromAscii( "TabStop/Metric" );

    return aNames;
}

ScDocCfg::ScDocCfg() :
    aCalcItem( OUString::createFromAscii( CFGPATH_CALC ) ),
    aLayoutItem( OUString::createFromAscii( CFGPATH_DOCLAYOUT ) )
{
    sal_Int32 nIntVal;
    double fDoubleVal;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    USHORT nDateDay, nDateMonth, nDateYear;
    GetDate( nDateDay, nDateMonth, nDateYear );

    aNames = GetCalcPropertyNames();
    aValues = aCalcItem.GetProperties(aNames);
    aCalcItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCCALCOPT_ITER_ITER:
                        SetIter( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_ITER_STEPS:
                        if (pValues[nProp] >>= nIntVal) SetIterCount( (USHORT) nIntVal );
                        break;
                    case SCCALCOPT_ITER_MINCHG:
                        if (pValues[nProp] >>= fDoubleVal) SetIterEps( fDoubleVal );
                        break;
                    case SCCALCOPT_DATE_DAY:
                        if (pValues[nProp] >>= nIntVal) nDateDay = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DATE_MONTH:
                        if (pValues[nProp] >>= nIntVal) nDateMonth = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DATE_YEAR:
                        if (pValues[nProp] >>= nIntVal) nDateYear = (USHORT) nIntVal;
                        break;
                    case SCCALCOPT_DECIMALS:
                        if (pValues[nProp] >>= nIntVal) SetStdPrecision( (USHORT) nIntVal );
                        break;
                    case SCCALCOPT_CASESENSITIVE:
                        // content is reversed
                        SetIgnoreCase( !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_PRECISION:
                        SetCalcAsShown( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_SEARCHCRIT:
                        SetMatchWholeCell( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCCALCOPT_FINDLABEL:
                        SetLookUpColRowNames( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    aCalcItem.SetCommitLink( LINK( this, ScDocCfg, CalcCommitHdl ) );

    SetDate( nDateDay, nDateMonth, nDateYear );

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCDOCLAYOUTOPT_TABSTOP:
                        // TabDistance in ScDocOptions is in twips
                        if (pValues[nProp] >>= nIntVal)
                            SetTabDistance( (USHORT) HMMToTwips( nIntVal ) );
                        break;
                }
            }
        }
    }
    aLayoutItem.SetCommitLink( LINK( this, ScDocCfg, LayoutCommitHdl ) );
}

IMPL_LINK( ScDocCfg, CalcCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetCalcPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    USHORT nDateDay, nDateMonth, nDateYear;
    GetDate( nDateDay, nDateMonth, nDateYear );

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCCALCOPT_ITER_ITER:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsIter() );
                break;
            case SCCALCOPT_ITER_STEPS:
                pValues[nProp] <<= (sal_Int32) GetIterCount();
                break;
            case SCCALCOPT_ITER_MINCHG:
                pValues[nProp] <<= (double) GetIterEps();
                break;
            case SCCALCOPT_DATE_DAY:
                pValues[nProp] <<= (sal_Int32) nDateDay;
                break;
            case SCCALCOPT_DATE_MONTH:
                pValues[nProp] <<= (sal_Int32) nDateMonth;
                break;
            case SCCALCOPT_DATE_YEAR:
                pValues[nProp] <<= (sal_Int32) nDateYear;
                break;
            case SCCALCOPT_DECIMALS:
                pValues[nProp] <<= (sal_Int32) GetStdPrecision();
                break;
            case SCCALCOPT_CASESENSITIVE:
                // content is reversed
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], !IsIgnoreCase() );
                break;
            case SCCALCOPT_PRECISION:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsCalcAsShown() );
                break;
            case SCCALCOPT_SEARCHCRIT:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsMatchWholeCell() );
                break;
            case SCCALCOPT_FINDLABEL:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], IsLookUpColRowNames() );
                break;
        }
    }
    aCalcItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK( ScDocCfg, LayoutCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCDOCLAYOUTOPT_TABSTOP:
                //  TabDistance in ScDocOptions is in twips
                //  use only even numbers, so defaults don't get changed
                //  by modifying other settings in the same config item
                pValues[nProp] <<= (sal_Int32) TwipsToEvenHMM( GetTabDistance() );
                break;
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);

    return 0;
}


void ScDocCfg::SetOptions( const ScDocOptions& rNew )
{
    *(ScDocOptions*)this = rNew;

    aCalcItem.SetModified();
    aLayoutItem.SetModified();
}


