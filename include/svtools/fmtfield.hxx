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

#ifndef _FMTFIELD_HXX_
#define _FMTFIELD_HXX_

#include "svtools/svtdllapi.h"
#include <vcl/spinfld.hxx>
#include <svl/zforlist.hxx>

namespace validation { class NumberValidator; }

typedef sal_uInt16 FORMAT_CHANGE_TYPE;
#define FCT_KEYONLY         0x00        // only a new key was set
#define FCT_FORMATTER       0x01        // a new formatter weas set, usually implies a change of the key, too
#define FCT_PRECISION       0x02        // a new precision was set
#define FCT_THOUSANDSSEP    0x03        // the thousands separator setting changed

//------------------------------------------------------------------------------
class SVT_DLLPUBLIC FormattedField : public SpinField
{
private:
    // Da ein SvNumberFormatter eine ziemlich teure (sowohl zeit- als auch platz-maessig) Angelegenheit ist,
    // haelt sich nicht jedes Field, an dem kein Formatter gesetzt wurde, eine eigenen Instanz, sondern es gibt nur eine
    // einzige statische.
    class StaticFormatter
    {
        static SvNumberFormatter*   s_cFormatter;
        static sal_uLong                s_nReferences;
    public:
        StaticFormatter();
        ~StaticFormatter();

        operator SvNumberFormatter* () { return GetFormatter(); }
        SVT_DLLPUBLIC SvNumberFormatter* GetFormatter();
    };

protected:
    OUString      m_sLastValidText;
        // hat nichts mit dem current value zu tun, ist der letzte Text, der waehrend einer Eingabe als gueltig erkannt
        // wurde (also durch CheckText geprueft, nicht durch den Formatter gejagt)
    Selection   m_aLastSelection;

    double              m_dMinValue;
    double              m_dMaxValue;
    sal_Bool                m_bHasMin : 1;
    sal_Bool                m_bHasMax : 1;

    sal_Bool                m_bStrictFormat : 1;

    sal_Bool                m_bValueDirty : 1;
    sal_Bool                m_bEnableEmptyField : 1;
    sal_Bool                m_bAutoColor : 1;
    sal_Bool                m_bEnableNaN : 1;
    double              m_dCurrentValue;
    double              m_dDefaultValue;

    sal_uLong               m_nFormatKey;
    SvNumberFormatter*  m_pFormatter;
    StaticFormatter     m_aStaticFormatter;

    double              m_dSpinSize;
    double              m_dSpinFirst;
    double              m_dSpinLast;

    // es macht einen Unterschied, ob man bei eingestellter Textformatierung beim LostFocus den aktuellen String durch
    // den Formatter jagt und das Ergebnis anzeigt oder erst aus dem String ein double macht, das formatiert und dann
    // ausgibt
    sal_Bool                m_bTreatAsNumber;
    // und mit den folgenden Members koennen wir das Ganze hier auch zur formatierten Text-Ausgabe benutzen ...
    OUString            m_sCurrentTextValue;
    OUString            m_sDefaultText;

    // die bei der letzten Ausgabe-Operation vom Formatter gelieferte Farbe (nicht dass wir sie beachten wuerden, aber
    // man kann sie von aussen abfragen)
    Color*              m_pLastOutputColor;

    bool                m_bUseInputStringForFormatting;

public:
    FormattedField(Window* pParent, WinBits nStyle = 0, SvNumberFormatter* pInitialFormatter = NULL, sal_Int32 nFormatKey = 0);
    FormattedField(Window* pParent, const ResId& rResId, SvNumberFormatter* pInitialFormatter = NULL, sal_Int32 nFormatKey = 0);
    virtual ~FormattedField();

    // Min-/Max-Verwaltung
    sal_Bool    HasMinValue() const         { return m_bHasMin; }
    void    ClearMinValue()             { m_bHasMin = sal_False; }
    void    SetMinValue(double dMin);
    double  GetMinValue() const         { return m_dMinValue; }

    sal_Bool    HasMaxValue() const         { return m_bHasMax; }
    void    ClearMaxValue()             { m_bHasMax = sal_False; }
    void    SetMaxValue(double dMax);
    double  GetMaxValue() const         { return m_dMaxValue; }

    // aktueller Wert
    virtual void    SetValue(double dVal);
    virtual double  GetValue();
        // die Standard-Implementierung jagt die Eingabe jeweils durch den Formatter, so einer vorhanden ist

    void    GetColor() const;

    void    SetTextValue(const OUString& rText);
        // der String wird in ein double umgewandelt (durch den Formatter) und anschliessen in SetValue gesteckt

