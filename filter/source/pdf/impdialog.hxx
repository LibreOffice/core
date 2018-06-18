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

#ifndef INCLUDED_FILTER_SOURCE_PDF_IMPDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_PDF_IMPDIALOG_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/group.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <vcl/weld.hxx>

#include "pdffilter.hxx"

#include <com/sun/star/beans/NamedValue.hpp>

class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;
class ImpPDFTabLinksPage;

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
class ImpPDFTabDialog final : public SfxTabDialog
{
    FilterConfigItem            maConfigItem;
    FilterConfigItem            maConfigI18N;

    Any                         maSelection;

    DECL_LINK(CancelHdl, Button*, void);

    sal_uInt16                  mnSigningPageId;
    sal_uInt16                  mnSecurityPageId;
    sal_uInt16                  mnLinksPage;
    sal_uInt16                  mnInterfacePageId;
    sal_uInt16                  mnViewPageId;
    sal_uInt16                  mnGeneralPageId;

    // the following data are the configuration used throughout the dialog and pages
    bool                        mbIsPresentation;
    bool                        mbIsSpreadsheet;
    bool                        mbIsWriter;
    bool                        mbSelectionPresent;
    bool                        mbUseCTLFont;
    bool                        mbUseLosslessCompression;
    sal_Int32                   mnQuality;
    bool                        mbReduceImageResolution;
    sal_Int32                   mnMaxImageResolution;
    bool                        mbUseTaggedPDF;
    sal_Int32                   mnPDFTypeSelection;
    bool                        mbExportNotes;
    bool                        mbViewPDF;
    bool                        mbUseReferenceXObject;
    bool                        mbExportNotesPages;
    bool                        mbExportOnlyNotesPages;
    bool                        mbUseTransitionEffects;
    bool                        mbIsSkipEmptyPages;
    bool                        mbIsExportPlaceholders;
    bool                        mbAddStream;
    sal_Int32                   mnFormsType;
    bool                        mbExportFormFields;
    bool                        mbAllowDuplicateFieldNames;
    bool                        mbExportBookmarks;
    bool                        mbExportHiddenSlides;
    sal_Int32                   mnOpenBookmarkLevels;

    bool                        mbHideViewerToolbar;
    bool                        mbHideViewerMenubar;
    bool                        mbHideViewerWindowControls;
    bool                        mbResizeWinToInit;
    bool                        mbCenterWindow;
    bool                        mbOpenInFullScreenMode;
    bool                        mbDisplayPDFDocumentTitle;
    sal_Int32                   mnMagnification;
    sal_Int32                   mnInitialView;
    sal_Int32                   mnZoom;
    sal_Int32                   mnInitialPage;

    sal_Int32                   mnPageLayout;
    bool                        mbFirstPageLeft;

    bool                        mbEncrypt;

    bool                        mbRestrictPermissions;
    css::uno::Sequence< css::beans::NamedValue > maPreparedOwnerPassword;
    sal_Int32                   mnPrint;
    sal_Int32                   mnChangesAllowed;
    bool                        mbCanCopyOrExtract;
    bool                        mbCanExtractForAccessibility;
    css::uno::Reference< css::beans::XMaterialHolder > mxPreparedPasswords;

    bool                        mbIsRangeChecked;
    OUString                    msPageRange;
    bool                        mbSelectionIsChecked;

    bool                        mbExportRelativeFsysLinks;
    sal_Int32                   mnViewPDFMode;
    bool                        mbConvertOOoTargets;
    bool                        mbExportBmkToPDFDestination;

    bool                        mbSignPDF;
    OUString                    msSignPassword;
    OUString                    msSignLocation;
    OUString                    msSignContact;
    OUString                    msSignReason;
    css::uno::Reference< css::security::XCertificate > maSignCertificate;
    OUString                    msSignTSA;

    OUString                    maWatermarkText;

public:

