/*************************************************************************
 *
 *  $RCSfile: fmtfield.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#include <stdio.h>

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _ZFORMAT_HXX //autogen
#include "zformat.hxx"
#endif

#ifndef _FMTFIELD_HXX_
#include "fmtfield.hxx"
#endif // _FMTFIELD_HXX_

//==============================================================================
// regulaerer Ausdruck, um komplette Zahlen und alles, was waehrend der Eingabe einer kompletten Zahl als Fragment vorkommt,
// abzudecken :
// [+/-][{ziffer}*.]*{ziffer}*[,{ziffer}*][e[+/-]{ziffer}*]
const char __FAR_DATA szNumericInput[] = "_[\\x2D\\x2B]?([0-9]*\\x++)*[0-9]*(\\x##[0-9]*)?(e[\\x2D\\x2B]?[0-9]*)?_";
    // (die beiden _ sind fuer die Normierung, damit kann ich erzwingen, dass nie ein Teilstring gefunden wird)

//==============================================================================
SvNumberFormatter* FormattedField::StaticFormatter::s_cFormatter = NULL;
ULONG FormattedField::StaticFormatter::s_nReferences = 0;

//------------------------------------------------------------------------------
SvNumberFormatter* FormattedField::StaticFormatter::GetFormatter()
{
    if (!s_cFormatter)
        s_cFormatter = new SvNumberFormatter(Application::GetAppInternational().GetLanguage());
    return s_cFormatter;
}

//------------------------------------------------------------------------------
FormattedField::StaticFormatter::StaticFormatter()
{
    ++s_nReferences;
}

//------------------------------------------------------------------------------
FormattedField::StaticFormatter::~StaticFormatter()
{
    if (--s_nReferences == 0)
    {
        delete s_cFormatter;
        s_cFormatter = NULL;
    }
}

//==============================================================================
DBG_NAME(FormattedField);

#define INIT_MEMBERS()              \
     m_aLastSelection(0,0)          \
    ,m_bStrictFormat(TRUE)          \
    ,m_nFormatKey(0)                \
    ,m_pFormatter(NULL)             \
    ,m_dMinValue(0)                 \
    ,m_dMaxValue(0)                 \
    ,m_bHasMin(FALSE)               \
    ,m_bHasMax(FALSE)               \
    ,m_dCurrentValue(0)             \
    ,m_bValueDirty(TRUE)            \
    ,m_bEnableEmptyField(TRUE)      \
    ,m_dDefaultValue(0)             \
    ,m_dSpinSize(1)                 \
    ,m_dSpinFirst(-1000000)         \
    ,m_dSpinLast(1000000)           \
    ,m_bTreatAsNumber(TRUE)         \
    ,m_pLastOutputColor(NULL)

//------------------------------------------------------------------------------
FormattedField::FormattedField(Window* pParent, WinBits nStyle, SvNumberFormatter* pInitialFormatter, INT32 nFormatKey)
    :SpinField(pParent, nStyle)
    ,INIT_MEMBERS()
{
    DBG_CTOR(FormattedField, NULL);

    if (pInitialFormatter)
    {
        m_pFormatter = pInitialFormatter;
        m_nFormatKey = nFormatKey;
    }
}

//------------------------------------------------------------------------------
FormattedField::FormattedField(Window* pParent, const ResId& rResId, SvNumberFormatter* pInitialFormatter, INT32 nFormatKey)
    :SpinField(pParent, rResId)
    ,INIT_MEMBERS()
{
    DBG_CTOR(FormattedField, NULL);

    if (pInitialFormatter)
    {
        m_pFormatter = pInitialFormatter;
        m_nFormatKey = nFormatKey;
    }
}

//------------------------------------------------------------------------------
FormattedField::~FormattedField()
{
    DBG_DTOR(FormattedField, NULL);
}

//------------------------------------------------------------------------------
void FormattedField::SetValidateText(const XubString& rText, const String* pErrorText)
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (CheckText(rText))
        SetText(rText);
    else
        if (pErrorText)
            ImplSetText(*pErrorText, NULL);
        else
            ImplSetValue(m_dDefaultValue, TRUE);
}

//------------------------------------------------------------------------------
void FormattedField::SetText(const XubString& rStr)
{
    DBG_CHKTHIS(FormattedField, NULL);

    SpinField::SetText(rStr);
    m_bValueDirty = TRUE;
}

//------------------------------------------------------------------------------
void FormattedField::SetTextFormatted(const XubString& rStr)
{
    DBG_CHKTHIS(FormattedField, NULL);

#if DBG_UTIL
    if (ImplGetFormatter()->IsTextFormat(m_nFormatKey))
         DBG_WARNING("FormattedField::SetTextFormatted : valid only with text formats !");
#endif

    m_sCurrentTextValue = rStr;

    String sFormatted;
    ImplGetFormatter()->GetOutputString(m_sCurrentTextValue, m_nFormatKey, sFormatted, &m_pLastOutputColor);

    // calculate the new selection
    Selection aSel(GetSelection());
    Selection aNewSel(aSel);
    aNewSel.Justify();
    USHORT nNewLen = sFormatted.Len();
    USHORT nCurrentLen = GetText().Len();
    if ((nNewLen > nCurrentLen) && (aNewSel.Max() == nCurrentLen))
    {   // the new text is longer and the cursor was behind the last char (of the old text)
        if (aNewSel.Min() == 0)
        {   // the whole text was selected -> select the new text on the whole, too
            aNewSel.Max() = nNewLen;
            if (!nCurrentLen)
            {   // there wasn't really a previous selection (as there was no previous text), we're setting a new one -> check the selection options
                ULONG nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
                if (nSelOptions & SELECTION_OPTION_SHOWFIRST)
                {   // selection should be from right to left -> swap min and max
                    aNewSel.Min() = aNewSel.Max();
                    aNewSel.Max() = 0;
                }
            }
        }
        else if (aNewSel.Max() == aNewSel.Min())
        {   // there was no selection -> set the cursor behind the new last char
            aNewSel.Max() = nNewLen;
            aNewSel.Min() = nNewLen;
        }
    }
    else if (aNewSel.Max() > nNewLen)
        aNewSel.Max() = nNewLen;
    else
        aNewSel = aSel; // don't use the justified version
    SpinField::SetText(sFormatted, aNewSel);
    m_bValueDirty = FALSE;
}

//------------------------------------------------------------------------------
String FormattedField::GetTextValue() const
{
    if (m_bValueDirty)
    {
        ((FormattedField*)this)->m_sCurrentTextValue = GetText();
        ((FormattedField*)this)->m_bValueDirty = FALSE;
    }
    return m_sCurrentTextValue;
}

//------------------------------------------------------------------------------
void FormattedField::Modify()
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (!IsStrictFormat())
    {
        m_bValueDirty = TRUE;
        SpinField::Modify();
        return;
    }

    String sCheck = GetText();
    if (CheckText(sCheck))
    {
        m_sLastValidText = sCheck;
        m_aLastSelection = GetSelection();
        m_bValueDirty = TRUE;
    }
    else
    {
        ImplSetText(m_sLastValidText, &m_aLastSelection);
    }

    SpinField::Modify();
}

//------------------------------------------------------------------------------
void FormattedField::ImplSetText(const XubString& rNew, Selection* pNewSel)
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (pNewSel)
        SpinField::SetText(rNew, *pNewSel);
    else
    {
        Selection aSel(GetSelection());
        aSel.Justify();

        USHORT nNewLen = rNew.Len();
        USHORT nCurrentLen = GetText().Len();

        if ((nNewLen > nCurrentLen) && (aSel.Max() == nCurrentLen))
        {   // new new text is longer and the cursor is behind the last char
            if (aSel.Min() == 0)
            {   // the whole text was selected -> select the new text on the whole, too
                aSel.Max() = nNewLen;
                if (!nCurrentLen)
                {   // there wasn't really a previous selection (as there was no previous text), we're setting a new one -> check the selection options
                    ULONG nSelOptions = GetSettings().GetStyleSettings().GetSelectionOptions();
                    if (nSelOptions & SELECTION_OPTION_SHOWFIRST)
                    {   // selection should be from right to left -> swap min and max
                        aSel.Min() = aSel.Max();
                        aSel.Max() = 0;
                    }
                }
            }
            else if (aSel.Max() == aSel.Min())
            {   // there was no selection -> set the cursor behind the new last char
                aSel.Max() = nNewLen;
                aSel.Min() = nNewLen;
            }
        }
        else if (aSel.Max() > nNewLen)
            aSel.Max() = nNewLen;
        SpinField::SetText(rNew, aSel);
    }

    m_bValueDirty = TRUE;
        // muss nicht stimmen, aber sicherheitshalber ...
}

//------------------------------------------------------------------------------
long FormattedField::PreNotify(NotifyEvent& rNEvt)
{
    DBG_CHKTHIS(FormattedField, NULL);
    if (rNEvt.GetType() == EVENT_KEYINPUT)
        m_aLastSelection = GetSelection();
    return SpinField::PreNotify(rNEvt);
}

//------------------------------------------------------------------------------
void FormattedField::ImplSetFormatKey(ULONG nFormatKey)
{
    DBG_CHKTHIS(FormattedField, NULL);

    m_nFormatKey = nFormatKey;
    BOOL bNeedFormatter = (m_pFormatter == NULL) && (nFormatKey != 0);
    if (bNeedFormatter)
    {
        ImplGetFormatter();     // damit wird ein Standard-Formatter angelegt

        m_nFormatKey = nFormatKey;
            // kann sein, dass das in dem Standard-Formatter keinen Sinn macht, aber der nimmt dann ein Default-Format an.
            // Auf diese Weise kann ich einfach einen der - formatteruebergreifended gleichen - Standard-Keys setzen.
        DBG_ASSERT(m_pFormatter->GetEntry(nFormatKey) != NULL, "FormattedField::ImplSetFormatKey : invalid format key !");
            // Wenn SetFormatKey aufgerufen wird, ohne dass ein Formatter existiert, muss der Key einer der Standard-Werte
            // sein, der in allen Formattern (also auch in meinem neu angelegten) vorhanden ist.
    }
}

//------------------------------------------------------------------------------
void FormattedField::SetFormatKey(ULONG nFormatKey)
{
    DBG_CHKTHIS(FormattedField, NULL);
    BOOL bNoFormatter = (m_pFormatter == NULL);
    ImplSetFormatKey(nFormatKey);
    FormatChanged((bNoFormatter && (m_pFormatter != NULL)) ? FCT_FORMATTER : FCT_KEYONLY);
}

//------------------------------------------------------------------------------
void FormattedField::SetFormatter(SvNumberFormatter* pFormatter, BOOL bResetFormat)
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (bResetFormat)
    {
        m_pFormatter = pFormatter;
        m_nFormatKey = 0;
    }
    else
    {
        XubString sOldFormat;
        LanguageType aOldLang;
        GetFormat(sOldFormat, aOldLang);

        ULONG nDestKey = pFormatter->TestNewString(sOldFormat);
        if (nDestKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            // die Sprache des neuen Formatters
            const SvNumberformat* pDefaultEntry = pFormatter->GetEntry(0);
            LanguageType aNewLang = pDefaultEntry ? pDefaultEntry->GetLanguage() : LANGUAGE_DONTKNOW;

            // den alten Format-String in die neue Sprache konvertieren
            USHORT nCheckPos;
            short nType;
            pFormatter->PutandConvertEntry(sOldFormat, nCheckPos, nType, nDestKey, aOldLang, aNewLang);
            m_nFormatKey = nDestKey;
        }
        m_pFormatter = pFormatter;
    }

    FormatChanged(FCT_FORMATTER);
}

//------------------------------------------------------------------------------
void FormattedField::GetFormat(XubString& rFormatString, LanguageType& eLang) const
{
    DBG_CHKTHIS(FormattedField, NULL);
    const SvNumberformat* pFormatEntry = ImplGetFormatter()->GetEntry(m_nFormatKey);
    DBG_ASSERT(pFormatEntry != NULL, "FormattedField::ResetConformanceTester : no number format for the given format key.");
    rFormatString = pFormatEntry ? pFormatEntry->GetFormatstring() : XubString();
    eLang = pFormatEntry ? pFormatEntry->GetLanguage() : LANGUAGE_DONTKNOW;
}

//------------------------------------------------------------------------------
BOOL FormattedField::SetFormat(const XubString& rFormatString, LanguageType eLang)
{
    DBG_CHKTHIS(FormattedField, NULL);
    ULONG nNewKey = ImplGetFormatter()->TestNewString(rFormatString, eLang);
    if (nNewKey == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        USHORT nCheckPos;
        short nType;
        XubString rFormat(rFormatString);
        if (!ImplGetFormatter()->PutEntry(rFormat, nCheckPos, nType, nNewKey))
            return FALSE;
        DBG_ASSERT(nNewKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "FormattedField::SetFormatString : PutEntry returned an invalid key !");
    }

    if (nNewKey != m_nFormatKey)
        SetFormatKey(nNewKey);
    return TRUE;
}

//------------------------------------------------------------------------------
BOOL FormattedField::GetThousandsSep() const
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::GetThousandsSep : your'e sure what your'e doing when setting the precision of a text format ?");

    BOOL bThousand, IsRed;
    USHORT nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);

    return bThousand;
}

//------------------------------------------------------------------------------
void FormattedField::SetThousandsSep(BOOL _bUseSeparator)
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::SetThousandsSep : your'e sure what your'e doing when setting the precision of a text format ?");

    // get the current settings
    BOOL bThousand, IsRed;
    USHORT nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);
    if (bThousand == _bUseSeparator)
        return;

    // we need the language for the following
    LanguageType eLang;
    String sFmtDescription;
    GetFormat(sFmtDescription, eLang);

    // generate a new format ...
    ImplGetFormatter()->GenerateFormat(sFmtDescription, m_nFormatKey, eLang, _bUseSeparator, IsRed, nPrecision, nAnzLeading);
    // ... and introduce it to the formatter
    USHORT nCheckPos;
    ULONG  nNewKey;
    short nType;
    ImplGetFormatter()->PutEntry(sFmtDescription, nCheckPos, nType, nNewKey, eLang);

    // set the new key
    ImplSetFormatKey(nNewKey);
    FormatChanged(FCT_THOUSANDSSEP);
}

//------------------------------------------------------------------------------
USHORT FormattedField::GetDecimalDigits() const
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::GetDecimalDigits : your'e sure what your'e doing when setting the precision of a text format ?");

    BOOL bThousand, IsRed;
    USHORT nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);

    return nPrecision;
}

//------------------------------------------------------------------------------
void FormattedField::SetDecimalDigits(USHORT _nPrecision)
{
    DBG_ASSERT(!ImplGetFormatter()->IsTextFormat(m_nFormatKey),
        "FormattedField::SetDecimalDigits : your'e sure what your'e doing when setting the precision of a text format ?");

    // get the current settings
    BOOL bThousand, IsRed;
    USHORT nPrecision, nAnzLeading;
    ImplGetFormatter()->GetFormatSpecialInfo(m_nFormatKey, bThousand, IsRed, nPrecision, nAnzLeading);
    if (nPrecision == _nPrecision)
        return;

    // we need the language for the following
    LanguageType eLang;
    String sFmtDescription;
    GetFormat(sFmtDescription, eLang);

    // generate a new format ...
    ImplGetFormatter()->GenerateFormat(sFmtDescription, m_nFormatKey, eLang, bThousand, IsRed, _nPrecision, nAnzLeading);
    // ... and introduce it to the formatter
    USHORT nCheckPos;
    ULONG nNewKey;
    short nType;
    ImplGetFormatter()->PutEntry(sFmtDescription, nCheckPos, nType, nNewKey, eLang);

    // set the new key
    ImplSetFormatKey(nNewKey);
    FormatChanged(FCT_PRECISION);
}

//------------------------------------------------------------------------------
void FormattedField::FormatChanged(FORMAT_CHANGE_TYPE /*nWhat*/)
{
    DBG_CHKTHIS(FormattedField, NULL);
    m_pLastOutputColor = NULL;
    ReFormat();
}

