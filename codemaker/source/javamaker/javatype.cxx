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

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <codemaker/codemaker.hxx>
#include <codemaker/exceptiontree.hxx>
#include <codemaker/generatedtypeset.hxx>
#include <codemaker/global.hxx>
#include <codemaker/options.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/unotype.hxx>
#include <codemaker/commonjava.hxx>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

#include "classfile.hxx"
#include "javaoptions.hxx"
#include "javatype.hxx"

using codemaker::javamaker::ClassFile;

namespace {

void appendUnoName(
    rtl::Reference< TypeManager > const & manager, OUString const & nucleus,
    sal_Int32 rank, std::vector< OUString > const & arguments,
    OUStringBuffer * buffer)
{
    assert(manager.is());
    assert(rank >= 0);
    assert(buffer != nullptr);
    for (sal_Int32 i = 0; i != rank; ++i) {
        buffer->append("[]");
    }
    buffer->append(nucleus);
    if (arguments.empty())
        return;

    buffer->append('<');
    for (std::vector< OUString >::const_iterator i(arguments.begin());
         i != arguments.end(); ++i)
    {
        if (i != arguments.begin()) {
            buffer->append(',');
        }
        OUString n;
        sal_Int32 k;
        std::vector< OUString > args;
        manager->decompose(*i, false, &n, &k, &args, nullptr);
        appendUnoName(manager, n, k, args, buffer);
    }
    buffer->append('>');
}

// Translate the name of a UNOIDL entity (enum type, plain struct type,
// polymorphic struct type template, or interface type, decomposed into nucleus,
// sequence rank, and template arguments) into a core UNO type name:
OUString createUnoName(
    rtl::Reference< TypeManager > const & manager, OUString const & nucleus,
    sal_Int32 rank, std::vector< OUString > const & arguments)
{
    OUStringBuffer buf(256);
    appendUnoName(manager, nucleus, rank, arguments, &buf);
    return buf.makeStringAndClear();
}

enum SpecialType {
    SPECIAL_TYPE_NONE,
    SPECIAL_TYPE_ANY,
    SPECIAL_TYPE_UNSIGNED,
    SPECIAL_TYPE_INTERFACE
};

bool isSpecialType(SpecialType special) {
    return special >= SPECIAL_TYPE_UNSIGNED;
}

OString translateUnoidlEntityNameToJavaFullyQualifiedName(
    OUString const & name, OString const & prefix)
{
    assert(!name.startsWith("[]"));
    assert(name.indexOf('<') == -1);
    sal_Int32 i = name.lastIndexOf('.') + 1;
    return codemaker::convertString(name.copy(0, i)).replace('.', '/')
        + codemaker::java::translateUnoToJavaIdentifier(
            codemaker::convertString(name.copy(i)), prefix);
}

struct PolymorphicUnoType {
    PolymorphicUnoType(): kind(KIND_NONE) {}

    enum Kind { KIND_NONE, KIND_STRUCT, KIND_SEQUENCE };
    Kind kind;
    OUString name;
};

SpecialType translateUnoTypeToDescriptor(
    rtl::Reference< TypeManager > const & manager, OUString const & type,
    bool array, bool classType, std::set<OUString> * dependencies,
    OStringBuffer * descriptor, OStringBuffer * signature,
    bool * needsSignature, PolymorphicUnoType * polymorphicUnoType);

SpecialType translateUnoTypeToDescriptor(
    rtl::Reference< TypeManager > const & manager,
    codemaker::UnoType::Sort sort, OUString const & nucleus, sal_Int32 rank,
    std::vector< OUString > const & arguments, bool array, bool classType,
    std::set<OUString> * dependencies, OStringBuffer * descriptor,
    OStringBuffer * signature, bool * needsSignature,
    PolymorphicUnoType * polymorphicUnoType)
{
    assert(rank >= 0);
    assert((signature == nullptr) == (needsSignature == nullptr));
    assert(
        arguments.empty()
        == (sort
            != codemaker::UnoType::Sort::InstantiatedPolymorphicStruct));
    if (rank > 0xFF - (array ? 1 : 0)) {
        throw CannotDumpException(
            "Too many array dimensions for Java class file format");
    }
    if (array) {
        ++rank;
    }
    for (sal_Int32 i = 0; i != rank; ++i) {
        if (descriptor != nullptr) {
            descriptor->append('[');
        }
        if (signature != nullptr) {
            signature->append('[');
        }
    }
    if (polymorphicUnoType != nullptr) {
        if (sort
            == codemaker::UnoType::Sort::InstantiatedPolymorphicStruct)
        {
            polymorphicUnoType->kind = rank == 0
                ? PolymorphicUnoType::KIND_STRUCT
                : PolymorphicUnoType::KIND_SEQUENCE;
            polymorphicUnoType->name = createUnoName(
                manager, nucleus, rank, arguments);
        } else {
            polymorphicUnoType->kind = PolymorphicUnoType::KIND_NONE;
        }
    }
    switch (sort) {
    case codemaker::UnoType::Sort::Void:
    case codemaker::UnoType::Sort::Boolean:
    case codemaker::UnoType::Sort::Byte:
    case codemaker::UnoType::Sort::Short:
    case codemaker::UnoType::Sort::UnsignedShort:
    case codemaker::UnoType::Sort::Long:
    case codemaker::UnoType::Sort::UnsignedLong:
    case codemaker::UnoType::Sort::Hyper:
    case codemaker::UnoType::Sort::UnsignedHyper:
    case codemaker::UnoType::Sort::Float:
    case codemaker::UnoType::Sort::Double:
    case codemaker::UnoType::Sort::Char:
    case codemaker::UnoType::Sort::String:
    case codemaker::UnoType::Sort::Type:
    case codemaker::UnoType::Sort::Any:
        {
            static char const * const
                simpleTypeDescriptors[static_cast<int>(codemaker::UnoType::Sort::Any) + 1][2] = {
                { "V", "Ljava/lang/Void;" },
                { "Z", "Ljava/lang/Boolean;" },
                { "B", "Ljava/lang/Byte;" },
                { "S", "Ljava/lang/Short;" },
                { "S", "Ljava/lang/Short;" },
                { "I", "Ljava/lang/Integer;" },
                { "I", "Ljava/lang/Integer;" },
                { "J", "Ljava/lang/Long;" },
                { "J", "Ljava/lang/Long;" },
                { "F", "Ljava/lang/Float;" },
                { "D", "Ljava/lang/Double;" },
                { "C", "Ljava/lang/Character;" },
                { "Ljava/lang/String;", "Ljava/lang/String;" },
                { "Lcom/sun/star/uno/Type;", "Lcom/sun/star/uno/Type;" },
                { "Ljava/lang/Object;", "Ljava/lang/Object;" } };
            char const * s
                = simpleTypeDescriptors[static_cast<int>(sort)][rank == 0 && classType];
            if (descriptor != nullptr) {
                descriptor->append(s);
            }
            if (signature != nullptr) {
                signature->append(s);
            }
            static SpecialType const
                simpleTypeSpecials[static_cast<int>(codemaker::UnoType::Sort::Any) + 1] = {
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_UNSIGNED, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_UNSIGNED, SPECIAL_TYPE_NONE, SPECIAL_TYPE_UNSIGNED,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_ANY };
            return simpleTypeSpecials[static_cast<int>(sort)];
        }
    case codemaker::UnoType::Sort::Interface:
        if (nucleus == "com.sun.star.uno.XInterface") {
            if (descriptor != nullptr) {
                descriptor->append("Ljava/lang/Object;");
            }
            if (signature != nullptr) {
                signature->append("Ljava/lang/Object;");
            }
            return SPECIAL_TYPE_INTERFACE;
        }
        [[fallthrough]];
    case codemaker::UnoType::Sort::Sequence:
    case codemaker::UnoType::Sort::Enum:
    case codemaker::UnoType::Sort::PlainStruct:
    case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
        if (dependencies != nullptr) {
            dependencies->insert(nucleus);
        }
        if (descriptor != nullptr) {
            descriptor->append(
                "L" + codemaker::convertString(nucleus).replace('.', '/')
                + ";");
        }
        if (signature != nullptr) {
            signature->append(
                "L" + codemaker::convertString(nucleus).replace('.', '/'));
            if (!arguments.empty()) {
                signature->append('<');
                for (const OUString& arg : arguments)
                {
                    translateUnoTypeToDescriptor(
                        manager, arg, false, true, dependencies, nullptr, signature,
                        needsSignature, nullptr);
                }
                signature->append('>');
                *needsSignature = true;
            }
            signature->append(';');
        }
        return SPECIAL_TYPE_NONE;
    default:
        throw CannotDumpException(
            "unexpected nucleus \"" + nucleus
            + "\" in call to translateUnoTypeToDescriptor");
    }
}

SpecialType translateUnoTypeToDescriptor(
    rtl::Reference< TypeManager > const & manager, OUString const & type,
    bool array, bool classType, std::set<OUString> * dependencies,
    OStringBuffer * descriptor, OStringBuffer * signature,
    bool * needsSignature, PolymorphicUnoType * polymorphicUnoType)
{
    assert(manager.is());
    OUString nucleus;
    sal_Int32 rank;
    std::vector< OUString > args;
    codemaker::UnoType::Sort sort = manager->decompose(
        type, true, &nucleus, &rank, &args, nullptr);
    return translateUnoTypeToDescriptor(
        manager, sort, nucleus, rank, args, array, classType, dependencies,
        descriptor, signature, needsSignature, polymorphicUnoType);
}

SpecialType getFieldDescriptor(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    OUString const & type, OString * descriptor, OString * signature,
    PolymorphicUnoType * polymorphicUnoType)
{
    assert(descriptor != nullptr);
    OStringBuffer desc(64);
    OStringBuffer sig(64);
    bool needsSig = false;
    SpecialType specialType = translateUnoTypeToDescriptor(
        manager, type, false, false, dependencies, &desc, &sig, &needsSig,
        polymorphicUnoType);
    *descriptor = desc.makeStringAndClear();
    if (signature != nullptr) {
        if (needsSig) {
            *signature = sig.makeStringAndClear();
        } else {
            signature->clear();
        }
    }
    return specialType;
}

class MethodDescriptor {
public:
    MethodDescriptor(
        rtl::Reference< TypeManager > const & manager,
        std::set<OUString> * dependencies, OUString const & returnType,
        SpecialType * specialReturnType,
        PolymorphicUnoType * polymorphicUnoType);

