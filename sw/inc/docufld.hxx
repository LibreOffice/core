/*************************************************************************
 *
 *  $RCSfile: docufld.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:01 $
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
#ifndef _DOCUFLD_HXX
#define _DOCUFLD_HXX

#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif

#include "fldbas.hxx"
#include "numrule.hxx"

class _SetGetExpFlds;
class SwTxtFld;
class SwFrm;

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

enum SwDocInfoSubType
{
    DI_SUBTYPE_BEGIN,
    DI_TITEL = DI_SUBTYPE_BEGIN,
    DI_THEMA,
    DI_KEYS,
    DI_COMMENT,
    DI_INFO1,
    DI_INFO2,
    DI_INFO3,
    DI_INFO4,
    DI_CREATE,
    DI_CHANGE,
    DI_PRINT,
    DI_DOCNO,
    DI_EDIT,
    DI_SUBTYPE_END
};

enum SwDocInfoExtSubType
{
    DI_SUB_AUTHOR   = 0x0100,
    DI_SUB_TIME     = 0x0200,
    DI_SUB_DATE     = 0x0300,
    DI_SUB_FIXED    = 0x1000
};

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
    USHORT          nNum, nMax;
    BOOL            bVirtuell;

public:
    SwPageNumberFieldType();

    String& Expand( ULONG nFmt, short nOff, const String&, String& rRet ) const;
    void ChangeExpansion( SwDoc* pDoc, USHORT nNum, USHORT nMax,
                            BOOL bVirtPageNum, const sal_Int16* pNumFmt = 0 );
    inline sal_Int16 GetNumFormat() const { return nNumberingType; }
    virtual SwFieldType* Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: Seitennummerierung
 --------------------------------------------------------------------*/

class SwPageNumberField : public SwField
{
    String  sUserStr;
    USHORT  nSubType;
    short   nOffset;

public:
    SwPageNumberField(SwPageNumberFieldType*, USHORT nSub = PG_RANDOM,
                      ULONG nFmt = 0, short nOff = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual String      GetPar2() const;
    virtual void        SetPar2(const String& rStr);

    virtual USHORT      GetSubType() const;
    // virtual void        SetSubType(USHORT nSub); // OM: entfernt, da unbenoetigt und gefaehrlich
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );

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

