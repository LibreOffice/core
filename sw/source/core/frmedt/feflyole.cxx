/*************************************************************************
 *
 *  $RCSfile: feflyole.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:44:22 $
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

#pragma hdrstop

#ifndef _IPOBJ_HXX
#include <so3/ipobj.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <so3/embobj.hxx>
#endif
#ifndef _SFX_CLIENTSH_HXX
#include <sfx2/clientsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SCH_DLL_HXX
#include <sch/schdll.hxx>
#endif
#ifndef _SCH_MEMCHRT_HXX
#include <sch/memchrt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#include <sot/exchange.hxx>

#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SWCLI_HXX
#include <swcli.hxx>
#endif

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
        BOOL bExist = FALSE;
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
                bExist = TRUE;
                SwFrm *pFrm = ((SwOLENode*)pNd)->GetFrm();
                if ( pFrm )
                    pFly = pFrm->FindFlyFrm();
                break;
            }
            nSttIdx = pStNd->EndOfSectionIndex() + 1;
        }

        ASSERT( bExist, "OLE-Object unknown and FlyFrm not found." );
    }
    return pFly;
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

BOOL SwFEShell::FinishOLEObj()                      // Server wird beendet
{
    SfxInPlaceClient* pIPClient = GetSfxViewShell()->GetIPClient();
    BOOL bRet = pIPClient && pIPClient->IsInPlaceActive();
    if( bRet )
    {
        if( CNT_OLE == GetCntType() )
            ClearAutomaticContour();

        //  Link fuer Daten-Highlighting im Chart zuruecksetzen
        SvtModuleOptions aMOpt;
        if( aMOpt.IsChart() )
        {
            SvInPlaceObject* pObj = pIPClient->GetIPObj();
            SvGlobalName aObjClsId( *pObj->GetSvFactory() );
            SchMemChart* pMemChart;
            if( SotExchange::IsChart( aObjClsId ) &&
                0 != (pMemChart = SchDLL::GetChartData( pObj ) ))
            {
                pMemChart->SetSelectionHdl( Link() );

//ggfs. auch die Selektion restaurieren
                LockView( TRUE );   //Scrollen im EndAction verhindern
                ClearMark();
                LockView( FALSE );
            }
        }

        if( ((SwOleClient*)pIPClient)->IsCheckForOLEInCaption() !=
            IsCheckForOLEInCaption() )
            SetCheckForOLEInCaption( !IsCheckForOLEInCaption() );

        //InPlace beenden.
        pIPClient->GetProtocol().Reset2Open();
        SFX_APP()->SetViewFrame( GetSfxViewShell()->GetViewFrame() );

    }
    return bRet;
}




