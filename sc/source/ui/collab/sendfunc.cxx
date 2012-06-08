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

#include <boost/signals2.hpp>
#include <vector>

#include "cell.hxx"
#include "contacts.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"
#include <tubes/manager.hxx>
#include <tubes/conference.hxx>
#include <tubes/contact-list.hxx>

// new file send/recv fun ...
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <com/sun/star/util/XCloseable.hpp>

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

bool isCollabMode( bool& rbMaster )
{
    const char* pEnv = getenv ("LIBO_TUBES");
    if (pEnv)
    {
        rbMaster = !strcmp( pEnv, "master");
        return true;
    }
    rbMaster = false;
    return false;
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


class ScDocFuncRecv
{
    boost::shared_ptr<ScDocFuncDirect>  mpChain;

protected:
    ScDocFuncRecv() {}
public:
    // FIXME: really ScDocFunc should be an abstract base
    ScDocFuncRecv( boost::shared_ptr<ScDocFuncDirect>& pChain )
        : mpChain( pChain )
    {
        fprintf( stderr, "Receiver created !\n" );
    }
    virtual ~ScDocFuncRecv() {}

    void packetReceived( TeleConference*, TelePacket &rPacket );

    virtual void fileReceived( const rtl::OUString &rStr );
    virtual void RecvMessage( const rtl::OString &rString )
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

void ScDocFuncRecv::packetReceived( TeleConference*, TelePacket &rPacket )
{
    rtl::OString aString( rPacket.getData(), rPacket.getSize() );
    RecvMessage( aString );
}

void ScDocFuncRecv::fileReceived( const rtl::OUString &rStr )
{
    fprintf( stderr, "incoming file '%s'\n",
             rtl::OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ).getStr() );

    // using the frame::XLoadable interface fails with a DoubleInitializationException
/*    css::uno::Sequence < css::beans::PropertyValue > aLoadArgs(5);
    aLoadArgs[0].Name = rtl::OUString( "URL" );
    aLoadArgs[0].Value <<= rpStr;
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
    catch ( css::uno::Exception& e )
    {
        fprintf( stderr, "exception when loading '%s' !\n",
                 rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                 } */
// #2 - tried out the SfxAutoReloadTimer_Impl - shove stuff at the SID_RELOAD slot ...

// #3 - can we use the framework/inc/services/frame.hxx 's "sTargetFrameName"
// magic to load into our current frame ? ... :-)

    css::uno::Reference< css::lang::XMultiServiceFactory > rFactory =
        ::comphelper::getProcessServiceFactory();

    css::uno::Sequence < css::beans::PropertyValue > args(0);
// FIXME: should this be hidden before it is synched & ready ? ...
//    args[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
//    args[0].Value <<= sal_True;
    try
    {
        css::uno::Reference < css::frame::XComponentLoader > xLoader(
                ::comphelper::getProcessServiceFactory()->createInstance(
                        "com.sun.star.frame.Desktop" ),
                        css::uno::UNO_QUERY_THROW );
        css::uno::Reference < css::util::XCloseable > xDoc(
                xLoader->loadComponentFromURL( rStr, "_blank", 0, args ),
                css::uno::UNO_QUERY_THROW );
    }
    catch ( css::uno::Exception& e )
    {
        fprintf( stderr, "exception when loading '%s' !\n",
                 rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

/*
 * Provides a local bus that doesn't require an IM channel for
 * quick demoing, export INTERCEPT=demo # to enable.
 */
class ScDocFuncDemo : public ScDocFuncRecv
{
    std::vector< boost::shared_ptr<ScDocFuncRecv> > maClients;
  public:
    // FIXME: really ScDocFuncRecv should be an abstract base
    ScDocFuncDemo()
        : ScDocFuncRecv()
    {
        fprintf( stderr, "Receiver created !\n" );
    }
    virtual ~ScDocFuncDemo() {}

    void add_client (const boost::shared_ptr<ScDocFuncRecv> &aClient)
    {
        maClients.push_back( aClient );
    }

    virtual void RecvMessage( const rtl::OString &rString )
    {
        // FIXME: Lifecycle nightmare
        std::vector< boost::shared_ptr<ScDocFuncRecv> > aCopy( maClients );
        for (std::vector< boost::shared_ptr<ScDocFuncRecv> >::iterator i
                 = aCopy.begin(); i != aCopy.end(); i++)
            (*i)->RecvMessage(rString);
    }

    virtual void fileReceived( const rtl::OUString &rStr )
    {
        // FIXME: Lifecycle nightmare
        std::vector< boost::shared_ptr<ScDocFuncRecv> > aCopy( maClients );
        for (std::vector< boost::shared_ptr<ScDocFuncRecv> >::iterator i
                 = aCopy.begin(); i != aCopy.end(); i++)
            (*i)->fileReceived( rStr );
    }
};

extern "C"
{
    static void file_sent_cb( bool aSuccess, void* /* pUserData */ )
    {
        fprintf( stderr, "File send %s\n", aSuccess ? "success" : "failed" );
    }
}

class ScDocFuncSend : public ScDocFunc
{
    boost::shared_ptr<ScDocFuncRecv>    mpDirect;
    TeleManager                         *mpManager;

    void SendMessage( ScChangeOpWriter &rOp )
    {
        fprintf( stderr, "Op: '%s'\n", rOp.toString().getStr() );
        if (mpManager)
        {
            TelePacket aPacket( "sender", rOp.toString().getStr(), rOp.toString().getLength() );
            mpManager->sendPacket( aPacket );
        }
        else // local demo mode
            mpDirect->RecvMessage( rOp.toString() );
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

        if (mpManager)
            mpManager->sendFile( aFileURL, file_sent_cb, NULL );
        else
            mpDirect->fileReceived( aFileURL );

        // FIXME: unlink the file after send ...
    }

public:
    // FIXME: really ScDocFunc should be an abstract base, so
    // we don't need the rDocSh hack/pointer
    ScDocFuncSend( ScDocShell& rDocSh, boost::shared_ptr<ScDocFuncRecv> pDirect )
            : ScDocFunc( rDocSh ),
            mpDirect( pDirect ),
            mpManager( NULL )
    {
        fprintf( stderr, "Sender created !\n" );
    }
    virtual ~ScDocFuncSend() {}

    void SetCollaboration( bool bIsMaster )
    {
        mpManager = TeleManager::get( !bIsMaster );
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

        if ( rtl::OUString( rText ) == "contacts" )
        {
            // For TeleManager::get() use the same master/slave mode we have
            // for collaboration, if any. This is a hack anyway so don't care
            // whether we really are in collab mode or not.
            bool bIsMaster = false;
            isCollabMode( bIsMaster );
            tubes::createContacts( TeleManager::get( bIsMaster ) );
        }

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
        return ScDocFunc::PutData( rPos, rEngine, bInterpret, bApi );
    }

    virtual sal_Bool SetCellText( const ScAddress& rPos, const String& rText,
                                  sal_Bool bInterpret, sal_Bool bEnglish, sal_Bool bApi,
                                  const String& rFormulaNmsp,
                                  const formula::FormulaGrammar::Grammar eGrammar )
    {
        fprintf( stderr, "set cell text '%s'\n",
                 rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ).getStr() );
        return ScDocFunc::SetCellText( rPos, rText, bInterpret, bEnglish, bApi, rFormulaNmsp, eGrammar );
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
        return ScDocFunc::ApplyAttributes( rMark, rPattern, bRecord, bApi );
    }

    virtual sal_Bool ApplyStyle( const ScMarkData& rMark, const String& rStyleName,
                                 sal_Bool bRecord, sal_Bool bApi )
    {
        fprintf( stderr, "Apply Style '%s'\n",
                 rtl::OUStringToOString( rStyleName, RTL_TEXTENCODING_UTF8 ).getStr() );
        return ScDocFunc::ApplyStyle( rMark, rStyleName, bRecord, bApi );
    }

    virtual sal_Bool MergeCells( const ScCellMergeOption& rOption, sal_Bool bContents,
                                 sal_Bool bRecord, sal_Bool bApi )
    {
        fprintf( stderr, "Merge cells\n" );
        return ScDocFunc::MergeCells( rOption, bContents, bRecord, bApi );
    }
};

} // anonymous namespace

SC_DLLPRIVATE ScDocFunc *ScDocShell::CreateDocFunc()
{
    // With ScDocFuncDirect shared_ptr it should even be possible during
    // runtime to replace a ScDocFuncDirect instance with a ScDocFuncSend
    // chained instance (holding the same ScDocFuncDirect instance) and vice
    // versa.
    bool bIsMaster = false;
    if (getenv ("INTERCEPT"))
    {
        boost::shared_ptr<ScDocFuncDirect> pDirect( new ScDocFuncDirect( *this ) );
        boost::shared_ptr<ScDocFuncRecv> pReceiver( new ScDocFuncRecv( pDirect ) );

        static boost::shared_ptr<ScDocFuncDemo> aDemoBus( new ScDocFuncDemo() );
        aDemoBus->add_client( pReceiver ); // a lifecycle horror no doubt.

        return new ScDocFuncSend( *this, boost::shared_ptr<ScDocFuncRecv>( aDemoBus.get() ) );
    }
    else if (isCollabMode( bIsMaster ))
    {
        boost::shared_ptr<ScDocFuncDirect> pDirect( new ScDocFuncDirect( *this ) );
        boost::shared_ptr<ScDocFuncRecv> pReceiver( new ScDocFuncRecv( pDirect ) );
        ScDocFuncSend* pSender = new ScDocFuncSend( *this, pReceiver );
        TeleManager *pManager = TeleManager::get( !bIsMaster );
        bool bOk = true;

        pManager->sigPacketReceived.connect(
                boost::bind( &ScDocFuncRecv::packetReceived, pReceiver.get(), _1, _2 ));
        pManager->sigFileReceived.connect(
                boost::bind( &ScDocFuncRecv::fileReceived, pReceiver.get(), _1 ));

        bOk = bOk && pManager->connect();
        pManager->prepareAccountManager();

        if (bIsMaster)
        {
            ContactList* pContactList = pManager->getContactList();
            AccountContactPairV aVec( pContactList->getContacts());

            fprintf( stderr, "%u contacts\n", (int) aVec.size() );
            if (aVec.empty())
                bOk = false;
            else
            {
                /* TODO: select a pair, for now just take the first */
                TpAccount* pAccount = aVec[0].first;
                TpContact* pContact = aVec[0].second;
                fprintf( stderr, "picked %s\n", tp_contact_get_identifier( pContact ) );
                bOk = bOk && pManager->startBuddySession( pAccount, pContact );
            }
        }
        if (bOk)
        {
            pSender->SetCollaboration( bIsMaster );
        }
        else
        {
            fprintf( stderr, "Could not start collaboration.\n");
        }
        return pSender;
    }
    else
        return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
