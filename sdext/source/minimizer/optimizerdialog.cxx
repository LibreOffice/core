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


#include "optimizerdialog.hxx"
#include "impoptimizer.hxx"
#include "fileopendialog.hxx"
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XModifiable.hpp>

#include <comphelper/propertyvalue.hxx>
#include <sal/macros.h>
#include <osl/time.h>
#include <vcl/errinf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <o3tl/string_view.hxx>
#include <bitmaps.hlst>

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

IntroPage::IntroPage(weld::Container* pPage, OptimizerDialog& rOptimizerDialog)
    : vcl::OWizardPage(pPage, &rOptimizerDialog, u"modules/simpress/ui/pmintropage.ui"_ustr, u"PMIntroPage"_ustr)
    , mrOptimizerDialog(rOptimizerDialog)
    , mxComboBox(m_xBuilder->weld_combo_box(u"LB_SETTINGS"_ustr))
    , mxButton(m_xBuilder->weld_button(u"STR_REMOVE"_ustr))
{
    rOptimizerDialog.SetIntroPage(this);
    mxComboBox->connect_changed(LINK(this, IntroPage, ComboBoxActionPerformed));
    mxButton->connect_clicked(LINK(this, IntroPage, ButtonActionPerformed));
}

void IntroPage::UpdateControlStates(const std::vector<OUString>& rItemList, int nSelectedItem, bool bRemoveButtonEnabled)
{
    mxComboBox->clear();
    for (const auto& a : rItemList)
        mxComboBox->append_text(a);
    mxComboBox->set_active(nSelectedItem);
    mxButton->set_sensitive(bRemoveButtonEnabled);
}

SlidesPage::SlidesPage(weld::Container* pPage, OptimizerDialog& rOptimizerDialog)
    : vcl::OWizardPage(pPage, &rOptimizerDialog, u"modules/simpress/ui/pmslidespage.ui"_ustr, u"PMSlidesPage"_ustr)
    , mrOptimizerDialog(rOptimizerDialog)
    , mxMasterSlides(m_xBuilder->weld_check_button(u"STR_DELETE_MASTER_PAGES"_ustr))
    , mxHiddenSlides(m_xBuilder->weld_check_button(u"STR_DELETE_HIDDEN_SLIDES"_ustr))
    , mxUnusedSlides(m_xBuilder->weld_check_button(u"STR_CUSTOM_SHOW"_ustr))
    , mxComboBox(m_xBuilder->weld_combo_box(u"LB_SLIDES"_ustr))
    , mxClearNodes(m_xBuilder->weld_check_button(u"STR_DELETE_NOTES_PAGES"_ustr))
{
    rOptimizerDialog.SetSlidesPage(this);
    mxMasterSlides->connect_toggled(LINK(this, SlidesPage, UnusedMasterPagesActionPerformed));
    mxHiddenSlides->connect_toggled(LINK(this, SlidesPage, UnusedHiddenSlidesActionPerformed));
    mxUnusedSlides->connect_toggled(LINK(this, SlidesPage, UnusedSlidesActionPerformed));
    mxClearNodes->connect_toggled(LINK(this, SlidesPage, DeleteNotesActionPerformed));
}

void SlidesPage::Init(const css::uno::Sequence<OUString>& rCustomShowList)
{
    mxComboBox->clear();
    for (const auto& a : rCustomShowList)
        mxComboBox->append_text(a);
    mxUnusedSlides->set_sensitive(rCustomShowList.hasElements());
}

void SlidesPage::UpdateControlStates(bool bDeleteUnusedMasterPages, bool bDeleteHiddenSlides, bool bDeleteNotesPages)
{
    mxMasterSlides->set_active(bDeleteUnusedMasterPages);
    mxHiddenSlides->set_active(bDeleteHiddenSlides);
    mxClearNodes->set_active(bDeleteNotesPages);
    mxComboBox->set_sensitive(mxUnusedSlides->get_sensitive());
}

