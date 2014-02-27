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

#include <com/sun/star/embed/EmbedStates.hpp>

#include <sfx2/ipclient.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/app.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/viewfrm.hxx>

#include <sot/exchange.hxx>
#include <fmtcntnt.hxx>
#include <fmtanchr.hxx>
#include <fesh.hxx>
#include <cntfrm.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <notxtfrm.hxx>
#include <ndole.hxx>
#include <swcli.hxx>

using namespace com::sun::star;

SwFlyFrm *SwFEShell::FindFlyFrm( const uno::Reference < embed::XEmbeddedObject >& xObj ) const
{
    SwFlyFrm *pFly = FindFlyFrm();
    if ( pFly && pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
    {
        SwOLENode *pNd = ((SwNoTxtFrm*)pFly->Lower())->GetNode()->GetOLENode();
        if ( !pNd || pNd->GetOLEObj().GetOleRef() != xObj )
            pFly = 0;
    }
    else
        pFly = 0;

    if ( !pFly )
    {
        // No or wrong fly selected: we have to search.
        bool bExist = false;
        SwStartNode *pStNd;
        sal_uLong nSttIdx = GetNodes().GetEndOfAutotext().StartOfSectionIndex() + 1,
              nEndIdx = GetNodes().GetEndOfAutotext().GetIndex();
        while( nSttIdx < nEndIdx &&
                0 != (pStNd = GetNodes()[ nSttIdx ]->GetStartNode()) )
        {
            SwNode *pNd = GetNodes()[ nSttIdx+1 ];
            if ( pNd->IsOLENode() &&
                 ((SwOLENode*)pNd)->GetOLEObj().GetOleRef() == xObj )
            {
                bExist = true;
                SwFrm *pFrm = ((SwOLENode*)pNd)->getLayoutFrm( GetLayout() );
                if ( pFrm )
                    pFly = pFrm->FindFlyFrm();
                break;
            }
            nSttIdx = pStNd->EndOfSectionIndex() + 1;
        }

        OSL_ENSURE( bExist, "OLE-Object unknown and FlyFrm not found." );
        (void)bExist;
    }
    return pFly;
}

OUString SwFEShell::GetUniqueOLEName() const
{
    return GetDoc()->GetUniqueOLEName();
}

OUString SwFEShell::GetUniqueFrameName() const
{
    return GetDoc()->GetUniqueFrameName();
}

void SwFEShell::MakeObjVisible( const uno::Reference < embed::XEmbeddedObject >& xObj ) const
{
    SwFlyFrm *pFly = FindFlyFrm( xObj );
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

sal_Bool SwFEShell::FinishOLEObj()                      // Server is terminated
{
    SfxInPlaceClient* pIPClient = GetSfxViewShell()->GetIPClient();
    if ( !pIPClient )
        return sal_False;

    sal_Bool bRet = pIPClient->IsObjectInPlaceActive();
    if( bRet )
    {
        uno::Reference < embed::XEmbeddedObject > xObj = pIPClient->GetObject();
        if( CNT_OLE == GetCntType() )
            ClearAutomaticContour();

        if( ((SwOleClient*)pIPClient)->IsCheckForOLEInCaption() !=
            IsCheckForOLEInCaption() )
            SetCheckForOLEInCaption( !IsCheckForOLEInCaption() );

        // leave UIActive state
        pIPClient->DeactivateObject();
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
