/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _RSCCLASS_HXX
#define _RSCCLASS_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

/******************* R s c C l a s s *************************************/
class RscClass : public RscTop
{
protected:
    struct RscClassInst{
        sal_uLong   nVarDflt;
    };
    struct VARTYPE_STRUCT {
        Atom            nVarName;   // Variablenname
        RSCVAR          nVarType;   // Variablentyp
        sal_uInt32      nMask;      // Maskierungsbit
        sal_uInt32      nOffset;    // Beginn der Instanzdaten
        RscTop *        pClass;     // Klasse
        CLASS_DATA      pDefault;   // Zeiger auf DefaultDaten
        Atom            nDataBaseName;//Name fuer Fremddatenbereich
    };
    sal_uInt32              nSuperSize; // Groesse der Instanzdaten der SuperKl.
    sal_uInt32              nSize;      // Groesse der Instanzdaten dieser Klasse
                                    // mit Superklassen
    sal_uInt32              nEntries;   // Eintraege in pVarTypeList
    VARTYPE_STRUCT *    pVarTypeList;   // Variablenliste
    RSCINST             GetInstData( CLASS_DATA pData, sal_uInt32 nEle,
                                     sal_Bool bGetCopy = sal_False );
    CLASS_DATA          GetDfltData( sal_uInt32 nEle );
    sal_Bool                IsDflt( CLASS_DATA pData, sal_uInt32 nEle );
    sal_Bool                IsValueDflt( CLASS_DATA pData, sal_uInt32 nEle );
    void                SetVarDflt( CLASS_DATA pData, sal_uInt32 nEle,
                                    sal_Bool bSet );
    sal_Int32               GetCorrectValues( const RSCINST & rInst, sal_uInt32 nVarPos,
                                        sal_uInt32 nTupelIdx, RscTypCont * pTC );
public:
                    RscClass( Atom nId, sal_uInt32 nTypId, RscTop * pSuperCl );
                    ~RscClass();

    virtual RSCCLASS_TYPE   GetClassType() const;

    void            Pre_dtor();
    ERRTYPE         SetVariable( Atom nVarName, RscTop * pClass,
                                 RSCINST * pDflt,
                                 RSCVAR nVarType, sal_uInt32 nMask,
                                 Atom nDataBaseName );
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );
    RSCINST         GetVariable( const RSCINST & rInst, Atom nVarName,
                                 const RSCINST & rInitInst,
                                 sal_Bool nInitDflt = sal_False,
                                 RscTop * pCreateClass = NULL );
    RSCINST         GetCopyVar( const RSCINST & rInst, Atom nVarName );

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return( nSize ); };

    sal_Bool            IsConsistent( const RSCINST & rInst );
    void            SetToDefault( const RSCINST & rInst );
    sal_Bool            IsDefault( const RSCINST & rInst );
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );
    void            SetDefault( const RSCINST & rData, Atom nVarId );
    using RscTop::GetDefault;
    RSCINST         GetDefault( Atom nVarId );

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool );
    void            Destroy( const RSCINST & rInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteInstRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
    void            WriteSyntax( FILE * fOutput, RscTypCont * pTC );

    void            WriteRcAccess( FILE * fOutput, RscTypCont * pTC,
                                const char * );
    void            WriteRcCtor( FILE * fOutput, RscTypCont * pTC );
};

class RscSysDepend : public RscClass
{
public:
                    RscSysDepend( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    ERRTYPE         WriteSysDependRc( const RSCINST &, RscWriteRc & aMem,
                                    RscTypCont * pTC, sal_uInt32, sal_Bool bExtra,
                                    sal_Bool bFirst = sal_False );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
};

class RscFirstSysDepend : public RscSysDepend
{
public:
                    RscFirstSysDepend( Atom nId, sal_uInt32 nTypId,
                                        RscTop * pSuper );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
};

class RscTupel : public RscClass
{
public:
    RscTupel( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    RSCINST         GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                const RSCINST & rInitInst );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
};

#endif //_RSCCLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
