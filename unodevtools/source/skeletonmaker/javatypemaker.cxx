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

#include "sal/config.h"

#include <cstring>

#include "codemaker/codemaker.hxx"
#include "codemaker/commonjava.hxx"
#include "codemaker/global.hxx"

#include "skeletoncommon.hxx"
#include "skeletonjava.hxx"

using namespace ::rtl;

namespace skeletonmaker { namespace java {

void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort, OUString const & nucleus, sal_Int32 rank,
    std::vector< OUString > const & arguments, bool referenceType,
    bool defaultvalue)
{
    if (defaultvalue && rank == 0 && sort <= codemaker::UnoType::SORT_CHAR) {
        switch (sort) {
        case codemaker::UnoType::SORT_BOOLEAN:
            o << "false";
            return;
        case codemaker::UnoType::SORT_CHAR:
        case codemaker::UnoType::SORT_BYTE:
        case codemaker::UnoType::SORT_SHORT:
        case codemaker::UnoType::SORT_UNSIGNED_SHORT:
        case codemaker::UnoType::SORT_LONG:
        case codemaker::UnoType::SORT_UNSIGNED_LONG:
        case codemaker::UnoType::SORT_HYPER:
        case codemaker::UnoType::SORT_UNSIGNED_HYPER:
        case codemaker::UnoType::SORT_FLOAT:
        case codemaker::UnoType::SORT_DOUBLE:
            o << "0";
            return;
        default:
            break;
        }
    }

    if (defaultvalue) {
        if (sort == codemaker::UnoType::SORT_INTERFACE_TYPE) {
            o << "null";
            return;
        } else if (sort == codemaker::UnoType::SORT_ANY && rank == 0) {
            o << "com.sun.star.uno.Any.VOID";
            return;
        } else if (sort == codemaker::UnoType::SORT_TYPE && rank == 0) {
            o << "com.sun.star.uno.Type.VOID";
            return;
        } else if (sort != codemaker::UnoType::SORT_ENUM_TYPE || rank != 0) {
            o << "new ";
        }
    }

    OString sType(
        codemaker::java::translateUnoToJavaType(
            sort, u2b(nucleus), referenceType && rank == 0));
    if (sType.startsWith("java.lang.")) {
        sType = sType.copy(std::strlen("java.lang."));
    }
    o << sType;
    if (!arguments.empty()) {
        o << '<';
        for (std::vector< OUString >::const_iterator i(arguments.begin());
             i != arguments.end(); ++i)
        {
            if (i != arguments.begin()) {
                o << ", ";
            }
            printType(o, options, manager, *i, true, false);
        }
        o << '>';
    }
    for (sal_Int32 i = 0; i != rank; ++i) {
        if (defaultvalue)
            o << "[0]";
        else
            o << "[]";
    }

    if (defaultvalue && sort > codemaker::UnoType::SORT_CHAR && rank == 0) {
        if (sort == codemaker::UnoType::SORT_ENUM_TYPE)
            o << ".getDefault()";
        else
            o << "()";
    }
}

void printType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, OUString const & name,
    bool referenceType, bool defaultvalue)
{
    OUString nucleus;
    sal_Int32 rank;
    std::vector< OUString > arguments;
    codemaker::UnoType::Sort sort = manager->decompose(
        name, true, &nucleus, &rank, &arguments, 0);
    printType(
        o, options, manager, sort, nucleus, rank, arguments, referenceType,
        defaultvalue);
}