    SpecialType addParameter(
        OUString const & type, bool array, bool dependency,
        PolymorphicUnoType * polymorphicUnoType);

    void addTypeParameter(OUString const & name);

    OString getDescriptor() const;

    OString getSignature() const { return m_needsSignature ? m_signatureStart.toString() + m_signatureEnd : OString();}

private:
    rtl::Reference< TypeManager > m_manager;
    std::set<OUString> * m_dependencies;
    OStringBuffer m_descriptorStart{16*1024};
    OString m_descriptorEnd;
    OStringBuffer m_signatureStart{16*1024};
    OString m_signatureEnd;
    bool m_needsSignature;
};

MethodDescriptor::MethodDescriptor(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    OUString const & returnType, SpecialType * specialReturnType,
    PolymorphicUnoType * polymorphicUnoType):
    m_manager(manager), m_dependencies(dependencies), m_needsSignature(false)
{
    assert(dependencies != nullptr);
    m_descriptorStart.append('(');
    m_signatureStart.append('(');
    OStringBuffer descEnd(128);
    descEnd.append(')');
    OStringBuffer sigEnd(128);
    sigEnd.append(')');
    SpecialType special = translateUnoTypeToDescriptor(
        m_manager, returnType, false, false, m_dependencies, &descEnd, &sigEnd,
        &m_needsSignature, polymorphicUnoType);
    m_descriptorEnd = descEnd.makeStringAndClear();
    m_signatureEnd = sigEnd.makeStringAndClear();
    if (specialReturnType != nullptr) {
        *specialReturnType = special;
    }
}

SpecialType MethodDescriptor::addParameter(
    OUString const & type, bool array, bool dependency,
    PolymorphicUnoType * polymorphicUnoType)
{
    return translateUnoTypeToDescriptor(
        m_manager, type, array, false, dependency ? m_dependencies : nullptr,
        &m_descriptorStart, &m_signatureStart, &m_needsSignature,
        polymorphicUnoType);
}

void MethodDescriptor::addTypeParameter(OUString const & name) {
    m_descriptorStart.append("Ljava/lang/Object;");
    m_signatureStart.append("T" + codemaker::convertString(name) + ";");
    m_needsSignature = true;
}

OString MethodDescriptor::getDescriptor() const {
    return m_descriptorStart + m_descriptorEnd;
}


class TypeInfo {
public:
    enum Kind { KIND_MEMBER, KIND_ATTRIBUTE, KIND_METHOD, KIND_PARAMETER };

    // Same values as in com/sun/star/lib/uno/typeinfo/TypeInfo.java:
    enum Flags {
        FLAG_READONLY = 0x008, FLAG_BOUND = 0x100
    };

    // KIND_MEMBER:
    TypeInfo(
        OString const & name, SpecialType specialType, sal_Int32 index,
        PolymorphicUnoType const & polymorphicUnoType,
        sal_Int32 typeParameterIndex);

    // KIND_ATTRIBUTE/METHOD:
    TypeInfo(
        Kind kind, OString const & name, SpecialType specialType, Flags flags,
        sal_Int32 index, PolymorphicUnoType const & polymorphicUnoType);

    // KIND_PARAMETER:
    TypeInfo(
        OString const & parameterName, SpecialType specialType,
        bool inParameter, bool outParameter, OString const & methodName,
        sal_Int32 index, PolymorphicUnoType const & polymorphicUnoType);

    sal_uInt16 generateCode(ClassFile::Code & code, std::set<OUString> * dependencies)
        const;

    void generatePolymorphicUnoTypeCode(
        ClassFile::Code & code, std::set<OUString> * dependencies) const;

private:
    Kind m_kind;
    OString m_name;
    sal_Int32 m_flags;
    sal_Int32 m_index;
    OString m_methodName;
    PolymorphicUnoType m_polymorphicUnoType;
    sal_Int32 m_typeParameterIndex;
};

sal_Int32 translateSpecialTypeFlags(
    SpecialType specialType, bool inParameter, bool outParameter)
{
    static sal_Int32 const specialTypeFlags[SPECIAL_TYPE_INTERFACE + 1] = {
        0, 0x0040 /* ANY */, 0x0004 /* UNSIGNED */, 0x0080 /* INTERFACE */ };
    sal_Int32 flags = specialTypeFlags[specialType];
    if (inParameter) {
        flags |= 0x0001; /* IN */
    }
    if (outParameter) {
        flags |= 0x0002; /* OUT */
    }
    return flags;
}

TypeInfo::TypeInfo(
    OString const & name, SpecialType specialType, sal_Int32 index,
    PolymorphicUnoType const & polymorphicUnoType,
    sal_Int32 typeParameterIndex):
    m_kind(KIND_MEMBER), m_name(name),
    m_flags(translateSpecialTypeFlags(specialType, false, false)),
    m_index(index), m_polymorphicUnoType(polymorphicUnoType),
    m_typeParameterIndex(typeParameterIndex)
{
    assert(
        polymorphicUnoType.kind == PolymorphicUnoType::KIND_NONE
        ? typeParameterIndex >= -1 : typeParameterIndex == -1);
}

TypeInfo::TypeInfo(
    Kind kind, OString const & name, SpecialType specialType, Flags flags,
    sal_Int32 index, PolymorphicUnoType const & polymorphicUnoType):
    m_kind(kind), m_name(name),
    m_flags(flags | translateSpecialTypeFlags(specialType, false, false)),
    m_index(index), m_polymorphicUnoType(polymorphicUnoType),
    m_typeParameterIndex(0)
{
    assert(kind == KIND_ATTRIBUTE || kind == KIND_METHOD);
}

TypeInfo::TypeInfo(
    OString const & parameterName, SpecialType specialType, bool inParameter,
    bool outParameter, OString const & methodName, sal_Int32 index,
    PolymorphicUnoType const & polymorphicUnoType):
    m_kind(KIND_PARAMETER), m_name(parameterName),
    m_flags(translateSpecialTypeFlags(specialType, inParameter, outParameter)),
    m_index(index), m_methodName(methodName),
    m_polymorphicUnoType(polymorphicUnoType),
    m_typeParameterIndex(0)
{}

sal_uInt16 TypeInfo::generateCode(
    ClassFile::Code & code, std::set<OUString> * dependencies) const
{
    switch (m_kind) {
    case KIND_MEMBER:
        code.instrNew("com/sun/star/lib/uno/typeinfo/MemberTypeInfo");
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.loadIntegerConstant(m_typeParameterIndex);
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/MemberTypeInfo", "<init>",
                "(Ljava/lang/String;IILcom/sun/star/uno/Type;I)V");
            return 8;
        } else if (m_typeParameterIndex >= 0) {
            code.instrAconstNull();
            code.loadIntegerConstant(m_typeParameterIndex);
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/MemberTypeInfo", "<init>",
                "(Ljava/lang/String;IILcom/sun/star/uno/Type;I)V");
            return 6;
        } else {
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/MemberTypeInfo", "<init>",
                "(Ljava/lang/String;II)V");
            return 4;
        }
    case KIND_ATTRIBUTE:
        code.instrNew("com/sun/star/lib/uno/typeinfo/AttributeTypeInfo");
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/AttributeTypeInfo", "<init>",
                "(Ljava/lang/String;IILcom/sun/star/uno/Type;)V");
            return 8;
        } else {
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/AttributeTypeInfo", "<init>",
                "(Ljava/lang/String;II)V");
            return 4;
        }
    case KIND_METHOD:
        code.instrNew("com/sun/star/lib/uno/typeinfo/MethodTypeInfo");
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/MethodTypeInfo", "<init>",
                "(Ljava/lang/String;IILcom/sun/star/uno/Type;)V");
            return 8;
        } else {
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/MethodTypeInfo", "<init>",
                "(Ljava/lang/String;II)V");
            return 4;
        }
    case KIND_PARAMETER:
        code.instrNew("com/sun/star/lib/uno/typeinfo/ParameterTypeInfo");
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadStringConstant(m_methodName);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/ParameterTypeInfo", "<init>",
                ("(Ljava/lang/String;Ljava/lang/String;II"
                 "Lcom/sun/star/uno/Type;)V"));
            return 9;
        } else {
            code.instrInvokespecial(
                "com/sun/star/lib/uno/typeinfo/ParameterTypeInfo", "<init>",
                "(Ljava/lang/String;Ljava/lang/String;II)V");
            return 5;
        }
    default:
        assert(false);
        return 0;
    }
}

void TypeInfo::generatePolymorphicUnoTypeCode(
    ClassFile::Code & code, std::set<OUString> * dependencies) const
{
    assert(dependencies != nullptr);
    assert(m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE);
    code.instrNew("com/sun/star/uno/Type");
    code.instrDup();
    code.loadStringConstant(
        codemaker::convertString(m_polymorphicUnoType.name));
    if (m_polymorphicUnoType.kind == PolymorphicUnoType::KIND_STRUCT) {
        code.instrGetstatic(
            "com/sun/star/uno/TypeClass", "STRUCT",
            "Lcom/sun/star/uno/TypeClass;");
    } else {
        code.instrGetstatic(
            "com/sun/star/uno/TypeClass", "SEQUENCE",
            "Lcom/sun/star/uno/TypeClass;");
    }
    dependencies->insert("com.sun.star.uno.TypeClass");
    code.instrInvokespecial(
        "com/sun/star/uno/Type", "<init>",
        "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V");
}

void writeClassFile(
    JavaOptions const & options, OString const & type,
    ClassFile const & classFile)
{
    OString path;
    if (options.isValid("-O")) {
        path = options.getOption("-O");
    }
    OString filename(createFileNameFromType(path, type, ".class"));
    bool bCheck = false;
    if (fileExists(filename)) {
        if (options.isValid("-G")) {
            return;
        }
        bCheck = options.isValid("-Gc");
    }
    FileStream tempfile;
    tempfile.createTempFile(getTempDir(filename));
    if (!tempfile.isValid()) {
        throw CannotDumpException(
            "Cannot create temporary file for " + b2u(filename));
    }
    OString tempname(tempfile.getName());
    try {
        classFile.write(tempfile);
    } catch (...) {
        // Remove existing file for consistency:
        if (fileExists(filename)) {
            removeTypeFile(filename);
        }
        tempfile.close();
        removeTypeFile(tempname);
        throw;
    }
    tempfile.close();
    if (!makeValidTypeFile(filename, tempname, bCheck)) {
        throw CannotDumpException(
            "Cannot create " + b2u(filename) + " from temporary file "
            + b2u(tempname));
    }
}

