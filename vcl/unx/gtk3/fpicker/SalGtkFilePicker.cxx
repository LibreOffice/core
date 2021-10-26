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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <config_gio.h>

#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <osl/diagnose.h>
#include <rtl/process.h>
#include <sal/log.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <vcl/svapp.hxx>

#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>

#include <algorithm>
#include <set>
#include <string.h>
#include <string_view>

#include "SalGtkFilePicker.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

SalGtkFilePicker::SalGtkFilePicker( const uno::Reference< uno::XComponentContext >& xContext ) :
    SalGtkPicker( xContext ),
    SalGtkFilePicker_Base( m_rbHelperMtx ),
    mnHID_FolderChange( 0 ),
    mnHID_SelectionChange( 0 ),
    bVersionWidthUnset( false ),
    mbInitialized(false),
    mHID_Preview( 0 ),
    m_pPseudoFilter( nullptr )
{
    OUString aFilePickerTitle = getResString( FILE_PICKER_TITLE_OPEN );

    m_pDialog = GTK_FILE_CHOOSER_NATIVE(g_object_new(GTK_TYPE_FILE_CHOOSER_NATIVE,
                                        "title", OUStringToOString(aFilePickerTitle, RTL_TEXTENCODING_UTF8).getStr(),
                                        "action", GTK_FILE_CHOOSER_ACTION_OPEN,
                                        nullptr));

    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(m_pDialog), true);

#if !GTK_CHECK_VERSION(4, 0, 0)
#if ENABLE_GIO
    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( m_pDialog ), false );
#endif
#endif

    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( m_pDialog ), false );

    OUString aLabel;

    aLabel = getResString( FILE_PICKER_FILE_TYPE );

    m_pFilterStore = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING);

    g_signal_connect( G_OBJECT( m_pDialog ), "notify::filter",
                      G_CALLBACK( filter_changed_cb ), this);
}

// XFilePickerNotifier

void SAL_CALL SalGtkFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
{
    SolarMutexGuard g;

    OSL_ENSURE(!m_xListener.is(),
            "SalGtkFilePicker only talks with one listener at a time...");
    m_xListener = xListener;
}

void SAL_CALL SalGtkFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
{
    SolarMutexGuard g;

    m_xListener.clear();
}

// FilePicker Event functions

void SalGtkFilePicker::impl_fileSelectionChanged( const FilePickerEvent& aEvent )
{
    if (m_xListener.is()) m_xListener->fileSelectionChanged( aEvent );
}

void SalGtkFilePicker::impl_directoryChanged( const FilePickerEvent& aEvent )
{
    if (m_xListener.is()) m_xListener->directoryChanged( aEvent );
}

void SalGtkFilePicker::impl_controlStateChanged( const FilePickerEvent& aEvent )
{
    if (m_xListener.is()) m_xListener->controlStateChanged( aEvent );
}

struct FilterEntry
{
protected:
    OUString     m_sTitle;
    OUString     m_sFilter;

    css::uno::Sequence< css::beans::StringPair >       m_aSubFilters;

public:
    FilterEntry( const OUString& _rTitle, const OUString& _rFilter )
        :m_sTitle( _rTitle )
        ,m_sFilter( _rFilter )
    {
    }

