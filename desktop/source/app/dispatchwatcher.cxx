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


#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include "osl/file.hxx"
#include "sfx2/app.hxx"
#include <svl/fstathelper.hxx>

#include "dispatchwatcher.hxx"
#include <rtl/ustring.hxx>
#include <tools/string.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/synchronousdispatch.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <tools/urlobj.hxx>
#include <comphelper/mediadescriptor.hxx>

#include <vector>
#include <osl/thread.hxx>
#include <rtl/instance.hxx>

using ::rtl::OUString;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;

namespace desktop
{

String GetURL_Impl(
    const String& rName, boost::optional< rtl::OUString > const & cwdUrl );

struct DispatchHolder
{
    DispatchHolder( const URL& rURL, Reference< XDispatch >& rDispatch ) :
        aURL( rURL ), xDispatch( rDispatch ) {}

    URL aURL;
    rtl::OUString cwdUrl;
    Reference< XDispatch > xDispatch;
};

static String impl_GetFilterFromExt( OUString aUrl, SfxFilterFlags nFlags,
                                        String aAppl )
{
    String aFilter;
    SfxMedium* pMedium = new SfxMedium( aUrl,
                                        STREAM_STD_READ );

    const SfxFilter *pSfxFilter = NULL;
    if( nFlags == SFX_FILTER_EXPORT )
    {
        SfxFilterMatcher( aAppl ).GuessFilterIgnoringContent( *pMedium, &pSfxFilter, nFlags, 0 );
    }
    else
    {
        SFX_APP()->GetFilterMatcher().GuessFilter( *pMedium, &pSfxFilter, nFlags, 0 );
    }

    if( pSfxFilter )
    {
        if (nFlags == SFX_FILTER_EXPORT)
            aFilter = pSfxFilter->GetFilterName();
        else
            aFilter = pSfxFilter->GetServiceName();
    }

    delete pMedium;
    return aFilter;
}
static OUString impl_GuessFilter( OUString aUrlIn, OUString aUrlOut )
{
    /* aAppl can also be set to Factory like scalc, swriter... */
    String aAppl;
    aAppl = impl_GetFilterFromExt( aUrlIn, SFX_FILTER_IMPORT, aAppl );
    return  impl_GetFilterFromExt( aUrlOut, SFX_FILTER_EXPORT, aAppl );
}

namespace
{
    class theWatcherMutex : public rtl::Static<Mutex, theWatcherMutex> {};
}

Mutex& DispatchWatcher::GetMutex()
{
    return theWatcherMutex::get();
}

// Create or get the dispatch watcher implementation. This implementation must be
// a singleton to prevent access to the framework after it wants to terminate.
DispatchWatcher* DispatchWatcher::GetDispatchWatcher()
{
    static Reference< XInterface > xDispatchWatcher;
    static DispatchWatcher*        pDispatchWatcher = NULL;

    if ( !xDispatchWatcher.is() )
    {
        ::osl::MutexGuard aGuard( GetMutex() );

        if ( !xDispatchWatcher.is() )
        {
            pDispatchWatcher = new DispatchWatcher();

            // We have to hold a reference to ourself forever to prevent our own destruction.
            xDispatchWatcher = static_cast< cppu::OWeakObject *>( pDispatchWatcher );
        }
    }

    return pDispatchWatcher;
}


DispatchWatcher::DispatchWatcher()
    : m_nRequestCount(0)
{
}


DispatchWatcher::~DispatchWatcher()
{
}


sal_Bool DispatchWatcher::executeDispatchRequests( const DispatchList& aDispatchRequestsList, bool bNoTerminate )
{
    Reference< XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                                                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                                            UNO_QUERY );

    DispatchList::const_iterator    p;
    std::vector< DispatchHolder >   aDispatches;
    ::rtl::OUString                 aAsTemplateArg( RTL_CONSTASCII_USTRINGPARAM( "AsTemplate"));
    sal_Bool                        bSetInputFilter = sal_False;
    ::rtl::OUString                 aForcedInputFilter;

