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

#include "osl/thread.hxx"

#include "codemaker/commonjava.hxx"
#include "codemaker/commoncpp.hxx"
#include "codemaker/generatedtypeset.hxx"
#include "codemaker/global.hxx"
#include "unoidl/unoidl.hxx"

#include "skeletoncommon.hxx"

#include <cassert>
#include <iostream>

using namespace ::rtl;
using namespace ::codemaker::cpp;

namespace skeletonmaker {

void printLicenseHeader(std::ostream& o, rtl::OString const & filename)
{
    sal_Int32 index = -1;
#ifdef SAL_UNX
    index = filename.lastIndexOf('/');
#else
    index = filename.lastIndexOf('\\');
#endif
    OString shortfilename(filename);
    if ( index != -1 )
        shortfilename = filename.copy(index+1);

    o << "/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */\n"
         "/*\n"
         " * This file is part of the LibreOffice project.\n"
         " *\n"
         " * This Source Code Form is subject to the terms of the Mozilla Public\n"
         " * License, v. 2.0. If a copy of the MPL was not distributed with this\n"
         " * file, You can obtain one at http://mozilla.org/MPL/2.0/.\n"
         " */\n\n";
}

bool getOutputStream(ProgramOptions const & options,
                     OString const & extension,
                     std::ostream** ppOutputStream,
                     OString & targetSourceFileName,
                     OString & tmpSourceFileName)
{
    bool bStandardout = false;
    if ( options.outputpath.equals("stdout") )
    {
        bStandardout = true;
        *ppOutputStream = &std::cout;
        return bStandardout;
    }

    targetSourceFileName = createFileNameFromType(
        options.outputpath, options.implname.replace('.','/'), extension);

    OString tmpDir = getTempDir(targetSourceFileName);
    FileStream file;
    file.createTempFile(tmpDir);

    if( !file.isValid() )
    {
        throw CannotDumpException(
            "cannot open " + b2u(targetSourceFileName) + " for writing");
    } else {
        tmpSourceFileName = file.getName();
    }
    file.close();
    *ppOutputStream = new std::ofstream(tmpSourceFileName.getStr(),
                                        std::ios_base::binary);

    return bStandardout;
}

bool containsAttribute(AttributeInfo& attributes, OUString const & attrname)
{
    for ( AttributeInfo::const_iterator i(attributes.begin());
          i != attributes.end(); ++i ) {
        if ( (*i).name == attrname ) {
            return true;
        }
    }
    return false;
}

// collect attributes including inherited attributes
void checkAttributes(rtl::Reference< TypeManager > const & manager,
                     OUString const & name,
                     AttributeInfo& attributes,
                     std::set< OUString >& propinterfaces)
{
    if ( name == "com.sun.star.beans.XPropertySet" ||
         name == "com.sun.star.beans.XFastPropertySet" ||
         name == "com.sun.star.beans.XPropertyAccess" )
    {
        propinterfaces.insert(name);
    }
    rtl::Reference< unoidl::Entity > ent;
    switch (manager->getSort(name, &ent)) {
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        {
            rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
            assert(ent2.is());
            for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                     ent2->getDirectMandatoryBases().begin());
                 i != ent2->getDirectMandatoryBases().end(); ++i)
            {
                checkAttributes(manager, i->name, attributes, propinterfaces);
            }
            for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::
                     const_iterator i(ent2->getDirectAttributes().begin());
                 i != ent2->getDirectAttributes().end(); ++i)
            {
                if (!containsAttribute(attributes, i->name)) {
                    attributes.push_back(
                        unoidl::AccumulationBasedServiceEntity::Property(
                            i->name,
                            i->type,
                            (unoidl::AccumulationBasedServiceEntity::Property::
                             Attributes(
                                 ((i->bound
                                   ? (unoidl::AccumulationBasedServiceEntity::
                                      Property::ATTRIBUTE_BOUND)
                                   : 0)
                                  | (i->readOnly
                                     ? (unoidl::AccumulationBasedServiceEntity::
                                        Property::ATTRIBUTE_READ_ONLY)
                                     : 0)))),
                            std::vector< OUString >()));
                }
            }
            break;
        }
    case codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE:
        {
            rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                     ent2->getDirectMandatoryBaseServices().begin());
                 i != ent2->getDirectMandatoryBaseServices().end(); ++i)
            {
                checkAttributes(manager, i->name, attributes, propinterfaces);
            }
            for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                     ent2->getDirectMandatoryBaseInterfaces().begin());
                 i != ent2->getDirectMandatoryBaseInterfaces().end(); ++i)
            {
                checkAttributes(manager, i->name, attributes, propinterfaces);
            }
            for (std::vector<
                     unoidl::AccumulationBasedServiceEntity::Property >::
                     const_iterator i(ent2->getDirectProperties().begin());
                 i != ent2->getDirectProperties().end(); ++i)
            {
                if (!containsAttribute(attributes, i->name)) {
                    attributes.push_back(*i);
                }
            }
            break;
        }
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::checkAttributes");
    }
}

