/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef AIX
#define _LINUX_SOURCE_COMPAT
#include <sys/timer.h>
#undef _LINUX_SOURCE_COMPAT
#endif

#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <osl/mutex.hxx>
#include "unx/gtk/gtkinst.hxx"

#include <vcl/svapp.hxx>

#include <tools/urlobj.hxx>

#include <algorithm>
#include <set>
#include <string.h>

#include "gtk/fpicker/SalGtkFilePicker.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

static void expandexpanders(GtkContainer *pWidget)
{
    GList *pChildren = gtk_container_get_children(pWidget);
    for( GList *p = pChildren; p; p = p->next )
    {
        if (GTK_IS_CONTAINER(GTK_WIDGET(p->data)))
            expandexpanders(GTK_CONTAINER(GTK_WIDGET(p->data)));
        if (GTK_IS_EXPANDER(GTK_WIDGET(p->data)))
            gtk_expander_set_expanded(GTK_EXPANDER(GTK_WIDGET(p->data)), sal_True);
    }
    g_list_free(pChildren);
}

void SalGtkFilePicker::dialog_mapped_cb(GtkWidget *, SalGtkFilePicker *pobjFP)
{
    pobjFP->InitialMapping();
}

void SalGtkFilePicker::InitialMapping()
{
    if (!mbPreviewState )
    {
        gtk_widget_hide( m_pPreview );
        gtk_file_chooser_set_preview_widget_active( GTK_FILE_CHOOSER( m_pDialog ), false);
    }
    gtk_widget_set_size_request (m_pPreview, -1, -1);
}

SalGtkFilePicker::SalGtkFilePicker( const uno::Reference< uno::XComponentContext >& xContext ) :
    SalGtkPicker( xContext ),
    cppu::WeakComponentImplHelper9<
        XFilterManager,
        XFilterGroupManager,
        XFilePickerControlAccess,
        XFilePickerNotifier,
        XFilePreview,
        XFilePicker2,
        lang::XInitialization,
        util::XCancellable,
        lang::XEventListener>( m_rbHelperMtx ),
    m_pFilterList( NULL ),
    m_pVBox ( NULL ),
    mnHID_FolderChange( 0 ),
    mnHID_SelectionChange( 0 ),
    bVersionWidthUnset( false ),
    mbPreviewState( sal_False ),
    mHID_Preview( 0 ),
    m_pPreview( NULL ),
    m_pPseudoFilter( NULL ),
    m_PreviewImageWidth( 256 ),
    m_PreviewImageHeight( 256 )
{
    int i;

    for( i = 0; i < TOGGLE_LAST; i++ )
    {
        m_pToggles[i] = NULL;
        mbToggleVisibility[i] = false;
    }

    for( i = 0; i < BUTTON_LAST; i++ )
    {
        m_pButtons[i] = NULL;
        mbButtonVisibility[i] = false;
    }

    for( i = 0; i < LIST_LAST; i++ )
    {
        m_pHBoxs[i] = NULL;
        m_pAligns[i] = NULL;
        m_pLists[i] = NULL;
        m_pListLabels[i] = NULL;
        mbListVisibility[i] = false;
    }

    OUString aFilePickerTitle = getResString( FILE_PICKER_TITLE_OPEN );

    m_pDialog = gtk_file_chooser_dialog_new(
            OUStringToOString( aFilePickerTitle, RTL_TEXTENCODING_UTF8 ).getStr(),
            NULL,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
            (char *)NULL );

    gtk_dialog_set_default_response( GTK_DIALOG (m_pDialog), GTK_RESPONSE_ACCEPT );

    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( m_pDialog ), sal_False );
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( m_pDialog ), sal_False );

    m_pVBox = gtk_vbox_new( sal_False, 0 );

    // We don't want clickable items to have a huge hit-area
    GtkWidget *pHBox = gtk_hbox_new( sal_False, 0 );
    GtkWidget *pThinVBox = gtk_vbox_new( sal_False, 0 );

    gtk_box_pack_end (GTK_BOX( m_pVBox ), pHBox, sal_False, sal_False, 0);
    gtk_box_pack_start (GTK_BOX( pHBox ), pThinVBox, sal_False, sal_False, 0);
    gtk_widget_show( pHBox );
    gtk_widget_show( pThinVBox );

    OUString aLabel;

    for( i = 0; i < TOGGLE_LAST; i++ )
    {
        m_pToggles[i] = gtk_check_button_new();

#define LABEL_TOGGLE( elem ) \
        case elem : \
            aLabel = getResString( CHECKBOX_##elem ); \
            setLabel( CHECKBOX_##elem, aLabel ); \
            break

          switch( i ) {

        LABEL_TOGGLE( AUTOEXTENSION );
        LABEL_TOGGLE( PASSWORD );
        LABEL_TOGGLE( FILTEROPTIONS );
        LABEL_TOGGLE( READONLY );
        LABEL_TOGGLE( LINK );
        LABEL_TOGGLE( PREVIEW );
        LABEL_TOGGLE( SELECTION );
            default:
                OSL_TRACE("Handle unknown control %d", i);
                break;
        }

        gtk_box_pack_end( GTK_BOX( pThinVBox ), m_pToggles[i], sal_False, sal_False, 0 );
    }

    for( i = 0; i < LIST_LAST; i++ )
    {
        m_pHBoxs[i] = gtk_hbox_new( sal_False, 0 );

        m_pAligns[i] = gtk_alignment_new(0, 0, 0, 1);

        m_pListStores[i] = gtk_list_store_new (1, G_TYPE_STRING);
        m_pLists[i] = gtk_combo_box_new_with_model(GTK_TREE_MODEL(m_pListStores[i]));
        g_object_unref (m_pListStores[i]); // owned by the widget.
        GtkCellRenderer *pCell = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start(
                GTK_CELL_LAYOUT(m_pLists[i]), pCell, TRUE);
        gtk_cell_layout_set_attributes(
            GTK_CELL_LAYOUT (m_pLists[i]), pCell, "text", 0, NULL);

        m_pListLabels[i] = gtk_label_new( "" );

#define LABEL_LIST( elem ) \
        case elem : \
            aLabel = getResString( LISTBOX_##elem##_LABEL ); \
            setLabel( LISTBOX_##elem##_LABEL, aLabel ); \
            break

          switch( i )
        {
            LABEL_LIST( VERSION );
            LABEL_LIST( TEMPLATE );
            LABEL_LIST( IMAGE_TEMPLATE );
            default:
                OSL_TRACE("Handle unknown control %d", i);
                break;
        }

        gtk_container_add( GTK_CONTAINER( m_pAligns[i]), m_pLists[i] );
        gtk_box_pack_end( GTK_BOX( m_pHBoxs[i] ), m_pAligns[i], sal_False, sal_False, 0 );

        gtk_box_pack_end( GTK_BOX( m_pHBoxs[i] ), m_pListLabels[i], sal_False, sal_False, 0 );

        gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pHBoxs[i], sal_False, sal_False, 0 );
    }

    aLabel = getResString( FILE_PICKER_FILE_TYPE );
    m_pFilterExpander = gtk_expander_new_with_mnemonic(
        OUStringToOString( aLabel, RTL_TEXTENCODING_UTF8 ).getStr());

    gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pFilterExpander, sal_False, sal_True, 0 );

    GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
        GTK_SHADOW_IN);
    gtk_container_add (GTK_CONTAINER (m_pFilterExpander), scrolled_window);
    gtk_widget_show (scrolled_window);

    rtl::OString sExpand(getenv("SAL_EXPANDFPICKER"));
    sal_Int32 nExpand  = sExpand.toInt32();
    switch (nExpand)
    {
        default:
        case 0:
            break;
        case 1:
            gtk_expander_set_expanded(GTK_EXPANDER(m_pFilterExpander), sal_True);
            break;
        case 2:
            expandexpanders(GTK_CONTAINER(m_pDialog));
            gtk_expander_set_expanded(GTK_EXPANDER(m_pFilterExpander), sal_True);
            break;
    }

    m_pFilterStore = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING);
    m_pFilterView = gtk_tree_view_new_with_model (GTK_TREE_MODEL(m_pFilterStore));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(m_pFilterView), false);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(m_pFilterView), true);

    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    for (i = 0; i < 2; ++i)
    {
        column = gtk_tree_view_column_new ();
        cell = gtk_cell_renderer_text_new ();
        gtk_tree_view_column_set_expand (column, sal_True);
        gtk_tree_view_column_pack_start (column, cell, sal_False);
        gtk_tree_view_column_set_attributes (column, cell, "text", i, (char *)NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW(m_pFilterView), column);
    }

    gtk_container_add (GTK_CONTAINER (scrolled_window), m_pFilterView);
    gtk_widget_show (m_pFilterView);

    gtk_file_chooser_set_extra_widget( GTK_FILE_CHOOSER( m_pDialog ), m_pVBox );

    m_pPreview = gtk_image_new();
    gtk_file_chooser_set_preview_widget( GTK_FILE_CHOOSER( m_pDialog ), m_pPreview );

    g_signal_connect( G_OBJECT( m_pToggles[PREVIEW] ), "toggled",
                      G_CALLBACK( preview_toggled_cb ), this );
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW(m_pFilterView)), "changed",
                      G_CALLBACK ( type_changed_cb ), this);
    g_signal_connect( G_OBJECT( m_pDialog ), "notify::filter",
                      G_CALLBACK( filter_changed_cb ), this);
    g_signal_connect( G_OBJECT( m_pFilterExpander ), "activate",
                      G_CALLBACK( expander_changed_cb ), this);
    g_signal_connect (G_OBJECT( m_pDialog ), "map",
                      G_CALLBACK (dialog_mapped_cb), this);

    gtk_widget_show( m_pVBox );

    PangoLayout  *layout = gtk_widget_create_pango_layout (m_pFilterView, NULL);
    guint ypad;
    PangoRectangle row_height;
    pango_layout_set_markup (layout, "All Files", -1);
    pango_layout_get_pixel_extents (layout, NULL, &row_height);
    g_object_get (cell, "ypad", &ypad, (char *)NULL);
    guint height = (row_height.height + 2*ypad) * 5;
    gtk_widget_set_size_request (m_pFilterView, -1, height);
    gtk_widget_set_size_request (m_pPreview, 1, height);

    gtk_file_chooser_set_preview_widget_active( GTK_FILE_CHOOSER( m_pDialog ), true);
}

