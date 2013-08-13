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
#ifndef _MAILMERGEWIZARD_HXX
#define _MAILMERGEWIZARD_HXX

#include <svtools/roadmapwizard.hxx>

class SwView;
class SwMailMergeConfigItem;

#define MM_DOCUMENTSELECTPAGE   0
#define MM_OUTPUTTYPETPAGE      1
#define MM_ADDRESSBLOCKPAGE     2
#define MM_GREETINGSPAGE        3
#define MM_LAYOUTPAGE           4
#define MM_PREPAREMERGEPAGE     5
#define MM_MERGEPAGE            6
#define MM_OUTPUTPAGE           7

class SwMailMergeWizard : public ::svt::RoadmapWizard
{
    SwView*                 m_pSwView;
    String                  sDocumentURL;
    bool                    m_bDocumentLoad;

    SwMailMergeConfigItem&  m_rConfigItem;

    String                  m_sStarting;
    String                  m_sDocumentType;
    String                  m_sAddressBlock;
    String                  m_sAddressList;
    String                  m_sGreetingsLine;
    String                  m_sLayout;
    String                  m_sPrepareMerge;
    String                  m_sMerge;
    String                  m_sOutput;
    String                  m_sFinish;

    sal_uInt16              m_nRestartPage;

    using svt::OWizardMachine::skipUntil;

protected:
    virtual svt::OWizardPage*       createPage(WizardState _nState);
    virtual void                    enterState( WizardState _nState );

// roadmap feature ??
//    virtual sal_Bool            prepareLeaveCurrentState( CommitPageReason _eReason );
    virtual OUString                getStateDisplayName( WizardState _nState ) const;

public:
    SwMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem);
    ~SwMailMergeWizard();

    SwView*                     GetSwView() {return m_pSwView;}
    SwMailMergeConfigItem&      GetConfigItem() { return m_rConfigItem;}

    void                    SetReloadDocument(const String& rURL){sDocumentURL = rURL;}
    const String&           GetReloadDocument() const {return sDocumentURL;}

    //next step requires loading of document
    void                    SetDocumentLoad(bool bSet) {m_bDocumentLoad = bSet;}

    void                    UpdateRoadmap();
    void                    CreateTargetDocument();

    sal_uInt16              GetRestartPage() const {return m_nRestartPage;}
    void                    SetRestartPage(sal_uInt16 nPage) { m_nRestartPage = nPage;}

    sal_Bool                skipUntil( sal_uInt16 nPage)
                                {return ::svt::RoadmapWizard::skipUntil(WizardState(nPage));}

    void                    updateRoadmapItemLabel( WizardState _nState );

     virtual short          Execute();
    virtual void            StartExecuteModal( const Link& rEndDialogHdl );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
