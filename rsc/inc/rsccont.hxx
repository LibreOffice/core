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

struct ENTRY_STRUCT
{
    RscId   aName;
    RSCINST aInst;
    void Create(){ aName.Create(); aInst = RSCINST(); }
    void Destroy();
};
struct RscBaseContInst
{
    sal_uInt32          nEntries;
    ENTRY_STRUCT *  pEntries;
    bool            bDflt;
};

class RscBaseCont : public RscTop
{
protected:
    RscTop *        pTypeClass; // Typ der Eintraege
    RscTop *        pTypeClass1;// Zwei verschiedene Typen moeglich
    bool            bNoId;      // Keine Identifier
    sal_uInt32      nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    sal_uInt32      nOffInstData;// Offset auf eigen Instanzdaten
    void            DestroyElements( RscBaseContInst * pClassData );
    RSCINST         SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                  RscTop * pClass, sal_uInt32 nPos );
protected:
    void            ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         ContWriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, bool bExtra );
public:
                    RscBaseCont( Atom nId, sal_uInt32 nTypId,
                                 RscTop * pSuper = NULL,
                                 bool bNoId = true );
                    ~RscBaseCont();
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
    void            SetTypeClass( RscTop * pClass, RscTop * pClass1 = NULL )
                        {
                            pTypeClass = pClass;
                            pTypeClass1 = pClass1;
                        }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool ) SAL_OVERRIDE;
    void            Destroy( const RSCINST & rInst ) SAL_OVERRIDE;
    ERRTYPE         GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst ) SAL_OVERRIDE;
    RSCINST         SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass ) SAL_OVERRIDE;
    sal_uInt32          GetCount( const RSCINST & rInst ) SAL_OVERRIDE;
    RSCINST         GetPosEle( const RSCINST & rInst, sal_uInt32 nPos ) SAL_OVERRIDE;
    ERRTYPE         MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                sal_uInt32 nSourcePos ) SAL_OVERRIDE;
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RscId & rRscId) SAL_OVERRIDE;
    SUBINFO_STRUCT  GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos ) SAL_OVERRIDE;
    ERRTYPE         SetString( const RSCINST &, const char * pStr ) SAL_OVERRIDE;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 lValue ) SAL_OVERRIDE;
    ERRTYPE         SetBool( const RSCINST & rInst, bool bValue ) SAL_OVERRIDE;
    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) SAL_OVERRIDE;
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId ) SAL_OVERRIDE;

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32      Size() SAL_OVERRIDE { return nSize; }

    bool            IsConsistent( const RSCINST & rInst ) SAL_OVERRIDE;
    void            SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;

    void            Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId ) SAL_OVERRIDE;
    void            DeletePos( const RSCINST & rInst, sal_uInt32 nPos ) SAL_OVERRIDE;

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 , bool bExtra) SAL_OVERRIDE;
};

class RscContWriteSrc : public RscBaseCont
{
public:
                    RscContWriteSrc( Atom nId, sal_uInt32 nTypId,
                                     RscTop * pSuper = NULL,
                                     bool bNoId = true );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
};

class RscCont : public RscContWriteSrc
{
public:
                    RscCont( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = NULL,
                             bool bNoId = true );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

class RscContExtraData : public RscContWriteSrc
{
public:
                    RscContExtraData( Atom nId, sal_uInt32 nTypId,
                                      RscTop * pSuper = NULL,
                                      bool bNoId = true );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

#endif //_RSCCONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
