/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: skeletoncommon.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2005-12-28 18:02:14 $
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
#ifndef _UNO_DEVTOOLS_SKELETONCOMMON_HXX_
#define _UNO_DEVTOOLS_SKELETONCOMMON_HXX_

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _REGISTRY_READER_HXX_
#include <registry/reader.hxx>
#endif
#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include <codemaker/typemanager.hxx>
#endif
#ifndef _CODEMAKER_UNOTYPE_HXX_
#include <codemaker/unotype.hxx>
#endif

#include <hash_set>
#include <hash_map>

// #ifndef _UNO_DEVTOOLS_SKELETONMAKER_HXX_
// #include "skeletonmaker.hxx"
// #endif

namespace skeletonmaker {

struct ProgramOptions {
    ProgramOptions(): java5(true), all(false), dump(false),
                      shortnames(false), language(1) {}

    bool java5;
    bool all;
    bool dump;
    bool shortnames;
    // language specifier - is extendable
    // 1 = Java
    // 2 = C++
    short language;
    rtl::OString outputpath;
    rtl::OString implname;
};

typedef ::std::hash_map< ::rtl::OString,
                         std::pair< rtl::OString, sal_Int16 >,
                         rtl::OStringHash > StringPairHashMap;


codemaker::UnoType::Sort decomposeResolveAndCheck(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments);

void checkType(TypeManager const & manager,
               rtl::OString const & type,
               std::hash_set< rtl::OString, rtl::OStringHash >& interfaceTypes,
               std::hash_set< rtl::OString, rtl::OStringHash >& serviceTypes,
               StringPairHashMap& properties);

void checkDefaultInterfaces(
         std::hash_set< rtl::OString, rtl::OStringHash >& interfaces,
         const std::hash_set< rtl::OString, rtl::OStringHash >& services,
         const rtl::OString & propertyhelper);

rtl::OString checkPropertyHelper(TypeManager const & manager,
         const std::hash_set< rtl::OString, rtl::OStringHash >& services,
         StringPairHashMap& attributes,
         std::hash_set< rtl::OString, rtl::OStringHash >& propinterfaces);

bool checkXComponentSupport(TypeManager const & manager,
                            typereg::Reader const & reader);

// if XComponent is directly specified, return true and remove it from the
// supported interfaces list
bool checkXComponentSupport(TypeManager const & manager,
         std::hash_set< rtl::OString, rtl::OStringHash >& interfaces);

sal_uInt16 checkAdditionalPropertyFlags(typereg::Reader const & reader,
                                        sal_uInt16 field, sal_uInt16 method);

}

#endif // _UNO_DEVTOOLS_SKELETONCOMMON_HXX_

