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

#include <com/sun/star/registry/MergeConflictException.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/configurationhelper.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/fcontnr.hxx>
#include "arrdecl.hxx"
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxuno.hxx>
#include "syspath.hxx"
#include <osl/file.hxx>
#include <osl/security.hxx>
#include "doc.hrc"

#include <rtl/strbuf.hxx>
#include <tools/globname.hxx>

#include <assert.h>
#include <memory>

using namespace ::com::sun::star;


typedef std::vector<SfxViewFactory*> SfxViewFactoryArr_Impl;

struct SfxObjectFactory_Impl
{
    SfxViewFactoryArr_Impl      aViewFactoryArr;// List of <SfxViewFactory>s
    ResId*                      pNameResId;
    OUString             aServiceName;
    SfxFilterContainer*         pFilterContainer;
    SfxModule*                  pModule;
    SvGlobalName                aClassName;

    SfxObjectFactory_Impl() :
        pNameResId          ( nullptr ),
        pFilterContainer    ( nullptr ),
        pModule             ( nullptr )
        {}
};


SfxFilterContainer* SfxObjectFactory::GetFilterContainer() const
{
    return pImpl->pFilterContainer;
}


SfxObjectFactory::SfxObjectFactory
(
    const SvGlobalName&     rName,
    SfxObjectShellFlags     nFlagsP,
    const char*             pName
) :    pShortName( pName ),
       pImpl( new SfxObjectFactory_Impl ),
       nFlags( nFlagsP )
{
    pImpl->pFilterContainer = new SfxFilterContainer( OUString::createFromAscii( pName ) );

    OUString aShortName( OUString::createFromAscii( pShortName ) );
    aShortName = aShortName.toAsciiLowerCase();
    pImpl->aClassName = rName;
    if ( aShortName == "swriter" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SW );
    else if ( aShortName == "swriter/web" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SWWEB );
    else if ( aShortName == "swriter/globaldocument" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SWGLOB );
    else if ( aShortName == "scalc" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SC );
    else if ( aShortName == "simpress" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SI );
    else if ( aShortName == "sdraw" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SD );
    else if ( aShortName == "message" )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_MESSAGE );
}


SfxObjectFactory::~SfxObjectFactory()
{
    delete pImpl->pNameResId;
    delete pImpl->pFilterContainer;
}


void SfxObjectFactory::RegisterViewFactory
(
    SfxViewFactory &rFactory
)
{
#if OSL_DEBUG_LEVEL > 0
    {
        const OUString sViewName( rFactory.GetAPIViewName() );
        for ( SfxViewFactoryArr_Impl::const_iterator it = pImpl->aViewFactoryArr.begin(); it != pImpl->aViewFactoryArr.end(); ++it )
        {
            if ( (*it)->GetAPIViewName() != sViewName )
                continue;
            OStringBuffer aStr("SfxObjectFactory::RegisterViewFactory: duplicate view name '");
            aStr.append(OUStringToOString(sViewName, RTL_TEXTENCODING_ASCII_US));
            aStr.append("'!");
            OSL_FAIL(aStr.getStr());
            break;
        }
    }
#endif
    SfxViewFactoryArr_Impl::iterator it = pImpl->aViewFactoryArr.begin();
    for ( ; it != pImpl->aViewFactoryArr.end() &&
          (*it)->GetOrdinal() <= rFactory.GetOrdinal();
          ++it )
    /* empty loop */;
    pImpl->aViewFactoryArr.insert(it, &rFactory);
}


sal_uInt16 SfxObjectFactory::GetViewFactoryCount() const
{
    return pImpl->aViewFactoryArr.size();
}


SfxViewFactory& SfxObjectFactory::GetViewFactory(sal_uInt16 i) const
{
    return *pImpl->aViewFactoryArr[i];
}


SfxModule* SfxObjectFactory::GetModule() const
{
    return pImpl->pModule;
}

void SfxObjectFactory::SetModule_Impl( SfxModule *pMod )
{
    pImpl->pModule = pMod;
}

