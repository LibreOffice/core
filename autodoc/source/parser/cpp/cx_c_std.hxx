/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_c_std.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:18:40 $
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

#ifndef ADC_CPP_CX_C_STD_HXX
#define ADC_CPP_CX_C_STD_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS



namespace cpp {

class Context_Comment;

/**
*/
class Context_CppStd : public Cx_Base,
                       private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_CppStd(
                            DYN autodoc::TkpDocuContext &
                                                let_drContext_Docu  );
                        ~Context_CppStd();
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  private:
    //  SERVICE FUNCTIONS
    void                PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            StmArrayStatus::F_CRTOK
                                                i_fTokenCreateFunction,
                            CharacterSource &   io_rText );
    void                SetupStateMachine();

    //  DATA
    StateMachine        aStateMachine;

        // Contexts
    Dyn<autodoc::TkpDocuContext>
                        pDocuContext;

    Dyn<Context_Comment>
                        pContext_Comment;
    Dyn<Cx_Base>        pContext_Preprocessor;
    Dyn<Cx_Base>        pContext_ConstString;
    Dyn<Cx_Base>        pContext_ConstChar;
    Dyn<Cx_Base>        pContext_ConstNumeric;
    Dyn<Cx_Base>        pContext_UnblockMacro;
};



}   // namespace cpp


#endif

