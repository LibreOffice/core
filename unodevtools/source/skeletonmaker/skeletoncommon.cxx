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

#include "osl/thread.hxx"

#include "codemaker/commonjava.hxx"
#include "codemaker/commoncpp.hxx"
#include "codemaker/generatedtypeset.hxx"

#include "skeletoncommon.hxx"

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

    o << "/*************************************************************************\n"
        " *\n"
        " * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.\n"
        " * \n"
        " * Copyright 2000, 2010 Oracle and/or its affiliates.\n"
        " *\n"
        " * OpenOffice.org - a multi-platform office productivity suite\n"
        " *\n"
        " * This file is part of OpenOffice.org.\n"
        " *\n"
        " * OpenOffice.org is free software: you can redistribute it and/or modify\n"
        " * it under the terms of the GNU Lesser General Public License version 3\n"
        " * only, as published by the Free Software Foundation.\n"
        " *\n"
        " * OpenOffice.org is distributed in the hope that it will be useful,\n"
        " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        " * GNU Lesser General Public License version 3 for more details\n"
        " * (a copy is included in the LICENSE file that accompanied this code).\n"
        " *\n"
        " * You should have received a copy of the GNU Lesser General Public License\n"
        " * version 3 along with OpenOffice.org.  If not, see\n"
        " * <http://www.openoffice.org/license.html>\n"
        " * for a copy of the LGPLv3 License.\n"
        " *\n"
        " ************************************************************************/\n\n";
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
        OString message("cannot open ");
        message += targetSourceFileName + " for writing";
        throw CannotDumpException(message);
    } else {
        tmpSourceFileName = file.getName();
    }
    file.close();
    *ppOutputStream = new std::ofstream(tmpSourceFileName.getStr(),
                                        std::ios_base::binary);

    return bStandardout;
}

codemaker::UnoType::Sort decomposeResolveAndCheck(
    TypeManager const & manager, OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, OString * name, sal_Int32 * rank,
    std::vector< OString > * arguments)
{
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, resolveTypedefs, allowVoid, allowExtraEntities,
        typeClass, name, rank, arguments);
    for ( std::vector< OString >::iterator i(arguments->begin());
          i != arguments->end(); ++i )
    {
        RTTypeClass typeClass2;
        OString name2;
        sal_Int32 rank2;
        std::vector< OString > arguments2;
        decomposeResolveAndCheck(
            manager, *i, true, false, false, &typeClass2, &name2, &rank2,
            &arguments2);
    }
    return sort;
}

bool containsAttribute(AttributeInfo& attributes, OString const & attrname)
{
    for ( AttributeInfo::const_iterator i(attributes.begin());
          i != attributes.end(); ++i ) {
        if ( (*i).first == attrname ) {
            return true;
        }
    }
    return false;
}

// collect attributes including inherited attributes
void checkAttributes(TypeManager const & manager,
                     const typereg::Reader& reader,
                     AttributeInfo& attributes,
                     boost::unordered_set< OString, OStringHash >& propinterfaces)
{
    OString typeName = codemaker::convertString(reader.getTypeName());
    if ( typeName.equals("com/sun/star/beans/XPropertySet") ||
         typeName.equals("com/sun/star/beans/XFastPropertySet") ||
//        typeName.equals("com/sun/star/beans/XMultiPropertySet") ||
         typeName.equals("com/sun/star/beans/XPropertyAccess") )
    {
        propinterfaces.insert(typeName);
    }

    for ( sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i ) {
        typereg::Reader supertype(manager.getTypeReader(
                                  codemaker::convertString(
                                      reader.getSuperTypeName(i))));
        if ( !supertype.isValid() ) {
            throw CannotDumpException(
                "Bad type library entity "
                + codemaker::convertString(reader.getSuperTypeName(i)));
        }
        checkAttributes(manager, supertype, attributes, propinterfaces);
    }

    for ( sal_uInt16 i = 0; i < reader.getFieldCount(); ++i ) {
        OString fieldName(
            codemaker::convertString(reader.getFieldName(i)).
            replace('/', '.'));

        if ( !containsAttribute(attributes, fieldName) ) {
            OString fieldType(
                codemaker::convertString(reader.getFieldTypeName(i)).
                replace('/', '.'));
            attributes.push_back(AttributeInfo::value_type(
                                     fieldName, std::pair<OString, sal_Int16>(
                                         fieldType, reader.getFieldFlags(i))));
        }
    }
}

