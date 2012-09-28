/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    Subtype of document statistics.
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
    /** NB: these must denote consecutive integers!
     NB2: these are extended by 4 DI_INFO values for backward compatibility
          in filter/html/htmlfld.cxx, so make sure that DI_SUBTYPE_END
          really is the end, and is at least 4 less than DI_SUB_*! */
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


enum RegInfoFormat  ///< Only for loading of old documents!!!
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
    EU_COMPANY,
    EU_FIRSTNAME,
    EU_NAME,
    EU_SHORTCUT,
    EU_STREET,
    EU_COUNTRY,
    EU_ZIP,
    EU_CITY,
    EU_TITLE,
    EU_POSITION,
    EU_PHONE_PRIVATE,
    EU_PHONE_COMPANY,
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
    Page number.
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
    Page numbering.
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

    virtual rtl::OUString GetPar2() const;
    virtual void        SetPar2(const rtl::OUString& rStr);

    virtual sal_uInt16  GetSubType() const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    const String& GetUserString() const         { return sUserStr; }
    void SetUserString( const String& rS )      { sUserStr = rS; }
};

/*--------------------------------------------------------------------
    Authors.
 --------------------------------------------------------------------*/

class SwAuthorFieldType : public SwFieldType
{
public:
    SwAuthorFieldType();

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Author field.
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

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Filename
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
    FileNameField
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

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    TemplName
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
    TemplNameField
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, sal_uInt32 nFmt = 0);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


/*--------------------------------------------------------------------
    Document statistics
 --------------------------------------------------------------------*/

class SwDocStatFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType; ///< com::sun::star::style::NumberingType

public:
    SwDocStatFieldType(SwDoc*);
    String                  Expand(sal_uInt16 nSubType, sal_uInt32 nFmt) const;
    virtual SwFieldType*    Copy() const;

    inline void             SetNumFormat( sal_Int16 eFmt )  { nNumberingType = eFmt; }
};

/*--------------------------------------------------------------------
    DocStatField
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
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Hidden text.
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
    HiddenTxtField
 --------------------------------------------------------------------*/

class SwHiddenTxtField : public SwField
{
    rtl::OUString aTRUETxt;         ///< Text if condition sal_True.
    rtl::OUString aFALSETxt;        ///< If condition false.
    rtl::OUString aContent;         ///< Evaluated DB-text.

    rtl::OUString aCond;            ///< Condition.
    sal_uInt16  nSubType;

    sal_Bool    bCanToggle : 1;     ///< Can field be toggled alone?
    sal_Bool    bIsHidden  : 1;     ///< Is it not visible?
    sal_Bool    bValid     : 1;     ///< Is DB-field evaluated?

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

    /// Condition
    virtual void        SetPar1(const rtl::OUString& rStr);
    virtual const rtl::OUString& GetPar1() const;

    /// True/False - String
    virtual void        SetPar2(const rtl::OUString& rStr);
    virtual rtl::OUString GetPar2() const;


    virtual sal_uInt16      GetSubType() const;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Field that expands to an empty line (without height).
 --------------------------------------------------------------------*/

class SwHiddenParaFieldType : public SwFieldType
{
public:
    SwHiddenParaFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Hidded paragraph.
 --------------------------------------------------------------------*/

class SwHiddenParaField : public SwField
{
    rtl::OUString           aCond;
    sal_Bool                bIsHidden:1;
public:
    /// Direct input, delete old value.
    SwHiddenParaField(SwHiddenParaFieldType*, const String& rCond);

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    void                SetHidden(sal_Bool bHidden)     { bIsHidden = bHidden; }
    sal_Bool                IsHidden() const            { return bIsHidden;    }

