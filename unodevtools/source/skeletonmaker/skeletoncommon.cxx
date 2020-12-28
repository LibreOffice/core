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

#include <osl/thread.hxx>

#include <codemaker/commonjava.hxx>
#include <codemaker/commoncpp.hxx>
#include <codemaker/generatedtypeset.hxx>
#include <codemaker/global.hxx>
#include <unoidl/unoidl.hxx>

#include "skeletoncommon.hxx"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string_view>

using namespace ::codemaker::cpp;

namespace skeletonmaker {

void printLicenseHeader(std::ostream& o, OString const & filename)
{
    sal_Int32 index;
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
    if ( options.outputpath == "stdout" )
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
    }
    tmpSourceFileName = file.getName();
    file.close();
    *ppOutputStream = new std::ofstream(tmpSourceFileName.getStr(),
                                        std::ios_base::binary);

    return bStandardout;
}

static bool containsAttribute(AttributeInfo& attributes, OUString const & attrname)
{
    return std::any_of(attributes.begin(), attributes.end(),
        [&attrname](const unoidl::AccumulationBasedServiceEntity::Property& rAttr) {
            return rAttr.name == attrname; });
}

// collect attributes including inherited attributes
static void checkAttributes(rtl::Reference< TypeManager > const & manager,
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
    case codemaker::UnoType::Sort::Interface:
        {
            rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
                dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
            assert(ent2.is());
            for (const auto& rBase : ent2->getDirectMandatoryBases())
            {
                checkAttributes(manager, rBase.name, attributes, propinterfaces);
            }
            for (const auto& rAttr : ent2->getDirectAttributes())
            {
                if (!containsAttribute(attributes, rAttr.name)) {
                    attributes.emplace_back(
                        rAttr.name, rAttr.type,
                        (unoidl::AccumulationBasedServiceEntity::Property::
                         Attributes(
                             ((rAttr.bound
                               ? unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_BOUND
                               : 0)
                              | (rAttr.readOnly
                                 ? unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_READ_ONLY
                                 : 0)))),
                        std::vector< OUString >());
                }
            }
            break;
        }
    case codemaker::UnoType::Sort::AccumulationBasedService:
        {
            rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            for (const auto& rService : ent2->getDirectMandatoryBaseServices())
            {
                checkAttributes(manager, rService.name, attributes, propinterfaces);
            }
            for (const auto& rIface : ent2->getDirectMandatoryBaseInterfaces())
            {
                checkAttributes(manager, rIface.name, attributes, propinterfaces);
            }
            for (const auto& rProp : ent2->getDirectProperties())
            {
                if (!containsAttribute(attributes, rProp.name)) {
                    attributes.push_back(rProp);
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
    case codemaker::UnoType::Sort::Interface:
        // com.sun.star.lang.XComponent should be also not in the list
        // but it will be used for checking the impl helper and will be
        // removed later if necessary.
        if ( name == "com.sun.star.lang.XTypeProvider" ||
             name == "com.sun.star.uno.XWeak" )
            return;
        interfaceTypes.insert(name);
        break;
    case codemaker::UnoType::Sort::SingleInterfaceBasedService:
        if (serviceTypes.insert(name).second) {
            rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
                dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            if (interfaceTypes.insert(ent2->getBase()).second) {
                // check if constructors are specified, if yes automatically
                // support of XInitialization. We will take care of the default
                // constructor because in this case XInitialization is not
                // called.
                if (ent2->getConstructors().size() > 1 ||
                    (ent2->getConstructors().size() == 1 &&
                     !ent2->getConstructors()[0].defaultConstructor))
                {
                    interfaceTypes.insert(OUString("com.sun.star.lang.XInitialization"));
                }
            }
        }
        break;
    case codemaker::UnoType::Sort::AccumulationBasedService:
        if ( serviceTypes.insert(name).second ) {
            rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    ent.get()));
            assert(ent2.is());
            for (const auto& rService : ent2->getDirectMandatoryBaseServices())
            {
                checkType(
                    manager, rService.name, interfaceTypes, serviceTypes, properties);
            }
            for (const auto& rIface : ent2->getDirectMandatoryBaseInterfaces())
            {
                checkType(
                    manager, rIface.name, interfaceTypes, serviceTypes, properties);
            }
            for (const auto& rProp : ent2->getDirectProperties())
            {
                properties.push_back(rProp);
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
    std::u16string_view propertyhelper)
{
    if ( services.empty() ) {
        interfaces.erase("com.sun.star.lang.XServiceInfo");
    } else {
        interfaces.insert("com.sun.star.lang.XServiceInfo");
    }

    if ( propertyhelper == u"_" ) {
        interfaces.erase("com.sun.star.beans.XPropertySet");
        interfaces.erase("com.sun.star.beans.XFastPropertySet");
        interfaces.erase("com.sun.star.beans.XPropertyAccess");
    }
}

static bool checkServiceProperties(rtl::Reference< TypeManager > const & manager,
                            OUString const & name)
{
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent)
        == codemaker::UnoType::Sort::AccumulationBasedService)
    {
        rtl::Reference< unoidl::AccumulationBasedServiceEntity > ent2(
            dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                ent.get()));
        assert(ent2.is());
        if (!ent2->getDirectProperties().empty()) {
            return true;
        }
        return std::any_of(ent2->getDirectMandatoryBaseServices().begin(),
            ent2->getDirectMandatoryBaseServices().end(),
            [&manager](const unoidl::AnnotatedReference& rService) {
                return checkServiceProperties(manager, rService.name); });
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
                    == codemaker::UnoType::Sort::SingleInterfaceBasedService))
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

