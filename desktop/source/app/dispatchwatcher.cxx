/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include "osl/file.hxx"
#include "sfx2/app.hxx"
#include <svl/fstathelper.hxx>

#include "dispatchwatcher.hxx"
#include <rtl/ustring.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/synchronousdispatch.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/Desktop.hpp>
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
#include <unotools/mediadescriptor.hxx>

#include <vector>
#include <osl/thread.hxx>
#include <rtl/instance.hxx>

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

OUString GetURL_Impl(
    const OUString& rName, boost::optional< OUString > const & cwdUrl );

struct DispatchHolder
{
    DispatchHolder( const URL& rURL, Reference< XDispatch >& rDispatch ) :
        aURL( rURL ), xDispatch( rDispatch ) {}

    URL aURL;
    OUString cwdUrl;
    Reference< XDispatch > xDispatch;
};

static OUString impl_GetFilterFromExt( OUString aUrl, SfxFilterFlags nFlags,
                                        OUString aAppl )
{
    OUString aFilter;
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
    OUString aAppl;
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
    Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );

    DispatchList::const_iterator    p;
    std::vector< DispatchHolder >   aDispatches;
    OUString                 aAsTemplateArg( "AsTemplate" );
    sal_Bool                        bSetInputFilter = sal_False;
    OUString                 aForcedInputFilter;

    for ( p = aDispatchRequestsList.begin(); p != aDispatchRequestsList.end(); ++p )
    {
        const DispatchRequest&  aDispatchRequest = *p;

        
        sal_Int32 nCount = 4;
        if ( !aDispatchRequest.aPreselectedFactory.isEmpty() )
            nCount++;

        
        if ( aDispatchRequest.aRequestType == REQUEST_INFILTER )
        {
            bSetInputFilter = sal_True;
            aForcedInputFilter = aDispatchRequest.aURL;
            OfficeIPCThread::RequestsCompleted( 1 );
            continue;
        }

        
        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
            nCount++;

        Sequence < PropertyValue > aArgs( nCount );

        
        aArgs[0].Name = "Referer";
        aArgs[0].Value <<= OUString("private:OpenEvent");

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
        {
            aArgs[1].Name = "ReadOnly";
            aArgs[2].Name = "OpenNewView";
            aArgs[3].Name = "Hidden";
            aArgs[4].Name = "Silent";
        }
        else
        {
            Reference < XInteractionHandler2 > xInteraction(
                InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), 0) );

            aArgs[1].Name = "InteractionHandler";
            aArgs[1].Value <<= xInteraction;

            sal_Int16 nMacroExecMode = ::com::sun::star::document::MacroExecMode::USE_CONFIG;
            aArgs[2].Name = "MacroExecutionMode";
            aArgs[2].Value <<= nMacroExecMode;

            sal_Int16 nUpdateDoc = ::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG;
            aArgs[3].Name = "UpdateDocMode";
            aArgs[3].Value <<= nUpdateDoc;
        }

        if ( !aDispatchRequest.aPreselectedFactory.isEmpty() )
        {
            aArgs[nCount-1].Name = utl::MediaDescriptor::PROP_DOCUMENTSERVICE();
            aArgs[nCount-1].Value <<= aDispatchRequest.aPreselectedFactory;
        }

        OUString aName( GetURL_Impl( aDispatchRequest.aURL, aDispatchRequest.aCwdUrl ) );
        OUString aTarget("_default");

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION)
        {
            
            
            aArgs[1].Value <<= sal_True;

            
            aArgs[2].Value <<= sal_True;

            
            aArgs[3].Value <<= sal_True;

            
            aArgs[4].Value <<= sal_True;

            
            aTarget = "_blank";
        }
        
        
        Reference < XPrintable > xDoc;
        if(
            ( aName.startsWith( ".uno" ) )  ||
            ( aName.startsWith( "slot:" ) )  ||
            ( aName.startsWith( "macro:" ) )  ||
            ( aName.startsWith("vnd.sun.star.script") )
          )
        {
            
            
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XURLTransformer >   xParser     ( URLTransformer::create(::comphelper::getProcessComponentContext()) );

            if( xParser.is() )
                xParser->parseStrict( aURL );

            xDispatcher = xDesktop->queryDispatch( aURL, OUString(), 0 );

            if( xDispatcher.is() )
            {
                {
                    ::osl::ClearableMutexGuard aGuard( GetMutex() );
                    
                    m_aRequestContainer.insert( DispatchWatcherHashMap::value_type( aURL.Complete, (sal_Int32)1 ) );
                    m_nRequestCount++;
                }

                
                
                aDispatches.push_back( DispatchHolder( aURL, xDispatcher ));
            }
        }
        else if ( ( aName.startsWith( "service:" ) ) )
        {
            
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XURLTransformer >   xParser     ( URLTransformer::create(::comphelper::getProcessComponentContext()) );

            if( xParser.is() )
                xParser->parseStrict( aURL );

            xDispatcher = xDesktop->queryDispatch( aURL, OUString(), 0 );

            if( xDispatcher.is() )
            {
                try
                {
                    
                    
                    
                    Sequence < PropertyValue > aArgs2(1);
                    aArgs2[0].Name    = "SynchronMode";
                    aArgs2[0].Value <<= sal_True;
                    Reference < XNotifyingDispatch > xDisp( xDispatcher, UNO_QUERY );
                    if ( xDisp.is() )
                        xDisp->dispatchWithNotification( aURL, aArgs2, DispatchWatcher::GetDispatchWatcher() );
                    else
                        xDispatcher->dispatch( aURL, aArgs2 );
                }
                catch (const ::com::sun::star::uno::Exception& e)
                {
                    SAL_WARN(
                        "desktop.app",
                        "Desktop::OpenDefault() ignoring Exception while"
                            " calling XNotifyingDispatch: \"" << e.Message
                            << "\"");
                }
            }
        }
        else
        {
            INetURLObject aObj( aName );
            if ( aObj.GetProtocol() == INET_PROT_PRIVATE )
                aTarget = "_default";

            
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

            
            if(aDispatchRequest.aRequestType == REQUEST_VIEW) {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = "ReadOnly";
                aArgs[nIndex].Value <<= sal_True;
            }

            
            if(aDispatchRequest.aRequestType == REQUEST_START) {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = "StartPresentation";
                aArgs[nIndex].Value <<= sal_True;
            }

            
            if( bSetInputFilter )
            {
                sal_Int32 nIndex = aArgs.getLength();
                aArgs.realloc(nIndex+1);
                aArgs[nIndex].Name = "FilterName";
                aArgs[nIndex].Value <<= aForcedInputFilter;
            }

            
            try
            {
                xDoc = Reference < XPrintable >( ::comphelper::SynchronousDispatch::dispatch( xDesktop, aName, aTarget, 0, aArgs ), UNO_QUERY );
            }
            catch (const ::com::sun::star::lang::IllegalArgumentException& iae)
            {
                SAL_WARN(
                    "desktop.app",
                    "Dispatchwatcher IllegalArgumentException while calling"
                        " loadComponentFromURL: \"" << iae.Message << "\"");
            }
            catch (const com::sun::star::io::IOException& ioe)
            {
                SAL_WARN(
                    "desktop.app",
                    "Dispatchwatcher IOException while calling"
                        " loadComponentFromURL: \"" << ioe.Message << "\"");
            }
            if ( aDispatchRequest.aRequestType == REQUEST_OPEN ||
                 aDispatchRequest.aRequestType == REQUEST_VIEW ||
                 aDispatchRequest.aRequestType == REQUEST_START ||
                 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN ||
                 aDispatchRequest.aRequestType == REQUEST_FORCENEW      )
            {
                
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
                            OUString aParam = aDispatchRequest.aPrinterName;
                            sal_Int32 nPathIndex =  aParam.lastIndexOf( ';' );
                            sal_Int32 nFilterIndex = aParam.indexOf( ':' );
                            if( nPathIndex < nFilterIndex )
                                nFilterIndex = -1;
                            OUString aFilterOut=aParam.copy( nPathIndex+1 );
                            OUString aFilter;
                            OUString aFilterExt;
                            sal_Bool bGuess = sal_False;

                            if( nFilterIndex >= 0 )
                            {
                                aFilter = aParam.copy( nFilterIndex+1, nPathIndex-nFilterIndex-1 );
                                aFilterExt = aParam.copy( 0, nFilterIndex );
                            }
                            else
                            {
                                
                                bGuess = sal_True;
                                aFilterExt = aParam.copy( 0, nPathIndex );
                            }
                            INetURLObject aOutFilename( aObj );
                            aOutFilename.SetExtension( aFilterExt );
                            FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
                            OUString aOutFile = aFilterOut+
                                                     "/" +
                                                     aOutFilename.getName();

                            if ( bGuess )
                            {
                                aFilter = impl_GuessFilter( aName, aOutFile );
                            }

                            Sequence<PropertyValue> conversionProperties( 2 );
                            conversionProperties[0].Name = "Overwrite";
                            conversionProperties[0].Value <<= sal_True;

                            conversionProperties[1].Name = "FilterName";
                            conversionProperties[1].Value <<= aFilter;

                            OUString aTempName;
                            FileBase::getSystemPathFromFileURL( aName, aTempName );
                            OString aSource8 = OUStringToOString ( aTempName, RTL_TEXTENCODING_UTF8 );
                            FileBase::getSystemPathFromFileURL( aOutFile, aTempName );
                            OString aTargetURL8 = OUStringToOString(aTempName, RTL_TEXTENCODING_UTF8 );
                            printf("convert %s -> %s using %s\n", aSource8.getStr(), aTargetURL8.getStr(),
                                   OUStringToOString( aFilter, RTL_TEXTENCODING_UTF8 ).getStr());
                            if( FStatHelper::IsDocument(aOutFile) )
                                printf("Overwriting: %s\n",OUStringToOString( aTempName, RTL_TEXTENCODING_UTF8 ).getStr() );
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
                        OUString aParam = aDispatchRequest.aPrinterName;
                        sal_Int32 nPathIndex =  aParam.lastIndexOf( ';' );

                        OUString aFilterOut;
                        OUString aPrinterName;
                        if( nPathIndex != -1 )
                            aFilterOut=aParam.copy( nPathIndex+1 );
                        if( nPathIndex != 0 )
                            aPrinterName=aParam.copy( 0, nPathIndex );

                        INetURLObject aOutFilename( aObj );
                        aOutFilename.SetExtension( "ps" );
                        FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
                        OUString aOutFile = aFilterOut+
                            "/" +
                            aOutFilename.getName();

                        OUString aTempName;
                        FileBase::getSystemPathFromFileURL( aName, aTempName );
                        OString aSource8 = OUStringToOString ( aTempName, RTL_TEXTENCODING_UTF8 );
                        FileBase::getSystemPathFromFileURL( aOutFile, aTempName );
                        OString aTargetURL8 = OUStringToOString(aTempName, RTL_TEXTENCODING_UTF8 );
                        printf("print %s -> %s using %s\n", aSource8.getStr(), aTargetURL8.getStr(),
                               aPrinterName.isEmpty() ?
                                                     "<default_printer>" : OUStringToOString( aPrinterName, RTL_TEXTENCODING_UTF8 ).getStr() );

                        
                        Sequence < PropertyValue > aPrinterArgs( 1 );
                        if( !aPrinterName.isEmpty() )
                        {
                            aPrinterArgs[0].Name = "Name";
                            aPrinterArgs[0].Value <<= aPrinterName;
                            xDoc->setPrinter( aPrinterArgs );
                        }

                        
                        aPrinterArgs.realloc(2);
                        aPrinterArgs[0].Name = "FileName";
                        aPrinterArgs[0].Value <<= aOutFile;
                        aPrinterArgs[1].Name = "Wait";
                        aPrinterArgs[1].Value <<= ( sal_Bool ) sal_True;
                        xDoc->print( aPrinterArgs );
                    } else {
                        if ( aDispatchRequest.aRequestType == REQUEST_PRINTTO )
                        {
                            
                            Sequence < PropertyValue > aPrinterArgs( 1 );
                            aPrinterArgs[0].Name = "Name";
                            aPrinterArgs[0].Value <<= OUString( aDispatchRequest.aPrinterName );
                            xDoc->setPrinter( aPrinterArgs );
                        }

                        
                        Sequence < PropertyValue > aPrinterArgs( 1 );
                        aPrinterArgs[0].Name = "Wait";
                        aPrinterArgs[0].Value <<= ( sal_Bool ) sal_True;
                        xDoc->print( aPrinterArgs );
                    }
                }
                else
                {
                    
                }

                
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

                
                OfficeIPCThread::RequestsCompleted( 1 );
            }
        }
    }

    if ( !aDispatches.empty() )
    {
        
        Sequence < PropertyValue > aArgs( 2 );
        aArgs[0].Name = "Referer";
        aArgs[0].Value <<= OUString("private:OpenEvent");
        aArgs[1].Name = "SynchronMode";
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

    
    
    
    if ( bEmpty && !bNoTerminate /*m_aRequestContainer.empty()*/ )
    {
        
        aGuard.clear();
        Reference< XElementAccess > xList( xDesktop->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
        {
            
            return xDesktop->terminate();
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
        
        Reference< XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        Reference< XElementAccess > xList( xDesktop->getFrames(), UNO_QUERY );

        if ( !xList->hasElements() )
        {
            
            xDesktop->terminate();
        }
    }
}

}








/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