void checkType(rtl::Reference< TypeManager > const & manager,
               OUString const & name,
               std::set< OUString >& interfaceTypes,
               std::set< OUString >& serviceTypes,
               AttributeInfo& properties)
{
    rtl::Reference< unoidl::Entity > ent;
    switch (manager->getSort(name, &ent)) {
    case codemaker::UnoType::SORT_INTERFACE_TYPE:
        // com.sun.star.lang.XComponent should be also not in the list
        // but it will be used for checking the impl helper and will be
        // removed later if necessary.
        if ( name == "com.sun.star.lang.XTypeProvider" ||
             name == "com.sun.star.uno.XWeak" )
            return;
        if (interfaceTypes.find(name) == interfaceTypes.end()) {
            interfaceTypes.insert(name);
        }
        break;
    case codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        if (serviceTypes.find(name) == serviceTypes.end()) {
            serviceTypes.insert(name);
            rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
                dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            if (interfaceTypes.find(ent2->getBase()) == interfaceTypes.end()) {
                interfaceTypes.insert(ent2->getBase());
                // check if constructors are specified, if yes automatically
                // support of XInitialization. We will take care of the default
                // constructor because in this case XInitialization is not
                // called.
                if (ent2->getConstructors().size() > 1 ||
                    (ent2->getConstructors().size() == 1 &&
                     !ent2->getConstructors()[0].defaultConstructor))
                {
                    OUString s("com.sun.star.lang.XInitialization");
                    if (interfaceTypes.find(s) == interfaceTypes.end())
                        interfaceTypes.insert(s);
                }
            }
        }
        break;
    case codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE:
        if ( serviceTypes.find(name) == serviceTypes.end() ) {
            serviceTypes.insert(name);
            rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                     ent2->getDirectMandatoryBaseServices().begin());
                 i != ent2->getDirectMandatoryBaseServices().end(); ++i)
            {
                checkType(
                    manager, i->name, interfaceTypes, serviceTypes, properties);
            }
            for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                     ent2->getDirectMandatoryBaseInterfaces().begin());
                 i != ent2->getDirectMandatoryBaseInterfaces().end(); ++i)
            {
                checkType(
                    manager, i->name, interfaceTypes, serviceTypes, properties);
            }
            for (std::vector<
                     unoidl::AccumulationBasedServiceEntity::Property >::
                     const_iterator i(ent2->getDirectProperties().begin());
                 i != ent2->getDirectProperties().end(); ++i)
            {
                properties.push_back(*i);
            }
        }
        break;
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::checkType");
    }
}

void checkDefaultInterfaces(
    std::set< OUString >& interfaces,
    const std::set< OUString >& services,
    const OUString & propertyhelper)
{
    if ( services.empty() ) {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") != interfaces.end())
            interfaces.erase("com.sun.star.lang.XServiceInfo");
    } else {
        if (interfaces.find("com.sun.star.lang.XServiceInfo") == interfaces.end())
            interfaces.insert("com.sun.star.lang.XServiceInfo");
    }

    if ( propertyhelper.equals("_") ) {
        if (interfaces.find("com.sun.star.beans.XPropertySet")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XPropertySet");
        if (interfaces.find("com.sun.star.beans.XFastPropertySet")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XFastPropertySet");
        if (interfaces.find("com.sun.star.beans.XPropertyAccess")
            != interfaces.end())
            interfaces.erase("com.sun.star.beans.XPropertyAccess");
    }
}

bool checkServiceProperties(rtl::Reference< TypeManager > const & manager,
                            OUString const & name)
{
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent)
        == codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE)
    {
        rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
            dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                ent.get()));
        assert(ent2.is());
        if (!ent2->getDirectProperties().empty()) {
            return true;
        }
        for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                 ent2->getDirectMandatoryBaseServices().begin());
             i != ent2->getDirectMandatoryBaseServices().end(); ++i)
        {
            if (checkServiceProperties(manager, i->name)) {
                return true;
            }
        }
    }
    return false;
}


