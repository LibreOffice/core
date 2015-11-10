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


#include "classfile.hxx"

#include "codemaker/global.hxx"
#include "codemaker/options.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "sal/types.h"

#include <map>
#include <utility>
#include <vector>

using codemaker::javamaker::ClassFile;

namespace {

void appendU1(std::vector< unsigned char > & stream, sal_uInt8 data) {
    stream.push_back(static_cast< unsigned char >(data));
}

void appendU2(std::vector< unsigned char > & stream, sal_uInt16 data) {
    stream.push_back(static_cast< unsigned char >(data >> 8));
    stream.push_back(static_cast< unsigned char >(data & 0xFF));
}

void appendU4(std::vector< unsigned char > & stream, sal_uInt32 data) {
    stream.push_back(static_cast< unsigned char >(data >> 24));
    stream.push_back(static_cast< unsigned char >((data >> 16) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 8) & 0xFF));
    stream.push_back(static_cast< unsigned char >(data & 0xFF));
}

void appendU8(std::vector< unsigned char > & stream, sal_uInt64 data) {
    stream.push_back(static_cast< unsigned char >(data >> 56));
    stream.push_back(static_cast< unsigned char >((data >> 48) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 40) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 32) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 24) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 16) & 0xFF));
    stream.push_back(static_cast< unsigned char >((data >> 8) & 0xFF));
    stream.push_back(static_cast< unsigned char >(data & 0xFF));
}

void appendStream(
    std::vector< unsigned char > & stream,
    std::vector< unsigned char > const & data)
{
    stream.insert(stream.end(), data.begin(), data.end());
}

void write(FileStream & file, void const * buffer, sal_uInt64 size) {
    if (!file.write(buffer, size))
        throw CannotDumpException("Error writing file");
}

void writeU2(FileStream & file, sal_uInt16 data) {
    unsigned char buf[] = {
        static_cast< unsigned char >(data >> 8),
        static_cast< unsigned char >(data & 0xFF) };
    write(file, buf, sizeof buf);
}

void writeU4(FileStream & file, sal_uInt32 data) {
    unsigned char buf[] = {
        static_cast< unsigned char >(data >> 24),
        static_cast< unsigned char >((data >> 16) & 0xFF),
        static_cast< unsigned char >((data >> 8) & 0xFF),
        static_cast< unsigned char >(data & 0xFF) };
    write(file, buf, sizeof buf);
}

void writeStream(FileStream & file, std::vector< unsigned char > const & stream)
{
    std::vector< unsigned char >::size_type n = stream.size();
    static_assert(
        sizeof (std::vector< unsigned char >::size_type)
        <= sizeof (sal_uInt64), "must be at most equal in size");
        // both unsigned integral, so sizeof is a practically sufficient
        // approximation of std::numeric_limits<T1>::max() <=
        // std::numeric_limits<T2>::max()
    if (n != 0) {
        write(file, &stream[0], static_cast< sal_uInt64 >(n));
    }
}

}

ClassFile::Code::~Code() {}

void ClassFile::Code::instrAastore() {
    // aastore:
    appendU1(m_code, 0x53);
}

void ClassFile::Code::instrAconstNull() {
    // aconst_null:
    appendU1(m_code, 0x01);
}

void ClassFile::Code::instrAnewarray(OString const & type) {
    // anewarray <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xBD);
    appendU2(m_code, m_classFile.addClassInfo(type));
}

void ClassFile::Code::instrAreturn() {
    // areturn:
    appendU1(m_code, 0xB0);
}

void ClassFile::Code::instrAthrow() {
    // athrow:
    appendU1(m_code, 0xBF);
}

void ClassFile::Code::instrCheckcast(OString const & type) {
    // checkcast <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xC0);
    appendU2(m_code, m_classFile.addClassInfo(type));
}

void ClassFile::Code::instrDup() {
    // dup:
    appendU1(m_code, 0x59);
}

void ClassFile::Code::instrGetstatic(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // getstatic <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB2);
    appendU2(m_code, m_classFile.addFieldrefInfo(type, name, descriptor));
}

