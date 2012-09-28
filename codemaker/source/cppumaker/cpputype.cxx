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


#include <algorithm>
#include <map>
#include <set>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "registry/reader.hxx"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"

#include "codemaker/dependencies.hxx"
#include "codemaker/exceptiontree.hxx"
#include "codemaker/generatedtypeset.hxx"
#include "codemaker/unotype.hxx"

#include "cpputype.hxx"
#include "cppuoptions.hxx"
#include "dumputils.hxx"
#include "includes.hxx"

using namespace codemaker::cpp;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringBuffer;

namespace {

rtl::OString translateSimpleUnoType(rtl::OString const & unoType, bool cppuUnoType=false) {
    static rtl::OString const trans[codemaker::UnoType::SORT_COMPLEX + 1] = {
        "void", "::sal_Bool", "::sal_Int8", "::sal_Int16", "::sal_uInt16",
        "::sal_Int32", "::sal_uInt32", "::sal_Int64", "::sal_uInt64", "float",
        "double", "::sal_Unicode", "::rtl::OUString",
        "::com::sun::star::uno::Type", "::com::sun::star::uno::Any",
        rtl::OString() };

    const codemaker::UnoType::Sort sort = codemaker::UnoType::getSort(unoType);
    if (cppuUnoType &&
        (sort == codemaker::UnoType::SORT_UNSIGNED_SHORT ||
         sort == codemaker::UnoType::SORT_CHAR) )
    {
        if (sort == codemaker::UnoType::SORT_CHAR)
            return "::cppu::UnoCharType";
        else
            return "::cppu::UnoUnsignedShortType";
    }

    return trans[sort];
}

bool isBootstrapType(rtl::OString const & name) {
    static char const * const names[] = {
        "com/sun/star/beans/PropertyAttribute",
        "com/sun/star/beans/PropertyValue",
        "com/sun/star/beans/XFastPropertySet",
        "com/sun/star/beans/XMultiPropertySet",
        "com/sun/star/beans/XPropertyAccess",
        "com/sun/star/beans/XPropertySet",
        "com/sun/star/beans/XPropertySetOption",
        "com/sun/star/bridge/UnoUrlResolver",
        "com/sun/star/bridge/XUnoUrlResolver",
        "com/sun/star/connection/SocketPermission",
        "com/sun/star/container/XElementAccess",
        "com/sun/star/container/XEnumerationAccess",
        "com/sun/star/container/XHierarchicalNameAccess",
        "com/sun/star/container/XNameAccess",
        "com/sun/star/container/XNameContainer",
        "com/sun/star/container/XNameReplace",
        "com/sun/star/container/XSet",
        "com/sun/star/io/FilePermission",
        "com/sun/star/io/IOException",
        "com/sun/star/lang/DisposedException",
        "com/sun/star/lang/WrappedTargetRuntimeException",
        "com/sun/star/lang/XComponent",
        "com/sun/star/lang/XEventListener",
        "com/sun/star/lang/XInitialization",
        "com/sun/star/lang/XMultiComponentFactory",
        "com/sun/star/lang/XMultiServiceFactory",
        "com/sun/star/lang/XServiceInfo",
        "com/sun/star/lang/XSingleComponentFactory",
        "com/sun/star/lang/XSingleServiceFactory",
        "com/sun/star/lang/XTypeProvider",
        "com/sun/star/loader/XImplementationLoader",
        "com/sun/star/reflection/XArrayTypeDescription",
        "com/sun/star/reflection/XCompoundTypeDescription",
        "com/sun/star/reflection/XEnumTypeDescription",
        "com/sun/star/reflection/XIdlClass",
        "com/sun/star/reflection/XIdlField2",
        "com/sun/star/reflection/XIdlReflection",
        "com/sun/star/reflection/XIndirectTypeDescription",
        "com/sun/star/reflection/XInterfaceAttributeTypeDescription",
        "com/sun/star/reflection/XInterfaceAttributeTypeDescription2",
        "com/sun/star/reflection/XInterfaceMemberTypeDescription",
        "com/sun/star/reflection/XInterfaceMethodTypeDescription",
        "com/sun/star/reflection/XInterfaceTypeDescription",
        "com/sun/star/reflection/XInterfaceTypeDescription2",
        "com/sun/star/reflection/XMethodParameter",
        "com/sun/star/reflection/XStructTypeDescription",
        "com/sun/star/reflection/XTypeDescription",
        "com/sun/star/reflection/XTypeDescriptionEnumerationAccess",
        "com/sun/star/reflection/XUnionTypeDescription",
        "com/sun/star/registry/XImplementationRegistration",
        "com/sun/star/registry/XRegistryKey",
        "com/sun/star/registry/XSimpleRegistry",
        "com/sun/star/security/RuntimePermission",
        "com/sun/star/security/XAccessController",
        "com/sun/star/uno/DeploymentException",
        "com/sun/star/uno/RuntimeException",
        "com/sun/star/uno/XAggregation",
        "com/sun/star/uno/XComponentContext",
        "com/sun/star/uno/XCurrentContext",
        "com/sun/star/uno/XUnloadingPreference",
        "com/sun/star/uno/XWeak",
        "com/sun/star/util/XMacroExpander"
    }; // cf. cppuhelper/unotypes/Makefile UNOTYPES (plus missing dependencies)
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(names); ++i) {
        if (name.equals(names[i])) {
            return true;
        }
    }
    return false;
}

}

//*************************************************************************
// CppuType
//*************************************************************************
CppuType::CppuType(typereg::Reader& typeReader,
                   const OString& typeName,
                   const TypeManager& typeMgr)
    : m_inheritedMemberCount(0)
    , m_cppuTypeLeak(sal_False)
    , m_cppuTypeDynamic(sal_True)
    , m_indentLength(0)
    , m_typeName(typeName)
    , m_name(typeName.copy(typeName.lastIndexOf('/') + 1))
    , m_reader(typeReader)
    , m_typeMgr(typeMgr)
    , m_dependencies(typeMgr, typeName)
{}

CppuType::~CppuType()
{

}

void CppuType::addGetCppuTypeIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    if (m_typeName.equals("com/sun/star/uno/XInterface")
        || m_typeName.equals("com/sun/star/uno/Exception"))
    {
        includes.addType();
        includes.addCppuUnotypeHxx();
        includes.addSalTypesH();
        includes.addTypelibTypeclassH();
        includes.addTypelibTypedescriptionH();
    } else if (m_cppuTypeLeak) {
        addLightGetCppuTypeIncludes(includes);
    } else if (m_cppuTypeDynamic) {
        addNormalGetCppuTypeIncludes(includes);
    } else {
        addComprehensiveGetCppuTypeIncludes(includes);
    }
}

void CppuType::dumpDeclaration(FileStream &) throw (CannotDumpException) {
    OSL_ASSERT(false);
}

bool CppuType::dumpFiles(CppuOptions * options, rtl::OString const & outPath) {
    return dumpFile(options, ".hdl", m_typeName, outPath)
        && dumpFile(options, ".hpp", m_typeName, outPath);
}

void CppuType::addLightGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

void CppuType::addNormalGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

void CppuType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    //TODO: Determine what is really needed, instead of relying on
    // addDefaultHxxIncludes
    includes.addCppuUnotypeHxx();
}

bool CppuType::isPolymorphic() const { return false; }

void CppuType::dumpTemplateHead(FileStream &) const {}

void CppuType::dumpTemplateParameters(FileStream &) const {}

void CppuType::dumpGetCppuTypePreamble(FileStream & out) {
    if (isPolymorphic()) {
        out << "namespace cppu {\n\n";
        dumpTemplateHead(out);
        out << "class UnoType< ";
        dumpType(out, m_typeName);
        dumpTemplateParameters(out);
        out << " > {\npublic:\n";
        inc();
        out << indent()
            << "static inline ::com::sun::star::uno::Type const & get() {\n";
    } else {
        if (codemaker::cppumaker::dumpNamespaceOpen(out, m_typeName, false)) {
            out << "\n\n";
        }
        out << ("inline ::com::sun::star::uno::Type const &"
                " cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ");
        dumpType(out, m_typeName, false, false, true);
        out << " const *) {\n";
    }
    inc();
}

void CppuType::dumpGetCppuTypePostamble(FileStream & out) {
    dec();
    if (isPolymorphic()) {
        out << indent() << "}\n\nprivate:\n"
            << indent() << "UnoType(UnoType &); // not defined\n"
            << indent() << "~UnoType(); // not defined\n"
            << indent()
            << "void operator =(UnoType); // not defined\n};\n\n}\n\n";
    } else {
        out << "}\n\n";
        if (codemaker::cppumaker::dumpNamespaceClose(out, m_typeName, false)) {
            out << "\n\n";
        }
    }
    dumpTemplateHead(out);
    out << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
            " getCppuType(SAL_UNUSED_PARAMETER ");
    dumpType(out, m_typeName);
    dumpTemplateParameters(out);
    out << " const *) SAL_THROW(()) {\n";
    inc();
    out << indent() << "return ::cppu::UnoType< ";
    dumpType(out, m_typeName);
    dumpTemplateParameters(out);
    out << " >::get();\n";
    dec();
    out << indent() << "}\n";
}

sal_Bool CppuType::dump(CppuOptions* pOptions)
    throw( CannotDumpException )
{
    if (!m_dependencies.isValid()) {
        return false;
    }
    addSpecialDependencies();

    if (isBootstrapType(m_typeName)) {
        m_cppuTypeDynamic = false;
    } else {
        // -CS was used as an undocumented option to generate static getCppuType
        // functions; since the introduction of cppu::UnoType this no longer is
        // meaningful (getCppuType is just a forward to cppu::UnoType::get now),
        // and -CS is handled the same way as -C now:
        if (pOptions->isValid("-L"))
            m_cppuTypeLeak = sal_True;
        if (pOptions->isValid("-C") || pOptions->isValid("-CS"))
            m_cppuTypeDynamic = sal_False;
    }

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    return dumpFiles(pOptions, outPath);
}

sal_Bool CppuType::dumpFile(CppuOptions* pOptions,
                            const OString& sExtension,
                            const OString& sName,
                            const OString& sOutPath )
    throw( CannotDumpException )
{
    sal_Bool ret = sal_False;

    OString sTmpExt(".tml");
    sal_Bool bHdl = sal_True;    ;
    if (sExtension.equals(".hpp")) {
        sTmpExt = ".tmp";
        bHdl = sal_False;
    }

    OString sFileName = createFileNameFromType(sOutPath, sName, sExtension);
    if (sFileName.isEmpty())
        return sal_False;

    sal_Bool bFileExists = fileExists( sFileName );
    sal_Bool bFileCheck = sal_False;

    if ( bFileExists && pOptions->isValid("-G") )
        return sal_True;

    if ( bFileExists && pOptions->isValid("-Gc") )
        bFileCheck = sal_True;

    OString sTmpDir = getTempDir(sFileName);
    FileStream oFile;
    oFile.createTempFile(sTmpDir);
    OString sTmpFileName;

    if(!oFile.isValid())
    {
        OString message("cannot open ");
        message += sFileName + " for writing";
        throw CannotDumpException(message);
    } else
        sTmpFileName = oFile.getName();

    codemaker::cppumaker::Includes includes(m_typeMgr, m_dependencies, !bHdl);
    if (bHdl)
        ret = dumpHFile(oFile, includes);
    else {
        addGetCppuTypeIncludes(includes);
        ret = dumpHxxFile(oFile, includes);
    }

    oFile.close();

    if (ret) {
        ret = makeValidTypeFile(sFileName, sTmpFileName, bFileCheck);
    } else {
        // remove existing type file if something goes wrong to ensure consistency
        if (fileExists(sFileName))
            removeTypeFile(sFileName);

        // remove tmp file if something goes wrong
        removeTypeFile(sTmpFileName);
    }

    return ret;
}

void CppuType::dumpDependedTypes(
    codemaker::GeneratedTypeSet & generated, CppuOptions * options)
{
    codemaker::Dependencies::Map const & map(m_dependencies.getMap());
    for (codemaker::Dependencies::Map::const_iterator i(map.begin());
         i != map.end(); ++i)
    {
        if (!produceType(i->first, m_typeMgr, generated, options)) {
            fprintf(
                stderr, "%s ERROR: cannot dump Type '%s'\n",
                options->getProgramName().getStr(), i->first.getStr());
            exit(99);
        }
    }
}

OString CppuType::dumpHeaderDefine(
    FileStream& o, char const * prefix, sal_Bool bExtended)
{
    if (m_typeName.equals("/"))
    {
        bExtended = sal_False;
        m_typeName = "global";
    }

    sal_uInt32 length = 3 + m_typeName.getLength() + strlen(prefix);

    if (bExtended)
        length += m_name.getLength() + 1;

    OStringBuffer tmpBuf(length);

    tmpBuf.append("INCLUDED_");
    tmpBuf.append(m_typeName);
    tmpBuf.append('_');
    if (bExtended)
    {
        tmpBuf.append(m_name);
        tmpBuf.append('_');
    }
    tmpBuf.append(prefix);

    OString tmp(tmpBuf.makeStringAndClear().replace('/', '_').toAsciiUpperCase());

    o << "#ifndef " << tmp << "\n#define " << tmp << "\n";

    return tmp;
}

void CppuType::addDefaultHIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    //TODO: Only include what is really needed
    includes.addCppuMacrosHxx();
    if (m_typeMgr.getTypeClass(m_typeName) == RT_TYPE_INTERFACE) {
        includes.addReference();
    }
}

void CppuType::addDefaultHxxIncludes(codemaker::cppumaker::Includes & includes)
    const
{
    //TODO: Only include what is really needed
    includes.addRtlInstanceHxx();
    includes.addOslMutexHxx();
    includes.addType();
    if (m_typeMgr.getTypeClass(m_typeName) == RT_TYPE_INTERFACE) {
        includes.addReference();
    }
}

void CppuType::dumpInitializer(
    FileStream & out, bool parameterized, rtl::OUString const & type) const
{
    out << "(";
    if (!parameterized) {
        for (rtl::OString t(
                 rtl::OUStringToOString(type, RTL_TEXTENCODING_UTF8));;)
        {
            sal_Int32 rank;
            std::vector< rtl::OString > args;
            t = codemaker::UnoType::decompose(t, &rank, &args);
            if (rank == 0) {
                switch (codemaker::UnoType::getSort(t)) {
                case codemaker::UnoType::SORT_BOOLEAN:
                    out << "false";
                    break;

                case codemaker::UnoType::SORT_BYTE:
                case codemaker::UnoType::SORT_SHORT:
                case codemaker::UnoType::SORT_UNSIGNED_SHORT:
                case codemaker::UnoType::SORT_LONG:
                case codemaker::UnoType::SORT_UNSIGNED_LONG:
                case codemaker::UnoType::SORT_HYPER:
                case codemaker::UnoType::SORT_UNSIGNED_HYPER:
                case codemaker::UnoType::SORT_FLOAT:
                case codemaker::UnoType::SORT_DOUBLE:
                case codemaker::UnoType::SORT_CHAR:
                    out << "0";
                    break;

                case codemaker::UnoType::SORT_COMPLEX:
                    switch (m_typeMgr.getTypeClass(t)) {
                    case RT_TYPE_ENUM:
                        {
                            typereg::Reader reader(m_typeMgr.getTypeReader(t));
                            OSL_ASSERT(reader.isValid());
                            out << scopedCppName(t) << "_"
                                << rtl::OUStringToOString(
                                    reader.getFieldName(0),
                                    RTL_TEXTENCODING_UTF8);
                            break;
                        }

                    case RT_TYPE_TYPEDEF:
                        t = resolveTypedefs(t);
                        continue;

                    default:
                        break;
                    }
                    break;

                default:
                    break;
                }
            }
            break;
        }
    }
    out << ")";
}

