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
#include <rtl/ref.hxx>
#include <svl/macitem.hxx>

#include "fldbas.hxx"
#include "numrule.hxx"

class SetGetExpFields;
class SwTextField;
class SwFrame;
class OutlinerParaObject;
class SwTextAPIObject;
class SwFormatField;

enum SwAuthorFormat
{
    AF_BEGIN,
    AF_NAME = AF_BEGIN,
    AF_SHORTCUT,
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

enum SwPageNumSubType
{
    PG_RANDOM,
    PG_NEXT,
    PG_PREV
};

// NOTE: Possibly the first 15 values in the below enum are required
// to exactly match the published constants in css::text::UserDataPart
// (see offapi/com/sun/star/text/UserDataPart.idl).

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

    OUString Expand( sal_uInt32 nFormat, short nOff, sal_uInt16 const nPageNumber,
                     sal_uInt16 const nMaxPage, const OUString& ) const;
    void ChangeExpansion( SwDoc* pDoc,
                          bool bVirtPageNum, const sal_Int16* pNumFormat );
    virtual SwFieldType* Copy() const override;
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
    SwPageNumberField(SwPageNumberFieldType*, sal_uInt16 nSub,
                      sal_uInt32 nFormat = 0, short nOff = 0,
                      sal_uInt16 const nPageNumber = 0,
                      sal_uInt16 const nMaxPage = 0);

    void ChangeExpansion(sal_uInt16 const nPageNumber,
            sal_uInt16 const nMaxPage);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    virtual OUString GetPar2() const override;
    virtual void        SetPar2(const OUString& rStr) override;

    virtual sal_uInt16  GetSubType() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    const OUString& GetUserString() const            { return sUserStr; }
    void SetUserString( const OUString& rS )  { sUserStr = rS; }
};

class SwAuthorFieldType : public SwFieldType
{
public:
    SwAuthorFieldType();

    static OUString         Expand(sal_uLong);
    virtual SwFieldType*    Copy() const override;
};

class SwAuthorField : public SwField
{
    OUString aContent;

public:
    SwAuthorField(SwAuthorFieldType*, sal_uInt32 nFormat);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwFileNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwFileNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwFileNameField : public SwField
{
    OUString aContent;

public:
    SwFileNameField(SwFileNameFieldType*, sal_uInt32 nFormat);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwTemplNameFieldType : public SwFieldType
{
    SwDoc *pDoc;
public:
    SwTemplNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, sal_uInt32 nFormat);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Document statistics
class SwDocStatFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType; ///< css::style::NumberingType

public:
    SwDocStatFieldType(SwDoc*);
    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFormat) const;
    virtual SwFieldType*    Copy() const override;

    inline void             SetNumFormat( sal_Int16 eFormat )  { nNumberingType = eFormat; }
};

class SW_DLLPUBLIC SwDocStatField : public SwField
{
    sal_uInt16 nSubType;

public:
    SwDocStatField( SwDocStatFieldType*,
                    sal_uInt16 nSubType, sal_uInt32 nFormat = 0);

    void ChangeExpansion( const SwFrame* pFrame );

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nSub) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwHiddenTextFieldType : public SwFieldType
{
    bool bHidden;
public:
    SwHiddenTextFieldType(bool bSetHidden = true);

    virtual SwFieldType*    Copy() const override;

    void                    SetHiddenFlag( bool bSetHidden );
    inline bool             GetHiddenFlag() const { return bHidden; }
};

class SwHiddenTextField : public SwField
{
    OUString aTRUEText;         ///< Text if condition true.
    OUString aFALSEText;        ///< If condition false.
    OUString aContent;         ///< Evaluated DB-text.

    OUString aCond;            ///< Condition.
    sal_uInt16  nSubType;

