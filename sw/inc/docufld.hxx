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

#include <tools/solar.h>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <rtl/ref.hxx>
#include <svl/macitem.hxx>

#include "fldbas.hxx"

class SetGetExpFields;
class SwTextField;
class SwFrame;
class OutlinerParaObject;
class SwTextAPIObject;
class SwCharFormat;

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
    const SwDocInfoSubType DI_TITLE         =  DI_SUBTYPE_BEGIN;
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
    SvxNumType      m_nNumberingType;
    bool            m_bVirtual;

public:
    SwPageNumberFieldType();

    OUString Expand( SvxNumType nFormat, short nOff, sal_uInt16 const nPageNumber,
                     sal_uInt16 const nMaxPage, const OUString&, LanguageType = LANGUAGE_NONE ) const;
    void ChangeExpansion( SwDoc* pDoc,
                          bool bVirtPageNum, const SvxNumType* pNumFormat );
    virtual SwFieldType* Copy() const override;
};

// Page numbering.
class SW_DLLPUBLIC SwPageNumberField : public SwField
{
    OUString m_sUserStr;
    sal_uInt16  m_nSubType;
    short   m_nOffset;
    // fdo#58074 store page number in SwField, not SwFieldType
    sal_uInt16 m_nPageNumber;
    sal_uInt16 m_nMaxPage;

public:
    SwPageNumberField(SwPageNumberFieldType*, sal_uInt16 nSub,
                      sal_uInt32 nFormat, short nOff = 0,
                      sal_uInt16 const nPageNumber = 0,
                      sal_uInt16 const nMaxPage = 0);

    void ChangeExpansion(sal_uInt16 const nPageNumber,
            sal_uInt16 const nMaxPage);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    virtual OUString GetPar2() const override;
    virtual void        SetPar2(const OUString& rStr) override;

