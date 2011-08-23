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

#ifndef ADC_CPP_C_DEALER_HXX
#define ADC_CPP_C_DEALER_HXX



// USED SERVICES
    // BASE CLASSES
#include <cpp/ctokdeal.hxx>
#include <adoc/atokdeal.hxx>
#include <ary/info/docstore.hxx>
    // COMPONENTS
#include "preproc.hxx"
#include "c_rcode.hxx"
#include <adoc/a_rdocu.hxx>
    // PARAMETERS

namespace csv
{
    namespace ploc
    {
        class Path;
        class DirectoryChain;
    }
}


class TokenParser;


namespace cpp
{

class PE_File;
class DefineDescription;


class Distributor : public cpp::TokenDealer,        /// Handle C++ code tokens.
                    public adoc::TokenDealer        /// Handle Autodoc documentation tokens.
{
  public:
    typedef std::map< String, DefineDescription* >     MacroMap;

    // LIFECYCLE
                        Distributor(
                            ary::cpp::Gate & 	io_rGate );
                        ~Distributor();
    // OPERATIONS
    void				AssignPartners(
                            CharacterSource &   io_rSourceText,
                            const MacroMap &    i_rValidMacros );
    void                StartNewFile(
                            const csv::ploc::Path &
                                                i_file );
    virtual void		Deal_Eol();
    virtual void		Deal_Eof();

    virtual void		Deal_CppCode(
                            cpp::Token & 		let_drToken );
    virtual void        Deal_Cpp_UnblockMacro(
                            Tok_UnblockMacro & 	let_drToken );

    virtual void		Deal_AdcDocu(
                            adoc::Token & 		let_drToken );
    virtual Distributor *
                        AsDistributor();
  private:
    // DATA
    PreProcessor        aCppPreProcessor;
    CodeExplorer	    aCodeExplorer;
    adoc::DocuExplorer	aDocuExplorer;
    ary::cpp::Gate *    pGate;
    FileScope_EventHandler *
                        pFileEventHandler;
    DocuDealer *        pDocuDistributor;
};



}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
