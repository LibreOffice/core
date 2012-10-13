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


#include "javatype.hxx"

#include "classfile.hxx"
#include "javaoptions.hxx"

#include "codemaker/exceptiontree.hxx"
#include "codemaker/generatedtypeset.hxx"
#include "codemaker/global.hxx"
#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"
#include "codemaker/commonjava.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/refltype.hxx"
#include "registry/types.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textcvt.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

using codemaker::javamaker::ClassFile;

namespace {

// helper function for createUnoName
void appendUnoName(
    TypeManager const & manager, rtl::OString const & nucleus, sal_Int32 rank,
    std::vector< rtl::OString > const & arguments, rtl::OStringBuffer * buffer)
{
    OSL_ASSERT(rank >= 0 && buffer != 0);
    for (sal_Int32 i = 0; i < rank; ++i) {
        buffer->append(RTL_CONSTASCII_STRINGPARAM("[]"));
    }
    buffer->append(nucleus.replace('/', '.'));
    if (!arguments.empty()) {
        buffer->append('<');
        for (std::vector< rtl::OString >::const_iterator i(arguments.begin());
             i != arguments.end(); ++i)
        {
            if (i != arguments.begin()) {
                buffer->append(',');
            }
            RTTypeClass argTypeClass;
            rtl::OString argNucleus;
            sal_Int32 argRank;
            std::vector< rtl::OString > argArgs;
            codemaker::decomposeAndResolve(
                manager, *i, true, false, false, &argTypeClass, &argNucleus,
                &argRank, &argArgs);
            appendUnoName(manager, argNucleus, argRank, argArgs, buffer);
        }
        buffer->append('>');
    }
}

// Translate the name of a UNO type registry entity (enum type, plain struct
// type, polymorphic struct type template, or interface type, decomposed into
// nucleus, rank, and arguments) into a core UNO type name:
rtl::OString createUnoName(
    TypeManager const & manager, rtl::OString const & nucleus, sal_Int32 rank,
    std::vector< rtl::OString > const & arguments)
{
    rtl::OStringBuffer buf;
    appendUnoName(manager, nucleus, rank, arguments, &buf);
    return buf.makeStringAndClear();
}

/**
   Set of UTF-8--encoded names of UNO type registry entities a given UNO type
   registry entity depends on.

   UNO type registry entities are enum types, plain struct types, polymorphic
   struct type templates, exception types, interface types, typedefs, modules,
   constant groupds, single-interface--based services, accumulation-based
   services, interface-based singletons, and service-based singletons.
 */
typedef std::set< rtl::OString > Dependencies;

enum SpecialType {
    SPECIAL_TYPE_NONE,
    SPECIAL_TYPE_ANY,
    SPECIAL_TYPE_UNSIGNED,
    SPECIAL_TYPE_INTERFACE
};

bool isSpecialType(SpecialType special) {
    return special >= SPECIAL_TYPE_UNSIGNED;
}

rtl::OString translateUnoTypeToJavaFullyQualifiedName(
    rtl::OString const & type, rtl::OString const & prefix)
{
    sal_Int32 i = type.lastIndexOf('/') + 1;
    return type.copy(0, i) +
        codemaker::java::translateUnoToJavaIdentifier(type.copy(i), prefix);
}

struct PolymorphicUnoType {
    PolymorphicUnoType(): kind(KIND_NONE) {}

    enum Kind { KIND_NONE, KIND_STRUCT, KIND_SEQUENCE };
    Kind kind;
    rtl::OString name;
};

SpecialType translateUnoTypeToDescriptor(
    TypeManager const & manager, rtl::OString const & type, bool array,
    bool classType, Dependencies * dependencies,
    rtl::OStringBuffer * descriptor, rtl::OStringBuffer * signature,
    bool * needsSignature, PolymorphicUnoType * polymorphicUnoType);

SpecialType translateUnoTypeToDescriptor(
    TypeManager const & manager, codemaker::UnoType::Sort sort,
    RTTypeClass typeClass, rtl::OString const & nucleus, sal_Int32 rank,
    std::vector< rtl::OString > const & arguments, bool array, bool classType,
    Dependencies * dependencies, rtl::OStringBuffer * descriptor,
    rtl::OStringBuffer * signature, bool * needsSignature,
    PolymorphicUnoType * polymorphicUnoType)
{
    OSL_ASSERT(rank >= 0 && (signature == 0) == (needsSignature == 0));
    if (rank > 0xFF - (array ? 1 : 0)) {
        throw CannotDumpException(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "Too many array dimensions for Java class file format")));
    }
    if (array) {
        ++rank;
    }
    for (sal_Int32 i = 0; i < rank; ++i) {
        if (descriptor != 0) {
            descriptor->append('[');
        }
        if (signature != 0) {
            signature->append('[');
        }
    }
    if (sort == codemaker::UnoType::SORT_COMPLEX) {
        //TODO: check that nucleus is a valid (Java-modified UTF-8) identifier
        if (typeClass == RT_TYPE_INTERFACE
            && (nucleus
                == rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XInterface"))))
        {
            if (descriptor != 0) {
                descriptor->append(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;")));
            }
            if (signature != 0) {
                signature->append(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;")));
            }
            if (polymorphicUnoType != 0) {
                polymorphicUnoType->kind = PolymorphicUnoType::KIND_NONE;
            }
            return SPECIAL_TYPE_INTERFACE;
        } else {
            if (dependencies != 0) {
                dependencies->insert(nucleus);
            }
            if (descriptor != 0) {
                descriptor->append('L');
                descriptor->append(nucleus);
                descriptor->append(';');
            }
            if (signature != 0) {
                signature->append('L');
                signature->append(nucleus);
                if (!arguments.empty()) {
                    signature->append('<');
                    for (std::vector< rtl::OString >::const_iterator i(
                             arguments.begin());
                         i != arguments.end(); ++i)
                    {
                        translateUnoTypeToDescriptor(
                            manager, *i, false, true, dependencies, 0,
                            signature, needsSignature, 0);
                    }
                    signature->append('>');
                    *needsSignature = true;
                }
                signature->append(';');
            }
            if (polymorphicUnoType != 0) {
                if (arguments.empty()) {
                    polymorphicUnoType->kind = PolymorphicUnoType::KIND_NONE;
                } else {
                    polymorphicUnoType->kind = rank == 0
                        ? PolymorphicUnoType::KIND_STRUCT
                        : PolymorphicUnoType::KIND_SEQUENCE;
                    polymorphicUnoType->name = createUnoName(
                        manager, nucleus, rank, arguments);
                }
            }
            return SPECIAL_TYPE_NONE;
        }
    } else {
        static rtl::OString const
            simpleTypeDescriptors[codemaker::UnoType::SORT_ANY + 1][2] = {
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("V")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Void;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("Z")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Boolean;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("B")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Byte;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("S")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Short;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("S")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Short;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("I")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Integer;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("I")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Integer;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("J")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Long;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("J")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Long;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("F")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Float;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("D")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Double;"))
                },
                { rtl::OString(RTL_CONSTASCII_STRINGPARAM("C")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Character;"))
                },
                { rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/String;")),
                  rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/String;"))
                },
                { rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Type;")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Type;"))
                },
                { rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;")),
                  rtl::OString(
                      RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;"))
                } };
        rtl::OString const & s
            = simpleTypeDescriptors[sort][rank == 0 && classType];
        if (descriptor != 0) {
            descriptor->append(s);
        }
        if (signature != 0) {
            signature->append(s);
        }
        if (polymorphicUnoType != 0) {
            polymorphicUnoType->kind = PolymorphicUnoType::KIND_NONE;
        }
        static SpecialType const
            simpleTypeSpecials[codemaker::UnoType::SORT_ANY + 1] = {
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_UNSIGNED, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_UNSIGNED, SPECIAL_TYPE_NONE, SPECIAL_TYPE_UNSIGNED,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE,
                SPECIAL_TYPE_NONE, SPECIAL_TYPE_NONE, SPECIAL_TYPE_ANY };
        return simpleTypeSpecials[sort];
    }
}

SpecialType translateUnoTypeToDescriptor(
    TypeManager const & manager, rtl::OString const & type, bool array,
    bool classType, Dependencies * dependencies,
    rtl::OStringBuffer * descriptor, rtl::OStringBuffer * signature,
    bool * needsSignature, PolymorphicUnoType * polymorphicUnoType)
{
    RTTypeClass typeClass;
    rtl::OString nucleus;
    sal_Int32 rank;
    std::vector< rtl::OString > args;
    codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
        manager, type, true, true, false, &typeClass, &nucleus, &rank, &args);
    OSL_ASSERT(rank < SAL_MAX_INT32);
    return translateUnoTypeToDescriptor(
        manager, sort, typeClass, nucleus, rank, args, array, classType,
        dependencies, descriptor, signature, needsSignature,
        polymorphicUnoType);
}

SpecialType getFieldDescriptor(
    TypeManager const & manager, Dependencies * dependencies,
    rtl::OString const & type, rtl::OString * descriptor,
    rtl::OString * signature, PolymorphicUnoType * polymorphicUnoType)
{
    OSL_ASSERT(dependencies != 0 && descriptor != 0);
    rtl::OStringBuffer desc;
    rtl::OStringBuffer sig;
    bool needsSig = false;
    SpecialType specialType = translateUnoTypeToDescriptor(
        manager, type, false, false, dependencies, &desc, &sig, &needsSig,
        polymorphicUnoType);
    *descriptor = desc.makeStringAndClear();
    if (signature != 0) {
        if (needsSig) {
            *signature = sig.makeStringAndClear();
        } else {
            *signature = rtl::OString();
        }
    }
    return specialType;
}

class MethodDescriptor {
public:
    MethodDescriptor(
        TypeManager const & manager, Dependencies * dependencies,
        rtl::OString const & returnType, SpecialType * specialReturnType,
        PolymorphicUnoType * polymorphicUnoType);

    SpecialType addParameter(
        rtl::OString const & type, bool array, bool dependency,
        PolymorphicUnoType * polymorphicUnoType);

    void addTypeParameter(rtl::OString const & name);

    rtl::OString getDescriptor() const;

    rtl::OString getSignature() const;

private:
    TypeManager const & m_manager;
    Dependencies * m_dependencies;
    rtl::OStringBuffer m_descriptorStart;
    rtl::OString m_descriptorEnd;
    rtl::OStringBuffer m_signatureStart;
    rtl::OString m_signatureEnd;
    bool m_needsSignature;
};

MethodDescriptor::MethodDescriptor(
    TypeManager const & manager, Dependencies * dependencies,
    rtl::OString const & returnType, SpecialType * specialReturnType,
    PolymorphicUnoType * polymorphicUnoType):
    m_manager(manager), m_dependencies(dependencies), m_needsSignature(false)
{
    OSL_ASSERT(dependencies != 0);
    m_descriptorStart.append('(');
    m_signatureStart.append('(');
    rtl::OStringBuffer descEnd;
    descEnd.append(')');
    rtl::OStringBuffer sigEnd;
    sigEnd.append(')');
    SpecialType special = translateUnoTypeToDescriptor(
        m_manager, returnType, false, false, m_dependencies, &descEnd, &sigEnd,
        &m_needsSignature, polymorphicUnoType);
    m_descriptorEnd = descEnd.makeStringAndClear();
    m_signatureEnd = sigEnd.makeStringAndClear();
    if (specialReturnType != 0) {
        *specialReturnType = special;
    }
}

SpecialType MethodDescriptor::addParameter(
    rtl::OString const & type, bool array, bool dependency,
    PolymorphicUnoType * polymorphicUnoType)
{
    return translateUnoTypeToDescriptor(
        m_manager, type, array, false, dependency ? m_dependencies : 0,
        &m_descriptorStart, &m_signatureStart, &m_needsSignature,
        polymorphicUnoType);
}

void MethodDescriptor::addTypeParameter(rtl::OString const & name) {
    m_descriptorStart.append(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;"));
    m_signatureStart.append('T');
    m_signatureStart.append(name);
    m_signatureStart.append(';');
    m_needsSignature = true;
}

rtl::OString MethodDescriptor::getDescriptor() const {
    rtl::OStringBuffer buf(m_descriptorStart);
    buf.append(m_descriptorEnd);
    return buf.makeStringAndClear();
}

rtl::OString MethodDescriptor::getSignature() const {
    if (m_needsSignature) {
        rtl::OStringBuffer buf(m_signatureStart);
        buf.append(m_signatureEnd);
        return buf.makeStringAndClear();
    } else {
        return rtl::OString();
    }
}

class TypeInfo {
public:
    enum Kind { KIND_MEMBER, KIND_ATTRIBUTE, KIND_METHOD, KIND_PARAMETER };

    // Same values as in com/sun/star/lib/uno/typeinfo/TypeInfo.java:
    enum Flags {
        FLAG_READONLY = 0x008, FLAG_BOUND = 0x100, FLAG_ONEWAY = 0x010
    };

    // KIND_MEMBER:
    TypeInfo(
        rtl::OString const & name, SpecialType specialType, sal_Int32 index,
        PolymorphicUnoType const & polymorphicUnoType,
        sal_Int32 typeParameterIndex);

    // KIND_ATTRIBUTE/METHOD:
    TypeInfo(
        Kind kind, rtl::OString const & name, SpecialType specialType,
        Flags flags, sal_Int32 index,
        PolymorphicUnoType const & polymorphicUnoType);

    // KIND_PARAMETER:
    TypeInfo(
        rtl::OString const & parameterName, SpecialType specialType,
        bool inParameter, bool outParameter, rtl::OString const & methodName,
        sal_Int32 index, PolymorphicUnoType const & polymorphicUnoType);

    sal_uInt16 generateCode(ClassFile::Code & code, Dependencies * dependencies)
        const;

    void generatePolymorphicUnoTypeCode(
        ClassFile::Code & code, Dependencies * dependencies) const;

private:
    Kind m_kind;
    rtl::OString m_name;
    sal_Int32 m_flags;
    sal_Int32 m_index;
    rtl::OString m_methodName;
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
    rtl::OString const & name, SpecialType specialType, sal_Int32 index,
    PolymorphicUnoType const & polymorphicUnoType,
    sal_Int32 typeParameterIndex):
    m_kind(KIND_MEMBER), m_name(name),
    m_flags(translateSpecialTypeFlags(specialType, false, false)),
    m_index(index), m_polymorphicUnoType(polymorphicUnoType),
    m_typeParameterIndex(typeParameterIndex)
{
    OSL_ASSERT(
        polymorphicUnoType.kind == PolymorphicUnoType::KIND_NONE
        ? typeParameterIndex >= -1 : typeParameterIndex == -1);
}

TypeInfo::TypeInfo(
    Kind kind, rtl::OString const & name, SpecialType specialType,
    Flags flags, sal_Int32 index,
    PolymorphicUnoType const & polymorphicUnoType):
    m_kind(kind), m_name(name),
    m_flags(flags | translateSpecialTypeFlags(specialType, false, false)),
    m_index(index), m_polymorphicUnoType(polymorphicUnoType)
{
    OSL_ASSERT(kind == KIND_ATTRIBUTE || kind == KIND_METHOD);
}

TypeInfo::TypeInfo(
    rtl::OString const & parameterName, SpecialType specialType,
    bool inParameter, bool outParameter, rtl::OString const & methodName,
    sal_Int32 index, PolymorphicUnoType const & polymorphicUnoType):
    m_kind(KIND_PARAMETER), m_name(parameterName),
    m_flags(translateSpecialTypeFlags(specialType, inParameter, outParameter)),
    m_index(index), m_methodName(methodName),
    m_polymorphicUnoType(polymorphicUnoType)
{}

sal_uInt16 TypeInfo::generateCode(
    ClassFile::Code & code, Dependencies * dependencies) const
{
    OSL_ASSERT(dependencies != 0);
    switch (m_kind) {
    case KIND_MEMBER:
        code.instrNew(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lib/uno/typeinfo/MemberTypeInfo")));
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.loadIntegerConstant(m_typeParameterIndex);
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/MemberTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;IILcom/sun/star/uno/Type;I)V")));
            return 8;
        } else if (m_typeParameterIndex >= 0) {
            code.instrAconstNull();
            code.loadIntegerConstant(m_typeParameterIndex);
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/MemberTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;IILcom/sun/star/uno/Type;I)V")));
            return 6;
        } else {
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/MemberTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;II)V")));
            return 4;
        }

    case KIND_ATTRIBUTE:
        code.instrNew(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lib/uno/typeinfo/AttributeTypeInfo")));
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/AttributeTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;IILcom/sun/star/uno/Type;)V")));
            return 8;
        } else {
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/AttributeTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;II)V")));
            return 4;
        }

    case KIND_METHOD:
        code.instrNew(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lib/uno/typeinfo/MethodTypeInfo")));
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/MethodTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;IILcom/sun/star/uno/Type;)V")));
            return 8;
        } else {
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/MethodTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;II)V")));
            return 4;
        }

    case KIND_PARAMETER:
        code.instrNew(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lib/uno/typeinfo/ParameterTypeInfo")));
        code.instrDup();
        code.loadStringConstant(m_name);
        code.loadStringConstant(m_methodName);
        code.loadIntegerConstant(m_index);
        code.loadIntegerConstant(m_flags);
        if (m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE) {
            generatePolymorphicUnoTypeCode(code, dependencies);
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/ParameterTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;Ljava/lang/String;II"
                        "Lcom/sun/star/uno/Type;)V")));
            return 9;
        } else {
            code.instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/lib/uno/typeinfo/ParameterTypeInfo")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;Ljava/lang/String;II)V")));
            return 5;
        }

    default:
        OSL_ASSERT(false);
        return 0;
    }
}

