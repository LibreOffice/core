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

#include <filter/msfilter/msvbahelper.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>

#include <com/sun/star/awt/KeyModifier.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <map>

using namespace ::com::sun::star;

namespace ooo {
namespace vba {

const char sUrlPart0[] = "vnd.sun.star.script:";
const char sUrlPart1[] = "?language=Basic&location=document";

String makeMacroURL( const String& sMacroName )
{
    return rtl::OUStringBuffer().
        appendAscii(RTL_CONSTASCII_STRINGPARAM(sUrlPart0)).
        append(sMacroName).
        appendAscii(RTL_CONSTASCII_STRINGPARAM(sUrlPart1)).
        makeStringAndClear();
}

::rtl::OUString extractMacroName( const ::rtl::OUString& rMacroUrl )
{
    if( (rMacroUrl.getLength() > RTL_CONSTASCII_LENGTH(sUrlPart0) + RTL_CONSTASCII_LENGTH(sUrlPart1)) &&
        rMacroUrl.matchAsciiL( RTL_CONSTASCII_STRINGPARAM(sUrlPart0) ) &&
        rMacroUrl.matchAsciiL( RTL_CONSTASCII_STRINGPARAM(sUrlPart1), rMacroUrl.getLength() - RTL_CONSTASCII_LENGTH(sUrlPart1) ) )
    {
        return rMacroUrl.copy( RTL_CONSTASCII_LENGTH(sUrlPart0),
            rMacroUrl.getLength() - RTL_CONSTASCII_LENGTH(sUrlPart0) - RTL_CONSTASCII_LENGTH(sUrlPart1) );
    }
    return ::rtl::OUString();
}

::rtl::OUString trimMacroName( const ::rtl::OUString& rMacroName )
{
    // the name may contain whitespaces and may be enclosed in apostrophs
    ::rtl::OUString aMacroName = rMacroName.trim();
    sal_Int32 nMacroLen = aMacroName.getLength();
    if( (nMacroLen >= 2) && (aMacroName[ 0 ] == '\'') && (aMacroName[ nMacroLen - 1 ] == '\'') )
        aMacroName = aMacroName.copy( 1, nMacroLen - 2 ).trim();
    return aMacroName;
}

SfxObjectShell* findShellForUrl( const rtl::OUString& sMacroURLOrPath )
{
    SfxObjectShell* pFoundShell=NULL;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    INetURLObject aObj;
    aObj.SetURL( sMacroURLOrPath );
    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
    rtl::OUString aURL;
    if ( bIsURL )
        aURL = sMacroURLOrPath;
    else
    {
        osl::FileBase::getFileURLFromSystemPath( sMacroURLOrPath, aURL );
        aObj.SetURL( aURL );
    }
    OSL_TRACE("Trying to find shell for url %s", rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() );
    while ( pShell )
    {

        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        // are we searching for a template? if so we have to cater for the
        // fact that in openoffice a document opened from a template is always
        // a new document :/
        if ( xModel.is() )
        {
            OSL_TRACE("shell 0x%x has model with url %s and we look for %s", pShell
                , rtl::OUStringToOString( xModel->getURL(), RTL_TEXTENCODING_UTF8 ).getStr()
                , rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr()
            );
            ::rtl::OUString aName = xModel->getURL() ;
            if (aName.isEmpty())
                {

                    const static rtl::OUString sTitle( RTL_CONSTASCII_USTRINGPARAM("Title" ) );
                    uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );
                    xProps->getPropertyValue(sTitle) >>= aName;
                    sal_Int32 pos = 0;
                    aName = aName.getToken(0,'-',pos);
                    aName = aName.trim();
                    if( sMacroURLOrPath.lastIndexOf( aName ) >= 0 )
                    {
                        pFoundShell = pShell;
                        break;
                    }
                }

            if ( sMacroURLOrPath.endsWithIgnoreAsciiCaseAsciiL( ".dot", 4 ) )
            {
                uno::Reference< document::XDocumentInfoSupplier > xDocInfoSupp( xModel, uno::UNO_QUERY );
                if( xDocInfoSupp.is() )
                {
                    uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSupp( xDocInfoSupp->getDocumentInfo(), uno::UNO_QUERY_THROW );
                    uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );
                    rtl::OUString sCurrName = xDocProps->getTemplateName();
                    if( sMacroURLOrPath.lastIndexOf( sCurrName ) >= 0 )
                    {
                        pFoundShell = pShell;
                        break;
                    }
                }
            }
            else
            {
                // sometimes just the name of the document ( without the path
                // is used
                bool bDocNameNoPathMatch = false;
                if ( !aURL.isEmpty() && aURL.indexOf( '/' ) == -1 )
                {
                    sal_Int32 lastSlashIndex = xModel->getURL().lastIndexOf( '/' );
                    if ( lastSlashIndex > -1 )
                    {
                        bDocNameNoPathMatch = xModel->getURL().copy( lastSlashIndex + 1 ).equals( aURL );
                        if ( !bDocNameNoPathMatch )
                        {
                            rtl::OUString aTmpName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "'" )) + xModel->getURL().copy( lastSlashIndex + 1 ) + rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "'" ));
                            bDocNameNoPathMatch = aTmpName.equals( aURL );
                        }
                    }
                }

                if ( aURL.equals( xModel->getURL() ) || bDocNameNoPathMatch )
                {
                    pFoundShell = pShell;
                    break;
                }
            }
        }
        pShell = SfxObjectShell::GetNext( *pShell );
    }
    return pFoundShell;
}

