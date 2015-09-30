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
    VclPtr<ListBox>        m_pErrors;
    VclPtr<FixedText>      m_pExplanation;

    DECL_LINK(SelectHdl, void *);
    public:
    ImplErrorDialog( const std::set< vcl::PDFWriter::ErrorCode >& );
    virtual ~ImplErrorDialog();
    virtual void dispose() SAL_OVERRIDE;
};

class ImpPDFTabSecurityPage;
class ImpPDFTabLinksPage;


//class tabbed dialog
class ImpPDFTabDialog : public SfxTabDialog
{
private:
    css::uno::Reference< css::uno::XComponentContext> mxContext;
    FilterConfigItem            maConfigItem;
    FilterConfigItem            maConfigI18N;

    Any                         maSelection;

    DECL_LINK_TYPED(CancelHdl, Button*, void);

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
    sal_Int32               mnQuality;
    bool                    mbReduceImageResolution;
    sal_Int32               mnMaxImageResolution;
    bool                    mbUseTaggedPDF;
    sal_Int32               mnPDFTypeSelection;
    bool                    mbExportNotes;
    bool                    mbViewPDF;
    bool                    mbExportNotesPages;
    bool                    mbUseTransitionEffects;
    bool                    mbIsSkipEmptyPages;
    bool                    mbAddStream;
    sal_Int32               mnFormsType;
    bool                    mbExportFormFields;
    bool                    mbAllowDuplicateFieldNames;
    bool                    mbExportBookmarks;
    bool                    mbExportHiddenSlides;
    sal_Int32               mnOpenBookmarkLevels;

    bool                    mbHideViewerToolbar;
    bool                    mbHideViewerMenubar;
    bool                    mbHideViewerWindowControls;
    bool                    mbResizeWinToInit;
    bool                    mbCenterWindow;
    bool                    mbOpenInFullScreenMode;
    bool                    mbDisplayPDFDocumentTitle;
    sal_Int32               mnMagnification;
    sal_Int32               mnInitialView;
    sal_Int32               mnZoom;
    sal_Int32               mnInitialPage;

    sal_Int32               mnPageLayout;
    bool                    mbFirstPageLeft;

    bool                    mbEncrypt;

    bool                    mbRestrictPermissions;
    css::uno::Sequence< css::beans::NamedValue > maPreparedOwnerPassword;
    sal_Int32               mnPrint;
    sal_Int32               mnChangesAllowed;
    bool                    mbCanCopyOrExtract;
    bool                    mbCanExtractForAccessibility;
    css::uno::Reference< css::beans::XMaterialHolder > mxPreparedPasswords;

    bool                    mbIsRangeChecked;
    OUString                msPageRange;
    bool                    mbSelectionIsChecked;

    bool                    mbExportRelativeFsysLinks;
    sal_Int32               mnViewPDFMode;
    bool                    mbConvertOOoTargets;
    bool                    mbExportBmkToPDFDestination;

    bool                    mbSignPDF;
    OUString                msSignPassword;
    OUString                msSignLocation;
    OUString                msSignContact;
    OUString                msSignReason;
    css::uno::Reference< css::security::XCertificate > maSignCertificate;
    OUString                msSignTSA;

    OUString                maWatermarkText;

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
    virtual void dispose() SAL_OVERRIDE;

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

    VclPtr<RadioButton>  mpRbAll;
    VclPtr<RadioButton>  mpRbRange;
    VclPtr<RadioButton>  mpRbSelection;
    VclPtr<Edit>         mpEdPages;

    VclPtr<RadioButton>  mpRbLosslessCompression;
    VclPtr<RadioButton>  mpRbJPEGCompression;
    VclPtr<VclContainer> mpQualityFrame;
    VclPtr<MetricField>  mpNfQuality;
    VclPtr<CheckBox>     mpCbReduceImageResolution;
    VclPtr<ComboBox>     mpCoReduceImageResolution;

    VclPtr<CheckBox>     mpCbPDFA1b;
    VclPtr<CheckBox>     mpCbTaggedPDF;
    bool          mbTaggedPDFUserSelection;

    VclPtr<CheckBox>     mpCbExportFormFields;
    bool          mbExportFormFieldsUserSelection;
    VclPtr<VclContainer> mpFormsFrame;
    VclPtr<ListBox>      mpLbFormsFormat;
    VclPtr<CheckBox>     mpCbAllowDuplicateFieldNames;