void addTypeInfo(
    OString const & className, std::vector< TypeInfo > const & typeInfo,
    std::set<OUString> * dependencies, ClassFile * classFile)
{
    assert(classFile != nullptr);
    std::vector< TypeInfo >::size_type typeInfos = typeInfo.size();
    if (typeInfos > SAL_MAX_INT32) {
        throw CannotDumpException(
            "UNOTYPEINFO array too big for Java class file format");
    }
    if (typeInfos == 0)
        return;

    classFile->addField(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
            | ClassFile::ACC_FINAL),
        "UNOTYPEINFO", "[Lcom/sun/star/lib/uno/typeinfo/TypeInfo;",
        0, "");
    std::unique_ptr< ClassFile::Code > code(classFile->newCode());
    code->loadIntegerConstant(static_cast< sal_Int32 >(typeInfos));
    code->instrAnewarray("com/sun/star/lib/uno/typeinfo/TypeInfo");
    sal_Int32 index = 0;
    sal_uInt16 stack = 0;
    for (const TypeInfo& ti : typeInfo)
    {
        code->instrDup();
        code->loadIntegerConstant(index++);
        stack = std::max(stack, ti.generateCode(*code, dependencies));
        code->instrAastore();
    }
    code->instrPutstatic(
        className, "UNOTYPEINFO",
        "[Lcom/sun/star/lib/uno/typeinfo/TypeInfo;");
    code->instrReturn();
    if (stack > SAL_MAX_UINT16 - 4) {
        throw CannotDumpException(
            "Stack too big for Java class file format");
    }
    code->setMaxStackAndLocals(static_cast< sal_uInt16 >(stack + 4), 0);
    classFile->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC),
        "<clinit>", "()V", code.get(), std::vector< OString >(), "");
}

void handleEnumType(
    const OUString& name, rtl::Reference< unoidl::EnumTypeEntity > const & entity,
    JavaOptions const & options)
{
    assert(entity.is());
    OString className(codemaker::convertString(name).replace('.', '/'));
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className, "com/sun/star/uno/Enum", ""));
    OString classDescriptor("L" + className + ";");
    for (const unoidl::EnumTypeEntity::Member& member : entity->getMembers())
    {
        OString fieldName(codemaker::convertString(member.name));
        cf->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            fieldName, classDescriptor, 0, OString());
        cf->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            fieldName + "_value", "I",
            cf->addIntegerInfo(member.value), "");
    }
    std::unique_ptr< ClassFile::Code > code(cf->newCode());
    code->loadLocalReference(0);
    code->loadLocalInteger(1);
    code->instrInvokespecial("com/sun/star/uno/Enum", "<init>", "(I)V");
    code->instrReturn();
    code->setMaxStackAndLocals(2, 2);
    cf->addMethod(
        ClassFile::ACC_PRIVATE,
        "<init>", "(I)V", code.get(),
        std::vector< OString >(), "");
    code = cf->newCode();
    code->instrGetstatic(
        className,
        codemaker::convertString(entity->getMembers()[0].name),
        classDescriptor);
    code->instrAreturn();
    code->setMaxStackAndLocals(1, 0);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        "getDefault", "()" + classDescriptor,
        code.get(), std::vector< OString >(), "");
    code = cf->newCode();
    code->loadLocalInteger(0);
    std::map< sal_Int32, OString > map;
    sal_Int32 min = SAL_MAX_INT32;
    sal_Int32 max = SAL_MIN_INT32;
    for (const unoidl::EnumTypeEntity::Member& member : entity->getMembers())
    {
        min = std::min(min, member.value);
        max = std::max(max, member.value);
        map.emplace(member.value, codemaker::convertString(member.name));
    }
    sal_uInt64 size = static_cast< sal_uInt64 >(map.size());
    if ((static_cast< sal_uInt64 >(max) - static_cast< sal_uInt64 >(min)
         <= 2 * size)
        || size > SAL_MAX_INT32)
    {
        std::unique_ptr< ClassFile::Code > defCode(cf->newCode());
        defCode->instrAconstNull();
        defCode->instrAreturn();
        std::vector< std::unique_ptr<ClassFile::Code> > blocks;
            //FIXME: pointers contained in blocks may leak
        sal_Int32 last = SAL_MAX_INT32;
        for (const auto& pair : map)
        {
            sal_Int32 value = pair.first;
            if (last != SAL_MAX_INT32) {
                for (sal_Int32 j = last + 1; j < value; ++j) {
                    blocks.push_back(nullptr);
                }
            }
            last = value;
            std::unique_ptr< ClassFile::Code > blockCode(cf->newCode());
            blockCode->instrGetstatic(className, pair.second, classDescriptor);
            blockCode->instrAreturn();
            blocks.push_back(std::move(blockCode));
        }
        code->instrTableswitch(defCode.get(), min, blocks);
    } else{
        std::unique_ptr< ClassFile::Code > defCode(cf->newCode());
        defCode->instrAconstNull();
        defCode->instrAreturn();
        std::vector< std::pair< sal_Int32, ClassFile::Code * > > blocks;
            //FIXME: pointers contained in blocks may leak
        for (const auto& pair : map )
        {
            std::unique_ptr< ClassFile::Code > blockCode(cf->newCode());
            blockCode->instrGetstatic(className, pair.second, classDescriptor);
            blockCode->instrAreturn();
            blocks.emplace_back(pair.first, blockCode.get());
            blockCode.release();
        }
        code->instrLookupswitch(defCode.get(), blocks);
        for (const std::pair< sal_Int32, ClassFile::Code * >& pair : blocks)
        {
            delete pair.second;
        }
    }
    code->setMaxStackAndLocals(1, 1);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        "fromInt", "(I)" + classDescriptor, code.get(),
        std::vector< OString >(), "");
    code = cf->newCode();
    for (const unoidl::EnumTypeEntity::Member& member : entity->getMembers())
    {
        code->instrNew(className);
        code->instrDup();
        code->loadIntegerConstant(member.value);
        code->instrInvokespecial(className, "<init>", "(I)V");
        code->instrPutstatic(
            className, codemaker::convertString(member.name), classDescriptor);
    }
    code->instrReturn();
    code->setMaxStackAndLocals(3, 0);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC),
        "<clinit>", "()V", code.get(), std::vector< OString >(), "");
    writeClassFile(options, className, *cf);
}

void addField(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    ClassFile * classFile, std::vector< TypeInfo > * typeInfo,
    sal_Int32 typeParameterIndex, OUString const & type, OUString const & name,
    sal_Int32 index)
{
    assert(classFile != nullptr);
    assert(typeInfo != nullptr);
    OString descriptor;
    OString signature;
    SpecialType specialType;
    PolymorphicUnoType polymorphicUnoType;
    if (typeParameterIndex >= 0) {
        descriptor = "Ljava/lang/Object;";
        signature = "T" + codemaker::convertString(type).replace('.', '/')
            + ";";
        specialType = SPECIAL_TYPE_NONE; //TODO: SPECIAL_TYPE_TYPE_PARAMETER?
    } else {
        specialType = getFieldDescriptor(
            manager, dependencies, type, &descriptor, &signature,
            &polymorphicUnoType);
    }
    classFile->addField(
        ClassFile::ACC_PUBLIC, codemaker::convertString(name), descriptor, 0,
        signature);
    typeInfo->push_back(
        TypeInfo(
            codemaker::convertString(name), specialType, index,
            polymorphicUnoType, typeParameterIndex));
}

sal_uInt16 addFieldInit(
    rtl::Reference< TypeManager > const & manager, OString const & className,
    OUString const & fieldName, bool typeParameter, OUString const & fieldType,
    std::set<OUString> * dependencies, ClassFile::Code * code)
{
    assert(manager.is());
    assert(code != nullptr);
    if (typeParameter) {
        return 0;
    }
    OString name(codemaker::convertString(fieldName));
    OUString nucleus;
    sal_Int32 rank;
    std::vector< OUString > args;
    rtl::Reference< unoidl::Entity > ent;
    codemaker::UnoType::Sort sort = manager->decompose(
        fieldType, true, &nucleus, &rank, &args, &ent);
    if (rank == 0) {
        switch (sort) {
        case codemaker::UnoType::Sort::Boolean:
        case codemaker::UnoType::Sort::Byte:
        case codemaker::UnoType::Sort::Short:
        case codemaker::UnoType::Sort::UnsignedShort:
        case codemaker::UnoType::Sort::Long:
        case codemaker::UnoType::Sort::UnsignedLong:
        case codemaker::UnoType::Sort::Hyper:
        case codemaker::UnoType::Sort::UnsignedHyper:
        case codemaker::UnoType::Sort::Float:
        case codemaker::UnoType::Sort::Double:
        case codemaker::UnoType::Sort::Char:
        case codemaker::UnoType::Sort::Interface:
            return 0;
        case codemaker::UnoType::Sort::String:
            code->loadLocalReference(0);
            code->loadStringConstant(OString());
            code->instrPutfield(className, name, "Ljava/lang/String;");
            return 2;
        case codemaker::UnoType::Sort::Type:
            code->loadLocalReference(0);
            code->instrGetstatic(
                "com/sun/star/uno/Type", "VOID", "Lcom/sun/star/uno/Type;");
            code->instrPutfield(className, name, "Lcom/sun/star/uno/Type;");
            return 2;
        case codemaker::UnoType::Sort::Any:
            code->loadLocalReference(0);
            code->instrGetstatic(
                "com/sun/star/uno/Any", "VOID", "Lcom/sun/star/uno/Any;");
            code->instrPutfield(className, name, "Ljava/lang/Object;");
            return 2;
        case codemaker::UnoType::Sort::Enum:
            {
                rtl::Reference< unoidl::EnumTypeEntity > ent2(
                    dynamic_cast< unoidl::EnumTypeEntity * >(ent.get()));
                assert(ent2.is());
                code->loadLocalReference(0);
                OStringBuffer descBuf(128);
                translateUnoTypeToDescriptor(
                    manager, sort, nucleus, 0, std::vector< OUString >(), false,
                    false, dependencies, &descBuf, nullptr, nullptr, nullptr);
                OString desc(descBuf.makeStringAndClear());
                code->instrGetstatic(
                    codemaker::convertString(nucleus).replace('.', '/'),
                    codemaker::convertString(ent2->getMembers()[0].name), desc);
                code->instrPutfield(className, name, desc);
                return 2;
            }
        case codemaker::UnoType::Sort::PlainStruct:
        case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
            {
                code->loadLocalReference(0);
                code->instrNew(
                    codemaker::convertString(nucleus).replace('.', '/'));
                code->instrDup();
                code->instrInvokespecial(
                    codemaker::convertString(nucleus).replace('.', '/'),
                    "<init>", "()V");
                OStringBuffer desc(128);
                translateUnoTypeToDescriptor(
                    manager, sort, nucleus, 0, args, false, false, dependencies,
                    &desc, nullptr, nullptr, nullptr);
                code->instrPutfield(className, name, desc.makeStringAndClear());
                return 3;
            }
        case codemaker::UnoType::Sort::Sequence:
        case codemaker::UnoType::Sort::Typedef:
            for (;;) std::abort(); // this cannot happen
        default:
            throw CannotDumpException(
                "unexpected entity \"" + fieldType
                + "\" in call to addFieldInit");
        }
    }
    code->loadLocalReference(0);
    code->loadIntegerConstant(0);
    if (rank == 1) {
        if (sort >= codemaker::UnoType::Sort::Boolean
            && sort <= codemaker::UnoType::Sort::Char)
        {
            code->instrNewarray(sort);
        } else {
            code->instrAnewarray(
                codemaker::java::translateUnoToJavaType(
                    sort, codemaker::convertString(nucleus).replace('.', '/'),
                    false));
        }
    } else {
        OStringBuffer desc(128);
        translateUnoTypeToDescriptor(
            manager, sort, nucleus, rank - 1, std::vector< OUString >(), false,
            false, dependencies, &desc, nullptr, nullptr, nullptr);
        code->instrAnewarray(desc.makeStringAndClear());
    }
    OStringBuffer desc(128);
    translateUnoTypeToDescriptor(
        manager, sort, nucleus, rank, std::vector< OUString >(), false, false,
        dependencies, &desc, nullptr, nullptr, nullptr);
    code->instrPutfield(className, name, desc.makeStringAndClear());
    return 2;
}

