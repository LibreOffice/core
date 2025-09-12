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

RustProducer::RustProducer(const OString& outputDir, bool verbose, bool dryRun,
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

    OString typeName(splitName(name)); // Simple name for Rust enum
    OString externFunctionPrefix = getRustTypeName(name); // Full name for extern "C" functions

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

    file.closeFile();
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

    OString typeName(splitName(name)); // Simple name for Rust struct
    OString externFunctionPrefix = getRustTypeName(name); // Full name for extern "C" functions

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

    // Implementation with opaque field accessors
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(typeName)
        .endLine()
        .beginBlock();

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

    // Opaque field accessors
    for (const auto& member : entity->getDirectMembers())
    {
        OString memberName = u2b(member.name); // Use original case, not snake_case

        // Getter
        file.beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("pub fn get_")
            .append(memberName)
            .append("(&self) -> *mut c_void {")
            .endLine()
            .extraIndent()
            .beginLine()
            .append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(self.ptr) }")
            .endLine()
            .beginLine()
            .append("}")
            .endLine();

        // Setter
        file.beginLine()
            .append("")
            .endLine()
            .beginLine()
            .append("pub fn set_")
            .append(memberName)
            .append("(&mut self, value: *mut c_void) {")
            .endLine()
            .extraIndent()
            .beginLine()
            .append("unsafe { ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(self.ptr, value) }")
            .endLine()
            .beginLine()
            .append("}")
            .endLine();
    }

    file.endBlock();

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

    // Extern "C" declarations for opaque operations
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
        .append("_destroy(ptr: *mut c_void);")
        .endLine()
        .beginLine()
        .append("fn ")
        .append(externFunctionPrefix)
        .append("_from_ptr(ptr: *mut c_void) -> *mut c_void;")
        .endLine();

    for (const auto& member : entity->getDirectMembers())
    {
        OString memberName = u2b(member.name); // Use original case, not snake_case
        file.beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_get_")
            .append(memberName)
            .append("(ptr: *mut c_void) -> *mut c_void;")
            .endLine()
            .beginLine()
            .append("fn ")
            .append(externFunctionPrefix)
            .append("_set_")
            .append(memberName)
            .append("(ptr: *mut c_void, value: *mut c_void);")
            .endLine();
    }

    file.beginLine().append("}").endLine();

    file.closeFile();
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

    OString typeName(splitName(name)); // Simple name for Rust interface
    OString externFunctionPrefix = getRustTypeName(name); // Full name for extern "C" functions

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

    // Implementation block
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(typeName)
        .endLine()
        .beginBlock();

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

    // as_ptr method for getting the raw pointer
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

    // Validity check - only add if there's no conflicting isValid method
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

    // Method wrappers - all methods return opaque pointers
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

        // All parameters are opaque void pointers
        for (const auto& param : method.parameters)
        {
            file.append(", ").append(param.name).append(": *mut c_void");
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
                file.append(", ").append(param.name);
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
                file.append(", ").append(param.name);
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
                    OString resolvedType = resolveTypedef(method.returnType);
                    if (resolvedType != rustType)
                    {
                        // It's a typedef - handle based on resolved type
                        if (resolvedType == "boolean")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u8) }").endLine();
                        }
                        else if (resolvedType == "byte")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i8) }").endLine();
                        }
                        else if (resolvedType == "short")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i16) }").endLine();
                        }
                        else if (resolvedType == "unsigned short")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
                        }
                        else if (resolvedType == "long")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i32) }").endLine();
                        }
                        else if (resolvedType == "unsigned long")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u32) }").endLine();
                        }
                        else if (resolvedType == "hyper")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i64) }").endLine();
                        }
                        else if (resolvedType == "unsigned hyper")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u64) }").endLine();
                        }
                        else if (resolvedType == "float")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const f32) }").endLine();
                        }
                        else if (resolvedType == "double")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const f64) }").endLine();
                        }
                        else if (resolvedType == "char")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
                        }
                        else
                        {
                            // Typedef resolves to other types - return raw pointer
                            file.beginLine().append("ptr").endLine();
                        }
                    }
                    else
                    {
                        // Not a typedef - check for primitive types that need dereferencing
                        if (rustType == "boolean")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u8) }").endLine();
                        }
                        else if (rustType == "byte")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i8) }").endLine();
                        }
                        else if (rustType == "short")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i16) }").endLine();
                        }
                        else if (rustType == "unsigned short")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
                        }
                        else if (rustType == "long")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i32) }").endLine();
                        }
                        else if (rustType == "unsigned long")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u32) }").endLine();
                        }
                        else if (rustType == "hyper")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const i64) }").endLine();
                        }
                        else if (rustType == "unsigned hyper")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u64) }").endLine();
                        }
                        else if (rustType == "float")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const f32) }").endLine();
                        }
                        else if (rustType == "double")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const f64) }").endLine();
                        }
                        else if (rustType == "char")
                        {
                            file.beginLine().append("unsafe { *(ptr as *const u16) }").endLine();
                        }
                        else
                        {
                            // For other types (sequences, templates, etc), return the raw pointer
                            file.beginLine().append("ptr").endLine();
                        }
                    }
                }
            }
        }

        file.beginLine().append("}").endLine();
    }

    file.endBlock();

    // Drop implementation for automatic cleanup
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
            file.append(", ").append(param.name).append(": *mut c_void");
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

    file.closeFile();
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

    OString serviceName(splitName(name)); // Simple name for Rust service
    OString externFunctionPrefix = getRustTypeName(name); // Full name for extern "C" functions
    OUString interfaceType = entity->getBase();

    // Generate proper module path for the interface
    OString interfaceTypeStr = u2b(interfaceType);
    std::string_view interfaceTypeName = splitName(interfaceTypeStr);

    // Convert interface type to full module path
    OString path = interfaceTypeStr;
    path = path.replaceAll("."_ostr, "::"_ostr);
    OString interfaceModulePath
        = "crate::generated::rustmaker::"_ostr + path + "::" + interfaceTypeName;

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

    // Implementation block
    file.beginLine()
        .append("")
        .endLine()
        .beginLine()
        .append("impl ")
        .append(serviceName)
        .endLine()
        .beginBlock();

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

    file.endBlock();

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

    file.closeFile();
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
        OString resolvedType = resolveTypedef(unoType);
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

OString RustProducer::getRustFFIReturnType(std::u16string_view unoType) const
{
    // Handle void returns
    if (unoType == u"void")
    {
        return "()"_ostr;
    }

    // For interfaces, structs, enums - return their opaque handle typedef
    if (isUnoInterface(unoType) || isUnoStruct(unoType) || isUnoEnum(unoType))
    {
        // Convert UNO type name to typedef handle name
        // com.sun.star.lang.XMain -> com__sun__star__lang__XMainHandle
        OString functionPrefix = getRustTypeName(std::string(unoType.begin(), unoType.end()));
        return functionPrefix + "Handle";
    }

    // For other types (primitives, sequences, etc) - use void*
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
