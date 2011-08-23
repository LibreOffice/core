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
    void				Process_Token(
                            DYN cpp::Token &	let_drToken );
    // ACCESS
    FileScope_EventHandler &
                        FileEventHandler()      { return aGlobalParseContext; }
    DocuDealer &        DocuDistributor()       { return aGlobalParseContext; }

  private:
    typedef std::vector< cpp::Cpp_PE* >      EnvironmentStack;

    void				AcknowledgeResult();
    const Token &		CurToken() const;
    Cpp_PE &	        CurEnv() const;
    Cpp_PE &			PushEnv() const;
    TokenProcessing_Result &
                        CurResult()             { return aGlobalParseContext.CurResult(); }

    // DATA
    ContextForAry       aGlobalParseContext;

    EnvironmentStack	aEnvironments;
    Dyn<PE_File>        pPE_File;

    ary::cpp::Gate *	pGate;
    cpp::Token *		dpCurToken;
};



}   // namespace cpp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