    friend class                ImpPDFTabGeneralPage;
    friend class                ImpPDFTabViewerPage;
    friend class                ImpPDFTabOpnFtrPage;
    friend class                ImpPDFTabSecurityPage;
    friend class                ImpPDFTabLinksPage;
    friend class                ImpPDFTabSigningPage;

                                ImpPDFTabDialog( vcl::Window* pParent,
                                                 Sequence< PropertyValue >& rFilterData,
                                                 const css::uno::Reference< XComponent >& rDoc);
    virtual                     ~ImpPDFTabDialog() override;

    virtual void                dispose() override;

    Sequence< PropertyValue >   GetFilterData();

    ImpPDFTabSecurityPage*      getSecurityPage() const;
    ImpPDFTabLinksPage*         getLinksPage() const;
    ImpPDFTabGeneralPage*       getGeneralPage() const;

private:
    virtual void                PageCreated( sal_uInt16 _nId, SfxTabPage& _rPage ) override;
    virtual short               Ok() override;
};


/// Class tab page general
class ImpPDFTabGeneralPage : public SfxTabPage
{
    friend class ImpPDFTabLinksPage;

    VclPtr<RadioButton>          mpRbAll;
    VclPtr<RadioButton>          mpRbRange;
    VclPtr<RadioButton>          mpRbSelection;
    VclPtr<Edit>                 mpEdPages;
    VclPtr<FixedText>            mpSelectedSheets;

    VclPtr<RadioButton>          mpRbLosslessCompression;
    VclPtr<RadioButton>          mpRbJPEGCompression;
    VclPtr<VclContainer>         mpQualityFrame;
    VclPtr<MetricField>          mpNfQuality;
    VclPtr<CheckBox>             mpCbReduceImageResolution;
    VclPtr<ComboBox>             mpCoReduceImageResolution;

    VclPtr<CheckBox>             mpCbPDFA1b;
    VclPtr<CheckBox>             mpCbTaggedPDF;
    bool                         mbTaggedPDFUserSelection;

    VclPtr<CheckBox>             mpCbExportFormFields;
    bool                         mbExportFormFieldsUserSelection;
    VclPtr<VclContainer>         mpFormsFrame;
    VclPtr<ListBox>              mpLbFormsFormat;
    VclPtr<CheckBox>             mpCbAllowDuplicateFieldNames;

    VclPtr<CheckBox>             mpCbExportBookmarks;
    VclPtr<CheckBox>             mpCbExportHiddenSlides;
    VclPtr<CheckBox>             mpCbExportNotes;
    VclPtr<CheckBox>             mpCbViewPDF;
    VclPtr<CheckBox>             mpCbUseReferenceXObject;
    VclPtr<CheckBox>             mpCbExportNotesPages;
    VclPtr<CheckBox>             mpCbExportOnlyNotesPages;

    VclPtr<CheckBox>             mpCbExportEmptyPages;
    VclPtr<CheckBox>             mpCbExportPlaceholders;
    VclPtr<CheckBox>             mpCbAddStream;

    VclPtr<CheckBox>             mpCbWatermark;
    VclPtr<FixedText>            mpFtWatermark;
    VclPtr<Edit>                mpEdWatermark;

    bool                        mbIsPresentation;
    bool                        mbIsSpreadsheet;
    bool                        mbIsWriter;

    VclPtr<ImpPDFTabDialog>     mpaParent;

    DECL_LINK( ToggleAllHdl, RadioButton&, void );
    DECL_LINK( TogglePagesHdl, RadioButton&, void );
    DECL_LINK( ToggleSelectionHdl, RadioButton&, void );
    DECL_LINK( ToggleCompressionHdl, RadioButton&, void );
    DECL_LINK( ToggleReduceImageResolutionHdl, CheckBox&, void );
    DECL_LINK( ToggleWatermarkHdl, CheckBox&, void );
    DECL_LINK( ToggleAddStreamHdl, CheckBox&, void );
    DECL_LINK( ToggleExportFormFieldsHdl, CheckBox&, void );
    DECL_LINK( ToggleExportNotesPagesHdl, CheckBox&, void );

