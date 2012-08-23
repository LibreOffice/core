/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <mailmergewizard.hxx>
#include <mmdocselectpage.hxx>
#include <mmoutputtypepage.hxx>
#include <mmaddressblockpage.hxx>
#include <mmpreparemergepage.hxx>
#include <mmmergepage.hxx>
#include <mmgreetingspage.hxx>
#include <mmoutputpage.hxx>
#include <mmlayoutpage.hxx>
#include <mmconfigitem.hxx>
#include <swabstdlg.hxx>
#include <dbui.hrc>
#include <view.hxx>
#include <dbmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <wrtsh.hxx>
#include "vcl/msgbox.hxx" // RET_CANCEL

#include <helpid.h>
#include <mailmergewizard.hrc>

using namespace svt;
using namespace ::com::sun::star;

SwMailMergeWizard::SwMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rItem) :
        RoadmapWizard(&rView.GetViewFrame()->GetWindow(),
                        SW_RES(DLG_MAILMERGEWIZARD),
                        WZB_NEXT|WZB_PREVIOUS|WZB_FINISH|WZB_CANCEL|WZB_HELP),
        m_pSwView(&rView),
        m_bDocumentLoad( false ),
        m_rConfigItem(rItem),
        m_sStarting(        SW_RES( ST_STARTING      )),
        m_sDocumentType(    SW_RES( ST_DOCUMETNTYPE   )),
        m_sAddressBlock(    SW_RES( ST_ADDRESSBLOCK   )),
        m_sAddressList(     SW_RES( ST_ADDRESSLIST )),
        m_sGreetingsLine(   SW_RES( ST_GREETINGSLINE   )),
        m_sLayout(          SW_RES( ST_LAYOUT        )),
        m_sPrepareMerge(    SW_RES( ST_PREPAREMERGE )),
        m_sMerge(           SW_RES( ST_MERGE        )),
        m_sOutput(          SW_RES( ST_OUTPUT       )),
        m_sFinish(          SW_RES( ST_FINISH       )),
        m_nRestartPage( MM_DOCUMENTSELECTPAGE )
{
    FreeResource();
    ShowButtonFixedLine(sal_True);
    defaultButton(WZB_NEXT);
    enableButtons(WZB_FINISH, sal_False);

    m_pFinish->SetText(m_sFinish);
    m_pNextPage->SetHelpId(HID_MM_NEXT_PAGE);
    m_pPrevPage->SetHelpId(HID_MM_PREV_PAGE);

    //#i51949# no output type page visible if e-Mail is not supported
    if(rItem.IsMailAvailable())
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
    else
        declarePath(
            0,
            MM_DOCUMENTSELECTPAGE,
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

SwMailMergeWizard::~SwMailMergeWizard()
{
}

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
    OSL_ENSURE(pRet, "no page created in ::createPage");
    return pRet;
}

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
        //set ResultSet back to start
        m_rConfigItem.MoveResultSet(1);
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

String  SwMailMergeWizard::getStateDisplayName( WizardState _nState ) const
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
/*----------------------------------------------------------------------
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
    TabPage* pCurPage = GetPage( nCurPage );
    if(!pCurPage)
        return;
    bool bEnable = false;
    bool bAddressFieldsConfigured = !m_rConfigItem.IsOutputToLetter() ||
                !m_rConfigItem.IsAddressBlock() ||
                m_rConfigItem.IsAddressFieldsAssigned();
    bool bGreetingFieldsConfigured = !m_rConfigItem.IsGreetingLine(sal_False) ||
            !m_rConfigItem.IsIndividualGreeting(sal_False)||
                    m_rConfigItem.IsGreetingFieldsAssigned();
    //#i97436# if a document has to be loaded then enable output type page only
    m_bDocumentLoad = false;
    bool bEnableOutputTypePage = (nCurPage != MM_DOCUMENTSELECTPAGE) ||
        static_cast<svt::OWizardPage*>(pCurPage)->commitPage( ::svt::WizardTypes::eValidate );

    for(sal_uInt16 nPage = MM_DOCUMENTSELECTPAGE; nPage <= MM_OUTPUTPAGE; ++nPage)
    {
        switch(nPage)
        {
            case MM_DOCUMENTSELECTPAGE :
                bEnable = sal_True;
            break;
            case MM_OUTPUTTYPETPAGE :
                bEnable = bEnableOutputTypePage;
            break;
            case MM_ADDRESSBLOCKPAGE  :
                bEnable = !m_bDocumentLoad && bEnableOutputTypePage;
            break;
            case MM_GREETINGSPAGE     :
                bEnable = !m_bDocumentLoad && bEnableOutputTypePage &&
                    m_rConfigItem.GetResultSet().is() &&
                            bAddressFieldsConfigured;
            break;
            case MM_PREPAREMERGEPAGE  :
            case MM_MERGEPAGE         :
            case MM_OUTPUTPAGE       :
            case MM_LAYOUTPAGE        :
                bEnable = !m_bDocumentLoad && bEnableOutputTypePage &&
                            m_rConfigItem.GetResultSet().is() &&
                            bAddressFieldsConfigured &&
                            bGreetingFieldsConfigured;
                if(MM_LAYOUTPAGE == nPage)
                    bEnable &=
                        ((m_rConfigItem.IsAddressBlock() && !m_rConfigItem.IsAddressInserted()) ||
                            (m_rConfigItem.IsGreetingLine(sal_False) && !m_rConfigItem.IsGreetingInserted() ));
            break;
        }
        enableState( nPage, bEnable );
    }
}
/*-- --------------------------------------------------------------------
    enables/disables pages in the roadmap depending on the current
    page and state
  -----------------------------------------------------------------------*/
void SwMailMergeWizard::CreateTargetDocument()
{
    GetSwView()->GetWrtShell().GetNewDBMgr()->
                MergeDocuments( m_rConfigItem, *GetSwView() );
    m_rConfigItem.SetMergeDone();
    if( m_rConfigItem.GetTargetView() )
        m_rConfigItem.GetTargetView()->GetViewFrame()->GetFrame().Appear();
}

void SwMailMergeWizard::updateRoadmapItemLabel( WizardState _nState )
{
    svt::RoadmapWizard::updateRoadmapItemLabel( _nState );
}

short SwMailMergeWizard::Execute()
{
    OSL_FAIL("SwMailMergeWizard cannot be executed via Dialog::Execute!\n"
               "It creates a thread (MailDispatcher instance) that will call"
               "back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

void SwMailMergeWizard::StartExecuteModal( const Link& rEndDialogHdl )
{
    ::svt::RoadmapWizard::StartExecuteModal( rEndDialogHdl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
