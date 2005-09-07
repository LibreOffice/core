/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docfac.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:38:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_REGISTRY_MERGECONFLICTEXCEPTION_HPP_
#include <com/sun/star/registry/MergeConflictException.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#include <svtools/pathoptions.hxx>
#include <svtools/moduleoptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>

#pragma hdrstop

#include "sfx.hrc"
#include "docfilt.hxx"
#include "docfac.hxx"
#include "viewfac.hxx"
#include "fltfnc.hxx"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "app.hxx"
#include "module.hxx"
#include "mnumgr.hxx"
#include "sfxresid.hxx"
#include "sfxuno.hxx"
#include "doc.hrc"

namespace css = ::com::sun::star;

//========================================================================

DECL_PTRARRAY( SfxViewFactoryArr_Impl, SfxViewFactory*, 2, 2 );

//========================================================================

DBG_NAME(SfxObjectFactory);

static SfxObjectFactoryArr_Impl* pObjFac = 0;

//========================================================================

struct SfxObjectFactory_Impl
{
    SfxViewFactoryArr_Impl      aViewFactoryArr;// Liste von <SfxViewFactory>s
    SfxFilterArr_Impl           aFilterArr;     // Liste von <SFxFilter>n
    ResId*                      pMenuBarResId;
    ResId*                      pAccelResId;
    ResId*                      pNameResId;
    String                      aHelpFile;
    String                      aHelpPIFile;
    ::rtl::OUString             aServiceName;
    SfxFilterContainer*         pFilterContainer;
    SfxModule*                  pModule;
    sal_uInt16                  nImageId;
    String                      aStandardTemplate;
    sal_Bool                    bTemplateInitialized;
    SvGlobalName                aClassName;

    SfxObjectFactory_Impl() :
        pMenuBarResId       ( NULL ),
        pAccelResId         ( NULL ),
        pNameResId          ( NULL ),
        pFilterContainer    ( NULL ),
        pModule             ( NULL ),
        nImageId            ( 0 ),
        bTemplateInitialized( sal_False )
        {}

    ~SfxObjectFactory_Impl()
    {
        delete pMenuBarResId;
        delete pAccelResId;
        // Jetzt vom FilterMatcher
        // delete pFilterContainer;
    }
};

//========================================================================

SfxFilterContainer* SfxObjectFactory::GetFilterContainer( sal_Bool bForceLoad ) const
{
    return pImpl->pFilterContainer;
}

//--------------------------------------------------------------------

SfxObjectFactory::SfxObjectFactory
(
    const SvGlobalName&     rName,
    SfxObjectShellFlags     nFlagsP,
    const char*             pName
)
    : nFlags( nFlagsP ),
    pShortName( pName ),
    pImpl( new SfxObjectFactory_Impl )
{
    DBG_CTOR(SfxObjectFactory, 0);
    pImpl->pFilterContainer = new SfxFilterContainer( String::CreateFromAscii( pName ) );

    pImpl->aHelpFile = String::CreateFromAscii(pShortName);
    pImpl->aHelpFile.Erase( 8 );
    pImpl->aHelpPIFile = String(pImpl->aHelpFile,0,3);
    pImpl->aHelpPIFile += DEFINE_CONST_UNICODE( "hlppi" );
    pImpl->aHelpFile += DEFINE_CONST_UNICODE( ".hlp" );
    pImpl->aHelpPIFile += DEFINE_CONST_UNICODE( ".hlp" );

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

void SfxObjectFactory::RegisterMenuBar( const ResId& rId )
{
    delete pImpl->pMenuBarResId;
    pImpl->pMenuBarResId = new ResId( rId );
}

//--------------------------------------------------------------------

const ResId* SfxObjectFactory::GetMenuBarId() const
{
    return pImpl->pMenuBarResId;
}

//--------------------------------------------------------------------

const ResId* SfxObjectFactory::GetAccelId() const
{
    return pImpl->pAccelResId;
}

//--------------------------------------------------------------------

void SfxObjectFactory::RegisterAccel( const ResId& rId )
{
    DBG_ASSERT( !pImpl->pAccelResId, "SfxObjectFactory: double registration of Accel" );
    pImpl->pAccelResId = new ResId(rId);
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------

void SfxObjectFactory::RegisterHelpFile( const String& rString )
{
    pImpl->aHelpFile = rString;
}

//--------------------------------------------------------------------

const String& SfxObjectFactory::GetHelpFile() const
{
    return pImpl->aHelpFile;
}

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