bool printConstructorParameters(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort,
    rtl::Reference< unoidl::Entity > const & entity, OUString const & name,
    std::vector< OUString > const & arguments)
{
    bool previous = false;
    switch (sort) {
    case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
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
            for (std::vector< unoidl::PlainStructTypeEntity::Member >::
                     const_iterator i(ent2->getDirectMembers().begin());
                 i != ent2->getDirectMembers().end(); ++i)
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                printType(o, options, manager, i->type, false);
                o << ' '
                  << codemaker::java::translateUnoToJavaIdentifier(
                      u2b(i->name), "param");
            }
            break;
        }
    case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    entity.get()));
            assert(ent2.is());
            for (std::vector<
                     unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                     const_iterator i(ent2->getMembers().begin());
                 i != ent2->getMembers().end(); ++i)
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                if (i->parameterized) {
                    o << i->type;
                } else {
                    printType(o, options, manager, i->type, false);
                }
                o << ' '
                  << codemaker::java::translateUnoToJavaIdentifier(
                      u2b(i->name), "param");
            }
            break;
        }
    case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
        {
            rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > ent2(
                dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                    entity.get()));
            assert(ent2.is());
            for (std::vector<
                     unoidl::PolymorphicStructTypeTemplateEntity::Member >::
                     const_iterator i(ent2->getMembers().begin());
                 i != ent2->getMembers().end(); ++i)
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                if (i->parameterized) {
                    for (std::vector< OUString >::const_iterator j(
                             ent2->getTypeParameters().begin());
                         j != ent2->getTypeParameters().end(); ++j)
                    {
                        if (i->type == *j) {
                            o << arguments[
                                j - ent2->getTypeParameters().begin()];
                            break;
                        }
                    }
                } else {
                    printType(o, options, manager, i->type, false);
                }
                o << ' '
                  << codemaker::java::translateUnoToJavaIdentifier(
                      u2b(i->name), "param");
            }
            break;
        }
    case codemaker::UnoType::SORT_EXCEPTION_TYPE:
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
            for (std::vector< unoidl::ExceptionTypeEntity::Member >::
                     const_iterator i(ent2->getDirectMembers().begin());
                 i != ent2->getDirectMembers().end(); ++i)
            {
                if (previous) {
                    o << ", ";
                }
                previous = true;
                printType(o, options, manager, i->type, false);
                o << ' '
                  << codemaker::java::translateUnoToJavaIdentifier(
                      u2b(i->name), "param");
            }
            break;
        }
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::cpp::printConstructorParameters");
    }
    return previous;
}

void printConstructor(
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

void printMethodParameters(
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
            printType(o, options, manager, i->type, false);
            if (i->direction
                != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN)
            {
                o << "[]";
            }
            o << ' ';
        }
        o << codemaker::java::translateUnoToJavaIdentifier(
            u2b(i->name), "param");
    }
}

void printExceptionSpecification(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    std::vector< OUString > const & exceptions)
{
    if (!exceptions.empty()) {
        o << " throws ";
        for (std::vector< OUString >::const_iterator i(exceptions.begin());
             i != exceptions.end(); ++i)
        {
            if (i !=exceptions.begin() ) {
                o << ", ";
            }
            printType(o, options, manager, *i, false);
        }
    }
}


void printSetPropertyMixinBody(
    std::ostream & o, unoidl::InterfaceTypeEntity::Attribute const & attribute,
    OString const & indentation)
{
    unoidl::AccumulationBasedServiceEntity::Property::Attributes propFlags
        = checkAdditionalPropertyFlags(attribute);

    o << "\n" << indentation << "{\n";

    if ( attribute.bound ) {
        o << indentation << "    PropertySetMixin.BoundListeners l = "
            "new PropertySetMixin.BoundListeners();\n\n";
    }

    o << indentation << "    m_prophlp.prepareSet(\""
      << attribute.name << "\", ";
    if ( propFlags & unoidl::AccumulationBasedServiceEntity::Property::ATTRIBUTE_CONSTRAINED ) {
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
            OStringBuffer buffer(16);
            buffer.append("((");
            buffer.append(s.copy(s.lastIndexOf('/')+1));
            buffer.append(')');
            OString t = buffer.makeStringAndClear();

            if ( t.equals("((Optional)") ) {
                optional=true;
                if (single) {
                    single=false;
                    buffer1.append("the_value.IsPresent");
                    buffer2.append("the_value.Value");
                } else {
                    buffer1.insert(0, t);
                    buffer1.append(").IsPresent");
                    buffer2.insert(0, t);
                    buffer2.append(").Value");
                }
            } else {
                if ( single ) {
                    single=false;
                    if ( !optional ) {
                        buffer1.append("the_value.Value");
                    }
                    buffer2.append("the_value.Value");
                } else {
                    if ( !optional ) {
                        buffer1.insert(0, t);
                        buffer1.append(").Value");
                    }
                    buffer2.insert(0, t);
                    buffer2.append(").Value");
                }
            }
        } while( nPos <= index );

        o << "Any.VOID,\n" << indentation << "        ";
        if ( optional )
            o << "(";
        o << buffer1.makeStringAndClear();
        if ( optional )
            o << ") ? " << buffer2.makeStringAndClear() << " : Any.VOID,\n"
              << indentation << "        ";
        else
            o << ", ";
    }

    if ( attribute.bound )
        o << "l";
    else
        o << "null";
    o << ");\n";

    o << indentation << "    synchronized (this) {\n"
      << indentation << "        m_" << attribute.name
      << " = the_value;\n" << indentation << "    }\n";

    if ( attribute.bound ) {
        o << indentation << "    l.notifyListeners();\n";
    }
    o  << indentation << "}\n\n";
}