void TypeInfo::generatePolymorphicUnoTypeCode(
    ClassFile::Code & code, Dependencies * dependencies) const
{
    OSL_ASSERT(
        dependencies != 0
        && m_polymorphicUnoType.kind != PolymorphicUnoType::KIND_NONE);
    code.instrNew(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")));
    code.instrDup();
    code.loadStringConstant(m_polymorphicUnoType.name);
    if (m_polymorphicUnoType.kind == PolymorphicUnoType::KIND_STRUCT) {
        code.instrGetstatic(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("STRUCT")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/TypeClass;")));
    } else {
        code.instrGetstatic(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("SEQUENCE")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/TypeClass;")));
    }
    dependencies->insert(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")));
    code.instrInvokespecial(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V")));
}

void writeClassFile(
    JavaOptions /*TODO const*/ & options, rtl::OString const & type,
    ClassFile const & classFile)
{
    rtl::OString path;
    if (options.isValid(rtl::OString(RTL_CONSTASCII_STRINGPARAM("-O")))) {
        path = options.getOption(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("-O")));
    }
    rtl::OString filename(
        createFileNameFromType(
            path, type, rtl::OString(RTL_CONSTASCII_STRINGPARAM(".class"))));
    bool check = false;
    if (fileExists(filename)) {
        if (options.isValid(rtl::OString(RTL_CONSTASCII_STRINGPARAM("-G")))) {
            return;
        }
        check = options.isValid(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("-Gc")));
    }
    FileStream tempfile;
    tempfile.createTempFile(getTempDir(filename));
    if (!tempfile.isValid()) {
        throw CannotDumpException(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("Cannot create temporary file for "))
            + filename);
    }
    rtl::OString tempname(tempfile.getName());
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
    if (!makeValidTypeFile(filename, tempname, check)) {
        rtl::OStringBuffer msg;
        msg.append(RTL_CONSTASCII_STRINGPARAM("Cannot create "));
        msg.append(filename);
        msg.append(RTL_CONSTASCII_STRINGPARAM(" from temporary file "));
        msg.append(tempname);
        throw CannotDumpException(msg.makeStringAndClear());
    }
}

void addTypeInfo(
    rtl::OString const & className, std::vector< TypeInfo > const & typeInfo,
    Dependencies * dependencies, ClassFile * classFile)
{
    OSL_ASSERT(dependencies != 0 && classFile != 0);
    std::vector< TypeInfo >::size_type typeInfos = typeInfo.size();
    if (typeInfos > SAL_MAX_INT32) {
        throw CannotDumpException(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "UNOTYPEINFO array too big for Java class file format")));
    }
    if (typeInfos != 0) {
        classFile->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("UNOTYPEINFO")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "[Lcom/sun/star/lib/uno/typeinfo/TypeInfo;")),
            0, rtl::OString());
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< ClassFile::Code > code(classFile->newCode());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        code->loadIntegerConstant(static_cast< sal_Int32 >(typeInfos));
        code->instrAnewarray(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lib/uno/typeinfo/TypeInfo")));
        sal_Int32 index = 0;
        sal_uInt16 stack = 0;
        for (std::vector< TypeInfo >::const_iterator i(typeInfo.begin());
             i != typeInfo.end(); ++i)
        {
            code->instrDup();
            code->loadIntegerConstant(index++);
            stack = std::max(stack, i->generateCode(*code, dependencies));
            code->instrAastore();
        }
        code->instrPutstatic(
            className, rtl::OString(RTL_CONSTASCII_STRINGPARAM("UNOTYPEINFO")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "[Lcom/sun/star/lib/uno/typeinfo/TypeInfo;")));
        code->instrReturn();
        if (stack > SAL_MAX_UINT16 - 4) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "Stack too big for Java class file format")));
        }
        code->setMaxStackAndLocals(static_cast< sal_uInt16 >(stack + 4), 0);
        classFile->addMethod(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<clinit>")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("()V")), code.get(),
            std::vector< rtl::OString >(), rtl::OString());
    }
}

