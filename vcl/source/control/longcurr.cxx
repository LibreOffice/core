/*************************************************************************
 *
 *  $RCSfile: longcurr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
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

#define VCL_LONGCURR_CXX

#include <sot/object.hxx>
#define _TOOLS_BIGINT
#include <sot/factory.hxx>
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#include <event.hxx>
#include <svapp.hxx>
#include <svdata.hxx>
#include <longcurr.hxx>

#pragma hdrstop

// =======================================================================

#define FORMAT_LONGCURRENCY      4

// =======================================================================

static BigInt ImplPower10( USHORT n )
{
    USHORT i;
    BigInt   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

// -----------------------------------------------------------------------

static BOOL ImplNumericProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                        BOOL bStrictFormat,
                                        const International& rInter )
{
    if ( !bStrictFormat )
        return FALSE;
    else
    {
        sal_Unicode cChar = rKEvt.GetCharCode();
        USHORT      nGroup = rKEvt.GetKeyCode().GetGroup();

        if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC) ||
             ((cChar >= '0') && (cChar <= '9')) ||
             (rInter.IsNumThousandSep() && (cChar == rInter.GetNumThousandSep())) ||
             (cChar == rInter.GetNumDecimalSep()) ||
             (cChar == '-') )
            return FALSE;
        else
            return TRUE;
    }
}

// -----------------------------------------------------------------------

static BOOL ImplNumericGetValue( const XubString& rStr, BigInt& rValue,
                                 USHORT nDecDigits, const International& rInter,
                                 BOOL bCurrency = FALSE )
{
    XubString   aStr = rStr;
    XubString   aStr1;
    XubString   aStr2;
    USHORT      nDecPos;
    BOOL        bNegative = FALSE;
    xub_StrLen  i;

    // Reaktion auf leeren String
    if ( !rStr.Len() )
        return FALSE;

    // Fuehrende und nachfolgende Leerzeichen entfernen
    aStr.EraseLeadingAndTrailingChars( ' ' );

    // Position des Dezimalpunktes suchen
    nDecPos = aStr.Search( rInter.GetNumDecimalSep() );

    if ( nDecPos != STRING_NOTFOUND )
    {
        aStr1 = aStr.Copy( 0, nDecPos );
        aStr2 = aStr.Copy( nDecPos+1 );
    }
    else
        aStr1 = aStr;

    // Negativ ?
    if ( bCurrency )
    {
        if ( (aStr.GetChar( 0 ) == '(') && (aStr.GetChar( aStr.Len()-1 ) == ')') )
            bNegative = TRUE;
        if ( !bNegative )
        {
            for ( i=0; i < aStr.Len(); i++ )
            {
                if ( (aStr.GetChar( i ) >= '0') && (aStr.GetChar( i ) <= '9') )
                    break;
                else if ( aStr.GetChar( i ) == '-' )
                {
                    bNegative = TRUE;
                    break;
                }
            }
        }
        if ( !bNegative && bCurrency && aStr.Len() )
        {
            USHORT nFormat = rInter.GetCurrNegativeFormat();
            if ( (nFormat == 3) || (nFormat == 6)  ||
                 (nFormat == 7) || (nFormat == 10) )
            {
                for ( i = (USHORT)(aStr.Len()-1); i > 0; i++ )
                {
                    if ( (aStr.GetChar( i ) >= '0') && (aStr.GetChar( i ) <= '9') )
                        break;
                    else if ( aStr.GetChar( i ) == '-' )
                    {
                        bNegative = TRUE;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if ( aStr1.GetChar( 0 ) == '-' )
            bNegative = TRUE;
    }

    // Alle unerwuenschten Zeichen rauswerfen
    for ( i=0; i < aStr1.Len(); )
    {
        if ( (aStr1.GetChar( i ) >= '0') && (aStr1.GetChar( i ) <= '9') )
            i++;
        else
            aStr1.Erase( i, 1 );
    }
    for ( i=0; i < aStr2.Len(); )
    {
        if ( (aStr2.GetChar( i ) >= '0') && (aStr2.GetChar( i ) <= '9') )
            i++;
        else
            aStr2.Erase( i, 1 );
    }

    if ( !aStr1.Len() && !aStr2.Len() )
        return FALSE;

    if ( !aStr1.Len() )
        aStr1.Insert( '0' );
    if ( bNegative )
        aStr1.Insert( '-', 0 );

    // Nachkommateil zurechtstutzen und dabei runden
    BOOL bRound = FALSE;
    if ( aStr2.Len() > nDecDigits )
    {
        if ( aStr2.GetChar( nDecDigits ) >= '5' )
            bRound = TRUE;
        aStr2.Erase( nDecDigits );
    }
    if ( aStr2.Len() < nDecDigits )
        aStr2.Expand( nDecDigits, '0' );

    aStr  = aStr1;
    aStr += aStr2;

    // Bereichsueberpruefung
    BigInt nValue( aStr );
    if ( bRound )
    {
        if ( !bNegative )
            nValue+=1;
        else
            nValue-=1;
    }

    rValue = nValue;

    return TRUE;
}

// =======================================================================

static BOOL ImplLongCurrencyProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                             BOOL, const International& rInter )
{
    // Es gibt hier kein sinnvolles StrictFormat, also alle
    // Zeichen erlauben
    return ImplNumericProcessKeyInput( pEdit, rKEvt, FALSE, rInter );
}

// -----------------------------------------------------------------------

inline XubString ImplLongCurrencySetValue( BigInt nValue, USHORT nDecDigits,
                                           const International& rInter )
{
    // Umwandeln in einen Waehrungsstring
    return rInter.GetCurr( nValue, nDecDigits ); // ???
}

// -----------------------------------------------------------------------

inline BOOL ImplLongCurrencyGetValue( const XubString& rStr, BigInt& rValue,
                                      USHORT nDecDigits, const International& rInter )
{
    // Zahlenwert holen
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rInter, TRUE );
}

// -----------------------------------------------------------------------

BOOL ImplLongCurrencyReformat( const XubString& rStr, BigInt nMin, BigInt nMax,
                               USHORT nDecDigits,
                               const International& rInter, String& rOutStr,
                               LongCurrencyFormatter& rFormatter )
{
    BigInt nValue;
    if ( !ImplNumericGetValue( rStr, nValue, nDecDigits, rInter, TRUE ) )
        return TRUE;
    else
    {
        BigInt nTempVal = nValue;
        if ( nTempVal > nMax )
            nTempVal = nMax;
        else if ( nTempVal < nMin )
            nTempVal = nMin;

        if ( rFormatter.GetErrorHdl().IsSet() && (nValue != nTempVal) )
        {
            rFormatter.mnCorrectedValue = nTempVal;
            if ( !rFormatter.GetErrorHdl().Call( &rFormatter ) )
            {
                rFormatter.mnCorrectedValue = 0;
                return FALSE;
            }
            else
                rFormatter.mnCorrectedValue = 0;
        }

        rOutStr = rInter.GetCurr( nTempVal, nDecDigits );
        return TRUE;
    }
}

// =======================================================================

void LongCurrencyFormatter::ImpInit()
{
    mnFieldValue        = 0;
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = 0x7FFFFFFF;
    mnMax              *= 0x7FFFFFFF;
    mnCorrectedValue    = 0;
    mnType              = FORMAT_LONGCURRENCY;
    SetDecimalDigits( 0 );
}

// -----------------------------------------------------------------------

LongCurrencyFormatter::LongCurrencyFormatter()
{
    ImpInit();
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::ImplLoadRes( const ResId& rResId )
{
    ImpInit();

    ResMgr*     pMgr = Resource::GetResManager();
    USHORT      nMask = pMgr->ReadShort();

    if ( NUMERICFORMATTER_MIN & nMask )
        mnMin = pMgr->ReadLong();

    if ( NUMERICFORMATTER_MAX & nMask )
        mnMax = pMgr->ReadLong();

    if ( NUMERICFORMATTER_STRICTFORMAT & nMask )
        SetStrictFormat(  (BOOL)pMgr->ReadShort() );

    if ( NUMERICFORMATTER_I12 & nMask )
    {
        SetInternational( International( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
    }
    if ( NUMERICFORMATTER_DECIMALDIGITS & nMask )
        SetDecimalDigits( pMgr->ReadShort() );

    if ( NUMERICFORMATTER_VALUE & nMask )
    {
        mnFieldValue = pMgr->ReadLong();
        if ( mnFieldValue > mnMax )
            mnFieldValue = mnMax;
        else if ( mnFieldValue < mnMin )
            mnFieldValue = mnMin;
        mnLastValue = mnFieldValue;
    }
}

// -----------------------------------------------------------------------

LongCurrencyFormatter::~LongCurrencyFormatter()
{
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetValue( BigInt nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    ImplGetEmptyFieldValue() = FALSE;
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetUserValue( BigInt nNewValue )
{
    if ( nNewValue > mnMax )
        nNewValue = mnMax;
    else if ( nNewValue < mnMin )
        nNewValue = mnMin;
    mnLastValue = nNewValue;

    if ( !GetField() )
        return;

    XubString aStr = ImplLongCurrencySetValue( nNewValue, GetDecimalDigits(), GetInternational() );
    if ( GetField()->HasFocus() )
    {
        Selection aSelection = GetField()->GetSelection();
        GetField()->SetText( aStr );
        GetField()->SetSelection( aSelection );
    }
    else
        GetField()->SetText( aStr );
    MarkToBeReformatted( FALSE );
}

// -----------------------------------------------------------------------

BigInt LongCurrencyFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    BigInt nTempValue;
    if ( ImplLongCurrencyGetValue( GetField()->GetText(), nTempValue, GetDecimalDigits(),
                                   GetInternational() ) )
    {
        if ( nTempValue > mnMax )
            nTempValue = mnMax;
        else if ( nTempValue < mnMin )
            nTempValue = mnMin;
        return nTempValue;
    }
    else
        return mnLastValue;
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    BOOL bOK = ImplLongCurrencyReformat( GetField()->GetText(), mnMin, mnMax,
                                         GetDecimalDigits(), GetInternational(), aStr, *this );
    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        GetField()->SetText( aStr );
        MarkToBeReformatted( FALSE );
        ImplLongCurrencyGetValue( aStr, mnLastValue, GetDecimalDigits(), GetInternational() );
    }
    else
        SetValue( mnLastValue );
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::ReformatAll()
{
    Reformat();
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetMin( BigInt nNewMin )
{
    mnMin = nNewMin;
    ReformatAll();
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetMax( BigInt nNewMax )
{
    mnMax = nNewMax;
    ReformatAll();
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetDecimalDigits( USHORT nDigits )
{
    International aInter( GetInternational() );
    aInter.SetCurrDigits( nDigits );
    SetInternational( aInter );

//  ReformatAll(); // macht SetInternational()
}

// -----------------------------------------------------------------------

USHORT LongCurrencyFormatter::GetDecimalDigits() const
{
    return GetInternational().GetCurrDigits();
}

// -----------------------------------------------------------------------

BOOL LongCurrencyFormatter::IsValueModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyValue();
    else if ( GetValue() != mnFieldValue )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void LongCurrencyFormatter::SetEmptyValue()
{
    GetField()->SetText( ImplGetSVEmptyStr() );
    ImplGetEmptyFieldValue() = TRUE;
}

// -----------------------------------------------------------------------

BigInt LongCurrencyFormatter::Normalize( BigInt nValue ) const
{
    return (nValue * ImplPower10( GetDecimalDigits() ) );
}

// -----------------------------------------------------------------------

BigInt LongCurrencyFormatter::Denormalize( BigInt nValue ) const
{
    BigInt nFactor = ImplPower10( GetDecimalDigits() );
    BigInt nTmp    = nFactor;
    nTmp /= 2;
    nTmp += nValue;
    nTmp /= nFactor;
    return nTmp;
}

// =======================================================================

void ImplNewLongCurrencyFieldValue( LongCurrencyField* pField, BigInt nNewValue )
{
    Selection aSelect = pField->GetSelection();
    aSelect.Justify();
    XubString aText = pField->GetText();
    BOOL bLastSelected = ((xub_StrLen)aSelect.Max() == aText.Len()) ? TRUE : FALSE;

    BigInt nOldLastValue  = pField->mnLastValue;
    pField->SetUserValue( nNewValue );
    pField->mnLastValue  = nOldLastValue;

    if ( bLastSelected )
    {
        if ( !aSelect.Len() )
            aSelect.Min() = SELECTION_MAX;
        aSelect.Max() = SELECTION_MAX;
    }
    pField->SetSelection( aSelect );
    pField->SetModifyFlag();
    pField->Modify();
}

// =======================================================================

LongCurrencyField::LongCurrencyField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    mnSpinSize   = 1;
    mnFirst      = mnMin;
    mnLast       = mnMax;

    Reformat();
}

// -----------------------------------------------------------------------

LongCurrencyField::LongCurrencyField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_NUMERICFIELD )
{
    rResId.SetRT( RSC_NUMERICFIELD );
    WinBits nStyle = ImplInitRes( rResId ) ;
    SpinField::ImplInit( pParent, nStyle );

    SetField( this );
    mnSpinSize   = 1;
    mnFirst      = mnMin;
    mnLast       = mnMax;

    Reformat();

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void LongCurrencyField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    LongCurrencyFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT nMask = ReadShortRes();
    if ( CURRENCYFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( CURRENCYFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( CURRENCYFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();
}

// -----------------------------------------------------------------------

LongCurrencyField::~LongCurrencyField()
{
}

// -----------------------------------------------------------------------

long LongCurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }
    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long LongCurrencyField::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( FALSE );
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            Reformat();
            SpinField::Modify();
        }
    }
    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void LongCurrencyField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void LongCurrencyField::Up()
{
    BigInt nValue = GetValue();
    nValue += mnSpinSize;
    if ( nValue > mnMax )
        nValue = mnMax;

    ImplNewLongCurrencyFieldValue( this, nValue );
    SpinField::Up();
}

// -----------------------------------------------------------------------

void LongCurrencyField::Down()
{
    BigInt nValue = GetValue();
    nValue -= mnSpinSize;
    if ( nValue < mnMin )
        nValue = mnMin;

    ImplNewLongCurrencyFieldValue( this, nValue );
    SpinField::Down();
}

// -----------------------------------------------------------------------

void LongCurrencyField::First()
{
    ImplNewLongCurrencyFieldValue( this, mnFirst );
    SpinField::First();
}

// -----------------------------------------------------------------------

void LongCurrencyField::Last()
{
    ImplNewLongCurrencyFieldValue( this, mnLast );
    SpinField::Last();
}

// =======================================================================

LongCurrencyBox::LongCurrencyBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

LongCurrencyBox::LongCurrencyBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_NUMERICFIELD )
{
    SetField( this );
    WinBits nStyle = ImplInitRes( rResId ) ;
    ComboBox::ImplLoadRes( rResId );
    LongCurrencyFormatter::ImplLoadRes( rResId );
    Reformat();

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

LongCurrencyBox::~LongCurrencyBox()
{
}

// -----------------------------------------------------------------------

long LongCurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( ImplLongCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }
    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long LongCurrencyBox::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        MarkToBeReformatted( FALSE );
    }
    else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            Reformat();
            ComboBox::Modify();
        }
    }
    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void LongCurrencyBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void LongCurrencyBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplLongCurrencyReformat( GetEntry( i ), mnMin, mnMax,
                                  GetDecimalDigits(), GetInternational(),
                                  aStr, *this );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    LongCurrencyFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void LongCurrencyBox::InsertValue( BigInt nValue, USHORT nPos )
{
    XubString aStr = ImplLongCurrencySetValue( nValue, GetDecimalDigits(), GetInternational() );
    ComboBox::InsertEntry( aStr, nPos );
}

// -----------------------------------------------------------------------

void LongCurrencyBox::RemoveValue( BigInt nValue )
{
    XubString aStr = ImplLongCurrencySetValue( nValue, GetDecimalDigits(), GetInternational() );
    ComboBox::RemoveEntry( aStr );
}

// -----------------------------------------------------------------------

BigInt LongCurrencyBox::GetValue( USHORT nPos ) const
{
    BigInt nValue = 0;
    ImplLongCurrencyGetValue( ComboBox::GetEntry( nPos ), nValue,
                              GetDecimalDigits(), GetInternational() );
    return nValue;
}

// -----------------------------------------------------------------------

USHORT LongCurrencyBox::GetValuePos( BigInt nValue ) const
{
    XubString aStr = ImplLongCurrencySetValue( nValue, GetDecimalDigits(), GetInternational() );
    return ComboBox::GetEntryPos( aStr );
}

// =======================================================================

XubString International::GetCurr( const BigInt &rNumber, USHORT nDigits ) const
{
    DBG_ASSERT( nDigits < 10, "LongCurrency duerfen nur maximal 9 Nachkommastellen haben" );

    if ( rNumber.IsZero() || (long)rNumber )
        return GetCurr( (long)rNumber, nDigits );

    BigInt aTmp( ImplPower10( nDigits ) );
    BigInt aInteger( rNumber );
    aInteger.Abs();
    aInteger  /= aTmp;
    BigInt aFraction( rNumber );
    aFraction.Abs();
    aFraction %= aTmp;
    if ( !aInteger.IsZero() )
    {
        aFraction += aTmp;
        aTmp       = 1000000000L;
    }
    if ( rNumber.IsNeg() )
        aFraction *= -1;

    XubString aTemplate = GetCurr( (long)aFraction, nDigits );
    while( !aInteger.IsZero() )
    {
        aFraction  = aInteger;
        aFraction %= aTmp;
        aInteger  /= aTmp;
        if( !aInteger.IsZero() )
            aFraction += aTmp;

        XubString aFractionStr = GetNum( (long)aFraction, 0 );

        xub_StrLen nSPos = aTemplate.Search( '1' );
        if ( aFractionStr.Len() == 1 )
            aTemplate.SetChar( nSPos, aFractionStr.GetChar( 0 ) );
        else
        {
            aTemplate.Erase( nSPos, 1 );
            aTemplate.Insert( aFractionStr, nSPos );
        }
    }

    return aTemplate;
}