    String                  Expand(ULONG) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: AutorenFeld
 --------------------------------------------------------------------*/

class SwAuthorField : public SwField
{
    String  aContent;

public:
    SwAuthorField(SwAuthorFieldType*, ULONG nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }
    inline const String& GetContent() const { return aContent; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: Filename
 --------------------------------------------------------------------*/

class SwFileNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwFileNameFieldType(SwDoc*);

    String                  Expand(ULONG) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: FileName
 --------------------------------------------------------------------*/

class SwFileNameField : public SwField
{
    String aContent;

public:
    SwFileNameField(SwFileNameFieldType*, ULONG nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }
    inline const String& GetContent() const { return aContent; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: TemplName
 --------------------------------------------------------------------*/

class SwTemplNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwTemplNameFieldType(SwDoc*);

    String                  Expand(ULONG) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: TemplName
 --------------------------------------------------------------------*/

class SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, ULONG nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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
    String                  Expand(USHORT nSubType, ULONG nFmt) const;
    virtual SwFieldType*    Copy() const;

    inline sal_Int16        GetNumFormat() const        { return nNumberingType; }
    inline void             SetNumFormat( sal_Int16 eFmt )  { nNumberingType = eFmt; }
};

/*--------------------------------------------------------------------
    Beschreibung: Dokumentstatistik
 --------------------------------------------------------------------*/

class SwDocStatField : public SwField
{
    USHORT nSubType;

public:
    SwDocStatField( SwDocStatFieldType*,
                    USHORT nSubType = 0, ULONG nFmt = 0);

    void ChangeExpansion( const SwFrm* pFrm );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual USHORT      GetSubType() const;
    virtual void        SetSubType(USHORT nSub);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: versteckter Text
 --------------------------------------------------------------------*/

class SwHiddenTxtFieldType : public SwFieldType
{
    BOOL bHidden;
public:
    SwHiddenTxtFieldType(BOOL bSetHidden = TRUE);

    virtual SwFieldType*    Copy() const;

    void                    SetHiddenFlag( BOOL bSetHidden = TRUE );
    inline BOOL             GetHiddenFlag() const { return bHidden; }
};

/*--------------------------------------------------------------------
    Beschreibung: Versteckter Text
 --------------------------------------------------------------------*/

class SwHiddenTxtField : public SwField
{
    String  aTRUETxt;           // Text wenn Bedingung TRUE
    String  aFALSETxt;          // Wenn Bedingung falsch
    String  aContent;           // Ausgewerteter DB-Text

    String  aCond;              // Bedingung
    USHORT  nSubType;

    BOOL    bCanToggle : 1;     // kann das Feld einzeln getoggelt werden?
    BOOL    bIsHidden  : 1;     // ist es nicht sichtbar?
    BOOL    bValid     : 1;     // DB-Feld evaluiert?

public:
    SwHiddenTxtField( SwHiddenTxtFieldType*,
                     BOOL   bConditional,
                     const  String& rCond,
                     const  String& rTxt,
                     BOOL   bHidden  = FALSE,
                     USHORT nSubType = TYP_HIDDENTXTFLD);

    SwHiddenTxtField( SwHiddenTxtFieldType*,
                      const String& rCond,
                      const String& rTrue,
                      const String& rFalse,
                      USHORT nSubType = TYP_HIDDENTXTFLD);

    virtual String      GetCntnt(BOOL bName = FALSE) const;
    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void                Evaluate(SwDoc*);

    inline void         SetValue(BOOL bHidden)  { bIsHidden = bHidden; }
    inline BOOL         GetValue() const        { return bIsHidden;    }
    inline BOOL         IsConditional() const   { return bCanToggle;   }
    inline BOOL         IsValid() const         { return bValid;       }
    String              GetColumnName(const String& rName);
    String              GetDBName(const String& rName, SwDoc *pDoc);

    // Condition
    virtual void        SetPar1(const String& rStr);
    virtual const String& GetPar1() const;

    // True/False - String
    virtual void        SetPar2(const String& rStr);
    virtual String      GetPar2() const;


    virtual USHORT      GetSubType() const;
    // virtual void        SetSubType(USHORT nSub); // OM: entfernt, da unbenoetigt und gefaehrlich
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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
    BOOL                bIsHidden:1;
public:
    // Direkte Eingabe alten Wert loeschen
    SwHiddenParaField(SwHiddenParaFieldType*, const String& rCond);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void                SetHidden(BOOL bHidden)     { bIsHidden = bHidden; }
    BOOL                IsHidden() const            { return bIsHidden;    }

    // Bedingung erfragen/setzen
    virtual const String& GetPar1() const;
    virtual void        SetPar1(const String& rStr);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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

class SwMacroField : public SwField
{
    String  aMacro;
    String  aText;

public:
    // Direkte Eingabe alten Wert loeschen
    SwMacroField( SwMacroFieldType*, const String& rLibAndName,
                  const String& rTxt);

    inline const String& GetMacro() const { return aMacro; }
    String           GetLibName() const;
    String           GetMacroName() const;

    virtual String   GetCntnt(BOOL bName = FALSE) const;
    virtual String   Expand() const;
    virtual SwField* Copy() const;

    // Library und FileName
    virtual const String& GetPar1() const;
    virtual void    SetPar1(const String& rStr);

    // Macrotext
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};


/*--------------------------------------------------------------------
    Beschreibung: PostIts
 --------------------------------------------------------------------*/

class SwPostItFieldType : public SwFieldType
{
public:
    SwPostItFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Beschreibung: PostIt
 --------------------------------------------------------------------*/

class SwPostItField : public SwField
{
    String  sTxt;       // die Anmerkung
    String  sAuthor;    // der Author
    Date    aDate;      // Datum ??/Zeit?? der Anmerkung

public:
    SwPostItField( SwPostItFieldType*,
                   const String& rAuthor, const String& rTxt, const Date& rDate);

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    inline const Date&      GetDate() const                 { return aDate; }
    inline void             SetDate( const Date& rDate )    { aDate = rDate;}

    // Author
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    // Text
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    const String&           GetTxt() const { return sTxt; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: DokumentInfo
 --------------------------------------------------------------------*/

class SwDocInfoFieldType : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    String                  Expand(USHORT nSubType, ULONG nFormat, USHORT nLang) const;
    virtual SwFieldType*    Copy() const;
};

class SwDocInfoField : public SwValueField
{
    USHORT  nSubType;
    String  aContent;

public:
    SwDocInfoField(SwDocInfoFieldType*, USHORT nSub, ULONG nFmt=0);

    virtual void            SetSubType(USHORT);
    virtual USHORT          GetSubType() const;
    virtual void            SetLanguage(USHORT nLng);
    virtual String          Expand() const;
    virtual String          GetCntnt(BOOL bName = FALSE) const;
    virtual SwField*        Copy() const;

    inline void             SetExpansion(const String& rStr) { aContent = rStr; }
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
};

/*--------------------------------------------------------------------
    Beschreibung: erweiterte Benutzereinstellung
 --------------------------------------------------------------------*/


class SwExtUserFieldType : public SwFieldType
{
    String aData;
public:
    SwExtUserFieldType();

    inline const String&    GetData() const                 { return aData; }
    inline void             SetData(const String& rStr)     { aData = rStr; }

    String                  Expand(USHORT nSubType, ULONG nFormat) const;
    virtual SwFieldType*    Copy() const;
};

class SwExtUserField : public SwField
{
    String  aContent;
    USHORT  nType;

public:
    SwExtUserField(SwExtUserFieldType*, USHORT nSub, ULONG nFmt=0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual USHORT      GetSubType() const;
    virtual void        SetSubType(USHORT nSub);

    inline void         SetExpansion(const String& rStr) { aContent = rStr; }
    inline const String& GetContent() const { return aContent; }

    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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
    BOOL    bOn;

public:
    SwRefPageSetField( SwRefPageSetFieldType*, short nOff = 0,
                        BOOL bOn = TRUE );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);

    BOOL IsOn() const               { return bOn; }
    void SetOn( BOOL bFlag )        { bOn = bFlag; }

    short GetOffset() const         { return nOffset; }
    void SetOffset( short nOff )    { nOffset = nOff; }
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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
    USHORT MakeSetList( _SetGetExpFlds& rTmpLst );

    sal_Int16       GetNumFormat() const            { return nNumberingType; }
    void            SetNumFormat( sal_Int16 eFmt )  { nNumberingType = eFmt; }
    SwDoc*  GetDoc() const                  { return pDoc; }
};

/*--------------------------------------------------------------------
    Beschreibung: Relative Seitennummerierung Abfragen
 --------------------------------------------------------------------*/

class SwRefPageGetField : public SwField
{
    String sTxt;
public:
    SwRefPageGetField( SwRefPageGetFieldType*, ULONG nFmt );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void SetText( const String& rTxt )      { sTxt = rTxt; }
    const String& GetText() const           { return sTxt; }

    void ChangeExpansion( const SwFrm* pFrm, const SwTxtFld* pFld );
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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

          SwDoc* GetDoc()               { return pDoc; }
    const SwDoc* GetDoc() const         { return pDoc; }

    SwCharFmt* GetCharFmt();
};

class SwJumpEditField : public SwField
{
    String sTxt, sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, ULONG nFormat,
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
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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

    BOOL    bCodeURL;   // Code enthaelt URL eines Scripts

public:
    SwScriptField( SwScriptFieldType*, const String& rType,
                   const String& rCode, BOOL bURL=FALSE );

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    // Type
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);
    // Text
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    const String&           GetCode() const { return sCode; }

    BOOL                    IsCodeURL() const { return bCodeURL; }
    void                    SetCodeURL( BOOL bURL ) { bCodeURL = bURL; }
    virtual BOOL        QueryValue( com::sun::star::uno::Any& rVal, const String& rProperty ) const;
    virtual BOOL        PutValue( const com::sun::star::uno::Any& rVal, const String& rProperty );
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

class SwCombinedCharField : public SwField
{
    String  sCharacters;    // combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const String& rChars );

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    // Characters
    virtual const String&   GetPar1() const;
    virtual void            SetPar1(const String& rStr);

    virtual BOOL            QueryValue( com::sun::star::uno::Any& rVal,
                                        const String& rProperty ) const;
    virtual BOOL            PutValue( const com::sun::star::uno::Any& rVal,
                                        const String& rProperty );
};


#endif // _DOCUFLD_HXX
