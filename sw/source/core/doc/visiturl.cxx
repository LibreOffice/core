/*************************************************************************
 *
 *  $RCSfile: visiturl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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


#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _INETHIST_HXX //autogen
#include <svtools/inethist.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VISITURL_HXX
#include <visiturl.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif


SwURLStateChanged::SwURLStateChanged( const SwDoc* pD )
    : pDoc( pD )
{
    StartListening( *INetURLHistory::GetOrCreate() );
}

SwURLStateChanged::~SwURLStateChanged()
{
    EndListening( *INetURLHistory::GetOrCreate() );
}

void SwURLStateChanged::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if( rHint.ISA( INetURLHistoryHint ) && pDoc->GetRootFrm() )
    {
        // diese URL wurde veraendert:
        const INetURLObject* pIURL = ((INetURLHistoryHint&)rHint).GetObject();
        String sURL( pIURL->GetMainURL() ), sBkmk;

        SwEditShell* pESh = pDoc->GetEditShell();

        if( pDoc->GetDocShell() && pDoc->GetDocShell()->GetMedium() &&
            // falls das unser Doc ist, kann es auch lokale Spruenge geben!
            sURL == pDoc->GetDocShell()->GetMedium()->GetName() )
            (sBkmk = pIURL->GetMark()).Insert( INET_MARK_TOKEN, 0 );

        BOOL bAction = FALSE, bUnLockView = FALSE;
        const SwFmtINetFmt* pItem;
        const SwTxtINetFmt* pTxtAttr;
        const SwTxtNode* pTxtNd;
        USHORT n, nMaxItems = pDoc->GetAttrPool().GetItemCount( RES_TXTATR_INETFMT );
        for( n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = (SwFmtINetFmt*)pDoc->GetAttrPool().GetItem(
                RES_TXTATR_INETFMT, n ) ) &&
                ( pItem->GetValue() == sURL ||
                    ( sBkmk.Len() && pItem->GetValue() == sBkmk )) &&
                0 != ( pTxtAttr = pItem->GetTxtINetFmt()) &&
                0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) )
            {
                if( !bAction && pESh )
                {
                    pESh->StartAllAction();
                    bAction = TRUE;
                    bUnLockView = !pESh->IsViewLocked();
                    pESh->LockView( TRUE );
                }
                ((SwTxtINetFmt*)pTxtAttr)->SetValidVis( FALSE );
                const SwTxtAttr* pAttr = pTxtAttr;
                SwUpdateAttr aUpdateAttr( *pAttr->GetStart(),
                                          *pAttr->GetEnd(),
                                          RES_FMT_CHG );
                ((SwTxtNode*)pTxtNd)->SwCntntNode::Modify( &aUpdateAttr,
                                                            &aUpdateAttr );
            }

        if( bAction )
            pESh->EndAllAction();
         if( bUnLockView )
             pESh->LockView( FALSE );
    }
}

    // erfrage ob die URL besucht war. Uebers Doc, falls nur ein Bookmark
    // angegeben ist. Dann muss der Doc. Name davor gesetzt werden!
BOOL SwDoc::IsVisitedURL( const String& rURL ) const
{
#ifdef DEBUG
    static long nTmp = 0;
    ++nTmp;
#endif

    BOOL bRet = FALSE;
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



