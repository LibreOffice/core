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
#include <rtl/strbuf.hxx>
#include <sfx2/sfx.hrc>
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

#include "doc.hxx"
#include <IDocumentFieldsAccess.hxx>
#include "docsh.hxx"
#include "docufld.hxx"
#include "wrthtml.hxx"
#include "swhtml.hxx"

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::container;

static HTMLOutEvent aBodyEventTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_O_SDonload,       OOO_STRING_SVTOOLS_HTML_O_onload,       SFX_EVENT_OPENDOC   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonunload, OOO_STRING_SVTOOLS_HTML_O_onunload, SFX_EVENT_PREPARECLOSEDOC   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonfocus,  OOO_STRING_SVTOOLS_HTML_O_onfocus,  SFX_EVENT_ACTIVATEDOC   },
    { OOO_STRING_SVTOOLS_HTML_O_SDonblur,       OOO_STRING_SVTOOLS_HTML_O_onblur,       SFX_EVENT_DEACTIVATEDOC },
    { 0,                    0,                  0                   }
};

void SwHTMLParser::NewScript()
{
    ParseScriptOptions( m_aScriptType, m_sBaseURL, m_eScriptLang, m_aScriptURL,
                        m_aBasicLib, m_aBasicModule );

    if( !m_aScriptURL.isEmpty() )
    {
        // Den Inhalt des Script-Tags ignorieren
        m_bIgnoreRawData = true;
    }
}

void SwHTMLParser::EndScript()
{
    bool bInsIntoBasic = false,
         bInsSrcIntoField = false;

    switch( m_eScriptLang )
    {
    case HTML_SL_STARBASIC:
        bInsIntoBasic = true;
        break;
    default:
        bInsSrcIntoField = true;
        break;
    }

    m_bIgnoreRawData = false;
    m_aScriptSource = convertLineEnd(m_aScriptSource, GetSystemLineEnd());

    // Ausser StarBasic und unbenutzem JavaScript jedes Script oder den
    // Modulnamen in einem Feld merken merken
    if( bInsSrcIntoField && !m_bIgnoreHTMLComments )
    {
        SwScriptFieldType *pType =
            static_cast<SwScriptFieldType*>(m_pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_SCRIPTFLD ));

        SwScriptField aField( pType, m_aScriptType,
                            !m_aScriptURL.isEmpty() ? m_aScriptURL : m_aScriptSource,
                            !m_aScriptURL.isEmpty() );
        InsertAttr( SwFormatField( aField ) );
    }

    SwDocShell *pDocSh = m_pDoc->GetDocShell();
    if( !m_aScriptSource.isEmpty() && pDocSh &&
        bInsIntoBasic && IsNewDoc() )
    {
    // Fuer JavaScript und StarBasic noch ein Basic-Modul anlegen
        // Das Basic entfernt natuerlich weiterhin keine SGML-Kommentare
        RemoveSGMLComment( m_aScriptSource, true );

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
                        m_aBasicModule = "Modul";
                        m_aBasicModule += OUString::number( (sal_Int32)(++m_nSBModuleCnt) );
                        bFound = xModLib->hasByName( m_aBasicModule );
                    }
                }

                // create module
                OUString aModName( m_aBasicModule );
                if ( !xModLib->hasByName( aModName ) )
                {
                    Any aElement;
                    aElement <<= OUString( m_aScriptSource );
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
    // Hier merken wir und nur ein par Strings
    if( aToken.getLength() > 2 &&
        (HTML_SL_STARBASIC==m_eScriptLang && aToken[ 0 ] == '\'') )
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
        // Leerzeilen am Anfang werden ignoriert
        if( !m_aScriptSource.isEmpty() )
        {
            m_aScriptSource += "\n";
        }
        else
        {
            // Wir stehen hinter dem CR/LF der Zeile davor
            m_nScriptStartLineNr = GetLineNr() - 1;
        }
        m_aScriptSource += aToken;
    }
}

void SwHTMLParser::InsertBasicDocEvent( const OUString& aEvent, const OUString& rName,
                                        ScriptType eScrType,
                                        const OUString& rScrType )
{
    OSL_ENSURE( !rName.isEmpty(), "InsertBasicDocEvent() ohne Macro gerufen" );
    if( rName.isEmpty() )
        return;

    SwDocShell *pDocSh = m_pDoc->GetDocShell();
    OSL_ENSURE( pDocSh, "Wo ist die DocShell?" );
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

    BasicManager *pBasicMan = pDoc->GetDocShell()->GetBasicManager();
    OSL_ENSURE( pBasicMan, "Wo ist der Basic-Manager?" );
    // nur das DocumentBasic schreiben
    if( !pBasicMan || pBasicMan == SfxApplication::GetBasicManager() )
    {
        return;
    }

    // und jetzt alle StarBasic-Module und alle unbenutzen JavaSrript-Module
    // ausgeben
    for( sal_uInt16 i=0; i<pBasicMan->GetLibCount(); i++ )
    {
        StarBASIC *pBasic = pBasicMan->GetLib( i  );
        const OUString& rLibName = pBasic->GetName();

        SbxArray *pModules = pBasic->GetModules();
        for( sal_uInt16 j=0; j<pModules->Count(); j++ )
        {
            const SbModule &rModule = dynamic_cast<const SbModule&>(*pModules->Get(j));

            OUString sLang(SVX_MACRO_LANGUAGE_STARBASIC);
            ScriptType eType = STARBASIC;

            if( 0==i && 0==j )
            {
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

            const OUString& rModName = rModule.GetName();
            Strm().WriteCharPtr( SAL_NEWLINE_STRING );   // nicht einruecken!
            HTMLOutFuncs::OutScript( Strm(), GetBaseURL(), rModule.GetSource(),
                                     sLang, eType, aEmptyOUStr,
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
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !pDocSh )
        return;

    SvxMacroTableDtor aDocTable;

    uno::Reference< document::XEventsSupplier > xSup( pDocSh->GetModel(), uno::UNO_QUERY );
    uno::Reference < container::XNameReplace > xEvents = xSup->getEvents();
    for ( sal_Int32 i=0; i<4; i++ )
    {
        SvxMacro* pMacro = SfxEventConfiguration::ConvertToMacro( xEvents->getByName( OUString::createFromAscii(aEventNames[i]) ), pDocSh, true );
        if ( pMacro )
        {
            aDocTable.Insert( aBodyEventTable[i].nEvent, *pMacro );
            delete pMacro;
        }
    }

    if( !aDocTable.empty() )
        HTMLOutFuncs::Out_Events( Strm(), aDocTable, aBodyEventTable,
                                  m_bCfgStarBasic, m_eDestEnc, &m_aNonConvertableCharacters );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
