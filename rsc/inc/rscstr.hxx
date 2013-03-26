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
#ifndef _RSCSTR_HXX
#define _RSCSTR_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscString : public RscTop
{
    RscTop * pRefClass;
    struct RscStringInst {
        char *  pStr;   // Zeiger auf String
        sal_Bool    bDflt;  // Ist Default
        RscId   aRefId; // ReferenzName
    };
    sal_uInt32  nSize;
public:
                    RscString( Atom nId, sal_uInt32 nTypId );
    virtual RSCCLASS_TYPE   GetClassType() const;

    void            SetRefClass( RscTop * pClass )
    {
                        pRefClass = pClass;
                    };
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDfltInst, sal_Bool );
                    // Der zulaessige Bereich wird gesetzt
    void            Destroy( const RSCINST & rInst );
    sal_uInt32          Size(){ return nSize; }
    void            SetToDefault( const RSCINST & rInst )
                    {
                        ((RscStringInst*)rInst.pData)->bDflt = sal_True;
                    }
    sal_Bool            IsDefault( const RSCINST & rInst)
                    {
                        return( ((RscStringInst*)rInst.pData)->bDflt );
                    };
                    // Als Default setzen
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    ERRTYPE         SetString( const RSCINST &, const char * pStr );
    ERRTYPE         GetString( const RSCINST &, char ** ppStr );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );
    void            WriteSrc( const RSCINST &, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
};

#endif // _RSCSTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
