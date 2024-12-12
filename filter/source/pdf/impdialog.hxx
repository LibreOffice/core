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

#pragma once

#include <sfx2/tabdlg.hxx>
#include <sfx2/passwd.hxx>

#include <vcl/pdfwriter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>

class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;
class ImpPDFTabLinksPage;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

class ImplErrorDialog : public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::TreeView> m_xErrors;
    std::unique_ptr<weld::Label> m_xExplanation;

    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    explicit ImplErrorDialog(weld::Window* pParent, const std::set<vcl::PDFWriter::ErrorCode>& rErrorCodes);
};


class ImpPDFTabSecurityPage;
class ImpPDFTabLinksPage;


/// Class tabbed dialog
class ImpPDFTabDialog final : public SfxTabDialogController
{
    css::uno::Reference<css::lang::XComponent> mrDoc;

    FilterConfigItem maConfigItem;
    FilterConfigItem maConfigI18N;

    Any maSelection;

    DECL_LINK(CancelHdl, weld::Button&, void);
    DECL_LINK(OkHdl, weld::Button&, void);

    // the following data are the configuration used throughout the dialog and pages
    bool mbIsPresentation = false;
    bool mbIsSpreadsheet = false;
    bool mbIsWriter = false;
    bool mbSelectionPresent = false;
    bool mbUseCTLFont = false;
    bool mbUseLosslessCompression = true;
    sal_Int32 mnQuality = 90;
    bool mbReduceImageResolution = false;
    sal_Int32 mnMaxImageResolution = 300;
    bool mbUseTaggedPDF = false;
    bool mbUseTaggedPDFUserSelection = false;
    sal_Int32 mnPDFTypeSelection;
    bool mbPDFUACompliance;
    bool mbExportNotes = true;
    bool mbExportNotesInMargin = false;
    bool mbViewPDF = false;
    bool mbUseReferenceXObject = false;
    bool mbUseReferenceXObjectUserSelection = false;
    bool mbExportNotesPages = false;
    bool mbExportOnlyNotesPages = false;
    bool mbUseTransitionEffects = false;
    bool mbIsSkipEmptyPages = true;
    bool mbIsExportPlaceholders = false;
    bool mbAddStream = false;
    sal_Int32 mnFormsType = 0;
    bool mbExportFormFields = false;
    bool mbAllowDuplicateFieldNames = false;
    bool mbExportBookmarks = true;
    bool mbExportBookmarksUserSelection = true;
    bool mbExportHiddenSlides = false;
    bool mbSinglePageSheets = false;
    sal_Int32 mnOpenBookmarkLevels = -1;

    bool mbHideViewerToolbar = false;
    bool mbHideViewerMenubar = false;
    bool mbHideViewerWindowControls = false;
    bool mbResizeWinToInit = false;
    bool mbCenterWindow = false;
    bool mbOpenInFullScreenMode = false;
    bool mbDisplayPDFDocumentTitle = false;
    sal_Int32 mnMagnification = 0;
    sal_Int32 mnInitialView = 1;
    sal_Int32 mnInitialViewUserSelection;
    sal_Int32 mnZoom = 0;
    sal_Int32 mnInitialPage = 1;

    sal_Int32 mnPageLayout = 0;
    bool mbFirstPageLeft = false;

    bool mbEncrypt = false;

    bool mbRestrictPermissions = false;
    css::uno::Sequence< css::beans::NamedValue > maPreparedOwnerPassword;
    sal_Int32 mnPrint = 0;
    sal_Int32 mnChangesAllowed = 0;
    bool mbCanCopyOrExtract = false;
    bool mbCanExtractForAccessibility = true;
    css::uno::Reference< css::beans::XMaterialHolder > mxPreparedPasswords;

    bool mbIsPageRangeChecked = false;
    OUString msPageRange = OUString(' ');
    bool mbIsSheetRangeChecked = false;
    OUString msSheetRange = OUString(' ');
    bool mbSelectionIsChecked = false;

    bool mbExportRelativeFsysLinks = false;
    sal_Int32 mnViewPDFMode = 0;
    bool mbConvertOOoTargets = false;
    bool mbExportBmkToPDFDestination = false;

    bool mbSignPDF = false;
    OUString msSignPassword;
    OUString msSignLocation;
    OUString msSignContact;
    OUString msSignReason;
    css::uno::Reference<css::security::XCertificate> maSignCertificate;
    OUString msSignTSA;