ImagesPage::ImagesPage(weld::Container* pPage, OptimizerDialog& rOptimizerDialog)
    : vcl::OWizardPage(pPage, &rOptimizerDialog, u"modules/simpress/ui/pmimagespage.ui"_ustr, u"PMImagesPage"_ustr)
    , mrOptimizerDialog(rOptimizerDialog)
    , m_xLossLessCompression(m_xBuilder->weld_radio_button(u"STR_LOSSLESS_COMPRESSION"_ustr))
    , m_xQualityLabel(m_xBuilder->weld_label(u"STR_QUALITY"_ustr))
    , m_xQuality(m_xBuilder->weld_spin_button(u"SB_QUALITY"_ustr))
    , m_xJpegCompression(m_xBuilder->weld_radio_button(u"STR_JPEG_COMPRESSION"_ustr))
    , m_xResolution(m_xBuilder->weld_combo_box(u"LB_RESOLUTION"_ustr))
    , m_xRemoveCropArea(m_xBuilder->weld_check_button(u"STR_REMOVE_CROP_AREA"_ustr))
    , m_xEmbedLinkedGraphics(m_xBuilder->weld_check_button(u"STR_EMBED_LINKED_GRAPHICS"_ustr))
{
    rOptimizerDialog.SetImagesPage(this);
    m_xRemoveCropArea->connect_toggled(LINK(this, ImagesPage, RemoveCropAreaActionPerformed));
    m_xEmbedLinkedGraphics->connect_toggled(LINK(this, ImagesPage, EmbedLinkedGraphicsActionPerformed));
    m_xResolution->connect_changed(LINK(this, ImagesPage, ComboBoxActionPerformed));
    m_xQuality->connect_value_changed(LINK(this, ImagesPage, SpinButtonActionPerformed));

    m_xJpegCompression->connect_toggled(LINK(this, ImagesPage, CompressionActionPerformed));
    m_xLossLessCompression->connect_toggled(LINK(this, ImagesPage, CompressionActionPerformed));
}

void ImagesPage::UpdateControlStates(bool bJPEGCompression, int nJPEGQuality, bool bRemoveCropArea,
                                     int nResolution, bool bEmbedLinkedGraphics)
{
    m_xLossLessCompression->set_active(!bJPEGCompression);
    m_xJpegCompression->set_active(bJPEGCompression);
    m_xQualityLabel->set_sensitive(bJPEGCompression);
    m_xQuality->set_sensitive(bJPEGCompression);
    m_xQuality->set_value(nJPEGQuality);
    m_xResolution->set_active_id(OUString::number(nResolution));
    m_xRemoveCropArea->set_active(bRemoveCropArea);
    m_xEmbedLinkedGraphics->set_active(bEmbedLinkedGraphics);
}

ObjectsPage::ObjectsPage(weld::Container* pPage, OptimizerDialog& rOptimizerDialog)
    : vcl::OWizardPage(pPage, &rOptimizerDialog, u"modules/simpress/ui/pmobjectspage.ui"_ustr, u"PMObjectsPage"_ustr)
    , mrOptimizerDialog(rOptimizerDialog)
    , m_xCreateStaticImage(m_xBuilder->weld_check_button(u"STR_OLE_REPLACE"_ustr))
    , m_xAllOLEObjects(m_xBuilder->weld_radio_button(u"STR_ALL_OLE_OBJECTS"_ustr))
    , m_xForeignOLEObjects(m_xBuilder->weld_radio_button(u"STR_ALIEN_OLE_OBJECTS_ONLY"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"STR_OLE_OBJECTS_DESC"_ustr))
{
    rOptimizerDialog.SetObjectsPage(this);
    m_xCreateStaticImage->connect_toggled(LINK(this, ObjectsPage, OLEOptimizationActionPerformed));
    m_xAllOLEObjects->connect_toggled(LINK(this, ObjectsPage, OLEActionPerformed));
    m_xForeignOLEObjects->connect_toggled(LINK(this, ObjectsPage, OLEActionPerformed));
}

void ObjectsPage::Init(const OUString& rDesc)
{
    m_xLabel->set_label(rDesc);
}