void SfxObjectFactory::SetSystemTemplate( const OUString& rServiceName, const OUString& rTemplateName )
{
    static const int nMaxPathSize = 16000;
    static const char SERVICE_FILTER_FACTORY[] = "com.sun.star.document.FilterFactory";
    static const char SERVICE_TYPE_DECTECTION[] = "com.sun.star.document.TypeDetection";

    static const char CONF_ROOT[] = "/org.openoffice.Setup";
    OUString CONF_PATH = "Office/Factories/" + rServiceName;
    static const char PROP_DEF_TEMPL_CHANGED[] = "ooSetupFactorySystemDefaultTemplateChanged";
    static const char PROP_ACTUAL_FILTER[] = "ooSetupFactoryActualFilter";

    static const char DEF_TPL_STR[] = "/soffice.";

    OUString sURL;
    OUString sPath;
    sal_Unicode aPathBuffer[nMaxPathSize];
    if ( SystemPath::GetUserTemplateLocation( aPathBuffer, nMaxPathSize ))
        sPath = OUString( aPathBuffer );
    osl::FileBase::getFileURLFromSystemPath( sPath, sURL );

    OUString aUserTemplateURL( sURL );
    if ( !aUserTemplateURL.isEmpty())
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
            uno::Reference< uno::XInterface > xConfig = ::comphelper::ConfigurationHelper::openConfig(
                ::comphelper::getProcessComponentContext(), CONF_ROOT, ::comphelper::EConfigurationModes::Standard );

            OUString aActualFilter;
            ::comphelper::ConfigurationHelper::readRelativeKey( xConfig, CONF_PATH, PROP_ACTUAL_FILTER ) >>= aActualFilter;
            bool bChanged(false);
            ::comphelper::ConfigurationHelper::readRelativeKey( xConfig, CONF_PATH, PROP_DEF_TEMPL_CHANGED ) >>= bChanged;

            uno::Reference< container::XNameAccess > xFilterFactory(
                xFactory->createInstance( SERVICE_FILTER_FACTORY ), uno::UNO_QUERY_THROW );
            uno::Reference< container::XNameAccess > xTypeDetection(
                xFactory->createInstance( SERVICE_TYPE_DECTECTION ), uno::UNO_QUERY_THROW );

            OUString aActualFilterTypeName;
            uno::Sequence< beans::PropertyValue > aActuralFilterData;
            xFilterFactory->getByName( aActualFilter ) >>= aActuralFilterData;
            for ( sal_Int32 nInd = 0; nInd < aActuralFilterData.getLength(); nInd++ )
                if ( aActuralFilterData[nInd].Name == "Type" )
                    aActuralFilterData[nInd].Value >>= aActualFilterTypeName;
            ::comphelper::SequenceAsHashMap aProps1( xTypeDetection->getByName( aActualFilterTypeName ) );
            uno::Sequence< OUString > aAllExt =
                aProps1.getUnpackedValueOrDefault("Extensions", uno::Sequence< OUString >() );
            //To-do: check if aAllExt is empty first
            OUString aExt = aAllExt[0];

            aUserTemplateURL += DEF_TPL_STR;
            aUserTemplateURL += aExt;

            uno::Reference<ucb::XSimpleFileAccess3> xSimpleFileAccess(
                ucb::SimpleFileAccess::create( ::comphelper::getComponentContext(xFactory) ) );

            OUString aBackupURL;
            ::osl::Security().getConfigDir(aBackupURL);
            aBackupURL += "/temp";

            if ( !xSimpleFileAccess->exists( aBackupURL ) )
                xSimpleFileAccess->createFolder( aBackupURL );

            aBackupURL += DEF_TPL_STR;
            aBackupURL += aExt;

            if ( !rTemplateName.isEmpty() )
            {
                if ( xSimpleFileAccess->exists( aUserTemplateURL ) && !bChanged )
                    xSimpleFileAccess->copy( aUserTemplateURL, aBackupURL );

                uno::Reference< document::XTypeDetection > xTypeDetector( xTypeDetection, uno::UNO_QUERY );
                ::comphelper::SequenceAsHashMap aProps2( xTypeDetection->getByName( xTypeDetector->queryTypeByURL( rTemplateName ) ) );
                OUString aFilterName =
                    aProps2.getUnpackedValueOrDefault("PreferredFilter", OUString() );

                uno::Sequence< beans::PropertyValue > aArgs( 3 );
                aArgs[0].Name = "FilterName";
                aArgs[0].Value <<= aFilterName;
                aArgs[1].Name = "AsTemplate";
                aArgs[1].Value <<= true;
                aArgs[2].Name = "URL";
                aArgs[2].Value <<= OUString( rTemplateName );

                uno::Reference< frame::XLoadable > xLoadable( xFactory->createInstance( rServiceName ), uno::UNO_QUERY );
                xLoadable->load( aArgs );

                aArgs.realloc( 2 );
                aArgs[1].Name = "Overwrite";
                aArgs[1].Value <<= true;

                uno::Reference< frame::XStorable > xStorable( xLoadable, uno::UNO_QUERY );
                xStorable->storeToURL( aUserTemplateURL, aArgs );
                ::comphelper::ConfigurationHelper::writeRelativeKey( xConfig, CONF_PATH, PROP_DEF_TEMPL_CHANGED, uno::makeAny( true ));
                ::comphelper::ConfigurationHelper::flush( xConfig );
            }
            else
            {
                DBG_ASSERT( bChanged, "invalid ooSetupFactorySystemDefaultTemplateChanged value!" );

                xSimpleFileAccess->copy( aBackupURL, aUserTemplateURL );
                xSimpleFileAccess->kill( aBackupURL );
                ::comphelper::ConfigurationHelper::writeRelativeKey( xConfig, CONF_PATH, PROP_DEF_TEMPL_CHANGED, uno::makeAny( false ));
                ::comphelper::ConfigurationHelper::flush( xConfig );
            }
        }
        catch(const uno::Exception&)
        {
        }
    }
}

