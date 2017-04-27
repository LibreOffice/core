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
#include <rsc/rsc-vcl-shared-types.hxx>

#include <rscmgr.hxx>
#include <rscclass.hxx>
#include <rsccont.hxx>
#include <rscdb.hxx>
#include <rsc/rscsfx.hxx>
#include <rsclex.hxx>
#include <rscyacc.hxx>

RscTop * RscTypCont::InitClassMgr()
{
    RscTop      *   pClassMgr;
    RscBaseCont *   pClass;
    Atom            nId;

    aBaseLst.push_back( pClass = new RscBaseCont( InvalidAtom, RSC_NOTYPE, false ) );

    nId = pHS->getID( "Resource" );
    pClassMgr = new RscMgr( nId, RSC_RESOURCE, pClass );
    aNmTb.Put( nId, CLASSNAME, pClassMgr );
    pClassMgr->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // initialize variables
    nId = aNmTb.Put( "Comment", VARNAME );
    pClassMgr->SetVariable( nId, &aString, nullptr, RSCVAR::NoRc );

    pClass->SetTypeClass( pClassMgr );

    return pClassMgr;
}

RscTop * RscTypCont::InitClassString( RscTop * pSuper )
{
    Atom        nId;
    RscTop *    pClassString;

    nId = pHS->getID( "String" );
    pClassString = new RscClass( nId, RSC_STRING, pSuper );
    aNmTb.Put( nId, CLASSNAME, pClassString );
    pClassString->SetCallPar( *pStdPar1, *pStdPar2, *pStdParType );

    // initialize variables
    nId = aNmTb.Put( "Text", VARNAME );
    pClassString->SetVariable( nId, &aLangString );
    return pClassString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
