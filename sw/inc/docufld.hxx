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
#ifndef INCLUDED_SW_INC_DOCUFLD_HXX
#define INCLUDED_SW_INC_DOCUFLD_HXX

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
class SwFmtFld;

enum SwAuthorFormat
{
    AF_BEGIN,
    AF_NAME = AF_BEGIN,
    AF_SHORTCUT,
    AF_END,
    AF_FIXED = 0x8000
};

// Subtype of document statistics.
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

class SwPageNumberFieldType : public SwFieldType
{
    sal_Int16   nNumberingType;
    bool            bVirtuell;

public:
    SwPageNumberFieldType();

    OUString Expand( sal_uInt32 nFmt, short nOff, sal_uInt16 const nPageNumber,
                     sal_uInt16 const nMaxPage, const OUString& ) const;
    void ChangeExpansion( SwDoc* pDoc,
                            bool bVirtPageNum, const sal_Int16* pNumFmt = 0 );
    virtual SwFieldType* Copy() const SAL_OVERRIDE;
};

// Page numbering.
class SW_DLLPUBLIC SwPageNumberField : public SwField
{
    OUString sUserStr;
    sal_uInt16  nSubType;
    short   nOffset;
    // fdo#58074 store page number in SwField, not SwFieldType
    sal_uInt16 m_nPageNumber;
    sal_uInt16 m_nMaxPage;

public:
    SwPageNumberField(SwPageNumberFieldType*, sal_uInt16 nSub = PG_RANDOM,
                      sal_uInt32 nFmt = 0, short nOff = 0,
                      sal_uInt16 const nPageNumber = 0,
                      sal_uInt16 const nMaxPage = 0);

    void ChangeExpansion(sal_uInt16 const nPageNumber,
            sal_uInt16 const nMaxPage);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    virtual OUString GetPar2() const SAL_OVERRIDE;
    virtual void        SetPar2(const OUString& rStr) SAL_OVERRIDE;

    virtual sal_uInt16  GetSubType() const SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;

    OUString GetUserString() const            { return sUserStr; }
    void SetUserString( const OUString& rS )  { sUserStr = rS; }
};

class SwAuthorFieldType : public SwFieldType
{
public:
    SwAuthorFieldType();

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SwAuthorField : public SwField
{
    OUString aContent;

public:
    SwAuthorField(SwAuthorFieldType*, sal_uInt32 nFmt = 0);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }
    inline OUString     GetContent() const { return aContent; }

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

class SwFileNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwFileNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwFileNameField : public SwField
{
    OUString aContent;

public:
    SwFileNameField(SwFileNameFieldType*, sal_uInt32 nFmt = 0);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }
    inline OUString     GetContent() const { return aContent; }

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

class SwTemplNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwTemplNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, sal_uInt32 nFmt = 0);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Document statistics
class SwDocStatFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType; ///< com::sun::star::style::NumberingType

public:
    SwDocStatFieldType(SwDoc*);
    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFmt) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;

    inline void             SetNumFormat( sal_Int16 eFmt )  { nNumberingType = eFmt; }
};

class SW_DLLPUBLIC SwDocStatField : public SwField
{
    sal_uInt16 nSubType;

public:
    SwDocStatField( SwDocStatFieldType*,
                    sal_uInt16 nSubType = 0, sal_uInt32 nFmt = 0);

    void ChangeExpansion( const SwFrm* pFrm );

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    virtual sal_uInt16      GetSubType() const SAL_OVERRIDE;
    virtual void        SetSubType(sal_uInt16 nSub) SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

class SwHiddenTxtFieldType : public SwFieldType
{
    bool bHidden;
public:
    SwHiddenTxtFieldType(bool bSetHidden = true);

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;

    void                    SetHiddenFlag( bool bSetHidden = true );
    inline bool             GetHiddenFlag() const { return bHidden; }
};

class SwHiddenTxtField : public SwField
{
    OUString aTRUETxt;         ///< Text if condition sal_True.
    OUString aFALSETxt;        ///< If condition false.
    OUString aContent;         ///< Evaluated DB-text.

    OUString aCond;            ///< Condition.
    sal_uInt16  nSubType;