void checkType(TypeManager const & manager,
               OString const & type,
               boost::unordered_set< OString, OStringHash >& interfaceTypes,
               boost::unordered_set< OString, OStringHash >& serviceTypes,
               AttributeInfo& properties)
{

    OString binType(type.replace('.', '/'));
    typereg::Reader reader(manager.getTypeReader(binType));
    if ( !reader.isValid() ) {
        throw CannotDumpException("Bad type library entity " + binType);
    }

    switch ( reader.getTypeClass() )
    {
    case RT_TYPE_INTERFACE:
    {
        // com/sun/star/lang/XComponent should be also not in the list
        // but it will be used for checking the impl helper and will be
        // removed later if necessary.
        if ( binType.equals("com/sun/star/lang/XTypeProvider") ||
             binType.equals("com/sun/star/uno/XWeak") )
            return;
        if (interfaceTypes.find(type) == interfaceTypes.end()) {
            interfaceTypes.insert(type);
        }
    }
        break;
    case RT_TYPE_SERVICE:
        if ( serviceTypes.find(binType) == serviceTypes.end() ) {
            serviceTypes.insert(binType);

            if ( reader.getSuperTypeCount() > 0 ) {
                OString supername(codemaker::convertString(
                    reader.getSuperTypeName(0).replace('/', '.')));
                if ( interfaceTypes.find(supername) == interfaceTypes.end() ) {
                    interfaceTypes.insert(supername);

                    typereg::Reader supertype(manager.getTypeReader(
                                  codemaker::convertString(
                                      reader.getSuperTypeName(0))));
                    if ( !supertype.isValid() ) {
                        throw CannotDumpException(
                            "Bad type library entity "
                            + codemaker::convertString(reader.getSuperTypeName(0)));
                    }
                }

                // check if constructors are specified, if yes automatically
                // support of XInitialization. We will take care of the default
                // constructor because in this case XInitialization is not called.
                if ( reader.getMethodCount() > 1 ||
                     ( reader.getMethodCount() == 1 &&
                       reader.getMethodName(0).getLength() > 0 ) )
                {
                    OString s("com.sun.star.lang.XInitialization");
                    if ( interfaceTypes.find(s) == interfaceTypes.end() )
                        interfaceTypes.insert(s);
                }
            } else {
                for ( sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i ) {
                    OString referenceType(
                        codemaker::convertString(
                            reader.getReferenceTypeName(i)).replace('/', '.'));

                    if ( reader.getReferenceSort(i) == RT_REF_SUPPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties);
                    } else if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
                        checkType(manager, referenceType, interfaceTypes,
                                  serviceTypes, properties);
                    }
                }

                for ( sal_uInt16 i = 0; i < reader.getFieldCount(); ++i ) {
                    OString fieldName(
                        codemaker::convertString(reader.getFieldName(i)).
                        replace('/', '.'));
                    OString fieldType(
                        codemaker::convertString(reader.getFieldTypeName(i)).
                        replace('/', '.'));

                    properties.push_back(AttributeInfo::value_type(
                        fieldName, std::pair<OString, sal_Int16>(
                            fieldType, reader.getFieldFlags(i))));
                }
            }
        }
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
}

void checkDefaultInterfaces(
         boost::unordered_set< OString, OStringHash >& interfaces,
         const boost::unordered_set< OString, OStringHash >& services,
       const OString & propertyhelper)
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

