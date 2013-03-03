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

#include "codemaker/commonjava.hxx"

#include "skeletoncommon.hxx"
#include "skeletonjava.hxx"

using namespace ::rtl;

namespace skeletonmaker { namespace java {

void printType(std::ostream & o,
               ProgramOptions const & options, TypeManager const & manager,
               OString const & type, bool referenceType,
               bool defaultvalue);

void printType(std::ostream & o,
               ProgramOptions const & options, TypeManager const & manager,
               codemaker::UnoType::Sort sort, RTTypeClass typeClass,
               OString const & name, sal_Int32 rank,
               std::vector< OString > const & arguments, bool referenceType,
               bool defaultvalue)
{
    if ( defaultvalue && rank == 0 && sort <= codemaker::UnoType::SORT_CHAR ) {
        switch (sort)
        {
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
        case codemaker::UnoType::SORT_VOID:
        case codemaker::UnoType::SORT_STRING:
        case codemaker::UnoType::SORT_TYPE:
        case codemaker::UnoType::SORT_ANY:
        case codemaker::UnoType::SORT_COMPLEX:
            break;
        }
    }

    if ( defaultvalue ) {
        if ( sort == codemaker::UnoType::SORT_COMPLEX &&
            typeClass == RT_TYPE_INTERFACE ) {
            o << "null";
            return;
        } else if ( sort == codemaker::UnoType::SORT_ANY && rank==0 ) {
            o << "com.sun.star.uno.Any.VOID";
            return;
        } else if ( sort == codemaker::UnoType::SORT_TYPE && rank==0 ) {
            o << "com.sun.star.uno.Type.VOID";
            return;
        } else
            if ( typeClass != RT_TYPE_ENUM || rank > 0 )
                o << "new ";
    }

    OString sType(codemaker::java::translateUnoToJavaType(
                      sort, typeClass, name, referenceType && rank==0).replace('/', '.'));
    if ( sType.indexOf("java.lang.") == 0 )
        sType = sType.copy(10);
    o << sType.getStr();
    if ( !arguments.empty() && options.java5 ) {
        o << '<';
        for ( std::vector< OString >::const_iterator i(arguments.begin());
              i != arguments.end(); ++i )
        {
            if ( i != arguments.begin() ) {
                o << ", ";
            }

            printType(o, options, manager, *i, true, false);
        }
        o << '>';
    }
    for ( sal_Int32 i = 0; i < rank; ++i ) {
        if ( defaultvalue )
            o << "[0]";
        else
            o << "[]";
    }

    if ( defaultvalue && sort > codemaker::UnoType::SORT_CHAR && rank == 0 ) {
        if ( typeClass == RT_TYPE_ENUM )
            o << ".getDefault()";
        else
            o << "()";
    }
}

void printType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    OString const & type, bool referenceType, bool defaultvalue)
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
    if ( reader.getSuperTypeCount() != 0 ) {
        OString super(
            codemaker::convertString(reader.getSuperTypeName(0)));
        typereg::Reader superReader(manager.getTypeReader(super));
        if ( !superReader.isValid() ) {
            throw CannotDumpException("Bad type library entity " + super);
        }
        previous = printConstructorParameters(o,
            options, manager, superReader, outerReader, arguments);
    }
    for ( sal_uInt16 i = 0; i < reader.getFieldCount(); ++i ) {
        if ( previous ) {
            o << ", ";
        }
        previous = true;
        if ( (reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) == 0  ) {
            printType(o,
                options, manager,
                codemaker::convertString(reader.getFieldTypeName(i)),
                false);
        } else if ( arguments.empty() ) {
            o << "java.lang.Object";
        } else {
            sal_uInt16 tparam = 0;
            while ( outerReader.getReferenceTypeName(tparam)
                    != reader.getFieldTypeName(i) )
            {
                ++tparam;
                OSL_ASSERT(tparam < outerReader.getReferenceCount());
            }
            // assume std::vector< OString >::size_type is at least as
            // large as sal_uInt16:
            printType(o, options, manager, arguments[tparam], true);
        }
        o
            << ' '
            << (codemaker::java::translateUnoToJavaIdentifier(
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
    o << "public " << type.copy(type.lastIndexOf('/') + 1) << '(';
    printConstructorParameters(o, options, manager, reader, reader, arguments);
    o << ");\n";
}

void printMethodParameters(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, sal_uInt16 method, bool previous,
    bool withtype)
{
    for ( sal_uInt16 i = 0; i < reader.getMethodParameterCount(method); ++i ) {
        if ( previous  )
            o << ", ";
        else
            previous = true;

        if ( withtype ) {
            printType(o, options, manager,
                      codemaker::convertString(
                          reader.getMethodParameterTypeName(method, i)),
                      false);
            if ( reader.getMethodParameterFlags(method, i) == RT_PARAM_OUT
                 || reader.getMethodParameterFlags(method, i) == RT_PARAM_INOUT )
            {
                o << "[]";
            } else if ( (reader.getMethodParameterFlags(method, i) & RT_PARAM_REST )
                       != 0)
            {
                o << (options.java5 ? "..." : "[]");
            }
            o << ' ';
        }
        o << (codemaker::java::translateUnoToJavaIdentifier(
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
    if ( reader.getMethodExceptionCount(method) > 0 ) {
        o << " throws ";
        for ( sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method); ++i )
        {
            if ( i != 0 ) {
                o << ", ";
            }
            printType(o,
                options, manager,
                codemaker::convertString(
                    reader.getMethodExceptionTypeName(method, i)),
                false);
        }
    }
}


void printSetPropertyMixinBody(std::ostream & o,
                               typereg::Reader const & reader,
                               sal_uInt16 field,
                               sal_uInt16 method,
                               OString const & indentation)
{
    RTFieldAccess propFlags = checkAdditionalPropertyFlags(reader, field, method);
    OString fieldname = codemaker::convertString(reader.getFieldName(field));
    bool bound = (reader.getFieldFlags(field) & RT_ACCESS_BOUND ? true : false);

    o << "\n" << indentation << "{\n";

    if ( bound ) {
        o << indentation << "    PropertySetMixin.BoundListeners l = "
            "new PropertySetMixin.BoundListeners();\n\n";
    }

    o << indentation << "    m_prophlp.prepareSet(\""
      << fieldname << "\", ";
    if ( propFlags & RT_ACCESS_CONSTRAINED ) {
        OString fieldtype = codemaker::convertString(reader.getFieldTypeName(field));

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

    if ( bound )
        o << "l";
    else
        o << "null";
    o << ");\n";

    o << indentation << "    synchronized (this) {\n"
      << indentation << "        m_" << fieldname
      << " = the_value;\n" << indentation << "    }\n";

    if ( bound ) {
        o << indentation << "    l.notifyListeners();\n";
    }
    o  << indentation << "}\n\n";
}

void generateXPropertySetBodies(std::ostream& o);
void generateXFastPropertySetBodies(std::ostream& o);
void generateXPropertyAccessBodies(std::ostream& o);

void printMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader,
    codemaker::GeneratedTypeSet & generated,
    OString const & delegate, OString const & indentation,
    bool defaultvalue, bool usepropertymixin)
{
    OString type(codemaker::convertString(reader.getTypeName()));
    if ( generated.contains(type) || type == "com/sun/star/uno/XInterface" ||
         ( defaultvalue &&
           ( type.equals("com/sun/star/lang/XComponent") ||
             type.equals("com/sun/star/lang/XTypeProvider") ||
             type.equals("com/sun/star/uno/XWeak") ) ) ) {
        return;
    }

    if ( usepropertymixin ) {
        if ( type.equals("com/sun/star/beans/XPropertySet") ) {
            generated.add(type);
            generateXPropertySetBodies(o);
            return;
        } else if ( type.equals("com/sun/star/beans/XFastPropertySet") ) {
            generated.add(type);
            generateXFastPropertySetBodies(o);
            return;
        } else if ( type.equals("com/sun/star/beans/XPropertyAccess") ) {
            generated.add(type);
            generateXPropertyAccessBodies(o);
            return;
        }
    }

    static OString sd(RTL_CONSTASCII_STRINGPARAM("_"));
    bool body = !delegate.isEmpty();
    bool defaultbody = ((delegate.equals(sd)) ? true : false);

    generated.add(type);
    if ( options.all || defaultvalue ) {
        for (sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i) {
            typereg::Reader super(
                manager.getTypeReader(
                    codemaker::convertString(
                        reader.getSuperTypeName(i))));
            if ( !super.isValid() ) {
                throw CannotDumpException(
                    "Bad type library entity "
                    + codemaker::convertString(
                        reader.getSuperTypeName(i)));
            }
            printMethods(o, options, manager, super, generated, delegate,
                         indentation, defaultvalue, usepropertymixin);
        }
        if ( reader.getFieldCount() > 0 || reader.getMethodCount() > 0 ) {
            o << indentation << "// ";
            printType(o, options, manager, type, false);
            o << ":\n";
        }
    }
    sal_uInt16 method = 0;
    for ( sal_uInt16 i = 0; i < reader.getFieldCount(); ++i ) {
        o << indentation << "public ";
        printType(o,
            options, manager,
            codemaker::convertString(reader.getFieldTypeName(i)), false);
        o << " get"
          << codemaker::convertString(reader.getFieldName(i)).getStr()
          << "()";

        if ( method < reader.getMethodCount()
             && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_GET
             && reader.getMethodName(method) == reader.getFieldName(i) )
        {
            printExceptionSpecification(o, options, manager, reader, method++);
        }
        if ( body ) {
            if ( defaultbody ) {
                if ( usepropertymixin ) {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    return m_"
                      << codemaker::convertString(reader.getFieldName(i)).getStr()
                      << ";\n" << indentation << "}\n\n";
                } else {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    return ";
                    printType(o,
                              options, manager,
                              codemaker::convertString(reader.getFieldTypeName(i)),
                              false, true);
                    o << ";\n" << indentation << "}\n\n";
                }
            } else {
                o << "\n" << indentation << "{\n" << indentation <<
                    "    return "
                  << delegate.getStr() << "get"
                  << codemaker::convertString(reader.getFieldName(i)).getStr()
                  << "();\n" << indentation << "}\n\n";
            }
        } else {
            o << ";\n";
        }

        // REMOVE next line
        if ( (reader.getFieldFlags(i) & RT_ACCESS_READONLY) == 0 ) {
            bool setAttrMethod = false;
            o << indentation << "public void set"
              << (codemaker::convertString(reader.getFieldName(i)).
                  getStr())
              << '(';
            printType(o,
                options, manager,
                codemaker::convertString(reader.getFieldTypeName(i)),
                false);
            o << " the_value)";
            if ( method < reader.getMethodCount()
                 && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_SET
                 && reader.getMethodName(method) == reader.getFieldName(i) )
            {
                setAttrMethod=true;
                printExceptionSpecification(o, options, manager, reader, method);
            }
            if ( body ) {
                if ( defaultbody ) {
                    if ( usepropertymixin ) {
                        printSetPropertyMixinBody(o, reader, i, method,
                                                  indentation);
                    } else {
                        o << "\n" << indentation << "{\n\n" << indentation
                          << "}\n\n";
                    }
                } else {
                    o << "\n" << indentation << "{\n" << indentation
                      << "    " << delegate.getStr() << "set"
                      << codemaker::convertString(reader.getFieldName(i)).getStr()
                      << "(the_value);\n" << indentation << "}\n\n";
                }
            } else {
                o << ";\n";
            }
            if (setAttrMethod) ++method;
        }
    }
    for ( ; method < reader.getMethodCount(); ++method ) {
        o << indentation << "public ";
        printType(o,
            options, manager,
            codemaker::convertString(
                reader.getMethodReturnTypeName(method)),
            false);

        const OString methodName(codemaker::convertString(reader.getMethodName(method)));

        o << ' ' << methodName.getStr() << '(';
        printMethodParameters(o, options, manager, reader, method, false, true);
        o << ')';
        printExceptionSpecification(o, options, manager, reader, method);
        if ( body ) {
            static OUString s(RTL_CONSTASCII_USTRINGPARAM("void"));
            if ( defaultbody ) {
                o << "\n" << indentation << "{\n";
                if ( !reader.getMethodReturnTypeName(method).equals(s) ) {
                    o << indentation << "    // TODO: Exchange the default return "
                      << "implementation for \"" << methodName << "\" !!!\n";
                    o << indentation << "    // NOTE: "
                        "Default initialized polymorphic structs can cause problems"
                        "\n" << indentation << "    // because of missing default "
                        "initialization of primitive types of\n" << indentation
                      << "    // some C++ compilers or different Any initialization"
                        " in Java and C++\n" << indentation
                      << "    // polymorphic structs.\n" << indentation
                      << "    return ";
                    printType(o,
                        options, manager,
                        codemaker::convertString(
                            reader.getMethodReturnTypeName(method)),
                        false, true);
                    o << ";";
                } else {
                    o << indentation << "    // TODO: Insert your implementation for \""
                      << methodName << "\" here.";
                }
                o << "\n" << indentation << "}\n\n";
            } else {
                o << "\n" << indentation << "{\n" << indentation << "    ";
                if ( !reader.getMethodReturnTypeName(method).equals(s) )
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
}

void printConstructionMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader)
{
    for ( sal_uInt16 i = 0; i < reader.getMethodCount(); ++i ) {
        o << "public static ";
        printType(o,
            options, manager,
            codemaker::convertString(reader.getSuperTypeName(0)), false);
        o << ' ';
        if ( reader.getMethodName(i).isEmpty() ) {
            o << "create";
        } else {
            o << (codemaker::java::translateUnoToJavaIdentifier(
                      codemaker::convertString(reader.getMethodName(i)),
                      "method").getStr());
        }
        o << "(com.sun.star.uno.XComponentContext the_context";
        printMethodParameters(o, options, manager, reader, i, true, true);
        o << ')';
        printExceptionSpecification(o, options, manager, reader, i);
        o << ";\n";
    }
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    OString const & type);

void printServiceMembers(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, OString const & type,
    OString const & delegate)
{
    for ( sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i ) {
        OString referenceType(
            codemaker::convertString(
                reader.getReferenceTypeName(i)).replace('/', '.'));

        if ( reader.getReferenceSort(i) == RT_REF_SUPPORTS ) {
            o << "\n// supported interface " << referenceType.getStr() << "\n";
            generateDocumentation(o, options, manager, referenceType, delegate);
        } else if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
            o << "\n// exported service " << referenceType.getStr() << "\n";
            generateDocumentation(o, options, manager, referenceType, delegate);
        }
    }

    o << "\n// properties of service \""<< type.getStr() << "\"\n";
    for ( sal_uInt16 i = 0; i < reader.getFieldCount(); ++i ) {
        OString fieldName(
            codemaker::convertString(reader.getFieldName(i)));
        OString fieldType(
            codemaker::convertString(reader.getFieldTypeName(i)));

        o << "// private ";
        printType(o, options, manager, fieldType, false);
        o << " "
          << codemaker::java::translateUnoToJavaIdentifier(
              fieldName, "property").getStr()
          << ";\n";
    }
}

void printMapsToJavaType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    OString const & name, sal_Int32 rank,
    std::vector< OString > const & arguments, const char * javaTypeSort)
{
    o << "maps to Java " << (options.java5 ? "1.5" : "1.4") << " ";
    if ( javaTypeSort != 0 ) {
        o << javaTypeSort << ' ';
    }
    o << "type \"";
    if ( rank == 0 && name == "com/sun/star/uno/XInterface" ) {
        o << "com.sun.star.uno.XInterface";
    } else {
        printType(o,
            options, manager, sort, typeClass, name, rank, arguments, false);
    }
    o << '"';
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    OString const & type, OString const & delegate)
{
    if ( type.indexOf('/') >= 0 ) {
        throw CannotDumpException("Illegal type name " + type);
    }
    OString binType(type.replace('.', '/'));
    RTTypeClass typeClass;
    OString name;
    sal_Int32 rank;
    std::vector< OString > arguments;
    codemaker::UnoType::Sort sort = decomposeResolveAndCheck(
        manager, binType, false, true, true, &typeClass, &name, &rank,
        &arguments);

    bool comment=true;
    if ( !delegate.isEmpty() ) {
        if ( typeClass != RT_TYPE_INTERFACE && typeClass != RT_TYPE_SERVICE )
            return;

        comment=false;
    }

    if ( comment ) {
        o << "\n// UNO";
        if ( rank > 0 ) {
            o << " sequence type";
        } else if ( sort != codemaker::UnoType::SORT_COMPLEX ) {
            o << " simple type";
        } else {
            typereg::Reader reader(manager.getTypeReader(name));
            if ( !reader.isValid() ) {
                throw CannotDumpException("Bad type library entity " + name);
            }
            switch ( typeClass ) {
            case RT_TYPE_INTERFACE:
                o << " interface type";
                break;

            case RT_TYPE_MODULE:
                o << "IDL module";
                break;

            case RT_TYPE_STRUCT:
                if ( reader.getReferenceCount() == 0 ) {
                    o << " simple struct type";
                } else if ( arguments.empty() ) {
                    o << " polymorphic struct type template";
                } else {
                    o << " instantiated polymorphic struct type";
                }
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
                if ( reader.getSuperTypeCount() > 0 ) {
                    o << " single-inheritance--based service";
                } else {
                    o << "IDL accumulation-based service";
                }
                break;

            case RT_TYPE_SINGLETON:
                if ( (manager.getTypeReader(
                          codemaker::convertString(
                              reader.getSuperTypeName(0))).getTypeClass())
                     == RT_TYPE_INTERFACE )
                {
                    o << " inheritance-based singleton";
                } else {
                    o << "IDL service-based singleton";
                }
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
    if ( rank > 0 ) {
        printMapsToJavaType(o,
            options, manager, sort, typeClass, name, rank, arguments, "array");
        o << '\n';
    } else if ( sort != codemaker::UnoType::SORT_COMPLEX ) {
        printMapsToJavaType(o,
            options, manager, sort, typeClass, name, rank, arguments, 0);
        o << '\n';
    } else {
        typereg::Reader reader(manager.getTypeReader(name));
        if ( !reader.isValid() ) {
            throw CannotDumpException("Bad type library entity " + name);
        }
        switch ( typeClass ) {
        case RT_TYPE_INTERFACE:
            printMapsToJavaType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "interface");
            if ( name == "com/sun/star/uno/XInterface" ) {
                o << '\n';
            } else {
                o
                    << "; " << (options.all ? "all" : "direct")
                    << " methods:\n";
                codemaker::GeneratedTypeSet generated;
                printMethods(o, options, manager, reader, generated,
                             delegate, "");
            }
            break;

        case RT_TYPE_MODULE:
            printMapsToJavaType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "package");
            o << '\n';
            break;

        case RT_TYPE_STRUCT:
            if ( reader.getReferenceCount() == 0 ) {
                printMapsToJavaType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    "class");
            } else if ( arguments.empty() ) {
                printMapsToJavaType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    options.java5 ? "generic class" : "class");
            } else {
                printMapsToJavaType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    options.java5 ? "generic class instantiation" : "class");
            }
            o << "; full constructor:\n";
            printConstructor(o, options, manager, reader, arguments);
            break;

        case RT_TYPE_ENUM:
        case RT_TYPE_CONSTANTS:
            printMapsToJavaType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "class");
            o << '\n';
            break;

        case RT_TYPE_EXCEPTION:
            printMapsToJavaType(o,
                options, manager, sort, typeClass, name, rank, arguments,
                "exception class");
            o << "; full constructor:\n";
            printConstructor(o, options, manager, reader, arguments);
            break;

        case RT_TYPE_SERVICE:
            if ( reader.getSuperTypeCount() > 0 ) {
                printMapsToJavaType(o,
                    options, manager, sort, typeClass, name, rank, arguments,
                    "class");
                o << "; construction methods:\n";
                printConstructionMethods(o, options, manager, reader);

                OString super(
                    codemaker::convertString(
                        reader.getSuperTypeName(0)).replace('/', '.'));

                generateDocumentation(o, options, manager, super, delegate);
            } else {
                o << ("does not map to Java\n"
                      "// the service members are generated instead\n");
                printServiceMembers(o, options, manager, reader, type, delegate);
            }
            break;

        case RT_TYPE_SINGLETON:
            if ( reader.getSuperTypeCount() > 0 &&
                 ((manager.getTypeReader(
                       codemaker::convertString(
                           reader.getSuperTypeName(0))).getTypeClass())
                  == RT_TYPE_INTERFACE) ) {
                printMapsToJavaType(o, options, manager, sort, typeClass,
                                    name, rank, arguments, "class");
                o << "; get method:\npublic static ";
                printType(o, options, manager,
                          codemaker::convertString(reader.getSuperTypeName(0)),
                          false);
                o
                    << " get(com.sun.star.uno.XComponentContext context);\n";
            } else {
                o << "does not map to Java\n";
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