sal_uInt16 addLoadLocal(
    rtl::Reference< TypeManager > const & manager, ClassFile::Code * code,
    sal_uInt16 * index, bool typeParameter, OUString const & type, bool any,
    std::set<OUString> * dependencies)
{
    assert(manager.is());
    assert(code != nullptr);
    assert(index != nullptr);
    assert(!(typeParameter && any));
    assert(dependencies != nullptr);
    sal_uInt16 stack = 1;
    sal_uInt16 size = 1;
    if (typeParameter) {
        code->loadLocalReference(*index);
        stack = size = 1;
    } else {
        OUString nucleus;
        sal_Int32 rank;
        std::vector< OUString > args;
        codemaker::UnoType::Sort sort = manager->decompose(
            type, true, &nucleus, &rank, &args, nullptr);
        if (rank == 0) {
            switch (sort) {
            case codemaker::UnoType::Sort::Boolean:
                if (any) {
                    code->instrNew("java/lang/Boolean");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Boolean", "<init>", "(Z)V");
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Byte:
                if (any) {
                    code->instrNew("java/lang/Byte");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Byte", "<init>", "(B)V");
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Short:
                if (any) {
                    code->instrNew("java/lang/Short");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Short", "<init>", "(S)V");
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::UnsignedShort:
                if (any) {
                    code->instrNew("com/sun/star/uno/Any");
                    code->instrDup();
                    code->instrGetstatic(
                        "com/sun/star/uno/Type", "UNSIGNED_SHORT",
                        "Lcom/sun/star/uno/Type;");
                    code->instrNew("java/lang/Short");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Short", "<init>", "(S)V");
                    code->instrInvokespecial(
                        "com/sun/star/uno/Any", "<init>",
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                    stack = 6;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Long:
                if (any) {
                    code->instrNew("java/lang/Integer");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Integer", "<init>", "(I)V");
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::UnsignedLong:
                if (any) {
                    code->instrNew("com/sun/star/uno/Any");
                    code->instrDup();
                    code->instrGetstatic(
                        "com/sun/star/uno/Type", "UNSIGNED_LONG",
                        "Lcom/sun/star/uno/Type;");
                    code->instrNew("java/lang/Integer");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Integer", "<init>", "(I)V");
                    code->instrInvokespecial(
                        "com/sun/star/uno/Any", "<init>",
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                    stack = 6;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Hyper:
                if (any) {
                    code->instrNew("java/lang/Long");
                    code->instrDup();
                    code->loadLocalLong(*index);
                    code->instrInvokespecial(
                        "java/lang/Long", "<init>", "(J)V");
                    stack = 4;
                } else {
                    code->loadLocalLong(*index);
                    stack = 2;
                }
                size = 2;
                break;
            case codemaker::UnoType::Sort::UnsignedHyper:
                if (any) {
                    code->instrNew("com/sun/star/uno/Any");
                    code->instrDup();
                    code->instrGetstatic(
                        "com/sun/star/uno/Type", "UNSIGNED_HYPER",
                        "Lcom/sun/star/uno/Type;");
                    code->instrNew("java/lang/Long");
                    code->instrDup();
                    code->loadLocalLong(*index);
                    code->instrInvokespecial(
                        "java/lang/Long", "<init>", "(J)V");
                    code->instrInvokespecial(
                        "com/sun/star/uno/Any", "<init>",
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                    stack = 7;
                } else {
                    code->loadLocalLong(*index);
                    stack = 2;
                }
                size = 2;
                break;
            case codemaker::UnoType::Sort::Float:
                if (any) {
                    code->instrNew("java/lang/Float");
                    code->instrDup();
                    code->loadLocalFloat(*index);
                    code->instrInvokespecial(
                        "java/lang/Float", "<init>", "(F)V");
                    stack = 3;
                } else {
                    code->loadLocalFloat(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Double:
                if (any) {
                    code->instrNew("java/lang/Double");
                    code->instrDup();
                    code->loadLocalDouble(*index);
                    code->instrInvokespecial(
                        "java/lang/Double", "<init>", "(D)V");
                    stack = 4;
                } else {
                    code->loadLocalDouble(*index);
                    stack = 2;
                }
                size = 2;
                break;
            case codemaker::UnoType::Sort::Char:
                if (any) {
                    code->instrNew("java/lang/Character");
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        "java/lang/Character", "<init>", "(C)V");
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::String:
            case codemaker::UnoType::Sort::Type:
            case codemaker::UnoType::Sort::Any:
                code->loadLocalReference(*index);
                stack = size = 1;
                break;
            case codemaker::UnoType::Sort::Enum:
                // Assuming that no Java types are derived from Java types that
                // are directly derived from com.sun.star.uno.Enum:
                code->loadLocalReference(*index);
                stack = size = 1;
                break;
            case codemaker::UnoType::Sort::PlainStruct:
            case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                if (any) {
                    code->instrNew("com/sun/star/uno/Any");
                    code->instrDup();
                    code->instrNew("com/sun/star/uno/Type");
                    code->instrDup();
                    code->loadStringConstant(
                        codemaker::convertString(
                            createUnoName(manager, nucleus, rank, args)));
                    code->instrGetstatic(
                        "com/sun/star/uno/TypeClass", "STRUCT",
                        "Lcom/sun/star/uno/TypeClass;");
                    dependencies->insert("com.sun.star.uno.TypeClass");
                    code->instrInvokespecial(
                        "com/sun/star/uno/Type", "<init>",
                        "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V");
                    code->loadLocalReference(*index);
                    code->instrInvokespecial(
                        "com/sun/star/uno/Any", "<init>",
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                    stack = 6;
                } else {
                    code->loadLocalReference(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Interface:
                if (any && nucleus != "com.sun.star.uno.XInterface") {
                    code->instrNew("com/sun/star/uno/Any");
                    code->instrDup();
                    code->instrNew("com/sun/star/uno/Type");
                    code->instrDup();
                    code->loadStringConstant(codemaker::convertString(nucleus));
                    code->instrGetstatic(
                        "com/sun/star/uno/TypeClass", "INTERFACE",
                        "Lcom/sun/star/uno/TypeClass;");
                    dependencies->insert("com.sun.star.uno.TypeClass");
                    code->instrInvokespecial(
                        "com/sun/star/uno/Type", "<init>",
                        "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V");
                    code->loadLocalReference(*index);
                    code->instrInvokespecial(
                        "com/sun/star/uno/Any", "<init>",
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                    stack = 6;
                } else {
                    code->loadLocalReference(*index);
                    stack = 1;
                }
                size = 1;
                break;
            case codemaker::UnoType::Sort::Sequence:
            case codemaker::UnoType::Sort::Typedef:
                for (;;) std::abort(); // this cannot happen
            default:
                throw CannotDumpException(
                    "unexpected entity \"" + type
                    + "\" in call to addLoadLocal");
            }
        } else {
            bool bWrap = false;
            if (any) {
                switch (sort) {
                case codemaker::UnoType::Sort::Boolean:
                case codemaker::UnoType::Sort::Byte:
                case codemaker::UnoType::Sort::Short:
                case codemaker::UnoType::Sort::Long:
                case codemaker::UnoType::Sort::Hyper:
                case codemaker::UnoType::Sort::Float:
                case codemaker::UnoType::Sort::Double:
                case codemaker::UnoType::Sort::Char:
                case codemaker::UnoType::Sort::String:
                case codemaker::UnoType::Sort::Type:
                        // assuming that no Java types are derived from
                        // com.sun.star.uno.Type
                case codemaker::UnoType::Sort::Enum:
                        // assuming that no Java types are derived from Java
                        // types that are directly derived from
                        // com.sun.star.uno.Enum
                    break;
                case codemaker::UnoType::Sort::UnsignedShort:
                case codemaker::UnoType::Sort::UnsignedLong:
                case codemaker::UnoType::Sort::UnsignedHyper:
                case codemaker::UnoType::Sort::Any:
                case codemaker::UnoType::Sort::PlainStruct:
                case codemaker::UnoType::Sort::InstantiatedPolymorphicStruct:
                case codemaker::UnoType::Sort::Interface:
                    bWrap = true;
                    break;
                case codemaker::UnoType::Sort::Sequence:
                case codemaker::UnoType::Sort::Typedef:
                    for (;;) std::abort(); // this cannot happen
                default:
                    throw CannotDumpException(
                        "unexpected entity \"" + type
                        + "\" in call to addLoadLocal");
                }
            }
            if (bWrap) {
                code->instrNew("com/sun/star/uno/Any");
                code->instrDup();
                code->instrNew("com/sun/star/uno/Type");
                code->instrDup();
                code->loadStringConstant(
                    codemaker::convertString(
                        createUnoName(manager, nucleus, rank, args)));
                code->instrInvokespecial(
                    "com/sun/star/uno/Type", "<init>", "(Ljava/lang/String;)V");
                code->loadLocalReference(*index);
                code->instrInvokespecial(
                    "com/sun/star/uno/Any", "<init>",
                    "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V");
                stack = 5;
            } else {
                code->loadLocalReference(*index);
                stack = 1;
            }
            size = 1;
        }
    }
    if (*index > SAL_MAX_UINT16 - size) {
        throw CannotDumpException(
            "Too many local variables for Java class file format");
    }
    *index = *index + size;
    return stack;
}

sal_uInt16 addDirectArgument(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    MethodDescriptor * methodDescriptor, ClassFile::Code * code,
    sal_uInt16 * index, OString const & className, OString const & fieldName,
    bool typeParameter, OUString const & fieldType)
{
    assert(methodDescriptor != nullptr);
    assert(code != nullptr);
    OString desc;
    if (typeParameter) {
        methodDescriptor->addTypeParameter(fieldType);
        desc = "Ljava/lang/Object;";
    } else {
        methodDescriptor->addParameter(fieldType, false, true, nullptr);
        getFieldDescriptor(manager, dependencies, fieldType, &desc, nullptr, nullptr);
    }
    code->loadLocalReference(0);
    sal_uInt16 stack = addLoadLocal(
        manager, code, index, typeParameter, fieldType, false, dependencies);
    code->instrPutfield(className, fieldName, desc);
    return stack + 1;
}

void addPlainStructBaseArguments(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    MethodDescriptor * methodDescriptor, ClassFile::Code * code,
    OUString const & base, sal_uInt16 * index)
{
    assert(manager.is());
    assert(methodDescriptor != nullptr);
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(base, &ent)
        != codemaker::UnoType::Sort::PlainStruct)
    {
        throw CannotDumpException(
            "unexpected entity \"" + base
            + "\" in call to addPlainStructBaseArguments");
    }
    unoidl::PlainStructTypeEntity& ent2(dynamic_cast<unoidl::PlainStructTypeEntity&>(*ent));
    if (!ent2.getDirectBase().isEmpty()) {
        addPlainStructBaseArguments(
            manager, dependencies, methodDescriptor, code,
            ent2.getDirectBase(), index);
    }
    for (const unoidl::PlainStructTypeEntity::Member& member : ent2.getDirectMembers())
    {
        methodDescriptor->addParameter(member.type, false, true, nullptr);
        addLoadLocal(manager, code, index, false, member.type, false, dependencies);
    }
}

void handlePlainStructType(
    const OUString& name,
    rtl::Reference< unoidl::PlainStructTypeEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(dependencies != nullptr);
    OString className(codemaker::convertString(name).replace('.', '/'));
    OString superClass;
    if (entity->getDirectBase().isEmpty()) {
        superClass = "java/lang/Object";
    } else {
        superClass = codemaker::convertString(entity->getDirectBase()).
            replace('.', '/');
        dependencies->insert(entity->getDirectBase());
    }
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_SUPER),
            className, superClass, ""));
    std::vector< TypeInfo > typeInfo;
    sal_Int32 index = 0;
    for (const unoidl::PlainStructTypeEntity::Member& member : entity->getDirectMembers())
    {
        addField(
            manager, dependencies, cf.get(), &typeInfo, -1, member.type, member.name,
            index++);
    }
    std::unique_ptr< ClassFile::Code > code(cf->newCode());
    code->loadLocalReference(0);
    code->instrInvokespecial(superClass, "<init>", "()V");
    sal_uInt16 stack = 0;
    for (const unoidl::PlainStructTypeEntity::Member& member : entity->getDirectMembers())
    {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, member.name, false, member.type, dependencies,
                code.get()));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 1, 1);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "()V", code.get(),
        std::vector< OString >(), "");
    MethodDescriptor desc(manager, dependencies, "void", nullptr, nullptr);
    code = cf->newCode();
    code->loadLocalReference(0);
    sal_uInt16 index2 = 1;
    if (!entity->getDirectBase().isEmpty()) {
        addPlainStructBaseArguments(
            manager, dependencies, &desc, code.get(), entity->getDirectBase(),
            &index2);
    }
    code->instrInvokespecial(superClass, "<init>", desc.getDescriptor());
    sal_uInt16 maxSize = index2;
    for (const unoidl::PlainStructTypeEntity::Member& member : entity->getDirectMembers())
    {
        maxSize = std::max(
            maxSize,
            addDirectArgument(
                manager, dependencies, &desc, code.get(), &index2, className,
                codemaker::convertString(member.name), false, member.type));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(maxSize, index2);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", desc.getDescriptor(), code.get(),
        std::vector< OString >(), desc.getSignature());
    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf);
}

void handlePolyStructType(
    const OUString& name,
    rtl::Reference< unoidl::PolymorphicStructTypeTemplateEntity > const &
        entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    OString className(codemaker::convertString(name).replace('.', '/'));
    std::map< OUString, sal_Int32 > typeParameters;
    OStringBuffer sig(128);
    sig.append("<");
    sal_Int32 index = 0;
    for (const OUString& param : entity->getTypeParameters())
    {
        sig.append(codemaker::convertString(param) + ":Ljava/lang/Object;");
        if (!typeParameters.emplace(param, index++).second)
        {
            throw CannotDumpException("Bad type information"); //TODO
        }
    }
    sig.append(">Ljava/lang/Object;");
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_SUPER),
            className, "java/lang/Object", sig.makeStringAndClear()));
    std::vector< TypeInfo > typeInfo;
    index = 0;
    for (const unoidl::PolymorphicStructTypeTemplateEntity::Member& member : entity->getMembers())
    {
        sal_Int32 typeParameterIndex;
        if (member.parameterized) {
            std::map< OUString, sal_Int32 >::iterator it(
                typeParameters.find(member.type));
            if (it == typeParameters.end()) {
                throw CannotDumpException("Bad type information"); //TODO
            }
            typeParameterIndex = it->second;
        } else {
            typeParameterIndex = -1;
        }
        addField(
            manager, dependencies, cf.get(), &typeInfo, typeParameterIndex,
            member.type, member.name, index++);
    }
    std::unique_ptr< ClassFile::Code > code(cf->newCode());
    code->loadLocalReference(0);
    code->instrInvokespecial("java/lang/Object", "<init>", "()V");
    sal_uInt16 stack = 0;
    for (const unoidl::PolymorphicStructTypeTemplateEntity::Member& member : entity->getMembers())
    {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, member.name, member.parameterized, member.type,
                dependencies, code.get()));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 1, 1);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "()V", code.get(),
        std::vector< OString >(), "");
    MethodDescriptor desc(manager, dependencies, "void", nullptr, nullptr);
    code = cf->newCode();
    code->loadLocalReference(0);
    sal_uInt16 index2 = 1;
    code->instrInvokespecial(
        "java/lang/Object", "<init>", desc.getDescriptor());
    sal_uInt16 maxSize = index2;
    for (const unoidl::PolymorphicStructTypeTemplateEntity::Member& member : entity->getMembers())
    {
        maxSize = std::max(
            maxSize,
            addDirectArgument(
                manager, dependencies, &desc, code.get(), &index2, className,
                codemaker::convertString(member.name), member.parameterized, member.type));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(maxSize, index2);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", desc.getDescriptor(), code.get(),
        std::vector< OString >(), desc.getSignature());
    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf);
}

