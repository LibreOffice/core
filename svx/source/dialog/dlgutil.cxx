/*************************************************************************
 *
 *  $RCSfile: dlgutil.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-09 11:36:50 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#include <limits.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif

#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>

#pragma hdrstop

#include "dialogs.hrc"

#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "langtab.hxx"

// -----------------------------------------------------------------------

String GetLanguageString( LanguageType eType )
{
    static const SvxLanguageTable aLangTable;
    return aLangTable.GetString( eType );
}

// -----------------------------------------------------------------------

String GetDicInfoStr( const String& rName, const USHORT nLang, const BOOL bNeg )
{
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( rName );
    String aTmp( aURLObj.GetBase() );
    aTmp += sal_Unicode( ' ' );

    if ( bNeg )
    {
        sal_Char const sTmp[] = " (-) ";
        aTmp.AppendAscii( sTmp );
    }

    aTmp += sal_Unicode( '[' );
    if ( LANGUAGE_NONE == nLang )
        aTmp += String( ResId( RID_SVXSTR_LANGUAGE_ALL, DIALOG_MGR() ) );
    else
        aTmp += ::GetLanguageString( (LanguageType)nLang );
    aTmp += sal_Unicode( ']' );

    return aTmp;
}

// -----------------------------------------------------------------------

void SetFieldUnit( MetricField& rField, FieldUnit eUnit, BOOL bAll )
{
    long nFirst = rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) );
    long nLast = rField.Denormalize( rField.GetLast( FUNIT_TWIP ) );
    long nMin = rField.Denormalize( rField.GetMin( FUNIT_TWIP ) );
    long nMax = rField.Denormalize( rField.GetMax( FUNIT_TWIP ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eUnit = FUNIT_CM;
                break;

            case FUNIT_FOOT:
            case FUNIT_MILE:
                eUnit = FUNIT_INCH;
                break;
        }
    }
    rField.SetUnit( eUnit );
    switch( eUnit )
    {
        case FUNIT_MM:
            rField.SetSpinSize( 50 );
            break;

        case FUNIT_INCH:
            rField.SetSpinSize( 2 );
            break;

        default:
            rField.SetSpinSize( 10 );
    }

    if ( FUNIT_POINT == eUnit && rField.GetDecimalDigits() > 1 )
        rField.SetDecimalDigits( 1 );
    else
        rField.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
        rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
        rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
        rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
    }
}

// -----------------------------------------------------------------------

void SetFieldUnit( MetricBox& rBox, FieldUnit eUnit, BOOL bAll )
{
    long nMin = rBox.Denormalize( rBox.GetMin( FUNIT_TWIP ) );
    long nMax = rBox.Denormalize( rBox.GetMax( FUNIT_TWIP ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eUnit = FUNIT_CM;
                break;

            case FUNIT_FOOT:
            case FUNIT_MILE:
                eUnit = FUNIT_INCH;
                break;
        }
    }
    rBox.SetUnit( eUnit );

    if ( FUNIT_POINT == eUnit && rBox.GetDecimalDigits() > 1 )
        rBox.SetDecimalDigits( 1 );
    else
        rBox.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rBox.SetMin( rBox.Normalize( nMin ), FUNIT_TWIP );
        rBox.SetMax( rBox.Normalize( nMax ), FUNIT_TWIP );
    }
}

// -----------------------------------------------------------------------

FieldUnit GetModuleFieldUnit()
{
    FieldUnit eUnit = FUNIT_INCH;
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    SfxObjectShell* pSh = NULL;
    if ( pFrame )
        pSh = pFrame->GetObjectShell();
    SfxModule* pModule = pSh ? pSh->GetModule() : NULL;
    if ( pModule )
    {
        const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
        if ( pItem )
            eUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
    }
    else
    {
        DBG_ERRORFILE( "GetModuleFieldUnit(): no module found" );
    }
    return eUnit;
}

// -----------------------------------------------------------------------

void SetMetricValue( MetricField& rField, long nCoreValue, SfxMapUnit eUnit )
{
    long nVal = OutputDevice::LogicToLogic( nCoreValue,
                                            (MapUnit)eUnit, MAP_100TH_MM );
    nVal = rField.Normalize( nVal );
    rField.SetValue( nVal, FUNIT_100TH_MM );

/*
    if ( SFX_MAPUNIT_100TH_MM == eUnit )
    {
        FieldUnit eFUnit = ( (MetricField&)rField ).GetUnit();
        USHORT nDigits = rField.GetDecimalDigits();

        if ( FUNIT_MM == eFUnit )
        {
            if ( 0 == nDigits )
                lCoreValue /= 100;
            else if ( 1 == nDigits )
                lCoreValue /= 10;
            else if ( nDigits > 2 )
            {
                DBG_ERROR( "too much decimal digits" );
                return;
            }
            rField.SetValue( lCoreValue, FUNIT_MM );
            return;
        }
        else if ( FUNIT_CM == eFUnit )
        {
            if ( 0 == nDigits )
                lCoreValue /= 1000;
            else if ( 1 == nDigits )
                lCoreValue /= 100;
            else if ( 2 == nDigits )
                lCoreValue /= 10;
            else if ( nDigits > 3 )
            {
                DBG_ERROR( "too much decimal digits" );
                return;
            }
            rField.SetValue( lCoreValue, FUNIT_CM );
            return;
        }
    }
    rField.SetValue( rField.Normalize(
        ConvertValueToUnit( lCoreValue, eUnit ) ), MapToFieldUnit( eUnit ) );
*/
}

