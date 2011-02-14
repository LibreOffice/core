/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
