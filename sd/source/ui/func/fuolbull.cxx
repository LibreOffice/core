/*************************************************************************
 *
 *  $RCSfile: fuolbull.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#include <svx/editdata.hxx>
#include <svx/svxids.hrc>

#include "fuolbull.hxx"
#include "outlview.hxx"
#include "outlnvsh.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "dlgolbul.hxx"
#include "drawdoc.hxx"

TYPEINIT1( FuOutlineBullet, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuOutlineBullet::FuOutlineBullet(SdViewShell* pViewShell, SdWindow* pWindow,
                                 SdView* pView, SdDrawDocument* pDoc,
                                 SfxRequest& rReq)
       : FuPoor(pViewShell, pWindow, pView, pDoc, rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        // ItemSet fuer Dialog fuellen
        SfxItemSet aEditAttr( pDoc->GetPool() );
        pView->GetAttributes( aEditAttr );

        SfxItemSet aNewAttr( pViewShell->GetPool(),
                             EE_ITEMS_START, EE_ITEMS_END );
        aNewAttr.Put( aEditAttr, FALSE );

        // Dialog hochfahren und ausfuehren
        SdOutlineBulletDlg* pDlg = new SdOutlineBulletDlg( NULL, &aNewAttr, pView );

        USHORT nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_OK:
            {
                SfxItemSet aSet( *pDlg->GetOutputItemSet() );

                if( pView->ISA(SdDrawViewShell) && pView->GetMarkList().GetMarkCount() == 0)
                {
                    SfxUInt16Item aBulletState( EE_PARA_BULLETSTATE, 0 );
                    aSet.Put(aBulletState);
                }

                rReq.Done( aSet );
                pArgs = rReq.GetArgs();
            }
            break;

            default:
            {
                delete pDlg;
                return;
            }
        }

        delete pDlg;
    }

    // nicht direkt an pOlView, damit SdDrawView::SetAttributes
    // Aenderungen auf der Masterpage abfangen und in eine
    // Vorlage umleiten kann
    pView->SetAttributes(*pArgs);

    // evtl. Betroffene Felder invalidieren
    pViewShell->Invalidate( FN_NUM_BULLET_ON );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

FuOutlineBullet::~FuOutlineBullet()
{
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuOutlineBullet::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuOutlineBullet::Deactivate()
{
    FuPoor::Deactivate();
}