    /// Query, set condition.
    virtual const rtl::OUString& GetPar1() const;
    virtual void        SetPar1(const rtl::OUString& rStr);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Run Macro.
 --------------------------------------------------------------------*/

class SwMacroFieldType : public SwFieldType
{
    SwDoc* pDoc;

public:
    SwMacroFieldType(SwDoc*);

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    MacroField.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwMacroField : public SwField
{
    rtl::OUString aMacro;
    rtl::OUString aText;
    sal_Bool      bIsScriptURL;

    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    /// Direct input, delete old value.
    SwMacroField( SwMacroFieldType*, const String& rLibAndName,
                  const String& rTxt);

    inline const rtl::OUString& GetMacro() const { return aMacro; }
    String           GetLibName() const;
    String           GetMacroName() const;
    SvxMacro         GetSvxMacro() const;

    virtual String   GetFieldName() const;

    /// Library and FileName
    virtual const rtl::OUString& GetPar1() const;
    virtual void    SetPar1(const rtl::OUString& rStr);

    /// Macrotext
    virtual rtl::OUString  GetPar2() const;
    virtual void    SetPar2(const rtl::OUString& rStr);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    static void CreateMacroString( rtl::OUString& rMacro,
                                   const String& rMacroName,
                                   const String& rLibraryName );

    static sal_Bool isScriptURL( const String& str );
};


/*--------------------------------------------------------------------
    PostIts
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
    PostItField
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwPostItField : public SwField
{
    rtl::OUString sTxt;
    rtl::OUString sAuthor;
    rtl::OUString sInitials; ///< Initials of the author.
    rtl::OUString sName;     ///< Name of the comment.
    DateTime    aDateTime;
    OutlinerParaObject* mpText;
    SwTextAPIObject* m_pTextObject;

public:
    SwPostItField( SwPostItFieldType*,
                   const String& rAuthor, const String& rTxt, const String& rInitials, const String& rName, const DateTime& rDate);
    ~SwPostItField();

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    inline const DateTime   GetDateTime() const             { return aDateTime; }
    inline const Date       GetDate() const                 { return aDateTime.GetDate(); }
    inline const Time       GetTime() const                 { return aDateTime.GetTime(); }

    /// Author
    virtual const rtl::OUString& GetPar1() const;
    virtual void            SetPar1(const rtl::OUString& rStr);

    /// Text
    virtual rtl::OUString   GetPar2() const;
    virtual void            SetPar2(const rtl::OUString& rStr);
    const rtl::OUString&    GetTxt() const { return sTxt; }
    const rtl::OUString&    GetInitials() const;
    void                    SetName(const rtl::OUString& rStr);
    const OUString&         GetName() const;

    const OutlinerParaObject*   GetTextObject() const;
    void SetTextObject( OutlinerParaObject* pText );

    sal_uInt32 GetNumberOfParagraphs() const;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
    virtual String      GetDescription() const;
};

/*--------------------------------------------------------------------
    DocumentInfo
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
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Extended User settings.
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

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


/*--------------------------------------------------------------------
    Relative page numbers - field.
 --------------------------------------------------------------------*/

class SwRefPageSetFieldType : public SwFieldType
{
public:
    SwRefPageSetFieldType();

    virtual SwFieldType*    Copy() const;

protected:
   /// Overlay, because there is nothing to update!
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem * );
};

/*--------------------------------------------------------------------
    Relative page numbering.
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

    virtual rtl::OUString  GetPar2() const;
    virtual void    SetPar2(const rtl::OUString& rStr);

    sal_Bool IsOn() const               { return bOn; }

    short GetOffset() const         { return nOffset; }
    void SetOffset( short nOff )    { nOffset = nOff; }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Relative page numbers - query field.
 --------------------------------------------------------------------*/

class SwRefPageGetFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType;

    void UpdateField( SwTxtFld* pTxtFld, _SetGetExpFlds& rSetList );
protected:
    /// overwritten to update all RefPageGet fields
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem * );
public:
    SwRefPageGetFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const;
    sal_uInt16 MakeSetList( _SetGetExpFlds& rTmpLst );
    SwDoc*  GetDoc() const                  { return pDoc; }
};

/*--------------------------------------------------------------------
    Query relative page numbering.
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
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Field to jump to and edit.
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
    rtl::OUString sTxt, sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, sal_uInt32 nFormat,
                    const String& sText, const String& sHelp );

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

    /// Placeholder-Text
    virtual const rtl::OUString& GetPar1() const;
    virtual void    SetPar1(const rtl::OUString& rStr);

    /// Hint-Text
    virtual rtl::OUString  GetPar2() const;
    virtual void    SetPar2(const rtl::OUString& rStr);

    SwCharFmt* GetCharFmt() const
        { return ((SwJumpEditFieldType*)GetTyp())->GetCharFmt(); }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Script Fieldtype.
 --------------------------------------------------------------------*/

class SwScriptFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwScriptFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    ScriptField.
 --------------------------------------------------------------------*/

class SwScriptField : public SwField
{
    rtl::OUString sType;  ///< Type of Code (Java/VBScript/...)
    rtl::OUString sCode;  /**< Code as text.
                          Code as JavaCode ? */

    sal_Bool    bCodeURL; ///< Code contains URL of a script.

public:
    SwScriptField( SwScriptFieldType*, const String& rType,
                   const String& rCode, sal_Bool bURL=sal_False );

    virtual String          GetDescription() const;

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    /// Type
    virtual const rtl::OUString&   GetPar1() const;
    virtual void            SetPar1(const rtl::OUString& rStr);
    /// Text
    virtual rtl::OUString   GetPar2() const;
    virtual void            SetPar2(const rtl::OUString& rStr);

    sal_Bool                    IsCodeURL() const { return bCodeURL; }
    void                    SetCodeURL( sal_Bool bURL ) { bCodeURL = bURL; }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Combined Character Fieldtype
 --------------------------------------------------------------------*/

class SwCombinedCharFieldType : public SwFieldType
{
public:
    SwCombinedCharFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    ScriptField
 --------------------------------------------------------------------*/

#define MAX_COMBINED_CHARACTERS     6

class SW_DLLPUBLIC SwCombinedCharField : public SwField
{
    rtl::OUString sCharacters;    ///< combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const String& rChars );

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    /// Characters
    virtual const rtl::OUString&   GetPar1() const;
    virtual void        SetPar1(const rtl::OUString& rStr);

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};


#endif // SW_DOCUFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
