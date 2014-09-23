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

#ifndef INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONPAGES_HXX
#define INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONPAGES_HXX

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
        MacroMigrationPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription);

    protected:
        const MacroMigrationDialog& getDialog() const;
              MacroMigrationDialog& getDialog();
    };

    // PreparationPage
    class PreparationPage : public MacroMigrationPage
    {
    public:
        PreparationPage(vcl::Window *pParent);

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

    public:
        void    showCloseDocsError(bool _bShow);

    protected:
        FixedText*  m_pCloseDocError;
    };

    // SaveDBDocPage
    class SaveDBDocPage : public MacroMigrationPage
    {
    public:
        SaveDBDocPage(MacroMigrationDialog& _rParentDialog);
        virtual ~SaveDBDocPage();
        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

    public:
        OUString getBackupLocation() const { return m_pLocationController->getURL(); }
        void            grabLocationFocus() { m_pSaveAsLocation->GrabFocus(); }

    protected:
        ::svt::OFileURLControl*  m_pSaveAsLocation;
        PushButton*             m_pBrowseSaveAsLocation;
        FixedText*              m_pStartMigration;
        ::svx::DatabaseLocationInputController* m_pLocationController;

    protected:
        // IWizardPageController overridables
        virtual void        initializePage() SAL_OVERRIDE;
        virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) SAL_OVERRIDE;
        virtual bool        canAdvance() const SAL_OVERRIDE;

    private:
        DECL_LINK( OnLocationModified, Edit* );
        void impl_updateLocationDependentItems();
    };

    // ProgressPage
    class ProgressPage : public MacroMigrationPage, public IMigrationProgress
    {
    public:
        ProgressPage(vcl::Window *pParent);

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void    setDocumentCounts( const sal_Int32 _nForms, const sal_Int32 _nReports );
        void    onFinishedSuccessfully();

    protected:
        // IMigrationProgress
        virtual void    startObject( const OUString& _rObjectName, const OUString& _rCurrentAction, const sal_uInt32 _bRange ) SAL_OVERRIDE;
        virtual void    setObjectProgressText( const OUString& _rText ) SAL_OVERRIDE;
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue ) SAL_OVERRIDE;
        virtual void    endObject() SAL_OVERRIDE;
        virtual void    start( const sal_uInt32 _nOverallRange ) SAL_OVERRIDE;
        virtual void    setOverallProgressText( const OUString& _rText ) SAL_OVERRIDE;
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue ) SAL_OVERRIDE;

    private:
        FixedText*          m_pObjectCount;
        FixedText*          m_pCurrentObject;
        FixedText*          m_pCurrentAction;
        RangeProgressBar    m_aCurrentProgress;
        FixedText*          m_pAllProgressText;
        RangeProgressBar    m_aAllProgress;
        FixedText*          m_pMigrationDone;
    };

    // ResultPage
    class ResultPage : public MacroMigrationPage
    {
    public:
        ResultPage(vcl::Window *pParent);

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void            displayMigrationLog( const bool _bSuccessful, const OUString& _rLog );

    private:
        FixedText*        m_pSuccessLabel;
        FixedText*        m_pFailureLabel;
        VclMultiLineEdit* m_pChanges;
    };

} // namespace dbmm

#endif // INCLUDED_DBACCESS_SOURCE_EXT_MACROMIGRATION_MACROMIGRATIONPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
