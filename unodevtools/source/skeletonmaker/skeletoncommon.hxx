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
#ifndef INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX
#define INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "rtl/string.hxx"
#include "registry/reader.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include <fstream>
#include <boost/unordered_set.hpp>
#include <map>

namespace skeletonmaker {

typedef ::std::map< OString, ::std::vector< OString >,
                    ::std::less< OString > > ProtocolCmdMap;

typedef ::std::vector< ::std::pair< OString,
                     ::std::pair< OString, sal_Int16 > > > AttributeInfo;


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
    OString outputpath;
    OString implname;
    ProtocolCmdMap protocolCmdMap;
};


/**
   print the standard OpenOffice.org license header

   @param o specifies the output stream
   @param filename specifies the source file name
*/
void printLicenseHeader(std::ostream& o, OString const & filename);

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
                     OString const & extension,
                     std::ostream** ppOutputStream,
                     OString & targetSourceFileName,
                     OString & tmpSourceFileName);

codemaker::UnoType::Sort decomposeResolveAndCheck(
    rtl::Reference< TypeManager > const & manager, OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, OString * name, sal_Int32 * rank,
    std::vector< OString > * arguments);

void checkType(rtl::Reference< TypeManager > const & manager,
               OString const & type,
               boost::unordered_set< OString, OStringHash >& interfaceTypes,
               boost::unordered_set< OString, OStringHash >& serviceTypes,
               AttributeInfo& properties);

void checkDefaultInterfaces(
    boost::unordered_set< OString, OStringHash >& interfaces,
    const boost::unordered_set< OString, OStringHash >& services,
    const OString & propertyhelper);

OString checkPropertyHelper(
    ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
    const boost::unordered_set< OString, OStringHash >& services,
    const boost::unordered_set< OString, OStringHash >& interfaces,
    AttributeInfo& attributes,
    boost::unordered_set< OString, OStringHash >& propinterfaces);

/**
   checks whether the return and parameters types are valid and allowed
   calc add-in type. The function throws a CannotDumpException with an
   detailed error description which type is wrong

   @param manager a type manager
   @param reader a registry type reader of an interface defining
                 calc add-in functions
*/
void checkAddInTypes(rtl::Reference< TypeManager > const & manager,
                     typereg::Reader const & reader);


/**
   checks if XComponent have to be supported, if yes it removes it from the
   supported interfaces list because it becomes implemented by the appropriate
   helper

   @param manager a type manager
   @param interfaces a list of interfaces which should be implemented

   @return true if XComponent have to be supported
*/
bool checkXComponentSupport(rtl::Reference< TypeManager > const & manager,
         boost::unordered_set< OString, OStringHash >& interfaces);


sal_uInt16 checkAdditionalPropertyFlags(typereg::Reader const & reader,
                                        sal_uInt16 field, sal_uInt16 method);


void generateFunctionParameterMap(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager,
         const boost::unordered_set< OString, OStringHash >& interfaces);

}

#endif // INCLUDED_UNODEVTOOLS_SOURCE_SKELETONMAKER_SKELETONCOMMON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