    const OUString& getTitle() const { return m_sTitle; }
    const OUString& getFilter() const { return m_sFilter; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    bool        hasSubFilters( ) const;

    /** retrieves the filters belonging to the entry
    */
    void       getSubFilters( css::uno::Sequence< css::beans::StringPair >& _rSubFilterList );

    // helpers for iterating the sub filters
    const css::beans::StringPair*   beginSubFilters() const { return m_aSubFilters.begin(); }
    const css::beans::StringPair*   endSubFilters() const { return m_aSubFilters.end(); }
};

bool FilterEntry::hasSubFilters() const
{
    return m_aSubFilters.hasElements();
}

void FilterEntry::getSubFilters( css::uno::Sequence< css::beans::StringPair >& _rSubFilterList )
{
    _rSubFilterList = m_aSubFilters;
}

static bool
isFilterString( const OUString &rFilterString, const char *pMatch )
{
        sal_Int32 nIndex = 0;
        OUString aToken;
        bool bIsFilter = true;

        OUString aMatch(OUString::createFromAscii(pMatch));

        do
        {
            aToken = rFilterString.getToken( 0, ';', nIndex );
            if( !aToken.match( aMatch ) )
            {
                bIsFilter = false;
                break;
            }
        }
        while( nIndex >= 0 );

        return bIsFilter;
}

static OUString
shrinkFilterName( const OUString &rFilterName, bool bAllowNoStar = false )
{
    int i;
    int nBracketLen = -1;
    int nBracketEnd = -1;
    const sal_Unicode *pStr = rFilterName.getStr();
    OUString aRealName = rFilterName;

    for( i = aRealName.getLength() - 1; i > 0; i-- )
    {
        if( pStr[i] == ')' )
            nBracketEnd = i;
        else if( pStr[i] == '(' )
        {
            nBracketLen = nBracketEnd - i;
            if( nBracketEnd <= 0 )
                continue;
            if( isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ), "*." ) )
                aRealName = aRealName.replaceAt( i, nBracketLen + 1, OUString() );
            else if (bAllowNoStar)
            {
                if( isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ), ".") )
                    aRealName = aRealName.replaceAt( i, nBracketLen + 1, OUString() );
            }
        }
    }

    return aRealName;
}

namespace {

    struct FilterTitleMatch
    {
    protected:
        const OUString& rTitle;

    public:
        explicit FilterTitleMatch( const OUString& _rTitle ) : rTitle( _rTitle ) { }

        bool operator () ( const FilterEntry& _rEntry )
        {
            bool bMatch;
            if( !_rEntry.hasSubFilters() )
                // a real filter
                bMatch = (_rEntry.getTitle() == rTitle)
                      || (shrinkFilterName(_rEntry.getTitle()) == rTitle);
            else
                // a filter group -> search the sub filters
                bMatch =
                    ::std::any_of(
                        _rEntry.beginSubFilters(),
                        _rEntry.endSubFilters(),
                        *this
                    );

            return bMatch;
        }
        bool operator () ( const css::beans::StringPair& _rEntry )
        {
            OUString aShrunkName = shrinkFilterName( _rEntry.First );
            return aShrunkName == rTitle;
        }
    };
}

bool SalGtkFilePicker::FilterNameExists( const OUString& rTitle )
{
    bool bRet = false;

    if( m_pFilterVector )
        bRet =
            ::std::any_of(
                m_pFilterVector->begin(),
                m_pFilterVector->end(),
                FilterTitleMatch( rTitle )
            );

    return bRet;
}

bool SalGtkFilePicker::FilterNameExists( const css::uno::Sequence< css::beans::StringPair >& _rGroupedFilters )
{
    bool bRet = false;

    if( m_pFilterVector )
    {
        bRet = std::any_of(_rGroupedFilters.begin(), _rGroupedFilters.end(),
            [&](const css::beans::StringPair& rFilter) {
                return ::std::any_of( m_pFilterVector->begin(), m_pFilterVector->end(), FilterTitleMatch( rFilter.First ) ); });
    }

    return bRet;
}

void SalGtkFilePicker::ensureFilterVector( const OUString& _rInitialCurrentFilter )
{
    if( !m_pFilterVector )
    {
        m_pFilterVector.reset( new std::vector<FilterEntry> );

        // set the first filter to the current filter
        if ( m_aCurrentFilter.isEmpty() )
            m_aCurrentFilter = _rInitialCurrentFilter;
    }
}

void SAL_CALL SalGtkFilePicker::appendFilter( const OUString& aTitle, const OUString& aFilter )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    if( FilterNameExists( aTitle ) )
            throw IllegalArgumentException();

    // ensure that we have a filter list
    ensureFilterVector( aTitle );

    // append the filter
    m_pFilterVector->insert( m_pFilterVector->end(), FilterEntry( aTitle, aFilter ) );
}

void SAL_CALL SalGtkFilePicker::setCurrentFilter( const OUString& aTitle )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    if( aTitle != m_aCurrentFilter )
    {
        m_aCurrentFilter = aTitle;
        SetCurFilter( m_aCurrentFilter );
    }

    // TODO m_pImpl->setCurrentFilter( aTitle );
}

