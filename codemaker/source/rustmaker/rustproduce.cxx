/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rustproduce.hxx"
#include "rustfile.hxx"
#include <codemaker/typemanager.hxx>
#include <sal/log.hxx>
#include <iostream>
#include <set>

RustProducer::RustProducer(std::string_view outputDir, bool verbose, bool dryRun,
                           const rtl::Reference<TypeManager>& typeManager)
    : m_outputDir(outputDir)
    , m_verbose(verbose)
    , m_dryRun(dryRun)
    , m_typeManager(typeManager)
{
}

void RustProducer::produceEnum(std::string_view name,
                               const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    RustFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[rust-opaque-enum] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    generateEnumDefinition(file, name, entity);
    generateEnumImplementation(file, name);
    generateEnumExternDeclarations(file, name);

    file.closeFile();
}

void RustProducer::generateEnumDefinition(RustFile& file, std::string_view name,
                                          const rtl::Reference<unoidl::EnumTypeEntity>& entity)
{
    OString typeName(splitName(name)); // Simple name for Rust enum

    file.beginLine()
        .append("/// Opaque Rust enum wrapper for ")
        .append(name)
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("#[repr(C)]")
        .endLine()
        .beginLine()
        .append("#[allow(non_camel_case_types)]")
        .endLine()
        .beginLine()
        .append("#[derive(Debug, Clone, Copy, PartialEq, Eq)]")
        .endLine()
        .beginLine()
        .append("pub enum ")
        .append(typeName)
        .endLine()
        .beginBlock();

    // Track used discriminant values to avoid duplicates
    std::set<sal_Int32> usedValues;

    for (const auto& member : entity->getMembers())
    {
        // Only include members with unique discriminant values
        if (usedValues.find(member.value) == usedValues.end())
        {
            usedValues.insert(member.value);
            file.beginLine()
                .append(member.name)
                .append(" = ")
                .append(OString::number(member.value))
                .append(",")
                .endLine();
        }
    }

    file.endBlock();
}

