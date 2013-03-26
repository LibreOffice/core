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
#ifndef INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX
#define INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX

#include <fstream>

#include "codemaker/generatedtypeset.hxx"
#include "skeletoncommon.hxx"

namespace skeletonmaker { namespace java {

void printType(std::ostream & o,
               ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
               codemaker::UnoType::Sort sort, RTTypeClass typeClass,
               rtl::OString const & name, sal_Int32 rank,
               std::vector< rtl::OString > const & arguments,
               bool referenceType, bool defaultvalue=false);

void printType(std::ostream & o,
               ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
               rtl::OString const & type, bool referenceType,
               bool defaultvalue=false);

bool printConstructorParameters(std::ostream & o,
                                ProgramOptions const & options,
                                rtl::Reference< TypeManager > const & manager,
                                typereg::Reader const & reader,
                                typereg::Reader const & outerReader,
                                std::vector< rtl::OString > const & arguments);

void printConstructor(std::ostream & o,
                      ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
                      typereg::Reader const & reader,
                      std::vector< rtl::OString > const & arguments);

void printMethodParameters(std::ostream & o,
                           ProgramOptions const & options,
                           rtl::Reference< TypeManager > const & manager,
                           typereg::Reader const & reader,
                           sal_uInt16 method, bool previous,
                           bool withtype);

void printExceptionSpecification(std::ostream & o,
                                 ProgramOptions const & options,
                                 rtl::Reference< TypeManager > const & manager,
                                 typereg::Reader const & reader,
                                 sal_uInt16 method);

void printMethods(std::ostream & o,
                  ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
                  typereg::Reader const & reader,
                  codemaker::GeneratedTypeSet & generated,
                  rtl::OString const & delegate,
                  rtl::OString const & indentation=rtl::OString(),
                  bool defaultvalue=false,
                  bool usepropertymixin=false);

void printConstructionMethods(std::ostream & o,
                              ProgramOptions const & options,
                              rtl::Reference< TypeManager > const & manager,
                              typereg::Reader const & reader);

void printServiceMembers(std::ostream & o,
                         ProgramOptions const & options,
                         rtl::Reference< TypeManager > const & manager,
                         typereg::Reader const & reader,
                         rtl::OString const & type,
                         rtl::OString const & delegate);

void printMapsToJavaType(std::ostream & o,
                         ProgramOptions const & options,
                         rtl::Reference< TypeManager > const & manager,
                         codemaker::UnoType::Sort sort,
                         RTTypeClass typeClass,
                         rtl::OString const & name, sal_Int32 rank,
                         std::vector< rtl::OString > const & arguments,
                         const char * javaTypeSort);

void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           rtl::Reference< TypeManager > const & manager,
                           rtl::OString const & type,
                           rtl::OString const & delegate);

void generateSkeleton(ProgramOptions const & options, rtl::Reference< TypeManager > const & manager, std::vector< rtl::OString > const & types);

} }

#endif // INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