ClassFile::Code::Branch ClassFile::Code::instrIfAcmpne() {
    // if_acmpne <branchbyte1> <branchbyte2>:
    Branch branch = m_code.size();
    appendU1(m_code, 0xA6);
    appendU2(m_code, 0);
    return branch;
}

ClassFile::Code::Branch ClassFile::Code::instrIfeq() {
    // ifeq <branchbyte1> <branchbyte2>:
    Branch branch = m_code.size();
    appendU1(m_code, 0x99);
    appendU2(m_code, 0);
    return branch;
}

ClassFile::Code::Branch ClassFile::Code::instrIfnull() {
    // ifnull <branchbyte1> <branchbyte2>:
    Branch branch = m_code.size();
    appendU1(m_code, 0xC6);
    appendU2(m_code, 0);
    return branch;
}

void ClassFile::Code::instrInstanceof(OString const & type) {
    // instanceof <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xC1);
    appendU2(m_code, m_classFile.addClassInfo(type));
}

void ClassFile::Code::instrInvokeinterface(
    OString const & type, OString const & name,
    OString const & descriptor, sal_uInt8 args)
{
    // invokeinterface <indexbyte1> <indexbyte2> <nargs> 0:
    appendU1(m_code, 0xB9);
    appendU2(
        m_code, m_classFile.addInterfaceMethodrefInfo(type, name, descriptor));
    appendU1(m_code, args);
    appendU1(m_code, 0);
}

void ClassFile::Code::instrInvokespecial(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // invokespecial <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB7);
    appendU2(m_code, m_classFile.addMethodrefInfo(type, name, descriptor));
}

void ClassFile::Code::instrInvokestatic(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // invokestatic <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB8);
    appendU2(m_code, m_classFile.addMethodrefInfo(type, name, descriptor));
}

void ClassFile::Code::instrInvokevirtual(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // invokevirtual <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB6);
    appendU2(m_code, m_classFile.addMethodrefInfo(type, name, descriptor));
}

void ClassFile::Code::instrLookupswitch(
    Code const * defaultBlock,
    std::list< std::pair< sal_Int32, Code * > > const & blocks)
{
    // lookupswitch <0--3 byte pad> <defaultbyte1> <defaultbyte2> <defaultbyte3>
    // <defaultbyte4> <npairs1> <npairs2> <npairs3> <npairs4>
    // <match--offset pairs...>:
    std::list< std::pair< sal_Int32, Code * > >::size_type size = blocks.size();
    if (size > SAL_MAX_INT32) {
        throw CannotDumpException("Lookup-switch too large for Java class file format");
    }
    Position pos1 = m_code.size();
    appendU1(m_code, 0xAB);
    int pad = (pos1 + 1) % 4;
    for (int i = 0; i < pad; ++i) {
        appendU1(m_code, 0);
    }
    Position pos2 = pos1 + 1 + pad + 8 + blocks.size() * 8; //FIXME: overflow
    appendU4(m_code, static_cast< sal_uInt32 >(pos2 - pos1)); //FIXME: overflow
    pos2 += defaultBlock->m_code.size(); //FIXME: overflow
    appendU4(m_code, static_cast< sal_uInt32 >(size));
    for (std::list< std::pair< sal_Int32, Code * > >::const_iterator i(
             blocks.begin());
         i != blocks.end(); ++i)
    {
        appendU4(m_code, static_cast< sal_uInt32 >(i->first));
        appendU4(m_code, static_cast< sal_uInt32 >(pos2 - pos1));
            //FIXME: overflow
        pos2 += i->second->m_code.size(); //FIXME: overflow
    }
    appendStream(m_code, defaultBlock->m_code);
    for (std::list< std::pair< sal_Int32, Code * > >::const_iterator i(
             blocks.begin());
         i != blocks.end(); ++i)
    {
        appendStream(m_code, i->second->m_code);
    }
}

void ClassFile::Code::instrNew(OString const & type) {
    // new <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xBB);
    appendU2(m_code, m_classFile.addClassInfo(type));
}