OUString checkPropertyHelper(
    ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    const std::set< OUString >& services,
    const std::set< OUString >& interfaces,
    AttributeInfo& attributes,
    std::set< OUString >& propinterfaces)
{
    std::set< OUString >::const_iterator iter;
    std::set< OUString >::const_iterator end;

    if ( !services.empty() ) {
        iter = services.begin();
        end = services.end();
    } else {
        iter = interfaces.begin();
        end = interfaces.end();
    }

    bool oldStyleWithProperties = false;
    while ( iter != end ) {
        rtl::Reference< unoidl::Entity > ent;
        codemaker::UnoType::Sort sort = manager->getSort(*iter, &ent);
        if ( !services.empty() ) {
            if (options.supportpropertysetmixin
                && (sort
                    == codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE))
            {
                rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity >
                    ent2(
                        dynamic_cast<
                        unoidl::SingleInterfaceBasedServiceEntity * >(
                            ent.get()));
                assert(ent2.is());
                checkAttributes(
                    manager, ent2->getBase(), attributes, propinterfaces);
                if (!(attributes.empty() || propinterfaces.empty())) {
                    return ent2->getBase();
                }
            } else {
                oldStyleWithProperties = checkServiceProperties(manager, *iter);
            }
        } else {
            checkAttributes(manager, *iter, attributes, propinterfaces);
            if (!(attributes.empty() || propinterfaces.empty())) {
                return *iter;
            }
        }
        ++iter;
    }

    return oldStyleWithProperties ? OUString("_") : OUString();
}

bool checkXComponentSupport(
    rtl::Reference< TypeManager > const & manager, OUString const & name)
{
    assert(manager.is());
    if (name == "com.sun.star.lang.XComponent") {
        return true;
    }
    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = manager->getSort(name, &ent);
    if (sort != codemaker::UnoType::SORT_INTERFACE_TYPE) {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::checkXComponentSupport");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             ent2->getDirectMandatoryBases().begin());
         i != ent2->getDirectMandatoryBases().end(); ++i)
    {
        if (checkXComponentSupport(manager, i->name)) {
            return true;
        }
    }
    return false;
}


// if XComponent is directly specified, return true and remove it from the
// supported interfaces list
bool checkXComponentSupport(rtl::Reference< TypeManager > const & manager,
                            std::set< OUString >& interfaces)
{
    if ( interfaces.empty() )
        return false;

    std::set< OUString >::const_iterator iter = interfaces.begin();
    while ( iter != interfaces.end() ) {
        if ( (*iter).equals("com.sun.star.lang.XComponent") ) {
            interfaces.erase("com.sun.star.lang.XComponent");
            return true;
        }
        if ( checkXComponentSupport(manager, *iter) )
            return true;
        ++iter;
    }

    return false;
}

unoidl::AccumulationBasedServiceEntity::Property::Attributes
checkAdditionalPropertyFlags(
    unoidl::InterfaceTypeEntity::Attribute const & attribute)
{
    int flags = 0;
    bool getterSupportsUnknown = false;
    for (std::vector< OUString >::const_iterator i(
             attribute.getExceptions.begin());
         i != attribute.getExceptions.end(); ++i)
    {
        if (*i == "com.sun.star.beans.UnknownPropertyException") {
            getterSupportsUnknown = true;
        }
    }
    for (std::vector< OUString >::const_iterator i(
             attribute.setExceptions.begin());
         i != attribute.setExceptions.end(); ++i)
    {
        if (*i == "com.sun.star.beans.PropertyVetoException") {
            flags |= unoidl::AccumulationBasedServiceEntity::Property::
                ATTRIBUTE_CONSTRAINED;
        } else if (getterSupportsUnknown
                   && *i == "com.sun.star.beans.UnknownPropertyException")
        {
            flags |= unoidl::AccumulationBasedServiceEntity::Property::
                ATTRIBUTE_OPTIONAL;
        }
    }
    return unoidl::AccumulationBasedServiceEntity::Property::Attributes(flags);
}

// This function checks if the specified types for parameters and return
// types are allowed add-in types, for more info see the com.sun.star.sheet.AddIn
// service description
bool checkAddinType(rtl::Reference< TypeManager > const & manager,
                    OUString const & type, bool & bLastAny,
                    bool & bHasXPropertySet, bool bIsReturn)
{
    assert(manager.is());
    sal_Int32 rank;
    codemaker::UnoType::Sort sort = manager->decompose(
        type, true, 0, &rank, 0, 0);

    if ( sort == codemaker::UnoType::SORT_LONG ||
         sort == codemaker::UnoType::SORT_DOUBLE ||
         sort == codemaker::UnoType::SORT_STRING )
    {
        if ( rank == 0 || rank ==2 )
            return true;
    }
    if ( sort == codemaker::UnoType::SORT_ANY )
    {
        if ( rank <= 2 ) {
            if ( rank ==1 ) {
                if ( bIsReturn )
                    return false;
                bLastAny = true;
            }

            return true;
        }
    }
    if ( sort == codemaker::UnoType::SORT_INTERFACE_TYPE )
    {
        if ( bIsReturn && type == "com.sun.star.sheet.XVolatileResult" )
            return true;
        if ( !bIsReturn && type == "com.sun.star.table.XCellRange" )
            return true;
        if ( !bIsReturn && type == "com.sun.star.beans.XPropertySet" )
        {
            if ( bHasXPropertySet ) {
                return false;
            } else {
                bHasXPropertySet = true;
                return true;
            }
        }
    }
    return false;
}

