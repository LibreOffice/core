/*************************************************************************
 *
 *  $RCSfile: SalGtkFilePicker.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-12-16 11:13:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Anil Bhatia
 *
 *
 ************************************************************************/

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _FPSERVICEINFO_HXX_
#include <FPServiceInfo.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SALGTKFILEPICKER_HXX_
#include <SalGtkFilePicker.hxx>
#endif

#include "filepickereventnotification.hxx"

#include <tools/urlobj.hxx>

#include <iostream>
#include "resourceprovider.hxx"
#ifndef _SV_RC_H
#include <tools/rc.hxx>
#endif

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

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    // controling event notifications
    const bool STARTUP_SUSPENDED = true;
    const bool STARTUP_ALIVE     = false;

    uno::Sequence<rtl::OUString> SAL_CALL FilePicker_getSupportedServiceNames()
    {
        uno::Sequence<rtl::OUString> aRet(3);
            aRet[0] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" );
        aRet[1] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFilePicker" );
        aRet[2] = rtl::OUString::createFromAscii( "com.sun.star.ui.dialogs.GtkFilePicker" );
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
// constructor
//-----------------------------------------------------------------------------------------

SalGtkFilePicker::SalGtkFilePicker( const uno::Reference<lang::XMultiServiceFactory>& xServiceMgr ) :
    cppu::WeakComponentImplHelper9<
        XFilterManager,
            XFilterGroupManager,
            XFilePickerControlAccess,
        XFilePickerNotifier,
            XFilePreview,
        lang::XInitialization,
        util::XCancellable,
        lang::XEventListener,
        lang::XServiceInfo>( m_rbHelperMtx ),
    m_xServiceMgr( xServiceMgr ),
    m_aAsyncEventNotifier( rBHelper ),
    m_pVBox ( NULL ),
    m_pFilterList( NULL ),
    mHID_Preview( ( gulong ) NULL ),
    m_pPreview( NULL ),
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
        m_pButtons[i] = NULL;

    for( i = 0; i < LIST_LAST; i++ )
        m_pLists[i] = NULL;

    for( i = 0; i < LIST_LABEL_LAST; i++ )
        m_pListLabels[i] = NULL;
}

//------------------------------------------------------------------------------------
// XFilePickerNotifier
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::addFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    if( rBHelper.bDisposed )
        throw lang::DisposedException( rtl::OUString::createFromAscii( "object is already disposed" ),
                        static_cast< XFilePicker* >( this ) );

    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
        rBHelper.aLC.addInterface( getCppuType( &xListener ), xListener );
}

void SAL_CALL SalGtkFilePicker::removeFilePickerListener( const uno::Reference<XFilePickerListener>& xListener )
    throw( uno::RuntimeException )
{
    if( rBHelper.bDisposed )
        throw lang::DisposedException( rtl::OUString::createFromAscii( "object is already disposed" ),
                           static_cast< XFilePicker* >( this ) );

    rBHelper.aLC.removeInterface( getCppuType( &xListener ), xListener );
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL SalGtkFilePicker::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    uno::Reference<XFilePickerListener> xFilePickerListener( aEvent.Source, ::com::sun::star::uno::UNO_QUERY );

    if( xFilePickerListener.is() )
        removeFilePickerListener( xFilePickerListener );
}

//-----------------------------------------------------------------------------------------
// FilePicker Event functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
{
    /* TODO
    aEvent.Source = uno::Reference<uno::XInterface>( static_cast<XFilePickerNotifier*>( this ) );
    m_aAsyncEventNotifier.notifyEvent(
        new SalGtkFilePickerParamEventNotification( &XFilePickerListener::fileSelectionChanged, aEvent ) );
    */
}

void SAL_CALL SalGtkFilePicker::directoryChanged( FilePickerEvent aEvent )
{
#if 0
    aEvent.Source = uno::Reference<uno::XInterface>( static_cast<XFilePickerNotifier*>( this ) );
    m_aAsyncEventNotifier.notifyEvent(
        new SalGtkFilePickerParamEventNotification( &XFilePickerListener::directoryChanged, aEvent ) );
#endif
}

