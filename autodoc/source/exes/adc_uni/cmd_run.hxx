/*************************************************************************
 *
 *  $RCSfile: cmd_run.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:27 $
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
}

namespace csi
{
    namespace uidl
    {
         class Uidl_Parser;
    }
}

namespace autodoc
{
    namespace command
    {
         class Parse;
         struct S_ProjectData;
         struct S_LanguageInfo;
    }

    class CommandLine;
    class FileCollector_Ifc;
    class ParseToolsFactory_Ifc;
    class CodeParser_Ifc;
    class DocumentationParser_Ifc;
    typedef csi::uidl::Uidl_Parser IdlParser;


class CommandRunner
{
  public:
                        CommandRunner();
                        ~CommandRunner();


    int                 Run(
                            const CommandLine & i_rCL );
  private:
    // Locals
    void                Parse();
    void                Load();
    void                Save();
    void                CreateHtml();
    void                CreateXml();

    CodeParser_Ifc &    Get_CppParser();
    IdlParser &         Get_IdlParser();
    void                Create_CppParser();
    void                Create_IdlParser();

    inline const ParseToolsFactory_Ifc &
                        ParseToolsFactory();
    inline const command::S_LanguageInfo &
                        Get_ProjectLanguage(
                            const command::Parse &
                                                i_rCommand,
                            const command::S_ProjectData &
                                                i_rProject );
    uintt               GatherFiles(
                            FileCollector_Ifc & o_rFiles,
                            const command::Parse &
                                                i_rCommand,
                            const command::S_ProjectData &
                                                i_rProject );
    void                CreateHtml_NewStyle();
    void                CreateHtml_OldIdlStyle();

    bool                HasParsedCpp() const;
    bool                HasParsedIdl() const;

    // DATA
    const CommandLine * pCommandLine;
    ary::Repository *   pReposy;
    int                 nResultCode;

    Dyn<CodeParser_Ifc> pCppParser;
    Dyn<DocumentationParser_Ifc>
                        pCppDocuInterpreter;
    Dyn<IdlParser>      pIdlParser;
};



// IMPLEMENTATION


}   // namespace autodoc

#endif

