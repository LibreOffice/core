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

//........................................................................
namespace dbmm
{
//........................................................................

    class MacroMigrationDialog;

    //====================================================================
    //= MacroMigrationPage
    //====================================================================
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

    //====================================================================
    //= PreparationPage
    //====================================================================
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

    //====================================================================
    //= SaveDBDocPage
    //====================================================================
    class SaveDBDocPage : public MacroMigrationPage
    {
    public:
        SaveDBDocPage( MacroMigrationDialog& _rParentDialog );
        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

    public:
        ::rtl::OUString getBackupLocation() const { return m_aLocationController.getURL(); }
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

    //====================================================================
    //= ProgressPage
    //====================================================================
    class ProgressPage : public MacroMigrationPage, public IMigrationProgress
    {
    public:
        ProgressPage( MacroMigrationDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void    setDocumentCounts( const sal_Int32 _nForms, const sal_Int32 _nReports );
        void    onFinishedSuccessfully();

    protected:
        // IMigrationProgress
        virtual void    startObject( const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rCurrentAction, const sal_uInt32 _bRange );
        virtual void    setObjectProgressText( const ::rtl::OUString& _rText );
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue );
        virtual void    endObject();
        virtual void    start( const sal_uInt32 _nOverallRange );
        virtual void    setOverallProgressText( const ::rtl::OUString& _rText );
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

    //====================================================================
    //= ResultPage
    //====================================================================
    class ResultPage : public MacroMigrationPage
    {
    public:
        ResultPage( MacroMigrationDialog& _rParentDialog );

        static TabPage* Create( ::svt::RoadmapWizard& _rParentDialog );

        void            displayMigrationLog( const bool _bSuccessful, const String& _rLog );

    private:
        FixedText       m_aChangesLabel;
        MultiLineEdit   m_aChanges;
        String          m_aSuccessful;
        String          m_aUnsuccessful;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MACROMIGRATIONPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