    virtual sal_uInt16  GetSubType() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    const OUString& GetUserString() const            { return m_sUserStr; }
    void SetUserString( const OUString& rS )  { m_sUserStr = rS; }
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
    OUString m_aContent;

public:
    SwAuthorField(SwAuthorFieldType*, sal_uInt32 nFormat);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwFileNameFieldType : public SwFieldType
{
    SwDoc *m_pDoc;
public:
    SwFileNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwFileNameField : public SwField
{
    OUString m_aContent;

public:
    SwFileNameField(SwFileNameFieldType*, sal_uInt32 nFormat);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwTemplNameFieldType : public SwFieldType
{
    SwDoc *m_pDoc;
public:
    SwTemplNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwTemplNameField : public SwField
{
public:
    SwTemplNameField(SwTemplNameFieldType*, sal_uInt32 nFormat);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Document statistics
class SwDocStatFieldType : public SwFieldType
{
    SwDoc*          m_pDoc;
    SvxNumType      m_nNumberingType;

public:
    SwDocStatFieldType(SwDoc*);
    OUString                Expand(sal_uInt16 nSubType, SvxNumType nFormat) const;
    virtual SwFieldType*    Copy() const override;

    void             SetNumFormat( SvxNumType eFormat )  { m_nNumberingType = eFormat; }
};

class SW_DLLPUBLIC SwDocStatField : public SwField
{
    sal_uInt16 m_nSubType;

public:
    SwDocStatField( SwDocStatFieldType*,
                    sal_uInt16 nSubType, sal_uInt32 nFormat);

    void ChangeExpansion( const SwFrame* pFrame );

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nSub) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwHiddenTextFieldType : public SwFieldType
{
    bool m_bHidden;
public:
    SwHiddenTextFieldType(bool bSetHidden = true);

    virtual SwFieldType*    Copy() const override;

    void                    SetHiddenFlag( bool bSetHidden );
    bool             GetHiddenFlag() const { return m_bHidden; }
};

class SW_DLLPUBLIC SwHiddenTextField : public SwField
{
    OUString m_aTRUEText; ///< Text if condition true.
    OUString m_aFALSEText; ///< If condition false.
    OUString m_aContent; ///< Evaluated DB-text.

    OUString m_aCond; ///< Condition.
    sal_uInt16 m_nSubType;

    bool m_bCanToggle : 1; ///< Can field be toggled alone?
    bool m_bIsHidden : 1; ///< Is it not visible?
    bool m_bValid : 1; ///< Is DB-field evaluated?

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwHiddenTextField( SwHiddenTextFieldType*,
                     bool   bConditional,
                     const OUString& rCond,
                     const OUString& rText,
                     bool   bHidden,
                     sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    SwHiddenTextField( SwHiddenTextFieldType*,
                      const OUString& rCond,
                      const OUString& rTrue,
                      const OUString& rFalse,
                      sal_uInt16 nSubType = TYP_HIDDENTXTFLD);

    virtual OUString    GetFieldName() const override;

    void                Evaluate(SwDoc*);

    void         SetValue(bool bHidden)  { m_bIsHidden = bHidden; }
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

    static void         ParseIfFieldDefinition(const OUString& aFieldDefinition,
                                               OUString& rCondition,
                                               OUString& rTrue,
                                               OUString& rFalse);
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
    OUString m_aCond;
    bool m_bIsHidden;
public:
    /// Direct input, delete old value.
    SwHiddenParaField(SwHiddenParaFieldType*, const OUString& rCond);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void                SetHidden(bool bHidden)     { m_bIsHidden = bHidden; }
    bool                IsHidden() const            { return m_bIsHidden;    }

    /// Query, set condition.
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SwMacroFieldType : public SwFieldType
{
    SwDoc* const m_pDoc;

public:
    SwMacroFieldType(SwDoc*);

    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwMacroField : public SwField
{
    OUString m_aMacro;
    OUString m_aText;
    bool      m_bIsScriptURL;

    virtual OUString ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    /// Direct input, delete old value.
    SwMacroField( SwMacroFieldType*, const OUString& rLibAndName,
                  const OUString& rText);

    const OUString&  GetMacro() const { return m_aMacro; }
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
    SwDoc* const mpDoc;
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
    OUString m_sText;
    OUString m_sAuthor;
    OUString m_sInitials; ///< Initials of the author.
    OUString m_sName;     ///< Name of the comment.
    DateTime    m_aDateTime;
    std::unique_ptr<OutlinerParaObject> mpText;
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

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    const DateTime&         GetDateTime() const             { return m_aDateTime; }
    const Date       GetDate() const                 { return Date(m_aDateTime.GetDate()); }
    const tools::Time GetTime() const                 { return tools::Time(m_aDateTime.GetTime()); }
    sal_uInt32 GetPostItId() const             { return m_nPostItId; }

    /// Author
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;

    /// Text
    virtual OUString   GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;
    const OUString&         GetText() const { return m_sText; }
    const OUString&         GetInitials() const { return m_sInitials;}
    void                    SetName(const OUString& rStr);
    const OUString&         GetName() const { return m_sName;}

    const OutlinerParaObject* GetTextObject() const { return mpText.get();}
    void SetTextObject( std::unique_ptr<OutlinerParaObject> pText );

    sal_Int32 GetNumberOfParagraphs() const;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
    virtual OUString    GetDescription() const override;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwDocInfoFieldType : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    OUString                Expand(sal_uInt16 nSubType, sal_uInt32 nFormat, LanguageType nLang, const OUString& rName) const;
    virtual SwFieldType*    Copy() const override;
};

class SW_DLLPUBLIC SwDocInfoField : public SwValueField
{
    sal_uInt16  m_nSubType;
    OUString  m_aContent;
    OUString  m_aName;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, sal_uInt32 nFormat=0);
    SwDocInfoField(SwDocInfoFieldType*, sal_uInt16 nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFormat=0);

    virtual void            SetSubType(sal_uInt16) override;
    virtual sal_uInt16      GetSubType() const override;
    virtual void            SetLanguage(LanguageType nLng) override;
    virtual OUString        GetFieldName() const override;
    const OUString&         GetName() const { return m_aName; }
    void                    SetName( const OUString& rName ) { m_aName = rName; }
    void             SetExpansion(const OUString& rStr) { m_aContent = rStr; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Extended User settings.
class SwExtUserFieldType : public SwFieldType
{
public:
    SwExtUserFieldType();

    static OUString         Expand(sal_uInt16 nSubType);
    virtual SwFieldType*    Copy() const override;
};

class SwExtUserField : public SwField
{
    OUString m_aContent;
    sal_uInt16  m_nType;

public:
    SwExtUserField(SwExtUserFieldType*, sal_uInt16 nSub, sal_uInt32 nFormat);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nSub) override;

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

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
    short   m_nOffset;
    bool    m_bOn;

public:
    SwRefPageSetField( SwRefPageSetFieldType*, short nOff, bool bOn );

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    virtual OUString  GetPar2() const override;
    virtual void    SetPar2(const OUString& rStr) override;

    bool IsOn() const               { return m_bOn; }

    short GetOffset() const         { return m_nOffset; }
    void SetOffset( short nOff )    { m_nOffset = nOff; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Relative page numbers - query field.
class SwRefPageGetFieldType : public SwFieldType
{
    SwDoc*          m_pDoc;
    sal_Int16       m_nNumberingType;

    void UpdateField(SwTextField const * pTextField,
            SetGetExpFields const & rSetList, SwRootFrame const* pLayout);

protected:
    /// overwritten to update all RefPageGet fields
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem * ) override;
public:
    SwRefPageGetFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const override;
    bool MakeSetList(SetGetExpFields& rTmpLst, SwRootFrame const* pLayout);
    SwDoc*  GetDoc() const                  { return m_pDoc; }
};

// Query relative page numbering.
class SwRefPageGetField : public SwField
{
    OUString m_sText;
    OUString m_sTextRLHidden; ///< hidden redlines

public:
    SwRefPageGetField( SwRefPageGetFieldType*, sal_uInt32 nFormat );

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void SetText(const OUString& rText, SwRootFrame const* pLayout);

    void ChangeExpansion(const SwFrame& rFrame, const SwTextField* pField);
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Field to jump to and edit.
class SwJumpEditFieldType : public SwFieldType
{
    SwDoc* m_pDoc;
    sw::WriterMultiListener m_aDep;

public:
    SwJumpEditFieldType( SwDoc* pDoc );
    virtual SwFieldType*    Copy() const override;

    SwCharFormat* GetCharFormat();
};

class SwJumpEditField : public SwField
{
    OUString m_sText;
    OUString m_sHelp;
public:
    SwJumpEditField( SwJumpEditFieldType*, sal_uInt32 nFormat,
                     const OUString& sText, const OUString& sHelp );

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

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
    SwDoc* const m_pDoc;
public:
    SwScriptFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const override;
};

class SwScriptField : public SwField
{
    OUString m_sType;  ///< Type of Code (Java/VBScript/...)
    OUString m_sCode;  /**< Code as text.
                          Code as JavaCode ? */

    bool    m_bCodeURL; ///< Code contains URL of a script.

public:
    SwScriptField( SwScriptFieldType*, const OUString& rType,
                   const OUString& rCode, bool bURL );

    virtual OUString        GetDescription() const override;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    /// Type
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;
    /// Text
    virtual OUString   GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;

    bool                    IsCodeURL() const { return m_bCodeURL; }
    void                    SetCodeURL( bool bURL ) { m_bCodeURL = bURL; }
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
    OUString m_sCharacters;    ///< combine these characters

public:
    SwCombinedCharField( SwCombinedCharFieldType*, const OUString& rChars );

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    /// Characters
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

#endif // INCLUDED_SW_INC_DOCUFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