void SalGtkFilePicker::updateCurrentFilterFromName(const gchar* filtername)
{
    OUString aFilterName(filtername, strlen(filtername), RTL_TEXTENCODING_UTF8);
    if (m_pFilterVector)
    {
        for (auto const& filter : *m_pFilterVector)
        {
            if (aFilterName == shrinkFilterName(filter.getTitle()))
            {
                m_aCurrentFilter = filter.getTitle();
                break;
            }
        }
    }
}

void SalGtkFilePicker::UpdateFilterfromUI()
{
    // Update the filtername from the users selection if they have had a chance to do so.
    // If the user explicitly sets a type then use that, if not then take the implicit type
    // from the filter of the files glob on which he is currently searching
    if (!mnHID_FolderChange || !mnHID_SelectionChange)
        return;

    if( GtkFileFilter *filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(m_pDialog)))
    {
        if (m_pPseudoFilter != filter)
            updateCurrentFilterFromName(gtk_file_filter_get_name( filter ));
        else
            updateCurrentFilterFromName(OUStringToOString( m_aInitialFilter, RTL_TEXTENCODING_UTF8 ).getStr());
    }
}

OUString SAL_CALL SalGtkFilePicker::getCurrentFilter()
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    UpdateFilterfromUI();

    return m_aCurrentFilter;
}

// XFilterGroupManager functions

void SAL_CALL SalGtkFilePicker::appendFilterGroup( const OUString& /*sGroupTitle*/, const uno::Sequence<beans::StringPair>& aFilters )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    // TODO m_pImpl->appendFilterGroup( sGroupTitle, aFilters );
    // check the names
    if( FilterNameExists( aFilters ) )
        // TODO: a more precise exception message
            throw IllegalArgumentException();

    // ensure that we have a filter list
    OUString sInitialCurrentFilter;
    if( aFilters.hasElements() )
        sInitialCurrentFilter = aFilters[0].First;

    ensureFilterVector( sInitialCurrentFilter );

    // append the filter
    for( const auto& rSubFilter : aFilters )
        m_pFilterVector->insert( m_pFilterVector->end(), FilterEntry( rSubFilter.First, rSubFilter.Second ) );

}

// XFilePicker functions

void SAL_CALL SalGtkFilePicker::setMultiSelectionMode( sal_Bool bMode )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER(m_pDialog), bMode );
}

void SAL_CALL SalGtkFilePicker::setDefaultName( const OUString& aName )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    OString aStr = OUStringToOString( aName, RTL_TEXTENCODING_UTF8 );
    GtkFileChooserAction eAction = gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) );

    // set_current_name launches a Gtk critical error if called for other than save
    if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
        gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( m_pDialog ), aStr.getStr() );
}

void SAL_CALL SalGtkFilePicker::setDisplayDirectory( const OUString& rDirectory )
{
    SolarMutexGuard g;

    implsetDisplayDirectory(rDirectory);
}

OUString SAL_CALL SalGtkFilePicker::getDisplayDirectory()
{
    SolarMutexGuard g;

    return implgetDisplayDirectory();
}

uno::Sequence<OUString> SAL_CALL SalGtkFilePicker::getFiles()
{
    // no member access => no mutex needed

    uno::Sequence< OUString > aFiles = getSelectedFiles();
    /*
      The previous multiselection API design was completely broken
      and unimplementable for some heterogeneous pseudo-URIs eg. search:
      Thus crop unconditionally to a single selection.
    */
    aFiles.realloc (1);
    return aFiles;
}

namespace
{

bool lcl_matchFilter( const OUString& rFilter, const OUString& rExt )
{
    const sal_Unicode cSep {';'};
    sal_Int32 nIdx {0};

    for (;;)
    {
        const sal_Int32 nBegin = rFilter.indexOf(rExt, nIdx);

        if (nBegin<0) // not found
            break;

        // Let nIdx point to end of matched string, useful in order to
        // check string boundaries and also for a possible next iteration
        nIdx = nBegin + rExt.getLength();

        // Check if the found occurrence is an exact match: right side
        if (nIdx<rFilter.getLength() && rFilter[nIdx]!=cSep)
            continue;

        // Check if the found occurrence is an exact match: left side
        if (nBegin>0 && rFilter[nBegin-1]!=cSep)
            continue;

        return true;
    }

    return false;
}

}

