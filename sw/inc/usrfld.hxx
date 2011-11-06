/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
