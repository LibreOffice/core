/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscflag.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _RSCFLAG_HXX
#define _RSCFLAG_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rscconst.hxx>

/******************* R s c F l a g ***************************************/
class RscFlag : public RscConst {
    struct RscFlagInst{
        sal_uInt32  nFlags;
        sal_uInt32  nDfltFlags;
    };
    RSCINST         CreateBasic( RSCINST * pInst );
public:
                    RscFlag( Atom nId, sal_uInt32 nTypId );
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    RSCINST         CreateClient( RSCINST * pInst, const RSCINST & rDflt,
                                  BOOL bOwnClass, Atom nConsId );
    sal_uInt32          Size();

    virtual void    SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst, Atom nConstId );

                    // Ist das Flag gesetzt
    BOOL            IsSet( const RSCINST & rInst, Atom nConstId );

                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef,
                                    Atom nConstId );

    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              INT32 nValue );
    ERRTYPE         SetNotConst( const RSCINST & rInst, Atom nConstId );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
};

/******************* R s c C l i e n t ***********************************/
class RscClient : public RscTop
{
    RscFlag *   pRefClass;  //Klasse die als Server benutzt wird
    Atom      nConstId;   //Id des zu setzenden Wertes
public:
                    RscClient( Atom nId, sal_uInt32 nTypId, RscFlag * pClass,
                               Atom nConstantId );
    virtual RSCCLASS_TYPE   GetClassType() const;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    sal_uInt32          Size(){ return( pRefClass->Size() ); };

                    // Eine Zuweisung an eine Variable
    BOOL            IsDefault( const RSCINST & rInst ){
                        return( pRefClass->IsDefault( rInst, nConstId ) );
                    };
                    // Als Default setzen
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
                        return pRefClass->IsValueDefault( rInst,
                                                    pDef, nConstId );
                    }
    ERRTYPE         SetBool( const RSCINST & rInst, BOOL bValue ){
                        if( bValue )
                            return( pRefClass->SetConst( rInst, nConstId, bValue ) );
                        else
                            return( pRefClass->
                                    SetNotConst( rInst, nConstId ) );
                    };
    ERRTYPE         GetBool( const RSCINST & rInst, BOOL * pB ){
                        *pB = pRefClass->IsSet( rInst, nConstId );
                        return( ERR_OK );
                    };
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
};

#endif // _RSCFLAG_HXX
