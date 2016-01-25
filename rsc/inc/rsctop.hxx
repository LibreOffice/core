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
#ifndef INCLUDED_RSC_INC_RSCTOP_HXX
#define INCLUDED_RSC_INC_RSCTOP_HXX

#include <rscerror.h>
#include <rsctools.hxx>
#include <rschash.hxx>
#include <rscclobj.hxx>

typedef sal_uInt32 RSCVAR;
#define VAR_POINTER     0x0001
#define VAR_HIDDEN      0x0002
#define VAR_NODATAINST  0x0004
#define VAR_NORC        0x0008
#define VAR_SVDYNAMIC   0x0010
#define VAR_NOENUM      0x0020
#define VAR_EXTENDABLE  0x0040  /* class derivation can all be given */

class RscTop : public RefNode
{
    RscTop *        pSuperClass;
    RSCINST         aDfltInst;
    sal_uInt32          nTypId;
    RscTop *        pRefClass;

protected:
                    RscTop( Atom nId, sal_uInt32 nTypIdent,
                            RscTop * pSuperCl = nullptr );

public:
         OString    aCallPar1;      // class call without types until ResId
         OString    aCallPar2;      // class call without types staring at ResId
         OString    aCallParType;   // class call with types

            RscTop* GetSuperClass() const
                    { return pSuperClass; }
                    // returns the type identifier
            sal_uInt32  GetTypId() const
                    { return nTypId; };
                    // returns the super class
            bool    InHierarchy( RscTop * pClass );
            void    SetCallPar( const OString& rPar1, const OString& rPar2,
                                const OString& rParType );
            RscTop* GetRefClass() const { return pRefClass; }
    virtual RSCCLASS_TYPE GetClassType() const = 0;
            RSCINST GetDefault();

                    // preparation fro the destructor call
                    // given that classes can have mutual dependencies,
                    // we cannot assume in destructor that all class pointer
                    // are still valid
    virtual void    Pre_dtor();

    virtual RscTop* GetTypeClass() const;

                    // returns the class size in bytes
    virtual sal_uInt32  Size();

                    // returns the reference
    virtual ERRTYPE GetRef( const RSCINST & rInst, RscId * );

                    // sets the reference
    virtual ERRTYPE SetRef( const RSCINST & rInst, const RscId & rRefId );

                    // sets the variable
    virtual ERRTYPE SetVariable( Atom nVarName, RscTop * pClass,
                                 RSCINST * pDflt = nullptr,
                                 RSCVAR nVarType = 0, sal_uInt32 nMask = 0,
                                 Atom nDataBaseName = InvalidAtom );

                    // enumerate all variables
    virtual void    EnumVariables( void * pData, VarEnumCallbackProc );

                    // returns variable instance
                    // returned pData, pClass may be NULL
    virtual RSCINST GetVariable( const RSCINST & rInst, Atom nVarName,
                                 const RSCINST & rInitInst,
                                 bool bInitDflt = false,
                                 RscTop * pCreateClass = nullptr );
    virtual RSCINST GetCopyVar( const RSCINST & rInst, Atom nVarName );

    virtual RSCINST GetTupelVar( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RSCINST & rInitInst );

                    // returns instance from a field
                    // returned pGetInst may be NULL
    virtual ERRTYPE GetElement( const RSCINST & rInst, const RscId & rEleName,
                                RscTop *pCreateClass, const RSCINST & rCreateInst,
                                RSCINST * pGetInst );

                    // returns instance from a value
                    // returned pGetInst may be NULL
    virtual ERRTYPE GetValueEle( const RSCINST & rInst, sal_Int32 lValue,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

                    // returns instance from an array
                    // returned pGetInst may be NULL
    virtual ERRTYPE GetArrayEle( const RSCINST & rInst, Atom nId,
                                RscTop * pCreateClass,
                                RSCINST * pGetInst );

    virtual RSCINST SearchEle( const RSCINST & rInst, const RscId & rEleName,
                               RscTop * pClass );

                    // returns instance at the position
    virtual RSCINST GetPosEle( const RSCINST & rInst, sal_uInt32 nPos );

                    // move an instance
    virtual ERRTYPE MovePosEle( const RSCINST & rInst, sal_uInt32 nDestPos,
                                sal_uInt32 nSourcePos );

                    // changes RscId at position
    virtual ERRTYPE SetPosRscId( const RSCINST & rInst, sal_uInt32 nPos,
                                 const RscId & rRscId);

                    // returns instance information at position
    virtual SUBINFO_STRUCT GetInfoEle( const RSCINST & rInst, sal_uInt32 nPos );

                    // number of entries
    virtual sal_uInt32 GetCount( const RSCINST & rInst );

                    // an assignment to a variable
    virtual ERRTYPE SetNumber( const RSCINST & rInst, sal_Int32 lValue );

                    // an assignment to a variable
    virtual ERRTYPE SetBool( const RSCINST & rInst, bool bValue );

                    // an assignment to a variable
    virtual ERRTYPE SetConst( const RSCINST & rInst, Atom nValueId,
                              sal_Int32 nValue );

                    // an assignment to a variable
    virtual ERRTYPE SetNotConst( const RSCINST & rInst, Atom nId );

    virtual ERRTYPE SetString( const RSCINST & rInst, const char * pStr );

    virtual ERRTYPE GetNumber( const RSCINST & rInst, sal_Int32 * pN );

    virtual ERRTYPE GetBool( const RSCINST & rInst, bool * pB );

    virtual ERRTYPE GetConst( const RSCINST & rInst, Atom * pH );

    virtual ERRTYPE GetString( const RSCINST & rInst, char ** ppStr );

    virtual RSCINST Create( RSCINST * pInst,
                            const RSCINST & rDefInst, bool bOwnClass = false );

                    // destroys instance
    virtual void    Destroy( const RSCINST & rInst );

                    // checks consistency
    virtual bool    IsConsistent( const RSCINST & rInst );

                    // sets all default values
    virtual void    SetToDefault( const RSCINST & rInst );

                    // wether input is equal to default
    virtual bool    IsDefault( const RSCINST & rInst );

                    // sets value to default
    virtual bool    IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef );

                    // sets intance to default
    virtual void    SetDefault( const RSCINST & rInst, Atom nVarId );

                    // returns a variable default
    virtual RSCINST GetDefault( Atom nVarId );

    virtual void    Delete( const RSCINST & rInst, RscTop * pClass,
                            const RscId & rId );

    virtual void    DeletePos( const RSCINST & rInst, sal_uInt32 nPos );

                    // writes header and footer of a resource script file
    virtual void    WriteSrcHeader( const RSCINST & rInst, FILE * fOutput,
                                    RscTypCont * pTC, sal_uInt32 nTab,
                                    const RscId & aId, const char * );
    virtual void    WriteSrc( const RSCINST & rInst, FILE * fOutput,
                              RscTypCont * pTC, sal_uInt32 nTab,const char * );
    virtual ERRTYPE WriteRcHeader( const RSCINST & rInst, RscWriteRc & aMem,
                                   RscTypCont * pTC, const RscId & aId,
                                    sal_uInt32 nDeep, bool bExtra );
    virtual ERRTYPE WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont * pTC, sal_uInt32 nDeep, bool bExtra );
};

#endif // INCLUDED_RSC_INC_RSCTOP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