    void                        TogglePagesHdl();
    void                        EnableExportNotesPages();

public:
    DECL_LINK( ToggleExportPDFAHdl, CheckBox&, void );

                                ImpPDFTabGeneralPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual                     ~ImpPDFTabGeneralPage() override;

    virtual void                dispose() override;
    static VclPtr<SfxTabPage>   Create( TabPageParent pParent, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem(ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem(ImpPDFTabDialog* paParent);
    bool                        IsPdfaSelected() const { return mpCbPDFA1b->IsChecked(); }
};

/// Class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
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

    DECL_LINK(ToggleRbPgLyContinueFacingHdl, weld::ToggleButton&, void);
    DECL_LINK(ToggleRbMagnHdl, weld::ToggleButton&, void);

    void                        ToggleRbPgLyContinueFacingHdl();

public:
    ImpPDFTabOpnFtrPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabOpnFtrPage() override;

    static VclPtr<SfxTabPage>   Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

/// Class tab page viewer
class ImpPDFTabViewerPage : public SfxTabPage
{
    VclPtr<CheckBox>            m_pCbResWinInit;
    VclPtr<CheckBox>            m_pCbCenterWindow;
    VclPtr<CheckBox>            m_pCbOpenFullScreen;
    VclPtr<CheckBox>            m_pCbDispDocTitle;


    VclPtr<CheckBox>            m_pCbHideViewerMenubar;
    VclPtr<CheckBox>            m_pCbHideViewerToolbar;
    VclPtr<CheckBox>            m_pCbHideViewerWindowControls;

    VclPtr<CheckBox>            m_pCbTransitionEffects;
    bool                        mbIsPresentation;

    VclPtr<RadioButton>         m_pRbAllBookmarkLevels;
    VclPtr<RadioButton>         m_pRbVisibleBookmarkLevels;
    VclPtr<NumericField>        m_pNumBookmarkLevels;

    DECL_LINK( ToggleRbBookmarksHdl, RadioButton&, void );

public:
                                ImpPDFTabViewerPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual                     ~ImpPDFTabViewerPage() override;

    virtual void                dispose() override;
    static VclPtr<SfxTabPage>   Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

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

    DECL_LINK(ClickmaPbSetPwdHdl, weld::Button&, void);

    void                        enablePermissionControls();

public:
    ImpPDFTabSecurityPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabSecurityPage() override;

    static VclPtr<SfxTabPage>   Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void                        ImplPDFASecurityControl( bool bEnableSecurity );
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

    DECL_LINK(ClickRbOpnLnksDefaultHdl, weld::ToggleButton&, void);
    DECL_LINK(ClickRbOpnLnksBrowserHdl, weld::ToggleButton&, void);

public:
    ImpPDFTabLinksPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual                     ~ImpPDFTabLinksPage() override;

    static VclPtr<SfxTabPage>   Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void                        ImplPDFALinkControl( bool bEnableLaunch );
};

//class to implement the digital signing
class ImpPDFTabSigningPage : public SfxTabPage
{
    VclPtr<Edit>                mpEdSignCert;
    VclPtr<PushButton>          mpPbSignCertSelect;
    VclPtr<PushButton>          mpPbSignCertClear;
    VclPtr<Edit>                mpEdSignPassword;
    VclPtr<Edit>                mpEdSignLocation;
    VclPtr<Edit>                mpEdSignContactInfo;
    VclPtr<Edit>                mpEdSignReason;
    VclPtr<ListBox>             mpLBSignTSA;
    css::uno::Reference< css::security::XCertificate > maSignCertificate;

    DECL_LINK( ClickmaPbSignCertSelect, Button*, void );
    DECL_LINK( ClickmaPbSignCertClear, Button*, void );

public:
                                ImpPDFTabSigningPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual                     ~ImpPDFTabSigningPage() override;

    virtual void                dispose() override;
    static VclPtr<SfxTabPage>   Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

#endif // INCLUDED_FILTER_SOURCE_PDF_IMPDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
