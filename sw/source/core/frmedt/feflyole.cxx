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


#include <sfx2/ipclient.hxx>
#include <sfx2/viewsh.hxx>
#include <osl/diagnose.h>

#include <fesh.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <doc.hxx>
#include <notxtfrm.hxx>
#include <ndole.hxx>
#include <swcli.hxx>
#include <docsh.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <sfx2/linkmgr.hxx>

using namespace com::sun::star;

SwFlyFrame *SwFEShell::FindFlyFrame( const uno::Reference < embed::XEmbeddedObject >& xObj ) const
{
    SwFlyFrame *pFly = GetSelectedFlyFrame();
    if ( pFly && pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
    {
        SwOLENode *pNd = static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetOLENode();
        if ( !pNd || pNd->GetOLEObj().GetOleRef() != xObj )
            pFly = nullptr;
    }
    else
        pFly = nullptr;

    if ( !pFly )
    {
        // No or wrong fly selected: we have to search.
        bool bExist = false;
        SwStartNode *pStNd;
        sal_uLong nSttIdx = GetNodes().GetEndOfAutotext().StartOfSectionIndex() + 1,
              nEndIdx = GetNodes().GetEndOfAutotext().GetIndex();
        while( nSttIdx < nEndIdx &&
                nullptr != (pStNd = GetNodes()[ nSttIdx ]->GetStartNode()) )
        {
            SwNode *pNd = GetNodes()[ nSttIdx+1 ];
            if ( pNd->IsOLENode() &&
                 static_cast<SwOLENode*>(pNd)->GetOLEObj().GetOleRef() == xObj )
            {
                bExist = true;
                SwFrame *pFrame = static_cast<SwOLENode*>(pNd)->getLayoutFrame( GetLayout() );
                if ( pFrame )
                    pFly = pFrame->FindFlyFrame();
                break;
            }
            nSttIdx = pStNd->EndOfSectionIndex() + 1;
        }

        OSL_ENSURE( bExist, "OLE-Object unknown and FlyFrame not found." );
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

OUString SwFEShell::GetUniqueShapeName() const
{
    return GetDoc()->GetUniqueShapeName();
}

bool SwFEShell::FinishOLEObj()                      // Server is terminated
{
    SfxInPlaceClient* pIPClient = GetSfxViewShell()->GetIPClient();
    if ( !pIPClient )
        return false;

    bool bRet = pIPClient->IsObjectInPlaceActive();
    if( bRet )
    {
        if( CNT_OLE == GetCntType() )
            ClearAutomaticContour();

        if( static_cast<SwOleClient*>(pIPClient)->IsCheckForOLEInCaption() !=
            IsCheckForOLEInCaption() )
            SetCheckForOLEInCaption( !IsCheckForOLEInCaption() );

        // enable update of the link preview
        comphelper::EmbeddedObjectContainer& rEmbeddedObjectContainer = GetDoc()->GetDocShell()->getEmbeddedObjectContainer();
        const bool aUserAllowsLinkUpdate = rEmbeddedObjectContainer.getUserAllowsLinkUpdate();
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(true);

        // leave UIActive state
        pIPClient->DeactivateObject();

        // if we have more than one link let's update them too
        sfx2::LinkManager& rLinkManager = GetDoc()->getIDocumentLinksAdministration().GetLinkManager();
        if (rLinkManager.GetLinks().size() > 1)
            rLinkManager.UpdateAllLinks(false, false, nullptr);

        // return back original value of the "update of the link preview" flag
        rEmbeddedObjectContainer.setUserAllowsLinkUpdate(aUserAllowsLinkUpdate);
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
