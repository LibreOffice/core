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
#include <strings.hrc>
#include <view.hxx>
#include <dbmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <wrtsh.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dataaccessdescriptor.hxx>

#include <helpid.h>

using namespace svt;
using namespace ::com::sun::star;

SwMailMergeWizard::SwMailMergeWizard(SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rItem) :
        RoadmapWizard(&rView.GetViewFrame()->GetWindow()),
        m_pSwView(&rView),
        m_bDocumentLoad( false ),
        m_xConfigItem(rItem),
        m_sStarting(        SwResId( ST_STARTING      )),
        m_sDocumentType(    SwResId( ST_DOCUMENTTYPE   )),
        m_sAddressBlock(    SwResId( ST_ADDRESSBLOCK   )),
        m_sAddressList(     SwResId( ST_ADDRESSLIST )),
        m_sGreetingsLine(   SwResId( ST_GREETINGSLINE   )),
        m_sLayout(          SwResId( ST_LAYOUT        )),
        m_sFinish(          SwResId( ST_FINISH       )),
        m_nRestartPage( MM_DOCUMENTSELECTPAGE )
{
    defaultButton(WizardButtonFlags::NEXT);
    enableButtons(WizardButtonFlags::FINISH, false);

    setTitleBase(SwResId( ST_MMWTITLE ) );

    m_pFinish->SetText(m_sFinish);
    m_pNextPage->SetHelpId(HID_MM_NEXT_PAGE);
    m_pPrevPage->SetHelpId(HID_MM_PREV_PAGE);

    //#i51949# no output type page visible if e-Mail is not supported
    if (m_xConfigItem->IsMailAvailable())
        declarePath(
            0,
            {MM_DOCUMENTSELECTPAGE,
            MM_OUTPUTTYPETPAGE,
            MM_ADDRESSBLOCKPAGE,
            MM_GREETINGSPAGE,
            MM_LAYOUTPAGE}
        );
    else
        declarePath(
            0,
            {MM_DOCUMENTSELECTPAGE,
            MM_ADDRESSBLOCKPAGE,
            MM_GREETINGSPAGE,
            MM_LAYOUTPAGE}
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
        case MM_DOCUMENTSELECTPAGE :
            pRet = VclPtr<SwMailMergeDocSelectPage>::Create(this);

            /* tdf#52986 Set help ID using SetRoadmapHelpId for all pages
            so that when by default the focus is on the left side pane of
            the wizard the relevant help page is displayed when hitting
            the Help / F1 button */
            SetRoadmapHelpId("modules/swriter/ui/mmselectpage/MMSelectPage");
        break;
        case MM_OUTPUTTYPETPAGE    :
            pRet = VclPtr<SwMailMergeOutputTypePage>::Create(this);
            SetRoadmapHelpId("modules/swriter/ui/mmoutputtypepage/MMOutputTypePage");
        break;
        case MM_ADDRESSBLOCKPAGE   :
            pRet = VclPtr<SwMailMergeAddressBlockPage>::Create(this);
            SetRoadmapHelpId("modules/swriter/ui/mmaddressblockpage/MMAddressBlockPage");
        break;
        case MM_GREETINGSPAGE      :
            pRet = VclPtr<SwMailMergeGreetingsPage>::Create(this);
            SetRoadmapHelpId("modules/swriter/ui/mmsalutationpage/MMSalutationPage");
        break;
        case MM_LAYOUTPAGE         :
            pRet = VclPtr<SwMailMergeLayoutPage>::Create(this);
            SetRoadmapHelpId("modules/swriter/ui/mmlayoutpage/MMLayoutPage");
        break;
    }
    OSL_ENSURE(pRet, "no page created in ::createPage");
    return pRet;
}

void SwMailMergeWizard::enterState( WizardState _nState )
{
    ::svt::RoadmapWizard::enterState( _nState );

    if (m_xConfigItem->GetTargetView())
    {
        //close the dialog, remove the target view, show the source view
        m_nRestartPage = _nState;
        //set ResultSet back to start
        m_xConfigItem->MoveResultSet(1);
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
            bEnableNext = m_xConfigItem->GetResultSet().is();
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
            return m_xConfigItem->IsOutputToLetter() ?
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
    bool bAddressFieldsConfigured = !m_xConfigItem->IsOutputToLetter() ||
                !m_xConfigItem->IsAddressBlock() ||
                m_xConfigItem->IsAddressFieldsAssigned();
    bool bGreetingFieldsConfigured = !m_xConfigItem->IsGreetingLine(false) ||
            !m_xConfigItem->IsIndividualGreeting(false) ||
                    m_xConfigItem->IsGreetingFieldsAssigned();

    //#i97436# if a document has to be loaded then enable output type page only
    m_bDocumentLoad = false;
    bool bEnableOutputTypePage = (nCurPage != MM_DOCUMENTSELECTPAGE) ||
        static_cast<svt::OWizardPage*>(pCurPage)->commitPage( ::svt::WizardTypes::eValidate );

    // handle the Finish button
    bool bCanFinish = !m_bDocumentLoad && bEnableOutputTypePage &&
        m_xConfigItem->GetResultSet().is() &&
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
                    m_xConfigItem->GetResultSet().is() &&
                            bAddressFieldsConfigured;
            break;
            case MM_LAYOUTPAGE:
                bEnable = bCanFinish &&
                        ((m_xConfigItem->IsAddressBlock() && !m_xConfigItem->IsAddressInserted()) ||
                            (m_xConfigItem->IsGreetingLine(false) && !m_xConfigItem->IsGreetingInserted() ));
            break;
        }
        enableState( nPage, bEnable );
    }
}

short SwMailMergeWizard::Execute()
{
    OSL_FAIL("SwMailMergeWizard cannot be executed via Dialog::Execute!\n"
               "It creates a thread (MailDispatcher instance) that will call"
               "back to VCL apartment => deadlock!\n"
               "Use Dialog::StartExecuteModal to execute the dialog!" );
    return RET_CANCEL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
