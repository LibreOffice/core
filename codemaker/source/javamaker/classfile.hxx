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

#ifndef INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_CLASSFILE_HXX
#define INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_CLASSFILE_HXX

#include "codemaker/unotype.hxx"
#include "sal/types.h"

#include <list>
#include <map>
#include <utility>
#include <vector>

class FileStream;

namespace codemaker { namespace javamaker {

class ClassFile {
public:
    enum AccessFlags {
        ACC_PUBLIC = 0x0001,
        ACC_PRIVATE = 0x0002,
        ACC_STATIC = 0x0008,
        ACC_FINAL = 0x0010,
        ACC_SUPER = 0x0020,
        ACC_VARARGS = 0x0080,
        ACC_INTERFACE = 0x0200,
        ACC_ABSTRACT = 0x0400,
        ACC_SYNTHETIC = 0x1000
    };

    class Code {
    public:
        typedef std::vector< unsigned char >::size_type Branch;
        typedef std::vector< unsigned char >::size_type Position;

        ~Code();

        void instrAastore();
        void instrAconstNull();
        void instrAnewarray(OString const & type);
        void instrAreturn();
        void instrAthrow();
        void instrCheckcast(OString const & type);
        void instrDup();

        void instrGetstatic(
            OString const & type, OString const & name,
            OString const & descriptor);

        Branch instrIfAcmpne();
        Branch instrIfeq();
        Branch instrIfnull();

        void instrInstanceof(OString const & type);

        void instrInvokeinterface(
            OString const & type, OString const & name,
            OString const & descriptor, sal_uInt8 args);

        void instrInvokespecial(
            OString const & type, OString const & name,
            OString const & descriptor);

        void instrInvokestatic(
            OString const & type, OString const & name,
            OString const & descriptor);

        void instrInvokevirtual(
            OString const & type, OString const & name,
            OString const & descriptor);

        void instrLookupswitch(
            Code const * defaultBlock,
            std::list< std::pair< sal_Int32, Code * > > const & blocks);

        void instrNew(OString const & type);
        void instrNewarray(codemaker::UnoType::Sort sort);
        void instrPop();

        void instrPutfield(
            OString const & type, OString const & name,
            OString const & descriptor);

        void instrPutstatic(
            OString const & type, OString const & name,
            OString const & descriptor);

        void instrReturn();
        void instrSwap();

        void instrTableswitch(
            Code const * defaultBlock, sal_Int32 low,
            std::list< Code * > const & blocks);

        void loadIntegerConstant(sal_Int32 value);
        void loadStringConstant(OString const & value);
        void loadLocalInteger(sal_uInt16 index);
        void loadLocalLong(sal_uInt16 index);
        void loadLocalFloat(sal_uInt16 index);
        void loadLocalDouble(sal_uInt16 index);
        void loadLocalReference(sal_uInt16 index);
        void storeLocalReference(sal_uInt16 index);
        void branchHere(Branch branch);

        void addException(
            Position start, Position end, Position handler,
            OString const & type);

        void setMaxStackAndLocals(sal_uInt16 maxStack, sal_uInt16 maxLocals)
        { m_maxStack = maxStack; m_maxLocals = maxLocals; }

        Position getPosition() const;

    private:
        Code(Code &); // not implemented
        void operator =(Code); // not implemented

        Code(ClassFile & classFile);

        void ldc(sal_uInt16 index);

        void accessLocal(
            sal_uInt16 index, sal_uInt8 fastOp, sal_uInt8 normalOp);

        ClassFile & m_classFile;
        sal_uInt16 m_maxStack;
        sal_uInt16 m_maxLocals;
        std::vector< unsigned char > m_code;
        sal_uInt16 m_exceptionTableLength;
        std::vector< unsigned char > m_exceptionTable;

        friend class ClassFile;
    };

    ClassFile(
        AccessFlags accessFlags, OString const & thisClass,
        OString const & superClass, OString const & signature);

    ~ClassFile();

    Code * newCode();

    sal_uInt16 addIntegerInfo(sal_Int32 value);
    sal_uInt16 addFloatInfo(float value);
    sal_uInt16 addLongInfo(sal_Int64 value);
    sal_uInt16 addDoubleInfo(double value);

    void addInterface(OString const & interface);

    void addField(
        AccessFlags accessFlags, OString const & name,
        OString const & descriptor, sal_uInt16 constantValueIndex,
        OString const & signature);

    void addMethod(
        AccessFlags accessFlags, OString const & name,
        OString const & descriptor, Code const * code,
        std::vector< OString > const & exceptions,
        OString const & signature);

    void write(FileStream & file) const; //TODO

private:
    typedef std::map< OString, sal_uInt16 > Map;

    ClassFile(ClassFile &); // not implemented
    void operator =(ClassFile); // not implemented

    sal_uInt16 nextConstantPoolIndex(sal_uInt16 width);
    sal_uInt16 addUtf8Info(OString const & value);
    sal_uInt16 addClassInfo(OString const & type);
    sal_uInt16 addStringInfo(OString const & value);

    sal_uInt16 addFieldrefInfo(
        OString const & type, OString const & name,
        OString const & descriptor);

    sal_uInt16 addMethodrefInfo(
        OString const & type, OString const & name,
        OString const & descriptor);

    sal_uInt16 addInterfaceMethodrefInfo(
        OString const & type, OString const & name,
        OString const & descriptor);

    sal_uInt16 addNameAndTypeInfo(
        OString const & name, OString const & descriptor);

    void appendSignatureAttribute(
        std::vector< unsigned char > & stream, OString const & signature);

    sal_uInt16 m_constantPoolCount;
    std::vector< unsigned char > m_constantPool;
    std::map< OString, sal_uInt16 > m_utf8Infos;
    std::map< sal_Int32, sal_uInt16 > m_integerInfos;
    std::map< sal_Int64, sal_uInt16 > m_longInfos;
    std::map< float, sal_uInt16 > m_floatInfos;
    std::map< double, sal_uInt16 > m_doubleInfos;
    std::map< sal_uInt16, sal_uInt16 > m_classInfos;
    std::map< sal_uInt16, sal_uInt16 > m_stringInfos;
    std::map< sal_uInt32, sal_uInt16 > m_fieldrefInfos;
    std::map< sal_uInt32, sal_uInt16 > m_methodrefInfos;
    std::map< sal_uInt32, sal_uInt16 > m_interfaceMethodrefInfos;
    std::map< sal_uInt32, sal_uInt16 > m_nameAndTypeInfos;
    AccessFlags m_accessFlags;
    sal_uInt16 m_thisClass;
    sal_uInt16 m_superClass;
    sal_uInt16 m_interfacesCount;
    std::vector< unsigned char > m_interfaces;
    sal_uInt16 m_fieldsCount;
    std::vector< unsigned char > m_fields;
    sal_uInt16 m_methodsCount;
    std::vector< unsigned char > m_methods;
    sal_uInt16 m_attributesCount;
    std::vector< unsigned char > m_attributes;

    friend class Code;
};

} }

#endif // INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_CLASSFILE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
