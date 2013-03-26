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


#include <rscclobj.hxx>
#include <rsctop.hxx>


RefNode::RefNode( Atom nTyp ){
    pObjBiTree = 0;
    nTypNameId = nTyp;
}

sal_uInt32 RefNode::GetId() const
{
    return( nTypNameId );
}

sal_Bool RefNode::PutObjNode( ObjNode * pPutObject ){
// insert a node in the b-tree pObjBiTree
// if the node with the same name is in pObjBiTree,
// return sal_False and no insert,

    if( pObjBiTree )
        return( pObjBiTree->Insert( pPutObject ) );

    pObjBiTree = pPutObject;
    return( sal_True );
}

ObjNode * RefNode :: GetObjNode( const RscId & rRscId ){
// insert a node in the b-tree pObjBiTree
// if the node with the same name is in pObjBiTree,
// return NULL and no insert,
// if not return the pointer to the Object

    if( pObjBiTree )
        return( pObjBiTree->Search( rRscId ) );
    return( NULL );
}

ObjNode::ObjNode( const RscId & rId, CLASS_DATA pData, sal_uLong lKey ){
    pRscObj  = pData;
    aRscId   = rId;
    lFileKey = lKey;
}

ObjNode * ObjNode::DelObjNode( RscTop * pClass, sal_uLong nFileKey ){
    ObjNode * pRetNode = this;

    if( Right() )
        pRight = ((ObjNode *)Right())->DelObjNode( pClass, nFileKey );
    if( Left() )
        pLeft = ((ObjNode *)Left())->DelObjNode( pClass, nFileKey );

    if( GetFileKey() == nFileKey ){
        if( GetRscObj() ){
            pClass->Destroy( RSCINST( pClass, GetRscObj() ) );
            rtl_freeMemory( GetRscObj() );
        }
        pRetNode = (ObjNode *)Right();
        if( pRetNode ){
            if( Left() )
                pRetNode->Insert( (ObjNode *)Left() );
        }
        else
            pRetNode = (ObjNode *)Left();
        delete this;
    }
    return pRetNode;
}

sal_uInt32 ObjNode::GetId() const
{
    return( (sal_uInt32)(long)aRscId );
}

sal_Bool ObjNode::IsConsistent()
{
    sal_Bool bRet = sal_True;

    if( (long)aRscId > 0x7FFF || (long)aRscId < 1 )
    {
        bRet = sal_False;
    }
    else
    {
        if( Left() )
        {
            if( !((ObjNode *)Left())->IsConsistent() )
                bRet = sal_False;
            if( ((ObjNode *)Left())->aRscId >= aRscId )
            {
                bRet = sal_False;
            }
        };
        if( Right() )
        {
            if( ((ObjNode *)Right())->aRscId <= aRscId )
            {
                bRet = sal_False;
            }
            if( !((ObjNode *)Right())->IsConsistent() )
                bRet = sal_False;
        };
    };

    return( bRet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