    for ( p = aDispatchRequestsList.begin(); p != aDispatchRequestsList.end(); ++p )
    {
        const DispatchRequest&  aDispatchRequest = *p;

        // create parameter array
        sal_Int32 nCount = 4;
        if ( !aDispatchRequest.aPreselectedFactory.isEmpty() )
            nCount++;

        // Set Input Filter
        if ( aDispatchRequest.aRequestType == REQUEST_INFILTER )
        {
            bSetInputFilter = sal_True;
            aForcedInputFilter = aDispatchRequest.aURL;
            OfficeIPCThread::RequestsCompleted( 1 );
            continue;
        }

        // we need more properties for a print/print to request
        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
            nCount++;

        Sequence < PropertyValue > aArgs( nCount );

        // mark request as user interaction from outside
        aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer"));
        aArgs[0].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:OpenEvent"));

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
        {
            aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OpenNewView"));
            aArgs[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
            aArgs[4].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Silent"));
        }
        else
        {
            Reference < XInteractionHandler2 > xInteraction(
                InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), 0) );

            aArgs[1].Name = OUString(RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
            aArgs[1].Value <<= xInteraction;

            sal_Int16 nMacroExecMode = ::com::sun::star::document::MacroExecMode::USE_CONFIG;
            aArgs[2].Name = OUString(RTL_CONSTASCII_USTRINGPARAM( "MacroExecutionMode" ));
            aArgs[2].Value <<= nMacroExecMode;

            sal_Int16 nUpdateDoc = ::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG;
            aArgs[3].Name = OUString(RTL_CONSTASCII_USTRINGPARAM( "UpdateDocMode" ));
            aArgs[3].Value <<= nUpdateDoc;
        }

        if ( !aDispatchRequest.aPreselectedFactory.isEmpty() )
        {
            aArgs[nCount-1].Name = ::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE();
            aArgs[nCount-1].Value <<= aDispatchRequest.aPreselectedFactory;
        }

        String aName( GetURL_Impl( aDispatchRequest.aURL, aDispatchRequest.aCwdUrl ) );
        ::rtl::OUString aTarget( RTL_CONSTASCII_USTRINGPARAM("_default") );

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
        {
            // documents opened for printing are opened readonly because they must be opened as a new document and this
            // document could be open already
            aArgs[1].Value <<= sal_True;

            // always open a new document for printing, because it must be disposed afterwards
            aArgs[2].Value <<= sal_True;

            // printing is done in a hidden view
            aArgs[3].Value <<= sal_True;

            // load document for printing without user interaction
            aArgs[4].Value <<= sal_True;

            // hidden documents should never be put into open tasks
            aTarget = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") );
        }
        // load the document ... if they are loadable!
        // Otherwise try to dispatch it ...
        Reference < XPrintable > xDoc;
        if(
            ( aName.CompareToAscii( ".uno"  , 4 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii( "slot:" , 5 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii( "macro:", 6 ) == COMPARE_EQUAL )  ||
            ( aName.CompareToAscii("vnd.sun.star.script", 19) == COMPARE_EQUAL)
          )
        {
            // Attention: URL must be parsed full. Otherwise some detections on it will fail!
            // It doesnt matter, if parser isn't available. Because; We try loading of URL then ...
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XDispatchProvider > xProvider   ( xDesktop, UNO_QUERY );
            Reference < XURLTransformer >   xParser     ( URLTransformer::create(::comphelper::getProcessComponentContext()) );

            if( xParser.is() == sal_True )
                xParser->parseStrict( aURL );

            if( xProvider.is() == sal_True )
                xDispatcher = xProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );

            if( xDispatcher.is() == sal_True )
            {
                {
                    ::osl::ClearableMutexGuard aGuard( GetMutex() );
                    // Remember request so we can find it in statusChanged!
                    m_aRequestContainer.insert( DispatchWatcherHashMap::value_type( aURL.Complete, (sal_Int32)1 ) );
                    m_nRequestCount++;
                }

                // Use local vector to store dispatcher because we have to fill our request container before
                // we can dispatch. Otherwise it would be possible that statusChanged is called before we dispatched all requests!!
                aDispatches.push_back( DispatchHolder( aURL, xDispatcher ));
            }
        }
        else if ( ( aName.CompareToAscii( "service:"  , 8 ) == COMPARE_EQUAL ) )
        {
            // TODO: the dispatch has to be done for loadComponentFromURL as well. Please ask AS for more details.
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XDispatchProvider > xProvider   ( xDesktop, UNO_QUERY );
            Reference < XURLTransformer >   xParser     ( URLTransformer::create(::comphelper::getProcessComponentContext()) );

            if( xParser.is() == sal_True )
                xParser->parseStrict( aURL );

            if( xProvider.is() == sal_True )
                xDispatcher = xProvider->queryDispatch( aURL, ::rtl::OUString(), 0 );

            if( xDispatcher.is() == sal_True )
            {
                try
                {
                    // We have to be listener to catch errors during dispatching URLs.
                    // Otherwise it would be possible to have an office running without an open
                    // window!!
                    Sequence < PropertyValue > aArgs2(1);
                    aArgs2[0].Name    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SynchronMode"));
                    aArgs2[0].Value <<= sal_True;
                    Reference < XNotifyingDispatch > xDisp( xDispatcher, UNO_QUERY );
                    if ( xDisp.is() )
                        xDisp->dispatchWithNotification( aURL, aArgs2, DispatchWatcher::GetDispatchWatcher() );
                    else
                        xDispatcher->dispatch( aURL, aArgs2 );
                }
                catch (const ::com::sun::star::uno::Exception&)
                {
                    OUString aMsg = OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Desktop::OpenDefault() IllegalArgumentException while calling XNotifyingDispatch: "));
                    OSL_FAIL( OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
                }
            }
        }
        else
        {
            INetURLObject aObj( aName );
            if ( aObj.GetProtocol() == INET_PROT_PRIVATE )
                aTarget = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_default") );

            // Set "AsTemplate" argument according to request type
            if ( aDispatchRequest.aRequestType == REQUEST_FORCENEW ||
                 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN     )
            {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc( nIndex+1 );
                aArgs[nIndex].Name = aAsTemplateArg;
                if ( aDispatchRequest.aRequestType == REQUEST_FORCENEW )
                    aArgs[nIndex].Value <<= sal_True;
                else
                    aArgs[nIndex].Value <<= sal_False;
            }

            // if we are called in viewmode, open document read-only
            if(aDispatchRequest.aRequestType == REQUEST_VIEW) {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
                aArgs[nIndex].Value <<= sal_True;
            }

            // if we are called with -start set Start in mediadescriptor
            if(aDispatchRequest.aRequestType == REQUEST_START) {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("StartPresentation"));
                aArgs[nIndex].Value <<= sal_True;
            }

            // Force input filter, if possible
            if( bSetInputFilter )
            {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
                aArgs[nIndex].Value <<= aForcedInputFilter;
            }

            // This is a synchron loading of a component so we don't have to deal with our statusChanged listener mechanism.
            try
            {
                xDoc = Reference < XPrintable >( ::comphelper::SynchronousDispatch::dispatch( xDesktop, aName, aTarget, 0, aArgs ), UNO_QUERY );
            }
            catch (const ::com::sun::star::lang::IllegalArgumentException& iae)
            {
                OUString aMsg = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Dispatchwatcher IllegalArgumentException while calling loadComponentFromURL: "))
                    + iae.Message;
                OSL_FAIL( OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
            }
            catch (const com::sun::star::io::IOException& ioe)
            {
                OUString aMsg = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Dispatchwatcher IOException while calling loadComponentFromURL: "))
                    + ioe.Message;
                OSL_FAIL( OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
            }
            if ( aDispatchRequest.aRequestType == REQUEST_OPEN ||
                 aDispatchRequest.aRequestType == REQUEST_VIEW ||
                 aDispatchRequest.aRequestType == REQUEST_START ||
                 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN ||
                 aDispatchRequest.aRequestType == REQUEST_FORCENEW      )
            {
                // request is completed
                OfficeIPCThread::RequestsCompleted( 1 );
            }
            else if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
                      aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
                      aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
                      aDispatchRequest.aRequestType == REQUEST_CONVERSION )
            {
                if ( xDoc.is() )
                {
                    if ( aDispatchRequest.aRequestType == REQUEST_CONVERSION ) {
                        Reference< XStorable > xStorable( xDoc, UNO_QUERY );
                        if ( xStorable.is() ) {
                            rtl::OUString aParam = aDispatchRequest.aPrinterName;
                            sal_Int32 nPathIndex =  aParam.lastIndexOf( ';' );
                            sal_Int32 nFilterIndex = aParam.indexOf( ':' );
                            if( nPathIndex < nFilterIndex )
                                nFilterIndex = -1;
                            rtl::OUString aFilterOut=aParam.copy( nPathIndex+1 );
                            rtl::OUString aFilter;
                            rtl::OUString aFilterExt;
                            sal_Bool bGuess = sal_False;

                            if( nFilterIndex >= 0 )
                            {
                                aFilter = aParam.copy( nFilterIndex+1, nPathIndex-nFilterIndex-1 );
                                aFilterExt = aParam.copy( 0, nFilterIndex );
                            }
                            else
                            {
                                // Guess
                                bGuess = sal_True;
                                aFilterExt = aParam.copy( 0, nPathIndex );
                            }
                            INetURLObject aOutFilename( aObj );
                            aOutFilename.SetExtension( aFilterExt );
                            FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
                            rtl::OUString aOutFile = aFilterOut+
                                                     ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/" ))+
                                                     aOutFilename.getName();

                            if ( bGuess )
                            {
                                aFilter = impl_GuessFilter( aName, aOutFile );
                            }

                            Sequence<PropertyValue> conversionProperties( 2 );
                            conversionProperties[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Overwrite" ));
                            conversionProperties[0].Value <<= sal_True;

                            conversionProperties[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
                            conversionProperties[1].Value <<= aFilter;

                            rtl::OUString aTempName;
                            FileBase::getSystemPathFromFileURL( aName, aTempName );
                            rtl::OString aSource8 = ::rtl::OUStringToOString ( aTempName, RTL_TEXTENCODING_UTF8 );
                            FileBase::getSystemPathFromFileURL( aOutFile, aTempName );
                            rtl::OString aTargetURL8 = ::rtl::OUStringToOString(aTempName, RTL_TEXTENCODING_UTF8 );
                            printf("convert %s -> %s using %s\n", aSource8.getStr(), aTargetURL8.getStr(),
                                   ::rtl::OUStringToOString( aFilter, RTL_TEXTENCODING_UTF8 ).getStr());
                            if( FStatHelper::IsDocument(aOutFile) )
                                printf("Overwriting: %s\n",::rtl::OUStringToOString( aTempName, RTL_TEXTENCODING_UTF8 ).getStr() );
                            try
                            {
                                xStorable->storeToURL( aOutFile, conversionProperties );
                            }
                            catch (const Exception&)
                            {
                                fprintf( stderr, "Error: Please reverify input parameters...\n" );
                            }
                        }
                    } else if ( aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ) {
                        rtl::OUString aParam = aDispatchRequest.aPrinterName;
                        sal_Int32 nPathIndex =  aParam.lastIndexOf( ';' );

                        rtl::OUString aFilterOut;
                        rtl::OUString aPrinterName;
                        if( nPathIndex != -1 )
                            aFilterOut=aParam.copy( nPathIndex+1 );
                        if( nPathIndex != 0 )
                            aPrinterName=aParam.copy( 0, nPathIndex );

                        INetURLObject aOutFilename( aObj );
                        aOutFilename.SetExtension( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ps")) );
                        FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
                        rtl::OUString aOutFile = aFilterOut+
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/" ))+
                            aOutFilename.getName();

                        rtl::OUString aTempName;
                        FileBase::getSystemPathFromFileURL( aName, aTempName );
                        rtl::OString aSource8 = ::rtl::OUStringToOString ( aTempName, RTL_TEXTENCODING_UTF8 );
                        FileBase::getSystemPathFromFileURL( aOutFile, aTempName );
                        rtl::OString aTargetURL8 = ::rtl::OUStringToOString(aTempName, RTL_TEXTENCODING_UTF8 );
                        printf("print %s -> %s using %s\n", aSource8.getStr(), aTargetURL8.getStr(),
                               aPrinterName.isEmpty() ?
                                                     "<default_printer>" : ::rtl::OUStringToOString( aPrinterName, RTL_TEXTENCODING_UTF8 ).getStr() );

                        // create the custom printer, if given
                        Sequence < PropertyValue > aPrinterArgs( 1 );
                        if( !aPrinterName.isEmpty() )
                        {
                            aPrinterArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
                            aPrinterArgs[0].Value <<= aPrinterName;
                            xDoc->setPrinter( aPrinterArgs );
                        }

                        // print ( also without user interaction )
                        aPrinterArgs.realloc(2);
                        aPrinterArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileName"));
                        aPrinterArgs[0].Value <<= aOutFile;
                        aPrinterArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wait"));
                        aPrinterArgs[1].Value <<= ( sal_Bool ) sal_True;
                        xDoc->print( aPrinterArgs );
                    } else {
                        if ( aDispatchRequest.aRequestType == REQUEST_PRINTTO )
                        {
                            // create the printer
                            Sequence < PropertyValue > aPrinterArgs( 1 );
                            aPrinterArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
                            aPrinterArgs[0].Value <<= ::rtl::OUString( aDispatchRequest.aPrinterName );
                            xDoc->setPrinter( aPrinterArgs );
                        }

                        // print ( also without user interaction )
                        Sequence < PropertyValue > aPrinterArgs( 1 );
                        aPrinterArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wait"));
                        aPrinterArgs[0].Value <<= ( sal_Bool ) sal_True;
                        xDoc->print( aPrinterArgs );
                    }
                }
                else
                {
                    // place error message here ...
                }

                // remove the document
                try
                {
                    Reference < XCloseable > xClose( xDoc, UNO_QUERY );
                    if ( xClose.is() )
                        xClose->close( sal_True );
                    else
                    {
                        Reference < XComponent > xComp( xDoc, UNO_QUERY );
                        if ( xComp.is() )
                            xComp->dispose();
                    }
                }
                catch (const com::sun::star::util::CloseVetoException&)
                {
                }

                // request is completed
                OfficeIPCThread::RequestsCompleted( 1 );
            }
        }
    }

    if ( !aDispatches.empty() )
    {
        // Execute all asynchronous dispatches now after we placed them into our request container!
        Sequence < PropertyValue > aArgs( 2 );
        aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer"));
        aArgs[0].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:OpenEvent"));
        aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SynchronMode"));
        aArgs[1].Value <<= sal_True;

        for ( sal_uInt32 n = 0; n < aDispatches.size(); n++ )
        {
            Reference< XDispatch > xDispatch = aDispatches[n].xDispatch;
            Reference < XNotifyingDispatch > xDisp( xDispatch, UNO_QUERY );
            if ( xDisp.is() )
                xDisp->dispatchWithNotification( aDispatches[n].aURL, aArgs, this );
            else
            {
                ::osl::ClearableMutexGuard aGuard( GetMutex() );
                m_nRequestCount--;
                aGuard.clear();
                xDispatch->dispatch( aDispatches[n].aURL, aArgs );
            }
        }
    }

    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    bool bEmpty = (m_nRequestCount == 0);
    aGuard.clear();

    // No more asynchronous requests?
    // The requests are removed from the request container after they called back to this
    // implementation via statusChanged!!
    if ( bEmpty && !bNoTerminate /*m_aRequestContainer.empty()*/ )
    {
        // We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XFramesSupplier > xTasksSupplier( xDesktop, UNO_QUERY );
        aGuard.clear();
        Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
        {
            // We don't have any task open so we have to shutdown ourself!!
            Reference< XDesktop > xDesktop2( xTasksSupplier, UNO_QUERY );
            if ( xDesktop2.is() )
                return xDesktop2->terminate();
        }
    }

    return sal_False;
}


void SAL_CALL DispatchWatcher::disposing( const ::com::sun::star::lang::EventObject& )
throw(::com::sun::star::uno::RuntimeException)
{
}


void SAL_CALL DispatchWatcher::dispatchFinished( const DispatchResultEvent& ) throw( RuntimeException )
{
    osl::ClearableMutexGuard aGuard( GetMutex() );
    sal_Int16 nCount = --m_nRequestCount;
    aGuard.clear();
    OfficeIPCThread::RequestsCompleted( 1 );
    if ( !nCount && !OfficeIPCThread::AreRequestsPending() )
    {
        // We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XFramesSupplier > xTasksSupplier( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                                                UNO_QUERY );
        Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
        {
            // We don't have any task open so we have to shutdown ourself!!
            Reference< XDesktop > xDesktop( xTasksSupplier, UNO_QUERY );
            if ( xDesktop.is() )
                xDesktop->terminate();
        }
    }
}

}








/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
