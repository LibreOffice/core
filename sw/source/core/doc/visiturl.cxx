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

#include <sfx2/docfile.hxx>
#include <svl/inethist.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <doc.hxx>
#include <visiturl.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <editsh.hxx>
#include <docsh.hxx>


SwURLStateChanged::SwURLStateChanged( const SwDoc* pD )
    : pDoc( pD )
{
    StartListening( *INetURLHistory::GetOrCreate() );
}

SwURLStateChanged::~SwURLStateChanged()
{
    EndListening( *INetURLHistory::GetOrCreate() );
}

void SwURLStateChanged::Notify( SfxBroadcaster& , const SfxHint& rHint )
{
    if( rHint.ISA( INetURLHistoryHint ) && pDoc->GetCurrentViewShell() )    //swmod 071108//swmod 071225
    {
        // This URL has been changed:
        const INetURLObject* pIURL = ((INetURLHistoryHint&)rHint).GetObject();
        String sURL( pIURL->GetMainURL( INetURLObject::NO_DECODE ) ), sBkmk;

        SwEditShell* pESh = pDoc->GetEditShell();

        if( pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() &&
            // If this is our Doc, we can also have local jumps!
            pDoc->GetDocShell()->GetMedium()->GetName().equals(sURL) )
            (sBkmk = pIURL->GetMark()).Insert( INET_MARK_TOKEN, 0 );

        bool bAction = false, bUnLockView = false;
        sal_uInt32 nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        {
            const SwFmtINetFmt* pItem = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem2(RES_TXTATR_INETFMT, n );
            if( pItem != 0 &&
                ( pItem->GetValue() == sURL || ( sBkmk.Len() && pItem->GetValue() == sBkmk )))
            {
                const SwTxtINetFmt* pTxtAttr = pItem->GetTxtINetFmt();
                if (pTxtAttr != 0)
                {
                    const SwTxtNode* pTxtNd = pTxtAttr->GetpTxtNode();
                    if (pTxtNd != 0)
                    {
                        if( !bAction && pESh )
                        {
                            pESh->StartAllAction();
                            bAction = true;
                            bUnLockView = !pESh->IsViewLocked();
                            pESh->LockView( sal_True );
                        }
                        const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisitedValid( false );
                        const SwTxtAttr* pAttr = pTxtAttr;
                        SwUpdateAttr aUpdateAttr( *pAttr->GetStart(),
                                                  *pAttr->GetEnd(),
                                                  RES_FMT_CHG );
                        ((SwTxtNode*)pTxtNd)->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
                    }
                }
            }
        }

        if( bAction )
            pESh->EndAllAction();
        if( bUnLockView )
            pESh->LockView( sal_False );
    }
}

// Check if the URL has been visited before. Via the Doc, if only one Bookmark is set
// We need to put the Doc's name before it!
bool SwDoc::IsVisitedURL( const String& rURL ) const
{
    bool bRet = false;
    if( rURL.Len() )
    {
        INetURLHistory *pHist = INetURLHistory::GetOrCreate();
        if( '#' == rURL.GetChar( 0 ) && mpDocShell && mpDocShell->GetMedium() )
        {
            INetURLObject aIObj( mpDocShell->GetMedium()->GetURLObject() );
            aIObj.SetMark( rURL.Copy( 1 ) );
            bRet = pHist->QueryUrl( aIObj );
        }
        else
            bRet = pHist->QueryUrl( rURL );

        // We also want to be informed about status updates in the History
        if( !mpURLStateChgd )
        {
            SwDoc* pD = (SwDoc*)this;
            pD->mpURLStateChgd = new SwURLStateChanged( this );
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