void checkAddInTypes(
    rtl::Reference< TypeManager > const & manager, OUString const & name,
    rtl::Reference< unoidl::InterfaceTypeEntity > const & entity)
{
    assert(entity.is());
    bool bLastAny = false;
    bool bHasXPropertySet = false;
    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             entity->getDirectMethods().begin());
         i != entity->getDirectMethods().end(); ++i)
    {
        if ( !checkAddinType(
                 manager, i->returnType, bLastAny, bHasXPropertySet, true) )
        {
            throw CannotDumpException(
                "the return type of the calc add-in function '" + name
                + ":" + i->name
                + "' is invalid. Please check your IDL defintion.");
        }

        bHasXPropertySet = false;
        for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                 const_iterator j(i->parameters.begin());
             j != i->parameters.end(); ++j)
        {
            bLastAny = false;
            if ( !checkAddinType(manager, j->type,
                                bLastAny, bHasXPropertySet, false) ||
                 bLastAny )
            {
                throw CannotDumpException(
                    "the type of the " + j->name
                    + " parameter of the calc add-in function '" + name
                    + ":" + i->name + "' is invalid."
                    + (bLastAny
                       ? OUString(
                           " The type 'sequence<any>' is allowed as last"
                           " parameter only.")
                       : OUString())
                    + (bHasXPropertySet
                       ? OUString(
                           " The type 'XPropertySet' is allowed only once.")
                       : OUString())
                    + " Please check your IDL definition.");
            }
        }
    }
}

void generateFunctionParameterMap(std::ostream& o,
                                 ProgramOptions const & options,
                                 rtl::Reference< TypeManager > const & manager,
                                 OUString const & name,
                                 ::codemaker::GeneratedTypeSet & generated,
                                 bool bFirst)
{
    if ( name == "com.sun.star.uno.XInterface" ||
         name == "com.sun.star.lang.XLocalizable" ||
         name == "com.sun.star.lang.XServiceInfo" ||
         // the next three checks becomes obsolete when configuration is used
         name == "com.sun.star.sheet.XAddIn" ||
         name == "com.sun.star.sheet.XCompatibilityNames" ||
         name == "com.sun.star.lang.XServiceName" )
    {
        return;
    }

    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = manager->getSort(name, &ent);
    if (sort != codemaker::UnoType::SORT_INTERFACE_TYPE) {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::generateFunctionParameterMap");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());

    // check if the specified add-in functions supports valid types
    checkAddInTypes(manager, name, ent2);

    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             ent2->getDirectMandatoryBases().begin());
         i != ent2->getDirectMandatoryBases().end(); ++i)
    {
        generateFunctionParameterMap(
            o, options, manager, i->name, generated, bFirst);
    }

    if ( generated.contains(u2b(name)) )
        return;
    else
        generated.add(u2b(name));

    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             ent2->getDirectMethods().begin());
         i != ent2->getDirectMethods().end(); ++i)
    {
        if ( bFirst ) {
            if (options.language == 2) {
                o << "        ParamMap fpm;\n";
            }
            else {
                o << "        java.util.Hashtable< Integer, String > fpm = "
                    "new java.util.Hashtable< Integer, String >();\n";
            }
            bFirst = false;
        } else
            if ( options.language == 2 ) {
                o << "        fpm = ParamMap();\n";
            }
            else {
                o << "        fpm = new java.util.Hashtable< "
                    "Integer, String >();\n";
            }

        std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::size_type
            n = 0;
        for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
                 const_iterator j(i->parameters.begin());
             j != i->parameters.end(); ++j)
        {
            if ( options.language == 2 ) {
                o << "        fpm[" << n
                  << "] = ::rtl::OUString(\""
                  << j->name
                  << "\");\n";
            }
            else {
                o << "        fpm.put(" << n << ", \""
                  << j->name
                  << "\");\n";
            }
            ++n;
        }

        if ( options.language == 2 ) {
            o << "        m_functionMap[::rtl::OUString(\""
              << i->name << "\")] = fpm;\n\n";
        }
        else {
            o << "        m_functionMap.put(\"" << i->name << "\", fpm);\n\n";
        }
    }
}

void generateFunctionParameterMap(std::ostream& o,
         ProgramOptions const & options,
         rtl::Reference< TypeManager > const & manager,
         const std::set< OUString >& interfaces)
{
    ::codemaker::GeneratedTypeSet generated;
    bool bFirst = true;
    std::set< OUString >::const_iterator iter = interfaces.begin();
    while ( iter != interfaces.end() ) {
        generateFunctionParameterMap(o, options, manager, *iter, generated, bFirst);
        ++iter;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
