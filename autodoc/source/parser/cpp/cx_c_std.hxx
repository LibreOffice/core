/*************************************************************************
 *
 *  $RCSfile: cx_c_std.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