//------------------------------------------------------------------------------------
// XFilePickerNotifier
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ENSURE(!m_xListener.is(),
            "SalGtkFilePicker only talks with one listener at a time...");
    m_xListener = xListener;
}

void SAL_CALL SalGtkFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    m_xListener.clear();
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL SalGtkFilePicker::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    // no member access => no mutex needed

    uno::Reference<XFilePickerListener> xFilePickerListener( aEvent.Source, ::com::sun::star::uno::UNO_QUERY );

    if( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );
}

//-----------------------------------------------------------------------------------------
// FilePicker Event functions
//-----------------------------------------------------------------------------------------

void SalGtkFilePicker::impl_fileSelectionChanged( FilePickerEvent aEvent )
{
    OSL_TRACE( "file selection changed");
    if (m_xListener.is()) m_xListener->fileSelectionChanged( aEvent );
}

void SalGtkFilePicker::impl_directoryChanged( FilePickerEvent aEvent )
{
    OSL_TRACE("directory changed");
    if (m_xListener.is()) m_xListener->directoryChanged( aEvent );
}

void SalGtkFilePicker::impl_controlStateChanged( FilePickerEvent aEvent )
{
    OSL_TRACE("control state changed");
    if (m_xListener.is()) m_xListener->controlStateChanged( aEvent );
}

//=====================================================================

struct FilterEntry
{
protected:
    ::rtl::OUString     m_sTitle;
    ::rtl::OUString     m_sFilter;

    UnoFilterList       m_aSubFilters;

public:
    FilterEntry( const ::rtl::OUString& _rTitle, const ::rtl::OUString& _rFilter )
        :m_sTitle( _rTitle )
        ,m_sFilter( _rFilter )
    {
    }

    FilterEntry( const ::rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters );

    ::rtl::OUString     getTitle() const { return m_sTitle; }
    ::rtl::OUString     getFilter() const { return m_sFilter; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    sal_Bool        hasSubFilters( ) const;

    /** retrieves the filters belonging to the entry
    @return
        the number of sub filters
    */
    sal_Int32       getSubFilters( UnoFilterList& _rSubFilterList );

    // helpers for iterating the sub filters
    const UnoFilterEntry*   beginSubFilters() const { return m_aSubFilters.getConstArray(); }
    const UnoFilterEntry*   endSubFilters() const { return m_aSubFilters.getConstArray() + m_aSubFilters.getLength(); }
};

//=====================================================================

//---------------------------------------------------------------------
FilterEntry::FilterEntry( const ::rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters )
    :m_sTitle( _rTitle )
    ,m_aSubFilters( _rSubFilters )
{
}

//---------------------------------------------------------------------
sal_Bool FilterEntry::hasSubFilters() const
{
    return( 0 < m_aSubFilters.getLength() );
}

//---------------------------------------------------------------------
sal_Int32 FilterEntry::getSubFilters( UnoFilterList& _rSubFilterList )
{
    _rSubFilterList = m_aSubFilters;
    return m_aSubFilters.getLength();
}

static bool
isFilterString( const rtl::OUString &rFilterString, const char *pMatch )
{
        sal_Int32 nIndex = 0;
        rtl::OUString aToken;
        bool bIsFilter = true;

        rtl::OUString aMatch(rtl::OUString::createFromAscii(pMatch));

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

static rtl::OUString
shrinkFilterName( const rtl::OUString &rFilterName, bool bAllowNoStar = false )
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
                aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            else if (bAllowNoStar)
            {
                if( isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ), ".") )
                    aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            }
        }
    }

    return aRealName;
}

static void
dialog_remove_buttons( GtkDialog *pDialog )
{
    g_return_if_fail( GTK_IS_DIALOG( pDialog ) );

    GtkWidget *pActionArea;

#if GTK_CHECK_VERSION(3,0,0)
    pActionArea = gtk_dialog_get_action_area( pDialog );
#else
    pActionArea = pDialog->action_area;
#endif

    GList *pChildren =
        gtk_container_get_children( GTK_CONTAINER( pActionArea ) );

    for( GList *p = pChildren; p; p = p->next )
        gtk_widget_destroy( GTK_WIDGET( p->data ) );

    g_list_free( pChildren );
}

//------------------------------------------------------------------------------------
namespace {
    //................................................................................
    struct FilterTitleMatch : public ::std::unary_function< FilterEntry, bool >
    {
    protected:
        const ::rtl::OUString& rTitle;

