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

#ifndef INCLUDED_RSC_INC_RSCARRAY_HXX
#define INCLUDED_RSC_INC_RSCARRAY_HXX

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
    virtual ~RscInstNode();
    virtual sal_uInt32  GetId() const override;
    RscInstNode *   Left() const { return static_cast<RscInstNode *>(pLeft); };
    RscInstNode *   Right() const{ return static_cast<RscInstNode *>(pRight); };
    RscInstNode *   Search( sal_uInt32 nId ) const
                        {
                            return static_cast<RscInstNode *>(IdNode::Search( nId ));
                        }
};

struct RscArrayInst
{
    RscInstNode *   pNode;
};

/* The tree is sorted against its enum value, not against its HashId */
class RscArray : public RscTop
{
protected:
    RscEnum *       pTypeClass; // type of entries
    sal_uInt32      nSize;      // size of this class instance data with super class
    sal_uInt32      nOffInstData;// Offset of self instance data
    void            WriteSrcArray( const RSCINST & rInst, FILE * fOutput,
                                   RscTypCont * pTC, sal_uInt32 nTab, const char * );
public:
                    RscArray( Atom nId, sal_uInt32 nTypId,
                              RscTop * pSuper, RscEnum * pTypeClass );
                    virtual ~RscArray();
    virtual RSCCLASS_TYPE   GetClassType() const override;

    virtual RscTop *    GetTypeClass() const override;
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool bOwnClass = false ) override;
    void            Destroy( const RSCINST & rInst ) override;
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, sal_Int32 lValue,
                                 RscTop * pCreateClass,
                                 RSCINST * pGetInst ) override;
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, Atom nId,
                                 RscTop * pCreateClass,
                                 RSCINST * pGetInst ) override;

                    // gives the size of the class in bytes
    sal_uInt32      Size() override { return nSize; }

    bool            IsConsistent( const RSCINST & rInst ) override;
    virtual void    SetToDefault( const RSCINST & rInst ) override;
    bool            IsDefault( const RSCINST & rInst ) override;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;

    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * ) override;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

class RscClassArray : public RscArray
{
public:
                    RscClassArray( Atom nId, sal_uInt32 nTypId,
                                   RscTop * pSuper, RscEnum * pTypeClass );
                    virtual ~RscClassArray();
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * ) override;
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                   sal_uInt32 nDeep, bool bExtra ) override;
};


class RscLangArray : public RscArray
{
public:
                    RscLangArray( Atom nId, sal_uInt32 nTypId,
                                  RscTop * pSuper, RscEnum * pTypeClass );
    virtual RSCCLASS_TYPE   GetClassType() const override;
};

#endif // INCLUDED_RSC_INC_RSCARRAY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
