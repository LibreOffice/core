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
#include <IDocumentLayoutAccess.hxx>
#include <visiturl.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <editsh.hxx>
#include <docsh.hxx>

SwURLStateChanged::SwURLStateChanged( SwDoc* pD )
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
    if( dynamic_cast<const INetURLHistoryHint*>(&rHint) && pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        // This URL has been changed:
        const INetURLObject* pIURL = static_cast<const INetURLHistoryHint&>(rHint).GetObject();
        OUString sURL( pIURL->GetMainURL( INetURLObject::NO_DECODE ) ), sBkmk;

        SwEditShell* pESh = pDoc->GetEditShell();

        if( pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() &&
            // If this is our Doc, we can also have local jumps!
            pDoc->GetDocShell()->GetMedium()->GetName().equals(sURL) )
            sBkmk = "#" + pIURL->GetMark();

        bool bAction = false, bUnLockView = false;
        sal_uInt32 nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        {
            const SwFormatINetFormat* pItem = static_cast<const SwFormatINetFormat*>(pDoc->GetAttrPool().GetItem2(RES_TXTATR_INETFMT, n ));
            if( pItem != nullptr &&
                ( pItem->GetValue() == sURL || ( !sBkmk.isEmpty() && pItem->GetValue() == sBkmk )))
            {
                const SwTextINetFormat* pTextAttr = pItem->GetTextINetFormat();
                if (pTextAttr != nullptr)
                {
                    const SwTextNode* pTextNd = pTextAttr->GetpTextNode();
                    if (pTextNd != nullptr)
                    {
                        if( !bAction && pESh )
                        {
                            pESh->StartAllAction();
                            bAction = true;
                            bUnLockView = !pESh->IsViewLocked();
                            pESh->LockView( true );
                        }
                        const_cast<SwTextINetFormat*>(pTextAttr)->SetVisitedValid(false);
                        const SwTextAttr* pAttr = pTextAttr;
                        SwUpdateAttr aUpdateAttr(
                            pAttr->GetStart(),
                            *pAttr->End(),
                            RES_FMT_CHG);

                        const_cast< SwTextNode* >(pTextNd)->ModifyNotification(&aUpdateAttr, &aUpdateAttr);
                    }
                }
            }
        }

        if( bAction )
            pESh->EndAllAction();
        if( bUnLockView )
            pESh->LockView( false );
    }
}

// Check if the URL has been visited before. Via the Doc, if only one Bookmark is set
// We need to put the Doc's name before it!
bool SwDoc::IsVisitedURL( const OUString& rURL )
{
    bool bRet = false;
    if( !rURL.isEmpty() )
    {
        INetURLHistory *pHist = INetURLHistory::GetOrCreate();
        if( '#' == rURL[0] && mpDocShell && mpDocShell->GetMedium() )
        {
            INetURLObject aIObj( mpDocShell->GetMedium()->GetURLObject() );
            aIObj.SetMark( rURL.copy( 1 ) );
            bRet = pHist->QueryUrl( aIObj );
        }
        else
            bRet = pHist->QueryUrl( rURL );

        // We also want to be informed about status updates in the History
        if( !mpURLStateChgd )
        {
            SwDoc* pD = this;
            pD->mpURLStateChgd = new SwURLStateChanged( this );
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
