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
#ifndef _RSCCLASS_HXX
#define _RSCCLASS_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rschash.hxx>
#include <rsctop.hxx>

class RscClass : public RscTop
{
protected:
    struct RscClassInst
    {
        sal_uLong       nVarDflt;
    };
    struct VARTYPE_STRUCT
    {
        Atom            nVarName;   // Variablenname
        RSCVAR          nVarType;   // Variablentyp
        sal_uInt32      nMask;      // Maskierungsbit
        sal_uInt32      nOffset;    // Beginn der Instanzdaten
        RscTop *        pClass;     // Klasse
        CLASS_DATA      pDefault;   // Zeiger auf DefaultDaten
        Atom            nDataBaseName;//Name fuer Fremddatenbereich
    };
    sal_uInt32          nSuperSize; // Groesse der Instanzdaten der SuperKl.
    sal_uInt32          nSize;      // Groesse der Instanzdaten dieser Klasse
                                    // mit Superklassen
    sal_uInt32          nEntries;   // Eintraege in pVarTypeList
    VARTYPE_STRUCT *    pVarTypeList;   // Variablenliste
    RSCINST             GetInstData( CLASS_DATA pData, sal_uInt32 nEle,
                                     bool bGetCopy = false );
    CLASS_DATA          GetDfltData( sal_uInt32 nEle );
    bool                IsDflt( CLASS_DATA pData, sal_uInt32 nEle );
    bool                IsValueDflt( CLASS_DATA pData, sal_uInt32 nEle );
    void                SetVarDflt( CLASS_DATA pData, sal_uInt32 nEle,
                                    bool bSet );
    sal_Int32           GetCorrectValues( const RSCINST & rInst, sal_uInt32 nVarPos,
                                          sal_uInt32 nTupelIdx, RscTypCont * pTC );
public:
                    RscClass( Atom nId, sal_uInt32 nTypId, RscTop * pSuperCl );
                    ~RscClass();

    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;

    void            Pre_dtor() SAL_OVERRIDE;
    ERRTYPE         SetVariable( Atom nVarName, RscTop * pClass,
                                 RSCINST * pDflt,
                                 RSCVAR nVarType, sal_uInt32 nMask,
                                 Atom nDataBaseName ) SAL_OVERRIDE;
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc ) SAL_OVERRIDE;
    RSCINST         GetVariable( const RSCINST & rInst, Atom nVarName,
                                 const RSCINST & rInitInst,
                                 bool nInitDflt = false,
                                 RscTop * pCreateClass = NULL ) SAL_OVERRIDE;
    RSCINST         GetCopyVar( const RSCINST & rInst, Atom nVarName ) SAL_OVERRIDE;

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32      Size() SAL_OVERRIDE { return nSize; }

    bool            IsConsistent( const RSCINST & rInst ) SAL_OVERRIDE;
    void            SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;
    void            SetDefault( const RSCINST & rData, Atom nVarId ) SAL_OVERRIDE;
    using RscTop::GetDefault;
    RSCINST         GetDefault( Atom nVarId ) SAL_OVERRIDE;

    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool ) SAL_OVERRIDE;
    void            Destroy( const RSCINST & rInst ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteInstRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, bool bExtra );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

class RscSysDepend : public RscClass
{
public:
                    RscSysDepend( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    ERRTYPE         WriteSysDependRc( const RSCINST &, RscWriteRc & aMem,
                                      RscTypCont * pTC, sal_uInt32, bool bExtra,
                                      bool bFirst = false );
    ERRTYPE         WriteRc( const RSCINST &, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

class RscTupel : public RscClass
{
public:
    RscTupel( Atom nId, sal_uInt32 nTypId, RscTop * pSuper );
    RSCINST         GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RSCINST & rInitInst ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
};

#endif //_RSCCLASS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
