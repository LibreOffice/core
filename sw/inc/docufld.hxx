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
#ifndef SW_DOCUFLD_HXX
#define SW_DOCUFLD_HXX


#include <tools/date.hxx>
#include <tools/datetime.hxx>


#include <svl/macitem.hxx>

#include "fldbas.hxx"
#include "numrule.hxx"

class _SetGetExpFlds;
class SwTxtFld;
class SwFrm;
class OutlinerParaObject;
class SwTextAPIObject;

enum SwAuthorFormat
{
    AF_BEGIN,
    AF_NAME = AF_BEGIN,
    AF_SHORTCUT,
    AF_END,
    AF_FIXED = 0x8000
};

/*--------------------------------------------------------------------
    Beschreibung: Untertyp der Dokumentstatistik
 --------------------------------------------------------------------*/

enum SwDocStatSubType
{
    DS_BEGIN,
    DS_PAGE = DS_BEGIN,
    DS_PARA,
    DS_WORD,
    DS_CHAR,
    DS_TBL,
    DS_GRF,
    DS_OLE,
    DS_END
};

typedef sal_uInt16  SwDocInfoSubType;
namespace nsSwDocInfoSubType
{
    // NB: these must denote consecutive integers!
    // NB2: these are extended by 4 DI_INFO values for backward compatibility
    //      in filter/html/htmlfld.cxx, so make sure that DI_SUBTYPE_END
    //      really is the end, and is at least 4 less than DI_SUB_*!
    const SwDocInfoSubType DI_SUBTYPE_BEGIN =  0;
    const SwDocInfoSubType DI_TITEL         =  DI_SUBTYPE_BEGIN;
    const SwDocInfoSubType DI_THEMA         =  1;
    const SwDocInfoSubType DI_KEYS          =  2;
    const SwDocInfoSubType DI_COMMENT       =  3;
    const SwDocInfoSubType DI_CREATE        =  4;
    const SwDocInfoSubType DI_CHANGE        =  5;
    const SwDocInfoSubType DI_PRINT         =  6;
    const SwDocInfoSubType DI_DOCNO         =  7;
    const SwDocInfoSubType DI_EDIT          =  8;
    const SwDocInfoSubType DI_CUSTOM        =  9;
    const SwDocInfoSubType DI_SUBTYPE_END   = 10;


    const SwDocInfoSubType DI_SUB_AUTHOR    = 0x0100;
    const SwDocInfoSubType DI_SUB_TIME      = 0x0200;
    const SwDocInfoSubType DI_SUB_DATE      = 0x0300;
    const SwDocInfoSubType DI_SUB_FIXED     = 0x1000;
    const SwDocInfoSubType DI_SUB_MASK      = 0xff00;
}


enum RegInfoFormat  // Nur noch zum laden alter Dokumente!!!
{
    RF_AUTHOR,
    RF_TIME,
    RF_DATE,
    RF_ALL
};

enum SwPageNumSubType
{
    PG_RANDOM,
    PG_NEXT,
    PG_PREV
};

enum SwExtUserSubType
{
    EU_COMPANY      /*EU_FIRMA*/,
    EU_FIRSTNAME    /*EU_VORNAME*/,
    EU_NAME         /*EU_NAME*/,
    EU_SHORTCUT     /*EU_ABK*/,
    EU_STREET       /*EU_STRASSE*/,
    EU_COUNTRY      /*EU_LAND*/,
    EU_ZIP          /*EU_PLZ*/,
    EU_CITY         /*EU_ORT*/,
    EU_TITLE        /*EU_TITEL*/,
    EU_POSITION     /*EU_POS*/,
    EU_PHONE_PRIVATE /*EU_TELPRIV*/,
    EU_PHONE_COMPANY /*EU_TELFIRMA*/,
    EU_FAX,
    EU_EMAIL,
    EU_STATE,
    EU_FATHERSNAME,
    EU_APARTMENT
};

enum SwJumpEditFormat
{
    JE_FMT_TEXT,
    JE_FMT_TABLE,
    JE_FMT_FRAME,
    JE_FMT_GRAPHIC,
    JE_FMT_OLE
};

/*--------------------------------------------------------------------
    Beschreibung: Seitennummer
 --------------------------------------------------------------------*/

class SwPageNumberFieldType : public SwFieldType
{
    sal_Int16   nNumberingType;
    sal_uInt16          nNum, nMax;
    sal_Bool            bVirtuell;

public:
    SwPageNumberFieldType();

