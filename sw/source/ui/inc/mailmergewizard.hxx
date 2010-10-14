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
    virtual String                  getStateDisplayName( WizardState _nState ) const;

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
