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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
