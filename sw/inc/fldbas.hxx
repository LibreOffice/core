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
#ifndef INCLUDED_SW_INC_FLDBAS_HXX
#define INCLUDED_SW_INC_FLDBAS_HXX

#include <i18nlangtag/lang.h>
#include "swdllapi.h"
#include "swtypes.hxx"
#include "calbck.hxx"
#include "nodeoffset.hxx"
#include "names.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/weakref.hxx>
#include <editeng/svxenum.hxx>
#include <unotools/weakref.hxx>
#include <rtl/ref.hxx>
#include <vector>
#include <climits>

class SwDoc;
class SwField;
class SwFormatField;
class SwDDETable;
class SwRootFrame;
class SvNumberFormatter;
class IDocumentRedlineAccess;
class SwGetRefField;
class SwXFieldMaster;
namespace com::sun::star::uno { class Any; }

typedef struct _xmlTextWriter* xmlTextWriterPtr;

enum class SwFieldIds : sal_uInt16 {
/// For old documents the Field-Which IDs must be preserved !!!
    Database,
    User,
    Filename,
    DatabaseName,
    Date,
    Time,
    PageNumber,
    Author,
    Chapter,
    DocStat,
    GetExp, // 10
    SetExp,
    GetRef,
    HiddenText,
    Postit,
    FixDate,
    FixTime,
    Reg,
    VarReg,
    SetRef,
    Input, // 20
    Macro,
    Dde,
    Table,
    HiddenPara,
    DocInfo,
    TemplateName,
    DbNextSet,
    DbNumSet,
    DbSetNumber,
    ExtUser, // 30
    RefPageSet,
    RefPageGet,
    Internet,
    JumpEdit,
    Script,
    DateTime,
    TableOfAuthorities,
    CombinedChars,
    Dropdown,
    ParagraphSignature, // 40
    LAST = ParagraphSignature,

    Unknown = USHRT_MAX, // used as default value in some method calls
};

/// List of FieldTypes at UI.
enum class SwFieldTypesEnum : sal_uInt16 {
    Begin,
    Date = Begin,    // 0
    Time,
    Filename,
    DatabaseName,
    Chapter,
    PageNumber,
    DocumentStatistics,
    Author,
    Set,
    Get,
    Formel,              // 10
    HiddenText,
    SetRef,
    GetRef,
    DDE,
    Macro,
    Input,
    HiddenParagraph,
    DocumentInfo,
    Database,
    User,                // 20
    Postit,
    TemplateName,
    Sequence,
    DatabaseNextSet,
    DatabaseNumberSet,
    DatabaseSetNumber,
    ConditionalText,
    NextPage,
    PreviousPage,
    ExtendedUser,             // 30
    FixedDate,
    FixedTime,
    SetInput,
    UserInput,
    SetRefPage,
    GetRefPage,
    Internet,
    JumpEdit,
    Script,
    Authority,              // 40
    CombinedChars,
    Dropdown,
    Custom,     // Unused - necessary for alignment with aSwFields in fldmgr.cxx
    ParagraphSignature,
    LAST,
    Unknown = USHRT_MAX // used by SwFieldMgr::GetCurTypeId
};
enum class SwFileNameFormat {
    // most of the constants are a regular enum
    Name,
    PathName,
    Path,
    NameNoExt,
    UIName,
    UIRange,
    End, // marker value, used for asserts
    // except for this, which is a flag
    Fixed = 0x8000
};
namespace o3tl { template<> struct typed_flags<SwFileNameFormat> : is_typed_flags<SwFileNameFormat, 0x800f> {}; }

typedef sal_uInt16 SwGetSetExpType;
namespace nsSwGetSetExpType
{
const SwGetSetExpType GSE_STRING  = 0x0001; ///< String
const SwGetSetExpType GSE_EXPR    = 0x0002; ///< Expression
const SwGetSetExpType GSE_SEQ     = 0x0008; ///< Sequence
const SwGetSetExpType GSE_FORMULA = 0x0010; ///< Formula
}

typedef sal_uInt16 SwExtendedSubType;
namespace nsSwExtendedSubType
{
const SwExtendedSubType SUB_CMD         = 0x0100;   ///< Show command.
const SwExtendedSubType SUB_INVISIBLE   = 0x0200;   ///< Invisible.
const SwExtendedSubType SUB_OWN_FMT     = 0x0400;   ///< SwDBField: Don't accept formatting from database.
}

enum SwInputFieldSubType {
    INP_TXT     = 0x01,
    INP_USR     = 0x02,
    INP_VAR     = 0x03
};