    public:
        FilterTitleMatch( const ::rtl::OUString& _rTitle ) : rTitle( _rTitle ) { }

        //............................................................................
        bool operator () ( const FilterEntry& _rEntry )
        {
            sal_Bool bMatch;
            if( !_rEntry.hasSubFilters() )
                // a real filter
                bMatch = ( _rEntry.getTitle() == rTitle );
            else
                // a filter group -> search the sub filters
                bMatch =
                    _rEntry.endSubFilters() != ::std::find_if(
                        _rEntry.beginSubFilters(),
                        _rEntry.endSubFilters(),
                        *this
                    );

            return bMatch ? true : false;
        }
        bool operator () ( const UnoFilterEntry& _rEntry )
        {
            OUString aShrunkName = shrinkFilterName( _rEntry.First );
            return aShrunkName == rTitle ? true : false;
        }
    };
}


//------------------------------------------------------------------------------------
sal_Bool SalGtkFilePicker::FilterNameExists( const ::rtl::OUString& rTitle )
{
    sal_Bool bRet = sal_False;

    if( m_pFilterList )
        bRet =
            m_pFilterList->end() != ::std::find_if(
                m_pFilterList->begin(),
                m_pFilterList->end(),
                FilterTitleMatch( rTitle )
            );

    return bRet;
}

//------------------------------------------------------------------------------------
sal_Bool SalGtkFilePicker::FilterNameExists( const UnoFilterList& _rGroupedFilters )
{
    sal_Bool bRet = sal_False;

    if( m_pFilterList )
    {
        const UnoFilterEntry* pStart = _rGroupedFilters.getConstArray();
        const UnoFilterEntry* pEnd = pStart + _rGroupedFilters.getLength();
        for( ; pStart != pEnd; ++pStart )
            if( m_pFilterList->end() != ::std::find_if(
                        m_pFilterList->begin(),
                        m_pFilterList->end(),
                        FilterTitleMatch( pStart->First ) ) )
                break;

        bRet = pStart != pEnd;
    }

    return bRet;
}

//------------------------------------------------------------------------------------
void SalGtkFilePicker::ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter )
{
    if( !m_pFilterList )
    {
        m_pFilterList = new FilterList;

        // set the first filter to the current filter
        if ( m_aCurrentFilter.isEmpty() )
            m_aCurrentFilter = _rInitialCurrentFilter;
    }
}


//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::appendFilter( const rtl::OUString& aTitle, const rtl::OUString& aFilter )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    if( FilterNameExists( aTitle ) )
            throw IllegalArgumentException();

    // ensure that we have a filter list
    ensureFilterList( aTitle );

    // append the filter
    m_pFilterList->insert( m_pFilterList->end(), FilterEntry( aTitle, aFilter ) );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    OSL_TRACE( "Setting current filter to %s\n",
        OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );

    if( aTitle != m_aCurrentFilter )
    {
        m_aCurrentFilter = aTitle;
        SetCurFilter( m_aCurrentFilter );
        OSL_TRACE( "REALLY Setting current filter to %s\n",
            OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );

    }

    // TODO m_pImpl->setCurrentFilter( aTitle );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SalGtkFilePicker::updateCurrentFilterFromName(const gchar* filtername)
{
    OUString aFilterName(filtername, strlen(filtername), RTL_TEXTENCODING_UTF8);
    FilterList::iterator aEnd = m_pFilterList->end();
    for (FilterList::iterator aIter = m_pFilterList->begin(); aIter != aEnd; ++aIter)
    {
        if (aFilterName == shrinkFilterName( aIter->getTitle()))
        {
            m_aCurrentFilter = aIter->getTitle();
            break;
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

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pFilterView));
    GtkTreeIter iter;
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gchar *title;
        gtk_tree_model_get (model, &iter, 2, &title, -1);
        updateCurrentFilterFromName(title);
        g_free (title);
    }
    else if( GtkFileFilter *filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(m_pDialog)))
    {
        if (m_pPseudoFilter != filter)
            updateCurrentFilterFromName(gtk_file_filter_get_name( filter ));
        else
            updateCurrentFilterFromName(OUStringToOString( m_aInitialFilter, RTL_TEXTENCODING_UTF8 ).getStr());
    }
}

rtl::OUString SAL_CALL SalGtkFilePicker::getCurrentFilter() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    OSL_TRACE( "GetCURRENTfilter" );

    UpdateFilterfromUI();

    OSL_TRACE( "Returning current filter of %s\n",
        OUStringToOString( m_aCurrentFilter, RTL_TEXTENCODING_UTF8 ).getStr() );

    return m_aCurrentFilter;
}

//-----------------------------------------------------------------------------------------
// XFilterGroupManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::appendFilterGroup( const rtl::OUString& /*sGroupTitle*/, const uno::Sequence<beans::StringPair>& aFilters )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO m_pImpl->appendFilterGroup( sGroupTitle, aFilters );
    // check the names
    if( FilterNameExists( aFilters ) )
        // TODO: a more precise exception message
            throw IllegalArgumentException();

    // ensure that we have a filter list
    ::rtl::OUString sInitialCurrentFilter;
    if( aFilters.getLength() )
        sInitialCurrentFilter = aFilters[0].First;

    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    const StringPair* pSubFilters   = aFilters.getConstArray();
    const StringPair* pSubFiltersEnd = pSubFilters + aFilters.getLength();
    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        m_pFilterList->insert( m_pFilterList->end(), FilterEntry( pSubFilters->First, pSubFilters->Second ) );

}

//-----------------------------------------------------------------------------------------
// XFilePicker functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER(m_pDialog), bMode );
}

void SAL_CALL SalGtkFilePicker::setDefaultName( const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    OString aStr = OUStringToOString( aName, RTL_TEXTENCODING_UTF8 );
    GtkFileChooserAction eAction = gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) );

    // set_current_name launches a Gtk critical error if called for other than save
    if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
        gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( m_pDialog ), aStr.getStr() );
}

void SAL_CALL SalGtkFilePicker::setDisplayDirectory( const rtl::OUString& rDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard g;

    implsetDisplayDirectory(rDirectory);
}

rtl::OUString SAL_CALL SalGtkFilePicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    return implgetDisplayDirectory();
}

uno::Sequence<rtl::OUString> SAL_CALL SalGtkFilePicker::getFiles() throw( uno::RuntimeException )
{
    // no member access => no mutex needed

    uno::Sequence< rtl::OUString > aFiles = getSelectedFiles();
    /*
      The previous multiselection API design was completely broken
      and unimplementable for some hetrogenous pseudo-URIs eg. search://
      Thus crop unconditionally to a single selection.
    */
    aFiles.realloc (1);
    return aFiles;
}