//------------------------------------------------------------------------------
void FormattedField::ReFormat()
{
    if (!IsEmptyFieldEnabled() || GetText().Len())
        if (TreatingAsNumber())
            ImplSetValue(GetValue(), TRUE);
        else
            SetTextFormatted(GetTextValue());
}

//------------------------------------------------------------------------------
long FormattedField::Notify(NotifyEvent& rNEvt)
{
    DBG_CHKTHIS(FormattedField, NULL);

    if ((rNEvt.GetType() == EVENT_KEYINPUT) && !IsReadOnly())
    {
        const KeyEvent& rKEvt = *rNEvt.GetKeyEvent();
        USHORT nMod = rKEvt.GetKeyCode().GetModifier();
        switch ( rKEvt.GetKeyCode().GetCode() )
        {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_PAGEUP:
            case KEY_PAGEDOWN:
                if (!nMod && ImplGetFormatter()->IsTextFormat(m_nFormatKey))
                {
                    // the base class would translate this into calls to Up/Down/First/Last,
                    // but we don't want this if we are text-formatted
                    return 1;
                }
        }
    }

    if ((rNEvt.GetType() == EVENT_COMMAND) && !IsReadOnly())
    {
        const CommandEvent* pCommand = rNEvt.GetCommandEvent();
        if (pCommand->GetCommand() == COMMAND_WHEEL)
        {
            const CommandWheelData* pData = rNEvt.GetCommandEvent()->GetWheelData();
            if ((pData->GetMode() == COMMAND_WHEEL_SCROLL) && ImplGetFormatter()->IsTextFormat(m_nFormatKey))
            {
                // same as above : prevent the base class from doing Up/Down-calls
                // (normally I should put this test into the Up/Down methods itself, shouldn't I ?)
                // FS - 71553 - 19.01.00
                return 1;
            }
        }
    }

    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
    {
        // Sonderbehandlung fuer leere Texte
        if (GetText().Len() == 0)
        {
            if (!IsEmptyFieldEnabled())
            {
                if (TreatingAsNumber())
                {
                    ImplSetValue(m_dCurrentValue, TRUE);
                    Modify();
                }
                else
                {
                    String sNew = GetTextValue();
                    if (sNew.Len())
                        SetTextFormatted(sNew);
                    else
                        SetTextFormatted(m_sDefaultText);
                }
                m_bValueDirty = FALSE;
            }
        }
        else
        {
            String sOld = GetText();
            ReFormat();
            if (GetText() != sOld)
                Modify();
        }
    }

    return SpinField::Notify( rNEvt );
}

