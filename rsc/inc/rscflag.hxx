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
#ifndef _RSCFLAG_HXX
#define _RSCFLAG_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rscconst.hxx>

/******************* R s c F l a g ***************************************/
class RscFlag : public RscConst
{
    struct RscFlagInst
    {
        sal_uInt32  nFlags;
        sal_uInt32  nDfltFlags;
    };
    RSCINST         CreateBasic( RSCINST * pInst );
public:
                    RscFlag( Atom nId, sal_uInt32 nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool ) SAL_OVERRIDE;
    RSCINST         CreateClient( RSCINST * pInst, const RSCINST & rDflt,
                                  bool bOwnClass, Atom nConsId );
    sal_uInt32      Size() SAL_OVERRIDE;

    virtual void    SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsDefault( const RSCINST & rInst, Atom nConstId );

                    // Ist das Flag gesetzt
    bool            IsSet( const RSCINST & rInst, Atom nConstId );

                    // Als Default setzen
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef,
                                    Atom nConstId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) SAL_OVERRIDE;
    ERRTYPE         SetNotConst( const RSCINST & rInst, Atom nConstId ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

/******************* R s c C l i e n t ***********************************/
class RscClient : public RscTop
{
    RscFlag *       pRefClass;  //Klasse die als Server benutzt wird
    Atom            nConstId;   //Id des zu setzenden Wertes
public:
                    RscClient( Atom nId, sal_uInt32 nTypId, RscFlag * pClass,
                               Atom nConstantId );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool ) SAL_OVERRIDE;
    sal_uInt32      Size() SAL_OVERRIDE { return( pRefClass->Size() ); };

                    // Eine Zuweisung an eine Variable
    bool            IsDefault( const RSCINST & rInst ) SAL_OVERRIDE {
                        return( pRefClass->IsDefault( rInst, nConstId ) );
                    };
                    // Als Default setzen
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE
                        {
                            return pRefClass->IsValueDefault( rInst,
                                                              pDef, nConstId );
                        }
    ERRTYPE         SetBool( const RSCINST & rInst, bool bValue ) SAL_OVERRIDE
                        {
                            if( bValue )
                                return( pRefClass->SetConst( rInst, nConstId, sal_Int32(bValue) ) );
                            else
                                return( pRefClass->
                                        SetNotConst( rInst, nConstId ) );
                        }
    ERRTYPE         GetBool( const RSCINST & rInst, bool * pB ) SAL_OVERRIDE
                        {
                            *pB = pRefClass->IsSet( rInst, nConstId );
                            return( ERR_OK );
                        }
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
};

#endif // _RSCFLAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