void generateXPropertySetBodies(std::ostream& o);
void generateXFastPropertySetBodies(std::ostream& o);
void generateXPropertyAccessBodies(std::ostream& o);

void printMethods(std::ostream & o,
    ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
    OUString const & name,
    codemaker::GeneratedTypeSet & generated,
    OString const & delegate, OString const & indentation,
    bool defaultvalue, bool usepropertymixin)
{
    if ( generated.contains(u2b(name)) || name == "com.sun.star.uno.XInterface" ||
         ( defaultvalue &&
           ( name == "com.sun.star.lang.XComponent" ||
             name == "com.sun.star.lang.XTypeProvider" ||
             name == "com.sun.star.uno.XWeak" ) ) ) {
        return;
    }

    if ( usepropertymixin ) {
        if (name == "com.sun.star.beans.XPropertySet") {
            generated.add(u2b(name));
            generateXPropertySetBodies(o);
            return;
        } else if (name == "com.sun.star.beans.XFastPropertySet") {
            generated.add(u2b(name));
            generateXFastPropertySetBodies(o);
            return;
        } else if (name == "com.sun.star.beans.XPropertyAccess") {
            generated.add(u2b(name));
            generateXPropertyAccessBodies(o);
            return;
        }
    }

    static OString sd("_");
    bool body = !delegate.isEmpty();
    bool defaultbody = ((delegate.equals(sd)) ? true : false);

    generated.add(u2b(name));
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent) != codemaker::UnoType::SORT_INTERFACE_TYPE)
    {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::java::printMethods");
    }
    rtl::Reference< unoidl::InterfaceTypeEntity > ent2(
        dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()));
    assert(ent2.is());
    if ( options.all || defaultvalue ) {
        for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
                 ent2->getDirectMandatoryBases().begin());
             i != ent2->getDirectMandatoryBases().end(); ++i)
        {
            printMethods(
                o, options, manager, i->name, generated, delegate, indentation,
                defaultvalue, usepropertymixin);
        }
        if (!(ent2->getDirectAttributes().empty()
              && ent2->getDirectMethods().empty()))
        {
            o << indentation << "// ";
            printType(o, options, manager, name, false);
            o << ":\n";
        }
    }
    for (std::vector< unoidl::InterfaceTypeEntity::Attribute >::const_iterator
             i(ent2->getDirectAttributes().begin());
         i != ent2->getDirectAttributes().end(); ++i)
    {
        o << indentation << "public ";
        printType(o, options, manager, i->type, false);
        o << " get" << i->name << "()";
        printExceptionSpecification(o, options, manager, i->getExceptions);
        if ( body ) {
            if ( defaultbody ) {
                if ( usepropertymixin ) {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    return m_" << i->name << ";\n" << indentation
                      << "}\n\n";
                } else {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    return ";
                    printType(o, options, manager, i->type, false, true);
                    o << ";\n" << indentation << "}\n\n";
                }
            } else {
                o << "\n" << indentation << "{\n" << indentation
                  << "    return " << delegate.getStr() << "get" << i->name
                  << "();\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }

        // REMOVE next line
        if (!i->readOnly) {
            o << indentation << "public void set" << i->name << '(';
            printType(o, options, manager, i->type, false);
            o << " the_value)";
            printExceptionSpecification(o, options, manager, i->setExceptions);
            if ( body ) {
                if ( defaultbody ) {
                    if ( usepropertymixin ) {
                        printSetPropertyMixinBody(o, *i, indentation);
                    } else {
                        o << "\n" << indentation << "{\n\n" << indentation
                          << "}\n\n";
                    }
                } else {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    " << delegate.getStr() << "set" << i->name
                      << "(the_value);\n" << indentation << "}\n\n";
                }
            } else {
                o << ";\n";
            }
        }
    }
    for (std::vector< unoidl::InterfaceTypeEntity::Method >::const_iterator i(
             ent2->getDirectMethods().begin());
         i != ent2->getDirectMethods().end(); ++i)
    {
        o << indentation << "public ";
        printType(o, options, manager, i->returnType, false);
        o << ' ' << i->name << '(';
        printMethodParameters(o, options, manager, i->parameters, true);
        o << ')';
        printExceptionSpecification(o, options, manager, i->exceptions);
        if ( body ) {
            if ( defaultbody ) {
                o << "\n" << indentation << "{\n";
                if (i->returnType != "void") {
                    o << indentation << "    // TODO: Exchange the default return "
                      << "implementation for \"" << i->name << "\" !!!\n";
                    o << indentation << "    // NOTE: "
                        "Default initialized polymorphic structs can cause problems"
                        "\n" << indentation << "    // because of missing default "
                        "initialization of primitive types of\n" << indentation
                      << "    // some C++ compilers or different Any initialization"
                        " in Java and C++\n" << indentation
                      << "    // polymorphic structs.\n" << indentation
                      << "    return ";
                    printType(o, options, manager, i->returnType, false, true);
                    o << ";";
                } else {
                    o << indentation << "    // TODO: Insert your implementation for \""
                      << i->name << "\" here.";
                }
                o << "\n" << indentation << "}\n\n";
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    ";
                if (i->returnType != "void") {
                    o << "return ";
                }
                o << delegate.getStr() << i->name << '(';
                printMethodParameters(
                    o, options, manager, i->parameters, false);
                o << ");\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }
    }
}

void printConstructors(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager, OUString const & name)
{
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(name, &ent)
        != codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE)
    {
        throw CannotDumpException(
            "unexpected entity \"" + name
            + "\" in call to skeletonmaker::java::printConstructors");
    }
    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > ent2(
        dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(ent.get()));
    assert(ent2.is());
    for (std::vector< unoidl::SingleInterfaceBasedServiceEntity::Constructor >::
             const_iterator i(ent2->getConstructors().begin());
         i != ent2->getConstructors().end(); ++i)
    {
        o << "public static ";
        printType(o, options, manager, ent2->getBase(), false);
        o << ' ';
        if (i->defaultConstructor) {
            o << "create";
        } else {
            o << codemaker::java::translateUnoToJavaIdentifier(
                u2b(i->name), "method");
        }
        o << "(com.sun.star.uno.XComponentContext the_context";
        for (std::vector<
                 unoidl::SingleInterfaceBasedServiceEntity::Constructor::
                 Parameter >::const_iterator j(i->parameters.begin());
             j != i->parameters.end(); ++i)
        {
            o << ", ";
            printType(o, options, manager, j->type, false);
            if (j->rest) {
                o << "...";
            }
            o << ' '
              << codemaker::java::translateUnoToJavaIdentifier(
                  u2b(j->name), "param");
        }
        o << ')';
        printExceptionSpecification(o, options, manager, i->exceptions);
        o << ";\n";
    }
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, rtl::Reference< TypeManager > const & manager,
    OString const & type);

void printServiceMembers(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    OUString const & name,
    rtl::Reference< unoidl::AccumulationBasedServiceEntity > const & entity,
    OString const & delegate)
{
    assert(entity.is());
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             entity->getDirectMandatoryBaseServices().begin());
         i != entity->getDirectMandatoryBaseServices().end(); ++i)
    {
        o << "\n// exported service " << i->name << "\n";
        generateDocumentation(o, options, manager, u2b(i->name), delegate);
    }
    for (std::vector< unoidl::AnnotatedReference >::const_iterator i(
             entity->getDirectMandatoryBaseInterfaces().begin());
         i != entity->getDirectMandatoryBaseInterfaces().end(); ++i)
    {
        o << "\n// supported interface " << i->name << "\n";
        generateDocumentation(o, options, manager, u2b(i->name), delegate);
    }
    o << "\n// properties of service \""<< name << "\"\n";
    for (std::vector< unoidl::AccumulationBasedServiceEntity::Property >::
             const_iterator i(entity->getDirectProperties().begin());
         i != entity->getDirectProperties().end(); ++i)
    {
        o << "// private ";
        printType(o, options, manager, i->type, false);
        o << " "
          << codemaker::java::translateUnoToJavaIdentifier(
              u2b(i->name), "property")
          << ";\n";
    }
}