void CppuType::dumpHFileContent(
    FileStream & out, codemaker::cppumaker::Includes & includes)
{
    addDefaultHIncludes(includes);
    dumpHeaderDefine(out, "HDL");
    out << "\n";
    includes.dump(out, 0);
    out << ("\nnamespace com { namespace sun { namespace star { namespace uno"
            " { class Type; } } } }\n\n");
    if (codemaker::cppumaker::dumpNamespaceOpen(out, m_typeName, false)) {
        out << "\n";
    }
    dumpDeclaration(out);
    if (!(m_typeName.equalsL(
              RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XInterface")) ||
          m_typeName.equalsL(
              RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Exception")) ||
          isPolymorphic()))
    {
        out << "\n" << indent()
            << ("inline ::com::sun::star::uno::Type const &"
                " cppu_detail_getUnoType(SAL_UNUSED_PARAMETER ");
        dumpType(out, m_typeName, false, false, true);
        out << " const *);\n";
    }
    if (codemaker::cppumaker::dumpNamespaceClose(out, m_typeName, false)) {
        out << "\n";
    }
    out << "\n";
    dumpTemplateHead(out);
    out << "inline ::com::sun::star::uno::Type const & SAL_CALL getCppuType(";
    dumpType(out, m_typeName, true);
    dumpTemplateParameters(out);
    out << " *) SAL_THROW(());\n\n#endif\n";
}

void CppuType::dumpGetCppuType(FileStream & out) {
    if (m_typeName.equals("com/sun/star/uno/XInterface")) {
        out << indent()
            << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
                " getCppuType(SAL_UNUSED_PARAMETER ");
        dumpType(out, m_typeName, true, false);
        out << " *) SAL_THROW(()) {\n";
        inc();
        out << indent()
            << ("return ::cppu::UnoType< ::com::sun::star::uno::XInterface"
                " >::get();\n");
        dec();
        out << indent() << "}\n";
    } else if (m_typeName.equals("com/sun/star/uno/Exception")) {
        out << indent()
            << ("inline ::com::sun::star::uno::Type const & SAL_CALL"
                " getCppuType(SAL_UNUSED_PARAMETER ");
        dumpType(out, m_typeName, true, false);
        out << " *) SAL_THROW(()) {\n";
        inc();
        out << indent()
            << ("return ::cppu::UnoType< ::com::sun::star::uno::Exception"
                " >::get();\n");
        dec();
        out << indent() << "}\n";
    } else if (m_cppuTypeLeak) {
        dumpLightGetCppuType(out);
    } else if (m_cppuTypeDynamic) {
        dumpNormalGetCppuType(out);
    } else {
        dumpComprehensiveGetCppuType(out);
    }
}

void CppuType::dumpLightGetCppuType(FileStream& o)
{
    dumpGetCppuTypePreamble(o);
    o << indent()
      << "static typelib_TypeDescriptionReference * the_type = 0;\n";

    o << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    o << indent() << "typelib_static_type_init( &the_type, "
      << getTypeClass(m_typeName, sal_True) << ", \"" << m_typeName.replace('/', '.') << "\" );\n";
    dec();
    o << indent() << "}\n";
    o << indent()
      << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
          " &the_type );\n");
    dumpGetCppuTypePostamble(o);
}

void CppuType::dumpNormalGetCppuType(FileStream& o)
{
    dumpGetCppuTypePreamble(o);

    o << indent()
      << "static typelib_TypeDescriptionReference * the_type = 0;\n";

    o << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();

    OString superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    sal_Bool bIsBaseException = sal_False;
    if (!superType.isEmpty())
    {
        if ( superType.equals("com/sun/star/uno/Exception") )
        {
            bIsBaseException = sal_True;
        } else
        {
            o << indent() << "const ::com::sun::star::uno::Type& rBaseType = ::cppu::UnoType< ";
            dumpType(o, superType, true, false, false, true);
            o << " >::get();\n\n";
        }
    }

    sal_uInt32 count = getMemberCount();
    if (count)
    {
        o << indent() << "typelib_TypeDescriptionReference * aMemberRefs[" << count << "];\n";

        sal_uInt16      fieldCount = m_reader.getFieldCount();
        RTFieldAccess   access = RT_ACCESS_INVALID;
        OString         fieldType, fieldName;
        OString         scope = m_typeName.replace('/', '.');
        OString         modFieldType;
        StringSet       generatedTypeSet;
        StringSet::iterator findIter;

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = checkRealBaseType(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8),
                sal_True);

            modFieldType = typeToIdentifier(fieldType);

            findIter = generatedTypeSet.find(fieldType);
            if ( findIter == generatedTypeSet.end() )
            {
                generatedTypeSet.insert(fieldType);
                 o << indent() << "const ::com::sun::star::uno::Type& rMemberType_"
                  << modFieldType/*i*/ << " = ::cppu::UnoType< ";
                dumpType(o, fieldType, false, false, false, true);
                o << " >::get();\n";
            }

            o << indent() << "aMemberRefs[" << i << "] = rMemberType_"
              << modFieldType/*i*/ << ".getTypeLibType();\n";
        }
        o << "\n";
    }

    o << indent() << "typelib_static_compound_type_init( &the_type, "
      << getTypeClass(m_typeName, sal_True) << ", \"" << m_typeName.replace('/', '.') << "\", ";
    if ( !superType.isEmpty() || bIsBaseException )
    {
        if ( bIsBaseException )
        {
            o << "* ::typelib_static_type_getByTypeClass( typelib_TypeClass_EXCEPTION ), "
              << count << ", ";
        } else
        {
            o << "rBaseType.getTypeLibType(), " << count << ", ";
        }
    } else
    {
        o << "0, " << count << ", ";
    }

    if (count)
    {
        o << " aMemberRefs );\n";
    } else
    {
        o << " 0 );\n";
    }
    dec();
    o << indent() << "}\n";
    o << indent()
      << ("return * reinterpret_cast< const ::com::sun::star::uno::Type * >("
          " &the_type );\n");

    dumpGetCppuTypePostamble(o);
}

void CppuType::dumpComprehensiveGetCppuType(FileStream& o)
{
    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false);
    else
        o << "namespace cppu { ";
    o << " namespace detail {\n\n";

    OString sType = m_typeName.copy(m_typeName.lastIndexOf('/') + 1);
    OString sStaticTypeClass = "the" + sType + "Type";
    o << indent() << "struct " << sStaticTypeClass << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, " << sStaticTypeClass << " >\n";
    o << indent() << "{\n";
    inc();
    o << indent() << "::com::sun::star::uno::Type * operator()() const\n";
    o << indent() << "{\n";
    inc();

    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    o << indent() << "// Start inline typedescription generation\n"
      << indent() << "typelib_TypeDescription * pTD = 0;\n";

    OString superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    if (!superType.isEmpty()) {
        o << indent()
          << "const ::com::sun::star::uno::Type& rSuperType = ::cppu::UnoType< ";
        dumpType(o, superType, false, false, false, true);
        o << " >::get();\n";
    }

    dumpCppuGetTypeMemberDecl(o, CPPUTYPEDECL_ALLTYPES);

    sal_uInt32 count = getMemberCount();
    if (count)
    {
        o << "\n" << indent() << "typelib_CompoundMember_Init aMembers["
          << count << "];\n";

        sal_uInt16      fieldCount = m_reader.getFieldCount();
        RTFieldAccess   access = RT_ACCESS_INVALID;
        OString         fieldType, fieldName;
        OString         scope = m_typeName.replace('/', '.');
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID) {
                continue;
            }

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = checkRealBaseType(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8),
                sal_True);

            o << indent() << "::rtl::OUString sMemberType" << i
              << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << fieldType.replace('/', '.') << "\") );\n";
            o << indent() << "::rtl::OUString sMemberName" << i
              << "( RTL_CONSTASCII_USTRINGPARAM(\"";
            o << fieldName << "\") );\n";
            o << indent() << "aMembers[" << i << "].eTypeClass = "
              << "(typelib_TypeClass)" << getTypeClass(fieldType) << ";\n"
              << indent() << "aMembers[" << i << "].pTypeName = sMemberType"
              << i << ".pData;\n"
              << indent() << "aMembers[" << i << "].pMemberName = sMemberName"
              << i << ".pData;\n";
        }
    }

    o << "\n" << indent() << "typelib_typedescription_new(\n";
    inc();
    o << indent() << "&pTD,\n" << indent() << "(typelib_TypeClass)"
      << getTypeClass() << ", sTypeName.pData,\n";

    if (!superType.isEmpty()) {
        o << indent() << "rSuperType.getTypeLibType(),\n";
    } else {
        o << indent() << "0,\n";
    }

    if ( count ) {
        o << indent() << count << ",\n" << indent() << "aMembers );\n\n";
    } else {
        o << indent() << count << ",\n" << indent() << "0 );\n\n";
    }
    dec();

    o << indent()
      << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
          " );\n\n");

    o << indent() << "typelib_typedescription_release( pTD );\n"
      << indent() << "// End inline typedescription generation\n\n";

    o << indent() << "return new ::com::sun::star::uno::Type( "
      << getTypeClass(m_typeName) << ", sTypeName ); // leaked\n";

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "};\n\n";

    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false);
    else
        o << " }";

    o << " }\n\n";

    dumpGetCppuTypePreamble(o);
    o  << indent() << "return *detail::" << sStaticTypeClass << "::get();\n";
    dumpGetCppuTypePostamble(o);
}

void CppuType::dumpCppuGetTypeMemberDecl(FileStream& o, CppuTypeDecl eDeclFlag)
{
    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;

    StringSet aFinishedTypes;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID
            || (access & RT_ACCESS_PARAMETERIZED_TYPE) != 0)
            continue;

        rtl::OString typeName(
            rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
        if (aFinishedTypes.count(typeName) == 0)
        {
            aFinishedTypes.insert(typeName);
            dumpCppuGetType(o, typeName, sal_True, eDeclFlag);
        }
    }
}

IdentifierTranslationMode CppuType::isGlobal() const {
    if ( m_typeName.indexOf('/') < 0 )
        return ITM_GLOBAL;
    else
        return ITM_NONGLOBAL;
}

sal_uInt32 CppuType::getMemberCount()
{
    sal_uInt16 count = m_reader.getMethodCount();

    sal_uInt16 fieldCount = m_reader.getFieldCount();
    RTFieldAccess access = RT_ACCESS_INVALID;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            count++;
    }
    return count;
}

sal_uInt32 CppuType::checkInheritedMemberCount(const typereg::Reader* pReader)
{
    sal_Bool bSelfCheck = sal_True;
    if (!pReader)
    {
        bSelfCheck = sal_False;
        pReader = &m_reader;
    }

    sal_uInt32 count = 0;
    OString superType;
    if (pReader->getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            pReader->getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    if (!superType.isEmpty())
    {
        typereg::Reader aSuperReader(m_typeMgr.getTypeReader(superType));
        if ( aSuperReader.isValid() )
        {
            count = checkInheritedMemberCount(&aSuperReader);
        }
    }

    if (bSelfCheck)
    {
        count += pReader->getMethodCount();
        sal_uInt16 fieldCount = pReader->getFieldCount();
        RTFieldAccess access = RT_ACCESS_INVALID;
        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = pReader->getFieldFlags(i);

            if (access != RT_ACCESS_CONST && access != RT_ACCESS_INVALID)
            {
                count++;
            }
        }
    }

    return count;
}

sal_uInt32 CppuType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}

