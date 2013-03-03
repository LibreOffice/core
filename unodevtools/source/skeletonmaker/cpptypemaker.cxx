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

#include "codemaker/commoncpp.hxx"

#include "skeletoncommon.hxx"
#include "skeletoncpp.hxx"

using namespace ::rtl;
using namespace ::codemaker::cpp;

namespace skeletonmaker { namespace cpp {

void printType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    OString const & name, sal_Int32 rank,
    std::vector< OString > const & arguments, short referenceType,
    bool defaultvalue)
{
    if (defaultvalue && rank == 0 && sort <= codemaker::UnoType::SORT_CHAR) {
        switch (sort)
        {
        case codemaker::UnoType::SORT_BOOLEAN:
            o << "sal_False";
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
        case codemaker::UnoType::SORT_VOID:
        case codemaker::UnoType::SORT_STRING:
        case codemaker::UnoType::SORT_TYPE:
        case codemaker::UnoType::SORT_ANY:
        case codemaker::UnoType::SORT_COMPLEX:
            break;
        }
    }

    if (defaultvalue && referenceType == 16) {
        if (typeClass == RT_TYPE_ENUM) {
            typereg::Reader reader(manager.getTypeReader(name));
            o << name.copy(name.lastIndexOf('/'))
              << "_"
              << codemaker::convertString(reader.getFieldName(0));
        }
        return;
    }
    bool bReference = false;
    if (((sort > codemaker::UnoType::SORT_CHAR ||
          rank > 0) && referenceType != 8 &&
         !(typeClass == RT_TYPE_ENUM && referenceType == 4 && rank == 0)) ||
        (sort <= codemaker::UnoType::SORT_CHAR && referenceType == 2))
    {
        bReference = true;
    }

    if (bReference && referenceType == 4)
        o << "const ";

    for (sal_Int32 i = 0; i < rank; ++i) {
        o << ((options.shortnames) ? "css::uno::Sequence< " :
              "::com::sun::star::uno::Sequence< ");
    }
    if (typeClass == RT_TYPE_INTERFACE && referenceType > 0) {
        o << ((options.shortnames) ? "css::uno::Reference< " :
              "::com::sun::star::uno::Reference< ");
    }

    o << scopedCppName(codemaker::cpp::translateUnoToCppType(
                           sort, typeClass, name, false),
                       options.shortnames && referenceType > 0);

    if (typeClass == RT_TYPE_INTERFACE && referenceType > 0)
        o << " >";

    if (!arguments.empty()) {
        o << "< ";
        for (std::vector< OString >::const_iterator i(arguments.begin());
             i != arguments.end(); ++i)
        {
            if (i != arguments.begin())
                o << ", ";

            printType(o, options, manager, *i, 1, false);
        }
        o << " >";
    }

    for (sal_Int32 i = 0; i < rank; ++i)
        o << " >";

    if (bReference && referenceType > 1)
        o << " &";

    if (referenceType == 8 && (sort > codemaker::UnoType::SORT_CHAR || rank > 0))
        o << "()";
}

void printType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    OString const & type, short referenceType, bool defaultvalue)
{
    RTTypeClass typeClass;
    OString name;
    sal_Int32 rank;
    std::vector< OString > arguments;
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, true, true, true, &typeClass, &name, &rank, &arguments);
    printType(o,
        options, manager, sort, typeClass, name, rank, arguments,
        referenceType, defaultvalue);
}

bool printConstructorParameters(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, typereg::Reader const & outerReader,
    std::vector< OString > const & arguments)
{
    bool previous = false;
    if (reader.getSuperTypeCount() != 0) {
        OString super(
            codemaker::convertString(reader.getSuperTypeName(0)));
        typereg::Reader superReader(manager.getTypeReader(super));
        if (!superReader.isValid())
            throw CannotDumpException("Bad type library entity " + super);

        previous = printConstructorParameters(o,
            options, manager, superReader, outerReader, arguments);
    }
    for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
        if (previous)
            o << ", ";
        else
            previous = true;

        if ((reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) == 0) {
            printType(o, options, manager,
                      codemaker::convertString(reader.getFieldTypeName(i)), 4);
        } else if (arguments.empty()) {
            // ToDo !
            // o << "com::sun::star::uno::Any";
        } else {
            sal_uInt16 tparam = 0;
            while (outerReader.getReferenceTypeName(tparam)
                   != reader.getFieldTypeName(i))
            {
                ++tparam;
                OSL_ASSERT(tparam < outerReader.getReferenceCount());
            }
            // assume std::vector< OString >::size_type is at least as
            // large as sal_uInt16:
            printType(o, options, manager, arguments[tparam], 4);
        }
        o << ' '
          << (codemaker::cpp::translateUnoToCppIdentifier(
                  codemaker::convertString(reader.getFieldName(i)),
                  "param").
              getStr());
    }
    return previous;
}

