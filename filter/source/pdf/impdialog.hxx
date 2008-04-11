/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impdialog.hxx,v $
 * $Revision: 1.23 $
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

#include "svtools/FilterConfigItem.hxx"

#include "sfx2/tabdlg.hxx"

// ----------------
// - ImpPDFDialog -
// ----------------

class ResMgr;
class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;
class ImpPDFTabLinksPage;

class ImplErrorDialog : public ModalDialog
{
    FixedImage      maFI;
    FixedText       maProcessText;
    ListBox         maErrors;
    FixedText       maExplanation;

    OKButton        maButton;

    DECL_LINK( SelectHdl, ListBox* );
    public:
    ImplErrorDialog( const std::set< vcl::PDFWriter::ErrorCode >&, ResMgr& rResMgr );
    ~ImplErrorDialog();
};

////////////////////////////////////////////////////////////////////////
//class tabbed dialog
class ImpPDFTabDialog : public SfxTabDialog
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;
    FilterConfigItem            maConfigItem;
    FilterConfigItem            maConfigI18N;

    Any                         maSelection;

protected:

    ResMgr*                     mprResMgr;
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
    sal_Bool                    mbExportNotesBoth;
    sal_Bool                    mbUseTransitionEffects;
    sal_Bool                    mbIsSkipEmptyPages;
    sal_Bool                    mbAddStream;
    sal_Int32                   mnFormsType;
    sal_Bool                    mbExportFormFields;
    sal_Bool                    mbExportBookmarks;
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
    String                      msUserPassword;

    sal_Bool                    mbRestrictPermissions;
    String                      msOwnerPassword;
    sal_Int32                   mnPrint;
    sal_Int32                   mnChangesAllowed;
    sal_Bool                    mbCanCopyOrExtract;
    sal_Bool                    mbCanExtractForAccessibility;

    sal_Bool                    mbIsRangeChecked;
    String                      msPageRange;
    sal_Bool                    mbSelectionIsChecked;

    sal_Bool                    mbExportRelativeFsysLinks;
    sal_Int32                   mnViewPDFMode;
    sal_Bool                    mbConvertOOoTargets;
    sal_Bool                    mbExportBmkToPDFDestination;

public:

    friend class                ImpPDFTabGeneralPage;
    friend class                ImpPDFTabViewerPage;
    friend class                ImpPDFTabOpnFtrPage;
    friend class                ImpPDFTabSecurityPage;
    friend class                ImpPDFTabLinksPage;

    ImpPDFTabDialog( Window* pParent, ResMgr& rResMgr,
                     Sequence< PropertyValue >& rFilterData,
                     const Reference< XComponent >& rDoc,
                     const Reference< lang::XMultiServiceFactory >& xFact
                     );
    ~ImpPDFTabDialog();

    Sequence< PropertyValue >   GetFilterData();
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& getServiceFactory() const { return mxMSF; }