uno::Sequence<rtl::OUString> SAL_CALL SalGtkFilePicker::getSelectedFiles() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    GSList* pPathList = gtk_file_chooser_get_uris( GTK_FILE_CHOOSER(m_pDialog) );

    int nCount = g_slist_length( pPathList );
    int nIndex = 0;
    OSL_TRACE( "GETFILES called %d files", nCount );

    // get the current action setting
    GtkFileChooserAction eAction = gtk_file_chooser_get_action(
        GTK_FILE_CHOOSER( m_pDialog ));

    uno::Sequence< rtl::OUString > aSelectedFiles(nCount);

    // Convert to OOo
    for( GSList *pElem = pPathList; pElem; pElem = pElem->next)
    {
        gchar *pURI = reinterpret_cast<gchar*>(pElem->data);
        aSelectedFiles[ nIndex ] = uritounicode(pURI);

        if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
        {
            OUString sFilterName;
            sal_Int32 nTokenIndex = 0;
            bool bExtensionTypedIn = false;

            GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pFilterView));
            GtkTreeIter iter;
            GtkTreeModel *model;
            if (gtk_tree_selection_get_selected (selection, &model, &iter))
            {
                gchar *title;
                gtk_tree_model_get (model, &iter, 2, &title, -1);
                sFilterName = OUString( title, strlen( title), RTL_TEXTENCODING_UTF8 );
                g_free (title);
            }
            else
            {
                if( aSelectedFiles[nIndex].indexOf('.') > 0 )
                {
                    rtl::OUString sExtension;
                    nTokenIndex = 0;
                    do
                        sExtension = aSelectedFiles[nIndex].getToken( 0, '.', nTokenIndex );
                    while( nTokenIndex >= 0 );

                    if( sExtension.getLength() >= 3 ) // 3 = typical/minimum extension length
                    {
                        static const OUString aStarDot(RTL_CONSTASCII_USTRINGPARAM( "*." ));

                        ::rtl::OUString aNewFilter;
                        ::rtl::OUString aOldFilter = getCurrentFilter();
                        sal_Bool bChangeFilter = sal_True;
                        for ( FilterList::iterator aListIter = m_pFilterList->begin();
                              aListIter != m_pFilterList->end();
                              ++aListIter
                        )
                        {
                            if( aListIter->getFilter().equalsIgnoreAsciiCase( aStarDot+sExtension ) )
                            {
                                if( aNewFilter.isEmpty() )
                                    aNewFilter = aListIter->getTitle();

                                if( aOldFilter == aListIter->getTitle() )
                                    bChangeFilter = sal_False;

                                bExtensionTypedIn = true;
                            }
                        }
                        if( bChangeFilter && bExtensionTypedIn )
                            setCurrentFilter( aNewFilter );
                    }
                }

                GtkFileFilter *filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(m_pDialog));
                if (m_pPseudoFilter != filter)
                {
                    const gchar* filtername = gtk_file_filter_get_name( filter );
                    sFilterName = OUString( filtername, strlen( filtername ), RTL_TEXTENCODING_UTF8 );
                }
                else
                    sFilterName = m_aInitialFilter;
            }

            OSL_TRACE( "2: current filter is %s\n",
                OUStringToOString( sFilterName, RTL_TEXTENCODING_UTF8 ).getStr() );

            FilterList::iterator aListIter = ::std::find_if(
                m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch(sFilterName) );

            OUString aFilter;
            if (aListIter != m_pFilterList->end())
                aFilter = aListIter->getFilter();

            OSL_TRACE( "turned into %s\n",
                OUStringToOString( aFilter, RTL_TEXTENCODING_UTF8 ).getStr() );

            nTokenIndex = 0;
            rtl::OUString sToken;
            //   rtl::OUString strExt;
            do
            {
                sToken = aFilter.getToken( 0, '.', nTokenIndex );

                if ( sToken.lastIndexOf( ';' ) != -1 )
                {
                    sal_Int32 nZero = 0;
                    OUString aCurrentToken = sToken.getToken( 0, ';', nZero);

                    sToken = aCurrentToken;
                    break;
                }
            }
            while( nTokenIndex >= 0 );

            if( !bExtensionTypedIn && ( sToken != "*" ) )
            {
                //if the filename does not already have the auto extension, stick it on
                OUString sExtension = OUString(RTL_CONSTASCII_USTRINGPARAM( "." )) + sToken;
                OUString &rBase = aSelectedFiles[nIndex];
                sal_Int32 nExtensionIdx = rBase.getLength() - sExtension.getLength();
                OSL_TRACE( "idx are %d %d", rBase.lastIndexOf( sExtension ), nExtensionIdx );

                if( rBase.lastIndexOf( sExtension ) != nExtensionIdx )
                    rBase += sExtension;
            }

        }

        nIndex++;
        g_free( pURI );
    }

    g_slist_free( pPathList );

    return aSelectedFiles;
}

//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::setTitle( const rtl::OUString& rTitle ) throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    implsetTitle(rTitle);
}

sal_Int16 SAL_CALL SalGtkFilePicker::execute() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_TRACE( "1: HERE WE ARE");
    OSL_ASSERT( m_pDialog != NULL );

    sal_Int16 retVal = 0;

    SetFilters();

    mnHID_FolderChange =
        g_signal_connect( GTK_FILE_CHOOSER( m_pDialog ), "current-folder-changed",
            G_CALLBACK( folder_changed_cb ), ( gpointer )this );

    mnHID_SelectionChange =
        g_signal_connect( GTK_FILE_CHOOSER( m_pDialog ), "selection-changed",
            G_CALLBACK( selection_changed_cb ), ( gpointer )this );

    int btn = GTK_RESPONSE_NO;

    uno::Reference< awt::XExtendedToolkit > xToolkit(
        awt::Toolkit::create(m_xContext),
        UNO_QUERY_THROW );

    uno::Reference< frame::XDesktop > xDesktop(
        createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
        UNO_QUERY_THROW );

    RunDialog* pRunDialog = new RunDialog(m_pDialog, xToolkit, xDesktop);
    uno::Reference < awt::XTopWindowListener > xLifeCycle(pRunDialog);
    while( GTK_RESPONSE_NO == btn )
    {
        btn = GTK_RESPONSE_YES; // we dont want to repeat unless user clicks NO for file save.

        gint nStatus = pRunDialog->run();
        switch( nStatus )
        {
            case GTK_RESPONSE_ACCEPT:
                if( GTK_FILE_CHOOSER_ACTION_SAVE == gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) ) )
                {
                    Sequence < OUString > aPathSeq = getFiles();
                    if( aPathSeq.getLength() == 1 )
                    {
                        OString sFileName = unicodetouri( aPathSeq[0] );
                        if( g_file_test( g_filename_from_uri( sFileName.getStr(), NULL, NULL ), G_FILE_TEST_IS_REGULAR ) )
                        {
                            GtkWidget *dlg;
                            INetURLObject aFileObj( sFileName );

                            OString baseName(
                              OUStringToOString(
                                aFileObj.getName(
                                  INetURLObject::LAST_SEGMENT,
                                  true,
                                  INetURLObject::DECODE_WITH_CHARSET
                                ),
                                RTL_TEXTENCODING_UTF8
                              )
                            );
                            OString aMsg(
                              OUStringToOString(
                                getResString( FILE_PICKER_OVERWRITE ),
                                RTL_TEXTENCODING_UTF8
                              )
                            );
                            OString toReplace( RTL_CONSTASCII_STRINGPARAM( "$filename$" ));

                            aMsg = aMsg.replaceAt(
                              aMsg.indexOf( toReplace ),
                              toReplace.getLength(),
                              baseName
                            );

                            dlg = gtk_message_dialog_new( NULL,
                                GTK_DIALOG_MODAL,
                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_YES_NO,
                                "%s",
                                aMsg.getStr()
                            );

                            gtk_window_set_title( GTK_WINDOW( dlg ),
                                OUStringToOString(getResString(FILE_PICKER_TITLE_SAVE ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );

                            RunDialog* pAnotherDialog = new RunDialog(dlg, xToolkit, xDesktop);
                            uno::Reference < awt::XTopWindowListener > xAnotherLifeCycle(pAnotherDialog);
                            btn = pAnotherDialog->run();

                            gtk_widget_destroy( dlg );
                        }

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
                    OSL_TRACE( "filter_changed, isn't it great %x", this);
                    impl_controlStateChanged( evt );
                    btn = GTK_RESPONSE_NO;
                }
                break;

            default:
                retVal = 0;
                break;
        }
    }

    if (mnHID_FolderChange)
        g_signal_handler_disconnect(GTK_FILE_CHOOSER( m_pDialog ), mnHID_FolderChange);
    if (mnHID_SelectionChange)
        g_signal_handler_disconnect(GTK_FILE_CHOOSER( m_pDialog ), mnHID_SelectionChange);

    return retVal;
}