enum SwUserType {
    UF_STRING   = 0x01,
    UF_EXPR     = 0x02
};

enum SwDateTimeSubType {
    FIXEDFLD = 1,
    DATEFLD  = 2,
    TIMEFLD  = 4
};

/// General tools.
OUString  FormatNumber(sal_uInt32 nNum, SvxNumType nFormat, LanguageType nLang = LANGUAGE_NONE);
SwFieldTypesEnum SwFieldTypeFromString(std::u16string_view rString);

/** Instances of SwFields and those derived from it occur 0 to n times.
 For each class there is one instance of the associated type class.
 Base class of all field types is SwFieldType. */

class SW_DLLPUBLIC SwFieldType : public sw::BroadcastingModify
{
    unotools::WeakReference<SwXFieldMaster> m_wXFieldMaster;

    SwFieldIds m_nWhich;

    friend void FinitUI();     ///< In order to delete pointer!
    static  std::vector<OUString>* s_pFieldNames;

    static void GetFieldName_();  ///< Sets up FieldNames; fldmgr.cxx!

protected:
    /// Single argument ctors shall be explicit.
    explicit SwFieldType( SwFieldIds nWhichId );

public:

    unotools::WeakReference<SwXFieldMaster> const& GetXObject() const {
        return m_wXFieldMaster;
    }
    void SetXObject(rtl::Reference<SwXFieldMaster> const& xFieldMaster);

    static const OUString & GetTypeStr( SwFieldTypesEnum nTypeId );

    /// Only in derived classes.
    virtual UIName        GetName() const;
    virtual std::unique_ptr<SwFieldType> Copy() const = 0;
    virtual void QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual void PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich );

    SwFieldIds              Which() const { return m_nWhich; }

    void PrintHiddenPara();
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
    SwFormatField* FindFormatForField(const SwField*) const;
    SwFormatField* FindFormatForPostItId(sal_uInt32 nPostItId) const;
    void CollectPostIts(std::vector<SwFormatField*>& rvFormatFields, IDocumentRedlineAccess const& rIDRA, bool HideRedlines);
    bool HasHiddenInformationNotes() const;
    void GatherNodeIndex(std::vector<SwNodeOffset>& rvNodeIndex);
    void GatherRefFields(std::vector<SwGetRefField*>& rvRFields, const sal_uInt16 nTyp);
    void GatherFields(std::vector<SwFormatField*>& rvFormatFields, bool bCollectOnlyInDocNodes=true) const;
    void GatherDdeTables(std::vector<SwDDETable*>& rvTables) const;
    void UpdateDocPos(const SwTwips nDocPos);
    virtual void UpdateFields();
};

/** Base class of all fields.
 Type of field is queried via Which.
 Expanded content of field is queried via ExpandField(). */
class SW_DLLPUBLIC SwField
{
private:
    mutable OUString    m_Cache;                ///< Cached expansion (for clipboard).
    SwFieldType*        m_pType;
    LanguageType        m_nLang;                ///< Always change via SetLanguage!
    bool                m_bUseFieldValueCache;  /// control the usage of the cached field value
    bool                m_bIsAutomaticLanguage;
    /// Used for tooltip purposes when it's not-empty.
    OUString m_aTitle;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const = 0;
    virtual std::unique_ptr<SwField> Copy() const = 0;

protected:
    SwField( SwFieldType* pTyp,
             LanguageType nLang = LANGUAGE_SYSTEM,
             bool m_bUseFieldValueCache = true );

public:
    virtual             ~SwField();

    SwField(SwField const &) = default;
    SwField(SwField &&) = default;
    SwField & operator =(SwField const &) = default;
    SwField & operator =(SwField &&) = default;

    inline SwFieldType* GetTyp() const;

    /// Set new type (used for copying among documents).
    virtual SwFieldType* ChgTyp( SwFieldType* );

    /** expand the field.
        @param  bCached     return cached field value.
        @remark     most callers should use the cached field value.
                    this is because various fields need special handing
                    (ChangeExpansion()) to return correct values, and only
                    SwTextFormatter::NewFieldPortion() sets things up properly.
        @param  pLayout     the layout to use for expansion; there are a few
                            fields that expand differently via layout mode.
        @return     the generated text (suitable for display)
      */
    OUString            ExpandField(bool bCached, SwRootFrame const* pLayout) const;

    /// @return name or content.
    virtual OUString    GetFieldName() const;

    std::unique_ptr<SwField> CopyField() const;

    /// ResId
    SwFieldIds          Which() const
    {
        assert(m_pType);
        return m_pType->Which();
    }