//------------------------------------------------------------------------------
void FormattedField::SetMinValue(double dMin)
{
    DBG_CHKTHIS(FormattedField, NULL);
    DBG_ASSERT(m_bTreatAsNumber, "FormattedField::SetMinValue : only to be used in numeric mode !");

    m_dMinValue = dMin;
    m_bHasMin = TRUE;
    // fuer die Ueberpruefung des aktuellen Wertes an der neuen Grenze -> ImplSetValue
    ReFormat();
}

//------------------------------------------------------------------------------
void FormattedField::SetMaxValue(double dMax)
{
    DBG_CHKTHIS(FormattedField, NULL);
    DBG_ASSERT(m_bTreatAsNumber, "FormattedField::SetMaxValue : only to be used in numeric mode !");

    m_dMaxValue = dMax;
    m_bHasMax = TRUE;
    // fuer die Ueberpruefung des aktuellen Wertes an der neuen Grenze -> ImplSetValue
    ReFormat();
}

//------------------------------------------------------------------------------
void FormattedField::SetTextValue(const XubString& rText)
{
    DBG_CHKTHIS(FormattedField, NULL);
    SetText(rText);
    ReFormat();
}

//------------------------------------------------------------------------------
void FormattedField::EnableEmptyField(BOOL bEnable)
{
    DBG_CHKTHIS(FormattedField, NULL);
    if (bEnable == m_bEnableEmptyField)
        return;

    m_bEnableEmptyField = bEnable;
    if (!m_bEnableEmptyField && GetText().Len()==0)
        ImplSetValue(m_dCurrentValue, TRUE);
}

