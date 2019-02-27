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
#ifndef INCLUDED_SFX2_SOURCE_DIALOG_FILEDLGIMPL_HXX
#define INCLUDED_SFX2_SOURCE_DIALOG_FILEDLGIMPL_HXX

#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/graph.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>

class SfxFilterMatcher;
class GraphicFilter;
class FileDialogHelper;

namespace sfx2
{
    class FileDialogHelper_Impl :
        public ::cppu::WeakImplHelper<
            css::ui::dialogs::XFilePickerListener,
            css::ui::dialogs::XDialogClosedListener >
    {
        friend class FileDialogHelper;

        css::uno::Reference < css::ui::dialogs::XFilePicker3 > mxFileDlg;
        css::uno::Reference < css::container::XNameAccess >   mxFilterCFG;

        std::vector< css::beans::StringPair >   maFilters;

        SfxFilterMatcher*           mpMatcher;
        std::unique_ptr<GraphicFilter> mpGraphicFilter;
        FileDialogHelper*           mpAntiImpl;
        weld::Window*               mpFrameWeld;

        ::std::vector< OUString > mlLastURLs;

        OUString             maPath;
        OUString             maFileName;
        OUString             maCurFilter;
        OUString             maSelectFilter;
        OUString             maButtonLabel;

        Idle                        maPreviewIdle;
        Graphic                     maGraphic;

        const short                 m_nDialogType;

        SfxFilterFlags              m_nMustFlags;
        SfxFilterFlags              m_nDontFlags;

        ImplSVEvent *               mnPostUserEventId;

        FileDialogHelper::Context   meContext;

        bool                    mbHasPassword           : 1;
        bool                    mbIsPwdEnabled          : 1;
        bool                    m_bHaveFilterOptions    : 1;
        bool                    mbHasVersions           : 1;
        bool                    mbHasAutoExt            : 1;
        bool                    mbHasPreview            : 1;
        bool                    mbShowPreview           : 1;
        bool                    mbIsSaveDlg             : 1;
        bool                    mbExport                : 1;

        bool                    mbDeleteMatcher         : 1;
        bool                    mbInsert                : 1;
        bool                    mbSystemPicker          : 1;
        bool                    mbPwdCheckBoxState      : 1;
        bool                    mbSelection             : 1;
        bool                    mbSelectionEnabled      : 1;
        bool                    mbHasSelectionBox       : 1;
        bool                    mbSelectionFltrEnabled  : 1;

    private:
        void                    addFilters( const OUString& rFactory,
                                            SfxFilterFlags nMust,
                                            SfxFilterFlags nDont );
        void                    addFilter( const OUString& rFilterName,
                                           const OUString& rExtension );
        void                    addGraphicFilter();
        void                    enablePasswordBox( bool bInit );
        void                    updateFilterOptionsBox();
        void                    updateExportButton();
        void                    updateSelectionBox();
        void                    updateVersions();
        void                    updatePreviewState( bool _bUpdatePreviewWindow );
        void                    dispose();

        void                    loadConfig();
        void                    saveConfig();

        std::shared_ptr<const SfxFilter>        getCurentSfxFilter();
        bool                updateExtendedControl( sal_Int16 _nExtendedControlId, bool _bEnable );

        ErrCode                 getGraphic( const OUString& rURL, Graphic& rGraphic ) const;
        void                    setDefaultValues();

        void                    preExecute();
        void                    postExecute( sal_Int16 _nResult );
        sal_Int16               implDoExecute();
        void                    implStartExecute();

        void                    setControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId );

        bool                    CheckFilterOptionsCapability( const std::shared_ptr<const SfxFilter>& _pFilter );

        bool                    isInOpenMode() const;
        OUString                getCurrentFilterUIName() const;

        void                    LoadLastUsedFilter( const OUString& _rContextIdentifier );
        void                    SaveLastUsedFilter();

        void                    implInitializeFileName( );

        void                    verifyPath( );

        void                    implGetAndCacheFiles( const css::uno::Reference< XInterface >& xPicker  ,
                                                      std::vector<OUString>&                   rpURLList,
                                                      const std::shared_ptr<const SfxFilter>&  pFilter  );

        DECL_LINK( TimeOutHdl_Impl, Timer *, void);
        DECL_LINK( InitControls, void*, void );

    public:
        // XFilePickerListener methods
        virtual void SAL_CALL               fileSelectionChanged( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
        virtual void SAL_CALL               directoryChanged( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
        virtual OUString SAL_CALL           helpRequested( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
        virtual void SAL_CALL               controlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent ) override;
        virtual void SAL_CALL               dialogSizeChanged() override;

        // XDialogClosedListener methods
        virtual void SAL_CALL               dialogClosed( const css::ui::dialogs::DialogClosedEvent& _rEvent ) override;

        // XEventListener methods
        virtual void SAL_CALL       disposing( const css::lang::EventObject& Source ) override;

        // handle XFilePickerListener events
        void                    handleFileSelectionChanged();
        void                    handleDirectoryChanged();
        static OUString         handleHelpRequested( const css::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleControlStateChanged( const css::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleDialogSizeChanged();

        // Own methods
                                FileDialogHelper_Impl(
                                    FileDialogHelper* _pAntiImpl,
                                    const sal_Int16 nDialogType,
                                    FileDialogFlags nFlags,
                                    sal_Int16 nDialog,
                                    weld::Window* pFrameWeld,
                                    const OUString& sStandardDir = OUString(),
                                    const css::uno::Sequence< OUString >&   rBlackList = css::uno::Sequence< OUString >()
                                );
        virtual                 ~FileDialogHelper_Impl() override;

        ErrCode                 execute( std::vector<OUString>& rpURLList,
                                         std::unique_ptr<SfxItemSet>& rpSet,
                                         OUString&       rFilter );
        ErrCode                 execute();

        void                    setFilter( const OUString& rFilter );

        /** sets the directory which should be browsed

            <p>If the given path does not point to a valid (existent and accessible) folder, the request
            is silently dropped</p>
        */
        void                    displayFolder( const OUString& rPath );
        void                    setFileName( const OUString& _rFile );

        OUString                getPath() const;
        OUString                getFilter() const;
        void                    getRealFilter( OUString& _rFilter ) const;

        ErrCode                 getGraphic( Graphic& rGraphic ) const;
        void                    createMatcher( const OUString& rFactory );

        bool                    isShowFilterExtensionEnabled() const;
        void                    addFilterPair( const OUString& rFilter,
                                               const OUString& rFilterWithExtension );
        OUString                getFilterName( const OUString& rFilterWithExtension ) const;
        OUString                getFilterWithExtension( const OUString& rFilter ) const;

        void                    SetContext( FileDialogHelper::Context _eNewContext );

        bool             isSystemFilePicker() const { return mbSystemPicker; }
        bool             isPasswordEnabled() const { return mbIsPwdEnabled; }

        css::uno::Reference<css::awt::XWindow> GetFrameInterface();
    };

}   // end of namespace sfx2

#endif // INCLUDED_SFX2_SOURCE_DIALOG_FILEDLGIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
