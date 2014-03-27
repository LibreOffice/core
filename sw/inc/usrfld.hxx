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
#ifndef INCLUDED_SW_INC_USRFLD_HXX
#define INCLUDED_SW_INC_USRFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"

class SfxPoolItem;
class SwCalc;
class SwDoc;

class SW_DLLPUBLIC SwUserFieldType : public SwValueFieldType
{
    bool    bValidValue : 1;
    bool    bDeleted : 1;
    double  nValue;
    OUString  aName;
    OUString  aContent;
    sal_uInt16  nType;

public:
    SwUserFieldType( SwDoc* pDocPtr, const OUString& );

    virtual OUString        GetName() const SAL_OVERRIDE;
    virtual SwFieldType*    Copy() const SAL_OVERRIDE;

    OUString                Expand(sal_uInt32 nFmt, sal_uInt16 nSubType, sal_uInt16 nLng);

    OUString                GetContent( sal_uInt32 nFmt = 0 );
           void             SetContent( const OUString& rStr, sal_uInt32 nFmt = 0 );

    inline bool             IsValid() const;
    inline void             ChgValid( bool bNew );

           double           GetValue(SwCalc& rCalc);    // Recalculate member nValue.
    inline double           GetValue() const;
    inline void             SetValue(const double nVal);

    inline sal_uInt16           GetType() const;
    inline void             SetType(sal_uInt16);

    bool                    IsDeleted() const       { return bDeleted; }
    void                    SetDeleted( bool b )    { bDeleted = b; }

    virtual bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nMId ) const SAL_OVERRIDE;
    virtual bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nMId ) SAL_OVERRIDE;

protected:
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) SAL_OVERRIDE;
};

inline bool SwUserFieldType::IsValid() const
    { return bValidValue; }

inline void SwUserFieldType::ChgValid( bool bNew )
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

class SW_DLLPUBLIC SwUserField : public SwValueField
{
    sal_uInt16  nSubType;

    virtual OUString        Expand() const SAL_OVERRIDE;
    virtual SwField*        Copy() const SAL_OVERRIDE;

public:
    SwUserField(SwUserFieldType*, sal_uInt16 nSub = 0, sal_uInt32 nFmt = 0);

    virtual sal_uInt16          GetSubType() const SAL_OVERRIDE;
    virtual void            SetSubType(sal_uInt16 nSub) SAL_OVERRIDE;

    virtual double          GetValue() const SAL_OVERRIDE;
    virtual void            SetValue( const double& rVal ) SAL_OVERRIDE;

    virtual OUString        GetFieldName() const SAL_OVERRIDE;

    // Name cannot be changed.
    virtual OUString   GetPar1() const SAL_OVERRIDE;

    // Content.
    virtual OUString   GetPar2() const SAL_OVERRIDE;
    virtual void            SetPar2(const OUString& rStr) SAL_OVERRIDE;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhichId ) SAL_OVERRIDE;
};

#endif // INCLUDED_SW_INC_USRFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