void SAL_CALL SalGtkFilePicker::controlStateChanged( FilePickerEvent aEvent )
{
    aEvent.Source = uno::Reference<uno::XInterface>( static_cast<XFilePickerNotifier*>( this ) );
    m_aAsyncEventNotifier.notifyEvent(
        new SalGtkFilePickerParamEventNotification( &XFilePickerListener::controlStateChanged, aEvent ) );
}

void SAL_CALL SalGtkFilePicker::dialogSizeChanged()
{
    /*
    m_aAsyncEventNotifier.notifyEvent(
        new SalGtkFilePickerEventNotification( &XFilePickerListener::dialogSizeChanged ) );
    */
}

//-----------------------------------------------------------------------------------------
// If there are more then one listener the return value of the last one wins
//-----------------------------------------------------------------------------------------

rtl::OUString SAL_CALL SalGtkFilePicker::helpRequested( FilePickerEvent aEvent ) const
{
    rtl::OUString aHelpText;

    ::cppu::OInterfaceContainerHelper* pICHelper =
        rBHelper.getContainer( getCppuType( ( uno::Reference<XFilePickerListener> * )0 ) );

    if( pICHelper )
    {
        ::cppu::OInterfaceIteratorHelper iter( *pICHelper );

        while( iter.hasMoreElements() )
        {
            try
            {
                /*
                      if there are multiple listeners responding
                          to this notification the next response
                  overwrittes  the one before if it is not empty
                        */

                rtl::OUString aTempString;

                uno::Reference<XFilePickerListener> xFPListener( iter.next(), uno::UNO_QUERY );
                if( xFPListener.is() )
                        {
                    aTempString = xFPListener->helpRequested( aEvent );
                    if( aTempString.getLength() )
                        aHelpText = aTempString;
                        }

            }
            catch( uno::RuntimeException& )
            {
                OSL_ENSURE( false, "RuntimeException during event dispatching" );
            }
        }
    }

    return aHelpText;
}

bool SalGtkFilePicker::startupEventNotification( bool bStartupSuspended )
{
    OSL_TRACE( "attempting to start\n");
    return m_aAsyncEventNotifier.startup( bStartupSuspended );
}

void SalGtkFilePicker::shutdownEventNotification()
{
    OSL_TRACE( "before terminate, risky for some reason\n" );
    m_aAsyncEventNotifier.shutdown();
    OSL_TRACE( "after terminate, risky for some reason\n" );
}

void SalGtkFilePicker::suspendEventNotification()
{
    OSL_TRACE( "suspend called\n" );
    m_aAsyncEventNotifier.suspend();
}

void SalGtkFilePicker::resumeEventNotification()
{
    OSL_TRACE( "resume called\n" );
    m_aAsyncEventNotifier.resume();
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

static inline bool
isFilterString( const rtl::OUString &rFilterString )
{
        sal_Int32 nIndex = 0;
        rtl::OUString aToken;
        bool bIsFilter = true;

        do
        {
            aToken = rFilterString.getToken( 0, ';', nIndex );
            if( !aToken.match( rtl::OUString::createFromAscii( "*." ) ) )
            {
                bIsFilter = false;
                break;
            }
        }
        while( nIndex >= 0 );

        return bIsFilter;
}

static rtl::OUString
shrinkFilterName( const rtl::OUString &rFilterName )
{
    int i;
    int nBracketLen = -1;
    int nBracketEnd = -1;
    const sal_Unicode *pStr = rFilterName;
    OUString aRealName = rFilterName;

    for( i = aRealName.getLength() - 1; i > 0; i-- )
    {
        if( pStr[i] == ')' )
            nBracketEnd = i;

        else if( pStr[i] == '(' )
        {
            nBracketLen = nBracketEnd - i;
            if( nBracketEnd > 0 &&
                isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ) ) )
                    aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
        }
    }

//  OSL_TRACE( "BEFORE %s\n",
//      OUStringToOString( rFilterName, RTL_TEXTENCODING_UTF8 ).getStr());
//  OSL_TRACE( "AFTER %s\n",
//      OUStringToOString( aRealName, RTL_TEXTENCODING_UTF8 ).getStr());

    return aRealName;
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
        if( ( !m_aCurrentFilter ) || ( !m_aCurrentFilter.getLength() ) )
            m_aCurrentFilter = _rInitialCurrentFilter;
    }
}


