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
#ifndef SW_USRFLD_HXX
#define SW_USRFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"

class SfxPoolItem;
class SwCalc;
class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwUserFieldType : public SwValueFieldType
{
    sal_Bool    bValidValue : 1;
    sal_Bool    bDeleted : 1;
    double  nValue;
    String  aName;
    String  aContent;
    sal_uInt16  nType;

public:
    SwUserFieldType( SwDoc* pDocPtr, const String& );

    virtual const String&   GetName() const;
    virtual SwFieldType*    Copy() const;

    String                  Expand(sal_uInt32 nFmt, sal_uInt16 nSubType, sal_uInt16 nLng);

    String                  GetContent( sal_uInt32 nFmt = 0 );
           void             SetContent( const String& rStr, sal_uInt32 nFmt = 0 );

    inline sal_Bool             IsValid() const;
    inline void             ChgValid( sal_Bool bNew );

           double           GetValue(SwCalc& rCalc);    // Member nValue neu berrechnen
    inline double           GetValue() const;
    inline void             SetValue(const double nVal);

    inline sal_uInt16           GetType() const;
    inline void             SetType(sal_uInt16);

    sal_Bool                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( sal_Bool b )    { bDeleted = b; }

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nMId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nMId );

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );
};

inline sal_Bool SwUserFieldType::IsValid() const
    { return bValidValue; }

inline void SwUserFieldType::ChgValid( sal_Bool bNew )
    { bValidValue = bNew; }

inline double SwUserFieldType::GetValue() const
    { return nValue; }

inline void SwUserFieldType::SetValue(const double nVal)
    { nValue = nVal; }

inline sal_uInt16 SwUserFieldType::GetType() const
    { return nType; }

inline void SwUserFieldType::SetType(sal_uInt16 nSub)
{
    nType = nSub;
    EnableFormat(!(nSub & nsSwGetSetExpType::GSE_STRING));
}

/*--------------------------------------------------------------------
    Beschreibung: Benutzerfelder
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwUserField : public SwValueField
{
    sal_uInt16  nSubType;

    virtual String          Expand() const;
    virtual SwField*        Copy() const;

public:
    SwUserField(SwUserFieldType*, sal_uInt16 nSub = 0, sal_uInt32 nFmt = 0);

    virtual sal_uInt16          GetSubType() const;
    virtual void            SetSubType(sal_uInt16 nSub);

    virtual double          GetValue() const;
    virtual void            SetValue( const double& rVal );

    virtual String          GetFieldName() const;

    // Name kann nicht geaendert werden
    virtual const String&   GetPar1() const;

    // Inhalt
    virtual String          GetPar2() const;
    virtual void            SetPar2(const String& rStr);
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId );
};

#endif // SW_USRFLD_HXX