protected:
    virtual void                PageCreated( USHORT _nId,
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

    FixedLine                   maFlGeneral;
    CheckBox                    maCbPDFA1b;
    CheckBox                    maCbTaggedPDF;
    sal_Bool                    mbTaggedPDFUserSelection;

    CheckBox                    maCbExportFormFields;
    sal_Bool                    mbExportFormFieldsUserSelection;
    FixedText                   maFtFormsFormat;
    ListBox                     maLbFormsFormat;

    CheckBox                    maCbExportBookmarks;
    CheckBox                    maCbExportNotes;

    CheckBox                    maCbExportEmptyPages;
    CheckBox                    maCbAddStream;

    sal_Bool                    mbIsPresentation;
    sal_Bool                    mbIsWriter;

    ResMgr*                     mpaResMgr;

const ImpPDFTabDialog*          mpaParent;

    DECL_LINK( TogglePagesHdl, void* );
    DECL_LINK( ToggleCompressionHdl, void* );
    DECL_LINK( ToggleReduceImageResolutionHdl, void* );
    DECL_LINK( ToggleAddStreamHdl, void* );
    DECL_LINK( ToggleExportFormFieldsHdl, void* );

public:
    DECL_LINK( ToggleExportPDFAHdl, void* );

    ImpPDFTabGeneralPage( Window* pParent,
                          const SfxItemSet& rSet,
                          ResMgr* paResMgr );

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

    FixedLine                   maFlPageLayout;
    RadioButton                 maRbPgLyDefault;
    RadioButton                 maRbPgLySinglePage;
    RadioButton                 maRbPgLyContinue;
    RadioButton                 maRbPgLyContinueFacing;
    CheckBox                    maCbPgLyFirstOnLeft;

    sal_Bool                    mbUseCTLFont;
    ResMgr*                     mpaResMgr;

    DECL_LINK( ToggleRbPgLyContinueFacingHdl, void* );
    DECL_LINK( ToggleRbMagnHdl, void* );

public:
    ImpPDFTabOpnFtrPage( Window* pParent,
                         const SfxItemSet& rSet,
                         ResMgr* paResMgr );

    ~ImpPDFTabOpnFtrPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class tab page viewer
class ImpPDFTabViewerPage : public SfxTabPage
{
    FixedLine                   maFlWindowOptions;
    CheckBox                    maCbResWinInit;
    CheckBox                    maCbCenterWindow;
    CheckBox                    maCbOpenFullScreen;
    CheckBox                    maCbDispDocTitle;

    FixedLine                   maFlUIOptions;
    CheckBox                    maCbHideViewerMenubar;
    CheckBox                    maCbHideViewerToolbar;
    CheckBox                    maCbHideViewerWindowControls;

    FixedLine                   maFlTransitions;
    CheckBox                    maCbTransitionEffects;
    sal_Bool                    mbIsPresentation;

    FixedLine                   maFlBookmarks;
    RadioButton                 maRbAllBookmarkLevels;
    RadioButton                 maRbVisibleBookmarkLevels;
    NumericField                maNumBookmarkLevels;

    ResMgr*                     mpaResMgr;

    DECL_LINK( ToggleRbBookmarksHdl, void* );
public:
    ImpPDFTabViewerPage( Window* pParent,
                         const SfxItemSet& rSet,
                         ResMgr* paResMgr );

    ~ImpPDFTabViewerPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet );

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class security tab page
class ImpPDFTabSecurityPage : public SfxTabPage
{
    CheckBox                    maCbEncrypt;

    PushButton                  maPbUserPwd;
    FixedText                   maFtUserPwdEmpty;

    CheckBox                    maCbPermissions;
    PushButton                  maPbOwnerPwd;
    FixedText                   maFtOwnerPwdEmpty;

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

    String                      msUserPassword;
    String                      msSetUserPwd;
    String                      msUserPwdTitle;

    String                      msRestrPermissions;
    String                      msOwnerPassword;
    String                      msSetOwnerPwd;
    String                      msOwnerPwdTitle;

    ResMgr*                     mpaResMgr;

    long nWidth;

    DECL_LINK( TogglemaCbEncryptHdl, void* );
    DECL_LINK( TogglemaCbPermissionsHdl, void* );

    DECL_LINK( ClickmaPbUserPwdHdl, void* );
    DECL_LINK( ClickmaPbOwnerPwdHdl, void* );

    void                        ImplPwdPushButton( String &, String & , FixedText & );

public:
    ImpPDFTabSecurityPage( Window* pParent,
                           const SfxItemSet& rSet,
                           ResMgr* paResMgr );

    ~ImpPDFTabSecurityPage();
    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );
    void    ImplPDFASecurityControl( sal_Bool bEnableSecurity );
};

//class to implement the relative link stuff
class ImpPDFTabLinksPage : public SfxTabPage
{
    CheckBox                    maCbExprtBmkrToNmDst;
    CheckBox                    maCbOOoToPDFTargets;
    CheckBox                    maCbExportRelativeFsysLinks;

    FixedLine                   maFlDefaultTitle;
    RadioButton                 maRbOpnLnksDefault;
    sal_Bool                    mbOpnLnksDefaultUserState;
    RadioButton                 maRbOpnLnksLaunch;
    sal_Bool                    mbOpnLnksLaunchUserState;
    RadioButton                 maRbOpnLnksBrowser;
    sal_Bool                    mbOpnLnksBrowserUserState;

    ResMgr*                     mpaResMgr;

    long nWidth;

    DECL_LINK( ClickRbOpnLnksDefaultHdl, void* );
    DECL_LINK( ClickRbOpnLnksBrowserHdl, void* );

public:
    ImpPDFTabLinksPage( Window* pParent,
                           const SfxItemSet& rSet,
                           ResMgr& rResMgr );

    ~ImpPDFTabLinksPage();
    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );

    void    GetFilterConfigItem( ImpPDFTabDialog* paParent);
    void    SetFilterConfigItem( const ImpPDFTabDialog* paParent );

    void    ImplPDFALinkControl( sal_Bool bEnableLaunch );
};

#endif // IMPDIALOG_HXX


