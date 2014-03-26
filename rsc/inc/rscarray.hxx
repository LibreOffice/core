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
#ifndef _RSCARRAY_HXX
#define _RSCARRAY_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctop.hxx>

class RscEnum;

class RscInstNode : public IdNode
{
    sal_uInt32  nTypeId;
protected:
    using NameNode::Search;
public:
    RSCINST aInst;
    RscInstNode( sal_uInt32 nId );
    ~RscInstNode();
    virtual sal_uInt32  GetId() const SAL_OVERRIDE;
    RscInstNode *   Left() const { return (RscInstNode *)pLeft  ; };
    RscInstNode *   Right() const{ return (RscInstNode *)pRight ; };
    RscInstNode *   Search( sal_uInt32 nId ) const
                        {
                            return (RscInstNode *)IdNode::Search( nId );
                        }
};

struct RscArrayInst
{
    RscInstNode *   pNode;
};

/* Der Baum wird ueber die Werte des Enums sortiert, nicht ueber
    seine HashId.
*/
class RscArray : public RscTop
{
protected:
    RscEnum *       pTypeClass; // Typ der Eintraege
    sal_uInt32      nSize;      // Groesse der Instanzdaten dieser Klasse
                                // mit Superklassen
    sal_uInt32      nOffInstData;// Offset auf eigen Instanzdaten
    void            WriteSrcArray( const RSCINST & rInst, FILE * fOutput,
                                   RscTypCont * pTC, sal_uInt32 nTab, const char * );
public:
                    RscArray( Atom nId, sal_uInt32 nTypId,
                              RscTop * pSuper, RscEnum * pTypeClass );
                    ~RscArray();
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;

    void            SetTypeClass( RscEnum * pClass ) { pTypeClass = pClass; }
    virtual RscTop *    GetTypeClass() const SAL_OVERRIDE;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool ) SAL_OVERRIDE;
    void            Destroy( const RSCINST & rInst ) SAL_OVERRIDE;
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, sal_Int32 lValue,
                                 RscTop * pCreateClass,
                                 RSCINST * pGetInst ) SAL_OVERRIDE;
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, Atom nId,
                                 RscTop * pCreateClass,
                                 RSCINST * pGetInst ) SAL_OVERRIDE;

                    // Gibt die Groesse der Klasse in Bytes
    sal_uInt32      Size() SAL_OVERRIDE { return nSize; }

    bool            IsConsistent( const RSCINST & rInst ) SAL_OVERRIDE;
    virtual void    SetToDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsDefault( const RSCINST & rInst ) SAL_OVERRIDE;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) SAL_OVERRIDE;

    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) SAL_OVERRIDE;
};

class RscClassArray : public RscArray
{
public:
                    RscClassArray( Atom nId, sal_uInt32 nTypId,
                                   RscTop * pSuper, RscEnum * pTypeClass );
                    ~RscClassArray();
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * ) SAL_OVERRIDE;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) SAL_OVERRIDE;
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                   sal_uInt32 nDeep, bool bExtra ) SAL_OVERRIDE;
};


class RscLangArray : public RscArray
{
public:
                    RscLangArray( Atom nId, sal_uInt32 nTypId,
                                  RscTop * pSuper, RscEnum * pTypeClass );
    virtual RSCCLASS_TYPE   GetClassType() const SAL_OVERRIDE;
};

#endif //_RSCARRAY

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
