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

#ifndef IMPDIALOG_HXX
#define IMPDIALOG_HXX

#include "pdffilter.hxx"

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/field.hxx"
#include "vcl/edit.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/combobox.hxx"
#include "vcl/group.hxx"
#include "vcl/pdfwriter.hxx"

#include <vcl/FilterConfigItem.hxx>

#include "sfx2/tabdlg.hxx"

#include "com/sun/star/beans/NamedValue.hpp"

// ----------------
// - ImpPDFDialog -
// ----------------

class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;
class ImpPDFTabLinksPage;

class PDFFilterResId : public ResId
{
public:
    PDFFilterResId( sal_uInt32 nId );
};

class ImplErrorDialog : public ModalDialog
{
    FixedImage      maFI;
    FixedText       maProcessText;
    ListBox         maErrors;
    FixedText       maExplanation;

    OKButton        maButton;

    DECL_LINK(SelectHdl, void *);
    public:
    ImplErrorDialog( const std::set< vcl::PDFWriter::ErrorCode >& );
    ~ImplErrorDialog();
};

////////////////////////////////////////////////////////////////////////
//class tabbed dialog
class ImpPDFTabDialog : public SfxTabDialog
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> mxContext;
    FilterConfigItem            maConfigItem;
    FilterConfigItem            maConfigI18N;

    Any                         maSelection;

    DECL_LINK(CancelHdl, void *);

protected:
//the following data are the configuration used throughout the dialog and pages
    sal_Bool                    mbIsPresentation;
    sal_Bool                    mbIsWriter;
    sal_Bool                    mbSelectionPresent;
    sal_Bool                    mbUseCTLFont;
    sal_Bool                    mbUseLosslessCompression;
    sal_Int32                   mnQuality;
    sal_Bool                    mbReduceImageResolution;
    sal_Int32                   mnMaxImageResolution;
    sal_Bool                    mbUseTaggedPDF;
    sal_Int32                   mnPDFTypeSelection;
    sal_Bool                    mbExportNotes;
    sal_Bool                    mbViewPDF;
    sal_Bool                    mbExportNotesPages;
    sal_Bool                    mbUseTransitionEffects;
    sal_Bool                    mbIsSkipEmptyPages;
    sal_Bool                    mbAddStream;
    sal_Int32                   mnFormsType;
    sal_Bool                    mbExportFormFields;
    sal_Bool                    mbAllowDuplicateFieldNames;
    sal_Bool                    mbExportBookmarks;
    sal_Bool                    mbExportHiddenSlides;
    sal_Int32                   mnOpenBookmarkLevels;

    sal_Bool                    mbHideViewerToolbar;
    sal_Bool                    mbHideViewerMenubar;
    sal_Bool                    mbHideViewerWindowControls;
    sal_Bool                    mbResizeWinToInit;
    sal_Bool                    mbCenterWindow;
    sal_Bool                    mbOpenInFullScreenMode;
    sal_Bool                    mbDisplayPDFDocumentTitle;
    sal_Int32                   mnMagnification;
    sal_Int32                   mnInitialView;
    sal_Int32                   mnZoom;
    sal_Int32                   mnInitialPage;

    sal_Int32                   mnPageLayout;
    sal_Bool                    mbFirstPageLeft;

    sal_Bool                    mbEncrypt;

    sal_Bool                    mbRestrictPermissions;
    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > maPreparedOwnerPassword;
    sal_Int32                   mnPrint;
    sal_Int32                   mnChangesAllowed;
    sal_Bool                    mbCanCopyOrExtract;
    sal_Bool                    mbCanExtractForAccessibility;
    com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder > mxPreparedPasswords;

    sal_Bool                    mbIsRangeChecked;
    OUString               msPageRange;
    sal_Bool                    mbSelectionIsChecked;

    sal_Bool                    mbExportRelativeFsysLinks;
    sal_Int32                   mnViewPDFMode;
    sal_Bool                    mbConvertOOoTargets;
    sal_Bool                    mbExportBmkToPDFDestination;

    sal_Bool                    mbSignPDF;
    OUString             msSignPassword;
    OUString             msSignLocation;
    OUString             msSignContact;
    OUString             msSignReason;
    com::sun::star::uno::Reference< com::sun::star::security::XCertificate > maSignCertificate;

    OUString             maWatermarkText;

public:

    friend class                ImpPDFTabGeneralPage;
    friend class                ImpPDFTabViewerPage;
    friend class                ImpPDFTabOpnFtrPage;
    friend class                ImpPDFTabSecurityPage;
    friend class                ImpPDFTabLinksPage;
    friend class                ImpPDFTabSigningPage;

    ImpPDFTabDialog( Window* pParent,
                     Sequence< PropertyValue >& rFilterData,
                     const Reference< XComponent >& rDoc
                     );
    ~ImpPDFTabDialog();

    Sequence< PropertyValue >   GetFilterData();

