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
#ifndef SW_FLDBAS_HXX
#define SW_FLDBAS_HXX

#include <i18npool/lang.h>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include "swdllapi.h"
#include <calbck.hxx>
#include <com/sun/star/uno/Any.hxx>

class SwDoc;
class SvStringsDtor;
class SvNumberFormatter;


/*--------------------------------------------------------------------
    Beschreibung:   die ID's fuer die Feldtypen
 --------------------------------------------------------------------*/

enum RES_FIELDS
{
// Fuer die alten Dokumente muessen die Field-Which IDS erhalten bleiben !!!!
    RES_FIELDS_BEGIN,
    RES_DBFLD = RES_FIELDS_BEGIN,
    RES_USERFLD,
    RES_FILENAMEFLD,
    RES_DBNAMEFLD,
    RES_DATEFLD,
    RES_TIMEFLD,
    RES_PAGENUMBERFLD,
    RES_AUTHORFLD,
    RES_CHAPTERFLD,
    RES_DOCSTATFLD,
    RES_GETEXPFLD,
    RES_SETEXPFLD,
    RES_GETREFFLD,
    RES_HIDDENTXTFLD,
    RES_POSTITFLD,
    RES_FIXDATEFLD,
    RES_FIXTIMEFLD,
    RES_REGFLD,
    RES_VARREGFLD,
    RES_SETREFFLD,
    RES_INPUTFLD,
    RES_MACROFLD,
    RES_DDEFLD,
    RES_TABLEFLD,
    RES_HIDDENPARAFLD,
    RES_DOCINFOFLD,
    RES_TEMPLNAMEFLD,
    RES_DBNEXTSETFLD,
    RES_DBNUMSETFLD,
    RES_DBSETNUMBERFLD,
    RES_EXTUSERFLD,
    RES_REFPAGESETFLD,
    RES_REFPAGEGETFLD,
    RES_INTERNETFLD,
    RES_JUMPEDITFLD,
    RES_SCRIPTFLD,
    RES_DATETIMEFLD,
    RES_AUTHORITY,  //Table of authorities
    RES_COMBINED_CHARS,
    RES_DROPDOWN,
    RES_FIELDS_END
};

/*--------------------------------------------------------------------
    Beschreibung: Liste der FieldTypes am UI
 --------------------------------------------------------------------*/

enum SwFldTypesEnum
{
    TYP_BEGIN,
    TYP_DATEFLD = TYP_BEGIN,    // 0
    TYP_TIMEFLD,
    TYP_FILENAMEFLD,
    TYP_DBNAMEFLD,
    TYP_CHAPTERFLD,
    TYP_PAGENUMBERFLD,
    TYP_DOCSTATFLD,
    TYP_AUTHORFLD,
    TYP_SETFLD,
    TYP_GETFLD,
    TYP_FORMELFLD,              // 10
    TYP_HIDDENTXTFLD,
    TYP_SETREFFLD,
    TYP_GETREFFLD,
    TYP_DDEFLD,
    TYP_MACROFLD,
    TYP_INPUTFLD,
    TYP_HIDDENPARAFLD,
    TYP_DOCINFOFLD,
    TYP_DBFLD,
    TYP_USERFLD,                // 20
    TYP_POSTITFLD,
    TYP_TEMPLNAMEFLD,
    TYP_SEQFLD,
    TYP_DBNEXTSETFLD,
    TYP_DBNUMSETFLD,
    TYP_DBSETNUMBERFLD,
    TYP_CONDTXTFLD,
    TYP_NEXTPAGEFLD,
    TYP_PREVPAGEFLD,
    TYP_EXTUSERFLD,             // 30
    TYP_FIXDATEFLD,
    TYP_FIXTIMEFLD,
    TYP_SETINPFLD,
    TYP_USRINPFLD,
    TYP_SETREFPAGEFLD,
    TYP_GETREFPAGEFLD,
    TYP_INTERNETFLD,
    TYP_JUMPEDITFLD,
    TYP_SCRIPTFLD,              // 40
    TYP_AUTHORITY,
    TYP_COMBINED_CHARS,
    TYP_DROPDOWN,
    TYP_END
};