//------------------------------------------------------------------------------
void FormattedField::ImplSetValue(double dVal, BOOL bForce)
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (m_bHasMin && (dVal<m_dMinValue))
        dVal = m_dMinValue;
    if (m_bHasMax && (dVal>m_dMaxValue))
        dVal = m_dMaxValue;
    if (!bForce && (dVal == GetValue()))
        return;

    DBG_ASSERT(ImplGetFormatter() != NULL, "FormattedField::ImplSetValue : can't set a value without a formatter !");

    m_bValueDirty = FALSE;
    m_dCurrentValue = dVal;

    String sNewText;
    if (ImplGetFormatter()->IsTextFormat(m_nFormatKey))
    {
        // zuerst die Zahl als String im Standard-Format
        String sTemp;
        ImplGetFormatter()->GetOutputString(dVal, 0, sTemp, &m_pLastOutputColor);
        // dann den String entsprechend dem Text-Format
        ImplGetFormatter()->GetOutputString(sTemp, m_nFormatKey, sNewText, &m_pLastOutputColor);
    }
    else
    {
        ImplGetFormatter()->GetOutputString(dVal, m_nFormatKey, sNewText, &m_pLastOutputColor);
    }

    ImplSetText(sNewText, NULL);
    m_bValueDirty = FALSE;
    DBG_ASSERT(CheckText(sNewText), "FormattedField::ImplSetValue : formatted string doesn't match the criteria !");
}

