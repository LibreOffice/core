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

#include "pdffilter.hxx"

#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/field.hxx"
#include "vcl/edit.hxx"
#include "vcl/layout.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/combobox.hxx"
#include "vcl/group.hxx"
#include "vcl/pdfwriter.hxx"

#include <vcl/FilterConfigItem.hxx>

#include "sfx2/tabdlg.hxx"

#include "com/sun/star/beans/NamedValue.hpp"


// - ImpPDFDialog -


class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;
class ImpPDFTabLinksPage;

class PDFFilterResId : public ResId
{
public:
    PDFFilterResId( sal_uInt32 nId );
};

class ImplErrorDialog : public MessageDialog
{
    ListBox*        m_pErrors;
    FixedText*      m_pExplanation;

    DECL_LINK(SelectHdl, void *);
    public:
    ImplErrorDialog( const std::set< vcl::PDFWriter::ErrorCode >& );
    virtual ~ImplErrorDialog();
};

class ImpPDFTabSecurityPage;
class ImpPDFTabLinksPage;


//class tabbed dialog
class ImpPDFTabDialog : public SfxTabDialog
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext> mxContext;
    FilterConfigItem            maConfigItem;
    FilterConfigItem            maConfigI18N;

    Any                         maSelection;

    DECL_LINK(CancelHdl, void *);

    sal_uInt16 mnSigningPageId;
    sal_uInt16 mnSecurityPageId;
    sal_uInt16 mnLinksPage;
    sal_uInt16 mnInterfacePageId;
    sal_uInt16 mnViewPageId;
    sal_uInt16 mnGeneralPageId;

protected:
//the following data are the configuration used throughout the dialog and pages
    bool                    mbIsPresentation;
    bool                    mbIsWriter;
    bool                    mbSelectionPresent;
    bool                    mbUseCTLFont;
    bool                    mbUseLosslessCompression;
    sal_Int32                   mnQuality;
    bool                    mbReduceImageResolution;
    sal_Int32                   mnMaxImageResolution;
    bool                    mbUseTaggedPDF;
    sal_Int32                   mnPDFTypeSelection;
    bool                    mbExportNotes;
    bool                    mbViewPDF;
    bool                    mbExportNotesPages;
    bool                    mbUseTransitionEffects;
    bool                    mbIsSkipEmptyPages;
    bool                    mbAddStream;
    sal_Int32                   mnFormsType;
    bool                    mbExportFormFields;
    bool                    mbAllowDuplicateFieldNames;
    bool                    mbExportBookmarks;
    bool                    mbExportHiddenSlides;
    sal_Int32                   mnOpenBookmarkLevels;

    bool                    mbHideViewerToolbar;
    bool                    mbHideViewerMenubar;
    bool                    mbHideViewerWindowControls;
    bool                    mbResizeWinToInit;
    bool                    mbCenterWindow;
    bool                    mbOpenInFullScreenMode;
    bool                    mbDisplayPDFDocumentTitle;
    sal_Int32                   mnMagnification;
    sal_Int32                   mnInitialView;
    sal_Int32                   mnZoom;
    sal_Int32                   mnInitialPage;

    sal_Int32                   mnPageLayout;
    bool                    mbFirstPageLeft;

    bool                    mbEncrypt;

    bool                    mbRestrictPermissions;
    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > maPreparedOwnerPassword;
    sal_Int32                   mnPrint;
    sal_Int32                   mnChangesAllowed;
    bool                    mbCanCopyOrExtract;
    bool                    mbCanExtractForAccessibility;
    com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder > mxPreparedPasswords;

    bool                    mbIsRangeChecked;
    OUString               msPageRange;
    bool                    mbSelectionIsChecked;

    bool                    mbExportRelativeFsysLinks;
    sal_Int32                   mnViewPDFMode;
    bool                    mbConvertOOoTargets;
    bool                    mbExportBmkToPDFDestination;

    bool                    mbSignPDF;
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

    ImpPDFTabDialog( vcl::Window* pParent,
                     Sequence< PropertyValue >& rFilterData,
                     const css::uno::Reference< XComponent >& rDoc
                     );
    virtual ~ImpPDFTabDialog();

    Sequence< PropertyValue >   GetFilterData();

    ImpPDFTabSecurityPage* getSecurityPage() const;
    ImpPDFTabLinksPage* getLinksPage() const;
    ImpPDFTabGeneralPage* getGeneralPage() const;

