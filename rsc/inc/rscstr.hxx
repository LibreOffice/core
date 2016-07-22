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
#ifndef INCLUDED_RSC_INC_RSCSTR_HXX
#define INCLUDED_RSC_INC_RSCSTR_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscString : public RscTop
{
    RscTop * pRefClass;
    struct RscStringInst
    {
        char *  pStr;   // pointer to string
        bool    bDflt;  // is default
        RscId   aRefId; // reference name
    };
    sal_uInt32  nSize;
public:
                    RscString( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const override;

    void            SetRefClass( RscTop * pClass ) { pRefClass = pClass; }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
                    // sets the allowed range
    void            Destroy( const RSCINST & rInst ) override;
    sal_uInt32      Size() override { return nSize; }
    void            SetToDefault( const RSCINST & rInst ) override
                        {
                            reinterpret_cast<RscStringInst*>(rInst.pData)->bDflt = true;
                        }
    bool            IsDefault( const RSCINST & rInst) override
                        {
                            return reinterpret_cast<RscStringInst*>(rInst.pData)->bDflt;
                        }
                    // sets as default
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;
    ERRTYPE         SetString( const RSCINST &, const char * pStr ) override;
    ERRTYPE         GetString( const RSCINST &, char ** ppStr ) override;
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * ) override;
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId ) override;
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

#endif // INCLUDED_RSC_INC_RSCSTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
