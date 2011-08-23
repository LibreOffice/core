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

#ifndef ADC_CPP_PREPROC_HXX
#define ADC_CPP_PREPROC_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <deque>
    // PARAMETERS

class CharacterSource;


namespace cpp
{

class Token;
class CodeExplorer;
class DefineDescription;


class PreProcessor
{
  public:
    typedef std::map< String, DefineDescription* > MacroMap;

    // LIFECYCLE
                        PreProcessor();
                        ~PreProcessor();
    // OPERATONS
    void                AssignPartners(
                            CodeExplorer &      o_rCodeExplorer,
                            CharacterSource &   o_rCharSource,
                            const MacroMap &    i_rCurValidDefines );
    void                Process_Token(
                            cpp::Token &        let_drToken );
    void                UnblockMacro(
                            const char *        i_sMacroName );
  private:
    public: // Necessary for instantiation of static variable:
    enum E_State
    {
        plain = 0,
        expect_macro_bracket_left,
        expect_macro_param,
        state_MAX
    };
    typedef void (PreProcessor::*                   F_TOKENPROC )(cpp::Token &);
    void                On_plain( cpp::Token & );
    void                On_expect_macro_bracket_left( cpp::Token & );
    void                On_expect_macro_param( cpp::Token & );

    private:  // Reprivate again:
    typedef std::deque< DYN cpp::Token * >          TokenQueue;
    typedef StringVector                  List_MacroParams;


    bool                CheckForDefine(
                            cpp::Token &        let_drToken );
    void                InterpretMacro();

    // DATA
    static F_TOKENPROC  aTokProcs[state_MAX];
        // Referenced extern objects
    CodeExplorer *      pCppExplorer;
    CharacterSource *   pSourceText;
    const MacroMap *    pCurValidDefines;

        // internal data
    TokenQueue          aTokens;

    E_State             eState;

    DefineDescription * pCurMacro;
    DYN Token *         dpCurMacroName;
    List_MacroParams    aCurMacroParams;
    csv::StreamStr      aCurParamText;

    intt                nBracketInParameterCounter;
    StringVector        aBlockedMacroNames;
};



}   // end namespace cpp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
