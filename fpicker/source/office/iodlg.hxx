/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iodlg.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _IODLGIMPL_HXX
#define _IODLGIMPL_HXX

#ifndef _VCL_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <unotools/confignode.hxx>
#include "svtools/inettype.hxx"
#include "svtools/urlfilter.hxx"
#include <svtools/restrictedpaths.hxx>
#include "asyncfilepicker.hxx"
#include "OfficeControlAccess.hxx"
#include "fpsmartcontent.hxx"

#include <set>

// @@@ using namespace com::sun::star::ucb;

//*****************************************************************************

class SvTabListBox;
class SvStringsDtor;
class SvtFileView;
struct ControlChain_Impl;
class SvtFileDialogFilter_Impl;

//*****************************************************************************

#define SFXWB_INSERT            ( 0x04000000L | WB_OPEN )
#define SFXWB_PASSWORD          WB_PASSWORD
#define SFXWB_READONLY          WB_READONLY
#define SFXWB_PATHDIALOG        WB_PATH
#define SFXWB_CLASSPATH         ( 0x08000000L | SFXWB_PATHDIALOG )
#define SFXWB_SHOWALLFOLDER     0x10000000L     // alle Ordner auch Mail/News/...
#define SFXWB_MULTISELECTION    0x20000000L     // Multiselection an
#define SFXWB_NOREMOTE          0x40000000L
#define SFXWB_SHOWVERSIONS      0x80000000L     // Versionsauswahl anzeigen

#define SFX_EXTRA_AUTOEXTENSION     0x00000001L
#define SFX_EXTRA_FILTEROPTIONS     0x00000002L
#define SFX_EXTRA_SHOWVERSIONS      0x00000004L
#define SFX_EXTRA_INSERTASLINK      0x00000008L
#define SFX_EXTRA_SHOWPREVIEW       0x00000010L
#define SFX_EXTRA_TEMPLATES         0x00000020L
#define SFX_EXTRA_PLAYBUTTON        0x00000040L
#define SFX_EXTRA_SELECTION         0x00000080L
#define SFX_EXTRA_IMAGE_TEMPLATE    0x00000100L

#define RET_MANAGER             100

#define FILEDIALOG_FILTER_ALL   "*.*"

//*****************************************************************************
// SvtFileDialog
//*****************************************************************************

class SvtExpFileDlg_Impl;
class SvtFileDialog : public ModalDialog, public ::svt::IFilePickerController
{
private:
    // originally from VclFileDialog
    ControlChain_Impl*          _pUserControls;

    CheckBox*                   _pCbReadOnly;
    CheckBox*                   _pCbLinkBox;
    CheckBox*                   _pCbPreviewBox;
    CheckBox*                   _pCbSelection;
    PushButton*                 _pPbPlay;
    Window*                     _pPrevWin;
    FixedBitmap*                _pPrevBmp;
    SvtFileView*                _pFileView;
    ::svt::IFilePickerListener* _pFileNotifier;
    SvtExpFileDlg_Impl*         _pImp;
    WinBits                     _nExtraBits;
    BOOL                        _bIsInExecute   :   1;

    ImageList                   m_aImages;
    ::svt::SmartContent         m_aContent;

    ::svt::RestrictedPaths      m_aURLFilter;
    ::std::set< Control* >      m_aDisabledControls;

    ::utl::OConfigurationNode   m_aConfiguration;
    ::rtl::Reference< ::svt::AsyncPickerAction >
                                m_pCurrentAsyncAction;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ui::dialogs::XDialogClosedListener >
                                m_xListener;
    bool                        m_bInExecuteAsync;
    bool                        m_bHasFilename;