void addExceptionBaseArguments(
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies,
    MethodDescriptor * methodDescriptor, ClassFile::Code * code,
    OUString const & base, sal_uInt16 * index)
{
    assert(manager.is());
    assert(methodDescriptor != nullptr);
    rtl::Reference< unoidl::Entity > ent;
    if (manager->getSort(base, &ent) != codemaker::UnoType::Sort::Exception)
    {
        throw CannotDumpException(
            "unexpected entity \"" + base
            + "\" in call to addExceptionBaseArguments");
    }
    unoidl::ExceptionTypeEntity& ent2(dynamic_cast<unoidl::ExceptionTypeEntity&>(*ent));
    bool baseException = base == "com.sun.star.uno.Exception";
    if (!baseException) {
        addExceptionBaseArguments(
            manager, dependencies, methodDescriptor, code,
            ent2.getDirectBase(), index);
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             ent2.getDirectMembers().begin());
         i != ent2.getDirectMembers().end(); ++i)
    {
        if (!baseException || i != ent2.getDirectMembers().begin()) {
            methodDescriptor->addParameter(i->type, false, true, nullptr);
            addLoadLocal(
                manager, code, index, false, i->type, false, dependencies);
        }
    }
}

void handleExceptionType(
    const OUString& name, rtl::Reference< unoidl::ExceptionTypeEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(dependencies != nullptr);
    OString className(codemaker::convertString(name).replace('.', '/'));
    bool baseException = false;
    bool baseRuntimeException = false;
    OString superClass;
    if (className == "com/sun/star/uno/Exception") {
        baseException = true;
        superClass = "java/lang/Exception";
    } else if (className == "com/sun/star/uno/RuntimeException") {
        baseRuntimeException = true;
        superClass = "java/lang/RuntimeException";
    } else {
        if (entity->getDirectBase().isEmpty()) {
            throw CannotDumpException(
                "Exception type \"" + name + "\" lacks base");
        }
        superClass = codemaker::convertString(entity->getDirectBase()).
            replace('.', '/');
        dependencies->insert(entity->getDirectBase());
    }
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_SUPER),
            className, superClass, ""));
    std::vector< TypeInfo > typeInfo;
    sal_Int32 index = 0;
    if (baseRuntimeException) {
        addField(
            manager, dependencies, cf.get(), &typeInfo, -1,
            "com.sun.star.uno.XInterface", "Context", index++);
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            addField(
                manager, dependencies, cf.get(), &typeInfo, -1, i->type,
                i->name, index++);
        }
    }

    // create default constructor
    std::unique_ptr< ClassFile::Code > code(cf->newCode());
    code->loadLocalReference(0);
    code->instrInvokespecial(superClass, "<init>", "()V");
    sal_uInt16 stack = 0;
    if (baseRuntimeException) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, "Context", false,
                "com.sun.star.uno.XInterface", dependencies, code.get()));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className, i->name, false, i->type, dependencies,
                    code.get()));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 1, 1);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "()V", code.get(),
        std::vector< OString >(), "");


    // create (Throwable Cause) constructor
    code = cf->newCode();
    code->loadLocalReference(0);
    code->loadLocalReference(1);
    code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/Throwable;)V");
    stack = 0;
    if (baseRuntimeException) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, "Context", false,
                "com.sun.star.uno.XInterface", dependencies, code.get()));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className, i->name, false, i->type, dependencies,
                    code.get()));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 2, 2);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "(Ljava/lang/Throwable;)V", code.get(),
        std::vector< OString >(), "");

    // create (Throwable Cause, String Message) constructor
    code = cf->newCode();
    code->loadLocalReference(0);
    if (baseException || baseRuntimeException) {
        code->loadLocalReference(2);
        code->loadLocalReference(1);
        code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/String;Ljava/lang/Throwable;)V");
    } else {
        code->loadLocalReference(1);
        code->loadLocalReference(2);
        code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/Throwable;Ljava/lang/String;)V");
    }
    stack = 0;
    if (baseRuntimeException) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, "Context", false,
                "com.sun.star.uno.XInterface", dependencies, code.get()));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className, i->name, false, i->type, dependencies,
                    code.get()));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 3, 3);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "(Ljava/lang/Throwable;Ljava/lang/String;)V", code.get(),
        std::vector< OString >(), "");

    // create (String Message) constructor
    code = cf->newCode();
    code->loadLocalReference(0);
    code->loadLocalReference(1);
    code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/String;)V");
    stack = 0;
    if (baseRuntimeException) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className, "Context", false,
                "com.sun.star.uno.XInterface", dependencies, code.get()));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className, i->name, false, i->type, dependencies,
                    code.get()));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 2, 2);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "(Ljava/lang/String;)V", code.get(),
        std::vector< OString >(), "");


    // create (String Message, Object Context, T1 m1, ..., Tn mn) constructor
    MethodDescriptor desc1(manager, dependencies, "void", nullptr, nullptr);
    code = cf->newCode();
    code->loadLocalReference(0);
    sal_uInt16 index2 = 1;
    code->loadLocalReference(index2++);
    desc1.addParameter("string", false, true, nullptr);
    if (!(baseException || baseRuntimeException)) {
        addExceptionBaseArguments(
            manager, dependencies, &desc1, code.get(), entity->getDirectBase(),
            &index2);
    }
    code->instrInvokespecial(superClass, "<init>", desc1.getDescriptor());
    sal_uInt16 maxSize = index2;
    if (baseRuntimeException) {
        maxSize = std::max(
            maxSize,
            addDirectArgument(
                manager, dependencies, &desc1, code.get(), &index2, className,
                "Context", false, "com.sun.star.uno.XInterface"));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            maxSize = std::max(
                maxSize,
                addDirectArgument(
                    manager, dependencies, &desc1, code.get(), &index2,
                    className, codemaker::convertString(i->name), false,
                    i->type));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(maxSize, index2);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", desc1.getDescriptor(), code.get(),
        std::vector< OString >(), desc1.getSignature());

    // create (Throwable Cause, String Message, Object Context, T1 m1, ..., Tn mn) constructor
    MethodDescriptor desc2(manager, dependencies, "void", nullptr, nullptr);
    code = cf->newCode();
    code->loadLocalReference(0);
    sal_uInt16 index3 = 3;
    // Note that we hack in the java.lang.Throwable parameter further down,
    // because MethodDescriptor does not know how to handle it.
    desc2.addParameter("string", false, true, nullptr);
    if (baseException || baseRuntimeException) {
        code->loadLocalReference(2);
        code->loadLocalReference(1);
        code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/String;Ljava/lang/Throwable;)V");
    } else {
        code->loadLocalReference(1);
        code->loadLocalReference(2);
        addExceptionBaseArguments(
            manager, dependencies, &desc2, code.get(), entity->getDirectBase(),
            &index3);
        code->instrInvokespecial(superClass, "<init>", "(Ljava/lang/Throwable;" + desc2.getDescriptor().copy(1));
    }
    sal_uInt16 maxSize2 = index3;
    if (baseRuntimeException) {
        maxSize2 = std::max(
            maxSize2,
            addDirectArgument(
                manager, dependencies, &desc2, code.get(), &index3, className,
                "Context", false, "com.sun.star.uno.XInterface"));
    }
    for (std::vector< unoidl::ExceptionTypeEntity::Member >::const_iterator i(
             entity->getDirectMembers().begin());
         i != entity->getDirectMembers().end(); ++i)
    {
        if (!baseException || i != entity->getDirectMembers().begin()) {
            maxSize2 = std::max(
                maxSize2,
                addDirectArgument(
                    manager, dependencies, &desc2, code.get(), &index3,
                    className, codemaker::convertString(i->name), false,
                    i->type));
        }
    }
    code->instrReturn();
    code->setMaxStackAndLocals(maxSize2, index3);
    cf->addMethod(
        ClassFile::ACC_PUBLIC, "<init>", "(Ljava/lang/Throwable;" + desc2.getDescriptor().copy(1), code.get(),
        std::vector< OString >(), desc2.getSignature());

    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf);
}

