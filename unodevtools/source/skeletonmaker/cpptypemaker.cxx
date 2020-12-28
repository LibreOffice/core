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

#include <sal/config.h>

#include <codemaker/codemaker.hxx>
#include <codemaker/commoncpp.hxx>
#include <codemaker/global.hxx>

#include "skeletoncommon.hxx"
#include "skeletoncpp.hxx"

#include <algorithm>
#include <string_view>

using namespace ::codemaker::cpp;

namespace skeletonmaker::cpp {

static void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort, OUString const & nucleus, sal_Int32 rank,
    std::vector< OUString > const & arguments,
    rtl::Reference< unoidl::Entity > const & entity, short referenceType,
    bool defaultvalue)
{
    if (defaultvalue && rank == 0 && sort <= codemaker::UnoType::Sort::Char) {
        switch (sort) {
        case codemaker::UnoType::Sort::Boolean:
            o << "sal_False";
            return;
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double:
            o << "0";
            return;
        default:
            break;
        }
    }

    if (defaultvalue && referenceType == 16) {
        if (sort == codemaker::UnoType::Sort::Enum) {
            auto pEnumTypeEntity(dynamic_cast<unoidl::EnumTypeEntity *>(entity.get()));
            assert(pEnumTypeEntity);
            o << nucleus.copy(nucleus.lastIndexOf('.') + 1) << "_"
              << pEnumTypeEntity->getMembers()[0].name;
        }
        return;
    }
    bool bReference = false;
    if (((sort > codemaker::UnoType::Sort::Char ||
          rank > 0) && referenceType != 8 &&
         !(sort == codemaker::UnoType::Sort::Enum && referenceType == 4 && rank == 0)) ||
        (sort <= codemaker::UnoType::Sort::Char && referenceType == 2))
    {
        bReference = true;
    }

    if (bReference && referenceType == 4)
        o << "const ";

    for (sal_Int32 i = 0; i < rank; ++i) {
        o << ((options.shortnames) ? "css::uno::Sequence< " :
              "::com::sun::star::uno::Sequence< ");
    }
    if (sort == codemaker::UnoType::Sort::Interface && referenceType > 0) {
        o << ((options.shortnames) ? "css::uno::Reference< " :
              "::com::sun::star::uno::Reference< ");
    }

    o << scopedCppName(codemaker::cpp::translateUnoToCppType(sort, nucleus),
                       options.shortnames && referenceType > 0);

    if (sort == codemaker::UnoType::Sort::Interface && referenceType > 0)
        o << " >";

    if (!arguments.empty()) {
        o << "< ";
        for (std::vector< OUString >::const_iterator i(arguments.begin());
             i != arguments.end(); ++i)
        {
            if (i != arguments.begin())
                o << ", ";

            printType(o, options, manager, *i, 1);
        }
        o << " >";
    }

    for (sal_Int32 i = 0; i < rank; ++i)
        o << " >";

    if (bReference && referenceType > 1)
        o << " &";

    if (referenceType == 8 && (sort > codemaker::UnoType::Sort::Char || rank > 0))
        o << "()";
}

void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, std::u16string_view name,
    short referenceType, bool defaultvalue)
{
    OUString nucleus;
    sal_Int32 rank;
    std::vector< OUString > arguments;
    rtl::Reference< unoidl::Entity > entity;
    codemaker::UnoType::Sort sort = manager->decompose(
        name, true, &nucleus, &rank, &arguments, &entity);
    printType(
        o, options, manager, sort, nucleus, rank, arguments, entity,
        referenceType, defaultvalue);
}

