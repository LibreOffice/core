/*************************************************************************
 *
 *  $RCSfile: field.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-03-09 14:57:22 $
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

#define _TOOLS_BIGINT
#define _SV_FIELD_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif

#include <field.hxx>
#include <event.hxx>
#include <svapp.hxx>
#include <svdata.hxx>

#pragma hdrstop

// -----------------------------------------------------------------------

#define FORMAT_NUMERIC       1
#define FORMAT_METRIC        2
#define FORMAT_CURRENCY      3

// -----------------------------------------------------------------------

static long ImplPower10( USHORT n )
{
    USHORT i;
    long   nValue = 1;

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
        xub_Unicode cChar = rKEvt.GetCharCode();
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

static BOOL ImplNumericGetValue( const XubString& rStr, double& rValue,
                                 USHORT nDecDigits, const International& rInter,
                                 BOOL bCurrency = FALSE )
{
    XubString   aStr = rStr;
    XubString   aStr1;
    XubString   aStr2;
    BOOL        bNegative = FALSE;
    xub_StrLen  nDecPos;
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
                for ( i = (xub_StrLen)(aStr.Len()-1); i > 0; i++ )
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
    double nValue = aStr.ToDouble();
    if ( bRound )
    {
        if ( !bNegative )
            nValue++;
        else
            nValue--;
    }

    rValue = nValue;

    return TRUE;
}

// -----------------------------------------------------------------------

FormatterBase::FormatterBase( Edit* pField )
{
    mpField                     = pField;
    mpInternational             = NULL;
    mbReformat                  = FALSE;
    mbStrictFormat              = FALSE;
    mbEmptyFieldValue           = FALSE;
    mbEmptyFieldValueEnabled    = FALSE;
}

// -----------------------------------------------------------------------

FormatterBase::~FormatterBase()
{
    delete mpInternational;
}

// -----------------------------------------------------------------------

void FormatterBase::Reformat()
{
}

// -----------------------------------------------------------------------

void FormatterBase::ReformatAll()
{
    Reformat();
};

// -----------------------------------------------------------------------

void FormatterBase::SetStrictFormat( BOOL bStrict )
{
    if ( bStrict != mbStrictFormat )
    {
        mbStrictFormat = bStrict;
        if ( mbStrictFormat )
            ReformatAll();
    }
}

// -----------------------------------------------------------------------

void FormatterBase::SetInternational( const International& rInternational )
{
    delete mpInternational;
    mpInternational = new International( rInternational );
    ReformatAll();
}

// -----------------------------------------------------------------------

const International& FormatterBase::GetInternational() const
{
    if ( !mpInternational )
    {
        if ( mpField )
            return mpField->GetSettings().GetInternational();
        else
            return Application::GetSettings().GetInternational();
    }

    return *mpInternational;
}

// -----------------------------------------------------------------------

const AllSettings& FormatterBase::GetFieldSettings() const
{
    if ( mpField )
        return mpField->GetSettings();
    else
        return Application::GetSettings();
}

// -----------------------------------------------------------------------

void FormatterBase::SetFieldText( const XubString& rText, BOOL bKeepSelection )
{
    if ( mpField )
    {
        Selection aNewSelection( 0xFFFF, 0xFFFF );
        if ( bKeepSelection )
            aNewSelection = mpField->GetSelection();

        ImplSetText( rText, &aNewSelection );
    }
}

// -----------------------------------------------------------------------

void FormatterBase::ImplSetText( const XubString& rText, Selection* pNewSelection )
{
    if ( mpField )
    {
        // !!! TH-18.2.99: Wenn wir Zeit haben sollte mal geklaert werden,
        // !!! warum SetText() intern bei gleichem Text nicht ImplSetSelection
        // !!! aufruft, sondern etwas anders macht, denn sehr haeufig kommt
        // !!! hier der gleiche Text an.

        // ggf. bleibt der Text gleich, aber die Selektion wird geaendert...
        BOOL bTextChanged = (mpField->GetText() != rText);

        if ( pNewSelection )
            mpField->SetText( rText, *pNewSelection );
        else
        {
            Selection aSel = mpField->GetSelection();
            aSel.Min() = aSel.Max();
            mpField->SetText( rText, aSel );
        }

        // !!! TH-18.2.99: Wenn wir Zeit haben sollte mal geklaert werden,
        // !!! warum hier der Modify-Handler gerufen wird !!!

        // !!! MT-8.7.99: Erstmal auskommentiert, koentest recht haben,
        // !!! Modify wird zu oft gerufen.
//      if ( MustBeReformatted() && bTextChanged )
//          mpField->Edit::Modify();    // Nur damit Modify-Hdl gerufen wird.

        MarkToBeReformatted( FALSE );
    }
}

// -----------------------------------------------------------------------

void FormatterBase::SetEmptyFieldValue()
{
    if ( mpField )
        mpField->SetText( ImplGetSVEmptyStr() );
    mbEmptyFieldValue = TRUE;
}

// -----------------------------------------------------------------------

BOOL FormatterBase::IsEmptyFieldValue() const
{
    return (!mpField || !mpField->GetText().Len());
}

// -----------------------------------------------------------------------

BOOL NumericFormatter::ImplNumericReformat( const XubString& rStr, double& rValue,
                                            XubString& rOutStr )
{
    if ( !ImplNumericGetValue( rStr, rValue, GetDecimalDigits(), GetInternational() ) )
        return TRUE;
    else
    {
        double nTempVal = rValue;
        if ( nTempVal > mnMax )
            nTempVal = mnMax;
        else if ( nTempVal < mnMin )
            nTempVal = mnMin;

        if ( GetErrorHdl().IsSet() && (rValue != nTempVal) )
        {
            mnCorrectedValue = (long)nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return FALSE;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = CreateFieldText( (long)nTempVal );
        return TRUE;
    }
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplInit()
{
    mnFieldValue        = 0;
    mnLastValue         = 0;
    mnMin               = 0;
    mnMax               = 0x7FFFFFFF;
    mnCorrectedValue    = 0;
    mnType              = FORMAT_NUMERIC;

    // Fuer Felder...
    mnSpinSize          = 1;
    mnFirst             = mnMin;
    mnLast              = mnMax;

    SetDecimalDigits( 0 );
}

// -----------------------------------------------------------------------

NumericFormatter::NumericFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplLoadRes( const ResId& rResId )
{
    ResMgr*     pMgr = Resource::GetResManager();
    USHORT      nMask;

    nMask = pMgr->ReadShort();

    if ( NUMERICFORMATTER_MIN & nMask )
        mnMin = pMgr->ReadLong();

    if ( NUMERICFORMATTER_MAX & nMask )
        mnMax = pMgr->ReadLong();

    if ( NUMERICFORMATTER_STRICTFORMAT & nMask )
        SetStrictFormat( (BOOL)pMgr->ReadShort() );

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

NumericFormatter::~NumericFormatter()
{
}

// -----------------------------------------------------------------------

void NumericFormatter::SetMin( long nNewMin )
{
    mnMin = nNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetMax( long nNewMax )
{
    mnMax = nNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetDecimalDigits( USHORT nDigits )
{
    International aInter( GetInternational() );
    aInter.SetCurrDigits( nDigits );
    SetInternational( aInter );
}

// -----------------------------------------------------------------------

USHORT NumericFormatter::GetDecimalDigits() const
{
    return GetInternational().GetCurrDigits();
}

// -----------------------------------------------------------------------

void NumericFormatter::SetValue( long nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( FALSE );
}

// -----------------------------------------------------------------------

XubString NumericFormatter::CreateFieldText( long nValue ) const
{
    return GetInternational().GetNum( nValue, GetDecimalDigits() );
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplSetUserValue( long nNewValue, Selection* pNewSelection )
{
    if ( nNewValue > mnMax )
        nNewValue = mnMax;
    else if ( nNewValue < mnMin )
        nNewValue = mnMin;
    mnLastValue = nNewValue;

    if ( GetField() )
        ImplSetText( CreateFieldText( nNewValue ), pNewSelection );
}

// -----------------------------------------------------------------------

void NumericFormatter::SetUserValue( long nNewValue )
{
    ImplSetUserValue( nNewValue );
}

// -----------------------------------------------------------------------

long NumericFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    double nTempValue;

    if ( ImplNumericGetValue( GetField()->GetText(), nTempValue,
                              GetDecimalDigits(), GetInternational() ) )
    {
        if ( nTempValue > mnMax )
            nTempValue = mnMax;
        else if ( nTempValue < mnMin )
            nTempValue = mnMin;
        return (long)nTempValue;
    }
    else
        return mnLastValue;
}

// -----------------------------------------------------------------------

BOOL NumericFormatter::IsValueModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyFieldValue();
    else if ( GetValue() != mnFieldValue )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

Fraction NumericFormatter::ConvertToFraction( long nValue )
{
    return Fraction( nValue, ImplPower10( GetDecimalDigits() ) );
}

// -----------------------------------------------------------------------

long NumericFormatter::ConvertToLong( const Fraction& rValue )
{
    Fraction aFract = rValue;
    aFract *= Fraction( ImplPower10( GetDecimalDigits() ) );
    return (long)aFract;
}

// -----------------------------------------------------------------------

long NumericFormatter::Normalize( long nValue ) const
{
    return (nValue * ImplPower10( GetDecimalDigits() ) );
}

// -----------------------------------------------------------------------

long NumericFormatter::Denormalize( long nValue ) const
{
    long nFactor = ImplPower10( GetDecimalDigits() );
    if( nValue < 0 )
        return ((nValue-(nFactor/2)) / nFactor );
    else
        return ((nValue+(nFactor/2)) / nFactor );
}

// -----------------------------------------------------------------------

void NumericFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    double nTemp = mnLastValue;
    BOOL bOK = ImplNumericReformat( GetField()->GetText(), nTemp, aStr );
    mnLastValue = (long)nTemp;
    if ( !bOK )
        return;

    if ( aStr.Len() )
        ImplSetText( aStr );
    else
        SetValue( mnLastValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldUp()
{
    long nValue = GetValue();
    nValue += mnSpinSize;
    if ( nValue > mnMax )
        nValue = mnMax;

    ImplNewFieldValue( nValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldDown()
{
    long nValue = GetValue();
    nValue -= mnSpinSize;
    if ( nValue < mnMin )
        nValue = mnMin;

    ImplNewFieldValue( nValue );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldFirst()
{
    ImplNewFieldValue( mnFirst );
}

// -----------------------------------------------------------------------

void NumericFormatter::FieldLast()
{
    ImplNewFieldValue( mnLast );
}

// -----------------------------------------------------------------------

void NumericFormatter::ImplNewFieldValue( long nNewValue )
{
    if ( GetField() )
    {
        // !!! TH-18.2.99: Wenn wir Zeit haben sollte mal geklaert werden,
        // !!! warum nicht bei ImplSetUserValue() geprueft wird, ob
        // !!! sich der Wert aendert. Denn auch hier muesste dieses
        // !!! gemacht werden, da ansonsten der Modify-Aufruf
        // !!! nicht gemacht werden duerfte. Jedenfalls sollten die
        // !!! Wege von ImplNewFieldValue, ImplSetUserValue und
        // !!! ImplSetText ueberprueft und klarer gestalltet (mit Kommentar)
        // !!! werden, damit wir mal wissen, was dort ablaeuft!!!

        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        XubString aText = GetField()->GetText();
        // Wenn bis ans Ende selektiert war, soll das auch so bleiben...
        if ( (xub_StrLen)aSelection.Max() == aText.Len() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        long nOldLastValue  = mnLastValue;
        ImplSetUserValue( nNewValue, &aSelection );
        mnLastValue = nOldLastValue;

        // Modify am Edit wird nur bei KeyInput gesetzt...
        if ( GetField()->GetText() != aText )
        {
            GetField()->SetModifyFlag();
            GetField()->Modify();
        }
    }
}

// -----------------------------------------------------------------------

NumericField::NumericField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

NumericField::NumericField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_NUMERICFIELD )
{
    rResId.SetRT( RSC_NUMERICFIELD );
    WinBits nStyle = ImplInitRes( rResId ) ;
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    ImplLoadRes( rResId );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

void NumericField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    NumericFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT      nMask = ReadShortRes();

    if ( NUMERICFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( NUMERICFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( NUMERICFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();
}

// -----------------------------------------------------------------------

NumericField::~NumericField()
{
}

// -----------------------------------------------------------------------

long NumericField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplNumericProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long NumericField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void NumericField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void NumericField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void NumericField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void NumericField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

void NumericField::Last()
{
    FieldLast();
    SpinField::Last();
}

// -----------------------------------------------------------------------

NumericBox::NumericBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

NumericBox::NumericBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_NUMERICBOX )
{
    rResId.SetRT( RSC_NUMERICBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    SetField( this );
    ComboBox::ImplLoadRes( rResId );
    NumericFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

NumericBox::~NumericBox()
{
}

// -----------------------------------------------------------------------

long NumericBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplNumericProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long NumericBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void NumericBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void NumericBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void NumericBox::ReformatAll()
{
    double nValue;
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplNumericReformat( GetEntry( i ), nValue, aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    NumericFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void NumericBox::InsertValue( long nValue, USHORT nPos )
{
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

// -----------------------------------------------------------------------

void NumericBox::RemoveValue( long nValue )
{
    ComboBox::RemoveEntry( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long NumericBox::GetValue( USHORT nPos ) const
{
    double nValue = 0;
    ImplNumericGetValue( ComboBox::GetEntry( nPos ), nValue,
                        GetDecimalDigits(), GetInternational() );
    return (long)nValue;
}

// -----------------------------------------------------------------------

USHORT NumericBox::GetValuePos( long nValue ) const
{
    return ComboBox::GetEntryPos( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long NumericBox::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return NumericFormatter::GetValue();
}

// -----------------------------------------------------------------------

static BOOL ImplMetricProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                       BOOL, const International& rInter )
{
    // Es gibt hier kein sinnvolles StrictFormat, also alle
    // Zeichen erlauben
    return ImplNumericProcessKeyInput( pEdit, rKEvt, FALSE, rInter );
}

// -----------------------------------------------------------------------

static XubString ImplMetricGetUnitText( const XubString& rStr )
{
    // Einheitentext holen
    XubString aStr;
    for ( short i = rStr.Len()-1; i >= 0; i-- )
    {
        xub_Unicode c = rStr.GetChar( i );
        if ( ((c >= 'A') && (c <= 'Z')) ||
             ((c >= 'a') && (c <= 'z')) ||
             (c == '\'') || (c == '\"') || (c == '%' ) )
            aStr.Insert( c, 0 );
        else
        {
            if ( aStr.Len() )
                break;
        }
    }

    return aStr;
}

// -----------------------------------------------------------------------

static FieldUnit ImplMetricGetUnit( const XubString& rStr )
{
    XubString aStr = ImplMetricGetUnitText( rStr );
    aStr.ToLowerAscii();

    if ( aStr.EqualsAscii( "mm" ) )             // Milimeter
        return FUNIT_MM;
    else if ( aStr.EqualsAscii( "cm" ) )        // Centimeter
        return FUNIT_CM;
    else if ( aStr.EqualsAscii( "m" ) )         // Meter
        return FUNIT_M;
    else if ( aStr.EqualsAscii( "km" ) )        // Km
        return FUNIT_KM;
    else if ( aStr.EqualsAscii( "twip" ) )      // Twips
        return FUNIT_TWIP;
    else if ( aStr.EqualsAscii( "twips" ) )     // Twips
        return FUNIT_TWIP;
    else if ( aStr.EqualsAscii( "pt" ) )        // Point
        return FUNIT_POINT;
    else if ( aStr.EqualsAscii( "pi" ) )        // Pica
        return FUNIT_PICA;
    else if ( aStr.EqualsAscii( "\"" ) )        // Inch
        return FUNIT_INCH;
    else if ( aStr.EqualsAscii( "in" ) )        // Inch
        return FUNIT_INCH;
    else if ( aStr.EqualsAscii( "inch" ) )      // Inch
        return FUNIT_INCH;
    else if ( aStr.EqualsAscii( "'" ) )         // Foot
        return FUNIT_FOOT;
    else if ( aStr.EqualsAscii( "ft" ) )        // Foot
        return FUNIT_FOOT;
    else if ( aStr.EqualsAscii( "foot" ) )      // Foot
        return FUNIT_FOOT;
    else if ( aStr.EqualsAscii( "feet" ) )      // Foot
        return FUNIT_FOOT;
    else if ( aStr.EqualsAscii( "mile" ) )      // Mile
        return FUNIT_MILE;
    else if ( aStr.EqualsAscii( "miles" ) )     // Mile
        return FUNIT_MILE;
    else if ( aStr.EqualsAscii( "%" ) )         // Percent
        return FUNIT_PERCENT;
    else
        return FUNIT_NONE;
}

#define K *1000L
#define M *1000000L
#define X *5280L

static const long aImplFactor[FUNIT_MILE+1][FUNIT_MILE+1] =
{ /*
mm/100    mm    cm       m     km  twip point  pica  inch    foot     mile */
{    1,  100,  1 K,  100 K, 100 M, 2540, 2540, 2540, 2540,2540*12,2540*12 X },
{    1,    1,   10,    1 K,   1 M, 2540, 2540, 2540, 2540,2540*12,2540*12 X },
{    1,    1,    1,    100, 100 K,  254,  254,  254,  254, 254*12, 254*12 X },
{    1,    1,    1,      1,   1 K,  254,  254,  254,  254, 254*12, 254*12 X },
{    1,    1,    1,      1,     1,    0,  254,  254,  254, 254*12, 254*12 X },
{ 1440,144 K,144 K,14400 K,     0,    1,   20,  240, 1440,1440*12,1440*12 X },
{   72, 7200, 7200,  720 K, 720 M,    1,    1,   12,   72,  72*12,  72*12 X },
{    6,  600,  600,   60 K,  60 M,    1,    1,    1,    6,   6*12,   6*12 X },
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,     12,     12 X },
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,      1,      1 X },
{    1,  100,  100,   10 K,  10 M,    1,    1,    1,    1,      1,        1 }
};

