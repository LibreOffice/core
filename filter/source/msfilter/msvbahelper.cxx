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

#include <config_features.h>

#include <filter/msfilter/msvbahelper.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <unotools/pathoptions.hxx>
#include <rtl/character.hxx>
#include <sfx2/objsh.hxx>
#include <o3tl/string_view.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

using namespace ::com::sun::star;

namespace ooo::vba {

constexpr OUString sUrlPart0( u"vnd.sun.star.script:"_ustr );
constexpr OUString sUrlPart1( u"?language=Basic&location=document"_ustr );

OUString makeMacroURL( std::u16string_view sMacroName )
{
    return sUrlPart0 + sMacroName + sUrlPart1;
}

OUString extractMacroName( std::u16string_view rMacroUrl )
{
    if( o3tl::starts_with(rMacroUrl, sUrlPart0 ) && o3tl::ends_with(rMacroUrl, sUrlPart1 ) )
    {
        return OUString(rMacroUrl.substr( sUrlPart0.getLength(),
            rMacroUrl.size() - sUrlPart0.getLength() - sUrlPart1.getLength() ));
    }
    return OUString();
}

static std::u16string_view trimMacroName( std::u16string_view rMacroName )
{
    // the name may contain whitespaces and may be enclosed in apostrophs
    std::u16string_view aMacroName = o3tl::trim(rMacroName);
    size_t nMacroLen = aMacroName.size();
    if( (nMacroLen >= 2) && (aMacroName[ 0 ] == '\'') && (aMacroName[ nMacroLen - 1 ] == '\'') )
        aMacroName = o3tl::trim(aMacroName.substr( 1, nMacroLen - 2 ));
    return aMacroName;
}

#if HAVE_FEATURE_SCRIPTING

static SfxObjectShell* findShellForUrl( const OUString& sMacroURLOrPath )
{
    SfxObjectShell* pFoundShell=nullptr;
    SfxObjectShell* pShell = SfxObjectShell::GetFirst();
    INetURLObject aObj;
    aObj.SetURL( sMacroURLOrPath );
    bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
    OUString aURL;
    if ( bIsURL )
        aURL = sMacroURLOrPath;
    else
    {
        osl::FileBase::getFileURLFromSystemPath( sMacroURLOrPath, aURL );
        aObj.SetURL( aURL );
    }
    while ( pShell )
    {

        uno::Reference< frame::XModel > xModel = pShell->GetModel();
        // are we searching for a template? if so we have to cater for the
        // fact that in openoffice a document opened from a template is always
        // a new document :/
        if ( xModel.is() )
        {
            SAL_INFO(
                "filter.ms",
                "shell " << pShell << " has model with url " << xModel->getURL()
                    << " and we look for " << aURL);
            OUString aName = xModel->getURL() ;
            if (aName.isEmpty())
            {
                uno::Reference< frame::XFrame > xFrame( xModel->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
                uno::Reference< beans::XPropertySet > xProps( xFrame, uno::UNO_QUERY_THROW );
                xProps->getPropertyValue(u"Title"_ustr) >>= aName;
                aName = o3tl::trim(o3tl::getToken(aName, 0, '-'));
                if( sMacroURLOrPath.lastIndexOf( aName ) >= 0 )
                {
                    pFoundShell = pShell;
                    break;
                }
            }

            if ( sMacroURLOrPath.endsWithIgnoreAsciiCase( ".dot" ) )
            {
                uno::Reference<document::XDocumentPropertiesSupplier> const
                    xDocPropSupp(xModel, uno::UNO_QUERY);
                if (xDocPropSupp.is())
                {
                    uno::Reference< document::XDocumentProperties > const
                        xDocProps(xDocPropSupp->getDocumentProperties(),
                                    uno::UNO_SET_THROW);
                    OUString sCurrName = xDocProps->getTemplateName();
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
                        bDocNameNoPathMatch = xModel->getURL().subView( lastSlashIndex + 1 ) == aURL;
                        if ( !bDocNameNoPathMatch )
                        {
                            OUString aTmpName = OUString::Concat("'") + xModel->getURL().subView( lastSlashIndex + 1 ) + "'";
                            bDocNameNoPathMatch = aTmpName == aURL;
                        }
                    }
                }

                if ( aURL == xModel->getURL() || bDocNameNoPathMatch )
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
static bool hasMacro(SfxObjectShell const* pShell, const OUString& sLibrary, OUString& sMod,
                     const OUString& sMacro, bool bOnlyPublic, const OUString& sSkipModule)
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pShell;
    (void) sLibrary;
    (void) sMod;
    (void) sMacro;
    (void) bOnlyPublic;
    (void) sSkipModule;
#else
    if (sLibrary.isEmpty() || sMacro.isEmpty())
        return false;

    BasicManager* pBasicMgr = pShell->GetBasicManager();
    if (!pBasicMgr)
        return false;

    StarBASIC* pBasic = pBasicMgr->GetLib(sLibrary);
    if (!pBasic)
    {
        sal_uInt16 nId = pBasicMgr->GetLibId(sLibrary);
        pBasicMgr->LoadLib(nId);
        pBasic = pBasicMgr->GetLib(sLibrary);
    }
    if (!pBasic)
        return false;

    if (!sMod.isEmpty()) // we wish to find the macro is a specific module
    {
        SbModule* pModule = pBasic->FindModule(sMod);
        if (!pModule)
            return false;
        SbMethod* pMeth = pModule->FindMethod(sMacro, SbxClassType::Method);

        // Must be compiled before we can trust SbxFlagBits::Private
        if (pMeth && bOnlyPublic && !pModule->IsCompiled())
            pModule->Compile();

        return pMeth && (!bOnlyPublic || !pMeth->IsSet(SbxFlagBits::Private));
    }

    for (auto const& rModuleRef : pBasic->GetModules())
    {
        SbMethod* pMeth = rModuleRef->FindMethod(sMacro, SbxClassType::Method);
        if (pMeth)
        {
            if (rModuleRef->GetName() == sSkipModule)
                continue;

            if (bOnlyPublic)
            {
                if (!rModuleRef->IsCompiled())
                    rModuleRef->Compile();

                if (pMeth->IsSet(SbxFlagBits::Private))
                    continue;
            }
            sMod = rModuleRef->GetName();
            return true;
        }
    }
#endif
    return false;
}

#endif

#if HAVE_FEATURE_SCRIPTING

OUString getDefaultProjectName( SfxObjectShell const * pShell )
{
    OUString aPrjName;
    if( BasicManager* pBasicMgr = pShell ? pShell->GetBasicManager() : nullptr )
    {
        aPrjName = pBasicMgr->GetName();
        if( aPrjName.isEmpty() )
            aPrjName = "Standard";
    }
    return aPrjName;
}

static void parseMacro( const OUString& sMacro, OUString& sContainer, OUString& sModule, OUString& sProcedure )
{
    sal_Int32 nMacroDot = sMacro.lastIndexOf( '.' );

    if ( nMacroDot != -1 )
    {
        sProcedure = sMacro.copy( nMacroDot + 1 );

        const sal_Int32 nContainerDot = sMacro.lastIndexOf('.',  nMacroDot);
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

#endif

OUString resolveVBAMacro(SfxObjectShell const* pShell, const OUString& rLibName,
                         const OUString& rModuleName, const OUString& rMacroName,
                         bool bOnlyPublic, const OUString& sSkipModule)
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pShell;
    (void) rLibName;
    (void) rModuleName;
    (void) rMacroName;
    (void) bOnlyPublic;
    (void) sSkipModule;
#else
    if( pShell )
    {
        OUString aLibName = rLibName.isEmpty() ?  getDefaultProjectName( pShell ) : rLibName ;
        OUString aModuleName = rModuleName;
        if (hasMacro(pShell, aLibName, aModuleName, rMacroName, bOnlyPublic, sSkipModule))
            return aLibName + "." + aModuleName + "." + rMacroName;
    }
#endif
    return OUString();
}

MacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const OUString& MacroName, bool bSearchGlobalTemplates )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pShell;
    (void) MacroName;
    (void) bSearchGlobalTemplates;

    return MacroResolvedInfo();
#else
    if( !pShell )
        return MacroResolvedInfo();

    // the name may be enclosed in apostrophs
    std::u16string_view aMacroName = trimMacroName( MacroName );

    // parse the macro name
    size_t nDocSepIndex = aMacroName.find( '!' );
    if( nDocSepIndex > 0 && nDocSepIndex != std::u16string_view::npos )
    {
        // macro specified by document name
        // find document shell for document name and call ourselves
        // recursively

        // assume for now that the document name is *this* document
        std::u16string_view sDocUrlOrPath = aMacroName.substr( 0, nDocSepIndex );
        aMacroName = aMacroName.substr( nDocSepIndex + 1 );
        SAL_INFO("filter.ms", "doc search, current shell is " << pShell);
        SfxObjectShell* pFoundShell = nullptr;
        if( bSearchGlobalTemplates )
        {
            SvtPathOptions aPathOpt;
            const OUString& aAddinPath = aPathOpt.GetAddinPath();
            if( o3tl::starts_with(sDocUrlOrPath, aAddinPath) )
                pFoundShell = pShell;
        }
        if( !pFoundShell )
            pFoundShell = findShellForUrl( OUString(sDocUrlOrPath) );
        SAL_INFO(
            "filter.ms",
            "doc search, after find, found shell is " << pFoundShell);
        return resolveVBAMacro( pFoundShell, OUString(aMacroName) );
    }

    // macro is contained in 'this' document ( or code imported from a template
    // where that template is a global template or perhaps the template this
    // document is created from )

    MacroResolvedInfo aRes( pShell );

    // macro format = Container.Module.Procedure
    OUString sContainer, sModule, sProcedure;
    parseMacro( OUString(aMacroName), sContainer, sModule, sProcedure );

#if 0
    // As long as service VBAProjectNameProvider isn't supported in the model, disable the createInstance call
    // (the ServiceNotRegisteredException is wrongly caught in ScModelObj::createInstance)
    uno::Reference< container::XNameContainer > xPrjNameCache;
    uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY);
    if ( xSF.is() ) try
    {
        xPrjNameCache.set( xSF->createInstance( "ooo.vba.VBAProjectNameProvider" ), uno::UNO_QUERY );
    }
    catch( const uno::Exception& )    // createInstance may throw
    {
    }
#endif

    std::vector< OUString > sSearchList;

    if ( !sContainer.isEmpty() )
    {
// service VBAProjectNameProvider not implemented
#if 0
        // get the Project associated with the Container
        if ( xPrjNameCache.is() )
        {
            if ( xPrjNameCache->hasByName( sContainer ) )
            {
                OUString sProject;
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
            OUString sThisProject( u"Standard"_ustr );
            try
            {
                uno::Reference< beans::XPropertySet > xProps( pShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< script::vba::XVBACompatibility > xVBAMode( xProps->getPropertyValue( u"BasicLibraries"_ustr ), uno::UNO_QUERY_THROW );
                sThisProject = xVBAMode->getProjectName();
            }
            catch( const uno::Exception& /*e*/) {}

            sSearchList.push_back( sThisProject ); // First Lib to search

// service VBAProjectNameProvider not implemented
#if 0
        if ( xPrjNameCache.is() )
        {
            // is this document created from a template?
            uno::Reference< document::XDocumentPropertiesSupplier > const
                xDocPropSupp(pShell->GetModel(), uno::UNO_QUERY_THROW);
            uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );

            OUString sCreatedFrom = xDocProps->getTemplateURL();
            if ( !sCreatedFrom.isEmpty() )
            {
                INetURLObject aObj;
                aObj.SetURL( sCreatedFrom );
                bool bIsURL = aObj.GetProtocol() != INetProtocol::NotValid;
                OUString aURL;
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

            OUString sPrj;
            if ( !sCreatedFrom.isEmpty() && xPrjNameCache->hasByName( sCreatedFrom ) )
            {
                xPrjNameCache->getByName( sCreatedFrom ) >>= sPrj;
                // Make sure we don't double up with this project
                if ( !sPrj.equals( sThisProject ) )
                    sSearchList.push_back( sPrj );
            }

            // get list of global template Names
            uno::Sequence< OUString > sTemplateNames = xPrjNameCache->getElementNames();
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

    for (auto const& search : sSearchList)
    {
        aRes.mbFound = hasMacro(pShell, search, sModule, sProcedure, /*bOnlyPublic=*/false, u""_ustr);
        if ( aRes.mbFound )
        {
            sContainer = search;
            break;
        }
    }
    //aRes.msResolvedMacro = sProcedure.Insert( '.', 0 ).Insert( sModule, 0).Insert( '.', 0 ).Insert( sContainer, 0 );
    aRes.msResolvedMacro = sContainer + "." + sModule + "." + sProcedure;

    return aRes;
#endif
}

// Treat the args as possible inputs (conversion at bottom of method)
bool executeMacro( SfxObjectShell* pShell, const OUString& sMacroName, uno::Sequence< uno::Any >& aArgs, uno::Any& aRet, const uno::Any& /*aCaller*/)
{
#if !HAVE_FEATURE_SCRIPTING
    (void) pShell;
    (void) sMacroName;
    (void) aArgs;
    (void) aRet;

    return false;
#else
    bool bRes = false;
    if ( !pShell )
        return bRes;
    OUString sUrl = makeMacroURL( sMacroName );

    uno::Sequence< sal_Int16 > aOutArgsIndex;
    uno::Sequence< uno::Any > aOutArgs;

    try
    {
        ErrCode nErr = pShell->CallXScript(sUrl, aArgs, aRet, aOutArgsIndex, aOutArgs, false);
        sal_Int32 nLen = aOutArgs.getLength();
        // convert any out params to seem like they were inputs
        if (nLen)
        {
            auto pArgs = aArgs.getArray();
            for (sal_Int32 index = 0; index < nLen; ++index)
            {
                sal_Int32 nOutIndex = aOutArgsIndex[index];
                pArgs[nOutIndex] = aOutArgs[index];
            }
        }
        bRes = ( nErr == ERRCODE_NONE );
    }
    catch ( const uno::Exception& )
    {
       bRes = false;
    }
    return bRes;
#endif
}



VBAMacroResolver::VBAMacroResolver() :
    mpObjShell( nullptr )
{
}

VBAMacroResolver::~VBAMacroResolver()
{
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL VBAMacroResolver::getImplementationName()
{
    return u"com.sun.star.comp.vba.VBAMacroResolver"_ustr;
}

sal_Bool SAL_CALL VBAMacroResolver::supportsService( const OUString& rService )
{
    return cppu::supportsService(this, rService);
}

uno::Sequence< OUString > SAL_CALL VBAMacroResolver::getSupportedServiceNames()
{
    return { u"com.sun.star.script.vba.VBAMacroResolver"_ustr };
}

// com.sun.star.lang.XInitialization interface --------------------------------

void SAL_CALL VBAMacroResolver::initialize( const uno::Sequence< uno::Any >& rArgs )
{
    OSL_ENSURE( rArgs.getLength() > 1, "VBAMacroResolver::initialize - missing arguments" );
    if( rArgs.getLength() < 2 )
        throw uno::RuntimeException();

    // first argument: document model
    mxModel.set( rArgs[ 0 ], uno::UNO_QUERY_THROW );
    mpObjShell = comphelper::getFromUnoTunnel<SfxObjectShell>(mxModel);
    if( !mpObjShell )
        throw uno::RuntimeException();

    // second argument: VBA project name
    if( !(rArgs[ 1 ] >>= maProjectName) || (maProjectName.isEmpty()) )
        throw uno::RuntimeException();
}

// com.sun.star.script.vba.XVBAMacroResolver interface ------------------------

OUString SAL_CALL VBAMacroResolver::resolveVBAMacroToScriptURL( const OUString& rVBAMacroName )
{
    if( !mpObjShell )
        throw uno::RuntimeException();

    // the name may be enclosed in apostrophs
    OUString aMacroName( trimMacroName( rVBAMacroName ) );
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
    MacroResolvedInfo aInfo = resolveVBAMacro( mpObjShell, aMacroName );
    if( !aInfo.mbFound )
        throw lang::IllegalArgumentException();

    // build and return the script URL
    return makeMacroURL( aInfo.msResolvedMacro );
}

OUString SAL_CALL VBAMacroResolver::resolveScriptURLtoVBAMacro( const OUString& /*rScriptURL*/ )
{
    OSL_ENSURE( false, "VBAMacroResolver::resolveScriptURLtoVBAMacro - not implemented" );
    throw uno::RuntimeException();
}

static bool getModifier( sal_Unicode c, sal_uInt16& mod )
{
    if ( c == '+' ) {
        mod |= KEY_SHIFT;
        return true;
    } else if ( c == '^' ) {
        mod |= KEY_MOD1;
        return true;
    } else if ( c == '%' ) {
        mod |= KEY_MOD2;
        return true;
    }
    return false;
}

/// @throws uno::RuntimeException
static sal_uInt16 parseChar( sal_Unicode c )
{
    sal_uInt16 nVclKey = 0;
    // do we care about locale here for letters/digits? probably not
    if ( rtl::isAsciiAlpha( c ) )
    {
        nVclKey |= ( rtl::toAsciiUpperCase( c ) - 'A' ) + KEY_A;
        if ( rtl::isAsciiUpperCase( c ) )
            nVclKey |= KEY_SHIFT;
    }
    else if ( rtl::isAsciiDigit( c ) )
        nVclKey |= ( c  - '0' ) + KEY_0;
    else if ( c == '~' ) // special case
        nVclKey = KEY_RETURN;
    else if ( c == ' ' ) // special case
        nVclKey = KEY_SPACE;
    else // I guess we have a problem ( but not sure if locale specific keys might come into play here )
        throw uno::RuntimeException();
    return nVclKey;
}

namespace
{

constexpr frozen::unordered_map<std::u16string_view, sal_uInt16, 34> s_KeyCodes
{
    { u"BACKSPACE", KEY_BACKSPACE },
    { u"BS", KEY_BACKSPACE },
    { u"DELETE", KEY_DELETE },
    { u"DEL", KEY_DELETE },
    { u"DOWN", KEY_DOWN },
    { u"UP", KEY_UP },
    { u"LEFT", KEY_LEFT },
    { u"RIGHT", KEY_RIGHT },
    { u"END", KEY_END },
    { u"ESCAPE", KEY_ESCAPE },
    { u"ESC", KEY_ESCAPE },
    { u"HELP", KEY_HELP },
    { u"HOME", KEY_HOME },
    { u"PGDN", KEY_PAGEDOWN },
    { u"PGUP", KEY_PAGEUP },
    { u"INSERT", KEY_INSERT },
    { u"SCROLLLOCK", KEY_SCROLLLOCK },
    { u"NUMLOCK", KEY_NUMLOCK },
    { u"TAB", KEY_TAB },
    { u"F1", KEY_F1 },
    { u"F2", KEY_F2 },
    { u"F3", KEY_F3 },
    { u"F4", KEY_F4 },
    { u"F5", KEY_F5 },
    { u"F6", KEY_F6 },
    { u"F7", KEY_F7 },
    { u"F8", KEY_F8 },
    { u"F9", KEY_F9 },
    { u"F10", KEY_F10 },
    { u"F11", KEY_F11 },
    { u"F12", KEY_F12 },
    { u"F13", KEY_F13 },
    { u"F14", KEY_F14 },
    { u"F15", KEY_F15 }
};

} // end anonymous namespace

awt::KeyEvent parseKeyEvent( std::u16string_view Key )
{
    std::u16string_view sKeyCode;
    sal_uInt16 nVclKey = 0;

    // parse the modifier if any
    for ( size_t i=0; i<Key.size(); ++i )
    {
        if ( ! getModifier( Key[ i ], nVclKey ) )
        {
            sKeyCode = Key.substr( i );
            break;
        }
    }

    // check if keycode is surrounded by '{}', if so scoop out the contents
    // else it should be just one char of ( 'a-z,A-Z,0-9' )
    if ( sKeyCode.size() == 1 ) // ( a single char )
    {
        nVclKey |= parseChar( sKeyCode[ 0 ] );
    }
    else // key should be enclosed in '{}'
    {
        if ( sKeyCode.size() < 3 || sKeyCode[0] != '{' || sKeyCode[sKeyCode.size() - 1 ] != '}' )
            throw uno::RuntimeException();

        sKeyCode = sKeyCode.substr(1, sKeyCode.size() - 2 );

        if ( sKeyCode.size() == 1 )
            nVclKey |= parseChar( sKeyCode[ 0 ] );
        else
        {
            auto it = s_KeyCodes.find(sKeyCode);
            if ( it == s_KeyCodes.end() ) // unknown or unsupported
                throw uno::RuntimeException();
            nVclKey |= it->second;
        }
    }

    awt::KeyEvent aKeyEvent = svt::AcceleratorExecute::st_VCLKey2AWTKey( vcl::KeyCode( nVclKey ) );
    return aKeyEvent;
}

void applyShortCutKeyBinding ( const uno::Reference< frame::XModel >& rxModel, const awt::KeyEvent& rKeyEvent, const OUString& rMacroName )
{
    OUString MacroName( rMacroName );
    if ( !MacroName.isEmpty() )
    {
        OUString aMacroName = MacroName.trim();
        if( aMacroName.startsWith("!") )
            aMacroName = o3tl::trim(aMacroName.subView(1));
        SfxObjectShell* pShell = nullptr;
        if ( rxModel.is() )
        {
            pShell = comphelper::getFromUnoTunnel<SfxObjectShell>(rxModel);
            if ( !pShell )
                throw uno::RuntimeException();
        }
        MacroResolvedInfo aMacroInfo = resolveVBAMacro( pShell, aMacroName );
        if( !aMacroInfo.mbFound )
            throw uno::RuntimeException( u"The procedure doesn't exist"_ustr );
        MacroName = aMacroInfo.msResolvedMacro;
    }
    uno::Reference< ui::XUIConfigurationManagerSupplier > xCfgSupplier(rxModel, uno::UNO_QUERY_THROW);
    uno::Reference< ui::XUIConfigurationManager > xCfgMgr = xCfgSupplier->getUIConfigurationManager();

    uno::Reference< ui::XAcceleratorConfiguration > xAcc( xCfgMgr->getShortCutManager(), uno::UNO_SET_THROW );
    if ( MacroName.isEmpty() )
        // I believe this should really restore the [application] default. Since
        // afaik we don't actually setup application default bindings on import
        // we don't even know what the 'default' would be for this key
        xAcc->removeKeyEvent( rKeyEvent );
    else
        xAcc->setKeyEvent( rKeyEvent, ooo::vba::makeMacroURL( MacroName ) );

}


} // namespace ooo

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_VBAMacroResolver_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ooo::vba::VBAMacroResolver());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

