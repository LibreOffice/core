/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: skeletonjava.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-15 09:20:45 $
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
#ifndef INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX
#define INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX

#include <fstream>

#ifndef INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX
#include "codemaker/generatedtypeset.hxx"
#endif
#ifndef INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX
#include "skeletoncommon.hxx"
#endif

namespace skeletonmaker { namespace java {

void printType(std::ostream & o,
               ProgramOptions const & options, TypeManager const & manager,
               codemaker::UnoType::Sort sort, RTTypeClass typeClass,
               rtl::OString const & name, sal_Int32 rank,
               std::vector< rtl::OString > const & arguments,
               bool referenceType, bool defaultvalue=false);

void printType(std::ostream & o,
               ProgramOptions const & options, TypeManager const & manager,
               rtl::OString const & type, bool referenceType,
               bool defaultvalue=false);

bool printConstructorParameters(std::ostream & o,
                                ProgramOptions const & options,
                                TypeManager const & manager,
                                typereg::Reader const & reader,
                                typereg::Reader const & outerReader,
                                std::vector< rtl::OString > const & arguments);

void printConstructor(std::ostream & o,
                      ProgramOptions const & options, TypeManager const & manager,
                      typereg::Reader const & reader,
                      std::vector< rtl::OString > const & arguments);

void printMethodParameters(std::ostream & o,
                           ProgramOptions const & options,
                           TypeManager const & manager,
                           typereg::Reader const & reader,
                           sal_uInt16 method, bool previous,
                           bool withtype,
                           bool shortname=false);

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
                  rtl::OString const & indentation=rtl::OString(),
                  bool defaultvalue=false,
                  bool usepropertymixin=false);

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

void printMapsToJavaType(std::ostream & o,
                         ProgramOptions const & options,
                         TypeManager const & manager,
                         codemaker::UnoType::Sort sort,
                         RTTypeClass typeClass,
                         rtl::OString const & name, sal_Int32 rank,
                         std::vector< rtl::OString > const & arguments,
                         const char * javaTypeSort);

void generateDocumentation(std::ostream & o,
                           ProgramOptions const & options,
                           TypeManager const & manager,
                           rtl::OString const & type,
                           rtl::OString const & delegate);

void generateSkeleton(ProgramOptions const & options, TypeManager const & manager, std::vector< rtl::OString > const & types, rtl::OString const & delegate);

} }

#endif // INCLUDED_UNODEVTOOLS_SKELETONJAVA_HXX

