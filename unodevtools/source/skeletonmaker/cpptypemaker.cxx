/*************************************************************************
 *
 *  $RCSfile: cpptypemaker.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-23 08:29:58 $
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

#include "skeletoncpp.hxx"

using namespace ::codemaker::cpp;

namespace skeletonmaker { namespace cpp {

void printType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & name, sal_Int32 rank,
    std::vector< rtl::OString > const & arguments, short referenceType,
    bool shortname, bool defaultvalue)
{
    if (defaultvalue && rank == 0 && sort <= codemaker::UnoType::SORT_CHAR) {
        switch (sort) {
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
        }
    }

    bool bReference = false;
    if ((sort > codemaker::UnoType::SORT_CHAR ||
        rank > 0) && referenceType != 8 )
        bReference = true;

    if (bReference && referenceType == 4)
        o << "const ";

    for (sal_Int32 i = 0; i < rank; ++i) {
        o << ((shortname) ? "css::uno::Sequence< " :
              "::com::sun::star::uno::Sequence< ");
    }
    if (typeClass == RT_TYPE_INTERFACE && referenceType > 0)
        o << ((shortname) ? "css::uno::Reference< " :
              "::com::sun::star::uno::Reference< ");
    o << scopedCppName(codemaker::cpp::translateUnoToCppType(
                                       sort, typeClass, name), false, shortname);
    if (typeClass == RT_TYPE_INTERFACE && referenceType > 0)
        o << " >";
    for (sal_Int32 i = 0; i < rank; ++i) {
        o << " >";
    }

    if (bReference && referenceType > 1)
        o << " &";

    if (referenceType == 8 &&
        (sort > codemaker::UnoType::SORT_CHAR))
        o << "()";
}

void printType(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    rtl::OString const & type, short referenceType, bool shortname,
    bool defaultvalue)
{
    RTTypeClass typeClass;
    rtl::OString name;
    sal_Int32 rank;
    std::vector< rtl::OString > arguments;
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, true, true, true, &typeClass, &name, &rank, &arguments);
    printType(o,
        options, manager, sort, typeClass, name, rank, arguments,
        referenceType, shortname, defaultvalue);
}

bool printConstructorParameters(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, typereg::Reader const & outerReader,
    std::vector< rtl::OString > const & arguments)
{
    bool previous = false;
    if (reader.getSuperTypeCount() != 0) {
        rtl::OString super(
            codemaker::convertString(reader.getSuperTypeName(0)));
        typereg::Reader superReader(manager.getTypeReader(super));
        if (!superReader.isValid()) {
            throw CannotDumpException("Bad type library entity " + super);
        }
        previous = printConstructorParameters(o,
            options, manager, superReader, outerReader, arguments);
    }
    for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
        if (previous) {
            o << ", ";
        }
        previous = true;
        if ((reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) == 0) {
            printType(o,
                options, manager,
                codemaker::convertString(reader.getFieldTypeName(i)),
                4);
        } else if (arguments.empty()) {
            o << "java.lang.Object";
        } else {
            sal_uInt16 tparam = 0;
            while (outerReader.getReferenceTypeName(tparam)
                   != reader.getFieldTypeName(i))
            {
                ++tparam;
                OSL_ASSERT(tparam < outerReader.getReferenceCount());
            }
            // assume std::vector< rtl::OString >::size_type is at least as
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
    std::vector< rtl::OString > const & arguments)
{
    rtl::OString type(codemaker::convertString(reader.getTypeName()));
//     o << "public ";
    o << type.copy(type.lastIndexOf('/') + 1) << '(';
    printConstructorParameters(o, options, manager, reader, reader, arguments);
    o << ");\n";
}

void printMethodParameters(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, sal_uInt16 method, bool previous,
    bool withtype, bool shortname)
{
    short referenceType = 4;
    for (sal_uInt16 i = 0; i < reader.getMethodParameterCount(method); ++i) {
        if (previous) {
            o << ", ";
        }
        previous = true;

        if (reader.getMethodParameterFlags(method, i) == RT_PARAM_OUT
            || reader.getMethodParameterFlags(method, i) == RT_PARAM_INOUT)
            referenceType = 2;

        if (withtype)
            printType(o, options, manager,
                codemaker::convertString(
                    reader.getMethodParameterTypeName(method, i)),
                         referenceType, shortname);
//         if (reader.getMethodParameterFlags(method, i) == RT_PARAM_OUT
//             || reader.getMethodParameterFlags(method, i) == RT_PARAM_INOUT)
//         {
//             o << "[]";
//         } else if ((reader.getMethodParameterFlags(method, i) & RT_PARAM_REST)
//                    != 0)
//         {
//             o << (options.java5 ? "..." : "[]");
//         }
        o << ' '
          << (codemaker::cpp::translateUnoToCppIdentifier(
                  codemaker::convertString(
                      reader.getMethodParameterName(method, i)),
                  "param").
              getStr());
    }
}

void printExceptionSpecification(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, sal_uInt16 method, bool shortname)
{
    o << ((shortname) ? " throw (css::uno::RuntimeException" :
          " throw (::com::sun::star::uno::RuntimeException");
    if (reader.getMethodExceptionCount(method) > 0) {
        for (sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method); ++i)
        {
            o << ", ";
            printType(o,
                options, manager,
                codemaker::convertString(
                    reader.getMethodExceptionTypeName(method, i)),
                0, shortname);
        }
    }
    o << ")";
}

void printMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, codemaker::GeneratedTypeSet & generated,
    rtl::OString const & delegate, rtl::OString const & classname,
    rtl::OString const & indentation, bool shortname, bool defaultvalue)
{
    rtl::OString type(codemaker::convertString(reader.getTypeName()));
    if (generated.contains(type) || type.equals("com/sun/star/uno/XInterface") ||
        (defaultvalue &&
         ( type.equals("com/sun/star/lang/XComponent") ||
           type.equals("com/sun/star/lang/XTypeProvider") ||
           type.equals("com/sun/star/uno/XWeak")) ) ) {
        return;
    }

    static rtl::OString sd(RTL_CONSTASCII_STRINGPARAM("_"));
    bool body = ((delegate.getLength() > 0) ? true : false);
    bool defaultbody = ((delegate.equals(sd)) ? true : false);

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
                            classname, indentation, shortname, defaultvalue);
        }
        if (reader.getFieldCount() > 0 || reader.getMethodCount() > 0) {
            o << indentation << "/* ";
            printType(o, options, manager, type, 0);
            o << ": */\n";
        }
    }
    sal_uInt16 method = 0;
    for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
        o << indentation;
        printType(o,
            options, manager,
            codemaker::convertString(reader.getFieldTypeName(i)), 1, shortname);
        o << " SAL_CALL ";
        if (classname.getLength() > 0)
            o << classname;
        o << "get"
          << codemaker::convertString(reader.getFieldName(i)).getStr()
          << "()";
        if (method < reader.getMethodCount()
            && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_GET
            && reader.getMethodName(method) == reader.getFieldName(i))
        {
            printExceptionSpecification(o, options, manager, reader,
                                           method++, shortname);
        }
        if (body) {
            if (defaultbody) {
                o << "\n{\n    return ";
                printType(o,
                    options, manager,
                    codemaker::convertString(reader.getFieldTypeName(i)), 8,
                    shortname, defaultvalue);
                o << "\n}\n\n";
            } else {
                o << "\n" << indentation << "    { return "
                  << delegate.getStr() << "get"
                  << codemaker::convertString(reader.getFieldName(i)).getStr()
                  << "(); }";
            }
        } else
            o << ";\n";
        if ((reader.getFieldFlags(i) & RT_ACCESS_READONLY) == 0) {
            o << indentation << "void SAL_CALL ";
            if (classname.getLength() > 0)
                o << classname;
            o << "set"
              << (codemaker::convertString(reader.getFieldName(i)).getStr())
              << '(';
            printType(o,
                options, manager,
                codemaker::convertString(reader.getFieldTypeName(i)),
                4, shortname);
            o << " the_value)";
            if (method < reader.getMethodCount()
                && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_SET
                && reader.getMethodName(method) == reader.getFieldName(i))
            {
                printExceptionSpecification(o, options, manager, reader,
                                               method++, shortname);
            }
            if (body) {
                if (defaultbody) {
                    o << "\n{\n\n}\n\n";
                } else {
                    o << "\n" << indentation << "    { "
                      << delegate.getStr() << "set"
                      << codemaker::convertString(reader.getFieldName(i)).getStr()
                      << "(the_value); }";
                }
            } else
                o << ";\n";
        }
    }
    for (; method < reader.getMethodCount(); ++method) {
        o << indentation;
        printType(o,
            options, manager,
            codemaker::convertString(
                reader.getMethodReturnTypeName(method)), 1, shortname);
        o << " SAL_CALL ";
        if (classname.getLength() > 0)
            o << classname;
        o << (codemaker::convertString(reader.getMethodName(method)).getStr())
          << '(';
        printMethodParameters(o, options, manager, reader, method, false,
                                 true, shortname);
        o << ')';
        printExceptionSpecification(o, options, manager, reader,
                                       method, shortname);
        if (body) {
            static rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("void"));
            if (defaultbody) {
                o << "\n{\n";
                if (!reader.getMethodReturnTypeName(method).equals(s)) {
                    o << "    return ";
                    printType(o,
                        options, manager,
                        codemaker::convertString(
                            reader.getMethodReturnTypeName(method)), 8,
                        shortname, defaultvalue);
                    o << ";";
                }
                o << "\n}\n\n";
            } else {
                o << "\n" << indentation << "    { ";
                if (!reader.getMethodReturnTypeName(method).equals(s))
                    o << "return ";
                o << delegate.getStr()
                  << (codemaker::convertString(
                          reader.getMethodName(method)).getStr())
                  << '(';
                printMethodParameters(o, options, manager, reader, method,
                                         false, false, shortname);
                o << "); }\n";
            }
        } else
            o << ";\n";
    }
}

