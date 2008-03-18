/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filedlgimpl.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 17:38:49 $
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
#ifndef _SFX_FILEDLGIMPL_HXX
#define _SFX_FILEDLGIMPL_HXX

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#ifndef  _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_STRINGPAIR_HPP_
#include <com/sun/star/beans/StringPair.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XDIALOGCLOSEDLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#endif

#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

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

        ULONG                       mnPostUserEventId;

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

        void                    setControlHelpIds( const sal_Int16* _pControlId, const sal_Int32* _pHelpId );
        void                    setDialogHelpId( const sal_Int32 _nHelpId );

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
                                    const String& sStandardDir = String::CreateFromAscii( "" )
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