uno::Sequence<OUString> SAL_CALL SalGtkFilePicker::getSelectedFiles()
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

#if !GTK_CHECK_VERSION(4, 0, 0)
    GSList* pPathList = gtk_file_chooser_get_uris( GTK_FILE_CHOOSER(m_pDialog) );
    int nCount = g_slist_length( pPathList );
#else
    GListModel* pPathList = gtk_file_chooser_get_files(GTK_FILE_CHOOSER(m_pDialog));
    int nCount = g_list_model_get_n_items(pPathList);
#endif

    int nIndex = 0;

    // get the current action setting
    GtkFileChooserAction eAction = gtk_file_chooser_get_action(
        GTK_FILE_CHOOSER( m_pDialog ));

    uno::Sequence< OUString > aSelectedFiles(nCount);

    // Convert to OOo
#if !GTK_CHECK_VERSION(4, 0, 0)
    for( GSList *pElem = pPathList; pElem; pElem = pElem->next)
    {
        gchar *pURI = static_cast<gchar*>(pElem->data);
#else
    while (gpointer pElem = g_list_model_get_item(pPathList, nIndex))
    {
        gchar *pURI = g_file_get_uri(static_cast<GFile*>(pElem));
#endif

        aSelectedFiles[ nIndex ] = uritounicode(pURI);

        if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
        {
            OUString sFilterName;
            sal_Int32 nTokenIndex = 0;
            bool bExtensionTypedIn = false;

            if( aSelectedFiles[nIndex].indexOf('.') > 0 )
            {
                OUString sExtension;
                nTokenIndex = 0;
                do
                    sExtension = aSelectedFiles[nIndex].getToken( 0, '.', nTokenIndex );
                while( nTokenIndex >= 0 );

                if( sExtension.getLength() >= 3 ) // 3 = typical/minimum extension length
                {
                    OUString aNewFilter;
                    OUString aOldFilter = getCurrentFilter();
                    bool bChangeFilter = true;
                    if ( m_pFilterVector)
                        for (auto const& filter : *m_pFilterVector)
                        {
                            if( lcl_matchFilter( filter.getFilter(), "*." + sExtension ) )
                            {
                                if( aNewFilter.isEmpty() )
                                    aNewFilter = filter.getTitle();

                                if( aOldFilter == filter.getTitle() )
                                    bChangeFilter = false;

                                bExtensionTypedIn = true;
                            }
                        }
                    if( bChangeFilter && bExtensionTypedIn )
                    {
                        gchar* pCurrentName = gtk_file_chooser_get_current_name(GTK_FILE_CHOOSER(m_pDialog));
                        setCurrentFilter( aNewFilter );
                        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(m_pDialog), pCurrentName);
                        g_free(pCurrentName);
                    }
                }
            }

            GtkFileFilter *filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(m_pDialog));
            if (m_pPseudoFilter != filter)
            {
                const gchar* filtername = filter ? gtk_file_filter_get_name(filter) : nullptr;
                if (filtername)
                    sFilterName = OUString(filtername, strlen( filtername ), RTL_TEXTENCODING_UTF8);
                else
                    sFilterName.clear();
            }
            else
                sFilterName = m_aInitialFilter;

            if (m_pFilterVector)
            {
                auto aVectorIter = ::std::find_if(
                    m_pFilterVector->begin(), m_pFilterVector->end(), FilterTitleMatch(sFilterName) );

                OUString aFilter;
                if (aVectorIter != m_pFilterVector->end())
                    aFilter = aVectorIter->getFilter();

                nTokenIndex = 0;
                OUString sToken;
                do
                {
                    sToken = aFilter.getToken( 0, '.', nTokenIndex );

                    if ( sToken.lastIndexOf( ';' ) != -1 )
                    {
                        sToken = sToken.getToken(0, ';');
                        break;
                    }
                }
                while( nTokenIndex >= 0 );

                if( !bExtensionTypedIn && ( sToken != "*" ) )
                {
                    //if the filename does not already have the auto extension, stick it on
                    OUString sExtension = "." + sToken;
                    OUString &rBase = aSelectedFiles[nIndex];
                    sal_Int32 nExtensionIdx = rBase.getLength() - sExtension.getLength();
                    SAL_INFO(
                        "vcl.gtk",
                        "idx are " << rBase.lastIndexOf(sExtension) << " "
                        << nExtensionIdx);

                    if( rBase.lastIndexOf( sExtension ) != nExtensionIdx )
                        rBase += sExtension;
                }
            }

        }

        nIndex++;
        g_free( pURI );
    }

