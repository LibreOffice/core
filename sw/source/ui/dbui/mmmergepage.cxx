/*************************************************************************
 *
 *  $RCSfile: mmmergepage.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 16:59:13 $
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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#pragma hdrstop
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
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
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
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
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
#pragma warning (disable : 4355)
    m_aHeaderFI(this,           ResId(  FI_HEADER           ) ),
    m_aEditFI(this, ResId(           FI_EDIT )),
    m_aEditPB(this, ResId(           PB_EDIT )),
    m_aFindFL(this, ResId(           FL_FIND )),
    m_aFineFT(this, ResId(           FT_FIND )),
    m_aFindED(this, ResId(           ED_FIND )),
    m_aFindPB(this, ResId(           PB_FIND )),
    m_aWholeWordsCB(this, ResId(     CB_WHOLEWORDS)),
    m_aBackwardsCB(this, ResId(      CB_BACKWARDS )),
    m_aMatchCaseCB(this, ResId(      CB_MATCHCASE )),
#pragma warning (default : 4355)
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
IMPL_LINK( SwMailMergeMergePage, EditDocumentHdl_Impl, PushButton*, pButton)
{
    m_pWizard->SetRestartPage(MM_MERGEPAGE);
    m_pWizard->EndDialog(RET_EDIT_RESULT_DOC);
    return 0;
}
/*-- 25.05.2004 16:14:49---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SwMailMergeMergePage, FindHdl_Impl, PushButton*, pButton)
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