//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::appendFilter( const rtl::OUString& aTitle, const rtl::OUString& aFilter )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( FilterNameExists( aTitle ) )
            throw IllegalArgumentException();

    // ensure that we have a filter list
    ensureFilterList( aTitle );

    // append the filter
    m_pFilterList->insert( m_pFilterList->end(), FilterEntry( aTitle, aFilter ) );
    // implAddFilter( aTitle, aFilter );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::setCurrentFilter( const rtl::OUString& aTitle )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

rtl::OUString SAL_CALL SalGtkFilePicker::getCurrentFilter() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    // TODO return m_pImpl->getCurrentFilter();

    OSL_TRACE( "GetCURRENTfilter\n" );

    // Update the filtername from the users selection if they have had a chance to do so.
    if( GtkFileFilter *filter = gtk_file_chooser_get_filter( GTK_FILE_CHOOSER( m_pDialog ) ) )
    {
        const gchar* filtername = gtk_file_filter_get_name( filter );
        m_aCurrentFilter = rtl::OUString( filtername, strlen( filtername ),
                            RTL_TEXTENCODING_UTF8 );
    }

    OSL_TRACE( "Returning current filter of %s\n",
        OUStringToOString( m_aCurrentFilter, RTL_TEXTENCODING_UTF8 ).getStr() );

    return m_aCurrentFilter;
}

//-----------------------------------------------------------------------------------------
// XFilterGroupManager functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::appendFilterGroup( const rtl::OUString& sGroupTitle, const uno::Sequence<beans::StringPair>& aFilters )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER(m_pDialog), bMode );
}

void SAL_CALL SalGtkFilePicker::setDefaultName( const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OString aStr = OUStringToOString( aName, RTL_TEXTENCODING_UTF8 );
    gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER( m_pDialog ),
                       aStr.getStr() );
}

void SAL_CALL SalGtkFilePicker::setDisplayDirectory( const rtl::OUString& rDirectory )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    implsetDisplayDirectory(rDirectory);
}

rtl::OUString SAL_CALL SalGtkFilePicker::getDisplayDirectory() throw( uno::RuntimeException )
{
    return implgetDisplayDirectory();
}

uno::Sequence<rtl::OUString> SAL_CALL SalGtkFilePicker::getFiles() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    GSList* pPathList = gtk_file_chooser_get_uris( GTK_FILE_CHOOSER(m_pDialog) );

    int nCount = g_slist_length( pPathList );
    uno::Sequence< rtl::OUString > aSelectedFiles( nCount );

    OSL_TRACE( "GETFILES called %d files\n", nCount );

    // get the current action setting
    GtkFileChooserAction eAction = gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) );

    // Convert to OOo
    for( int nIndex = 0; ((nIndex < nCount) && pPathList); ++nIndex, pPathList = g_slist_next( pPathList ) )
    {
        const gchar *path = reinterpret_cast<gchar*>( pPathList->data );
        aSelectedFiles[ nIndex ] =
            OUString( reinterpret_cast<const sal_Char*>( path ),
                strlen( path ), RTL_TEXTENCODING_UTF8 );

        if( GTK_FILE_CHOOSER_ACTION_SAVE == eAction )
        {
            const gchar* filtername =
                gtk_file_filter_get_name( gtk_file_chooser_get_filter( GTK_FILE_CHOOSER( m_pDialog ) ) );

            OSL_TRACE( "2: current filter is %s\n", filtername );


            FilterList::iterator aListIter = ::std::find_if(
                               m_pFilterList->begin(), m_pFilterList->end(),
                        FilterTitleMatch( OUString( filtername,
                            strlen( filtername ), RTL_TEXTENCODING_UTF8 ) ) );

            OUString aFilter = aListIter->getFilter();

            OSL_TRACE( "turned into %s\n",
                OUStringToOString( aFilter, RTL_TEXTENCODING_UTF8 ).getStr() );


            sal_Int32 nTokenIndex = 0;
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

            // if AutoExtension is enabled and checked and current filter is not *,
            // then complete the file name by concatinating the filter
            if( mbToggleVisibility[AUTOEXTENSION]
                 && ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( m_pToggles[AUTOEXTENSION] ) ) )
                 && ( !sToken.equalsAscii( "*" ) ) )
            {
                //if the filename does not already have the auto extension, stick it on
                OUString sExtension = OUString::createFromAscii( "." ) + sToken;
                OUString &rBase = aSelectedFiles[nIndex];
                sal_Int32 nExtensionIdx = rBase.getLength() - sExtension.getLength();
                OSL_TRACE( "idx are %d %d\n", rBase.lastIndexOf( sExtension ), nExtensionIdx );

                if( rBase.lastIndexOf( sExtension ) != nExtensionIdx )
                    rBase += sExtension;
            }

        }

        g_free( ( char* )( pPathList->data ) );
    }

    g_slist_free( pPathList );

    return aSelectedFiles;
}