typedef void (* handleUnoTypeRegistryEntityFunction)(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies);

void handleEnumType(
    SAL_UNUSED_PARAMETER TypeManager const &,
    JavaOptions /*TODO const*/ & options, typereg::Reader const & reader,
    SAL_UNUSED_PARAMETER Dependencies *)
{
    sal_uInt16 fields = reader.getFieldCount();
    if (fields == 0 || reader.getSuperTypeCount() != 0
        || reader.getMethodCount() != 0 || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    rtl::OString className(codemaker::convertString(reader.getTypeName()));
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Enum")),
            rtl::OString()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    rtl::OStringBuffer buf;
    buf.append('L');
    buf.append(className);
    buf.append(';');
    rtl::OString classDescriptor(buf.makeStringAndClear());
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        RTConstValue fieldValue(reader.getFieldValue(i));
        if (fieldValue.m_type != RT_TYPE_INT32
            || reader.getFieldFlags(i) != RT_ACCESS_CONST
            || reader.getFieldTypeName(i).getLength() != 0)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        rtl::OString fieldName(
            codemaker::convertString(reader.getFieldName(i)));
        cf->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            fieldName, classDescriptor, 0, rtl::OString());
        cf->addField(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
                | ClassFile::ACC_FINAL),
            fieldName + rtl::OString(RTL_CONSTASCII_STRINGPARAM("_value")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("I")),
            cf->addIntegerInfo(fieldValue.m_value.aLong), rtl::OString());
    }}
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile::Code > code(cf->newCode());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    code->loadLocalReference(0);
    code->loadLocalInteger(1);
    code->instrInvokespecial(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Enum")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)V")));
    code->instrReturn();
    code->setMaxStackAndLocals(2, 2);
    cf->addMethod(
        ClassFile::ACC_PRIVATE,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)V")), code.get(),
        std::vector< rtl::OString >(), rtl::OString());
    code.reset(cf->newCode());
    code->instrGetstatic(
        className,
        codemaker::convertString(reader.getFieldName(0)), classDescriptor);
    code->instrAreturn();
    code->setMaxStackAndLocals(1, 0);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getDefault")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()")) + classDescriptor,
        code.get(), std::vector< rtl::OString >(), rtl::OString());
    code.reset(cf->newCode());
    code->loadLocalInteger(0);
    std::map< sal_Int32, rtl::OString > map;
    sal_Int32 min = SAL_MAX_INT32;
    sal_Int32 max = SAL_MIN_INT32;
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        sal_Int32 value = reader.getFieldValue(i).m_value.aLong;
        min = std::min(min, value);
        max = std::max(max, value);
        map.insert(
            std::map< sal_Int32, rtl::OString >::value_type(
                value, codemaker::convertString(reader.getFieldName(i))));
    }}
    sal_uInt64 size = static_cast< sal_uInt64 >(map.size());
    if ((static_cast< sal_uInt64 >(max) - static_cast< sal_uInt64 >(min)
         <= 2 * size)
        || size > SAL_MAX_INT32)
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< ClassFile::Code > defCode(cf->newCode());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        defCode->instrAconstNull();
        defCode->instrAreturn();
        std::list< ClassFile::Code * > blocks;
            //FIXME: pointers contained in blocks may leak
        sal_Int32 last = SAL_MAX_INT32;
        for (std::map< sal_Int32, rtl::OString >::iterator i(map.begin());
             i != map.end(); ++i)
        {
            sal_Int32 value = i->first;
            if (last != SAL_MAX_INT32) {
                for (sal_Int32 j = last + 1; j < value; ++j) {
                    blocks.push_back(0);
                }
            }
            last = value;
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            std::auto_ptr< ClassFile::Code > blockCode(cf->newCode());
            SAL_WNODEPRECATED_DECLARATIONS_POP
            blockCode->instrGetstatic(className, i->second, classDescriptor);
            blockCode->instrAreturn();
            blocks.push_back(blockCode.get());
            blockCode.release();
        }
        code->instrTableswitch(defCode.get(), min, blocks);
        {for (std::list< ClassFile::Code * >::iterator i(blocks.begin());
              i != blocks.end(); ++i)
        {
            delete *i;
        }}
    } else{
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< ClassFile::Code > defCode(cf->newCode());
        SAL_WNODEPRECATED_DECLARATIONS_POP
        defCode->instrAconstNull();
        defCode->instrAreturn();
        std::list< std::pair< sal_Int32, ClassFile::Code * > > blocks;
            //FIXME: pointers contained in blocks may leak
        for (std::map< sal_Int32, rtl::OString >::iterator i(map.begin());
             i != map.end(); ++i)
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            std::auto_ptr< ClassFile::Code > blockCode(cf->newCode());
            SAL_WNODEPRECATED_DECLARATIONS_POP
            blockCode->instrGetstatic(className, i->second, classDescriptor);
            blockCode->instrAreturn();
            blocks.push_back(std::make_pair(i->first, blockCode.get()));
            blockCode.release();
        }
        code->instrLookupswitch(defCode.get(), blocks);
        {for (std::list< std::pair< sal_Int32, ClassFile::Code * > >::iterator
                  i(blocks.begin());
              i != blocks.end(); ++i)
        {
            delete i->second;
        }}
    }
    code->setMaxStackAndLocals(1, 1);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("fromInt")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)")) + classDescriptor,
        code.get(), std::vector< rtl::OString >(), rtl::OString());
    code.reset(cf->newCode());
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        code->instrNew(className);
        code->instrDup();
        code->loadIntegerConstant(reader.getFieldValue(i).m_value.aLong);
        code->instrInvokespecial(
            className, rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)V")));
        code->instrPutstatic(
            className,
            codemaker::convertString(reader.getFieldName(i)),
            classDescriptor);
    }}
    code->instrReturn();
    code->setMaxStackAndLocals(3, 0);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<clinit>")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()V")), code.get(),
        std::vector< rtl::OString >(), rtl::OString());
    writeClassFile(options, className, *cf.get());
}

void addField(
    TypeManager const & manager, Dependencies * dependencies,
    ClassFile * classFile, std::vector< TypeInfo > * typeInfo,
    sal_Int32 typeParameterIndex, rtl::OString const & type,
    rtl::OString const & name, sal_Int32 index)
{
    OSL_ASSERT(dependencies != 0 && classFile != 0 && typeInfo != 0);
    rtl::OString descriptor;
    rtl::OString signature;
    SpecialType specialType;
    PolymorphicUnoType polymorphicUnoType;
    if (typeParameterIndex >= 0) {
        descriptor = rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;"));
        rtl::OStringBuffer buf;
        buf.append('T');
        buf.append(type);
        buf.append(';');
        signature = buf.makeStringAndClear();
        specialType = SPECIAL_TYPE_NONE; //TODO: SPECIAL_TYPE_TYPE_PARAMETER?
    } else {
        specialType = getFieldDescriptor(
            manager, dependencies, type, &descriptor, &signature,
            &polymorphicUnoType);
    }
    classFile->addField(ClassFile::ACC_PUBLIC, name, descriptor, 0, signature);
    typeInfo->push_back(
        TypeInfo(
            name, specialType, index, polymorphicUnoType, typeParameterIndex));
}