    // TYP_ID
    SwFieldTypesEnum    GetTypeId() const;

    // for code that is still passing around untyped values
    sal_uInt16 GetUntypedSubType() const;
    void SetUntypedSubType(sal_uInt16);

    /// Language at field position.
    inline LanguageType GetLanguage() const;
    virtual void        SetLanguage(LanguageType nLng);

    /// Query parameters for dialog and for BASIC.
    virtual OUString GetPar1() const;
    virtual OUString GetPar2() const;

    virtual OUString    GetFormula() const;

    virtual void        SetPar1(const OUString& rStr);
    virtual void        SetPar2(const OUString& rStr);

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId );

    /// Does the field possess an action on its ClickHandler? (e.g. INetFields, ...).
    bool            HasClickHdl() const;
    bool            IsFixed() const;

    bool IsAutomaticLanguage() const {
        return m_bIsAutomaticLanguage;
    }
    void SetAutomaticLanguage(bool const bSet) {
        m_bIsAutomaticLanguage = bSet;
    }

    virtual OUString    GetDescription() const;
    /// Is this field clickable?
    bool IsClickable() const;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
    const OUString & GetTitle() const { return m_aTitle; }
    void SetTitle(const OUString& rTitle) { m_aTitle = rTitle; }

    /// Helpers for those places still passing untyped format ids around for SwField
    sal_uInt32 GetUntypedFormat() const;
    void SetUntypedFormat(sal_uInt32);
};

inline SwFieldType* SwField::GetTyp() const
{
    return m_pType;
}

inline LanguageType SwField::GetLanguage() const
{
    return m_nLang;
}

/// Fields containing values that have to be formatted via number formatter.
class SwValueFieldType : public SwFieldType
{
private:
    SwDoc*  m_pDoc;
    bool    m_bUseFormat; ///< Use number formatter.

protected:
    SwValueFieldType( SwDoc* pDocPtr, SwFieldIds nWhichId );
    SwValueFieldType( const SwValueFieldType& rTyp );

public:
    SwDoc*   GetDoc() const                      {
        return m_pDoc;
    }
    void     SetDoc(SwDoc* pNewDoc)              {
        m_pDoc = pNewDoc;
    }

    bool     UseFormat() const                   {
        return m_bUseFormat;
    }
    void     EnableFormat(bool bFormat = true)   {
        m_bUseFormat = bFormat;
    }

    OUString        ExpandValue(const double& rVal, sal_uInt32 nFormat, LanguageType nLng) const;
    OUString        DoubleToString(const double &rVal, LanguageType eLng) const;
    OUString        DoubleToString(const double &rVal, sal_uInt32 nFormat) const;
    /// Query input or formatted value for dialog.
    OUString        GetInputOrDateTime( const OUString& rInput, const double& rVal, sal_uInt32 nFormat ) const;
};

class SW_DLLPUBLIC SwValueField : public SwField
{
private:
    double m_fValue;
    sal_uInt32 m_nFormat;

protected:
    SwValueField( SwValueFieldType* pFieldType, sal_uInt32 nFormat, LanguageType nLang = LANGUAGE_SYSTEM, const double fVal = 0.0 );
    SwValueField( const SwValueField& rField );

public:
    virtual                 ~SwValueField() override;

    sal_uInt32 GetFormat() const { return m_nFormat; }
    void SetFormat(sal_uInt32 nFormat) { m_nFormat = nFormat; }
    virtual SwFieldType*    ChgTyp( SwFieldType* ) override;
    virtual void            SetLanguage(LanguageType nLng) override;

    SwDoc*           GetDoc() const          {
        return static_cast<const SwValueFieldType*>(GetTyp())->GetDoc();
    }

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    OUString ExpandValue(const double& rVal, sal_uInt32 nFormat, LanguageType nLng) const {
        return static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(rVal, nFormat, nLng);
    }

    static sal_uInt32       GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFormat);
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SW_DLLPUBLIC SwFormulaField : public SwValueField
{
private:
    OUString m_sFormula;

protected:
    SwFormulaField( SwValueFieldType* pFieldType, sal_uInt32 nFormat, const double fVal );
    SwFormulaField( const SwFormulaField& rField );

public:
    virtual OUString        GetFormula() const override;
    void                    SetFormula(const OUString& rStr);

    void                    SetExpandedFormula(const OUString& rStr);
    OUString                GetExpandedFormula() const;

    /// Query formula or formatted value for dialog.
    OUString                GetInputOrDateTime() const;
};

#endif // INCLUDED_SW_INC_FLDBAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
