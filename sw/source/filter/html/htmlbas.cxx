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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <basic/sbx.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/app.hxx>
#include <svtools/htmlout.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <fmtornt.hxx>
#include <fmtfld.hxx>

#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <docsh.hxx>
#include <docufld.hxx>
#include "wrthtml.hxx"
#include "swhtml.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

static HTMLOutEvent const aBodyEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonload,    OOO_STRING_SVTOOLS_HTML_O_onload,    SvMacroItemId::OpenDoc   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonunload,  OOO_STRING_SVTOOLS_HTML_O_onunload,  SvMacroItemId::PrepareCloseDoc   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonfocus,   OOO_STRING_SVTOOLS_HTML_O_onfocus,   SvMacroItemId::ActivateDoc   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonblur,    OOO_STRING_SVTOOLS_HTML_O_onblur,    SvMacroItemId::DeactivateDoc },
    { nullptr,                               nullptr,                             SvMacroItemId::NONE }
};

void SwHTMLParser::NewScript()
{
    ParseScriptOptions( m_aScriptType, m_sBaseURL, m_eScriptLang, m_aScriptURL,
                        m_aBasicLib, m_aBasicModule );

    if( !m_aScriptURL.isEmpty() )
    {
        // Ignore the script tag
        m_bIgnoreRawData = true;
    }
}

void SwHTMLParser::EndScript()
{
    bool bInsIntoBasic = false,
         bInsSrcIntoField = false;

    switch( m_eScriptLang )
    {
    case HTMLScriptLanguage::StarBasic:
        bInsIntoBasic = true;
        break;
    default:
        bInsSrcIntoField = true;
        break;
    }

    m_bIgnoreRawData = false;
    m_aScriptSource = convertLineEnd(m_aScriptSource, GetSystemLineEnd());

    // Except for StarBasic and unused JavaScript, save each script or module name in a field
    if( bInsSrcIntoField && !m_bIgnoreHTMLComments )
    {
        SwScriptFieldType *pType =
            static_cast<SwScriptFieldType*>(m_xDoc->getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Script ));

        SwScriptField aField( pType, m_aScriptType,
                            !m_aScriptURL.isEmpty() ? m_aScriptURL : m_aScriptSource,
                            !m_aScriptURL.isEmpty() );
        InsertAttr( SwFormatField( aField ), false );
    }

    SwDocShell *pDocSh = m_xDoc->GetDocShell();
    if( !m_aScriptSource.isEmpty() && pDocSh &&
        bInsIntoBasic && IsNewDoc() )
    {
    // Create a Basic module for javascript and StarBasic.

        // The Basic does still not remove SGML comments
        RemoveSGMLComment( m_aScriptSource );

        // get library name
        OUString aLibName;
        if( !m_aBasicLib.isEmpty() )
            aLibName = m_aBasicLib;
        else
            aLibName = "Standard";

        // get module library container
        Reference< script::XLibraryContainer > xModLibContainer( pDocSh->GetBasicContainer(), UNO_QUERY );

        if ( xModLibContainer.is() )
        {
            Reference< container::XNameContainer > xModLib;
            if ( xModLibContainer->hasByName( aLibName ) )
            {
                // get module library
                Any aElement = xModLibContainer->getByName( aLibName );
                aElement >>= xModLib;
            }
            else
            {
                // create module library
                xModLib = xModLibContainer->createLibrary( aLibName );
            }

            if ( xModLib.is() )
            {
                if( m_aBasicModule.isEmpty() )
                {
                    // create module name
                    bool bFound = true;
                    while( bFound )
                    {
                        m_aBasicModule = "Modul" + OUString::number( static_cast<sal_Int32>(++m_nSBModuleCnt) );
                        bFound = xModLib->hasByName( m_aBasicModule );
                    }
                }

                // create module
                OUString aModName( m_aBasicModule );
                if ( !xModLib->hasByName( aModName ) )
                {
                    Any aElement;
                    aElement <<= m_aScriptSource;
                    xModLib->insertByName( aModName , aElement );
                }
            }
        }

        // get dialog library container
        Reference< script::XLibraryContainer > xDlgLibContainer( pDocSh->GetDialogContainer(), UNO_QUERY );

        if ( xDlgLibContainer.is() )
        {
            if ( !xDlgLibContainer->hasByName( aLibName ) )
            {
                // create dialog library
                xDlgLibContainer->createLibrary( aLibName );
            }
        }
    }

    m_aScriptSource.clear();
    m_aScriptType.clear();
    m_aScriptURL.clear();

    m_aBasicLib.clear();
    m_aBasicModule.clear();
}

