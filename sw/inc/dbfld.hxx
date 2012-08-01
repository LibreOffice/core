/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_DBFLD_HXX
#define SW_DBFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"
#include "swdbdata.hxx"

class SwDoc;
class SwTxtFld;
class SwFrm;

/*--------------------------------------------------------------------
    Database field.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBFieldType : public SwValueFieldType
{
    SwDBData    aDBData;        //
    rtl::OUString sName;          ///< only used in ::GetName() !
    String      sColumn;
    long        nRefCnt;

public:

    SwDBFieldType(SwDoc* pDocPtr, const String& rColumnName, const SwDBData& rDBData);
    ~SwDBFieldType();

    virtual const rtl::OUString& GetName() const;
    virtual SwFieldType*  Copy() const;

    inline void     AddRef() { nRefCnt++; }
    void            ReleaseRef();

    const String&   GetColumnName() const {return sColumn;}
    const SwDBData& GetDBData() const {return aDBData;}

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Classes derived from SwFields. They overlay the expand-function.
    Content is formated according to the format (if available).
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBField : public SwValueField
{
    rtl::OUString aContent;
    rtl::OUString sFieldCode; ///< contains Word's field code
    sal_uInt16  nSubType;
    sal_Bool    bIsInBodyTxt    : 1;
    sal_Bool    bValidValue     : 1;
    sal_Bool    bInitialized    : 1;

    virtual String      Expand() const;
    virtual SwField*    Copy() const;

public:
    SwDBField(SwDBFieldType*, sal_uLong nFmt = 0);
    virtual ~SwDBField();

    virtual SwFieldType*    ChgTyp( SwFieldType* );

    /// Current text.
    inline  void        SetExpansion(const String& rStr);

    virtual sal_uInt16      GetSubType() const;
    virtual void        SetSubType(sal_uInt16 nType);

    virtual String      GetFieldName() const;

    /// For calculations in expressions.
    void                ChgValue( double d, sal_Bool bVal );

    /// Get the evaluation via DBMgr string.
    void                Evaluate();

    /// Evaluation for header and footer.
    void                ChangeExpansion( const SwFrm*, const SwTxtFld* );
    void                InitContent();
    void                InitContent(const String& rExpansion);

    inline void         ChgBodyTxtFlag( sal_Bool bIsInBody );

    inline sal_Bool         IsInitialized() const   { return bInitialized; }
    inline void         ClearInitialized()      { bInitialized = sal_False; }
    inline void         SetInitialized()        { bInitialized = sal_True; }

    /// Get name.
    virtual const rtl::OUString& GetPar1() const;

    /// access to the command string
    const rtl::OUString& GetFieldCode() const { return sFieldCode;}
    void                SetFieldCode(const rtl::OUString& rStr) { sFieldCode = rStr; }

    /// DBName
    inline const SwDBData&  GetDBData() const { return ((SwDBFieldType*)GetTyp())->GetDBData(); }
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline  void SwDBField::SetExpansion(const String& rStr)
    { aContent = rStr; }

/// set from UpdateExpFlds (the Node-Position is known there)
inline void SwDBField::ChgBodyTxtFlag( sal_Bool bIsInBody )
    { bIsInBodyTxt = bIsInBody; }

/*--------------------------------------------------------------------
    Base class for all other database fields.
 --------------------------------------------------------------------*/

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

    virtual String          GetFieldName() const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
    virtual sal_uInt16          GetSubType() const;
    virtual void            SetSubType(sal_uInt16 nType);
};

/*--------------------------------------------------------------------
    Database field next record.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNextSetFieldType : public SwFieldType
{
public:
    SwDBNextSetFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Next data record with condition.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNextSetField : public SwDBNameInfField
{
    rtl::OUString  aCond;
    sal_Bool    bCondValid;

public:
    SwDBNextSetField( SwDBNextSetFieldType*,
                      const String& rCond, const String& rDummy, const SwDBData& rDBData);

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    void                    Evaluate(SwDoc*);
    inline void             SetCondValid(sal_Bool bCond);
    inline sal_Bool             IsCondValid() const;

    // Condition
    virtual const rtl::OUString&   GetPar1() const;
    virtual void        SetPar1(const rtl::OUString& rStr);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_Bool SwDBNextSetField::IsCondValid() const
    { return bCondValid; }

inline void SwDBNextSetField::SetCondValid(sal_Bool bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Database field next record.
 --------------------------------------------------------------------*/

class SwDBNumSetFieldType : public SwFieldType
{
public:
    SwDBNumSetFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Data record with number xxx.
    Number is in nFormat (bit of a misuse!)
 --------------------------------------------------------------------*/

class SwDBNumSetField : public SwDBNameInfField
{
    rtl::OUString  aCond;
    String  aPar2;
    sal_Bool    bCondValid;

public:
    SwDBNumSetField(SwDBNumSetFieldType*, const String& rCond, const String& rDBNum, const SwDBData& rDBData);

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

    inline sal_Bool             IsCondValid() const;
    inline void             SetCondValid(sal_Bool bCond);
    void                    Evaluate(SwDoc*);

    // Condition
    virtual const rtl::OUString&   GetPar1() const;
    virtual void            SetPar1(const rtl::OUString& rStr);

    // Number of data record.
    virtual rtl::OUString   GetPar2()   const;
    virtual void            SetPar2(const rtl::OUString& rStr);

    // Number of data record is in nFormat!!
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline sal_Bool SwDBNumSetField::IsCondValid() const
    { return bCondValid; }

inline void SwDBNumSetField::SetCondValid(sal_Bool bCond)
    { bCondValid = bCond; }

/*--------------------------------------------------------------------
    Database name.
 --------------------------------------------------------------------*/

class SwDBNameFieldType : public SwFieldType
{
        SwDoc *pDoc;
public:
    SwDBNameFieldType(SwDoc*);

    String                  Expand(sal_uLong) const;
    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Database field.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBNameField : public SwDBNameInfField
{
public:
    SwDBNameField(SwDBNameFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual String   Expand() const;
    virtual SwField* Copy() const;
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

/*--------------------------------------------------------------------
    Number of data record.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBSetNumberFieldType : public SwFieldType
{
public:
    SwDBSetNumberFieldType();

    virtual SwFieldType*    Copy() const;
};

/*--------------------------------------------------------------------
    Database field.
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDBSetNumberField : public SwDBNameInfField
{
    long    nNumber;

public:
    SwDBSetNumberField(SwDBSetNumberFieldType*, const SwDBData& rDBData, sal_uLong nFmt = 0);

    virtual String  Expand() const;
    virtual         SwField* Copy() const;
    void            Evaluate(SwDoc*);

    inline long     GetSetNumber() const;
    inline void     SetSetNumber(long nNum);
    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );
};

inline long SwDBSetNumberField::GetSetNumber() const
    { return nNumber; }

inline void SwDBSetNumberField::SetSetNumber(long nNum)
    { nNumber = nNum; }

#endif // SW_DBFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