#if !GTK_CHECK_VERSION(4, 0, 0)
    g_slist_free( pPathList );
#else
    g_object_unref(pPathList);
#endif

    return aSelectedFiles;
}

// XExecutableDialog functions

void SAL_CALL SalGtkFilePicker::setTitle( const OUString& rTitle )
{
    SolarMutexGuard g;

    implsetTitle(rTitle);
}

sal_Int16 SAL_CALL SalGtkFilePicker::execute()
{
    SolarMutexGuard g;

    if (!mbInitialized)
    {
        // tdf#144084 if not initialized default to FILEOPEN_SIMPLE
        impl_initialize(nullptr, FILEOPEN_SIMPLE);
        assert(mbInitialized);
    }

    OSL_ASSERT( m_pDialog != nullptr );

    sal_Int16 retVal = 0;

    SetFilters();

    // tdf#84431 - set the filter after the corresponding widget is created
    if ( !m_aCurrentFilter.isEmpty() )
        SetCurFilter(m_aCurrentFilter);

#if !GTK_CHECK_VERSION(4, 0, 0)
    mnHID_FolderChange =
        g_signal_connect( GTK_FILE_CHOOSER( m_pDialog ), "current-folder-changed",
            G_CALLBACK( folder_changed_cb ), static_cast<gpointer>(this) );
#else
    // no replacement in 4-0 that I can see :-(
    mnHID_FolderChange = 0;
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
    mnHID_SelectionChange =
        g_signal_connect( GTK_FILE_CHOOSER( m_pDialog ), "selection-changed",
            G_CALLBACK( selection_changed_cb ), static_cast<gpointer>(this) );
#else
    // no replacement in 4-0 that I can see :-(
    mnHID_SelectionChange = 0;
#endif

    int btn = GTK_RESPONSE_NO;

    uno::Reference< awt::XExtendedToolkit > xToolkit(
        awt::Toolkit::create(m_xContext),
        UNO_QUERY_THROW );

    uno::Reference< frame::XDesktop > xDesktop(
        frame::Desktop::create(m_xContext),
        UNO_QUERY_THROW );

    GtkWindow *pParent = GTK_WINDOW(m_pParentWidget);
    if (!pParent)
    {
        SAL_WARN( "vcl.gtk", "no parent widget set");
        pParent = RunDialog::GetTransientFor();
    }
    if (pParent)
        gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(m_pDialog), pParent);
    rtl::Reference<RunDialog> pRunDialog = new RunDialog(GTK_NATIVE_DIALOG(m_pDialog), xToolkit, xDesktop);
    while( GTK_RESPONSE_NO == btn )
    {
        btn = GTK_RESPONSE_YES; // we don't want to repeat unless user clicks NO for file save.

        gint nStatus = pRunDialog->run();
        switch( nStatus )
        {
            case GTK_RESPONSE_ACCEPT:
                if( GTK_FILE_CHOOSER_ACTION_SAVE == gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) ) )
                {
                    Sequence < OUString > aPathSeq = getFiles();
                    if( aPathSeq.getLength() == 1 )
                    {
                        if( btn == GTK_RESPONSE_YES )
                            retVal = ExecutableDialogResults::OK;
                    }
                }
                else
                    retVal = ExecutableDialogResults::OK;
                break;

            case GTK_RESPONSE_CANCEL:
                retVal = ExecutableDialogResults::CANCEL;
                break;

            case 1: //PLAY
                {
                    FilePickerEvent evt;
                    evt.ElementId = PUSHBUTTON_PLAY;
                    impl_controlStateChanged( evt );
                    btn = GTK_RESPONSE_NO;
                }
                break;

            default:
                retVal = 0;
                break;
        }
    }

    gtk_native_dialog_hide(GTK_NATIVE_DIALOG( m_pDialog ));

    if (mnHID_FolderChange)
        g_signal_handler_disconnect(GTK_FILE_CHOOSER( m_pDialog ), mnHID_FolderChange);
    if (mnHID_SelectionChange)
        g_signal_handler_disconnect(GTK_FILE_CHOOSER( m_pDialog ), mnHID_SelectionChange);

    return retVal;
}