// sMod can be empty ( but we really need the library to search in )
// if sMod is empty and a macro is found then sMod is updated
// if sMod is empty, only standard modules will be searched (no class, document, form modules)
bool hasMacro( SfxObjectShell* pShell, const String& sLibrary, String& sMod, const String& sMacro )
{
    bool bFound = false;

#ifdef DISABLE_SCRIPTING
    (void) pShell;
    (void) sLibrary;
    (void) sMod;
    (void) sMacro;
#else
    if ( sLibrary.Len() && sMacro.Len() )
    {
        OSL_TRACE("** Searching for %s.%s in library %s"
            ,rtl::OUStringToOString( sMod, RTL_TEXTENCODING_UTF8 ).getStr()
            ,rtl::OUStringToOString( sMacro, RTL_TEXTENCODING_UTF8 ).getStr()
            ,rtl::OUStringToOString( sLibrary, RTL_TEXTENCODING_UTF8 ).getStr() );
        BasicManager* pBasicMgr = pShell-> GetBasicManager();
        if ( pBasicMgr )
        {
            StarBASIC* pBasic = pBasicMgr->GetLib( sLibrary );
            if ( !pBasic )
            {
                sal_uInt16 nId = pBasicMgr->GetLibId( sLibrary );
                pBasicMgr->LoadLib( nId );
                pBasic = pBasicMgr->GetLib( sLibrary );
            }
            if ( pBasic )
            {
                if ( sMod.Len() ) // we wish to find the macro is a specific module
                {
                    SbModule* pModule = pBasic->FindModule( sMod );
                    if ( pModule )
                    {
                        SbxArray* pMethods = pModule->GetMethods();
                        if ( pMethods )
                        {
                            SbMethod* pMethod = static_cast< SbMethod* >( pMethods->Find( sMacro, SbxCLASS_METHOD ) );
                            if ( pMethod )
                              bFound = true;
                        }
                    }
                }
                else if( SbMethod* pMethod = dynamic_cast< SbMethod* >( pBasic->Find( sMacro, SbxCLASS_METHOD ) ) )
                {
                    if( SbModule* pModule = pMethod->GetModule() )
                    {
                        // when searching for a macro without module name, do not search in class/document/form modules
                        if( pModule->GetModuleType() == script::ModuleType::NORMAL )
                        {
                            sMod = pModule->GetName();
                            bFound = true;
                        }
                    }
                }
            }
        }
    }
#endif
    return bFound;
}

::rtl::OUString getDefaultProjectName( SfxObjectShell* pShell )
{
    ::rtl::OUString aPrjName;
    if( BasicManager* pBasicMgr = pShell ? pShell->GetBasicManager() : 0 )
    {
        aPrjName = pBasicMgr->GetName();
        if( aPrjName.isEmpty() )
            aPrjName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
    }
    return aPrjName;
}