void createExceptionsAttribute(
    rtl::Reference< TypeManager > const & manager,
    std::vector< OUString > const & exceptionTypes,
    std::set<OUString> * dependencies, std::vector< OString > * exceptions,
    codemaker::ExceptionTree * tree)
{
    assert(dependencies != nullptr);
    assert(exceptions != nullptr);
    for (const OUString& ex : exceptionTypes)
    {
        dependencies->insert(ex);
        OString type(codemaker::convertString(ex).replace('.', '/'));
        exceptions->push_back(type);
        if (tree != nullptr) {
            tree->add(type.replace('/', '.'), manager);
        }
    }
}

void handleInterfaceType(
    const OUString& name, rtl::Reference< unoidl::InterfaceTypeEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(dependencies != nullptr);
    OString className(codemaker::convertString(name).replace('.', '/'));
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_INTERFACE
                | ClassFile::ACC_ABSTRACT),
            className, "java/lang/Object", ""));
    for (const unoidl::AnnotatedReference& ar : entity->getDirectMandatoryBases())
    {
        dependencies->insert(ar.name);
        cf->addInterface(codemaker::convertString(ar.name).replace('.', '/'));
    }
    // As a special case, let com.sun.star.lang.XEventListener extend
    // java.util.EventListener ("A tagging interface that all event listener
    // interfaces must extend"):
    if (className == "com/sun/star/lang/XEventListener") {
        cf->addInterface("java/util/EventListener");
    }
    std::vector< TypeInfo > typeInfo;
    if (className != "com/sun/star/uno/XInterface") {
        sal_Int32 index = 0;
        for (const unoidl::InterfaceTypeEntity::Attribute& attr : entity->getDirectAttributes())
        {
            SpecialType specialType;
            PolymorphicUnoType polymorphicUnoType;
            MethodDescriptor gdesc(
                manager, dependencies, attr.type, &specialType,
                &polymorphicUnoType);
            std::vector< OString > exc;
            createExceptionsAttribute(
                manager, attr.getExceptions, dependencies, &exc, nullptr);
            OString attrName(codemaker::convertString(attr.name));
            cf->addMethod(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
                "get" + attrName, gdesc.getDescriptor(), nullptr, exc,
                gdesc.getSignature());
            if (!attr.readOnly) {
                MethodDescriptor sdesc(manager, dependencies, "void", nullptr, nullptr);
                sdesc.addParameter(attr.type, false, true, nullptr);
                std::vector< OString > exc2;
                createExceptionsAttribute(
                    manager, attr.setExceptions, dependencies, &exc2, nullptr);
                cf->addMethod(
                    static_cast< ClassFile::AccessFlags >(
                        ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
                    "set" + attrName, sdesc.getDescriptor(), nullptr, exc2,
                    sdesc.getSignature());
            }
            typeInfo.emplace_back(
                    TypeInfo::KIND_ATTRIBUTE, attrName, specialType,
                    static_cast< TypeInfo::Flags >(
                        (attr.readOnly ? TypeInfo::FLAG_READONLY : 0)
                        | (attr.bound ? TypeInfo::FLAG_BOUND : 0)),
                    index, polymorphicUnoType);
            index += (attr.readOnly ? 1 : 2);
        }
        for (const unoidl::InterfaceTypeEntity::Method& method : entity->getDirectMethods())
        {
            OString methodName(codemaker::convertString(method.name));
            SpecialType specialReturnType;
            PolymorphicUnoType polymorphicUnoReturnType;
            MethodDescriptor desc(
                manager, dependencies, method.returnType, &specialReturnType,
                &polymorphicUnoReturnType);
            typeInfo.emplace_back(
                    TypeInfo::KIND_METHOD, methodName, specialReturnType,
                    static_cast< TypeInfo::Flags >(0), index++,
                    polymorphicUnoReturnType);
            sal_Int32 paramIndex = 0;
            for (const unoidl::InterfaceTypeEntity::Method::Parameter& param : method.parameters)
            {
                bool in = param.direction
                    != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_OUT;
                bool out = param.direction
                    != unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN;
                PolymorphicUnoType polymorphicUnoType;
                SpecialType specialType = desc.addParameter(
                    param.type, out, true, &polymorphicUnoType);
                if (out || isSpecialType(specialType)
                    || polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE)
                {
                    typeInfo.emplace_back(
                            codemaker::convertString(param.name), specialType, in,
                            out, methodName, paramIndex, polymorphicUnoType);
                }
                ++paramIndex;
            }
            std::vector< OString > exc2;
            createExceptionsAttribute(
                manager, method.exceptions, dependencies, &exc2, nullptr);
            cf->addMethod(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
                methodName, desc.getDescriptor(), nullptr, exc2, desc.getSignature());
        }
    }
    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf);
}