static bool printConstructorParameters(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort,
    rtl::Reference< unoidl::Entity > const & entity, std::u16string_view name,
    std::vector< OUString > const & arguments)
{
    bool previous = false;
    switch (sort) {
    case codemaker::UnoType::Sort::PlainStruct:
        {
            rtl::Reference< unoidl::PlainStructTypeEntity > ent2(
                dynamic_cast< unoidl::PlainStructTypeEntity * >(entity.get()));
            assert(ent2.is());
            if (!ent2->getDirectBase().isEmpty()) {
                rtl::Reference< unoidl::Entity > baseEnt;
                codemaker::UnoType::Sort baseSort = manager->getSort(
                    ent2->getDirectBase(), &baseEnt);
                previous = printConstructorParameters(
                    o, options, manager, baseSort, baseEnt,
                    ent2->getDirectBase(), std::vector< OUString >());
            }
            for (const auto& rMember : ent2->getDirectMembers())
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                printType(o, options, manager, rMember.type, 4);
                o << ' '
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      u2b(rMember.name), "param");
            }
            break;
        }
    case codemaker::UnoType::Sort::PolymorphicStructTemplate:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    entity.get()));
            assert(ent2.is());
            for (const auto& rMember : ent2->getMembers())
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                if (rMember.parameterized) {
                    o << rMember.type;
                } else {
                    printType(o, options, manager, rMember.type, 4);
                }
                o << ' '
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      u2b(rMember.name), "param");
            }
            break;
        }
    case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    entity.get()));
            assert(ent2.is());
            for (const auto& rMember : ent2->getMembers())
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                if (rMember.parameterized) {
                    auto j = std::find(ent2->getTypeParameters().begin(),
                        ent2->getTypeParameters().end(), rMember.type);
                    if (j != ent2->getTypeParameters().end()) {
                        o << arguments[j - ent2->getTypeParameters().begin()];
                    }
                } else {
                    printType(o, options, manager, rMember.type, 4);
                }
                o << ' '
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      u2b(rMember.name), "param");
            }
            break;
        }
    case codemaker::UnoType::Sort::Exception:
        {
            rtl::Reference< unoidl::ExceptionTypeEntity > ent2(
                dynamic_cast< unoidl::ExceptionTypeEntity * >(entity.get()));
            assert(ent2.is());
            if (!ent2->getDirectBase().isEmpty()) {
                rtl::Reference< unoidl::Entity > baseEnt;
                codemaker::UnoType::Sort baseSort = manager->getSort(
                    ent2->getDirectBase(), &baseEnt);
                previous = printConstructorParameters(
                    o, options, manager, baseSort, baseEnt,
                    ent2->getDirectBase(), std::vector< OUString >());
            }
            for (const auto& rMember : ent2->getDirectMembers())
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                printType(o, options, manager, rMember.type, 4);
                o << ' '
                  << codemaker::cpp::translateUnoToCppIdentifier(
                      u2b(rMember.name), "param");
            }
            break;
        }
    default:
        throw CannotDumpException(
            OUString::Concat("unexpected entity \"") + name
            + "\" in call to skeletonmaker::cpp::printConstructorParameters");
    }
    return previous;
}

static void printConstructor(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort,
    rtl::Reference< unoidl::Entity > const & entity, OUString const & name,
    std::vector< OUString > const & arguments)
{
    o << "public " << name.copy(name.lastIndexOf('.') + 1) << '(';
    printConstructorParameters(
        o, options, manager, sort, entity, name, arguments);
    o << ");\n";
}