enum SwFileNameFormat
{
    FF_BEGIN,
    FF_NAME = FF_BEGIN,
    FF_PATHNAME,
    FF_PATH,
    FF_NAME_NOEXT,
    FF_UI_NAME,
    FF_UI_RANGE,
    FF_END,
    FF_FIXED = 0x8000
};

enum SwVarFormat
{
    VVF_CMD         = 0x0010,   // Kommando anzeigen
    VVF_INVISIBLE   = 0x0040,   // unsichtbar
    VVF_XXP         = 0x0400,   // 1234%
    VVF_XX_XXP      = 0x0800,   // 1.234,56%
    VVF_CLEAR       = 0x000f,

// ab hier neue Formate
    VVF_SYS         = 0x2000,   //Zahlenformat aus der Systemeinstellung
    VVF_X           = 0x2100,   // 1234
    VVF_X_X         = 0x2200,   // 1234,5
    VVF_X_XX        = 0x2300,   // 1245,56
    VVF_XX_X        = 0x2400,   // 1.234,5
    VVF_XX_XX       = 0x2500,   // 1.234,56
    VVF_XX_XXX      = 0x2600,   // 1.234,567
    VVF_SYS_CUR     = 0x2700,   // W???hrungsformat aus der Systemeinstellung
    VVF_CUR_X       = 0x2800,   // DM 1234
    VVF_CUR_XX_XX   = 0x2900,   // DM 1234,56 DM 1234,00
    VVF_CUR_XX_X0   = 0x2a00,   // DM 1234,56 DM 1234,--
    VVF_X_CUR       = 0x2b00,   // 1234 DM
    VVF_XX_XX_CUR   = 0x2c00,   // 1234,56 DM 1234,00 DM
    VVF_XX_X0_CUR   = 0x2d00,    // 1234,56 DM 1234,-- DM
// Kompatibilitaet:
    VF_CMD          = VVF_CMD,
    VF_INVISIBLE    = VVF_INVISIBLE,
    VF_XXP          = VVF_XXP,
    VF_XX_XXP       = VVF_XX_XXP,
    VF_VISIBLE      = VVF_SYS,
    VF_XX           = VVF_X,
    VF_XX_XX        = VVF_XX_XX,
    VF_XX_XX_CUR    = VVF_SYS_CUR,
    VF_CLEAR        = VVF_CLEAR

};

typedef sal_uInt16 SwGetSetExpType;
namespace nsSwGetSetExpType
{
    const SwGetSetExpType GSE_STRING  = 0x0001; // String
    const SwGetSetExpType GSE_EXPR    = 0x0002; // Expression
    const SwGetSetExpType GSE_INP     = 0x0004; // InputField
    const SwGetSetExpType GSE_SEQ     = 0x0008; // Sequence
    const SwGetSetExpType GSE_FORMULA = 0x0010; // Formel
}

typedef sal_uInt16 SwExtendedSubType;
namespace nsSwExtendedSubType
{
    const SwExtendedSubType SUB_CMD         = 0x0100;   // Kommando anzeigen
    const SwExtendedSubType SUB_INVISIBLE   = 0x0200;   // unsichtbar
    const SwExtendedSubType SUB_OWN_FMT     = 0x0400;   // SwDBField: Uebernimmt Formatierung nicht
                                                        // aus Datenbank
}

enum SwInputFieldSubType
{
    INP_TXT     = 0x01,
    INP_USR     = 0x02,
    INP_VAR     = 0x03
};


enum SwUserType
{
    UF_STRING   = 0x01,
    UF_EXPR     = 0x02
};

enum SwDateTimeSubType
{
    FIXEDFLD = 1,
    DATEFLD  = 2,
    TIMEFLD  = 4
};


