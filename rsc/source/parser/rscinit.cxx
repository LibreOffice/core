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


#include <stdlib.h>
#include <stdio.h>

#include <tools/rcid.h>
#include <tools/wintypes.hxx>

#include <rsctree.hxx>
#include <rsctop.hxx>
#include <rscrange.hxx>
#include <rscconst.hxx>
#include <rscstr.hxx>
#include <rsccont.hxx>
#include <rscmgr.hxx>
#include <rscclass.hxx>
#include <rsckey.hxx>
#include <rscdb.hxx>
#include <rsclex.hxx>
#include <rscyacc.hxx>

void RscTypCont::Init()
{
    RscTupel *  pStringLongTupel;
    RscCont  *  pStringLongTupelList;
    RscArray *  pLangStringLongTupelList;

    RscTop   *  pClassMgr;
    RscTop   *  pClassString;
    RscTop   *  pClassStringArray;

    Atom        nId;

    aNmTb.SetSort( false );
{
    aNmTb.Put( "LINE",               LINE,           (sal_IntPtr)0 );
    aNmTb.Put( "NOT",                NOT,            (sal_IntPtr)0 );
    aNmTb.Put( "DEFINE",             DEFINE,         (sal_IntPtr)0 );
    aNmTb.Put( "INCLUDE",            INCLUDE,        (sal_IntPtr)0 );
    aNmTb.Put( "DEFAULT",            DEFAULT,        (sal_IntPtr)0  );
    aNmTb.Put( "class",              CLASS,          (sal_IntPtr)0  );
    aNmTb.Put( "extendable",         EXTENDABLE,     (sal_IntPtr)0  );
    aNmTb.Put( "writeifset",         WRITEIFSET,     (sal_IntPtr)0  );

/* values for integer types */
    aNmTb.Put( "TRUE",               BOOLEAN,        (sal_IntPtr)true  );
    aNmTb.Put( "FALSE",              BOOLEAN,        (sal_IntPtr)false );
}
{
    aEnumLong.SetRange( SAL_MIN_INT32, SAL_MAX_INT32 );
}
{
    aLangType.Init( aNmTb );
    aBaseLst.push_back( InitFieldUnitsType() );

    aBaseLst.push_back( pStringLongTupel = InitStringLongTupel() );
    aBaseLst.push_back( pStringLongTupelList = InitStringLongTupelList( pStringLongTupel ) );
    aBaseLst.push_back( pLangStringLongTupelList = InitLangStringLongTupelList( pStringLongTupelList ) );
}
{
    pRoot = pClassMgr = InitClassMgr();

    aVersion.pClass = new RscClass( pHS->getID( "VersionControl" ),
                                    RSC_VERSIONCONTROL, pClassMgr );
    aVersion = aVersion.pClass->Create( nullptr, RSCINST() );

    pClassString = InitClassString( pClassMgr );
    pRoot->Insert( pClassString );

    // set String as reference class of the base strings
    aString.SetRefClass( pClassString );

    // initialize class
    nId = pHS->getID( "StringArray" );
    pClassStringArray = new RscClass( nId, RSC_STRINGARRAY, pClassMgr );
    pClassStringArray->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );
    aNmTb.Put( nId, CLASSNAME, pClassStringArray );
    pRoot->Insert( pClassStringArray );

    // initialize variables
    nId = aNmTb.Put( "ItemList", VARNAME );
    pClassStringArray->SetVariable( nId, pLangStringLongTupelList );
}

    aNmTb.SetSort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