protected:
    virtual void                PageCreated( sal_uInt16 _nId,
                                             SfxTabPage& _rPage );
    virtual short               Ok();
};

//class tab page general
class ImpPDFTabGeneralPage : public SfxTabPage
{
    friend class                ImpPDFTabLinksPage;

    FixedLine                   maFlPages;
    RadioButton                 maRbAll;
    RadioButton                 maRbRange;
    RadioButton                 maRbSelection;
    Edit                        maEdPages;

    FixedLine                   maFlCompression;
    RadioButton                 maRbLosslessCompression;
    RadioButton                 maRbJPEGCompression;
    FixedText                   maFtQuality;
    MetricField                 maNfQuality;
    CheckBox                    maCbReduceImageResolution;
    ComboBox                    maCoReduceImageResolution;

    FixedLine m_aVerticalLine;

    FixedLine                   maFlGeneral;
    CheckBox                    maCbPDFA1b;
    CheckBox                    maCbTaggedPDF;
    sal_Bool                    mbTaggedPDFUserSelection;

    CheckBox                    maCbExportFormFields;
    sal_Bool                    mbExportFormFieldsUserSelection;
    FixedText                   maFtFormsFormat;
    ListBox                     maLbFormsFormat;
    CheckBox                    maCbAllowDuplicateFieldNames;

    CheckBox                    maCbExportBookmarks;
    CheckBox                    maCbExportHiddenSlides;
    CheckBox                    maCbExportNotes;
    CheckBox                    maCbViewPDF;
    CheckBox                    maCbExportNotesPages;

    CheckBox                    maCbExportEmptyPages;
    CheckBox                    maCbAddStream;
    FixedText                   maFtAddStreamDescription;

    FixedLine                   maFlWatermark;
    CheckBox                    maCbWatermark;
    FixedText                   maFtWatermark;
    Edit                        maEdWatermark;

    sal_Bool                    mbIsPresentation;
    sal_Bool                    mbIsWriter;

const ImpPDFTabDialog*          mpaParent;

    DECL_LINK( TogglePagesHdl, void* );
    DECL_LINK( ToggleCompressionHdl, void* );
    DECL_LINK( ToggleReduceImageResolutionHdl, void* );
    DECL_LINK( ToggleWatermarkHdl, void* );
    DECL_LINK( ToggleAddStreamHdl, void* );
    DECL_LINK( ToggleExportFormFieldsHdl, void* );

public:
    DECL_LINK( ToggleExportPDFAHdl, void* );

    ImpPDFTabGeneralPage( Window* pParent,
                          const SfxItemSet& rSet );

    ~ImpPDFTabGeneralPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent );
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    sal_Bool                    IsPdfaSelected() { return maCbPDFA1b.IsChecked(); };
};

//class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
    FixedLine                   maFlInitialView;
    RadioButton                 maRbOpnPageOnly;
    RadioButton                 maRbOpnOutline;
    RadioButton                 maRbOpnThumbs;
    FixedText                   maFtInitialPage;
    NumericField                maNumInitialPage;

    FixedLine                   maFlMagnification;
    RadioButton                 maRbMagnDefault;
    RadioButton                 maRbMagnFitWin;
    RadioButton                 maRbMagnFitWidth;
    RadioButton                 maRbMagnFitVisible;
    RadioButton                 maRbMagnZoom;
    MetricField                 maNumZoom;

    FixedLine m_aVerticalLine;

    FixedLine                   maFlPageLayout;
    RadioButton                 maRbPgLyDefault;
    RadioButton                 maRbPgLySinglePage;
    RadioButton                 maRbPgLyContinue;
    RadioButton                 maRbPgLyContinueFacing;
    CheckBox                    maCbPgLyFirstOnLeft;

    sal_Bool                    mbUseCTLFont;

    DECL_LINK( ToggleRbPgLyContinueFacingHdl, void* );
    DECL_LINK( ToggleRbMagnHdl, void* );

public:
    ImpPDFTabOpnFtrPage( Window* pParent,
                         const SfxItemSet& rSet );

    ~ImpPDFTabOpnFtrPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class tab page viewer
class ImpPDFTabViewerPage : public SfxTabPage
{
    CheckBox*                   m_pCbResWinInit;
    CheckBox*                   m_pCbCenterWindow;
    CheckBox*                   m_pCbOpenFullScreen;
    CheckBox*                   m_pCbDispDocTitle;


