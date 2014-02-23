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
#ifndef _IODLGIMPL_HXX
#define _IODLGIMPL_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/split.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <unotools/confignode.hxx>
#include "svl/inettype.hxx"
#include "asyncfilepicker.hxx"
#include "OfficeControlAccess.hxx"
#include "fpsmartcontent.hxx"
#include <comphelper/configuration.hxx>
#include <comphelper/processfactory.hxx>

#include <set>

// @@@ using namespace com::sun::star::ucb;



class SvTabListBox;
class SvtFileView;
struct ControlChain_Impl;
class SvtFileDialogFilter_Impl;



#define SFXWB_INSERT            ( 0x04000000L | WB_OPEN )
#define SFXWB_PASSWORD          WB_PASSWORD
#define SFXWB_READONLY          WB_READONLY
#define SFXWB_PATHDIALOG        WB_PATH
#define SFXWB_CLASSPATH         ( 0x08000000L | SFXWB_PATHDIALOG )
#define SFXWB_SHOWALLFOLDER     0x10000000L     // all directories including Mail/News/...
#define SFXWB_MULTISELECTION    0x20000000L     // activate Multiselection
#define SFXWB_NOREMOTE          0x40000000L
#define SFXWB_SHOWVERSIONS      0x80000000L     // show version selection

#define SFX_EXTRA_AUTOEXTENSION     0x00000001L
#define SFX_EXTRA_FILTEROPTIONS     0x00000002L
#define SFX_EXTRA_SHOWVERSIONS      0x00000004L
#define SFX_EXTRA_INSERTASLINK      0x00000008L
#define SFX_EXTRA_SHOWPREVIEW       0x00000010L
#define SFX_EXTRA_TEMPLATES         0x00000020L
#define SFX_EXTRA_PLAYBUTTON        0x00000040L
#define SFX_EXTRA_SELECTION         0x00000080L
#define SFX_EXTRA_IMAGE_TEMPLATE    0x00000100L

#define FILEDIALOG_FILTER_ALL   "*.*"


// SvtFileDialog


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
    Splitter*                   _pSplitter;
    ::svt::IFilePickerListener* _pFileNotifier;
    SvtExpFileDlg_Impl*         _pImp;
    WinBits                     _nExtraBits;
    sal_Bool                        _bIsInExecute   :   1;

    ImageList                   m_aImages;
    ::svt::SmartContent         m_aContent;

    ::std::set< Control* >      m_aDisabledControls;

    ::utl::OConfigurationNode   m_aConfiguration;
    ::rtl::Reference< ::svt::AsyncPickerAction >
                                m_pCurrentAsyncAction;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::ui::dialogs::XDialogClosedListener >
                                m_xListener;
    bool                        m_bInExecuteAsync;
    bool                        m_bHasFilename;
    ::com::sun::star::uno::Reference < com::sun::star::uno::XComponentContext > m_context;

    DECL_STATIC_LINK( SvtFileDialog, FilterSelectHdl_Impl, ListBox* );
    DECL_STATIC_LINK( SvtFileDialog, NewFolderHdl_Impl, PushButton* );
    DECL_STATIC_LINK( SvtFileDialog, ViewHdl_Impl, ImageButton* );
    DECL_STATIC_LINK( SvtFileDialog, OpenHdl_Impl, void* );
    DECL_LINK       (                CancelHdl_Impl, void* );
    DECL_STATIC_LINK( SvtFileDialog, FileNameGetFocusHdl_Impl, void* );
    DECL_STATIC_LINK( SvtFileDialog, FileNameModifiedHdl_Impl, void* );

    DECL_STATIC_LINK( SvtFileDialog, URLBoxModifiedHdl_Impl, void* );
    DECL_STATIC_LINK( SvtFileDialog, ConnectToServerPressed_Hdl, void* );

    DECL_LINK       (                AddPlacePressed_Hdl, void* );
    DECL_LINK       (                RemovePlacePressed_Hdl, void* );
    DECL_LINK       (                Split_Hdl, void* );

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
    SvtFileDialogFilter_Impl*   FindFilter_Impl( const OUString& _rFilter,
                                                 sal_Bool _bMultiExt,
                                                 sal_Bool& _rFilterChanged
                                                 );
    void                        ExecuteFilter();
    void                        OpenMultiSelection_Impl();
    void                        AddControls_Impl( );

    DECL_LINK( SelectHdl_Impl, SvTabListBox* );
    DECL_LINK(DblClickHdl_Impl, void *);
    DECL_LINK(EntrySelectHdl_Impl, void *);
    DECL_LINK( OpenDoneHdl_Impl, SvtFileView* );
    DECL_LINK(AutoExtensionHdl_Impl, void *);
    DECL_LINK( ClickHdl_Impl, CheckBox* );
    DECL_LINK(PlayButtonHdl_Impl, void *);


    // removes a filter with wildcards from the path and returns it
    sal_Bool IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter );

    void    implArrangeControls();
    void    implUpdateImages( );