void ClassFile::Code::instrNewarray(codemaker::UnoType::Sort sort) {
    OSL_ASSERT(
        sort >= codemaker::UnoType::SORT_BOOLEAN
        && sort <= codemaker::UnoType::SORT_CHAR);
    // newarray <atype>:
    appendU1(m_code, 0xBC);
    static sal_uInt8 const atypes[codemaker::UnoType::SORT_CHAR] = {
        0x04, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 0x06, 0x07, 0x05 };
    appendU1(m_code, atypes[sort - 1]);
}

void ClassFile::Code::instrPop() {
    // pop:
    appendU1(m_code, 0x57);
}

void ClassFile::Code::instrPutfield(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // putfield <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB5);
    appendU2(m_code, m_classFile.addFieldrefInfo(type, name, descriptor));
}

void ClassFile::Code::instrPutstatic(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    // putstatic <indexbyte1> <indexbyte2>:
    appendU1(m_code, 0xB3);
    appendU2(m_code, m_classFile.addFieldrefInfo(type, name, descriptor));
}

void ClassFile::Code::instrReturn() {
    // return:
    appendU1(m_code, 0xB1);
}

void ClassFile::Code::instrSwap() {
    // swap:
    appendU1(m_code, 0x5F);
}

void ClassFile::Code::instrTableswitch(
    Code const * defaultBlock, sal_Int32 low,
    std::list< Code * > const & blocks)
{
    // tableswitch <0--3 byte pad> <defaultbyte1> <defaultbyte2> <defaultbyte3>
    // <defaultbyte4> <lowbyte1> <lowbyte2> <lowbyte3> <lowbyte4> <highbyte1>
    // <highbyte2> <highbyte3> <highbyte4> <jump offsets...>:
    Position pos1 = m_code.size();
    appendU1(m_code, 0xAA);
    int pad = (pos1 + 1) % 4;
    for (int i = 0; i < pad; ++i) {
        appendU1(m_code, 0);
    }
    std::list< Code * >::size_type size = blocks.size();
    Position pos2 = pos1 + 1 + pad + 12 + size * 4; //FIXME: overflow
    sal_uInt32 defaultOffset = static_cast< sal_uInt32 >(pos2 - pos1);
        //FIXME: overflow
    appendU4(m_code, defaultOffset);
    pos2 += defaultBlock->m_code.size(); //FIXME: overflow
    appendU4(m_code, static_cast< sal_uInt32 >(low));
    appendU4(m_code, static_cast< sal_uInt32 >(low + (size - 1)));
    for (std::list< Code * >::const_iterator i(blocks.begin());
         i != blocks.end(); ++i)
    {
        if (*i == nullptr) {
            appendU4(m_code, defaultOffset);
        } else {
            appendU4(m_code, static_cast< sal_uInt32 >(pos2 - pos1));
                //FIXME: overflow
            pos2 += (*i)->m_code.size(); //FIXME: overflow
        }
    }
    appendStream(m_code, defaultBlock->m_code);
    for (std::list< Code * >::const_iterator i(blocks.begin());
         i != blocks.end(); ++i)
    {
        if (*i != nullptr) {
            appendStream(m_code, (*i)->m_code);
        }
    }
}

void ClassFile::Code::loadIntegerConstant(sal_Int32 value) {
    if (value >= -1 && value <= 5) {
        // iconst_<i>:
        appendU1(m_code, static_cast< sal_uInt8 >(0x02 + value + 1));
    } else if (value >= -128 && value <= 127) {
        // bipush <byte>:
        appendU1(m_code, 0x10);
        appendU1(m_code, static_cast< sal_uInt8 >(value));
    } else if (value >= -32768 && value <= 32767) {
        // sipush <byte1> <byte2>:
        appendU1(m_code, 0x11);
        appendU2(m_code, static_cast< sal_uInt16 >(value));
    } else {
        ldc(m_classFile.addIntegerInfo(value));
    }
}

void ClassFile::Code::loadStringConstant(OString const & value) {
    ldc(m_classFile.addStringInfo(value));
}

void ClassFile::Code::loadLocalInteger(sal_uInt16 index) {
    accessLocal(index, 0x1A, 0x15); // iload_<n>, iload
}

