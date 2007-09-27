/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmmergepage.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:34:27 $
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

#ifndef _MAILMERGEMERGEPAGE_HXX
#include <mmmergepage.hxx>
#endif
#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <dbui.hrc>
#include <mmmergepage.hrc>
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SW_ABSTDLG_HXX
#include <swabstdlg.hxx>
#endif

/*-- 02.04.2004 16:38:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeMergePage::SwMailMergeMergePage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, SW_RES(DLG_MM_MERGE_PAGE)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aHeaderFI(this,           SW_RES(  FI_HEADER           ) ),
    m_aEditFI(this, SW_RES(           FI_EDIT )),
    m_aEditPB(this, SW_RES(           PB_EDIT )),
    m_aFindFL(this, SW_RES(           FL_FIND )),
    m_aFineFT(this, SW_RES(           FT_FIND )),
    m_aFindED(this, SW_RES(           ED_FIND )),
    m_aFindPB(this, SW_RES(           PB_FIND )),
    m_aWholeWordsCB(this, SW_RES(     CB_WHOLEWORDS)),
    m_aBackwardsCB(this, SW_RES(      CB_BACKWARDS )),
    m_aMatchCaseCB(this, SW_RES(      CB_MATCHCASE )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_pWizard(_pParent)
{
    FreeResource();
    String sTemp(m_aEditFI.GetText());
    sTemp.SearchAndReplace(String::CreateFromAscii("%1"), m_aEditPB.GetText());
    m_aEditFI.SetText(sTemp);
    m_aEditPB.SetClickHdl( LINK( this, SwMailMergeMergePage, EditDocumentHdl_Impl));
    m_aFindPB.SetClickHdl( LINK( this, SwMailMergeMergePage, FindHdl_Impl ));

    m_aFindED.SetReturnActionLink( LINK(this, SwMailMergeMergePage, EnteredFindStringHdl_Impl ));

}
/*-- 02.04.2004 16:38:45---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeMergePage::~SwMailMergeMergePage()
{
}
/*-- 25.05.2004 16:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeMergePage, EditDocumentHdl_Impl, PushButton*, EMPTYARG)
{
    m_pWizard->SetRestartPage(MM_MERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_RESULT_DOC);
    return 0;
}
/*-- 25.05.2004 16:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeMergePage, FindHdl_Impl, PushButton*, EMPTYARG)
{
    SvxSearchItem aSearchItem( SID_SEARCH_ITEM );

    SfxBoolItem aQuiet( SID_SEARCH_QUIET, sal_False );
    aSearchItem.SetSearchString(m_aFindED.GetText());

    aSearchItem.SetWordOnly(m_aWholeWordsCB.IsChecked());
    aSearchItem.SetExact(m_aMatchCaseCB.IsChecked());
    aSearchItem.SetBackward(m_aBackwardsCB.IsChecked());

    SwView* pTargetView = m_pWizard->GetConfigItem().GetTargetView();
    DBG_ASSERT(pTargetView, "no target view exists")
    if(pTargetView)
    {
        pTargetView->GetViewFrame()->GetDispatcher()->Execute(
            FID_SEARCH_NOW, SFX_CALLMODE_SYNCHRON, &aSearchItem, &aQuiet, 0L );
    }

    return 0;
}
IMPL_LINK( SwMailMergeMergePage, EnteredFindStringHdl_Impl, void*, EMPTYARG )
{
    m_aFindPB.GetClickHdl().Call( &m_aFindPB );
    return 0;
}
