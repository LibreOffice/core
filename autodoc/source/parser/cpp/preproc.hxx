/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: preproc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:32:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    typedef std::map< udmstri, DefineDescription* > MacroMap;

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