#undef X
#undef M
#undef K
// twip in km 254/14400 M

static FieldUnit eDefaultUnit = FUNIT_NONE;

FieldUnit MetricField::GetDefaultUnit() { return eDefaultUnit; }
void MetricField::SetDefaultUnit( FieldUnit meUnit ) { eDefaultUnit = meUnit; }

static FieldUnit ImplMap2FieldUnit( MapUnit meUnit, long& nDecDigits )
{
    switch( meUnit )
    {
        case MAP_100TH_MM :
            nDecDigits -= 2;
            return FUNIT_MM;
        case MAP_10TH_MM :
            nDecDigits -= 1;
            return FUNIT_MM;
        case MAP_MM :
            return FUNIT_MM;
        case MAP_CM :
            return FUNIT_CM;
        case MAP_1000TH_INCH :
            nDecDigits -= 3;
            return FUNIT_INCH;
        case MAP_100TH_INCH :
            nDecDigits -= 2;
            return FUNIT_INCH;
        case MAP_10TH_INCH :
            nDecDigits -= 1;
            return FUNIT_INCH;
        case MAP_INCH :
            return FUNIT_INCH;
        case MAP_POINT :
            return FUNIT_POINT;
        case MAP_TWIP :
            return FUNIT_TWIP;
        default:
            DBG_ERROR( "default eInUnit" );
            break;
    }
    return FUNIT_NONE;
}