void SfxObjectFactory::SetStandardTemplate( const OUString& rServiceName, const OUString& rTemplate )
{
    SvtModuleOptions::EFactory eFac = SvtModuleOptions::ClassifyFactoryByServiceName(rServiceName);
    if (eFac == SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
        eFac = SvtModuleOptions::ClassifyFactoryByShortName(rServiceName);
    if (eFac != SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
    {
        SetSystemTemplate( rServiceName, rTemplate );
        SvtModuleOptions().SetFactoryStandardTemplate(eFac, rTemplate);
    }
}

OUString SfxObjectFactory::GetStandardTemplate( const OUString& rServiceName )
{
    SvtModuleOptions::EFactory eFac = SvtModuleOptions::ClassifyFactoryByServiceName(rServiceName);
    if (eFac == SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
        eFac = SvtModuleOptions::ClassifyFactoryByShortName(rServiceName);

    OUString sTemplate;
    if (eFac != SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
        sTemplate = SvtModuleOptions().GetFactoryStandardTemplate(eFac);

    return sTemplate;
}

std::shared_ptr<const SfxFilter> SfxObjectFactory::GetTemplateFilter() const
{
    sal_uInt16 nVersion=0;
    SfxFilterMatcher aMatcher ( OUString::createFromAscii( pShortName ) );
    SfxFilterMatcherIter aIter( aMatcher );
    std::shared_ptr<const SfxFilter> pFilter;
    std::shared_ptr<const SfxFilter> pTemp = aIter.First();
    while ( pTemp )
    {
        if( pTemp->IsOwnFormat() && pTemp->IsOwnTemplateFormat() && ( pTemp->GetVersion() > nVersion ) )
        {
            pFilter = pTemp;
            nVersion = (sal_uInt16) pTemp->GetVersion();
        }

        pTemp = aIter.Next();
    }

    return pFilter;
}

void SfxObjectFactory::SetDocumentServiceName( const OUString& rServiceName )
{
    pImpl->aServiceName = rServiceName;
}

const OUString& SfxObjectFactory::GetDocumentServiceName() const
{
    return pImpl->aServiceName;
}

const SvGlobalName& SfxObjectFactory::GetClassId() const
{
    return pImpl->aClassName;
}

OUString SfxObjectFactory::GetFactoryURL() const
{
    OUStringBuffer aURLComposer;
    aURLComposer.append("private:factory/");
    aURLComposer.appendAscii(GetShortName());
    return aURLComposer.makeStringAndClear();
}

OUString SfxObjectFactory::GetModuleName() const
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

        css::uno::Reference< css::frame::XModuleManager2 > xModuleManager(
            css::frame::ModuleManager::create(xContext));

        OUString sDocService(GetDocumentServiceName());
        ::comphelper::SequenceAsHashMap aPropSet( xModuleManager->getByName(sDocService) );
        OUString sModuleName = aPropSet.getUnpackedValueOrDefault("ooSetupFactoryUIName", OUString());
        return sModuleName;
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception&)
    {
    }

    return OUString();
}


sal_uInt16 SfxObjectFactory::GetViewNo_Impl( const sal_uInt16 i_nViewId, const sal_uInt16 i_nFallback ) const
{
    for ( sal_uInt16 curViewNo = 0; curViewNo < GetViewFactoryCount(); ++curViewNo )
    {
        const sal_uInt16 curViewId = GetViewFactory( curViewNo ).GetOrdinal();
        if ( i_nViewId == curViewId )
           return curViewNo;
    }
    return i_nFallback;
}

SfxViewFactory* SfxObjectFactory::GetViewFactoryByViewName( const OUString& i_rViewName ) const
{
    for (   sal_uInt16 nViewNo = 0;
            nViewNo < GetViewFactoryCount();
            ++nViewNo
        )
    {
        SfxViewFactory& rViewFac( GetViewFactory( nViewNo ) );
        if  (   ( rViewFac.GetAPIViewName() == i_rViewName )
            ||  ( rViewFac.GetLegacyViewName() == i_rViewName )
            )
            return &rViewFac;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
