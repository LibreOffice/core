/*************************************************************************
 *
 *  $RCSfile: futxtatt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:49:21 $
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

#include "futxtatt.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

//CHINA001 #include <svx/textattr.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "drawdoc.hxx"

namespace sd {

TYPEINIT1( FuTextAttrDlg, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuTextAttrDlg::FuTextAttrDlg (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    SfxItemSet aNewAttr( pDoc->GetPool() );
    pView->GetAttributes( aNewAttr );

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        SfxAbstractTabDialog *pDlg = pFact->CreateTextTabDialog( NULL, &aNewAttr, ResId( RID_SVXDLG_TEXT ), pView );

        USHORT nResult = pDlg->Execute();

        switch( nResult )
        {
            case RET_OK:
            {
                rReq.Done( *( pDlg->GetOutputItemSet() ) );

                pArgs = rReq.GetArgs();
            }
            break;

            default:
            {
                delete( pDlg );
            }
            return; // Abbruch
        }
        delete( pDlg );
    }
    pView->SetAttributes( *pArgs );
}


} // end of namespace sd
