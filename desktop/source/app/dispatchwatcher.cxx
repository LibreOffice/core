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

#include <sal/config.h>

#include <sal/log.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/fstathelper.hxx>

#include <app.hxx>
#include "dispatchwatcher.hxx"
#include "officeipcthread.hxx"
#include <rtl/ustring.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/synchronousdispatch.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

#include <osl/thread.hxx>
#include <osl/file.hxx>
#include <iostream>
#include <string_view>
#include <utility>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;

namespace document = ::com::sun::star::document;

namespace desktop
{

namespace {

struct DispatchHolder
{
    DispatchHolder( URL _aURL, Reference< XDispatch > const & rDispatch ) :
        aURL(std::move( _aURL )), xDispatch( rDispatch ) {}

    URL aURL;
    Reference< XDispatch > xDispatch;
};

std::shared_ptr<const SfxFilter> impl_lookupExportFilterForUrl( std::u16string_view rUrl, std::u16string_view rFactory )
{
    // create the list of filters
    OUString sQuery = "getSortedFilterList()"
        ":module=" +
        OUString::Concat(rFactory) + // use long name here !
        ":iflags=" +
        OUString::number(static_cast<sal_Int32>(SfxFilterFlags::EXPORT)) +
        ":eflags=" +
        OUString::number(static_cast<int>(SFX_FILTER_NOTINSTALLED));

    const Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
    const Reference< XContainerQuery > xFilterFactory(
            xContext->getServiceManager()->createInstanceWithContext( u"com.sun.star.document.FilterFactory"_ustr, xContext ),
            UNO_QUERY_THROW );

    std::shared_ptr<const SfxFilter> pBestMatch;

    const Reference< XEnumeration > xFilterEnum(
            xFilterFactory->createSubSetEnumerationByQuery( sQuery ), UNO_SET_THROW );
    while ( xFilterEnum->hasMoreElements() )
    {
        comphelper::SequenceAsHashMap aFilterProps( xFilterEnum->nextElement() );
        const OUString aName( aFilterProps.getUnpackedValueOrDefault( u"Name"_ustr, OUString() ) );
        if ( !aName.isEmpty() )
        {
            std::shared_ptr<const SfxFilter> pFilter( SfxFilter::GetFilterByName( aName ) );
            if ( pFilter && pFilter->CanExport() && pFilter->GetWildcard().Matches( rUrl ) )
            {
                if ( !pBestMatch || ( SfxFilterFlags::PREFERED & pFilter->GetFilterFlags() ) )
                    pBestMatch = pFilter;
            }
        }
    }

    return pBestMatch;
}

std::shared_ptr<const SfxFilter> impl_getExportFilterFromUrl(
        const OUString& rUrl, const OUString& rFactory)
{
    try
    {
        const Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        const Reference< document::XTypeDetection > xTypeDetector(
            xContext->getServiceManager()->createInstanceWithContext( u"com.sun.star.document.TypeDetection"_ustr, xContext ),
            UNO_QUERY_THROW );
        const OUString aTypeName( xTypeDetector->queryTypeByURL( rUrl ) );

        std::shared_ptr<const SfxFilter> pFilter( SfxFilterMatcher( rFactory ).GetFilter4EA( aTypeName, SfxFilterFlags::EXPORT ) );
        if ( !pFilter )
            pFilter = impl_lookupExportFilterForUrl( rUrl, rFactory );
        if ( !pFilter )
        {
            OUString aTempName;
            FileBase::getSystemPathFromFileURL( rUrl, aTempName );
            OString aSource = OUStringToOString ( aTempName, osl_getThreadTextEncoding() );
            std::cerr << "Error: no export filter for " << aSource << " found, aborting." << std::endl;
        }

        return pFilter;
    }
    catch ( const Exception& )
    {
        return nullptr;
    }
}

OUString impl_GuessFilter( const OUString& rUrlOut, const OUString& rDocService )
{
    OUString aOutFilter;
    std::shared_ptr<const SfxFilter> pOutFilter = impl_getExportFilterFromUrl( rUrlOut, rDocService );
    if (pOutFilter)
        aOutFilter = pOutFilter->GetFilterName();

    return aOutFilter;
}

/// dump scripts in a document to the console.
void scriptCat(const Reference< XModel >& xDoc )
{
    Reference< XEmbeddedScripts > xScriptAccess( xDoc, UNO_QUERY );
    if (!xScriptAccess)
    {
        std::cout << "No script access\n";
        return;
    }

    // ignore xScriptAccess->getDialogLibraries() for now
    Reference< css::script::XLibraryContainer2 > xLibraries(
        xScriptAccess->getBasicLibraries() );

    if ( !xLibraries.is() )
    {
        std::cout << "No script libraries\n";
        return;
    }

    const Sequence< OUString > aLibNames = xLibraries->getElementNames();
    std::cout << "Libraries: " << aLibNames.getLength() << "\n";
    for (OUString const & libName : aLibNames)
    {
        std::cout << "Library: '" << libName << "' children: ";
        Reference< XNameContainer > xContainer;
        try {
            if (!xLibraries->isLibraryLoaded( libName ))
                xLibraries->loadLibrary( libName );
            xContainer = Reference< XNameContainer >(
                xLibraries->getByName( libName ), UNO_QUERY );
        }
        catch (const css::uno::Exception &e)
        {
            std::cout << "[" << libName << "] - failed to load library: " << e.Message << "\n";
            continue;
        }
        if( !xContainer.is() )
            std::cout << "0\n";
        else
        {
            Sequence< OUString > aObjectNames = xContainer->getElementNames();

            std::cout << aObjectNames.getLength() << "\n\n";
            for ( sal_Int32 j = 0 ; j < aObjectNames.getLength() ; ++j )
            {
                const OUString &rObjectName = aObjectNames[j];

                try
                {
                    Any aCode = xContainer->getByName( rObjectName );
                    OUString aCodeString;

                    if (! (aCode >>= aCodeString ) )
                        std::cout << "[" << rObjectName << "] - error fetching code\n";
                    else
                        std::cout << "[" << rObjectName << "]\n"
                                  << aCodeString.trim()
                                  << "\n[/" << rObjectName << "]\n";
                }
                catch (const css::uno::Exception &e)
                {
                    std::cout << "[" << rObjectName << "] - exception " << e.Message << " fetching code\n";
                }

                if (j < aObjectNames.getLength() - 1)
                    std::cout << "\n----------------------------------------------------------\n";
                std::cout << "\n";
            }
        }
    }
}

// Perform batch print
void batchPrint( std::u16string_view rPrinterName, const Reference< XPrintable > &xDoc,
                 const INetURLObject &aObj, const OUString &aName )
{
    OUString aFilterOut;
    OUString aPrinterName;
    size_t nPathIndex =  rPrinterName.rfind( ';' );
    if( nPathIndex != std::u16string_view::npos )
        aFilterOut=rPrinterName.substr( nPathIndex+1 );
    if( nPathIndex != 0 )
        aPrinterName=rPrinterName.substr( 0, nPathIndex );

    INetURLObject aOutFilename( aObj );
    aOutFilename.SetExtension( u"pdf" );
    FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
    OUString aOutFile = aFilterOut + "/" + aOutFilename.getName();

    OUString aTempName;
    FileBase::getSystemPathFromFileURL( aName, aTempName );
    OString aSource8 = OUStringToOString ( aTempName, osl_getThreadTextEncoding() );
    FileBase::getSystemPathFromFileURL( aOutFile, aTempName );
    OString aTargetURL8 = OUStringToOString(aTempName, osl_getThreadTextEncoding() );

    std::cout << "print " << aSource8 << " -> " << aTargetURL8;
    std::cout << " using " << (aPrinterName.isEmpty() ? "<default_printer>"_ostr : OUStringToOString( aPrinterName, osl_getThreadTextEncoding() ));
    std::cout << std::endl;

    // create the custom printer, if given
    Sequence < PropertyValue > aPrinterArgs;
    if( !aPrinterName.isEmpty() )
    {
        aPrinterArgs = { comphelper::makePropertyValue(u"Name"_ustr, aPrinterName) };
        xDoc->setPrinter( aPrinterArgs );
    }

    // print ( also without user interaction )
    aPrinterArgs = { comphelper::makePropertyValue(u"FileName"_ustr, aOutFile),
                     comphelper::makePropertyValue(u"Wait"_ustr, true) };
    xDoc->print( aPrinterArgs );
}

// Get xDoc module name
OUString getName(const Reference< XInterface > & xDoc)
{
    Reference< XModel > xModel( xDoc, UNO_QUERY );
    if (!xModel)
        return OUString();
    utl::MediaDescriptor aMediaDesc( xModel->getArgs() );
    OUString aDocService = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTSERVICE, OUString() );
    if (aDocService == "com.sun.star.text.TextDocument")
        return u"Writer"_ustr;
    else if (aDocService == "com.sun.star.text.GlobalDocument")
        return u"Writer master"_ustr;
    else if (aDocService == "com.sun.star.text.WebDocument")
        return u"Writer/Web"_ustr;
    else if (aDocService == "com.sun.star.drawing.DrawingDocument")
        return u"Draw"_ustr;
    else if (aDocService == "com.sun.star.presentation.PresentationDocument")
        return u"Impress"_ustr;
    else if (aDocService == "com.sun.star.sheet.SpreadsheetDocument")
        return u"Calc"_ustr;
    else if (aDocService == "com.sun.star.script.BasicIDE")
        return u"Basic"_ustr;
    else if (aDocService == "com.sun.star.formula.FormulaProperties")
        return u"Math"_ustr;
    else if (aDocService == "com.sun.star.sdb.RelationDesign")
        return u"Relation Design"_ustr;
    else if (aDocService == "com.sun.star.sdb.QueryDesign")
        return u"Query Design"_ustr;
    else if (aDocService == "com.sun.star.sdb.TableDesign")
        return u"Table Design"_ustr;
    else if (aDocService == "com.sun.star.sdb.DataSourceBrowser")
        return u"Data Source Browser"_ustr;
    else if (aDocService == "com.sun.star.sdb.DatabaseDocument")
        return u"Database"_ustr;