void printConstructor(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader,
    std::vector< OString > const & arguments)
{
    OString type(codemaker::convertString(reader.getTypeName()));
    o << "public ";
    o << type.copy(type.lastIndexOf('/') + 1) << '(';
    printConstructorParameters(o, options, manager, reader, reader,
                               arguments);
    o << ");\n";
}

void printMethodParameters(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, sal_uInt16 method, bool previous,
    bool withtype)
{
    short referenceType = 4;
    for (sal_uInt16 i = 0; i < reader.getMethodParameterCount(method); ++i) {
        if (previous)
            o << ", ";

        previous = true;

        if (reader.getMethodParameterFlags(method, i) == RT_PARAM_OUT
            || reader.getMethodParameterFlags(method, i) == RT_PARAM_INOUT)
        {
            referenceType = 2;
        } else {
            referenceType = 4;
        }

        if (withtype) {
            printType(o, options, manager,
                codemaker::convertString(
                    reader.getMethodParameterTypeName(method, i)),
                         referenceType);
            o << ' ';
        }

        o << (codemaker::cpp::translateUnoToCppIdentifier(
                  codemaker::convertString(
                      reader.getMethodParameterName(method, i)),
                  "param").
              getStr());
    }
}

void printExceptionSpecification(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, sal_uInt16 method)
{
    o << ((options.shortnames) ? " throw (css::uno::RuntimeException" :
          " throw (::com::sun::star::uno::RuntimeException");
    if (reader.getMethodExceptionCount(method) > 0) {
        for (sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method); ++i) {
            o << ", ";
            printType(o, options, manager,
                codemaker::convertString(
                    reader.getMethodExceptionTypeName(method, i)), 1);
        }
    }
    o << ")";
}

void printSetPropertyMixinBody(std::ostream & o,
                               typereg::Reader const & reader,
                               sal_uInt16 field,
                               sal_uInt16 method)
{
    RTFieldAccess propFlags = checkAdditionalPropertyFlags(reader, field, method);
    OString fieldname = codemaker::convertString(reader.getFieldName(field));
    bool bound = (reader.getFieldFlags(field) & RT_ACCESS_BOUND ? true : false);

    o << "\n{\n";

    if (bound)
        o << "    BoundListeners l;\n";

    if (propFlags & RT_ACCESS_CONSTRAINED) {
        OString fieldtype = codemaker::convertString(
                                reader.getFieldTypeName(field));

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

            if (t.equals("Optional")) {
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
            o << "    if(" << buffer1.makeStringAndClear() << ")\n    {\n"
              << "        v <<= " << buffer2.makeStringAndClear() << ";\n    }\n";
        } else {
            o << "    v <<= " << buffer2.makeStringAndClear() << ";\n\n";
        }

        o << "    prepareSet(\n        rtl::OUString(\""
          << fieldname << "\"),\n        css::uno::Any(), v, ";
    } else {
        o << "    prepareSet(\n        rtl::OUString(\""
          << fieldname << "\"),\n        css::uno::Any(), css::uno::Any(), ";
    }

    if (bound)
        o << "&l);\n";
    else
        o << "0);\n";

    o << "    {\n        osl::MutexGuard g(m_aMutex);\n        m_"
      << fieldname << " = the_value;\n    }\n";

    if (bound)
        o << "    l.notify();\n";

    o  << "}\n\n";
}

void generateXPropertySetBodies(std::ostream& o,
                                const OString & classname,
                                const OString & interfaceName);
void generateXFastPropertySetBodies(std::ostream& o,
                                    const OString & classname,
                                    const OString & interfaceName);
void generateXPropertyAccessBodies(std::ostream& o,
                                   const OString & classname,
                                   const OString & interfaceName);

void generateXAddInBodies(std::ostream& o, const OString & classname);

void generateXLocalizable(std::ostream& o, const OString & classname);

void generateXCompatibilityNamesBodies(std::ostream& o, const OString & classname);

void generateXInitialization(std::ostream& o, const OString & classname);

