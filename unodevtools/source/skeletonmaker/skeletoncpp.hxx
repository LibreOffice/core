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
#pragma once

#include <fstream>
#include <string_view>

#include <codemaker/generatedtypeset.hxx>
#include "skeletoncommon.hxx"

namespace skeletonmaker::cpp {

// referenceType
// 0 = no reference
// 1 = use of css::uno::Reference for interfaces
// 2 = reference (includes css::uno::Reference for interfaces)
// 4 = const reference  (includes css::uno::Reference for interfaces)
// 8 = default construction for example for return types, means "return <type>();"
// 16 = default member initialization in a constructor
void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, std::u16string_view name,
    short referenceType, bool defaultvalue = false);

void printMethods(std::ostream & o,
                  ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
                  OUString const & name,
                  codemaker::GeneratedTypeSet & generated,
                  OString const & delegate,
                  OString const & classname,
                  OString const & indentation,
                  bool defaultvalue=false,
                  OUString const & propertyhelper=OUString());

void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           rtl::Reference< TypeManager > const & manager,
                           OString const & type,
                           OString const & delegate);


void generateSkeleton(ProgramOptions const & options,
                      rtl::Reference< TypeManager > const & manager,
                      std::vector< OString > const & types);

void generateCalcAddin(ProgramOptions const & options,
                       rtl::Reference< TypeManager > const & manager,
                       std::vector< OString > const & types);

void generateXPropertySetBodies(std::ostream& o,
                                const OString & classname,
                                const OString & interfaceName);
void generateXFastPropertySetBodies(std::ostream& o,
                                    const OString & classname,
                                    const OString & interfaceName);
void generateXPropertyAccessBodies(std::ostream& o,
                                   const OString & classname,
                                   const OString & interfaceName);

void generateXAddInBodies(std::ostream& o, const OString & classname);

void generateXLocalizable(std::ostream& o, const OString & classname);

void generateXCompatibilityNamesBodies(std::ostream& o, const OString & classname);

void generateXInitialization(std::ostream& o, const OString & classname);

void generateXDispatch(std::ostream& o,
                       const OString & classname,
                       const ProtocolCmdMap & protocolCmdMap);

void generateXDispatchProvider(std::ostream& o,
                               const OString & classname,
                               const ProtocolCmdMap & protocolCmdMap);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