//------------------------------------------------------------------------------------

// cf. offapi/com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.idl
GtkWidget *SalGtkFilePicker::getWidget( sal_Int16 nControlId, GType *pType )
{
    OSL_TRACE("control id is %d", nControlId);
    GType      tType = GTK_TYPE_TOGGLE_BUTTON; //prevent waring by initializing
    GtkWidget *pWidget = 0;

#define MAP_TOGGLE( elem ) \
        case ExtendedFilePickerElementIds::CHECKBOX_##elem: \
            pWidget = m_pToggles[elem]; tType = GTK_TYPE_TOGGLE_BUTTON; \
            break
#define MAP_BUTTON( elem ) \
        case ExtendedFilePickerElementIds::PUSHBUTTON_##elem: \
            pWidget = m_pButtons[elem]; tType = GTK_TYPE_BUTTON; \
            break
#define MAP_LIST( elem ) \
        case ExtendedFilePickerElementIds::LISTBOX_##elem: \
            pWidget = m_pLists[elem]; tType = GTK_TYPE_COMBO_BOX; \
            break
#define MAP_LIST_LABEL( elem ) \
        case ExtendedFilePickerElementIds::LISTBOX_##elem##_LABEL: \
            pWidget = m_pListLabels[elem]; tType = GTK_TYPE_LABEL; \
            break

    switch( nControlId )
    {
        MAP_TOGGLE( AUTOEXTENSION );
        MAP_TOGGLE( PASSWORD );
        MAP_TOGGLE( FILTEROPTIONS );
        MAP_TOGGLE( READONLY );
        MAP_TOGGLE( LINK );
        MAP_TOGGLE( PREVIEW );
        MAP_TOGGLE( SELECTION );
        MAP_BUTTON( PLAY );
        MAP_LIST( VERSION );
        MAP_LIST( TEMPLATE );
        MAP_LIST( IMAGE_TEMPLATE );
        MAP_LIST_LABEL( VERSION );
        MAP_LIST_LABEL( TEMPLATE );
        MAP_LIST_LABEL( IMAGE_TEMPLATE );
    default:
        OSL_TRACE("Handle unknown control %d", nControlId);
        break;
    }
#undef MAP

    if( pType )
        *pType = tType;
    return pWidget;
}



//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------
static void HackWidthToFirst(GtkComboBox *pWidget)
{
    GtkRequisition requisition;
    gtk_widget_size_request(GTK_WIDGET(pWidget), &requisition);
    gtk_widget_set_size_request(GTK_WIDGET(pWidget), requisition.width, -1);
}

static void ComboBoxAppendText(GtkComboBox *pCombo, const rtl::OUString &rStr)
{
  GtkTreeIter aIter;
  GtkListStore *pStore = GTK_LIST_STORE(gtk_combo_box_get_model(pCombo));
  rtl::OString aStr = rtl::OUStringToOString(rStr, RTL_TEXTENCODING_UTF8);
  gtk_list_store_append(pStore, &aIter);
  gtk_list_store_set(pStore, &aIter, 0, aStr.getStr(), -1);
}

void SalGtkFilePicker::HandleSetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction, const uno::Any& rValue)
{
    switch (nControlAction)
    {
        case ControlActions::ADD_ITEM:
            {
                OUString sItem;
                rValue >>= sItem;
                ComboBoxAppendText(pWidget, sItem);
                if (!bVersionWidthUnset)
                {
                    HackWidthToFirst(pWidget);
                    bVersionWidthUnset = true;
                }
            }
            break;
        case ControlActions::ADD_ITEMS:
            {
                Sequence< OUString > aStringList;
                rValue >>= aStringList;
                sal_Int32 nItemCount = aStringList.getLength();
                for (sal_Int32 i = 0; i < nItemCount; ++i)
                {
                    ComboBoxAppendText(pWidget,aStringList[i]);
                    if (!bVersionWidthUnset)
                    {
                        HackWidthToFirst(pWidget);
                        bVersionWidthUnset = true;
                    }
                }
            }
            break;
        case ControlActions::DELETE_ITEM:
            {
                sal_Int32 nPos=0;
                rValue >>= nPos;

                GtkTreeIter aIter;
                GtkListStore *pStore = GTK_LIST_STORE(
                        gtk_combo_box_get_model(GTK_COMBO_BOX(pWidget)));
                if(gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(pStore), &aIter, NULL, nPos))
                    gtk_list_store_remove(pStore, &aIter);
            }
            break;
        case ControlActions::DELETE_ITEMS:
            {
                gtk_combo_box_set_active(pWidget, -1);
                GtkListStore *pStore = GTK_LIST_STORE(
                        gtk_combo_box_get_model(GTK_COMBO_BOX(pWidget)));
                gtk_list_store_clear(pStore);
            }
            break;
        case ControlActions::SET_SELECT_ITEM:
            {
                sal_Int32 nPos=0;
                rValue >>= nPos;
                gtk_combo_box_set_active(pWidget, nPos);
            }
            break;
        default:
            OSL_TRACE("undocumented/unimplemented ControlAction for a list");
            break;
    }

    //I think its best to make it insensitive unless there is the chance to
    //actually select something from the list.
    gint nItems = gtk_tree_model_iter_n_children(
                    gtk_combo_box_get_model(pWidget), NULL);
    gtk_widget_set_sensitive(GTK_WIDGET(pWidget), nItems > 1 ? true : false);
}

uno::Any SalGtkFilePicker::HandleGetListValue(GtkComboBox *pWidget, sal_Int16 nControlAction) const
{
    uno::Any aAny;
    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
            {
                Sequence< OUString > aItemList;

                GtkTreeModel *pTree = gtk_combo_box_get_model(pWidget);
                GtkTreeIter iter;
                if (gtk_tree_model_get_iter_first(pTree, &iter))
                {
                    sal_Int32 nSize = gtk_tree_model_iter_n_children(
                        pTree, NULL);

                    aItemList.realloc(nSize);
                    for (sal_Int32 i=0; i < nSize; ++i)
                    {
                        gchar *item;
                        gtk_tree_model_get(gtk_combo_box_get_model(pWidget),
                            &iter, 0, &item, -1);
                        aItemList[i] = OUString(item, strlen(item), RTL_TEXTENCODING_UTF8);
                        g_free(item);
                        gtk_tree_model_iter_next(pTree, &iter);
                    }
                }
                aAny <<= aItemList;
            }
            break;
        case ControlActions::GET_SELECTED_ITEM:
            {
                GtkTreeIter iter;
                if (gtk_combo_box_get_active_iter(pWidget, &iter))
                {
                        gchar *item;
                        gtk_tree_model_get(gtk_combo_box_get_model(pWidget),
                            &iter, 0, &item, -1);
                        OUString sItem(item, strlen(item), RTL_TEXTENCODING_UTF8);
                        aAny <<= sItem;
                        g_free(item);
                }
            }
            break;
        case ControlActions::GET_SELECTED_ITEM_INDEX:
            {
                gint nActive = gtk_combo_box_get_active(pWidget);
                aAny <<= static_cast< sal_Int32 >(nActive);
            }
            break;
        default:
            OSL_TRACE("undocumented/unimplemented ControlAction for a list");
            break;
    }
    return aAny;
}

