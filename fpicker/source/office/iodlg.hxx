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

#include <memory>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <unotools/confignode.hxx>
#include "asyncfilepicker.hxx"
#include "fpsmartcontent.hxx"
#include "fpdialogbase.hxx"
#include <o3tl/typed_flags_set.hxx>
#include <vcl/timer.hxx>

#include <set>
#include <string_view>

class SvtFileView;
class SvtFileDialogFilter_Impl;
class SvtExpFileDlg_Impl;
class SvtURLBox;

enum class AdjustFilterFlags {
    NONE            = 0x0000,
    NonEmpty        = 0x0001,
    Changed         = 0x0002,
    UserFilter      = 0x0004,
};
namespace o3tl {
    template<> struct typed_flags<AdjustFilterFlags> : is_typed_flags<AdjustFilterFlags, 0x0007> {};
}


class SvtFileDialog final : public SvtFileDialog_Base
{
private:
    std::unique_ptr<weld::CheckButton> m_xCbReadOnly;
    std::unique_ptr<weld::CheckButton> m_xCbLinkBox;
    std::unique_ptr<weld::CheckButton> m_xCbPreviewBox;
    std::unique_ptr<weld::CheckButton> m_xCbSelection;
    std::unique_ptr<weld::Button> m_xPbPlay;
    std::unique_ptr<weld::Widget> m_xPreviewFrame;
    std::unique_ptr<weld::Image> m_xPrevBmp;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<SvtFileView> m_xFileView;
    ::svt::IFilePickerListener* m_pFileNotifier;
    std::unique_ptr<SvtExpFileDlg_Impl> m_xImpl;
    Size                        m_aPreviewSize;
    PickerFlags                 m_nPickerFlags;
    bool                        m_bIsInExecute   :   1;

    ::svt::SmartContent         m_aContent;

    ::std::set<weld::Widget*>   m_aDisabledControls;

    ::utl::OConfigurationNode   m_aConfiguration;
    ::rtl::Reference< ::svt::AsyncPickerAction >
                                m_pCurrentAsyncAction;
    bool                        m_bInExecuteAsync;
    bool                        m_bHasFilename;
    css::uno::Reference < css::uno::XComponentContext >
                                m_xContext;

    DECL_LINK(            FilterSelectHdl_Impl, weld::ComboBox&, void );
    DECL_LINK(            FilterSelectTimerHdl_Impl, Timer*, void );
    DECL_LINK(            NewFolderHdl_Impl, weld::Button&, void );
    DECL_LINK(            OpenUrlHdl_Impl, weld::ComboBox&, bool );
    DECL_LINK(            OpenClickHdl_Impl, weld::Button&, void );
    DECL_LINK(            CancelHdl_Impl, weld::Button&, void );
    DECL_LINK(            FileNameGetFocusHdl_Impl, weld::Widget&, void );
    DECL_LINK(            FileNameModifiedHdl_Impl, weld::ComboBox&, void );

    DECL_LINK(            URLBoxModifiedHdl_Impl, weld::ComboBox&, bool );
    DECL_LINK(            ConnectToServerPressed_Hdl, weld::Button&, void );

    DECL_LINK(            AddPlacePressed_Hdl, weld::Button&, void );
    DECL_LINK(            RemovePlacePressed_Hdl, weld::Button&, void );
    DECL_LINK(            PreviewSizeAllocHdl, const Size&, void);

    void                  OpenHdl_Impl(void const * pVoid);

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

    DECL_LINK(SelectHdl_Impl, SvtFileView*, void);
    DECL_LINK(DblClickHdl_Impl, SvtFileView*, bool);
    DECL_LINK(EntrySelectHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(OpenDoneHdl_Impl, SvtFileView*, void);
    DECL_LINK(AutoExtensionHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickHdl_Impl, weld::Button&, void);
    DECL_LINK(PlayButtonHdl_Impl, weld::Button&, void);
    DECL_LINK(SizeAllocHdl, const Size&, void);

    // removes a filter with wildcards from the path and returns it
    static bool IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter );

    OUString                    m_aPath;
    OUString                    m_aDefExt;

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

        You are strongly encouraged to prefer this method over pControl->Enable( bEnable ). See
        <member>EnableUI</member> for details.
    */
    void                        EnableControl(weld::Widget* pControl, bool bEnable);
    virtual bool                PrepareExecute() override;

public:
                                SvtFileDialog( weld::Window* pParent, PickerFlags nBits );
                                virtual ~SvtFileDialog() override;

    virtual short               run() override;

            void                FileSelect();
            void                FilterSelect() override;

    void                        SetDenyList( const css::uno::Sequence< OUString >& rDenyList ) override;
    const css::uno::Sequence< OUString >& GetDenyList() const override;
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

    void                        PrevLevel_Impl();
    void                        OpenURL_Impl( const OUString& rURL );

    SvtFileView*                GetView() override;

    void                        InitSize();
    void                        UpdateControls( const OUString& rURL ) override;
    void                        EnableAutocompletion( bool _bEnable = true ) override;

    void                        SetFileCallback( ::svt::IFilePickerListener *pNotifier ) override { m_pFileNotifier = pNotifier; }

    sal_Int32                   getAvailableWidth() override;
    sal_Int32                   getAvailableHeight() override;
    void                        setImage( const css::uno::Any& rImage ) override;
    bool                        getShowState() override;
    bool                        isAutoExtensionEnabled() const;

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

    bool                        ContentIsFolder( const OUString& rURL ) override { return m_aContent.isFolder( rURL ) && m_aContent.isValid(); }
    bool                        ContentHasParentFolder( const OUString& rURL );
    bool                        ContentCanMakeFolder( const OUString& rURL );
    bool                        ContentGetTitle( const OUString& rURL, OUString& rTitle );

private:
    SvtFileDialogFilter_Impl*   implAddFilter( const OUString& _rFilter, const OUString& _rType );

    /** updates m_xUserFilter with a new filter
        <p>No checks for necessity are made.</p>
    */
    void                        createNewUserFilter( const OUString& _rNewFilter );

    AdjustFilterFlags           adjustFilter( const OUString& _rFilter );

    // IFilePickerController, needed by OControlAccess
    virtual weld::Widget*       getControl( sal_Int16 nControlId, bool bLabelControl = false ) const override;
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
                                        std::u16string_view _rFilterDefaultExtension,
                                        const OUString& _rFilterExtensions);

    void                        initDefaultPlaces( );
};


inline void SvtFileDialog::SetPath( const OUString& rNewURL )
{
    m_aPath = rNewURL;
}


inline void SvtFileDialog::SetHasFilename( bool bHasFilename )
{
    m_bHasFilename = bHasFilename;
}


inline const OUString& SvtFileDialog::GetPath()
{
    return m_aPath;
}


inline void SvtFileDialog::SetDefaultExt( const OUString& rExt )
{
    m_aDefExt = rExt;
}

inline void SvtFileDialog::EraseDefaultExt( sal_Int32 _nIndex )
{
    m_aDefExt = m_aDefExt.copy( 0, _nIndex );
}

inline const OUString& SvtFileDialog::GetDefaultExt() const
{
    return m_aDefExt;
}


inline SvtFileView* SvtFileDialog::GetView()
{
    return m_xFileView.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
