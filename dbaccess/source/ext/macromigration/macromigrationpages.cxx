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

#include "dbaccess_helpid.hrc"
#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "macromigration.hrc"
#include "macromigrationpages.hxx"
#include "macromigrationdialog.hxx"

#include <com/sun/star/frame/XModel.hpp>

#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/metric.hxx>

namespace dbmm
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::frame::XModel;

    MacroMigrationPage::MacroMigrationPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription)
        : MacroMigrationPage_Base(pParent, rID, rUIXMLDescription)
    {
    }

    const MacroMigrationDialog& MacroMigrationPage::getDialog() const
    {
        return *dynamic_cast< const MacroMigrationDialog* >( GetParentDialog() );
    }

    MacroMigrationDialog& MacroMigrationPage::getDialog()
    {
        return *dynamic_cast< MacroMigrationDialog* >( GetParentDialog() );
    }

    // PreparationPage
    PreparationPage::PreparationPage(vcl::Window *pParent)
        : MacroMigrationPage(pParent, "PreparePage" ,"dbaccess/ui/preparepage.ui")
    {
        get(m_pCloseDocError, "closedocerror");
    }

    PreparationPage::~PreparationPage()
    {
        disposeOnce();
    }

    void PreparationPage::dispose()
    {
        m_pCloseDocError.clear();
        MacroMigrationPage::dispose();
    }

    void PreparationPage::showCloseDocsError( bool _bShow )
    {
        m_pCloseDocError->Show( _bShow );
    }

    VclPtr<TabPage> PreparationPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return VclPtr<PreparationPage>::Create(&_rParentDialog);
    }

    // SaveDBDocPage
    SaveDBDocPage::SaveDBDocPage(MacroMigrationDialog& _rParentDialog)
        : MacroMigrationPage(&_rParentDialog, "BackupPage" ,"dbaccess/ui/backuppage.ui")
    {
        get(m_pStartMigration, "startmigrate");
        get(m_pBrowseSaveAsLocation, "browse");
        get(m_pSaveAsLocation, "location");
        m_pLocationController = new svx::DatabaseLocationInputController(
            _rParentDialog.getComponentContext(), *m_pSaveAsLocation, *m_pBrowseSaveAsLocation);

        m_pSaveAsLocation->SetModifyHdl( LINK( this, SaveDBDocPage, OnLocationModified ) );
        m_pSaveAsLocation->SetDropDownLineCount( 20 );

        impl_updateLocationDependentItems();
    }

    SaveDBDocPage::~SaveDBDocPage()
    {
        disposeOnce();
    }

    void SaveDBDocPage::dispose()
    {
        delete m_pLocationController;
        m_pSaveAsLocation.clear();
        m_pBrowseSaveAsLocation.clear();
        m_pStartMigration.clear();
        MacroMigrationPage::dispose();
    }

    void SaveDBDocPage::impl_updateLocationDependentItems()
    {
        updateDialogTravelUI();
        m_pStartMigration->Show(!m_pSaveAsLocation->GetText().isEmpty());
    }

    IMPL_LINK_NOARG_TYPED( SaveDBDocPage, OnLocationModified, Edit&, void )
    {
        impl_updateLocationDependentItems();
    }

    void SaveDBDocPage::initializePage()
    {
        OWizardPage::initializePage();

        try
        {
            // get the document's current URL
            Reference< XModel > xDocument( getDialog().getDocument(), UNO_QUERY_THROW );
            INetURLObject aURLParser( xDocument->getURL() );
            OSL_ENSURE( aURLParser.GetProtocol() != INetProtocol::NotValid, "SaveDBDocPage::initializePage: illegal document URL!" );

            OUStringBuffer aBaseName( aURLParser.getBase() );
            aBaseName.append( ".backup" );
            aURLParser.setBase( aBaseName.makeStringAndClear() );

            m_pLocationController->setURL( aURLParser.GetMainURL( INetURLObject::NO_DECODE ) );
            impl_updateLocationDependentItems();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    bool SaveDBDocPage::canAdvance() const
    {
        if ( !MacroMigrationPage::canAdvance() )
            return false;

        return !m_pSaveAsLocation->GetText().isEmpty();
    }

    bool SaveDBDocPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if ( !MacroMigrationPage::commitPage( _eReason ) )
            return false;

        if ( ::svt::WizardTypes::eTravelBackward == _eReason )
            return true;

        if ( !m_pLocationController->prepareCommit() )
            return false;

        return true;
    }

    VclPtr<TabPage> SaveDBDocPage::Create( ::svt::RoadmapWizard& _rParentDialog )
    {
        return VclPtr<SaveDBDocPage>::Create( dynamic_cast< MacroMigrationDialog& >( _rParentDialog ) );
    }

    // ProgressPage
    ProgressPage::ProgressPage(vcl::Window* pParent)
        : MacroMigrationPage(pParent, "MigratePage" ,"dbaccess/ui/migratepage.ui")
    {
        get(m_pObjectCount, "count");
        get(m_pCurrentObject, "object");
        get(m_pCurrentAction, "current");
        m_aCurrentProgress.Set(get<ProgressBar>("currentprogress"));
        get(m_pAllProgressText, "overall");
        m_aAllProgress.Set(get<ProgressBar>("allprogress"));
        get(m_pMigrationDone, "done");
    }

    ProgressPage::~ProgressPage()
    {
        disposeOnce();
    }

    void ProgressPage::dispose()
    {
        m_pObjectCount.clear();
        m_pCurrentObject.clear();
        m_pCurrentAction.clear();
        m_pAllProgressText.clear();
        m_pMigrationDone.clear();
        MacroMigrationPage::dispose();
    }

    VclPtr<TabPage> ProgressPage::Create(::svt::RoadmapWizard& _rParentDialog)
    {
        return VclPtr<ProgressPage>::Create(&_rParentDialog);
    }

    void ProgressPage::setDocumentCounts( const sal_Int32 _nForms, const sal_Int32 _nReports )
    {
        OUString sText( m_pObjectCount->GetText() );
        sText = sText.replaceFirst( "$forms$", OUString::number( _nForms ) );
        sText = sText.replaceFirst( "$reports$", OUString::number( _nReports ) );
        m_pObjectCount->SetText( sText );
    }

    void ProgressPage::onFinishedSuccessfully()
    {
        m_pMigrationDone->Show();
    }

    void ProgressPage::startObject( const OUString& _rObjectName, const OUString& _rCurrentAction, const sal_uInt32 _nRange )
    {
        m_pCurrentObject->SetText( _rObjectName );
        m_pCurrentAction->SetText( _rCurrentAction );
        m_aCurrentProgress.SetRange( _nRange );
        m_aCurrentProgress.SetValue( (sal_uInt32)0 );

        // since this is currently called from the main thread, which does not have the chance
        // to re-schedule, we need to explicitly update the display
        m_pCurrentObject->Update();
        m_pCurrentAction->Update();
        Update();
    }

    void ProgressPage::setObjectProgressText( const OUString& _rText )
    {
        m_pCurrentAction->SetText( _rText );
        m_pCurrentAction->Update();
        Update();
    }

    void ProgressPage::setObjectProgressValue( const sal_uInt32 _nValue )
    {
        m_aCurrentProgress.SetValue( _nValue );
        Update();
    }

    void ProgressPage::endObject()
    {
        m_pCurrentAction->SetText( OUString() );
        m_aCurrentProgress.SetValue( m_aCurrentProgress.GetRange() );
        m_pCurrentAction->Update();
        Update();
    }

    void ProgressPage::start( const sal_uInt32 _nOverallRange )
    {
        m_aAllProgress.SetRange( _nOverallRange );
        Update();
    }

    void ProgressPage::setOverallProgressText( const OUString& _rText )
    {
        m_pAllProgressText->SetText( _rText );
        Update();
    }

    void ProgressPage::setOverallProgressValue( const sal_uInt32 _nValue )
    {
        m_aAllProgress.SetValue( _nValue );
        Update();
    }

    // ResultPage
    ResultPage::ResultPage(vcl::Window* pParent)
        : MacroMigrationPage(pParent, "SummaryPage" ,"dbaccess/ui/summarypage.ui")
    {
        get(m_pChanges, "textview");
        m_pChanges->set_height_request(GetTextHeight() * 10);
        m_pChanges->set_width_request(approximate_char_width() * 40);
        get(m_pSuccessLabel, "success");
        get(m_pFailureLabel, "failure");
    }

    ResultPage::~ResultPage()
    {
        disposeOnce();
    }

    void ResultPage::dispose()
    {
        m_pSuccessLabel.clear();
        m_pFailureLabel.clear();
        m_pChanges.clear();
        MacroMigrationPage::dispose();
    }

    VclPtr<TabPage> ResultPage::Create(::svt::RoadmapWizard& _rParentDialog)
    {
        return VclPtr<ResultPage>::Create(&_rParentDialog);
    }

    void ResultPage::displayMigrationLog(const bool _bSuccessful, const OUString& _rSummary)
    {
        if (_bSuccessful)
            m_pFailureLabel->Hide();
        else
            m_pSuccessLabel->Hide();
        m_pChanges->SetText(_rSummary);
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
