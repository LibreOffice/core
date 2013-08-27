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
#ifndef _SFX_FILEDLGIMPL_HXX
#define _SFX_FILEDLGIMPL_HXX

#include <vcl/timer.hxx>
#include <vcl/graph.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/sequenceasvector.hxx>

class SfxFilterMatcher;
class GraphicFilter;
class FileDialogHelper;

namespace sfx2
{
    typedef ::com::sun::star::beans::StringPair FilterPair;

    class FileDialogHelper_Impl :
        public ::cppu::WeakImplHelper2<
            ::com::sun::star::ui::dialogs::XFilePickerListener,
            ::com::sun::star::ui::dialogs::XDialogClosedListener >
    {
        friend class FileDialogHelper;

        ::com::sun::star::uno::Reference < ::com::sun::star::ui::dialogs::XFilePicker > mxFileDlg;
        ::com::sun::star::uno::Reference < ::com::sun::star::container::XNameAccess >   mxFilterCFG;

        std::vector< FilterPair >   maFilters;

        SfxFilterMatcher*           mpMatcher;
        GraphicFilter*              mpGraphicFilter;
        FileDialogHelper*           mpAntiImpl;
        Window*                     mpPreferredParentWindow;

        ::comphelper::SequenceAsVector< OUString > mlLastURLs;

        OUString             maPath;
        OUString             maFileName;
        OUString             maCurFilter;
        OUString             maSelectFilter;
        OUString             maButtonLabel;

        Timer                       maPreViewTimer;
        Graphic                     maGraphic;

        const short                 m_nDialogType;

        SfxFilterFlags              m_nMustFlags;
        SfxFilterFlags              m_nDontFlags;

        sal_uIntPtr                     mnPostUserEventId;

        ErrCode                     mnError;

        FileDialogHelper::Context   meContext;

        sal_Bool                    mbHasPassword           : 1;
        sal_Bool                    mbIsPwdEnabled          : 1;
        sal_Bool                    m_bHaveFilterOptions    : 1;
        sal_Bool                    mbHasVersions           : 1;
        sal_Bool                    mbHasAutoExt            : 1;
        sal_Bool                    mbAddGraphicFilter      : 1;
        sal_Bool                    mbHasPreview            : 1;
        sal_Bool                    mbShowPreview           : 1;
        sal_Bool                    mbIsSaveDlg             : 1;
        sal_Bool                    mbIsSaveACopyDlg        : 1;
        sal_Bool                    mbExport                : 1;

        sal_Bool                    mbDeleteMatcher         : 1;
        sal_Bool                    mbInsert                : 1;
        sal_Bool                    mbSystemPicker          : 1;
        sal_Bool                    mbPwdCheckBoxState      : 1;
        sal_Bool                    mbSelection             : 1;
        sal_Bool                    mbSelectionEnabled      : 1;
        sal_Bool                    mbHasSelectionBox       : 1;
        sal_Bool                    mbSelectionFltrEnabled  : 1;

    private:
        void                    addFilters( const String& rFactory,
                                            SfxFilterFlags nMust,
                                            SfxFilterFlags nDont );
        void                    addFilter( const OUString& rFilterName,
                                           const OUString& rExtension );
        void                    addGraphicFilter();
        void                    enablePasswordBox( sal_Bool bInit );
        void                    updateFilterOptionsBox();
        void                    updateExportButton();
        void                    updateSelectionBox();
        void                    updateVersions();
        void                    updatePreviewState( sal_Bool _bUpdatePreviewWindow = sal_True );
        void                    dispose();

        void                    loadConfig();
        void                    saveConfig();

        const SfxFilter*        getCurentSfxFilter();
        sal_Bool                updateExtendedControl( sal_Int16 _nExtendedControlId, sal_Bool _bEnable );

        ErrCode                 getGraphic( const OUString& rURL, Graphic& rGraphic ) const;
        void                    setDefaultValues();

        void                    preExecute();
        void                    postExecute( sal_Int16 _nResult );
        sal_Int16               implDoExecute();
        void                    implStartExecute();

        void                    correctVirtualDialogType();

        void                    setControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId );

        sal_Bool                CheckFilterOptionsCapability( const SfxFilter* _pFilter );

        sal_Bool                isInOpenMode() const;
        String                  getCurrentFilterUIName() const;

        void                    LoadLastUsedFilter( const OUString& _rContextIdentifier );
        void                    SaveLastUsedFilter( const OUString& _rContextIdentifier );
        void                    SaveLastUsedFilter( void );

        void                    implInitializeFileName( );

        void                    verifyPath( );

        void                    implGetAndCacheFiles( const ::com::sun::star::uno::Reference< XInterface >& xPicker  ,
                                                      std::vector<OUString>&               rpURLList,
                                                      const SfxFilter*                    pFilter  );

        DECL_LINK(TimeOutHdl_Impl, void *);
        DECL_LINK( HandleEvent, FileDialogHelper* );
        DECL_LINK( InitControls, void* );

    public:
        // XFilePickerListener methods
        virtual void SAL_CALL               fileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               directoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual OUString SAL_CALL    helpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               controlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               dialogSizeChanged() throw( ::com::sun::star::uno::RuntimeException );

        // XDialogClosedListener methods
        virtual void SAL_CALL               dialogClosed( const ::com::sun::star::ui::dialogs::DialogClosedEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener methods
        virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );

        // handle XFilePickerListener events
        void                    handleFileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleDirectoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        OUString         handleHelpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleControlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleDialogSizeChanged();

        // Own methods
                                FileDialogHelper_Impl(
                                    FileDialogHelper* _pAntiImpl,
                                    const short nDialogType,
                                    sal_Int64 nFlags,
                                    sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG,
                                    Window* _pPreferredParentWindow = NULL,
                                    const String& sStandardDir = OUString(),
                                    const ::com::sun::star::uno::Sequence< OUString >&   rBlackList = ::com::sun::star::uno::Sequence< OUString >()
                                );
        virtual                 ~FileDialogHelper_Impl();

        ErrCode                 execute( std::vector<OUString>& rpURLList,
                                         SfxItemSet *&   rpSet,
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
        void                    createMatcher( const String& rFactory );

        sal_Bool                isShowFilterExtensionEnabled() const;
        void                    addFilterPair( const OUString& rFilter,
                                               const OUString& rFilterWithExtension );
        OUString         getFilterName( const OUString& rFilterWithExtension ) const;
        OUString         getFilterWithExtension( const OUString& rFilter ) const;

        void                    SetContext( FileDialogHelper::Context _eNewContext );

        inline sal_Bool         isSystemFilePicker() const { return mbSystemPicker; }
        inline sal_Bool         isPasswordEnabled() const { return mbIsPwdEnabled; }
    };

}   // end of namespace sfx2

#endif // _SFX_FILEDLGIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