void parseMacro( const rtl::OUString& sMacro, String& sContainer, String& sModule, String& sProcedure )
{
    sal_Int32 nMacroDot = sMacro.lastIndexOf( '.' );

    if ( nMacroDot != -1 )
    {
        sProcedure = sMacro.copy( nMacroDot + 1 );

        sal_Int32 nContainerDot = sMacro.lastIndexOf( '.',  nMacroDot - 1 );
        if ( nContainerDot != -1 )
        {
            sModule = sMacro.copy( nContainerDot + 1, nMacroDot - nContainerDot - 1 );
            sContainer = sMacro.copy( 0, nContainerDot );
        }
        else
            sModule = sMacro.copy( 0, nMacroDot );
    }
    else
       sProcedure = sMacro;
}

::rtl::OUString resolveVBAMacro( SfxObjectShell* pShell, const ::rtl::OUString& rLibName, const ::rtl::OUString& rModuleName, const ::rtl::OUString& rMacroName )
{
#ifdef DISABLE_SCRIPTING
    (void) pShell;
    (void) rLibName;
    (void) rModuleName;
    (void) rMacroName;
#else
    if( pShell )
    {
        ::rtl::OUString aLibName = rLibName.isEmpty() ?  getDefaultProjectName( pShell ) : rLibName ;
        String aModuleName = rModuleName;
        if( hasMacro( pShell, aLibName, aModuleName, rMacroName ) )
            return ::rtl::OUStringBuffer( aLibName ).append( sal_Unicode( '.' ) ).append( aModuleName ).append( sal_Unicode( '.' ) ).append( rMacroName ).makeStringAndClear();
    }
#endif
    return ::rtl::OUString();
}

MacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const rtl::OUString& MacroName, bool bSearchGlobalTemplates )
{
#ifdef DISABLE_SCRIPTING
    (void) pShell;
    (void) MacroName;
    (void) bSearchGlobalTemplates;

    return MacroResolvedInfo();
#else
    if( !pShell )
        return MacroResolvedInfo();

    // the name may be enclosed in apostrophs
    ::rtl::OUString aMacroName = trimMacroName( MacroName );

    // parse the macro name
    sal_Int32 nDocSepIndex = aMacroName.indexOf( '!' );
    if( nDocSepIndex > 0 )
    {
        // macro specified by document name
        // find document shell for document name and call ourselves
        // recursively

        // assume for now that the document name is *this* document
        String sDocUrlOrPath = aMacroName.copy( 0, nDocSepIndex );
        aMacroName = aMacroName.copy( nDocSepIndex + 1 );
        OSL_TRACE("doc search, current shell is 0x%x", pShell );
        SfxObjectShell* pFoundShell = 0;
        if( bSearchGlobalTemplates )
        {
            SvtPathOptions aPathOpt;
            String aAddinPath = aPathOpt.GetAddinPath();
            if( rtl::OUString( sDocUrlOrPath ).indexOf( aAddinPath ) == 0 )
                pFoundShell = pShell;
        }
        if( !pFoundShell )
            pFoundShell = findShellForUrl( sDocUrlOrPath );
        OSL_TRACE("doc search, after find, found shell is 0x%x", pFoundShell );
        return resolveVBAMacro( pFoundShell, aMacroName );
    }

    // macro is contained in 'this' document ( or code imported from a template
    // where that template is a global template or perhaps the template this
    // document is created from )

    MacroResolvedInfo aRes( pShell );

    // macro format = Container.Module.Procedure
    String sContainer, sModule, sProcedure;
    parseMacro( aMacroName, sContainer, sModule, sProcedure );

#if 0
    // As long as service VBAProjectNameProvider isn't supported in the model, disable the createInstance call
    // (the ServiceNotRegisteredException is wrongly caught in ScModelObj::createInstance)
    uno::Reference< container::XNameContainer > xPrjNameCache;
    uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY);
    if ( xSF.is() ) try
    {
        xPrjNameCache.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAProjectNameProvider" ) ) ), uno::UNO_QUERY );
    }
    catch( const uno::Exception& )    // createInstance may throw
    {
    }