    OUString maWatermarkText;

public:

    friend class                ImpPDFTabGeneralPage;
    friend class                ImpPDFTabViewerPage;
    friend class                ImpPDFTabOpnFtrPage;
    friend class                ImpPDFTabSecurityPage;
    friend class                ImpPDFTabLinksPage;
    friend class                ImpPDFTabSigningPage;

    ImpPDFTabDialog(weld::Window* pParent, const Sequence< PropertyValue >& rFilterData,
                    const css::uno::Reference< XComponent >& rDoc);
    virtual                     ~ImpPDFTabDialog() override;

    Sequence< PropertyValue >   GetFilterData();

    ImpPDFTabOpnFtrPage*        getOpenPage() const;
    ImpPDFTabSecurityPage*      getSecurityPage() const;
    ImpPDFTabLinksPage*         getLinksPage() const;
    ImpPDFTabGeneralPage*       getGeneralPage() const;

private:
    virtual void                PageCreated(const OUString& rId, SfxTabPage& rPage) override;
};


/// Class tab page general
class ImpPDFTabGeneralPage : public SfxTabPage
{
    friend class ImpPDFTabLinksPage;

    bool                         mbUseTaggedPDFUserSelection;
    bool                         mbIsPresentation;
    bool                         mbIsSpreadsheet;
    bool                         mbIsWriter;
    ImpPDFTabDialog*             mpParent;

    std::unique_ptr<weld::RadioButton> mxRbAll;
    std::unique_ptr<weld::RadioButton> mxRbPageRange;
    std::unique_ptr<weld::RadioButton> mxRbSheetRange;
    std::unique_ptr<weld::RadioButton> mxRbSelection;
    std::unique_ptr<weld::Entry> mxEdPages;
    std::unique_ptr<weld::Entry> mxEdSheets;
    std::unique_ptr<weld::RadioButton> mxRbLosslessCompression;
    std::unique_ptr<weld::RadioButton> mxRbJPEGCompression;
    std::unique_ptr<weld::Widget> mxQualityFrame;
    std::unique_ptr<weld::MetricSpinButton> mxNfQuality;
    std::unique_ptr<weld::CheckButton> mxCbReduceImageResolution;
    std::unique_ptr<weld::ComboBox> mxCoReduceImageResolution;
    std::unique_ptr<weld::CheckButton> mxCbPDFUA;
    std::unique_ptr<weld::ComboBox> mxRbPDFVersion;
    std::unique_ptr<weld::CheckButton> mxCbTaggedPDF;
    std::unique_ptr<weld::CheckButton> mxCbExportFormFields;
    std::unique_ptr<weld::Widget> mxFormsFrame;
    std::unique_ptr<weld::ComboBox> mxLbFormsFormat;
    std::unique_ptr<weld::CheckButton> mxCbAllowDuplicateFieldNames;
    std::unique_ptr<weld::CheckButton> mxCbExportBookmarks;
    std::unique_ptr<weld::CheckButton> mxCbExportHiddenSlides;
    std::unique_ptr<weld::CheckButton> mxCbSinglePageSheets;
    std::unique_ptr<weld::CheckButton> mxCbExportNotes;
    std::unique_ptr<weld::CheckButton> mxCbExportNotesInMargin;
    std::unique_ptr<weld::CheckButton> mxCbViewPDF;
    std::unique_ptr<weld::CheckButton> mxCbUseReferenceXObject;
    std::unique_ptr<weld::CheckButton> mxCbExportNotesPages;
    std::unique_ptr<weld::CheckButton> mxCbExportOnlyNotesPages;
    std::unique_ptr<weld::CheckButton> mxCbExportEmptyPages;
    std::unique_ptr<weld::CheckButton> mxCbExportPlaceholders;
    std::unique_ptr<weld::CheckButton> mxCbAddStream;
    std::unique_ptr<weld::CheckButton> mxCbWatermark;
    std::unique_ptr<weld::Label> mxFtWatermark;
    std::unique_ptr<weld::Entry> mxEdWatermark;
    std::unique_ptr<weld::Label> mxSlidesFt;
    std::unique_ptr<weld::Label> mxSheetsSelectionFt;

    DECL_LINK(ToggleAllHdl, weld::Toggleable&, void);
    DECL_LINK(TogglePagesHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleSheetsHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleSelectionHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleCompressionHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleReduceImageResolutionHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleWatermarkHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleAddStreamHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleExportFormFieldsHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleExportNotesPagesHdl, weld::Toggleable&, void);