    DECL_STATIC_LINK( SvtFileDialog, FilterSelectHdl_Impl, ListBox* );
    DECL_STATIC_LINK( SvtFileDialog, NewFolderHdl_Impl, PushButton* );
    DECL_STATIC_LINK( SvtFileDialog, ViewHdl_Impl, ImageButton* );
    DECL_STATIC_LINK( SvtFileDialog, OpenHdl_Impl, void* );
    DECL_LINK       (                CancelHdl_Impl, void* );
    DECL_STATIC_LINK( SvtFileDialog, FileNameGetFocusHdl_Impl, void* );
    DECL_STATIC_LINK( SvtFileDialog, FileNameModifiedHdl_Impl, void* );

    void                        Init_Impl( WinBits nBits );
    /** find a filter with the given wildcard
    @param _rFilter
        the wildcard pattern to look for in the filter list
    @param _bMultiExt
        allow for filters with more than one extension pattern
    @param _rFilterChanged
        set to <TRUE/> if the filter changed
    @return
        the filter which has been found
    */
    SvtFileDialogFilter_Impl*   FindFilter_Impl( const String& _rFilter,
                                                 sal_Bool _bMultiExt,
                                                 sal_Bool& _rFilterChanged
                                                 );
    void                        ExecuteFilter();
    void                        OpenMultiSelection_Impl();
    void                        AddControls_Impl( );

    DECL_LINK( SelectHdl_Impl, SvTabListBox* );
    DECL_LINK( DblClickHdl_Impl, SvTabListBox* );
    DECL_LINK( EntrySelectHdl_Impl, ComboBox* );
    DECL_LINK( OpenDoneHdl_Impl, SvtFileView* );
    DECL_LINK( AutoExtensionHdl_Impl, CheckBox* );
    DECL_LINK( ClickHdl_Impl, CheckBox* );
    DECL_LINK( PlayButtonHdl_Impl, PushButton* );

    // entfernt einen Filter mit Wildcards aus dem Path und gibt in zurueck
    BOOL IsolateFilterFromPath_Impl( String& rPath, String& rFilter );

    void    implArrangeControls();
    void    implUpdateImages( );

protected:
    virtual long                Notify( NotifyEvent& rNEvt );
    void                        EnableInternet( BOOL bInternet );

    // originally from VclFileDialog
    Link                        _aOKHdl;
    Link                        _aFileSelectHdl;
    Link                        _aFilterSelectHdl;

    String                      _aPath;
    String                      _aDefExt;

    void                        ReleaseOwnerShip( Window* pUserControl );

    /** enables or disables the complete UI of the file picker, with only offering a
        cancel button

        This method preserves the "enabled" state of its controls in the following sense:
        If you disable a certain control, then disable the dialog UI, then enable the dialog
        UI, the control will still be disabled.
        This is under the assumption that you'll use EnableControl. Direct access to the control
        (such as pControl->Enable()) will break this.
    */
    void                        EnableUI( BOOL _bEnable );

    /** enables or disables a control

        You are strongly encouraged to prefer this method over pControl->Enable( _bEnable ). See
        <member>EnableUI</member> for details.
    */
    void                        EnableControl( Control* _pControl, BOOL _bEnable );
    short                       PrepareExecute();

public:
                                SvtFileDialog( Window* _pParent, WinBits nBits, WinBits nExtraBits );
                                SvtFileDialog( Window* _pParent, WinBits nBits );
                                ~SvtFileDialog();

    virtual long                OK();
    virtual short               Execute();
    virtual void                StartExecuteModal( const Link& rEndDialogHdl );

            void                FileSelect();
            void                FilterSelect();

    void                        SetStandardDir( const String& rStdDir );
    const String&               GetStandardDir() const;
    SvStringsDtor*              GetPathList() const;        // bei MultiSelektion

            void                AddFilter( const String& rFilter,
                                           const String& rType );

            void                AddFilterGroup(
                const String& _rFilter,
                const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& rFilters );

            void                SetCurFilter( const String& rFilter );
            String              GetCurFilter() const;
            USHORT              GetFilterCount() const;
            const String&       GetFilterName( USHORT nPos ) const;

    virtual void                Resize();
    virtual void                DataChanged( const DataChangedEvent& _rDCEvt );