void ObjectsPage::UpdateControlStates(bool bConvertOLEObjects, int nOLEOptimizationType)
{
    m_xCreateStaticImage->set_active(bConvertOLEObjects);
    m_xAllOLEObjects->set_sensitive(bConvertOLEObjects);
    m_xForeignOLEObjects->set_sensitive(bConvertOLEObjects);
    m_xAllOLEObjects->set_active(nOLEOptimizationType == 0);
    m_xForeignOLEObjects->set_active(nOLEOptimizationType == 1);
}

SummaryPage::SummaryPage(weld::Container* pPage, OptimizerDialog& rOptimizerDialog)
    : vcl::OWizardPage(pPage, &rOptimizerDialog, u"modules/simpress/ui/pmsummarypage.ui"_ustr, u"PMSummaryPage"_ustr)
    , mrOptimizerDialog(rOptimizerDialog)
    , m_xLabel1(m_xBuilder->weld_label(u"LABEL1"_ustr))
    , m_xLabel2(m_xBuilder->weld_label(u"LABEL2"_ustr))
    , m_xLabel3(m_xBuilder->weld_label(u"LABEL3"_ustr))
    , m_xCurrentSize(m_xBuilder->weld_label(u"CURRENT_FILESIZE"_ustr))
    , m_xEstimatedSize(m_xBuilder->weld_label(u"ESTIMATED_FILESIZE"_ustr))
    , m_xStatus(m_xBuilder->weld_label(u"STR_STATUS"_ustr))
    , m_xProgress(m_xBuilder->weld_progress_bar(u"PROGRESS"_ustr))
    , m_xApplyToCurrent(m_xBuilder->weld_radio_button(u"STR_APPLY_TO_CURRENT"_ustr))
    , m_xSaveToNew(m_xBuilder->weld_radio_button(u"STR_SAVE_AS"_ustr))
    , m_xComboBox(m_xBuilder->weld_combo_box(u"MY_SETTINGS"_ustr))
    , m_xSaveSettings(m_xBuilder->weld_check_button(u"STR_SAVE_SETTINGS"_ustr))
{
    rOptimizerDialog.SetSummaryPage(this);
    m_xApplyToCurrent->connect_toggled(LINK(this, SummaryPage, SaveAsNewActionPerformed));
    m_xSaveToNew->connect_toggled(LINK(this, SummaryPage, SaveAsNewActionPerformed));
    m_xSaveSettings->connect_toggled(LINK(this, SummaryPage, SaveSettingsActionPerformed));
}

void SummaryPage::Init(const OUString& rSettingsName, bool bIsReadonly)
{
    m_xComboBox->set_entry_text(rSettingsName);
    m_xApplyToCurrent->set_sensitive(!bIsReadonly);
    m_xSaveToNew->set_sensitive(!bIsReadonly);
}

void SummaryPage::UpdateControlStates(bool bSaveAs, bool bSaveSettingsEnabled,
                                      const std::vector<OUString>& rItemList,
                                      const std::vector<OUString>& rSummaryStrings,
                                      const OUString& rCurrentFileSize,
                                      const OUString& rEstimatedFileSize)
{
    m_xApplyToCurrent->set_active(!bSaveAs);
    m_xSaveToNew->set_active(bSaveAs);

    for (const auto& a : rItemList)
        m_xComboBox->append_text(a);

    m_xSaveSettings->set_sensitive(bSaveSettingsEnabled);
    m_xComboBox->set_sensitive(bSaveSettingsEnabled && m_xSaveSettings->get_active());

    assert(rSummaryStrings.size() == 3);
    m_xLabel1->set_label(rSummaryStrings[0]);
    m_xLabel2->set_label(rSummaryStrings[1]);
    m_xLabel3->set_label(rSummaryStrings[2]);

    m_xCurrentSize->set_label(rCurrentFileSize);
    m_xEstimatedSize->set_label(rEstimatedFileSize);
}

void SummaryPage::UpdateStatusLabel(const OUString& rStatus)
{
    m_xStatus->set_label(rStatus);
}

void SummaryPage::UpdateProgressValue(int nProgress)
{
    m_xProgress->set_percentage(nProgress);
}

void OptimizerDialog::InitDialog()
{
    set_title(getString(STR_SUN_OPTIMIZATION_WIZARD2));
}