void ClassFile::Code::loadLocalLong(sal_uInt16 index) {
    accessLocal(index, 0x1E, 0x16); // load_<n>, load
}

void ClassFile::Code::loadLocalFloat(sal_uInt16 index) {
    accessLocal(index, 0x22, 0x17); // load_<n>, load
}

void ClassFile::Code::loadLocalDouble(sal_uInt16 index) {
    accessLocal(index, 0x26, 0x18); // load_<n>, load
}

void ClassFile::Code::loadLocalReference(sal_uInt16 index) {
    accessLocal(index, 0x2A, 0x19); // aload_<n>, aload
}

void ClassFile::Code::storeLocalReference(sal_uInt16 index) {
    accessLocal(index, 0x4B, 0x3A); // astore_<n>, astore
}

void ClassFile::Code::branchHere(Branch branch) {
    std::vector< unsigned char >::size_type n = m_code.size();
    OSL_ASSERT(n > branch && n - branch <= SAL_MAX_INT16);
    n -= branch;
    m_code[branch + 1] = static_cast< sal_uInt8 >(n >> 8);
    m_code[branch + 2] = static_cast< sal_uInt8 >(n & 0xFF);
}

void ClassFile::Code::addException(
    Position start, Position end, Position handler, OString const & type)
{
    OSL_ASSERT(start < end && end <= m_code.size() && handler <= m_code.size());
    if (m_exceptionTableLength == SAL_MAX_UINT16) {
        throw CannotDumpException("Too many exception handlers for Java class file format");
    }
    ++m_exceptionTableLength;
    appendU2(m_exceptionTable, static_cast< sal_uInt16 >(start));
        //FIXME: overflow
    appendU2(m_exceptionTable, static_cast< sal_uInt16 >(end));
        //FIXME: overflow
    appendU2(m_exceptionTable, static_cast< sal_uInt16 >(handler));
        //FIXME: overflow
    appendU2(m_exceptionTable, m_classFile.addClassInfo(type));
}

ClassFile::Code::Position ClassFile::Code::getPosition() const {
    return m_code.size();
}

ClassFile::Code::Code(ClassFile & classFile)
    : m_classFile(classFile)
    , m_maxStack(0)
    , m_maxLocals(0)
    , m_exceptionTableLength(0)
{}

void ClassFile::Code::ldc(sal_uInt16 index) {
    if (index <= 0xFF) {
        // ldc <index>:
        appendU1(m_code, 0x12);
        appendU1(m_code, static_cast< sal_uInt8 >(index));
    } else {
        // ldc_w <indexbyte1> <indexbyte2>:
        appendU1(m_code, 0x13);
        appendU2(m_code, index);
    }
}

void ClassFile::Code::accessLocal(
    sal_uInt16 index, sal_uInt8 fastOp, sal_uInt8 normalOp)
{
    if (index <= 3) {
        // ...load/store_<n>:
        appendU1(m_code, static_cast< sal_uInt8 >(fastOp + index));
    } else if (index <= 0xFF) {
        // ...load/store <index>:
        appendU1(m_code, normalOp);
        appendU1(m_code, static_cast< sal_uInt8 >(index));
    } else {
        // wide ...load/store <indexbyte1> <indexbyte2>:
        appendU1(m_code, 0xC4);
        appendU1(m_code, normalOp);
        appendU2(m_code, index);
    }
}

ClassFile::ClassFile(
    AccessFlags accessFlags, OString const & thisClass,
    OString const & superClass, OString const & signature):
    m_constantPoolCount(1), m_accessFlags(accessFlags), m_interfacesCount(0),
    m_fieldsCount(0), m_methodsCount(0), m_attributesCount(0)
{
    m_thisClass = addClassInfo(thisClass);
    m_superClass = addClassInfo(superClass);
    if (!signature.isEmpty()) {
        ++m_attributesCount;
        appendU2(m_attributes, addUtf8Info("Signature"));
        appendU4(m_attributes, 2);
        appendU2(m_attributes, addUtf8Info(signature));
    }
}

ClassFile::~ClassFile() {}

