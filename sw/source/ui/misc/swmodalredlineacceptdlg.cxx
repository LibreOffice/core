/*************************************************************************
 *
 *  $RCSfile: swmodalredlineacceptdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:34:25 $
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

#define _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _DATETIME_HXX //autogen
#include <tools/datetime.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVSTDARR_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_CTREDLIN_HXX //autogen
#include <svx/ctredlin.hxx>
#endif
#ifndef _SVX_POSTATTR_HXX //autogen
#include <svx/postattr.hxx>
#endif
//CHINA001 #ifndef _SVX_POSTDLG_HXX //autogen
//CHINA001 #include <svx/postdlg.hxx>
//CHINA001 #endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif
#ifndef _REDLNDLG_HRC
#include <redlndlg.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif

#include <vector>
#ifndef _REDLNDLG_HXX
#define _REDLNACCEPTDLG
#include <redlndlg.hxx>
#endif
#include "swmodalredlineacceptdlg.hxx"

#define C2S(cChar) UniString::CreateFromAscii(cChar)
SwModalRedlineAcceptDlg::SwModalRedlineAcceptDlg(Window *pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MOD_REDLINE_ACCEPT))
{
    pImplDlg = new SwRedlineAcceptDlg(this, TRUE);

    pImplDlg->Initialize(GetExtraData());
    pImplDlg->Activate();   // Zur Initialisierung der Daten

    FreeResource();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

SwModalRedlineAcceptDlg::~SwModalRedlineAcceptDlg()
{
    AcceptAll(FALSE);   // Alles uebriggebliebene ablehnen
    pImplDlg->FillInfo(GetExtraData());

    delete pImplDlg;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Activate()
{
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModalDialog::Resize();
}

/*------------------------------------------------------------------------
    Beschreibung:
------------------------------------------------------------------------*/

void SwModalRedlineAcceptDlg::AcceptAll( BOOL bAccept )
{
    SvxTPFilter* pFilterTP = pImplDlg->GetChgCtrl()->GetFilterPage();

    if (pFilterTP->IsDate() || pFilterTP->IsAuthor() ||
        pFilterTP->IsRange() || pFilterTP->IsAction())
    {
        pFilterTP->CheckDate(FALSE);    // Alle Filter abschalten
        pFilterTP->CheckAuthor(FALSE);
        pFilterTP->CheckRange(FALSE);
        pFilterTP->CheckAction(FALSE);
        pImplDlg->FilterChangedHdl();
    }

    pImplDlg->CallAcceptReject( FALSE, bAccept );
}