//------------------------------------------------------------------------------
BOOL FormattedField::ImplGetValue(double& dNewVal)
{
    DBG_CHKTHIS(FormattedField, NULL);

    dNewVal = m_dCurrentValue;
    if (!m_bValueDirty)
        return TRUE;

    dNewVal = m_dDefaultValue;
    String sText(GetText());
    if (!sText.Len())
        return TRUE;

    DBG_ASSERT(ImplGetFormatter() != NULL, "FormattedField::ImplGetValue : can't give you a current value without a formatter !");

    ULONG nFormatKey = m_nFormatKey;    // IsNumberFormat veraendert den FormatKey ...

    if (ImplGetFormatter()->IsTextFormat(nFormatKey) && m_bTreatAsNumber)
        // damit wir in einem als Text formatierten Feld trotzdem eine Eingabe wie '1,1' erkennen ...
        nFormatKey = 0;

    // Sonderbehandlung fuer %-Formatierung
    if (ImplGetFormatter()->GetType(m_nFormatKey) == NUMBERFORMAT_PERCENT)
    {
        ULONG nTempFormat = 0;
        double dTemp;
        if (m_pFormatter->IsNumberFormat(sText, nTempFormat, dTemp) &&
            NUMBERFORMAT_NUMBER == m_pFormatter->GetType(nTempFormat))
            // der String entspricht einer Number-Formatierung, hat also nur kein %
            // -> dranhaengen
            sText += '%';
        // (damit wird aus einer Eingabe '3' ein '3%', und der Formatter macht dann daraus
        // ein double 0.03. Ansonsten wuerden wir hier bei einer Eingabe von '3' ein double
        // 3 zurueckliefern, was 300 Prozent entspricht ...
    }
    if (!ImplGetFormatter()->IsNumberFormat(sText, nFormatKey, dNewVal))
        return FALSE;


    if (m_bHasMin && (dNewVal<m_dMinValue))
        dNewVal = m_dMinValue;
    if (m_bHasMax && (dNewVal>m_dMaxValue))
        dNewVal = m_dMaxValue;
    return TRUE;
}