    void                        PrevLevel_Impl();
    void                        OpenURL_Impl( const String& rURL );

    inline SvtFileView*         GetView() const;

    void                        DisableSaveLastDirectory();
    void                        InitSize();
    void                        UpdateControls( const String& rURL );
    void                        EnableAutocompletion( BOOL _bEnable = TRUE );

    void                        SetFileCallback( ::svt::IFilePickerListener *pNotifier ) { _pFileNotifier = pNotifier; }

    sal_Int32                   getTargetColorDepth();
    sal_Int32                   getAvailableWidth();
    sal_Int32                   getAvailableHeight();
    void                        setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& rImage );
    sal_Bool                    setShowState( sal_Bool bShowState );
    sal_Bool                    getShowState();
    sal_Bool                    isAutoExtensionEnabled();

    String                      getCurrentFileText( ) const;
    void                        setCurrentFileText( const String& _rText, bool _bSelectAll = false );

    void                        onAsyncOperationStarted();
    void                        onAsyncOperationFinished();

    void                        displayIOException( const String& _rURL, ::com::sun::star::ucb::IOErrorCode _eCode );
    void                        simulateAccessDenied( const String& _rURL )
    {
        displayIOException( _rURL, ::com::sun::star::ucb::IOErrorCode_ACCESS_DENIED );
    }

    // originally from VclFileDialog
    virtual BOOL                AddControl( Window* pControl, BOOL bNewLine = FALSE );

    // inline
    inline void                 SetPath( const String& rNewURL );
    inline void                 SetHasFilename( bool bHasFilename );
    inline const String&        GetPath() const;
    inline void                 SetDefaultExt( const String& rExt );
    inline void                 EraseDefaultExt( xub_StrLen _nIndex = 0 );
    inline const String&        GetDefaultExt() const;
    inline void                 SetOKHdl( const Link& rLink );
    inline const Link&          GetOKHdl() const;
    inline void                 SetFileSelectHdl( const Link& rLink );
    inline const Link&          GetFileSelectHdl() const;
    inline void                 SetFilterSelectHdl( const Link& rLink );
    inline const Link&          GetFilterSelectHdl() const;

    inline Image                GetButtonImage( sal_uInt16 _nButtonId ) const { return m_aImages.GetImage( _nButtonId ); }

    sal_Bool                    ContentIsFolder( const rtl::OUString& rURL ) { return m_aContent.isFolder( rURL ) && m_aContent.isValid(); }
    sal_Bool                    ContentHasParentFolder( const rtl::OUString& rURL );
    sal_Bool                    ContentCanMakeFolder( const rtl::OUString& rURL );
    sal_Bool                    ContentGetTitle( const rtl::OUString& rURL, String& rTitle );

    /** updates the sizes of the listboxes in the bottom area of the dialog, and of their labels,
        according to the space occupied by the current label texts

        @since #i42824#
    */
    void                        updateListboxLabelSizes();

    /** checks URL access permissions

        <p>with the "restriction" feature we have in the file dialog, it's possible that
        only certain URLs can be browsed. This method checks whether a given URL belongs
        to this set of permitted URLs.</p>

        <p>If no "access restriction" is effective, this method always returns <TRUE/>.</p>
    */
    inline bool isUrlAllowed( const String& _rURL ) const { return m_aURLFilter.isUrlAllowed( _rURL ); }