bool checkServiceProperties(TypeManager const & manager,
                            const typereg::Reader & reader)
{
    if ( reader.getFieldCount() > 0 )
        return true;

    if ( reader.getReferenceCount() > 0 ) {
        for ( sal_uInt16 i = 0; i < reader.getReferenceCount(); ++i ) {
            if ( reader.getReferenceSort(i) == RT_REF_EXPORTS ) {
                typereg::Reader refreader(
                    manager.getTypeReader(
                        codemaker::convertString(reader.getReferenceTypeName(i))));

                if ( checkServiceProperties(manager, refreader) )
                    return true;
            }
        }
    }
    return false;
}


OString checkPropertyHelper(
    ProgramOptions const & options,
    TypeManager const & manager,
    const boost::unordered_set< OString, OStringHash >& services,
    const boost::unordered_set< OString, OStringHash >& interfaces,
    AttributeInfo& attributes,
    boost::unordered_set< OString, OStringHash >& propinterfaces)
{
    boost::unordered_set< OString, OStringHash >::const_iterator iter;
    boost::unordered_set< OString, OStringHash >::const_iterator end;

    if ( !services.empty() ) {
        iter = services.begin();
        end = services.end();
    } else {
        iter = interfaces.begin();
        end = interfaces.end();
    }

    bool oldStyleWithProperties = false;
    while ( iter != end ) {
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.', '/')));

        if ( !services.empty() ) {
            if ( options.supportpropertysetmixin && reader.getSuperTypeCount() > 0 )
            {
                typereg::Reader supertype(
                    manager.getTypeReader(
                        codemaker::convertString(
                            reader.getSuperTypeName(0))));
                if ( !supertype.isValid() ) {
                    throw CannotDumpException(
                        "Bad type library entity "
                        + codemaker::convertString(
                            reader.getSuperTypeName(0)));
                }

                checkAttributes(manager, supertype, attributes, propinterfaces);

                if ( !(attributes.empty() || propinterfaces.empty()) ) {
                    return OUStringToOString(
                        supertype.getTypeName().replace('/', '.'),
                        osl_getThreadTextEncoding());
                }
            } else {
                oldStyleWithProperties = checkServiceProperties(manager, reader);
            }
        } else {
            checkAttributes(manager, reader, attributes, propinterfaces);
            if ( !(attributes.empty() || propinterfaces.empty()) ) {
                return OUStringToOString(
                    reader.getTypeName().replace('/', '.'),
                    osl_getThreadTextEncoding());
            }
        }
        ++iter;
    }

    return (oldStyleWithProperties ? "_" : "");
}

bool checkXComponentSupport(TypeManager const & manager,
                            typereg::Reader const & reader)
{
    static OUString s(RTL_CONSTASCII_USTRINGPARAM(
                               "com/sun/star/lang/XComponent"));
    if ( reader.getTypeName().equals(s) )
        return true;

    for ( sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i ) {
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
        if ( checkXComponentSupport(manager, super) )
            return true;
    }

    return false;
}


// if XComponent is directly specified, return true and remove it from the
// supported interfaces list
bool checkXComponentSupport(TypeManager const & manager,
         boost::unordered_set< OString, OStringHash >& interfaces)
{
    if ( interfaces.empty() )
        return false;

    boost::unordered_set< OString, OStringHash >::const_iterator iter =
        interfaces.begin();
    while ( iter != interfaces.end() ) {
        if ( (*iter).equals("com.sun.star.lang.XComponent") ) {
            interfaces.erase("com.sun.star.lang.XComponent");
            return true;
        }
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.', '/')));
        if ( checkXComponentSupport(manager, reader) )
            return true;
        ++iter;
    }

    return false;
}