static bool checkXComponentSupport(
    rtl::Reference< TypeManager > const & manager, OUString const & name)
{
    assert(manager.is());
    if (name == "com.sun.star.lang.XComponent") {
        return true;
    }
    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = manager->getSort(name, &ent);
    if (sort != codemaker::UnoType::Sort::Interface) {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::checkXComponentSupport");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());
    return std::any_of(ent2->getDirectMandatoryBases().begin(), ent2->getDirectMandatoryBases().end(),
        [&manager](const unoidl::AnnotatedReference& rBase) { return checkXComponentSupport(manager, rBase.name); });
}


// if XComponent is directly specified, return true and remove it from the
// supported interfaces list
bool checkXComponentSupport(rtl::Reference< TypeManager > const & manager,
                            std::set< OUString >& interfaces)
{
    if ( interfaces.empty() )
        return false;

    for ( const auto& rIface : interfaces ) {
        if ( rIface == "com.sun.star.lang.XComponent" ) {
            interfaces.erase("com.sun.star.lang.XComponent");
            return true;
        }
        if ( checkXComponentSupport(manager, rIface) )
            return true;
    }

    return false;
}

unoidl::AccumulationBasedServiceEntity::Property::Attributes
checkAdditionalPropertyFlags(
    unoidl::InterfaceTypeEntity::Attribute const & attribute)
{
    int flags = 0;
    bool getterSupportsUnknown = false;
    for (const auto& rException : attribute.getExceptions)
    {
        if (rException == "com.sun.star.beans.UnknownPropertyException") {
            getterSupportsUnknown = true;
        }
    }
    for (const auto& rException : attribute.setExceptions)
    {
        if (rException == "com.sun.star.beans.PropertyVetoException") {
            flags |= unoidl::AccumulationBasedServiceEntity::Property::
                ATTRIBUTE_CONSTRAINED;
        } else if (getterSupportsUnknown
                   && rException == "com.sun.star.beans.UnknownPropertyException")
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
static bool checkAddinType(rtl::Reference< TypeManager > const & manager,
                    std::u16string_view type, bool & bLastAny,
                    bool & bHasXPropertySet, bool bIsReturn)
{
    assert(manager.is());
    sal_Int32 rank;
    codemaker::UnoType::Sort sort = manager->decompose(
        type, true, nullptr, &rank, nullptr, nullptr);

    if ( sort == codemaker::UnoType::Sort::Long ||
         sort == codemaker::UnoType::Sort::Double ||
         sort == codemaker::UnoType::Sort::String )
    {
        if ( rank == 0 || rank ==2 )
            return true;
    }
    if ( sort == codemaker::UnoType::Sort::Any )
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
    if ( sort == codemaker::UnoType::Sort::Interface )
    {
        if ( bIsReturn && type == u"com.sun.star.sheet.XVolatileResult" )
            return true;
        if ( !bIsReturn && type == u"com.sun.star.table.XCellRange" )
            return true;
        if ( !bIsReturn && type == u"com.sun.star.beans.XPropertySet" )
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

static void checkAddInTypes(
    rtl::Reference< TypeManager > const & manager, std::u16string_view name,
    rtl::Reference< unoidl::InterfaceTypeEntity > const & entity)
{
    assert(entity.is());
    bool bLastAny = false;
    bool bHasXPropertySet = false;
    for (const auto& rMethod : entity->getDirectMethods())
    {
        if ( !checkAddinType(
                 manager, rMethod.returnType, bLastAny, bHasXPropertySet, true) )
        {
            throw CannotDumpException(
                OUString::Concat("the return type of the calc add-in function '") + name
                + ":" + rMethod.name
                + "' is invalid. Please check your IDL definition.");
        }

        bHasXPropertySet = false;
        for (const auto& rParam : rMethod.parameters)
        {
            bLastAny = false;
            if ( !checkAddinType(manager, rParam.type,
                                bLastAny, bHasXPropertySet, false) ||
                 bLastAny )
            {
                throw CannotDumpException(
                    "the type of the " + rParam.name
                    + " parameter of the calc add-in function '" + name
                    + ":" + rMethod.name + "' is invalid."
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

static void generateFunctionParameterMap(std::ostream& o,
                                 ProgramOptions const & options,
                                 rtl::Reference< TypeManager > const & manager,
                                 OUString const & name,
                                 ::codemaker::GeneratedTypeSet & generated,
                                 bool& bFirst)
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
    if (sort != codemaker::UnoType::Sort::Interface) {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::generateFunctionParameterMap");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());

    // check if the specified add-in functions supports valid types
    checkAddInTypes(manager, name, ent2);

    for (const auto& rBase : ent2->getDirectMandatoryBases())
    {
        generateFunctionParameterMap(
            o, options, manager, rBase.name, generated, bFirst);
    }

    if ( generated.contains(u2b(name)) )
        return;
    else
        generated.add(u2b(name));

    for (const auto& rMethod : ent2->getDirectMethods())
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
        for (const auto& rParam : rMethod.parameters)
        {
            if ( options.language == 2 ) {
                o << "        fpm[" << n
                  << "] = OUString(\""
                  << rParam.name
                  << "\");\n";
            }
            else {
                o << "        fpm.put(" << n << ", \""
                  << rParam.name
                  << "\");\n";
            }
            ++n;
        }

        if ( options.language == 2 ) {
            o << "        m_functionMap[OUString(\""
              << rMethod.name << "\")] = fpm;\n\n";
        }
        else {
            o << "        m_functionMap.put(\"" << rMethod.name << "\", fpm);\n\n";
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
    for ( const auto& rIface : interfaces ) {
        generateFunctionParameterMap(o, options, manager, rIface, generated, bFirst);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
