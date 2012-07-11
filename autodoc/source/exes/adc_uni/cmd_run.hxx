/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_CMD_RUN_HXX
#define ADC_CMD_RUN_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/comdline.hxx>
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
    class FileCollector_Ifc;
    class ParseToolsFactory_Ifc;
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
    IdlParser &         Get_IdlParser();
    void                Create_IdlParser();
    const ParseToolsFactory_Ifc &
                        ParseToolsFactory();
    uintt               GatherFiles(
                            FileCollector_Ifc & o_rFiles,
                            const S_ProjectData &
                                                i_rProject );
    // DATA
    const Parse &       rCommand;

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
