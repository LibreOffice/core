/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Michael Meeks <michael.meeks@suse.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"

#include <vector>

#include "cell.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "sccollaboration.hxx"
#include "sendfunc.hxx"
#include <tubes/conference.hxx>
#include <tubes/contacts.hxx>
#include <tubes/manager.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/document/XDocumentRecovery.hpp>

namespace css = ::com::sun::star;

void ScDocFuncSend::RecvMessage( const rtl::OString &rString )
{
    try {
        ScChangeOpReader aReader( rtl::OUString( rString.getStr(),
                                                 rString.getLength(),
                                                 RTL_TEXTENCODING_UTF8 ) );
        // FIXME: have some hash to enumeration mapping here
        if ( aReader.getMethod() == "setNormalString" )
        {
            bool bNumFmtSet = false;
            mpDirect->SetNormalString( bNumFmtSet, aReader.getAddress( 1 ), aReader.getString( 2 ),
                                      aReader.getBool( 3 ) );
        }
        else if ( aReader.getMethod() == "putCell" )
        {
            ScBaseCell *pNewCell = aReader.getCell( 2 );
            if ( pNewCell )
                mpDirect->PutCell( aReader.getAddress( 1 ), pNewCell, aReader.getBool( 3 ) );
        }
        else if ( aReader.getMethod() == "enterListAction" )
            mpDirect->EnterListAction( aReader.getInt( 1 ) );
        else if ( aReader.getMethod() == "endListAction" )
            mpDirect->EndListAction();
        else if ( aReader.getMethod() == "showNote" )
            mpDirect->ShowNote( aReader.getAddress( 1 ), aReader.getBool( 2 ) );
        else if ( aReader.getMethod() == "setNoteText" )
            mpDirect->SetNoteText( aReader.getAddress( 1 ), aReader.getString( 2 ),
                                  aReader.getBool( 3 ) );
        else if ( aReader.getMethod() == "renameTable" )
            mpDirect->RenameTable( aReader.getInt( 1 ), aReader.getString( 2 ),
                                  aReader.getBool( 3 ), aReader.getBool( 4 ) );
        else
            fprintf( stderr, "Error: unknown message '%s' (%d)\n",
                     rString.getStr(), (int)aReader.getArgCount() );
    } catch (const ProtocolError &e) {
        fprintf( stderr, "Error: protocol twisting '%s'\n", e.message );
    }
}

extern "C"
{
    static void file_sent_cb( bool aSuccess, void* /* pUserData */ )
    {
        fprintf( stderr, "File send %s\n", aSuccess ? "success" : "failed" );
    }
}

void ScDocFuncSend::SendMessage( ScChangeOpWriter &rOp )
{
    fprintf( stderr, "Op: '%s'\n", rOp.toString().getStr() );
    if (mpConference)
        mpConference->sendPacket( rOp.toString() );
}

void ScDocFuncSend::SendFile( TpContact* pContact, const rtl::OUString &sUuid )
{
    String aTmpPath = utl::TempFile::CreateTempName();
    aTmpPath.Append( OUString("_") );
    aTmpPath.Append( sUuid );
    aTmpPath.Append( OUString("_") );
    aTmpPath.Append( OUString(".ods") );

    rtl::OUString aFileURL;
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTmpPath, aFileURL );

    ::comphelper::MediaDescriptor aDescriptor;
    // some issue with hyperlinks:
    aDescriptor[::comphelper::MediaDescriptor::PROP_DOCUMENTBASEURL()] <<= ::rtl::OUString();
    try {
        css::uno::Reference< css::document::XDocumentRecovery > xDocRecovery(
                    rDocShell.GetBaseModel(), css::uno::UNO_QUERY_THROW);

        xDocRecovery->storeToRecoveryFile( aFileURL, aDescriptor.getAsConstPropertyValueList() );
    } catch (const css::uno::Exception &ex) {
        fprintf( stderr, "exception foo !\n" );
    }

    fprintf( stderr, "Temp file is '%s'\n",
             rtl::OUStringToOString( aFileURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    mpConference->sendFile( pContact, aFileURL, file_sent_cb, NULL );

    // FIXME: unlink the file after send ...
}

// FIXME: really ScDocFunc should be an abstract base, so
// we don't need the rDocSh hack/pointer
ScDocFuncSend::ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncDirect *pDirect )
        : ScDocFunc( rDocSh ),
        mpDirect( pDirect ),
        mpConference( NULL )
{
    fprintf( stderr, "Sender created !\n" );
}

ScDocFuncSend::~ScDocFuncSend()
{
    fprintf( stderr, "Sender destroyed !\n" );
    if (mpConference)
        mpConference->close();

    delete mpDirect;
}