protected:
    virtual void                PageCreated( sal_uInt16 _nId,
                                             SfxTabPage& _rPage ) SAL_OVERRIDE;
    virtual short               Ok() SAL_OVERRIDE;
};

//class tab page general
class ImpPDFTabGeneralPage : public SfxTabPage
{
    friend class  ImpPDFTabLinksPage;

    RadioButton*  mpRbAll;
    RadioButton*  mpRbRange;
    RadioButton*  mpRbSelection;
    Edit*         mpEdPages;

    RadioButton*  mpRbLosslessCompression;
    RadioButton*  mpRbJPEGCompression;
    VclContainer* mpQualityFrame;
    MetricField*  mpNfQuality;
    CheckBox*     mpCbReduceImageResolution;
    ComboBox*     mpCoReduceImageResolution;

    CheckBox*     mpCbPDFA1b;
    CheckBox*     mpCbTaggedPDF;
    bool          mbTaggedPDFUserSelection;

    CheckBox*     mpCbExportFormFields;
    bool          mbExportFormFieldsUserSelection;
    VclContainer* mpFormsFrame;
    ListBox*      mpLbFormsFormat;
    CheckBox*     mpCbAllowDuplicateFieldNames;

    CheckBox*     mpCbExportBookmarks;
    CheckBox*     mpCbExportHiddenSlides;
    CheckBox*     mpCbExportNotes;
    CheckBox*     mpCbViewPDF;
    CheckBox*     mpCbExportNotesPages;

    CheckBox*     mpCbExportEmptyPages;
    CheckBox*     mpCbAddStream;

    CheckBox*     mpCbWatermark;
    FixedText*    mpFtWatermark;
    Edit*         mpEdWatermark;

    bool          mbIsPresentation;
    bool          mbIsWriter;

    const ImpPDFTabDialog* mpaParent;

    DECL_LINK( TogglePagesHdl, void* );
    DECL_LINK( ToggleCompressionHdl, void* );
    DECL_LINK( ToggleReduceImageResolutionHdl, void* );
    DECL_LINK( ToggleWatermarkHdl, void* );
    DECL_LINK( ToggleAddStreamHdl, void* );
    DECL_LINK( ToggleExportFormFieldsHdl, void* );

public:
    DECL_LINK( ToggleExportPDFAHdl, void* );

    ImpPDFTabGeneralPage( vcl::Window* pParent,
                          const SfxItemSet& rSet );

    virtual ~ImpPDFTabGeneralPage();
    static SfxTabPage*          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet);

    void    GetFilterConfigItem(ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem(const ImpPDFTabDialog* paParent);
    bool    IsPdfaSelected() const { return mpCbPDFA1b->IsChecked(); }
};

//class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
    RadioButton*                mpRbOpnPageOnly;
    RadioButton*                mpRbOpnOutline;
    RadioButton*                mpRbOpnThumbs;
    NumericField*               mpNumInitialPage;

    RadioButton*                mpRbMagnDefault;
    RadioButton*                mpRbMagnFitWin;
    RadioButton*                mpRbMagnFitWidth;
    RadioButton*                mpRbMagnFitVisible;
    RadioButton*                mpRbMagnZoom;
    NumericField*               mpNumZoom;

    RadioButton*                mpRbPgLyDefault;
    RadioButton*                mpRbPgLySinglePage;
    RadioButton*                mpRbPgLyContinue;
    RadioButton*                mpRbPgLyContinueFacing;
    CheckBox*                   mpCbPgLyFirstOnLeft;

    bool                    mbUseCTLFont;

    DECL_LINK( ToggleRbPgLyContinueFacingHdl, void* );
    DECL_LINK( ToggleRbMagnHdl, void* );

public:
    ImpPDFTabOpnFtrPage( vcl::Window* pParent,
                         const SfxItemSet& rSet );

    virtual ~ImpPDFTabOpnFtrPage();
    static SfxTabPage*          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet );

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
    bool                    mbIsPresentation;

    RadioButton*                m_pRbAllBookmarkLevels;
    RadioButton*                m_pRbVisibleBookmarkLevels;
    NumericField*               m_pNumBookmarkLevels;

    DECL_LINK( ToggleRbBookmarksHdl, void* );