void handleTypedef(
    rtl::Reference< unoidl::TypedefEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(manager.is());
    assert(dependencies != nullptr);
    OUString nucleus;
    switch (manager->decompose(entity->getType(), false, &nucleus, nullptr, nullptr, nullptr))
    {
    case codemaker::UnoType::Sort::Boolean:
    case codemaker::UnoType::Sort::Byte:
    case codemaker::UnoType::Sort::Short:
    case codemaker::UnoType::Sort::UnsignedShort:
    case codemaker::UnoType::Sort::Long:
    case codemaker::UnoType::Sort::UnsignedLong:
    case codemaker::UnoType::Sort::Hyper:
    case codemaker::UnoType::Sort::UnsignedHyper:
    case codemaker::UnoType::Sort::Float:
    case codemaker::UnoType::Sort::Double:
    case codemaker::UnoType::Sort::Char:
    case codemaker::UnoType::Sort::String:
    case codemaker::UnoType::Sort::Type:
    case codemaker::UnoType::Sort::Any:
        break;
    case codemaker::UnoType::Sort::Enum:
    case codemaker::UnoType::Sort::PlainStruct:
    case codemaker::UnoType::Sort::Interface:
    case codemaker::UnoType::Sort::Typedef:
        dependencies->insert(nucleus);
        break;
    default:
        assert(false); // this cannot happen
    }
}

void handleConstantGroup(
    const OUString& name, rtl::Reference< unoidl::ConstantGroupEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    OString className(codemaker::convertString(name).replace('.', '/'));
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_INTERFACE
                | ClassFile::ACC_ABSTRACT),
            className, "java/lang/Object", ""));
    for (const unoidl::ConstantGroupEntity::Member& member : entity->getMembers())
    {
        OUString type;
        sal_uInt16 valueIndex = sal_uInt16(); // avoid false warnings
        switch (member.value.type) {
        case unoidl::ConstantValue::TYPE_BOOLEAN:
            type = "boolean";
            valueIndex = cf->addIntegerInfo(sal_Int32(member.value.booleanValue));
            break;
        case unoidl::ConstantValue::TYPE_BYTE:
            type = "byte";
            valueIndex = cf->addIntegerInfo(member.value.byteValue);
            break;
        case unoidl::ConstantValue::TYPE_SHORT:
            type = "short";
            valueIndex = cf->addIntegerInfo(member.value.shortValue);
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_SHORT:
            type = "unsigned short";
            valueIndex = cf->addIntegerInfo(member.value.unsignedShortValue);
            break;
        case unoidl::ConstantValue::TYPE_LONG:
            type = "long";
            valueIndex = cf->addIntegerInfo(member.value.longValue);
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_LONG:
            type = "unsigned long";
            valueIndex = cf->addIntegerInfo(
                static_cast< sal_Int32 >(member.value.unsignedLongValue));
            break;
        case unoidl::ConstantValue::TYPE_HYPER:
            type = "hyper";
            valueIndex = cf->addLongInfo(member.value.hyperValue);
            break;
        case unoidl::ConstantValue::TYPE_UNSIGNED_HYPER:
            type = "unsigned hyper";
            valueIndex = cf->addLongInfo(
                static_cast< sal_Int64 >(member.value.unsignedHyperValue));
            break;
        case unoidl::ConstantValue::TYPE_FLOAT:
            type = "float";
            valueIndex = cf->addFloatInfo(member.value.floatValue);
            break;
        case unoidl::ConstantValue::TYPE_DOUBLE:
            type = "double";
            valueIndex = cf->addDoubleInfo(member.value.doubleValue);
            break;
        }
        OString desc;
        OString sig;
        getFieldDescriptor(manager, dependencies, type, &desc, &sig, nullptr);
        cf->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            codemaker::convertString(member.name), desc, valueIndex, sig);
    }
    writeClassFile(options, className, *cf);
}

void addExceptionHandlers(
    codemaker::ExceptionTreeNode const * node,
    ClassFile::Code::Position start, ClassFile::Code::Position end,
    ClassFile::Code::Position handler, ClassFile::Code * code)
{
    assert(node != nullptr);
    assert(code != nullptr);
    if (node->present) {
        code->addException(start, end, handler, node->name.replace('.', '/'));
    } else {
        for (std::unique_ptr<codemaker::ExceptionTreeNode> const & p : node->children)
        {
            addExceptionHandlers(p.get(), start, end, handler, code);
        }
    }
}

void addConstructor(
    rtl::Reference< TypeManager > const & manager,
    OString const & realJavaBaseName, OString const & unoName,
    OString const & className,
    unoidl::SingleInterfaceBasedServiceEntity::Constructor const & constructor,
    OUString const & returnType, std::set<OUString> * dependencies,
    ClassFile * classFile)
{
    assert(dependencies != nullptr);
    assert(classFile != nullptr);
    MethodDescriptor desc(manager, dependencies, returnType, nullptr, nullptr);
    desc.addParameter("com.sun.star.uno.XComponentContext", false, false, nullptr);
    std::unique_ptr< ClassFile::Code > code(classFile->newCode());
    code->loadLocalReference(0);
    // stack: context
    code->instrInvokeinterface(
        "com/sun/star/uno/XComponentContext", "getServiceManager",
        "()Lcom/sun/star/lang/XMultiComponentFactory;", 1);
    // stack: factory
    code->loadStringConstant(unoName);
    // stack: factory serviceName
    codemaker::ExceptionTree tree;
    ClassFile::Code::Position tryStart;
    ClassFile::Code::Position tryEnd;
    std::vector< OString > exc;
    sal_uInt16 stack;
    sal_uInt16 localIndex = 1;
    ClassFile::AccessFlags access = static_cast< ClassFile::AccessFlags >(
        ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC);
    if (constructor.defaultConstructor) {
        code->loadLocalReference(0);
        // stack: factory serviceName context
        tryStart = code->getPosition();
        code->instrInvokeinterface(
            "com/sun/star/lang/XMultiComponentFactory",
            "createInstanceWithContext",
            ("(Ljava/lang/String;Lcom/sun/star/uno/XComponentContext;)"
             "Ljava/lang/Object;"),
            3);
        tryEnd = code->getPosition();
        // stack: instance
        stack = 3;
    } else {
        if (constructor.parameters.size() == 1
            && constructor.parameters[0].rest)
        {
            desc.addParameter("any", true, true, nullptr);
            code->loadLocalReference(localIndex++);
            // stack: factory serviceName args
            stack = 4;
            access = static_cast< ClassFile::AccessFlags >(
                access | ClassFile::ACC_VARARGS);
        } else {
            code->loadIntegerConstant(constructor.parameters.size());
            // stack: factory serviceName N
            code->instrAnewarray("java/lang/Object");
            // stack: factory serviceName args
            stack = 0;
            sal_Int32 n = 0;
            for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor::Parameter& param :
                         constructor.parameters)
            {
                desc.addParameter(param.type, false, true, nullptr);
                code->instrDup();
                // stack: factory serviceName args args
                code->loadIntegerConstant(n++);
                // stack: factory serviceName args args i
                stack = std::max(
                    stack,
                    addLoadLocal(
                        manager, code.get(), &localIndex, false, param.type, true,
                        dependencies));
                // stack: factory serviceName args args i any
                code->instrAastore();
                // stack: factory serviceName args
            }
            stack += 5;
        }
        code->loadLocalReference(0);
        // stack: factory serviceName args context
        tryStart = code->getPosition();
        code->instrInvokeinterface(
            "com/sun/star/lang/XMultiComponentFactory",
            "createInstanceWithArgumentsAndContext",
            ("(Ljava/lang/String;[Ljava/lang/Object;"
             "Lcom/sun/star/uno/XComponentContext;)Ljava/lang/Object;"),
            4);
        tryEnd = code->getPosition();
        // stack: instance
        createExceptionsAttribute(
            manager, constructor.exceptions, dependencies, &exc, &tree);
    }
    code->loadLocalReference(0);
    // stack: instance context
    code->instrInvokestatic(
        className, "$castInstance",
        ("(Ljava/lang/Object;Lcom/sun/star/uno/XComponentContext;)"
         "Ljava/lang/Object;"));
    // stack: instance
    code->instrCheckcast(
        codemaker::convertString(returnType).replace('.', '/'));
    // stack: instance
    code->instrAreturn();
    if (!tree.getRoot().present) {
        ClassFile::Code::Position pos1 = code->getPosition();
        // stack: e
        code->instrInvokevirtual(
            "java/lang/Throwable", "toString", "()Ljava/lang/String;");
        // stack: str
        localIndex = std::max< sal_uInt16 >(localIndex, 2);
        code->storeLocalReference(1);
        // stack: -
        code->instrNew("com/sun/star/uno/DeploymentException");
        // stack: ex
        code->instrDup();
        // stack: ex ex
        code->loadStringConstant(
            "component context fails to supply service " + unoName + " of type "
            + realJavaBaseName + ": ");
        // stack: ex ex "..."
        code->loadLocalReference(1);
        // stack: ex ex "..." str
        code->instrInvokevirtual(
            "java/lang/String", "concat",
            "(Ljava/lang/String;)Ljava/lang/String;");
        // stack: ex ex "..."
        code->loadLocalReference(0);
        // stack: ex ex "..." context
        code->instrInvokespecial(
            "com/sun/star/uno/DeploymentException", "<init>",
            "(Ljava/lang/String;Ljava/lang/Object;)V");
        // stack: ex
        ClassFile::Code::Position pos2 = code->getPosition();
        code->instrAthrow();
        addExceptionHandlers(
            &tree.getRoot(), tryStart, tryEnd, pos2, code.get());
        code->addException(
            tryStart, tryEnd, pos1, "com/sun/star/uno/Exception");
        dependencies->insert("com.sun.star.uno.Exception");
        stack = std::max< sal_uInt16 >(stack, 4);
    }
    code->setMaxStackAndLocals(stack, localIndex);
    classFile->addMethod(
        access,
        codemaker::java::translateUnoToJavaIdentifier(
            (constructor.defaultConstructor
             ? OString("create") : codemaker::convertString(constructor.name)),
            "method"),
        desc.getDescriptor(), code.get(), exc, desc.getSignature());
}