sal_uInt16 addFieldInit(
    TypeManager const & manager, rtl::OString const & className,
    rtl::OString const & fieldName, bool typeParameter,
    rtl::OString const & fieldType, Dependencies * dependencies,
    ClassFile::Code * code)
{
    OSL_ASSERT(dependencies != 0 && code != 0);
    if (typeParameter) {
        return 0;
    } else {
        RTTypeClass typeClass;
        rtl::OString nucleus;
        sal_Int32 rank;
        std::vector< rtl::OString > args;
        codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
            manager, fieldType, true, false, false, &typeClass, &nucleus, &rank,
            &args);
        if (rank == 0) {
            switch (sort) {
            case codemaker::UnoType::SORT_STRING:
                code->loadLocalReference(0);
                code->loadStringConstant(rtl::OString());
                code->instrPutfield(
                    className, fieldName,
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Ljava/lang/String;")));
                return 2;

            case codemaker::UnoType::SORT_TYPE:
                code->loadLocalReference(0);
                code->instrGetstatic(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
                    rtl::OString(RTL_CONSTASCII_STRINGPARAM("VOID")),
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Type;")));
                code->instrPutfield(
                    className, fieldName,
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Type;")));
                return 2;

            case codemaker::UnoType::SORT_ANY:
                code->loadLocalReference(0);
                code->instrGetstatic(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
                    rtl::OString(RTL_CONSTASCII_STRINGPARAM("VOID")),
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/Any;")));
                code->instrPutfield(
                    className, fieldName,
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;")));
                return 2;

            case codemaker::UnoType::SORT_COMPLEX:
                switch (typeClass) {
                case RT_TYPE_ENUM:
                    {
                        code->loadLocalReference(0);
                        typereg::Reader reader(manager.getTypeReader(nucleus));
                        if (reader.getFieldCount() == 0) {
                            throw CannotDumpException(
                                rtl::OString(
                                    RTL_CONSTASCII_STRINGPARAM(
                                        "Bad type information"))); //TODO
                        }
                        rtl::OStringBuffer descBuf;
                        translateUnoTypeToDescriptor(
                            manager, sort, typeClass, nucleus, 0,
                            std::vector< rtl::OString >(), false, false,
                            dependencies, &descBuf, 0, 0, 0);
                        rtl::OString desc(descBuf.makeStringAndClear());
                        code->instrGetstatic(
                            nucleus,
                            codemaker::convertString(reader.getFieldName(0)),
                            desc);
                        code->instrPutfield(className, fieldName, desc);
                        return 2;
                    }

                case RT_TYPE_STRUCT:
                    {
                        code->loadLocalReference(0);
                        code->instrNew(nucleus);
                        code->instrDup();
                        code->instrInvokespecial(
                            nucleus,
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("()V")));
                        rtl::OStringBuffer desc;
                        translateUnoTypeToDescriptor(
                            manager, sort, typeClass, nucleus, 0,
                            std::vector< rtl::OString >(), false, false,
                            dependencies, &desc, 0, 0, 0);
                        code->instrPutfield(
                            className, fieldName, desc.makeStringAndClear());
                        return 3;
                    }

                default:
                    OSL_ASSERT(typeClass == RT_TYPE_INTERFACE);
                    return 0;
                }

            default:
                return 0;
            }
        } else {
            code->loadLocalReference(0);
            code->loadIntegerConstant(0);
            if (rank == 1) {
                if (sort >= codemaker::UnoType::SORT_BOOLEAN
                    && sort <= codemaker::UnoType::SORT_CHAR)
                {
                    code->instrNewarray(sort);
                } else {
                    code->instrAnewarray(
                        codemaker::java::translateUnoToJavaType(sort, typeClass,
                                                                nucleus, 0));
                }
            } else {
                rtl::OStringBuffer desc;
                translateUnoTypeToDescriptor(
                    manager, sort, typeClass, nucleus, rank - 1,
                    std::vector< rtl::OString >(), false, false, dependencies,
                    &desc, 0, 0, 0);
                code->instrAnewarray(desc.makeStringAndClear());
            }
            rtl::OStringBuffer desc;
            translateUnoTypeToDescriptor(
                manager, sort, typeClass, nucleus, rank,
                std::vector< rtl::OString >(), false, false, dependencies,
                &desc, 0, 0, 0);
            code->instrPutfield(
                className, fieldName, desc.makeStringAndClear());
            return 2;
        }
    }
}

sal_uInt16 addLoadLocal(
    TypeManager const & manager, ClassFile::Code * code, sal_uInt16 * index,
    bool typeParameter, rtl::OString const & type, bool any,
    Dependencies * dependencies)
{
    OSL_ASSERT(
        code != 0 && index != 0 && !(typeParameter && any)
        && dependencies != 0);
    sal_uInt16 stack = 1;
    sal_uInt16 size = 1;
    if (typeParameter) {
        code->loadLocalReference(*index);
        stack = size = 1;
    } else {
        RTTypeClass typeClass;
        rtl::OString nucleus;
        sal_Int32 rank;
        std::vector< rtl::OString > args;
        codemaker::UnoType::Sort sort = codemaker::decomposeAndResolve(
            manager, type, true, false, false, &typeClass, &nucleus, &rank, &args);
        if (rank == 0) {
            switch (sort) {
            case codemaker::UnoType::SORT_BOOLEAN:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Boolean")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Boolean")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(Z)V")));
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_BYTE:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Byte")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Byte")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(B)V")));
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_SHORT:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Short")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Short")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(S)V")));
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_UNSIGNED_SHORT:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Any")));
                    code->instrDup();
                    code->instrGetstatic(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Type")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("UNSIGNED_SHORT")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Lcom/sun/star/uno/Type;")));
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Short")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Short")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(S)V")));
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                                "V")));
                    stack = 6;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_LONG:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)V")));
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_UNSIGNED_LONG:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Any")));
                    code->instrDup();
                    code->instrGetstatic(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Type")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("UNSIGNED_LONG")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Lcom/sun/star/uno/Type;")));
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(I)V")));
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                                "V")));
                    stack = 6;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_HYPER:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Long")));
                    code->instrDup();
                    code->loadLocalLong(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Long")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(J)V")));
                    stack = 4;
                } else {
                    code->loadLocalLong(*index);
                    stack = 2;
                }
                size = 2;
                break;

            case codemaker::UnoType::SORT_UNSIGNED_HYPER:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Any")));
                    code->instrDup();
                    code->instrGetstatic(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "com/sun/star/uno/Type")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("UNSIGNED_HYPER")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Lcom/sun/star/uno/Type;")));
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Long")));
                    code->instrDup();
                    code->loadLocalLong(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Long")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(J)V")));
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                                "V")));
                    stack = 7;
                } else {
                    code->loadLocalLong(*index);
                    stack = 2;
                }
                size = 2;
                break;

            case codemaker::UnoType::SORT_FLOAT:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Float")));
                    code->instrDup();
                    code->loadLocalFloat(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Float")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(F)V")));
                    stack = 3;
                } else {
                    code->loadLocalFloat(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_DOUBLE:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Double")));
                    code->instrDup();
                    code->loadLocalDouble(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Double")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(D)V")));
                    stack = 4;
                } else {
                    code->loadLocalDouble(*index);
                    stack = 2;
                }
                size = 2;
                break;

            case codemaker::UnoType::SORT_CHAR:
                if (any) {
                    code->instrNew(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Character")));
                    code->instrDup();
                    code->loadLocalInteger(*index);
                    code->instrInvokespecial(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM("java/lang/Character")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                        rtl::OString(RTL_CONSTASCII_STRINGPARAM("(C)V")));
                    stack = 3;
                } else {
                    code->loadLocalInteger(*index);
                    stack = 1;
                }
                size = 1;
                break;

            case codemaker::UnoType::SORT_STRING:
            case codemaker::UnoType::SORT_TYPE:
            case codemaker::UnoType::SORT_ANY:
                code->loadLocalReference(*index);
                stack = size = 1;
                break;

            case codemaker::UnoType::SORT_COMPLEX:
                switch (typeClass) {
                case RT_TYPE_ENUM:
                    // Assuming that no Java types are derived from Java types
                    // that are directly derived from com.sun.star.uno.Enum:
                    code->loadLocalReference(*index);
                    stack = size = 1;
                    break;

                case RT_TYPE_STRUCT:
                    if (any) {
                        code->instrNew(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Any")));
                        code->instrDup();
                        code->instrNew(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Type")));
                        code->instrDup();
                        code->loadStringConstant(
                            createUnoName(manager, nucleus, rank, args));
                        code->instrGetstatic(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/TypeClass")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("STRUCT")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Lcom/sun/star/uno/TypeClass;")));
                        dependencies->insert(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/TypeClass")));
                        code->instrInvokespecial(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Type")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "(Ljava/lang/String;"
                                    "Lcom/sun/star/uno/TypeClass;)V")));
                        code->loadLocalReference(*index);
                        code->instrInvokespecial(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Any")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "(Lcom/sun/star/uno/Type;"
                                    "Ljava/lang/Object;)V")));
                        stack = 6;
                    } else {
                        code->loadLocalReference(*index);
                        stack = 1;
                    }
                    size = 1;
                    break;

                case RT_TYPE_INTERFACE:
                    if (any
                        && (nucleus
                            != rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/XInterface"))))
                    {
                        code->instrNew(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Any")));
                        code->instrDup();
                        code->instrNew(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Type")));
                        code->instrDup();
                        code->loadStringConstant(nucleus.replace('/', '.'));
                        code->instrGetstatic(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/TypeClass")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM("INTERFACE")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Lcom/sun/star/uno/TypeClass;")));
                        dependencies->insert(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/TypeClass")));
                        code->instrInvokespecial(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Type")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "(Ljava/lang/String;"
                                    "Lcom/sun/star/uno/TypeClass;)V")));
                        code->loadLocalReference(*index);
                        code->instrInvokespecial(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "com/sun/star/uno/Any")),
                            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "(Lcom/sun/star/uno/Type;"
                                    "Ljava/lang/Object;)V")));
                        stack = 6;
                    } else {
                        code->loadLocalReference(*index);
                        stack = 1;
                    }
                    size = 1;
                    break;

                default:
                    OSL_ASSERT(false);
                    break;
                }
                break;

            default:
                OSL_ASSERT(false);
                break;
            }
        } else {
            bool wrap = false;
            if (any) {
                switch (sort) {
                case codemaker::UnoType::SORT_BOOLEAN:
                case codemaker::UnoType::SORT_BYTE:
                case codemaker::UnoType::SORT_SHORT:
                case codemaker::UnoType::SORT_LONG:
                case codemaker::UnoType::SORT_HYPER:
                case codemaker::UnoType::SORT_FLOAT:
                case codemaker::UnoType::SORT_DOUBLE:
                case codemaker::UnoType::SORT_CHAR:
                case codemaker::UnoType::SORT_STRING:
                case codemaker::UnoType::SORT_TYPE:
                        // assuming that no Java types are derived from
                        // com.sun.star.uno.Type
                    break;

                case codemaker::UnoType::SORT_UNSIGNED_SHORT:
                case codemaker::UnoType::SORT_UNSIGNED_LONG:
                case codemaker::UnoType::SORT_UNSIGNED_HYPER:
                case codemaker::UnoType::SORT_ANY:
                    wrap = true;
                    break;

                case codemaker::UnoType::SORT_COMPLEX:
                    switch (typeClass) {
                    case RT_TYPE_ENUM:
                            // assuming that no Java types are derived from Java
                            // types that are directly derived from
                            // com.sun.star.uno.Enum
                        break;

                    case RT_TYPE_STRUCT:
                    case RT_TYPE_INTERFACE:
                        wrap = true;
                        break;

                    default:
                        OSL_ASSERT(false);
                        break;
                    }
                    break;

                default:
                    OSL_ASSERT(false);
                    break;
                }
            }
            if (wrap) {
                code->instrNew(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")));
                code->instrDup();
                code->instrNew(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")));
                code->instrDup();
                code->loadStringConstant(
                    createUnoName(manager, nucleus, rank, args));
                code->instrInvokespecial(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
                    rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;)V")));
                code->loadLocalReference(*index);
                code->instrInvokespecial(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
                    rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM(
                            "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)V")));
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
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "Too many local variables for Java class file format")));
    }
    *index = *index + size;
    return stack;
}

