/*************************************************************************
 *
 *  $RCSfile: mailmergewizard.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-22 13:57:53 $
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

#ifndef _MAILMERGEWIZARD_HXX
#include <mailmergewizard.hxx>
#endif
#ifndef _MAILMERGEDOCSELECTPAGE_HXX
#include <mmdocselectpage.hxx>
#endif
#ifndef _MAILMERGEOUTPUTTYPEPAGE_HXX
#include <mmoutputtypepage.hxx>
#endif
#ifndef _MAILMERGEADDRESSBLOCKPAGE_HXX
#include <mmaddressblockpage.hxx>
#endif
#ifndef _MAILMERGEPREPAREMERGEPAGE_HXX
#include <mmpreparemergepage.hxx>
#endif
#ifndef _MAILMERGEMERGEPAGE_HXX
#include <mmmergepage.hxx>
#endif
#ifndef _MAILMERGEGREETINGSPAGE_HXX
#include <mmgreetingspage.hxx>
#endif
#ifndef _MAILMERGEOUTPUTPAGE_HXX
#include <mmoutputpage.hxx>
#endif
#ifndef _MAILMERGELAYOUTPAGE_HXX
#include <mmlayoutpage.hxx>
#endif
#ifndef _MMCONFIGITEM_HXX
#include <mmconfigitem.hxx>
#endif
#ifndef _SW_ABSTDLG_HXX
#include <swabstdlg.hxx>
#endif
#ifndef _DBUI_HRC
#include <dbui.hrc>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#include <helpid.h>
#include <dbui.hrc>
#include <mailmergewizard.hrc>

using namespace svt;
/*-- 01.04.2004 16:52:49---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeWizard::SwMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rItem) :
        RoadmapWizard(&rView.GetViewFrame()->GetWindow(),
                        SW_RES(DLG_MAILMERGEWIZARD),
                        WZB_NEXT|WZB_PREVIOUS|WZB_FINISH|WZB_CANCEL|WZB_HELP, ResId( STR_ROADMAP_TITLE )),
        m_pSwView(&rView),
        m_rConfigItem(rItem),
        m_sStarting(        ResId( ST_STARTING      )),
        m_sDocumentType(    ResId( ST_DOCUMETNTYPE   )),
        m_sAddressBlock(    ResId( ST_ADDRESSBLOCK   )),
        m_sAddressList(     ResId( ST_ADDRESSLIST )),
        m_sGreetingsLine(   ResId( ST_GREETINGSLINE   )),
        m_sLayout(          ResId( ST_LAYOUT        )),
        m_sPrepareMerge(    ResId( ST_PREPAREMERGE )),
        m_sMerge(           ResId( ST_MERGE        )),
        m_sOutput(          ResId( ST_OUTPUT       )),
        m_sFinish(          ResId( ST_FINISH       )),
        m_nRestartPage( MM_DOCUMENTSELECTPAGE )
{
    FreeResource();
    ShowButtonFixedLine(sal_True);
    defaultButton(WZB_NEXT);
    enableButtons(WZB_FINISH, sal_False);

    m_pFinish->SetText(m_sFinish);
    m_pNextPage->SetHelpId(HID_MM_NEXT_PAGE);
    m_pPrevPage->SetHelpId(HID_MM_PREV_PAGE);

    declarePath(
        0,
        MM_DOCUMENTSELECTPAGE,
        MM_OUTPUTTYPETPAGE,
        MM_ADDRESSBLOCKPAGE,
        MM_GREETINGSPAGE,
        MM_LAYOUTPAGE,
        MM_PREPAREMERGEPAGE,
        MM_MERGEPAGE,
        MM_OUTPUTPAGE,
        WZS_INVALID_STATE
    );

    ActivatePage();
    UpdateRoadmap();
}
/*-- 01.04.2004 16:52:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwMailMergeWizard::~SwMailMergeWizard()
{
}
/*-- 01.04.2004 16:52:53---------------------------------------------------

  -----------------------------------------------------------------------*/
