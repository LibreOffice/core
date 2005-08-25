/*************************************************************************
 *
 *  $RCSfile: skeletonjava.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-25 15:30:33 $
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
#ifndef _UNO_DEVTOOLS_SKELETONJAVA_HXX_
#define _UNO_DEVTOOLS_SKELETONJAVA_HXX_

#include "skeletoncommon.hxx"

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
                  StringPairHashMap& attributes,
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
                         rtl::OString const & type);

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
                           rtl::OString const & type);

void generateSkeleton(ProgramOptions const & options, TypeManager const & manager, std::vector< rtl::OString > const & types, rtl::OString const & delegate);

} }

#endif // _UNO_DEVTOOLS_SKELETONJAVA_HXX_