void OptimizerDialog::InitRoadmap()
{
    declarePath(
        0,
        {ITEM_ID_INTRODUCTION,
         ITEM_ID_SLIDES,
         ITEM_ID_GRAPHIC_OPTIMIZATION,
         ITEM_ID_OLE_OPTIMIZATION,
         ITEM_ID_SUMMARY}
    );

    m_xAssistant->set_page_side_image(u"" BMP_PRESENTATION_MINIMIZER ""_ustr);
}

void OptimizerDialog::UpdateConfiguration()
{
    // page0
    OUString sTKName(mpPage0->Get_TK_Name());
    if (!sTKName.isEmpty())
        SetConfigProperty(TK_Name, Any(sTKName));

    // page1
    OUString sTKCustomShowName(mpPage1->Get_TK_CustomShowName());
    if (!sTKCustomShowName.isEmpty())
        SetConfigProperty(TK_CustomShowName, Any(sTKCustomShowName));
}

OptimizerDialog::OptimizerDialog( const Reference< XComponentContext > &rxContext, Reference< XFrame > const & rxFrame, Reference< XDispatch > const & rxStatusDispatcher )
    : vcl::RoadmapWizardMachine(Application::GetFrameWeld(rxFrame->getComponentWindow()))
    , ConfigurationAccess(rxContext)
    , mnEndStatus(RET_CANCEL)
    , mxFrame(rxFrame)
    , mxController(rxFrame->getController())
    , mxStatusDispatcher(rxStatusDispatcher)
{
    Reference< XStorable > xStorable( mxController->getModel(), UNO_QUERY_THROW );
    mbIsReadonly = xStorable->isReadonly();

    InitDialog();
    InitRoadmap();
    InitNavigationBar();
    InitPage0();
    InitPage1();
    InitPage2();
    InitPage3();
    InitPage4();

    ActivatePage();
    m_xAssistant->set_current_page(0);

    OptimizationStats aStats;
    aStats.InitializeStatusValuesFromDocument( mxController->getModel() );
    Sequence< PropertyValue > aStatusSequence( aStats.GetStatusSequence() );
    UpdateStatus( aStatusSequence );
}

OUString OptimizerDialog::getStateDisplayName(vcl::WizardTypes::WizardState nState) const
{
    switch (nState)
    {
        case ITEM_ID_INTRODUCTION:
            return getString(STR_INTRODUCTION);
        case ITEM_ID_SLIDES:
            return getString(STR_SLIDES);
        case ITEM_ID_GRAPHIC_OPTIMIZATION:
            return getString(STR_IMAGE_OPTIMIZATION);
        case ITEM_ID_OLE_OPTIMIZATION:
            return getString(STR_OLE_OBJECTS);
        case ITEM_ID_SUMMARY:
            return getString(STR_SUMMARY);
    }
    return OUString();
}

std::unique_ptr<BuilderPage> OptimizerDialog::createPage(vcl::WizardTypes::WizardState nState)
{
    OUString sIdent(OUString::number(nState));
    weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

    std::unique_ptr<vcl::OWizardPage> xRet;

    switch (nState)
    {
        case ITEM_ID_INTRODUCTION:
            xRet.reset(new IntroPage(pPageContainer, *this));
            break;
        case ITEM_ID_SLIDES:
            xRet.reset(new SlidesPage(pPageContainer, *this));
            break;
        case ITEM_ID_GRAPHIC_OPTIMIZATION:
            xRet.reset(new ImagesPage(pPageContainer, *this));
            break;
        case ITEM_ID_OLE_OPTIMIZATION:
            xRet.reset(new ObjectsPage(pPageContainer, *this));
            break;
        case ITEM_ID_SUMMARY:
            xRet.reset(new SummaryPage(pPageContainer, *this));
            break;
    }

    m_xAssistant->set_page_title(sIdent, getStateDisplayName(nState));

    return xRet;
}

OptimizerDialog::~OptimizerDialog()
{
    // not saving configuration if the dialog has been finished via cancel or close window
    if (mnEndStatus == RET_OK)
        SaveConfiguration();
}

void OptimizerDialog::execute()
{
    mnEndStatus = run();
    UpdateConfiguration();          // taking actual control settings for the configuration
}