void handleService(
    const OUString& name,
    rtl::Reference< unoidl::SingleInterfaceBasedServiceEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(dependencies != nullptr);
    OString unoName(codemaker::convertString(name));
    OString className(
        translateUnoidlEntityNameToJavaFullyQualifiedName(name, "service"));
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className, "java/lang/Object", ""));
    if (!entity->getConstructors().empty()) {
        OString realJavaBaseName(
            codemaker::convertString(entity->getBase()));
        dependencies->insert(entity->getBase());
        dependencies->insert("com.sun.star.lang.XMultiComponentFactory");
        dependencies->insert("com.sun.star.uno.DeploymentException");
        dependencies->insert("com.sun.star.uno.TypeClass");
        dependencies->insert("com.sun.star.uno.XComponentContext");
        for (const unoidl::SingleInterfaceBasedServiceEntity::Constructor& cons :
                 entity->getConstructors())
        {
            addConstructor(
                manager, realJavaBaseName, unoName, className, cons,
                entity->getBase(), dependencies, cf.get());
        }
        // Synthetic castInstance method:
        {
            std::unique_ptr< ClassFile::Code > code(cf->newCode());
            code->instrNew("com/sun/star/uno/Type");
            // stack: type
            code->instrDup();
            // stack: type type
            code->loadStringConstant(realJavaBaseName);
            // stack: type type "..."
            code->instrGetstatic(
                "com/sun/star/uno/TypeClass", "INTERFACE",
                "Lcom/sun/star/uno/TypeClass;");
            // stack: type type "..." INTERFACE
            code->instrInvokespecial(
                "com/sun/star/uno/Type", "<init>",
                "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V");
            // stack: type
            code->loadLocalReference(0);
            // stack: type instance
            code->instrInvokestatic(
                "com/sun/star/uno/UnoRuntime", "queryInterface",
                ("(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                 "Ljava/lang/Object;"));
            // stack: instance
            code->instrDup();
            // stack: instance instance
            ClassFile::Code::Branch branch = code->instrIfnull();
            // stack: instance
            code->instrAreturn();
            code->branchHere(branch);
            code->instrPop();
            // stack: -
            code->instrNew("com/sun/star/uno/DeploymentException");
            // stack: ex
            code->instrDup();
            // stack: ex ex
            code->loadStringConstant(
                "component context fails to supply service " + unoName
                + " of type " + realJavaBaseName);
            // stack: ex ex "..."
            code->loadLocalReference(1);
            // stack: ex ex "..." context
            code->instrInvokespecial(
                "com/sun/star/uno/DeploymentException", "<init>",
                "(Ljava/lang/String;Ljava/lang/Object;)V");
            // stack: ex
            code->instrAthrow();
            code->setMaxStackAndLocals(4, 2);
            cf->addMethod(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC
                    | ClassFile::ACC_SYNTHETIC),
                "$castInstance",
                ("(Ljava/lang/Object;Lcom/sun/star/uno/XComponentContext;)"
                 "Ljava/lang/Object;"),
                code.get(), std::vector< OString >(), "");
        }
    }
    writeClassFile(options, className, *cf);
}

void handleSingleton(
    const OUString& name,
    rtl::Reference< unoidl::InterfaceBasedSingletonEntity > const & entity,
    rtl::Reference< TypeManager > const & manager, JavaOptions const & options,
    std::set<OUString> * dependencies)
{
    assert(entity.is());
    assert(dependencies != nullptr);
    OString realJavaBaseName(codemaker::convertString(entity->getBase()));
    OString base(realJavaBaseName.replace('.', '/'));
    dependencies->insert(entity->getBase());
    OString unoName(codemaker::convertString(name));
    OString className(
        translateUnoidlEntityNameToJavaFullyQualifiedName(name, "singleton"));
    dependencies->insert("com.sun.star.uno.DeploymentException");
    dependencies->insert("com.sun.star.uno.TypeClass");
    dependencies->insert("com.sun.star.uno.XComponentContext");
    std::unique_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className, "java/lang/Object", ""));
    MethodDescriptor desc(manager, dependencies, entity->getBase(), nullptr, nullptr);
    desc.addParameter("com.sun.star.uno.XComponentContext", false, false, nullptr);
    std::unique_ptr< ClassFile::Code > code(cf->newCode());
    code->loadLocalReference(0);
    // stack: context
    code->loadStringConstant("/singletons/" + unoName);
    // stack: context "..."
    code->instrInvokeinterface(
        "com/sun/star/uno/XComponentContext", "getValueByName",
        "(Ljava/lang/String;)Ljava/lang/Object;", 2);
    // stack: value
    code->instrDup();
    // stack: value value
    code->instrInstanceof("com/sun/star/uno/Any");
    // stack: value 0/1
    ClassFile::Code::Branch branch1 = code->instrIfeq();
    // stack: value
    code->instrCheckcast("com/sun/star/uno/Any");
    // stack: value
    code->instrDup();
    // stack: value value
    code->instrInvokevirtual(
        "com/sun/star/uno/Any", "getType", "()Lcom/sun/star/uno/Type;");
    // stack: value type
    code->instrInvokevirtual(
        "com/sun/star/uno/Type", "getTypeClass",
        "()Lcom/sun/star/uno/TypeClass;");
    // stack: value typeClass
    code->instrGetstatic(
        "com/sun/star/uno/TypeClass", "INTERFACE",
        "Lcom/sun/star/uno/TypeClass;");
    // stack: value typeClass INTERFACE
    ClassFile::Code::Branch branch2 = code->instrIfAcmpne();
    // stack: value
    code->instrInvokevirtual(
        "com/sun/star/uno/Any", "getObject", "()Ljava/lang/Object;");
    // stack: value
    code->branchHere(branch1);
    code->instrNew("com/sun/star/uno/Type");
    // stack: value type
    code->instrDup();
    // stack: value type type
    code->loadStringConstant(realJavaBaseName);
    // stack: value type type "..."
    code->instrGetstatic(
        "com/sun/star/uno/TypeClass", "INTERFACE",
        "Lcom/sun/star/uno/TypeClass;");
    // stack: value type type "..." INTERFACE
    code->instrInvokespecial(
        "com/sun/star/uno/Type", "<init>",
        "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V");
    // stack: value type
    code->instrSwap();
    // stack: type value
    code->instrInvokestatic(
        "com/sun/star/uno/UnoRuntime", "queryInterface",
        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)Ljava/lang/Object;");
    // stack: instance
    code->instrDup();
    // stack: instance instance
    ClassFile::Code::Branch branch3 = code->instrIfnull();
    // stack: instance
    code->instrCheckcast(base);
    // stack: instance
    code->instrAreturn();
    code->branchHere(branch2);
    code->branchHere(branch3);
    code->instrPop();
    // stack: -
    code->instrNew("com/sun/star/uno/DeploymentException");
    // stack: ex
    code->instrDup();
    // stack: ex ex
    code->loadStringConstant(
        "component context fails to supply singleton " + unoName + " of type "
        + realJavaBaseName);
    // stack: ex ex "..."
    code->loadLocalReference(0);
    // stack: ex ex "..." context
    code->instrInvokespecial(
        "com/sun/star/uno/DeploymentException", "<init>",
        "(Ljava/lang/String;Ljava/lang/Object;)V");
    // stack: ex
    code->instrAthrow();
    code->setMaxStackAndLocals(5, 1);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        "get", desc.getDescriptor(), code.get(), std::vector< OString >(),
        desc.getSignature());
    writeClassFile(options, className, *cf);
}

}

void produce(
    OUString const & name, rtl::Reference< TypeManager > const & manager,
    codemaker::GeneratedTypeSet & generated, JavaOptions const & options)
{
    if (generated.contains(u2b(name))) {
        return;
    }
    generated.add(u2b(name));
    if (!manager->foundAtPrimaryProvider(name)) {
        return;
    }
    std::set<OUString> deps;
    rtl::Reference< unoidl::Entity > ent;
    rtl::Reference< unoidl::MapCursor > cur;
    switch (manager->getSort(name, &ent, &cur)) {
    case codemaker::UnoType::Sort::Module:
        {
            OUString prefix;
            if (!name.isEmpty()) {
                prefix = name + ".";
            }
            for (;;) {
                OUString mem;
                if (!cur->getNext(&mem).is()) {
                    break;
                }
                produce(prefix + mem, manager, generated, options);
            }
            return;
        }
    case codemaker::UnoType::Sort::Enum:
        handleEnumType(
            name, dynamic_cast< unoidl::EnumTypeEntity * >(ent.get()), options);
        break;
    case codemaker::UnoType::Sort::PlainStruct:
        handlePlainStructType(
            name, dynamic_cast< unoidl::PlainStructTypeEntity * >(ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::PolymorphicStructTemplate:
        handlePolyStructType(
            name,
            dynamic_cast< unoidl::PolymorphicStructTypeTemplateEntity * >(
                ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::Exception:
        handleExceptionType(
            name, dynamic_cast< unoidl::ExceptionTypeEntity * >(ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::Interface:
        handleInterfaceType(
            name, dynamic_cast< unoidl::InterfaceTypeEntity * >(ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::Typedef:
        handleTypedef(
            dynamic_cast< unoidl::TypedefEntity * >(ent.get()), manager, &deps);
        break;
    case codemaker::UnoType::Sort::ConstantGroup:
        handleConstantGroup(
            name, dynamic_cast< unoidl::ConstantGroupEntity * >(ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::SingleInterfaceBasedService:
        handleService(
            name,
            dynamic_cast< unoidl::SingleInterfaceBasedServiceEntity * >(
                ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::InterfaceBasedSingleton:
        handleSingleton(
            name,
            dynamic_cast< unoidl::InterfaceBasedSingletonEntity * >(ent.get()),
            manager, options, &deps);
        break;
    case codemaker::UnoType::Sort::AccumulationBasedService:
    case codemaker::UnoType::Sort::ServiceBasedSingleton:
        break;
    default:
        throw CannotDumpException(
            "unexpected entity \"" + name + "\" in call to produce");
    }
    if (!options.isValid("-nD")) {
        for (const OUString& d : deps) {
            produce(d, manager, generated, options);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
