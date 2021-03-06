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

namespace skeletonmaker::java {

void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, std::u16string_view name,
    bool referenceType, bool defaultvalue = false);

void printMethods(std::ostream & o,
                  ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
                  OUString const & name,
                  codemaker::GeneratedTypeSet & generated,
                  OString const & delegate,
                  OString const & indentation,
                  bool defaultvalue=false,
                  bool usepropertymixin=false);

void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           rtl::Reference< TypeManager > const & manager,
                           OString const & type,
                           OString const & delegate);

void generateSkeleton(ProgramOptions const & options, rtl::Reference< TypeManager > const & manager, std::vector< OString > const & types);

void generateXPropertySetBodies(std::ostream& o);
void generateXFastPropertySetBodies(std::ostream& o);
void generateXPropertyAccessBodies(std::ostream& o);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