    String& Expand( sal_uInt32 nFmt, short nOff, const String&, String& rRet ) const;
    void ChangeExpansion( SwDoc* pDoc, sal_uInt16 nNum, sal_uInt16 nMax,
                            sal_Bool bVirtPageNum, const sal_Int16* pNumFmt = 0 );
    virtual SwFieldType* Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Seitennummerierung
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwPageNumberField : public SwField
{
    String  sUserStr;
    sal_uInt16  nSubType;
    short   nOffset;

public:
    SwPageNumberField(SwPageNumberFieldType*, sal_uInt16 nSub = PG_RANDOM,
                      sal_uInt32 nFmt = 0, short nOff = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual String      GetPar2() const;
    virtual void        SetPar2(const String& rStr);

    virtual sal_uInt16      GetSubType() const;
    // virtual void        SetSubType(sal_uInt16 nSub); // OM: entfernt, da unbenoetigt und gefaehrlich
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    const String& GetUserString() const         { return sUserStr; }
    void SetUserString( const String& rS )      { sUserStr = rS; }
};

/*--------------------------------------------------------------------
    Beschreibung: Autoren
 --------------------------------------------------------------------*/

class SwAuthorFieldType : public SwFieldType
{
public:
    SwAuthorFieldType();

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: AutorenFeld
 --------------------------------------------------------------------*/

class SwAuthorField : public SwField
{
    String  aContent;

public:
    SwAuthorField(SwAuthorFieldType*, sal_uInt32 nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }
    inline const String& GetContent() const { return aContent; }

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Filename
 --------------------------------------------------------------------*/

class SwFileNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwFileNameFieldType(SwDoc*);

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: FileName
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwFileNameField : public SwField
{
    String aContent;

public:
    SwFileNameField(SwFileNameFieldType*, sal_uInt32 nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }
    inline const String& GetContent() const { return aContent; }

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: TemplName
 --------------------------------------------------------------------*/

class SwTemplNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwTemplNameFieldType(SwDoc*);

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: TemplName
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, sal_uInt32 nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


/*--------------------------------------------------------------------
    Beschreibung: Docstatistik
 --------------------------------------------------------------------*/

class SwDocStatFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType;//com::sun::star::style::NumberingType

public:
    SwDocStatFieldType(SwDoc*);
    String                  Expand(sal_uInt16 nSubType, sal_uInt32 nFmt) const;
    virtual SwFieldType*    Copy() const;

    inline void             SetNumFormat( sal_Int16 eFmt )  { nNumberingType = eFmt; }
};

/*--------------------------------------------------------------------
    Beschreibung: Dokumentstatistik
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDocStatField : public SwField
{
    sal_uInt16 nSubType;

public:
    SwDocStatField( SwDocStatFieldType*,
                    sal_uInt16 nSubType = 0, sal_uInt32 nFmt = 0);

    void ChangeExpansion( const SwFrm* pFrm );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16 nSub);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: versteckter Text
 --------------------------------------------------------------------*/

class SwHiddenTxtFieldType : public SwFieldType
{
    sal_Bool bHidden;
public:
    SwHiddenTxtFieldType(sal_Bool bSetHidden = sal_True);

    virtual SwFieldType*    Copy() const;

    void                    SetHiddenFlag( sal_Bool bSetHidden = sal_True );
    inline sal_Bool             GetHiddenFlag() const { return bHidden; }
};

/*--------------------------------------------------------------------
    Beschreibung: Versteckter Text
 --------------------------------------------------------------------*/

class SwHiddenTxtField : public SwField
{
    String  aTRUETxt;           // Text wenn Bedingung sal_True
    String  aFALSETxt;          // Wenn Bedingung falsch
    String  aContent;           // Ausgewerteter DB-Text

    String  aCond;              // Bedingung
    sal_uInt16  nSubType;

    sal_Bool    bCanToggle : 1;     // kann das Feld einzeln getoggelt werden?
    sal_Bool    bIsHidden  : 1;     // ist es nicht sichtbar?
    sal_Bool    bValid     : 1;     // DB-Feld evaluiert?

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

public:
    SwHiddenTxtField( SwHiddenTxtFieldType*,
                     sal_Bool   bConditional,
                     const  String& rCond,
                     const  String& rTxt,
                     sal_Bool   bHidden  = sal_False,
                     sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    SwHiddenTxtField( SwHiddenTxtFieldType*,
                      const String& rCond,
                      const String& rTrue,
                      const String& rFalse,
                      sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    virtual String      GetFieldName() const;

    void                Evaluate(SwDoc*);

    inline void         SetValue(sal_Bool bHidden)  { bIsHidden = bHidden; }
    String              GetColumnName(const String& rName);
    String              GetDBName(const String& rName, SwDoc *pDoc);

    // Condition
    virtual void        SetPar1(const String& rStr);
    virtual const String& GetPar1() const;

    // True/False - String
    virtual void        SetPar2(const String& rStr);
    virtual String      GetPar2() const;


    virtual sal_uInt16      GetSubType() const;
    // virtual void        SetSubType(sal_uInt16 nSub); // OM: entfernt, da unbenoetigt und gefaehrlich
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Feld das zu einer Leerzeile (ohne Hoehe) expandiert
 --------------------------------------------------------------------*/

class SwHiddenParaFieldType : public SwFieldType
{
public:
    SwHiddenParaFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Versteckter Absatz
 --------------------------------------------------------------------*/

class SwHiddenParaField : public SwField
{
    String              aCond;
    sal_Bool                bIsHidden:1;
public:
    // Direkte Eingabe alten Wert loeschen
    SwHiddenParaField(SwHiddenParaFieldType*, const String& rCond);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void                SetHidden(sal_Bool bHidden)     { bIsHidden = bHidden; }
    sal_Bool                IsHidden() const            { return bIsHidden;    }

    // Bedingung erfragen/setzen
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Macro ausfuehren
 --------------------------------------------------------------------*/

class SwMacroFieldType : public SwFieldType
{
    SwDoc* pDoc;

public:
    SwMacroFieldType(SwDoc*);

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Macrofeld
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwMacroField : public SwField
{
    String  aMacro;
    String  aText;
    sal_Bool      bIsScriptURL;

    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    // Direkte Eingabe alten Wert loeschen
    SwMacroField( SwMacroFieldType*, const String& rLibAndName,
                  const String& rTxt);

    inline const String& GetMacro() const { return aMacro; }
    String           GetLibName() const;
    String           GetMacroName() const;
    SvxMacro         GetSvxMacro() const;

    virtual String   GetFieldName() const;

    // Library und FileName
    virtual const String& GetPar1() const;
    virtual void    SetPar1(const String& rStr);

    // Macrotext
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    static void CreateMacroString( String& rMacro,
                                   const String& rMacroName,
                                   const String& rLibraryName );

    static sal_Bool isScriptURL( const String& str );
};


/*--------------------------------------------------------------------
    Beschreibung: PostIts
 --------------------------------------------------------------------*/

class SwPostItFieldType : public SwFieldType
{
private:
    SwDoc*  mpDoc;
public:
    SwPostItFieldType(SwDoc* pDoc);

    virtual SwFieldType*    Copy() const;
    SwDoc* GetDoc()         {return mpDoc;};
};

/*--------------------------------------------------------------------
    Beschreibung: PostIt
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwPostItField : public SwField
{
    String      sTxt;       // die Anmerkung
    String      sAuthor;    // der Author
    DateTime    aDateTime;  // Datum und Zeit der Anmerkung
    OutlinerParaObject* mpText;
    SwTextAPIObject* m_pTextObject;

public:
    SwPostItField( SwPostItFieldType*,
                   const String& rAuthor, const String& rTxt, const DateTime& rDate);
    ~SwPostItField();

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    inline const Date       GetDate() const                 { return aDateTime.GetDate(); }
    inline const Time       GetTime() const                 { return aDateTime.GetTime(); }

    // Author
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // Text
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    const String&           GetTxt() const { return sTxt; }

    const OutlinerParaObject*   GetTextObject() const;
    void SetTextObject( OutlinerParaObject* pText );

    sal_uInt32 GetNumberOfParagraphs() const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
    virtual String      GetDescription() const;
};

/*--------------------------------------------------------------------
    Beschreibung: DokumentInfo
 --------------------------------------------------------------------*/

class SwDocInfoFieldType : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    String                  Expand(sal_uInt16 nSubType, sal_uInt32 nFormat, sal_uInt16 nLang, const String& rName) const;
    virtual SwFieldType*    Copy() const;
};

class SW_DLLPUBLIC SwDocInfoField : public SwValueField
{
    sal_uInt16  nSubType;
    String  aContent;
    String  aName;

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

public:
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const String& rName, sal_uInt32 nFmt=0);
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const String& rName, const String& rValue, sal_uInt32 nFmt=0);

    virtual void            SetSubType(sal_uInt16);
    virtual sal_uInt16          GetSubType() const;
    virtual void            SetLanguage(sal_uInt16 nLng);
    virtual String          GetFieldName() const;
    String                  GetName() const { return aName; }
    void                    SetName( const String& rName ) { aName = rName; }
    inline void             SetExpansion(const String& rStr) { aContent = rStr; }
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: erweiterte Benutzereinstellung
 --------------------------------------------------------------------*/


class SwExtUserFieldType : public SwFieldType
{
    String aData;
public:
    SwExtUserFieldType();

    inline void             SetData(const String& rStr)     { aData = rStr; }

    String                  Expand(sal_uInt16 nSubType, sal_uInt32 nFormat) const;
    virtual SwFieldType*    Copy() const;
};

class SwExtUserField : public SwField
{
    String  aContent;
    sal_uInt16  nType;

public:
    SwExtUserField(SwExtUserFieldType*, sal_uInt16 nSub, sal_uInt32 nFmt=0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16 nSub);

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


/*--------------------------------------------------------------------
    Beschreibung: Relatives Seitennummern - Feld
 --------------------------------------------------------------------*/

class SwRefPageSetFieldType : public SwFieldType
{
public:
    SwRefPageSetFieldType();

    virtual SwFieldType*    Copy() const;
    // ueberlagert, weil es nichts zum Updaten gibt!
    virtual void            Modify( SfxPoolItem *, SfxPoolItem * );
};

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung
 --------------------------------------------------------------------*/

class SwRefPageSetField : public SwField
{
    short   nOffset;
    sal_Bool    bOn;

public:
    SwRefPageSetField( SwRefPageSetFieldType*, short nOff = 0,
                        sal_Bool bOn = sal_True );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);

    sal_Bool IsOn() const               { return bOn; }

    short GetOffset() const         { return nOffset; }
    void SetOffset( short nOff )    { nOffset = nOff; }
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: relatives Seitennummern - Abfrage Feld
 --------------------------------------------------------------------*/

class SwRefPageGetFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType;

    void UpdateField( SwTxtFld* pTxtFld, _SetGetExpFlds& rSetList );

public:
    SwRefPageGetFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;

    // ueberlagert, um alle RefPageGet-Felder zu updaten
    virtual void Modify( SfxPoolItem *, SfxPoolItem * );
    sal_uInt16 MakeSetList( _SetGetExpFlds& rTmpLst );

    SwDoc*  GetDoc() const                  { return pDoc; }
};

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung Abfragen
 --------------------------------------------------------------------*/

class SwRefPageGetField : public SwField
{
    String sTxt;
public:
    SwRefPageGetField( SwRefPageGetFieldType*, sal_uInt32 nFmt );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void SetText( const String& rTxt )      { sTxt = rTxt; }

    void ChangeExpansion( const SwFrm* pFrm, const SwTxtFld* pFld );
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Feld zum Anspringen und Editieren
 --------------------------------------------------------------------*/

class SwJumpEditFieldType : public SwFieldType
{
    SwDoc* pDoc;
    SwDepend aDep;

public:
    SwJumpEditFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;

    SwCharFmt* GetCharFmt();
};

class SwJumpEditField : public SwField
{
    String sTxt, sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, sal_uInt32 nFormat,
                    const String& sText, const String& sHelp );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    // Platzhalter-Text
    virtual const String& GetPar1() const;
    virtual void    SetPar1(const String& rStr);

    // HinweisText
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);

    SwCharFmt* GetCharFmt() const
        { return ((SwJumpEditFieldType*)GetTyp())->GetCharFmt(); }
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Script Fieldtype
 --------------------------------------------------------------------*/

class SwScriptFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwScriptFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Script Field
 --------------------------------------------------------------------*/

class SwScriptField : public SwField
{
    String  sType;      // Type von Code (Java/VBScript/...)
    String  sCode;      // der Code als Text
                        // der Code als JavaCode ?

    sal_Bool    bCodeURL;   // Code enthaelt URL eines Scripts

public:
    SwScriptField( SwScriptFieldType*, const String& rType,
                   const String& rCode, sal_Bool bURL=sal_False );

    virtual String          GetDescription() const;

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    // Type
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);
    // Text
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);

    sal_Bool                    IsCodeURL() const { return bCodeURL; }
    void                    SetCodeURL( sal_Bool bURL ) { bCodeURL = bURL; }
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Beschreibung: Combined Character Fieldtype
 --------------------------------------------------------------------*/

class SwCombinedCharFieldType : public SwFieldType
{
public:
    SwCombinedCharFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Script Field
 --------------------------------------------------------------------*/

#define MAX_COMBINED_CHARACTERS     6

class SW_DLLPUBLIC SwCombinedCharField : public SwField
{
    String  sCharacters;    // combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const String& rChars );

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    // Characters
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


#endif // SW_DOCUFLD_HXX
