/*************************************************************************
 *
 *  $RCSfile: docoptio.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

#include "cfgids.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "sc.hrc"

//------------------------------------------------------------------------

#define SC_VERSION ((USHORT)251)

TYPEINIT1(ScTpCalcItem, SfxPoolItem);

//------------------------------------------------------------------------

USHORT lcl_GetDefaultTabDist()
{
    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    if ( eSys == MEASURE_METRIC )
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
// CfgItem fuer Doc-Optionen
//==================================================================

ScDocCfg::ScDocCfg() : SfxConfigItem( SCCFG_DOC )
{
}

//------------------------------------------------------------------------

int __EXPORT ScDocCfg::Load( SvStream& rStream )
{
    USHORT  nVersion;

    rStream >> nVersion;
    ScDocOptions::Load( rStream );

    SetDefault( FALSE );

    return ( nVersion == SC_VERSION )
                ? SfxConfigItem::ERR_OK
                : SfxConfigItem::WARNING_VERSION;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScDocCfg::Store( SvStream& rStream)
{
    //  TRUE = alles speichern, unabhaengig von der Stream-Version
    //  (am Config-Stream ist noch SOFFICE_FILEFORMAT_40 eingestellt)

    rStream << SC_VERSION;
    ScDocOptions::Save( rStream, TRUE );

    SetDefault( FALSE );

    return SfxConfigItem::ERR_OK;
}

//------------------------------------------------------------------------

void __EXPORT ScDocCfg::UseDefault()
{
    ResetDocOptions();
    SetDefault( TRUE );
}

//------------------------------------------------------------------------

String __EXPORT ScDocCfg::GetName() const
{
    return String( ScResId( SCSTR_CFG_DOC ) );
}