// -----------------------------------------------------------------------

long MetricField::ConvertValue( long nValue, long mnBaseValue, USHORT nDecDigits,
                                FieldUnit eInUnit, FieldUnit eOutUnit )
{
    return (long)ConvertDoubleValue( nValue, mnBaseValue, nDecDigits,
                                    eInUnit, eOutUnit );
}

// -----------------------------------------------------------------------

long MetricField::ConvertValue( long nValue, USHORT nDigits,
                                MapUnit eInUnit, FieldUnit eOutUnit )
{
    return (long)ConvertDoubleValue( nValue, nDigits, eInUnit, eOutUnit );
}

// -----------------------------------------------------------------------

long MetricField::ConvertValue( long nValue, USHORT nDigits,
                                FieldUnit eInUnit, MapUnit eOutUnit )
{
    return (long)ConvertValue( nValue, nDigits, eInUnit, eOutUnit );
}

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, long mnBaseValue, USHORT nDecDigits,
                                        FieldUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eInUnit != eOutUnit )
    {
        long nMult, nDiv;

        if ( eInUnit == FUNIT_PERCENT )
        {
            if ( (mnBaseValue <= 0) || (nValue <= 0) )
                return nValue;
            nDiv = 100;
            for ( USHORT i=0; i < nDecDigits; i++ )
                nDiv *= 10;

            nMult = mnBaseValue;
        }
        else if ( eOutUnit == FUNIT_PERCENT ||
                  eOutUnit == FUNIT_CUSTOM ||
                  eOutUnit == FUNIT_NONE ||
                  eInUnit  == FUNIT_CUSTOM ||
                  eInUnit  == FUNIT_NONE )
             return nValue;
        else
        {
            if ( eOutUnit == FUNIT_100TH_MM )
                eOutUnit = FUNIT_NONE;
            if ( eInUnit == FUNIT_100TH_MM )
                eInUnit = FUNIT_NONE;

            nDiv  = aImplFactor[eInUnit][eOutUnit];
            nMult = aImplFactor[eOutUnit][eInUnit];

            DBG_ASSERT( nMult > 0, "illegal *" );
            DBG_ASSERT( nDiv  > 0, "illegal /" );
        }

        if ( nMult != 1 )
            nValue *= nMult;
        if ( nDiv != 1 )
        {
            nValue += ( nValue < 0 ) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }

    return nValue;
}

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, USHORT nDigits,
                                        MapUnit eInUnit, FieldUnit eOutUnit )
{
    if ( eOutUnit == FUNIT_PERCENT ||
         eOutUnit == FUNIT_CUSTOM ||
         eOutUnit == FUNIT_NONE ||
         eInUnit == MAP_PIXEL ||
         eInUnit == MAP_SYSFONT ||
         eInUnit == MAP_APPFONT ||
         eInUnit == MAP_RELATIVE )
    {
        DBG_ERROR( "invalid parameters" );
        return nValue;
    }

    long nDecDigits = nDigits;
    FieldUnit eFieldUnit = ImplMap2FieldUnit( eInUnit, nDecDigits );

    if ( (long)nDecDigits < 0 )
    {
        while ( nDecDigits )
        {
            nValue += 5;
            nValue /= 10;
            nDecDigits++;
        }
    }
    else
    {
        while ( nDecDigits )
        {
            nValue *= 10;
            nDecDigits--;
        }
    }

    if ( eFieldUnit != eOutUnit )
    {
        long nDiv  = aImplFactor[eFieldUnit][eOutUnit];
        long nMult = aImplFactor[eOutUnit][eFieldUnit];

        if ( nMult != 1 )
            nValue *= nMult;
        if ( nDiv != 1 )
        {
            nValue += (nValue < 0) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }
    return nValue;
}

// -----------------------------------------------------------------------

double MetricField::ConvertDoubleValue( double nValue, USHORT nDigits,
                                        FieldUnit eInUnit, MapUnit eOutUnit )
{
    if ( eInUnit == FUNIT_PERCENT ||
         eInUnit == FUNIT_CUSTOM ||
         eInUnit == FUNIT_NONE ||
         eOutUnit == MAP_PIXEL ||
         eOutUnit == MAP_SYSFONT ||
         eOutUnit == MAP_APPFONT ||
         eOutUnit == MAP_RELATIVE )
    {
        DBG_ERROR( "invalid parameters" );
        return nValue;
    }

    long nDecDigits = nDigits;
    FieldUnit eFieldUnit = ImplMap2FieldUnit( eOutUnit, nDecDigits );

    if ( (long)nDecDigits < 0 )
    {
        while ( nDecDigits )
        {
            nValue *= 10;
            nDecDigits++;
        }
    }
    else
    {
        while ( nDecDigits )
        {
            nValue += 5;
            nValue /= 10;
            nDecDigits--;
        }
    }

    if ( eFieldUnit != eInUnit )
    {
        long nDiv  = aImplFactor[eInUnit][eFieldUnit];
        long nMult = aImplFactor[eFieldUnit][eInUnit];

        if( nMult != 1 )
            nValue *= nMult;
        if( nDiv != 1 )
        {
            nValue += (nValue < 0) ? (-nDiv/2) : (nDiv/2);
            nValue /= nDiv;
        }
    }
    return nValue;
}

// -----------------------------------------------------------------------

static BOOL ImplMetricGetValue( const XubString& rStr, double& rValue, long nBaseValue,
                                USHORT nDecDigits,
                                const International& rInter, FieldUnit eUnit )
{
    // Zahlenwert holen
    if ( !ImplNumericGetValue( rStr, rValue, nDecDigits, rInter ) )
        return FALSE;

    // Einheit rausfinden
    FieldUnit eEntryUnit = ImplMetricGetUnit( rStr );

    // Einheiten umrechnen
    rValue = MetricField::ConvertDoubleValue( rValue, nBaseValue, nDecDigits, eEntryUnit, eUnit );

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL MetricFormatter::ImplMetricReformat( const XubString& rStr, double& rValue, XubString& rOutStr )
{
    if ( !ImplMetricGetValue( rStr, rValue, mnBaseValue, GetDecimalDigits(), GetInternational(), meUnit ) )
        return TRUE;
    else
    {
        double nTempVal = rValue;
        if ( nTempVal > GetMax() )
            nTempVal = GetMax();
        else if ( nTempVal < GetMin())
            nTempVal = GetMin();

        if ( GetErrorHdl().IsSet() && (rValue != nTempVal) )
        {
            mnCorrectedValue = (long)nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return FALSE;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = CreateFieldText( (long)nTempVal );
        return TRUE;
    }
}

// -----------------------------------------------------------------------

inline void MetricFormatter::ImplInit()
{
    mnBaseValue = 0;
    meUnit = MetricField::GetDefaultUnit();
    mnType = FORMAT_METRIC;
}

// -----------------------------------------------------------------------

MetricFormatter::MetricFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

void MetricFormatter::ImplLoadRes( const ResId& rResId )
{
    NumericFormatter::ImplLoadRes( rResId );

    ResMgr*     pMgr = Resource::GetResManager();
    USHORT      nMask = pMgr->ReadShort();

    if ( METRICFORMATTER_UNIT & nMask )
        meUnit = (FieldUnit)pMgr->ReadShort();

    if ( METRICFORMATTER_CUSTOMUNITTEXT & nMask )
        maCustomUnitText = pMgr->ReadString();
}

// -----------------------------------------------------------------------

MetricFormatter::~MetricFormatter()
{
}

// -----------------------------------------------------------------------

void MetricFormatter::SetUnit( FieldUnit eNewUnit )
{
    if ( eNewUnit == FUNIT_100TH_MM )
    {
        SetDecimalDigits( GetDecimalDigits() + 2 );
        meUnit = FUNIT_MM;
    }
    else
        meUnit = eNewUnit;
    ReformatAll();
}

// -----------------------------------------------------------------------

void MetricFormatter::SetCustomUnitText( const XubString& rStr )
{
    maCustomUnitText = rStr;
    ReformatAll();
}

// -----------------------------------------------------------------------

void MetricFormatter::SetValue( long nNewValue, FieldUnit eInUnit )
{
    SetUserValue( nNewValue, eInUnit );
    mnFieldValue = mnLastValue;
}

// -----------------------------------------------------------------------

XubString MetricFormatter::CreateFieldText( long nValue ) const
{
    XubString aStr = NumericFormatter::CreateFieldText( nValue );

    // Einheit dranhaengen
    switch ( meUnit )
    {
        case FUNIT_MM:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "mm" ) );
            break;
        case FUNIT_CM:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "cm" ) );
            break;
        case FUNIT_M:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "m" ) );
            break;
        case FUNIT_KM:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "km" ) );
            break;
        case FUNIT_TWIP:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "twips" ) );
            break;
        case FUNIT_POINT:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "pt" ) );
            break;
        case FUNIT_PICA:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "pi" ) );
            break;
        case FUNIT_INCH:
            aStr.Append( '"' );
            break;
        case FUNIT_FOOT:
            aStr.Append( '\'' );
            break;
        case FUNIT_MILE:
            aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "miles" ) );
            break;
        case FUNIT_CUSTOM:
            aStr += maCustomUnitText;
            break;
        default:
            break;
    }

    return aStr;
}

