/*************************************************************************
 *
 *  $RCSfile: c_rcode.hxx,v $
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