//-----------------------------------------------------------------------------------------
// XExecutableDialog functions
//-----------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::setTitle( const rtl::OUString& rTitle ) throw( uno::RuntimeException )
{
    implsetTitle(rTitle);
}

sal_Int16 SAL_CALL SalGtkFilePicker::execute() throw( uno::RuntimeException )
{
    OSL_TRACE( "1: HERE WE ARE\n");
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_Int16 retVal = 0;

    if( !startupEventNotification( STARTUP_SUSPENDED ) )
    {
        OSL_ENSURE( sal_False, "Could not start event notifier thread!" );

        throw uno::RuntimeException(
            rtl::OUString::createFromAscii( "Error executing dialog" ),
            static_cast<XFilePicker*>( this ) );
    }

    SetFilters();

    int btn = GTK_RESPONSE_NO;
    int nRes = GTK_RESPONSE_CANCEL;

    while( GTK_RESPONSE_NO == btn )
    {
        btn = GTK_RESPONSE_YES; // we dont want to repeat unless user clicks NO for file save.

        resumeEventNotification();
        gint nStatus = gtk_dialog_run( GTK_DIALOG( m_pDialog ) );
        suspendEventNotification();
        switch( nStatus )
        {
            case GTK_RESPONSE_ACCEPT:
                if( GTK_FILE_CHOOSER_ACTION_SAVE == gtk_file_chooser_get_action( GTK_FILE_CHOOSER( m_pDialog ) ) )
                {
                    Sequence < OUString > aPathSeq = getFiles();
                    if( aPathSeq.getLength() == 1 )
                    {
                        OString sFileName = rtl::OUStringToOString( aPathSeq[0], RTL_TEXTENCODING_UTF8 );
                        if( g_file_test( g_filename_from_uri( sFileName.getStr(), NULL, NULL ), G_FILE_TEST_IS_REGULAR ) )
                        {
                            GtkWidget *dlg;

                            dlg = gtk_message_dialog_new( GTK_WINDOW( m_pDialog ), GTK_DIALOG_MODAL,
                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_YES_NO,
                                "The file already exists. Overwrite?" );

                            gtk_window_set_title( GTK_WINDOW( dlg ), "Overwrite file?" );
                            gtk_dialog_set_has_separator( GTK_DIALOG( dlg ), FALSE );

                            btn = gtk_dialog_run( GTK_DIALOG( dlg ) );
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

            default:
                retVal = 0;
                break;
        }
    }

    gtk_widget_hide( m_pDialog );

    shutdownEventNotification();
    return retVal;
}

//------------------------------------------------------------------------------------



// cf. offapi/com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.idl
GtkWidget *SalGtkFilePicker::getWidget( sal_Int16 nControlId, GType *pType )
{
    GType      tType;
    GtkWidget *pWidget;

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
            pWidget = m_pLists[elem]; tType = GTK_TYPE_LIST; \
            break
#define MAP_LIST_LABEL( elem ) \
        case ExtendedFilePickerElementIds::LISTBOX_##elem: \
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
        MAP_LIST_LABEL( VERSION_LABEL );
        MAP_LIST_LABEL( TEMPLATE_LABEL );
        MAP_LIST_LABEL( IMAGE_TEMPLATE_LABEL );
    default:
        OSL_TRACE("Handle unknown control %d\n", nControlId);
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

void SAL_CALL SalGtkFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
    throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OSL_TRACE( "SETTING VALUE %d\n", nControlAction );
    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("enable unknown control %d\n", nControlId);

    else if( tType == GTK_TYPE_TOGGLE_BUTTON )
    {
        sal_Bool bChecked;
        rValue >>= bChecked;
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pWidget ), bChecked );
    }
    else
        OSL_TRACE("Can't set value on button / list %d %d\n",
            nControlId, nControlAction);
}

uno::Any SAL_CALL SalGtkFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction )
    throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Any aRetval;

    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("enable unknown control %d\n", nControlId);

    else if( tType == GTK_TYPE_TOGGLE_BUTTON )
        aRetval <<= (sal_Bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( pWidget ) );

    else
        OSL_TRACE("Can't get value on button / list %d %d\n",
            nControlId, nControlAction );

    return aRetval;
}

