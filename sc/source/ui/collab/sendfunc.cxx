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
#include "collab.hxx"
#include <tubes/conference.hxx>

// new file send/recv fun ...
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>

namespace css = ::com::sun::star;

namespace {

rtl::OUString cellToString( ScBaseCell *pCell )
{
    (void)pCell; // FIXME: implement me
    return rtl::OUString();
}

ScBaseCell *stringToCell( const rtl::OUString &rString )
{
    (void)rString; // FIXME: implement me
    return NULL;
}


// Ye noddy mangling - needs improvement ...
// method name ';' then arguments ; separated
class ScChangeOpWriter
{
  rtl::OUStringBuffer aMessage;
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

  void appendString( const rtl::OUString &rStr )
  {
      if ( rStr.indexOf( sal_Unicode( '"' ) ) >= 0 ||
           rStr.indexOf( sal_Unicode( ';' ) ) >= 0 )
      {
          String aQuoted( rStr );
          ScGlobal::AddQuotes( aQuoted, sal_Unicode( '"' ) );
          aMessage.append( aQuoted );
      }
      else
          aMessage.append( rStr );
      appendSeparator();
  }

  void appendAddress( const ScAddress &rPos )
  {
      rtl::OUString aStr;
      rPos.Format( aStr, SCA_VALID );
      aMessage.append( aStr );
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

  void appendCell( ScBaseCell *pCell )
  {
      appendString( cellToString( pCell ) );
  }

  rtl::OString toString()
  {
      return rtl::OUStringToOString( aMessage.toString(), RTL_TEXTENCODING_UTF8 );
  }
};

struct ProtocolError {
    const char *message;
};

class ScChangeOpReader {
    std::vector< rtl::OUString > maArgs;

public:

    ScChangeOpReader( const rtl::OUString &rString)
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

        for (size_t i = 0; i < maArgs.size(); i++)
            fprintf( stderr, "arg %d: '%s'\n", (int)i,
                     rtl::OUStringToOString( maArgs[i], RTL_TEXTENCODING_UTF8).getStr() );
    }
    ~ScChangeOpReader() {}

    rtl::OUString getMethod()
    {
        return maArgs[0];
    }

    size_t getArgCount() { return maArgs.size(); }

    rtl::OUString getString( sal_Int32 n )
    {
        if (n > 0 && (size_t)n < getArgCount() )
        {
            String aUStr( maArgs[ n ] );
            ScGlobal::EraseQuotes( aUStr );
            return aUStr;
        } else
            return rtl::OUString();
    }

    ScAddress getAddress( sal_Int32 n )
    {
        ScAddress aAddr;
        rtl::OUString aToken( getString( n ) );
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

    ScBaseCell *getCell( sal_Int32 n )
    {
        return stringToCell( getString( n ) );
    }
};


class ScDocFuncRecv : public ScDocFunc
{
    ScDocFunc *mpChain;
    ScCollaboration* mpCollab;
public:
    // FIXME: really ScDocFunc should be an abstract base
    ScDocFuncRecv( ScDocShell& rDocSh, ScDocFunc *pChain )
        : ScDocFunc( rDocSh ),
          mpChain( pChain ),
          mpCollab( NULL)
    {
        fprintf( stderr, "Receiver created !\n" );
    }
    virtual ~ScDocFuncRecv() {}

    void SetCollaboration( ScCollaboration* pCollab )
    {
        mpCollab = pCollab;
    }

    DECL_LINK( ReceiverCallback, TeleConference* );
    DECL_LINK( ReceiveFileCallback, rtl::OUString * );

