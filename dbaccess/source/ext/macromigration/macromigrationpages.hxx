/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macromigrationpages.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:04:17 $
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

    protected:
        FixedText               m_aExplanation;
        FixedText               m_aSaveAsLabel;
        ::svt::OFileURLControl  m_aSaveAsLocation;
        PushButton              m_aBrowseSaveAsLocation;
        FixedText               m_aStartMigration;
        ::svx::DatabaseLocationInputController
                                m_aLocationController;

    protected:
        // OWizardPage overridables
        virtual void    initializePage();
        virtual bool    canAdvance() const;
        // IWizardPage overridables
        virtual sal_Bool    commitPage( CommitPageReason _eReason );

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

        void            displaySummary( const String& _rSummary );

    private:
        FixedText       m_aChangesLabel;
        MultiLineEdit   m_aChanges;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MACROMIGRATIONPAGES_HXX
