/*************************************************************************
 *
 *  $RCSfile: rscclobj.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscclobj.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.9  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.8  2000/07/26 17:13:22  willem.vandorp
    Headers/footers replaced

    Revision 1.7  1998/01/08 13:20:44  MM
    includes angepasst


      Rev 1.6   08 Jan 1998 13:20:44   MM
   includes angepasst

      Rev 1.5   12 Sep 1996 12:44:06   MM
   Eigene Ressourcen definieren

      Rev 1.4   21 Nov 1995 19:50:20   TLX
   Neuer Link

      Rev 1.3   03 Mar 1994 12:03:28   mm
   C40 Warnings beseitigt

      Rev 1.2   12 Jan 1994 15:20:18   mm
   Alignementprobleme und Warnings fuer DEC-Alpha beseitigt

      Rev 1.1   25 Aug 1993 15:29:00   mm
   Fehler und Warnings beseitigt

      Rev 1.0   10 Aug 1992 07:20:50   MM
   Initial revision.

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.

// Solar Definitionen
#include <tools/solar.h>

// Programmabhaengige Includes.
#ifndef _RSCCLOBJ_HXX
#include <rscclobj.hxx>
#endif
#ifndef _RSCTOP_HXX
#include <rsctop.hxx>
#endif

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
RefNode::RefNode( HASHID nTyp ){
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
USHORT RefNode::GetId() const
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
BOOL RefNode::PutObjNode( ObjNode * pPutObject ){
// insert a node in the b-tree pObjBiTree
// if the node with the same name is in pObjBiTree,
// return FALSE and no insert,

    if( pObjBiTree )
        return( pObjBiTree->Insert( pPutObject ) );

    pObjBiTree = pPutObject;
    return( TRUE );
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
ObjNode::ObjNode( const RscId & rId, CLASS_DATA pData, ULONG lKey ){
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
ObjNode * ObjNode::DelObjNode( RscTop * pClass, ULONG nFileKey ){
    ObjNode * pRetNode = this;

    if( Right() )
        pRight = ((ObjNode *)Right())->DelObjNode( pClass, nFileKey );
    if( Left() )
        pLeft = ((ObjNode *)Left())->DelObjNode( pClass, nFileKey );

    if( GetFileKey() == nFileKey ){
        if( GetRscObj() ){
            pClass->Destroy( RSCINST( pClass, GetRscObj() ) );
            RscMem::Free( GetRscObj() );
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
USHORT ObjNode::GetId() const
{
    return( (USHORT)(long)aRscId );
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
BOOL ObjNode::IsConsistent( RscInconsList * pList )
{
    BOOL bRet = TRUE;

    if( (long)aRscId > 0x7FFF || (long)aRscId < 1 )
    {
        bRet = FALSE;
        if( pList )
            pList->Insert( new RscInconsistent( aRscId, aRscId ) );
    }
    else
    {
        if( Left() )
        {
            if( !((ObjNode *)Left())->IsConsistent( pList ) )
                bRet = FALSE;
            if( ((ObjNode *)Left())->aRscId >= aRscId )
            {
                bRet = FALSE;
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
                bRet = FALSE;
                if( pList )
                    pList->Insert(
                        new RscInconsistent( GetRscId(),
                                             ((ObjNode *)Right())->GetRscId() ) );
            }
            if( !((ObjNode *)Right())->IsConsistent( pList ) )
                bRet = FALSE;
        };
    };

    return( bRet );
}