sal_uInt16 checkAdditionalPropertyFlags(typereg::Reader const & reader,
                                        sal_uInt16 field, sal_uInt16 method)
{
    sal_uInt16 flags = 0;
    bool getterSupportsUnknown = false;

    OUString su(RTL_CONSTASCII_USTRINGPARAM(
                   "com/sun/star/beans/UnknownPropertyException"));
    if ( method < reader.getMethodCount()
         && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_GET
         && reader.getMethodName(method) == reader.getFieldName(field) )
    {
        if ( reader.getMethodExceptionCount(method) > 0 ) {
            for ( sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method);
                  ++i )
            {
                if (su.equals(reader.getMethodExceptionTypeName(method, i)))
                    getterSupportsUnknown = true;
            }
        }
        method++;
    }
    if ( method < reader.getMethodCount()
         && reader.getMethodFlags(method) == RT_MODE_ATTRIBUTE_SET
         && reader.getMethodName(method) == reader.getFieldName(field) )
    {
        if ( reader.getMethodExceptionCount(method) > 0 ) {
            OUString s(RTL_CONSTASCII_USTRINGPARAM(
                           "com/sun/star/beans/PropertyVetoException"));
            for ( sal_uInt16 i = 0; i < reader.getMethodExceptionCount(method);
                  ++i )
            {
                if ( s.equals(reader.getMethodExceptionTypeName(method, i)) )
                    flags |= RT_ACCESS_CONSTRAINED;
                if ( getterSupportsUnknown &&
                     su.equals(reader.getMethodExceptionTypeName(method, i)) )
                    flags |= RT_ACCESS_OPTIONAL;
            }
        }
    }
    return flags;
}