void OptimizerDialog::UpdateControlStates( sal_Int16 nPage )
{
    switch( nPage )
    {
        case 0 : UpdateControlStatesPage0(); break;
        case 1 : UpdateControlStatesPage1(); break;
        case 2 : UpdateControlStatesPage2(); break;
        case 3 : UpdateControlStatesPage3(); break;
        case 4 : UpdateControlStatesPage4(); break;
        default:
        {
            UpdateControlStatesPage0();
            UpdateControlStatesPage1();
            UpdateControlStatesPage2();
            UpdateControlStatesPage3();
            UpdateControlStatesPage4();
        }
    }
}

void OptimizerDialog::UpdateStatus( const css::uno::Sequence< css::beans::PropertyValue >& rStatus )
{
    maStats.InitializeStatusValues( rStatus );
    const Any* pVal( maStats.GetStatusValue( TK_Status ) );
    if ( pVal )
    {
        OUString sStatus;
        if ( *pVal >>= sStatus )
        {
            mpPage4->UpdateStatusLabel(getString(TKGet(sStatus)));
        }
    }
    pVal = maStats.GetStatusValue( TK_Progress );
    if ( pVal )
    {
        sal_Int32 nProgress = 0;
        if ( *pVal >>= nProgress )
            mpPage4->UpdateProgressValue(nProgress);
    }
    pVal = maStats.GetStatusValue( TK_OpenNewDocument );
    if ( pVal )
        SetConfigProperty( TK_OpenNewDocument, *pVal );

    Application::Reschedule(true);
}

IMPL_LINK(SummaryPage, SaveAsNewActionPerformed, weld::Toggleable&, rBox, void)
{
    if (!rBox.get_active())
        return;

    const bool bSaveToNew = &rBox == m_xSaveToNew.get();
    mrOptimizerDialog.SetConfigProperty( TK_SaveAs, Any(bSaveToNew) );
}

IMPL_LINK(SummaryPage, SaveSettingsActionPerformed, weld::Toggleable&, rBox, void)
{
    m_xComboBox->set_sensitive(rBox.get_active());
}

IMPL_LINK(ObjectsPage, OLEActionPerformed, weld::Toggleable&, rBox, void)
{
    if (!rBox.get_active())
        return;

    const bool bALLOles = &rBox == m_xAllOLEObjects.get();
    sal_Int16 nInt16 = bALLOles ? 0 : 1;
    mrOptimizerDialog.SetConfigProperty( TK_OLEOptimizationType, Any( nInt16 ) );
}

IMPL_LINK(ObjectsPage, OLEOptimizationActionPerformed, weld::Toggleable&, rBox, void)
{
    const bool bOLEOptimization = rBox.get_active();
    mrOptimizerDialog.SetConfigProperty( TK_OLEOptimization, Any(bOLEOptimization) );
    m_xAllOLEObjects->set_sensitive(bOLEOptimization);
    m_xForeignOLEObjects->set_sensitive(bOLEOptimization);
}

IMPL_LINK(ImagesPage, CompressionActionPerformed, weld::Toggleable&, rBox, void)
{
    if (!rBox.get_active())
        return;

    const bool bJPEGCompression = &rBox == m_xJpegCompression.get();
    mrOptimizerDialog.SetConfigProperty(TK_JPEGCompression, Any(bJPEGCompression));
    m_xQualityLabel->set_sensitive(bJPEGCompression);
    m_xQuality->set_sensitive(bJPEGCompression);
}

IMPL_LINK(ImagesPage, RemoveCropAreaActionPerformed, weld::Toggleable&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty(TK_RemoveCropArea, Any(rBox.get_active()));
}

IMPL_LINK(ImagesPage, EmbedLinkedGraphicsActionPerformed, weld::Toggleable&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty(TK_EmbedLinkedGraphics, Any(rBox.get_active()));
}

IMPL_LINK(SlidesPage, UnusedHiddenSlidesActionPerformed, weld::Toggleable&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty(TK_DeleteHiddenSlides, Any(rBox.get_active()));
}

IMPL_LINK(SlidesPage, UnusedMasterPagesActionPerformed, weld::Toggleable&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty(TK_DeleteUnusedMasterPages, Any(rBox.get_active()));
}

