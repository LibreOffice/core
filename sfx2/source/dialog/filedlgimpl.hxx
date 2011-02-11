/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

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

        ::comphelper::SequenceAsVector< ::rtl::OUString > mlLastURLs;

        ::rtl::OUString             maPath;
        ::rtl::OUString             maFileName;
        ::rtl::OUString             maCurFilter;
        ::rtl::OUString             maSelectFilter;
        ::rtl::OUString             maButtonLabel;

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
        sal_Bool                    mbHasLink               : 1;
        sal_Bool                    mbHasPreview            : 1;
        sal_Bool                    mbShowPreview           : 1;
        sal_Bool                    mbIsSaveDlg             : 1;
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
        void                    addFilters( sal_Int64 nFlags,
                                            const String& rFactory,
                                            SfxFilterFlags nMust,
                                            SfxFilterFlags nDont );
        void                    addFilter( const ::rtl::OUString& rFilterName,
                                           const ::rtl::OUString& rExtension );
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

        ErrCode                 getGraphic( const ::rtl::OUString& rURL, Graphic& rGraphic ) const;
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

        void                    LoadLastUsedFilter( const ::rtl::OUString& _rContextIdentifier );
        void                    SaveLastUsedFilter( const ::rtl::OUString& _rContextIdentifier );
        void                    SaveLastUsedFilter( void );

        void                    implInitializeFileName( );

        void                    implGetAndCacheFiles( const ::com::sun::star::uno::Reference< XInterface >& xPicker  ,
                                                            SvStringsDtor*&               rpURLList,
                                                      const SfxFilter*                    pFilter  );
        String                  implEnsureURLExtension(const String& sURL      ,
                                                       const String& sExtension);

        DECL_LINK( TimeOutHdl_Impl, Timer* );
        DECL_LINK( HandleEvent, FileDialogHelper* );
        DECL_LINK( InitControls, void* );

    public:
        // XFilePickerListener methods
        virtual void SAL_CALL               fileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               directoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString SAL_CALL    helpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               controlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL               dialogSizeChanged() throw( ::com::sun::star::uno::RuntimeException );

        // XDialogClosedListener methods
        virtual void SAL_CALL               dialogClosed( const ::com::sun::star::ui::dialogs::DialogClosedEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener methods
        virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );

        // handle XFilePickerListener events
        void                    handleFileSelectionChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleDirectoryChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        ::rtl::OUString         handleHelpRequested( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleControlStateChanged( const ::com::sun::star::ui::dialogs::FilePickerEvent& aEvent );
        void                    handleDialogSizeChanged();

        // Own methods
                                FileDialogHelper_Impl(
                                    FileDialogHelper* _pAntiImpl,
                                    const short nDialogType,
                                    sal_Int64 nFlags,
                                    sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG,
                                    Window* _pPreferredParentWindow = NULL,
                                    const String& sStandardDir = String::CreateFromAscii( "" ),
                                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >&   rBlackList = ::com::sun::star::uno::Sequence< ::rtl::OUString >()
                                );
        virtual                 ~FileDialogHelper_Impl();

        ErrCode                 execute( SvStringsDtor*& rpURLList,
                                         SfxItemSet *&   rpSet,
                                         String&         rFilter );
        ErrCode                 execute();

        void                    setFilter( const ::rtl::OUString& rFilter );

        /** sets the directory which should be browsed

            <p>If the given path does not point to a valid (existent and accessible) folder, the request
            is silently dropped</p>
        */
        void                    displayFolder( const ::rtl::OUString& rPath );
        void                    setFileName( const ::rtl::OUString& _rFile );

        ::rtl::OUString         getPath() const;
        ::rtl::OUString         getFilter() const;
        void                    getRealFilter( String& _rFilter ) const;

        ErrCode                 getGraphic( Graphic& rGraphic ) const;
        void                    createMatcher( const String& rFactory );

        sal_Bool                isShowFilterExtensionEnabled() const;
        void                    addFilterPair( const ::rtl::OUString& rFilter,
                                               const ::rtl::OUString& rFilterWithExtension );
        ::rtl::OUString         getFilterName( const ::rtl::OUString& rFilterWithExtension ) const;
        ::rtl::OUString         getFilterWithExtension( const ::rtl::OUString& rFilter ) const;

        void                    SetContext( FileDialogHelper::Context _eNewContext );

        inline sal_Bool         isSystemFilePicker() const { return mbSystemPicker; }
        inline sal_Bool         isPasswordEnabled() const { return mbIsPwdEnabled; }
    };

}   // end of namespace sfx2

#endif // _SFX_FILEDLGIMPL_HXX

