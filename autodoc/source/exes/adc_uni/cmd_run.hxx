/*************************************************************************
 *
 *  $RCSfile: cmd_run.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:35:36 $
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

    namespace n22
    {
         class Repository;
    }
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