OString CppuType::getTypeClass(const OString& type, sal_Bool bCStyle)
{
    OString     typeName = (!type.isEmpty() ? type : m_typeName);
    RTTypeClass rtTypeClass = RT_TYPE_INVALID;

    if (!type.isEmpty())
    {
        typeName = type;
        rtTypeClass = m_typeMgr.getTypeClass(typeName);
    } else
    {
        typeName = m_typeName;
        rtTypeClass = m_reader.getTypeClass();
    }

    if (codemaker::UnoType::isSequenceType(typeName))
        return bCStyle ? "typelib_TypeClass_SEQUENCE" : "::com::sun::star::uno::TypeClass_SEQUENCE";

    switch (rtTypeClass)
    {
        case RT_TYPE_INTERFACE:
            return bCStyle ? "typelib_TypeClass_INTERFACE" : "::com::sun::star::uno::TypeClass_INTERFACE";
        case RT_TYPE_MODULE:
            return bCStyle ? "typelib_TypeClass_MODULE" : "::com::sun::star::uno::TypeClass_MODULE";
        case RT_TYPE_STRUCT:
            return bCStyle ? "typelib_TypeClass_STRUCT" : "::com::sun::star::uno::TypeClass_STRUCT";
        case RT_TYPE_ENUM:
            return bCStyle ? "typelib_TypeClass_ENUM" : "::com::sun::star::uno::TypeClass_ENUM";
        case RT_TYPE_EXCEPTION:
            return bCStyle ? "typelib_TypeClass_EXCEPTION" : "::com::sun::star::uno::TypeClass_EXCEPTION";
        case RT_TYPE_TYPEDEF:
            {
                OString realType = checkRealBaseType( typeName );
                return getTypeClass( realType, bCStyle );
            }
        case RT_TYPE_SERVICE:
            return bCStyle ? "typelib_TypeClass_SERVICE" : "::com::sun::star::uno::TypeClass_SERVICE";
        case RT_TYPE_INVALID:
            {
                if (type.equals("long"))
                    return bCStyle ? "typelib_TypeClass_LONG" : "::com::sun::star::uno::TypeClass_LONG";
                if (type.equals("short"))
                    return bCStyle ? "typelib_TypeClass_SHORT" : "::com::sun::star::uno::TypeClass_SHORT";
                if (type.equals("hyper"))
                    return bCStyle ? "typelib_TypeClass_HYPER" : "::com::sun::star::uno::TypeClass_HYPER";
                if (type.equals("string"))
                    return bCStyle ? "typelib_TypeClass_STRING" : "::com::sun::star::uno::TypeClass_STRING";
                if (type.equals("boolean"))
                    return bCStyle ? "typelib_TypeClass_BOOLEAN" : "::com::sun::star::uno::TypeClass_BOOLEAN";
                if (type.equals("char"))
                    return bCStyle ? "typelib_TypeClass_CHAR" : "::com::sun::star::uno::TypeClass_CHAR";
                if (type.equals("byte"))
                    return bCStyle ? "typelib_TypeClass_BYTE" : "::com::sun::star::uno::TypeClass_BYTE";
                if (type.equals("any"))
                    return bCStyle ? "typelib_TypeClass_ANY" : "::com::sun::star::uno::TypeClass_ANY";
                if (type.equals("type"))
                    return bCStyle ? "typelib_TypeClass_TYPE" : "::com::sun::star::uno::TypeClass_TYPE";
                if (type.equals("float"))
                    return bCStyle ? "typelib_TypeClass_FLOAT" : "::com::sun::star::uno::TypeClass_FLOAT";
                if (type.equals("double"))
                    return bCStyle ? "typelib_TypeClass_DOUBLE" : "::com::sun::star::uno::TypeClass_DOUBLE";
                if (type.equals("void"))
                    return bCStyle ? "typelib_TypeClass_VOID" : "::com::sun::star::uno::TypeClass_VOID";
                if (type.equals("unsigned long"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_LONG" : "::com::sun::star::uno::TypeClass_UNSIGNED_LONG";
                if (type.equals("unsigned short"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_SHORT" : "::com::sun::star::uno::TypeClass_UNSIGNED_SHORT";
                if (type.equals("unsigned hyper"))
                    return bCStyle ? "typelib_TypeClass_UNSIGNED_HYPER" : "::com::sun::star::uno::TypeClass_UNSIGNED_HYPER";
            }
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    return bCStyle ? "typelib_TypeClass_UNKNOWN" : "::com::sun::star::uno::TypeClass_UNKNOWN";
}

void CppuType::dumpType(FileStream& o, const OString& type,
                        bool bConst, bool bRef, bool bNative, bool cppuUnoType)
    const throw( CannotDumpException )
{
    sal_Int32 seqNum;
    std::vector< rtl::OString > args;
    rtl::OString relType(
        codemaker::UnoType::decompose(
            checkRealBaseType(type, true), &seqNum, &args));

    RTTypeClass typeClass = m_typeMgr.getTypeClass(relType);

    if (bConst) o << "const ";

    {for (sal_Int32 i = 0; i < seqNum; ++i) {
        if (cppuUnoType)
            o << "::cppu::UnoSequenceType< ";
        else
            o << "::com::sun::star::uno::Sequence< ";
    }}

    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            if (bNative)
                o << scopedCppName(relType);
            else
                o << "::com::sun::star::uno::Reference< "
                  << scopedCppName(relType) << " >";
            break;
        case RT_TYPE_INVALID:
            {
                OString tmp(translateSimpleUnoType(relType, cppuUnoType));
                if (!tmp.isEmpty())
                {
                    o << tmp;
                } else
                    throw CannotDumpException("Unknown type '" + relType +
                                              "', incomplete type library.");
            }
            break;
        case RT_TYPE_STRUCT:
        case RT_TYPE_ENUM:
        case RT_TYPE_TYPEDEF:
        case RT_TYPE_EXCEPTION:
            {
                o << scopedCppName(relType);
                if (!args.empty()) {
                    o << "< ";
                    for (std::vector< rtl::OString >::iterator i(args.begin());
                         i != args.end(); ++i)
                    {
                        if (i != args.begin()) {
                            o << ", ";
                        }
                        dumpType(o, *i);
                    }
                    o << " >";
                }
                break;
            }
        default:
            OSL_ASSERT(false);
            break;
    }

    {for (sal_Int32 i = 0; i < seqNum; ++i) {
        o << " >";
    }}

    if (bRef) o << "&";
}

void CppuType::dumpCppuGetType(FileStream& o, const OString& type, sal_Bool bDecl, CppuTypeDecl eDeclFlag)
{
    rtl::OString relType(
        codemaker::UnoType::decompose(checkRealBaseType(type, true)));

    if (eDeclFlag == CPPUTYPEDECL_ONLYINTERFACES)
    {
         if (m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
        {
            o << indent() << "::cppu::UnoType< ";
            dumpType(o, type, false, false, false, true);
            o << " >::get()";

            if (bDecl)
                o << ";\n";
        }
    } else
    {
        if (codemaker::UnoType::getSort(type)
            != codemaker::UnoType::SORT_COMPLEX)
        {
            return;
        } else
        {
            if (eDeclFlag == CPPUTYPEDECL_NOINTERFACES &&
                m_typeMgr.getTypeClass(relType) == RT_TYPE_INTERFACE)
                return;

//          if (m_typeMgr.getTypeClass(type) == RT_TYPE_TYPEDEF)
//          {
//              o << indent() << "get_" << type.replace('/', '_') << "_Type()";
//          } else
//          {
                o << indent() << "::cppu::UnoType< ";
                dumpType(o, type, false, false, false, true);
                o << " >::get()";
//          }
        }
        if (bDecl)
            o << ";\n";
    }
}

OString CppuType::typeToIdentifier(const OString& type)
{
    sal_Int32 seqNum;
    rtl::OString relType(codemaker::UnoType::decompose(type, &seqNum));
    OString sIdentifier;

    while( seqNum > 0 )
    {
        sIdentifier += OString("seq");

        if ( --seqNum == 0 )
        {
            sIdentifier += OString("_");
        }
    }

    sIdentifier += relType.replace(
        ((codemaker::UnoType::getSort(relType)
          == codemaker::UnoType::SORT_COMPLEX)
         ? '/' : ' '),
        '_');

    // TODO: find better solution to create an identifier
    sIdentifier = sIdentifier.replace('<', '_');
    sIdentifier = sIdentifier.replace('>', '_');
    sIdentifier = sIdentifier.replace(',', '_');

    return sIdentifier;
}

bool CppuType::passByReference(rtl::OString const & unoType) {
    rtl::OString type(resolveTypedefs(unoType));
    switch (codemaker::UnoType::getSort(type)) {
    default:
        return false;

    case codemaker::UnoType::SORT_STRING:
    case codemaker::UnoType::SORT_TYPE:
    case codemaker::UnoType::SORT_ANY:
        return true;

    case codemaker::UnoType::SORT_COMPLEX:
        return m_typeMgr.getTypeClass(type) != RT_TYPE_ENUM;
    }
}

OString CppuType::resolveTypedefs(const OString& type) const
{
    OString baseType(type);

    RegistryKey     key;
    RTTypeClass     typeClass;
    sal_Bool        isTypeDef = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    typereg::Reader reader;

    while (isTypeDef)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
                baseType = rtl::OUStringToOString(
                    reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
            else
                isTypeDef = sal_False;
        } else
        {
            break;
        }
    }

    return baseType;
}

OString CppuType::checkRealBaseType(const OString& type, sal_Bool bResolveTypeOnly) const
{
    sal_Int32 rank;
    rtl::OString baseType(codemaker::UnoType::decompose(type, &rank));

    RegistryKey     key;
    RTTypeClass     typeClass;
    sal_Bool        mustBeChecked = (m_typeMgr.getTypeClass(baseType) == RT_TYPE_TYPEDEF);
    typereg::Reader reader;

    while (mustBeChecked)
    {
        reader = m_typeMgr.getTypeReader(baseType);

        if (reader.isValid())
        {
            typeClass = reader.getTypeClass();

            if (typeClass == RT_TYPE_TYPEDEF)
            {
                sal_Int32 n;
                baseType = codemaker::UnoType::decompose(
                    rtl::OUStringToOString(
                        reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8),
                    &n);
                OSL_ASSERT(n <= SAL_MAX_INT32 - rank); //TODO
                rank += n;
            } else
                mustBeChecked = sal_False;
        } else
        {
            break;
        }
    }

    if (bResolveTypeOnly) {
        rtl::OStringBuffer buf;
        for (sal_Int32 i = 0; i < rank; ++i) {
            buf.append(RTL_CONSTASCII_STRINGPARAM("[]"));
        }
        baseType = buf.makeStringAndClear() + baseType;
    }

    return baseType;
}

void CppuType::dumpConstantValue(FileStream& o, sal_uInt16 index)
{
    RTConstValue constValue = m_reader.getFieldValue(index);

    switch (constValue.m_type)
    {
        case RT_TYPE_NONE:
            break;
        case RT_TYPE_BOOL:
            if (constValue.m_value.aBool)
                o << "sal_True";
            else
                o << "sal_False";
            break;
        case RT_TYPE_BYTE:
            o << "(sal_Int8)" << constValue.m_value.aByte;
            break;
        case RT_TYPE_INT16:
            o << "(sal_Int16)" << constValue.m_value.aShort;
            break;
        case RT_TYPE_UINT16:
            o << "(sal_uInt16)" << constValue.m_value.aUShort;
            break;
        case RT_TYPE_INT32:
            // Avoid C++ compiler warnings about (un)signedness of literal
            // -2^31:
            if (constValue.m_value.aLong == SAL_MIN_INT32) {
                o << "SAL_MIN_INT32";
            } else {
                o << "(sal_Int32)" << constValue.m_value.aLong;
            }
            break;
        case RT_TYPE_UINT32:
            o << "(sal_uInt32)"
              << OString::valueOf(
                  static_cast< sal_Int64 >(constValue.m_value.aULong)).getStr()
              << "U";
            break;
        case RT_TYPE_INT64:
            // Avoid C++ compiler warnings about (un)signedness of literal
            // -2^63:
            if (constValue.m_value.aHyper == SAL_MIN_INT64) {
                o << "SAL_MIN_INT64";
            } else {
                ::rtl::OString tmp(OString::valueOf(constValue.m_value.aHyper));
                o << "(sal_Int64) SAL_CONST_INT64(" << tmp.getStr() << ")";
            }
            break;
        case RT_TYPE_UINT64:
            {
                o << "SAL_CONST_UINT64(";
                sal_uInt64 n = constValue.m_value.aUHyper;
                if (n == 0) {
                    o << "0";
                } else {
                    std::vector< char > buf;
                    for (; n != 0; n /= 10) {
                        buf.push_back('0' + static_cast< char >(n % 10));
                    }
                    for (std::vector< char >::reverse_iterator i(buf.rbegin());
                         i != buf.rend(); ++i)
                    {
                        o << rtl::OString::valueOf(*i).getStr();
                    }
                }
                o << ")";
            }
            break;
        case RT_TYPE_FLOAT:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aFloat) );
                o << "(float)" << tmp.getStr();
            }
            break;
        case RT_TYPE_DOUBLE:
            {
                ::rtl::OString tmp( OString::valueOf(constValue.m_value.aDouble) );
                o << "(double)" << tmp.getStr();
            }
            break;
        case RT_TYPE_STRING:
            {
                ::rtl::OUString aUStr(constValue.m_value.aString);
                ::rtl::OString aStr = ::rtl::OUStringToOString(aUStr, RTL_TEXTENCODING_ASCII_US);
                o << "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\"" << aStr.getStr() << "\"))";
            }
            break;
    }
}

void CppuType::inc(sal_Int32 num)
{
    m_indentLength += num;
}

void CppuType::dec(sal_Int32 num)
{
    m_indentLength = std::max< sal_Int32 >(m_indentLength - num, 0);
}

OString CppuType::indent() const
{
    OStringBuffer tmp(m_indentLength);

    for (sal_Int32 i=0; i < m_indentLength; i++)
    {
        tmp.append(' ');
    }
    return tmp.makeStringAndClear();
}

//*************************************************************************
// InterfaceType
//*************************************************************************
InterfaceType::InterfaceType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
    m_inheritedMemberCount = 0;
    m_hasAttributes = sal_False;
    m_hasMethods = sal_False;
}

InterfaceType::~InterfaceType()
{

}

sal_Bool InterfaceType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    if (m_reader.getMethodCount() != 0) {
        includes.add("com/sun/star/uno/RuntimeException");
    }
    dumpHFileContent(o, includes);
    return sal_True;
}

void InterfaceType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
//     rtl::OString cppName(translateUnoToCppIdentifier(
//                              m_name, "interface", ITM_KEYWORDSONLY, &m_name));

//  o << "\nclass SAL_NO_VTABLE " << cppName;
    o << "\nclass SAL_NO_VTABLE " << m_name;

    for (sal_Int16 i = 0; i < m_reader.getSuperTypeCount(); ++i) {
        o << (i == 0 ? " :" : ",") << " public "
          << scopedCppName(rtl::OUStringToOString(
                            m_reader.getSuperTypeName(i), RTL_TEXTENCODING_UTF8));
    }

    o << "\n{\npublic:\n";

    inc();

    dumpAttributes(o);
    dumpMethods(o);

    o << "\n" << indent()
      << ("static inline ::com::sun::star::uno::Type const & SAL_CALL"
          " static_type(void * = 0);\n\n");
    dec();
    o << "protected:\n";
    inc();
    o << indent() << "~" << m_name
      << ("() throw () {} // avoid warnings about virtual members and"
          " non-virtual dtor\n");
    dec();
    o << "};\n\n";
}

sal_Bool InterfaceType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &m_typeName);
    o << "\n";

    dumpGetCppuType(o);

//     rtl::OString cppName(translateUnoToCppIdentifier(
//                              m_name, "interface", ITM_KEYWORDSONLY, &m_name));

    o << "\n::com::sun::star::uno::Type const & "
      << scopedCppName(m_typeName)
      << "::static_type(SAL_UNUSED_PARAMETER void *) {\n";
    inc();
    o << indent() << "return ::getCppuType(static_cast< ";
    dumpType(o, m_typeName);
    o << " * >(0));\n";
    dec();
    o << "}\n";

    o << "\n#endif // "<< headerDefine << "\n";
    return sal_True;
}

void InterfaceType::dumpAttributes(FileStream& o)
{
    sal_uInt16 fieldCount = m_reader.getFieldCount();
    sal_Bool first=sal_True;

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        rtl::OUString name(m_reader.getFieldName(i));
        fieldName = rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "// Attributes\n";
        }

        o << indent() << "virtual ";
        dumpType(o, fieldType);
        o << " SAL_CALL get" << fieldName << "()";
        dumpAttributeExceptionSpecification(o, name, RT_MODE_ATTRIBUTE_GET);
        o << " = 0;\n";

        if ((access & RT_ACCESS_READONLY) == 0)
        {
            bool byRef = passByReference(fieldType);
            o << indent() << "virtual void SAL_CALL set" << fieldName << "( ";
            dumpType(o, fieldType, byRef, byRef);
            o << " _" << fieldName.toAsciiLowerCase() << " )";
            dumpAttributeExceptionSpecification(o, name, RT_MODE_ATTRIBUTE_SET);
            o << " = 0;\n";
        }
    }
}

void InterfaceType::dumpMethods(FileStream& o)
{
    sal_uInt16 methodCount = m_reader.getMethodCount();
    sal_Bool first=sal_True;

    OString methodName, returnType, paramType, paramName;
    sal_uInt16 paramCount = 0;
    RTMethodMode methodMode = RT_MODE_INVALID;
    RTParamMode  paramMode = RT_PARAM_INVALID;

    sal_Bool bRef = sal_False;
    sal_Bool bConst = sal_False;
    sal_Bool bWithRunTimeExcp = sal_True;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        methodMode = m_reader.getMethodFlags(i);
        if (methodMode == RT_MODE_ATTRIBUTE_GET
            || methodMode == RT_MODE_ATTRIBUTE_SET)
        {
            continue;
        }

        methodName = rtl::OUStringToOString(
            m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8);
        returnType = rtl::OUStringToOString(
            m_reader.getMethodReturnTypeName(i), RTL_TEXTENCODING_UTF8);
        paramCount = m_reader.getMethodParameterCount(i);

        if ( methodName.equals("acquire") || methodName.equals("release") )
        {
            bWithRunTimeExcp = sal_False;
        }

        if (first)
        {
            first = sal_False;
            o << "\n" << indent() << "// Methods\n";
        }

        o << indent() << "virtual ";
        dumpType(o, returnType);
        o << " SAL_CALL " << methodName << "( ";
        for (sal_uInt16 j=0; j < paramCount; j++)
        {
            paramName = rtl::OUStringToOString(
                m_reader.getMethodParameterName(i, j), RTL_TEXTENCODING_UTF8);
            paramType = rtl::OUStringToOString(
                m_reader.getMethodParameterTypeName(i, j),
                RTL_TEXTENCODING_UTF8);
            paramMode = m_reader.getMethodParameterFlags(i, j);

            switch (paramMode)
            {
                case RT_PARAM_IN:
                    bConst = passByReference(paramType);
                    bRef = bConst;
                    break;
                case RT_PARAM_OUT:
                case RT_PARAM_INOUT:
                    bConst = sal_False;
                    bRef = sal_True;
                    break;
                default:
                    break;
            }

            dumpType(o, paramType, bConst, bRef);
//          o << " " << translateUnoToCppIdentifier(
//                 paramName, "param", ITM_KEYWORDSONLY, NULL);
            o << " " << paramName;

            if (j+1 < (sal_uInt16)paramCount) o << ", ";
        }
        o << " )";
        dumpExceptionSpecification(o, i, bWithRunTimeExcp);
        o << " = 0;\n";
    }
}

void InterfaceType::dumpNormalGetCppuType(FileStream& o)
{
    dumpGetCppuTypePreamble(o);

    o << indent()
      << "static typelib_TypeDescriptionReference * the_type = 0;\n";

    o << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();
    sal_Int16 nBases = m_reader.getSuperTypeCount();
    OSL_ASSERT(nBases > 0);
    if (nBases == 1 && m_reader.getSuperTypeName(0) == "com/sun/star/uno/XInterface" )
    {
        nBases = 0;
    }
    if (nBases > 0) {
        o << indent() << "typelib_TypeDescriptionReference * aSuperTypes["
          << nBases << "];\n";
        for (sal_Int16 i = 0; i < nBases; ++i) {
            o << indent() << "aSuperTypes[" << i << "] = ::cppu::UnoType< ";
            dumpType(
                o,
                rtl::OUStringToOString(
                    m_reader.getSuperTypeName(i), RTL_TEXTENCODING_UTF8),
                true, false, false, true);
            o << " >::get().getTypeLibType();\n";
        }
    }

    o << indent() << "typelib_static_mi_interface_type_init( &the_type, \""
      << m_typeName.replace('/', '.') << "\", " << nBases << ", ";

    if ( nBases > 0 )
    {
        o << "aSuperTypes );\n";
    } else
    {
        o << "0 );\n";
    }

    dec();
    o << indent() << "}\n";
    o << indent()
      << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
          " &the_type );\n");

    dumpGetCppuTypePostamble(o);
}

