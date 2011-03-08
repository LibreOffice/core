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
#ifndef INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX
#define INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX

#include "rtl/string.hxx"
#include "registry/reader.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include <fstream>
#include <boost/unordered_set.hpp>
#include <map>

namespace skeletonmaker {

typedef ::std::map< ::rtl::OString, ::std::vector< ::rtl::OString >,
                    ::std::less< ::rtl::OString > > ProtocolCmdMap;

typedef ::std::vector< ::std::pair< rtl::OString,
                     ::std::pair< rtl::OString, sal_Int16 > > > AttributeInfo;


struct ProgramOptions {
    ProgramOptions(): java5(true), all(false), dump(false), license(false),
                      shortnames(false), supportpropertysetmixin(false),
                      backwardcompatible(false), language(1), componenttype(1) {}

    bool java5;
    bool all;
    bool dump;
    bool license;
    bool shortnames;
    bool supportpropertysetmixin;
    bool backwardcompatible;
    // language specifier - is extendable
    // 1 = Java
    // 2 = C++
    short language;
    // component type
    // 1 = default UNO component - is extendable
    // 2 = calc add-in
    // 3 = add-on
    short componenttype;
    rtl::OString outputpath;
    rtl::OString implname;
    ProtocolCmdMap protocolCmdMap;
};


/**
   print the standard OpenOffice.org license header

   @param o specifies the output stream
   @param filename specifies the source file name
*/
void printLicenseHeader(std::ostream& o, rtl::OString const & filename);

/**
   create dependent on the output path, the implementation name and the
   extension a new output file. If output path is equal "stdout" the tool
   generates the output to standard out.

   @param options the program options including the output path and the
                  implementation name
   @param extension specifies the file extensions (e.g. .cxx or .java)
   @param ppOutputStream out parameter returning the output stream
   @param targetSourceFileName out parameter returning the generated file name
                               constructed on base of the output path, the
                               implementation name and the extension
   @param tmpSourceFileName out parameter returning the temporary file name based
                            on the output path and a generated temporary file name.
   @return true if output is generated to standard out or else false
*/
bool getOutputStream(ProgramOptions const & options,
                     rtl::OString const & extension,
                     std::ostream** ppOutputStream,
                     rtl::OString & targetSourceFileName,
                     rtl::OString & tmpSourceFileName);

codemaker::UnoType::Sort decomposeResolveAndCheck(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments);

void checkType(TypeManager const & manager,
               rtl::OString const & type,
               boost::unordered_set< rtl::OString, rtl::OStringHash >& interfaceTypes,
               boost::unordered_set< rtl::OString, rtl::OStringHash >& serviceTypes,
               AttributeInfo& properties);

void checkDefaultInterfaces(
    boost::unordered_set< rtl::OString, rtl::OStringHash >& interfaces,
    const boost::unordered_set< rtl::OString, rtl::OStringHash >& services,
    const rtl::OString & propertyhelper);

rtl::OString checkPropertyHelper(
    ProgramOptions const & options, TypeManager const & manager,
    const boost::unordered_set< rtl::OString, rtl::OStringHash >& services,
    const boost::unordered_set< rtl::OString, rtl::OStringHash >& interfaces,
    AttributeInfo& attributes,
    boost::unordered_set< rtl::OString, rtl::OStringHash >& propinterfaces);

/**
   checks whether the return and parameters types are valid and allowed
   calc add-in type. The function throws a CannotDumpException with an
   detailed error description which type is wrong

   @param manager a type manager
   @param reader a registry type reader of an interface defining
                 calc add-in functions
*/
void checkAddInTypes(TypeManager const & manager,
                     typereg::Reader const & reader);


/**
   checks if XComponent have to be supported, if yes it removes it from the
   supported interfaces list becuase it becmoes implmented by the appropriate
   helper

   @param manager a type manager
   @param interfaces a list of interfaces which should be implemented

   @return true if XComponent have to be supported
*/
bool checkXComponentSupport(TypeManager const & manager,
         boost::unordered_set< rtl::OString, rtl::OStringHash >& interfaces);


sal_uInt16 checkAdditionalPropertyFlags(typereg::Reader const & reader,
                                        sal_uInt16 field, sal_uInt16 method);


void generateFunctionParameterMap(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const boost::unordered_set< ::rtl::OString, ::rtl::OStringHash >& interfaces);

}

#endif // INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
