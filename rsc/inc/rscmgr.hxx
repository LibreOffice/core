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
#ifndef _RSCMGR_HXX
#define _RSCMGR_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>
#include <rscclass.hxx>

/******************* R s c M g r *****************************************/
class RscMgr : public RscClass
{
    struct RscMgrInst
    {
        RscId   aRefId; // nRefId = Referenz Identifier
        bool    bDflt;  // Ist Default
        void Create(){ aRefId.Create(); bDflt = true; }
        void Destroy(){ aRefId.Destroy(); }
    };
    ERRTYPE         IsToDeep( const RSCINST & rInst, sal_uInt32 nDeep = 0 );
public:
                    RscMgr( Atom nId, sal_uInt32 nTypId, RscTop * pSuperCl );

    void            SetToDefault( const RSCINST & rInst );
    bool            IsDefault( const RSCINST & rInst );
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool );
    void            Destroy( const RSCINST & rInst );
    sal_uInt32      Size();
    void            WriteSrcHeader( const RSCINST & aInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & rId, const char * );

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & rId,
                                   sal_uInt32, bool bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra );
    bool            IsConsistent( const RSCINST & rInst );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );
};

#endif //_RSCMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