void ScDocFuncSend::SetCollaboration( TeleConference* pConference )
{
    mpConference = pConference;
}

TeleConference* ScDocFuncSend::GetConference()
{
    return mpConference;
}

void ScDocFuncSend::EnterListAction( sal_uInt16 nNameResId )
{
    // Want to group these operations for the other side ...
    String aUndo( ScGlobal::GetRscString( nNameResId ) );
    ScChangeOpWriter aOp( "enterListAction" );
    aOp.appendInt( nNameResId ); // nasty but translate-able ...
    SendMessage( aOp );
}

void ScDocFuncSend::EndListAction()
{
    ScChangeOpWriter aOp( "endListAction" );
    SendMessage( aOp );
}

sal_Bool ScDocFuncSend::SetNormalString( bool& o_rbNumFmtSet, const ScAddress& rPos, const String& rText, sal_Bool bApi )
{
    ScChangeOpWriter aOp( "setNormalString" );
    aOp.appendAddress( rPos );
    aOp.appendString( rText );
    aOp.appendBool( bApi );
    SendMessage( aOp );

    o_rbNumFmtSet = false;

    if ( rtl::OUString( rText ) == "saveme" )
        SendFile( NULL, rText );

    if ( rtl::OUString( rText ) == "contacts" )
        tubes::createContacts( rDocShell.GetCollaboration() );

    return true; // needs some code auditing action
}

sal_Bool ScDocFuncSend::PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi )
{
    fprintf( stderr, "put cell '%p' type %d %d\n", pNewCell, pNewCell->GetCellType(), bApi );
    ScChangeOpWriter aOp( "putCell" );
    aOp.appendAddress( rPos );
    aOp.appendCell( pNewCell );
    aOp.appendBool( bApi );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

sal_Bool ScDocFuncSend::PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine,
                          sal_Bool bInterpret, sal_Bool bApi )
{
    fprintf( stderr, "put data\n" );
    return ScDocFunc::PutData( rPos, rEngine, bInterpret, bApi );
}

sal_Bool ScDocFuncSend::SetCellText( const ScAddress& rPos, const String& rText,
                              sal_Bool bInterpret, sal_Bool bEnglish, sal_Bool bApi,
                              const String& rFormulaNmsp,
                              const formula::FormulaGrammar::Grammar eGrammar )
{
    fprintf( stderr, "set cell text '%s'\n",
             rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ).getStr() );
    return ScDocFunc::SetCellText( rPos, rText, bInterpret, bEnglish, bApi, rFormulaNmsp, eGrammar );
}

bool ScDocFuncSend::ShowNote( const ScAddress& rPos, bool bShow )
{
    ScChangeOpWriter aOp( "showNote" );
    aOp.appendAddress( rPos );
    aOp.appendBool( bShow );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

bool ScDocFuncSend::SetNoteText( const ScAddress& rPos, const String& rNoteText, sal_Bool bApi )
{
    ScChangeOpWriter aOp( "setNoteText" );
    aOp.appendAddress( rPos );
    aOp.appendString( rNoteText );
    aOp.appendBool( bApi );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

sal_Bool ScDocFuncSend::RenameTable( SCTAB nTab, const String& rName,
                              sal_Bool bRecord, sal_Bool bApi )
{
    ScChangeOpWriter aOp( "renameTable" );
    aOp.appendInt( nTab );
    aOp.appendString( rName );
    aOp.appendBool( bRecord );
    aOp.appendBool( bApi );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

sal_Bool ScDocFuncSend::ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                  sal_Bool bRecord, sal_Bool bApi )
{
    fprintf( stderr, "Apply Attributes\n" );
    return ScDocFunc::ApplyAttributes( rMark, rPattern, bRecord, bApi );
}

sal_Bool ScDocFuncSend::ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                             sal_Bool bRecord, sal_Bool bApi )
{
    fprintf( stderr, "Apply Style '%s'\n",
             rtl::OUStringToOString( rStyleName, RTL_TEXTENCODING_UTF8 ).getStr() );
    return ScDocFunc::ApplyStyle( rMark, rStyleName, bRecord, bApi );
}

sal_Bool ScDocFuncSend::MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                             sal_Bool bRecord, sal_Bool bApi )
{
    fprintf( stderr, "Merge cells\n" );
    return ScDocFunc::MergeCells( rOption, bContents, bRecord, bApi );
}

ScDocFunc *ScDocShell::CreateDocFunc()
{
    if (TeleManager::hasWaitingConference())
    {
        ScDocFuncSend *pSender = new ScDocFuncSend( *this, new ScDocFuncDirect( *this ) );
        TeleConference* pConference = TeleManager::getConference();
        pConference->setCollaboration( mpCollaboration );
        pSender->SetCollaboration( pConference );
        return pSender;
    }
    else
        return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
