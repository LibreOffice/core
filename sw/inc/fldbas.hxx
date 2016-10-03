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
#include <calbck.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weakref.hxx>
#include <vector>
#include <toxe.hxx>
class SwDoc;
class SvNumberFormatter;

enum RES_FIELDS
{
 /// For old documents the Field-Which IDs must be preserved !!!
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
    RES_AUTHORITY,  ///< Table of authorities
    RES_COMBINED_CHARS,
    RES_DROPDOWN,
    RES_FIELDS_END
};

 /// List of FieldTypes at UI.
enum SwFieldTypesEnum
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
    TYP_SCRIPTFLD,
    TYP_AUTHORITY,              // 40
    TYP_COMBINED_CHARS,
    TYP_DROPDOWN,
    TYP_END
};
enum SwAttrFieldTYpe
{
    ATTR_NONE,
    ATTR_DATEFLD,
    ATTR_TIMEFLD,
    ATTR_PAGENUMBERFLD,
    ATTR_PAGECOOUNTFLD,
    ATTR_BOOKMARKFLD,
    ATTR_SETREFATTRFLD
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
    VVF_CMD         = 0x0010,   ///< Show command.
    VVF_INVISIBLE   = 0x0040,   ///< Invisible.
    VVF_XXP         = 0x0400,   ///< 1234%
    VVF_XX_XXP      = 0x0800,   ///< 1.234,56%
    VVF_CLEAR       = 0x000f,

// From here new formats:
    VVF_SYS         = 0x2000,   ///< Format for numbers from system.
    VVF_X           = 0x2100,   ///< 1234
    VVF_X_X         = 0x2200,   ///< 1234.5
    VVF_X_XX        = 0x2300,   ///< 1245.56
    VVF_XX_X        = 0x2400,   ///< 1.234.5
    VVF_XX_XX       = 0x2500,   ///< 1.234.56
    VVF_XX_XXX      = 0x2600,   ///< 1.234.567
    VVF_SYS_CUR     = 0x2700,   ///< Format for currency from system.
    VVF_CUR_X       = 0x2800,   ///< EUR 1234
    VVF_CUR_XX_XX   = 0x2900,   ///< EUR 1234.56 EUR 1234.00
    VVF_CUR_XX_X0   = 0x2a00,   ///< EUR 1234.56 EUR 1234.--
    VVF_X_CUR       = 0x2b00,   ///< 1234 EUR
    VVF_XX_XX_CUR   = 0x2c00,   ///< 1234.56 EUR 1234.00 EUR
    VVF_XX_X0_CUR   = 0x2d00,   ///< 1234.56 EUR 1234.-- EUR
/// Compatibility:
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
    const SwGetSetExpType GSE_STRING  = 0x0001; ///< String
    const SwGetSetExpType GSE_EXPR    = 0x0002; ///< Expression
    const SwGetSetExpType GSE_INP     = 0x0004; ///< InputField
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

/// General tools.
OUString  FormatNumber(sal_uInt32 nNum, sal_uInt32 nFormat);

/** Instances of SwFields and those derived from it occur 0 to n times.
 For each class there is one instance of the associated type class.
 Base class of all field types is SwFieldType. */

class SW_DLLPUBLIC SwFieldType : public SwModify
{
    css::uno::WeakReference<css::beans::XPropertySet> m_wXFieldMaster;

    sal_uInt16 m_nWhich;

    friend void FinitUI();     ///< In order to delete pointer!
    static  std::vector<OUString>* s_pFieldNames;

    static void GetFieldName_();  ///< Sets up FieldNames; fldmgr.cxx!

protected:
    /// Single argument ctors shall be explicit.
    explicit SwFieldType( sal_uInt16 nWhichId );

public:

    SAL_DLLPRIVATE css::uno::WeakReference<css::beans::XPropertySet> const& GetXObject() const
            { return m_wXFieldMaster; }
    SAL_DLLPRIVATE void SetXObject(css::uno::Reference<css::beans::XPropertySet> const& xFieldMaster)
            { m_wXFieldMaster = xFieldMaster; }

    static OUString    GetTypeStr( sal_uInt16 nTypeId );

    /// Only in derived classes.
    virtual OUString        GetName() const;
    virtual SwFieldType*    Copy()    const = 0;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich );

            sal_uInt16          Which() const { return m_nWhich; }

    inline  void            UpdateFields() const;
};

inline void SwFieldType::UpdateFields() const
{
    const_cast<SwFieldType*>(this)->ModifyNotification( nullptr, nullptr );
}

/** Base class of all fields.
 Type of field is queried via Which.
 Expanded content of field is queried via Expand(). */
class SW_DLLPUBLIC SwField
{
private:
    mutable OUString    m_Cache;     ///< Cached expansion (for clipboard).
    bool m_bUseFieldValueCache; /// control the usage of the cached field value

    sal_uInt16          m_nLang;   ///< Always change via SetLanguage!
    bool                m_bIsAutomaticLanguage;
    sal_uInt32          m_nFormat;

    SwFieldType*        m_pType;

    virtual OUString    Expand() const = 0;
    virtual SwField*    Copy() const = 0;

protected:
    void                SetFormat(sal_uInt32 const nSet) { m_nFormat = nSet; }

