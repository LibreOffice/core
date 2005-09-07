/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_rcode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:15:32 $
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

#ifndef ADC_CPP_C_RCODE_HXX
#define ADC_CPP_C_RCODE_HXX



// USED SERVICES
    // BASE CLASSES
#include <tokens/tokproct.hxx>
    // COMPONENTS
#include <cosv/ploc.hxx>
#include "cxt2ary.hxx"
    // PARAMETERS
#include <ary/ids.hxx>



namespace ary
{
    namespace cpp
    {
        class RwGate;
    }

    class Documentation;
}

namespace cpp
{

    class PE_File;
    class Token;
    class Cpp_PE;

class CodeExplorer : private TokenProcessing_Types

{
  public:
                        CodeExplorer(
                            ary::cpp::RwGate & io_rAryGate );
                        ~CodeExplorer();

    void                StartNewFile();
    void                Process_Token(
                            DYN cpp::Token &    let_drToken );
    ary::Cid            CurFile() const;

    // ACCESS
    FileScope_EventHandler &
                        FileEventHandler()      { return aGlobalParseContext; }
    DocuDealer &        DocuDistributor()       { return aGlobalParseContext; }

  private:
    typedef std::vector< cpp::Cpp_PE* >      EnvironmentStack;

    void                AcknowledgeResult();
    const Token &       CurToken() const;
    Cpp_PE &            CurEnv() const;
    Cpp_PE &            PushEnv() const;
    TokenProcessing_Result &
                        CurResult()             { return aGlobalParseContext.CurResult(); }

    // DATA
    ContextForAry       aGlobalParseContext;

    EnvironmentStack    aEnvironments;
    Dyn<PE_File>        pPE_File;

    ary::cpp::RwGate *  pGate;
    cpp::Token *        dpCurToken;
};



}   // namespace cpp


#endif