static void printMethodParameters(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    std::vector< unoidl::InterfaceTypeEntity::Method::Parameter > const &
        parameters,
    bool withType)
{
    for (std::vector< unoidl::InterfaceTypeEntity::Method::Parameter >::
             const_iterator i(parameters.begin());
         i != parameters.end(); ++i)
    {
        if (i != parameters.begin()) {
            o << ", ";
        }
        if (withType) {
            short referenceType;
            if (i->direction
                == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            {
                referenceType = 4;
            } else {
                referenceType = 2;
            }
            printType(o, options, manager, i->type, referenceType);
            o << ' ';
        }
        o << codemaker::cpp::translateUnoToCppIdentifier(u2b(i->name), "param");
    }
}

static void printExceptionSpecification(
    std::ostream & o,
    ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    std::vector< OUString > const & exceptions)
{
    o << ((options.shortnames) ? " throw (css::uno::RuntimeException" :
          " throw (::com::sun::star::uno::RuntimeException");
    for (const auto& rException : exceptions)
    {
        o << ", ";
        printType(o, options, manager, rException, 1);
    }
    o << ")";
}

static void printSetPropertyMixinBody(
    std::ostream & o, unoidl::InterfaceTypeEntity::Attribute const & attribute)
{
    unoidl::AccumulationBasedServiceEntity::Property::Attributes propFlags
        = checkAdditionalPropertyFlags(attribute);

    o << "\n{\n";

    if (attribute.bound)
        o << "    BoundListeners l;\n";

    if (propFlags & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_CONSTRAINED) {
        OString fieldtype = codemaker::convertString(attribute.type);

        sal_Int32 index = fieldtype.lastIndexOf('<');
        sal_Int32 nPos=0;
        bool single = true;
        bool optional = false;
        OStringBuffer buffer1(64);
        OStringBuffer buffer2(64);
        do
        {
            OString s(fieldtype.getToken(0, '<', nPos));
            OString t = s.copy(s.lastIndexOf('/')+1);

            if (t == "Optional") {
                optional=true;
                if (single) {
                    single=false;
                    buffer1.append("the_value.IsPresent");
                    buffer2.append("the_value.Value");
                } else {
                    buffer1.insert(0, t);
                    buffer1.append(".IsPresent");
                    buffer2.insert(0, t);
                    buffer2.append(".Value");
                }
            } else {
                if (single) {
                    single=false;
                    if (!optional)
                        buffer1.append("the_value.Value");

                    buffer2.append("the_value.Value");
                } else {
                    if (!optional) {
                        buffer1.insert(0, t);
                        buffer1.append(".Value");
                    }
                    buffer2.insert(0, t);
                    buffer2.append(".Value");
                }
            }
        } while( nPos <= index );

        o << "    css::uno::Any v;\n";
        if (optional) {
            o << "    if(" << buffer1.makeStringAndClear() << ")\n    {\n        v <<= " << buffer2.makeStringAndClear() << ";\n    }\n";
        } else {
            o << "    v <<= " << buffer2.makeStringAndClear() << ";\n\n";
        }

        o << "    prepareSet(\n        OUString(\""
          << attribute.name << "\"),\n        css::uno::Any(), v, ";
    } else {
        o << "    prepareSet(\n        OUString(\""
          << attribute.name << "\"),\n        css::uno::Any(), css::uno::Any(), ";
    }

    if (attribute.bound)
        o << "&l);\n";
    else
        o << "0);\n";

    o << "    {\n        osl::MutexGuard g(m_aMutex);\n        m_"
      << attribute.name << " = the_value;\n    }\n";

    if (attribute.bound)
        o << "    l.notify();\n";

    o  << "}\n\n";
}

void printMethods(std::ostream & o,
    ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
    OUString const & name, codemaker::GeneratedTypeSet & generated,
    OString const & delegate, OString const & classname,
    OString const & indentation, bool defaultvalue,
    OUString const & propertyhelper)
{
    if (generated.contains(u2b(name)) || name == "com.sun.star.uno.XInterface" ||
        (defaultvalue &&
         ( name == "com.sun.star.lang.XComponent" ||
           name == "com.sun.star.lang.XTypeProvider" ||
           name == "com.sun.star.uno.XWeak" ) ) )
    {
        return;
    }

    static OString sd("_");
    bool body = !delegate.isEmpty();
    bool defaultbody = delegate == sd;

    if (body && propertyhelper.getLength() > 1) {
        if (name == "com.sun.star.beans.XPropertySet") {
            generated.add(u2b(name));
            generateXPropertySetBodies(
                o, classname, scopedCppName(u2b(propertyhelper)));
            return;
        } else if (name == "com.sun.star.beans.XFastPropertySet") {
            generated.add(u2b(name));
            generateXFastPropertySetBodies(
                o, classname, scopedCppName(u2b(propertyhelper)));
            return;
        } else if (name == "com.sun.star.beans.XPropertyAccess") {
            generated.add(u2b(name));
            generateXPropertyAccessBodies(
                o, classname, scopedCppName(u2b(propertyhelper)));
            return;
        }
    }

    if (body && options.componenttype == 2) {
        if (name == "com.sun.star.lang.XServiceName") {
            o << "// ::com::sun::star::lang::XServiceName:\n"
                "OUString SAL_CALL " << classname << "getServiceName() "
                "throw (css::uno::RuntimeException)\n{\n    "
                "return OUString("
                "sADDIN_SERVICENAME);\n}\n";
            generated.add(u2b(name));
            return;
        } else if (name == "com.sun.star.sheet.XAddIn") {
            generateXAddInBodies(o, classname);
            generated.add(u2b(name));

            // special handling of XLocalizable -> parent of XAddIn
            if (!generated.contains("com.sun.star.lang.XLocalizable")) {
                generateXLocalizable(o, classname);
                generated.add("com.sun.star.lang.XLocalizable");
            }
            return;
        } else if (name == "com.sun.star.lang.XLocalizable") {
            generateXLocalizable(o, classname);
            generated.add(u2b(name));
            return;
        } else if (name == "com.sun.star.sheet.XCompatibilityNames") {
            generateXCompatibilityNamesBodies(o, classname);
            generated.add(u2b(name));
            return;
        }
    }

    if (body && options.componenttype == 3) {
        if (name == "com.sun.star.lang.XInitialization") {
            generateXInitialization(o, classname);
            generated.add(u2b(name));
            return;
        } else if (name == "com.sun.star.frame.XDispatch") {
            generateXDispatch(o, classname, options.protocolCmdMap);
            generated.add(u2b(name));
            return;
        } else if (name == "com.sun.star.frame.XDispatchProvider") {
            generateXDispatchProvider(o, classname, options.protocolCmdMap);
            generated.add(u2b(name));
            return;
        }
    }

    generated.add(u2b(name));
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent) != codemaker::UnoType::Sort::Interface)
    {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::cpp::printMethods");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());
    if (options.all || defaultvalue) {
        for (const auto& rBase : ent2->getDirectMandatoryBases())
        {
            printMethods(
                o, options, manager, rBase.name, generated, delegate, classname,
                indentation, defaultvalue, propertyhelper);
        }
        if (!(ent2->getDirectAttributes().empty()
              && ent2->getDirectMethods().empty()))
        {
            o << indentation << "// ";
            printType(o, options, manager, name, 0);
            o << ":\n";
        }
    }
    for (const auto& rAttr : ent2->getDirectAttributes())
    {
        o << indentation;
        if (!body)
            o << "virtual ";

        printType(o, options, manager, rAttr.type, 1);
        o << " SAL_CALL ";
        if (!classname.isEmpty())
            o << classname;

        o << "get" << rAttr.name << "()";
        printExceptionSpecification(o, options, manager, rAttr.getExceptions);
        if (body) {
            if (defaultbody) {
                if (!propertyhelper.isEmpty()) {
                    o << "\n{\n    osl::MutexGuard g(m_aMutex);\n    return m_"
                      << rAttr.name << ";\n}\n\n";
                } else {
                    o << "\n{\n    return ";
                    if (options.componenttype == 1) {
                        o << "m_" << rAttr.name;
                    } else {
                        printType(o, options, manager, rAttr.type, 8, true);
                    }
                    o << ";\n}\n\n";
                }
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    return "
                  << delegate << "get" << rAttr.name << "();\n"
                  << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }

        if (!rAttr.readOnly) {
            o << indentation;
            if (!body)
                o << "virtual ";

            o << "void SAL_CALL ";
            if (!classname.isEmpty())
                o << classname;

            o << "set" << rAttr.name << '(';
            printType(o, options, manager, rAttr.type, 4);
            o << " the_value)";
            printExceptionSpecification(o, options, manager, rAttr.setExceptions);
            if (body) {
                if (defaultbody) {
                    if (!propertyhelper.isEmpty()) {
                        printSetPropertyMixinBody(o, rAttr);
                    } else {
                        if (options.componenttype == 1) {
                            o << "\n{\n    m_" << rAttr.name
                              << " = the_value;\n}\n\n";
                        } else {
                            o << "\n{\n\n}\n\n";
                        }
                    }
                } else {
                    o << "\n" << indentation << "{\n" << indentation << "    "
                      << delegate << "set" << rAttr.name
                      << "(the_value);\n" << indentation << "}\n\n";
                }
            } else {
                o << ";\n";
            }
        }
    }
    for (const auto& rMethod : ent2->getDirectMethods())
    {
        o << indentation;
        if (!body)
            o << "virtual ";

        printType(o, options, manager, rMethod.returnType, 1);
        o << " SAL_CALL ";
        if (!classname.isEmpty())
            o << classname;

        o << rMethod.name << '(';
        printMethodParameters(o, options, manager, rMethod.parameters, true);
        o << ')';
        printExceptionSpecification(o, options, manager, rMethod.exceptions);
        if (body) {
            if (defaultbody) {
                o << "\n{\n";
                if (rMethod.returnType != "void") {
                    o << "    // TODO: Exchange the default return implementation for \""
                      << rMethod.name << "\" !!!\n";
                    o << "    // Exchange the default return implementation.\n"
                        "    // NOTE: Default initialized polymorphic structs "
                        "can cause problems because of\n    // missing default "
                        "initialization of primitive types of some C++ compilers or"
                        "\n    // different Any initialization in Java and C++ "
                        "polymorphic structs.\n    return ";
                    printType(o, options, manager, rMethod.returnType, 8, true);
                    o << ";";
                } else {
                    o << "    // TODO: Insert your implementation for \""
                      << rMethod.name << "\" here.";
                }
                o << "\n}\n\n";
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    ";
                if (rMethod.returnType != "void")
                    o << "return ";

                o << delegate << rMethod.name << '(';
                printMethodParameters(
                    o, options, manager, rMethod.parameters, false);
                o << ");\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }
    }

    if (!body)
        o << "\n";
}

static void printConstructors(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, OUString const & name)
{
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent)
        != codemaker::UnoType::Sort::SingleInterfaceBasedService)
    {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::java::printConstructors");
    }
    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
        dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(ent.get()));
    assert(ent2.is());
    for (const auto& rConstructor : ent2->getConstructors())
    {
        o << "static ";
        printType(o, options, manager, ent2->getBase(), 1);
        o << ' ';
        if (rConstructor.defaultConstructor) {
            o << "create";
        } else {
            o << codemaker::cpp::translateUnoToCppIdentifier(
                u2b(rConstructor.name), "method");
        }
        o << ((options.shortnames) ? "(css::uno::Reference< css" :
              "(::com::sun::star::uno::Reference< ::com::sun::star")
          << "::uno::XComponentContext > const & the_context";
        for (const auto& rParam : rConstructor.parameters)
        {
            o << ", ";
            printType(o, options, manager, rParam.type, 4);
            o << ' '
              << codemaker::cpp::translateUnoToCppIdentifier(
                  u2b(rParam.name), "param");
        }
        o << ')';
        printExceptionSpecification(o, options, manager, rConstructor.exceptions);
        o << ";\n";
    }
}