// -----------------------------------------------------------------------

long GetCoreValue( const MetricField& rField, SfxMapUnit eUnit )
{
    long nVal = rField.GetValue( FUNIT_100TH_MM );
    long nUnitVal = OutputDevice::LogicToLogic( nVal, MAP_100TH_MM,
                                                (MapUnit)eUnit      );
    nUnitVal = rField.Denormalize( nUnitVal );
    return nUnitVal;

/*
    long nRet = rField.GetValue( MapToFieldUnit( eUnit ) );
    FieldUnit eFUnit = ( (MetricField&)rField ).GetUnit();
    USHORT nDigits = rField.GetDecimalDigits();
    DBG_ASSERT( nDigits <= 2, "decimal digits > 2!" );

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        {
            if ( 2 == nDigits )
                return nRet;
            else if ( 1 == nDigits )
                return nRet * 10;
            else
                return nRet * 100;
        }

        case SFX_MAPUNIT_TWIP:
        {
            if ( 2 == nDigits )
            {
                long nMod = 100;
                long nTmp = nRet % nMod;

                if ( nTmp >= 49 )
                    nRet += 100 - nTmp;
                return nRet / 100;
            }
            else if ( 1 == nDigits )
            {
                long nMod = 10;
                long nTmp = nRet % nMod;

                if ( nTmp >= 4 )
                    nRet += 10 - nTmp;
                return nRet / 10;
            }
            else
                return nRet;
        }

        default:
            DBG_ERROR( "this unit is not implemented" );
            return 0;
    }
*/

/*!!!
    return ConvertValueToMap( rField.Denormalize(
        rField.GetValue( MapToFieldUnit( eUnit ) ) ), eUnit );
*/
}

// -----------------------------------------------------------------------

long CalcToUnit( float nIn, SfxMapUnit eUnit )
{
    // nIn ist in Points

    DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP       ||
                eUnit == SFX_MAPUNIT_100TH_MM   ||
                eUnit == SFX_MAPUNIT_10TH_MM    ||
                eUnit == SFX_MAPUNIT_MM         ||
                eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );

    float nTmp = nIn;

    if ( SFX_MAPUNIT_TWIP != eUnit )
        nTmp = nIn * 10 / 567;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:  nTmp *= 100; break;
        case SFX_MAPUNIT_10TH_MM:   nTmp *= 10;  break;
        case SFX_MAPUNIT_MM:                     break;
        case SFX_MAPUNIT_CM:        nTmp /= 10;  break;
    }

    nTmp *= 20;
    long nRet = (long)nTmp;
    return nRet;
//! return (long)(nTmp * 20);
}

// -----------------------------------------------------------------------

long ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl )
{
    long nOut = 0;

    switch ( eItem )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
        {
            if ( eItem == SFX_MAPUNIT_10TH_MM )
                nIn /= 10;
            else if ( eItem == SFX_MAPUNIT_100TH_MM )
                nIn /= 100;
            nOut = TransformMetric( nIn, FUNIT_MM, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_CM:
        {
            nOut = TransformMetric( nIn, FUNIT_CM, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
        {
            if ( eItem == SFX_MAPUNIT_10TH_INCH )
                nIn /= 10;
            else if ( eItem == SFX_MAPUNIT_100TH_INCH )
                nIn /= 100;
            else if ( eItem == SFX_MAPUNIT_1000TH_INCH )
                nIn /= 1000;
            nOut = TransformMetric( nIn, FUNIT_INCH, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_POINT:
        {
            nOut = TransformMetric( nIn, FUNIT_POINT, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_TWIP:
        {
            nOut = TransformMetric( nIn, FUNIT_TWIP, (FieldUnit)eCtrl );
        }
        break;
    }
    return nOut;
}

// -----------------------------------------------------------------------

long ControlToItem( long nIn, SfxFieldUnit eCtrl, SfxMapUnit eItem )
{
    return ItemToControl( nIn, eItem, eCtrl );
}

// -----------------------------------------------------------------------

FieldUnit MapToFieldUnit( const SfxMapUnit eUnit )
{
    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
            return FUNIT_MM;

        case SFX_MAPUNIT_CM:
            return FUNIT_CM;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
            return FUNIT_INCH;

        case SFX_MAPUNIT_POINT:
            return FUNIT_POINT;

        case SFX_MAPUNIT_TWIP:
            return FUNIT_TWIP;
    }
    return FUNIT_NONE;
}

// -----------------------------------------------------------------------

MapUnit FieldToMapUnit( const SfxFieldUnit eUnit )
{
    return MAP_APPFONT;
}

// -----------------------------------------------------------------------

long ConvertValueToMap( long nVal, SfxMapUnit eUnit )
{
    long nNew = nVal;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_10TH_INCH:
            nNew *= 10;
            break;

        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_100TH_INCH:
            nNew *= 100;
            break;

        case SFX_MAPUNIT_1000TH_INCH:
            nNew *= 1000;
    }
    return nNew;
}

// -----------------------------------------------------------------------

long ConvertValueToUnit( long nVal, SfxMapUnit eUnit )
{
    long nNew = nVal;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_10TH_INCH:
            nNew /= 10;
            break;

        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_100TH_INCH:
            nNew /= 100;
            break;

        case SFX_MAPUNIT_1000TH_INCH:
            nNew /= 1000;
    }
    return nNew;
}

// -----------------------------------------------------------------------

long CalcToPoint( long nIn, SfxMapUnit eUnit, USHORT nFaktor )
{
    DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP       ||
                eUnit == SFX_MAPUNIT_100TH_MM   ||
                eUnit == SFX_MAPUNIT_10TH_MM    ||
                eUnit == SFX_MAPUNIT_MM         ||
                eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );

    long nRet = 0;

    if ( SFX_MAPUNIT_TWIP == eUnit )
        nRet = nIn;
    else
        nRet = nIn * 567;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:  nRet /= 100; break;
        case SFX_MAPUNIT_10TH_MM:   nRet /= 10;  break;
        case SFX_MAPUNIT_MM:                     break;
        case SFX_MAPUNIT_CM:        nRet *= 10;  break;
    }

    // ggf. aufrunden
    if ( SFX_MAPUNIT_TWIP != eUnit )
    {
        long nMod = 10;
        long nTmp = nRet % nMod;

        if ( nTmp >= 4 )
            nRet += 10 - nTmp;
        nRet /= 10;
    }
    return nRet * nFaktor / 20;
}

// -----------------------------------------------------------------------

long CMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567;
    return nRet;
}

// -----------------------------------------------------------------------

long MMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 10;
    return nRet;
}

// -----------------------------------------------------------------------

long InchToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 1440 ) && nIn >= ( LONG_MIN / 1440 ) )
        nRet = nIn * 1440;
    return nRet;
}

// -----------------------------------------------------------------------

long PointToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 20 ) && nIn >= ( LONG_MIN / 20 ) )
        nRet = nIn * 20;
    return nRet;
}

// -----------------------------------------------------------------------

long PicaToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 240 ) && nIn >= ( LONG_MIN / 240 ) )
        nRet = nIn * 240;
    return nRet;
}

// -----------------------------------------------------------------------

long TwipsToCM( long nIn )
{
    long nRet = nIn / 567;
    return nRet;
}

// -----------------------------------------------------------------------

long InchToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 100;
    return nRet;
}

// -----------------------------------------------------------------------

long MMToCM( long nIn )
{
    long nRet = nIn / 10;
    return nRet;
}

// -----------------------------------------------------------------------

long PointToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 20 ) && nIn >= ( LONG_MIN / 20 ) )
        nRet = nIn * 20 / 567;
    return nRet;
}

// -----------------------------------------------------------------------

long PicaToCM( long nIn)
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 20 ) && nIn >= ( LONG_MIN / 12 / 20 ) )
        nRet = nIn * 12 * 20 / 567;
    return nRet;
}

// -----------------------------------------------------------------------

long TwipsToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 566;
    return nRet;
}

// -----------------------------------------------------------------------

long CMToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10;
    return nRet;
}

// -----------------------------------------------------------------------

long InchToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 10;
    return nRet;
}

// -----------------------------------------------------------------------

long PointToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 200 ) && nIn >= ( LONG_MIN / 200 ) )
        nRet = nIn * 200 / 567;
    return nRet;
}

// -----------------------------------------------------------------------

long PicaToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 200 ) && nIn >= ( LONG_MIN / 12 / 200 ) )
        nRet = nIn * 12 * 200 / 567;
    return nRet;
}

// -----------------------------------------------------------------------

long TwipsToInch( long nIn )
{
    long nRet = nIn / 1440;
    return nRet;
}

// -----------------------------------------------------------------------

long CMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 100 ) && nIn >= ( LONG_MIN / 100 ) )
        nRet = nIn * 100 / 254;
    return nRet;
}

// -----------------------------------------------------------------------

long MMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 254;
    return nRet;
}

// -----------------------------------------------------------------------

long PointToInch( long nIn )
{
    long nRet = nIn / 72;
    return nRet;
}

// -----------------------------------------------------------------------

long PicaToInch( long nIn )
{
    long nRet = nIn / 6;
    return nRet;
}

// -----------------------------------------------------------------------

long TwipsToPoint( long nIn )
{
    long nRet = nIn / 20;
    return nRet;
}

// -----------------------------------------------------------------------

long InchToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 72 ) && nIn >= ( LONG_MIN / 72 ) )
        nRet = nIn * 72;
    return nRet;
}

// -----------------------------------------------------------------------

long CMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20;
    return nRet;
}

// -----------------------------------------------------------------------

long MMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200;
    return nRet;
}

// -----------------------------------------------------------------------

long PicaToPoint( long nIn )
{
    long nRet = nIn / 12;
    return nRet;
}

// -----------------------------------------------------------------------

long TwipsToPica( long nIn )
{
    long nRet = nIn / 240;
    return nRet;
}

// -----------------------------------------------------------------------

long InchToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 6 ) && nIn >= ( LONG_MIN / 6 ) )
        nRet = nIn * 6;
    return nRet;
}

// -----------------------------------------------------------------------

long PointToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 ) && nIn >= ( LONG_MIN / 12 ) )
        nRet = nIn * 12;
    return nRet;
}

// -----------------------------------------------------------------------

long CMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20 / 12;
    return nRet;
}

// -----------------------------------------------------------------------

long MMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200 / 12;
    return nRet;
}

// -----------------------------------------------------------------------

long Nothing( long nIn )
{
    long nRet = nIn;
    return nRet;
}

FUNC_CONVERT ConvertTable[6][6] =
{
//  CM,         MM          INCH         POINT        PICAS=32     TWIPS
    Nothing,    CMToMM,     CMToInch,    CMToPoint,   CMToPica,    CMToTwips,
    MMToCM,     Nothing,    MMToInch,    MMToPoint,   MMToPica,    MMToTwips,
    InchToCM,   InchToMM,   Nothing,     InchToPoint, InchToPica,  InchToTwips,
    PointToCM,  PointToMM,  PointToInch, Nothing,     PointToPica, PointToTwips,
    PicaToCM,   PicaToMM,   PicaToInch,  PicaToPoint, Nothing,     PicaToTwips,
    TwipsToCM,  TwipsToMM,  TwipsToInch, TwipsToPoint,TwipsToPica, Nothing
};

// -----------------------------------------------------------------------

long TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew )
{
    if ( aOld == FUNIT_NONE   || aNew == FUNIT_NONE ||
         aOld == FUNIT_CUSTOM || aNew == FUNIT_CUSTOM )
    {
        return nVal;
    }

    USHORT nOld = 0;
    USHORT nNew = 0;

    switch ( aOld )
    {
        case FUNIT_CM:
            nOld = 0; break;
        case FUNIT_MM:
            nOld = 1; break;
        case FUNIT_INCH:
            nOld = 2; break;
        case FUNIT_POINT:
            nOld = 3; break;
        case FUNIT_PICA:
            nOld = 4; break;
        case FUNIT_TWIP:
            nOld = 5; break;
    }

    switch ( aNew )
    {
        case FUNIT_CM:
            nNew = 0; break;
        case FUNIT_MM:
            nNew = 1; break;
        case FUNIT_INCH:
            nNew = 2; break;
        case FUNIT_POINT:
            nNew = 3; break;
        case FUNIT_PICA:
            nNew = 4; break;
        case FUNIT_TWIP:
            nNew = 5; break;
    }
    return ConvertTable[nOld][nNew]( nVal );
}