IMPL_LINK(SlidesPage, DeleteNotesActionPerformed, weld::Toggleable&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty(TK_DeleteNotesPages, Any(rBox.get_active()));
}

IMPL_LINK(SlidesPage, UnusedSlidesActionPerformed, weld::Toggleable&, rBox, void)
{
    mxComboBox->set_sensitive(rBox.get_active());
}

bool OptimizerDialog::onFinish()
{
    UpdateConfiguration();

    ShowPage(ITEM_ID_SUMMARY);
    m_xPrevPage->set_sensitive(false);
    m_xNextPage->set_sensitive(false);
    m_xFinish->set_sensitive(false);
    m_xCancel->set_sensitive(false);

    // check if we have to open the FileDialog
    bool bSuccessfullyExecuted = true;
    if (mpPage4->GetSaveAsNew())
    {
        // Duplicate presentation before applying changes
        OUString aSaveAsURL;
        FileOpenDialog aFileOpenDialog(mxContext);

        // generating default file name
        OUString aName;
        Reference< XStorable > xStorable( mxController->getModel(), UNO_QUERY );
        if ( xStorable.is() && xStorable->hasLocation() )
        {
            INetURLObject aURLObj( xStorable->getLocation() );
            if ( !aURLObj.hasFinalSlash() )
            {
                // tdf#105382 uri-decode file name
                aURLObj.removeExtension(INetURLObject::LAST_SEGMENT, false);
                aName = aURLObj.getName(INetURLObject::LAST_SEGMENT, false,
                                        INetURLObject::DecodeMechanism::WithCharset);
            }
        }
        else
        {
            // If no filename, try to use model title ("Untitled 1" or something like this)
            Reference<XTitle> xTitle(
                GetFrame()->getController()->getModel(), UNO_QUERY);
            aName = xTitle->getTitle();
        }

        if (!aName.isEmpty())
        {
            aName += " " + getString(STR_FILENAME_SUFFIX);
            aFileOpenDialog.setDefaultName(aName);
        }

        if (aFileOpenDialog.execute() == dialogs::ExecutableDialogResults::OK)
        {
            aSaveAsURL = aFileOpenDialog.getURL();
            SetConfigProperty( TK_SaveAsURL, Any( aSaveAsURL ) );
            SetConfigProperty( TK_FilterName, Any( aFileOpenDialog.getFilterName() ) );
        }
        if ( aSaveAsURL.isEmpty() )
        {
            // something goes wrong...
            bSuccessfullyExecuted = false;
        }

        // waiting for 500ms
        Application::Reschedule(true);
        for ( sal_uInt32 i = osl_getGlobalTimer(); ( i + 500 ) > ( osl_getGlobalTimer() ); )
        Application::Reschedule(true);
    }
    else
    {
        // Apply changes to current presentation
        Reference<XModifiable> xModifiable(mxController->getModel(),
                                           UNO_QUERY_THROW );
        if ( xModifiable->isModified() )
        {
            SolarMutexGuard aSolarGuard;
            std::unique_ptr<weld::MessageDialog> popupDlg(Application::CreateMessageDialog(
                m_xAssistant.get(), VclMessageType::Question, VclButtonsType::YesNo,
                getString(STR_WARN_UNSAVED_PRESENTATION)));
            if (popupDlg->run() != RET_YES)
            {
                // Selected not "yes" ("no" or dialog was cancelled) so return to previous step
                m_xPrevPage->set_sensitive(true);
                m_xNextPage->set_sensitive(true);
                m_xFinish->set_sensitive(true);
                m_xCancel->set_sensitive(true);
                return false;
            }
        }
    }
    if ( bSuccessfullyExecuted )
    {
        // now check if we have to store a session template
        const bool bSaveSettings = mpPage4->GetSaveSettings();
        OUString aSettingsName = mpPage4->GetSettingsName();
        if (bSaveSettings && !aSettingsName.isEmpty())
        {
            std::vector< OptimizerSettings >::iterator aIter( GetOptimizerSettingsByName( aSettingsName ) );
            std::vector< OptimizerSettings >& rSettings( GetOptimizerSettings() );
            OptimizerSettings aNewSettings( rSettings[ 0 ] );
            aNewSettings.maName = aSettingsName;
            if ( aIter == rSettings.end() )
                rSettings.push_back( aNewSettings );
            else
                *aIter = aNewSettings;
        }
    }
    if ( bSuccessfullyExecuted )
    {
        URL aURL;
        aURL.Protocol = "vnd.com.sun.star.comp.PPPOptimizer:";
        aURL.Path = "optimize";

        Sequence< PropertyValue > lArguments{
            comphelper::makePropertyValue(u"Settings"_ustr, GetConfigurationSequence()),
            comphelper::makePropertyValue(u"StatusDispatcher"_ustr, GetStatusDispatcher()),
            comphelper::makePropertyValue(u"DocumentFrame"_ustr, GetFrame()),
            comphelper::makePropertyValue(u"DialogParentWindow"_ustr, m_xAssistant->GetXWindow())
        };

        ErrCode errorCode;
        try
        {
            ImpOptimizer aOptimizer(mxContext, GetFrame()->getController()->getModel());
            aOptimizer.Optimize(lArguments);
        }
        catch (css::io::IOException&)
        {
            // We always receive just ERRCODE_IO_CANTWRITE in case of problems, so no need to bother
            // about extracting error code from exception text
            errorCode = ERRCODE_IO_CANTWRITE;
        }
        catch (css::uno::Exception&)
        {
            // Other general exception
            errorCode = ERRCODE_IO_GENERAL;
        }

        if (errorCode != ERRCODE_NONE)
        {
            // Restore wizard controls
            maStats.SetStatusValue(TK_Progress, Any(static_cast<sal_Int32>(0)));
            m_xPrevPage->set_sensitive(true);
            m_xNextPage->set_sensitive(false);
            m_xFinish->set_sensitive(true);
            m_xCancel->set_sensitive(true);

            OUString aFileName;
            GetConfigProperty(TK_SaveAsURL) >>= aFileName;
            SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC, aFileName);
            ErrorHandler::HandleError(errorCode);
            return false;
        }
    }
    else
    {
        m_xPrevPage->set_sensitive(true);
        m_xNextPage->set_sensitive(false);
        m_xFinish->set_sensitive(true);
        m_xCancel->set_sensitive(true);
    }

    return vcl::RoadmapWizardMachine::onFinish();
}

