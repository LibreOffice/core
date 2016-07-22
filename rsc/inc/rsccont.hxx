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
#ifndef INCLUDED_RSC_INC_RSCCONT_HXX
#define INCLUDED_RSC_INC_RSCCONT_HXX

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
    RscTop *        pTypeClass; // type of entries
    RscTop *        pTypeClass1;// two different types may exist
    bool            bNoId;      // whether there is no identifier
    sal_uInt32      nSize;      // size of this class and super classes instance data
    sal_uInt32      nOffInstData;// offset of own instance data
    static void     DestroyElements( RscBaseContInst * pClassData );
    RSCINST         SearchElePos( const RSCINST & rInst, const RscId & rEleName,
                                  RscTop * pClass, sal_uInt32 nPos );
protected:
    void            ContWriteSrc( const RSCINST & rInst, FILE * fOutput,
                                  RscTypCont * pTC, sal_uInt32 nTab, const char * );
    ERRTYPE         ContWriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                                 RscTypCont * pTC, sal_uInt32, bool bExtra );
public:
                    RscBaseCont( Atom nId, sal_uInt32 nTypId,
                                 RscTop * pSuper = nullptr,
                                 bool bNoId = true );
                    virtual ~RscBaseCont();
    virtual RSCCLASS_TYPE   GetClassType() const override;
    void            SetTypeClass( RscTop * pClass, RscTop * pClass1 = nullptr )
                        {
                            pTypeClass = pClass;
                            pTypeClass1 = pClass1;
                        }
    RSCINST         Create( RSCINST * pInst, const RSCINST & rDflt, bool bOwnClass = false ) override;
    void            Destroy( const RSCINST & rInst ) override;
    ERRTYPE         GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop * pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst ) override;
    RSCINST         SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass ) override;
    sal_uInt32          GetCount( const RSCINST & rInst ) override;
    RSCINST         GetPosEle( const RSCINST & rInst, sal_uInt32 nPos ) override;
    ERRTYPE         MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                sal_uInt32 nSourcePos ) override;
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RscId & rRscId) override;
    SUBINFO_STRUCT  GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos ) override;
    ERRTYPE         SetString( const RSCINST &, const char * pStr ) override;
    ERRTYPE         SetNumber( const RSCINST &, sal_Int32 lValue ) override;
    ERRTYPE         SetBool( const RSCINST & rInst, bool bValue ) override;
    ERRTYPE         SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue ) override;
    ERRTYPE         SetRef( const RSCINST & rInst, const RscId & rRefId ) override;

                    // returns the class size in bytes
    sal_uInt32      Size() override { return nSize; }

    bool            IsConsistent( const RSCINST & rInst ) override;
    void            SetToDefault( const RSCINST & rInst ) override;
    bool            IsDefault( const RSCINST & rInst ) override;
    bool            IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ) override;

    void            Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId ) override;
    void            DeletePos( const RSCINST & rInst, sal_uInt32 nPos ) override;

    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 , bool bExtra) override;
};

class RscContWriteSrc : public RscBaseCont
{
public:
                    RscContWriteSrc( Atom nId, sal_uInt32 nTypId,
                                     RscTop * pSuper = nullptr );
    void            WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab, const char * ) override;
};

class RscCont : public RscContWriteSrc
{
public:
                    RscCont( Atom nId, sal_uInt32 nTypId,
                             RscTop * pSuper = nullptr );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

class RscContExtraData : public RscContWriteSrc
{
public:
                    RscContExtraData( Atom nId, sal_uInt32 nTypId,
                                      RscTop * pSuper = nullptr );
    ERRTYPE         WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32, bool bExtra ) override;
};

#endif // INCLUDED_RSC_INC_RSCCONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