void addBaseArguments(
    TypeManager const & manager, Dependencies * dependencies,
    MethodDescriptor * methodDescriptor, ClassFile::Code * code,
    RTTypeClass typeClass, rtl::OString const & type, sal_uInt16 * index)
{
    OSL_ASSERT(
        dependencies != 0 && methodDescriptor != 0 && code != 0 && index != 0);
    typereg::Reader reader(manager.getTypeReader(type));
    if (!reader.isValid() || reader.getTypeClass() != typeClass
        || codemaker::convertString(reader.getTypeName()) != type
        || reader.getMethodCount() != 0 || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    sal_uInt16 superTypes = reader.getSuperTypeCount();
    sal_uInt16 fields = reader.getFieldCount();
    sal_uInt16 firstField = 0;
    if (type
        == rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Exception")))
    {
        if (typeClass != RT_TYPE_EXCEPTION || superTypes != 0 || fields != 2) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        firstField = 1;
    } else {
        if (
            (typeClass == RT_TYPE_STRUCT && (superTypes > 1 || fields == 0)) ||
            (typeClass == RT_TYPE_EXCEPTION && superTypes != 1)
           )
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        if (superTypes == 1) {
            addBaseArguments(
                manager, dependencies, methodDescriptor, code, typeClass,
                codemaker::convertString(reader.getSuperTypeName(0)), index);
        }
    }
    for (sal_uInt16 i = firstField; i < fields; ++i) {
        if (reader.getFieldFlags(i) != RT_ACCESS_READWRITE
            || reader.getFieldValue(i).m_type != RT_TYPE_NONE)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        rtl::OString fieldType(
            codemaker::convertString(reader.getFieldTypeName(i)));
        methodDescriptor->addParameter(fieldType, false, true, 0);
        addLoadLocal(
            manager, code, index, false, fieldType, false, dependencies);
    }
}

sal_uInt16 addDirectArgument(
    TypeManager const & manager, Dependencies * dependencies,
    MethodDescriptor * methodDescriptor, ClassFile::Code * code,
    sal_uInt16 * index, rtl::OString const & className,
    rtl::OString const & fieldName, bool typeParameter,
    rtl::OString const & fieldType)
{
    OSL_ASSERT(
        dependencies != 0 && methodDescriptor != 0 && code != 0 && index != 0);
    rtl::OString desc;
    if (typeParameter) {
        methodDescriptor->addTypeParameter(fieldType);
        desc = rtl::OString(RTL_CONSTASCII_STRINGPARAM("Ljava/lang/Object;"));
    } else {
        methodDescriptor->addParameter(fieldType, false, true, 0);
        getFieldDescriptor(manager, dependencies, fieldType, &desc, 0, 0);
    }
    code->loadLocalReference(0);
    sal_uInt16 stack = addLoadLocal(
        manager, code, index, typeParameter, fieldType, false, dependencies);
    code->instrPutfield(className, fieldName, desc);
    return stack + 1;
}

void handleAggregatingType(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    if (reader.getMethodCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    RTTypeClass typeClass = reader.getTypeClass();
    rtl::OString className(codemaker::convertString(reader.getTypeName()));
    sal_uInt16 superTypes = reader.getSuperTypeCount();
    sal_uInt16 fields = reader.getFieldCount();
    sal_uInt16 firstField = 0;
    sal_uInt16 references = reader.getReferenceCount();
    bool runtimeException = false;
    rtl::OString superClass;
    if (className
        == rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Exception")))
    {
        if (typeClass != RT_TYPE_EXCEPTION || superTypes != 0 || fields != 2
            || references != 0)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        firstField = 1;
        superClass = rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("java/lang/Exception"));
    } else if (className
               == rtl::OString(
                   RTL_CONSTASCII_STRINGPARAM(
                       "com/sun/star/uno/RuntimeException")))
    {
        if (typeClass != RT_TYPE_EXCEPTION || superTypes != 1 || fields != 0
            || references != 0)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        superTypes = 0;
        superClass = rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("java/lang/RuntimeException"));
        runtimeException = true;
    } else {
        if (
             (
              typeClass == RT_TYPE_STRUCT &&
              (
               fields == 0 ||
               (references == 0 ? superTypes > 1 : superTypes != 0)
              )
             ) ||
             (typeClass == RT_TYPE_EXCEPTION && superTypes != 1)
           )
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        if (superTypes == 0) {
            superClass = rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("java/lang/Object"));
        } else {
            superClass = codemaker::convertString(reader.getSuperTypeName(0));
            dependencies->insert(superClass);
        }
    }
    rtl::OString sig;
    std::map< rtl::OString, sal_Int32 > typeParameters;
    if (references != 0) {
        rtl::OStringBuffer buf;
        buf.append('<');
        for (sal_uInt16 i = 0; i < references; ++i) {
            if (reader.getReferenceFlags(i) != RT_ACCESS_INVALID
                || reader.getReferenceSort(i) != RT_REF_TYPE_PARAMETER)
            {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
            rtl::OString name(
                codemaker::convertString(reader.getReferenceTypeName(i)));
            buf.append(name);
            buf.append(RTL_CONSTASCII_STRINGPARAM(":Ljava/lang/Object;"));
            if (!typeParameters.insert(
                    std::map< rtl::OString, sal_Int32 >::value_type(name, i)).
                second)
            {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
        }
        buf.append(RTL_CONSTASCII_STRINGPARAM(">Ljava/lang/Object;"));
        sig = buf.makeStringAndClear();
    }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_SUPER),
            className, superClass, sig));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    std::vector< TypeInfo > typeInfo;
    {for (sal_uInt16 i = firstField; i < fields; ++i) {
        RTFieldAccess flags = reader.getFieldFlags(i);
        if ((flags != RT_ACCESS_READWRITE
             && flags != (RT_ACCESS_READWRITE | RT_ACCESS_PARAMETERIZED_TYPE))
            || ((flags & RT_ACCESS_PARAMETERIZED_TYPE) != 0 && references == 0)
            || reader.getFieldValue(i).m_type != RT_TYPE_NONE)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        rtl::OString type(
            codemaker::convertString(reader.getFieldTypeName(i)));
        sal_Int32 typeParameterIndex;
        if ((flags & RT_ACCESS_PARAMETERIZED_TYPE) == 0) {
            typeParameterIndex = -1;
        } else {
            std::map< rtl::OString, sal_Int32 >::iterator it(
                typeParameters.find(type));
            if (it == typeParameters.end()) {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
            typeParameterIndex = it->second;
        }
        addField(
            manager, dependencies, cf.get(), &typeInfo, typeParameterIndex,
            type, codemaker::convertString(reader.getFieldName(i)), i - firstField);
    }}
    if (runtimeException) {
        addField(
            manager, dependencies, cf.get(), &typeInfo, -1,
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XInterface")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Context")), 0);
    }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile::Code > code(cf->newCode());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    code->loadLocalReference(0);
    code->instrInvokespecial(
        superClass, rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()V")));
    sal_uInt16 stack = 0;
    {for (sal_uInt16 i = firstField; i < fields; ++i) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className,
                codemaker::convertString(reader.getFieldName(i)),
                (reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0,
                codemaker::convertString(reader.getFieldTypeName(i)),
                dependencies, code.get()));
    }}
    if (runtimeException) {
        stack = std::max(
            stack,
            addFieldInit(
                manager, className,
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("Context")), false,
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XInterface")),
                dependencies, code.get()));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(stack + 1, 1);
    cf->addMethod(
        ClassFile::ACC_PUBLIC,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()V")), code.get(),
        std::vector< rtl::OString >(), rtl::OString());
    if (typeClass == RT_TYPE_EXCEPTION) {
        code.reset(cf->newCode());
        code->loadLocalReference(0);
        code->loadLocalReference(1);
        code->instrInvokespecial(
            superClass, rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;)V")));
        stack = 0;
        for (sal_uInt16 i = firstField; i < fields; ++i) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className,
                    codemaker::convertString(reader.getFieldName(i)),
                    ((reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE)
                     != 0),
                    codemaker::convertString(reader.getFieldTypeName(i)),
                    dependencies, code.get()));
        }
        if (runtimeException) {
            stack = std::max(
                stack,
                addFieldInit(
                    manager, className,
                    rtl::OString(RTL_CONSTASCII_STRINGPARAM("Context")), false,
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM(
                            "com/sun/star/uno/XInterface")),
                    dependencies, code.get()));
        }
        code->instrReturn();
        code->setMaxStackAndLocals(stack + 2, 2);
        cf->addMethod(
            ClassFile::ACC_PUBLIC,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("(Ljava/lang/String;)V")),
            code.get(), std::vector< rtl::OString >(), rtl::OString());
    }
    MethodDescriptor desc(
        manager, dependencies, rtl::OString(RTL_CONSTASCII_STRINGPARAM("void")),
        0, 0);
    code.reset(cf->newCode());
    code->loadLocalReference(0);
    sal_uInt16 index = 1;
    if (typeClass == RT_TYPE_EXCEPTION) {
        desc.addParameter(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("string")), false, true, 0);
        code->loadLocalReference(index++);
    }
    if (superTypes != 0) {
        addBaseArguments(
            manager, dependencies, &desc, code.get(), typeClass, superClass,
            &index);
    }
    code->instrInvokespecial(
        superClass, rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        desc.getDescriptor());
    sal_uInt16 maxSize = index;
    {for (sal_uInt16 i = firstField; i < fields; ++i) {
        maxSize = std::max(
            maxSize,
            addDirectArgument(
                manager, dependencies, &desc, code.get(), &index, className,
                codemaker::convertString(reader.getFieldName(i)),
                (reader.getFieldFlags(i) & RT_ACCESS_PARAMETERIZED_TYPE) != 0,
                codemaker::convertString(reader.getFieldTypeName(i))));
    }}
    if (runtimeException) {
        maxSize = std::max(
            maxSize,
            addDirectArgument(
                manager, dependencies, &desc, code.get(), &index, className,
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("Context")), false,
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/uno/XInterface"))));
    }
    code->instrReturn();
    code->setMaxStackAndLocals(maxSize, index);
    cf->addMethod(
        ClassFile::ACC_PUBLIC,
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        desc.getDescriptor(), code.get(), std::vector< rtl::OString >(),
        desc.getSignature());
    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf.get());
}

void createExceptionsAttribute(
    TypeManager const & manager, typereg::Reader const & reader,
    sal_uInt16 methodIndex, Dependencies * dependencies,
    std::vector< rtl::OString > * exceptions, codemaker::ExceptionTree * tree)
{
    OSL_ASSERT(dependencies != 0 && exceptions != 0);
    sal_uInt16 n = reader.getMethodExceptionCount(methodIndex);
    for (sal_uInt16 i = 0; i < n; ++i) {
        rtl::OString type(
            codemaker::convertString(
                reader.getMethodExceptionTypeName(methodIndex, i)));
        dependencies->insert(type);
        exceptions->push_back(type);
        if (tree != 0) {
            tree->add(type, manager);
        }
    }
}