void RustProducer::generateEnumImplementation(RustFile& file, std::string_view name)
{
    OString typeName(splitName(name)); // Simple name for Rust enum
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Add conversion functions using extern "C" bridge
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(typeName)
        .endLine()
        .beginBlock()
        .beginLine()
        .append("pub fn from_i32(value: i32) -> Self {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("unsafe { ")
        .append(externFunctionPrefix)
        .append("_from_i32(value) }")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn to_i32(self) -> i32 {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("unsafe { ")
        .append(externFunctionPrefix)
        .append("_to_i32(self) }")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .endBlock();
}

void RustProducer::generateEnumExternDeclarations(RustFile& file, std::string_view name)
{
    OString typeName(splitName(name)); // Simple name for Rust enum
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Extern "C" declarations
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("unsafe extern \"C\" {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_from_i32(value: i32) -> ")
        .append(typeName)
        .append(";")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_to_i32(value: ")
        .append(typeName)
        .append(") -> i32;")
        .endLine()
        .beginLine()
        .append("}")
        .endLine();
}

void RustProducer::produceStruct(std::string_view name,
                                 const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    RustFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[rust-opaque-struct] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    generateStructDefinition(file, name, entity);
    generateStructImplementation(file, name, entity);
    generateStructDropTrait(file, name, entity);
    generateStructExternDeclarations(file, name, entity);

    file.closeFile();
}

void RustProducer::generateStructDefinition(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::PlainStructTypeEntity>& /* entity */)
{
    OString typeName(splitName(name)); // Simple name for Rust struct

    file.beginLine()
        .append("/// Opaque Rust struct wrapper for ")
        .append(name)
        .endLine()
        .beginLine()
        .append("use std::ffi::c_void;")
        .endLine()
        .beginLine()
        .append("#[allow(non_camel_case_types)]")
        .endLine()
        .beginLine()
        .append("pub struct ")
        .append(typeName)
        .endLine()
        .beginBlock()
        .beginLine()
        .append("ptr: *mut c_void,")
        .endLine()
        .endBlock();
}

void RustProducer::generateStructImplementation(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    OString typeName(splitName(name)); // Simple name for Rust struct
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Implementation with opaque field accessors
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(typeName)
        .endLine()
        .beginBlock();

    generateStructConstructor(file, externFunctionPrefix);
    generateStructFromPtr(file, externFunctionPrefix);

    // Generate as_ptr method
    generateStructAsPtr(file);

    generateStructAccessors(file, entity, externFunctionPrefix);

    file.endBlock();
}

void RustProducer::generateStructConstructor(RustFile& file, std::string_view externFunctionPrefix)
{
    // Constructor
    file.beginLine()
        .append("pub fn new() -> Option<Self> {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("let ptr = unsafe { ")
        .append(externFunctionPrefix)
        .append("_constructor() };")
        .endLine()
        .beginLine()
        .append("if ptr.is_null() {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("None")
        .endLine()
        .beginLine()
        .append("} else {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("Some(Self { ptr })")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .beginLine()
        .append("}")
        .endLine();
}

void RustProducer::generateStructFromPtr(RustFile& file, std::string_view externFunctionPrefix)
{
    // from_ptr method for creating wrapper from existing pointer with C++ type casting
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn from_ptr(ptr: *mut c_void) -> Option<Self>")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("let casted_ptr = unsafe { ")
        .append(externFunctionPrefix)
        .append("_from_ptr(ptr) };")
        .endLine()
        .beginLine()
        .append("if casted_ptr.is_null()")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("None")
        .endLine()
        .endBlock()
        .beginLine()
        .append("else")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("Some(Self { ptr: casted_ptr })")
        .endLine()
        .endBlock()
        .endBlock();
}

void RustProducer::generateStructAsPtr(RustFile& file)
{
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn as_ptr(&self) -> *mut c_void")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("self.ptr")
        .endLine()
        .endBlock();
}

void RustProducer::generateStructAccessors(
    RustFile& file, const rtl::Reference<unoidl::PlainStructTypeEntity>& entity,
    std::string_view externFunctionPrefix)
{
    // Generate getters and setters for all struct members
    for (const auto& member : entity->getDirectMembers())
    {
        OString memberName = u2b(member.name); // Use original case, not snake_case
        std::u16string_view memberType = member.type;

        generateStructMemberGetter(file, memberName, memberType, externFunctionPrefix);
        generateStructMemberSetter(file, memberName, memberType, externFunctionPrefix);
    }
}

void RustProducer::generateStructMemberGetter(RustFile& file, std::string_view memberName,
                                              std::u16string_view memberType,
                                              std::string_view externFunctionPrefix)
{
    OString returnType = getRustStructGetterReturnType(memberType);

    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn get_")
        .append(memberName)
        .append("(&self) -> ")
        .append(returnType)
        .append(" {")
        .endLine()
        .extraIndent()
        .beginLine();

    // Handle conversion based on type
    if (memberType == u"string")
    {
        // String type - convert from FFI to high-level type
        file.append("unsafe { crate::core::OUString::from_raw(")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(self.ptr)) }");
    }
    else if (memberType == u"any")
    {
        // Any type - convert from FFI to high-level type
        file.append("unsafe { crate::core::Any::from_raw(std::ptr::read(")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(self.ptr) as *const crate::ffi::uno_any::uno_Any)) }");
    }
    else if (memberType == u"boolean" || memberType == u"byte" || memberType == u"short"
             || memberType == u"unsigned short" || memberType == u"long"
             || memberType == u"unsigned long" || memberType == u"hyper"
             || memberType == u"unsigned hyper" || memberType == u"float"
             || memberType == u"double")
    {
        // Primitive types - dereference the pointer returned from FFI
        file.append("unsafe { *")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(self.ptr) }");
    }
    else
    {
        // Other types - return raw pointer
        file.append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(self.ptr) }");
    }

    file.endLine().beginLine().append("}").endLine();
}

void RustProducer::generateStructMemberSetter(RustFile& file, std::string_view memberName,
                                              std::u16string_view memberType,
                                              std::string_view externFunctionPrefix)
{
    OString parameterType = getRustStructSetterParameterType(memberType);

    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn set_")
        .append(memberName)
        .append("(&mut self, value: ")
        .append(parameterType)
        .append(") {")
        .endLine()
        .extraIndent()
        .beginLine();

    // Handle conversion based on type
    if (memberType == u"string")
    {
        // String type - convert from high-level to FFI type
        file.append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(self.ptr, value.as_ptr()) }");
    }
    else if (memberType == u"any")
    {
        // Any type - convert from high-level to FFI type
        file.append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(self.ptr, value.as_ptr()) }");
    }
    else if (memberType == u"boolean" || memberType == u"byte" || memberType == u"short"
             || memberType == u"unsigned short" || memberType == u"long"
             || memberType == u"unsigned long" || memberType == u"hyper"
             || memberType == u"unsigned hyper" || memberType == u"float"
             || memberType == u"double")
    {
        // Primitive types - pass by value directly
        file.append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(self.ptr, value) }");
    }
    else
    {
        // Other types - pass raw pointer through
        file.append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(self.ptr, value) }");
    }

    file.endLine().beginLine().append("}").endLine();
}