//------------------------------------------------------------------------------
void FormattedField::SetValue(double dVal)
{
    DBG_CHKTHIS(FormattedField, NULL);
    ImplSetValue(dVal, m_bValueDirty);
}

//------------------------------------------------------------------------------
double FormattedField::GetValue()
{
    DBG_CHKTHIS(FormattedField, NULL);

    if (!ImplGetValue(m_dCurrentValue))
        m_dCurrentValue = m_dDefaultValue;

    m_bValueDirty = FALSE;
    return m_dCurrentValue;
}

//------------------------------------------------------------------------------
void FormattedField::Up()
{
    DBG_CHKTHIS(FormattedField, NULL);
    SetValue(GetValue() + m_dSpinSize);
        // das setValue handelt Bereichsueberschreitungen (min/max) automatisch
    SetModifyFlag();
    Modify();
}

//------------------------------------------------------------------------------
void FormattedField::Down()
{
    DBG_CHKTHIS(FormattedField, NULL);
    SetValue(GetValue() - m_dSpinSize);
    SetModifyFlag();
    Modify();
}

//------------------------------------------------------------------------------
void FormattedField::First()
{
    DBG_CHKTHIS(FormattedField, NULL);
    if (m_bHasMin)
    {
        SetValue(m_dMinValue);
        SetModifyFlag();
        Modify();
    }
}

