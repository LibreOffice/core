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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#include <svx/dialogs.hrc>
#include <hintids.hxx>
#include <tools/list.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/htmlmode.hxx>
#include <fmtfsize.hxx>
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <docsh.hxx>
#include <viewopt.hxx>
#include <frmdlg.hxx>
#include <frmpage.hxx>
#include <wrap.hxx>
#include <column.hxx>
#include <macassgn.hxx>

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
                    sal_Bool                bNewFrm,
                    sal_uInt16              nResType,
                    sal_Bool                bFormat,
                    sal_uInt16              nDefPage,
                    const String*       pStr) :

    SfxTabDialog(pViewFrame, pParent, SW_RES(nResType), &rCoreSet, pStr != 0),
    m_bFormat(bFormat),
    m_bNew(bNewFrm),
    m_rSet(rCoreSet),
    m_nDlgType(nResType),
    m_pWrtShell(((SwView*)pViewFrame->GetViewShell())->GetWrtShellPtr())
{
    FreeResource();
    sal_uInt16 nHtmlMode = ::GetHtmlMode(m_pWrtShell->GetView().GetDocShell());
    m_bHTMLMode = static_cast< sal_Bool >(nHtmlMode & HTMLMODE_ON);

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

    //UUUU remove?
    // AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );

    //UUUU add Area and Transparence TabPages
    AddTabPage(RID_SVXPAGE_AREA);
    AddTabPage(RID_SVXPAGE_TRANSPARENCE);

    AddTabPage( TP_MACRO_ASSIGN, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), 0);
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
        if( 0  == (nHtmlMode & HTMLMODE_SOME_ABS_POS) || m_nDlgType != DLG_FRM_STD )
        {
            //UUUU RemoveTabPage(TP_BACKGROUND);
            RemoveTabPage(RID_SVXPAGE_AREA);
            RemoveTabPage(RID_SVXPAGE_TRANSPARENCE);
        }
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




void SwFrmDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
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
        ((SwWrapTabPage&)rPage).SetFormatUsed(m_bFormat, sal_False);
        ((SwWrapTabPage&)rPage).SetShell(m_pWrtShell);
        break;

    case TP_COLUMN:
        {
            ((SwColumnPage&)rPage).SetFrmMode(sal_True);
            ((SwColumnPage&)rPage).SetFormatUsed(m_bFormat);

            const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)
                                                m_rSet.Get( RES_FRM_SIZE );
            ((SwColumnPage&)rPage).SetPageWidth( rSize.GetWidth() );
        }
        break;

    case TP_MACRO_ASSIGN:
        {
        SfxAllItemSet aNewSet(*GetInputSetImpl()->GetPool());
        aNewSet.Put( SwMacroAssignDlg::AddEvents(
            DLG_FRM_GRF == m_nDlgType ? MACASSGN_GRAPHIC : DLG_FRM_OLE == m_nDlgType ? MACASSGN_OLE : MACASSGN_FRMURL ) );
        if ( m_pWrtShell )
            rPage.SetFrame( m_pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface() );
        rPage.PageCreated(aNewSet);
        break;
        }

    //UUUU
    //case TP_BACKGROUND:
    //    if( DLG_FRM_STD == m_nDlgType )
    //    {
    //        sal_Int32 nFlagType = SVX_SHOW_SELECTOR;
    //        if(!m_bHTMLMode)
    //            nFlagType |= SVX_ENABLE_TRANSPARENCY;
    //        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    //        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
    //        rPage.PageCreated(aSet);
    //    }
    //  break;

    case TP_BORDER:
        {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            rPage.PageCreated(aSet);
        }
        break;

        //UUUU inits for Area and Transparency TabPages
        // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
        // the color table) need to be added as items (e.g. SvxColorTableItem) to make
        // these pages find the needed attributes for fill style suggestions.
        // These are set in preparation to trigger this dialog (FN_FORMAT_FRAME_DLG and
        // FN_DRAW_WRAP_DLG), but could also be directly added from the DrawModel.
        case RID_SVXPAGE_AREA:
        {
            SfxItemSet aNew(*GetInputSetImpl()->GetPool(),
                SID_COLOR_TABLE, SID_BITMAP_LIST,
                SID_OFFER_IMPORT, SID_OFFER_IMPORT, 0, 0);

            aNew.Put(m_rSet);

            // add flag for direct graphic content selection
            aNew.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

            rPage.PageCreated(aNew);
        }
        break;

        case RID_SVXPAGE_TRANSPARENCE:
        {
            rPage.PageCreated(m_rSet);
        }
        break;
    }
}

// eof