void RustProducer::generateStructDropTrait(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::PlainStructTypeEntity>& /* entity */)
{
    OString typeName(splitName(name)); // Simple name for Rust struct
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Drop implementation
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl Drop for ")
        .append(typeName)
        .endLine()
        .beginBlock()
        .beginLine()
        .append("fn drop(&mut self) {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("if !self.ptr.is_null() {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("unsafe { ")
        .append(externFunctionPrefix)
        .append("_destroy(self.ptr) }")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .endBlock();
}

void RustProducer::generateStructExternDeclarations(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::PlainStructTypeEntity>& entity)
{
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Extern "C" declarations for opaque operations
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("unsafe extern \"C\" {")
        .endLine()
        .extraIndent();

    generateStructBasicExternDeclarations(file, externFunctionPrefix);
    generateStructMemberExternDeclarations(file, entity, externFunctionPrefix);

    file.beginLine().append("}").endLine();
}

void RustProducer::generateStructBasicExternDeclarations(RustFile& file,
                                                         std::string_view externFunctionPrefix)
{
    file.beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_constructor() -> *mut c_void;")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_destroy(ptr: *mut c_void);")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_from_ptr(ptr: *mut c_void) -> *mut c_void;")
        .endLine();
}

void RustProducer::generateStructMemberExternDeclarations(
    RustFile& file, const rtl::Reference<unoidl::PlainStructTypeEntity>& entity,
    std::string_view externFunctionPrefix)
{
    for (const auto& member : entity->getDirectMembers())
    {
        OString memberName = u2b(member.name); // Use original case, not snake_case
        OString getterReturnType = getRustStructExternGetterReturnType(member.type);
        OString setterParamType = getRustStructExternSetterParameterType(member.type);

        file.beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(ptr: *mut c_void) -> ")
            .append(getterReturnType)
            .append(";")
            .endLine()
            .beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(ptr: *mut c_void, value: ")
            .append(setterParamType)
            .append(");")
            .endLine();
    }
}

void RustProducer::produceInterface(std::string_view name,
                                    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    RustFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[rust-opaque-interface] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    generateInterfaceWrapper(file, name, entity);
    generateInterfaceExternDeclarations(file, name, entity);

    file.closeFile();
}

void RustProducer::generateInterfaceWrapper(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    OString typeName(splitName(name)); // Simple name for Rust interface
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Generate struct header and documentation
    file.beginLine()
        .append("/// Opaque Rust interface wrapper for ")
        .append(name)
        .endLine()
        .beginLine()
        .append("use std::ffi::c_void;")
        .endLine()
        .beginLine()
        .append("#[allow(non_camel_case_types)]")
        .endLine()
        .beginLine()
        .append("pub struct ")
        .append(typeName)
        .endLine()
        .beginBlock()
        .beginLine()
        .append("ptr: *mut c_void,")
        .endLine()
        .endBlock();

    // Generate implementation block with constructor, methods, etc.
    generateInterfaceImplementation(file, typeName, externFunctionPrefix, entity);

    // Generate Drop trait implementation
    generateInterfaceDropTrait(file, typeName, externFunctionPrefix);

    // Generate thread safety markers
    generateInterfaceThreadSafety(file, typeName);
}

void RustProducer::generateInterfaceImplementation(
    RustFile& file, std::string_view typeName, std::string_view externFunctionPrefix,
    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(typeName)
        .endLine()
        .beginBlock();

    // Generate constructor
    generateInterfaceConstructor(file, externFunctionPrefix);

    // Generate from_ptr method
    generateInterfaceFromPtr(file, externFunctionPrefix);

    // Generate as_ptr method
    generateInterfaceAsPtr(file);

    // Generate validity check if no conflicting method
    generateInterfaceValidityCheck(file, externFunctionPrefix, entity);

    // Generate method wrappers
    generateInterfaceMethodWrappers(file, externFunctionPrefix, entity);

    file.endBlock();
}

void RustProducer::generateInterfaceConstructor(RustFile& file,
                                                std::string_view externFunctionPrefix)
{
    file.beginLine()
        .append("pub fn new() -> Option<Self> {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("let ptr = unsafe { ")
        .append(externFunctionPrefix)
        .append("_constructor() };")
        .endLine()
        .beginLine()
        .append("if ptr.is_null() {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("None")
        .endLine()
        .beginLine()
        .append("} else {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("Some(Self { ptr })")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .beginLine()
        .append("}")
        .endLine();
}

void RustProducer::generateInterfaceFromPtr(RustFile& file, std::string_view externFunctionPrefix)
{
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn from_ptr(ptr: *mut c_void) -> Option<Self>")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("let casted_ptr = unsafe { ")
        .append(externFunctionPrefix)
        .append("_from_ptr(ptr) };")
        .endLine()
        .beginLine()
        .append("if casted_ptr.is_null()")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("None")
        .endLine()
        .endBlock()
        .beginLine()
        .append("else")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("Some(Self { ptr: casted_ptr })")
        .endLine()
        .endBlock()
        .endBlock();
}

void RustProducer::generateInterfaceAsPtr(RustFile& file)
{
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub fn as_ptr(&self) -> *mut c_void")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("self.ptr")
        .endLine()
        .endBlock();
}

void RustProducer::generateInterfaceValidityCheck(
    RustFile& file, std::string_view externFunctionPrefix,
    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Check if there's already an isValid method to avoid conflicts
    bool hasIsValidMethod = false;
    for (const auto& method : entity->getDirectMethods())
    {
        if (u2b(method.name).equalsIgnoreAsciiCase("isValid"))
        {
            hasIsValidMethod = true;
            break;
        }
    }

    if (!hasIsValidMethod)
    {
        file.beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("pub fn is_valid(&self) -> bool {")
            .endLine()
            .extraIndent()
            .beginLine()
            .append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_is_valid(self.ptr) }")
            .endLine()
            .beginLine()
            .append("}")
            .endLine();
    }
}

void RustProducer::generateInterfaceMethodWrappers(
    RustFile& file, std::string_view externFunctionPrefix,
    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    // Generate method wrappers - all methods return opaque pointers
    for (const auto& method : entity->getDirectMethods())
    {
        OString rustMethodName = getRustFunctionName(u2b(method.name));

        file.beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("pub fn ")
            .append(rustMethodName)
            .append("(&self");

        // Parameters with typed support for primitives
        for (const auto& param : method.parameters)
        {
            file.append(", ")
                .append(param.name)
                .append(": ")
                .append(getRustParameterType(param.type, param.direction));
        }

        file.append(") -> ")
            .append(getRustReturnType(method.returnType))
            .append(" {")
            .endLine()
            .extraIndent();

        // Handle return value conversion based on return type
        if (method.returnType == u"void")
        {
            // Void methods - just call the function with semicolon (no return)
            file.beginLine()
                .append("unsafe { ")
                .append(externFunctionPrefix)
                .append("_")
                .append(method.name)
                .append("(self.ptr");

            for (const auto& param : method.parameters)
            {
                file.append(", ").append(
                    convertRustParameterForFFICall(param.type, u2b(param.name), param.direction));
            }

            file.append(") };").endLine();
        }
        else
        {
            // Non-void methods - call function and convert result
            file.beginLine()
                .append("let ptr = unsafe { ")
                .append(externFunctionPrefix)
                .append("_")
                .append(method.name)
                .append("(self.ptr");

            for (const auto& param : method.parameters)
            {
                file.append(", ").append(
                    convertRustParameterForFFICall(param.type, u2b(param.name), param.direction));
            }

            file.append(") };").endLine();

            // Convert result based on return type
            if (isUnoInterface(method.returnType) || isUnoStruct(method.returnType)
                || isUnoEnum(method.returnType))
            {
                // For interface/struct/enum returns - use service-style conversion
                OString rustType = u2b(method.returnType);
                if (rustType.indexOf('.') != -1)
                {
                    sal_Int32 lastDot = rustType.lastIndexOf('.');
                    if (lastDot != -1)
                    {
                        OString simpleName = rustType.copy(lastDot + 1);
                        OString modulePath = rustType.replaceAll("."_ostr, "::"_ostr);

                        file.beginLine().append("if ptr.is_null() {").endLine();
                        file.extraIndent().beginLine().append("None").endLine();
                        file.beginLine().append("} else {").endLine();
                        // Check if this is an enum type
                        if (isUnoEnum(method.returnType))
                        {
                            // Enum types - cast from void* to i32 and use from_i32
                            file.extraIndent()
                                .beginLine()
                                .append("Some(crate::generated::rustmaker::")
                                .append(modulePath)
                                .append("::")
                                .append(simpleName)
                                .append("::from_i32(unsafe { *(ptr as *const i32) }))")
                                .endLine();
                        }
                        else
                        {
                            // Interface and struct types - use from_ptr
                            file.extraIndent()
                                .beginLine()
                                .append("crate::generated::rustmaker::")
                                .append(modulePath)
                                .append("::")
                                .append(simpleName)
                                .append("::from_ptr(ptr)")
                                .endLine();
                        }
                        file.beginLine().append("}").endLine();
                    }
                }
            }
            else
            {
                // Convert raw pointer to wrapper type for other types
                OString rustType = u2b(method.returnType);

                // Handle special types that need conversion
                if (rustType == "string")
                {
                    file.beginLine().append("if ptr.is_null() {").endLine();
                    file.extraIndent().beginLine().append("None").endLine();
                    file.beginLine().append("} else {").endLine();
                    file.extraIndent()
                        .beginLine()
                        .append("Some(unsafe { crate::core::OUString::from_raw(ptr as *mut "
                                "crate::ffi::rtl_string::rtl_uString) })")
                        .endLine();
                    file.beginLine().append("}").endLine();
                }
                else if (rustType == "any" || rustType == "com.sun.star.uno.Any")
                {
                    file.beginLine().append("if ptr.is_null() {").endLine();
                    file.extraIndent().beginLine().append("None").endLine();
                    file.beginLine().append("} else {").endLine();
                    file.extraIndent()
                        .beginLine()
                        .append("Some(unsafe { crate::core::Any::from_raw(std::ptr::read(ptr as "
                                "*mut crate::ffi::uno_any::uno_Any)) })")
                        .endLine();
                    file.beginLine().append("}").endLine();
                }
                else
                {
                    // Check for typedef resolution first
                    OString resolvedType(resolveTypedef(method.returnType));
                    // It's a typedef - handle based on resolved type
                    if (!generateTypeCastReturn(file, resolvedType))
                    {
                        // Typedef resolves to non-primitive types - return raw pointer
                        file.beginLine().append("ptr").endLine();
                    }
                }
            }
        }

        file.beginLine().append("}").endLine();
    }
}

void RustProducer::generateInterfaceDropTrait(RustFile& file, std::string_view typeName,
                                              std::string_view externFunctionPrefix)
{
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl Drop for ")
        .append(typeName)
        .endLine()
        .beginBlock()
        .beginLine()
        .append("fn drop(&mut self) {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("if !self.ptr.is_null() {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("unsafe { ")
        .append(externFunctionPrefix)
        .append("_destructor(self.ptr) }")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .beginLine()
        .append("}")
        .endLine()
        .endBlock();
}

void RustProducer::generateInterfaceThreadSafety(RustFile& file, std::string_view typeName)
{
    // Thread safety markers
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("unsafe impl Send for ")
        .append(typeName)
        .append(" {}")
        .endLine()
        .beginLine()
        .append("unsafe impl Sync for ")
        .append(typeName)
        .append(" {}")
        .endLine();
}

void RustProducer::generateInterfaceExternDeclarations(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::InterfaceTypeEntity>& entity)
{
    OString externFunctionPrefix(getRustTypeName(name));

    // Extern "C" declarations - connects to C-side bridge
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("unsafe extern \"C\" {")
        .endLine()
        .extraIndent()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_constructor() -> *mut c_void;")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_destructor(ptr: *mut c_void);")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_from_ptr(ptr: *mut c_void) -> *mut c_void;")
        .endLine();

    // Check if there's already an isValid method to avoid conflicts
    bool hasIsValidMethod = false;
    for (const auto& method : entity->getDirectMethods())
    {
        if (u2b(method.name).equalsIgnoreAsciiCase("isValid"))
        {
            hasIsValidMethod = true;
            break;
        }
    }

    // Only declare _is_valid if we don't have a conflicting isValid method
    if (!hasIsValidMethod)
    {
        file.beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_is_valid(ptr: *mut c_void) -> bool;")
            .endLine();
    }

    for (const auto& method : entity->getDirectMethods())
    {
        file.beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_")
            .append(method.name)
            .append("(ptr: *mut c_void");

        for (const auto& param : method.parameters)
        {
            file.append(", ")
                .append(param.name)
                .append(": ")
                .append(getRustExternParameterType(param.type, param.direction));
        }

        file.append(") -> ");

        if (method.returnType == u"void")
        {
            file.append("()");
        }
        else
        {
            file.append("*mut c_void");
        }

        file.append(";").endLine();
    }

    file.beginLine().append("}").endLine();
}

void RustProducer::produceService(
    std::string_view name, const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    RustFile file(m_outputDir, name);

    if (m_verbose)
        std::cout << "[rust-opaque-service] " << name << " -> " << file.getPath() << '\n';
    if (m_dryRun)
        return;

    file.openFile();

    generateServiceDefinition(file, name, entity);
    generateServiceImplementation(file, name, entity);
    generateServiceExternDeclarations(file, name, entity);

    file.closeFile();
}

void RustProducer::generateServiceDefinition(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& /* entity */)
{
    OString serviceName(splitName(name)); // Simple name for Rust service

    file.beginLine()
        .append("/// Opaque Rust service wrapper for ")
        .append(name)
        .endLine()
        .beginLine()
        .append("use std::ffi::c_void;")
        .endLine()
        .beginLine()
        .append("#[allow(non_camel_case_types)]")
        .endLine()
        .append("")
        .endLine()
        .beginLine()
        .append("pub struct ")
        .append(serviceName)
        .append(";")
        .endLine();
}

void RustProducer::generateServiceImplementation(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    OString serviceName(splitName(name)); // Simple name for Rust service

    // Implementation block
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(serviceName)
        .endLine()
        .beginBlock();

    generateServiceCreateMethod(file, name, entity);

    file.endBlock();
}

void RustProducer::generateServiceCreateMethod(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& entity)
{
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions
    OUString interfaceType = entity->getBase();

    // Generate proper module path for the interface
    OString interfaceModulePath = generateServiceInterfaceModulePath(interfaceType);

    // Service creation method
    file.beginLine()
        .append("/// Create a new instance of ")
        .append(name)
        .endLine()
        .beginLine()
        .append("pub fn create(context: *mut c_void) -> Option<")
        .append(interfaceModulePath)
        .append("> ")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("let ptr = unsafe { ")
        .append(externFunctionPrefix)
        .append("_create(context) };")
        .endLine()
        .beginLine()
        .append("if ptr.is_null() ")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("None")
        .endLine()
        .endBlock()
        .beginLine()
        .append(" else ")
        .endLine()
        .beginBlock()
        .beginLine()
        .append(interfaceModulePath)
        .append("::from_ptr(ptr)")
        .endLine()
        .endBlock()
        .endBlock();
}

OString RustProducer::generateServiceInterfaceModulePath(const OUString& interfaceType)
{
    // Generate proper module path for the interface
    OString interfaceTypeStr = u2b(interfaceType);
    std::string_view interfaceTypeName = splitName(interfaceTypeStr);

    // Convert interface type to full module path
    OString path = interfaceTypeStr;
    path = path.replaceAll("."_ostr, "::"_ostr);
    return "crate::generated::rustmaker::"_ostr + path + "::" + interfaceTypeName;
}

void RustProducer::generateServiceExternDeclarations(
    RustFile& file, std::string_view name,
    const rtl::Reference<unoidl::SingleInterfaceBasedServiceEntity>& /* entity */)
{
    OString externFunctionPrefix(getRustTypeName(name)); // Full name for extern "C" functions

    // Extern "C" declarations
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("unsafe extern \"C\" ")
        .endLine()
        .beginBlock()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_create(context: *mut c_void) -> *mut c_void;")
        .endLine()
        .endBlock();
}

bool RustProducer::generateTypeCastReturn(RustFile& file, std::string_view resolvedType) const
{
    if (resolvedType == "boolean")
    {
        file.beginLine().append("unsafe { *(ptr as *const u8) }").endLine();
        return true;
    }
    else if (resolvedType == "byte")
    {
        file.beginLine().append("unsafe { *(ptr as *const i8) }").endLine();
        return true;
    }
    else if (resolvedType == "short")
    {
        file.beginLine().append("unsafe { *(ptr as *const i16) }").endLine();
        return true;
    }
    else if (resolvedType == "unsigned short")
    {
        file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
        return true;
    }
    else if (resolvedType == "long")
    {
        file.beginLine().append("unsafe { *(ptr as *const i32) }").endLine();
        return true;
    }
    else if (resolvedType == "unsigned long")
    {
        file.beginLine().append("unsafe { *(ptr as *const u32) }").endLine();
        return true;
    }
    else if (resolvedType == "hyper")
    {
        file.beginLine().append("unsafe { *(ptr as *const i64) }").endLine();
        return true;
    }
    else if (resolvedType == "unsigned hyper")
    {
        file.beginLine().append("unsafe { *(ptr as *const u64) }").endLine();
        return true;
    }
    else if (resolvedType == "float")
    {
        file.beginLine().append("unsafe { *(ptr as *const f32) }").endLine();
        return true;
    }
    else if (resolvedType == "double")
    {
        file.beginLine().append("unsafe { *(ptr as *const f64) }").endLine();
        return true;
    }
    else if (resolvedType == "char")
    {
        file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
        return true;
    }

    // Return false if no type casting was generated (not a primitive type)
    return false;
}

// Helper functions
std::string_view RustProducer::splitName(std::string_view name)
{
    size_t split = name.find_last_of(".::");
    if (split != std::string_view::npos)
        return name.substr(split + 1);
    else
        return name;
}

OString RustProducer::getRustFunctionName(std::string_view methodName)
{
    // Keep original UNO method name (no snake_case conversion)
    std::string result(methodName);

    // Handle Rust reserved keywords by prefixing with r#
    if (result == "move" || result == "type" || result == "ref" || result == "mut"
        || result == "impl" || result == "fn" || result == "let" || result == "const"
        || result == "static" || result == "match" || result == "if" || result == "else"
        || result == "for" || result == "while" || result == "loop" || result == "break"
        || result == "continue" || result == "return" || result == "self" || result == "Self"
        || result == "super" || result == "crate" || result == "mod" || result == "pub"
        || result == "use" || result == "extern" || result == "struct" || result == "enum"
        || result == "trait" || result == "async" || result == "await" || result == "where"
        || result == "unsafe" || result == "dyn" || result == "true" || result == "false")
    {
        result = "r#" + result;
    }

    return OString(result.c_str());
}

OString RustProducer::getRustTypeName(std::string_view unoName)
{
    // Convert com.sun.star.lang.XMain to com__sun__star__lang__XMain
    OString result(unoName);
    result = result.replaceAll("."_ostr, "__"_ostr);
    return result;
}

OString RustProducer::getRustWrapperTypeName(std::u16string_view unoType) const
{
    OString rustType = u2b(unoType);

    // Handle void returns
    if (rustType == "void")
    {
        return "()"_ostr;
    }

    // Handle primitive types
    if (rustType == "boolean")
        return "u8"_ostr;
    if (rustType == "byte")
        return "i8"_ostr;
    if (rustType == "short")
        return "i16"_ostr;
    if (rustType == "unsigned short")
        return "u16"_ostr;
    if (rustType == "long")
        return "i32"_ostr;
    if (rustType == "unsigned long")
        return "u32"_ostr;
    if (rustType == "hyper")
        return "i64"_ostr;
    if (rustType == "unsigned hyper")
        return "u64"_ostr;
    if (rustType == "float")
        return "f32"_ostr;
    if (rustType == "double")
        return "f64"_ostr;
    if (rustType == "char")
        return "u16"_ostr;

    // Handle special UNO types
    if (rustType == "string")
        return "Option<crate::core::OUString>"_ostr;
    if (rustType == "any" || rustType == "com.sun.star.uno.Any")
        return "Option<crate::core::Any>"_ostr;

    // Handle sequence types (arrays) - return raw pointer for now
    if (rustType.startsWith("[]"))
    {
        return "*mut c_void"_ostr;
    }

    // Handle template types like Pair<T,U> - return raw pointer for now to avoid parsing issues
    if (rustType.indexOf('<') != -1 && rustType.indexOf('>') != -1)
    {
        return "*mut c_void"_ostr;
    }

    // Handle generated UNO types (interface, struct, enum)
    if (rustType.indexOf('.') != -1)
    {
        // Skip types that look malformed or have invalid characters
        if (rustType.indexOf('[') != -1 || rustType.indexOf('<') != -1)
        {
            return "*mut c_void"_ostr;
        }

        // First resolve typedefs
        OString resolvedType(resolveTypedef(unoType));
        if (resolvedType != rustType)
        {
            // It's a typedef - recursively get wrapper type for resolved type
            return getRustWrapperTypeName(b2u(resolvedType));
        }

        // Not a typedef - convert dots to :: for module path
        OString modulePath = rustType.replaceAll("."_ostr, "::"_ostr);

        // Extract simple type name
        sal_Int32 lastDot = rustType.lastIndexOf('.');
        if (lastDot != -1)
        {
            OString simpleName = rustType.copy(lastDot + 1);
            return "Option<crate::generated::rustmaker::" + modulePath + "::" + simpleName + ">";
        }
    }

    // Default to raw pointer for unknown types
    return "*mut c_void"_ostr;
}

OString RustProducer::resolveTypedef(std::u16string_view unoType) const
{
    // Recursively resolve typedefs to underlying types
    rtl::Reference<unoidl::Entity> entity;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(unoType), &entity);

    if (sort == codemaker::UnoType::Sort::Typedef)
    {
        rtl::Reference<unoidl::TypedefEntity> typedefEntity(
            static_cast<unoidl::TypedefEntity*>(entity.get()));
        if (typedefEntity.is())
        {
            // Recursively resolve typedefs
            return resolveTypedef(typedefEntity->getType());
        }
    }

    // Return original type if not a typedef
    return u2b(unoType);
}

OString RustProducer::getRustReturnType(std::u16string_view unoType) const
{
    // Handle void returns
    if (unoType == u"void")
    {
        return "()"_ostr;
    }

    // For interfaces, structs, enums - return Option<TypeWrapper> like services
    if (isUnoInterface(unoType) || isUnoStruct(unoType) || isUnoEnum(unoType))
    {
        OString rustType = u2b(unoType);
        if (rustType.indexOf('.') != -1)
        {
            // Convert dots to :: for module path
            OString modulePath = rustType.replaceAll("."_ostr, "::"_ostr);

            // Extract simple type name
            sal_Int32 lastDot = rustType.lastIndexOf('.');
            if (lastDot != -1)
            {
                OString simpleName = rustType.copy(lastDot + 1);
                return "Option<crate::generated::rustmaker::" + modulePath + "::" + simpleName
                       + ">";
            }
        }
    }

    // For primitive and other types, use the existing logic
    return getRustWrapperTypeName(unoType);
}

OString RustProducer::getRustParameterType(
    std::u16string_view unoType,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction) const
{
    // Handle string types with typed parameters (high-level Rust API)
    if (unoType == u"string")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input string: take high-level OUString in public API
            return "crate::core::OUString"_ostr;
        }
        else
        {
            // Input/output string: still use raw pointer for output parameters
            return "*mut *mut crate::ffi::rtl_string::rtl_uString"_ostr;
        }
    }

    // Handle any types with typed parameters (high-level Rust API)
    if (unoType == u"any")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input any: take high-level Any in public API
            return "crate::core::Any"_ostr;
        }
        else
        {
            // Input/output any: still use raw pointer for output parameters
            return "*mut *mut crate::ffi::uno_any::uno_Any"_ostr;
        }
    }

    // Handle struct types with typed parameters (high-level Rust API)
    if (isUnoStruct(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input struct: take high-level generated struct in public API
            OString rustType = u2b(unoType);
            if (rustType.indexOf('.') != -1)
            {
                // Convert dots to :: for module path
                OString modulePath = rustType.replaceAll("."_ostr, "::"_ostr);

                // Extract simple type name
                sal_Int32 lastDot = rustType.lastIndexOf('.');
                if (lastDot != -1)
                {
                    OString simpleName = rustType.copy(lastDot + 1);
                    return "crate::generated::rustmaker::" + modulePath + "::" + simpleName;
                }
            }
            // Fallback for simple struct names without namespace
            return "crate::generated::rustmaker::" + rustType;
        }
        else
        {
            // Input/output struct: still use raw pointer for output parameters
            return "*mut c_void"_ostr;
        }
    }

    // Handle interface types with typed parameters (high-level Rust API)
    if (isUnoInterface(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input interface: take high-level generated interface in public API
            OString rustType = u2b(unoType);
            if (rustType.indexOf('.') != -1)
            {
                // Convert dots to :: for module path
                OString modulePath = rustType.replaceAll("."_ostr, "::"_ostr);

                // Extract simple type name
                sal_Int32 lastDot = rustType.lastIndexOf('.');
                if (lastDot != -1)
                {
                    OString simpleName = rustType.copy(lastDot + 1);
                    return "crate::generated::rustmaker::" + modulePath + "::" + simpleName;
                }
            }
            // Fallback for simple interface names without namespace
            return "crate::generated::rustmaker::" + rustType;
        }
        else
        {
            // Input/output interface: still use raw pointer for output parameters
            return "*mut c_void"_ostr;
        }
    }

    // Only use typed parameters for basic primitive types (matching CppProducer approach)
    // This ensures we don't break existing logic for complex types
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        // Get the Rust primitive type
        OString rustPrimitiveType = getRustWrapperTypeName(unoType);
        if (!rustPrimitiveType.isEmpty())
        {
            bool isInputOnly
                = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
            if (isInputOnly)
            {
                // Input parameters: pass by value (u8 for boolean, i32 for long, etc.)
                return rustPrimitiveType;
            }
            else
            {
                // Input/output parameters: pass by mutable pointer (*mut u8, *mut i32, etc.)
                return "*mut " + rustPrimitiveType;
            }
        }
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    // This maintains compatibility with existing null-checking and casting logic
    return "*mut c_void"_ostr;
}

OString RustProducer::getRustExternParameterType(
    std::u16string_view unoType,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction) const
{
    // Handle string types with C-compatible FFI parameters (different from high-level API)
    if (unoType == u"string")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input string: pass as C-compatible pointer for FFI
            return "*const crate::ffi::rtl_string::rtl_uString"_ostr;
        }
        else
        {
            // Input/output string: pass as mutable pointer for output parameters
            return "*mut *mut crate::ffi::rtl_string::rtl_uString"_ostr;
        }
    }

    // Handle any types with C-compatible FFI parameters (different from high-level API)
    if (unoType == u"any")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input any: pass as C-compatible pointer for FFI
            return "*const crate::ffi::uno_any::uno_Any"_ostr;
        }
        else
        {
            // Input/output any: pass as mutable pointer for output parameters
            return "*mut *mut crate::ffi::uno_any::uno_Any"_ostr;
        }
    }

    // Handle struct types with C-compatible FFI parameters (different from high-level API)
    if (isUnoStruct(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input struct: pass as C-compatible pointer for FFI (void* for now, can be refined later)
            return "*mut c_void"_ostr;
        }
        else
        {
            // Input/output struct: still use raw pointer for output parameters
            return "*mut c_void"_ostr;
        }
    }

    // Handle interface types with C-compatible FFI parameters (different from high-level API)
    if (isUnoInterface(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input interface: pass as C-compatible pointer for FFI (void* for interfaces)
            return "*mut c_void"_ostr;
        }
        else
        {
            // Input/output interface: still use raw pointer for output parameters
            return "*mut c_void"_ostr;
        }
    }

    // Handle enums - revert to void* to avoid nullptr comparison issues
    if (isUnoEnum(unoType))
    {
        return "*mut c_void"_ostr;
    }

    // For primitive types, use the same logic as high-level API (primitives are the same)
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        // Get the Rust primitive type
        OString rustPrimitiveType = getRustWrapperTypeName(unoType);
        if (!rustPrimitiveType.isEmpty())
        {
            bool isInputOnly
                = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
            if (isInputOnly)
            {
                // Input parameters: pass by value (u8 for boolean, i32 for long, etc.)
                return rustPrimitiveType;
            }
            else
            {
                // Input/output parameters: pass by mutable pointer (*mut u8, *mut i32, etc.)
                return "*mut " + rustPrimitiveType;
            }
        }
    }

    // For all other types (typedefs, structs, interfaces, sequences), use void*
    return "*mut c_void"_ostr;
}

