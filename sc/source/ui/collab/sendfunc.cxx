/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sendfunc.hxx"

#include "formulacell.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include "sccollaboration.hxx"
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <tubes/manager.hxx>

namespace {

OUString formulaCellToString( ScFormulaCell *pCell )
{
    (void)pCell; // FIXME: implement me
    return OUString();
}

OUString editToString( const EditTextObject& /*rEditText*/ )
{
    // FIXME: implement me.
    return OUString();
}

EditTextObject stringToEdit( const OUString& /* rStr */ )
{
    // FIXME: implement me.
    // The code here only serves to make this file compilable.
    EditEngine aEditEngine(0);
    return *aEditEngine.CreateTextObject();
}

ScFormulaCell* stringToFormulaCell( const OUString &rString )
{
    (void)rString; // FIXME: implement me
    return NULL;
}

struct ProtocolError {
    const char *message;
};

class ScChangeOpReader {
    std::vector< OUString > maArgs;

public:
    ScChangeOpReader( const OUString &rString)
    {
        // will need to handle escaping etc.
        // Surely someone else wrote this before ! [!?]
        enum {
            IN_TEXT, CHECK_QUOTE, FIND_LAST_QUOTE, SKIP_SEMI
        } eState = CHECK_QUOTE;

        sal_Int32 nStart = 0;
        for (sal_Int32 n = 0; n < rString.getLength(); n++)
        {
            if (rString[n] == '\\')
            {
                n++; // skip next char
                continue;
            }
            switch (eState) {
            case CHECK_QUOTE:
                if (rString[n] == '"')
                {
                    nStart = n + 1;
                    eState = FIND_LAST_QUOTE;
                    break;
                }
                // else drop through
            case IN_TEXT:
                if (rString[n] == ';')
                {
                    maArgs.push_back( rString.copy( nStart, n - nStart ) );
                    nStart = n + 1;
                    eState = CHECK_QUOTE;
                }
                break;
            case FIND_LAST_QUOTE:
                if (rString[n] == '"')
                {
                    maArgs.push_back( rString.copy( nStart, n - nStart ) );
                    eState = SKIP_SEMI;
                    break;
                }
                break;
            case SKIP_SEMI:
                if (rString[n] == ';')
                {
                    nStart = n + 1;
                    eState = CHECK_QUOTE;
                }
                break;
            }
        }
        if ( nStart < rString.getLength())
            maArgs.push_back( rString.copy( nStart, rString.getLength() - nStart ) );
    }
    ~ScChangeOpReader() {}

    OUString getMethod()
    {
        return maArgs[0];
    }

    size_t getArgCount() { return maArgs.size(); }

    OUString getString( sal_Int32 n )
    {
        if (n > 0 && (size_t)n < getArgCount() )
        {
            OUString aUStr( maArgs[ n ] );
            ScGlobal::EraseQuotes( aUStr );
            return aUStr;
        } else
            return OUString();
    }

    ScAddress getAddress( sal_Int32 n )
    {
        ScAddress aAddr;
        OUString aToken( getString( n ) );
        aAddr.Parse( aToken );
        return aAddr;
    }

    sal_Int32 getInt( sal_Int32 n )
    {
        return getString( n ).toInt32();
    }

    bool getBool( sal_Int32 n )
    {
        return getString( n ).equalsIgnoreAsciiCase( "true" );
    }

    ScFormulaCell* getFormulaCell( sal_Int32 n )
    {
        return stringToFormulaCell( getString( n ) );
    }

    double getDouble( sal_Int32 n )
    {
        return getString(n).toDouble();
    }

    EditTextObject getEdit( sal_Int32 n )
    {
        return stringToEdit(getString(n));
    }

};

} // anonymous namespace

// Ye noddy mangling - needs improvement ...
// method name ';' then arguments ; separated
class ScChangeOpWriter
{
    OUStringBuffer aMessage;
    void appendSeparator()
    {
        aMessage.append( sal_Unicode( ';' ) );
    }

public:
    ScChangeOpWriter( const char *pName )
    {
        aMessage.appendAscii( pName );
        appendSeparator();
    }

    void appendString( const OUString &rStr )
    {
        if ( rStr.indexOf( sal_Unicode( '"' ) ) >= 0 ||
             rStr.indexOf( sal_Unicode( ';' ) ) >= 0 )
        {
            OUString aQuoted( rStr );
            ScGlobal::AddQuotes( aQuoted, sal_Unicode( '"' ) );
            aMessage.append( aQuoted );
        }
        else
            aMessage.append( rStr );
        appendSeparator();
    }

    void appendAddress( const ScAddress &rPos )
    {
        aMessage.append( rPos.Format( SCA_VALID ) );
        appendSeparator();
    }

    void appendInt( sal_Int32 i )
    {
        aMessage.append( i );
        appendSeparator();
    }

    void appendBool( sal_Bool b )
    {
        aMessage.appendAscii( b ? "true" : "false" );
        appendSeparator();
    }

    void appendFormulaCell( ScFormulaCell *pCell )
    {
        appendString( formulaCellToString( pCell ) );
    }

    void appendEditText( const EditTextObject& rStr )
    {
        appendString( editToString(rStr) );
    }

    void appendDouble( double fVal )
    {
        aMessage.append(fVal);
        appendSeparator();
    }

    OString toString()
    {
        return OUStringToOString( aMessage.toString(), RTL_TEXTENCODING_UTF8 );
    }
};

