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
#pragma once

#include <sal/config.h>

#include <config_collab.h>

#include <string_view>

#include <editeng/outlobj.hxx>
#include <tools/solar.h>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <rtl/ref.hxx>
#include <svl/macitem.hxx>

#include "fldbas.hxx"
#include "names.hxx"

class SetGetExpFields;
class SwTextField;
class SwFrame;
class SwTextAPIObject;
class SwCharFormat;

enum class SwAuthorFormat : sal_uInt32
{
    // most of the constants are a regular enum
    Name,
    Shortcut,
    Mask = Name | Shortcut, // mask off the enum part
    // except for this, which is a flag
    Fixed = 0x8000
};
namespace o3tl { template<> struct typed_flags<SwAuthorFormat> : is_typed_flags<SwAuthorFormat, 0x8003> {}; }

// Subtype of document statistics.
enum class SwDocStatSubType : sal_uInt16
{
    Page,
    // page count in current section
    PageRange,
    Paragraph,
    Word,
    Character,
    Table,
    Graphic,
    OLE
};

enum class SwDocInfoSubType : sal_uInt16
{
    /** NB: these must denote consecutive integers!
     NB2: these are extended by 4 DI_INFO values for backward compatibility
          in filter/html/htmlfld.cxx, so make sure that DI_SUBTYPE_END
          really is the end, and is at least 4 less than DI_SUB_*! */
    SubtypeBegin =  0,
    Title         = SubtypeBegin,
    Subject       =  1,
    Keys          =  2,
    Comment       =  3,
    Create        =  4,
    Change        =  5,
    Print         =  6,
    DocNo         =  7,
    Edit          =  8,
    Custom        =  9,
    SubtypeEnd    = 10,
    LowerMask     = 0x00ff,
    UpperMask     = 0xff00,

    // UGLY: these are necessary for importing document info fields written by
    //       older versions of OOo (< 3.0) which did not have Custom fields
    Info1         =  SubtypeEnd + 1,
    Info2         =  SubtypeEnd + 2,
    Info3         =  SubtypeEnd + 3,
    Info4         =  SubtypeEnd + 4,

    SubAuthor     = 0x0100,
    SubTime       = 0x0200,
    SubDate       = 0x0300,
    SubFixed      = 0x1000,
    SubMask       = 0x0f00,

    Max           = 0xffff // used as a flag by SwFieldDokInfPage
};
namespace o3tl { template<> struct typed_flags<SwDocInfoSubType> : is_typed_flags<SwDocInfoSubType, 0xffff> {}; }

enum class SwPageNumSubType
{
    Random,
    Next,
    Previous
};

// NOTE: Possibly the first 15 values in the below enum are required
// to exactly match the published constants in css::text::UserDataPart
// (see offapi/com/sun/star/text/UserDataPart.idl).

enum class SwExtUserSubType : sal_uInt16
{
    Company,
    Firstname,
    Name,
    Shortcut,
    Street,
    Country,
    Zip,
    City,
    Title,
    Position,
    PhonePrivate,
    PhoneCompany,
    Fax,
    Email,
    State,
    FathersName,
    Apartment
};

enum class SwJumpEditFormat
{
    Text,
    Table,
    Frame,
    Graphic,
    OLE
};

class SAL_DLLPUBLIC_RTTI SwPageNumberFieldType final : public SwFieldType
{
    SvxNumType      m_nNumberingType;
    bool            m_bVirtual;

public:
    SwPageNumberFieldType();

    OUString Expand( SvxNumType nFormat, short nOff, sal_uInt16 const nPageNumber,
                     sal_uInt16 const nMaxPage, const OUString&, LanguageType = LANGUAGE_NONE ) const;
    void ChangeExpansion( SwDoc* pDoc,
                          bool bVirtPageNum, const SvxNumType* pNumFormat );
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

// Page numbering.
class SW_DLLPUBLIC SwPageNumberField final : public SwField
{
    OUString m_sUserStr;
    SwPageNumSubType  m_nSubType;
    short   m_nOffset;
    // fdo#58074 store page number in SwField, not SwFieldType
    sal_uInt16 m_nPageNumber;
    sal_uInt16 m_nMaxPage;
    SvxNumType m_nFormat;

public:
    SwPageNumberField(SwPageNumberFieldType*, SwPageNumSubType nSub,
                      SvxNumType nFormat, short nOff = 0,
                      sal_uInt16 const nPageNumber = 0,
                      sal_uInt16 const nMaxPage = 0);