bool RustProducer::isUnoInterface(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::Interface;
}

bool RustProducer::isUnoStruct(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::PlainStruct;
}

bool RustProducer::isUnoEnum(std::u16string_view typeName) const
{
    rtl::Reference<unoidl::Entity> entity;
    rtl::Reference<unoidl::MapCursor> cursor;
    codemaker::UnoType::Sort sort = m_typeManager->getSort(OUString(typeName), &entity, &cursor);
    return sort == codemaker::UnoType::Sort::Enum;
}

OString RustProducer::convertRustParameterForFFICall(
    std::u16string_view unoType, std::string_view paramName,
    unoidl::InterfaceTypeEntity::Method::Parameter::Direction direction) const
{
    // Handle string types with automatic conversion
    if (unoType == u"string")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input string: convert OUString to rtl_uString* using .as_ptr()
            return OString(paramName) + ".as_ptr()";
        }
        else
        {
            // Input/output string: pass through as pointer (already proper type)
            return OString(paramName);
        }
    }

    // Handle any types with automatic conversion
    if (unoType == u"any")
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input any: convert Any to uno_Any* using .as_ptr()
            return OString(paramName) + ".as_ptr()";
        }
        else
        {
            // Input/output any: pass through as pointer (already proper type)
            return OString(paramName);
        }
    }

    // Handle struct types with automatic conversion
    if (isUnoStruct(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input struct: convert generated struct to void* using .as_ptr()
            return OString(paramName) + ".as_ptr()";
        }
        else
        {
            // Input/output struct: pass through as pointer (already proper type)
            return OString(paramName);
        }
    }

    // Handle interface types with automatic conversion
    if (isUnoInterface(unoType))
    {
        bool isInputOnly
            = (direction == unoidl::InterfaceTypeEntity::Method::Parameter::DIRECTION_IN);
        if (isInputOnly)
        {
            // Input interface: convert generated interface to void* using .as_ptr()
            return OString(paramName) + ".as_ptr()";
        }
        else
        {
            // Input/output interface: pass through as pointer (already proper type)
            return OString(paramName);
        }
    }

    // Handle primitive types - pass through directly (no conversion needed)
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        // Primitive types already match FFI expectations - pass through directly
        return OString(paramName);
    }

    // For all other types, pass through without conversion (fallback)
    return OString(paramName);
}