    VclPtr<CheckBox>     mpCbExportBookmarks;
    VclPtr<CheckBox>     mpCbExportHiddenSlides;
    VclPtr<CheckBox>     mpCbExportNotes;
    VclPtr<CheckBox>     mpCbViewPDF;
    VclPtr<CheckBox>     mpCbExportNotesPages;

    VclPtr<CheckBox>     mpCbExportEmptyPages;
    VclPtr<CheckBox>     mpCbAddStream;

    VclPtr<CheckBox>     mpCbWatermark;
    VclPtr<FixedText>    mpFtWatermark;
    VclPtr<Edit>         mpEdWatermark;

    bool          mbIsPresentation;
    bool          mbIsWriter;

    VclPtr<ImpPDFTabDialog> mpaParent;

    DECL_LINK_TYPED( TogglePagesHdl, RadioButton&, void );
    DECL_LINK_TYPED( ToggleCompressionHdl, RadioButton&, void );
    DECL_LINK_TYPED( ToggleReduceImageResolutionHdl, CheckBox&, void );
    DECL_LINK_TYPED( ToggleWatermarkHdl, CheckBox&, void );
    DECL_LINK_TYPED( ToggleAddStreamHdl, CheckBox&, void );
    DECL_LINK_TYPED( ToggleExportFormFieldsHdl, CheckBox&, void );
    void TogglePagesHdl();

public:
    DECL_LINK_TYPED( ToggleExportPDFAHdl, CheckBox&, void );

    ImpPDFTabGeneralPage( vcl::Window* pParent,
                          const SfxItemSet& rSet );
    virtual ~ImpPDFTabGeneralPage();
    virtual void dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet);

    void    GetFilterConfigItem(ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem(ImpPDFTabDialog* paParent);
    bool    IsPdfaSelected() const { return mpCbPDFA1b->IsChecked(); }
};

//class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
    VclPtr<RadioButton>                mpRbOpnPageOnly;
    VclPtr<RadioButton>                mpRbOpnOutline;
    VclPtr<RadioButton>                mpRbOpnThumbs;
    VclPtr<NumericField>               mpNumInitialPage;

    VclPtr<RadioButton>                mpRbMagnDefault;
    VclPtr<RadioButton>                mpRbMagnFitWin;
    VclPtr<RadioButton>                mpRbMagnFitWidth;
    VclPtr<RadioButton>                mpRbMagnFitVisible;
    VclPtr<RadioButton>                mpRbMagnZoom;
    VclPtr<NumericField>               mpNumZoom;

    VclPtr<RadioButton>                mpRbPgLyDefault;
    VclPtr<RadioButton>                mpRbPgLySinglePage;
    VclPtr<RadioButton>                mpRbPgLyContinue;
    VclPtr<RadioButton>                mpRbPgLyContinueFacing;
    VclPtr<CheckBox>                   mpCbPgLyFirstOnLeft;

    bool                    mbUseCTLFont;

    DECL_LINK_TYPED( ToggleRbPgLyContinueFacingHdl, RadioButton&, void );
    DECL_LINK_TYPED( ToggleRbMagnHdl, RadioButton&, void );
    void ToggleRbPgLyContinueFacingHdl();

public:
    ImpPDFTabOpnFtrPage( vcl::Window* pParent,
                         const SfxItemSet& rSet );

    virtual ~ImpPDFTabOpnFtrPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class tab page viewer
class ImpPDFTabViewerPage : public SfxTabPage
{
    VclPtr<CheckBox>                   m_pCbResWinInit;
    VclPtr<CheckBox>                   m_pCbCenterWindow;
    VclPtr<CheckBox>                   m_pCbOpenFullScreen;
    VclPtr<CheckBox>                   m_pCbDispDocTitle;


    VclPtr<CheckBox>                   m_pCbHideViewerMenubar;
    VclPtr<CheckBox>                   m_pCbHideViewerToolbar;
    VclPtr<CheckBox>                   m_pCbHideViewerWindowControls;

    VclPtr<CheckBox>                   m_pCbTransitionEffects;
    bool                    mbIsPresentation;

    VclPtr<RadioButton>                m_pRbAllBookmarkLevels;
    VclPtr<RadioButton>                m_pRbVisibleBookmarkLevels;
    VclPtr<NumericField>               m_pNumBookmarkLevels;

    DECL_LINK_TYPED( ToggleRbBookmarksHdl, RadioButton&, void );
public:
    ImpPDFTabViewerPage( vcl::Window* pParent,
                         const SfxItemSet& rSet );