    void RecvMessage( const rtl::OString &rString )
    {
        try {
            ScChangeOpReader aReader( rtl::OUString( rString.getStr(),
                                                     rString.getLength(),
                                                     RTL_TEXTENCODING_UTF8 ) );
            // FIXME: have some hash to enumeration mapping here
            if ( aReader.getMethod() == "setNormalString" )
                mpChain->SetNormalString( aReader.getAddress( 1 ), aReader.getString( 2 ),
                                          aReader.getBool( 3 ) );
            else if ( aReader.getMethod() == "putCell" )
            {
                ScBaseCell *pNewCell = aReader.getCell( 2 );
                if ( pNewCell )
                    mpChain->PutCell( aReader.getAddress( 1 ), pNewCell, aReader.getBool( 3 ) );
            }
            else if ( aReader.getMethod() == "enterListAction" )
                mpChain->EnterListAction( aReader.getInt( 1 ) );
            else if ( aReader.getMethod() == "endListAction" )
                mpChain->EndListAction();
            else if ( aReader.getMethod() == "showNote" )
                mpChain->ShowNote( aReader.getAddress( 1 ), aReader.getBool( 2 ) );
            else if ( aReader.getMethod() == "setNoteText" )
                mpChain->SetNoteText( aReader.getAddress( 1 ), aReader.getString( 2 ),
                                      aReader.getBool( 3 ) );
            else if ( aReader.getMethod() == "renameTable" )
                mpChain->RenameTable( aReader.getInt( 1 ), aReader.getString( 2 ),
                                      aReader.getBool( 3 ), aReader.getBool( 4 ) );
            else
                fprintf( stderr, "Error: unknown message '%s' (%d)\n",
                         rString.getStr(), (int)aReader.getArgCount() );
        } catch (const ProtocolError &e) {
            fprintf( stderr, "Error: protocol twisting '%s'\n", e.message );
        }
    }
};

IMPL_LINK( ScDocFuncRecv, ReceiverCallback, TeleConference*, pConference )
{
    rtl::OString aStr;
    if (mpCollab && mpCollab->recvPacket( aStr, pConference))
        RecvMessage( aStr);
    return 0;
}

IMPL_LINK( ScDocFuncRecv, ReceiveFileCallback, rtl::OUString *, pStr )
{
    fprintf( stderr, "incoming file '%s'\n",
             rtl::OUStringToOString( *pStr, RTL_TEXTENCODING_UTF8 ).getStr() );

    css::uno::Sequence < css::beans::PropertyValue > aLoadArgs(5);
    aLoadArgs[0].Name = rtl::OUString( "URL" );
    aLoadArgs[0].Value <<= (*pStr);
    aLoadArgs[1].Name = rtl::OUString( "FilterName" );
    aLoadArgs[1].Value <<= rtl::OUString( "calc8" );
    aLoadArgs[2].Name = rtl::OUString( "Referer" );
    aLoadArgs[2].Value <<= rtl::OUString( "" );
    // no interaction handler ?
    aLoadArgs[3].Name = rtl::OUString( "MacroExecutionMode" );
    aLoadArgs[3].Value <<= sal_Int32( 3 );
    aLoadArgs[4].Name = rtl::OUString( "UpdateDocMode" );
    aLoadArgs[4].Value <<= sal_Int32( 2 );

    try
    {
        css::uno::Reference < css::frame::XLoadable > xLoad(
                rDocShell.GetBaseModel(), css::uno::UNO_QUERY_THROW );
        xLoad->load( aLoadArgs );
    }
    catch ( css::uno::Exception& )
    {
        fprintf( stderr, "exception when loading !\n" );
    }

    return 0;
}

class ScDocFuncSend : public ScDocFunc
{
    ScDocFuncRecv *mpChain;
    ScCollaboration* mpCollab;

    void SendMessage( ScChangeOpWriter &rOp )
    {
        fprintf( stderr, "Op: '%s'\n", rOp.toString().getStr() );
        if (mpCollab)
            mpCollab->sendPacket( rOp.toString());
        else
            mpChain->RecvMessage( rOp.toString() );
    }

    void SendFile( const rtl::OUString &rURL )
    {
        (void)rURL;

        String aTmpPath = utl::TempFile::CreateTempName();
        aTmpPath.Append( rtl::OUString( ".ods" ) );

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

        mpCollab->sendFile( aFileURL );
    }

public:
    // FIXME: really ScDocFunc should be an abstract base, so
    // we don't need the rDocSh hack/pointer
    ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncRecv *pChain )
            : ScDocFunc( rDocSh ),
            mpChain( pChain ),
            mpCollab( NULL)
    {
        fprintf( stderr, "Sender created !\n" );
    }
    virtual ~ScDocFuncSend() {}

    void SetCollaboration( ScCollaboration* pCollab )
    {
        mpCollab = pCollab;
    }

    virtual void EnterListAction( sal_uInt16 nNameResId )
    {
        // Want to group these operations for the other side ...
        String aUndo( ScGlobal::GetRscString( nNameResId ) );
        ScChangeOpWriter aOp( "enterListAction" );
        aOp.appendInt( nNameResId ); // nasty but translate-able ...
        SendMessage( aOp );
    }
    virtual void EndListAction()
    {
        ScChangeOpWriter aOp( "endListAction" );
        SendMessage( aOp );
    }

    virtual sal_Bool SetNormalString( const ScAddress& rPos, const String& rText, sal_Bool bApi )
    {
        ScChangeOpWriter aOp( "setNormalString" );
        aOp.appendAddress( rPos );
        aOp.appendString( rText );
        aOp.appendBool( bApi );
        SendMessage( aOp );

        if ( rtl::OUString( rText ) == "saveme" )
            SendFile( rText );

        return true; // needs some code auditing action
    }