//------------------------------------------------------------------------------
void FormattedField::Last()
{
    DBG_CHKTHIS(FormattedField, NULL);
    if (m_bHasMax)
    {
        SetValue(m_dMaxValue);
        SetModifyFlag();
        Modify();
    }
}

//==============================================================================
//------------------------------------------------------------------------------
DoubleNumericField::~DoubleNumericField()
{
    delete m_pConformanceTester;
}

//------------------------------------------------------------------------------
void DoubleNumericField::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    ResetConformanceTester();
    FormattedField::FormatChanged(nWhat);
}

//------------------------------------------------------------------------------
BOOL DoubleNumericField::CheckText(const XubString& sText) const
{
    // Ich wuerde das CheckText gern ueber den NumberFormatter laufen lassen (da gibt es schliesslich ein IsNumberFormat),
    // aber der erkennt leider keine Fragmente (also zum Beispiel '1e', was waehrend der Eingabe zwangslaeufig irgendwann mal
    // vorkommt), also dieser Umweg ueber einen SearchText.

    if (!sText.Len())
        return TRUE;

    String sForceComplete = '_';
    sForceComplete += sText;
    sForceComplete += '_';

    USHORT nStart = 0, nEnd = sForceComplete.Len();
    BOOL bFound = m_pConformanceTester->SearchFrwrd(sForceComplete, &nStart, &nEnd);

    if (bFound && (nStart == 0) && (nEnd == sForceComplete.Len() - 1))
        return TRUE;

    return FALSE;
}

//------------------------------------------------------------------------------
void DoubleNumericField::ResetConformanceTester()
{
    String sDescription = String::CreateFromAscii(szNumericInput);
    // the thousands and the decimal separator are language dependent
    const SvNumberformat* pFormatEntry = ImplGetFormatter()->GetEntry(m_nFormatKey);
    char cSepHexCode[3];

    unsigned char cSeparator = pFormatEntry ? International(pFormatEntry->GetLanguage()).GetNumThousandSep() : ',';
    sprintf(cSepHexCode, "%0X", cSeparator);
    sDescription.SearchAndReplaceAscii("++", String::CreateFromAscii(cSepHexCode));

    cSeparator = pFormatEntry ? International(pFormatEntry->GetLanguage()).GetNumDecimalSep() : '.';
    sprintf(cSepHexCode, "%0X", cSeparator);
    sDescription.SearchAndReplaceAscii("##", String::CreateFromAscii(cSepHexCode));

    delete m_pConformanceTester;
    m_pConformanceTester = new SearchText(SearchParam(sDescription, SearchParam::SRCH_REGEXP, FALSE), GetpApp()->GetAppInternational());
}


//==============================================================================

//------------------------------------------------------------------------------
DoubleCurrencyField::DoubleCurrencyField(Window* pParent, WinBits nStyle)
    :FormattedField(pParent, nStyle)
    ,m_bChangingFormat(FALSE)
{
    m_bPrependCurrSym = FALSE;

    // initialize with a system currency format
    SvNumberFormatter* pFormatter = StandardFormatter();
    const NfCurrencyEntry* pSystemCurrency = pFormatter->MatchSystemCurrency();
    if (!pSystemCurrency)
        pSystemCurrency = &pFormatter->GetCurrencyEntry(LANGUAGE_SYSTEM);
    if (pSystemCurrency)
        m_sCurrencySymbol = pSystemCurrency->GetSymbol();
    else
        // no more options ...
        m_sCurrencySymbol.AssignAscii("DM");
    UpdateCurrencyFormat();
}