extern sal_uInt16 __FAR_DATA aTypeTab[];

/*--------------------------------------------------------------------
    Beschreibung: Allgemeine Tools
 --------------------------------------------------------------------*/

String  GetResult(double nVal, sal_uInt32 nNumFmt, sal_uInt16 nLang = LANGUAGE_SYSTEM);
void    SetErrorStr(const String& rStr);
//String    ExpandDate(const Date& rDate, sal_uLong nFmt, sal_uInt16 nLang);
//String    ExpandTime(const Time& rTime, sal_uLong nFmt, sal_uInt16 nLang);
String  FormatNumber(sal_uInt16 nNum, sal_uInt32 nFormat);

/*--------------------------------------------------------------------
    Beschreibung:   Instanzen von SwFields und Abgeleiteten kommen 0-n mal vor.
                    Zu jeder Klasse SwFields existiert
                    1 Instanz der zugehoerigen Typenklasse
                    Basisklasse aller Feldtypen ist SwFieldType
 --------------------------------------------------------------------*/

DBG_NAMEEX(SwFieldType)

class SW_DLLPUBLIC SwFieldType : public SwModify
{
    sal_uInt16 nWhich;

    friend void _FinitUI();     // um den Pointer zu loeschen!
    static  SvStringsDtor*  pFldNames;

    static void _GetFldName();          // legt die FldNames an, fldmgr.cxx!

protected:
    // single argument ctors shall be explicit.
    explicit SwFieldType( sal_uInt16 nWhichId );

public:

#ifdef DBG_UTIL
    virtual ~SwFieldType();
#endif
    static  const String&   GetTypeStr( sal_uInt16 nTypeId );

    // nur in abgeleiteten Klassen
    virtual const String&   GetName() const;
    virtual SwFieldType*    Copy()    const = 0;
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

            sal_uInt16          Which() const { return nWhich; }

    inline  void            UpdateFlds() const;
};

inline void SwFieldType::UpdateFlds() const
{
    ((SwFieldType*)this)->Modify( 0, 0 );
}

/*--------------------------------------------------------------------
    Beschreibung:  Basisklasse aller Felder.
                   Ueber Which wird der Typ des Feldes abgefragt.
                   Ueber Expand() wird der expandierte Inhalt
                   des Feldes in Erfahrung gebracht.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwField
{
private:
    mutable String      m_Cache; /// #i85766# cached expansion (for clipboard)
    sal_uInt16              nLang;  // Immer ueber SetLanguage aendern!
    sal_Bool                bIsAutomaticLanguage;
    sal_uInt32          nFormat;

    SwFieldType*        pType;

    virtual String      Expand() const = 0;
    virtual SwField*    Copy() const = 0;

protected:
    void                SetFormat(sal_uInt32 nSet) {nFormat = nSet;}

    SwField(SwFieldType* pTyp, sal_uInt32 nFmt = 0, sal_uInt16 nLang = LANGUAGE_SYSTEM);

public:
    virtual             ~SwField();

    // Typ feststellen
    inline SwFieldType* GetTyp() const;

    // neuen Typ setzen (wird fuer das Kopieren zwischen Dokumenten benutzt)
    virtual SwFieldType* ChgTyp( SwFieldType* );

    /** expand the field.
        @param  bCached     return cached field value.
        @remark     most callers should use the cached field value.
                    this is because various fields need special handing
                    (ChangeExpansion()) to return correct values, and only
                    SwTxtFormatter::NewFldPortion() sets things up properly.
        @return     the generated text (suitable for display)
      */
    String              ExpandField(bool const bCached) const;

    virtual String      GetFieldName() const;

    SwField *           CopyField() const;

    // ResId
    sal_uInt16              Which() const
#ifndef DBG_UTIL
        { return pType->Which(); }
#else
        ;       // in fldbas.cxx implementiert
