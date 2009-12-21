/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docfac.cxx,v $
 * $Revision: 1.28 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/registry/MergeConflictException.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <tools/config.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <sfx2/sfx.hrc>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "viewfac.hxx"
#include "fltfnc.hxx"
#include "arrdecl.hxx"
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/mnumgr.hxx>
#include "sfxresid.hxx"
#include <sfx2/sfxuno.hxx>
#include "doc.hrc"

namespace css = ::com::sun::star;

//========================================================================

DECL_PTRARRAY( SfxViewFactoryArr_Impl, SfxViewFactory*, 2, 2 )

//========================================================================

DBG_NAME(SfxObjectFactory)

//static SfxObjectFactoryArr_Impl* pObjFac = 0;

//========================================================================

struct SfxObjectFactory_Impl
{
    SfxViewFactoryArr_Impl      aViewFactoryArr;// Liste von <SfxViewFactory>s
    SfxFilterArr_Impl           aFilterArr;     // Liste von <SFxFilter>n
    ResId*                      pNameResId;
    ::rtl::OUString             aServiceName;
    SfxFilterContainer*         pFilterContainer;
    SfxModule*                  pModule;
    sal_uInt16                  nImageId;
    String                      aStandardTemplate;
    sal_Bool                    bTemplateInitialized;
    SvGlobalName                aClassName;

    SfxObjectFactory_Impl() :
        pNameResId          ( NULL ),
        pFilterContainer    ( NULL ),
        pModule             ( NULL ),
        nImageId            ( 0 ),
        bTemplateInitialized( sal_False )
        {}
};

//========================================================================

SfxFilterContainer* SfxObjectFactory::GetFilterContainer( sal_Bool /*bForceLoad*/ ) const
{
    return pImpl->pFilterContainer;
}

//--------------------------------------------------------------------

