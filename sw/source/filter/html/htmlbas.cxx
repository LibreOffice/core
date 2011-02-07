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
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#include <sfx2/sfx.hrc>

#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
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
#include "docsh.hxx"
#include "docufld.hxx"
#include "wrthtml.hxx"
#include "swhtml.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;


static HTMLOutEvent __FAR_DATA aBodyEventTable[] =
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

    if( aScriptURL.Len() )
    {
        // Den Inhalt des Script-Tags ignorieren
        bIgnoreRawData = sal_True;
    }
}

void SwHTMLParser::EndScript()
{
    sal_Bool bInsIntoBasic = sal_False,
         bInsSrcIntoFld = sal_False;

    switch( eScriptLang )
    {
    case HTML_SL_STARBASIC:
        bInsIntoBasic = sal_True;
        break;
    default:
        bInsSrcIntoFld = sal_True;
        break;
    }

    bIgnoreRawData = sal_False;
    aScriptSource.ConvertLineEnd();

//  MIB 23.5.97: SGML-Kommentare brauchen nicht mehr entfernt zu werden,
//  weil JS das jetzt selber kann.
//  RemoveSGMLComment( aScriptSource, sal_True );

    // Ausser StarBasic und unbenutzem JavaScript jedes Script oder den
    // Modulnamen in einem Feld merken merken
    if( bInsSrcIntoFld && !bIgnoreHTMLComments )
    {
        SwScriptFieldType *pType =
            (SwScriptFieldType*)pDoc->GetSysFldType( RES_SCRIPTFLD );

        SwScriptField aFld( pType, aScriptType,
                            aScriptURL.Len() ? aScriptURL : aScriptSource,
                            aScriptURL.Len()!=0 );
        InsertAttr( SwFmtFld( aFld ) );
    }

    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( aScriptSource.Len() && pDocSh &&
        bInsIntoBasic && IsNewDoc() )
    {
    // Fuer JavaScript und StarBasic noch ein Basic-Modul anlegen
        // Das Basic entfernt natuerlich weiterhin keine SGML-Kommentare
        RemoveSGMLComment( aScriptSource, sal_True );

        // get library name
        ::rtl::OUString aLibName;
        if( aBasicLib.Len() )
            aLibName = aBasicLib;
        else
            aLibName = ::rtl::OUString::createFromAscii( "Standard" );

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
                if( !aBasicModule.Len() )
                {
                    // create module name
                    sal_Bool bFound = sal_True;
                    while( bFound )
                    {
                        aBasicModule.AssignAscii( "Modul" );
                        aBasicModule += String::CreateFromInt32( (sal_Int32)(++nSBModuleCnt) );
                        bFound = xModLib->hasByName( ::rtl::OUString( aBasicModule ) );
                    }
                }

                // create module
                ::rtl::OUString aModName( aBasicModule );
                if ( !xModLib->hasByName( aModName ) )
                {
                    Any aElement;
                    aElement <<= ::rtl::OUString( aScriptSource );
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

    aScriptSource.Erase();
    aScriptType.Erase();
    aScriptURL.Erase();

    aBasicLib.Erase();
    aBasicModule.Erase();
}

void SwHTMLParser::AddScriptSource()
{
    // Hier merken wir und nur ein par Strings
    if( aToken.Len() > 2 &&
        (HTML_SL_STARBASIC==eScriptLang && aToken.GetChar( 0 ) == '\'') )
    {
        xub_StrLen nPos = STRING_NOTFOUND;
        if( !aBasicLib.Len() )
        {
            nPos = aToken.SearchAscii( OOO_STRING_SVTOOLS_HTML_SB_library );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicLib =
                    aToken.Copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_library) - 1 );
                aBasicLib.EraseLeadingChars().EraseTrailingChars();
            }
        }

        if( !aBasicModule.Len() && nPos==STRING_NOTFOUND )
        {
            nPos = aToken.SearchAscii( OOO_STRING_SVTOOLS_HTML_SB_module );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicModule =
                    aToken.Copy( nPos + sizeof(OOO_STRING_SVTOOLS_HTML_SB_module) - 1 );
                aBasicModule.EraseLeadingChars().EraseTrailingChars();
            }
        }

        if( nPos==STRING_NOTFOUND )
        {
            if( aScriptSource.Len() )
                aScriptSource += '\n';
            (aScriptSource += aToken);
        }
    }
    else if( aScriptSource.Len() || aToken.Len() )
    {
        // Leerzeilen am Anfang werden ignoriert
        if( aScriptSource.Len() )
        {
            aScriptSource += '\n';
        }
        else
        {
            // Wir stehen hinter dem CR/LF der Zeile davor
            nScriptStartLineNr = GetLineNr() - 1;
        }
        aScriptSource += aToken;
    }
}

