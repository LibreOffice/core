/*************************************************************************
 *
 *  $RCSfile: feflyole.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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
#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif


#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#include "fesh.hxx"
#include "cntfrm.hxx"
#include "frmfmt.hxx"
#include "flyfrm.hxx"
#include "pam.hxx"
#include "edimp.hxx"
#include "ndtxt.hxx"
#include "notxtfrm.hxx"
#include "ndole.hxx"



SwFlyFrm *SwFEShell::FindFlyFrm( const SvEmbeddedObject *pIPObj ) const
{
    SwFlyFrm *pFly = FindFlyFrm();
    if ( pFly && pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
    {
        SwOLENode *pNd = ((SwNoTxtFrm*)pFly->Lower())->GetNode()->GetOLENode();
        if ( !pNd || &pNd->GetOLEObj().GetOleRef() != pIPObj )
            pFly = 0;
    }
    else
        pFly = 0;

    if ( !pFly )
    {
        //Kein Fly oder der falsche selektiert. Ergo muessen wir leider suchen.
        SwStartNode *pStNd;
        ULONG nSttIdx = GetNodes().GetEndOfAutotext().StartOfSectionIndex() + 1,
              nEndIdx = GetNodes().GetEndOfAutotext().GetIndex();
        while( nSttIdx < nEndIdx &&
                0 != (pStNd = GetNodes()[ nSttIdx ]->GetStartNode()) )
        {
            SwNode *pNd = GetNodes()[ nSttIdx+1 ];
            if ( pNd->IsOLENode() &&
                                        //do not load Objects! must not be neccessary here
                 ((SwOLENode*)pNd)->GetOLEObj().IsOleRef() &&
                 &((SwOLENode*)pNd)->GetOLEObj().GetOleRef() == pIPObj )
            {
                SwFrm *pFrm = ((SwOLENode*)pNd)->GetFrm();
                if ( pFrm )
                    pFly = pFrm->FindFlyFrm();
                break;
            }
            nSttIdx = pStNd->EndOfSectionIndex() + 1;
        }

        if ( !pFly )
        {
            ASSERT( !this, "FlyFrm not found." );
            return 0;
        }
    }
    return pFly;
}


void SwFEShell::GrfToOle( SvInPlaceObject * pObj )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    FOREACHPAM_START( this )

        GetDoc()->GrfToOle( *PCURCRSR, pObj );

    FOREACHPAM_END()
    EndAllAction();
}


void SwFEShell::OleToGrf( const Graphic *pGrf )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    FOREACHPAM_START( this )

        GetDoc()->OleToGrf( *PCURCRSR, pGrf );

    FOREACHPAM_END()
    EndAllAction();
}


String SwFEShell::GetUniqueOLEName() const
{
    return GetDoc()->GetUniqueOLEName();
}


String SwFEShell::GetUniqueFrameName() const
{
    return GetDoc()->GetUniqueFrameName();
}


void SwFEShell::MakeObjVisible( const SvEmbeddedObject *pIPObj ) const
{
    SwFlyFrm *pFly = FindFlyFrm( pIPObj );
    if ( pFly )
    {
        SwRect aTmp( pFly->Prt() );
        aTmp += pFly->Frm().Pos();
        if ( !aTmp.IsOver( VisArea() ) )
        {
            ((SwFEShell*)this)->StartAction();
            ((SwFEShell*)this)->MakeVisible( aTmp );
            ((SwFEShell*)this)->EndAction();
        }
    }
}




