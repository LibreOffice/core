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
#ifndef INCLUDED_RSC_INC_RSCCONST_HXX
#define INCLUDED_RSC_INC_RSCCONST_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscConst : public RscTop
{
protected:
    struct VarEle
    {
        Atom    nId;    // constant name
        sal_Int32   lValue; // constant value
    };
    VarEle *        pVarArray;  // pointer to the field with constant
    sal_uInt32      nEntries;   // number of entries in field
public:
                    RscConst( Atom nId, sal_uInt32 nTypId );
                    virtual ~RscConst();
    virtual RSCCLASS_TYPE   GetClassType() const override;
                    // sets the allowed values
    void            SetConstant( Atom nVarName, sal_Int32 lValue );
    bool            GetConstValue( Atom nConstId, sal_Int32 * pVal ) const;
    bool            GetValueConst( sal_Int32 nValue, Atom  * pConstId ) const;
    sal_uInt32      GetConstPos( Atom nConstId );
};

class RscEnum : public RscConst
{
    struct RscEnumInst
    {
        sal_uInt32  nValue; // constant position in the array
        bool        bDflt;  // is default
    };
    sal_uInt32      nSize;
public:
                    RscEnum( Atom nId, sal_uInt32 nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
    sal_uInt32      Size() override { return nSize; }

    virtual void    SetToDefault( const RSCINST & rInst ) override
                    {
                        reinterpret_cast<RscEnumInst*>(rInst.pData)->bDflt = true;
                    }
    bool            IsDefault( const RSCINST & rInst ) override
                    {
                        return reinterpret_cast<RscEnumInst*>(rInst.pData)->bDflt;
                    };
                    // sets as default
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) override;
    ERRTYPE         SetNumber( const RSCINST & rInst, sal_Int32 nValue ) override;
    ERRTYPE         GetConst( const RSCINST & rInst, Atom * ) override;
    ERRTYPE         GetNumber( const RSCINST & rInst, sal_Int32 * nValue ) override;
    void            WriteSrc( const RSCINST &rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

class RscNameTable;

sal_uInt32 GetLangId( const OString& rLang);

class RscLangEnum : public RscEnum
{
    long mnLangId;
public:
    RscLangEnum();

    void Init( RscNameTable& rNames );

    Atom AddLanguage( const char* pLang, RscNameTable& rNames );
};

#endif // INCLUDED_RSC_INC_RSCCONST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