void SAL_CALL SalGtkFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    OSL_TRACE( "SETTING VALUE %d", nControlAction );
    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("enable unknown control %d", nControlId);
    else if( tType == GTK_TYPE_TOGGLE_BUTTON )
    {
        sal_Bool bChecked = false;
        rValue >>= bChecked;
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pWidget ), bChecked );
    }
    else if( tType == GTK_TYPE_COMBO_BOX )
        HandleSetListValue(GTK_COMBO_BOX(pWidget), nControlAction, rValue);
    else
    {
        OSL_TRACE("Can't set value on button / list %d %d\n",
            nControlId, nControlAction);
    }
}

uno::Any SAL_CALL SalGtkFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    uno::Any aRetval;

    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("enable unknown control %d", nControlId);
    else if( tType == GTK_TYPE_TOGGLE_BUTTON )
        aRetval <<= (sal_Bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pWidget ) );
    else if( tType == GTK_TYPE_COMBO_BOX )
        aRetval = HandleGetListValue(GTK_COMBO_BOX(pWidget), nControlAction);
    else
        OSL_TRACE("Can't get value on button / list %d %d\n",
            nControlId, nControlAction );

    return aRetval;
}

void SAL_CALL SalGtkFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    GtkWidget *pWidget;

    if ( nControlId == ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR )
        gtk_expander_set_expanded( GTK_EXPANDER( m_pFilterExpander ), bEnable );
    else if( ( pWidget = getWidget( nControlId ) ) )
    {
        if( bEnable )
        {
            OSL_TRACE( "enable" );
            gtk_widget_set_sensitive( pWidget, sal_True );
        }
        else
        {
            OSL_TRACE( "disable" );
            gtk_widget_set_sensitive( pWidget, sal_False );
        }
    }
    else
        OSL_TRACE("enable unknown control %d", nControlId );
}

void SAL_CALL SalGtkFilePicker::setLabel( sal_Int16 nControlId, const ::rtl::OUString& rLabel )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
    {
          OSL_TRACE("Set label on unknown control %d", nControlId);
        return;
    }

    OString aTxt = OUStringToOString( rLabel.replace('~', '_'), RTL_TEXTENCODING_UTF8 );
    if (nControlId == ExtendedFilePickerElementIds::PUSHBUTTON_PLAY)
    {
#ifdef GTK_STOCK_MEDIA_PLAY
        if (msPlayLabel.isEmpty())
            msPlayLabel = rLabel;
        if (msPlayLabel == rLabel)
            gtk_button_set_label(GTK_BUTTON(pWidget), GTK_STOCK_MEDIA_PLAY);
        else
            gtk_button_set_label(GTK_BUTTON(pWidget), GTK_STOCK_MEDIA_STOP);
#else
        gtk_button_set_label(GTK_BUTTON(pWidget), aTxt.getStr());
#endif
    }
    else if( tType == GTK_TYPE_TOGGLE_BUTTON || tType == GTK_TYPE_BUTTON || tType == GTK_TYPE_LABEL )
        g_object_set( pWidget, "label", aTxt.getStr(),
                      "use_underline", sal_True, (char *)NULL );
    else
        OSL_TRACE("Can't set label on list");
}

rtl::OUString SAL_CALL SalGtkFilePicker::getLabel( sal_Int16 nControlId )
    throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    GType tType;
    OString aTxt;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("Get label on unknown control %d", nControlId);
    else if( tType == GTK_TYPE_TOGGLE_BUTTON || tType == GTK_TYPE_BUTTON || tType == GTK_TYPE_LABEL )
        aTxt = gtk_button_get_label( GTK_BUTTON( pWidget ) );
    else
        OSL_TRACE("Can't get label on list");

    return OStringToOUString( aTxt, RTL_TEXTENCODING_UTF8 );
}

//------------------------------------------------------------------------------------
// XFilePreview functions
//------------------------------------------------------------------------------------

uno::Sequence<sal_Int16> SAL_CALL SalGtkFilePicker::getSupportedImageFormats() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO return m_pImpl->getSupportedImageFormats();
    return uno::Sequence<sal_Int16>();
}

sal_Int32 SAL_CALL SalGtkFilePicker::getTargetColorDepth() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO return m_pImpl->getTargetColorDepth();
    return 0;
}

sal_Int32 SAL_CALL SalGtkFilePicker::getAvailableWidth() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    return m_PreviewImageWidth;
}

sal_Int32 SAL_CALL SalGtkFilePicker::getAvailableHeight() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    return m_PreviewImageHeight;
}

void SAL_CALL SalGtkFilePicker::setImage( sal_Int16 /*aImageFormat*/, const uno::Any& /*aImage*/ )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO m_pImpl->setImage( aImageFormat, aImage );
}

void SalGtkFilePicker::implChangeType( GtkTreeSelection *selection )
{
    OUString aLabel = getResString( FILE_PICKER_FILE_TYPE );

    GtkTreeIter iter;
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        gchar *title;
        gtk_tree_model_get (model, &iter, 2, &title, -1);
        aLabel += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ": " ));
        aLabel += rtl::OUString( title, strlen(title), RTL_TEXTENCODING_UTF8 );
        g_free (title);
    }
    gtk_expander_set_label (GTK_EXPANDER (m_pFilterExpander),
        OUStringToOString( aLabel, RTL_TEXTENCODING_UTF8 ).getStr());
    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    impl_controlStateChanged( evt );
}

void SalGtkFilePicker::type_changed_cb( GtkTreeSelection *selection, SalGtkFilePicker *pobjFP )
{
    pobjFP->implChangeType(selection);
}

void SalGtkFilePicker::unselect_type()
{
    gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(m_pFilterView)));
}

void SalGtkFilePicker::expander_changed_cb( GtkExpander *expander, SalGtkFilePicker *pobjFP )
{
    if (gtk_expander_get_expanded(expander))
        pobjFP->unselect_type();
}

void SalGtkFilePicker::filter_changed_cb( GtkFileChooser *, GParamSpec *,
    SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    OSL_TRACE( "filter_changed, isn't it great %x", pobjFP );
    pobjFP->impl_controlStateChanged( evt );
}

void SalGtkFilePicker::folder_changed_cb( GtkFileChooser *, SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    OSL_TRACE( "folder_changed, isn't it great %x", pobjFP );
    pobjFP->impl_directoryChanged( evt );
}

void SalGtkFilePicker::selection_changed_cb( GtkFileChooser *, SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    OSL_TRACE( "selection_changed, isn't it great %x", pobjFP );
    pobjFP->impl_fileSelectionChanged( evt );
}

