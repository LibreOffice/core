/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmdlg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:51:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#include <svx/dialogs.hrc>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
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
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>

/*--------------------------------------------------------------------
    Beschreibung:   Der Traeger des Dialoges
 --------------------------------------------------------------------*/

SwFrmDlg::SwFrmDlg( SfxViewFrame*       pViewFrame,
                    Window*             pParent,
                    const SfxItemSet&   rCoreSet,
                    BOOL                bNewFrm,
                    USHORT              nResType,
                    BOOL                bFormat,
                    UINT16              nDefPage,
                    const String*       pStr) :

    SfxTabDialog(pViewFrame, pParent, SW_RES(nResType), &rCoreSet, pStr != 0),
    m_bFormat(bFormat),
    m_bNew(bNewFrm),
    m_rSet(rCoreSet),
    m_nDlgType(nResType),
    m_pWrtShell(((SwView*)pViewFrame->GetViewShell())->GetWrtShellPtr())
{
    FreeResource();
    USHORT nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    m_bHTMLMode = static_cast< BOOL >(nHtmlMode & HTMLMODE_ON);

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
    if(m_nDlgType == DLG_FRM_GRF)
    {
        AddTabPage( TP_GRF_EXT, SwGrfExtPage::Create, 0 );
        AddTabPage( RID_SVXPAGE_GRFCROP );
    }
    if (m_nDlgType == DLG_FRM_STD)
    {
        AddTabPage(TP_COLUMN,   SwColumnPage::Create,    0);
    }
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage( TP_MACRO_ASSIGN, SfxMacroTabPage::Create, 0);
    AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), 0 );

    if(m_bHTMLMode)
    {
        switch( m_nDlgType )
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
            m_nDlgType != DLG_FRM_STD )
            RemoveTabPage(TP_BACKGROUND);
    }

    if (m_bNew)
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
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch ( nId )
    {
    case TP_FRM_STD:
        ((SwFrmPage&)rPage).SetNewFrame(m_bNew);
        ((SwFrmPage&)rPage).SetFormatUsed(m_bFormat);
        ((SwFrmPage&)rPage).SetFrmType(m_nDlgType);
        break;

    case TP_FRM_ADD:
        ((SwFrmAddPage&)rPage).SetFormatUsed(m_bFormat);
        ((SwFrmAddPage&)rPage).SetFrmType(m_nDlgType);
        ((SwFrmAddPage&)rPage).SetNewFrame(m_bNew);
        ((SwFrmAddPage&)rPage).SetShell(m_pWrtShell);
        break;

    case TP_FRM_WRAP:
        ((SwWrapTabPage&)rPage).SetNewFrame(m_bNew);
        ((SwWrapTabPage&)rPage).SetFormatUsed(m_bFormat, FALSE);
        ((SwWrapTabPage&)rPage).SetShell(m_pWrtShell);
        break;

    case TP_COLUMN:
        {
            ((SwColumnPage&)rPage).SetFrmMode(TRUE);
            ((SwColumnPage&)rPage).SetFormatUsed(m_bFormat);

            const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)
                                                m_rSet.Get( RES_FRM_SIZE );
            ((SwColumnPage&)rPage).SetPageWidth( rSize.GetWidth() );
        }
        break;

    case TP_MACRO_ASSIGN:
        SwMacroAssignDlg::AddEvents( (SfxMacroTabPage&)rPage,
            DLG_FRM_GRF == m_nDlgType ? MACASSGN_GRAPHIC
                            : DLG_FRM_OLE == m_nDlgType ? MACASSGN_OLE
                                                      : MACASSGN_FRMURL );
        break;

    case TP_BACKGROUND:
        if( DLG_FRM_STD == m_nDlgType )
        {
            sal_Int32 nFlagType = SVX_SHOW_SELECTOR;
            if(!m_bHTMLMode)
                nFlagType |= SVX_ENABLE_TRANSPARENCY;
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
            rPage.PageCreated(aSet);
        }
        break;

    case TP_BORDER:
        {
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            rPage.PageCreated(aSet);
        }
        break;
    }
}
