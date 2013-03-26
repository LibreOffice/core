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
#ifndef _RSCCONT_HXX
#define _RSCCONT_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctop.hxx>

struct ENTRY_STRUCT {
    RscId   aName;
    RSCINST aInst;
    void Create(){ aName.Create(); aInst = RSCINST(); }
    void Destroy();
};
struct RscBaseContInst {
    sal_uInt32          nEntries;
    ENTRY_STRUCT *  pEntries;
    sal_Bool            bDflt;
};

class RscBaseCont : public RscTop
{
protected:
    RscTop *        pTypeClass; // Typ der Eintraege
    RscTop *        pTypeClass1;// Zwei verschiedene Typen moeglich
    sal_Bool            bNoId;      // Keine Identifier
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
                                 RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
public:
                    RscBaseCont( Atom nId, sal_uInt32 nTypId,
                                 RscTop * pSuper = NULL,
                                 sal_Bool bNoId = sal_True );
                    ~RscBaseCont();
    virtual RSCCLASS_TYPE   GetClassType() const;
    void            SetTypeClass( RscTop * pClass, RscTop * pClass1 = NULL )
                    {
                        pTypeClass = pClass;
                        pTypeClass1 = pClass1;
                    };
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, sal_Bool );
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
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 lValue );
    ERRTYPE         SetBool( const RSCINST & rInst, sal_Bool bValue );
    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue );
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32          Size(){ return( nSize ); };

    sal_Bool            IsConsistent( const RSCINST & rInst );
    void            SetToDefault( const RSCINST & rInst );
    sal_Bool            IsDefault( const RSCINST & rInst );
    sal_Bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

    void            Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );
    void            DeletePos( const RSCINST & rInst, sal_uInt32 nPos );

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 , sal_Bool bExtra);
};

class RscContWriteSrc : public RscBaseCont
{
public:
                    RscContWriteSrc( Atom nId, sal_uInt32 nTypId,
                                     RscTop * pSuper = NULL,
                                     sal_Bool bNoId = sal_True );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * );
};

class RscCont : public RscContWriteSrc {
public:
                    RscCont( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = NULL,
                             sal_Bool bNoId = sal_True );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
};

class RscContExtraData : public RscContWriteSrc {
public:
                    RscContExtraData( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = NULL,
                             sal_Bool bNoId = sal_True );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, sal_Bool bExtra );
};

#endif //_RSCCONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