void SAL_CALL SalGtkFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable )
throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    GtkWidget *pWidget;

    if( ( pWidget = getWidget( nControlId ) ) )
    {
        if( bEnable )
        {
            OSL_TRACE( "enable\n" );
//          gtk_widget_show( pWidget );
            gtk_widget_set_sensitive( pWidget, TRUE );
        }
        else
        {
            OSL_TRACE( "disable\n" );
//          gtk_widget_hide( pWidget );
            gtk_widget_set_sensitive( pWidget, FALSE );
        }
    }
    else
        OSL_TRACE("enable unknown control %d\n", nControlId );
}

void SAL_CALL SalGtkFilePicker::setLabel( sal_Int16 nControlId, const ::rtl::OUString& aLabel )
    throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OString aTxt = OUStringToOString( aLabel, RTL_TEXTENCODING_UTF8 );

    GType tType;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
    {
          OSL_TRACE("Set label on unknown control %d\n", nControlId);
        return;
    }
    if( tType == GTK_TYPE_TOGGLE_BUTTON || tType == GTK_TYPE_BUTTON || tType == GTK_TYPE_LABEL )
        g_object_set( pWidget, "label", aTxt.getStr(),
                      "use_underline", TRUE, NULL );
    else
        OSL_TRACE("Can't set label on list\n");
}

rtl::OUString SAL_CALL SalGtkFilePicker::getLabel( sal_Int16 nControlId )
    throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    GType tType;
    OString aTxt;
    GtkWidget *pWidget;

    if( !( pWidget = getWidget( nControlId, &tType ) ) )
        OSL_TRACE("Get label on unknown control %d\n", nControlId);

    else if( tType == GTK_TYPE_TOGGLE_BUTTON || tType == GTK_TYPE_BUTTON || tType == GTK_TYPE_LABEL )
        aTxt = gtk_button_get_label( GTK_BUTTON( pWidget ) );

    else
        OSL_TRACE("Can't get label on list\n");

    return OStringToOUString( aTxt, RTL_TEXTENCODING_UTF8 );
}

//------------------------------------------------------------------------------------
// XFilePreview functions
//------------------------------------------------------------------------------------

uno::Sequence<sal_Int16> SAL_CALL SalGtkFilePicker::getSupportedImageFormats() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getSupportedImageFormats();
    return 0;
}

sal_Int32 SAL_CALL SalGtkFilePicker::getTargetColorDepth() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getTargetColorDepth();
    return 0;
}

sal_Int32 SAL_CALL SalGtkFilePicker::getAvailableWidth() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getAvailableWidth();
    return m_PreviewImageWidth;
}

sal_Int32 SAL_CALL SalGtkFilePicker::getAvailableHeight() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getAvailableHeight();
    return m_PreviewImageHeight;
}

void SAL_CALL SalGtkFilePicker::setImage( sal_Int16 aImageFormat, const uno::Any& aImage )
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO m_pImpl->setImage( aImageFormat, aImage );
}

void SalGtkFilePicker::filter_changed_cb( GtkFileChooser *file_chooser, GParamSpec *pspec,
    SalGtkFilePicker *pobjFP )
{
    FilePickerEvent evt;
    evt.ElementId = LISTBOX_FILTER;
    OSL_TRACE( "filter_changed, isn't it great %x\n", pobjFP );
    pobjFP->controlStateChanged( evt );
}

