/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