#endif

    std::vector< rtl::OUString > sSearchList;

    if ( sContainer.Len() > 0 )
    {
// service VBAProjectNameProvider not implemented
#if 0
        // get the Project associated with the Container
        if ( xPrjNameCache.is() )
        {
            if ( xPrjNameCache->hasByName( sContainer ) )
            {
                rtl::OUString sProject;
                xPrjNameCache->getByName( sContainer ) >>= sProject;
                sContainer = sProject;
            }
        }
#endif
        sSearchList.push_back( sContainer ); // First Lib to search
    }
    else
    {
        // Ok, if we have no Container specified then we need to search them in order, this document, template this document created from, global templates,
        // get the name of Project/Library for 'this' document
            rtl::OUString sThisProject = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") );
            try
            {
                uno::Reference< beans::XPropertySet > xProps( pShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< script::vba::XVBACompatibility > xVBAMode( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BasicLibraries") ) ), uno::UNO_QUERY_THROW );
                sThisProject = xVBAMode->getProjectName();
            }
            catch( const uno::Exception& /*e*/) {}

        sSearchList.push_back( sThisProject ); // First Lib to search

// service VBAProjectNameProvider not implemented
#if 0
        if ( xPrjNameCache.is() )
        {
            // is this document created from a template?
            uno::Reference< document::XDocumentInfoSupplier > xDocInfoSupp( pShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSupp( xDocInfoSupp->getDocumentInfo(), uno::UNO_QUERY_THROW );
            uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );

            rtl::OUString sCreatedFrom = xDocProps->getTemplateURL();
            if ( !sCreatedFrom.isEmpty() )
            {
                INetURLObject aObj;
                aObj.SetURL( sCreatedFrom );
                bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
                rtl::OUString aURL;
                if ( bIsURL )
                    aURL = sCreatedFrom;
                else
                {
                    osl::FileBase::getFileURLFromSystemPath( sCreatedFrom, aURL );
                    aObj.SetURL( aURL );
                }
                sCreatedFrom =  aObj.GetLastName();
            }

            sal_Int32 nIndex =  sCreatedFrom.lastIndexOf( '.' );
            if ( nIndex != -1 )
                sCreatedFrom = sCreatedFrom.copy( 0, nIndex );

            rtl::OUString sPrj;
            if ( !sCreatedFrom.isEmpty() && xPrjNameCache->hasByName( sCreatedFrom ) )
            {
                xPrjNameCache->getByName( sCreatedFrom ) >>= sPrj;
                // Make sure we don't double up with this project
                if ( !sPrj.equals( sThisProject ) )
                    sSearchList.push_back( sPrj );
            }

            // get list of global template Names
            uno::Sequence< rtl::OUString > sTemplateNames = xPrjNameCache->getElementNames();
            sal_Int32 nLen = sTemplateNames.getLength();
            for ( sal_Int32 index = 0; ( bSearchGlobalTemplates && index < nLen ); ++index )
            {

                if ( !sCreatedFrom.equals( sTemplateNames[ index ] ) )
                {
                    if ( xPrjNameCache->hasByName( sTemplateNames[ index ] ) )
                    {
                        xPrjNameCache->getByName( sTemplateNames[ index ] ) >>= sPrj;
                        // Make sure we don't double up with this project
                        if ( !sPrj.equals( sThisProject ) )
                            sSearchList.push_back( sPrj );
                    }
                }

            }
        }
#endif
    }

    std::vector< rtl::OUString >::iterator it_end = sSearchList.end();
    for ( std::vector< rtl::OUString >::iterator it = sSearchList.begin(); !aRes.mbFound && (it != it_end); ++it )
    {
        aRes.mbFound = hasMacro( pShell, *it, sModule, sProcedure );
        if ( aRes.mbFound )
            sContainer = *it;
    }
    aRes.msResolvedMacro = sProcedure.Insert( '.', 0 ).Insert( sModule, 0).Insert( '.', 0 ).Insert( sContainer, 0 );

    return aRes;
#endif
}

// Treat the args as possible inouts ( convertion at bottom of method )
sal_Bool executeMacro( SfxObjectShell* pShell, const String& sMacroName, uno::Sequence< uno::Any >& aArgs, uno::Any& aRet, const uno::Any& /*aCaller*/)
{
#ifdef DISABLE_SCRIPTING
    (void) pShell;
    (void) sMacroName;
    (void) aArgs;
    (void) aRet;

    return sal_False;
#else
    sal_Bool bRes = sal_False;
    if ( !pShell )
        return bRes;
    rtl::OUString sUrl = makeMacroURL( sMacroName );

    uno::Sequence< sal_Int16 > aOutArgsIndex;
    uno::Sequence< uno::Any > aOutArgs;

    try
    {   ErrCode nErr( ERRCODE_BASIC_INTERNAL_ERROR );
        if ( pShell )
        {
            nErr = pShell->CallXScript( sUrl,
                               aArgs, aRet, aOutArgsIndex, aOutArgs, false );
            sal_Int32 nLen = aOutArgs.getLength();
            // convert any out params to seem like they were inouts
            if ( nLen )
            {
                for ( sal_Int32 index=0; index < nLen; ++index )
                {
                    sal_Int32 nOutIndex = aOutArgsIndex[ index ];
                    aArgs[ nOutIndex ] = aOutArgs[ index ];
                }
            }
        }
        bRes = ( nErr == ERRCODE_NONE );
    }
    catch ( const uno::Exception& )
    {
       bRes = sal_False;
    }
    return bRes;
#endif
}

// ============================================================================

uno::Sequence< ::rtl::OUString > VBAMacroResolver_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.vba.VBAMacroResolver" ) );
    return aServiceNames;
}