    void                        TogglePagesHdl();
    void                        ToggleSheetsHdl();
    void                        EnableExportNotesPages();

    DECL_LINK(TogglePDFUniversalAccessibilityHandle, weld::Toggleable&, void);
    DECL_LINK(SelectPDFVersion, weld::ComboBox&, void);

    std::shared_ptr<weld::MessageDialog> mxPasswordUnusedWarnDialog;

    bool IsReadOnlyProperty(const OUString& rPropertyName) const
    {
        return mpParent && mpParent->maConfigItem.IsReadOnly(rPropertyName);
    }

    void thePDFVersionChanged();

public:

    ImpPDFTabGeneralPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabGeneralPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem(ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem(ImpPDFTabDialog* paParent);
    bool IsPdfaSelected() const;
    bool IsPDFAVersionSelected(sal_Int32 nVersion) const;
    bool IsPdfUaSelected() const { return mxCbPDFUA->get_active(); }
};

/// Class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
    friend class ImpPDFTabGeneralPage;

    bool                        mbUseCTLFont;

    std::unique_ptr<weld::RadioButton> mxRbOpnPageOnly;
    std::unique_ptr<weld::RadioButton> mxRbOpnOutline;
    std::unique_ptr<weld::RadioButton> mxRbOpnThumbs;
    std::unique_ptr<weld::SpinButton> mxNumInitialPage;
    std::unique_ptr<weld::RadioButton> mxRbMagnDefault;
    std::unique_ptr<weld::RadioButton> mxRbMagnFitWin;
    std::unique_ptr<weld::RadioButton> mxRbMagnFitWidth;
    std::unique_ptr<weld::RadioButton> mxRbMagnFitVisible;
    std::unique_ptr<weld::RadioButton> mxRbMagnZoom;
    std::unique_ptr<weld::SpinButton> mxNumZoom;
    std::unique_ptr<weld::RadioButton> mxRbPgLyDefault;
    std::unique_ptr<weld::RadioButton> mxRbPgLySinglePage;
    std::unique_ptr<weld::RadioButton> mxRbPgLyContinue;
    std::unique_ptr<weld::RadioButton> mxRbPgLyContinueFacing;
    std::unique_ptr<weld::CheckButton> mxCbPgLyFirstOnLeft;

    DECL_LINK(ToggleRbPgLyContinueFacingHdl, weld::Toggleable&, void);
    DECL_LINK(ToggleRbMagnHdl, weld::Toggleable&, void);

    void                        ToggleRbPgLyContinueFacingHdl();
    void ToggleInitialView(ImpPDFTabDialog & rParent);

public:
    ImpPDFTabOpnFtrPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabOpnFtrPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem(ImpPDFTabDialog* pParent);
};

/// Class tab page viewer
class ImpPDFTabViewerPage : public SfxTabPage
{
    bool                        mbIsPresentation;

    std::unique_ptr<weld::CheckButton> m_xCbResWinInit;
    std::unique_ptr<weld::CheckButton> m_xCbCenterWindow;
    std::unique_ptr<weld::CheckButton> m_xCbOpenFullScreen;
    std::unique_ptr<weld::CheckButton> m_xCbDispDocTitle;
    std::unique_ptr<weld::CheckButton> m_xCbHideViewerMenubar;
    std::unique_ptr<weld::CheckButton> m_xCbHideViewerToolbar;
    std::unique_ptr<weld::CheckButton> m_xCbHideViewerWindowControls;
    std::unique_ptr<weld::CheckButton> m_xCbTransitionEffects;
    std::unique_ptr<weld::RadioButton> m_xRbAllBookmarkLevels;
    std::unique_ptr<weld::RadioButton> m_xRbVisibleBookmarkLevels;
    std::unique_ptr<weld::SpinButton>m_xNumBookmarkLevels;

    DECL_LINK(ToggleRbBookmarksHdl, weld::Toggleable&, void);

public:
    ImpPDFTabViewerPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabViewerPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

/// Class security tab page
class ImpPDFTabSecurityPage : public SfxTabPage
{
    OUString                    msStrSetPwd;
    OUString                    msUserPwdTitle;
    bool                        mbHaveOwnerPassword;
    bool                        mbHaveUserPassword;
    css::uno::Sequence< css::beans::NamedValue > maPreparedOwnerPassword;
    OUString                    msOwnerPwdTitle;

