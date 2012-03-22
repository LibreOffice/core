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

namespace {

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
      aMessage.append( rStr );
      appendSeparator();
  }

  void appendString( const String &rStr )
  {
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

  rtl::OString toString()
  {
      return rtl::OUStringToOString( aMessage.toString(), RTL_TEXTENCODING_UTF8 );
  }
};

struct ProtocolError {
    const char *message;
};

class ScChangeOpReader {
    std::vector< rtl::OString > maArgs;

public:

    ScChangeOpReader( const rtl::OString &rString)
    {
        // will need to handle escaping etc.
        for (sal_Int32 n = 0; n >= 0 && n < rString.getLength();)
            maArgs.push_back( rString.getToken( 0, ';', n ) );
    }
    ~ScChangeOpReader() {}

    rtl::OString getMethod()
    {
        return maArgs[0];
    }

    size_t getArgCount() { return maArgs.size(); }

    rtl::OUString getString( sal_Int32 n )
    {
        if (n > 0 && (size_t)n < getArgCount() )
            return rtl::OUString( maArgs[n].getStr(), maArgs[n].getLength(),
                                  RTL_TEXTENCODING_UTF8 );
        else
            return rtl::OUString();
    }

    ScAddress getAddress( sal_Int32 n )
    {
        ScAddress aAddr;
        rtl::OUString aToken( getString( n ) );
        aAddr.Parse( aToken );
        return aAddr;
    }

    bool getBool( sal_Int32 n )
    {
        if (n > 0 && (size_t)n < getArgCount() )
            return maArgs[n].equalsIgnoreAsciiCase( "true" );
        else
            return false;
    }
};


class ScDocFuncRecv : public ScDocFunc
{
    ScDocFunc *mpChain;
public:
    // FIXME: really ScDocFunc should be an abstract base
    ScDocFuncRecv( ScDocShell& rDocSh, ScDocFunc *pChain )
        : ScDocFunc( rDocSh ),
          mpChain( pChain )
    {
        fprintf( stderr, "Receiver created !\n" );
    }
    virtual ~ScDocFuncRecv() {}

    void RecvMessage( const rtl::OString &rString )
    {
        try {
            ScChangeOpReader aReader( rString );
            // FIXME: have some hash to enumeration mapping here
            if ( aReader.getMethod() == "setNormalString" )
                mpChain->SetNormalString( aReader.getAddress( 1 ), aReader.getString( 2 ),
                                          aReader.getBool( 3 ) );
            else
                fprintf( stderr, "Error: unknown message '%s' (%d)\n",
                         rString.getStr(), (int)aReader.getArgCount() );
        } catch (const ProtocolError &e) {
            fprintf( stderr, "Error: protocol twisting '%s'\n", e.message );
        }
    }
};

class ScDocFuncSend : public ScDocFunc
{
    ScDocFuncRecv *mpChain;

    void SendMessage( ScChangeOpWriter &rOp )
    {
        fprintf( stderr, "Op: '%s'\n", rOp.toString().getStr() );
        mpChain->RecvMessage( rOp.toString() );
    }

public:
    // FIXME: really ScDocFunc should be an abstract base, so
    // we don't need the rDocSh hack/pointer
    ScDocFuncSend( ScDocShell& rDocSh, ScDocFuncRecv *pChain )
            : ScDocFunc( rDocSh ),
            mpChain( pChain )
    {
        fprintf( stderr, "Sender created !\n" );
    }
    virtual ~ScDocFuncSend() {}

    virtual void EnterListAction( sal_uInt16 nNameResId )
    {
        // Want to group these operations for the other side ...
        String aUndo( ScGlobal::GetRscString( nNameResId ) );
    }
    virtual void EndListAction()
    {
    }

    virtual sal_Bool SetNormalString( const ScAddress& rPos, const String& rText, sal_Bool bApi )
    {
        ScChangeOpWriter aOp( "setNormalString" );
        aOp.appendAddress( rPos );
        aOp.appendString( rText );
        aOp.appendBool( bApi );
        SendMessage( aOp );
        //        return mpChain->SetNormalString( rPos, rText, bApi );
        return true; // needs some code auditing action
    }

    virtual sal_Bool PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi )
    {
        fprintf( stderr, "put cell '%p' type %d %d\n", pNewCell, pNewCell->GetCellType(), bApi );
        return mpChain->PutCell( rPos, pNewCell, bApi );
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
        fprintf( stderr, "%s note\n", bShow ? "show" : "hide" );
        return mpChain->ShowNote( rPos, bShow );
    }
};

} // anonymous namespace

SC_DLLPRIVATE ScDocFunc *ScDocShell::CreateDocFunc()
{
    // FIXME: the chains should be auto-ptrs.
    if (getenv ("INTERCEPT"))
        return new ScDocFuncSend( *this, new ScDocFuncRecv( *this, new ScDocFuncDirect( *this ) ) );
    else
        return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