    bool    bCanToggle : 1;     ///< Can field be toggled alone?
    bool    bIsHidden  : 1;     ///< Is it not visible?
    bool    bValid     : 1;     ///< Is DB-field evaluated?

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

public:
    SwHiddenTxtField( SwHiddenTxtFieldType*,
                     bool   bConditional,
                     const OUString& rCond,
                     const OUString& rTxt,
                     bool   bHidden  = false,
                     sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    SwHiddenTxtField( SwHiddenTxtFieldType*,
                      const OUString& rCond,
                      const OUString& rTrue,
                      const OUString& rFalse,
                      sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    virtual OUString    GetFieldName() const SAL_OVERRIDE;

    void                Evaluate(SwDoc*);

    inline void         SetValue(bool bHidden)  { bIsHidden = bHidden; }
    OUString            GetColumnName(const OUString& rName);
    OUString            GetDBName(const OUString& rName, SwDoc *pDoc);

    /// Condition
    virtual void        SetPar1(const OUString& rStr) SAL_OVERRIDE;
    virtual OUString    GetPar1() const SAL_OVERRIDE;

    /// True/False - String
    virtual void        SetPar2(const OUString& rStr) SAL_OVERRIDE;
    virtual OUString GetPar2() const SAL_OVERRIDE;

    virtual sal_uInt16      GetSubType() const SAL_OVERRIDE;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Field that expands to an empty line (without height).
class SwHiddenParaFieldType : public SwFieldType
{
public:
    SwHiddenParaFieldType();

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SwHiddenParaField : public SwField
{
    OUString           aCond;
    bool                bIsHidden:1;
public:
    /// Direct input, delete old value.
    SwHiddenParaField(SwHiddenParaFieldType*, const OUString& rCond);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    void                SetHidden(bool bHidden)     { bIsHidden = bHidden; }
    bool                IsHidden() const            { return bIsHidden;    }

    /// Query, set condition.
    virtual OUString    GetPar1() const SAL_OVERRIDE;
    virtual void        SetPar1(const OUString& rStr) SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

class SwMacroFieldType : public SwFieldType
{
    SwDoc* pDoc;

public:
    SwMacroFieldType(SwDoc*);

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwMacroField : public SwField
{
    OUString aMacro;
    OUString aText;
    bool      bIsScriptURL;

    virtual OUString Expand() const SAL_OVERRIDE;
    virtual SwField* Copy() const SAL_OVERRIDE;

public:
    /// Direct input, delete old value.
    SwMacroField( SwMacroFieldType*, const OUString& rLibAndName,
                  const OUString& rTxt);

    inline OUString  GetMacro() const { return aMacro; }
    OUString         GetLibName() const;
    OUString         GetMacroName() const;
    SvxMacro         GetSvxMacro() const;

    virtual OUString GetFieldName() const SAL_OVERRIDE;

    /// Library and FileName
    virtual OUString GetPar1() const SAL_OVERRIDE;
    virtual void     SetPar1(const OUString& rStr) SAL_OVERRIDE;

    /// Macrotext
    virtual OUString  GetPar2() const SAL_OVERRIDE;
    virtual void    SetPar2(const OUString& rStr) SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;

    static void CreateMacroString( OUString& rMacro,
                                   const OUString& rMacroName,
                                   const OUString& rLibraryName );

    static bool isScriptURL( const OUString& str );
};

class SwPostItFieldType : public SwFieldType
{
private:
    SwDoc* mpDoc;
public:
    SwPostItFieldType(SwDoc* pDoc);

    virtual SwFieldType* Copy() const SAL_OVERRIDE;

    SwDoc* GetDoc() const
    {
        return mpDoc;
    };
};

class SW_DLLPUBLIC SwPostItField : public SwField
{
    OUString sTxt;
    OUString sAuthor;
    OUString sInitials; ///< Initials of the author.
    OUString sName;     ///< Name of the comment.
    DateTime    aDateTime;
    OutlinerParaObject* mpText;
    SwTextAPIObject* m_pTextObject;

public:
    SwPostItField( SwPostItFieldType*,
                   const OUString& rAuthor,
                   const OUString& rTxt,
                   const OUString& rInitials,
                   const OUString& rName,
                   const DateTime& rDate);
    virtual ~SwPostItField();

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

    inline const DateTime   GetDateTime() const             { return aDateTime; }
    inline const Date       GetDate() const                 { return aDateTime.GetDate(); }
    inline const Time       GetTime() const                 { return aDateTime.GetTime(); }

    /// Author
    virtual OUString        GetPar1() const SAL_OVERRIDE;
    virtual void            SetPar1(const OUString& rStr) SAL_OVERRIDE;

    /// Text
    virtual OUString   GetPar2() const SAL_OVERRIDE;
    virtual void            SetPar2(const OUString& rStr) SAL_OVERRIDE;
    const OUString&         GetTxt() const { return sTxt; }
    const OUString&         GetInitials() const { return sInitials;}
    void                    SetName(const OUString& rStr);
    const OUString&         GetName() const { return sName;}

    const OutlinerParaObject* GetTextObject() const { return mpText;}
    void SetTextObject( OutlinerParaObject* pText );

    sal_Int32 GetNumberOfParagraphs() const;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
    virtual OUString    GetDescription() const SAL_OVERRIDE;
};

class SwDocInfoFieldType : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFormat, sal_uInt16 nLang, const OUString& rName) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwDocInfoField : public SwValueField
{
    sal_uInt16  nSubType;
    OUString  aContent;
    OUString  aName;

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

public:
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, sal_uInt32 nFmt=0);
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFmt=0);

    virtual void            SetSubType(sal_uInt16) SAL_OVERRIDE;
    virtual sal_uInt16          GetSubType() const SAL_OVERRIDE;
    virtual void            SetLanguage(sal_uInt16 nLng) SAL_OVERRIDE;
    virtual OUString        GetFieldName() const SAL_OVERRIDE;
    OUString                GetName() const { return aName; }
    void                    SetName( const OUString& rName ) { aName = rName; }
    inline void             SetExpansion(const OUString& rStr) { aContent = rStr; }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Extended User settings.
class SwExtUserFieldType : public SwFieldType
{
    OUString aData;
public:
    SwExtUserFieldType();

    inline void             SetData(const OUString& rStr)     { aData = rStr; }

    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFormat) const;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SwExtUserField : public SwField
{
    OUString aContent;
    sal_uInt16  nType;

public:
    SwExtUserField(SwExtUserFieldType*, sal_uInt16 nSub, sal_uInt32 nFmt=0);

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    virtual sal_uInt16      GetSubType() const SAL_OVERRIDE;
    virtual void        SetSubType(sal_uInt16 nSub) SAL_OVERRIDE;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Relative page numbers - field.
class SwRefPageSetFieldType : public SwFieldType
{
public:
    SwRefPageSetFieldType();

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;

protected:
   /// Overlay, because there is nothing to update!
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem * ) SAL_OVERRIDE;
};

// Relative page numbering.
class SwRefPageSetField : public SwField
{
    short   nOffset;
    bool    bOn;

public:
    SwRefPageSetField( SwRefPageSetFieldType*, short nOff = 0,
                        bool bOn = true );

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    virtual OUString  GetPar2() const SAL_OVERRIDE;
    virtual void    SetPar2(const OUString& rStr) SAL_OVERRIDE;

    bool IsOn() const               { return bOn; }

    short GetOffset() const         { return nOffset; }
    void SetOffset( short nOff )    { nOffset = nOff; }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Relative page numbers - query field.
class SwRefPageGetFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType;

    void UpdateField( SwTxtFld* pTxtFld, _SetGetExpFlds& rSetList );
protected:
    /// overwritten to update all RefPageGet fields
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem * ) SAL_OVERRIDE;
public:
    SwRefPageGetFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
    sal_uInt16 MakeSetList( _SetGetExpFlds& rTmpLst );
    SwDoc*  GetDoc() const                  { return pDoc; }
};

// Query relative page numbering.
class SwRefPageGetField : public SwField
{
    OUString sTxt;
public:
    SwRefPageGetField( SwRefPageGetFieldType*, sal_uInt32 nFmt );

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    void SetText( const OUString& rTxt )      { sTxt = rTxt; }

    void ChangeExpansion( const SwFrm* pFrm, const SwTxtFld* pFld );
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Field to jump to and edit.
class SwJumpEditFieldType : public SwFieldType
{
    SwDoc* pDoc;
    SwDepend aDep;

public:
    SwJumpEditFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;

    SwCharFmt* GetCharFmt();
};

class SwJumpEditField : public SwField
{
    OUString sTxt, sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, sal_uInt32 nFormat,
                     const OUString& sText, const OUString& sHelp );

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

    /// Placeholder-Text
    virtual OUString GetPar1() const SAL_OVERRIDE;
    virtual void    SetPar1(const OUString& rStr) SAL_OVERRIDE;

    /// Hint-Text
    virtual OUString  GetPar2() const SAL_OVERRIDE;
    virtual void    SetPar2(const OUString& rStr) SAL_OVERRIDE;

    SwCharFmt* GetCharFmt() const
        { return ((SwJumpEditFieldType*)GetTyp())->GetCharFmt(); }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

class SwScriptFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwScriptFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

class SwScriptField : public SwField
{
    OUString sType;  ///< Type of Code (Java/VBScript/...)
    OUString sCode;  /**< Code as text.
                          Code as JavaCode ? */

    bool    bCodeURL; ///< Code contains URL of a script.

public:
    SwScriptField( SwScriptFieldType*, const OUString& rType,
                   const OUString& rCode, bool bURL=false );

    virtual OUString        GetDescription() const SAL_OVERRIDE;

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

    /// Type
    virtual OUString        GetPar1() const SAL_OVERRIDE;
    virtual void            SetPar1(const OUString& rStr) SAL_OVERRIDE;
    /// Text
    virtual OUString   GetPar2() const SAL_OVERRIDE;
    virtual void            SetPar2(const OUString& rStr) SAL_OVERRIDE;

    bool                    IsCodeURL() const { return bCodeURL; }
    void                    SetCodeURL( bool bURL ) { bCodeURL = bURL; }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Combined Character Fieldtype
class SwCombinedCharFieldType : public SwFieldType
{
public:
    SwCombinedCharFieldType();

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

// ScriptField
#define MAX_COMBINED_CHARACTERS     6

class SW_DLLPUBLIC SwCombinedCharField : public SwField
{
    OUString sCharacters;    ///< combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const OUString& rChars );

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

    /// Characters
    virtual OUString    GetPar1() const SAL_OVERRIDE;
    virtual void        SetPar1(const OUString& rStr) SAL_OVERRIDE;

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

#endif // INCLUDED_SW_INC_DOCUFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