SfxObjectFactory::SfxObjectFactory
(
    const SvGlobalName&     rName,
    SfxObjectShellFlags     nFlagsP,
    const char*             pName
) :    pShortName( pName ),
       pImpl( new SfxObjectFactory_Impl ),
       nFlags( nFlagsP )
{
    DBG_CTOR(SfxObjectFactory, 0);
    pImpl->pFilterContainer = new SfxFilterContainer( String::CreateFromAscii( pName ) );

    String aShortName( String::CreateFromAscii( pShortName ) );
    aShortName.ToLowerAscii();
    pImpl->aClassName = rName;
    if ( aShortName.EqualsAscii( "swriter" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SW );
    else if ( aShortName.EqualsAscii( "swriter/web" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SWWEB );
    else if ( aShortName.EqualsAscii( "swriter/globaldocument" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SWGLOB );
    else if ( aShortName.EqualsAscii( "scalc" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SC );
    else if ( aShortName.EqualsAscii( "simpress" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SI );
    else if ( aShortName.EqualsAscii( "sdraw" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_SD );
    else if ( aShortName.EqualsAscii( "message" ) )
        pImpl->pNameResId = new SfxResId( STR_DOCTYPENAME_MESSAGE );
}

//--------------------------------------------------------------------

SfxObjectFactory::~SfxObjectFactory()
{
    DBG_DTOR(SfxObjectFactory, 0);

    const sal_uInt16 nCount = pImpl->aFilterArr.Count();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
        delete pImpl->aFilterArr[i];
    delete pImpl->pNameResId;
    delete pImpl;
}

//--------------------------------------------------------------------

void SfxObjectFactory::RegisterViewFactory
(
    SfxViewFactory &rFactory
)
{
    sal_uInt16 nPos;
    for ( nPos = 0;
          nPos < pImpl->aViewFactoryArr.Count() &&
          pImpl->aViewFactoryArr[nPos]->GetOrdinal() <= rFactory.GetOrdinal();
          ++nPos )
    /* empty loop */;
    pImpl->aViewFactoryArr.Insert(nPos, &rFactory);
}

//--------------------------------------------------------------------

sal_uInt16 SfxObjectFactory::GetViewFactoryCount() const
{
    return pImpl->aViewFactoryArr.Count();
}

//--------------------------------------------------------------------

SfxViewFactory& SfxObjectFactory::GetViewFactory(sal_uInt16 i) const
{
    return *pImpl->aViewFactoryArr[i];
}

//--------------------------------------------------------------------

SfxModule* SfxObjectFactory::GetModule() const
{
    return pImpl->pModule;
}

void SfxObjectFactory::SetModule_Impl( SfxModule *pMod )
{
    pImpl->pModule = pMod;
}

void SfxObjectFactory::SetStandardTemplate( const String& rServiceName, const String& rTemplate )
{
    SvtModuleOptions::EFactory eFac = SvtModuleOptions::ClassifyFactoryByServiceName(rServiceName);
    if (eFac == SvtModuleOptions::E_UNKNOWN_FACTORY)
        eFac = SvtModuleOptions::ClassifyFactoryByShortName(rServiceName);
    if (eFac != SvtModuleOptions::E_UNKNOWN_FACTORY)
        SvtModuleOptions().SetFactoryStandardTemplate(eFac, rTemplate);
}

String SfxObjectFactory::GetStandardTemplate( const String& rServiceName )
{
    SvtModuleOptions::EFactory eFac = SvtModuleOptions::ClassifyFactoryByServiceName(rServiceName);
    if (eFac == SvtModuleOptions::E_UNKNOWN_FACTORY)
        eFac = SvtModuleOptions::ClassifyFactoryByShortName(rServiceName);

    String sTemplate;
    if (eFac != SvtModuleOptions::E_UNKNOWN_FACTORY)
        sTemplate = SvtModuleOptions().GetFactoryStandardTemplate(eFac);

    return sTemplate;
}

/*
const SfxObjectFactory* SfxObjectFactory::GetFactory( const String& rFactoryURL )
{
    const SfxObjectFactory* pFactory = 0;
    String aFact( rFactoryURL );
    String aPrefix( DEFINE_CONST_UNICODE( "private:factory/" ) );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
        // Aufruf m"oglich mit z.B. "swriter" oder "private:factory/swriter"
        aFact.Erase( 0, aPrefix.Len() );
    sal_uInt16 nPos = aFact.Search( '?' );

    // Etwaige Parameter abschneiden
    aFact.Erase( nPos, aFact.Len() );

    SfxApplication *pApp = SFX_APP();

    // "swriter4" durch "swriter" ersetzen, zum Vergleichen uppercase verwenden
    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( sal_uInt16 n = GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = &GetObjectFactory_Impl( n );
        String aCompareTo = String::CreateFromAscii( pFactory->GetShortName() );
        aCompareTo.ToUpperAscii();
        if( !aSearchedFac.Matches( aCompareTo ) )
            pFactory = 0;
    }

    return pFactory;
}
*/

const SfxFilter* SfxObjectFactory::GetTemplateFilter() const
{
    USHORT nVersion=0;
    SfxFilterMatcher aMatcher ( String::CreateFromAscii( pShortName ) );
    SfxFilterMatcherIter aIter( &aMatcher );
    const SfxFilter *pFilter = 0;
    const SfxFilter *pTemp = aIter.First();
    while ( pTemp )
    {
        if( pTemp->IsOwnFormat() && pTemp->IsOwnTemplateFormat() && ( pTemp->GetVersion() > nVersion ) )
        {
            pFilter = pTemp;
            nVersion = (USHORT) pTemp->GetVersion();
        }

        pTemp = aIter.Next();
    }

    return pFilter;
}

void SfxObjectFactory::SetDocumentTypeNameResource( const ResId& rId )
{
    DBG_ASSERT( !pImpl->pNameResId, "UI-Namensresource mehrfach gesetzt!" );
    pImpl->pNameResId = new ResId( rId );
}

String SfxObjectFactory::GetDocumentTypeName() const
{
    if ( pImpl->pNameResId )
        return String( *pImpl->pNameResId );
    return String();
}

void SfxObjectFactory::SetDocumentServiceName( const ::rtl::OUString& rServiceName )
{
    pImpl->aServiceName = rServiceName;
}

const ::rtl::OUString& SfxObjectFactory::GetDocumentServiceName() const
{
    return pImpl->aServiceName;
}

const SvGlobalName& SfxObjectFactory::GetClassId() const
{
    return pImpl->aClassName;
}

String SfxObjectFactory::GetModuleName() const
{
    static ::rtl::OUString SERVICENAME_MODULEMANAGER = ::rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager");
    static ::rtl::OUString PROP_MODULEUINAME         = ::rtl::OUString::createFromAscii("ooSetupFactoryUIName");

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

        css::uno::Reference< css::container::XNameAccess > xModuleManager(
            xSMGR->createInstance(SERVICENAME_MODULEMANAGER),
            css::uno::UNO_QUERY_THROW);

        ::rtl::OUString sDocService(GetDocumentServiceName());
        ::comphelper::SequenceAsHashMap aPropSet( xModuleManager->getByName(sDocService) );
        ::rtl::OUString sModuleName = aPropSet.getUnpackedValueOrDefault(PROP_MODULEUINAME, ::rtl::OUString());
        return String(sModuleName);
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        {}

    return String();
}
