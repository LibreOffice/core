/*************************************************************************
 *
 *  $RCSfile: rscflag.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-03 17:21:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _RSCFLAG_HXX
#define _RSCFLAG_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif
#ifndef _RSCCONST_HXX
#include <rscconst.hxx>
#endif

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
