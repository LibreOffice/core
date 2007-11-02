/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmd_run.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:44:23 $
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

#ifndef ADC_CMD_RUN_HXX
#define ADC_CMD_RUN_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
     class Repository;
}

namespace autodoc
{
    class FileCollector_Ifc;
    class ParseToolsFactory_Ifc;
    class CodeParser_Ifc;
    class DocumentationParser_Ifc;
    class IdlParser;


namespace command
{
    class Parse;
     class S_ProjectData;
     struct S_LanguageInfo;

namespace run
{

/** Performs an ::autodoc::command::Parse .
*/
class Parser
{
  public:
                        Parser(
                            const Parse &       i_command );
                        ~Parser();

    bool                Perform();

  private:
    // Locals
    CodeParser_Ifc &    Get_CppParser();
    IdlParser &         Get_IdlParser();
    void                Create_CppParser();
    void                Create_IdlParser();
    const ParseToolsFactory_Ifc &
                        ParseToolsFactory();
    uintt               GatherFiles(
                            FileCollector_Ifc & o_rFiles,
                            const S_ProjectData &
                                                i_rProject );
    // DATA
    const Parse &       rCommand;

    Dyn<CodeParser_Ifc> pCppParser;
    Dyn<DocumentationParser_Ifc>
                        pCppDocuInterpreter;
    Dyn<IdlParser>      pIdlParser;
};




// IMPLEMENTATION


}   // namespace run
}   // namespace command
}   // namespace autodoc

#endif