    return OUString();
}

} // anonymous namespace

DispatchWatcher::DispatchWatcher()
    : m_nRequestCount(0)
{
}


DispatchWatcher::~DispatchWatcher()
{
}


bool DispatchWatcher::executeDispatchRequests( const std::vector<DispatchRequest>& aDispatchRequestsList, bool bNoTerminate )
{
    Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );

    std::vector< DispatchHolder >   aDispatches;
    bool                     bSetInputFilter = false;
    OUString                 aForcedInputFilter;

    for (auto const & aDispatchRequest: aDispatchRequestsList)
    {
        // Set Input Filter
        if ( aDispatchRequest.aRequestType == REQUEST_INFILTER )
        {
            bSetInputFilter = true;
            aForcedInputFilter = aDispatchRequest.aURL;
            RequestHandler::RequestsCompleted();
            continue;
        }

        // create parameter array
        std::vector<PropertyValue> aArgs;

        // mark request as user interaction from outside
        aArgs.emplace_back("Referer", 0, Any(u"private:OpenEvent"_ustr),
                           PropertyState_DIRECT_VALUE);

        OUString aTarget(u"_default"_ustr);

        if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
             aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
             aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
             aDispatchRequest.aRequestType == REQUEST_CONVERSION ||
             aDispatchRequest.aRequestType == REQUEST_CAT ||
             aDispatchRequest.aRequestType == REQUEST_SCRIPT_CAT)
        {
            // documents opened for printing are opened readonly because they must be opened as a
            // new document and this document could be open already
            aArgs.emplace_back("ReadOnly", 0, Any(true), PropertyState_DIRECT_VALUE);
            // always open a new document for printing, because it must be disposed afterwards
            aArgs.emplace_back("OpenNewView", 0, Any(true), PropertyState_DIRECT_VALUE);
            // printing is done in a hidden view
            aArgs.emplace_back("Hidden", 0, Any(true), PropertyState_DIRECT_VALUE);
            // load document for printing without user interaction
            aArgs.emplace_back("Silent", 0, Any(true), PropertyState_DIRECT_VALUE);

            // hidden documents should never be put into open tasks
            aTarget = "_blank";
        }
        else
        {
            Reference < XInteractionHandler2 > xInteraction(
                InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), nullptr) );

            aArgs.emplace_back("InteractionHandler", 0, Any(xInteraction),
                               PropertyState_DIRECT_VALUE);

            aArgs.emplace_back("MacroExecutionMode", 0,
                               Any(css::document::MacroExecMode::USE_CONFIG),
                               PropertyState_DIRECT_VALUE);

            aArgs.emplace_back("UpdateDocMode", 0,
                               Any(css::document::UpdateDocMode::ACCORDING_TO_CONFIG),
                               PropertyState_DIRECT_VALUE);
        }

        if ( !aDispatchRequest.aPreselectedFactory.isEmpty() )
        {
            aArgs.emplace_back(utl::MediaDescriptor::PROP_DOCUMENTSERVICE, 0,
                               Any(aDispatchRequest.aPreselectedFactory),
                               PropertyState_DIRECT_VALUE);
        }

        OUString aName( GetURL_Impl( aDispatchRequest.aURL, aDispatchRequest.aCwdUrl ) );

        // load the document ... if they are loadable!
        // Otherwise try to dispatch it ...
        Reference < XPrintable > xDoc;
        if(
            ( aName.startsWith( ".uno" ) )  ||
            ( aName.startsWith( "slot:" ) )  ||
            ( aName.startsWith( "macro:" ) )  ||
            ( aName.startsWith("vnd.sun.star.script") )
          )
        {
            // Attention: URL must be parsed full. Otherwise some detections on it will fail!
            // It doesn't matter, if parser isn't available. Because; We try loading of URL then ...
            URL             aURL ;
            aURL.Complete = aName;

            Reference < XDispatch >         xDispatcher ;
            Reference < XURLTransformer >   xParser     ( URLTransformer::create(::comphelper::getProcessComponentContext()) );

            if( xParser.is() )
                xParser->parseStrict( aURL );

            xDispatcher = xDesktop->queryDispatch( aURL, OUString(), 0 );
            SAL_WARN_IF(
                !xDispatcher.is(), "desktop.app",
                "unsupported dispatch request <" << aName << ">");
            if( xDispatcher.is() )
            {
                // Remember request so we can find it in statusChanged!
                m_nRequestCount++;

                // Use local vector to store dispatcher because we have to fill our request container before
                // we can dispatch. Otherwise it would be possible that statusChanged is called before we dispatched all requests!!
                aDispatches.emplace_back( aURL, xDispatcher );
            }
        }
        else if ( aName.startsWith( "service:" ) )
        {
            // TODO: the dispatch has to be done for loadComponentFromURL as well.
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
                    // We have to be listener to catch errors during dispatching URLs.
                    // Otherwise it would be possible to have an office running without an open
                    // window!!
                    Sequence < PropertyValue > aArgs2{ comphelper::makePropertyValue(u"SynchronMode"_ustr,
                                                                                     true) };
                    Reference < XNotifyingDispatch > xDisp( xDispatcher, UNO_QUERY );
                    if ( xDisp.is() )
                        xDisp->dispatchWithNotification( aURL, aArgs2, this );
                    else
                        xDispatcher->dispatch( aURL, aArgs2 );
                }
                catch (const css::uno::Exception&)
                {
                    TOOLS_WARN_EXCEPTION(
                        "desktop.app",
                        "Desktop::OpenDefault() ignoring Exception while calling XNotifyingDispatch");
                }
            }
        }
        else
        {
            INetURLObject aObj( aName );
            if ( aObj.GetProtocol() == INetProtocol::PrivSoffice )
                aTarget = "_default";

            // Set "AsTemplate" argument according to request type
            if ( aDispatchRequest.aRequestType == REQUEST_FORCENEW ||
                 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN     )
            {
                aArgs.emplace_back("AsTemplate", 0,
                                   Any(aDispatchRequest.aRequestType == REQUEST_FORCENEW),
                                   PropertyState_DIRECT_VALUE);
            }

            // if we are called in viewmode, open document read-only
            if(aDispatchRequest.aRequestType == REQUEST_VIEW) {
                aArgs.emplace_back("ReadOnly", 0, Any(true), PropertyState_DIRECT_VALUE);
            }

            // if we are called with --show set Start in mediadescriptor
            if(aDispatchRequest.aRequestType == REQUEST_START) {
                const sal_Int32 nStartingSlide = aDispatchRequest.aParam.toInt32();
                const sal_uInt16 nSlide = nStartingSlide > 0 ? nStartingSlide : 1;
                aArgs.emplace_back("StartPresentation", 0, Any(nSlide), PropertyState_DIRECT_VALUE);
            }

            // Force input filter, if possible
            if( bSetInputFilter )
            {
                sal_Int32 nFilterOptionsIndex = 0;
                aArgs.emplace_back("FilterName", 0,
                                   Any(aForcedInputFilter.getToken(0, ':', nFilterOptionsIndex)),
                                   PropertyState_DIRECT_VALUE);

                if (0 < nFilterOptionsIndex)
                {
                    aArgs.emplace_back("FilterOptions", 0,
                                       Any(aForcedInputFilter.copy(nFilterOptionsIndex)),
                                       PropertyState_DIRECT_VALUE);
                }
            }

            // This is a synchron loading of a component so we don't have to deal with our statusChanged listener mechanism.
            try
            {
                xDoc.set(comphelper::SynchronousDispatch::dispatch(
                             xDesktop, aName, aTarget, comphelper::containerToSequence(aArgs)),
                         UNO_QUERY);
            }
            catch (const css::lang::IllegalArgumentException&)
            {
                TOOLS_WARN_EXCEPTION(
                    "desktop.app",
                    "Dispatchwatcher IllegalArgumentException while calling loadComponentFromURL");
            }
            catch (const css::io::IOException&)
            {
                TOOLS_WARN_EXCEPTION(
                    "desktop.app",
                    "Dispatchwatcher IOException while calling loadComponentFromURL");
            }
            if ( aDispatchRequest.aRequestType == REQUEST_OPEN ||
                 aDispatchRequest.aRequestType == REQUEST_VIEW ||
                 aDispatchRequest.aRequestType == REQUEST_START ||
                 aDispatchRequest.aRequestType == REQUEST_FORCEOPEN ||
                 aDispatchRequest.aRequestType == REQUEST_FORCENEW      )
            {
                // request is completed
                RequestHandler::RequestsCompleted();
            }
            else if ( aDispatchRequest.aRequestType == REQUEST_PRINT ||
                      aDispatchRequest.aRequestType == REQUEST_PRINTTO ||
                      aDispatchRequest.aRequestType == REQUEST_BATCHPRINT ||
                      aDispatchRequest.aRequestType == REQUEST_CONVERSION ||
                      aDispatchRequest.aRequestType == REQUEST_CAT ||
                      aDispatchRequest.aRequestType == REQUEST_SCRIPT_CAT )
            {
                if ( xDoc.is() )
                {
                    // Do we need to save the document in a different format?
                    if ( aDispatchRequest.aRequestType == REQUEST_CONVERSION ||
                         aDispatchRequest.aRequestType == REQUEST_CAT )
                    {
// FIXME: factor out into a method ...
                        Reference< XStorable > xStorable( xDoc, UNO_QUERY );
                        if ( xStorable.is() ) {
                            const OUString& aParam = aDispatchRequest.aParam;
                            sal_Int32 nPathIndex =  aParam.lastIndexOf( ';' );
                            sal_Int32 nFilterIndex = aParam.indexOf( ':' );
                            sal_Int32 nImgFilterIndex = aParam.lastIndexOf( '|' );
                            if( nPathIndex < nFilterIndex )
                                nFilterIndex = -1;

                            OUString aFilterOut;
                            OUString aImgOut;
                            OUString aFilter;
                            OUString aFilterExt;
                            bool bGuess = false;

                            if( nFilterIndex >= 0 )
                            {
                                aFilter = aParam.copy( nFilterIndex+1, nPathIndex-nFilterIndex-1 );
                                aFilterExt = aParam.copy( 0, nFilterIndex );
                            }
                            else
                            {
                                // Guess
                                bGuess = true;
                                aFilterExt = aParam.copy( 0, nPathIndex );
                            }

                            if( nImgFilterIndex >= 0 )
                            {
                                aImgOut = aParam.copy( nImgFilterIndex+1 );
                                aFilterOut = aParam.copy( nPathIndex+1, nImgFilterIndex-nPathIndex-1 );
                            }
                            else
                                aFilterOut = aParam.copy( nPathIndex+1 );

                            FileBase::getFileURLFromSystemPath( aFilterOut, aFilterOut );
                            INetURLObject aOutFilename(aFilterOut);
                            aOutFilename.Append(aObj.getName(INetURLObject::LAST_SEGMENT, true,
                                                             INetURLObject::DecodeMechanism::NONE));
                            aOutFilename.SetExtension(aFilterExt);
                            OUString aOutFile
                                = aOutFilename.GetMainURL(INetURLObject::DecodeMechanism::NONE);

                            std::unique_ptr<utl::TempFileNamed> fileForCat;
                            if( aDispatchRequest.aRequestType == REQUEST_CAT )
                            {
                                fileForCat = std::make_unique<utl::TempFileNamed>();
                                if (fileForCat->IsValid())
                                    fileForCat->EnableKillingFile();
                                else
                                    std::cerr << "Error: Cannot create temporary file..." << std::endl ;
                                aOutFile = fileForCat->GetURL();
                            }

                            if ( bGuess )
                            {
                                OUString aDocService;
                                Reference< XModel > xModel( xDoc, UNO_QUERY );
                                if ( xModel.is() )
                                {
                                    utl::MediaDescriptor aMediaDesc( xModel->getArgs() );
                                    aDocService = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTSERVICE, OUString() );
                                }
                                aFilter = impl_GuessFilter( aOutFile, aDocService );
                            }

                            bool bMultiFileTarget = false;

                            if (aFilter.isEmpty())
                            {
                                std::cerr << "Error: no export filter" << std::endl;
                            }
                            else
                            {
                                sal_Int32 nFilterOptionsIndex = aFilter.indexOf(':');
                                sal_Int32 nProps = ( 0 < nFilterOptionsIndex ) ? 4 : 3;

                                if ( !aImgOut.isEmpty() )
                                    nProps +=1;
                                Sequence<PropertyValue> conversionProperties( nProps );
                                auto pconversionProperties = conversionProperties.getArray();
                                pconversionProperties[0].Name = "ConversionRequestOrigin";
                                pconversionProperties[0].Value <<= u"CommandLine"_ustr;
                                pconversionProperties[1].Name = "Overwrite";
                                pconversionProperties[1].Value <<= true;

                                pconversionProperties[2].Name = "FilterName";
                                if( 0 < nFilterOptionsIndex )
                                {
                                    OUString sFilterName = aFilter.copy(0, nFilterOptionsIndex);
                                    OUString sFilterOptions = aFilter.copy(nFilterOptionsIndex + 1);

                                    if (sFilterName == "Text - txt - csv (StarCalc)")
                                    {
                                        sal_Int32 nIdx(0);
                                        // If the 11th token is '-1' then we export a file
                                        // per sheet where the file name is based on the suggested
                                        // output filename concatenated with the sheet name, so adjust
                                        // the output and overwrite messages
                                        // If the 11th token is not present or numeric 0 then the
                                        // default sheet is exported with the output filename. If it
                                        // is numeric >0 then that sheet (1-based) with the output
                                        // filename concatenated with the sheet name. So even if
                                        // that is a single file, the multi file target mechanism is
                                        // used.
                                        const OUString aTok(sFilterOptions.getToken(11, ',', nIdx));
                                        // Actual validity is checked in Calc, here just check for
                                        // presence of numeric value at start.
                                        bMultiFileTarget = (!aTok.isEmpty() && aTok.toInt32() != 0);
                                    }

                                    pconversionProperties[2].Value <<= sFilterName;

                                    pconversionProperties[3].Name = "FilterOptions";
                                    pconversionProperties[3].Value <<= sFilterOptions;
                                }
                                else
                                {
                                    pconversionProperties[2].Value <<= aFilter;
                                }

                                if ( !aImgOut.isEmpty() )
                                {
                                    assert(conversionProperties[nProps-1].Name.isEmpty());
                                    pconversionProperties[nProps-1].Name = "ImageFilter";
                                    pconversionProperties[nProps-1].Value <<= aImgOut;
                                }

                                OUString aTempName;
                                FileBase::getSystemPathFromFileURL(aName, aTempName);
                                OString aSource8 = OUStringToOString(aTempName, osl_getThreadTextEncoding());
                                FileBase::getSystemPathFromFileURL(aOutFile, aTempName);
                                OString aTargetURL8 = OUStringToOString(aTempName, osl_getThreadTextEncoding());
                                if (aDispatchRequest.aRequestType != REQUEST_CAT)
                                {
                                    OUString name=getName(xDoc);
                                    std::cout << "convert " << aSource8;
                                    if (!name.isEmpty())
                                        std::cout << " as a " << name <<" document";
                                    if (!bMultiFileTarget)
                                        std::cout << " -> " << aTargetURL8;
                                    std::cout << " using filter : " << OUStringToOString(aFilter, osl_getThreadTextEncoding()) << std::endl;
                                    if (!bMultiFileTarget && FStatHelper::IsDocument(aOutFile))
                                        std::cout << "Overwriting: " << OUStringToOString(aTempName, osl_getThreadTextEncoding()) << std::endl ;
                                }
                                try
                                {
                                    xStorable->storeToURL(aOutFile, conversionProperties);
                                }
                                catch (const Exception& rException)
                                {
                                    std::cerr << "Error: Please verify input parameters...";
                                    if (!rException.Message.isEmpty())
                                        std::cerr << " (" << rException.Message << ")";
                                    std::cerr << std::endl;
                                }

                                if (fileForCat && fileForCat->IsValid())
                                {
                                    SvStream* aStream = fileForCat->GetStream(StreamMode::STD_READ);
                                    while (aStream->good())
                                    {
                                        OString aStr;
                                        aStream->ReadLine(aStr, SAL_MAX_INT32);
                                        for (sal_Int32 i = 0; i < aStr.getLength(); ++i)
                                        {
                                            std::cout << aStr[i];
                                        }
                                        std::cout << std::endl;
                                    }
                                }
                            }
                        }
                    }
                    else if ( aDispatchRequest.aRequestType == REQUEST_SCRIPT_CAT )
                    {
                        Reference< XModel > xModel( xDoc, UNO_QUERY );
                        if( xModel.is() )
                            scriptCat( xModel );
                    }
                    else if ( aDispatchRequest.aRequestType == REQUEST_BATCHPRINT )
                    {
                        batchPrint(aDispatchRequest.aParam, xDoc, aObj, aName);
                    }
                    else
                    {
                        if ( aDispatchRequest.aRequestType == REQUEST_PRINTTO )
                        {
                            // create the printer
                            Sequence < PropertyValue > aPrinterArgs{ comphelper::makePropertyValue(
                                u"Name"_ustr, aDispatchRequest.aParam) };
                            xDoc->setPrinter( aPrinterArgs );
                        }

                        // print ( also without user interaction )
                        Sequence < PropertyValue > aPrinterArgs{ comphelper::makePropertyValue(u"Wait"_ustr,
                                                                                               true) };
                        xDoc->print( aPrinterArgs );
                    }
                }
                else
                {
                    std::cerr << "Error: source file could not be loaded" << std::endl;
                }

                // remove the document
                try
                {
                    Reference < XCloseable > xClose( xDoc, UNO_QUERY );
                    if ( xClose.is() )
                        xClose->close( true );
                    else
                    {
                        Reference < XComponent > xComp( xDoc, UNO_QUERY );
                        if ( xComp.is() )
                            xComp->dispose();
                    }
                }
                catch (const css::util::CloseVetoException&)
                {
                }

                // request is completed
                RequestHandler::RequestsCompleted();
            }
        }
    }

    if ( !aDispatches.empty() )
    {
        // Execute all asynchronous dispatches now after we placed them into our request container!
        Sequence < PropertyValue > aArgs{
            comphelper::makePropertyValue(u"Referer"_ustr, u"private:OpenEvent"_ustr),
            comphelper::makePropertyValue(u"SynchronMode"_ustr, true)
        };

        for (const DispatchHolder & aDispatche : aDispatches)
        {
            Reference< XDispatch > xDispatch = aDispatche.xDispatch;
            Reference < XNotifyingDispatch > xDisp( xDispatch, UNO_QUERY );
            if ( xDisp.is() )
                xDisp->dispatchWithNotification( aDispatche.aURL, aArgs, this );
            else
            {
                m_nRequestCount--;
                xDispatch->dispatch( aDispatche.aURL, aArgs );
            }
        }
    }

    bool bEmpty = (m_nRequestCount == 0);

    // No more asynchronous requests?
    // The requests are removed from the request container after they called back to this
    // implementation via statusChanged!!
    if ( bEmpty && !bNoTerminate /*m_aRequestContainer.empty()*/ )
    {
        // We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XElementAccess > xList = xDesktop->getFrames();

        if ( !xList->hasElements() )
        {
            // We don't have any task open so we have to shutdown ourself!!
            return xDesktop->terminate();
        }
    }

    return false;
}


void SAL_CALL DispatchWatcher::disposing( const css::lang::EventObject& )
{
}


void SAL_CALL DispatchWatcher::dispatchFinished( const DispatchResultEvent& )
{
    int nCount = --m_nRequestCount;
    RequestHandler::RequestsCompleted();
    if ( !nCount && !RequestHandler::AreRequestsPending() )
    {
        // We have to check if we have an open task otherwise we have to shutdown the office.
        Reference< XDesktop2 > xDesktop = css::frame::Desktop::create( ::comphelper::getProcessComponentContext() );
        Reference< XElementAccess > xList = xDesktop->getFrames();

        if ( !xList->hasElements() )
        {
            // We don't have any task open so we have to shutdown ourself!!
            xDesktop->terminate();
        }
    }
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