    css::uno::Reference< css::beans::XMaterialHolder > mxPreparedPasswords;

    std::unique_ptr<weld::Button> mxPbSetPwd;
    std::unique_ptr<weld::Widget> mxUserPwdSet;
    std::unique_ptr<weld::Widget> mxUserPwdUnset;
    std::unique_ptr<weld::Widget> mxUserPwdPdfa;
    std::unique_ptr<weld::Widget> mxOwnerPwdSet;
    std::unique_ptr<weld::Widget> mxOwnerPwdUnset;
    std::unique_ptr<weld::Widget> mxOwnerPwdPdfa;
    std::unique_ptr<weld::Widget> mxPrintPermissions;
    std::unique_ptr<weld::RadioButton> mxRbPrintNone;
    std::unique_ptr<weld::RadioButton> mxRbPrintLowRes;
    std::unique_ptr<weld::RadioButton> mxRbPrintHighRes;
    std::unique_ptr<weld::Widget> mxChangesAllowed;
    std::unique_ptr<weld::RadioButton> mxRbChangesNone;
    std::unique_ptr<weld::RadioButton> mxRbChangesInsDel;
    std::unique_ptr<weld::RadioButton> mxRbChangesFillForm;
    std::unique_ptr<weld::RadioButton> mxRbChangesComment;
    std::unique_ptr<weld::RadioButton> mxRbChangesAnyNoCopy;
    std::unique_ptr<weld::Widget> mxContent;
    std::unique_ptr<weld::CheckButton> mxCbEnableCopy;
    std::unique_ptr<weld::CheckButton> mxCbEnableAccessibility;
    std::unique_ptr<weld::Label> mxPasswordTitle;
    std::unique_ptr<weld::Label> mxPermissionTitle;

    std::shared_ptr< SfxPasswordDialog > mpPasswordDialog;
    std::shared_ptr< weld::MessageDialog > mpUnsupportedMsgDialog;

    DECL_LINK(ClickmaPbSetPwdHdl, weld::Button&, void);

    void                        enablePermissionControls();

public:
    ImpPDFTabSecurityPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabSecurityPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void                        ImplPDFASecurityControl();
    bool                        hasPassword() const { return mbHaveOwnerPassword || mbHaveUserPassword; }
};

/// Implements the relative link stuff
class ImpPDFTabLinksPage : public SfxTabPage
{
    bool                        mbOpnLnksDefaultUserState;
    bool                        mbOpnLnksLaunchUserState;
    bool                        mbOpnLnksBrowserUserState;

    std::unique_ptr<weld::CheckButton> m_xCbExprtBmkrToNmDst;
    std::unique_ptr<weld::CheckButton> m_xCbOOoToPDFTargets;
    std::unique_ptr<weld::CheckButton> m_xCbExportRelativeFsysLinks;
    std::unique_ptr<weld::RadioButton> m_xRbOpnLnksDefault;
    std::unique_ptr<weld::RadioButton> m_xRbOpnLnksLaunch;
    std::unique_ptr<weld::RadioButton> m_xRbOpnLnksBrowser;

    DECL_LINK(ClickRbOpnLnksDefaultHdl, weld::Toggleable&, void);
    DECL_LINK(ClickRbOpnLnksBrowserHdl, weld::Toggleable&, void);

public:
    ImpPDFTabLinksPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabLinksPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void                        ImplPDFALinkControl( bool bEnableLaunch );
};

//class to implement the digital signing
class ImpPDFTabSigningPage : public SfxTabPage
{
    css::uno::Reference< css::security::XCertificate > maSignCertificate;

    std::unique_ptr<weld::Entry> mxEdSignCert;
    std::unique_ptr<weld::Button> mxPbSignCertSelect;
    std::unique_ptr<weld::Button> mxPbSignCertClear;
    std::unique_ptr<weld::Entry> mxEdSignPassword;
    std::unique_ptr<weld::Entry> mxEdSignLocation;
    std::unique_ptr<weld::Entry> mxEdSignContactInfo;
    std::unique_ptr<weld::Entry> mxEdSignReason;
    std::unique_ptr<weld::ComboBox> mxLBSignTSA;

    DECL_LINK(ClickmaPbSignCertSelect, weld::Button&, void);
    DECL_LINK(ClickmaPbSignCertClear, weld::Button&, void);

public:
    ImpPDFTabSigningPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabSigningPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
