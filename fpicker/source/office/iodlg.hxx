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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_IODLG_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_IODLG_HXX

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
#include "fpdialogbase.hxx"

#include <set>


class SvTabListBox;
class SvtFileView;
class SvtFileDialogFilter_Impl;
class SvtURLBox;
class SvtExpFileDlg_Impl;
class CustomContainer;

class SvtFileDialog : public SvtFileDialog_Base
{
private:
    VclPtr<CheckBox>            _pCbReadOnly;
    VclPtr<CheckBox>            _pCbLinkBox;
    VclPtr<CheckBox>            _pCbPreviewBox;
    VclPtr<CheckBox>            _pCbSelection;
    VclPtr<PushButton>          _pPbPlay;
    VclPtr<vcl::Window>         _pPrevWin;
    VclPtr<FixedBitmap>         _pPrevBmp;
    VclPtr<CustomContainer>     _pContainer;
    VclPtr<SvtFileView>         _pFileView;
    VclPtr<Splitter>            _pSplitter;
    ::svt::IFilePickerListener* _pFileNotifier;
    SvtExpFileDlg_Impl*         _pImp;
    WinBits                     _nExtraBits;
    bool                        _bIsInExecute   :   1;

    ImageList                   m_aImages;
    ::svt::SmartContent         m_aContent;

    ::std::set< VclPtr<Control> >
                                m_aDisabledControls;

    ::utl::OConfigurationNode   m_aConfiguration;
    ::rtl::Reference< ::svt::AsyncPickerAction >
                                m_pCurrentAsyncAction;
    css::uno::Reference< css::ui::dialogs::XDialogClosedListener >
                                m_xListener;
    bool                        m_bInExecuteAsync;
    bool                        m_bHasFilename;
    css::uno::Reference < css::uno::XComponentContext >
                                m_context;

