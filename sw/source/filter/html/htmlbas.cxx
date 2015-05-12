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
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

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
    ParseScriptOptions( aScriptType, sBaseURL, eScriptLang, aScriptURL,
                        aBasicLib, aBasicModule );

    if( !aScriptURL.isEmpty() )
    {
        // Den Inhalt des Script-Tags ignorieren
        bIgnoreRawData = true;
    }
}

void SwHTMLParser::EndScript()
{
    bool bInsIntoBasic = false,
         bInsSrcIntoField = false;

    switch( eScriptLang )
    {
    case HTML_SL_STARBASIC:
        bInsIntoBasic = true;
        break;
    default:
        bInsSrcIntoField = true;
        break;
    }

    bIgnoreRawData = false;
    aScriptSource = convertLineEnd(aScriptSource, GetSystemLineEnd());

    // Ausser StarBasic und unbenutzem JavaScript jedes Script oder den
    // Modulnamen in einem Feld merken merken
    if( bInsSrcIntoField && !bIgnoreHTMLComments )
    {
        SwScriptFieldType *pType =
            static_cast<SwScriptFieldType*>(pDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_SCRIPTFLD ));

        SwScriptField aField( pType, aScriptType,
                            !aScriptURL.isEmpty() ? aScriptURL : aScriptSource,
                            !aScriptURL.isEmpty() );
        InsertAttr( SwFormatField( aField ) );
    }

    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !aScriptSource.isEmpty() && pDocSh &&
        bInsIntoBasic && IsNewDoc() )
    {
    // Fuer JavaScript und StarBasic noch ein Basic-Modul anlegen
        // Das Basic entfernt natuerlich weiterhin keine SGML-Kommentare
        RemoveSGMLComment( aScriptSource, true );

        // get library name
        OUString aLibName;
        if( !aBasicLib.isEmpty() )
            aLibName = aBasicLib;
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
                if( aBasicModule.isEmpty() )
                {
                    // create module name
                    bool bFound = true;
                    while( bFound )
                    {
                        aBasicModule = "Modul";
                        aBasicModule += OUString::number( (sal_Int32)(++nSBModuleCnt) );
                        bFound = xModLib->hasByName( OUString( aBasicModule ) );
                    }
                }

                // create module
                OUString aModName( aBasicModule );
                if ( !xModLib->hasByName( aModName ) )
                {
                    Any aElement;
                    aElement <<= OUString( aScriptSource );
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

    aScriptSource.clear();
    aScriptType.clear();
    aScriptURL.clear();

    aBasicLib.clear();
    aBasicModule.clear();
}

void SwHTMLParser::AddScriptSource()
{
    // Hier merken wir und nur ein par Strings
    if( aToken.getLength() > 2 &&
        (HTML_SL_STARBASIC==eScriptLang && aToken[ 0 ] == '\'') )
    {
        sal_Int32 nPos = -1;
        if( aBasicLib.isEmpty() )
        {
            nPos = aToken.indexOf( OOO_STRING_SVTOOLS_HTML_SB_library );
            if( nPos != -1 )
            {
                aBasicLib =
                    aToken.copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_library) - 1 );
                aBasicLib = comphelper::string::strip(aBasicLib, ' ');
            }
        }

        if( aBasicModule.isEmpty() && nPos == -1 )
        {
            nPos = aToken.indexOf( OOO_STRING_SVTOOLS_HTML_SB_module );
            if( nPos != -1 )
            {
                aBasicModule =
                    aToken.copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_module) - 1 );
                aBasicModule = comphelper::string::strip(aBasicModule, ' ');
            }
        }

        if( nPos == -1 )
        {
            if( !aScriptSource.isEmpty() )
                aScriptSource += "\n";
            aScriptSource += aToken;
        }
    }
    else if( !aScriptSource.isEmpty() || !aToken.isEmpty() )
    {
        // Leerzeilen am Anfang werden ignoriert
        if( !aScriptSource.isEmpty() )
        {
            aScriptSource += "\n";
        }
        else
        {
            // Wir stehen hinter dem CR/LF der Zeile davor
            nScriptStartLineNr = GetLineNr() - 1;
        }
        aScriptSource += aToken;
    }
}

void SwHTMLParser::InsertBasicDocEvent( const OUString& aEvent, const OUString& rName,
                                        ScriptType eScrType,
                                        const OUString& rScrType )
{
    OSL_ENSURE( !rName.isEmpty(), "InsertBasicDocEvent() ohne Macro gerufen" );
    if( rName.isEmpty() )
        return;

    SwDocShell *pDocSh = pDoc->GetDocShell();
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
    if( !bCfgStarBasic )
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
                Strm().WriteCharPtr( OUStringToOString(sLang, eDestEnc).getStr() )
                   .WriteCharPtr( "\">" );
            }

            const OUString& rModName = rModule.GetName();
            Strm().WriteCharPtr( SAL_NEWLINE_STRING );   // nicht einruecken!
            HTMLOutFuncs::OutScript( Strm(), GetBaseURL(), rModule.GetSource(),
                                     sLang, eType, aEmptyOUStr,
                                     &rLibName, &rModName,
                                     eDestEnc, &aNonConvertableCharacters );
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
                                  bCfgStarBasic, eDestEnc, &aNonConvertableCharacters );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
