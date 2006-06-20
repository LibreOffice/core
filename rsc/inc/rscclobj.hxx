/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscclobj.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:44:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _RSCCLOBJ_HXX
#define _RSCCLOBJ_HXX

#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif
#ifndef _RSCALL_H
#include <rscall.h>
#endif

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
    ULONG       lFileKey;// Dateischluessel
protected:
    using NameNode::Search;

public:
    using NameNode::Insert;

                ObjNode( const RscId & rId, CLASS_DATA pData, ULONG lKey );
    ObjNode *   DelObjNode( RscTop * pClass, ULONG lFileKey );
    sal_uInt32      GetId() const;
    RscId       GetRscId(){ return( aRscId ); }
    ULONG       GetFileKey(){ return lFileKey; };
    ObjNode*    Search( const RscId &rName ) const{
                    // search the index in the b-tree
                    return( (ObjNode *)IdNode::Search( rName ) );
                }
    BOOL        Insert( ObjNode* pTN ){
                    // insert a new node in the b-tree
                    return( IdNode::Insert( (IdNode *)pTN ) );
                }
    CLASS_DATA  GetRscObj(){
                    // get the Object from this Node
                    return( pRscObj );
                }
    BOOL        IsConsistent( RscInconsList * pList = NULL );
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
    BOOL        Insert( RefNode* pTN ){
                    // insert a new node in the b-tree
                    return( IdNode::Insert( (IdNode *)pTN ) );
                };
    BOOL        PutObjNode( ObjNode * pPutObject );

                // insert new node in b-tree pObjBiTree
    ObjNode *   GetObjNode( const RscId &rRscId );

    ObjNode *   GetObjNode(){
                    // hole  pObjBiTree
                    return( pObjBiTree );
                };
};

#endif // _RSCCLOBJ_HXX