public:
    ImpPDFTabViewerPage( vcl::Window* pParent,
                         const SfxItemSet& rSet );

    virtual ~ImpPDFTabViewerPage();
    static SfxTabPage*          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class security tab page
class ImpPDFTabSecurityPage : public SfxTabPage
{
    PushButton*                 mpPbSetPwd;
    OUString                    msStrSetPwd;

    VclContainer*               mpUserPwdSet;
    VclContainer*               mpUserPwdUnset;
    VclContainer*               mpUserPwdPdfa;

    VclContainer*               mpOwnerPwdSet;
    VclContainer*               mpOwnerPwdUnset;
    VclContainer*               mpOwnerPwdPdfa;

    VclContainer*               mpPrintPermissions;
    RadioButton*                mpRbPrintNone;
    RadioButton*                mpRbPrintLowRes;
    RadioButton*                mpRbPrintHighRes;

    VclContainer*               mpChangesAllowed;
    RadioButton*                mpRbChangesNone;
    RadioButton*                mpRbChangesInsDel;
    RadioButton*                mpRbChangesFillForm;
    RadioButton*                mpRbChangesComment;
    RadioButton*                mpRbChangesAnyNoCopy;

    VclContainer*               mpContent;
    CheckBox*                   mpCbEnableCopy;
    CheckBox*                   mpCbEnableAccessibility;

    OUString                    msUserPwdTitle;

    bool                        mbHaveOwnerPassword;
    bool                        mbHaveUserPassword;
    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > maPreparedOwnerPassword;
    OUString                    msOwnerPwdTitle;

    com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder > mxPreparedPasswords;

    DECL_LINK( ClickmaPbSetPwdHdl, void* );

    void enablePermissionControls();

public:
    ImpPDFTabSecurityPage( vcl::Window* pParent,
                           const SfxItemSet& rSet );

    virtual ~ImpPDFTabSecurityPage();
    static SfxTabPage*      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void    ImplPDFASecurityControl( bool bEnableSecurity );
    bool    hasPassword() const { return mbHaveOwnerPassword || mbHaveUserPassword; }
};

//class to implement the relative link stuff
class ImpPDFTabLinksPage : public SfxTabPage
{
    CheckBox*                   m_pCbExprtBmkrToNmDst;
    CheckBox*                   m_pCbOOoToPDFTargets;
    CheckBox*                   m_pCbExportRelativeFsysLinks;

    RadioButton*                m_pRbOpnLnksDefault;
    bool                    mbOpnLnksDefaultUserState;
    RadioButton*                m_pRbOpnLnksLaunch;
    bool                    mbOpnLnksLaunchUserState;
    RadioButton*                m_pRbOpnLnksBrowser;
    bool                    mbOpnLnksBrowserUserState;

    DECL_LINK( ClickRbOpnLnksDefaultHdl, void* );
    DECL_LINK( ClickRbOpnLnksBrowserHdl, void* );

public:
    ImpPDFTabLinksPage( vcl::Window* pParent,
                           const SfxItemSet& rSet );

    virtual ~ImpPDFTabLinksPage();
    static SfxTabPage*      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void    ImplPDFALinkControl( bool bEnableLaunch );
};

//class to implement the digital signing
class ImpPDFTabSigningPage : public SfxTabPage
{
    Edit*                       mpEdSignCert;
    PushButton*                 mpPbSignCertSelect;
    PushButton*                 mpPbSignCertClear;
    Edit*                       mpEdSignPassword;
    Edit*                       mpEdSignLocation;
    Edit*                       mpEdSignContactInfo;
    Edit*                       mpEdSignReason;
    com::sun::star::uno::Reference< com::sun::star::security::XCertificate > maSignCertificate;

    DECL_LINK( ClickmaPbSignCertSelect, void* );
    DECL_LINK( ClickmaPbSignCertClear, void* );

public:
    ImpPDFTabSigningPage( vcl::Window* pParent,
                          const SfxItemSet& rSet );

    virtual ~ImpPDFTabSigningPage();
    static SfxTabPage*      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

#endif // INCLUDED_FILTER_SOURCE_PDF_IMPDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