    CheckBox*                   m_pCbHideViewerMenubar;
    CheckBox*                   m_pCbHideViewerToolbar;
    CheckBox*                   m_pCbHideViewerWindowControls;

    CheckBox*                   m_pCbTransitionEffects;
    sal_Bool                    mbIsPresentation;

    RadioButton*                m_pRbAllBookmarkLevels;
    RadioButton*                m_pRbVisibleBookmarkLevels;
    NumericField*               m_pNumBookmarkLevels;

    DECL_LINK( ToggleRbBookmarksHdl, void* );
public:
    ImpPDFTabViewerPage( Window* pParent,
                         const SfxItemSet& rSet );

    ~ImpPDFTabViewerPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class security tab page
class ImpPDFTabSecurityPage : public SfxTabPage
{
    FixedLine                   maFlGroup;
    PushButton                  maPbSetPwd;
    FixedText                   maFtUserPwd;
    String                      maUserPwdSet;
    String                      maUserPwdUnset;
    String                      maUserPwdPdfa;
    String                      maStrSetPwd;

    FixedText                   maFtOwnerPwd;
    String                      maOwnerPwdSet;
    String                      maOwnerPwdUnset;
    String                      maOwnerPwdPdfa;

    FixedLine m_aVerticalLine;

    FixedLine                   maFlPrintPermissions;
    RadioButton                 maRbPrintNone;
    RadioButton                 maRbPrintLowRes;
    RadioButton                 maRbPrintHighRes;

    FixedLine                   maFlChangesAllowed;
    RadioButton                 maRbChangesNone;
    RadioButton                 maRbChangesInsDel;
    RadioButton                 maRbChangesFillForm;
    RadioButton                 maRbChangesComment;
    RadioButton                 maRbChangesAnyNoCopy;

    CheckBox                    maCbEnableCopy;
    CheckBox                    maCbEnableAccessibility;

    String                      msUserPwdTitle;

    bool                        mbHaveOwnerPassword;
    bool                        mbHaveUserPassword;
    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > maPreparedOwnerPassword;
    String                      msOwnerPwdTitle;

    com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder > mxPreparedPasswords;

    long nWidth;

    DECL_LINK( ClickmaPbSetPwdHdl, void* );

    void enablePermissionControls();

    void                        ImplPwdPushButton( const String &, String & );

public:
    ImpPDFTabSecurityPage( Window* pParent,
                           const SfxItemSet& rSet );

    ~ImpPDFTabSecurityPage();
    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void    ImplPDFASecurityControl( sal_Bool bEnableSecurity );
    bool    hasPassword() const { return mbHaveOwnerPassword || mbHaveUserPassword; }
};

//class to implement the relative link stuff
class ImpPDFTabLinksPage : public SfxTabPage
{
    CheckBox*                   m_pCbExprtBmkrToNmDst;
    CheckBox*                   m_pCbOOoToPDFTargets;
    CheckBox*                   m_pCbExportRelativeFsysLinks;

    RadioButton*                m_pRbOpnLnksDefault;
    sal_Bool                    mbOpnLnksDefaultUserState;
    RadioButton*                m_pRbOpnLnksLaunch;
    sal_Bool                    mbOpnLnksLaunchUserState;
    RadioButton*                m_pRbOpnLnksBrowser;
    sal_Bool                    mbOpnLnksBrowserUserState;

    DECL_LINK( ClickRbOpnLnksDefaultHdl, void* );
    DECL_LINK( ClickRbOpnLnksBrowserHdl, void* );

public:
    ImpPDFTabLinksPage( Window* pParent,
                           const SfxItemSet& rSet );

    ~ImpPDFTabLinksPage();
    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void    ImplPDFALinkControl( sal_Bool bEnableLaunch );
};

//class to implement the digital signing
class ImpPDFTabSigningPage : public SfxTabPage
{
    FixedText                   maFtSignCert;
    Edit                        maEdSignCert;
    PushButton                  maPbSignCertSelect;
    PushButton                  maPbSignCertClear;
    FixedText                   maFtSignPassword;
    Edit                        maEdSignPassword;
    FixedText                   maFtSignLocation;
    Edit                        maEdSignLocation;
    FixedText                   maFtSignContactInfo;
    Edit                        maEdSignContactInfo;
    FixedText                   maFtSignReason;
    Edit                        maEdSignReason;
    com::sun::star::uno::Reference< com::sun::star::security::XCertificate > maSignCertificate;

    DECL_LINK( ClickmaPbSignCertSelect, void* );
    DECL_LINK( ClickmaPbSignCertClear, void* );

public:
    ImpPDFTabSigningPage( Window* pParent,
                          const SfxItemSet& rSet );

    ~ImpPDFTabSigningPage();
    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

#endif // IMPDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