static void printServiceMembers(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    OUString const & name,
    rtl::Reference< unoidl::AccumulationBasedServiceEntity > const & entity,
    OString const & delegate)
{
    assert(entity.is());
    for (const auto& rService : entity->getDirectMandatoryBaseServices())
    {
        o << "\n// exported service " << rService.name << "\n";
        generateDocumentation(o, options, manager, u2b(rService.name), delegate);
        o << "\n// end of exported service " << rService.name << "\n";
    }
    for (const auto& rIface : entity->getDirectMandatoryBaseInterfaces())
    {
        o << "\n// supported interface " << rIface.name << "\n";
        generateDocumentation(o, options, manager, u2b(rIface.name), delegate);
    }
    if (delegate.isEmpty()) {
        o << "\n// properties of service \""<< name << "\"\n";
        for (const auto& rProp : entity->getDirectProperties())
        {
            o << "// private ";
            printType(o, options, manager, rProp.type, 1);
            o << " "
              << codemaker::cpp::translateUnoToCppIdentifier(
                  u2b(rProp.name), "property")
              << ";\n";
        }
    }
}

static void printMapsToCppType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort, OUString const & nucleus, sal_Int32 rank,
    std::vector< OUString > const & arguments,
    rtl::Reference< unoidl::Entity > const & entity, const char * cppTypeSort)
{
    o << "maps to C++ ";
    if (cppTypeSort != nullptr)
        o << cppTypeSort << ' ';

    o << "type \"";
    if (rank == 0 && nucleus == "com.sun.star.uno.XInterface") {
        o << "Reference< com::sun::star::uno::XInterface >";
    } else {
        printType(
            o, options, manager, sort, nucleus, rank, arguments, entity, 0,
            false);
    }
    o << '"';
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
    OString const & type, OString const & delegate)
{
    OUString nucleus;
    sal_Int32 rank;
    codemaker::UnoType::Sort sort = manager->decompose(
        b2u(type), false, &nucleus, &rank, nullptr, nullptr);

    bool comment = true;
    if (!delegate.isEmpty()) {
        if (sort != codemaker::UnoType::Sort::Interface &&
            sort != codemaker::UnoType::Sort::SingleInterfaceBasedService &&
            sort != codemaker::UnoType::Sort::AccumulationBasedService )
        {
            return;
        }
        comment = false;
    }

    if (comment) {
        o << "\n// UNO";
        if (rank != 0) {
            o << " sequence type";
        } else if (sort <= codemaker::UnoType::Sort::Any) {
            o << " simple type";
        } else {
            switch (sort) {
            case codemaker::UnoType::Sort::Interface:
                o << " interface type";
                break;

            case codemaker::UnoType::Sort::Module:
                o << "IDL module";
                break;

            case codemaker::UnoType::Sort::PlainStruct:
                o << " simple struct type";
                break;

            case codemaker::UnoType::Sort::PolymorphicStructTemplate:
                o << " polymorphic struct type template";
                break;

            case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                o << " instantiated polymorphic struct type";
                break;

            case codemaker::UnoType::Sort::Enum:
                o << " enum type";
                break;

            case codemaker::UnoType::Sort::Exception:
                o << " exception type";
                break;

            case codemaker::UnoType::Sort::Typedef:
                o << "IDL typedef";
                break;

            case codemaker::UnoType::Sort::SingleInterfaceBasedService:
                o << " single-inheritance--based service";
                break;

            case codemaker::UnoType::Sort::AccumulationBasedService:
                o << "IDL accumulation-based service";
                break;

            case codemaker::UnoType::Sort::InterfaceBasedSingleton:
                o << " inheritance-based singleton";
                break;

            case codemaker::UnoType::Sort::ServiceBasedSingleton:
                o << "IDL service-based singleton";
                break;

            case codemaker::UnoType::Sort::ConstantGroup:
                o << "IDL constant group";
                break;

            default:
                OSL_ASSERT(false);
                break;
            }
        }
        o << " \"" << type << "\" ";
    }
    std::vector< OUString > arguments;
    rtl::Reference< unoidl::Entity > entity;
    sort = manager->decompose(
        b2u(type), true, &nucleus, &rank, &arguments, &entity);
    if (rank != 0) {
        if (comment) {
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "array");
            o << '\n';
        }
    } else if (sort <= codemaker::UnoType::Sort::Any) {
        if (comment) {
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity, nullptr);
            o << '\n';
        }
    } else {
        switch (sort) {
        case codemaker::UnoType::Sort::Interface:
            if (comment)
                printMapsToCppType(
                    o, options, manager, sort, nucleus, rank, arguments, entity,
                    "interface");
            if (nucleus == "com.sun.star.uno.XInterface") {
                if (comment)
                    o << '\n';
            } else {
                if (comment)
                    o << "; " << (options.all ? "all" : "direct") << " methods:\n";

                codemaker::GeneratedTypeSet generated;
                printMethods(
                    o, options, manager, nucleus, generated, delegate,
                    options.implname, "");
            }
            break;

        case codemaker::UnoType::Sort::Module:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "namespace");
            o << '\n';
            break;

        case codemaker::UnoType::Sort::PlainStruct:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "class");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager, codemaker::UnoType::Sort::PlainStruct,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::Sort::PolymorphicStructTemplate:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "class template");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager,
                codemaker::UnoType::Sort::PolymorphicStructTemplate,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "class template instantiation");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager,
                codemaker::UnoType::Sort::InstantiatedPolymorphicStruct,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::Sort::Enum:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "enum");
            o << '\n';
            break;

        case codemaker::UnoType::Sort::ConstantGroup:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "namespace");
            o << '\n';
            break;

        case codemaker::UnoType::Sort::Exception:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "exception class");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager, codemaker::UnoType::Sort::Exception,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::Sort::SingleInterfaceBasedService:
            if (comment) {
                printMapsToCppType(
                    o, options, manager, sort, nucleus, rank, arguments, entity,
                    "class");
                o << "; construction methods:\n";
                printConstructors(o, options, manager, nucleus);
            }
            generateDocumentation(
                o, options, manager,
                u2b(dynamic_cast<unoidl::SingleInterfaceBasedServiceEntity&>(*entity)
                    .getBase()),
                delegate);
            break;

        case codemaker::UnoType::Sort::AccumulationBasedService:
            if (comment)
                o << ("does not map to C++\n"
                      "// the service members are generated instead\n");
            printServiceMembers(
                o, options, manager, nucleus,
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    entity.get()),
                delegate);
            break;

        case codemaker::UnoType::Sort::InterfaceBasedSingleton:
            printMapsToCppType(
                o, options, manager, sort, nucleus, rank, arguments, entity,
                "class");
            o << "; get method:\nstatic ";
            printType(
                o, options, manager,
                dynamic_cast< unoidl::InterfaceBasedSingletonEntity & >(
                    *entity).getBase(),
                1);
            o << " get(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);\n";
            break;

        case codemaker::UnoType::Sort::ServiceBasedSingleton:
            o << "does not map to C++\n";
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