OWizardPage*    SwMailMergeWizard::createPage(WizardState _nState)
{
    OWizardPage* pRet = 0;
    switch(_nState)
    {
        case MM_DOCUMENTSELECTPAGE : pRet = new SwMailMergeDocSelectPage(this);     break;
        case MM_OUTPUTTYPETPAGE : pRet = new SwMailMergeOutputTypePage(this);       break;
        case MM_ADDRESSBLOCKPAGE  : pRet = new SwMailMergeAddressBlockPage(this);     break;
        case MM_GREETINGSPAGE     : pRet = new SwMailMergeGreetingsPage(this);      break;
        case MM_LAYOUTPAGE        : pRet = new SwMailMergeLayoutPage(this);     break;
        case MM_PREPAREMERGEPAGE  : pRet = new SwMailMergePrepareMergePage(this);   break;
        case MM_MERGEPAGE         : pRet = new SwMailMergeMergePage(this);          break;
        case MM_OUTPUTPAGE       :  pRet = new SwMailMergeOutputPage(this);         break;
    }
    DBG_ASSERT(pRet, "no page created in ::createPage");
    return pRet;
}
/*-- 04.06.2004 13:18:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeWizard::enterState( WizardState _nState )
{
    ::svt::RoadmapWizard::enterState( _nState );
/*
    entering a page after the layoutpage requires the insertion
    of greeting and address block - if not yet done
    entering the merge or output page requires to create the output document
*/
    if(_nState > MM_LAYOUTPAGE && m_rConfigItem.GetSourceView() &&
            ((m_rConfigItem.IsAddressBlock() && !m_rConfigItem.IsAddressInserted()) ||
             (m_rConfigItem.IsGreetingLine(sal_False) && !m_rConfigItem.IsGreetingInserted() )))
    {
        SwMailMergeLayoutPage::InsertAddressAndGreeting(m_rConfigItem.GetSourceView(),
                                m_rConfigItem, Point(-1, -1), true);
    }
    if(_nState >= MM_MERGEPAGE && !m_rConfigItem.GetTargetView())
    {
        CreateTargetDocument();
        m_nRestartPage = _nState;
        EndDialog(RET_TARGET_CREATED);
        return;
    }
    else if(_nState < MM_MERGEPAGE && m_rConfigItem.GetTargetView())
    {
        //close the dialog, remove the target view, show the source view
        m_nRestartPage = _nState;
        EndDialog(RET_REMOVE_TARGET);
        return;
    }
    bool bEnablePrev = true;
    bool bEnableNext = true;
    switch(_nState)
    {
        case MM_DOCUMENTSELECTPAGE :
            bEnablePrev = false;
        break;
        case MM_ADDRESSBLOCKPAGE  :
            bEnableNext = m_rConfigItem.GetResultSet().is();
        break;
        case MM_OUTPUTPAGE       :
            bEnableNext = false;
        break;
    }
    enableButtons( WZB_PREVIOUS, bEnablePrev);
    enableButtons( WZB_NEXT, bEnableNext);

    UpdateRoadmap();
}
/*-- 04.06.2004 12:54:13---------------------------------------------------

  -----------------------------------------------------------------------*/