void InterfaceType::dumpComprehensiveGetCppuType(FileStream& o)
{
    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false);
    else
        o << "namespace cppu { ";
    o << " namespace detail {\n\n";

    OString sType = m_typeName.copy(m_typeName.lastIndexOf('/') + 1);
    OString sStaticTypeClass = "the" + sType + "Type";
    o << indent() << "struct " << sStaticTypeClass << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, " << sStaticTypeClass << " >\n";
    o << indent() << "{\n";
    inc();
    o << indent() << "::com::sun::star::uno::Type * operator()() const\n";
    o << indent() << "{\n";

    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    o << indent() << "// Start inline typedescription generation\n"
      << indent() << "typelib_InterfaceTypeDescription * pTD = 0;\n\n";

    OSL_ASSERT(m_reader.getSuperTypeCount() > 0);
    o << indent() << "typelib_TypeDescriptionReference * aSuperTypes["
      << m_reader.getSuperTypeCount() << "];\n";
    for (sal_Int16 i = 0; i < m_reader.getSuperTypeCount(); ++i) {
        o << indent() << "aSuperTypes[" << i << "] = ::cppu::UnoType< ";
        dumpType(
            o,
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(i), RTL_TEXTENCODING_UTF8),
            false, false, false, true);
        o << " >::get().getTypeLibType();\n";
    }

    sal_uInt32 count = getMemberCount();
    if (count)
    {
        o << indent() << "typelib_TypeDescriptionReference * pMembers[" << count
          << "] = { ";
        for (sal_uInt32 i = 0; i < count; i++)
        {
            o << "0";
            if (i+1 < count) {
                o << ",";
            } else {
                o << " };\n";
            }
        }

        sal_uInt32 index = 0;
        dumpCppuAttributeRefs(o, index);
        dumpCppuMethodRefs(o, index);
    }

    o << "\n" << indent() << "typelib_typedescription_newMIInterface(\n";
    inc();
    o << indent() << "&pTD,\n" << indent() << "sTypeName.pData, ";

    o << "0x00000000, 0x0000, 0x0000, 0x00000000, 0x00000000,\n"; // UIK

    o << indent() << m_reader.getSuperTypeCount() << ", aSuperTypes,\n";

    if ( count ) {
        o << indent() << count << ",\n" << indent() << "pMembers );\n\n";
    } else {
        o << indent() << count << ",\n" << indent() << "0 );\n\n";
    }
    dec();

    o << indent()
      << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
          " );\n");
    if ( count )
    {
        for (sal_uInt16 i=0; i < count; i++)
        {
            o << indent()
              << "typelib_typedescriptionreference_release( pMembers[" << i
              << "] );\n";
        }
    }
    o << indent()
      << ("typelib_typedescription_release( (typelib_TypeDescription*)pTD"
          " );\n\n");

    o << indent() << "return new ::com::sun::star::uno::Type( "
      << getTypeClass(m_typeName) << ", sTypeName ); // leaked\n";

    dec();

    o << indent() << "}\n";
    dec();
    o << indent() << "};\n\n";

    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false);
    else
        o << " }";
    o << " }\n\n";

    dumpGetCppuTypePreamble(o);
    o  << indent() << "const ::com::sun::star::uno::Type &rRet = *detail::" << sStaticTypeClass << "::get();\n";

    o << indent() << "// End inline typedescription generation\n";

    o  << indent() << "static bool bInitStarted = false;\n";
    o  << indent() << "if (!bInitStarted)\n";
    o  << indent() << "{\n";
    inc();
    o  << indent() << "::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );\n";
    o  << indent() << "if (!bInitStarted)\n";
    o  << indent() << "{\n";
    inc();
    o  << indent() << "OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();\n";
    o  << indent() << "bInitStarted = true;\n";

    StringSet   aTypes;
    // type for RuntimeException is always needed
    OString     sRunTimeExceptionType("com/sun/star/uno/RuntimeException");
    aTypes.insert(sRunTimeExceptionType);
    dumpCppuGetType(o, sRunTimeExceptionType, sal_True, CPPUTYPEDECL_ALLTYPES);

    dumpAttributesCppuDecl(o, &aTypes, CPPUTYPEDECL_ALLTYPES);
    dumpMethodsCppuDecl(o, &aTypes, CPPUTYPEDECL_ALLTYPES);

    if (count)
    {
        sal_uInt32 index = getInheritedMemberCount();
        dumpCppuAttributes(o, index);
        dumpCppuMethods(o, index);
    }

    dec();
    o  << indent() << "}\n";
    dec();
    o  << indent() << "}\n";
    o  << indent() << "else\n";
    o  << indent() << "{\n";
    inc();
    o  << indent() << "OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();\n";
    dec();
    o  << indent() << "}\n";

    o  << indent() << "return rRet;\n";
    dumpGetCppuTypePostamble(o);
}

void InterfaceType::dumpCppuAttributeRefs(FileStream& o, sal_uInt32& index)
{
    sal_uInt16 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString scope = m_typeName.replace('/', '.');

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

        o << indent() << "::rtl::OUString sAttributeName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
          << scope.replace('/', '.') << "::" << fieldName << "\") );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "(typelib_TypeClass)::com::sun::star::uno::TypeClass_INTERFACE_ATTRIBUTE,\n"
          << indent() << "sAttributeName" << i << ".pData );\n";
        dec(38);
        index++;
    }
}

void InterfaceType::dumpCppuMethodRefs(FileStream& o, sal_uInt32& index)
{
    sal_uInt16  methodCount = m_reader.getMethodCount();
    OString     methodName;
    OString     scope = m_typeName.replace('/', '.');

    for (sal_uInt16 i = 0; i < methodCount; i++)
    {
        RTMethodMode flags = m_reader.getMethodFlags(i);
        if (flags == RT_MODE_ATTRIBUTE_GET || flags == RT_MODE_ATTRIBUTE_SET) {
            continue;
        }

        methodName = rtl::OUStringToOString(
            m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8);

        o << indent() << "::rtl::OUString sMethodName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
          << scope.replace('/', '.') << "::" << methodName << "\") );\n";
        o << indent() << "typelib_typedescriptionreference_new( &pMembers["
          << index << "],\n";
        inc(38);
        o << indent() << "(typelib_TypeClass)::com::sun::star::uno::TypeClass_INTERFACE_METHOD,\n"
          << indent() << "sMethodName" << i << ".pData );\n";
        dec(38);
        index++;
    }
}

sal_uInt32 InterfaceType::getMemberCount() {
    sal_uInt16 count = 0;
    sal_uInt16 methodCount = m_reader.getMethodCount();
    {for (sal_uInt16 i = 0; i < methodCount; ++i) {
        RTMethodMode flags = m_reader.getMethodFlags(i);
        if (flags != RT_MODE_ATTRIBUTE_GET && flags != RT_MODE_ATTRIBUTE_SET) {
            m_hasMethods = true;
            ++count;
        }
    }}
    sal_uInt16 fieldCount = m_reader.getFieldCount();
    {for (sal_uInt16 i = 0; i < fieldCount; ++i) {
        RTFieldAccess flags = m_reader.getFieldFlags(i);
        if (flags != RT_ACCESS_CONST && flags != RT_ACCESS_INVALID) {
            m_hasAttributes = true;
            ++count;
        }
    }}
    return count;
}

namespace {

class BaseOffset {
public:
    BaseOffset(TypeManager const & theManager, typereg::Reader const & reader);

    sal_Int32 get() const { return offset; }

private:
    void calculateBases(typereg::Reader const & reader);

    void calculate(typereg::Reader const & reader);

    TypeManager const & manager;
    std::set< rtl::OString > set;
    sal_Int32 offset;
};

BaseOffset::BaseOffset(
    TypeManager const & theManager, typereg::Reader const & reader):
    manager(theManager)
{
    offset = 0;
    calculateBases(reader);
}

void BaseOffset::calculateBases(typereg::Reader const & reader) {
    for (sal_Int16 i = 0; i < reader.getSuperTypeCount(); ++i) {
        typereg::Reader super(
            manager.getTypeReader(
                rtl::OUStringToOString(
                    reader.getSuperTypeName(i), RTL_TEXTENCODING_UTF8)));
        if (super.isValid()) {
            calculate(super);
        }
    }
}

void BaseOffset::calculate(typereg::Reader const & reader) {
    if (set.insert(
            rtl::OUStringToOString(reader.getTypeName(), RTL_TEXTENCODING_UTF8))
        .second)
    {
        calculateBases(reader);
        {for (sal_uInt16 i = 0; i < reader.getMethodCount(); ++i) {
            RTMethodMode flags = reader.getMethodFlags(i);
            if (flags != RT_MODE_ATTRIBUTE_GET
                && flags != RT_MODE_ATTRIBUTE_SET)
            {
                ++offset;
            }
        }}
        {for (sal_uInt16 i = 0; i < reader.getFieldCount(); ++i) {
            RTFieldAccess flags = reader.getFieldFlags(i);
            if (flags != RT_ACCESS_CONST && flags != RT_ACCESS_INVALID) {
                ++offset;
            }
        }}
    }
}

}

void InterfaceType::addSpecialDependencies() {
    if (m_reader.getMethodCount() > 0 || m_reader.getFieldCount() > 0) {
        m_dependencies.add("com/sun/star/uno/RuntimeException");
    }
}

void InterfaceType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    // The comprehensive getCppuType method always includes a line
    // "getCppuType( (const ::com::sun::star::uno::RuntimeException*)0 );":
    includes.addCppuUnotypeHxx();
    includes.add("com/sun/star/uno/RuntimeException");
}

sal_uInt32 InterfaceType::checkInheritedMemberCount(const typereg::Reader*)
{
    return BaseOffset(m_typeMgr, m_reader).get();
}

sal_uInt32 InterfaceType::getInheritedMemberCount()
{
    if (m_inheritedMemberCount == 0)
    {
        m_inheritedMemberCount = checkInheritedMemberCount(0);
    }

    return m_inheritedMemberCount;
}

void InterfaceType::dumpCppuAttributes(FileStream& o, sal_uInt32& index)
{
    sal_uInt16 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    OString scope = m_typeName.replace('/', '.');

    sal_uInt32 absoluteIndex = index;

    if (m_hasAttributes)
    {
        o << "\n" << indent() << "typelib_InterfaceAttributeTypeDescription * pAttribute = 0;\n";

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            rtl::OUString name(m_reader.getFieldName(i));
            fieldName = rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            fieldType = checkRealBaseType(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8),
                sal_True);

            o << indent() << "{\n";
            inc();
            o << indent() << "::rtl::OUString sAttributeType" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << fieldType.replace('/', '.') << "\") );\n";

            o << indent() << "::rtl::OUString sAttributeName" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << scope.replace('/', '.') << "::" << fieldName << "\") );\n";

            sal_Int32 getExceptions = dumpAttributeExceptionTypeNames(
                o, "get", name, RT_MODE_ATTRIBUTE_GET);
            sal_Int32 setExceptions = dumpAttributeExceptionTypeNames(
                o, "set", name, RT_MODE_ATTRIBUTE_SET);
            o << indent()
              << ("typelib_typedescription_newExtendedInterfaceAttribute("
                  " &pAttribute,\n");
            inc();
            o << indent() << absoluteIndex++ << ", sAttributeName" << i << ".pData,\n";
            o << indent() << "(typelib_TypeClass)" << getTypeClass(fieldType)
              << ", sAttributeType" << i << ".pData,\n";
            o << indent() << "sal_"
              << ((access & RT_ACCESS_READONLY) == 0 ? "False" : "True") << ", "
              << getExceptions << ", "
              << (getExceptions == 0 ? "0" : "the_getExceptions") << ", "
              << setExceptions << ", "
              << (setExceptions == 0 ? "0" : "the_setExceptions") << " );\n";
            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pAttribute );\n";
            dec();
            o << indent() << "}\n";
        }
         o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pAttribute );\n";

        index = absoluteIndex;
    }
}

void InterfaceType::dumpCppuMethods(FileStream& o, sal_uInt32& index)
{
    sal_uInt16      methodCount = m_reader.getMethodCount();
    OString         methodName, returnType, paramType, paramName;
    RTMethodMode    methodMode = RT_MODE_INVALID;
    RTParamMode     paramMode = RT_PARAM_INVALID;
    sal_Bool        bWithRuntimeException = sal_True;

    sal_uInt32 absoluteIndex = index;

    if (m_hasMethods)
    {
        o << "\n" << indent() << "typelib_InterfaceMethodTypeDescription * pMethod = 0;\n";
        OString             scope = m_typeName.replace('/', '.');

        sal_uInt16 paramCount = 0;
        for (sal_uInt16 i=0; i < methodCount; i++)
        {
            methodMode = m_reader.getMethodFlags(i);
            if (methodMode == RT_MODE_ATTRIBUTE_GET
                || methodMode == RT_MODE_ATTRIBUTE_SET)
            {
                continue;
            }

            methodName = rtl::OUStringToOString(
                m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8);
            returnType = checkRealBaseType(
                rtl::OUStringToOString(
                    m_reader.getMethodReturnTypeName(i), RTL_TEXTENCODING_UTF8),
                sal_True);
            paramCount = m_reader.getMethodParameterCount(i);

            if ( methodName.equals("acquire") || methodName.equals("release") )
            {
                bWithRuntimeException = sal_False;
            }

            o << indent() << "{\n";
            inc();

            if (paramCount)
            {
                o << indent() << "typelib_Parameter_Init aParameters[" << paramCount << "];\n";
            }

            for (sal_uInt16 j = 0; j < paramCount; j++)
            {
                paramName = rtl::OUStringToOString(
                    m_reader.getMethodParameterName(i, j),
                    RTL_TEXTENCODING_UTF8);
                paramType = checkRealBaseType(
                    rtl::OUStringToOString(
                        m_reader.getMethodParameterTypeName(i, j),
                        RTL_TEXTENCODING_UTF8),
                    sal_True);
                paramMode = m_reader.getMethodParameterFlags(i, j);

                o << indent() << "::rtl::OUString sParamName" << j << "( RTL_CONSTASCII_USTRINGPARAM(\""
                  << paramName << "\") );\n";
                o << indent() << "::rtl::OUString sParamType" << j << "( RTL_CONSTASCII_USTRINGPARAM(\""
                  << paramType.replace('/', '.') << "\") );\n";
                o << indent() << "aParameters[" << j << "].pParamName = sParamName" << j << ".pData;\n";
                o << indent() << "aParameters[" << j << "].eTypeClass = (typelib_TypeClass)"
                  << getTypeClass(paramType) << ";\n";
                o << indent() << "aParameters[" << j << "].pTypeName = sParamType" << j << ".pData;\n";

                if (paramMode == RT_PARAM_IN || paramMode == RT_PARAM_INOUT)
                    o << indent() << "aParameters[" << j << "].bIn = sal_True;\n";
                else
                    o << indent() << "aParameters[" << j << "].bIn = sal_False;\n";

                if (paramMode == RT_PARAM_OUT || paramMode == RT_PARAM_INOUT)
                    o << indent() << "aParameters[" << j << "].bOut = sal_True;\n";
                else
                    o << indent() << "aParameters[" << j << "].bOut = sal_False;\n";
            }

            sal_Int32 excCount = dumpExceptionTypeNames(
                o, "", i, bWithRuntimeException);

            o << indent() << "::rtl::OUString sReturnType" << i << "( RTL_CONSTASCII_USTRINGPARAM(\""
              << returnType.replace('/', '.') << "\") );\n";
            o << indent() << "::rtl::OUString sMethodName" << i <<
                "( RTL_CONSTASCII_USTRINGPARAM(\""
              << scope.replace('/', '.') << "::" << methodName << "\") );\n";
            o << indent() << "typelib_typedescription_newInterfaceMethod( &pMethod,\n";
            inc();
            o << indent() << absoluteIndex++ << ", ";
            if (methodMode == RT_MODE_ONEWAY || methodMode == RT_MODE_ONEWAY_CONST)
                o << "sal_True,\n";
            else
                o << "sal_False,\n";

            o << indent() << "sMethodName" << i << ".pData,\n";
            o << indent() << "(typelib_TypeClass)" << getTypeClass(returnType)
              << ", sReturnType" << i << ".pData,\n";
            if (paramCount)
                o << indent() << paramCount << ", aParameters,\n";
            else
                o << indent() << "0, 0,\n";
            o << indent() << excCount << ", "
              << (excCount == 0 ? "0" : "the_Exceptions") << " );\n";

            dec();
            o << indent() << "typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );\n";

            dec();
            o << indent() << "}\n";
        }
        o << indent() << "typelib_typedescription_release( (typelib_TypeDescription*)pMethod );\n";

        index = absoluteIndex;
    }
}