void SwHTMLParser::InsertBasicDocEvent( rtl::OUString aEvent, const String& rName,
                                        ScriptType eScrType,
                                        const String& rScrType )
{
    ASSERT( rName.Len(), "InsertBasicDocEvent() ohne Macro gerufen" );
    if( !rName.Len() )
        return;

    SwDocShell *pDocSh = pDoc->GetDocShell();
    ASSERT( pDocSh, "Wo ist die DocShell?" );
    if( !pDocSh )
        return;

    String sEvent( rName );
    sEvent.ConvertLineEnd();
    String sScriptType;
    if( EXTENDED_STYPE == eScrType )
        sScriptType = rScrType;

    rtl::OUString aEventName;

    SfxEventConfiguration::ConfigureEvent( aEvent, SvxMacro( sEvent, sScriptType, eScrType ),
                           pDocSh );
}

void SwHTMLWriter::OutBasic()
{
    if( !bCfgStarBasic )
        return;

    BasicManager *pBasicMan = pDoc->GetDocShell()->GetBasicManager();
    ASSERT( pBasicMan, "Wo ist der Basic-Manager?" );
    //JP 17.07.96: Bug 29538 - nur das DocumentBasic schreiben
    if( !pBasicMan || pBasicMan == SFX_APP()->GetBasicManager() )
    {
        return;
    }

    // und jetzt alle StarBasic-Module und alle unbenutzen JavaSrript-Module
    // ausgeben
    for( sal_uInt16 i=0; i<pBasicMan->GetLibCount(); i++ )
    {
        StarBASIC *pBasic = pBasicMan->GetLib( i  );
        const String& rLibName = pBasic->GetName();

        SbxArray *pModules = pBasic->GetModules();
        for( sal_uInt16 j=0; j<pModules->Count(); j++ )
        {
            const SbModule *pModule = PTR_CAST( SbModule, pModules->Get(j) );
            ASSERT( pModule, "Wo ist das Modul?" );

            String sLang(
                    String::CreateFromAscii( SVX_MACRO_LANGUAGE_STARBASIC ) );
            ScriptType eType = STARBASIC;

            if( 0==i && 0==j )
            {
                OutNewLine();
                ByteString sOut( '<' );
                sOut.Append( OOO_STRING_SVTOOLS_HTML_meta );
                sOut.Append( ' ' );
                sOut.Append( OOO_STRING_SVTOOLS_HTML_O_httpequiv );
                  sOut.Append( "=\"" );
                sOut.Append( OOO_STRING_SVTOOLS_HTML_META_content_script_type );
                sOut.Append( "\" " );
                sOut.Append( OOO_STRING_SVTOOLS_HTML_O_content );
                sOut.Append( "=\"text/x-" );
                Strm() << sOut.GetBuffer();
                // Entities aren't welcome here
                ByteString sLang8( sLang, eDestEnc );
                Strm() << sLang8.GetBuffer() << "\">";
            }

            const String& rModName = pModule->GetName();
            Strm() << SwHTMLWriter::sNewLine;   // nicht einruecken!
            HTMLOutFuncs::OutScript( Strm(), GetBaseURL(), pModule->GetSource(),
                                     sLang, eType, aEmptyStr,
                                     &rLibName, &rModName,
                                     eDestEnc, &aNonConvertableCharacters );
        }
    }
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

    SvxMacroTableDtor *pDocTable = new SvxMacroTableDtor;

    uno::Reference< document::XEventsSupplier > xSup( pDocSh->GetModel(), uno::UNO_QUERY );
    uno::Reference < container::XNameReplace > xEvents = xSup->getEvents();
    for ( sal_Int32 i=0; i<4; i++ )
    {
        SvxMacro* pMacro = SfxEventConfiguration::ConvertToMacro( xEvents->getByName( ::rtl::OUString::createFromAscii(aEventNames[i]) ), pDocSh, sal_True );
        if ( pMacro )
            pDocTable->Insert( aBodyEventTable[i].nEvent, pMacro );
    }

    if( pDocTable && pDocTable->Count() )
        HTMLOutFuncs::Out_Events( Strm(), *pDocTable, aBodyEventTable,
                                  bCfgStarBasic, eDestEnc, &aNonConvertableCharacters );
}


