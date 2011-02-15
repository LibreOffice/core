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
#ifndef _RSCMGR_HXX
#define _RSCMGR_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>
#include <rscclass.hxx>

/******************* R s c M g r *****************************************/
class RscMgr : public RscClass {
    struct RscMgrInst {
        RscId   aRefId; // nRefId = Referenz Identifier
        sal_Bool    bDflt;  // Ist Default
        void Create(){ aRefId.Create(); bDflt = sal_True; }
        void Destroy(){ aRefId.Destroy(); }
    };
    ERRTYPE         IsToDeep( const RSCINST & rInst, sal_uInt32 nDeep = 0 );
public:
                    RscMgr( Atom nId, sal_uInt32 nTypId, RscTop * pSuperCl );

    void            SetToDefault( const RSCINST & rInst );
    sal_Bool            IsDefault( const RSCINST & rInst );
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool );
    void            Destroy( const RSCINST & rInst );
    sal_uInt32          Size();
    void            WriteSrcHeader( const RSCINST & aInst, FILE * fOutput,
                            RscTypCont * pTC, sal_uInt32 nTab,
                            const RscId & rId, const char * );

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & rId,
                                   sal_uInt32, sal_Bool bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    ERRTYPE         WriteHxxHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, const RscId & rId );
    ERRTYPE         WriteHxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId & rId );
    ERRTYPE         WriteCxxHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, const RscId & rId );
    ERRTYPE         WriteCxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId & rId );
    sal_Bool            IsConsistent( const RSCINST & rInst,
                                  RscInconsList * pList = NULL );
    ERRTYPE         GetRef( const RSCINST & rInst, RscId * );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );
};

#endif //_RSCMGR_HXX