void generateXDispatch(std::ostream& o,
                       const OString & classname,
                       const ProtocolCmdMap & protocolCmdMap);

void generateXDispatchProvider(std::ostream& o,
                               const OString & classname,
                               const ProtocolCmdMap & protocolCmdMap);


void printMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, codemaker::GeneratedTypeSet & generated,
    OString const & delegate, OString const & classname,
    OString const & indentation, bool defaultvalue,
    OString const & propertyhelper)
{
    OString type(codemaker::convertString(reader.getTypeName()));
    if (generated.contains(type) || type.equals("com/sun/star/uno/XInterface") ||
        (defaultvalue &&
         ( type.equals("com/sun/star/lang/XComponent") ||
           type.equals("com/sun/star/lang/XTypeProvider") ||
           type.equals("com/sun/star/uno/XWeak")) ) )
    {
        return;
    }

    static OString sd(RTL_CONSTASCII_STRINGPARAM("_"));
    bool body = !delegate.isEmpty();
    bool defaultbody = ((delegate.equals(sd)) ? true : false);

    if (body && propertyhelper.getLength() > 1) {
        if ( type.equals("com/sun/star/beans/XPropertySet")) {
            generated.add(type);
            generateXPropertySetBodies(
                o, classname, scopedCppName(propertyhelper));
            return;
        } else if ( type.equals("com/sun/star/beans/XFastPropertySet")) {
            generated.add(type);
            generateXFastPropertySetBodies(
                o, classname, scopedCppName(propertyhelper));
            return;
        } else if ( type.equals("com/sun/star/beans/XPropertyAccess")) {
            generated.add(type);
            generateXPropertyAccessBodies(
                o, classname, scopedCppName(propertyhelper));
            return;
        }
    }

    if (body && options.componenttype == 2) {
        if (type.equals("com/sun/star/lang/XServiceName")) {
            o << "// ::com::sun::star::lang::XServiceName:\n"
                "::rtl::OUString SAL_CALL " << classname << "getServiceName() "
                "throw (css::uno::RuntimeException)\n{\n    "
                "return ::rtl::OUString("
                "sADDIN_SERVICENAME);\n}\n";
            generated.add(type);
            return;
        } else if (type.equals("com/sun/star/sheet/XAddIn")) {
            generateXAddInBodies(o, classname);
            generated.add(type);

            // special handling of XLocalizable -> parent of XAddIn
            if (!generated.contains("com/sun/star/lang/XLocalizable")) {
                generateXLocalizable(o, classname);
                generated.add("com/sun/star/lang/XLocalizable");
            }
            return;
        } else if (type.equals("com/sun/star/lang/XLocalizable")) {
            generateXLocalizable(o, classname);
            generated.add(type);
            return;
        } else if (type.equals("com/sun/star/sheet/XCompatibilityNames")) {
            generateXCompatibilityNamesBodies(o, classname);
            generated.add(type);
            return;
        }
    }

    if (body && options.componenttype == 3) {
        if (type.equals("com/sun/star/lang/XInitialization")) {
            generateXInitialization(o, classname);
            generated.add(type);
            return;
        } else if (type.equals("com/sun/star/frame/XDispatch")) {
            generateXDispatch(o, classname, options.protocolCmdMap);
            generated.add(type);
            return;
        } else if (type.equals("com/sun/star/frame/XDispatchProvider")) {
            generateXDispatchProvider(o, classname, options.protocolCmdMap);
            generated.add(type);
            return;
        }
    }

    generated.add(type);
    if (options.all || defaultvalue) {
        for (sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i) {
            typereg::Reader super(
                manager.getTypeReader(
                    codemaker::convertString(
                        reader.getSuperTypeName(i))));
            if (!super.isValid()) {
                throw CannotDumpException(
                    "Bad type library entity "
                    + codemaker::convertString(
                        reader.getSuperTypeName(i)));
            }

            printMethods(o, options, manager, super, generated, delegate,
                            classname, indentation, defaultvalue, propertyhelper);
        }

        if (reader.getFieldCount() > 0 || reader.getMethodCount() > 0) {
            o << indentation << "// ";
            printType(o, options, manager, type, 0);
            o << ":\n";
        }
    }
    sal_uInt16 method = 0;
    for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
        o << indentation;
        if (!body)
            o << "virtual ";

        printType(o, options, manager,
                  codemaker::convertString(reader.getFieldTypeName(i)), 1);
        o << " SAL_CALL ";
        if (!classname.isEmpty())
            o << classname;

        o << "get"
          << codemaker::convertString(reader.getFieldName(i)).getStr()
          << "()";
        if (method < reader.getMethodCount()
            && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_GET
            && reader.getMethodName(method) == reader.getFieldName(i))
        {
            printExceptionSpecification(o, options, manager, reader, method++);
        } else {
            o << ((options.shortnames) ? " throw (css::uno::RuntimeException)" :
                  " throw (::com::sun::star::uno::RuntimeException)");
        }
        if (body) {
            if (defaultbody) {
                if (!propertyhelper.isEmpty()) {
                    o << "\n{\n    osl::MutexGuard g(m_aMutex);\n    return m_"
                      << codemaker::convertString(reader.getFieldName(i)).getStr()
                      << ";\n}\n\n";
                } else {
                    o << "\n{\n    return ";
                    if (options.componenttype == 1) {
                        o << "m_"
                          << codemaker::convertString(
                              reader.getFieldName(i)).getStr();
                    } else {
                        printType(o, options, manager,
                                  codemaker::convertString(
                                      reader.getFieldTypeName(i)),
                                  8, true);
                    }
                    o << ";\n}\n\n";
                }
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    return "
                  << delegate.getStr() << "get"
                  << codemaker::convertString(reader.getFieldName(i)).getStr()
                  << "();\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }

        if ((reader.getFieldFlags(i) & RT_ACCESS_READONLY) == 0) {
            o << indentation;
            if (!body)
                o << "virtual ";

            o << "void SAL_CALL ";
            if (!classname.isEmpty())
                o << classname;

            o << "set"
              << (codemaker::convertString(reader.getFieldName(i)).getStr())
              << '(';
            printType(o, options, manager,
                      codemaker::convertString(reader.getFieldTypeName(i)), 4);
            o << " the_value)";
            if (method < reader.getMethodCount()
                && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_SET
                && reader.getMethodName(method) == reader.getFieldName(i))
            {
                printExceptionSpecification(o, options, manager, reader, method++);
            } else {
                o << ((options.shortnames) ? " throw (css::uno::RuntimeException)" :
                      " throw (::com::sun::star::uno::RuntimeException)");
            }
            if (body) {
                if (defaultbody) {
                    if (!propertyhelper.isEmpty()) {
                        printSetPropertyMixinBody(o, reader, i, method);
                    } else {
                        if (options.componenttype == 1) {
                            o << "\n{\n    m_"
                              << codemaker::convertString(
                                  reader.getFieldName(i)).getStr()
                              << " = the_value;\n}\n\n";
                        } else {
                            o << "\n{\n\n}\n\n";
                        }
                    }
                } else {
                    o << "\n" << indentation << "{\n" << indentation << "    "
                      << delegate.getStr() << "set"
                      << codemaker::convertString(reader.getFieldName(i)).getStr()
                      << "(the_value);\n" << indentation << "}\n\n";
                }
            } else {
                o << ";\n";
            }
        }
    }
    for (; method < reader.getMethodCount(); ++method) {
        o << indentation;
        if (!body)
            o << "virtual ";

        printType(o, options, manager,
                  codemaker::convertString(
                      reader.getMethodReturnTypeName(method)), 1);
        o << " SAL_CALL ";
        if (!classname.isEmpty())
            o << classname;

        const OString methodName(codemaker::convertString(reader.getMethodName(method)));

        o << methodName << '(';
        printMethodParameters(o, options, manager, reader, method, false, true);
        o << ')';
        printExceptionSpecification(o, options, manager, reader, method);
        if (body) {
            static OUString s("void");
            if (defaultbody) {
                o << "\n{\n";
                if (!reader.getMethodReturnTypeName(method).equals(s)) {
                    o << "    // TODO: Exchange the default return implementation for \""
                      << methodName << "\" !!!\n";
                    o << "    // Exchange the default return implementation.\n"
                        "    // NOTE: Default initialized polymorphic structs "
                        "can cause problems because of\n    // missing default "
                        "initialization of primitive types of some C++ compilers or"
                        "\n    // different Any initialization in Java and C++ "
                        "polymorphic structs.\n    return ";
                    printType(o, options, manager,
                        codemaker::convertString(
                            reader.getMethodReturnTypeName(method)), 8, true);
                    o << ";";
                } else {
                    o << "    // TODO: Insert your implementation for \""
                      << methodName << "\" here.";
                }
                o << "\n}\n\n";
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    ";
                if (!reader.getMethodReturnTypeName(method).equals(s))
                    o << "return ";

                o << delegate.getStr()
                  << (codemaker::convertString(
                          reader.getMethodName(method)).getStr())
                  << '(';
                printMethodParameters(o, options, manager, reader, method,
                                      false, false);
                o << ");\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }
    }

    if (method > 0 && !body)
        o << "\n";
}

void printConstructionMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader)
{
    for (sal_uInt16 i = 0; i < reader.getMethodCount(); ++i) {
        o << "static ";
        printType(o,
            options, manager,
            codemaker::convertString(reader.getSuperTypeName(0)), 1);
        o << ' ';
        if (reader.getMethodName(i).isEmpty()) {
            o << "create";
        } else {
            o << (codemaker::cpp::translateUnoToCppIdentifier(
                      codemaker::convertString(reader.getMethodName(i)),
                      "method").
                  getStr());
        }
        o << ((options.shortnames) ? "(css::uno::Reference< css" :
              "(::com::sun::star::uno::Reference< ::com::sun::star")
          << "::uno::XComponentContext > const & the_context";
        printMethodParameters(o, options, manager, reader, i,
                              true, true);
        o << ')';
        printExceptionSpecification(o, options, manager, reader, i);
        o << ";\n";
    }
}

void printServiceMembers(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, OString const & type,
    OString const & delegate)
{
    for (sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i) {
        OString referenceType(
            codemaker::convertString(
                reader.getReferenceTypeName(i)).replace('/', '.'));

        if ( reader.getReferenceSort(i) == RT_REF_SUPPORTS ) {
            o << "\n// supported interface " << referenceType.getStr() << "\n";
            generateDocumentation(o, options, manager, referenceType, delegate);
        } else if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
            o << "\n// exported service " << referenceType.getStr() << "\n";
            generateDocumentation(o, options, manager, referenceType, delegate);
            o << "\n// end of exported service " << referenceType.getStr() << "\n";
        }
    }

    if (delegate.isEmpty()) {
        o << "\n// properties of service \""<< type.getStr() << "\"\n";
        for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
            OString fieldName(
                codemaker::convertString(reader.getFieldName(i)));
            OString fieldType(
                codemaker::convertString(reader.getFieldTypeName(i)));

            o << "// private ";
            printType(o, options, manager, fieldType, 1);
            o << " "
              << codemaker::cpp::translateUnoToCppIdentifier(
                  fieldName, "property").getStr()
              << ";\n";
        }
    }
}

