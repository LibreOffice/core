/*************************************************************************
 *
 *  $RCSfile: frmdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:30:20 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_BORDER_HXX
#include <svx/border.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_GRFPAGE_HXX //autogen
#include <svx/grfpage.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif


#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _FRMDLG_HXX
#include <frmdlg.hxx>
#endif
#ifndef _FRMPAGE_HXX
#include <frmpage.hxx>
#endif
#ifndef _WRAP_HXX
#include <wrap.hxx>
#endif
#ifndef _COLUMN_HXX
#include <column.hxx>
#endif
#ifndef _MACASSGN_HXX
#include <macassgn.hxx>
#endif

#ifndef _FRMUI_HRC
#include <frmui.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/

SwFrmDlg::SwFrmDlg( SfxViewFrame*       pFrame,
                    Window*             pParent,
                    const SfxItemSet&   rCoreSet,
                    BOOL                bNewFrm,
                    USHORT              nResType,
                    BOOL                bFmt,
                    UINT16              nDefPage,
                    const String*       pStr) :

    SfxTabDialog(pFrame, pParent, SW_RES(nResType), &rCoreSet, pStr != 0),
    bNew(bNewFrm),
    bFormat(bFmt),
    rSet(rCoreSet),
    nDlgType(nResType),
    pWrtShell(((SwView*)pFrame->GetViewShell())->GetWrtShellPtr())
{
    FreeResource();
    USHORT nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
    bHTMLMode = nHtmlMode & HTMLMODE_ON;

    // BspFont fuer beide Bsp-TabPages
    //
    if(pStr)
    {
        String aTmp( GetText() );
        aTmp += SW_RESSTR(STR_COLL_HEADER);
        aTmp += *pStr;
        aTmp += ')';
    }
    AddTabPage(TP_FRM_STD,  SwFrmPage::Create, 0);
    AddTabPage(TP_FRM_ADD,  SwFrmAddPage::Create, 0);
    AddTabPage(TP_FRM_WRAP, SwWrapTabPage::Create, 0);
    AddTabPage(TP_FRM_URL,  SwFrmURLPage::Create, 0);
    if(nDlgType == DLG_FRM_GRF)
    {
        AddTabPage( TP_GRF_EXT, SwGrfExtPage::Create, 0 );
        AddTabPage( RID_SVXPAGE_GRFCROP, SvxGrfCropPage::Create, 0 );
    }
    if (nDlgType == DLG_FRM_STD)
    {
        AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    }
    AddTabPage(TP_BACKGROUND,SvxBackgroundTabPage::Create,  0);
    AddTabPage( TP_MACRO_ASSIGN, SfxMacroTabPage::Create, 0);
    AddTabPage( TP_BORDER,   SvxBorderTabPage::Create,      0);

    if(bHTMLMode)
    {
        switch( nDlgType )
        {
        case DLG_FRM_STD:
                if(0  == (nHtmlMode & HTMLMODE_SOME_ABS_POS))
                    RemoveTabPage(TP_BORDER);
                RemoveTabPage(TP_COLUMN);
            // kein break
        case DLG_FRM_OLE:
                RemoveTabPage(TP_FRM_URL);
                RemoveTabPage(TP_MACRO_ASSIGN);
            break;
        case DLG_FRM_GRF:
                RemoveTabPage(RID_SVXPAGE_GRFCROP);
            break;
        }
        if( 0  == (nHtmlMode & HTMLMODE_SOME_ABS_POS) ||
            nDlgType != DLG_FRM_STD )
            RemoveTabPage(TP_BACKGROUND);
    }

    if (bNew)
        SetCurPageId(TP_FRM_STD);

    if (nDefPage)
        SetCurPageId(nDefPage);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFrmDlg::~SwFrmDlg()
{
}




void SwFrmDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch ( nId )
    {
    case TP_FRM_STD:
        ((SwFrmPage&)rPage).SetNewFrame(bNew);
        ((SwFrmPage&)rPage).SetFormatUsed(bFormat);
        ((SwFrmPage&)rPage).SetFrmType(nDlgType);
        break;

    case TP_FRM_ADD:
        ((SwFrmAddPage&)rPage).SetFormatUsed(bFormat);
        ((SwFrmAddPage&)rPage).SetFrmType(nDlgType);
        ((SwFrmAddPage&)rPage).SetNewFrame(bNew);
        ((SwFrmAddPage&)rPage).SetShell(pWrtShell);
        break;

    case TP_FRM_WRAP:
        ((SwWrapTabPage&)rPage).SetNewFrame(bNew);
        ((SwWrapTabPage&)rPage).SetFormatUsed(bFormat, FALSE);
        ((SwWrapTabPage&)rPage).SetShell(pWrtShell);
        break;

    case TP_COLUMN:
        {
            ((SwColumnPage&)rPage).SetFrmMode(TRUE);
            ((SwColumnPage&)rPage).SetFormatUsed(bFormat);

            const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)
                                                rSet.Get( RES_FRM_SIZE );
            ((SwColumnPage&)rPage).SetPageWidth( rSize.GetWidth() );
        }
        break;

    case TP_MACRO_ASSIGN:
        SwMacroAssignDlg::AddEvents( (SfxMacroTabPage&)rPage,
            DLG_FRM_GRF == nDlgType ? MACASSGN_GRAPHIC
                            : DLG_FRM_OLE == nDlgType ? MACASSGN_OLE
                                                      : MACASSGN_FRMURL );
        break;

    case TP_BACKGROUND:
        if( DLG_FRM_STD == nDlgType )
        {
            ((SvxBackgroundTabPage&)rPage).ShowSelector();
            if(!bHTMLMode)
                ((SvxBackgroundTabPage&)rPage).EnableTransparency(TRUE, TRUE);
        }
        break;

    case TP_BORDER:
        ((SvxBorderTabPage&) rPage).SetSWMode(SW_BORDER_MODE_FRAME);
        break;
    }
}