::rtl::OUString VBAMacroResolver_getImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.vba.VBAMacroResolver" ) );
}

uno::Reference< uno::XInterface > SAL_CALL VBAMacroResolver_createInstance( const uno::Reference< uno::XComponentContext >& ) throw (uno::Exception)
{
    return static_cast< ::cppu::OWeakObject* >( new VBAMacroResolver );
}

// ============================================================================

VBAMacroResolver::VBAMacroResolver() :
    mpObjShell( 0 )
{
}

VBAMacroResolver::~VBAMacroResolver()
{
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

::rtl::OUString SAL_CALL VBAMacroResolver::getImplementationName() throw (uno::RuntimeException)
{
    return VBAMacroResolver_getImplementationName();
}

sal_Bool SAL_CALL VBAMacroResolver::supportsService( const ::rtl::OUString& rService ) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices = VBAMacroResolver_getSupportedServiceNames();
    const ::rtl::OUString* pArray = aServices.getConstArray();
    const ::rtl::OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rService ) != pArrayEnd;
}

uno::Sequence< ::rtl::OUString > SAL_CALL VBAMacroResolver::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return VBAMacroResolver_getSupportedServiceNames();
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL VBAMacroResolver::initialize( const uno::Sequence< uno::Any >& rArgs ) throw (uno::Exception, uno::RuntimeException)
{
    OSL_ENSURE( rArgs.getLength() < 2, "VBAMacroResolver::initialize - missing arguments" );
    if( rArgs.getLength() < 2 )
        throw uno::RuntimeException();

    // first argument: document model
    mxModel.set( rArgs[ 0 ], uno::UNO_QUERY_THROW );
    uno::Reference< lang::XUnoTunnel > xUnoTunnel( mxModel, uno::UNO_QUERY_THROW );
    mpObjShell = reinterpret_cast< SfxObjectShell* >( xUnoTunnel->getSomething( SfxObjectShell::getUnoTunnelId() ) );
    if( !mpObjShell )
        throw uno::RuntimeException();

    // second argument: VBA project name
    if( !(rArgs[ 1 ] >>= maProjectName) || (maProjectName.isEmpty()) )
        throw uno::RuntimeException();
}

// com.sun.star.script.vba.XVBAMacroResolver interface ------------------------

