/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rsccont.hxx,v $
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
#ifndef _RSCCONT_HXX
#define _RSCCONT_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctop.hxx>

/******************* R s c B a s e C o n t *******************************/
struct ENTRY_STRUCT {
    RscId   aName;
    RSCINST aInst;
    void Create(){ aName.Create(); aInst = RSCINST(); }
    void Destroy();
};
struct RscBaseContInst {
    sal_uInt32          nEntries;
    ENTRY_STRUCT *  pEntries;
    BOOL            bDflt;
};

class RscBaseCont : public RscTop
{
protected:
    RscTop *        pTypeClass; // Typ der Eintraege
    RscTop *        pTypeClass1;// Zwei verschiedene Typen moeglich
    BOOL            bNoId;      // Keine Identifier
    sal_uInt32          nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    sal_uInt32          nOffInstData;// Offset auf eigen Instanzdaten
    void            DestroyElements( RscBaseContInst * pClassData );
    RSCINST         SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                  RscTop * pClass, sal_uInt32 nPos );
protected:
    void            ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         ContWriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    void            ContWriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                        const char *, BOOL nWriteSize );
public:
                    RscBaseCont( Atom nId, sal_uInt32 nTypId,
                                 RscTop * pSuper = NULL,
                                 BOOL bNoId = TRUE );
                    ~RscBaseCont();
    virtual RSCCLASS_TYPE   GetClassType() const;
    void            SetTypeClass( RscTop * pClass, RscTop * pClass1 = NULL )
                    {
                        pTypeClass = pClass;
                        pTypeClass1 = pClass1;
                    };
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, BOOL );
    void            Destroy( const RSCINST & rInst );
    ERRTYPE         GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst );
    RSCINST         SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass );
    sal_uInt32          GetCount( const RSCINST & rInst );
    RSCINST         GetPosEle( const RSCINST & rInst, sal_uInt32 nPos );
    ERRTYPE         MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                sal_uInt32 nSourcePos );
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RscId & rRscId);
    SUBINFO_STRUCT  GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos );
    ERRTYPE         SetString( const RSCINST &, const char * pStr );
    ERRTYPE         SetNumber( const RSCINST &, INT32 lValue );
    ERRTYPE         SetBool( const RSCINST & rInst, BOOL bValue );
    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              INT32 nValue );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return( nSize ); };

    BOOL            IsConsistent( const RSCINST & rInst, RscInconsList * pList );
    void            SetToDefault( const RSCINST & rInst );
    BOOL            IsDefault( const RSCINST & rInst );
    BOOL            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    void            Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );
    void            DeletePos( const RSCINST & rInst, sal_uInt32 nPos );

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 , BOOL bExtra);
    ERRTYPE         WriteHxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId & rId );
    ERRTYPE         WriteCxx( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, const RscId &rId );
};

/******************* R s c C o n t W r i t e S r c ***********************/
class RscContWriteSrc : public RscBaseCont
{
public:
                    RscContWriteSrc( Atom nId, sal_uInt32 nTypId,
                                     RscTop * pSuper = NULL,
                                     BOOL bNoId = TRUE );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
};

/******************* R s c C o n t ***************************************/
class RscCont : public RscContWriteSrc {
public:
                    RscCont( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = NULL,
                             BOOL bNoId = TRUE );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                    const char * );
};

/******************* R s c C o n t E x t r a D a t a *********************/
class RscContExtraData : public RscContWriteSrc {
public:
                    RscContExtraData( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = NULL,
                             BOOL bNoId = TRUE );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, BOOL bExtra );
};

#endif //_RSCCONT_HXX
