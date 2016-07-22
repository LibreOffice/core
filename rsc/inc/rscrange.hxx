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
#ifndef INCLUDED_RSC_INC_RSCRANGE_HXX
#define INCLUDED_RSC_INC_RSCRANGE_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscRange : public RscTop
{
protected:
    struct RscRangeInst
    {
        sal_uInt16  nValue; // nValue = output value - nMin
        bool        bDflt;  // is default
    };
    sal_Int32       nMin;   // range minimum value
    sal_Int32       nMax;   // range maximum value
    sal_uInt32      nSize;
public:
                    RscRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const override;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
                    // sets the allowed range
    void            SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // returns the class size in bytes
    sal_uInt32      Size() override { return nSize; }
                    // an assignment to a variable
    virtual void    SetToDefault( const RSCINST & rInst ) override
                        {
                            reinterpret_cast<RscRangeInst*>(rInst.pData)->bDflt = true;
                        }
    bool            IsDefault( const RSCINST & rInst) override
                        {
                            return reinterpret_cast<RscRangeInst*>(rInst.pData)->bDflt;
                        };
                    // sets as default
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) override;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) override;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

class RscLongRange : public RscTop
{
protected:
    struct RscLongRangeInst
    {
        sal_Int32    nValue; // nValue = output value - nMin
        bool    bDflt;  // is default
    };
    sal_Int32    nMin;   // range minimum value
    sal_Int32    nMax;   // range maximum value
    sal_uInt32  nSize;
public:
                    RscLongRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const override;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
                    // sets the allowed range
    void            SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // returns the class size in bytes
    sal_uInt32      Size() override { return nSize; }
                    // an assignment to a variable
    virtual void    SetToDefault( const RSCINST & rInst ) override
                        {
                            reinterpret_cast<RscLongRangeInst*>(rInst.pData)->bDflt = true;
                        }
    bool            IsDefault( const RSCINST & rInst) override
                        {
                            return reinterpret_cast<RscLongRangeInst*>(rInst.pData)->bDflt;
                        };
                    // sets as default
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) override;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) override;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;

};

class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( Atom nId, sal_uInt32 nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) override;
};

class RscIdRange : public RscTop
{
    sal_uInt32  nSize;
protected:
    sal_Int32    nMin;   // range minimum value
    sal_Int32    nMax;   // range maximum value
public:
                    RscIdRange( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const override;
                    // sets the allowed range
    void            SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum )
                        {
                            nMin = nMinimum;
                            nMax = nMaximum;
                        }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
    void            Destroy( const RSCINST & rInst ) override;
    sal_uInt32          Size() override { return nSize; }
    virtual void    SetToDefault( const RSCINST & rInst ) override
                        {
                            reinterpret_cast<RscId*>(rInst.pData)->aExp.cUnused = true;
                        }
    bool            IsDefault( const RSCINST & rInst) override
                        {
                            //cUnused is used as default identifier
                            return reinterpret_cast<RscId*>(rInst.pData)->aExp.cUnused;
                        }
                    // sets as default
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) override;
    ERRTYPE         GetNumber( const RSCINST &, sal_Int32 * ) override;
    ERRTYPE         SetRef( const RSCINST &, const RscId & rRscId ) override;
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * ) override;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
    bool            IsConsistent( const RSCINST & rInst ) override;
};

class RscBool : public RscRange
{
public:
                    RscBool( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const override;
    ERRTYPE         SetBool( const RSCINST & rInst, bool b ) override
                        {
                            return SetNumber( rInst, (sal_Int32)b );
                        };
    ERRTYPE         GetBool( const RSCINST & rInst, bool * pB) override
                        {
                            sal_Int32 l;
                            GetNumber( rInst, &l );
                            *pB = (0 != l);
                            return ERR_OK;
                        };
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
};

class RscBreakRange : public RscRange
{
    sal_Int32        nOutRange;
public:
                    RscBreakRange( Atom nId, sal_uInt32 nTypId );
    void            SetOutRange( sal_Int32 nNumber ) { nOutRange = nNumber; }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 ) override;
};

#endif // INCLUDED_RSC_INC_RSCRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
