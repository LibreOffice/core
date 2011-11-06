/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// C and C++ Includes.

// Solar Definitionen
#include <tools/solar.h>

// Programmabhaengige Includes.
#include <rscclobj.hxx>
#include <rsctop.hxx>

/****************** C O D E **********************************************/

/****************** R e f N o d e ****************************************/
/*************************************************************************
|*
|*    RefNode::RefNode()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.05.91
|*    Letzte Aenderung  MM 03.05.91
|*
*************************************************************************/
RefNode::RefNode( Atom nTyp ){
    pObjBiTree = 0;
    nTypNameId = nTyp;
}

/*************************************************************************
|*
|*    RefNode::GetId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
sal_uInt32 RefNode::GetId() const
{
    return( nTypNameId );
}

/*************************************************************************
|*
|*    RefNode::PutObjNode()
|*
|*    Beschreibung      NAME.DOC
|*    Ersterstellung    MM 21.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
sal_Bool RefNode::PutObjNode( ObjNode * pPutObject ){
// insert a node in the b-tree pObjBiTree
// if the node with the same name is in pObjBiTree,
// return sal_False and no insert,

    if( pObjBiTree )
        return( pObjBiTree->Insert( pPutObject ) );

    pObjBiTree = pPutObject;
    return( sal_True );
}

/****************** O b j N o d e ****************************************/
/*************************************************************************
|*
|*    RefNode::GetObjNode()
|*
|*    Beschreibung      NAME.DOC
|*    Ersterstellung    MM 21.03.90
|*    Letzte Aenderung  MM 27.06.90
|*
*************************************************************************/
ObjNode * RefNode :: GetObjNode( const RscId & rRscId ){
// insert a node in the b-tree pObjBiTree
// if the node with the same name is in pObjBiTree,
// return NULL and no insert,
// if not return the pointer to the Object

    if( pObjBiTree )
        return( pObjBiTree->Search( rRscId ) );
    return( NULL );
}

/*************************************************************************
|*
|*    ObjNode::ObjNode()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
ObjNode::ObjNode( const RscId & rId, CLASS_DATA pData, sal_uLong lKey ){
    pRscObj  = pData;
    aRscId   = rId;
    lFileKey = lKey;
}

/*************************************************************************
|*
|*    ObjNode::DelObjNode()
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    ObjNode::GetId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.10.91
|*    Letzte Aenderung  MM 29.10.91
|*
*************************************************************************/
sal_uInt32 ObjNode::GetId() const
{
    return( (sal_uInt32)(long)aRscId );
}

/*************************************************************************
|*
|*    ObjNode::IsConsistent()
|*
|*    Beschreibung
|*    Ersterstellung    MM 23.09.91
|*    Letzte Aenderung  MM 23.09.91
|*
*************************************************************************/
sal_Bool ObjNode::IsConsistent( RscInconsList * pList )
{
    sal_Bool bRet = sal_True;

    if( (long)aRscId > 0x7FFF || (long)aRscId < 1 )
    {
        bRet = sal_False;
        if( pList )
            pList->Insert( new RscInconsistent( aRscId, aRscId ) );
    }
    else
    {
        if( Left() )
        {
            if( !((ObjNode *)Left())->IsConsistent( pList ) )
                bRet = sal_False;
            if( ((ObjNode *)Left())->aRscId >= aRscId )
            {
                bRet = sal_False;
                if( pList )
                    pList->Insert(
                        new RscInconsistent( ((ObjNode *)Left())->GetRscId(),
                                             GetRscId() ) );
            }
        };
        if( Right() )
        {
            if( ((ObjNode *)Right())->aRscId <= aRscId )
            {
                bRet = sal_False;
                if( pList )
                    pList->Insert(
                        new RscInconsistent( GetRscId(),
                                             ((ObjNode *)Right())->GetRscId() ) );
            }
            if( !((ObjNode *)Right())->IsConsistent( pList ) )
                bRet = sal_False;
        };
    };

    return( bRet );
}