    SwField( SwFieldType* pTyp,
             sal_uInt32 nFormat = 0,
             sal_uInt16 nLang = LANGUAGE_SYSTEM,
             bool m_bUseFieldValueCache = true );

public:
    virtual             ~SwField();

    inline SwFieldType* GetTyp() const;

    /// Set new type (used for copying among documents).
    virtual SwFieldType* ChgTyp( SwFieldType* );

    /** expand the field.
        @param  bCached     return cached field value.
        @remark     most callers should use the cached field value.
                    this is because various fields need special handing
                    (ChangeExpansion()) to return correct values, and only
                    SwTextFormatter::NewFieldPortion() sets things up properly.
        @return     the generated text (suitable for display)
      */
    OUString            ExpandField(bool const bCached) const;

    /// @return name or content.
    virtual OUString    GetFieldName() const;

    SwField *           CopyField() const;

    /// ResId
    sal_uInt16              Which() const
#ifdef DBG_UTIL
        ;       // implemented in fldbas.cxx
#else
        { return m_pType->Which(); }
#endif

    // TYP_ID
            sal_uInt16      GetTypeId() const;
    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16);

    /// Language at field position.
    inline sal_uInt16       GetLanguage() const;
    virtual void        SetLanguage(sal_uInt16 nLng);

    /// Query parameters for dialog and for BASIC.
    inline sal_uInt32   GetFormat() const;
    virtual OUString GetPar1() const;
    virtual OUString GetPar2() const;

    virtual OUString    GetFormula() const;

    void        ChangeFormat(sal_uInt32 n);
    virtual void        SetPar1(const OUString& rStr);
    virtual void        SetPar2(const OUString& rStr);

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhichId );

    /// Does the field possess an action on its ClickHandler? (e.g. INetFields, ...).
    bool            HasClickHdl() const;
    bool            IsFixed() const;

    bool IsAutomaticLanguage() const { return m_bIsAutomaticLanguage;}
    void SetAutomaticLanguage(bool const bSet) {m_bIsAutomaticLanguage = bSet;}

    virtual OUString    GetDescription() const;
    /// Is this field clickable?
    bool IsClickable() const;
    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

inline SwFieldType* SwField::GetTyp() const
    { return m_pType; }

inline sal_uInt32 SwField::GetFormat() const
    { return m_nFormat; }

inline sal_uInt16 SwField::GetLanguage() const
    { return m_nLang; }

 /// Fields containing values that have to be formatted via number formatter.
class SwValueFieldType : public SwFieldType
{
private:
    SwDoc*  m_pDoc;
    bool    m_bUseFormat; ///< Use number formatter.

protected:
    SwValueFieldType( SwDoc* pDocPtr, sal_uInt16 nWhichId );
    SwValueFieldType( const SwValueFieldType& rTyp );

public:
    inline SwDoc*   GetDoc() const                      { return m_pDoc; }
    inline void     SetDoc(SwDoc* pNewDoc)              { m_pDoc = pNewDoc; }

    inline bool     UseFormat() const                   { return m_bUseFormat; }
    inline void     EnableFormat(bool bFormat = true)   { m_bUseFormat = bFormat; }

    OUString        ExpandValue(const double& rVal, sal_uInt32 nFormat, sal_uInt16 nLng) const;
    OUString        DoubleToString(const double &rVal, LanguageType eLng) const;
    OUString        DoubleToString(const double &rVal, sal_uInt32 nFormat) const;
};

class SW_DLLPUBLIC SwValueField : public SwField
{
private:
    double m_fValue;

protected:
    SwValueField( SwValueFieldType* pFieldType, sal_uInt32 nFormat, sal_uInt16 nLang = LANGUAGE_SYSTEM, const double fVal = 0.0 );
    SwValueField( const SwValueField& rField );

public:
    virtual                 ~SwValueField() override;

    virtual SwFieldType*    ChgTyp( SwFieldType* ) override;
    virtual void            SetLanguage(sal_uInt16 nLng) override;

    inline SwDoc*           GetDoc() const          { return static_cast<const SwValueFieldType*>(GetTyp())->GetDoc(); }

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    inline OUString ExpandValue(const double& rVal, sal_uInt32 nFormat, sal_uInt16 nLng) const
        { return static_cast<SwValueFieldType*>(GetTyp())->ExpandValue(rVal, nFormat, nLng); }

    static sal_uInt32       GetSystemFormat(SvNumberFormatter* pFormatter, sal_uInt32 nFormat);
};

class SW_DLLPUBLIC SwFormulaField : public SwValueField
{
private:
    OUString m_sFormula;

protected:
    SwFormulaField( SwValueFieldType* pFieldType, sal_uInt32 nFormat, const double fVal = 0.0 );
    SwFormulaField( const SwFormulaField& rField );

public:
    virtual OUString        GetFormula() const override;
    void                    SetFormula(const OUString& rStr);

    void                    SetExpandedFormula(const OUString& rStr);
    OUString                GetExpandedFormula() const;
};

#endif // INCLUDED_SW_INC_FLDBAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