private:
    SvtFileDialogFilter_Impl*   implAddFilter( const String& _rFilter, const String& _rType );

    /** updates _pUserFilter with a new filter
        <p>No checks for necessity are made.</p>
        @param _bAllowUserDefExt
            set to <TRUE/> if a filter like "*.txt" should reset the DefaultExtension to doc.
            <p>
            In a file-save-dialog this would have the following effect:<br/>
            Say that auto-extension is checked, and the user enters *.txt, while a non-txt filter is selected.<br/>
            If _bAllowUserDefExt is set to <TRUE/>, then a user input of "foo" would save a foo.txt, but in a format
            which is determined by the filter selected (which is no txt file as said above).<br/>
            If _bAllowUserDefExt is set to <FALSE/>, the default extension will be the one of the selected filter, means
            in the above scenario a file "foo.<ext>" will be saved where ext is the extension of the selected filter.
            </p>
        @return <TRUE/> if the new filter is "*.*"
    */
    sal_Bool                    createNewUserFilter( const String& _rNewFilter, sal_Bool _bAllowUserDefExt );

    sal_uInt16                  adjustFilter( const String& _rFilter );

    // IFilePickerController, needed by OControlAccess
    virtual Control*            getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const;
    virtual void                enableControl( sal_Int16 _nControlId, sal_Bool _bEnable );
    virtual String              getCurFilter( ) const;

    String                      implGetInitialURL( const String& _rPath, const String& _rFallback );

    /// initializes the special URL lists, such as our favourites and our restricted paths
    void                        implInitializeSpecialURLLists( );

    /// executes a certain FileView action asynchronously
    void                        executeAsync(
                                    ::svt::AsyncPickerAction::Action _eAction,
                                    const String& _rURL,
                                    const String& _rFilter
                                );

    /** helper function to check and append the default filter extension if
        necessary.
        The function checks if the specified filename already contains one of
        the valid extensions of the specified filter. If not the filter default
        extension is appended to the filename.

        @param _rFileName the filename which is checked and extended if necessary.
        @param _rFilterDefaultExtension the default extension of the used filter.
        @param _rFilterExtensions a list of one or more valid filter extensions
               of the used filter.

     */
    static void                 appendDefaultExtension(
                                        String& _rFileName,
                                        const String& _rFilterDefaultExtension,
                                        const String& _rFilterExtensions);
};

//***************************************************************************

inline void SvtFileDialog::SetPath( const String& rNewURL )
{
    _aPath = rNewURL;
}

//***************************************************************************

inline void SvtFileDialog::SetHasFilename( bool bHasFilename )
{
    m_bHasFilename = bHasFilename;
}

//***************************************************************************

inline const String& SvtFileDialog::GetPath() const
{
    return _aPath;
}

//***************************************************************************

inline void SvtFileDialog::SetDefaultExt( const String& rExt )
{
    _aDefExt = rExt;
}

inline void SvtFileDialog::EraseDefaultExt( xub_StrLen _nIndex )
{
    _aDefExt.Erase( _nIndex );
}

inline const String& SvtFileDialog::GetDefaultExt() const
{
    return _aDefExt;
}

//*****************************************************************************

inline void SvtFileDialog::SetOKHdl
(
    const Link& rLink
)
{
    _aOKHdl = rLink;
}

//*****************************************************************************

inline const Link& SvtFileDialog::GetOKHdl() const
{
    return _aOKHdl;
}

//*****************************************************************************

inline void SvtFileDialog::SetFileSelectHdl
(
    const Link& rLink
)
{
    _aFileSelectHdl = rLink;
}

//*****************************************************************************

inline const Link& SvtFileDialog::GetFileSelectHdl() const
{
    return _aFileSelectHdl;
}

//*****************************************************************************

inline void SvtFileDialog::SetFilterSelectHdl
(
    const Link& rLink
)
{
    _aFilterSelectHdl = rLink;
}

//*****************************************************************************

inline const Link& SvtFileDialog::GetFilterSelectHdl() const
{
    return _aFilterSelectHdl;
}

//*****************************************************************************

inline SvtFileView* SvtFileDialog::GetView() const
{
    return _pFileView;
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

class SvtFilePicker;

#define FILE_SELECTION_CHANGED  1
#define DIRECTORY_CHANGED       2
#define HELP_REQUESTED          3
#define CTRL_STATE_CHANGED      4
#define DIALOG_SIZE_CHANGED     5


#endif // #ifndef _IODLG_HXX