void SalGtkFilePicker::update_preview_cb( GtkFileChooser *file_chooser, SalGtkFilePicker* pobjFP )
{
    GtkWidget *preview;
    char *filename;
    GdkPixbuf *pixbuf;
    gboolean have_preview = sal_False;

    preview = pobjFP->m_pPreview;
    filename = gtk_file_chooser_get_preview_filename( file_chooser );

    if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pobjFP->m_pToggles[PREVIEW] ) ) && g_file_test( filename, G_FILE_TEST_IS_REGULAR ) )
    {
        pixbuf = gdk_pixbuf_new_from_file_at_size(
                filename,
                pobjFP->m_PreviewImageWidth,
                pobjFP->m_PreviewImageHeight, NULL );

        have_preview = ( pixbuf != NULL );

        gtk_image_set_from_pixbuf( GTK_IMAGE( preview ), pixbuf );
        if( pixbuf )
            g_object_unref( G_OBJECT( pixbuf ) );

    }

    gtk_file_chooser_set_preview_widget_active( file_chooser, have_preview );

    if( filename )
        g_free( filename );
}

sal_Bool SAL_CALL SalGtkFilePicker::setShowState( sal_Bool bShowState ) throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO return m_pImpl->setShowState( bShowState );
    if( bShowState != mbPreviewState )
    {
        if( bShowState )
        {
            // Show
            if( !mHID_Preview )
            {
                mHID_Preview = g_signal_connect(
                    GTK_FILE_CHOOSER( m_pDialog ), "update-preview",
                    G_CALLBACK( update_preview_cb ), ( gpointer )this );
            }
            gtk_widget_show( m_pPreview );
        }
        else
        {
            // Hide
            gtk_widget_hide( m_pPreview );
        }

        // also emit the signal
        g_signal_emit_by_name( G_OBJECT( m_pDialog ), "update-preview" );

        mbPreviewState = bShowState;
    }
    return true;
}

sal_Bool SAL_CALL SalGtkFilePicker::getShowState() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    return mbPreviewState;
}

//------------------------------------------------------------------------------------
// XInitialization
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::initialize( const uno::Sequence<uno::Any>& aArguments )
    throw( uno::Exception, uno::RuntimeException )
{
    // parameter checking
    uno::Any aAny;
    if( 0 == aArguments.getLength() )
        throw lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "no arguments" )),
            static_cast<XFilePicker2*>( this ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != ::getCppuType( ( sal_Int16* )0 ) ) &&
         (aAny.getValueType() != ::getCppuType( ( sal_Int8* )0 ) ) )
         throw lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "invalid argument type" )),
            static_cast<XFilePicker2*>( this ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    GtkFileChooserAction eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
    const gchar *first_button_text = GTK_STOCK_OPEN;

    SolarMutexGuard g;

    //   TODO: extract full semantic from
    //   svtools/source/filepicker/filepicker.cxx (getWinBits)
    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            OSL_TRACE( "3all true" );
            break;
        case FILESAVE_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            OSL_TRACE( "2all true" );
                break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[PASSWORD] = true;
            OSL_TRACE( "1all true" );
            // TODO
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[PASSWORD] = true;
            mbToggleVisibility[FILTEROPTIONS] = true;
            OSL_TRACE( "4all true" );
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE; // SELECT_FOLDER ?
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[SELECTION] = true;
            OSL_TRACE( "5all true" );
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbListVisibility[TEMPLATE] = true;
            OSL_TRACE( "6all true" );
            // TODO
                break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[LINK] = true;
            mbToggleVisibility[PREVIEW] = true;
            mbListVisibility[IMAGE_TEMPLATE] = true;
            // TODO
                break;
        case FILEOPEN_PLAY:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbButtonVisibility[PLAY] = true;
            // TODO
                break;
        case FILEOPEN_READONLY_VERSION:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[READONLY] = true;
            mbListVisibility[VERSION] = true;
            break;
        case FILEOPEN_LINK_PREVIEW:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[LINK] = true;
            mbToggleVisibility[PREVIEW] = true;
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            OSL_TRACE( "7all true" );
            // TODO
                break;
        default:
                throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Unknown template" )),
                static_cast< XFilePicker2* >( this ),
                1 );
    }

    if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
    {
        OUString aFilePickerTitle(getResString( FILE_PICKER_TITLE_SAVE ));
        gtk_window_set_title ( GTK_WINDOW( m_pDialog ),
            OUStringToOString( aFilePickerTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    gtk_file_chooser_set_action( GTK_FILE_CHOOSER( m_pDialog ), eAction);
    dialog_remove_buttons( GTK_DIALOG( m_pDialog ) );
    gtk_dialog_add_button( GTK_DIALOG( m_pDialog ), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL );
    for( int nTVIndex = 0; nTVIndex < BUTTON_LAST; nTVIndex++ )
    {
        if( mbButtonVisibility[nTVIndex] )
        {
#ifdef GTK_STOCK_MEDIA_PLAY
            m_pButtons[ nTVIndex ] = gtk_dialog_add_button( GTK_DIALOG( m_pDialog ), GTK_STOCK_MEDIA_PLAY, 1 );
#else
            OString aPlay = OUStringToOString( getResString( PUSHBUTTON_PLAY ), RTL_TEXTENCODING_UTF8 );
            m_pButtons[ nTVIndex ] = gtk_dialog_add_button( GTK_DIALOG( m_pDialog ), aPlay.getStr(), 1 );
#endif
        }
    }
    gtk_dialog_add_button( GTK_DIALOG( m_pDialog ), first_button_text, GTK_RESPONSE_ACCEPT );

    gtk_dialog_set_default_response( GTK_DIALOG (m_pDialog), GTK_RESPONSE_ACCEPT );

    // Setup special flags
    for( int nTVIndex = 0; nTVIndex < TOGGLE_LAST; nTVIndex++ )
    {
        if( mbToggleVisibility[nTVIndex] )
            gtk_widget_show( m_pToggles[ nTVIndex ] );
    }

    for( int nTVIndex = 0; nTVIndex < LIST_LAST; nTVIndex++ )
    {
        if( mbListVisibility[nTVIndex] )
        {
            gtk_widget_set_sensitive( m_pLists[ nTVIndex ], false );
            gtk_widget_show( m_pLists[ nTVIndex ] );
            gtk_widget_show( m_pListLabels[ nTVIndex ] );
            gtk_widget_show( m_pAligns[ nTVIndex ] );
            gtk_widget_show( m_pHBoxs[ nTVIndex ] );
        }
    }
}

void SalGtkFilePicker::preview_toggled_cb( GObject *cb, SalGtkFilePicker* pobjFP )
{
    if( pobjFP->mbToggleVisibility[PREVIEW] )
        pobjFP->setShowState( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( cb ) ) );
}

//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::cancel() throw( uno::RuntimeException )
{
    SolarMutexGuard g;

    OSL_ASSERT( m_pDialog != NULL );

    // TODO m_pImpl->cancel();
}

//--------------------------------------------------
// Misc
//-------------------------------------------------
void SalGtkFilePicker::SetCurFilter( const OUString& rFilter )
{
    // Get all the filters already added
    GSList *filters = gtk_file_chooser_list_filters ( GTK_FILE_CHOOSER( m_pDialog ) );
    bool bFound = false;

    for( GSList *iter = filters; !bFound && iter; iter = iter->next )
    {
        GtkFileFilter* pFilter = reinterpret_cast<GtkFileFilter *>( iter->data );
        G_CONST_RETURN gchar * filtername = gtk_file_filter_get_name( pFilter );
        OUString sFilterName( filtername, strlen( filtername ), RTL_TEXTENCODING_UTF8 );

        OUString aShrunkName = shrinkFilterName( rFilter );
        if( aShrunkName.equals( sFilterName) )
        {
            OSL_TRACE( "actually setting %s", filtername );
            gtk_file_chooser_set_filter( GTK_FILE_CHOOSER( m_pDialog ), pFilter );
            bFound = true;
        }
    }

    g_slist_free( filters );
}

extern "C"
{
static gboolean
case_insensitive_filter (const GtkFileFilterInfo *filter_info, gpointer data)
{
    gboolean bRetval = sal_False;
    const char *pFilter = (const char *) data;

    g_return_val_if_fail( data != NULL, sal_False );
    g_return_val_if_fail( filter_info != NULL, sal_False );

    if( !filter_info->uri )
        return sal_False;

    const char *pExtn = strrchr( filter_info->uri, '.' );
    if( !pExtn )
        return sal_False;
    pExtn++;

    if( !g_ascii_strcasecmp( pFilter, pExtn ) )
        bRetval = sal_True;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "'%s' match extn '%s' vs '%s' yeilds %d\n",
        filter_info->uri, pExtn, pFilter, bRetval );
#endif

    return bRetval;
}
}