protected:
    virtual bool                Notify( NotifyEvent& rNEvt );
    void                        EnableInternet( sal_Bool bInternet );

    // originally from VclFileDialog
    Link                        _aOKHdl;
    Link                        _aFileSelectHdl;
    Link                        _aFilterSelectHdl;

    OUString                    _aPath;
    OUString                    _aDefExt;

    void                        ReleaseOwnership( Window* pUserControl );

    /** enables or disables the complete UI of the file picker, with only offering a
        cancel button

        This method preserves the "enabled" state of its controls in the following sense:
        If you disable a certain control, then disable the dialog UI, then enable the dialog
        UI, the control will still be disabled.
        This is under the assumption that you'll use EnableControl. Direct access to the control
        (such as pControl->Enable()) will break this.
    */
    void                        EnableUI( sal_Bool _bEnable );

    /** enables or disables a control

        You are strongly encouraged to prefer this method over pControl->Enable( _bEnable ). See
        <member>EnableUI</member> for details.
    */
    void                        EnableControl( Control* _pControl, sal_Bool _bEnable );
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

    void                        SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList );
    const ::com::sun::star::uno::Sequence< OUString >& GetBlackList() const;
    void                        SetStandardDir( const OUString& rStdDir );
    const OUString&            GetStandardDir() const;
    std::vector<OUString>       GetPathList() const;        // for MultiSelection

            void                AddFilter( const OUString& rFilter,
                                           const OUString& rType );

            void                AddFilterGroup(
                                  const OUString& _rFilter,
                                  const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& rFilters );

            void                SetCurFilter( const OUString& rFilter );
            OUString            GetCurFilter() const;
            sal_uInt16          GetFilterCount() const;
            const OUString&     GetFilterName( sal_uInt16 nPos ) const;

    virtual void                Resize();
    virtual void                DataChanged( const DataChangedEvent& _rDCEvt );

    void                        PrevLevel_Impl();
    void                        OpenURL_Impl( const OUString& rURL );

    inline SvtFileView*         GetView() const;

    void                        DisableSaveLastDirectory();
    void                        InitSize();
    void                        UpdateControls( const OUString& rURL );
    void                        EnableAutocompletion( sal_Bool _bEnable = sal_True );

    void                        SetFileCallback( ::svt::IFilePickerListener *pNotifier ) { _pFileNotifier = pNotifier; }

    sal_Int32                   getTargetColorDepth();
    sal_Int32                   getAvailableWidth();
    sal_Int32                   getAvailableHeight();
    void                        setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& rImage );
    sal_Bool                    setShowState( sal_Bool bShowState );
    sal_Bool                    getShowState();
    sal_Bool                    isAutoExtensionEnabled();

    OUString                    getCurrentFileText( ) const;
    void                        setCurrentFileText( const OUString& _rText, bool _bSelectAll = false );

    void                        onAsyncOperationStarted();
    void                        onAsyncOperationFinished();

    void                        RemovablePlaceSelected(bool enable = true);

    void                        displayIOException( const OUString& _rURL, ::com::sun::star::ucb::IOErrorCode _eCode );

    // originally from VclFileDialog
    virtual sal_Bool                AddControl( Window* pControl, sal_Bool bNewLine = sal_False );

    // inline
    inline void                 SetPath( const OUString& rNewURL );
    inline void                 SetHasFilename( bool bHasFilename );
    inline const OUString&      GetPath() const;
    inline void                 SetDefaultExt( const OUString& rExt );
    inline void                 EraseDefaultExt( sal_Int32 _nIndex = 0 );
    inline const OUString&      GetDefaultExt() const;
    inline void                 SetOKHdl( const Link& rLink );
    inline const Link&          GetOKHdl() const;
    inline void                 SetFileSelectHdl( const Link& rLink );
    inline const Link&          GetFileSelectHdl() const;
    inline void                 SetFilterSelectHdl( const Link& rLink );
    inline const Link&          GetFilterSelectHdl() const;

    inline Image                GetButtonImage( sal_uInt16 _nButtonId ) const { return m_aImages.GetImage( _nButtonId ); }

    sal_Bool                    ContentIsFolder( const OUString& rURL ) { return m_aContent.isFolder( rURL ) && m_aContent.isValid(); }
    sal_Bool                    ContentHasParentFolder( const OUString& rURL );
    sal_Bool                    ContentCanMakeFolder( const OUString& rURL );
    sal_Bool                    ContentGetTitle( const OUString& rURL, OUString& rTitle );

    /** updates the sizes of the listboxes in the bottom area of the dialog, and of their labels,
        according to the space occupied by the current label texts

        @since #i42824#
    */
    void                        updateListboxLabelSizes();