void handleInterfaceType(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);

    rtl::OString className(codemaker::convertString(reader.getTypeName()));
    sal_uInt16 superTypes = reader.getSuperTypeCount();
    sal_uInt16 fields = reader.getFieldCount();
    sal_uInt16 methods = reader.getMethodCount();
    if (className
        == rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XInterface")))
    {
        if (superTypes != 0 || fields != 0 || methods != 3) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        methods = 0;
    } else if (superTypes == 0) {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_INTERFACE
                | ClassFile::ACC_ABSTRACT),
            className,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
            rtl::OString()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    {for (sal_uInt16 i = 0; i < superTypes; ++i) {
        rtl::OString t(codemaker::convertString(reader.getSuperTypeName(i)));
        dependencies->insert(t);
        cf->addInterface(t);
    }}
    // As a special case, let com.sun.star.lang.XEventListener extend
    // java.util.EventListener ("A tagging interface that all event listener
    // interfaces must extend"):
    if (className ==
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/lang/XEventListener")))
    {
        cf->addInterface(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("java/util/EventListener")));
    }
    std::vector< TypeInfo > typeInfo;
    sal_Int32 index = 0;
    {for (sal_uInt16 i = 0; i < fields; ++i) {
        RTFieldAccess flags = reader.getFieldFlags(i);
        //TODO: ok if both READONLY and BOUND?
        if (((((flags & RT_ACCESS_READWRITE) != 0)
              ^ ((flags & RT_ACCESS_READONLY) != 0))
             == 0)
            || ((flags
                 & ~(RT_ACCESS_READWRITE | RT_ACCESS_READONLY
                     | RT_ACCESS_BOUND))
                != 0)
            || reader.getFieldValue(i).m_type != RT_TYPE_NONE)
        {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        //TODO: exploit the fact that attribute getter/setter methods preceed
        // real methods
        rtl::OUString attrNameUtf16(reader.getFieldName(i));
        sal_uInt16 getter = SAL_MAX_UINT16;
        sal_uInt16 setter = SAL_MAX_UINT16;
        for (sal_uInt16 j = 0; j < methods; ++j) {
            RTMethodMode mflags = reader.getMethodFlags(j);
            if ((mflags == RT_MODE_ATTRIBUTE_GET
                 || mflags == RT_MODE_ATTRIBUTE_SET)
                && reader.getMethodName(j) == attrNameUtf16)
            {
                if (reader.getMethodReturnTypeName(j) != "void"
                    || reader.getMethodParameterCount(j) != 0
                    || (mflags == RT_MODE_ATTRIBUTE_GET
                        ? getter != SAL_MAX_UINT16
                        : (setter != SAL_MAX_UINT16
                           || (flags & RT_ACCESS_READONLY) != 0)))
                {
                    throw CannotDumpException(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Bad type information"))); //TODO
                }
                OSL_ASSERT(j != SAL_MAX_UINT16);
                (mflags == RT_MODE_ATTRIBUTE_GET ? getter : setter) = j;
            }
        }
        rtl::OString fieldType(
            codemaker::convertString(reader.getFieldTypeName(i)));
        SpecialType specialType;
        PolymorphicUnoType polymorphicUnoType;
        MethodDescriptor gdesc(
            manager, dependencies, fieldType, &specialType,
            &polymorphicUnoType);
        std::vector< rtl::OString > exc;
        if (getter != SAL_MAX_UINT16) {
            createExceptionsAttribute(
                manager, reader, getter, dependencies, &exc, 0);
        }
        rtl::OString attrName(codemaker::convertString(attrNameUtf16));
        cf->addMethod(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("get")) + attrName,
            gdesc.getDescriptor(), 0, exc, gdesc.getSignature());
        if ((flags & RT_ACCESS_READONLY) == 0) {
            MethodDescriptor sdesc(
                manager, dependencies,
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("void")), 0, 0);
            sdesc.addParameter(fieldType, false, true, 0);
            std::vector< rtl::OString > exc2;
            if (setter != SAL_MAX_UINT16) {
                createExceptionsAttribute(
                    manager, reader, setter, dependencies, &exc2, 0);
            }
            cf->addMethod(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("set")) + attrName,
                sdesc.getDescriptor(), 0, exc2, sdesc.getSignature());
        }
        typeInfo.push_back(
            TypeInfo(
                TypeInfo::KIND_ATTRIBUTE, attrName, specialType,
                static_cast< TypeInfo::Flags >(
                    ((flags & RT_ACCESS_READONLY) == 0
                     ? 0 : TypeInfo::FLAG_READONLY)
                    | ((flags & RT_ACCESS_BOUND) == 0
                       ? 0 : TypeInfo::FLAG_BOUND)),
                index, polymorphicUnoType));
        index += ((flags & RT_ACCESS_READONLY) == 0 ? 2 : 1);
    }}
    {for (sal_uInt16 i = 0; i < methods; ++i) {
        RTMethodMode flags = reader.getMethodFlags(i);
        switch (flags) {
        case RT_MODE_ONEWAY:
        case RT_MODE_TWOWAY:
            {
                rtl::OString methodName(
                    codemaker::convertString(reader.getMethodName(i)));
                SpecialType specialReturnType;
                PolymorphicUnoType polymorphicUnoReturnType;
                MethodDescriptor desc(
                    manager, dependencies,
                    codemaker::convertString(
                        reader.getMethodReturnTypeName(i)),
                    &specialReturnType, &polymorphicUnoReturnType);
                typeInfo.push_back(
                    TypeInfo(
                        TypeInfo::KIND_METHOD, methodName, specialReturnType,
                        static_cast< TypeInfo::Flags >(
                            flags == RT_MODE_ONEWAY
                            ? TypeInfo::FLAG_ONEWAY : 0),
                        index++, polymorphicUnoReturnType));
                for (sal_uInt16 j = 0; j < reader.getMethodParameterCount(i);
                     ++j)
                {
                    bool in;
                    bool out;
                    switch (reader.getMethodParameterFlags(i, j)) {
                    case RT_PARAM_IN:
                        in = true;
                        out = false;
                        break;

                    case RT_PARAM_OUT:
                        in = false;
                        out = true;
                        break;

                    case RT_PARAM_INOUT:
                        in = true;
                        out = true;
                        break;

                    default:
                        throw CannotDumpException(
                            rtl::OString(
                                RTL_CONSTASCII_STRINGPARAM(
                                    "Bad type information"))); //TODO
                    }
                    PolymorphicUnoType polymorphicUnoType;
                    SpecialType specialType = desc.addParameter(
                        codemaker::convertString(
                            reader.getMethodParameterTypeName(i, j)),
                        out, true, &polymorphicUnoType);
                    if (out || isSpecialType(specialType)
                        || (polymorphicUnoType.kind
                            != PolymorphicUnoType::KIND_NONE))
                    {
                        typeInfo.push_back(
                            TypeInfo(
                                codemaker::convertString(
                                    reader.getMethodParameterName(i, j)),
                                specialType, in, out, methodName, j,
                                polymorphicUnoType));
                    }
                }
                std::vector< rtl::OString > exc2;
                createExceptionsAttribute(
                    manager, reader, i, dependencies, &exc2, 0);
                cf->addMethod(
                    static_cast< ClassFile::AccessFlags >(
                        ClassFile::ACC_PUBLIC | ClassFile::ACC_ABSTRACT),
                    methodName, desc.getDescriptor(), 0, exc2,
                    desc.getSignature());
                break;
            }

        case RT_MODE_ATTRIBUTE_GET:
        case RT_MODE_ATTRIBUTE_SET:
            {
                //TODO: exploit the fact that attribute getter/setter methods
                // are ordered the same way as the attribute fields themselves
                rtl::OUString methodNameUtf16(reader.getMethodName(i));
                bool found = false;
                for (sal_uInt16 j = 0; j < fields; ++j) {
                    if (reader.getFieldName(j) == methodNameUtf16) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    break;
                }
            }
        default:
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
    }}
    addTypeInfo(className, typeInfo, dependencies, cf.get());
    writeClassFile(options, className, *cf.get());
}