// This function checks if the specified types for paramters and return
// types are allowed add-in types, for more info see the com.sun.star.sheet.AddIn
// service description
bool checkAddinType(TypeManager const & manager,
                    OString const & type, bool & bLastAny,
                    bool & bHasXPropertySet, bool bIsReturn)
{
    RTTypeClass typeClass;
    OString name;
    sal_Int32 rank;
    std::vector< OString > arguments;
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, true, true, true, &typeClass, &name, &rank, &arguments);

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
    if ( sort == codemaker::UnoType::SORT_COMPLEX &&
         typeClass == RT_TYPE_INTERFACE )
    {
        if ( bIsReturn && type.equals("com/sun/star/sheet/XVolatileResult") )
            return true;
        if ( !bIsReturn && type.equals("com/sun/star/table/XCellRange") )
            return true;
        if ( !bIsReturn && type.equals("com/sun/star/beans/XPropertySet") )
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

void checkAddInTypes(TypeManager const & manager,
                     typereg::Reader const & reader)
{
    OString sType(codemaker::convertString(reader.getTypeName()).replace('/', '.'));
    bool bLastAny = false;
    bool bHasXPropertySet = false;
    for ( sal_uInt16 m = 0; m < reader.getMethodCount(); ++m ) {
        OString sMethod(codemaker::convertString(reader.getMethodName(m)));

        OString sReturnType(codemaker::convertString(
                                reader.getMethodReturnTypeName(m)));
        if ( !checkAddinType(
                 manager, sReturnType, bLastAny, bHasXPropertySet, true) )
        {
            OStringBuffer msg("the return type of the calc add-in function '");
            msg.append(sType);
            msg.append(":");
            msg.append(sMethod);
            msg.append("' is invalid. Please check your IDL defintion.");
            throw CannotDumpException(msg.makeStringAndClear());
        }

        bHasXPropertySet = false;
        for ( sal_uInt16 p = 0; p < reader.getMethodParameterCount(m); ++p ) {
            bLastAny = false;
            OString sParamType(codemaker::convertString(
                                   reader.getMethodParameterTypeName(m, p)));
            if ( !checkAddinType(manager, sParamType,
                                bLastAny, bHasXPropertySet, false) ||
                 bLastAny )
            {
                OStringBuffer msg("the type of the ");
                msg.append((sal_Int32)p+1);
                msg.append(". parameter of the calc add-in function '");
                msg.append(sType);
                msg.append(":");
                msg.append(sMethod);
                msg.append("' is invalid.");
                if ( bLastAny )
                    msg.append(" The type 'sequence<any>' is allowed as last "
                               "parameter only.");
                if ( bHasXPropertySet )
                    msg.append(" The type 'XPropertySet' is allowed only once.");

                msg.append(" Please check your IDL definition.");
                throw CannotDumpException(msg.makeStringAndClear());
            }
        }
    }
}

void generateFunctionParamterMap(std::ostream& o,
                                 ProgramOptions const & options,
                                 TypeManager const & manager,
                                 typereg::Reader const & reader,
                                 ::codemaker::GeneratedTypeSet & generated,
                                 bool bFirst)
{
    OString sType(codemaker::convertString(reader.getTypeName()));
    if ( sType.equals("com/sun/star/uno/XInterface") ||
         sType.equals("com/sun/star/lang/XLocalizable") ||
         sType.equals("com/sun/star/lang/XServiceInfo") ||
         // the next three checks becomes obsolete when configuration is used
         sType.equals("com/sun/star/sheet/XAddIn") ||
         sType.equals("com/sun/star/sheet/XCompatibilityNames") ||
         sType.equals("com/sun/star/lang/XServiceName") )
    {
        return;
    }

    // check if the specified add-in functions supports valid types
    checkAddInTypes(manager, reader);

    for ( sal_uInt16 i = 0; i < reader.getSuperTypeCount(); ++i ) {
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
        generateFunctionParamterMap(o, options, manager, super, generated, bFirst);
    }

    OString type(codemaker::convertString(reader.getTypeName()));
    if ( generated.contains(type) )
        return;
    else
        generated.add(type);

    for ( sal_uInt16 m = 0; m < reader.getMethodCount(); ++m ) {
        OString sMethod(codemaker::convertString(reader.getMethodName(m)));

        if ( bFirst ) {
            if (options.language == 2) {
                o << "        ParamMap fpm;\n";
            }
            else {
                if ( options.java5 )
                    o << "        java.util.Hashtable< Integer, String > fpm = "
                        "new java.util.Hashtable< Integer, String >();\n";
                else
                    o << "        java.util.Hashtable fpm = "
                        "new java.util.Hashtable();\n";
            }
            bFirst = false;
        } else
            if ( options.language == 2 ) {
                o << "        fpm = ParamMap();\n";
            }
            else {
                if ( options.java5 )
                    o << "        fpm = new java.util.Hashtable< "
                        "Integer, String >();\n";
                else
                    o << "        fpm = new java.util.Hashtable();\n";
            }

        for ( sal_uInt16 p = 0; p < reader.getMethodParameterCount(m); ++p ) {
            if ( options.language == 2 ) {
                o << "        fpm[" << p
                  << "] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\""
                  << codemaker::convertString(reader.getMethodParameterName(m, p))
                  << "\"));\n";
            }
            else {
                if ( options.java5 )
                    o << "        fpm.put(" << p << ", \""
                      << codemaker::convertString(
                          reader.getMethodParameterName(m, p))
                      << "\");\n";
                else
                    o << "       fpm.put(new Integer(" << p << "), \""
                      << codemaker::convertString(
                          reader.getMethodParameterName(m, p))
                      << "\");\n";
            }
        }

        if ( options.language == 2 ) {
            o << "        m_functionMap[::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\""
              << sMethod << "\"))] = fpm;\n\n";
        }
        else {
            o << "        m_functionMap.put(\"" << sMethod << "\", fpm);\n\n";
        }
    }
}

void generateFunctionParameterMap(std::ostream& o,
         ProgramOptions const & options,
         TypeManager const & manager,
         const boost::unordered_set< OString, OStringHash >& interfaces)
{
    ::codemaker::GeneratedTypeSet generated;
    bool bFirst = true;
    boost::unordered_set< OString, OStringHash >::const_iterator iter = interfaces.begin();
    while ( iter != interfaces.end() ) {
        typereg::Reader reader(manager.getTypeReader((*iter).replace('.','/')));
        if (!reader.isValid()) {
            throw CannotDumpException(
                "Bad type library entity "
                + codemaker::convertString(
                    reader.getTypeName()));
        }

        generateFunctionParamterMap(o, options, manager, reader, generated, bFirst);
        ++iter;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
