/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <precomp.h>
#include "preproc.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include "all_toks.hxx"
#include "defdescr.hxx"
#include <tools/tkpchars.hxx>
#include "c_rcode.hxx"


namespace cpp
{


PreProcessor::F_TOKENPROC PreProcessor::aTokProcs[PreProcessor::state_MAX] =
    {
        &PreProcessor::On_plain,
        &PreProcessor::On_expect_macro_bracket_left,
        &PreProcessor::On_expect_macro_param
    };


PreProcessor::PreProcessor()
    :   pCppExplorer(0),
        pSourceText(0),
        pCurValidDefines(0),
        // aTokens,
        eState(plain),
        pCurMacro(0),
        dpCurMacroName(0),
        // aCurMacroParams,
        aCurParamText(60000),
        nBracketInParameterCounter(0)
        // aBlockedMacroNames
{
}

PreProcessor::~PreProcessor()
{
}

void
PreProcessor::AssignPartners( CodeExplorer &      o_rCodeExplorer,
                              CharacterSource &   o_rCharSource,
                              const MacroMap &    i_rCurValidDefines )
{
    pCppExplorer = &o_rCodeExplorer;
    pSourceText = &o_rCharSource;
    pCurValidDefines = &i_rCurValidDefines;
}

void
PreProcessor::Process_Token( cpp::Token & let_drToken )
{
    csv_assert(pCppExplorer != 0);  // Implies pSourceText and pCurValidDefines.

    (this->*aTokProcs[eState])(let_drToken);
}

void
PreProcessor::On_plain( cpp::Token & let_drToken )
{
    if ( let_drToken.TypeId() == Tid_Identifier )
    {
        if (CheckForDefine(let_drToken))
            return;
    }

    pCppExplorer->Process_Token(let_drToken);
}

void
PreProcessor::On_expect_macro_bracket_left( cpp::Token & let_drToken )
{
    if ( let_drToken.TypeId() == Tid_Bracket_Left )
    {
        aCurParamText.seekp(0);
        eState = expect_macro_param;
    }
    else
    {
        pCppExplorer->Process_Token(*dpCurMacroName);
        dpCurMacroName = 0;
        pCppExplorer->Process_Token(let_drToken);
        eState = plain;
    }
}

void
PreProcessor::On_expect_macro_param( cpp::Token & let_drToken )
{
    if ( let_drToken.TypeId() == Tid_Bracket_Left )
        nBracketInParameterCounter++;
    else if ( let_drToken.TypeId() == Tid_Bracket_Right )
    {
        if ( nBracketInParameterCounter > 0 )
            nBracketInParameterCounter--;
        else
        {
            if ( NOT csv::no_str(aCurParamText.c_str()) )
            {
                aCurMacroParams.push_back( String(aCurParamText.c_str()) );
            }
            csv_assert( aCurMacroParams.size() == pCurMacro->ParamCount() );

            InterpretMacro();
            eState = plain;
            return;
        }
    }
    else if ( let_drToken.TypeId() == Tid_Comma AND nBracketInParameterCounter == 0 )
    {
        aCurMacroParams.push_back( String (aCurParamText.c_str()) );
        aCurParamText.seekp(0);
        return;
    }

    // KORR_FUTURE:
    //  If in future whitespace is parsed also, that should match exactly and the
    //  safety spaces, " ", here should be removed.
    aCurParamText << let_drToken.Text() << " ";
}

bool
PreProcessor::CheckForDefine( cpp::Token & let_drToken )
{
    String  sTokenText(let_drToken.Text());
     pCurMacro = csv::value_from_map( *pCurValidDefines, sTokenText );
    if (pCurMacro == 0 )
        return false;
    for ( StringVector::const_iterator it = aBlockedMacroNames.begin();
          it != aBlockedMacroNames.end();
          ++it )
    {
        if ( strcmp( (*it).c_str(), let_drToken.Text() ) == 0 )
            return false;
    }

    if ( pCurMacro->DefineType() == DefineDescription::type_define )
    {
        delete &let_drToken;

        aCurParamText.seekp(0);
        pCurMacro->GetDefineText(aCurParamText);

        if ( aCurParamText.tellp() > 1 )
            pSourceText->InsertTextAtCurPos(aCurParamText.c_str());
    }
    else // ( pCurMacro->DefineType() == DefineDescription::type_macro )
    {
        dpCurMacroName = &let_drToken;
        eState = expect_macro_bracket_left;
        csv::erase_container( aCurMacroParams );
        aCurParamText.seekp(0);
        nBracketInParameterCounter = 0;
    }  // endif

    return true;
}

void
PreProcessor::UnblockMacro( const char * i_sMacroName )
{
    for ( StringVector::iterator it = aBlockedMacroNames.begin();
          it != aBlockedMacroNames.end();
          ++it )
    {
        if ( strcmp( (*it), i_sMacroName ) == 0 )
        {
            aBlockedMacroNames.erase(it);
            break;
        }
    }   /// end for
}

void
PreProcessor::InterpretMacro()
{
    aCurParamText.seekp(0);
    pCurMacro->GetMacroText(aCurParamText, aCurMacroParams);

    if ( NOT csv::no_str(aCurParamText.c_str()) )
    {
        aCurParamText.seekp(-1, csv::cur);
        aCurParamText << " #unblock-" << dpCurMacroName->Text() << " ";

        pSourceText->InsertTextAtCurPos(aCurParamText.c_str());
        String  sCurMacroName(dpCurMacroName->Text());
        aBlockedMacroNames.insert( aBlockedMacroNames.begin(), sCurMacroName );
    }

    delete dpCurMacroName;
    dpCurMacroName = 0;
    pCurMacro = 0;
    csv::erase_container(aCurMacroParams);
    aCurParamText.seekp(0);
}


}   // end namespace cpp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