    bool    bCanToggle : 1;     ///< Can field be toggled alone?
    bool    bIsHidden  : 1;     ///< Is it not visible?
    bool    bValid     : 1;     ///< Is DB-field evaluated?

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

public:
    SwHiddenTextField( SwHiddenTextFieldType*,
                     bool   bConditional,
                     const OUString& rCond,
                     const OUString& rText,
                     bool   bHidden  = false,
                     sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    SwHiddenTextField( SwHiddenTextFieldType*,
                      const OUString& rCond,
                      const OUString& rTrue,
                      const OUString& rFalse,
                      sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    virtual OUString    GetFieldName() const override;

    void                Evaluate(SwDoc*);

    inline void         SetValue(bool bHidden)  { bIsHidden = bHidden; }
    static OUString     GetColumnName(const OUString& rName);
    static OUString     GetDBName(const OUString& rName, SwDoc *pDoc);

    /// Condition
    virtual void        SetPar1(const OUString& rStr) override;
    virtual OUString    GetPar1() const override;

    /// True/False - String
    virtual void        SetPar2(const OUString& rStr) override;
    virtual OUString    GetPar2() const override;

    virtual sal_uInt16  GetSubType() const override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Field that expands to an empty line (without height).
class SwHiddenParaFieldType : public SwFieldType
{
public:
    SwHiddenParaFieldType();

    virtual SwFieldType*    Copy() const override;
};

class SwHiddenParaField : public SwField
{
    OUString           aCond;
    bool                bIsHidden:1;
public:
    /// Direct input, delete old value.
    SwHiddenParaField(SwHiddenParaFieldType*, const OUString& rCond);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    void                SetHidden(bool bHidden)     { bIsHidden = bHidden; }
    bool                IsHidden() const            { return bIsHidden;    }

    /// Query, set condition.
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwMacroFieldType : public SwFieldType
{
    SwDoc* pDoc;

public:
    SwMacroFieldType(SwDoc*);

    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwMacroField : public SwField
{
    OUString aMacro;
    OUString aText;
    bool      bIsScriptURL;

    virtual OUString Expand() const override;
    virtual SwField* Copy() const override;

public:
    /// Direct input, delete old value.
    SwMacroField( SwMacroFieldType*, const OUString& rLibAndName,
                  const OUString& rText);

    const OUString&  GetMacro() const { return aMacro; }
    OUString         GetLibName() const;
    OUString         GetMacroName() const;
    SvxMacro         GetSvxMacro() const;

    virtual OUString GetFieldName() const override;

    /// Library and FileName
    virtual OUString GetPar1() const override;
    virtual void     SetPar1(const OUString& rStr) override;

    /// Macrotext
    virtual OUString  GetPar2() const override;
    virtual void    SetPar2(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

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

    virtual SwFieldType* Copy() const override;

    SwDoc* GetDoc() const
    {
        return mpDoc;
    };
};

class SW_DLLPUBLIC SwPostItField : public SwField
{
    OUString sText;
    OUString sAuthor;
    OUString sInitials; ///< Initials of the author.
    OUString sName;     ///< Name of the comment.
    DateTime    aDateTime;
    OutlinerParaObject* mpText;
    rtl::Reference<SwTextAPIObject> m_xTextObject;
    sal_uInt32 m_nPostItId;

public:
    static sal_uInt32 m_nLastPostItId;

    SwPostItField( SwPostItFieldType*,
                   const OUString& rAuthor,
                   const OUString& rText,
                   const OUString& rInitials,
                   const OUString& rName,
                   const DateTime& rDate,
                   const sal_uInt32 nPostItId = 0);

    SwPostItField(const SwPostItField&) = delete;
    SwPostItField* operator=(const SwPostItField&) = delete;

    virtual ~SwPostItField() override;

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

    const DateTime&         GetDateTime() const             { return aDateTime; }
    inline const Date       GetDate() const                 { return Date(aDateTime.GetDate()); }
    inline const tools::Time GetTime() const                 { return aDateTime.GetTime(); }
    inline sal_uInt32 GetPostItId() const             { return m_nPostItId; }

    /// Author
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;

    /// Text
    virtual OUString   GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;
    const OUString&         GetText() const { return sText; }
    const OUString&         GetInitials() const { return sInitials;}
    void                    SetName(const OUString& rStr);
    const OUString&         GetName() const { return sName;}

    const OutlinerParaObject* GetTextObject() const { return mpText;}
    void SetTextObject( OutlinerParaObject* pText );

    sal_Int32 GetNumberOfParagraphs() const;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
    virtual OUString    GetDescription() const override;
    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

class SwDocInfoFieldType : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFormat, sal_uInt16 nLang, const OUString& rName) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwDocInfoField : public SwValueField
{
    sal_uInt16  nSubType;
    OUString  aContent;
    OUString  aName;

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

public:
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, sal_uInt32 nFormat=0);
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFormat=0);

    virtual void            SetSubType(sal_uInt16) override;
    virtual sal_uInt16      GetSubType() const override;
    virtual void            SetLanguage(sal_uInt16 nLng) override;
    virtual OUString        GetFieldName() const override;
    const OUString&         GetName() const { return aName; }
    void                    SetName( const OUString& rName ) { aName = rName; }
    inline void             SetExpansion(const OUString& rStr) { aContent = rStr; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Extended User settings.
class SwExtUserFieldType : public SwFieldType
{
public:
    SwExtUserFieldType();

    static OUString         Expand(sal_uInt16 nSubType, sal_uInt32 nFormat);
    virtual SwFieldType*    Copy() const override;
};

class SwExtUserField : public SwField
{
    OUString aContent;
    sal_uInt16  nType;

public:
    SwExtUserField(SwExtUserFieldType*, sal_uInt16 nSub, sal_uInt32 nFormat);

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nSub) override;

    inline void         SetExpansion(const OUString& rStr) { aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Relative page numbers - field.
class SwRefPageSetFieldType : public SwFieldType
{
public:
    SwRefPageSetFieldType();

    virtual SwFieldType*    Copy() const override;

protected:
    /// Overlay, because there is nothing to update!
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem * ) override;
};

// Relative page numbering.
class SwRefPageSetField : public SwField
{
    short   nOffset;
    bool    bOn;

public:
    SwRefPageSetField( SwRefPageSetFieldType*, short nOff,
                        bool bOn = true );

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    virtual OUString  GetPar2() const override;
    virtual void    SetPar2(const OUString& rStr) override;

    bool IsOn() const               { return bOn; }

    short GetOffset() const         { return nOffset; }
    void SetOffset( short nOff )    { nOffset = nOff; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Relative page numbers - query field.
class SwRefPageGetFieldType : public SwFieldType
{
    SwDoc*          pDoc;
    sal_Int16       nNumberingType;

    void UpdateField( SwTextField* pTextField, SetGetExpFields& rSetList );
protected:
    /// overwritten to update all RefPageGet fields
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem * ) override;
public:
    SwRefPageGetFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const override;
    bool MakeSetList( SetGetExpFields& rTmpLst );
    SwDoc*  GetDoc() const                  { return pDoc; }
};

// Query relative page numbering.
class SwRefPageGetField : public SwField
{
    OUString sText;
public:
    SwRefPageGetField( SwRefPageGetFieldType*, sal_uInt32 nFormat );

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    void SetText( const OUString& rText )      { sText = rText; }

    void ChangeExpansion( const SwFrame* pFrame, const SwTextField* pField );
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Field to jump to and edit.
class SwJumpEditFieldType : public SwFieldType
{
    SwDoc* pDoc;
    SwDepend aDep;

public:
    SwJumpEditFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const override;

    SwCharFormat* GetCharFormat();
};

class SwJumpEditField : public SwField
{
    OUString sText, sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, sal_uInt32 nFormat,
                     const OUString& sText, const OUString& sHelp );

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

    /// Placeholder-Text
    virtual OUString GetPar1() const override;
    virtual void    SetPar1(const OUString& rStr) override;

    /// Hint-Text
    virtual OUString  GetPar2() const override;
    virtual void    SetPar2(const OUString& rStr) override;

    SwCharFormat* GetCharFormat() const
        { return static_cast<SwJumpEditFieldType*>(GetTyp())->GetCharFormat(); }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwScriptFieldType : public SwFieldType
{
    SwDoc* pDoc;
public:
    SwScriptFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const override;
};

class SwScriptField : public SwField
{
    OUString sType;  ///< Type of Code (Java/VBScript/...)
    OUString sCode;  /**< Code as text.
                          Code as JavaCode ? */

    bool    bCodeURL; ///< Code contains URL of a script.

public:
    SwScriptField( SwScriptFieldType*, const OUString& rType,
                   const OUString& rCode, bool bURL );

    virtual OUString        GetDescription() const override;

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

    /// Type
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;
    /// Text
    virtual OUString   GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;

    bool                    IsCodeURL() const { return bCodeURL; }
    void                    SetCodeURL( bool bURL ) { bCodeURL = bURL; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Combined Character Fieldtype
class SwCombinedCharFieldType : public SwFieldType
{
public:
    SwCombinedCharFieldType();

    virtual SwFieldType*    Copy() const override;
};

// ScriptField
#define MAX_COMBINED_CHARACTERS     6

class SW_DLLPUBLIC SwCombinedCharField : public SwField
{
    OUString sCharacters;    ///< combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const OUString& rChars );

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

    /// Characters
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

#endif // INCLUDED_SW_INC_DOCUFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