ClassFile::Code * ClassFile::newCode() {
    return new Code(*this);
}

sal_uInt16 ClassFile::addIntegerInfo(sal_Int32 value) {
    std::map< sal_Int32, sal_uInt16 >::iterator i(m_integerInfos.find(value));
    if (i != m_integerInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 3);
    appendU4(m_constantPool, static_cast< sal_uInt32 >(value));
    if (!m_integerInfos.insert(
            std::map< sal_Int32, sal_uInt16 >::value_type(value, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addFloatInfo(float value) {
    std::map< float, sal_uInt16 >::iterator i(m_floatInfos.find(value));
    if (i != m_floatInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 4);
    union { float floatBytes; sal_uInt32 uint32Bytes; } bytes;
    bytes.floatBytes = value;
    appendU4(m_constantPool, bytes.uint32Bytes);
    if (!m_floatInfos.insert(
            std::map< float, sal_uInt16 >::value_type(value, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addLongInfo(sal_Int64 value) {
    std::map< sal_Int64, sal_uInt16 >::iterator i(m_longInfos.find(value));
    if (i != m_longInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(2);
    appendU1(m_constantPool, 5);
    appendU8(m_constantPool, static_cast< sal_uInt64 >(value));
    if (!m_longInfos.insert(
            std::map< sal_Int64, sal_uInt16 >::value_type(value, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addDoubleInfo(double value) {
    std::map< double, sal_uInt16 >::iterator i(m_doubleInfos.find(value));
    if (i != m_doubleInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(2);
    appendU1(m_constantPool, 6);
    union { double doubleBytes; sal_uInt64 uint64Bytes; } bytes;
    bytes.doubleBytes = value;
    appendU8(m_constantPool, bytes.uint64Bytes);
    if (!m_doubleInfos.insert(
            std::map< double, sal_uInt16 >::value_type(value, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

void ClassFile::addInterface(OString const & interface) {
    if (m_interfacesCount == SAL_MAX_UINT16) {
        throw CannotDumpException("Too many interfaces for Java class file format");
    }
    ++m_interfacesCount;
    appendU2(m_interfaces, addClassInfo(interface));
}

void ClassFile::addField(
    AccessFlags accessFlags, OString const & name,
    OString const & descriptor, sal_uInt16 constantValueIndex,
    OString const & signature)
{
    if (m_fieldsCount == SAL_MAX_UINT16) {
        throw CannotDumpException("Too many fields for Java class file format");
    }
    ++m_fieldsCount;
    appendU2(m_fields, static_cast< sal_uInt16 >(accessFlags));
    appendU2(m_fields, addUtf8Info(name));
    appendU2(m_fields, addUtf8Info(descriptor));
    appendU2(
        m_fields,
        ((constantValueIndex == 0 ? 0 : 1)
         + (signature.isEmpty() ? 0 : 1)));
    if (constantValueIndex != 0) {
        appendU2(m_fields, addUtf8Info("ConstantValue"));
        appendU4(m_fields, 2);
        appendU2(m_fields, constantValueIndex);
    }
    appendSignatureAttribute(m_fields, signature);
}

void ClassFile::addMethod(
    AccessFlags accessFlags, OString const & name,
    OString const & descriptor, Code const * code,
    std::vector< OString > const & exceptions,
    OString const & signature)
{
    if (m_methodsCount == SAL_MAX_UINT16) {
        throw CannotDumpException("Too many methods for Java class file format");
    }
    ++m_methodsCount;
    appendU2(m_methods, static_cast< sal_uInt16 >(accessFlags));
    appendU2(m_methods, addUtf8Info(name));
    appendU2(m_methods, addUtf8Info(descriptor));
    std::vector< OString >::size_type excs = exceptions.size();
    if (excs > SAL_MAX_UINT16) {
        throw CannotDumpException("Too many exception specifications for Java class file format");
    }
    appendU2(
        m_methods,
        ((code == nullptr ? 0 : 1) + (exceptions.empty() ? 0 : 1)
         + (signature.isEmpty() ? 0 : 1)));
    if (code != nullptr) {
        std::vector< unsigned char >::size_type codeSize = code->m_code.size();
        std::vector< unsigned char >::size_type exceptionTableSize
            = code->m_exceptionTable.size();
        if (codeSize > SAL_MAX_UINT32 - (2 + 2 + 4 + 2 + 2)
            || (exceptionTableSize
                > (SAL_MAX_UINT32 - (2 + 2 + 4 + 2 + 2)
                   - static_cast< sal_uInt32 >(codeSize))))
        {
            throw CannotDumpException("Code block is too big for Java class file format");
        }
        appendU2(m_methods, addUtf8Info("Code"));
        appendU4(
            m_methods,
            (2 + 2 + 4 + static_cast< sal_uInt32 >(codeSize) + 2
             + static_cast< sal_uInt32 >(exceptionTableSize) + 2));
        appendU2(m_methods, code->m_maxStack);
        appendU2(m_methods, code->m_maxLocals);
        appendU4(m_methods, static_cast< sal_uInt32 >(codeSize));
        appendStream(m_methods, code->m_code);
        appendU2(m_methods, code->m_exceptionTableLength);
        appendStream(m_methods, code->m_exceptionTable);
        appendU2(m_methods, 0);
    }
    if (!exceptions.empty()) {
        appendU2(m_methods, addUtf8Info("Exceptions"));
        appendU4(
            m_methods,
            static_cast< sal_uInt32 >(2 + 2 * static_cast< sal_uInt32 >(excs)));
        appendU2(m_methods, static_cast< sal_uInt16 >(excs));
        for (std::vector< OString >::const_iterator i(exceptions.begin());
             i != exceptions.end(); ++i)
        {
            appendU2(m_methods, addClassInfo(*i));
        }
    }
    appendSignatureAttribute(m_methods, signature);
}

void ClassFile::write(FileStream & file) const {
    writeU4(file, 0xCAFEBABE);
    writeU2(file, 0);
    writeU2(file, 49); // class file version of JRE 1.5
    writeU2(file, m_constantPoolCount);
    writeStream(file, m_constantPool);
    writeU2(file, static_cast< sal_uInt16 >(m_accessFlags));
    writeU2(file, m_thisClass);
    writeU2(file, m_superClass);
    writeU2(file, m_interfacesCount);
    writeStream(file, m_interfaces);
    writeU2(file, m_fieldsCount);
    writeStream(file, m_fields);
    writeU2(file, m_methodsCount);
    writeStream(file, m_methods);
    writeU2(file, m_attributesCount);
    writeStream(file, m_attributes);
}

sal_uInt16 ClassFile::nextConstantPoolIndex(sal_uInt16 width) {
    OSL_ASSERT(width == 1 || width == 2);
    if (m_constantPoolCount > SAL_MAX_UINT16 - width) {
        throw CannotDumpException("Too many constant pool items for Java class file format");
    }
    sal_uInt16 index = m_constantPoolCount;
    m_constantPoolCount = m_constantPoolCount + width;
    return index;
}

sal_uInt16 ClassFile::addUtf8Info(OString const & value) {
    std::map< OString, sal_uInt16 >::iterator i(m_utf8Infos.find(value));
    if (i != m_utf8Infos.end()) {
        return i->second;
    }
    if (value.getLength() > SAL_MAX_UINT16) {
        throw CannotDumpException("UTF-8 string too long for Java class file format");
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 1);
    appendU2(m_constantPool, static_cast< sal_uInt16 >(value.getLength()));
    for (sal_Int32 j = 0; j < value.getLength(); ++j) {
        appendU1(m_constantPool, static_cast< sal_uInt8 >(value[j]));
    }
    if (!m_utf8Infos.insert(
            std::map< OString, sal_uInt16 >::value_type(value, index)).
        second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addClassInfo(OString const & type) {
    sal_uInt16 nameIndex = addUtf8Info(type);
    std::map< sal_uInt16, sal_uInt16 >::iterator i(
        m_classInfos.find(nameIndex));
    if (i != m_classInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 7);
    appendU2(m_constantPool, nameIndex);
    if (!m_classInfos.insert(
            std::map< sal_uInt16, sal_uInt16 >::value_type(nameIndex, index)).
        second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addStringInfo(OString const & value) {
    sal_uInt16 stringIndex = addUtf8Info(value);
    std::map< sal_uInt16, sal_uInt16 >::iterator i(
        m_stringInfos.find(stringIndex));
    if (i != m_stringInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 8);
    appendU2(m_constantPool, stringIndex);
    if (!m_stringInfos.insert(
            std::map< sal_uInt16, sal_uInt16 >::value_type(stringIndex, index)).
        second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addFieldrefInfo(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    sal_uInt16 classIndex = addClassInfo(type);
    sal_uInt16 nameAndTypeIndex = addNameAndTypeInfo(name, descriptor);
    sal_uInt32 key = (static_cast< sal_uInt32 >(classIndex) << 16)
        | nameAndTypeIndex;
    std::map< sal_uInt32, sal_uInt16 >::iterator i(m_fieldrefInfos.find(key));
    if (i != m_fieldrefInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 9);
    appendU2(m_constantPool, classIndex);
    appendU2(m_constantPool, nameAndTypeIndex);
    if (!m_fieldrefInfos.insert(
            std::map< sal_uInt32, sal_uInt16 >::value_type(key, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addMethodrefInfo(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    sal_uInt16 classIndex = addClassInfo(type);
    sal_uInt16 nameAndTypeIndex = addNameAndTypeInfo(name, descriptor);
    sal_uInt32 key = (static_cast< sal_uInt32 >(classIndex) << 16)
        | nameAndTypeIndex;
    std::map< sal_uInt32, sal_uInt16 >::iterator i(m_methodrefInfos.find(key));
    if (i != m_methodrefInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 10);
    appendU2(m_constantPool, classIndex);
    appendU2(m_constantPool, nameAndTypeIndex);
    if (!m_methodrefInfos.insert(
            std::map< sal_uInt32, sal_uInt16 >::value_type(key, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addInterfaceMethodrefInfo(
    OString const & type, OString const & name,
    OString const & descriptor)
{
    sal_uInt16 classIndex = addClassInfo(type);
    sal_uInt16 nameAndTypeIndex = addNameAndTypeInfo(name, descriptor);
    sal_uInt32 key = (static_cast< sal_uInt32 >(classIndex) << 16)
        | nameAndTypeIndex;
    std::map< sal_uInt32, sal_uInt16 >::iterator i(
        m_interfaceMethodrefInfos.find(key));
    if (i != m_interfaceMethodrefInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 11);
    appendU2(m_constantPool, classIndex);
    appendU2(m_constantPool, nameAndTypeIndex);
    if (!m_interfaceMethodrefInfos.insert(
            std::map< sal_uInt32, sal_uInt16 >::value_type(key, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

sal_uInt16 ClassFile::addNameAndTypeInfo(
    OString const & name, OString const & descriptor)
{
    sal_uInt16 nameIndex = addUtf8Info(name);
    sal_uInt16 descriptorIndex = addUtf8Info(descriptor);
    sal_uInt32 key = (static_cast< sal_uInt32 >(nameIndex) << 16)
        | descriptorIndex;
    std::map< sal_uInt32, sal_uInt16 >::iterator i(
        m_nameAndTypeInfos.find(key));
    if (i != m_nameAndTypeInfos.end()) {
        return i->second;
    }
    sal_uInt16 index = nextConstantPoolIndex(1);
    appendU1(m_constantPool, 12);
    appendU2(m_constantPool, nameIndex);
    appendU2(m_constantPool, descriptorIndex);
    if (!m_nameAndTypeInfos.insert(
            std::map< sal_uInt32, sal_uInt16 >::value_type(key, index)).second)
    {
        OSL_ASSERT(false);
    }
    return index;
}

void ClassFile::appendSignatureAttribute(
    std::vector< unsigned char > & stream, OString const & signature)
{
    if (!signature.isEmpty()) {
        appendU2(stream, addUtf8Info("Signature"));
        appendU4(stream, 2);
        appendU2(stream, addUtf8Info(signature));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