    DECL_LINK_TYPED(            FilterSelectHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED(            FilterSelectTimerHdl_Impl, Timer*, void );
    DECL_LINK_TYPED(            NewFolderHdl_Impl, Button*, void );
    DECL_LINK_TYPED(            OpenUrlHdl_Impl, SvtURLBox*, void );
    DECL_LINK_TYPED(            OpenClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED(            CancelHdl_Impl, Button*, void );
    DECL_LINK_TYPED(            FileNameGetFocusHdl_Impl, Control&, void );
    DECL_LINK_TYPED(            FileNameModifiedHdl_Impl, Edit&, void );

    DECL_LINK_TYPED(            URLBoxModifiedHdl_Impl, SvtURLBox*, void );
    DECL_LINK_TYPED(            ConnectToServerPressed_Hdl, Button*, void );

    DECL_LINK_TYPED(            AddPlacePressed_Hdl, Button*, void );
    DECL_LINK_TYPED(            RemovePlacePressed_Hdl, Button*, void );
    DECL_LINK_TYPED(            Split_Hdl, Splitter*, void );

    void                        OpenHdl_Impl(void* pVoid);
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
                                                 bool _bMultiExt,
                                                 bool& _rFilterChanged
                                                 );
    void                        ExecuteFilter();
    void                        OpenMultiSelection_Impl();
    void                        AddControls_Impl( );

    DECL_LINK_TYPED( SelectHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED( DblClickHdl_Impl, SvTreeListBox*, bool);
    DECL_LINK_TYPED( EntrySelectHdl_Impl, ComboBox&, void);
    DECL_LINK_TYPED( OpenDoneHdl_Impl, SvtFileView*, void );
    DECL_LINK_TYPED( AutoExtensionHdl_Impl, Button*, void);
    DECL_LINK_TYPED( ClickHdl_Impl, Button*, void );
    DECL_LINK_TYPED( PlayButtonHdl_Impl, Button*, void);


    // removes a filter with wildcards from the path and returns it
    static bool IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter );

    void    implUpdateImages( );

protected:
    virtual bool                Notify( NotifyEvent& rNEvt ) override;

    OUString                    _aPath;
    OUString                    _aDefExt;

    /** enables or disables the complete UI of the file picker, with only offering a
        cancel button

        This method preserves the "enabled" state of its controls in the following sense:
        If you disable a certain control, then disable the dialog UI, then enable the dialog
        UI, the control will still be disabled.
        This is under the assumption that you'll use EnableControl. Direct access to the control
        (such as pControl->Enable()) will break this.
    */
    void                        EnableUI( bool _bEnable );

    /** enables or disables a control

        You are strongly encouraged to prefer this method over pControl->Enable( _bEnable ). See
        <member>EnableUI</member> for details.
    */
    void                        EnableControl( Control* _pControl, bool _bEnable );
    short                       PrepareExecute();

public:
                                SvtFileDialog( vcl::Window* _pParent, WinBits nBits, WinBits nExtraBits );
                                SvtFileDialog( vcl::Window* _pParent, WinBits nBits );
                                virtual ~SvtFileDialog();
    virtual void                dispose() override;

    virtual short               Execute() override;
    virtual void                StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;

            void                FileSelect();
            void                FilterSelect() override;

    void                        SetBlackList( const css::uno::Sequence< OUString >& rBlackList ) override;
    const css::uno::Sequence< OUString >& GetBlackList() const override;
    void                        SetStandardDir( const OUString& rStdDir ) override;
    const OUString&             GetStandardDir() const override;
    std::vector<OUString>       GetPathList() const override;        // for MultiSelection

            void                AddFilter( const OUString& rFilter,
                                           const OUString& rType ) override;

            void                AddFilterGroup(
                                  const OUString& _rFilter,
                                  const css::uno::Sequence< css::beans::StringPair >& rFilters ) override;

            void                SetCurFilter( const OUString& rFilter ) override;
            OUString            GetCurFilter() const override;
            sal_uInt16          GetFilterCount() const;
            const OUString&     GetFilterName( sal_uInt16 nPos ) const;

    virtual void                Resize() override;
    virtual void                DataChanged( const DataChangedEvent& _rDCEvt ) override;

    void                        PrevLevel_Impl();
    void                        OpenURL_Impl( const OUString& rURL );

    SvtFileView*                GetView() override;

    void                        InitSize();
    void                        UpdateControls( const OUString& rURL ) override;
    void                        EnableAutocompletion( bool _bEnable = true ) override;

    void                        SetFileCallback( ::svt::IFilePickerListener *pNotifier ) override { _pFileNotifier = pNotifier; }

    sal_Int32                   getTargetColorDepth() override;
    sal_Int32                   getAvailableWidth() override;
    sal_Int32                   getAvailableHeight() override;
    void                        setImage( sal_Int16 aImageFormat, const css::uno::Any& rImage ) override;
    bool                        getShowState() override;
    bool                        isAutoExtensionEnabled();

    OUString                    getCurrentFileText( ) const override;
    void                        setCurrentFileText( const OUString& _rText, bool _bSelectAll = false ) override;

    void                        onAsyncOperationStarted() override;
    void                        onAsyncOperationFinished() override;

    void                        RemovablePlaceSelected(bool enable = true);

    static void                 displayIOException( const OUString& _rURL, css::ucb::IOErrorCode _eCode );

    // inline
    inline void                 SetPath( const OUString& rNewURL ) override;
    inline void                 SetHasFilename( bool bHasFilename ) override;
    inline const OUString&      GetPath() override;
    inline void                 SetDefaultExt( const OUString& rExt );
    inline void                 EraseDefaultExt( sal_Int32 _nIndex = 0 );
    inline const OUString&      GetDefaultExt() const;

    inline Image                GetButtonImage( sal_uInt16 _nButtonId ) const { return m_aImages.GetImage( _nButtonId ); }

    bool                        ContentIsFolder( const OUString& rURL ) override { return m_aContent.isFolder( rURL ) && m_aContent.isValid(); }
    bool                        ContentHasParentFolder( const OUString& rURL );
    bool                        ContentCanMakeFolder( const OUString& rURL );
    bool                        ContentGetTitle( const OUString& rURL, OUString& rTitle );

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
    bool                        createNewUserFilter( const OUString& _rNewFilter, bool _bAllowUserDefExt );

    sal_uInt16                  adjustFilter( const OUString& _rFilter );

    // IFilePickerController, needed by OControlAccess
    virtual Control*            getControl( sal_Int16 _nControlId, bool _bLabelControl = false ) const override;
    virtual void                enableControl( sal_Int16 _nControlId, bool _bEnable ) override;
    virtual OUString            getCurFilter( ) const override;

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



inline const OUString& SvtFileDialog::GetPath()
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


inline SvtFileView* SvtFileDialog::GetView()
{
    return _pFileView;
}


#endif // INCLUDED_FPICKER_SOURCE_OFFICE_IODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