::rtl::OUString SAL_CALL VBAMacroResolver::resolveVBAMacroToScriptURL( const ::rtl::OUString& rVBAMacroName ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if( !mpObjShell )
        throw uno::RuntimeException();

    // the name may be enclosed in apostrophs
    ::rtl::OUString aMacroName = trimMacroName( rVBAMacroName );
    if( aMacroName.isEmpty() )
        throw lang::IllegalArgumentException();

    // external references not supported here (syntax is "url!macroname" or "[url]!macroname" or "[url]macroname")
    if( (aMacroName[ 0 ] == '[') || (aMacroName.indexOf( '!' ) >= 0) )
        throw lang::IllegalArgumentException();

    // check if macro name starts with project name, replace with "Standard"
    // TODO: adjust this when custom VBA project name is supported
    sal_Int32 nDotPos = aMacroName.indexOf( '.' );
    if( (nDotPos == 0) || (nDotPos + 1 == aMacroName.getLength()) )
        throw lang::IllegalArgumentException();
    if( (nDotPos > 0) && aMacroName.matchIgnoreAsciiCase( maProjectName ) )
        aMacroName = aMacroName.copy( nDotPos + 1 );

    // try to find the macro
    MacroResolvedInfo aInfo = resolveVBAMacro( mpObjShell, aMacroName, false );
    if( !aInfo.mbFound )
        throw lang::IllegalArgumentException();

    // build and return the script URL
    return makeMacroURL( aInfo.msResolvedMacro );
}

::rtl::OUString SAL_CALL VBAMacroResolver::resolveScriptURLtoVBAMacro( const ::rtl::OUString& /*rScriptURL*/ ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    OSL_ENSURE( false, "VBAMacroResolver::resolveScriptURLtoVBAMacro - not implemented" );
    throw uno::RuntimeException();
}

bool getModifier( char c, sal_uInt16& mod )
{
    static const char modifiers[] = "+^%";
    static const sal_uInt16 KEY_MODS[] = {KEY_SHIFT, KEY_MOD1, KEY_MOD2};

    for ( unsigned int i=0; i<SAL_N_ELEMENTS(modifiers); ++i )
    {
        if ( c == modifiers[i] )
        {
            mod = mod | KEY_MODS[ i ];
            return true;
        }
    }
    return false;
}

typedef std::map< rtl::OUString, sal_uInt16 > MSKeyCodeMap;

sal_uInt16 parseChar( char c ) throw ( uno::RuntimeException )
{
    sal_uInt16 nVclKey = 0;
    // do we care about locale here for isupper etc. ? probably not
    if ( isalpha( c ) )
    {
        nVclKey |= ( toupper( c ) - 'A' ) + KEY_A;
        if ( isupper( c ) )
            nVclKey |= KEY_SHIFT;
    }
    else if ( isdigit( c ) )
        nVclKey |= ( c  - '0' ) + KEY_0;
    else if ( c == '~' ) // special case
        nVclKey = KEY_RETURN;
    else if ( c == ' ' ) // special case
        nVclKey = KEY_SPACE;
    else // I guess we have a problem ( but not sure if locale specific keys might come into play here )
        throw uno::RuntimeException();
    return nVclKey;
}

struct KeyCodeEntry
{
   const char* sName;
   sal_uInt16 nCode;
};

KeyCodeEntry aMSKeyCodesData[] = {
    { "BACKSPACE", KEY_BACKSPACE },
    { "BS", KEY_BACKSPACE },
    { "DELETE", KEY_DELETE },
    { "DEL", KEY_DELETE },
    { "DOWN", KEY_DOWN },
    { "UP", KEY_UP },
    { "LEFT", KEY_LEFT },
    { "RIGHT", KEY_RIGHT },
    { "END", KEY_END },
    { "ESCAPE", KEY_ESCAPE },
    { "ESC", KEY_ESCAPE },
    { "HELP", KEY_HELP },
    { "HOME", KEY_HOME },
    { "PGDN", KEY_PAGEDOWN },
    { "PGUP", KEY_PAGEUP },
    { "INSERT", KEY_INSERT },
    { "SCROLLLOCK", KEY_SCROLLLOCK },
    { "NUMLOCK", KEY_NUMLOCK },
    { "TAB", KEY_TAB },
    { "F1", KEY_F1 },
    { "F2", KEY_F2 },
    { "F3", KEY_F3 },
    { "F4", KEY_F4 },
    { "F5", KEY_F5 },
    { "F6", KEY_F6 },
    { "F7", KEY_F7 },
    { "F8", KEY_F8 },
    { "F9", KEY_F1 },
    { "F10", KEY_F10 },
    { "F11", KEY_F11 },
    { "F12", KEY_F12 },
    { "F13", KEY_F13 },
    { "F14", KEY_F14 },
    { "F15", KEY_F15 },
};