void SalGtkFilePicker::update_preview_cb( GtkFileChooser *file_chooser, gpointer data )
{
    GtkWidget *preview;
    char *filename;
    GdkPixbuf *pixbuf;
    gboolean have_preview = FALSE;

    SalGtkFilePicker* pobjFP = ( SalGtkFilePicker * )data;

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
            gdk_pixbuf_unref( pixbuf );

    }

    gtk_file_chooser_set_preview_widget_active( file_chooser, have_preview );

    if( filename )
        g_free( filename );

}


sal_Bool SAL_CALL SalGtkFilePicker::setShowState( sal_Bool bShowState ) throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->setShowState( bShowState );
    if( bShowState != mbPreviewState )
    {
        if( bShowState )
        {
            // Show
            if( !mHID_Preview )
            {
                // This is the first time
                m_pPreview = gtk_image_new();
                gtk_file_chooser_set_preview_widget(
                    GTK_FILE_CHOOSER( m_pDialog ), m_pPreview );
                mHID_Preview = g_signal_connect(
                        GTK_FILE_CHOOSER( m_pDialog ), "update-preview",
                        G_CALLBACK( update_preview_cb ), ( gpointer )this );
            }
            else
            {
                gtk_widget_show( m_pPreview );
            }

        }
        else
        {
            // Hide
            gtk_widget_hide( m_pPreview );

            if( m_pPreview )
                gtk_widget_hide( m_pPreview );
        }

        // also emit the signal
        g_signal_emit_by_name( GTK_OBJECT( m_pDialog ), "update-preview" );

        mbPreviewState = bShowState;
    }
    return true;
}