#endif

    // TYP_ID
            sal_uInt16      GetTypeId() const;
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16);

    // Sprache an der Feldposition
    inline sal_uInt16       GetLanguage() const;
    virtual void        SetLanguage(sal_uInt16 nLng);

    // Parameter fuer Dialog und BASIC erfragen
    inline sal_uInt32   GetFormat() const;
    virtual const String& GetPar1() const;
    virtual String      GetPar2()   const;

    virtual String      GetFormula() const;

    virtual void        ChangeFormat(sal_uInt32 n);
    virtual void        SetPar1(const String& rStr);
    virtual void        SetPar2(const String& rStr);

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );
    // hat das Feld eine Action auf dem ClickHandler ? (z.B. INetFelder,..)
    sal_Bool            HasClickHdl() const;
    // ist es ein Fix-Feld?
    sal_Bool            IsFixed() const;

    sal_Bool                IsAutomaticLanguage() const { return bIsAutomaticLanguage;}
    void                SetAutomaticLanguage(sal_Bool bSet){bIsAutomaticLanguage = bSet;}

    virtual String      GetDescription() const;
};

inline SwFieldType* SwField::GetTyp() const
    { return pType;     }

inline sal_uInt32 SwField::GetFormat() const
    { return nFormat;   }

inline sal_uInt16 SwField::GetLanguage() const
    { return nLang;     }

/*--------------------------------------------------------------------
    Beschreibung:   Felder mit Values, die ueber der Numberformatter
                    formatiert werden muessen
 --------------------------------------------------------------------*/

class SwValueFieldType : public SwFieldType
{
    SwDoc*  pDoc;
    sal_Bool    bUseFormat; // Numberformatter verwenden

protected:
    SwValueFieldType( SwDoc* pDocPtr, sal_uInt16 nWhichId );
    SwValueFieldType( const SwValueFieldType& rTyp );

public:
    inline SwDoc*   GetDoc() const                      { return pDoc; }
    inline void     SetDoc(SwDoc* pNewDoc)              { pDoc = pNewDoc; }

    inline sal_Bool     UseFormat() const                   { return bUseFormat; }
    inline void     EnableFormat(sal_Bool bFormat = sal_True)   { bUseFormat = bFormat; }

    String          ExpandValue(const double& rVal, sal_uInt32 nFmt, sal_uInt16 nLng=0) const;
    void            DoubleToString(String &rValue, const double &rVal, LanguageType eLng) const;
    void            DoubleToString(String &rValue, const double &rVal, sal_uInt32 nFmt) const;
};

class SW_DLLPUBLIC SwValueField : public SwField
{
    double fValue;
//  String sExpand;

protected:
    SwValueField( SwValueFieldType* pFldType, sal_uInt32 nFmt = 0, sal_uInt16 nLang = LANGUAGE_SYSTEM, const double fVal = 0.0 );
    SwValueField( const SwValueField& rFld );

public:
    virtual                 ~SwValueField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );
    virtual void            SetLanguage(sal_uInt16 nLng);
//  os: wozu war das denn da?
//  virtual void            ChangeFormat(sal_uLong n);

    inline SwDoc*           GetDoc() const          { return ((SwValueFieldType*)GetTyp())->GetDoc(); }

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    inline String   ExpandValue(const double& rVal, sal_uInt32 nFmt, sal_uInt16 nLng=0) const
        { return ((SwValueFieldType*)GetTyp())->ExpandValue(rVal, nFmt, nLng); }

    static sal_uInt32       GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFmt);
};

class SW_DLLPUBLIC SwFormulaField : public SwValueField
{
    String sFormula;

protected:
    SwFormulaField( SwValueFieldType* pFldType, sal_uInt32 nFmt = 0, const double fVal = 0.0 );
    SwFormulaField( const SwFormulaField& rFld );

public:
    virtual String          GetFormula() const;
    void                    SetFormula(const String& rStr);

    void                    SetExpandedFormula(const String& rStr);
    String                  GetExpandedFormula() const;
};

#endif // SW_FLDBAS_HXX
