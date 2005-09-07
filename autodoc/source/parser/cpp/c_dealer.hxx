/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_dealer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:14:56 $
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
    typedef std::map< udmstri, DefineDescription* >     MacroMap;

    // LIFECYCLE
                        Distributor(
                            ary::cpp::RwGate &  io_rGate );
                        ~Distributor();
    // OPERATIONS
    void                AssignPartners(
                            CharacterSource &   io_rSourceText,
                            const MacroMap &    i_rValidMacros );
    void                StartNewProject(
                            const udmstri &     i_sProjectName,
                            const csv::ploc::Path &
                                                i_rProjectRootDirectory );
    void                StartNewFile(
                            const udmstri &     i_sFileName,
                            const csv::ploc::DirectoryChain &
                                                i_rFileSubPath );
    virtual void        Deal_Eol();
    virtual void        Deal_Eof();

    virtual void        Deal_CppCode(
                            cpp::Token &        let_drToken );
    virtual void        Deal_Cpp_UnblockMacro(
                            Tok_UnblockMacro &  let_drToken );

    virtual void        Deal_AdcDocu(
                            adoc::Token &       let_drToken );
    virtual Distributor *
                        AsDistributor();
  private:
    // DATA
    PreProcessor        aCppPreProcessor;
    CodeExplorer        aCodeExplorer;
    adoc::DocuExplorer  aDocuExplorer;
    ary::cpp::RwGate *  pGate;
    FileScope_EventHandler *
                        pFileEventHandler;
    DocuDealer *        pDocuDistributor;
};



// IMPLEMENTATION



}   // namespace cpp



#endif