sal_Bool SAL_CALL SalGtkFilePicker::getShowState() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO return m_pImpl->getShowState();
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
            rtl::OUString::createFromAscii( "no arguments" ),
            static_cast<XFilePicker*>( this ), 1 );

    aAny = aArguments[0];

    if( ( aAny.getValueType() != ::getCppuType( ( sal_Int16* )0 ) ) &&
         (aAny.getValueType() != ::getCppuType( ( sal_Int8* )0 ) ) )
         throw lang::IllegalArgumentException(
            rtl::OUString::createFromAscii( "invalid argument type" ),
            static_cast<XFilePicker*>( this ), 1 );

    sal_Int16 templateId = -1;
    aAny >>= templateId;

    GtkFileChooserAction eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
    gchar *first_button_text = GTK_STOCK_OPEN;


    //   TODO: extract full semantic from
    //   svtools/source/filepicker/filepicker.cxx (getWinBits)
    switch( templateId )
    {
        case FILEOPEN_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            OSL_TRACE( "3all true\n" );
            break;
        case FILESAVE_SIMPLE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            OSL_TRACE( "2all true\n" );
                break;
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[AUTOEXTENSION] = true;
            mbToggleVisibility[PASSWORD] = true;
            OSL_TRACE( "1all true\n" );
            // TODO
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[AUTOEXTENSION] = true;
            mbToggleVisibility[PASSWORD] = true;
            mbToggleVisibility[FILTEROPTIONS] = true;
            OSL_TRACE( "4all true\n" );
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE; // SELECT_FOLDER ?
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[AUTOEXTENSION] = true;
            mbToggleVisibility[SELECTION] = true;
            OSL_TRACE( "5all true\n" );
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[AUTOEXTENSION] = true;
            OSL_TRACE( "6all true\n" );
            // TODO
                break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[PREVIEW] = true;
            // TODO
                break;
        case FILEOPEN_PLAY:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            // TODO
                break;
        case FILEOPEN_READONLY_VERSION:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[READONLY] = true;
            // TODO show 'readonly' control cf.
            break;
        case FILEOPEN_LINK_PREVIEW:
            eAction = GTK_FILE_CHOOSER_ACTION_OPEN;
            first_button_text = GTK_STOCK_OPEN;
            mbToggleVisibility[PREVIEW] = true;
            // TODO
                break;
        case FILESAVE_AUTOEXTENSION:
            eAction = GTK_FILE_CHOOSER_ACTION_SAVE;
            first_button_text = GTK_STOCK_SAVE;
            mbToggleVisibility[AUTOEXTENSION] = true;
            OSL_TRACE( "7all true\n" );
            // TODO
                break;
        default:
                throw lang::IllegalArgumentException(
                rtl::OUString::createFromAscii( "Unknown template" ),
                static_cast< XFilePicker* >( this ),
                1 );
    }

    CResourceProvider aResProvider;
    OUString aLabel;

    m_pDialog = gtk_file_chooser_dialog_new( "File Selection", NULL,
            eAction,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            first_button_text, GTK_RESPONSE_ACCEPT,
            NULL );

    gtk_dialog_set_default_response( GTK_DIALOG (m_pDialog), GTK_RESPONSE_ACCEPT );

    gtk_file_chooser_set_local_only( GTK_FILE_CHOOSER( m_pDialog ), FALSE );
    gtk_file_chooser_set_select_multiple( GTK_FILE_CHOOSER( m_pDialog ), FALSE );

    m_pVBox = gtk_vbox_new( FALSE, 0 );

        int i;

    for( i = 0; i < TOGGLE_LAST; i++ )
    {
        m_pToggles[i] = gtk_check_button_new();

#define LABEL_TOGGLE( elem ) \
        case elem : \
            aLabel = aResProvider.getResString( CHECKBOX_##elem ); \
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
                OSL_TRACE("Handle unknown control %d\n", i);
                break;
        }


        gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pToggles[i], FALSE, TRUE, 0 );
    }

    for( i = 0; i < BUTTON_LAST; i++ )
    {
        m_pButtons[i] = gtk_button_new();

#define LABEL_BUTTON( elem ) \
        case elem : \
            aLabel = aResProvider.getResString( PUSHBUTTON_##elem ); \
            setLabel( PUSHBUTTON_##elem, aLabel ); \
            break

          switch( i ) {

        LABEL_BUTTON( PLAY );
            default:
                OSL_TRACE("Handle unknown control %d\n", i);
                break;
        }

        gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pButtons[i], FALSE, TRUE, 0 );
    }

    for( i = 0; i < LIST_LAST; i++ )
    {
        m_pLists[i] = gtk_list_new();
        gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pLists[i], FALSE, TRUE, 0 );
    }

    for( i = 0; i < LIST_LABEL_LAST; i++ )
    {
        m_pListLabels[i] = gtk_label_new( "" );

#define LABEL_LIST( elem ) \
        case elem : \
            aLabel = aResProvider.getResString( LISTBOX_##elem ); \
            setLabel( LISTBOX_##elem, aLabel ); \
            break

          switch( i ) {

        LABEL_LIST( VERSION_LABEL );
        LABEL_LIST( TEMPLATE_LABEL );
        LABEL_LIST( IMAGE_TEMPLATE_LABEL );
            default:
                OSL_TRACE("Handle unknown control %d\n", i);
                break;
        }

        gtk_box_pack_end( GTK_BOX( m_pVBox ), m_pListLabels[i], FALSE, TRUE, 0 );
    }

    gtk_file_chooser_set_extra_widget( GTK_FILE_CHOOSER( m_pDialog ), m_pVBox );

    // Setup special flags
    for( int nTVIndex = 0; nTVIndex < TOGGLE_LAST; nTVIndex++ )
    {
        if( mbToggleVisibility[nTVIndex] )
            gtk_widget_show( m_pToggles[ nTVIndex ] );
    }

    gtk_widget_show( m_pVBox );

    // if Preview check is visible, connect the signal handler
    if( mbToggleVisibility[PREVIEW] )
        gtk_signal_connect( GTK_OBJECT( m_pToggles[PREVIEW] ), "toggled",
            GTK_SIGNAL_FUNC( preview_toggled ), ( gpointer )this );

    //Be informed when a filter changes
    g_signal_connect( GTK_OBJECT( m_pDialog ), "notify::filter",
            G_CALLBACK( filter_changed_cb ), this );
}

void SalGtkFilePicker::preview_toggled( GtkObject *cb, gpointer data )
{
    SalGtkFilePicker* pobjFP = ( SalGtkFilePicker *) data;

    pobjFP->setShowState( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( cb ) ) );
}

//------------------------------------------------------------------------------------
// XCancellable
//------------------------------------------------------------------------------------