void printMapsToJavaType(
    std::ostream & o, ProgramOptions const & options,
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort, OUString const & nucleus, sal_Int32 rank,
    std::vector< OUString > const & arguments, const char * javaTypeSort)
{
    o << "maps to Java " << "1.5" << " ";
    if (javaTypeSort != 0) {
        o << javaTypeSort << ' ';
    }
    o << "type \"";
    if (rank == 0 && nucleus == "com.sun.star.uno.XInterface") {
        o << "com.sun.star.uno.XInterface";
    } else {
        printType(
            o, options, manager, sort, nucleus, rank, arguments, false, false);
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
        b2u(type), false, &nucleus, &rank, 0, 0);

    bool comment = true;
    if (!delegate.isEmpty()) {
        if (sort != codemaker::UnoType::SORT_INTERFACE_TYPE &&
            sort != codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE &&
            sort != codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE )
        {
            return;
        }
        comment = false;
    }

    if (comment) {
        o << "\n// UNO";
        if (rank != 0) {
            o << " sequence type";
        } else if (sort <= codemaker::UnoType::SORT_ANY) {
            o << " simple type";
        } else {
            switch (sort) {
            case codemaker::UnoType::SORT_INTERFACE_TYPE:
                o << " interface type";
                break;

            case codemaker::UnoType::SORT_MODULE:
                o << "IDL module";
                break;

            case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
                o << " simple struct type";
                break;

            case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
                o << " polymorphic struct type template";
                break;

            case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
                o << " instantiated polymorphic struct type";
                break;

            case codemaker::UnoType::SORT_ENUM_TYPE:
                o << " enum type";
                break;

            case codemaker::UnoType::SORT_EXCEPTION_TYPE:
                o << " exception type";
                break;

            case codemaker::UnoType::SORT_TYPEDEF:
                o << "IDL typedef";
                break;

            case codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE:
                o << " single-inheritance--based service";
                break;

            case codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE:
                o << "IDL accumulation-based service";
                break;

            case codemaker::UnoType::SORT_INTERFACE_BASED_SINGLETON:
                o << " inheritance-based singleton";
                break;

            case codemaker::UnoType::SORT_SERVICE_BASED_SINGLETON:
                o << "IDL service-based singleton";
                break;

            case codemaker::UnoType::SORT_CONSTANT_GROUP:
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
        printMapsToJavaType(
            o, options, manager, sort, nucleus, rank, arguments, "array");
        o << '\n';
    } else if (sort <= codemaker::UnoType::SORT_ANY) {
        printMapsToJavaType(
            o, options, manager, sort, nucleus, rank, arguments, 0);
        o << '\n';
    } else {
        switch (sort) {
        case codemaker::UnoType::SORT_INTERFACE_TYPE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments,
                "interface");
            if (nucleus == "com.sun.star.uno.XInterface") {
                o << '\n';
            } else {
                o << "; " << (options.all ? "all" : "direct") << " methods:\n";
                codemaker::GeneratedTypeSet generated;
                printMethods(
                    o, options, manager, nucleus, generated, delegate, "");
            }
            break;

        case codemaker::UnoType::SORT_MODULE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments, "package");
            o << '\n';
            break;

        case codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments, "class");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager, codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments,
                "generic class");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager,
                codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments,
                "generic class instantiation");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager,
                codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::SORT_ENUM_TYPE:
        case codemaker::UnoType::SORT_CONSTANT_GROUP:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments, "class");
            o << '\n';
            break;

        case codemaker::UnoType::SORT_EXCEPTION_TYPE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments,
                "exception class");
            o << "; full constructor:\n";
            printConstructor(
                o, options, manager, codemaker::UnoType::SORT_EXCEPTION_TYPE,
                entity, nucleus, arguments);
            break;

        case codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments, "class");
            o << "; construction methods:\n";
            printConstructors(o, options, manager, nucleus);
            generateDocumentation(
                o, options, manager,
                u2b(dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                        entity.get())->getBase()),
                delegate);
            break;

        case codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE:
            o << ("does not map to Java\n"
                  "// the service members are generated instead\n");
            printServiceMembers(
                o, options, manager, nucleus,
                dynamic_cast< unoidl::AccumulationBasedServiceEntity * >(
                    entity.get()),
                delegate);
            break;

        case codemaker::UnoType::SORT_INTERFACE_BASED_SINGLETON:
            printMapsToJavaType(
                o, options, manager, sort, nucleus, rank, arguments, "class");
            o << "; get method:\npublic static ";
            printType(
                o, options, manager,
                dynamic_cast< unoidl::InterfaceBasedSingletonEntity * >(
                    entity.get())->getBase(),
                false);
            o << " get(com.sun.star.uno.XComponentContext context);\n";
            break;

        case codemaker::UnoType::SORT_SERVICE_BASED_SINGLETON:
            o << "does not map to Java\n";
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
}

} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