void ScDocFuncSend::RecvMessage( const OString &rString )
{
    try {
        ScChangeOpReader aReader( OUString( rString.getStr(),
                                                 rString.getLength(),
                                                 RTL_TEXTENCODING_UTF8 ) );
        // FIXME: have some hash to enumeration mapping here
        if ( aReader.getMethod() == "setNormalString" )
        {
            bool bNumFmtSet = false;
            mpDirect->SetNormalString( bNumFmtSet, aReader.getAddress( 1 ), aReader.getString( 2 ),
                                      aReader.getBool( 3 ) );
        }
        else if (aReader.getMethod() == "setValueCell")
        {
            mpDirect->SetValueCell(
                aReader.getAddress(1), aReader.getDouble(2), aReader.getBool(3));
        }
        else if (aReader.getMethod() == "setStringCell")
        {
            mpDirect->SetStringCell(
                aReader.getAddress(1), aReader.getString(2), aReader.getBool(3));
        }
        else if (aReader.getMethod() == "setEditCell")
        {
            mpDirect->SetEditCell(
                aReader.getAddress(1), aReader.getEdit(2), aReader.getBool(3));
        }
        else if (aReader.getMethod() == "setFormulaCell")
        {
            mpDirect->SetFormulaCell(
                aReader.getAddress(1), aReader.getFormulaCell(2), aReader.getBool(3));
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
            SAL_WARN( "sc.tubes", "Error: unknown message '" << rString.getStr()
                    << "' (" << aReader.getArgCount() << ")" );
    } catch (const ProtocolError &e) {
        SAL_WARN( "sc.tubes", "Error: protocol twisting: " << e.message );
    }
}

void ScDocFuncSend::SendMessage( ScChangeOpWriter &rOp )
{
    mpCollaboration->SendPacket( rOp.toString() );
}

// FIXME: really ScDocFunc should be an abstract base, so
// we don't need the rDocSh hack/pointer
ScDocFuncSend::ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncDirect *pDirect, ScCollaboration* pCollaboration )
        : ScDocFunc( rDocSh ),
        mpDirect( pDirect ),
        mpCollaboration( pCollaboration )
{
}

ScDocFuncSend::~ScDocFuncSend()
{
    delete mpDirect;
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

    if ( OUString( rText ) == "saveme" )
        mpCollaboration->SaveAndSendFile( NULL );

    if ( OUString( rText ) == "contacts" )
        mpCollaboration->DisplayContacts();

    return true; // needs some code auditing action
}

bool ScDocFuncSend::SetValueCell( const ScAddress& rPos, double fVal, bool bInteraction )
{
    ScChangeOpWriter aOp("setValueCell");
    aOp.appendAddress( rPos );
    aOp.appendDouble( fVal );
    aOp.appendBool( bInteraction );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

bool ScDocFuncSend::SetStringCell( const ScAddress& rPos, const OUString& rStr, bool bInteraction )
{
    ScChangeOpWriter aOp("setStringCell");
    aOp.appendAddress( rPos );
    aOp.appendString( rStr );
    aOp.appendBool( bInteraction );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

bool ScDocFuncSend::SetEditCell( const ScAddress& rPos, const EditTextObject& rStr, bool bInteraction )
{
    ScChangeOpWriter aOp("setEditCell");
    aOp.appendAddress( rPos );
    aOp.appendEditText( rStr );
    aOp.appendBool( bInteraction );
    SendMessage( aOp );
    return true; // needs some code auditing action
}

bool ScDocFuncSend::SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell, bool bInteraction )
{
    ScChangeOpWriter aOp("setFormulaCell");
    aOp.appendAddress( rPos );
    aOp.appendFormulaCell( pCell );
    aOp.appendBool( bInteraction );
    SendMessage( aOp );
    delete pCell;
    return true; // needs some code auditing action
}

bool ScDocFuncSend::PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine, bool bApi )
{
    SAL_INFO( "sc.tubes", "PutData not implemented!" );
    return ScDocFunc::PutData( rPos, rEngine, bApi );
}

bool ScDocFuncSend::SetCellText(
    const ScAddress& rPos, const OUString& rText, bool bInterpret, bool bEnglish, bool bApi,
    const formula::FormulaGrammar::Grammar eGrammar )
{
    SAL_INFO( "sc.tubes", "SetCellText not implemented!" );
    return ScDocFunc::SetCellText( rPos, rText, bInterpret, bEnglish, bApi, eGrammar );
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
    SAL_INFO( "sc.tubes", "ApplyAttributes not implemented!" );
    return ScDocFunc::ApplyAttributes( rMark, rPattern, bRecord, bApi );
}

sal_Bool ScDocFuncSend::ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                             sal_Bool bRecord, sal_Bool bApi )
{
    SAL_INFO( "sc.tubes", "ApplyStyle not implemented!" );
    return ScDocFunc::ApplyStyle( rMark, rStyleName, bRecord, bApi );
}

sal_Bool ScDocFuncSend::MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                             sal_Bool bRecord, sal_Bool bApi )
{
    SAL_INFO( "sc.tubes", "MergeCells not implemented!" );
    return ScDocFunc::MergeCells( rOption, bContents, bRecord, bApi );
}

ScDocFunc *ScDocShell::CreateDocFunc()
{
    if (TeleManager::hasWaitingConference())
    {
        TeleConference* pConference = TeleManager::getConference();
        if (pConference)
        {
            mpCollaboration->SetConference( pConference );
            return new ScDocFuncSend( *this, new ScDocFuncDirect( *this ), mpCollaboration );
        }
    }
    return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