IMPL_LINK_NOARG(IntroPage, ButtonActionPerformed, weld::Button&, void)
{
    // delete configuration
    OUString aSelectedItem(mxComboBox->get_active_text());
    if ( !aSelectedItem.isEmpty() )
    {
        std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName( aSelectedItem ) );
        std::vector< OptimizerSettings >& rList( mrOptimizerDialog.GetOptimizerSettings() );
        if ( aIter != rList.end() )
        {
            rList.erase( aIter );
            mrOptimizerDialog.UpdateControlStates();
        }
    }
}

IMPL_LINK(IntroPage, ComboBoxActionPerformed, weld::ComboBox&, rBox, void)
{
    OUString sActionCommand(rBox.get_active_text());
    if (!sActionCommand.isEmpty())
    {
        std::vector< OptimizerSettings >::iterator aIter( mrOptimizerDialog.GetOptimizerSettingsByName(sActionCommand) );
        std::vector< OptimizerSettings >& rList( mrOptimizerDialog.GetOptimizerSettings() );
        if ( aIter != rList.end() )
            rList[ 0 ] = *aIter;
    }
    mrOptimizerDialog.UpdateControlStates();
}

IMPL_LINK(ImagesPage, SpinButtonActionPerformed, weld::SpinButton&, rBox, void)
{
    mrOptimizerDialog.SetConfigProperty( TK_JPEGQuality, Any( static_cast<sal_Int32>(rBox.get_value()) ) );
}

IMPL_LINK_NOARG(ImagesPage, ComboBoxActionPerformed, weld::ComboBox&, void)
{
    mrOptimizerDialog.SetConfigProperty( TK_ImageResolution, Any( m_xResolution->get_active_id().toInt32() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
