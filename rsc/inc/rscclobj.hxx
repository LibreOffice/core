/*************************************************************************
 *
 *  $RCSfile: rscclobj.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscclobj.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.4  1997/08/27 18:18:12  MM
    neue Headerstruktur


      Rev 1.3   27 Aug 1997 18:18:12   MM
   neue Headerstruktur

      Rev 1.2   21 Nov 1995 19:49:10   TLX
   Neuer Link

      Rev 1.1   12 Jan 1994 15:18:10   mm
   Alignementprobleme und Warnings fuer DEC-Alpha beseitigt

      Rev 1.0   10 Aug 1992 07:14:38   MM
   Initial revision.

      Rev 1.2   23 Mar 1992 15:23:42   MM

      Rev 1.1   10 Dec 1991 11:49:10   MM
   ObjNode::DelObjNode

      Rev 1.0   26 Nov 1991 08:57:04   MM
   Initial revision.

**************************************************************************/

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
public:
                ObjNode( const RscId & rId, CLASS_DATA pData, ULONG lKey );
    ObjNode *   DelObjNode( RscTop * pClass, ULONG lFileKey );
    USHORT      GetId() const;
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
    HASHID      nTypNameId; // index of a Name in a hashtabel
public:
    ObjNode*    pObjBiTree; // Zeiger auf Objektbaum
                RefNode( HASHID nTyp );
    USHORT      GetId() const;
    RefNode*    Search( HASHID typ ) const{
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