String  SwMailMergeWizard::getStateDisplayName( WizardState _nState )
{
    String sRet;
    switch(_nState)
    {
        case MM_DOCUMENTSELECTPAGE :sRet = m_sStarting;      break;
        case MM_OUTPUTTYPETPAGE :   sRet = m_sDocumentType;  break;
        case MM_ADDRESSBLOCKPAGE  :
            sRet =  m_rConfigItem.IsOutputToLetter() ?
                    m_sAddressBlock : m_sAddressList;

        break;
        case MM_GREETINGSPAGE     : sRet = m_sGreetingsLine; break;
        case MM_LAYOUTPAGE        : sRet = m_sLayout;        break;
        case MM_PREPAREMERGEPAGE  : sRet = m_sPrepareMerge;  break;
        case MM_MERGEPAGE         : sRet = m_sMerge;         break;
        case MM_OUTPUTPAGE       : sRet = m_sOutput;        break;
    }
    return sRet;
}
/*-- 07.06.2004 15:23:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeWizard::StateChanged( StateChangedType nStateChange )
{
    ::svt::RoadmapWizard::StateChanged(nStateChange);
    if(nStateChange == STATE_CHANGE_INITSHOW)
    {
        //re-position the buttons:
        Size aOutputSize(GetOutputSizePixel());
        sal_Int32 nButtonWidth = m_pFinish->GetSizePixel().Width();
        sal_Int32 nSmallDistance = PixelToLogic(Size(3, 3)).Width();
        sal_Int32 nBigDistance = 2 * nSmallDistance;

        Point aMostRight(m_pFinish->GetPosPixel());

        Point aFinishPos(aMostRight.X() - nSmallDistance - nButtonWidth, aMostRight.Y());
        Point aNextPos(aFinishPos.X() - nBigDistance - nButtonWidth, aFinishPos.Y());
        Point aPrevPos(aNextPos.X() - nSmallDistance - nButtonWidth, aNextPos.Y());

        //help button to the left - same distance as finish from right
        Point aMostLeft(aMostRight);
        aMostLeft.X() = aOutputSize.Width() - aMostRight.X() - nButtonWidth;
        m_pHelp->SetPosPixel(aMostLeft);

        //cancel button to the right
        m_pCancel->SetPosPixel(aMostRight);
        m_pFinish->SetPosPixel(aFinishPos);
        m_pNextPage->SetPosPixel(aNextPos);
        m_pPrevPage->SetPosPixel(aPrevPos);
    }
}
/*-- 24.06.2004 09:24:45---------------------------------------------------
    enables/disables pages in the roadmap depending on the current
    page and state
  -----------------------------------------------------------------------*/
void SwMailMergeWizard::UpdateRoadmap()
{
/*
    MM_DOCUMENTSELECTPAGE       > inactive after the layoutpage
    MM_OUTPUTTYPETPAGE :        > inactive after the layoutpage
    MM_ADDRESSBLOCKPAGE         > inactive after the layoutpage
    MM_GREETINGSPAGE            > inactive after the layoutpage
    MM_LAYOUTPAGE               >   inactive after the layoutpage
                                    inactive if address block and greeting are switched off
                                    or are already inserted into the source document
    MM_PREPAREMERGEPAGE         > only active if address data has been selected
                                    inactive after preparemerge page
    MM_MERGEPAGE                > only active if address data has been selected

    MM_OUTPUTPAGE               > only active if address data has been selected
*/

    // enableState( <page id>, false );
    const sal_uInt16 nCurPage = GetCurLevel();
    bool bEnable = false;
    for(sal_uInt16 nPage = MM_DOCUMENTSELECTPAGE; nPage <= MM_OUTPUTPAGE; ++nPage)
    {
        switch(nPage)
        {
            case MM_DOCUMENTSELECTPAGE :
            case MM_OUTPUTTYPETPAGE :
            case MM_ADDRESSBLOCKPAGE  :
                bEnable = sal_True;
            break;
            case MM_LAYOUTPAGE        :
                bEnable = m_rConfigItem.GetResultSet().is() &&
                        ((m_rConfigItem.IsAddressBlock() && !m_rConfigItem.IsAddressInserted()) ||
                            (m_rConfigItem.IsGreetingLine(sal_False) && !m_rConfigItem.IsGreetingInserted() ));
            break;
            case MM_PREPAREMERGEPAGE  :
            case MM_GREETINGSPAGE     :
            case MM_MERGEPAGE         :
            case MM_OUTPUTPAGE       :
                bEnable = m_rConfigItem.GetResultSet().is();
            break;
        }
        enableState( nPage, bEnable );
    }
}
/*-- 24.06.2004 09:24:45---------------------------------------------------
    enables/disables pages in the roadmap depending on the current
    page and state
  -----------------------------------------------------------------------*/
void SwMailMergeWizard::CreateTargetDocument()
{
    sal_Int32 nMerged = GetSwView()->GetWrtShell().GetNewDBMgr()->
                MergeDocuments( m_rConfigItem, *GetSwView() );
    m_rConfigItem.SetMergeDone();
    if( m_rConfigItem.GetTargetView() )
        m_rConfigItem.GetTargetView()->GetViewFrame()->GetFrame()->Appear();
}
/*-- 23.08.2004 08:51:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwMailMergeWizard::updateRoadmapItemLabel( WizardState _nState )
{
    svt::RoadmapWizard::updateRoadmapItemLabel( _nState );
}
