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
#ifndef SW_FLDBAS_HXX
#define SW_FLDBAS_HXX

#include <i18nlangtag/lang.h>
#include <tools/string.hxx>
#include "swdllapi.h"
#include <calbck.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weakref.hxx>
#include <vector>

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
    TYP_SCRIPTFLD,
    TYP_AUTHORITY,              // 40
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
    const SwExtendedSubType SUB_OWN_FMT     = 0x0400;   ///< SwDBField: Don't accept formating from database.
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
String  GetResult(double nVal, sal_uInt32 nNumFmt, sal_uInt16 nLang = LANGUAGE_SYSTEM);
void    SetErrorStr(const String& rStr);
String  FormatNumber(sal_uInt32 nNum, sal_uInt32 nFormat);

/** Instances of SwFields and those derived from it occur 0 to n times.
 For each class there is one instance of the associated type class.
 Base class of all field types is SwFieldType. */

class SW_DLLPUBLIC SwFieldType : public SwModify
{
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::beans::XPropertySet> m_wXFieldMaster;

    sal_uInt16 nWhich;

    friend void _FinitUI();     ///< In order to delete pointer!
    static  std::vector<String>* pFldNames;

    static void _GetFldName();  ///< Sets up FldNames; fldmgr.cxx!

protected:
    /// Single argument ctors shall be explicit.
    explicit SwFieldType( sal_uInt16 nWhichId );

public:

    SW_DLLPRIVATE ::com::sun::star::uno::WeakReference<
        ::com::sun::star::beans::XPropertySet> const& GetXObject() const
            { return m_wXFieldMaster; }
    SW_DLLPRIVATE void SetXObject(::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> const& xFieldMaster)
            { m_wXFieldMaster = xFieldMaster; }

    static  const String&   GetTypeStr( sal_uInt16 nTypeId );

    /// Only in derived classes.
    virtual const OUString&   GetName() const;
    virtual SwFieldType*    Copy()    const = 0;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

            sal_uInt16          Which() const { return nWhich; }

    inline  void            UpdateFlds() const;
};

inline void SwFieldType::UpdateFlds() const
{
    ((SwFieldType*)this)->ModifyNotification( 0, 0 );
}

/** Base class of all fields.
 Type of field is queried via Which.
 Expanded content of field is queried via Expand(). */
class SW_DLLPUBLIC SwField
{
private:
    mutable String      m_Cache;     ///< Cached expansion (for clipboard).
    sal_uInt16              nLang;   ///< Always change via SetLanguage!
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

    inline SwFieldType* GetTyp() const;

    /// Set new type (used for copying among documents).
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

    /// @return name or content.
    virtual String      GetFieldName() const;

    SwField *           CopyField() const;

    /// ResId
    sal_uInt16              Which() const
#ifdef DBG_UTIL
        ;       // implemented in fldbas.cxx
#else
        { return pType->Which(); }
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
    virtual const OUString& GetPar1() const;
    virtual OUString GetPar2()   const;

    virtual String      GetFormula() const;

    virtual void        ChangeFormat(sal_uInt32 n);
    virtual void        SetPar1(const OUString& rStr);
    virtual void        SetPar2(const OUString& rStr);

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );

    /// Does the field possess an action on its ClickHandler? (e.g. INetFields, ...).
    sal_Bool            HasClickHdl() const;
    sal_Bool            IsFixed() const;

    sal_Bool                IsAutomaticLanguage() const { return bIsAutomaticLanguage;}
    void                SetAutomaticLanguage(sal_Bool bSet){bIsAutomaticLanguage = bSet;}

    virtual String      GetDescription() const;
    /// Is this field clickable?
    bool IsClickable() const;
};

inline SwFieldType* SwField::GetTyp() const
    { return pType;     }

inline sal_uInt32 SwField::GetFormat() const
    { return nFormat;   }

inline sal_uInt16 SwField::GetLanguage() const
    { return nLang;     }


 /// Fields containing values that have to be formated via number formatter.
class SwValueFieldType : public SwFieldType
{
    SwDoc*  pDoc;
    sal_Bool    bUseFormat; ///< Use number formatter.

protected:
    SwValueFieldType( SwDoc* pDocPtr, sal_uInt16 nWhichId );
    SwValueFieldType( const SwValueFieldType& rTyp );

public:
    inline SwDoc*   GetDoc() const                      { return pDoc; }
    inline void     SetDoc(SwDoc* pNewDoc)              { pDoc = pNewDoc; }

    inline sal_Bool     UseFormat() const                   { return bUseFormat; }
    inline void     EnableFormat(sal_Bool bFormat = sal_True)   { bUseFormat = bFormat; }

    String          ExpandValue(const double& rVal, sal_uInt32 nFmt, sal_uInt16 nLng=0) const;
    String          DoubleToString(const double &rVal, LanguageType eLng) const;
    String          DoubleToString(const double &rVal, sal_uInt32 nFmt) const;
};

class SW_DLLPUBLIC SwValueField : public SwField
{
    double fValue;

protected:
    SwValueField( SwValueFieldType* pFldType, sal_uInt32 nFmt = 0, sal_uInt16 nLang = LANGUAGE_SYSTEM, const double fVal = 0.0 );
    SwValueField( const SwValueField& rFld );

public:
    virtual                 ~SwValueField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );
    virtual void            SetLanguage(sal_uInt16 nLng);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
