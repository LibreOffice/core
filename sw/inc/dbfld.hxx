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
class SwTextField;
class SwFrm;

// Database field.
class SW_DLLPUBLIC SwDBFieldType : public SwValueFieldType
{
    SwDBData    aDBData;
    OUString sName;          ///< only used in ::GetName() !
    OUString sColumn;
    long        nRefCnt;

public:

    SwDBFieldType(SwDoc* pDocPtr, const OUString& rColumnName, const SwDBData& rDBData);
    virtual ~SwDBFieldType();

    virtual OUString GetName() const override;
    virtual SwFieldType*  Copy() const override;

    inline void     AddRef() { nRefCnt++; }
    void            ReleaseRef();

    OUString        GetColumnName() const {return sColumn;}
    const SwDBData& GetDBData() const {return aDBData;}

    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

// Classes derived from SwFields. They overlay the expand-function.
// Content is formatted according to the format (if available).
class SW_DLLPUBLIC SwDBField : public SwValueField
{
    OUString aContent;
    OUString sFieldCode; ///< contains Word's field code
    sal_uInt16  nSubType;
    bool    bIsInBodyText    : 1;
    bool    bValidValue     : 1;
    bool    bInitialized    : 1;

    virtual OUString    Expand() const override;
    virtual SwField*    Copy() const override;

public:
    SwDBField(SwDBFieldType*, sal_uLong nFormat = 0);
    virtual ~SwDBField();

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

    inline bool         IsInitialized() const   { return bInitialized; }
    inline void         ClearInitialized()      { bInitialized = false; }
    inline void         SetInitialized()        { bInitialized = true; }

    /// Get name.
    virtual OUString    GetPar1() const override;

    /// access to the command string
    void                SetFieldCode(const OUString& rStr) { sFieldCode = rStr; }

    /// DBName
    inline const SwDBData&  GetDBData() const { return static_cast<SwDBFieldType*>(GetTyp())->GetDBData(); }
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;

    static bool FormatValue( SvNumberFormatter* pDocFormatter, OUString &aString, sal_uInt32 nFormat,
                             double &aNumber, sal_Int32 nColumnType, SwDBField *pField = nullptr );
};

inline  void SwDBField::SetExpansion(const OUString& rStr)
    { aContent = rStr; }

/// set from UpdateExpFields (the Node-Position is known there)
inline void SwDBField::ChgBodyTextFlag( bool bIsInBody )
    { bIsInBodyText = bIsInBody; }

// Base class for all other database fields.
class SW_DLLPUBLIC SwDBNameInfField : public SwField
{
    SwDBData        aDBData;
    sal_uInt16      nSubType;

protected:
    const SwDBData& GetDBData() const {return aDBData;}
    SwDBData&       GetDBData() {return aDBData;}

    SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFormat = 0);

public:
    /// DBName
    inline const SwDBData&  GetRealDBData() const { return aDBData; }
    inline SwDBData&        GetRealDBData() { return aDBData; }

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
    OUString  aCond;
    bool    bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const OUString& rCond, const OUString& rDummy, const SwDBData& rDBData);

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

    void                    Evaluate(SwDoc*);
    inline void             SetCondValid(bool bCond);
    inline bool             IsCondValid() const;

    // Condition
    virtual OUString    GetPar1() const override;
    virtual void        SetPar1(const OUString& rStr) override;
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline bool SwDBNextSetField::IsCondValid() const
    { return bCondValid; }

inline void SwDBNextSetField::SetCondValid(bool bCond)
    { bCondValid = bCond; }

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
    OUString  aCond;
    OUString  aPar2;
    bool    bCondValid;

public:
    SwDBNumSetField(SwDBNumSetFieldType*, const OUString& rCond, const OUString& rDBNum, const SwDBData& rDBData);

    virtual OUString        Expand() const override;
    virtual SwField*        Copy() const override;

    inline bool             IsCondValid() const;
    inline void             SetCondValid(bool bCond);
    void                    Evaluate(SwDoc*);

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
    { return bCondValid; }

inline void SwDBNumSetField::SetCondValid(bool bCond)
    { bCondValid = bCond; }

// Database name.
class SwDBNameFieldType : public SwFieldType
{
        SwDoc *pDoc;
public:
    SwDBNameFieldType(SwDoc*);

    OUString                Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const override;
};

// Database field.
class SW_DLLPUBLIC SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData, sal_uLong nFormat = 0);

    virtual OUString Expand() const override;
    virtual SwField* Copy() const override;
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
    long    nNumber;
public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, sal_uLong nFormat = 0);

    virtual OUString Expand() const override;
    virtual         SwField* Copy() const override;
    void            Evaluate(SwDoc*);

    inline long     GetSetNumber() const;
    inline void     SetSetNumber(long nNum);
    virtual bool        QueryValue( css::uno::Any& rVal, sal_uInt16 nWhich ) const override;
    virtual bool        PutValue( const css::uno::Any& rVal, sal_uInt16 nWhich ) override;
};

inline long SwDBSetNumberField::GetSetNumber() const
    { return nNumber; }

inline void SwDBSetNumberField::SetSetNumber(long nNum)
    { nNumber = nNum; }

#endif // INCLUDED_SW_INC_DBFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