OString RustProducer::getRustStructGetterReturnType(std::u16string_view unoType) const
{
    // Struct getters return the actual type (for direct access, not modification)

    // Handle string types
    if (unoType == u"string")
    {
        return "crate::core::OUString"_ostr; // Return high-level string type
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "crate::core::Any"_ostr; // Return high-level Any type
    }

    // Handle primitive types - return the Rust primitive directly
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        return getRustWrapperTypeName(unoType); // Return native Rust type (u8, i32, f64, etc.)
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    return "*mut c_void"_ostr;
}

OString RustProducer::getRustStructSetterParameterType(std::u16string_view unoType) const
{
    // Struct setters take the value to set

    // Handle string types
    if (unoType == u"string")
    {
        return "crate::core::OUString"_ostr; // Take high-level string type
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "crate::core::Any"_ostr; // Take high-level Any type
    }

    // Handle primitive types - take by value for direct assignment
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        return getRustWrapperTypeName(unoType); // Take native Rust type (u8, i32, f64, etc.)
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    return "*mut c_void"_ostr;
}

OString RustProducer::getRustStructExternGetterReturnType(std::u16string_view unoType) const
{
    // Struct extern getter return types (FFI-compatible)

    // Handle string types
    if (unoType == u"string")
    {
        return "*mut crate::ffi::rtl_string::rtl_uString"_ostr; // Return mutable pointer for from_raw compatibility
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "*const crate::ffi::uno_any::uno_Any"_ostr; // Return C-compatible Any pointer
    }

    // Handle primitive types - return pointer to primitive for FFI
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        return "*const " + getRustWrapperTypeName(unoType); // Return pointer to primitive type
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    return "*mut c_void"_ostr;
}

OString RustProducer::getRustStructExternSetterParameterType(std::u16string_view unoType) const
{
    // Struct extern setter parameter types (FFI-compatible)

    // Handle string types
    if (unoType == u"string")
    {
        return "*const crate::ffi::rtl_string::rtl_uString"_ostr; // Take const pointer for setter input (this is fine)
    }

    // Handle any types
    if (unoType == u"any")
    {
        return "*const crate::ffi::uno_any::uno_Any"_ostr; // Take C-compatible Any pointer
    }

    // Handle primitive types - take by value for FFI (matches C++ side)
    if (unoType == u"boolean" || unoType == u"byte" || unoType == u"short"
        || unoType == u"unsigned short" || unoType == u"long" || unoType == u"unsigned long"
        || unoType == u"hyper" || unoType == u"unsigned hyper" || unoType == u"float"
        || unoType == u"double")
    {
        return getRustWrapperTypeName(
            unoType); // Take primitive type by value (matches C++ approach)
    }

    // For all other types (typedefs, structs, enums, interfaces, sequences), use void*
    return "*mut c_void"_ostr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