    virtual ~ImpPDFTabViewerPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>          Create( vcl::Window* pParent,
                                        const SfxItemSet* rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class security tab page
class ImpPDFTabSecurityPage : public SfxTabPage
{
    VclPtr<PushButton>                 mpPbSetPwd;
    OUString                           msStrSetPwd;

    VclPtr<VclContainer>               mpUserPwdSet;
    VclPtr<VclContainer>               mpUserPwdUnset;
    VclPtr<VclContainer>               mpUserPwdPdfa;

    VclPtr<VclContainer>               mpOwnerPwdSet;
    VclPtr<VclContainer>               mpOwnerPwdUnset;
    VclPtr<VclContainer>               mpOwnerPwdPdfa;

    VclPtr<VclContainer>               mpPrintPermissions;
    VclPtr<RadioButton>                mpRbPrintNone;
    VclPtr<RadioButton>                mpRbPrintLowRes;
    VclPtr<RadioButton>                mpRbPrintHighRes;

    VclPtr<VclContainer>               mpChangesAllowed;
    VclPtr<RadioButton>                mpRbChangesNone;
    VclPtr<RadioButton>                mpRbChangesInsDel;
    VclPtr<RadioButton>                mpRbChangesFillForm;
    VclPtr<RadioButton>                mpRbChangesComment;
    VclPtr<RadioButton>                mpRbChangesAnyNoCopy;

    VclPtr<VclContainer>               mpContent;
    VclPtr<CheckBox>                   mpCbEnableCopy;
    VclPtr<CheckBox>                   mpCbEnableAccessibility;

    OUString                    msUserPwdTitle;

    bool                        mbHaveOwnerPassword;
    bool                        mbHaveUserPassword;
    css::uno::Sequence< css::beans::NamedValue > maPreparedOwnerPassword;
    OUString                    msOwnerPwdTitle;

    css::uno::Reference< css::beans::XMaterialHolder > mxPreparedPasswords;

    DECL_LINK_TYPED( ClickmaPbSetPwdHdl, Button*, void );

    void enablePermissionControls();

public:
    ImpPDFTabSecurityPage( vcl::Window* pParent,
                           const SfxItemSet& rSet );

    virtual ~ImpPDFTabSecurityPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void    ImplPDFASecurityControl( bool bEnableSecurity );
    bool    hasPassword() const { return mbHaveOwnerPassword || mbHaveUserPassword; }
};

//class to implement the relative link stuff
class ImpPDFTabLinksPage : public SfxTabPage
{
    VclPtr<CheckBox>        m_pCbExprtBmkrToNmDst;
    VclPtr<CheckBox>        m_pCbOOoToPDFTargets;
    VclPtr<CheckBox>        m_pCbExportRelativeFsysLinks;

    VclPtr<RadioButton>     m_pRbOpnLnksDefault;
    bool                    mbOpnLnksDefaultUserState;
    VclPtr<RadioButton>     m_pRbOpnLnksLaunch;
    bool                    mbOpnLnksLaunchUserState;
    VclPtr<RadioButton>     m_pRbOpnLnksBrowser;
    bool                    mbOpnLnksBrowserUserState;

    DECL_LINK_TYPED( ClickRbOpnLnksDefaultHdl, Button*, void );
    DECL_LINK_TYPED( ClickRbOpnLnksBrowserHdl, Button*, void );

public:
    ImpPDFTabLinksPage( vcl::Window* pParent,
                           const SfxItemSet& rSet );

    virtual ~ImpPDFTabLinksPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void    ImplPDFALinkControl( bool bEnableLaunch );
};

//class to implement the digital signing
class ImpPDFTabSigningPage : public SfxTabPage
{
    VclPtr<Edit>                       mpEdSignCert;
    VclPtr<PushButton>                 mpPbSignCertSelect;
    VclPtr<PushButton>                 mpPbSignCertClear;
    VclPtr<Edit>                       mpEdSignPassword;
    VclPtr<Edit>                       mpEdSignLocation;
    VclPtr<Edit>                       mpEdSignContactInfo;
    VclPtr<Edit>                       mpEdSignReason;
    VclPtr<ListBox>                    mpLBSignTSA;
    css::uno::Reference< css::security::XCertificate > maSignCertificate;

    DECL_LINK_TYPED( ClickmaPbSignCertSelect, Button*, void );
    DECL_LINK_TYPED( ClickmaPbSignCertClear, Button*, void );

public:
    ImpPDFTabSigningPage( vcl::Window* pParent,
                          const SfxItemSet& rSet );

    virtual ~ImpPDFTabSigningPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent,
                                    const SfxItemSet* rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

#endif // INCLUDED_FILTER_SOURCE_PDF_IMPDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
