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
#include "cell.hxx"
#include "docsh.hxx"
#include "docfunc.hxx"

namespace {

// Ye noddy protocol !
// method name ',' then arguments comma separated
class ScChangeOp
{
  rtl::OUStringBuffer aMessage;
public:
  ScChangeOp( const char *pName )
  {
    aMessage.appendAscii( pName );
    aMessage.append( sal_Unicode( ',' ) );
  }

  void appendString( const ScAddress &rPos )
  {
    rtl::OUString aStr;
    rPos.Format( aStr );
    aMessage.append( aStr );
  }

  void appendAddress( const ScAddress &rPos )
  {
    (void)rPos;
  }
};

class ScDocFuncIntercept : public ScDocFunc
{
public:
    ScDocFuncIntercept( ScDocShell& rDocSh ) : ScDocFunc( rDocSh )
    {
        fprintf( stderr, "Interceptor created !\n" );
    }
    virtual ~ScDocFuncIntercept() {}
    virtual void EnterListAction( sal_uInt16 nNameResId )
    {
        // Want to group these operations for the other side ...
        String aUndo( ScGlobal::GetRscString( nNameResId ) );
    }
    virtual void EndListAction()
    {
    }
    virtual ScBaseCell* InterpretEnglishString( const ScAddress& rPos, const String& rText,
                                                const String& rFormulaNmsp,
                                                const formula::FormulaGrammar::Grammar eGrammar,
                                                short* pRetFormatType )
    {
        fprintf( stderr, "interp. english string '%s'\n",
                 rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ).getStr() );
        return ScDocFunc::InterpretEnglishString( rPos, rText, rFormulaNmsp,
                                                  eGrammar, pRetFormatType );
    }
    virtual sal_Bool SetNormalString( const ScAddress& rPos, const String& rText, sal_Bool bApi )
    {
        fprintf( stderr, "set normal string '%s'\n",
                 rtl::OUStringToOString( rText, RTL_TEXTENCODING_UTF8 ).getStr() );
        return ScDocFunc::SetNormalString( rPos, rText, bApi );
    }

    virtual sal_Bool PutCell( const ScAddress& rPos, ScBaseCell* pNewCell, sal_Bool bApi )
    {
        fprintf( stderr, "put cell '%p' type %d %d\n", pNewCell, pNewCell->GetCellType(), bApi );
        return ScDocFunc::PutCell( rPos, pNewCell, bApi );
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
        fprintf( stderr, "%s note\n", bShow ? "show" : "hide" );
        return ScDocFunc::ShowNote( rPos, bShow );
    }
};

} // anonymous namespace

SC_DLLPRIVATE ScDocFunc *ScDocShell::CreateDocFunc()
{
    if (getenv ("INTERCEPT"))
        return new ScDocFuncIntercept( *this );
    else
        return new ScDocFuncDirect( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
