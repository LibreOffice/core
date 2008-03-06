/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mailmergewizard.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:07:48 $
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
#ifndef _MAILMERGEWIZARD_HXX
#define _MAILMERGEWIZARD_HXX

#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif

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