GtkFileFilter* SalGtkFilePicker::implAddFilter( const OUString& rFilter, const OUString& rType )
{
    GtkFileFilter *filter = gtk_file_filter_new();

    OUString aShrunkName = shrinkFilterName( rFilter );
    OString aFilterName = rtl::OUStringToOString( aShrunkName, RTL_TEXTENCODING_UTF8 );
    gtk_file_filter_set_name( filter, aFilterName.getStr() );

    static const OUString aStarDot(RTL_CONSTASCII_USTRINGPARAM( "*." ));
    OUString aTokens;

    bool bAllGlob = !rType.compareToAscii( "*.*" ) || !rType.compareToAscii( "*" );
    if (bAllGlob)
        gtk_file_filter_add_pattern( filter, "*" );
    else
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aToken;
        do
        {
            aToken = rType.getToken( 0, ';', nIndex );
            // Assume all have the "*.<extn>" syntax
            aToken = aToken.copy( aToken.lastIndexOf( aStarDot ) + 2 );
            if (!aToken.isEmpty())
            {
                if (!aTokens.isEmpty())
                    aTokens += OUString(RTL_CONSTASCII_USTRINGPARAM(","));
                aTokens = aTokens += aToken;
                gtk_file_filter_add_custom (filter, GTK_FILE_FILTER_URI,
                    case_insensitive_filter,
                    g_strdup( rtl::OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr() ),
                    (GDestroyNotify) g_free );

                OSL_TRACE( "fustering with %s", rtl::OUStringToOString( aToken, RTL_TEXTENCODING_UTF8 ).getStr());
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                g_warning( "Duff filter token '%s'\n",
                    rtl::OUStringToOString(
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
            1, OUStringToOString(aTokens, RTL_TEXTENCODING_UTF8).getStr(),
            2, aFilterName.getStr(),
            3, OUStringToOString(rType, RTL_TEXTENCODING_UTF8).getStr(),
            -1);
    }
    return filter;
}

void SalGtkFilePicker::implAddFilterGroup( const OUString& /*_rFilter*/, const Sequence< StringPair >& _rFilters )
{
    // Gtk+ has no filter group concept I think so ...
    // implAddFilter( _rFilter, String() );
    const StringPair* pSubFilters   = _rFilters.getConstArray();
    const StringPair* pSubFiltersEnd = pSubFilters + _rFilters.getLength();
    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        implAddFilter( pSubFilters->First, pSubFilters->Second );
}

void SalGtkFilePicker::SetFilters()
{
    if (m_aInitialFilter.isEmpty())
        m_aInitialFilter = m_aCurrentFilter;

    rtl::OUString sPseudoFilter;
    if( GTK_FILE_CHOOSER_ACTION_SAVE == gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) ) )
    {
        std::set<OUString> aAllFormats;
        if( m_pFilterList && !m_pFilterList->empty() )
        {
            for (   FilterList::iterator aListIter = m_pFilterList->begin();
                    aListIter != m_pFilterList->end();
                    ++aListIter
                )
            {
                if( aListIter->hasSubFilters() )
                {   // it's a filter group
                    UnoFilterList aSubFilters;
                    aListIter->getSubFilters( aSubFilters );
                    const StringPair* pSubFilters   = aSubFilters.getConstArray();
                    const StringPair* pSubFiltersEnd = pSubFilters + aSubFilters.getLength();
                    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
                        aAllFormats.insert(pSubFilters->Second);
                }
                else
                    aAllFormats.insert(aListIter->getFilter());
            }
        }
        if (aAllFormats.size() > 1)
        {
            rtl::OUString sAllFilter;
            std::set<OUString>::const_iterator aEnd = aAllFormats.end();
            for (std::set<OUString>::const_iterator aIter = aAllFormats.begin(); aIter != aEnd; ++aIter)
            {
                if (!sAllFilter.isEmpty())
                    sAllFilter += OUString(sal_Unicode(';'));
                sAllFilter += *aIter;
            }
            sPseudoFilter = getResString(FILE_PICKER_ALLFORMATS);
            m_pPseudoFilter = implAddFilter( sPseudoFilter, sAllFilter );
        }
    }

    if( m_pFilterList && !m_pFilterList->empty() )
    {
        for (   FilterList::iterator aListIter = m_pFilterList->begin();
                aListIter != m_pFilterList->end();
                ++aListIter
            )
        {
            if( aListIter->hasSubFilters() )
            {   // it's a filter group

                UnoFilterList aSubFilters;
                aListIter->getSubFilters( aSubFilters );

                implAddFilterGroup( aListIter->getTitle(), aSubFilters );
            }
            else
            {
                // it's a single filter

                implAddFilter( aListIter->getTitle(), aListIter->getFilter() );
            }
        }
    }

    if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(m_pFilterStore), NULL)) //If m_pFilterStore is not empty
        gtk_widget_show( m_pFilterExpander );
    else
        gtk_widget_hide( m_pFilterExpander );

    // set the default filter
    if (!sPseudoFilter.isEmpty())
        SetCurFilter( sPseudoFilter );
    else if(!m_aCurrentFilter.isEmpty())
        SetCurFilter( m_aCurrentFilter );

    OSL_TRACE( "end setting filters");
}

SalGtkFilePicker::~SalGtkFilePicker()
{
    SolarMutexGuard g;

    int i;

    for( i = 0; i < TOGGLE_LAST; i++ )
        gtk_widget_destroy( m_pToggles[i] );

    for( i = 0; i < LIST_LAST; i++ )
    {
        gtk_widget_destroy( m_pListLabels[i] );
        gtk_widget_destroy( m_pAligns[i] ); //m_pAligns[i] owns m_pLists[i]
        gtk_widget_destroy( m_pHBoxs[i] );
    }

    delete m_pFilterList;

    gtk_widget_destroy( m_pVBox );
}

using namespace ::com::sun::star;

uno::Reference< ui::dialogs::XFilePicker2 >
GtkInstance::createFilePicker( const com::sun::star::uno::Reference<
                                        com::sun::star::uno::XComponentContext > &xMSF )
{
    return uno::Reference< ui::dialogs::XFilePicker2 >(
                new SalGtkFilePicker( xMSF ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