    virtual sal_Bool PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi )
    {
        fprintf( stderr, "put cell '%p' type %d %d\n", pNewCell, pNewCell->GetCellType(), bApi );
        ScChangeOpWriter aOp( "putCell" );
        aOp.appendAddress( rPos );
        aOp.appendCell( pNewCell );
        aOp.appendBool( bApi );
        SendMessage( aOp );
        return true; // needs some code auditing action
    }

    virtual sal_Bool PutData( const ScAddress& rPos, ScEditEngineDefaulter& rEngine,
                              sal_Bool bInterpret, sal_Bool bApi )
    {
        fprintf( stderr, "put data\n" );
        return mpChain->PutData( rPos, rEngine, bInterpret, bApi );
    }

    virtual sal_Bool SetCellText( const ScAddress& rPos, const String& rText,
                                  sal_Bool bInterpret, sal_Bool bEnglish, sal_Bool bApi,
                                  const String& rFormulaNmsp,
                                  const formula::FormulaGrammar::Grammar eGrammar )
    {
        fprintf( stderr, "set cell text '%s'\n",
                 rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ).getStr() );
        return mpChain->SetCellText( rPos, rText, bInterpret, bEnglish, bApi, rFormulaNmsp, eGrammar );
    }

    virtual bool ShowNote( const ScAddress& rPos, bool bShow = true )
    {
        ScChangeOpWriter aOp( "showNote" );
        aOp.appendAddress( rPos );
        aOp.appendBool( bShow );
        SendMessage( aOp );
        return true; // needs some code auditing action
    }

    virtual bool SetNoteText( const ScAddress& rPos, const String& rNoteText, sal_Bool bApi )
    {
        ScChangeOpWriter aOp( "setNoteText" );
        aOp.appendAddress( rPos );
        aOp.appendString( rNoteText );
        aOp.appendBool( bApi );
        SendMessage( aOp );
        return true; // needs some code auditing action
    }

    virtual sal_Bool RenameTable( SCTAB nTab, const String& rName,
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

    virtual sal_Bool ApplyAttributes( const ScMarkData& rMark, const ScPatternAttr& rPattern,
                                      sal_Bool bRecord, sal_Bool bApi )
    {
        fprintf( stderr, "Apply Attributes\n" );
        return mpChain->ApplyAttributes( rMark, rPattern, bRecord, bApi );
    }

    virtual sal_Bool ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                 sal_Bool bRecord, sal_Bool bApi )
    {
        fprintf( stderr, "Apply Style '%s'\n",
                 rtl::OUStringToOString( rStyleName, RTL_TEXTENCODING_UTF8 ).getStr() );
        return mpChain->ApplyStyle( rMark, rStyleName, bRecord, bApi );
    }

    virtual sal_Bool MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                                 sal_Bool bRecord, sal_Bool bApi )
    {
        fprintf( stderr, "Merge cells\n" );
        return mpChain->MergeCells( rOption, bContents, bRecord, bApi );
    }
};

} // anonymous namespace

SC_DLLPRIVATE ScDocFunc *ScDocShell::CreateDocFunc()
{
    // FIXME: the chains should be auto-ptrs, so should be collab
    const char* pEnv;
    if (getenv ("INTERCEPT"))
        return new ScDocFuncSend( *this, new ScDocFuncRecv( *this, new ScDocFuncDirect( *this ) ) );
    else if ((pEnv = getenv ("LIBO_TUBES")) != NULL)
    {
        ScDocFuncRecv* pReceiver = new ScDocFuncRecv( *this, new ScDocFuncDirect( *this ) );
        ScDocFuncSend* pSender = new ScDocFuncSend( *this, pReceiver );
        bool bOk = true;
        ScCollaboration* pCollab = new ScCollaboration(
                LINK( pReceiver, ScDocFuncRecv, ReceiverCallback),
                LINK( pReceiver, ScDocFuncRecv, ReceiveFileCallback) );
        bOk = bOk && pCollab->initManager();
        bOk = bOk && pCollab->initAccountContact();
        if (!strcmp( pEnv, "master"))
            bOk = bOk && pCollab->startCollaboration();
        if (bOk)
        {
            pReceiver->SetCollaboration( pCollab);
            pSender->SetCollaboration( pCollab);
        }
        else
        {
            fprintf( stderr, "Could not start collaboration.\n");
            delete pCollab;
        }
        return pSender;
    }
    else
        return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