    SvxNumType GetFormat() const { return m_nFormat; }
    void SetFormat(SvxNumType n) { m_nFormat = n; }

    void ChangeExpansion(sal_uInt16 const nPageNumber,
            sal_uInt16 const nMaxPage);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    virtual OUString GetPar2() const override;
    virtual void        SetPar2(const OUString& rStr) override;

    SwPageNumSubType    GetSubType() const;
    void                SetSubType(SwPageNumSubType n) { m_nSubType = n; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    const OUString& GetUserString() const            { return m_sUserStr; }
    void SetUserString( const OUString& rS )  { m_sUserStr = rS; }
};

class SwAuthorFieldType final : public SwFieldType
{
public:
    SwAuthorFieldType();

    static OUString         Expand(SwAuthorFormat);
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SAL_DLLPUBLIC_RTTI SwAuthorField final : public SwField
{
    OUString m_aContent;
    SwAuthorFormat m_nFormat;

public:
    SwAuthorField(SwAuthorFieldType*, SwAuthorFormat nFormat);

    SwAuthorFormat GetFormat() const { return m_nFormat; }
    void SetFormat(SwAuthorFormat n) { m_nFormat = n; }

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SAL_DLLPUBLIC_RTTI SwFileNameFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;
public:
    SwFileNameFieldType(SwDoc&);

    OUString                Expand(SwFileNameFormat) const;
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SW_DLLPUBLIC SwFileNameField final : public SwField
{
    OUString m_aContent;
    SwFileNameFormat m_nFormat;

public:
    SwFileNameField(SwFileNameFieldType*, SwFileNameFormat nFormat);

    SwFileNameFormat GetFormat() const { return m_nFormat; }
    void SetFormat(SwFileNameFormat n) { m_nFormat = n; }

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SAL_DLLPUBLIC_RTTI SwTemplNameFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;
public:
    SwTemplNameFieldType(SwDoc&);

    OUString                Expand(SwFileNameFormat) const;
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SW_DLLPUBLIC SwTemplNameField final : public SwField
{
    SwFileNameFormat m_nFormat;
public:
    SwTemplNameField(SwTemplNameFieldType*, SwFileNameFormat nFormat);

    SwFileNameFormat GetFormat() const { return m_nFormat; }
    void SetFormat(SwFileNameFormat n) { m_nFormat = n; }
    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Document statistics
class SAL_DLLPUBLIC_RTTI SwDocStatFieldType final : public SwFieldType
{
    SwDoc&          m_rDoc;
    SvxNumType      m_nNumberingType;

public:
    SwDocStatFieldType(SwDoc&);
    OUString                Expand(SwDocStatSubType nSubType, SvxNumType nFormat,
        sal_uInt16 nVirtPageCount) const;
    virtual std::unique_ptr<SwFieldType> Copy() const override;

    void             SetNumFormat( SvxNumType eFormat )  { m_nNumberingType = eFormat; }
    void             UpdateRangeFields(SwRootFrame const*const pLayout);
};

class SW_DLLPUBLIC SwDocStatField final : public SwField
{
    SwDocStatSubType m_nSubType;
    sal_uInt16 m_nVirtPageCount;
    SvxNumType m_nFormat;
public:
    SwDocStatField( SwDocStatFieldType*,
                    SwDocStatSubType nSubType, SvxNumType nFormat, sal_uInt16 nVirtPageCount = 0);

    SvxNumType GetFormat() const { return m_nFormat; }
    void SetFormat(SvxNumType n) { m_nFormat = n; }

    void ChangeExpansion( const SwFrame* pFrame, sal_uInt16 nVirtPageCount);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    SwDocStatSubType    GetSubType() const;
    void                SetSubType(SwDocStatSubType nSub);
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

class SAL_DLLPUBLIC_RTTI SwHiddenTextFieldType final : public SwFieldType
{
    bool m_bHidden;
public:
    SwHiddenTextFieldType(bool bSetHidden = true);

    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};

    void SetHiddenFlag( bool bSetHidden );
    bool GetHiddenFlag() const { return m_bHidden; }
};

class SW_DLLPUBLIC SwHiddenTextField final : public SwField
{
    OUString m_aTRUEText; ///< Text if condition true.
    OUString m_aFALSEText; ///< If condition false.
    OUString m_aContent; ///< Evaluated DB-text.

    OUString m_aCond; ///< Condition.
    SwFieldTypesEnum m_nSubType;

    bool m_bCanToggle : 1; ///< Can field be toggled alone?
    bool m_bIsHidden : 1; ///< Is it not visible?
    bool m_bValid : 1; ///< Is DB-field evaluated?

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwHiddenTextField( SwHiddenTextFieldType*,
                     bool   bConditional,
                     OUString aCond,
                     const OUString& rText,
                     bool   bHidden,
                     SwFieldTypesEnum nSubType = SwFieldTypesEnum::HiddenText);

    SwHiddenTextField( SwHiddenTextFieldType*,
                      OUString aCond,
                      OUString aTrue,
                      OUString aFalse,
                      SwFieldTypesEnum nSubType = SwFieldTypesEnum::HiddenText);

    virtual OUString    GetFieldName() const override;

    void                Evaluate(SwDoc& rDoc);

    void         SetValue(bool bHidden)  { m_bIsHidden = bHidden; }
    static OUString     GetColumnName(const OUString& rName);
    static OUString     GetDBName(std::u16string_view rName, SwDoc& rDoc);

    /// Condition
    virtual void        SetPar1(const OUString& rStr) override;
    virtual OUString    GetPar1() const override;

    /// True/False - String
    virtual void        SetPar2(const OUString& rStr) override;
    virtual OUString    GetPar2() const override;

    SwFieldTypesEnum    GetSubType() const;
    void SetSubType(SwFieldTypesEnum n) { m_nSubType = n; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    static void         ParseIfFieldDefinition(std::u16string_view aFieldDefinition,
                                               OUString& rCondition,
                                               OUString& rTrue,
                                               OUString& rFalse);
};

// Field that expands to an empty line (without height).
class SwHiddenParaFieldType final : public SwFieldType
{
public:
    SwHiddenParaFieldType();

    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};
};

class SwHiddenParaField final : public SwField
{
    OUString m_aCond;
    bool m_bIsHidden;
public:
    /// Direct input, delete old value.
    SwHiddenParaField(SwHiddenParaFieldType*, OUString aCond);

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

class SAL_DLLPUBLIC_RTTI SwMacroFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;

public:
    SwMacroFieldType(SwDoc&);

    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SW_DLLPUBLIC SwMacroField final : public SwField
{
    OUString m_aMacro;
    OUString m_aText;
    bool      m_bIsScriptURL;

    virtual OUString ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    /// Direct input, delete old value.
    SwMacroField( SwMacroFieldType*, OUString aLibAndName,
                  OUString aText);

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
                                   std::u16string_view rMacroName,
                                   const OUString& rLibraryName );

    static bool isScriptURL( const OUString& str );
};

class SAL_DLLPUBLIC_RTTI SwPostItFieldType final : public SwFieldType
{
private:
    SwDoc& mrDoc;
public:
    SwPostItFieldType(SwDoc& rDoc);

    virtual std::unique_ptr<SwFieldType> Copy() const override;

    SwDoc& GetDoc() const
    {
        return mrDoc;
    };
};

class SW_DLLPUBLIC SwPostItField final : public SwField
{
    OUString m_sText;
    OUString m_sAuthor;
    OUString m_sInitials; ///< Initials of the author.
    SwMarkName m_sName;     ///< Name of the comment.
    DateTime    m_aDateTime;
    bool     m_bResolved;
    std::optional<OutlinerParaObject> mpText;
    rtl::Reference<SwTextAPIObject> m_xTextObject;
    sal_uInt32 m_nPostItId;
    sal_uInt32 m_nParentId;
    sal_uInt32 m_nParaId;
    sal_uInt32 m_nParentPostItId;
    SwMarkName m_sParentName; /// Parent comment's name.
#if ENABLE_YRS
    OString m_CommentId;
public:
    OString GetYrsCommentId() const { return m_CommentId; }
    void SetYrsCommentId(OString const& rCommentId) { m_CommentId = rCommentId; }
#endif

public:
    static sal_uInt32 s_nLastPostItId;

    SwPostItField( SwPostItFieldType*,
                   OUString aAuthor,
                   OUString aText,
                   OUString aInitials,
                   SwMarkName aName,
                   const DateTime& rDate,
                   const bool bResolved = false,
                   const sal_uInt32 nPostItId = 0,
                   const sal_uInt32 nParentId = 0,
                   const sal_uInt32 nParaId = 0,
                   const sal_uInt32 nParentPostItId = 0,
                   const SwMarkName aParentName = SwMarkName());

    SwPostItField(const SwPostItField&) = delete;
    SwPostItField* operator=(const SwPostItField&) = delete;

    virtual ~SwPostItField() override;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    const DateTime&         GetDateTime() const             { return m_aDateTime; }
    Date       GetDate() const                 { return Date(m_aDateTime); }
    tools::Time GetTime() const                 { return tools::Time(m_aDateTime); }
    sal_uInt32 GetPostItId() const             { return m_nPostItId; }
    void SetPostItId(const sal_uInt32 nPostItId = 0);
    void SetParentPostItId(const sal_uInt32 nParentPostItId = 0);
    sal_uInt32 GetParentId() const             { return m_nParentId; }
    sal_uInt32 GetParentPostItId() const       { return m_nParentPostItId; }
    void SetParentId(const sal_uInt32 nParentId);
    sal_uInt32 GetParaId() const               { return m_nParaId; }
    void SetParaId(const sal_uInt32 nParaId);

    /// Author
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;

    /// Text
    virtual OUString   GetPar2() const override;
    virtual void            SetPar2(const OUString& rStr) override;
    const OUString&         GetText() const { return m_sText; }
    const OUString&         GetInitials() const { return m_sInitials;}
    void                    SetName(const SwMarkName& rStr);
    const SwMarkName& GetName() const { return m_sName;}
    const SwMarkName& GetParentName() const { return m_sParentName; }
    void                    SetParentName(const SwMarkName& rStr);

    const OutlinerParaObject* GetTextObject() const { return mpText ? &*mpText : nullptr;}
    void SetTextObject( std::optional<OutlinerParaObject> pText );

    void SetResolved(bool bNewState);
    void ToggleResolved();
    bool GetResolved() const;

    sal_Int32 GetNumberOfParagraphs() const;
    void ChangeStyleSheetName(std::u16string_view rOldName, const SfxStyleSheetBase* pStyleSheet);

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
    virtual OUString    GetDescription() const override;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SAL_DLLPUBLIC_RTTI SwDocInfoFieldType final : public SwValueFieldType
{
public:
    SwDocInfoFieldType(SwDoc* pDc);

    OUString                Expand(SwDocInfoSubType nSubType, sal_uInt32 nFormat, LanguageType nLang, const OUString& rName) const;
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

/// A field that expands to the value of some document information, e.g. Insert -> Field -> First
/// Author on the UI.
class SW_DLLPUBLIC SwDocInfoField final : public SwValueField
{
    SwDocInfoSubType  m_nSubType;
    OUString  m_aContent;
    OUString  m_aName;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwDocInfoField(SwDocInfoFieldType*, SwDocInfoSubType nSub, const OUString& rName, sal_uInt32 nFormat=0);
    SwDocInfoField(SwDocInfoFieldType*, SwDocInfoSubType nSub, const OUString& rName, const OUString& rValue, sal_uInt32 nFormat=0);

    void                    SetSubType(SwDocInfoSubType);
    SwDocInfoSubType        GetSubType() const;
    virtual void            SetLanguage(LanguageType nLng) override;
    virtual OUString        GetFieldName() const override;
    const OUString&         GetName() const { return m_aName; }
    void                    SetName( const OUString& rName ) { m_aName = rName; }
    void             SetExpansion(const OUString& rStr) { m_aContent = rStr; }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Extended User settings.
class SwExtUserFieldType final : public SwFieldType
{
public:
    SwExtUserFieldType();

    static OUString         Expand(SwExtUserSubType nSubType);
    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SAL_DLLPUBLIC_RTTI SwExtUserField final : public SwField
{
    OUString m_aContent;
    SwExtUserSubType  m_nType;
    SwAuthorFormat m_nFormat;

public:
    SwExtUserField(SwExtUserFieldType*, SwExtUserSubType nSub, SwAuthorFormat nFormat);

    SwAuthorFormat GetFormat() const { return m_nFormat; }
    void SetFormat(SwAuthorFormat n) { m_nFormat = n; }

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    SW_DLLPUBLIC SwExtUserSubType GetSubType() const;
    void SetSubType(SwExtUserSubType nSub);

    void         SetExpansion(const OUString& rStr) { m_aContent = rStr; }

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Relative page numbers - field.
class SwRefPageSetFieldType final : public SwFieldType
{
public:
    SwRefPageSetFieldType();

    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};

private:
    /// noop, there is nothing to update!
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
};

// Relative page numbering.
class SAL_DLLPUBLIC_RTTI SwRefPageSetField final : public SwField
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
class SwRefPageGetFieldType final : public SwFieldType
{
    SwDoc&          m_rDoc;
    sal_Int16       m_nNumberingType;

    void UpdateField(SwTextField const * pTextField,
            SetGetExpFields const & rSetList, SwRootFrame const* pLayout);

    /// overwritten to update all RefPageGet fields
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
public:
    SwRefPageGetFieldType( SwDoc& rDoc );
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    bool MakeSetList(SetGetExpFields& rTmpLst, SwRootFrame const* pLayout);
    SwDoc&  GetDoc() const                  { return m_rDoc; }
};

// Query relative page numbering.
class SwRefPageGetField final : public SwField
{
    OUString m_sText;
    OUString m_sTextRLHidden; ///< hidden redlines
    SvxNumType m_nFormat;

public:
    SwRefPageGetField( SwRefPageGetFieldType*, SvxNumType nFormat );

    SvxNumType GetFormat() const { return m_nFormat; }
    void SetFormat(SvxNumType n) { m_nFormat = n; }

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void SetText(const OUString& rText, SwRootFrame const* pLayout);

    void ChangeExpansion(const SwFrame& rFrame, const SwTextField* pField);
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Field to jump to and edit.
class SwJumpEditFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;
    sw::WriterMultiListener m_aDep;

public:
    SwJumpEditFieldType( SwDoc& rDoc );
    virtual std::unique_ptr<SwFieldType> Copy() const override;
    virtual void UpdateFields() override {};

    SwCharFormat* GetCharFormat();
};

class SAL_DLLPUBLIC_RTTI SwJumpEditField final : public SwField
{
    OUString m_sText;
    OUString m_sHelp;
    SwJumpEditFormat m_nFormat;
public:
    SwJumpEditField( SwJumpEditFieldType*, SwJumpEditFormat nFormat,
                     OUString sText, OUString sHelp );

    SwJumpEditFormat GetFormat() const { return m_nFormat; }
    void SetFormat(SwJumpEditFormat n) { m_nFormat = n; }

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

class SwScriptFieldType final : public SwFieldType
{
    SwDoc& m_rDoc;
public:
    SwScriptFieldType( SwDoc& rDoc );

    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

class SW_DLLPUBLIC SwScriptField final : public SwField
{
    OUString m_sType;  ///< Type of Code (Java/VBScript/...)
    OUString m_sCode;  /**< Code as text.
                          Code as JavaCode ? */

    bool    m_bCodeURL; ///< Code contains URL of a script.

public:
    SwScriptField( SwScriptFieldType*, OUString aType,
                   OUString aCode, bool bURL );

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
class SAL_DLLPUBLIC_RTTI SwCombinedCharFieldType final : public SwFieldType
{
public:
    SwCombinedCharFieldType();

    virtual std::unique_ptr<SwFieldType> Copy() const override;
};

// ScriptField
#define MAX_COMBINED_CHARACTERS     6

class SW_DLLPUBLIC SwCombinedCharField final : public SwField
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