void printMapsToCppType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    OString const & name, sal_Int32 rank,
    std::vector< OString > const & arguments, const char * cppTypeSort)
{
    o << "maps to C++ ";
    if (cppTypeSort != 0)
        o << cppTypeSort << ' ';

    o << "type \"";
    if (rank == 0 && name == "com/sun/star/uno/XInterface") {
        o << "Reference< com::sun::star::uno::XInterface >";
    } else {
        printType(o, options, manager, sort, typeClass, name, rank, arguments, 0);
    }
    o << '"';
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    OString const & type, OString const & delegate)
{
    if (type.indexOf('/') >= 0)
        throw CannotDumpException("Illegal type name " + type);

    OString binType(type.replace('.', '/'));
    RTTypeClass typeClass;
    OString name;
    sal_Int32 rank;
    std::vector< OString > arguments;
    codemaker::UnoType::Sort sort = decomposeResolveAndCheck(
        manager, binType, false, true, true, &typeClass, &name, &rank,
        &arguments);

    bool comment=true;
    if (!delegate.isEmpty()) {
        if (typeClass != RT_TYPE_INTERFACE &&
            typeClass != RT_TYPE_SERVICE )
        {
            return;
        }
        comment=false;
    }

    if (comment) {
        o << "\n// UNO";
        if (rank > 0) {
            o << " sequence type";
        } else if (sort != codemaker::UnoType::SORT_COMPLEX) {
            o << " simple type";
        } else {
            typereg::Reader reader(manager.getTypeReader(name));
            if (!reader.isValid())
                throw CannotDumpException("Bad type library entity " + name);

            switch (typeClass)
            {
            case RT_TYPE_INTERFACE:
                o << " interface type";
                break;

            case RT_TYPE_MODULE:
                o << "IDL module";
                break;

            case RT_TYPE_STRUCT:
                if (reader.getReferenceCount() == 0)
                    o << " simple struct type";
                else if (arguments.empty())
                    o << " polymorphic struct type template";
                else
                    o << " instantiated polymorphic struct type";
                break;

            case RT_TYPE_ENUM:
                o << " enum type";
                break;

            case RT_TYPE_EXCEPTION:
                o << " exception type";
                break;

            case RT_TYPE_TYPEDEF:
                o << "IDL typedef";
                break;

            case RT_TYPE_SERVICE:
                if (reader.getSuperTypeCount() > 0)
                    o << " single-inheritance--based service";
                else
                    o << "IDL accumulation-based service";
                break;

            case RT_TYPE_SINGLETON:
                if ((manager.getTypeReader(
                         codemaker::convertString(
                             reader.getSuperTypeName(0))).getTypeClass())
                    == RT_TYPE_INTERFACE)
                    o << " inheritance-based singleton";
                else
                    o << "IDL service-based singleton";
                break;

            case RT_TYPE_CONSTANTS:
                o << "IDL constant group";
                break;

            default:
                OSL_ASSERT(false);
                break;
            }
        }
        o << " \"" << type.getStr() << "\" ";
    }
    sort = codemaker::decomposeAndResolve(
        manager, binType, true, true, true, &typeClass, &name, &rank,
        &arguments);
    if (rank > 0) {
        if (comment) {
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments, "array");
            o << '\n';
        }
    } else if (sort != codemaker::UnoType::SORT_COMPLEX) {
        if (comment) {
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments, 0);
            o << '\n';
        }
    } else {
        typereg::Reader reader(manager.getTypeReader(name));
        if (!reader.isValid())
            throw CannotDumpException("Bad type library entity " + name);

        switch (typeClass)
        {
        case RT_TYPE_INTERFACE:
            if (comment)
                printMapsToCppType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    "interface");
            if (name == "com/sun/star/uno/XInterface") {
                if (comment)
                    o << '\n';
            } else {
                if (comment)
                    o << "; " << (options.all ? "all" : "direct") << " methods:\n";

                codemaker::GeneratedTypeSet generated;
                printMethods(o, options, manager, reader, generated,
                             delegate, options.implname, "");
            }
            break;

        case RT_TYPE_MODULE:
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "namespace");
            o << '\n';
            break;

        case RT_TYPE_STRUCT:
            if (reader.getReferenceCount() == 0) {
                printMapsToCppType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    "class");
            } else if (arguments.empty()) {
                printMapsToCppType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    options.java5 ? "generic class" : "class");
            } else {
                printMapsToCppType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    options.java5 ? "generic class instantiation" : "class");
            }
            o << "; full constructor:\n";
            printConstructor(o, options, manager, reader, arguments);
            break;

        case RT_TYPE_ENUM:
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "enum");
            o << '\n';
            break;

        case RT_TYPE_CONSTANTS:
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "namespace");
            o << '\n';
            break;

        case RT_TYPE_EXCEPTION:
            printMapsToCppType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "exception class");
            o << "; full constructor:\n";
            printConstructor(o, options, manager, reader, arguments);
            break;

        case RT_TYPE_SERVICE:
            if (reader.getSuperTypeCount() > 0) {
                if (comment) {
                    printMapsToCppType(o, options, manager, sort, typeClass,
                                       name, rank, arguments, "class");
                    o << "; construction methods:\n";
                    printConstructionMethods(o, options, manager, reader);
                }

                OString super(
                    codemaker::convertString(
                        reader.getSuperTypeName(0)).replace('/', '.'));
                generateDocumentation(o, options, manager, super, delegate);
            } else {
                if (comment)
                    o << ("does not map to C++\n"
                          "// the service members are generated instead\n");
                printServiceMembers(o, options, manager, reader, type, delegate);
            }
            break;

        case RT_TYPE_SINGLETON:
            if (reader.getSuperTypeCount() > 0 &&
                ((manager.getTypeReader(
                     codemaker::convertString(
                         reader.getSuperTypeName(0))).
                    getTypeClass()) == RT_TYPE_INTERFACE) )
            {
                printMapsToCppType(o, options, manager, sort, typeClass,
                                   name, rank, arguments,
                    "class");
                o << "; get method:\nstatic ";
                printType(o, options, manager,
                          codemaker::convertString(reader.getSuperTypeName(0)), 1);
                o << " get(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);\n";
            } else {
                o << "does not map to C++\n";
            }
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
}

} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