void SalGtkFilePicker::filter_changed_cb( GtkFileChooser *, GParamSpec *,
    SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    SAL_INFO( "vcl.gtk", "filter_changed, isn't it great " << pobjFP );
    pobjFP->impl_controlStateChanged( evt );
}

void SalGtkFilePicker::folder_changed_cb( GtkFileChooser *, SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    SAL_INFO( "vcl.gtk", "folder_changed, isn't it great " << pobjFP );
    pobjFP->impl_directoryChanged( evt );
}

void SalGtkFilePicker::selection_changed_cb( GtkFileChooser *, SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    SAL_INFO( "vcl.gtk", "selection_changed, isn't it great " << pobjFP );
    pobjFP->impl_fileSelectionChanged( evt );
}

GtkWidget* SalGtkPicker::GetParentWidget(const uno::Sequence<uno::Any>& rArguments)
{
    GtkWidget* pParentWidget = nullptr;

    css::uno::Reference<css::awt::XWindow> xParentWindow;
    if (rArguments.getLength() > 1)
    {
        rArguments[1] >>= xParentWindow;
    }

    if (xParentWindow.is())
    {
        if (SalGtkXWindow* pGtkXWindow = dynamic_cast<SalGtkXWindow*>(xParentWindow.get()))
            pParentWidget = pGtkXWindow->getGtkWidget();
        else
        {
            css::uno::Reference<css::awt::XSystemDependentWindowPeer> xSysDepWin(xParentWindow, css::uno::UNO_QUERY);
            if (xSysDepWin.is())
            {
                css::uno::Sequence<sal_Int8> aProcessIdent(16);
                rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8*>(aProcessIdent.getArray()));
                uno::Any aAny = xSysDepWin->getWindowHandle(aProcessIdent, css::lang::SystemDependent::SYSTEM_XWINDOW);
                css::awt::SystemDependentXWindow tmp;
                aAny >>= tmp;
                pParentWidget = GetGtkSalData()->GetGtkDisplay()->findGtkWidgetForNativeHandle(tmp.WindowHandle);
            }
        }
    }

    return pParentWidget;
}

// XInitialization