private:
    SvtFileDialogFilter_Impl*   implAddFilter( const OUString& _rFilter, const OUString& _rType );

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
    sal_Bool                    createNewUserFilter( const OUString& _rNewFilter, sal_Bool _bAllowUserDefExt );

    sal_uInt16                  adjustFilter( const OUString& _rFilter );

    // IFilePickerController, needed by OControlAccess
    virtual Control*            getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const;
    virtual void                enableControl( sal_Int16 _nControlId, sal_Bool _bEnable );
    virtual OUString            getCurFilter( ) const;

    OUString                    implGetInitialURL( const OUString& _rPath, const OUString& _rFallback );

    /// executes a certain FileView action asynchronously
    void                        executeAsync(
                                    ::svt::AsyncPickerAction::Action _eAction,
                                    const OUString& _rURL,
                                    const OUString& _rFilter
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
                                        OUString& _rFileName,
                                        const OUString& _rFilterDefaultExtension,
                                        const OUString& _rFilterExtensions);

    void                        initDefaultPlaces( );
};



inline void SvtFileDialog::SetPath( const OUString& rNewURL )
{
    _aPath = rNewURL;
}



inline void SvtFileDialog::SetHasFilename( bool bHasFilename )
{
    m_bHasFilename = bHasFilename;
}



inline const OUString& SvtFileDialog::GetPath() const
{
    return _aPath;
}



inline void SvtFileDialog::SetDefaultExt( const OUString& rExt )
{
    _aDefExt = rExt;
}

inline void SvtFileDialog::EraseDefaultExt( sal_Int32 _nIndex )
{
    _aDefExt = _aDefExt.copy( 0, _nIndex );
}

inline const OUString& SvtFileDialog::GetDefaultExt() const
{
    return _aDefExt;
}



inline void SvtFileDialog::SetOKHdl
(
    const Link& rLink
)
{
    _aOKHdl = rLink;
}



inline const Link& SvtFileDialog::GetOKHdl() const
{
    return _aOKHdl;
}



inline void SvtFileDialog::SetFileSelectHdl
(
    const Link& rLink
)
{
    _aFileSelectHdl = rLink;
}



inline const Link& SvtFileDialog::GetFileSelectHdl() const
{
    return _aFileSelectHdl;
}



inline void SvtFileDialog::SetFilterSelectHdl
(
    const Link& rLink
)
{
    _aFilterSelectHdl = rLink;
}



inline const Link& SvtFileDialog::GetFilterSelectHdl() const
{
    return _aFilterSelectHdl;
}



inline SvtFileView* SvtFileDialog::GetView() const
{
    return _pFileView;
}





#define FILE_SELECTION_CHANGED  1
#define DIRECTORY_CHANGED       2
#define HELP_REQUESTED          3
#define CTRL_STATE_CHANGED      4
#define DIALOG_SIZE_CHANGED     5


#endif // #ifndef _IODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