// -----------------------------------------------------------------------

void MetricFormatter::SetUserValue( long nNewValue, FieldUnit eInUnit )
{
    // Umrechnen auf eingestellte Einheiten
    nNewValue = MetricField::ConvertValue( nNewValue, mnBaseValue, GetDecimalDigits(), eInUnit, meUnit );
    NumericFormatter::SetUserValue( nNewValue );
}

// -----------------------------------------------------------------------

long MetricFormatter::GetValue( FieldUnit eOutUnit ) const
{
    if ( !GetField() )
        return 0;

    double nTempValue;
    if ( !ImplMetricGetValue( GetField()->GetText(), nTempValue, mnBaseValue, GetDecimalDigits(),
                              GetInternational(), meUnit ) )
        nTempValue = mnLastValue;

    if ( nTempValue > mnMax )
        nTempValue = mnMax;
    else if ( nTempValue < mnMin )
        nTempValue = mnMin;

    // Umrechnen auf gewuenschte Einheiten
    return MetricField::ConvertValue( (long)nTempValue, mnBaseValue, GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetValue( long nValue )
{
    // Implementation not inline, because it is a virtual Function
    SetValue( nValue, FUNIT_NONE );
}

// -----------------------------------------------------------------------

long MetricFormatter::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetMin( long nNewMin, FieldUnit eInUnit )
{
    // Umrechnen auf gewuenschte Einheiten
    NumericFormatter::SetMin( MetricField::ConvertValue( nNewMin, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

// -----------------------------------------------------------------------

long MetricFormatter::GetMin( FieldUnit eOutUnit ) const
{
    // Umrechnen auf gewuenschte Einheiten
    return MetricField::ConvertValue( NumericFormatter::GetMin(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetMax( long nNewMax, FieldUnit eInUnit )
{
    // Umrechnen auf gewuenschte Einheiten
    NumericFormatter::SetMax( MetricField::ConvertValue( nNewMax, mnBaseValue, GetDecimalDigits(),
                                                         eInUnit, meUnit ) );
}

// -----------------------------------------------------------------------

long MetricFormatter::GetMax( FieldUnit eOutUnit ) const
{
    // Umrechnen auf gewuenschte Einheiten
    return MetricField::ConvertValue( NumericFormatter::GetMax(), mnBaseValue,
                                      GetDecimalDigits(), meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::SetBaseValue( long nNewBase, FieldUnit eInUnit )
{
    mnBaseValue = MetricField::ConvertValue( nNewBase, mnBaseValue, GetDecimalDigits(),
                                             eInUnit, meUnit );
}

// -----------------------------------------------------------------------

long MetricFormatter::GetBaseValue( FieldUnit eOutUnit ) const
{
    // Umrechnen auf gewuenschte Einheiten
    return MetricField::ConvertValue( mnBaseValue, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricFormatter::Reformat()
{
    if ( !GetField() )
        return;

    XubString aText = GetField()->GetText();
    if ( meUnit == FUNIT_CUSTOM )
        maCurUnitText = ImplMetricGetUnitText( aText );

    XubString aStr;
    double nTemp = mnLastValue;
    BOOL bOK = ImplMetricReformat( aText, nTemp, aStr );
    mnLastValue = (long)nTemp;

    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr );
        if ( meUnit == FUNIT_CUSTOM )
            CustomConvert();
    }
    else
        SetValue( mnLastValue );
    maCurUnitText.Erase();
}

// -----------------------------------------------------------------------

long MetricFormatter::GetCorrectedValue( FieldUnit eOutUnit ) const
{
    // Umrechnen auf gewuenschte Einheiten
    return MetricField::ConvertValue( mnCorrectedValue, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

MetricField::MetricField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

MetricField::MetricField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_METRICFIELD )
{
    rResId.SetRT( RSC_METRICFIELD );
    WinBits nStyle = ImplInitRes( rResId ) ;
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

void MetricField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    MetricFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT      nMask = ReadShortRes();

    if ( METRICFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( METRICFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( METRICFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();

    Reformat();
}

// -----------------------------------------------------------------------

MetricField::~MetricField()
{
}

// -----------------------------------------------------------------------

void MetricField::SetFirst( long nNewFirst, FieldUnit eInUnit )
{
    // Umrechnen
    nNewFirst = MetricField::ConvertValue( nNewFirst, mnBaseValue, GetDecimalDigits(),
                                           eInUnit, meUnit );
    mnFirst = nNewFirst;
}

// -----------------------------------------------------------------------

long MetricField::GetFirst( FieldUnit eOutUnit ) const
{
    // Umrechnen
    return MetricField::ConvertValue( mnFirst, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

void MetricField::SetLast( long nNewLast, FieldUnit eInUnit )
{
    // Umrechnen
    nNewLast = MetricField::ConvertValue( nNewLast, mnBaseValue, GetDecimalDigits(),
                                          eInUnit, meUnit );
    mnLast = nNewLast;
}

// -----------------------------------------------------------------------

long MetricField::GetLast( FieldUnit eOutUnit ) const
{
    // Umrechnen
    return MetricField::ConvertValue( mnLast, mnBaseValue, GetDecimalDigits(),
                                      meUnit, eOutUnit );
}

// -----------------------------------------------------------------------

long MetricField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplMetricProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long MetricField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void MetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void MetricField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void MetricField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void MetricField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void MetricField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

void MetricField::Last()
{
    FieldLast();
    SpinField::Last();
}

// -----------------------------------------------------------------------

void MetricField::CustomConvert()
{
    maCustomConvertLink.Call( this );
}

// -----------------------------------------------------------------------

MetricBox::MetricBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

MetricBox::MetricBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_METRICBOX )
{
    rResId.SetRT( RSC_METRICBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    SetField( this );
    Reformat();
    ComboBox::ImplLoadRes( rResId );
    MetricFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

MetricBox::~MetricBox()
{
}

// -----------------------------------------------------------------------

long MetricBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplMetricProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long MetricBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void MetricBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void MetricBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void MetricBox::ReformatAll()
{
    double nValue;
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplMetricReformat( GetEntry( i ), nValue, aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    MetricFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void MetricBox::CustomConvert()
{
    maCustomConvertLink.Call( this );
}

// -----------------------------------------------------------------------

void MetricBox::InsertValue( long nValue, FieldUnit eInUnit, USHORT nPos )
{
    // Umrechnen auf eingestellte Einheiten
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

// -----------------------------------------------------------------------

void MetricBox::RemoveValue( long nValue, FieldUnit eInUnit )
{
    // Umrechnen auf eingestellte Einheiten
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    ComboBox::RemoveEntry( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long MetricBox::GetValue( USHORT nPos, FieldUnit eOutUnit ) const
{
    double nValue = 0;
    ImplMetricGetValue( ComboBox::GetEntry( nPos ), nValue, mnBaseValue,
                        GetDecimalDigits(), GetInternational(), meUnit );

    // Umrechnen auf eingestellte Einheiten
    long nRetValue = MetricField::ConvertValue( (long)nValue, mnBaseValue, GetDecimalDigits(),
                                                meUnit, eOutUnit );

    return nRetValue;
}

// -----------------------------------------------------------------------

USHORT MetricBox::GetValuePos( long nValue, FieldUnit eInUnit ) const
{
    // Umrechnen auf eingestellte Einheiten
    nValue = MetricField::ConvertValue( nValue, mnBaseValue, GetDecimalDigits(),
                                        eInUnit, meUnit );
    return ComboBox::GetEntryPos( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long MetricBox::GetValue( FieldUnit eOutUnit ) const
{
    // Implementation not inline, because it is a virtual Function
    return MetricFormatter::GetValue( eOutUnit );
}

// -----------------------------------------------------------------------

long MetricBox::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return GetValue( FUNIT_NONE );
}

// -----------------------------------------------------------------------

static BOOL ImplCurrencyProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                         BOOL, const International& rInter )
{
    // Es gibt hier kein sinnvolles StrictFormat, also alle
    // Zeichen erlauben
    return ImplNumericProcessKeyInput( pEdit, rKEvt, FALSE, rInter );
}

// -----------------------------------------------------------------------

inline BOOL ImplCurrencyGetValue( const XubString& rStr, double& rValue,
                                  USHORT nDecDigits, const International& rInter )
{
    // Zahlenwert holen
    return ImplNumericGetValue( rStr, rValue, nDecDigits, rInter, TRUE );
}

// -----------------------------------------------------------------------

BOOL CurrencyFormatter::ImplCurrencyReformat( const XubString& rStr,
                                              XubString& rOutStr )
{
    double nValue;
    if ( !ImplNumericGetValue( rStr, nValue, GetDecimalDigits(), GetInternational(), TRUE ) )
        return TRUE;
    else
    {
        double nTempVal = nValue;
        if ( nTempVal > GetMax() )
            nTempVal = GetMax();
        else if ( nTempVal < GetMin())
            nTempVal = GetMin();

        if ( GetErrorHdl().IsSet() && (nValue != nTempVal) )
        {
            mnCorrectedValue = (long)nTempVal;
            if ( !GetErrorHdl().Call( this ) )
            {
                mnCorrectedValue = 0;
                return FALSE;
            }
            else
                mnCorrectedValue = 0;
        }

        rOutStr = GetInternational().GetCurr( (long)nTempVal );
        return TRUE;
    }
}

// -----------------------------------------------------------------------

inline void CurrencyFormatter::ImplInit()
{
    mnType = FORMAT_CURRENCY;
}

// -----------------------------------------------------------------------

CurrencyFormatter::CurrencyFormatter()
{
    ImplInit();
}

// -----------------------------------------------------------------------

CurrencyFormatter::~CurrencyFormatter()
{
}

// -----------------------------------------------------------------------

void CurrencyFormatter::SetValue( long nNewValue )
{
    SetUserValue( nNewValue );
    mnFieldValue = mnLastValue;
    SetEmptyFieldValueData( FALSE );
}

// -----------------------------------------------------------------------

XubString CurrencyFormatter::CreateFieldText( long nValue ) const
{
    return GetInternational().GetCurr( nValue, GetDecimalDigits() );
}

// -----------------------------------------------------------------------

long CurrencyFormatter::GetValue() const
{
    if ( !GetField() )
        return 0;

    double nTempValue;
    if ( ImplCurrencyGetValue( GetField()->GetText(), nTempValue, GetDecimalDigits(),
                               GetInternational() ) )
    {
        if ( nTempValue > mnMax )
            nTempValue = mnMax;
        else if ( nTempValue < mnMin )
            nTempValue = mnMin;
        return (long)nTempValue;
    }
    else
        return mnLastValue;
}

// -----------------------------------------------------------------------

void CurrencyFormatter::Reformat()
{
    if ( !GetField() )
        return;

    XubString aStr;
    BOOL bOK = ImplCurrencyReformat( GetField()->GetText(), aStr );
    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr  );
        double nTemp = mnLastValue;
        ImplCurrencyGetValue( aStr, nTemp, GetDecimalDigits(), GetInternational() );
        mnLastValue = (long)nTemp;
    }
    else
        SetValue( mnLastValue );
}

// -----------------------------------------------------------------------

CurrencyField::CurrencyField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

CurrencyField::CurrencyField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_CURRENCYFIELD )
{
    rResId.SetRT( RSC_CURRENCYFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    SpinField::ImplInit( pParent, nStyle);
    SetField( this );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

void CurrencyField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    CurrencyFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT      nMask = ReadShortRes();

    if ( CURRENCYFIELD_FIRST & nMask )
        mnFirst = ReadLongRes();

    if ( CURRENCYFIELD_LAST & nMask )
        mnLast = ReadLongRes();

    if ( CURRENCYFIELD_SPINSIZE & nMask )
        mnSpinSize = ReadLongRes();

    Reformat();
}

// -----------------------------------------------------------------------

CurrencyField::~CurrencyField()
{
}

// -----------------------------------------------------------------------

long CurrencyField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long CurrencyField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void CurrencyField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void CurrencyField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void CurrencyField::Up()
{
    FieldUp();
    SpinField::Up();
}

// -----------------------------------------------------------------------

void CurrencyField::Down()
{
    FieldDown();
    SpinField::Down();
}

// -----------------------------------------------------------------------

void CurrencyField::First()
{
    FieldFirst();
    SpinField::First();
}

// -----------------------------------------------------------------------

void CurrencyField::Last()
{
    FieldLast();
    SpinField::Last();
}

// -----------------------------------------------------------------------

CurrencyBox::CurrencyBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

CurrencyBox::CurrencyBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_CURRENCYBOX )
{
    rResId.SetRT( RSC_CURRENCYBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    CurrencyFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    SetField( this );
    ComboBox::ImplLoadRes( rResId );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

CurrencyBox::~CurrencyBox()
{
}

// -----------------------------------------------------------------------

long CurrencyBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplCurrencyProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), GetInternational() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long CurrencyBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void CurrencyBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void CurrencyBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void CurrencyBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplCurrencyReformat( GetEntry( i ), aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    CurrencyFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void CurrencyBox::InsertValue( long nValue, USHORT nPos )
{
    ComboBox::InsertEntry( CreateFieldText( nValue ), nPos );
}

// -----------------------------------------------------------------------

void CurrencyBox::RemoveValue( long nValue )
{
    ComboBox::RemoveEntry( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long CurrencyBox::GetValue( USHORT nPos ) const
{
    double nValue = 0;
    ImplCurrencyGetValue( ComboBox::GetEntry( nPos ), nValue,
                          GetDecimalDigits(), GetInternational() );
    return (long)nValue;
}

// -----------------------------------------------------------------------

USHORT CurrencyBox::GetValuePos( long nValue ) const
{
    return ComboBox::GetEntryPos( CreateFieldText( nValue ) );
}

// -----------------------------------------------------------------------

long CurrencyBox::GetValue() const
{
    // Implementation not inline, because it is a virtual Function
    return CurrencyFormatter::GetValue();
}