void SAL_CALL SalGtkFilePicker::initialize( const uno::Sequence<uno::Any>& aArguments )
{
    // parameter checking
    uno::Any aAny;
    if( !aArguments.hasElements() )
        throw lang::IllegalArgumentException(
            "no arguments",
            static_cast<XFilePicker2*>( this ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != cppu::UnoType<sal_Int16>::get()) &&
         (aAny.getValueType() != cppu::UnoType<sal_Int8>::get()) )
         throw lang::IllegalArgumentException(
            "invalid argument type",
            static_cast<XFilePicker2*>( this ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    impl_initialize(GetParentWidget(aArguments), templateId);
}

void SalGtkFilePicker::impl_initialize(GtkWidget* pParentWidget, sal_Int16 templateId)
{
    m_pParentWidget = pParentWidget;

    GtkFileChooserAction eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
    OString sOpen = getOpenText();
    OString sSave = getSaveText();

    const gchar *accept_label_text = sOpen.getStr();

    SolarMutexGuard g;

    //   TODO: extract full semantic from
    //   svtools/source/filepicker/filepicker.cxx (getWinBits)
    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            break;
        case FILESAVE_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_label_text = sSave.getStr();
                break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_label_text = sSave.getStr();
            // TODO
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_label_text = sSave.getStr();
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE; // SELECT_FOLDER ?
            accept_label_text = sSave.getStr();
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_label_text = sSave.getStr();
            // TODO
                break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        case FILEOPEN_PLAY:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        case FILEOPEN_LINK_PLAY:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        case FILEOPEN_READONLY_VERSION:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            break;
        case FILEOPEN_LINK_PREVIEW:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            accept_label_text = sSave.getStr();
            // TODO
                break;
        case FILEOPEN_PREVIEW:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            accept_label_text = sOpen.getStr();
            // TODO
                break;
        default:
                throw lang::IllegalArgumentException(
                "Unknown template",
                static_cast< XFilePicker2* >( this ),
                1 );
    }

    if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
    {
        OUString aFilePickerTitle(getResString( FILE_PICKER_TITLE_SAVE ));
        gtk_native_dialog_set_title ( GTK_NATIVE_DIALOG( m_pDialog ),
            OUStringToOString( aFilePickerTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    gtk_file_chooser_native_set_accept_label ( m_pDialog, accept_label_text );
    gtk_file_chooser_set_action( GTK_FILE_CHOOSER( m_pDialog ), eAction);

    mbInitialized = true;
}

// XCancellable

void SAL_CALL SalGtkFilePicker::cancel()
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != nullptr );

    // TODO m_pImpl->cancel();
}

// Misc

void SalGtkFilePicker::SetCurFilter( const OUString& rFilter )
{
#if !GTK_CHECK_VERSION(4, 0, 0)
    // Get all the filters already added
    GSList *filters = gtk_file_chooser_list_filters ( GTK_FILE_CHOOSER( m_pDialog ) );
    bool bFound = false;

    for( GSList *iter = filters; !bFound && iter; iter = iter->next )
    {
        GtkFileFilter* pFilter = static_cast<GtkFileFilter *>( iter->data );
        const gchar * filtername = gtk_file_filter_get_name( pFilter );
        OUString sFilterName( filtername, strlen( filtername ), RTL_TEXTENCODING_UTF8 );

        OUString aShrunkName = shrinkFilterName( rFilter );
        if( aShrunkName == sFilterName )
        {
            SAL_INFO( "vcl.gtk", "actually setting " << filtername );
            gtk_file_chooser_set_filter( GTK_FILE_CHOOSER( m_pDialog ), pFilter );
            bFound = true;
        }
    }

    g_slist_free( filters );
#else
    (void)rFilter;
#endif
}

GtkFileFilter* SalGtkFilePicker::implAddFilter( const OUString& rFilter, const OUString& rType )
{
    GtkFileFilter *filter = gtk_file_filter_new();

    OUString aShrunkName = shrinkFilterName( rFilter );
    OString aFilterName = OUStringToOString( aShrunkName, RTL_TEXTENCODING_UTF8 );
    gtk_file_filter_set_name( filter, aFilterName.getStr() );

    OUStringBuffer aTokens;

    bool bAllGlob = rType == "*.*" || rType == "*";
    if (bAllGlob)
        gtk_file_filter_add_pattern( filter, "*" );
    else
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = rType.getToken( 0, ';', nIndex );
            // Assume all have the "*.<extn>" syntax
            sal_Int32 nStarDot = aToken.lastIndexOf( "*." );
            if (nStarDot >= 0)
                aToken = aToken.copy( nStarDot + 2 );
            if (!aToken.isEmpty())
            {
                if (!aTokens.isEmpty())
                    aTokens.append(",");
                aTokens.append(aToken);

#if GTK_CHECK_VERSION(4,0,0)
                gtk_file_filter_add_suffix( filter, OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr() );
#else
                sal_Int32 nLength = aToken.getLength();

                OUStringBuffer aFilterBuffer = OUStringBuffer("*.");

                for (sal_Int32 i = 0; i < nLength; i++) {
                    sal_Unicode tokenChar = aToken[i];
                    OUString tokenCharString = OUString(tokenChar);

                    aFilterBuffer.append("[");
                    aFilterBuffer.append(tokenCharString.toAsciiLowerCase());
                    aFilterBuffer.append(tokenCharString.toAsciiUpperCase());
                    aFilterBuffer.append("]");
                }

                OUString aFilter = aFilterBuffer.toString();
                gtk_file_filter_add_pattern( filter, OUStringToOString(aFilter, RTL_TEXTENCODING_UTF8).getStr() );
#endif

                SAL_INFO( "vcl.gtk", "fustering with " << aToken );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                g_warning( "Duff filter token '%s'\n",
                    OUStringToOString(
                        rType.getToken( 0, ';', nIndex ), RTL_TEXTENCODING_UTF8 ).getStr() );
            }
#endif
        }
        while( nIndex >= 0 );
    }

    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER( m_pDialog ), filter );

    if (!bAllGlob)
    {
        GtkTreeIter iter;
        gtk_list_store_append (m_pFilterStore, &iter);
        gtk_list_store_set (m_pFilterStore, &iter,
            0, OUStringToOString(shrinkFilterName( rFilter, true ), RTL_TEXTENCODING_UTF8).getStr(),
            1, OUStringToOString(aTokens.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr(),
            2, aFilterName.getStr(),
            3, OUStringToOString(rType, RTL_TEXTENCODING_UTF8).getStr(),
            -1);
    }
    return filter;
}

