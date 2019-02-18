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
#ifndef INCLUDED_SW_INC_DBFLD_HXX
#define INCLUDED_SW_INC_DBFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"
#include "swdbdata.hxx"

class SwDoc;

// Database field.
class SW_DLLPUBLIC SwDBFieldType : public SwValueFieldType
{
    SwDBData    m_aDBData;
    OUString m_sName;          ///< only used in ::GetName() !
    OUString m_sColumn;
    long        m_nRefCnt;

public:

    SwDBFieldType(SwDoc* pDocPtr, const OUString& rColumnName, const SwDBData& rDBData);
    virtual ~SwDBFieldType() override;

    virtual OUString GetName() const override;
    virtual SwFieldType*  Copy() const override;

    void     AddRef() { m_nRefCnt++; }
    void            ReleaseRef();

    const OUString&     GetColumnName() const {return m_sColumn;}
    const SwDBData& GetDBData() const {return m_aDBData;}

    virtual void        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual void        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Classes derived from SwFields. They overlay the expand-function.
// Content is formatted according to the format (if available).
class SW_DLLPUBLIC SwDBField : public SwValueField
{
    OUString m_aContent;
    OUString m_sFieldCode; ///< contains Word's field code
    sal_uInt16  m_nSubType;
    bool    m_bIsInBodyText    : 1;
    bool    m_bValidValue     : 1;
    bool    m_bInitialized    : 1;

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

public:
    SwDBField(SwDBFieldType*, sal_uInt32 nFormat = 0);
    virtual ~SwDBField() override;

    virtual SwFieldType*    ChgTyp( SwFieldType* ) override;

    /// Current text.
    inline  void        SetExpansion(const OUString& rStr);

    virtual sal_uInt16      GetSubType() const override;
    virtual void        SetSubType(sal_uInt16 nType) override;

    virtual OUString    GetFieldName() const override;

    /// For calculations in expressions.
    void                ChgValue( double d, bool bVal );

    /// Get the evaluation via DBManager string.
    void                Evaluate();

    /// Evaluation for header and footer.
    void                InitContent();
    void                InitContent(const OUString& rExpansion);

    inline void         ChgBodyTextFlag( bool bIsInBody );

    bool         IsInitialized() const   { return m_bInitialized; }
    void         ClearInitialized()      { m_bInitialized = false; }
    void         SetInitialized()        { m_bInitialized = true; }

    /// Get name.
    virtual OUString    GetPar1() const override;

    /// access to the command string
    void                SetFieldCode(const OUString& rStr) { m_sFieldCode = rStr; }

    /// DBName
    const SwDBData&  GetDBData() const { return static_cast<SwDBFieldType*>(GetTyp())->GetDBData(); }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    static bool FormatValue( SvNumberFormatter const * pDocFormatter, OUString const &aString, sal_uInt32 nFormat,
                             double &aNumber, sal_Int32 nColumnType, SwDBField *pField = nullptr );
};

inline  void SwDBField::SetExpansion(const OUString& rStr)
    { m_aContent = rStr; }

/// set from UpdateExpFields (the Node-Position is known there)
inline void SwDBField::ChgBodyTextFlag( bool bIsInBody )
    { m_bIsInBodyText = bIsInBody; }

// Base class for all other database fields.
class SW_DLLPUBLIC SwDBNameInfField : public SwField
{
    SwDBData        m_aDBData;
    sal_uInt16      m_nSubType;

protected:
    const SwDBData& GetDBData() const {return m_aDBData;}
    SwDBData&       GetDBData() {return m_aDBData;}

    SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uInt32 nFormat = 0);

public:
    /// DBName
    const SwDBData&  GetRealDBData() const { return m_aDBData; }
    SwDBData&        GetRealDBData() { return m_aDBData; }

    SwDBData                GetDBData(SwDoc* pDoc);
    void                    SetDBData(const SwDBData& rDBData);

    virtual OUString        GetFieldName() const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
    virtual sal_uInt16      GetSubType() const override;
    virtual void            SetSubType(sal_uInt16 nType) override;
};

// Database field next record.
class SW_DLLPUBLIC SwDBNextSetFieldType : public SwFieldType
{
public:
    SwDBNextSetFieldType();

    virtual SwFieldType*    Copy() const override;
};

// Next data record with condition.
class SW_DLLPUBLIC SwDBNextSetField : public SwDBNameInfField
{
    OUString  m_aCond;
    bool    m_bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const OUString& rCond, const SwDBData& rDBData);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    void                    Evaluate(SwDoc const *);
    inline void             SetCondValid(bool bCond);
    inline bool             IsCondValid() const;

    // Condition
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline bool SwDBNextSetField::IsCondValid() const
    { return m_bCondValid; }

inline void SwDBNextSetField::SetCondValid(bool bCond)
    { m_bCondValid = bCond; }

// Database field next record.
class SwDBNumSetFieldType : public SwFieldType
{
public:
    SwDBNumSetFieldType();

    virtual SwFieldType*    Copy() const override;
};

// Data record with number xxx.
// Number is in nFormat (bit of a misuse!)
class SwDBNumSetField : public SwDBNameInfField
{
    OUString  m_aCond;
    OUString  m_aPar2;
    bool    m_bCondValid;

public:
    SwDBNumSetField(SwDBNumSetFieldType*, const OUString& rCond, const OUString& rDBNum, const SwDBData& rDBData);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;

    inline bool             IsCondValid() const;
    inline void             SetCondValid(bool bCond);
    void                    Evaluate(SwDoc const *);

    // Condition
    virtual OUString        GetPar1() const override;
    virtual void            SetPar1(const OUString& rStr) override;

    // Number of data record.
    virtual OUString   GetPar2()   const override;
    virtual void            SetPar2(const OUString& rStr) override;

    // Number of data record is in nFormat!!
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline bool SwDBNumSetField::IsCondValid() const
    { return m_bCondValid; }

inline void SwDBNumSetField::SetCondValid(bool bCond)
    { m_bCondValid = bCond; }

// Database name.
class SwDBNameFieldType : public SwFieldType
{
        SwDoc *m_pDoc;
public:
    SwDBNameFieldType(SwDoc*);

    OUString                Expand() const;
    virtual SwFieldType*    Copy() const override;
};

// Database field.
class SW_DLLPUBLIC SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Number of data record.
class SW_DLLPUBLIC SwDBSetNumberFieldType : public SwFieldType
{
public:
    SwDBSetNumberFieldType();

    virtual SwFieldType*    Copy() const override;
};

// Database field.
class SW_DLLPUBLIC SwDBSetNumberField : public SwDBNameInfField
{
    long    m_nNumber;
public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, sal_uInt32 nFormat = 0);

    virtual OUString    ExpandImpl(SwRootFrame const* pLayout) const override;
    virtual std::unique_ptr<SwField> Copy() const override;
    void            Evaluate(SwDoc const *);

    inline long     GetSetNumber() const;
    inline void     SetSetNumber(long nNum);
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline long SwDBSetNumberField::GetSetNumber() const
    { return m_nNumber; }

inline void SwDBSetNumberField::SetSetNumber(long nNum)
    { m_nNumber = nNum; }

#endif // INCLUDED_SW_INC_DBFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
