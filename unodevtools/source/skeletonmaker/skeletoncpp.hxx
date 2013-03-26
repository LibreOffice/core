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
#ifndef INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCPP_HXX
#define INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCPP_HXX

#include <fstream>

#include "codemaker/generatedtypeset.hxx"
#include "skeletoncommon.hxx"

namespace skeletonmaker { namespace cpp {

// referenceType
// 0 = no reference
// 1 = use of css::uno::Reference for interfaces
// 2 = reference (includes css::uno::Reference for interfaces)
// 4 = const reference  (includes css::uno::Reference for interfaces)
// 8 = default construction for example for return types, means "return <type>();"
// 16 = default member initialization in a constructor
void printType(std::ostream & o,
               ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
               codemaker::UnoType::Sort sort, RTTypeClass typeClass,
               rtl::OString const & name, sal_Int32 rank,
               std::vector< rtl::OString > const & arguments,
               short referenceType, bool defaultvalue=false);

void printType(std::ostream & o,
               ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
               rtl::OString const & type, short referenceType,
               bool defaultvalue=false);


bool printConstructorParameters(std::ostream & o,
                                ProgramOptions const & options,
                                rtl::Reference< TypeManager > const & manager,
                                typereg::Reader const & reader,
                                typereg::Reader const & outerReader,
                                std::vector< rtl::OString > const & arguments);


void printConstructor(std::ostream & o,
                      ProgramOptions const & options,
                      rtl::Reference< TypeManager > const & manager,
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
                  rtl::OString const & classname=rtl::OString(),
                  rtl::OString const & indentation=rtl::OString(),
                  bool defaultvalue=false,
                  rtl::OString const & propertyhelper=rtl::OString());


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


void printMapsToCppType(std::ostream & o,
                        ProgramOptions const & options,
                        rtl::Reference< TypeManager > const & manager,
                        codemaker::UnoType::Sort sort,
                        RTTypeClass typeClass,
                        rtl::OString const & name,
                        sal_Int32 rank,
                        std::vector< rtl::OString > const & arguments,
                        const char * cppTypeSort);


void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           rtl::Reference< TypeManager > const & manager,
                           rtl::OString const & type,
                           rtl::OString const & delegate);


void generateSkeleton(ProgramOptions const & options,
                      rtl::Reference< TypeManager > const & manager,
                      std::vector< rtl::OString > const & types);

void generateCalcAddin(ProgramOptions const & options,
                       rtl::Reference< TypeManager > const & manager,
                       std::vector< rtl::OString > const & types);

} }

#endif // INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCPP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
