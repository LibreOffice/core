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
public:
                    RscLongRange( Atom nId, RESOURCE_TYPE nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, bool bOwnClass = false ) override;
                    // sets the allowed range
    void            SetRange( sal_Int32 nMinimum, sal_Int32 nMaximum );
                    // returns the class size in bytes
    sal_uInt32      Size() const override { return ALIGNED_SIZE(sizeof(RscLongRangeInst)); }
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
                             RscTypCont * pTC, sal_uInt32 ) override;

};

class RscLongEnumRange : public RscLongRange
{
public:
                    RscLongEnumRange( Atom nId, RESOURCE_TYPE nTypId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) override;
};

#endif // INCLUDED_RSC_INC_RSCRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