void SAL_CALL SalGtkFilePicker::cancel() throw( uno::RuntimeException )
{
    OSL_ASSERT( m_pDialog != NULL );
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // TODO m_pImpl->cancel();
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

rtl::OUString SAL_CALL SalGtkFilePicker::getImplementationName()
    throw( uno::RuntimeException )
{
    return rtl::OUString::createFromAscii( FILE_PICKER_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL SalGtkFilePicker::supportsService( const rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    uno::Sequence <rtl::OUString> SupportedServicesNames = FilePicker_getSupportedServiceNames();

    for( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if( SupportedServicesNames[n].compareTo( ServiceName ) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL SalGtkFilePicker::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    return FilePicker_getSupportedServiceNames();
}


//--------------------------------------------------
// Misc
//-------------------------------------------------
void SalGtkFilePicker::SetCurFilter( const OUString& rFilter )
{
    // Get all the filters already added
    GSList *filters = gtk_file_chooser_list_filters ( GTK_FILE_CHOOSER( m_pDialog ) );
    bool bFound = false;

    while( ( !bFound ) && ( NULL != filters ) )
    {
        GtkFileFilter* pFilter = reinterpret_cast<GtkFileFilter *>( filters->data );
        G_CONST_RETURN gchar * filtername = gtk_file_filter_get_name( pFilter );
        OUString sFilterName( filtername, strlen( filtername ), RTL_TEXTENCODING_UTF8 );

        OUString aShrunkName = shrinkFilterName( rFilter );
        if( aShrunkName.equals( sFilterName) )
        {
            OSL_TRACE( "actually setting %s\n", filtername );
            gtk_file_chooser_set_filter( GTK_FILE_CHOOSER( m_pDialog ), pFilter );
            bFound = true;
        }

        // Free the node
        g_object_ref( pFilter );
        gtk_object_sink( GTK_OBJECT( pFilter ) );
        g_object_unref( pFilter );

        //    g_free(filters->data);
        filters = g_slist_next( filters );
    }

    g_slist_free( filters );
}

void SalGtkFilePicker::implAddFilter( const OUString& rFilter, const OUString& rType )
{
    GtkFileFilter *filter = gtk_file_filter_new();

    OUString aShrunkName = shrinkFilterName( rFilter );
    OString aFilterName = rtl::OUStringToOString( aShrunkName, RTL_TEXTENCODING_UTF8 );
    gtk_file_filter_set_name( filter, aFilterName );

    if( !rType.compareToAscii( "*.*" ) )
        gtk_file_filter_add_pattern( filter, "*" );
    else
    {
        sal_Int32 nIndex = 0;
        rtl::OUString aToken;
        do
        {
            aToken = rType.getToken( 0, ';', nIndex );
            if( aToken.getLength() )
            {
                OString aStr = rtl::OUStringToOString( aToken, RTL_TEXTENCODING_UTF8 );
                gtk_file_filter_add_pattern( filter, aStr );
            }
        }
        while( nIndex >= 0 );
    }

    gtk_file_chooser_add_filter( GTK_FILE_CHOOSER( m_pDialog ), filter );
}

void SalGtkFilePicker::implAddFilterGroup( const OUString& _rFilter, const Sequence< StringPair >& _rFilters )
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
    OSL_TRACE( "start setting filters\n");
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


    // set the default filter
    if( m_aCurrentFilter && (m_aCurrentFilter.getLength() > 0) )
    {
        OSL_TRACE( "Setting current filter to %s\n",
            OUStringToOString( m_aCurrentFilter, RTL_TEXTENCODING_UTF8 ).getStr() );

        SetCurFilter( m_aCurrentFilter );
    }

    OSL_TRACE( "end setting filters\n");
}

SalGtkFilePicker::~SalGtkFilePicker()
{
    int i;

    for( i = 0; i < TOGGLE_LAST; i++ )
        gtk_widget_destroy( m_pToggles[i] );

    for( i = 0; i < BUTTON_LAST; i++ )
        gtk_widget_destroy( m_pButtons[i] );

    for( i = 0; i < LIST_LAST; i++ )
        gtk_widget_destroy( m_pLists[i] );

    for( i = 0; i < LIST_LABEL_LAST; i++ )
        gtk_widget_destroy( m_pListLabels[i] );

    if ( m_pFilterList )
        m_pFilterList->clear();

    gtk_widget_destroy( m_pVBox );
}