void printConstructionMethods(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader)
{
    for (sal_uInt16 i = 0; i < reader.getMethodCount(); ++i) {
//        o << "public static ";
        o << "static ";
        printType(o,
            options, manager,
            codemaker::convertString(reader.getSuperTypeName(0)), 1);
        o << ' ';
        if (reader.getMethodName(i).getLength() == 0) {
            o << "create";
        } else {
            o << (codemaker::cpp::translateUnoToCppIdentifier(
                      codemaker::convertString(reader.getMethodName(i)),
                      "method").
                  getStr());
        }
        o << '(';
        printMethodParameters(o, options, manager, reader, i, false, true);
        o << ')';
        printExceptionSpecification(o, options, manager, reader, i);
        o << ";\n";
    }
}

void printServiceMembers(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    typereg::Reader const & reader, rtl::OString const & type,
    rtl::OString const & delegate)
{
    for (sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i) {
        rtl::OString referenceType(
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

    if (delegate.getLength() == 0) {
        o << "\n// properties of service \""<< type.getStr() << "\"\n";
        for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
            rtl::OString fieldName(
                codemaker::convertString(reader.getFieldName(i)));
            rtl::OString fieldType(
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
    rtl::OString const & name, sal_Int32 rank,
    std::vector< rtl::OString > const & arguments, const char * cppTypeSort)
{
    o << "maps to C++ ";
    if (cppTypeSort != 0) {
        o << cppTypeSort << ' ';
    }
    o << "type \"";
    if (rank == 0 && name == "com/sun/star/uno/XInterface") {
        o << "Reference< com::sun::star::uno::XInterface >";
    } else {
        printType(o,
            options, manager, sort, typeClass, name, rank, arguments, 0);
    }
    o << '"';
}

void generateDocumentation(std::ostream & o,
    ProgramOptions const & options, TypeManager const & manager,
    rtl::OString const & type, rtl::OString const & delegate)
{
    if (type.indexOf('/') >= 0) {
        throw CannotDumpException("Illegal type name " + type);
    }
    rtl::OString binType(type.replace('.', '/'));
    RTTypeClass typeClass;
    rtl::OString name;
    sal_Int32 rank;
    std::vector< rtl::OString > arguments;
    codemaker::UnoType::Sort sort = decomposeResolveAndCheck(
        manager, binType, false, true, true, &typeClass, &name, &rank,
        &arguments);

    bool comment=true;
    if (delegate.getLength() > 0) {
        if (typeClass != RT_TYPE_INTERFACE &&
            typeClass != RT_TYPE_SERVICE )
            return;

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
            if (!reader.isValid()) {
                throw CannotDumpException("Bad type library entity " + name);
            }
            switch (typeClass) {
            case RT_TYPE_INTERFACE:
                o << " interface type";
                break;

            case RT_TYPE_MODULE:
                o << "IDL module";
                break;

            case RT_TYPE_STRUCT:
                if (reader.getReferenceCount() == 0) {
                    o << " simple struct type";
                } else if (arguments.empty()) {
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
                if (reader.getSuperTypeCount() > 0) {
                    o << " single-inheritance--based service";
                } else {
                    o << "IDL accumulation-based service";
                }
                break;

            case RT_TYPE_SINGLETON:
                if ((manager.getTypeReader(
                         codemaker::convertString(
                             reader.getSuperTypeName(0))).
                     getTypeClass())
                    == RT_TYPE_INTERFACE)
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
        if (!reader.isValid()) {
            throw CannotDumpException("Bad type library entity " + name);
        }
        switch (typeClass) {
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
                    o << "; " << (options.all ? "all" : "direct")
                      << " methods:\n";

                codemaker::GeneratedTypeSet generated;
                printMethods(o, options, manager, reader, generated,
                                delegate, options.implname);
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

                rtl::OString super(
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
                    getTypeClass()) == RT_TYPE_INTERFACE) ) {
                printMapsToCppType(o, options, manager, sort, typeClass,
                                   name, rank, arguments,
                    "class");
//                o << "; get method:\npublic static ";
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