    sal_Bool    IsEmptyFieldEnabled() const         { return m_bEnableEmptyField; }
    void    EnableEmptyField(sal_Bool bEnable);
        // wenn nicht enabled, wird beim Verlassen des Feldes der Text auf den letzten gueltigen zurueckgesetzt

    void    SetDefaultValue(double dDefault)    { m_dDefaultValue = dDefault; m_bValueDirty = sal_True; }
        // wenn der aktuelle String ungueltig ist, liefert GetValue() diesen Default-Wert
    double  GetDefaultValue() const             { return m_dDefaultValue; }

    // Einstellungen fuer das Format
    sal_uLong   GetFormatKey() const                { return m_nFormatKey; }
    void    SetFormatKey(sal_uLong nFormatKey);

    SvNumberFormatter*  GetFormatter() const    { return m_pFormatter; }
    void    SetFormatter(SvNumberFormatter* pFormatter, sal_Bool bResetFormat = sal_True);
        // wenn bResetFormat sal_False ist, wird versucht, das alte eingestellte Format mit 'hinueberzuretten' (teuer, wenn es sich nicht
        // um eines der Standard-Formate handelt, die in allen Formattern gleich sind)
        // wenn sal_True, wird als neuer FormatKey 0 gesetzt

    sal_Bool    GetThousandsSep() const;
    void    SetThousandsSep(sal_Bool _bUseSeparator);
        // the is no check if the current format is numeric, so be cautious when calling these functions

    sal_uInt16  GetDecimalDigits() const;
    void    SetDecimalDigits(sal_uInt16 _nPrecision);
        // the is no check if the current format is numeric, so be cautious when calling these functions

    SvNumberFormatter*  StandardFormatter() { return m_aStaticFormatter; }
        // Wenn man keinen eigenen Formatter explizit anlegen will, kann man diesen hier in SetFormatter stecken ...
        // Das hier gelieferte Objekt wird allerdings zwischen allen Instanzen der Klasse geteilt (aus Zeit- und Platzgruenden),
        // also ist etwas Vorsicht angebracht ...

    OUString        GetFormat(LanguageType& eLang) const;
    sal_Bool        SetFormat(const OUString& rFormatString, LanguageType eLang);
        // sal_False, wenn der FormatString nicht gesetzt werden konnte (also wahrscheinlich ungueltig ist)

    sal_Bool    IsStrictFormat() const              { return m_bStrictFormat; }
    void    SetStrictFormat(sal_Bool bEnable)       { m_bStrictFormat = bEnable; }
        // Formatueberpruefung waehrend der Eingabe ?

    // Spin-Handling
    virtual void Up();
    virtual void Down();
        // Standard-Implementierung : hoch- oder runterzaehlen des aktuellen double einfach um die gesetzte SpinSize
    virtual void First();
    virtual void Last();
        // Standard-Implementierung : aktuelles double setzen auf eingestellten first respektive last value

    void    SetSpinSize(double dStep)   { m_dSpinSize = dStep; }
    double  GetSpinSize() const         { return m_dSpinSize; }

    void    SetSpinFirst(double dFirst) { m_dSpinFirst = dFirst; }
    double  GetSpinFirst() const        { return m_dSpinFirst; }

    void    SetSpinLast(double dLast)   { m_dSpinLast = dLast; }
    double  GetSpinLast() const         { return m_dSpinLast; }

    sal_Bool    TreatingAsNumber() const    { return m_bTreatAsNumber; }
    void    TreatAsNumber(sal_Bool bDoSo) { m_bTreatAsNumber = bDoSo; }

public:
    virtual void SetText( const OUString& rStr );
    virtual void SetText( const OUString& rStr, const Selection& rNewSelection );

    // die folgenden Methoden sind interesant, wenn m_bTreatAsNumber auf sal_False sitzt
    /** nehmen wir mal an, irgendjemand will das ganze schoene double-Handling gar nicht haben, sondern
        einfach den Text formatiert ausgeben ...
        (der Text wird einfach nur durch den Formatter gejagt und dann gesetzt)
    */
    void SetTextFormatted(const OUString& rText);
    OUString  GetTextValue() const;

    void      SetDefaultText(const OUString& rDefault) { m_sDefaultText = rDefault; }
    OUString  GetDefaultText() const { return m_sDefaultText; }

    // die bei der letzten Ausgabe-Operation vom Formatter gelieferte Farbe (Ausgabe-Operationen werden getriggert durch
    // SetValue, SetTextValue, SetTextFormatted, also indirekt eventuell auch durch SetMin-/-MaxValue)
    Color*  GetLastOutputColor() const { return m_pLastOutputColor; }