void InterfaceType::dumpAttributesCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag)
{
    sal_uInt16 fieldCount = m_reader.getFieldCount();

    RTFieldAccess access = RT_ACCESS_INVALID;
    OString fieldName;
    OString fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        if (pFinishedTypes->count(fieldType) == 0)
        {
            pFinishedTypes->insert(fieldType);
            dumpCppuGetType(o, fieldType, sal_True, eDeclFlag);
        }
    }
}

void InterfaceType::dumpMethodsCppuDecl(FileStream& o, StringSet* pFinishedTypes, CppuTypeDecl eDeclFlag)
{
    sal_uInt16      methodCount = m_reader.getMethodCount();
    OString         returnType, paramType, excType;
    sal_uInt16      paramCount = 0;
    sal_uInt16      excCount = 0;

    for (sal_uInt16 i=0; i < methodCount; i++)
    {
        returnType = rtl::OUStringToOString(
            m_reader.getMethodReturnTypeName(i), RTL_TEXTENCODING_UTF8);
        paramCount = m_reader.getMethodParameterCount(i);
        excCount = m_reader.getMethodExceptionCount(i);

        if (pFinishedTypes->count(returnType) == 0)
        {
            pFinishedTypes->insert(returnType);
            dumpCppuGetType(o, returnType, sal_True, eDeclFlag);
        }
        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramType = rtl::OUStringToOString(
                m_reader.getMethodParameterTypeName(i, j),
                RTL_TEXTENCODING_UTF8);

            if (pFinishedTypes->count(paramType) == 0)
            {
                pFinishedTypes->insert(paramType);
                dumpCppuGetType(o, paramType, sal_True, eDeclFlag);
            }
        }

        for (j=0; j < excCount; j++)
        {
            excType = rtl::OUStringToOString(
                m_reader.getMethodExceptionTypeName(i, j),
                RTL_TEXTENCODING_UTF8);
            if (pFinishedTypes->count(excType) == 0)
            {
                pFinishedTypes->insert(excType);
                dumpCppuGetType(o, excType, sal_True, eDeclFlag);
            }
        }
    }
}

void InterfaceType::dumpExceptionSpecification(
    FileStream & out, sal_uInt32 methodIndex, bool runtimeException)
{
    out << " throw (";
    bool first = true;
    if (methodIndex <= SAL_MAX_UINT16) {
        sal_uInt16 count = m_reader.getMethodExceptionCount(
            static_cast< sal_uInt16 >(methodIndex));
        for (sal_uInt16 i = 0; i < count; ++i) {
            rtl::OUString name(
                m_reader.getMethodExceptionTypeName(
                    static_cast< sal_uInt16 >(methodIndex), i));
            if ( name != "com/sun/star/uno/RuntimeException" )
            {
                if (!first) {
                    out << ", ";
                }
                first = false;
                out << scopedCppName(
                    rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8));
            }
        }
    }
    if (runtimeException) {
        if (!first) {
            out << ", ";
        }
        out << "::com::sun::star::uno::RuntimeException";
    }
    out << ")";
}

void InterfaceType::dumpAttributeExceptionSpecification(
    FileStream & out, rtl::OUString const & name, RTMethodMode sort)
{
    sal_uInt16 methodCount = m_reader.getMethodCount();
    for (sal_uInt16 i = 0; i < methodCount; ++i) {
        if (m_reader.getMethodFlags(i) == sort
            && m_reader.getMethodName(i) == name)
        {
            dumpExceptionSpecification(out, i, true);
            return;
        }
    }
    dumpExceptionSpecification(out, 0xFFFFFFFF, true);
}

void InterfaceType::dumpExceptionTypeName(
    FileStream & out, char const * prefix, sal_uInt32 index, rtl::OUString name)
{
    out << indent() << "::rtl::OUString the_" << prefix << "ExceptionName"
        << index << "(RTL_CONSTASCII_USTRINGPARAM(\""
        << rtl::OUStringToOString(name, RTL_TEXTENCODING_UTF8).replace('/', '.')
        << "\"));\n";
}

sal_Int32 InterfaceType::dumpExceptionTypeNames(
    FileStream & out, char const * prefix, sal_uInt16 methodIndex,
    bool runtimeException)
{
    sal_Int32 count = 0;
    sal_uInt16 n = m_reader.getMethodExceptionCount(methodIndex);
    for (sal_uInt16 i = 0; i < n; ++i) {
        rtl::OUString name(m_reader.getMethodExceptionTypeName(methodIndex, i));
        if ( name != "com/sun/star/uno/RuntimeException" )
        {
            dumpExceptionTypeName(out, prefix, count++, name);
        }
    }
    if (runtimeException) {
        dumpExceptionTypeName(
            out, prefix, count++,
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "com/sun/star/uno/RuntimeException")));
    }
    if (count > 0) {
        out << indent() << "rtl_uString * the_" << prefix << "Exceptions[] = {";
        for (sal_Int32 i = 0; i < count; ++i) {
            out << (i == 0 ? " " : ", ") << "the_" << prefix << "ExceptionName"
                << i << ".pData";
        }
        out << " };\n";
    }
    return count;
}

sal_Int32 InterfaceType::dumpAttributeExceptionTypeNames(
    FileStream & out, char const * prefix, rtl::OUString const & name,
    RTMethodMode sort)
{
    sal_uInt16 methodCount = m_reader.getMethodCount();
    for (sal_uInt16 i = 0; i < methodCount; ++i) {
        if (m_reader.getMethodFlags(i) == sort
            && m_reader.getMethodName(i) == name)
        {
            return dumpExceptionTypeNames(out, prefix, i, false);
        }
    }
    return 0;
}

//*************************************************************************
// ConstantsType
//*************************************************************************
ConstantsType::ConstantsType(typereg::Reader& typeReader,
                             const OString& typeName,
                             const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
}

ConstantsType::~ConstantsType()
{

}

sal_Bool ConstantsType::dump(CppuOptions* pOptions)
    throw( CannotDumpException )
{
    if (!m_dependencies.isValid()) {
        return false;
    }
    addSpecialDependencies();

    if (pOptions->isValid("-U"))
        m_cppuTypeDynamic = sal_True;

    OString outPath;
    if (pOptions->isValid("-O"))
        outPath = pOptions->getOption("-O");

    return dumpFiles(pOptions, outPath);
}

sal_Bool ConstantsType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    sal_Bool bSpecialDefine = sal_True;

    if (m_reader.getTypeClass() == RT_TYPE_CONSTANTS)
    {
        bSpecialDefine = sal_False;
    }

    OString headerDefine(dumpHeaderDefine(o, "HDL", bSpecialDefine));
    o << "\n";

    addDefaultHIncludes(includes);
    includes.dump(o, 0);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, true)) {
        o << "\n";
    }
    o << "\n";

    dumpDeclaration(o);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, true)) {
        o << "\n";
    }
    o << "\n#endif // "<< headerDefine << "\n";

    return sal_True;
}

void ConstantsType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    sal_uInt16      fieldCount = m_reader.getFieldCount();
    OString         fieldName;
    OString         fieldType;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        o << "static const ";
        dumpType(o, fieldType);
        o << " " << fieldName << " = ";
        dumpConstantValue(o, i);
        o << ";\n";
    }
}

sal_Bool ConstantsType::hasConstants()
{
    return m_reader.getFieldCount() > 0;
}

sal_Bool ConstantsType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes &)
    throw( CannotDumpException )
{
    sal_Bool bSpecialDefine = sal_True;

    if (m_reader.getTypeClass() == RT_TYPE_CONSTANTS)
    {
        bSpecialDefine = sal_False;
    }

    OString headerDefine(dumpHeaderDefine(o, "HPP", bSpecialDefine));
    o << "\n";

    rtl::OString suffix;
    if (bSpecialDefine) {
        suffix = m_name;
    }
    codemaker::cppumaker::Includes::dumpInclude(o, m_typeName, false, suffix);

    o << "\n#endif // "<< headerDefine << "\n";

    return sal_True;
}

//*************************************************************************
// ModuleType
//*************************************************************************
ModuleType::ModuleType(typereg::Reader& typeReader,
                       const OString& typeName,
                       const TypeManager& typeMgr)
    : ConstantsType(typeReader, typeName, typeMgr)
{
}

ModuleType::~ModuleType()
{

}

bool ModuleType::dumpFiles(
    CppuOptions * options, rtl::OString const & outPath)
{
    rtl::OString tmpName(m_typeName);
    if (tmpName.equals("/")) {
        tmpName = "global";
    } else {
        tmpName += "/" + m_typeName.copy(m_typeName.lastIndexOf('/') + 1);
    }
    return dumpFile(options, ".hdl", tmpName, outPath)
        && dumpFile(options, ".hpp", tmpName, outPath);
}

//*************************************************************************
// StructureType
//*************************************************************************

namespace {

void dumpTypeParameterName(FileStream & out, rtl::OString const & name) {
    // Prefix all type parameters with "typeparam_" to avoid problems when a
    // struct member has the same name as a type parameter, as in
    // struct<T> { T T; };
    out << "typeparam_" << name;
}

}

StructureType::StructureType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
}

StructureType::~StructureType()
{

}

sal_Bool StructureType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    dumpHFileContent(o, includes);
    return sal_True;
}

void StructureType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\n#ifdef SAL_W32\n"
      << "#   pragma pack(push, 8)\n"
      << "#endif\n\n";

    OSL_ASSERT(!isPolymorphic() || m_reader.getSuperTypeCount() == 0); //TODO
    o << indent();
    dumpTemplateHead(o);
    o << "struct " << m_name;
    rtl::OString base;
    if (m_reader.getSuperTypeCount() != 0) {
        base = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
        OSL_ASSERT(!base.isEmpty()); //TODO
    }
    if (!base.isEmpty()) {
        o << ": public " << scopedCppName(base);
    }
    o << " {\n";
    inc();
    o << indent() << "inline " << m_name << "() SAL_THROW(());\n";
    sal_uInt16 members = m_reader.getFieldCount();
    if (members > 0 || getInheritedMemberCount() > 0) {
        o << "\n" << indent() << "inline " << m_name << "(";
        bool prev = dumpSuperMember(o, base, true);
        for (sal_uInt16 i = 0; i < members; ++i) {
            if (prev) {
                o << ", ";
            }
            prev = true;
            rtl::OString type(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
            if ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0)
            {
                dumpTypeParameterName(o, type);
                o << " const &";
            } else {
                dumpType(o, type, true, true);
            }
            o << " "
              << rtl::OUStringToOString(
                  m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8)
              << "_";
        }
        o << ") SAL_THROW(());\n";
    }
    if (members > 0) {
        o << "\n";
        for (sal_uInt16 i = 0; i < members; ++i) {
            o << indent();
            bool parameterized
                = ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE)
                   != 0);
            rtl::OString type(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
            if (parameterized) {
                dumpTypeParameterName(o, type);
            } else {
                dumpType(o, type);
            }
            o << " "
                << rtl::OUStringToOString(
                    m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            if (i == 0 && !base.isEmpty() && type != "double"
                && type != "hyper" && type != "unsigned hyper")
            {
                OSL_ASSERT(!parameterized);
                o << " CPPU_GCC3_ALIGN(" << scopedCppName(base) << ")";
            }
            o << ";\n";
        }
    }
    dec();
    o << "};\n\n";

    o << "#ifdef SAL_W32\n"
      << "#   pragma pack(pop)\n"
      << "#endif\n\n";
}

sal_Bool StructureType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    includes.dump(o, &m_typeName);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\n";

    dumpTemplateHead(o);
    o << "inline " << m_name;
    dumpTemplateParameters(o);
    o << "::" << m_name << "() SAL_THROW(())\n";
    inc();
    OString superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    sal_Bool first = sal_True;
    if (!superType.isEmpty())
    {
        o << indent() << ": " << scopedCppName(superType) << "()\n";
        first = sal_False;
    }

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

        if (first)
        {
            first = sal_False;
            o << indent() << ": ";
        } else
            o << indent() << ", ";

        o << fieldName;
        dumpInitializer(
            o, (access & RT_ACCESS_PARAMETERIZED_TYPE) != 0,
            m_reader.getFieldTypeName(i));
        o << "\n";
    }
    dec();
    o << "{\n}\n\n";

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        dumpTemplateHead(o);
        o << "inline " << m_name;
        dumpTemplateParameters(o);
        o << "::" << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            if ((access & RT_ACCESS_PARAMETERIZED_TYPE) != 0) {
                dumpTypeParameterName(o, fieldType);
                o << " const &";
            } else {
                dumpType(o, fieldType, sal_True, sal_True);
            }
//          o << " __" << fieldName;
            o << " " << fieldName << "_";
        }
        o << ") SAL_THROW(())\n";

        inc();
        first = sal_True;
        if (!superType.isEmpty())
        {
            o << indent() << ": " << scopedCppName(superType) << "(";
            dumpSuperMember(o, superType, sal_False);
            o << ")\n";
            first = sal_False;
        }

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

            if (first)
            {
                first = sal_False;
                o << indent() << ": ";
            } else
                o << indent() << ", ";

//          o << fieldName << "(__" << fieldName << ")\n";
            o << fieldName << "(" << fieldName << "_)\n";
        }

        dec();
        o << "{\n}\n\n";
    }

    if (isPolymorphic() && fieldCount > 0) {
        o << indent();
        dumpTemplateHead(o);
        o << "\n";
        o << indent();
        o << "inline " << m_name;
        dumpTemplateParameters(o);
        o << "\n";
        o << indent();
        o << "make_" << m_name << "(";
        for (sal_uInt16 i = 0; i < fieldCount; ++i) {
            if (i > 0) {
                o << ", ";
            }
            rtl::OString type(
                rtl::OUStringToOString(
                    m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
            if ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0)
            {
                dumpTypeParameterName(o, type);
                o << " const &";
            } else {
                dumpType(o, type, true, true);
            }
            o << " "
              << rtl::OUStringToOString(
                  m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8)
              << "_";
        }
        o << ") SAL_THROW(())\n";
        o << indent() << "{\n";
        inc();
        o << indent() << "return " << m_name;
        dumpTemplateParameters(o);
        o << "(";
        for (sal_uInt16 i = 0; i < fieldCount; ++i) {
            if (i > 0) {
                o << ", ";
            }
            o << rtl::OUStringToOString(
                  m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8)
              << "_";
        }
        o << ");\n";
        dec();
        o << indent() << "}\n\n";
    }

    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false)) {
        o << "\n";
    }

    o << "\n";
    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << "\n";

    return sal_True;
}

