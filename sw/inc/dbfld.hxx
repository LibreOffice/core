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
class SwTxtFld;
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

    virtual OUString GetName() const SAL_OVERRIDE;
    virtual SwFieldType*  Copy() const SAL_OVERRIDE;

    inline void     AddRef() { nRefCnt++; }
    void            ReleaseRef();

    OUString        GetColumnName() const {return sColumn;}
    const SwDBData& GetDBData() const {return aDBData;}

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Classes derived from SwFields. They overlay the expand-function.
// Content is formated according to the format (if available).
class SW_DLLPUBLIC SwDBField : public SwValueField
{
    OUString aContent;
    OUString sFieldCode; ///< contains Word's field code
    sal_uInt16  nSubType;
    bool    bIsInBodyTxt    : 1;
    bool    bValidValue     : 1;
    bool    bInitialized    : 1;

    virtual OUString    Expand() const SAL_OVERRIDE;
    virtual SwField*    Copy() const SAL_OVERRIDE;

public:
    SwDBField(SwDBFieldType*, sal_uLong nFmt = 0);
    virtual ~SwDBField();

    virtual SwFieldType*    ChgTyp( SwFieldType* ) SAL_OVERRIDE;

    /// Current text.
    inline  void        SetExpansion(const OUString& rStr);

    virtual sal_uInt16      GetSubType() const SAL_OVERRIDE;
    virtual void        SetSubType(sal_uInt16 nType) SAL_OVERRIDE;

    virtual OUString    GetFieldName() const SAL_OVERRIDE;

    /// For calculations in expressions.
    void                ChgValue( double d, bool bVal );

    /// Get the evaluation via DBManager string.
    void                Evaluate();

    /// Evaluation for header and footer.
    void                ChangeExpansion( const SwFrm*, const SwTxtFld* );
    void                InitContent();
    void                InitContent(const OUString& rExpansion);

    inline void         ChgBodyTxtFlag( bool bIsInBody );

    inline bool         IsInitialized() const   { return bInitialized; }
    inline void         ClearInitialized()      { bInitialized = false; }
    inline void         SetInitialized()        { bInitialized = true; }

    /// Get name.
    virtual OUString    GetPar1() const SAL_OVERRIDE;

    /// access to the command string
    OUString            GetFieldCode() const { return sFieldCode;}
    void                SetFieldCode(const OUString& rStr) { sFieldCode = rStr; }

    /// DBName
    inline const SwDBData&  GetDBData() const { return ((SwDBFieldType*)GetTyp())->GetDBData(); }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;

    static bool FormatValue( SvNumberFormatter* pDocFormatter, OUString &aString, sal_uInt32 nFmt,
                             double &aNumber, sal_Int32 nColumnType, SwDBField *pField = NULL );
};

inline  void SwDBField::SetExpansion(const OUString& rStr)
    { aContent = rStr; }

/// set from UpdateExpFlds (the Node-Position is known there)
inline void SwDBField::ChgBodyTxtFlag( bool bIsInBody )
    { bIsInBodyTxt = bIsInBody; }

// Base class for all other database fields.
class SW_DLLPUBLIC SwDBNameInfField : public SwField
{
    SwDBData    aDBData;
    sal_uInt16      nSubType;

protected:
    const SwDBData& GetDBData() const {return aDBData;}
    SwDBData&       GetDBData() {return aDBData;}

    SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, sal_uLong nFmt = 0);

public:
    /// DBName
    inline const SwDBData&  GetRealDBData() { return aDBData; }

    SwDBData                GetDBData(SwDoc* pDoc);
    void                    SetDBData(const SwDBData& rDBData);

    virtual OUString        GetFieldName() const SAL_OVERRIDE;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
    virtual sal_uInt16          GetSubType() const SAL_OVERRIDE;
    virtual void            SetSubType(sal_uInt16 nType) SAL_OVERRIDE;
};

// Database field next record.
class SW_DLLPUBLIC SwDBNextSetFieldType : public SwFieldType
{
public:
    SwDBNextSetFieldType();

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

// Next data record with condition.
class SW_DLLPUBLIC SwDBNextSetField : public SwDBNameInfField
{
    OUString  aCond;
    bool    bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const OUString& rCond, const OUString& rDummy, const SwDBData& rDBData);

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

    void                    Evaluate(SwDoc*);
    inline void             SetCondValid(bool bCond);
    inline bool             IsCondValid() const;

    // Condition
    virtual OUString    GetPar1() const SAL_OVERRIDE;
    virtual void        SetPar1(const OUString& rStr) SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
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

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
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

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

    inline bool             IsCondValid() const;
    inline void             SetCondValid(bool bCond);
    void                    Evaluate(SwDoc*);

    // Condition
    virtual OUString        GetPar1() const SAL_OVERRIDE;
    virtual void            SetPar1(const OUString& rStr) SAL_OVERRIDE;

    // Number of data record.
    virtual OUString   GetPar2()   const SAL_OVERRIDE;
    virtual void            SetPar2(const OUString& rStr) SAL_OVERRIDE;

    // Number of data record is in nFormat!!
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
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
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

// Database field.
class SW_DLLPUBLIC SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual OUString Expand() const SAL_OVERRIDE;
    virtual SwField* Copy() const SAL_OVERRIDE;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

// Number of data record.
class SW_DLLPUBLIC SwDBSetNumberFieldType : public SwFieldType
{
public:
    SwDBSetNumberFieldType();

    virtual SwFieldType*    Copy() const SAL_OVERRIDE;
};

// Database field.
class SW_DLLPUBLIC SwDBSetNumberField : public SwDBNameInfField
{
    long    nNumber;
public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual OUString Expand() const SAL_OVERRIDE;
    virtual         SwField* Copy() const SAL_OVERRIDE;
    void            Evaluate(SwDoc*);

    inline long     GetSetNumber() const;
    inline void     SetSetNumber(long nNum);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;
};

inline long SwDBSetNumberField::GetSetNumber() const
    { return nNumber; }

inline void SwDBSetNumberField::SetSetNumber(long nNum)
    { nNumber = nNum; }

#endif // INCLUDED_SW_INC_DBFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