    /** reformats the current text. Interesting if the user entered some text in an "input format", and
        this should be formatted in the "output format" (which may differ, e.g. by additional numeric
        digits or such).
    */
    void    Commit();

    // enable automatic coloring. if set to sal_True, and the format the field is working with for any current value
    // says that it has to be painted in a special color (e.g. a format where negative numbers should be printed
    // red), the text is painted with that color automatically.
    // The color used is the same as returned by GetLastOutputColor()
    void    SetAutoColor(sal_Bool _bAutomatic);
    sal_Bool    GetAutoColor() const { return m_bAutoColor; }

    /** enables handling of not-a-number value.

        When this is set to <FALSE/> (the default), then invalid inputs (i.e. text which cannot be
        intepreted, according to the current formatting) will be handled as if the default value
        has been entered. GetValue the will return this default value.

        When set to <TRUE/>, then GetValue will return NaN (not a number, see <method scope="rtl::math">isNan</method>)
        when the current input is invalid.

        Note that setting this to <TRUE/> implies that upon leaving the control, the input
        will *not* be corrected to a valid value. For example, if the user enters "foo" in the
        control, and then tabs out of it, the text "foo" will persist, and GetValue will
        return NaN in subsequent calls.
    */
    void    EnableNotANumber( sal_Bool _bEnable );
    sal_Bool    IsNotANumberEnabled( ) const { return m_bEnableNaN; }

    /** When being set to true, the strings in the field are formatted using the
        InputLine format.  That's also what you get in Calc when you edit a cell
        using F2
     */
    void    UseInputStringForFormatting( bool bUseInputStr = true );
    bool    IsUsingInputStringForFormatting() const;

protected:
    virtual long Notify(NotifyEvent& rNEvt);
    void impl_Modify(bool makeValueDirty = true);
    virtual void Modify();

    // CheckText ueberschreiben fuer Ueberpruefung zur Eingabezeit
    virtual sal_Bool CheckText(const OUString&) const { return sal_True; }

    // any aspect of the current format has changed
    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat);

    void ImplSetTextImpl(const OUString& rNew, Selection* pNewSel);
    void ImplSetValue(double dValue, sal_Bool bForce);
    sal_Bool ImplGetValue(double& dNewVal);

    void ImplSetFormatKey(sal_uLong nFormatKey);
        // SetFormatKey without FormatChanged notification

    virtual SvNumberFormatter*  CreateFormatter() { SetFormatter(StandardFormatter()); return m_pFormatter; }
    SvNumberFormatter*  ImplGetFormatter() const { return m_pFormatter ? m_pFormatter : ((FormattedField*)this)->CreateFormatter(); }

    long PreNotify(NotifyEvent& rNEvt);

    virtual void ReFormat();
};

//------------------------------------------------------------------------------
class SVT_DLLPUBLIC DoubleNumericField : public FormattedField
{
protected:
    validation::NumberValidator*    m_pNumberValidator;

public:
    DoubleNumericField(Window* pParent, WinBits nStyle = 0)
        :FormattedField(pParent, nStyle)
        ,m_pNumberValidator( NULL )
    {
        ResetConformanceTester();
    }

    DoubleNumericField(Window* pParent, const ResId& rResId)
        :FormattedField(pParent, rResId)
        ,m_pNumberValidator( NULL )
    {
        ResetConformanceTester();
    }
    virtual ~DoubleNumericField();

protected:
    virtual sal_Bool CheckText(const OUString& sText) const;

    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat);
    void ResetConformanceTester();
};

//==============================================================================
#define FCT_CURRENCY_SYMBOL     0x10
#define FCT_CURRSYM_POSITION    0x20

//------------------------------------------------------------------------------
class DoubleCurrencyField : public FormattedField
{
    OUString   m_sCurrencySymbol;
    sal_Bool   m_bPrependCurrSym;
    sal_Bool   m_bChangingFormat;

public:
    DoubleCurrencyField(Window* pParent, WinBits nStyle = 0);

    OUString    getCurrencySymbol() const { return m_sCurrencySymbol; }
    void        setCurrencySymbol(const OUString& rSymbol);

    sal_Bool    getPrependCurrSym() const { return m_bPrependCurrSym; }
    void        setPrependCurrSym(sal_Bool _bPrepend);

protected:
    virtual void FormatChanged(FORMAT_CHANGE_TYPE nWhat);

    void UpdateCurrencyFormat();
};

#endif // _FMTFIELD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
