/*************************************************************************
 *
 *  $RCSfile: fucushow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:11:57 $
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

#include <svx/svxids.hrc>

#pragma hdrstop

#include "fucushow.hxx"
#include "custsdlg.hxx"

#include "app.hrc"
#include "sdresid.hxx"

#include "viewshel.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

TYPEINIT1( FuCustomShowDlg, FuPoor );


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuCustomShowDlg::FuCustomShowDlg( SdViewShell* pViewSh, SdWindow*    pWin,
                                SdView* pView, SdDrawDocument* pDoc, SfxRequest& rReq) :
            FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
    USHORT nRet = RET_YES;

    SdCustomShowDlg  aDlg( NULL, *pDoc );

    nRet = aDlg.Execute();
    if( nRet != RET_CANCEL )
    {
        // wenn sich etwas geaendert hat, setzen wir das Modified-Flag,
        if( aDlg.IsModified() )
        {
            pDoc->SetChanged( TRUE );
            pDoc->SetCustomShow( aDlg.IsCustomShow() );
        }

        if( nRet == RET_YES )
        {
            pViewSh->SetStartShowWithDialog();

            pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
        }
    }
}