void StructureType::dumpLightGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    if (isPolymorphic()) {
        out << indent() << "::rtl::OStringBuffer the_buffer(\""
            << m_typeName.replace('/', '.') << "<\");\n";
        sal_uInt16 n = m_reader.getReferenceCount();
        for (sal_uInt16 i = 0; i < n; ++i) {
            out << indent()
                << ("the_buffer.append(::rtl::OUStringToOString("
                    "::cppu::getTypeFavourChar(static_cast< ");
            dumpTypeParameterName(
                out,
                rtl::OUStringToOString(
                    m_reader.getReferenceTypeName(i), RTL_TEXTENCODING_UTF8));
            out << " * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));\n";
            if (i != n - 1) {
                out << indent() << "the_buffer.append(',');\n";
            }
        }
        out << indent() << "the_buffer.append('>');\n";
    }
    out << indent() << "::typelib_static_type_init(&the_type, "
        << getTypeClass(m_typeName, true) << ", ";
    if (isPolymorphic()) {
        out << "the_buffer.getStr()";
    } else {
        out << "\"" << m_typeName.replace('/', '.') << "\"";
    }
    out << ");\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void StructureType::dumpNormalGetCppuType(FileStream & out) {
    dumpGetCppuTypePreamble(out);
    out << indent()
        << ("//TODO: On certain platforms with weak memory models, the"
            " following code can result in some threads observing that the_type"
            " points to garbage\n")
        << indent()
        << "static ::typelib_TypeDescriptionReference * the_type = 0;\n"
        << indent() << "if (the_type == 0) {\n";
    inc();
    if (isPolymorphic()) {
        out << indent() << "::rtl::OStringBuffer the_buffer(\""
            << m_typeName.replace('/', '.') << "<\");\n";
        sal_uInt16 n = m_reader.getReferenceCount();
        for (sal_uInt16 i = 0; i < n; ++i) {
            out << indent()
                << ("the_buffer.append(::rtl::OUStringToOString("
                    "::cppu::getTypeFavourChar(static_cast< ");
            dumpTypeParameterName(
                out,
                rtl::OUStringToOString(
                    m_reader.getReferenceTypeName(i), RTL_TEXTENCODING_UTF8));
            out << " * >(0)).getTypeName(), RTL_TEXTENCODING_UTF8));\n";
            if (i != n - 1) {
                out << indent() << "the_buffer.append(',');\n";
            }
        }
        out << indent() << "the_buffer.append('>');\n";
    }
    out << indent()
        << "::typelib_TypeDescriptionReference * the_members[] = {\n";
    inc();
    sal_uInt16 fields = m_reader.getFieldCount();
    for (sal_uInt16 i = 0; i < fields; ++i) {
        out << indent();
        rtl::OString type(
            rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
        if ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0) {
            out << "::cppu::getTypeFavourChar(static_cast< ";
            dumpTypeParameterName(out, type);
            out << " * >(0))";
        } else {
            out << "::cppu::UnoType< ";
            dumpType(out, type, false, false, false, true);
            out << " >::get()";
        }
        out << ".getTypeLibType()" << (i == fields - 1 ? " };" : ",")
            << "\n";
    }
    dec();
    if (isPolymorphic()) {
        out << indent()
            << "static ::sal_Bool const the_parameterizedTypes[] = { ";
        for (sal_uInt16 i = 0; i < fields; ++i) {
            if (i != 0) {
                out << ", ";
            }
            out << (((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE)
                     == 0)
                    ? "false" : "true");
        }
        out << " };\n";
    }
    out << indent() << "::typelib_static_struct_type_init(&the_type, ";
    if (isPolymorphic()) {
        out << "the_buffer.getStr()";
    } else {
        out << "\"" << m_typeName.replace('/', '.') << "\"";
    }
    out << ", ";
    if (m_reader.getSuperTypeCount() == 0) {
        out << "0";
    } else {
        out << "::cppu::UnoType< ";
        dumpType(
            out,
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8),
            false, false, false, true);
        out << " >::get().getTypeLibType()";
    }
    out << ", " << fields << ", the_members, "
        << (isPolymorphic() ? "the_parameterizedTypes" : "0") << ");\n";
    dec();
    out << indent() << "}\n" << indent()
        << ("return *reinterpret_cast< ::com::sun::star::uno::Type * >("
            "&the_type);\n");
    dumpGetCppuTypePostamble(out);
}

void StructureType::dumpComprehensiveGetCppuType(FileStream & out)
{
    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceOpen(out, m_typeName, false);
    else
        out << "namespace cppu { ";
    out << " namespace detail {\n\n";

    OString sType = m_typeName.copy(m_typeName.lastIndexOf('/') + 1);
    OString sStaticTypeClass = "the" + sType + "Type";
    out << indent();
    if (isPolymorphic())
        dumpTemplateHead(out);
    out << "struct " << sStaticTypeClass << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, ";
    out << sStaticTypeClass;
    if (isPolymorphic())
        dumpTemplateParameters(out);
    out << " >\n";

    out << indent() << "{\n";
    inc();
    out << indent() << "::com::sun::star::uno::Type * operator()() const\n";
    out << indent() << "{\n";

    inc();
    if (isPolymorphic()) {
        out << indent() << "::rtl::OUStringBuffer the_buffer;\n" << indent()
            << "the_buffer.appendAscii(RTL_CONSTASCII_STRINGPARAM(\""
            << m_typeName.replace('/', '.') << "<\"));\n";
        sal_uInt16 n = m_reader.getReferenceCount();
        for (sal_uInt16 i = 0; i < n; ++i) {
            out << indent()
                << "the_buffer.append(::cppu::getTypeFavourChar(static_cast< ";
            dumpTypeParameterName(
                out,
                rtl::OUStringToOString(
                    m_reader.getReferenceTypeName(i), RTL_TEXTENCODING_UTF8));
            out << " * >(0)).getTypeName());\n";
            if (i != n - 1) {
                out << indent()
                    << ("the_buffer.append("
                        "static_cast< ::sal_Unicode >(','));\n");
            }
        }
        out << indent()
            << "the_buffer.append(static_cast< ::sal_Unicode >('>'));\n"
            << indent()
            << "::rtl::OUString the_name(the_buffer.makeStringAndClear());\n";
    } else {
        out << indent()
            << "::rtl::OUString the_name(RTL_CONSTASCII_USTRINGPARAM(\""
            << m_typeName.replace('/', '.') << "\"));\n";
    }
    sal_uInt16 fields = m_reader.getFieldCount();
    typedef std::map< rtl::OString, sal_uInt32 > Map;
    Map parameters;
    Map types;
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        rtl::OString type(
            rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
        if ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0) {
            if (parameters.insert(
                    Map::value_type(
                        type, static_cast< sal_uInt32 >(parameters.size()))).
                second)
            {
                sal_uInt32 n = static_cast< sal_uInt32 >(parameters.size() - 1);
                out << indent()
                    << "::com::sun::star::uno::Type const & the_ptype" << n
                    << " = ::cppu::getTypeFavourChar(static_cast< ";
                dumpTypeParameterName(out, type);
                out << " * >(0));\n" << indent()
                    << "::typelib_TypeClass the_pclass" << n
                    << " = (::typelib_TypeClass) the_ptype" << n
                    << ".getTypeClass();\n" << indent()
                    << "::rtl::OUString the_pname" << n << "(the_ptype" << n
                    << ".getTypeName());\n";
            }
        } else if (types.insert(
                       Map::value_type(
                           type, static_cast< sal_uInt32 >(types.size()))).
                   second)
        {
            if ((codemaker::UnoType::getSort(type) ==
                 codemaker::UnoType::SORT_COMPLEX) &&
                codemaker::UnoType::decompose(type) != m_typeName)
                    // take care of recursion like struct S { sequence<S> x; };
            {
                out << indent() << "::cppu::UnoType< ";
                dumpType(out, type, false, false, false, true);
                out << " >::get();\n";
            }
            // For typedefs, use the resolved type name, as there will be no
            // information available about the typedef itself at runtime (the
            // above getCppuType call will make available information about the
            // resolved type); no extra #include for the resolved type is
            // needed, as the header for the typedef includes it already:
            out << indent() << "::rtl::OUString the_tname"
                << static_cast< sal_uInt32 >(types.size() - 1)
                << "(RTL_CONSTASCII_USTRINGPARAM(\""
                << checkRealBaseType(type, true).replace('/', '.') << "\"));\n";
        }
        out << indent() << "::rtl::OUString the_name" << i
            << "(RTL_CONSTASCII_USTRINGPARAM(\""
            << rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8).replace(
                    '/', '.')
            << "\"));\n";
    }}
    out << indent() << "::typelib_StructMember_Init the_members[] = {\n";
    inc();
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        out << indent() << "{ { ";
        rtl::OString type(
            rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8));
        if ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0) {
            sal_uInt32 n = parameters.find(type)->second;
            out << "the_pclass" << n << ", the_pname" << n << ".pData";
        } else {
            out << getTypeClass(type, true) << ", the_tname"
                << types.find(type)->second << ".pData";
        }
        out << ", the_name" << i << ".pData }, "
            << ((m_reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) == 0
                ? "false" : "true")
            << " }" << (i == fields - 1 ? " };" : ",") << "\n";
    }}
    dec();
    out << indent() << "::typelib_TypeDescription * the_newType = 0;\n";
    out << indent()
        << "::typelib_typedescription_newStruct(&the_newType, the_name.pData, ";
    if (m_reader.getSuperTypeCount() == 0) {
        out << "0";
    } else {
        out << "::cppu::UnoType< ";
        dumpType(
            out,
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8),
            false, false, false, true);
        out << " >::get().getTypeLibType()";
    }
    out << ", " << fields << ", the_members);\n";
    out << indent() << "::typelib_typedescription_register(&the_newType);\n";
    out << indent() << "::typelib_typedescription_release(the_newType);\n";

    out << indent() << "return new ::com::sun::star::uno::Type("
        << getTypeClass(m_typeName) << ", the_name); // leaked\n";
    dec();
    out << indent() << "}\n";
    dec();
    out << indent() << "};\n";

    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceClose(out, m_typeName, false);
    else
        out << " }";
    out << " }\n\n";

    dumpGetCppuTypePreamble(out);
    out  << indent() << "return *detail::" << sStaticTypeClass;
    if (isPolymorphic())
        dumpTemplateParameters(out);
    out  << "::get();\n";
    dumpGetCppuTypePostamble(out);
}

sal_Bool StructureType::dumpSuperMember(FileStream& o, const OString& superType, sal_Bool bWithType)
{
    sal_Bool hasMember = sal_False;

    if (!superType.isEmpty())
    {
        typereg::Reader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            rtl::OString superSuper;
            if (aSuperReader.getSuperTypeCount() >= 1) {
                superSuper = rtl::OUStringToOString(
                    aSuperReader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
            }
            hasMember = dumpSuperMember(o, superSuper, bWithType);

            sal_uInt16      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldFlags(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = rtl::OUStringToOString(
                    aSuperReader.getFieldName(i), RTL_TEXTENCODING_UTF8);
                fieldType = rtl::OUStringToOString(
                    aSuperReader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

                if (hasMember)
                {
                    o << ", ";
                } else
                {
                    hasMember = (fieldCount > 0);
                }

                if (bWithType)
                {
                    dumpType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
//              o << "__" << fieldName;
                o << fieldName << "_";
            }
        }
    }

    return hasMember;
}

void StructureType::addLightGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    if (isPolymorphic()) {
        includes.addRtlStrbufHxx();
        includes.addRtlTextencH();
        includes.addRtlUstringHxx();
    }
}

void StructureType::addNormalGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    if (isPolymorphic()) {
        includes.addRtlStrbufHxx();
        includes.addRtlTextencH();
        includes.addRtlUstringHxx();
    }
}

void StructureType::addComprehensiveGetCppuTypeIncludes(
    codemaker::cppumaker::Includes & includes) const
{
    includes.addType();
    includes.addCppuUnotypeHxx();
    includes.addRtlInstanceHxx();
    includes.addOslMutexHxx();
    includes.addRtlUstringH();
    includes.addRtlUstringHxx();
    includes.addSalTypesH();
    includes.addTypelibTypeclassH();
    includes.addTypelibTypedescriptionH();
    if (isPolymorphic()) {
        includes.addRtlStringH();
        includes.addRtlUstrbufHxx();
    }
}

bool StructureType::isPolymorphic() const {
    return m_reader.getReferenceCount() > 0;
}

void StructureType::dumpTemplateHead(FileStream & out) const {
    if (isPolymorphic()) {
        out << "template< ";
        for (sal_uInt16 i = 0; i < m_reader.getReferenceCount(); ++i) {
            if (i != 0) {
                out << ", ";
            }
            OSL_ASSERT(
                m_reader.getReferenceFlags(i) == RT_ACCESS_INVALID
                && m_reader.getReferenceSort(i) == RT_REF_TYPE_PARAMETER);
            out << "typename ";
            dumpTypeParameterName(
                out,
                rtl::OUStringToOString(
                    m_reader.getReferenceTypeName(i), RTL_TEXTENCODING_UTF8));
        }
        out << " > ";
    }
}

void StructureType::dumpTemplateParameters(FileStream & out) const {
    if (isPolymorphic()) {
        out << "< ";
        for (sal_uInt16 i = 0; i < m_reader.getReferenceCount(); ++i) {
            if (i != 0) {
                out << ", ";
            }
            OSL_ASSERT(
                m_reader.getReferenceFlags(i) == RT_ACCESS_INVALID
                && m_reader.getReferenceSort(i) == RT_REF_TYPE_PARAMETER);
            dumpTypeParameterName(
                out,
                rtl::OUStringToOString(
                    m_reader.getReferenceTypeName(i), RTL_TEXTENCODING_UTF8));
        }
        out << " >";
    }
}

//*************************************************************************
// ExceptionType
//*************************************************************************
ExceptionType::ExceptionType(typereg::Reader& typeReader,
                              const OString& typeName,
                             const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
}

ExceptionType::~ExceptionType()
{

}

sal_Bool ExceptionType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    dumpHFileContent(o, includes);
    return sal_True;
}

void ExceptionType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\nclass CPPU_GCC_DLLPUBLIC_EXPORT " << m_name;

    OString superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    if (!superType.isEmpty())
                    o << " : public " << scopedCppName(superType);

    o << "\n{\npublic:\n";
    inc();
    o << indent() << "inline CPPU_GCC_DLLPRIVATE " << m_name
      << "() SAL_THROW(());\n\n";

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;
    sal_uInt16      i = 0;

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << "inline CPPU_GCC_DLLPRIVATE " << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                            continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
//          o << " __" << fieldName;
            o << " " << fieldName << "_";
        }
        o << ") SAL_THROW(());\n\n";
    }
    o << indent() << "inline CPPU_GCC_DLLPRIVATE " << m_name << "(" << m_name
      << " const &);\n\n"
      << indent() << "inline CPPU_GCC_DLLPRIVATE ~" << m_name << "();\n\n"
      << indent() << "inline CPPU_GCC_DLLPRIVATE " << m_name << " & operator =("
      << m_name << " const &);\n\n";

    for (i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        fieldType = rtl::OUStringToOString(
            m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

        o << indent();
        dumpType(o, fieldType);
        o << " " << fieldName;
        if (i == 0 && !superType.isEmpty() &&
            !fieldType.equals("double") && !fieldType.equals("hyper") && !fieldType.equals("unsigned hyper"))
        {
            o << " CPPU_GCC3_ALIGN( " << scopedCppName(superType) << " )";
        }
        o << ";\n";
    }


    dec();
    o << "};\n\n";
}