void SwHTMLParser::AddScriptSource()
{
    // We'll just remember a few strings here
    if( aToken.getLength() > 2 &&
        (HTMLScriptLanguage::StarBasic==m_eScriptLang && aToken[ 0 ] == '\'') )
    {
        sal_Int32 nPos = -1;
        if( m_aBasicLib.isEmpty() )
        {
            nPos = aToken.indexOf( OOO_STRING_SVTOOLS_HTML_SB_library );
            if( nPos != -1 )
            {
                m_aBasicLib =
                    aToken.copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_library) - 1 );
                m_aBasicLib = comphelper::string::strip(m_aBasicLib, ' ');
            }
        }

        if( m_aBasicModule.isEmpty() && nPos == -1 )
        {
            nPos = aToken.indexOf( OOO_STRING_SVTOOLS_HTML_SB_module );
            if( nPos != -1 )
            {
                m_aBasicModule =
                    aToken.copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_module) - 1 );
                m_aBasicModule = comphelper::string::strip(m_aBasicModule, ' ');
            }
        }

        if( nPos == -1 )
        {
            if( !m_aScriptSource.isEmpty() )
                m_aScriptSource += "\n";
            m_aScriptSource += aToken;
        }
    }
    else if( !m_aScriptSource.isEmpty() || !aToken.isEmpty() )
    {
        // Empty lines are ignored on the beginning
        if( !m_aScriptSource.isEmpty() )
        {
            m_aScriptSource += "\n";
        }
        m_aScriptSource += aToken;
    }
}

void SwHTMLParser::InsertBasicDocEvent( const OUString& aEvent, const OUString& rName,
                                        ScriptType eScrType,
                                        const OUString& rScrType )
{
    OSL_ENSURE( !rName.isEmpty(), "InsertBasicDocEvent() called without macro" );
    if( rName.isEmpty() )
        return;

    SwDocShell *pDocSh = m_xDoc->GetDocShell();
    OSL_ENSURE( pDocSh, "Where is the DocShell?" );
    if( !pDocSh )
        return;

    OUString sEvent(convertLineEnd(rName, GetSystemLineEnd()));
    OUString sScriptType;
    if( EXTENDED_STYPE == eScrType )
        sScriptType = rScrType;

    SfxEventConfiguration::ConfigureEvent( aEvent, SvxMacro( sEvent, sScriptType, eScrType ),
                           pDocSh );
}

void SwHTMLWriter::OutBasic()
{
#if HAVE_FEATURE_SCRIPTING
    if( !m_bCfgStarBasic )
        return;

    BasicManager *pBasicMan = m_pDoc->GetDocShell()->GetBasicManager();
    OSL_ENSURE( pBasicMan, "Where is the Basic-Manager?" );
    // Only write DocumentBasic
    if( !pBasicMan || pBasicMan == SfxApplication::GetBasicManager() )
    {
        return;
    }

    bool bFirst=true;
    // Now write all StarBasic and unused Javascript modules
    for( sal_uInt16 i=0; i<pBasicMan->GetLibCount(); i++ )
    {
        StarBASIC *pBasic = pBasicMan->GetLib( i  );
        const OUString& rLibName = pBasic->GetName();
        for( const auto& pModule: pBasic->GetModules() )
        {
            OUString sLang(SVX_MACRO_LANGUAGE_STARBASIC);

            if( bFirst )
            {
                bFirst = false;
                OutNewLine();
                OStringBuffer sOut;
                sOut.append('<').append(OOO_STRING_SVTOOLS_HTML_meta)
                    .append(' ').append(OOO_STRING_SVTOOLS_HTML_O_httpequiv)
                    .append("=\"")
                    .append(OOO_STRING_SVTOOLS_HTML_META_content_script_type)
                    .append("\" ").append(OOO_STRING_SVTOOLS_HTML_O_content)
                    .append("=\"text/x-");
                Strm().WriteCharPtr( sOut.getStr() );
                // Entities aren't welcome here
                Strm().WriteCharPtr( OUStringToOString(sLang, m_eDestEnc).getStr() )
                   .WriteCharPtr( "\">" );
            }

            const OUString& rModName = pModule->GetName();
            Strm().WriteCharPtr( SAL_NEWLINE_STRING );   // don't indent!
            HTMLOutFuncs::OutScript( Strm(), GetBaseURL(), pModule->GetSource(),
                                     sLang, STARBASIC, OUString(),
                                     &rLibName, &rModName,
                                     m_eDestEnc, &m_aNonConvertableCharacters );
        }
    }
#endif
}

static const char* aEventNames[] =
{
    "OnLoad", "OnPrepareUnload", "OnFocus", "OnUnfocus"
};

void SwHTMLWriter::OutBasicBodyEvents()
{
    SwDocShell *pDocSh = m_pDoc->GetDocShell();
    if( !pDocSh )
        return;

    SvxMacroTableDtor aDocTable;

    uno::Reference< document::XEventsSupplier > xSup( pDocSh->GetModel(), uno::UNO_QUERY );
    uno::Reference < container::XNameReplace > xEvents = xSup->getEvents();
    for ( sal_Int32 i=0; i<4; i++ )
    {
        std::unique_ptr<SvxMacro> pMacro = SfxEventConfiguration::ConvertToMacro( xEvents->getByName( OUString::createFromAscii(aEventNames[i]) ), pDocSh );
        if ( pMacro )
        {
            aDocTable.Insert( aBodyEventTable[i].nEvent, *pMacro );
        }
    }

    if( !aDocTable.empty() )
        HTMLOutFuncs::Out_Events( Strm(), aDocTable, aBodyEventTable,
                                  m_bCfgStarBasic, m_eDestEnc, &m_aNonConvertableCharacters );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
