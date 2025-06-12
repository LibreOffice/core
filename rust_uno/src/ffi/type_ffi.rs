/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Type System FFI
//!
//! This module contains FFI bindings for the UNO type system, including the
//! typelib_TypeClass enum and related structures for type introspection.

// Allow non-standard naming for FFI types that must match C API exactly
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::ffi::rtl_string::rtl_uString;
use crate::ffi::sal_types::*;

// === UNO Type System FFI Declarations ===

/// UNO type class enumeration - binary compatible with the IDL enum com.sun.star.uno.TypeClass
///
/// This enumeration defines all possible type categories in the UNO type system.
/// Each type has a specific class that determines how it's handled in marshalling,
/// method dispatch, and memory management. The values must match exactly with
/// the corresponding C/C++ enum to ensure binary compatibility.
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum typelib_TypeClass {
    typelib_TypeClass_VOID = 0,
    typelib_TypeClass_CHAR = 1,
    typelib_TypeClass_BOOLEAN = 2,
    typelib_TypeClass_BYTE = 3,
    typelib_TypeClass_SHORT = 4,
    typelib_TypeClass_UNSIGNED_SHORT = 5,
    typelib_TypeClass_LONG = 6,
    typelib_TypeClass_UNSIGNED_LONG = 7,
    typelib_TypeClass_HYPER = 8,
    typelib_TypeClass_UNSIGNED_HYPER = 9,
    typelib_TypeClass_FLOAT = 10,
    typelib_TypeClass_DOUBLE = 11,
    typelib_TypeClass_STRING = 12,
    typelib_TypeClass_TYPE = 13,
    typelib_TypeClass_ANY = 14,
    typelib_TypeClass_ENUM = 15,
    typelib_TypeClass_TYPEDEF = 16,
    typelib_TypeClass_STRUCT = 17,

    #[deprecated = "UNOIDL does not have a union concept"]
    typelib_TypeClass_UNION = 18,
    typelib_TypeClass_EXCEPTION = 19,
    typelib_TypeClass_SEQUENCE = 20,

    #[deprecated = "UNOIDL does not have an array concept"]
    typelib_TypeClass_ARRAY = 21,
    typelib_TypeClass_INTERFACE = 22,
    typelib_TypeClass_SERVICE = 23,
    typelib_TypeClass_MODULE = 24,
    typelib_TypeClass_INTERFACE_METHOD = 25,
    typelib_TypeClass_INTERFACE_ATTRIBUTE = 26,
    typelib_TypeClass_UNKNOWN = 27,
    typelib_TypeClass_PROPERTY = 28,
    typelib_TypeClass_CONSTANT = 29,
    typelib_TypeClass_CONSTANTS = 30,
    typelib_TypeClass_SINGLETON = 31,
    typelib_TypeClass_MAKE_FIXED_SIZE = 0x7fffffff,
}

/// Weak reference to a type description - the core UNO type reference structure
///
/// This structure holds a weak reference to a type description in the UNO type system.
/// It provides efficient type identification without requiring the full type description
/// to be loaded. Memory layout is designed for binary compatibility with LibreOffice's
/// C implementation.
#[repr(C)]
pub struct typelib_TypeDescriptionReference {
    /// Reference count of type - managed automatically by acquire/release functions
    pub nRefCount: sal_Int32,
    /// Static reference count for types that persist until program termination
    pub nStaticRefCount: sal_Int32,
    /// Type class category (VOID, STRING, INTERFACE, etc.)
    pub eTypeClass: typelib_TypeClass,
    /// Fully qualified name of the type (e.g., "com.sun.star.lang.XComponent")
    pub pTypeName: *mut rtl_uString,
    /// Pointer to full type description (may be null if not loaded)
    pub pType: *mut typelib_TypeDescription,
    /// Internal unique identifier for runtime optimization
    pub pUniqueIdentifier: *mut std::os::raw::c_void,
    /// Reserved for future use
    pub pReserved: *mut std::os::raw::c_void,
}

/// Full type description with complete metadata
///
/// Contains comprehensive information about a UNO type including size, alignment,
/// and member details. This is loaded on demand when detailed type information
/// is needed for marshalling, method dispatch, or introspection.
#[repr(C)]
pub struct typelib_TypeDescription {
    /// Reference count - managed automatically by acquire/release functions
    pub nRefCount: sal_Int32,
    /// Static reference count for types that persist until program termination
    pub nStaticRefCount: sal_Int32,
    /// Type class category (VOID, STRING, INTERFACE, etc.)
    pub eTypeClass: typelib_TypeClass,
    /// Fully qualified name of the type
    pub pTypeName: *mut rtl_uString,
    /// Self-reference pointer for type identification
    pub pSelf: *mut typelib_TypeDescription,
    /// Internal unique identifier for runtime optimization
    pub pUniqueIdentifier: *mut std::os::raw::c_void,
    /// Reserved for future use
    pub pReserved: *mut std::os::raw::c_void,
    /// Flag indicating whether the description is complete and ready to use
    pub bComplete: sal_Bool,
    /// Size of the type in bytes
    pub nSize: sal_Int32,
    /// Memory alignment requirements for the type
    pub nAlignment: sal_Int32,
    /// Back-reference to the weak type reference
    pub pWeakRef: *mut typelib_TypeDescriptionReference,
    /// Flag indicating if type can be unloaded and reloaded on demand
    pub bOnDemand: sal_Bool,
}

unsafe extern "C" {
    /// Increment reference count of type description reference
    pub fn typelib_typedescriptionreference_acquire(pRef: *mut typelib_TypeDescriptionReference);

    /// Decrement reference count of type description reference
    pub fn typelib_typedescriptionreference_release(pRef: *mut typelib_TypeDescriptionReference);

    /// Create new type description reference from type class and name
    pub fn typelib_typedescriptionreference_new(
        ppTDR: *mut *mut typelib_TypeDescriptionReference,
        eTypeClass: typelib_TypeClass,
        pTypeName: *mut rtl_uString,
    );

    /// Test equality of two type description references
    pub fn typelib_typedescriptionreference_equals(
        p1: *const typelib_TypeDescriptionReference,
        p2: *const typelib_TypeDescriptionReference,
    ) -> sal_Bool;

    /// Test type assignment compatibility
    pub fn typelib_typedescriptionreference_isAssignableFrom(
        pAssignable: *mut typelib_TypeDescriptionReference,
        pFrom: *mut typelib_TypeDescriptionReference,
    ) -> sal_Bool;

    /// Get full type description from type reference
    pub fn typelib_typedescriptionreference_getDescription(
        ppRet: *mut *mut typelib_TypeDescription,
        pRef: *mut typelib_TypeDescriptionReference,
    );

    /// Get static type reference for primitive type classes
    pub fn typelib_static_type_getByTypeClass(
        eTypeClass: typelib_TypeClass,
    ) -> *mut *mut typelib_TypeDescriptionReference;

    /// Assign type reference (with proper reference counting)
    pub fn typelib_typedescriptionreference_assign(
        ppDest: *mut *mut typelib_TypeDescriptionReference,
        pSource: *mut typelib_TypeDescriptionReference,
    );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