awt::KeyEvent parseKeyEvent( const ::rtl::OUString& Key ) throw ( uno::RuntimeException )
{
    static MSKeyCodeMap msKeyCodes;
    if ( msKeyCodes.empty() )
    {
        for ( unsigned int i = 0; i < SAL_N_ELEMENTS( aMSKeyCodesData ); ++i )
        {
            msKeyCodes[ rtl::OUString::createFromAscii( aMSKeyCodesData[ i ].sName ) ] = aMSKeyCodesData[ i ].nCode;
        }
    }
    rtl::OUString sKeyCode;
    sal_uInt16 nVclKey = 0;

    // parse the modifier if any
    for ( int i=0; i<Key.getLength(); ++i )
    {
        if ( ! getModifier( Key[ i ], nVclKey ) )
        {
            sKeyCode = Key.copy( i );
            break;
        }
    }

    // check if keycode is surrounded by '{}', if so scoop out the contents
    // else it should be just one char of ( 'a-z,A-Z,0-9' )
    if ( sKeyCode.getLength() == 1 ) // ( a single char )
    {
        char c = (char)( sKeyCode[ 0 ] );
        nVclKey |= parseChar( c );
    }
    else // key should be enclosed in '{}'
    {
        if ( sKeyCode.getLength() < 3 ||  !( sKeyCode[0] == '{' && sKeyCode[sKeyCode.getLength() - 1 ] == '}' ) )
            throw uno::RuntimeException();

        sKeyCode = sKeyCode.copy(1, sKeyCode.getLength() - 2 );

        if ( sKeyCode.getLength() == 1 )
            nVclKey |= parseChar( (char)( sKeyCode[ 0 ] ) );
        else
        {
            MSKeyCodeMap::iterator it = msKeyCodes.find( sKeyCode );
            if ( it == msKeyCodes.end() ) // unknown or unsupported
                throw uno::RuntimeException();
            nVclKey |= it->second;
        }
    }

    awt::KeyEvent aKeyEvent = svt::AcceleratorExecute::st_VCLKey2AWTKey( KeyCode( nVclKey ) );
    return aKeyEvent;
}

void applyShortCutKeyBinding ( const uno::Reference< frame::XModel >& rxModel, const awt::KeyEvent& rKeyEvent, const ::rtl::OUString& rMacroName ) throw (uno::RuntimeException)
{
    rtl::OUString MacroName( rMacroName );
    if ( !MacroName.isEmpty() )
    {
        ::rtl::OUString aMacroName = MacroName.trim();
        if (0 == aMacroName.indexOf('!'))
            MacroName = aMacroName.copy(1).trim();
        SfxObjectShell* pShell = NULL;
        if ( rxModel.is() )
        {
            uno::Reference< lang::XUnoTunnel >  xObjShellTunnel( rxModel, uno::UNO_QUERY_THROW );
            pShell = reinterpret_cast<SfxObjectShell*>( xObjShellTunnel->getSomething(SfxObjectShell::getUnoTunnelId()));
            if ( !pShell )
                throw uno::RuntimeException();
        }
        MacroResolvedInfo aMacroInfo = resolveVBAMacro( pShell, aMacroName );
        if( !aMacroInfo.mbFound )
            throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("The procedure doesn't exist") ), uno::Reference< uno::XInterface >() );
       MacroName = aMacroInfo.msResolvedMacro;
    }
    uno::Reference< ui::XUIConfigurationManagerSupplier > xCfgSupplier(rxModel, uno::UNO_QUERY_THROW);
    uno::Reference< ui::XUIConfigurationManager > xCfgMgr = xCfgSupplier->getUIConfigurationManager();

    uno::Reference< ui::XAcceleratorConfiguration > xAcc( xCfgMgr->getShortCutManager(), uno::UNO_QUERY_THROW );
    if ( MacroName.isEmpty() )
        // I believe this should really restore the [application] default. Since
        // afaik we don't actually setup application default bindings on import
        // we don't even know what the 'default' would be for this key
        xAcc->removeKeyEvent( rKeyEvent );
    else
        xAcc->setKeyEvent( rKeyEvent, ooo::vba::makeMacroURL( MacroName ) );

}
// ============================================================================

} // namespace vba
} // namespace ooo
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