void SalGtkFilePicker::implAddFilterGroup( const Sequence< StringPair >& _rFilters )
{
    // Gtk+ has no filter group concept I think so ...
    // implAddFilter( _rFilter, String() );
    for( const auto& rSubFilter : _rFilters )
        implAddFilter( rSubFilter.First, rSubFilter.Second );
}

void SalGtkFilePicker::SetFilters()
{
    if (m_aInitialFilter.isEmpty())
        m_aInitialFilter = m_aCurrentFilter;

    OUString sPseudoFilter;
    if( GTK_FILE_CHOOSER_ACTION_SAVE == gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) ) )
    {
        std::set<OUString> aAllFormats;
        if( m_pFilterVector )
        {
            for (auto & filter : *m_pFilterVector)
            {
                if( filter.hasSubFilters() )
                {   // it's a filter group
                    css::uno::Sequence< css::beans::StringPair > aSubFilters;
                    filter.getSubFilters( aSubFilters );
                    for( const auto& rSubFilter : std::as_const(aSubFilters) )
                        aAllFormats.insert(rSubFilter.Second);
                }
                else
                    aAllFormats.insert(filter.getFilter());
            }
        }
        if (aAllFormats.size() > 1)
        {
            OUStringBuffer sAllFilter;
            for (auto const& format : aAllFormats)
            {
                if (!sAllFilter.isEmpty())
                    sAllFilter.append(";");
                sAllFilter.append(format);
            }
            sPseudoFilter = getResString(FILE_PICKER_ALLFORMATS);
            m_pPseudoFilter = implAddFilter( sPseudoFilter, sAllFilter.makeStringAndClear() );
        }
    }

    if( m_pFilterVector )
    {
        for (auto & filter : *m_pFilterVector)
        {
            if( filter.hasSubFilters() )
            {   // it's a filter group

                css::uno::Sequence< css::beans::StringPair > aSubFilters;
                filter.getSubFilters( aSubFilters );

                implAddFilterGroup( aSubFilters );
            }
            else
            {
                // it's a single filter

                implAddFilter( filter.getTitle(), filter.getFilter() );
            }
        }
    }

    // set the default filter
    if (!sPseudoFilter.isEmpty())
        SetCurFilter( sPseudoFilter );
    else if(!m_aCurrentFilter.isEmpty())
        SetCurFilter( m_aCurrentFilter );

    SAL_INFO( "vcl.gtk", "end setting filters");
}

SalGtkFilePicker::~SalGtkFilePicker()
{
}

uno::Reference< ui::dialogs::XFilePicker2 >
GtkInstance::createFilePicker( const css::uno::Reference< css::uno::XComponentContext > &xMSF )
{
    return uno::Reference< ui::dialogs::XFilePicker2 >(
                new SalGtkFilePicker( xMSF ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
