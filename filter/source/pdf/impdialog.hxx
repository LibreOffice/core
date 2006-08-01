/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impdialog.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:29:00 $
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

#ifndef IMPDIALOG_HXX
#define IMPDIALOG_HXX

#include "pdffilter.hxx"
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

#ifndef _GROUP_HXX
#include <vcl/group.hxx>
#endif
#include <svtools/FilterConfigItem.hxx>

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

// ----------------
// - ImpPDFDialog -
// ----------------

class ResMgr;
class ImpPDFTabGeneralPage;
class ImpPDFTabViewerPage;
class ImpPDFTabOpnFtrPage;

////////////////////////////////////////////////////////////////////////
//class tabbed dialog
class ImpPDFTabDialog : public SfxTabDialog
{
private:

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
    sal_Bool                    mbExportNotesBoth;
    sal_Bool                    mbUseTransitionEffects;
    sal_Bool                    mbIsSkipEmptyPages;
    sal_Int32                   mnFormsType;

    sal_Bool                    mbHideViewerToolbar;
    sal_Bool                    mbHideViewerMenubar;
    sal_Bool                    mbHideViewerWindowControls;
    sal_Bool                    mbResizeWinToInit;
    sal_Bool                    mbCenterWindow;
    sal_Bool                    mbOpenInFullScreenMode;
    sal_Bool                    mbDisplayPDFDocumentTitle;
    sal_Int32                   mnMagnification;
    sal_Int32                   mnInitialView;

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

public:

    friend class                ImpPDFTabGeneralPage;
    friend class                ImpPDFTabViewerPage;
    friend class                ImpPDFTabOpnFtrPage;
    friend class                ImpPDFTabSecurityPage;

    ImpPDFTabDialog( Window* pParent, ResMgr& rResMgr,
                     Sequence< PropertyValue >& rFilterData,
                     const Reference< XComponent >& rDoc );
    ~ImpPDFTabDialog();

    Sequence< PropertyValue >   GetFilterData();

protected:
    virtual void                PageCreated( USHORT _nId,
                                             SfxTabPage& _rPage );
    virtual short               Ok();
};

//class tab page general
class ImpPDFTabGeneralPage : public SfxTabPage
{
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
    CheckBox                    maCbTaggedPDF;
    CheckBox                    maCbExportNotes;
    CheckBox                    maCbTransitionEffects;

    FixedText                   maFtFormsFormat;
    ListBox                     maLbFormsFormat;
    CheckBox                    maCbExportEmptyPages;

    sal_Bool                    mbIsPresentation;
    sal_Bool                    mbIsWriter;

    ResMgr*                     mpaResMgr;

    DECL_LINK( TogglePagesHdl, void* );
    DECL_LINK( ToggleCompressionHdl, void* );
    DECL_LINK( ToggleReduceImageResolutionHdl, void* );

public:
    ImpPDFTabGeneralPage( Window* pParent,
                          const SfxItemSet& rSet,
                          ResMgr* paResMgr );

    ~ImpPDFTabGeneralPage();
    static SfxTabPage*          Create( Window* pParent,
                                        const SfxItemSet& rAttrSet);

    void                        GetFilterConfigItem( ImpPDFTabDialog* paParent );
    void                        SetFilterConfigItem( const ImpPDFTabDialog* paParent );
};

//class tab page viewer
class ImpPDFTabOpnFtrPage : public SfxTabPage
{
    FixedLine                   maFlInitialView;
    RadioButton                 maRbOpnPageOnly;
    RadioButton                 maRbOpnOutline;
    RadioButton                 maRbOpnThumbs;

    FixedLine                   maFlMagnification;
    RadioButton                 maRbMagnDefault;
    RadioButton                 maRbMagnFitWin;
    RadioButton                 maRbMagnFitWidth;
    RadioButton                 maRbMagnFitVisible;

    FixedLine                   maFlPageLayout;
    RadioButton                 maRbPgLyDefault;
    RadioButton                 maRbPgLySinglePage;
    RadioButton                 maRbPgLyContinue;
    RadioButton                 maRbPgLyContinueFacing;
    CheckBox                    maCbPgLyFirstOnLeft;

    sal_Bool                    mbUseCTLFont;
    ResMgr*                     mpaResMgr;

    DECL_LINK( ToggleRbPgLyContinueFacingHdl, void* );

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

    ResMgr*                     mpaResMgr;

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

    FixedLine                   maFlPermissions;
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
};

#endif // IMPDIALOG_HXX
