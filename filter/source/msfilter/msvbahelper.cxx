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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <filter/msfilter/msvbahelper.hxx>
#include <basic/sbx.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>

using namespace ::com::sun::star;

namespace ooo { namespace vba {

const static rtl::OUString sUrlPart0 = rtl::OUString::createFromAscii( "vnd.sun.star.script:");
const static rtl::OUString sUrlPart1 = rtl::OUString::createFromAscii( "?language=Basic&location=document");

String makeMacroURL( const String& sMacroName )
{
    return sUrlPart0.concat( sMacroName ).concat( sUrlPart1 ) ;
}

::rtl::OUString extractMacroName( const ::rtl::OUString& rMacroUrl )
{
    if( (rMacroUrl.getLength() > sUrlPart0.getLength() + sUrlPart1.getLength()) &&
        rMacroUrl.match( sUrlPart0 ) &&
        rMacroUrl.match( sUrlPart1, rMacroUrl.getLength() - sUrlPart1.getLength() ) )
    {
        return rMacroUrl.copy( sUrlPart0.getLength(), rMacroUrl.getLength() - sUrlPart0.getLength() - sUrlPart1.getLength() );
    }
    return ::rtl::OUString();
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
                if ( aURL.getLength() && aURL.indexOf( '/' ) == -1 )
                {
                    sal_Int32 lastSlashIndex = xModel->getURL().lastIndexOf( '/' );
                    if ( lastSlashIndex > -1 )
                    {
                        bDocNameNoPathMatch = xModel->getURL().copy( lastSlashIndex + 1 ).equals( aURL );
                        if ( !bDocNameNoPathMatch )
                        {
                            rtl::OUString aTmpName = rtl::OUString::createFromAscii("'") + xModel->getURL().copy( lastSlashIndex + 1 ) + rtl::OUString::createFromAscii("'");
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
bool hasMacro( SfxObjectShell* pShell, const String& sLibrary, String& sMod, const String& sMacro )
{
    bool bFound = false;
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
                USHORT nId = pBasicMgr->GetLibId( sLibrary );
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
                        sMod = pModule->GetName();
                        bFound = true;
                    }
                }
            }
        }
    }
    return bFound;
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

VBAMacroResolvedInfo resolveVBAMacro( SfxObjectShell* pShell, const rtl::OUString& MacroName, bool bSearchGlobalTemplates )
{
    VBAMacroResolvedInfo aRes;
    if ( !pShell )
        return aRes;
    aRes.SetMacroDocContext( pShell );
    
    // the name may be enclosed in apostrophs
    ::rtl::OUString sMacroUrl = MacroName;
    sal_Int32 nMacroLen = MacroName.getLength();
    if( (nMacroLen >= 2) && (MacroName[0] == '\'') && (MacroName[nMacroLen-1] == '\'') )
        sMacroUrl = MacroName.copy( 1, nMacroLen - 2 );
    
    // parse the macro name
    sal_Int32 nDocSepIndex = sMacroUrl.indexOf( '!' );

    String sContainer;
    String sModule;
    String sProcedure;
    
    if( nDocSepIndex > 0 )
    {
        // macro specified by document name
        // find document shell for document name and call ourselves 
        // recursively

        // assume for now that the document name is *this* document
        String sDocUrlOrPath = sMacroUrl.copy( 0, nDocSepIndex );
        sMacroUrl = sMacroUrl.copy( nDocSepIndex + 1 );
        OSL_TRACE("doc search, current shell is 0x%x", pShell );
        SfxObjectShell* pFoundShell = NULL;
        if( bSearchGlobalTemplates )
        {
            SvtPathOptions aPathOpt;
            String aAddinPath = aPathOpt.GetAddinPath();
            if( rtl::OUString( sDocUrlOrPath ).indexOf( aAddinPath ) == 0 )
                pFoundShell = pShell; 
        }
        if( pFoundShell == NULL )
            pFoundShell = findShellForUrl( sDocUrlOrPath );
        OSL_TRACE("doc search, after find, found shell is 0x%x", pFoundShell );
        aRes = resolveVBAMacro( pFoundShell, sMacroUrl, bSearchGlobalTemplates ); 
        return aRes;
    }    
    else
    {
        // macro is contained in 'this' document ( or code imported from a template
        // where that template is a global template or perhaps the template this
        // document is created from ) 
    
        // macro format = Container.Module.Procedure
        parseMacro( sMacroUrl, sContainer, sModule, sProcedure );
        uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY);
        uno::Reference< container::XNameContainer > xPrjNameCache;
        if ( xSF.is() )
            xPrjNameCache.set( xSF->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.VBAProjectNameProvider" ) ) ), uno::UNO_QUERY );
    
        std::vector< rtl::OUString > sSearchList; 

        if ( sContainer.Len() > 0 )
        { 
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
            sSearchList.push_back( sContainer ); // First Lib to search
        }
        else
        {
            // Ok, if we have no Container specified then we need to search them in order, this document, template this document created from, global templates, 
            // get the name of Project/Library for 'this' document
            rtl::OUString sThisProject;
            BasicManager* pBasicMgr = pShell-> GetBasicManager();
            if ( pBasicMgr )
            {
                if ( pBasicMgr->GetName().Len() )
                   sThisProject = pBasicMgr->GetName();
                else // cater for the case where VBA is not enabled
                   sThisProject = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Standard") );
            }
            sSearchList.push_back( sThisProject ); // First Lib to search
            if ( xPrjNameCache.is() )
            {
                // is this document created from a template?
                uno::Reference< document::XDocumentInfoSupplier > xDocInfoSupp( pShell->GetModel(), uno::UNO_QUERY_THROW );
                uno::Reference< document::XDocumentPropertiesSupplier > xDocPropSupp( xDocInfoSupp->getDocumentInfo(), uno::UNO_QUERY_THROW );
                uno::Reference< document::XDocumentProperties > xDocProps( xDocPropSupp->getDocumentProperties(), uno::UNO_QUERY_THROW );
            
                rtl::OUString sCreatedFrom = xDocProps->getTemplateURL();
                if ( sCreatedFrom.getLength() )
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
                if ( sCreatedFrom.getLength() && xPrjNameCache->hasByName( sCreatedFrom ) )
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
        }
        std::vector< rtl::OUString >::iterator it_end = sSearchList.end();
        for ( std::vector< rtl::OUString >::iterator it = sSearchList.begin(); it != it_end; ++it )
        {
            bool bRes = hasMacro( pShell, *it, sModule, sProcedure );
            if ( bRes )
            {
                aRes.SetResolved( true );
                aRes.SetMacroDocContext( pShell );
                sContainer = *it;
                break;
            }
        }
        aRes.SetResolvedMacro( sProcedure.Insert( '.', 0 ).Insert( sModule, 0).Insert( '.', 0 ).Insert( sContainer, 0 ) );
    }

    return aRes;
}

// Treat the args as possible inouts ( convertion at bottom of method )
sal_Bool executeMacro( SfxObjectShell* pShell, const String& sMacroName, uno::Sequence< uno::Any >& aArgs, uno::Any& aRet, const uno::Any& /*aCaller*/)
{
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
    catch ( uno::Exception& )
    {
       bRes = sal_False;
    }
    return bRes; 
}
} } // vba // ooo
