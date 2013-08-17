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

#ifndef DBACCESS_MACROMIGRATIONPAGES_HXX
#define DBACCESS_MACROMIGRATIONPAGES_HXX

#include "migrationprogress.hxx"
#include "rangeprogressbar.hxx"

#include <svtools/svmedit.hxx>
#include <svtools/urlcontrol.hxx>
#include <svtools/wizardmachine.hxx>
#include <svx/databaselocationinput.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>

namespace svt
{
    class RoadmapWizard;
}

namespace dbmm
{

    class MacroMigrationDialog;

    // MacroMigrationPage
    typedef ::svt::OWizardPage  MacroMigrationPage_Base;
    class MacroMigrationPage : public MacroMigrationPage_Base
    {
    public:
        MacroMigrationPage( MacroMigrationDialog& _rParentDialog, const ResId& _rRes );
        ~MacroMigrationPage();

    protected:
        const MacroMigrationDialog& getDialog() const;
              MacroMigrationDialog& getDialog()      ;

    protected:
        FixedText   m_aHeader;
    };

    // PreparationPage
    class PreparationPage : public MacroMigrationPage
    {
    public:
        PreparationPage( MacroMigrationDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

    public:
        void    showCloseDocsError( bool _bShow );

    protected:
        FixedText   m_aIntroduction;
        FixedText   m_aCloseDocError;
    };

    // SaveDBDocPage
    class SaveDBDocPage : public MacroMigrationPage
    {
    public:
        SaveDBDocPage( MacroMigrationDialog& _rParentDialog );
        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

    public:
        OUString getBackupLocation() const { return m_aLocationController.getURL(); }
        void            grabLocationFocus() { m_aSaveAsLocation.GrabFocus(); }

    protected:
        FixedText               m_aExplanation;
        FixedText               m_aSaveAsLabel;
        ::svt::OFileURLControl  m_aSaveAsLocation;
        PushButton              m_aBrowseSaveAsLocation;
        FixedText               m_aStartMigration;
        ::svx::DatabaseLocationInputController
                                m_aLocationController;

    protected:
        // IWizardPageController overridables
        virtual void        initializePage();
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
        virtual bool        canAdvance() const;

    private:
        DECL_LINK( OnLocationModified, Edit* );
        void impl_updateLocationDependentItems();
    };

    // ProgressPage
    class ProgressPage : public MacroMigrationPage, public IMigrationProgress
    {
    public:
        ProgressPage( MacroMigrationDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void    setDocumentCounts( const sal_Int32 _nForms, const sal_Int32 _nReports );
        void    onFinishedSuccessfully();

    protected:
        // IMigrationProgress
        virtual void    startObject( const OUString& _rObjectName, const OUString& _rCurrentAction, const sal_uInt32 _bRange );
        virtual void    setObjectProgressText( const OUString& _rText );
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue );
        virtual void    endObject();
        virtual void    start( const sal_uInt32 _nOverallRange );
        virtual void    setOverallProgressText( const OUString& _rText );
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue );

    private:
        FixedText           m_aObjectCount;
        FixedText           m_aCurrentObjectLabel;
        FixedText           m_aCurrentObject;
        FixedText           m_aCurrentActionLabel;
        FixedText           m_aCurrentAction;
        RangeProgressBar    m_aCurrentProgress;
        FixedText           m_aAllProgressLabel;
        FixedText           m_aAllProgressText;
        RangeProgressBar    m_aAllProgress;
        FixedText           m_aMigrationDone;
    };

    // ResultPage
    class ResultPage : public MacroMigrationPage
    {
    public:
        ResultPage( MacroMigrationDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void            displayMigrationLog( const bool _bSuccessful, const OUString& _rLog );

    private:
        FixedText       m_aChangesLabel;
        MultiLineEdit   m_aChanges;
        OUString          m_aSuccessful;
        OUString          m_aUnsuccessful;
    };

} // namespace dbmm

#endif // DBACCESS_MACROMIGRATIONPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