sal_Bool ExceptionType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &m_typeName);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\n";

    o << "inline " << m_name << "::" << m_name << "() SAL_THROW(())\n";
    inc();
    OString superType;
    if (m_reader.getSuperTypeCount() >= 1) {
        superType = rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
    }
    sal_Bool first = sal_True;
    if (!superType.isEmpty())
    {
        o << indent() << ": " << scopedCppName(superType) << "()\n";
        first = sal_False;
    }

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    OString         fieldName;
    OString         fieldType;

    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

        if (first)
        {
            first = sal_False;
            o << indent() << ": ";
        } else
            o << indent() << ", ";

        o << fieldName;
        dumpInitializer(o, false, m_reader.getFieldTypeName(i));
        o << "\n";
    }
    dec();
    if ( !m_cppuTypeDynamic )
    {
        o << "{\n";
        inc();
        dumpCppuGetType(o, m_typeName, sal_True);
        dec();
        o << "}\n\n";
    } else
    {
        o << "{ }\n\n";
    }

    if (fieldCount > 0 || getInheritedMemberCount() > 0)
    {
        o << indent() << "inline " << m_name << "::" << m_name << "(";

        sal_Bool superHasMember = dumpSuperMember(o, superType, sal_True);

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
            fieldType = rtl::OUStringToOString(
                m_reader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

            if (superHasMember)
                o << ", ";
            else
                superHasMember = sal_True;

            dumpType(o, fieldType, sal_True, sal_True);
//          o << " __" << fieldName;
            o << " " << fieldName << "_";
        }
        o << ") SAL_THROW(())\n";

        inc();
        first = sal_True;
        if (!superType.isEmpty())
        {
            o << indent() << ": " << scopedCppName(superType) << "(";
            dumpSuperMember(o, superType, sal_False);
            o << ")\n";
            first = sal_False;
        }

        for (sal_uInt16 i=0; i < fieldCount; i++)
        {
            access = m_reader.getFieldFlags(i);

            if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                continue;

            fieldName = rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);

            if (first)
            {
                first = sal_False;
                o << indent() << ": ";
            } else
                o << indent() << ", ";

//          o << fieldName << "(__" << fieldName << ")\n";
            o << fieldName << "(" << fieldName << "_)\n";
        }

        dec();
        if ( !m_cppuTypeDynamic )
        {
            o << "{\n";
            inc();
            dumpCppuGetType(o, m_typeName, sal_True);
            dec();
            o << "}\n\n";
        } else
        {
            o << "{ }\n\n";
        }
    }
    o << indent() << m_name << "::" << m_name << "(" << m_name
      << " const & the_other)";
    first = true;
    if (!superType.isEmpty()) {
        o << ": " << scopedCppName(superType) << "(the_other)";
        first = false;
    }
    for (sal_uInt16 i = 0; i < fieldCount; ++i) {
        rtl::OString name(
            rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8));
        o << (first ? ": " : ", ") << name << "(the_other." << name << ")";
        first = false;
    }
    o << indent() << " {}\n\n"
      << indent() << m_name << "::~" << m_name << "() {}\n\n"
      << indent() << m_name << " & " << m_name << "::operator =(" << m_name
      << " const & the_other) {\n";
    inc();
    o << indent()
      << ("//TODO: Just like its implicitly-defined counterpart, this function"
          " definition is not exception-safe\n");
    if (!superType.isEmpty()) {
        o << indent() << scopedCppName(superType)
          << "::operator =(the_other);\n";
    }
    for (sal_uInt16 i = 0; i < fieldCount; ++i) {
        rtl::OString name(
            rtl::OUStringToOString(
                m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8));
        o << indent() << name << " = the_other." << name << ";\n";
    }
    o << indent() << "return *this;\n";
    dec();
    o << indent() << "}\n\n";

    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false)) {
        o << "\n";
    }

    o << "\n";
    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << "\n";
    return sal_True;
}

sal_Bool ExceptionType::dumpSuperMember(FileStream& o, const OString& superType, sal_Bool bWithType)
{
    sal_Bool hasMember = sal_False;

    if (!superType.isEmpty())
    {
        typereg::Reader aSuperReader(m_typeMgr.getTypeReader(superType));

        if (aSuperReader.isValid())
        {
            rtl::OString superSuper;
            if (aSuperReader.getSuperTypeCount() >= 1) {
                superSuper = rtl::OUStringToOString(
                    aSuperReader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8);
            }
            hasMember = dumpSuperMember(o, superSuper, bWithType);

            sal_uInt16      fieldCount = aSuperReader.getFieldCount();
            RTFieldAccess   access = RT_ACCESS_INVALID;
            OString         fieldName;
            OString         fieldType;
            for (sal_uInt16 i=0; i < fieldCount; i++)
            {
                access = aSuperReader.getFieldFlags(i);

                if (access == RT_ACCESS_CONST || access == RT_ACCESS_INVALID)
                    continue;

                fieldName = rtl::OUStringToOString(
                    aSuperReader.getFieldName(i), RTL_TEXTENCODING_UTF8);
                fieldType = rtl::OUStringToOString(
                    aSuperReader.getFieldTypeName(i), RTL_TEXTENCODING_UTF8);

                if (hasMember)
                {
                    o << ", ";
                } else
                {
                    hasMember = (fieldCount > 0);
                }

                if (bWithType)
                {
                    dumpType(o, fieldType, sal_True, sal_True);
                    o << " ";
                }
//              o << "__" << fieldName;
                o << fieldName << "_";
            }
        }
    }

    return hasMember;
}

//*************************************************************************
// EnumType
//*************************************************************************
EnumType::EnumType(typereg::Reader& typeReader,
                    const OString& typeName,
                   const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
}

EnumType::~EnumType()
{

}

sal_Bool EnumType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    dumpHFileContent(o, includes);
    return sal_True;
}

void EnumType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\nenum " << m_name << "\n{\n";
    inc();

    sal_uInt16      fieldCount = m_reader.getFieldCount();
    RTFieldAccess   access = RT_ACCESS_INVALID;
    RTConstValue    constValue;
    OString         fieldName;
    sal_Int32       value=0;
    for (sal_uInt16 i=0; i < fieldCount; i++)
    {
        access = m_reader.getFieldFlags(i);

        if (access != RT_ACCESS_CONST)
            continue;

        fieldName = rtl::OUStringToOString(
            m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8);
        constValue = m_reader.getFieldValue(i);

        if (constValue.m_type == RT_TYPE_INT32)
            value = constValue.m_value.aLong;
        else
            value++;

        o << indent() << m_name << "_" << fieldName << " = " << value << ",\n";
    }

    o << indent() << m_name << "_MAKE_FIXED_SIZE = SAL_MAX_ENUM\n";

    dec();
    o << "};\n\n";
}

sal_Bool EnumType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &m_typeName);
    o << "\n";

    dumpGetCppuType(o);

    o << "\n#endif // "<< headerDefine << "\n";
    return sal_True;
}

void EnumType::dumpNormalGetCppuType(FileStream& o)
{
    dumpGetCppuTypePreamble(o);

    o << indent()
      << "static typelib_TypeDescriptionReference * the_type = 0;\n";

    o << indent() << "if ( !the_type )\n" << indent() << "{\n";
    inc();

    o << indent() << "typelib_static_enum_type_init( &the_type,\n";
    inc(31);
    o << indent() << "\"" << m_typeName.replace('/', '.') << "\",\n"
      << indent() << scopedCppName(m_typeName) << "_"
      << rtl::OUStringToOString(m_reader.getFieldName(0), RTL_TEXTENCODING_UTF8)
      << " );\n";
    dec(31);
    dec();
    o << indent() << "}\n";
    o << indent()
      << ("return * reinterpret_cast< ::com::sun::star::uno::Type * >("
          " &the_type );\n");
    dumpGetCppuTypePostamble(o);
}

void EnumType::dumpComprehensiveGetCppuType(FileStream& o)
{
    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false);
    else
        o << "namespace cppu { ";
    o << " namespace detail {\n\n";

    OString sType = m_typeName.copy(m_typeName.lastIndexOf('/') + 1);
    OString sStaticTypeClass = "the" + sType + "Type";
    o << indent() << "struct " << sStaticTypeClass << " : public rtl::StaticWithInit< ::com::sun::star::uno::Type *, " << sStaticTypeClass << " >\n";
    o << indent() << "{\n";
    inc();
    o << indent() << "::com::sun::star::uno::Type * operator()() const\n";
    o << indent() << "{\n";

    inc();
    o << indent() << "::rtl::OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM(\""
      << m_typeName.replace('/', '.') << "\") );\n\n";

    o << indent() << "// Start inline typedescription generation\n"
      << indent() << "typelib_TypeDescription * pTD = 0;\n\n";

    sal_uInt16 count = m_reader.getFieldCount();
    o << indent() << "rtl_uString* enumValueNames[" << count << "];\n";
    sal_uInt16 i;
    for (i = 0; i < count; i++)
    {
        o << indent() << "::rtl::OUString sEnumValue" << i
          << "( RTL_CONSTASCII_USTRINGPARAM(\""
          << rtl::OUStringToOString(
              m_reader.getFieldName(i), RTL_TEXTENCODING_UTF8)
          << "\") );\n";
        o << indent() << "enumValueNames[" << i << "] = sEnumValue" << i
          << ".pData;\n";
    }

    o << "\n" << indent() << "sal_Int32 enumValues[" << count << "];\n";
    RTConstValue    constValue;
    sal_Int32       value=0;
    for (i = 0; i < count; i++)
    {
        o << indent() << "enumValues[" << i << "] = ";
        constValue = m_reader.getFieldValue(i);
        if (constValue.m_type == RT_TYPE_INT32)
            value = constValue.m_value.aLong;
        else
            value++;
        o << value << ";\n";
    }

    o << "\n" << indent() << "typelib_typedescription_newEnum( &pTD,\n";
    inc();
    o << indent() << "sTypeName.pData,\n"
      << indent() << "(sal_Int32)" << scopedCppName(m_typeName, sal_False)
      << "_"
      << rtl::OUStringToOString(m_reader.getFieldName(0), RTL_TEXTENCODING_UTF8)
      << ",\n"
      << indent() << count << ", enumValueNames, enumValues );\n\n";
    dec();

    o << indent()
      << ("typelib_typedescription_register( (typelib_TypeDescription**)&pTD"
          " );\n");
    o << indent() << "typelib_typedescription_release( pTD );\n"
      << indent() << "// End inline typedescription generation\n\n";

    o << indent() << "return new ::com::sun::star::uno::Type( "
      << getTypeClass(m_typeName) << ", sTypeName ); // leaked\n";

    dec();
    o << indent() << "}\n";
    dec();
    o << indent() << "};\n\n";

    if (!isPolymorphic())
        codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false);
    else
        o << " }";
    o << " }\n\n";

    dumpGetCppuTypePreamble(o);
    o  << indent() << "return *detail::" << sStaticTypeClass << "::get();\n";
    dumpGetCppuTypePostamble(o);
}

//*************************************************************************
// TypeDefType
//*************************************************************************
TypeDefType::TypeDefType(typereg::Reader& typeReader,
                             const OString& typeName,
                            const TypeManager& typeMgr)
    : CppuType(typeReader, typeName, typeMgr)
{
}

TypeDefType::~TypeDefType()
{

}

sal_Bool TypeDefType::dumpHFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HDL"));
    o << "\n";

    addDefaultHIncludes(includes);
    includes.dump(o, 0);
    o << "\n";

    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false)) {
        o << "\n";
    }

    dumpDeclaration(o);

    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false)) {
        o << "\n";
    }

//  o << "\nnamespace com { namespace sun { namespace star { namespace uno {\n"
//    << "class Type;\n} } } }\n\n";
//  o << "inline const ::com::sun::star::uno::Type& SAL_CALL get_" << m_typeName.replace('/', '_')
//    <<  "_Type( ) SAL_THROW(());\n\n";

    o << "#endif // "<< headerDefine << "\n";

    return sal_True;
}

