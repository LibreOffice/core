/*************************************************************************
 *
 *  $RCSfile: htmlbas.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:41:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX //autogen
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif



#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif

#ifndef _SFX_OBJUNO_HXX //autogen
#include <sfx2/objuno.hxx>
#endif

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
    { sHTML_O_SDonload,     sHTML_O_onload,     SFX_EVENT_OPENDOC   },
    { sHTML_O_SDonunload,   sHTML_O_onunload,   SFX_EVENT_PREPARECLOSEDOC   },
    { sHTML_O_SDonfocus,    sHTML_O_onfocus,    SFX_EVENT_ACTIVATEDOC   },
    { sHTML_O_SDonblur,     sHTML_O_onblur,     SFX_EVENT_DEACTIVATEDOC },
    { 0,                    0,                  0                   }
};


void SwHTMLParser::NewScript()
{
    ParseScriptOptions( aScriptType, eScriptLang, aScriptURL,
                        aBasicLib, aBasicModule );

    if( aScriptURL.Len() )
    {
        // Den Inhalt des Script-Tags ignorieren
        bIgnoreRawData = TRUE;
    }
}

void SwHTMLParser::EndScript()
{
    BOOL bInsIntoBasic = FALSE,
         bInsSrcIntoFld = FALSE;

    switch( eScriptLang )
    {
    case HTML_SL_STARBASIC:
        bInsIntoBasic = TRUE;
        break;
    default:
        bInsSrcIntoFld = TRUE;
        break;
    }

    bIgnoreRawData = FALSE;
    aScriptSource.ConvertLineEnd();

//  MIB 23.5.97: SGML-Kommentare brauchen nicht mehr entfernt zu werden,
//  weil JS das jetzt selber kann.
//  RemoveSGMLComment( aScriptSource, TRUE );

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
        RemoveSGMLComment( aScriptSource, TRUE );

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
                    BOOL bFound = TRUE;
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
            nPos = aToken.SearchAscii( sHTML_SB_library );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicLib =
                    aToken.Copy( nPos + sizeof(sHTML_SB_library) - 1 );
                aBasicLib.EraseLeadingChars().EraseTrailingChars();
            }
        }

        if( !aBasicModule.Len() && nPos==STRING_NOTFOUND )
        {
            nPos = aToken.SearchAscii( sHTML_SB_module );
            if( nPos != STRING_NOTFOUND )
            {
                aBasicModule =
                    aToken.Copy( nPos + sizeof(sHTML_SB_module) - 1 );
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

void SwHTMLParser::InsertBasicDocEvent( USHORT nEvent, const String& rName,
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

    SfxEventConfiguration* pECfg = SFX_APP()->GetEventConfig();
    pECfg->ConfigureEvent( nEvent, SvxMacro( sEvent, sScriptType, eScrType ),
                           pDocSh );
}

void SwHTMLWriter::OutBasic()
{
    if( !bCfgStarBasic )
        return;

    SFX_APP()->EnterBasicCall();

    BasicManager *pBasicMan = pDoc->GetDocShell()->GetBasicManager();
    ASSERT( pBasicMan, "Wo ist der Basic-Manager?" );
    //JP 17.07.96: Bug 29538 - nur das DocumentBasic schreiben
    if( !pBasicMan || pBasicMan == SFX_APP()->GetBasicManager() )
    {
        SFX_APP()->LeaveBasicCall();
        return;
    }

    // und jetzt alle StarBasic-Module und alle unbenutzen JavaSrript-Module
    // ausgeben
    for( USHORT i=0; i<pBasicMan->GetLibCount(); i++ )
    {
        StarBASIC *pBasic = pBasicMan->GetLib( i  );
        const String& rLibName = pBasic->GetName();

        SbxArray *pModules = pBasic->GetModules();
        for( USHORT j=0; j<pModules->Count(); j++ )
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
                sOut.Append( sHTML_meta );
                sOut.Append( ' ' );
                sOut.Append( sHTML_O_httpequiv );
                  sOut.Append( "=\"" );
                sOut.Append( sHTML_META_content_script_type );
                sOut.Append( "\" " );
                sOut.Append( sHTML_O_content );
                sOut.Append( "=\"text/x-" );
                Strm() << sOut.GetBuffer();
                // Entities aren't welcome here
                ByteString sLang8( sLang, eDestEnc );
                Strm() << sLang8.GetBuffer() << "\">";
            }

            const String& rModName = pModule->GetName();
            Strm() << SwHTMLWriter::sNewLine;   // nicht einruecken!
            HTMLOutFuncs::OutScript( Strm(), pModule->GetSource(),
                                     sLang, eType, aEmptyStr,
                                     &rLibName, &rModName,
                                     eDestEnc, &aNonConvertableCharacters );
        }
    }

    SFX_APP()->LeaveBasicCall();
}


void SwHTMLWriter::OutBasicBodyEvents()
{
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if( !pDocSh )
        return;

    SfxEventConfiguration* pECfg = SFX_APP()->GetEventConfig();
    ASSERT( pECfg, "Wo ist die Event-Konfiguration?" );
    if( !pECfg )
        return;

    SvxMacroTableDtor *pMacTable = pECfg->GetDocEventTable( pDocSh );
    if( pMacTable && pMacTable->Count() )
        HTMLOutFuncs::Out_Events( Strm(), *pMacTable, aBodyEventTable,
                                  bCfgStarBasic, eDestEnc, &aNonConvertableCharacters );
}


