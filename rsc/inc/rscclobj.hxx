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


#ifndef _RSCCLOBJ_HXX
#define _RSCCLOBJ_HXX

#include <rsctree.hxx>
#include <rscdef.hxx>
#include <rscall.h>

/****************** C L A S S E S ****************************************/
class RscTypCont;

/*********** R s c I n c o n s i s t e n t *******************************/
class RscInconsistent {
public:
    RscId aFirstId;   //Paar von inkonsistenten Id's
    RscId aSecondId;
    RscInconsistent( const RscId & rFirst, const RscId & rSecond ){
            aFirstId = rFirst;
            aSecondId = rSecond;
        };
};

DECLARE_LIST( RscInconsList, RscInconsistent * )

/******************* O b j N o d e ***************************************/
class ObjNode : public IdNode{
    RscId       aRscId; // Id der Resource
    CLASS_DATA  pRscObj;// pointer to a resourceobject
    sal_uLong       lFileKey;// Dateischluessel
protected:
    using NameNode::Search;

public:
    using NameNode::Insert;

                ObjNode( const RscId & rId, CLASS_DATA pData, sal_uLong lKey );
    ObjNode *   DelObjNode( RscTop * pClass, sal_uLong lFileKey );
    sal_uInt32      GetId() const;
    RscId       GetRscId(){ return( aRscId ); }
    sal_uLong       GetFileKey(){ return lFileKey; };
    ObjNode*    Search( const RscId &rName ) const{
                    // search the index in the b-tree
                    return( (ObjNode *)IdNode::Search( rName ) );
                }
    sal_Bool        Insert( ObjNode* pTN ){
                    // insert a new node in the b-tree
                    return( IdNode::Insert( (IdNode *)pTN ) );
                }
    CLASS_DATA  GetRscObj(){
                    // get the Object from this Node
                    return( pRscObj );
                }
    sal_Bool        IsConsistent( RscInconsList * pList = NULL );
};

/******************* R e f N o d e ***************************************/
class RefNode : public IdNode{
    Atom        nTypNameId; // index of a Name in a hashtabel
protected:
    using NameNode::Search;

public:
    using NameNode::Insert;

    ObjNode*    pObjBiTree; // Zeiger auf Objektbaum
                RefNode( Atom nTyp );
    sal_uInt32      GetId() const;
    RefNode*    Search( Atom typ ) const{
                    // search the index in the b-tree
                    return( (RefNode *)IdNode::Search( typ ) );
                };
    sal_Bool        Insert( RefNode* pTN ){
                    // insert a new node in the b-tree
                    return( IdNode::Insert( (IdNode *)pTN ) );
                };
    sal_Bool        PutObjNode( ObjNode * pPutObject );

                // insert new node in b-tree pObjBiTree
    ObjNode *   GetObjNode( const RscId &rRscId );

    ObjNode *   GetObjNode(){
                    // hole  pObjBiTree
                    return( pObjBiTree );
                };
};

#endif // _RSCCLOBJ_HXX
