/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <mailmergewizard.hxx>
#include <mmdocselectpage.hxx>
#include <mmoutputtypepage.hxx>
#include <mmaddressblockpage.hxx>
#include <mmgreetingspage.hxx>
#include <mmlayoutpage.hxx>
#include <mmconfigitem.hxx>
#include <swabstdlg.hxx>
#include <dbui.hrc>
#include <view.hxx>
#include <dbmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <wrtsh.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dataaccessdescriptor.hxx>

#include <helpid.h>

using namespace svt;
using namespace ::com::sun::star;

SwMailMergeWizard::SwMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rItem) :
        RoadmapWizard(&rView.GetViewFrame()->GetWindow(),
                        WizardButtonFlags::NEXT|WizardButtonFlags::PREVIOUS|WizardButtonFlags::FINISH|WizardButtonFlags::CANCEL|WizardButtonFlags::HELP),
        m_pSwView(&rView),
        m_bDocumentLoad( false ),
        m_rConfigItem(rItem),
        m_sStarting(        SW_RES( ST_STARTING      )),
        m_sDocumentType(    SW_RES( ST_DOCUMENTTYPE   )),
        m_sAddressBlock(    SW_RES( ST_ADDRESSBLOCK   )),
        m_sAddressList(     SW_RES( ST_ADDRESSLIST )),
        m_sGreetingsLine(   SW_RES( ST_GREETINGSLINE   )),
        m_sLayout(          SW_RES( ST_LAYOUT        )),
        m_sFinish(          SW_RES( ST_FINISH       )),
        m_nRestartPage( MM_DOCUMENTSELECTPAGE )
{
    defaultButton(WizardButtonFlags::NEXT);
    enableButtons(WizardButtonFlags::FINISH, false);

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
            WZS_INVALID_STATE
        );
    else
        declarePath(
            0,
            MM_DOCUMENTSELECTPAGE,
            MM_ADDRESSBLOCKPAGE,
            MM_GREETINGSPAGE,
            MM_LAYOUTPAGE,
            WZS_INVALID_STATE
        );

    ActivatePage();
    UpdateRoadmap();
}

SwMailMergeWizard::~SwMailMergeWizard()
{
}

VclPtr<TabPage> SwMailMergeWizard::createPage(WizardState _nState)
{
    VclPtr<OWizardPage> pRet;
    switch(_nState)
    {
        case MM_DOCUMENTSELECTPAGE : pRet = VclPtr<SwMailMergeDocSelectPage>::Create(this);     break;
        case MM_OUTPUTTYPETPAGE : pRet = VclPtr<SwMailMergeOutputTypePage>::Create(this);       break;
        case MM_ADDRESSBLOCKPAGE  : pRet = VclPtr<SwMailMergeAddressBlockPage>::Create(this);     break;
        case MM_GREETINGSPAGE     : pRet = VclPtr<SwMailMergeGreetingsPage>::Create(this);      break;
        case MM_LAYOUTPAGE        : pRet = VclPtr<SwMailMergeLayoutPage>::Create(this);     break;
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
             (m_rConfigItem.IsGreetingLine(false) && !m_rConfigItem.IsGreetingInserted() )))
    {
        SwMailMergeLayoutPage::InsertAddressAndGreeting(m_rConfigItem.GetSourceView(),
                                m_rConfigItem, Point(-1, -1), true);
    }

    if(m_rConfigItem.GetTargetView())
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
        case MM_DOCUMENTSELECTPAGE:
            bEnablePrev = false; // the first page
        break;
        case MM_ADDRESSBLOCKPAGE  :
            bEnableNext = m_rConfigItem.GetResultSet().is();
        break;
        case MM_LAYOUTPAGE:
            bEnableNext = false; // the last page
        break;
    }
    enableButtons( WizardButtonFlags::PREVIOUS, bEnablePrev);
    enableButtons( WizardButtonFlags::NEXT, bEnableNext);

    UpdateRoadmap();
}

OUString SwMailMergeWizard::getStateDisplayName( WizardState _nState ) const
{
    switch(_nState)
    {
        case MM_DOCUMENTSELECTPAGE:
            return m_sStarting;
        case MM_OUTPUTTYPETPAGE:
            return m_sDocumentType;
        case MM_ADDRESSBLOCKPAGE:
            return m_rConfigItem.IsOutputToLetter() ?
                   m_sAddressBlock : m_sAddressList;
        case MM_GREETINGSPAGE:
            return m_sGreetingsLine;
        case MM_LAYOUTPAGE:
            return m_sLayout;
    }
    return OUString();
}

