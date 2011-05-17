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
               ProgramOptions const & options, TypeManager const & manager,
               codemaker::UnoType::Sort sort, RTTypeClass typeClass,
               rtl::OString const & name, sal_Int32 rank,
               std::vector< rtl::OString > const & arguments,
               short referenceType, bool defaultvalue=false);

void printType(std::ostream & o,
               ProgramOptions const & options, TypeManager const & manager,
               rtl::OString const & type, short referenceType,
               bool defaultvalue=false);


bool printConstructorParameters(std::ostream & o,
                                ProgramOptions const & options,
                                TypeManager const & manager,
                                typereg::Reader const & reader,
                                typereg::Reader const & outerReader,
                                std::vector< rtl::OString > const & arguments);


void printConstructor(std::ostream & o,
                      ProgramOptions const & options,
                      TypeManager const & manager,
                      typereg::Reader const & reader,
                      std::vector< rtl::OString > const & arguments);


void printMethodParameters(std::ostream & o,
                           ProgramOptions const & options,
                           TypeManager const & manager,
                           typereg::Reader const & reader,
                           sal_uInt16 method, bool previous,
                           bool withtype);


void printExceptionSpecification(std::ostream & o,
                                 ProgramOptions const & options,
                                 TypeManager const & manager,
                                 typereg::Reader const & reader,
                                 sal_uInt16 method);


void printMethods(std::ostream & o,
                  ProgramOptions const & options, TypeManager const & manager,
                  typereg::Reader const & reader,
                  codemaker::GeneratedTypeSet & generated,
                  rtl::OString const & delegate,
                  rtl::OString const & classname=rtl::OString(),
                  rtl::OString const & indentation=rtl::OString(),
                  bool defaultvalue=false,
                  rtl::OString const & propertyhelper=rtl::OString());


void printConstructionMethods(std::ostream & o,
                              ProgramOptions const & options,
                              TypeManager const & manager,
                              typereg::Reader const & reader);


void printServiceMembers(std::ostream & o,
                         ProgramOptions const & options,
                         TypeManager const & manager,
                         typereg::Reader const & reader,
                         rtl::OString const & type,
                         rtl::OString const & delegate);


void printMapsToCppType(std::ostream & o,
                        ProgramOptions const & options,
                        TypeManager const & manager,
                        codemaker::UnoType::Sort sort,
                        RTTypeClass typeClass,
                        rtl::OString const & name,
                        sal_Int32 rank,
                        std::vector< rtl::OString > const & arguments,
                        const char * cppTypeSort);


void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           TypeManager const & manager,
                           rtl::OString const & type,
                           rtl::OString const & delegate);


void generateSkeleton(ProgramOptions const & options,
                      TypeManager const & manager,
                      std::vector< rtl::OString > const & types,
                      rtl::OString const & delegate);

void generateCalcAddin(ProgramOptions const & options,
                       TypeManager const & manager,
                       std::vector< rtl::OString > const & types);

} }

#endif // INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCPP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
