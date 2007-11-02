/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_rcode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:48:40 $
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

// BASE CLASSES
#include <tokens/tokproct.hxx>
// USED SERVICES
#include <cosv/ploc.hxx>
#include "cxt2ary.hxx"
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/loc/loc_types4loc.hxx>



namespace ary
{
namespace cpp
{
    class Gate;
}
namespace doc
{
    class OldCppDocu;
}
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
                            ary::cpp::Gate &    io_rAryGate );
                        ~CodeExplorer();

    void                StartNewFile();
    void                Process_Token(
                            DYN cpp::Token &    let_drToken );
    ary::loc::Le_id     CurFile() const;

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

    ary::cpp::Gate *    pGate;
    cpp::Token *        dpCurToken;
};



}   // namespace cpp


#endif