// enables/disables pages in the roadmap depending on the current page and state
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
*/

    // enableState( <page id>, false );
    const sal_uInt16 nCurPage = GetCurLevel();
    TabPage* pCurPage = GetPage( nCurPage );
    if(!pCurPage)
        return;
    bool bAddressFieldsConfigured = !m_rConfigItem.IsOutputToLetter() ||
                !m_rConfigItem.IsAddressBlock() ||
                m_rConfigItem.IsAddressFieldsAssigned();
    bool bGreetingFieldsConfigured = !m_rConfigItem.IsGreetingLine(false) ||
            !m_rConfigItem.IsIndividualGreeting(false)||
                    m_rConfigItem.IsGreetingFieldsAssigned();

    //#i97436# if a document has to be loaded then enable output type page only
    m_bDocumentLoad = false;
    bool bEnableOutputTypePage = (nCurPage != MM_DOCUMENTSELECTPAGE) ||
        static_cast<svt::OWizardPage*>(pCurPage)->commitPage( ::svt::WizardTypes::eValidate );

    // handle the Finish button
    bool bCanFinish = !m_bDocumentLoad && bEnableOutputTypePage &&
        m_rConfigItem.GetResultSet().is() &&
        bAddressFieldsConfigured &&
        bGreetingFieldsConfigured;
    enableButtons(WizardButtonFlags::FINISH, (nCurPage != MM_DOCUMENTSELECTPAGE) && bCanFinish);

    for(sal_uInt16 nPage = MM_DOCUMENTSELECTPAGE; nPage <= MM_LAYOUTPAGE; ++nPage)
    {
        bool bEnable = true;
        switch(nPage)
        {
            case MM_DOCUMENTSELECTPAGE:
                bEnable = true;
            break;
            case MM_OUTPUTTYPETPAGE:
                bEnable = bEnableOutputTypePage;
            break;
            case MM_ADDRESSBLOCKPAGE:
                bEnable = !m_bDocumentLoad && bEnableOutputTypePage;
            break;
            case MM_GREETINGSPAGE:
                bEnable = !m_bDocumentLoad && bEnableOutputTypePage &&
                    m_rConfigItem.GetResultSet().is() &&
                            bAddressFieldsConfigured;
            break;
            case MM_LAYOUTPAGE:
                bEnable = bCanFinish &&
                        ((m_rConfigItem.IsAddressBlock() && !m_rConfigItem.IsAddressInserted()) ||
                            (m_rConfigItem.IsGreetingLine(false) && !m_rConfigItem.IsGreetingInserted() ));
            break;
        }
        enableState( nPage, bEnable );
    }
}

// enables/disables pages in the roadmap depending on the current page and state
void SwMailMergeWizard::CreateTargetDocument()
{
    svx::ODataAccessDescriptor aDescriptor;
    aDescriptor.setDataSource( m_rConfigItem.GetCurrentDBData().sDataSource );
    aDescriptor[ svx::daConnection ]  <<= m_rConfigItem.GetConnection().getTyped();
    aDescriptor[ svx::daCursor ]      <<= m_rConfigItem.GetResultSet();
    aDescriptor[ svx::daCommand ]     <<= m_rConfigItem.GetCurrentDBData().sCommand;
    aDescriptor[ svx::daCommandType ] <<= m_rConfigItem.GetCurrentDBData().nCommandType;
    aDescriptor[ svx::daSelection ]   <<= m_rConfigItem.GetSelection();

    SwMergeDescriptor aMergeDesc( DBMGR_MERGE_SHELL, GetSwView()->GetWrtShell(),
        aDescriptor);
    aMergeDesc.pMailMergeConfigItem = &m_rConfigItem;
    aMergeDesc.bCreateSingleFile = true;

    GetSwView()->GetWrtShell().GetDBManager()->MergeNew( aMergeDesc, this );
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

void SwMailMergeWizard::StartExecuteModal( const Link<Dialog&, void>& rEndDialogHdl )
{
    ::svt::RoadmapWizard::StartExecuteModal( rEndDialogHdl );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