void TypeDefType::dumpDeclaration(FileStream& o)
    throw( CannotDumpException )
{
    o << "\ntypedef ";
    dumpType(
        o,
        rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    o << " " << m_name << ";\n\n";
}

sal_Bool TypeDefType::dumpHxxFile(
    FileStream& o, codemaker::cppumaker::Includes & includes)
    throw( CannotDumpException )
{
    OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";

    addDefaultHxxIncludes(includes);
    includes.dump(o, &m_typeName);
    o << "\n";

    o << "\n#endif // "<< headerDefine << "\n";
    return sal_True;
}

//*************************************************************************
// ConstructiveType
//*************************************************************************

sal_Bool ConstructiveType::dumpHFile(
    FileStream &, codemaker::cppumaker::Includes &) throw (CannotDumpException)
{
    OSL_ASSERT(false);
    return false;
}

bool ConstructiveType::dumpFiles(
    CppuOptions * options, rtl::OString const & outPath)
{
    return dumpFile(options, ".hpp", m_typeName, outPath);
}

//*************************************************************************
// ServiceType
//*************************************************************************

namespace {

void includeExceptions(
    codemaker::cppumaker::Includes & includes,
    codemaker::ExceptionTreeNode const * node)
{
    if (node->present) {
        includes.add(node->name);
    } else {
        for (codemaker::ExceptionTreeNode::Children::const_iterator i(
                 node->children.begin());
             i != node->children.end(); ++i)
        {
            includeExceptions(includes, *i);
        }
    }
}

}

bool ServiceType::isSingleInterfaceBased() {
    return m_reader.getSuperTypeCount() == 1;
}

sal_Bool ServiceType::dumpHxxFile(
    FileStream & o, codemaker::cppumaker::Includes & includes)
    throw (CannotDumpException)
{
    sal_uInt16 ctors = m_reader.getMethodCount();
    if (ctors > 0) {
        //TODO: Decide whether the types added to includes should rather be
        // added to m_dependencies (and thus be generated during
        // dumpDependedTypes):
        includes.addCassert();
        includes.addReference();
        includes.addRtlUstringH();
        includes.addRtlUstringHxx();
        includes.add("com/sun/star/uno/DeploymentException");
        includes.add("com/sun/star/uno/XComponentContext");
        for (sal_uInt16 i = 0; i < ctors; ++i) {
            if (isDefaultConstructor(i)) {
                includes.add("com/sun/star/uno/Exception");
                includes.add("com/sun/star/uno/RuntimeException");
            } else {
                if (!hasRestParameter(i)) {
                    includes.addAny();
                    includes.addSequence();
                    sal_uInt16 params = m_reader.getMethodParameterCount(i);
                    for (sal_uInt16 j = 0; j < params; ++j) {
                        if (codemaker::UnoType::getSort(
                                codemaker::UnoType::decompose(
                                    rtl::OUStringToOString(
                                        m_reader.getMethodParameterTypeName(
                                            i, j),
                                        RTL_TEXTENCODING_UTF8),
                                    0, 0))
                            == codemaker::UnoType::SORT_CHAR)
                        {
                            includes.addCppuUnotypeHxx();
                            break;
                        }
                    }
                }
                codemaker::ExceptionTree tree;
                for (sal_uInt16 j = 0; j < m_reader.getMethodExceptionCount(i);
                     ++j)
                {
                    tree.add(
                        rtl::OUStringToOString(
                            m_reader.getMethodExceptionTypeName(i, j),
                            RTL_TEXTENCODING_UTF8),
                        m_typeMgr);
                }
                if (!tree.getRoot()->present) {
                    includes.add("com/sun/star/uno/Exception");
                    includes.add("com/sun/star/uno/RuntimeException");
                    includeExceptions(includes, tree.getRoot());
                }
            }
        }
    }
    rtl::OString cppName(translateUnoToCppIdentifier(
                             m_name, "service", isGlobal()));
    rtl::OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";
    includes.dump(o, 0);
    o << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\nclass " << cppName << " {\n";
    inc();
    if (ctors > 0) {
        rtl::OString fullName(m_typeName.replace('/', '.'));
        rtl::OString baseName(
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
        rtl::OString fullBaseName(baseName.replace('/', '.'));
        rtl::OString scopedBaseName(scopedCppName(baseName));
        o << "public:\n";
        for (sal_uInt16 i = 0; i < ctors; ++i) {
            if (isDefaultConstructor(i)) {
                o << indent() << "static ::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > "
                  << translateUnoToCppIdentifier(
                      "create", "method", ITM_NONGLOBAL, &cppName)
                  << ("(::com::sun::star::uno::Reference<"
                      " ::com::sun::star::uno::XComponentContext > const &"
                      " the_context) {\n");
                inc();
                o << indent() << "assert(the_context.is());\n" << indent()
                  << "::com::sun::star::uno::Reference< " << scopedBaseName
                  << " > the_instance;\n" << indent() << "try {\n";
                inc();
                o << indent()
                  << "the_instance = ::com::sun::star::uno::Reference< "
                  << scopedBaseName
                  << (" >(the_context->getServiceManager()->"
                      "createInstanceWithContext(::rtl::OUString("
                      "RTL_CONSTASCII_USTRINGPARAM(\"")
                  << fullName
                  << "\")), the_context), ::com::sun::star::uno::UNO_QUERY);\n";
                dec();
                o << indent()
                  << "} catch (::com::sun::star::uno::RuntimeException &) {\n";
                inc();
                o << indent() << "throw;\n";
                dec();
                o << indent()
                  << ("} catch (::com::sun::star::uno::Exception &"
                      " the_exception) {\n");
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("
                      "\"component context fails to supply service ")
                  << fullName << " of type " << fullBaseName
                  << ": \")) + the_exception.Message, the_context);\n";
                dec();
                o << indent() << "}\n" << indent()
                  << "if (!the_instance.is()) {\n";
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("
                      "\"component context fails to supply service ")
                  << fullName << " of type " << fullBaseName
                  << "\")), the_context);\n";
                dec();
                o << indent() << "}\n" << indent() << "return the_instance;\n";
                dec();
                o << indent() << "}\n\n";
            } else {
                o << indent() << "static ::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > "
                  << translateUnoToCppIdentifier(
                      rtl::OUStringToOString(
                          m_reader.getMethodName(i), RTL_TEXTENCODING_UTF8),
                      "method", ITM_NONGLOBAL, &cppName)
                  << ("(::com::sun::star::uno::Reference<"
                      " ::com::sun::star::uno::XComponentContext > const &"
                      " the_context");
                sal_uInt16 params = m_reader.getMethodParameterCount(i);
                bool rest = hasRestParameter(i);
                for (sal_uInt16 j = 0; j < params; ++j) {
                    o << ", ";
                    rtl::OStringBuffer buf;
                    if ((m_reader.getMethodParameterFlags(i, j) & RT_PARAM_REST)
                        != 0)
                    {
                        buf.append(RTL_CONSTASCII_STRINGPARAM("[]"));
                    }
                    buf.append(
                        rtl::OUStringToOString(
                            m_reader.getMethodParameterTypeName(i, j),
                            RTL_TEXTENCODING_UTF8));
                    rtl::OString type(buf.makeStringAndClear());
                    bool byRef = passByReference(type);
                    dumpType(o, type, byRef, byRef);
                    o << " "
                      << translateUnoToCppIdentifier(
                          rtl::OUStringToOString(
                              m_reader.getMethodParameterName(i, j),
                              RTL_TEXTENCODING_UTF8),
                          "param", ITM_NONGLOBAL);
                }
                o << ") {\n";
                inc();
                o << indent() << "assert(the_context.is());\n";
                if (!rest && params > 0) {
                    o << indent()
                      << ("::com::sun::star::uno::Sequence<"
                          " ::com::sun::star::uno::Any > the_arguments(")
                      << params << ");\n";
                    for (sal_uInt16 j = 0; j < params; ++j) {
                        o << indent() << "the_arguments[" << j << "] ";
                        rtl::OString param(
                            translateUnoToCppIdentifier(
                                rtl::OUStringToOString(
                                    m_reader.getMethodParameterName(i, j),
                                    RTL_TEXTENCODING_UTF8),
                                "param", ITM_NONGLOBAL));
                        sal_Int32 rank;
                        if (codemaker::UnoType::getSort(
                                codemaker::UnoType::decompose(
                                    rtl::OUStringToOString(
                                        m_reader.getMethodParameterTypeName(
                                            i, j),
                                        RTL_TEXTENCODING_UTF8),
                                    &rank, 0))
                            == codemaker::UnoType::SORT_CHAR)
                        {
                            o << "= ::com::sun::star::uno::Any(&" << param
                              << ", ::cppu::UnoType< ";
                            for (sal_Int32 k = 0; k < rank; ++k) {
                                o << "::cppu::UnoSequenceType< ";
                            }
                            o << "::cppu::UnoCharType";
                            for (sal_Int32 k = 0; k < rank; ++k) {
                                o << " >";
                            }
                            o << " >::get())";
                        } else {
                            o << "<<= " << param;
                        }
                        o << ";\n";
                    }
                }
                o << indent() << "::com::sun::star::uno::Reference< "
                  << scopedBaseName << " > the_instance;\n";
                codemaker::ExceptionTree tree;
                sal_uInt16 exceptions = m_reader.getMethodExceptionCount(i);
                for (sal_uInt16 j = 0; j < exceptions; ++j) {
                    tree.add(
                        rtl::OUStringToOString(
                            m_reader.getMethodExceptionTypeName(i, j),
                            RTL_TEXTENCODING_UTF8),
                        m_typeMgr);
                }
                if (!tree.getRoot()->present) {
                    o << indent() << "try {\n";
                    inc();
                }
                o << indent()
                  << "the_instance = ::com::sun::star::uno::Reference< "
                  << scopedBaseName
                  << (" >(the_context->getServiceManager()->"
                      "createInstanceWithArgumentsAndContext(::rtl::OUString("
                      "RTL_CONSTASCII_USTRINGPARAM(\"")
                  << fullName << "\")), ";
                if (rest) {
                    o << translateUnoToCppIdentifier(
                        rtl::OUStringToOString(
                            m_reader.getMethodParameterName(i, 0),
                            RTL_TEXTENCODING_UTF8),
                        "param", ITM_NONGLOBAL);
                } else if (params == 0) {
                    o << ("::com::sun::star::uno::Sequence<"
                          " ::com::sun::star::uno::Any >()");
                } else {
                    o << "the_arguments";
                }
                o << ", the_context), ::com::sun::star::uno::UNO_QUERY);\n";
                if (!tree.getRoot()->present) {
                    dec();
                    o << indent()
                      << ("} catch (::com::sun::star::uno::RuntimeException &)"
                          " {\n");
                    inc();
                    o << indent() << "throw;\n";
                    dec();
                    dumpCatchClauses(o, tree.getRoot());
                    o << indent()
                      << ("} catch (::com::sun::star::uno::Exception &"
                          " the_exception) {\n");
                    inc();
                    o << indent()
                      << ("throw ::com::sun::star::uno::DeploymentException("
                          "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("
                          "\"component context fails to supply service ")
                      << fullName << " of type " << fullBaseName
                      << ": \")) + the_exception.Message, the_context);\n";
                    dec();
                    o << indent() << "}\n";
                }
                o << indent() << "if (!the_instance.is()) {\n";
                inc();
                o << indent()
                  << ("throw ::com::sun::star::uno::DeploymentException("
                      "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("
                      "\"component context fails to supply service ")
                  << fullName << " of type " << fullBaseName
                  << "\")), the_context);\n";
                dec();
                o << indent() << "}\n" << indent() << "return the_instance;\n";
                dec();
                o << indent() << "}\n\n";
            }
        }
    }
    o << "private:\n";
    o << indent() << cppName << "(); // not implemented\n"
      << indent() << cppName << "(" << cppName << " &); // not implemented\n"
      << indent() << "~" << cppName << "(); // not implemented\n"
      << indent() << "void operator =(" << cppName << "); // not implemented\n";
    dec();
    o << "};\n\n";
    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\n#endif // "<< headerDefine << "\n";
    return true;
}

void ServiceType::addSpecialDependencies() {
    if (m_reader.getMethodCount() > 0) {
        OSL_ASSERT(m_reader.getSuperTypeCount() == 1);
        m_dependencies.add(
            rtl::OUStringToOString(
                m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    }
}

bool ServiceType::isDefaultConstructor(sal_uInt16 ctorIndex) const {
    return m_reader.getMethodName(ctorIndex).isEmpty();
}

bool ServiceType::hasRestParameter(sal_uInt16 ctorIndex) const {
    return m_reader.getMethodParameterCount(ctorIndex) == 1
        && ((m_reader.getMethodParameterFlags(ctorIndex, 0) & RT_PARAM_REST)
            != 0);
}

void ServiceType::dumpCatchClauses(
    FileStream & out, codemaker::ExceptionTreeNode const * node)
{
    if (node->present) {
        out << indent() << "} catch (";
        dumpType(out, node->name);
        out << " &) {\n";
        inc();
        out << indent() << "throw;\n";
        dec();
    } else {
        for (codemaker::ExceptionTreeNode::Children::const_iterator i(
                 node->children.begin());
             i != node->children.end(); ++i)
        {
            dumpCatchClauses(out, *i);
        }
    }
}

//*************************************************************************
// SingletonType
//*************************************************************************

bool SingletonType::isInterfaceBased() {
    return (m_typeMgr.getTypeClass(
                rtl::OUStringToOString(
                    m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8)))
        == RT_TYPE_INTERFACE;
}

sal_Bool SingletonType::dumpHxxFile(
    FileStream & o, codemaker::cppumaker::Includes & includes)
    throw (CannotDumpException)
{
    rtl::OString cppName(translateUnoToCppIdentifier(
                             m_name, "singleton", isGlobal()));
    rtl::OString fullName(m_typeName.replace('/', '.'));
    rtl::OString baseName(
        rtl::OUStringToOString(
            m_reader.getSuperTypeName(0), RTL_TEXTENCODING_UTF8));
    rtl::OString fullBaseName(baseName.replace('/', '.'));
    rtl::OString scopedBaseName(scopedCppName(baseName));
    rtl::OString headerDefine(dumpHeaderDefine(o, "HPP"));
    o << "\n";
    //TODO: Decide whether the types added to includes should rather be added to
    // m_dependencies (and thus be generated during dumpDependedTypes):
    includes.add("com/sun/star/uno/DeploymentException");
    includes.add("com/sun/star/uno/XComponentContext");
    includes.addCassert();
    includes.addAny();
    includes.addReference();
    includes.addRtlUstringH();
    includes.addRtlUstringHxx();
    includes.dump(o, 0);
    o << "\n";
    if (codemaker::cppumaker::dumpNamespaceOpen(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\nclass " << cppName << " {\npublic:\n";
    inc();
    o << indent() << "static ::com::sun::star::uno::Reference< "
      << scopedBaseName << " > "
      << translateUnoToCppIdentifier("get", "method", ITM_NONGLOBAL, &cppName)
      << ("(::com::sun::star::uno::Reference<"
          " ::com::sun::star::uno::XComponentContext > const & the_context)"
          " {\n");
    inc();
    o << indent() << "assert(the_context.is());\n" << indent()
      << "::com::sun::star::uno::Reference< " << scopedBaseName
      << " > instance;\n" << indent()
      << ("if (!(the_context->getValueByName("
          "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\"/singletons/")
      << fullName << "\"))) >>= instance) || !instance.is()) {\n";
    inc();
    o << indent()
      << ("throw ::com::sun::star::uno::DeploymentException("
          "::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(\"component context"
          " fails to supply singleton ")
      << fullName << " of type " << fullBaseName << "\")), the_context);\n";
    dec();
    o << indent() << "}\n" << indent() << "return instance;\n";
    dec();
    o << indent() << "}\n\n";
    o << "private:\n";
    o << indent() << cppName << "(); // not implemented\n"
      << indent() << cppName << "(" << cppName << " &); // not implemented\n"
      << indent() << "~" << cppName << "(); // not implemented\n"
      << indent() << "void operator =(" << cppName << "); // not implemented\n";
    dec();
    o << "};\n\n";
    if (codemaker::cppumaker::dumpNamespaceClose(o, m_typeName, false)) {
        o << "\n";
    }
    o << "\n#endif // "<< headerDefine << "\n";
    return true;
}

//*************************************************************************
// produceType
//*************************************************************************
bool produceType(const OString& typeName,
                 TypeManager const & typeMgr,
                 codemaker::GeneratedTypeSet & generated,
                 CppuOptions* pOptions)
    throw( CannotDumpException )
{
    if (typeName.equals("/") || typeName.equals(typeMgr.getBase()) ||
        TypeManager::isBaseType(typeName) || generated.contains(typeName))
    {
        return true;
    }

    sal_Bool bIsExtraType = sal_False;
    typereg::Reader reader(typeMgr.getTypeReader(typeName, &bIsExtraType));
    if (bIsExtraType) {
        generated.add(typeName);
        return true;
    }

    if (!reader.isValid()) {
        return false;
    }

    RTTypeClass typeClass = reader.getTypeClass();
    bool ret = false;
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            {
                InterfaceType iType(reader, typeName, typeMgr);
                ret = iType.dump(pOptions);
                if (ret) generated.add(typeName);
                iType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_MODULE:
            {
                ModuleType mType(reader, typeName, typeMgr);
                if (mType.hasConstants())
                {
                    ret = mType.dump(pOptions);
                    if (ret) generated.add(typeName);
                } else
                {
                    generated.add(typeName);
                    ret = true;
                }
            }
            break;
        case RT_TYPE_STRUCT:
            {
                StructureType sType(reader, typeName, typeMgr);
                ret = sType.dump(pOptions);
                if (ret) generated.add(typeName);
                sType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_ENUM:
            {
                EnumType enType(reader, typeName, typeMgr);
                ret = enType.dump(pOptions);
                if (ret) generated.add(typeName);
                enType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_EXCEPTION:
            {
                ExceptionType eType(reader, typeName, typeMgr);
                ret = eType.dump(pOptions);
                if (ret) generated.add(typeName);
                eType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_TYPEDEF:
            {
                TypeDefType tdType(reader, typeName, typeMgr);
                ret = tdType.dump(pOptions);
                if (ret) generated.add(typeName);
                tdType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_CONSTANTS:
            {
                ConstantsType cType(reader, typeName, typeMgr);
                if (cType.hasConstants())
                {
                    ret = cType.dump(pOptions);
                    if (ret) generated.add(typeName);
                } else
                {
                    generated.add(typeName);
                    ret = true;
                }
            }
            break;
        case RT_TYPE_SERVICE:
            {
                ServiceType t(reader, typeName, typeMgr);
                if (t.isSingleInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        generated.add(typeName);
                        t.dumpDependedTypes(generated, pOptions);
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_SINGLETON:
            {
                SingletonType t(reader, typeName, typeMgr);
                if (t.isInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        generated.add(typeName);
                        t.dumpDependedTypes(generated, pOptions);
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_OBJECT:
            ret = true;
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    return ret;
}

bool produceType(RegistryKey& rTypeKey, bool bIsExtraType,
                     TypeManager const & typeMgr,
                     codemaker::GeneratedTypeSet & generated,
                     CppuOptions* pOptions)
    throw( CannotDumpException )
{
    OString typeName = typeMgr.getTypeName(rTypeKey);

    if (typeName.equals("/") ||typeName.equals(typeMgr.getBase()) ||
        TypeManager::isBaseType(typeName) || generated.contains(typeName))
    {
        return true;
    }

    if (bIsExtraType) {
        generated.add(typeName);
        return true;
    }

    typereg::Reader reader(typeMgr.getTypeReader(rTypeKey));
    if (!reader.isValid()) {
        return false;
    }

    RTTypeClass typeClass = reader.getTypeClass();
    bool ret = false;
    switch (typeClass)
    {
        case RT_TYPE_INTERFACE:
            {
                InterfaceType iType(reader, typeName, typeMgr);
                ret = iType.dump(pOptions);
                if (ret) generated.add(typeName);
                iType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_MODULE:
            {
                ModuleType mType(reader, typeName, typeMgr);
                if (mType.hasConstants())
                {
                    ret = mType.dump(pOptions);
                    if (ret) generated.add(typeName);
                } else
                {
                    generated.add(typeName);
                    ret = true;
                }
            }
            break;
        case RT_TYPE_STRUCT:
            {
                StructureType sType(reader, typeName, typeMgr);
                ret = sType.dump(pOptions);
                if (ret) generated.add(typeName);
                sType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_ENUM:
            {
                EnumType enType(reader, typeName, typeMgr);
                ret = enType.dump(pOptions);
                if (ret) generated.add(typeName);
                enType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_EXCEPTION:
            {
                ExceptionType eType(reader, typeName, typeMgr);
                ret = eType.dump(pOptions);
                if (ret) generated.add(typeName);
                eType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_TYPEDEF:
            {
                TypeDefType tdType(reader, typeName, typeMgr);
                ret = tdType.dump(pOptions);
                if (ret) generated.add(typeName);
                tdType.dumpDependedTypes(generated, pOptions);
            }
            break;
        case RT_TYPE_CONSTANTS:
            {
                ConstantsType cType(reader, typeName, typeMgr);
                if (cType.hasConstants())
                {
                    ret = cType.dump(pOptions);
                    if (ret) generated.add(typeName);
                } else
                {
                    generated.add(typeName);
                    ret = true;
                }
            }
            break;
        case RT_TYPE_SERVICE:
            {
                ServiceType t(reader, typeName, typeMgr);
                if (t.isSingleInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        generated.add(typeName);
                        t.dumpDependedTypes(generated, pOptions);
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_SINGLETON:
            {
                SingletonType t(reader, typeName, typeMgr);
                if (t.isInterfaceBased()) {
                    ret = t.dump(pOptions);
                    if (ret) {
                        generated.add(typeName);
                        t.dumpDependedTypes(generated, pOptions);
                    }
                } else {
                    ret = true;
                }
            }
            break;
        case RT_TYPE_OBJECT:
            ret = true;
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
