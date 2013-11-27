/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
    if( dynamic_cast< const INetURLHistoryHint* >(&rHint) && pDoc->GetCurrentViewShell() )  //swmod 071108//swmod 071225
    {
        // diese URL wurde veraendert:
        const INetURLObject* pIURL = ((INetURLHistoryHint&)rHint).GetObject();
        String sURL( pIURL->GetMainURL( INetURLObject::NO_DECODE ) ), sBkmk;

        SwEditShell* pESh = pDoc->GetEditShell();

        if( pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() &&
            // falls das unser Doc ist, kann es auch lokale Spruenge geben!
            sURL == pDoc->GetDocShell()->GetMedium()->GetName() )
            (sBkmk = pIURL->GetMark()).Insert( INET_MARK_TOKEN, 0 );

        sal_Bool bAction = sal_False, bUnLockView = sal_False;
        const SwFmtINetFmt* pItem;
        const SwTxtINetFmt* pTxtAttr;
        const SwTxtNode* pTxtNd;
        sal_uInt32 n, nMaxItems = pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem2(
                RES_TXTATR_INETFMT, n ) ) &&
                ( pItem->GetValue() == sURL ||
                    ( sBkmk.Len() && pItem->GetValue() == sBkmk )) &&
                0 != ( pTxtAttr = pItem->GetTxtINetFmt()) &&
                0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) )
            {
                if( !bAction && pESh )
                {
                    pESh->StartAllAction();
                    bAction = sal_True;
                    bUnLockView = !pESh->IsViewLocked();
                    pESh->LockView( sal_True );
                }
                const_cast<SwTxtINetFmt*>(pTxtAttr)->SetVisitedValid( false );
                const SwTxtAttr* pAttr = pTxtAttr;
                SwUpdateAttr aUpdateAttr( *pAttr->GetStart(),
                                          *pAttr->End(),
                                          RES_FMT_CHG );
                ((SwTxtNode*)pTxtNd)->ModifyNotification( &aUpdateAttr, &aUpdateAttr );
            }

        if( bAction )
            pESh->EndAllAction();
         if( bUnLockView )
             pESh->LockView( sal_False );
    }
}

    // erfrage ob die URL besucht war. Uebers Doc, falls nur ein Bookmark
    // angegeben ist. Dann muss der Doc. Name davor gesetzt werden!
sal_Bool SwDoc::IsVisitedURL( const String& rURL ) const
{
#if OSL_DEBUG_LEVEL > 1
    static long nTmp = 0;
    ++nTmp;
#endif

    sal_Bool bRet = sal_False;
    if( rURL.Len() )
    {
        INetURLHistory *pHist = INetURLHistory::GetOrCreate();
        if( '#' == rURL.GetChar( 0 ) && pDocShell && pDocShell->GetMedium() )
        {
            INetURLObject aIObj( pDocShell->GetMedium()->GetURLObject() );
            aIObj.SetMark( rURL.Copy( 1 ) );
            bRet = pHist->QueryUrl( aIObj );
        }
        else
            bRet = pHist->QueryUrl( rURL );

        // dann  wollen wird auch ueber Statusaenderungen in der History
        // informiert werden!
        if( !pURLStateChgd )
        {
            SwDoc* pD = (SwDoc*)this;
            pD->pURLStateChgd = new SwURLStateChanged( this );
        }
    }
    return bRet;
}