void handleTypedef(
    TypeManager const & manager,
    SAL_UNUSED_PARAMETER JavaOptions /*TODO const*/ &,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    if (reader.getSuperTypeCount() != 1 || reader.getFieldCount() != 0
        || reader.getMethodCount() != 0 || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    RTTypeClass typeClass;
    rtl::OString nucleus;
    sal_Int32 rank;
    std::vector< rtl::OString > args;
    if (codemaker::decomposeAndResolve(
            manager, codemaker::convertString(reader.getSuperTypeName(0)),
            false, false, false, &typeClass, &nucleus, &rank, &args)
        == codemaker::UnoType::SORT_COMPLEX)
    {
        switch (typeClass) {
        case RT_TYPE_STRUCT:
            if (!args.empty()) {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
        case RT_TYPE_ENUM:
        case RT_TYPE_INTERFACE:
        case RT_TYPE_TYPEDEF:
            dependencies->insert(nucleus);
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
    }
}

void addConstant(
    TypeManager const & manager, typereg::Reader const & reader,
    bool publishable, sal_uInt16 index, Dependencies * dependencies,
    ClassFile * classFile)
{
    OSL_ASSERT(dependencies != 0 && classFile != 0);
    RTFieldAccess flags = reader.getFieldFlags(index);
    if (flags != RT_ACCESS_CONST
        && (!publishable || flags != (RT_ACCESS_CONST | RT_ACCESS_PUBLISHED)))
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    RTConstValue fieldValue(reader.getFieldValue(index));
    sal_uInt16 valueIndex;
    RTTypeClass typeClass;
    rtl::OString nucleus;
    sal_Int32 rank;
    std::vector< rtl::OString > args;
    switch (codemaker::decomposeAndResolve(
                manager,
                codemaker::convertString(reader.getFieldTypeName(index)),
                true, false, false, &typeClass, &nucleus, &rank, &args))
    {
    case codemaker::UnoType::SORT_BOOLEAN:
        if (fieldValue.m_type != RT_TYPE_BOOL) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(fieldValue.m_value.aBool);
        break;

    case codemaker::UnoType::SORT_BYTE:
        if (fieldValue.m_type != RT_TYPE_BYTE) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(fieldValue.m_value.aByte);
        break;

    case codemaker::UnoType::SORT_SHORT:
        if (fieldValue.m_type != RT_TYPE_INT16) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(fieldValue.m_value.aShort);
        break;

    case codemaker::UnoType::SORT_UNSIGNED_SHORT:
    case codemaker::UnoType::SORT_CHAR:
        if (fieldValue.m_type != RT_TYPE_UINT16) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(fieldValue.m_value.aUShort);
        break;

    case codemaker::UnoType::SORT_LONG:
        if (fieldValue.m_type != RT_TYPE_INT32) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(fieldValue.m_value.aLong);
        break;

    case codemaker::UnoType::SORT_UNSIGNED_LONG:
        if (fieldValue.m_type != RT_TYPE_UINT32) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addIntegerInfo(
            static_cast< sal_Int32 >(fieldValue.m_value.aULong));
        break;

    case codemaker::UnoType::SORT_HYPER:
        if (fieldValue.m_type != RT_TYPE_INT64) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addLongInfo(fieldValue.m_value.aHyper);
        break;

    case codemaker::UnoType::SORT_UNSIGNED_HYPER:
        if (fieldValue.m_type != RT_TYPE_UINT64) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addLongInfo(
            static_cast< sal_Int64 >(fieldValue.m_value.aUHyper));
        break;

    case codemaker::UnoType::SORT_FLOAT:
        if (fieldValue.m_type != RT_TYPE_FLOAT) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addFloatInfo(fieldValue.m_value.aFloat);
        break;

    case codemaker::UnoType::SORT_DOUBLE:
        if (fieldValue.m_type != RT_TYPE_DOUBLE) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        valueIndex = classFile->addDoubleInfo(fieldValue.m_value.aDouble);
        break;

    default:
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    rtl::OString desc;
    rtl::OString sig;
    getFieldDescriptor(
        manager, dependencies,
        codemaker::convertString(reader.getFieldTypeName(index)),
        &desc, &sig, 0);
    classFile->addField(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC
            | ClassFile::ACC_FINAL),
        codemaker::convertString(reader.getFieldName(index)),
        desc, valueIndex, sig);
}

void handleConstantGroup(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    if (reader.getSuperTypeCount() != 0 || reader.getMethodCount() != 0
        || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    rtl::OString className(codemaker::convertString(reader.getTypeName()));
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_INTERFACE
                | ClassFile::ACC_ABSTRACT),
            className,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
            rtl::OString()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_uInt16 fields = reader.getFieldCount();
    for (sal_uInt16 i = 0; i < fields; ++i) {
        addConstant(manager, reader, false, i, dependencies, cf.get());
    }
    writeClassFile(options, className, *cf.get());
}

void handleModule(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    if (reader.getSuperTypeCount() != 0 || reader.getMethodCount() != 0
        || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    rtl::OStringBuffer buf(codemaker::convertString(reader.getTypeName()));
    buf.append('/');
    rtl::OString prefix(buf.makeStringAndClear());
    sal_uInt16 fields = reader.getFieldCount();
    for (sal_uInt16 i = 0; i < fields; ++i) {
        rtl::OString className(
            prefix + codemaker::convertString(reader.getFieldName(i)));
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< ClassFile > cf(
            new ClassFile(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PUBLIC | ClassFile::ACC_INTERFACE
                    | ClassFile::ACC_ABSTRACT),
                className,
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
                rtl::OString()));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        addConstant(manager, reader, true, i, dependencies, cf.get());
        writeClassFile(options, className, *cf.get());
    }
}

void addExceptionHandlers(
    codemaker::ExceptionTreeNode const * node,
    ClassFile::Code::Position start, ClassFile::Code::Position end,
    ClassFile::Code::Position handler, ClassFile::Code * code)
{
    OSL_ASSERT(node != 0 && code != 0);
    if (node->present) {
        code->addException(start, end, handler, node->name);
    } else {
        for (codemaker::ExceptionTreeNode::Children::const_iterator i(
                 node->children.begin());
             i != node->children.end(); ++i)
        {
            addExceptionHandlers(*i, start, end, handler, code);
        }
    }
}

void addConstructor(
    TypeManager const & manager, rtl::OString const & realJavaBaseName,
    rtl::OString const & unoName, rtl::OString const & className,
    typereg::Reader const & reader, sal_uInt16 methodIndex,
    rtl::OString const & methodName, rtl::OString const & returnType,
    bool defaultConstructor, Dependencies * dependencies, ClassFile * classFile)
{
    OSL_ASSERT(dependencies != 0 && classFile != 0);
    MethodDescriptor desc(manager, dependencies, returnType, 0, 0);
    desc.addParameter(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XComponentContext")),
        false, false, 0);
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile::Code > code(classFile->newCode());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    code->loadLocalReference(0);
    // stack: context
    code->instrInvokeinterface(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XComponentContext")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getServiceManager")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "()Lcom/sun/star/lang/XMultiComponentFactory;")),
        1);
    // stack: factory
    code->loadStringConstant(unoName);
    // stack: factory serviceName
    codemaker::ExceptionTree tree;
    ClassFile::Code::Position tryStart;
    ClassFile::Code::Position tryEnd;
    std::vector< rtl::OString > exc;
    sal_uInt16 stack;
    sal_uInt16 localIndex = 1;
    ClassFile::AccessFlags access = static_cast< ClassFile::AccessFlags >(
        ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC);
    if (defaultConstructor) {
        code->loadLocalReference(0);
        // stack: factory serviceName context
        tryStart = code->getPosition();
        code->instrInvokeinterface(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lang/XMultiComponentFactory")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "createInstanceWithContext")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "(Ljava/lang/String;Lcom/sun/star/uno/XComponentContext;)"
                    "Ljava/lang/Object;")),
            3);
        tryEnd = code->getPosition();
        // stack: instance
        stack = 3;
    } else {
        sal_uInt16 parameters = reader.getMethodParameterCount(methodIndex);
        if (parameters == 1
            && (reader.getMethodParameterFlags(methodIndex, 0)
                == (RT_PARAM_IN | RT_PARAM_REST))
            && (reader.getMethodParameterTypeName(methodIndex, 0)
                == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any"))))
        {
            desc.addParameter(
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("any")), true, true, 0);
            code->loadLocalReference(localIndex++);
            // stack: factory serviceName args
            stack = 4;
            access = static_cast< ClassFile::AccessFlags >(
                access | ClassFile::ACC_VARARGS);
        } else {
            code->loadIntegerConstant(parameters);
            // stack: factory serviceName N
            code->instrAnewarray(
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")));
            // stack: factory serviceName args
            stack = 0;
            for (sal_uInt16 i = 0; i < parameters; ++i) {
                RTParamMode flags = reader.getMethodParameterFlags(
                    methodIndex, i);
                rtl::OString paramType(
                    codemaker::convertString(
                        reader.getMethodParameterTypeName(methodIndex, i)));
                if ((flags != RT_PARAM_IN
                     && flags != (RT_PARAM_IN | RT_PARAM_REST))
                    || ((flags & RT_PARAM_REST) != 0
                        && (parameters != 1
                            || (paramType
                                != rtl::OString(
                                    RTL_CONSTASCII_STRINGPARAM("any"))))))
                {
                    throw CannotDumpException(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Bad type information"))); //TODO
                }
                desc.addParameter(paramType, false, true, 0);
                code->instrDup();
                // stack: factory serviceName args args
                code->loadIntegerConstant(i);
                // stack: factory serviceName args args i
                stack = std::max(
                    stack,
                    addLoadLocal(
                        manager, code.get(), &localIndex, false, paramType,
                        true, dependencies));
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
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lang/XMultiComponentFactory")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "createInstanceWithArgumentsAndContext")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "(Ljava/lang/String;[Ljava/lang/Object;"
                    "Lcom/sun/star/uno/XComponentContext;)Ljava/lang/Object;")),
            4);
        tryEnd = code->getPosition();
        // stack: instance
        createExceptionsAttribute(
            manager, reader, methodIndex, dependencies, &exc, &tree);
    }
    code->loadLocalReference(0);
    // stack: instance context
    code->instrInvokestatic(
        className, rtl::OString(RTL_CONSTASCII_STRINGPARAM("$castInstance")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Ljava/lang/Object;Lcom/sun/star/uno/XComponentContext;)"
                "Ljava/lang/Object;")));
    // stack: instance
    code->instrCheckcast(returnType);
    // stack: instance
    code->instrAreturn();
    if (!tree.getRoot()->present) {
        ClassFile::Code::Position pos1 = code->getPosition();
        // stack: e
        code->instrInvokevirtual(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Throwable")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("toString")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("()Ljava/lang/String;")));
        // stack: str
        localIndex = std::max< sal_uInt16 >(localIndex, 2);
        code->storeLocalReference(1);
        // stack: -
        code->instrNew(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/uno/DeploymentException")));
        // stack: ex
        code->instrDup();
        // stack: ex ex
        rtl::OStringBuffer msg;
        msg.append(
            RTL_CONSTASCII_STRINGPARAM(
                "component context fails to supply service "));
        msg.append(unoName);
        msg.append(RTL_CONSTASCII_STRINGPARAM(" of type "));
        msg.append(realJavaBaseName);
        msg.append(RTL_CONSTASCII_STRINGPARAM(": "));
        code->loadStringConstant(msg.makeStringAndClear());
        // stack: ex ex "..."
        code->loadLocalReference(1);
        // stack: ex ex "..." str
        code->instrInvokevirtual(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/String")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("concat")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "(Ljava/lang/String;)Ljava/lang/String;")));
        // stack: ex ex "..."
        code->loadLocalReference(0);
        // stack: ex ex "..." context
        code->instrInvokespecial(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/uno/DeploymentException")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "(Ljava/lang/String;Ljava/lang/Object;)V")));
        // stack: ex
        ClassFile::Code::Position pos2 = code->getPosition();
        code->instrAthrow();
        addExceptionHandlers(
            tree.getRoot(), tryStart, tryEnd, pos2, code.get());
        code->addException(
            tryStart, tryEnd, pos1,
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Exception")));
        dependencies->insert(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Exception")));
        stack = std::max< sal_uInt16 >(stack, 4);
    }
    code->setMaxStackAndLocals(stack, localIndex);
    classFile->addMethod(
        access, methodName, desc.getDescriptor(), code.get(), exc,
        desc.getSignature());
}

void handleService(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    sal_uInt16 superTypes = reader.getSuperTypeCount();
    sal_uInt16 methods = reader.getMethodCount();
    if (superTypes == 0
        ? methods != 0
        : (superTypes != 1 || reader.getFieldCount() != 0
           || reader.getReferenceCount() != 0))
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    if (superTypes == 0) {
        return;
    }
    rtl::OString unoName(codemaker::convertString(reader.getTypeName()));
    rtl::OString className(
        translateUnoTypeToJavaFullyQualifiedName(
            unoName, rtl::OString(RTL_CONSTASCII_STRINGPARAM("service"))));
    unoName = unoName.replace('/', '.');
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
            rtl::OString()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (methods > 0) {
        rtl::OString base(codemaker::convertString(
                              reader.getSuperTypeName(0)));
        rtl::OString realJavaBaseName(base.replace('/', '.'));
        dependencies->insert(base);
        dependencies->insert(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/lang/XMultiComponentFactory")));
        dependencies->insert(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/uno/DeploymentException")));
        dependencies->insert(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")));
        dependencies->insert(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "com/sun/star/uno/XComponentContext")));
        for (sal_uInt16 i = 0; i < methods; ++i) {
            rtl::OString name(codemaker::convertString(
                                  reader.getMethodName(i)));
            bool defaultCtor = name.getLength() == 0;
            if (reader.getMethodFlags(i) != RT_MODE_TWOWAY
                || (reader.getMethodReturnTypeName(i) != "void")
                || (defaultCtor
                    && (methods != 1 || reader.getMethodParameterCount(i) != 0
                        || reader.getMethodExceptionCount(i) != 0)))
            {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
            if (defaultCtor) {
                name = rtl::OString(RTL_CONSTASCII_STRINGPARAM("create"));
            } else {
                name = codemaker::java::translateUnoToJavaIdentifier(
                    name, rtl::OString(RTL_CONSTASCII_STRINGPARAM("method")));
            }
            addConstructor(
                manager, realJavaBaseName, unoName, className, reader, i, name,
                base, defaultCtor, dependencies, cf.get());
        }
        // Synthetic castInstance method:
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            std::auto_ptr< ClassFile::Code > code(cf->newCode());
            SAL_WNODEPRECATED_DECLARATIONS_POP
            code->instrNew(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")));
            // stack: type
            code->instrDup();
            // stack: type type
            code->loadStringConstant(realJavaBaseName);
            // stack: type type "..."
            code->instrGetstatic(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("INTERFACE")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "Lcom/sun/star/uno/TypeClass;")));
            // stack: type type "..." INTERFACE
            code->instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V")));
            // stack: type
            code->loadLocalReference(0);
            // stack: type instance
            code->instrInvokestatic(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/UnoRuntime")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("queryInterface")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                        "Ljava/lang/Object;")));
            // stack: instance
            code->instrDup();
            // stack: instance instance
            ClassFile::Code::Branch branch = code->instrIfnull();
            // stack: instance
            code->instrAreturn();
            code->branchHere(branch);
            code->instrPop();
            // stack: -
            code->instrNew(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/uno/DeploymentException")));
            // stack: ex
            code->instrDup();
            // stack: ex ex
            rtl::OStringBuffer msg;
            msg.append(
                RTL_CONSTASCII_STRINGPARAM(
                    "component context fails to supply service "));
            msg.append(unoName);
            msg.append(RTL_CONSTASCII_STRINGPARAM(" of type "));
            msg.append(realJavaBaseName);
            code->loadStringConstant(msg.makeStringAndClear());
            // stack: ex ex "..."
            code->loadLocalReference(1);
            // stack: ex ex "..." context
            code->instrInvokespecial(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com/sun/star/uno/DeploymentException")),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/String;Ljava/lang/Object;)V")));
            // stack: ex
            code->instrAthrow();
            code->setMaxStackAndLocals(4, 2);
            cf->addMethod(
                static_cast< ClassFile::AccessFlags >(
                    ClassFile::ACC_PRIVATE | ClassFile::ACC_STATIC
                    | ClassFile::ACC_SYNTHETIC),
                rtl::OString(RTL_CONSTASCII_STRINGPARAM("$castInstance")),
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM(
                        "(Ljava/lang/Object;Lcom/sun/star/uno/"
                        "XComponentContext;)Ljava/lang/Object;")),
                code.get(), std::vector< rtl::OString >(), rtl::OString());
        }
    }
    writeClassFile(options, className, *cf.get());
}

void handleSingleton(
    TypeManager const & manager, JavaOptions /*TODO const*/ & options,
    typereg::Reader const & reader, Dependencies * dependencies)
{
    OSL_ASSERT(dependencies != 0);
    if (reader.getSuperTypeCount() != 1 || reader.getFieldCount() != 0
        || reader.getMethodCount() != 0 || reader.getReferenceCount() != 0)
    {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    rtl::OString base(codemaker::convertString(reader.getSuperTypeName(0)));
    rtl::OString realJavaBaseName(base.replace('/', '.'));
    switch (manager.getTypeReader(base).getTypeClass()) {
    case RT_TYPE_INTERFACE:
        break;

    case RT_TYPE_SERVICE:
        return;

    default:
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
    dependencies->insert(base);
    rtl::OString unoName(codemaker::convertString(reader.getTypeName()));
    rtl::OString className(
        translateUnoTypeToJavaFullyQualifiedName(
            unoName, rtl::OString(RTL_CONSTASCII_STRINGPARAM("singleton"))));
    unoName = unoName.replace('/', '.');
    dependencies->insert(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "com/sun/star/uno/DeploymentException")));
    dependencies->insert(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")));
    dependencies->insert(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XComponentContext")));
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile > cf(
        new ClassFile(
            static_cast< ClassFile::AccessFlags >(
                ClassFile::ACC_PUBLIC | ClassFile::ACC_FINAL
                | ClassFile::ACC_SUPER),
            className,
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
            rtl::OString()));
    SAL_WNODEPRECATED_DECLARATIONS_POP
    MethodDescriptor desc(manager, dependencies, base, 0, 0);
    desc.addParameter(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XComponentContext")),
        false, false, 0);
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< ClassFile::Code > code(cf->newCode());
    SAL_WNODEPRECATED_DECLARATIONS_POP
    code->loadLocalReference(0);
    // stack: context
    code->loadStringConstant(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("/singletons/")) + unoName);
    // stack: context "..."
    code->instrInvokeinterface(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/XComponentContext")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getValueByName")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Ljava/lang/String;)Ljava/lang/Object;")),
        2);
    // stack: value
    code->instrDup();
    // stack: value value
    code->instrInstanceof(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")));
    // stack: value 0/1
    ClassFile::Code::Branch branch1 = code->instrIfeq();
    // stack: value
    code->instrCheckcast(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")));
    // stack: value
    code->instrDup();
    // stack: value value
    code->instrInvokevirtual(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getType")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()Lcom/sun/star/uno/Type;")));
    // stack: value type
    code->instrInvokevirtual(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getTypeClass")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("()Lcom/sun/star/uno/TypeClass;")));
    // stack: value typeClass
    code->instrGetstatic(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("INTERFACE")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/TypeClass;")));
    // stack: value typeClass INTERFACE
    ClassFile::Code::Branch branch2 = code->instrIfAcmpne();
    // stack: value
    code->instrInvokevirtual(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Any")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("getObject")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("()Ljava/lang/Object;")));
    // stack: value
    code->branchHere(branch1);
    code->instrNew(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")));
    // stack: value type
    code->instrDup();
    // stack: value type type
    code->loadStringConstant(realJavaBaseName);
    // stack: value type type "..."
    code->instrGetstatic(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/TypeClass")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("INTERFACE")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("Lcom/sun/star/uno/TypeClass;")));
    // stack: value type type "..." INTERFACE
    code->instrInvokespecial(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Ljava/lang/String;Lcom/sun/star/uno/TypeClass;)V")));
    // stack: value type
    code->instrSwap();
    // stack: type value
    code->instrInvokestatic(
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/UnoRuntime")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("queryInterface")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Lcom/sun/star/uno/Type;Ljava/lang/Object;)"
                "Ljava/lang/Object;")));
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
    code->instrNew(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "com/sun/star/uno/DeploymentException")));
    // stack: ex
    code->instrDup();
    // stack: ex ex
    rtl::OStringBuffer msg;
    msg.append(
        RTL_CONSTASCII_STRINGPARAM(
            "component context fails to supply singleton "));
    msg.append(unoName);
    msg.append(RTL_CONSTASCII_STRINGPARAM(" of type "));
    msg.append(realJavaBaseName);
    code->loadStringConstant(msg.makeStringAndClear());
    // stack: ex ex "..."
    code->loadLocalReference(0);
    // stack: ex ex "..." context
    code->instrInvokespecial(
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/DeploymentException")),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("<init>")),
        rtl::OString(
            RTL_CONSTASCII_STRINGPARAM(
                "(Ljava/lang/String;Ljava/lang/Object;)V")));
    // stack: ex
    code->instrAthrow();
    code->setMaxStackAndLocals(5, 1);
    cf->addMethod(
        static_cast< ClassFile::AccessFlags >(
            ClassFile::ACC_PUBLIC | ClassFile::ACC_STATIC),
        rtl::OString(RTL_CONSTASCII_STRINGPARAM("get")), desc.getDescriptor(),
        code.get(), std::vector< rtl::OString >(), desc.getSignature());
    writeClassFile(options, className, *cf.get());
}

}

bool produceType(
    rtl::OString const & type, TypeManager const & manager,
    codemaker::GeneratedTypeSet & generated, JavaOptions * options)
{
    OSL_ASSERT(options != 0);
    if (type.equals("/")
        || type.equals(manager.getBase())
        || generated.contains(type))
    {
        return true;
    }
    sal_Bool extra = sal_False;
    typereg::Reader reader(manager.getTypeReader(type, &extra));
    if (extra) {
        generated.add(type);
        return true;
    }
    if (!reader.isValid()) {
        return false;
    }

    handleUnoTypeRegistryEntityFunction handler;
    switch (reader.getTypeClass()) {
    case RT_TYPE_ENUM:
        handler = handleEnumType;
        break;

    case RT_TYPE_STRUCT:
    case RT_TYPE_EXCEPTION:
        handler = handleAggregatingType;
        break;

    case RT_TYPE_INTERFACE:
        handler = handleInterfaceType;
        break;

    case RT_TYPE_TYPEDEF:
        handler = handleTypedef;
        break;

    case RT_TYPE_CONSTANTS:
        handler = handleConstantGroup;
        break;

    case RT_TYPE_MODULE:
        handler = handleModule;
        break;

    case RT_TYPE_SERVICE:
        handler = handleService;
        break;

    case RT_TYPE_SINGLETON:
        handler = handleSingleton;
        break;

    default:
        return false;
    }
    Dependencies deps;
    handler(manager, *options, reader, &deps);
    generated.add(type);
    if (!options->isValid(rtl::OString(RTL_CONSTASCII_STRINGPARAM("-nD")))) {
        for (Dependencies::iterator i(deps.begin()); i != deps.end(); ++i) {
            if (!produceType(*i, manager, generated, options)) {
                return false;
            }
        }
    }
    return true;
}

bool produceType(
    RegistryKey & rTypeKey, bool bIsExtraType, TypeManager const & manager,
    codemaker::GeneratedTypeSet & generated, JavaOptions * options)
{
    ::rtl::OString typeName = manager.getTypeName(rTypeKey);

    OSL_ASSERT(options != 0);
    if (typeName.equals("/")
        || typeName.equals(manager.getBase())
        || generated.contains(typeName))
    {
        return true;
    }
    typereg::Reader reader(manager.getTypeReader(rTypeKey));
    if (bIsExtraType) {
        generated.add(typeName);
        return true;
    }
    if (!reader.isValid()) {
        return false;
    }
    handleUnoTypeRegistryEntityFunction handler;
    switch (reader.getTypeClass()) {
    case RT_TYPE_ENUM:
        handler = handleEnumType;
        break;

    case RT_TYPE_STRUCT:
    case RT_TYPE_EXCEPTION:
        handler = handleAggregatingType;
        break;

    case RT_TYPE_INTERFACE:
        handler = handleInterfaceType;
        break;

    case RT_TYPE_TYPEDEF:
        handler = handleTypedef;
        break;

    case RT_TYPE_CONSTANTS:
        handler = handleConstantGroup;
        break;

    case RT_TYPE_MODULE:
        handler = handleModule;
        break;

    case RT_TYPE_SERVICE:
        handler = handleService;
        break;

    case RT_TYPE_SINGLETON:
        handler = handleSingleton;
        break;

    default:
        return false;
    }
    Dependencies deps;
    handler(manager, *options, reader, &deps);
    generated.add(typeName);
    if (!options->isValid(rtl::OString(RTL_CONSTASCII_STRINGPARAM("-nD")))) {
        for (Dependencies::iterator i(deps.begin()); i != deps.end(); ++i) {
            if (!produceType(*i, manager, generated, options)) {
                return false;
            }
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