//------------------------------------------------------------------------------
DoubleCurrencyField::DoubleCurrencyField(Window* pParent, const ResId& rResId)
    :FormattedField(pParent, rResId)
    ,m_bChangingFormat(FALSE)
{
    m_bPrependCurrSym = FALSE;

    // initialize with a system currency format
    SvNumberFormatter* pFormatter = StandardFormatter();
    const NfCurrencyEntry* pSystemCurrency = pFormatter->MatchSystemCurrency();
    if (!pSystemCurrency)
        pSystemCurrency = &pFormatter->GetCurrencyEntry(LANGUAGE_SYSTEM);
    if (pSystemCurrency)
        m_sCurrencySymbol = pSystemCurrency->GetSymbol();
    else
        // no more options ...
        m_sCurrencySymbol.AssignAscii("DM");
    UpdateCurrencyFormat();
}

//------------------------------------------------------------------------------
void DoubleCurrencyField::FormatChanged(FORMAT_CHANGE_TYPE nWhat)
{
    if (m_bChangingFormat)
    {
        FormattedField::FormatChanged(nWhat);
        return;
    }

    switch (nWhat)
    {
        case FCT_FORMATTER:
        case FCT_PRECISION:
        case FCT_THOUSANDSSEP:
            // the aspects which changed don't take our currency settings into account (in fact, they most probably
            // destroyed them)
            UpdateCurrencyFormat();
            break;
        case FCT_KEYONLY:
            DBG_ERROR("DoubleCurrencyField::FormatChanged : somebody modified my key !");
            // We always build our own format from the settings we get via special methods (setCurrencySymbol etc.).
            // Nobody but ourself should modifiy the format key directly !
            break;
    }

    FormattedField::FormatChanged(nWhat);
}

//------------------------------------------------------------------------------
void DoubleCurrencyField::setCurrencySymbol(const String& _sSymbol)
{
    if (m_sCurrencySymbol == _sSymbol)
        return;

    m_sCurrencySymbol  = _sSymbol;
    UpdateCurrencyFormat();
    FormatChanged(FCT_CURRENCY_SYMBOL);
}

//------------------------------------------------------------------------------
void DoubleCurrencyField::setPrependCurrSym(BOOL _bPrepend)
{
    if (m_bPrependCurrSym == _bPrepend)
         return;

    m_bPrependCurrSym = _bPrepend;
    UpdateCurrencyFormat();
    FormatChanged(FCT_CURRSYM_POSITION);
}

//------------------------------------------------------------------------------
void DoubleCurrencyField::UpdateCurrencyFormat()
{
    // the old settings
    XubString sOldFormat;
    LanguageType eLanguage;
    GetFormat(sOldFormat, eLanguage);
    BOOL bThSep = GetThousandsSep();
    USHORT nDigits = GetDecimalDigits();

    // build a new format string with the base class' and my own settings
    International aIntl(eLanguage);
    XubString sNewFormat;
    if (bThSep)
    {
        sNewFormat = '#';
        sNewFormat += aIntl.GetNumThousandSep();
        sNewFormat.AppendAscii("##0");
    }
    else
        sNewFormat = '0';

    if (nDigits)
    {
        sNewFormat += aIntl.GetNumDecimalSep();

        XubString sTemp;
        sTemp.Fill(nDigits, '0');
        sNewFormat += sTemp;
    }

    if (getPrependCurrSym())
    {
        XubString sSymbol = getCurrencySymbol();
        sSymbol.EraseLeadingChars(' ');
        sSymbol.EraseTrailingChars(' ');

        XubString sTemp = sSymbol;
        sTemp += ' ';
        sTemp += sNewFormat;

        // for negative values : $ -0.00, not -$ 0.00 ...
        // (the real solution would be a possibility to choose a "positive currency format" and a "negative currency format" ...
        // But not now ... (and hey, you could take a formatted field for this ....))
        // FS - 31.03.00 74642
        sTemp += ';';
        sTemp += sSymbol;
        sTemp.AppendAscii(" -");
        sTemp += sNewFormat;

        sNewFormat = sTemp;
    }
    else
    {
        XubString sTemp = getCurrencySymbol();
        sTemp.EraseLeadingChars(' ');
        sTemp.EraseTrailingChars(' ');

        sNewFormat += ' ';
        sNewFormat += sTemp;
    }

    // set this new basic format
    m_bChangingFormat = TRUE;
    SetFormat(sNewFormat, eLanguage);
    m_bChangingFormat = FALSE;
}

