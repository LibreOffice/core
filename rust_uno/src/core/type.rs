/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Type System and Type References
//!
//! This module provides the high-level safe Rust wrapper around LibreOffice's type system.
//! It includes safe type creation, automatic memory management, and type compatibility testing.
//!
//! ## Key Components
//! - `Type` - Safe Rust wrapper around LibreOffice's typelib_TypeDescriptionReference
//!
//! ## Features
//! - Safe type creation with automatic memory management via RAII
//! - Type equality and assignment compatibility testing
//! - Type class and name retrieval
//! - Clone, equality, and default trait implementations
//! - Direct FFI interop with raw pointer access methods
//! - Support for all UNO type classes (void, primitive types, interfaces, etc.)
//! - Safe From trait implementations for creating types from type classes and explicit type/name tuples

use crate::core::oustring::*;
use crate::ffi::type_ffi::*;
use std::ptr;

// === Safe Type Wrapper ===

/// Safe Rust wrapper for UNO Type
///
/// This struct provides a safe, ergonomic interface to UNO's type system.
/// It handles memory management automatically and provides idiomatic Rust
/// methods for type operations and comparisons.
///
/// # Examples
///
/// ```rust
/// use rust_uno::{Type, typelib_TypeClass};
///
/// // Create a void type
/// let void_type = Type::new();
///
/// // Create a primitive type
/// let int_type = Type::from(typelib_TypeClass::typelib_TypeClass_LONG);
///
/// // Create an interface type
/// let interface_type = Type::from((
///     typelib_TypeClass::typelib_TypeClass_INTERFACE,
///     "com.sun.star.lang.XComponent"
/// ));
/// ```
#[allow(non_snake_case)] // _pType follows FFI naming conventions
pub struct Type {
    /// C typelib reference pointer
    _pType: *mut typelib_TypeDescriptionReference,
}

impl Type {
    /// Create a new Type set to void
    ///
    /// Creates the default UNO type (void). This is equivalent to the default
    /// constructor in C++ com::sun::star::uno::Type.
    pub fn new() -> Self {
        unsafe {
            let void_type_ref =
                typelib_static_type_getByTypeClass(typelib_TypeClass::typelib_TypeClass_VOID);
            typelib_typedescriptionreference_acquire(*void_type_ref);
            Type {
                _pType: *void_type_ref,
            }
        }
    }

    /// Create a Type from type class and name
    ///
    /// Creates a new Type from the specified type class and a string that will be
    /// converted to OUString. Accepts any type that can be converted to OUString
    /// (such as &str, String, etc.) through the Into trait. This is the most common
    /// way to create custom types with specific names.
    ///
    /// # Arguments
    /// * `type_class` - The UNO type class (VOID, STRING, INTERFACE, etc.)
    /// * `type_name` - The fully qualified type name (convertible to OUString)
    pub fn new_with_name<T: Into<OUString>>(type_class: typelib_TypeClass, type_name: T) -> Self {
        let mut oustring_name = type_name.into();
        let mut type_ref: *mut typelib_TypeDescriptionReference = ptr::null_mut();

        unsafe {
            typelib_typedescriptionreference_new(
                &mut type_ref,
                type_class,
                oustring_name.as_mut_ptr(),
            );
        }

        Type { _pType: type_ref }
    }

    /// Create Type from existing typelib reference (acquires reference)
    ///
    /// Takes ownership of an existing type reference by incrementing its reference count.
    /// This is useful when interfacing with C code that provides type references.
    ///
    /// # Safety
    /// The caller must ensure that `type_ref` points to a valid typelib_TypeDescriptionReference.
    pub unsafe fn from_typelib_ref(type_ref: *mut typelib_TypeDescriptionReference) -> Self {
        unsafe {
            typelib_typedescriptionreference_acquire(type_ref);
        }
        Type { _pType: type_ref }
    }

    /// Create Type from existing typelib reference (no acquire)
    ///
    /// Takes ownership of an existing type reference without incrementing reference count.
    /// Use this when transferring ownership from C code that already has a reference.
    /// This avoids unnecessary reference count manipulation when you know the ownership
    /// is being transferred.
    ///
    /// # Safety
    /// The caller must ensure that:
    /// - `type_ref` points to a valid typelib_TypeDescriptionReference
    /// - The reference count is properly managed (caller transfers ownership)
    pub unsafe fn from_typelib_ref_no_acquire(
        type_ref: *mut typelib_TypeDescriptionReference,
    ) -> Self {
        Type { _pType: type_ref }
    }

    /// Get the type class of this type
    ///
    /// Returns the UNO type class (VOID, STRING, INTERFACE, etc.) that categorizes
    /// this type within the UNO type system. This is used for type checking and
    /// dispatch in UNO method calls.
    pub fn get_type_class(&self) -> typelib_TypeClass {
        unsafe { (*self._pType).eTypeClass }
    }

    /// Get the fully qualified name of this type
    ///
    /// Returns an OUString containing the type name (e.g., "com.sun.star.lang.XComponent").
    /// For primitive types, returns the type name (e.g., "long", "string"). The returned
    /// string is a copy, so it can be safely used without worrying about the lifetime
    /// of the original Type instance.
    pub fn get_type_name(&self) -> OUString {
        unsafe {
            let name_ptr = (*self._pType).pTypeName;
            if name_ptr.is_null() {
                return OUString::new();
            }
            // Use OUString::from_raw which handles the acquisition internally
            OUString::from_raw(name_ptr)
        }
    }

    /// Get the raw typelib reference pointer for FFI calls
    ///
    /// Returns the underlying pointer for FFI interoperability. The returned
    /// pointer remains owned by this Type instance and should not be freed
    /// by the caller. Useful when calling LibreOffice C APIs directly.
    pub fn get_typelib_type(&self) -> *mut typelib_TypeDescriptionReference {
        self._pType
    }

    /// Convert Type into raw pointer (releases Rust ownership)
    ///
    /// Transfers ownership of the underlying type reference to the caller.
    /// The caller becomes responsible for calling typelib_typedescriptionreference_release
    /// when done with the pointer. The Type instance is consumed and cannot be used
    /// after this call.
    pub fn into_raw(self) -> *mut typelib_TypeDescriptionReference {
        let ptr = self._pType;
        std::mem::forget(self); // Don't run destructor since we're transferring ownership
        ptr
    }

    /// Test if values of this type can be assigned from values of the given type
    ///
    /// This includes widening conversions (e.g., long assignable from short)
    /// as long as there is no data loss. The test follows UNO assignment rules
    /// and is used in method dispatch to determine if argument types are compatible.
    ///
    /// # Arguments
    /// * `other` - The source type to test assignment compatibility from
    ///
    /// # Returns
    /// `true` if values of `other` type can be assigned to this type, `false` otherwise
    pub fn is_assignable_from(&self, other: &Type) -> bool {
        unsafe { typelib_typedescriptionreference_isAssignableFrom(self._pType, other._pType) != 0 }
    }

    /// Test if this type is equal to another type
    ///
    /// Two types are equal if they have the same type class and fully qualified name.
    /// This is used for type matching in UNO method dispatch and interface queries.
    /// The comparison is efficient and uses LibreOffice's native type comparison.
    ///
    /// # Arguments
    /// * `other` - The type to compare with
    ///
    /// # Returns
    /// `true` if the types are equal, `false` otherwise
    pub fn equals(&self, other: &Type) -> bool {
        unsafe { typelib_typedescriptionreference_equals(self._pType, other._pType) != 0 }
    }
}

/// Default implementation: Creates a void type
///
/// When a Type is created without explicit initialization, it defaults to
/// the void type, which is the UNO equivalent of "no type" or "empty type".
impl Default for Type {
    fn default() -> Self {
        Self::new()
    }
}

/// Clone implementation: Create an independent copy of another Type
///
/// This creates a new Type that references the same type description but is
/// independently managed. The clone operation increments the reference count
/// of the underlying type description, ensuring proper memory management.
impl Clone for Type {
    fn clone(&self) -> Self {
        unsafe {
            typelib_typedescriptionreference_acquire(self._pType);
        }
        Type {
            _pType: self._pType,
        }
    }
}

/// Equality comparison with other Type instances
///
/// Two Type instances are considered equal if they refer to the same type
/// (same type class and name). This uses LibreOffice's native type comparison
/// which is efficient and handles all type categories correctly.
impl PartialEq for Type {
    fn eq(&self, other: &Self) -> bool {
        self.equals(other)
    }
}

impl Eq for Type {}

/// Debug trait for debugging - shows type class and name for UNO type identification
///
/// Displays the Type in a debug-friendly format showing both the type class
/// and the type name, which is useful for debugging UNO type-related issues.
impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let type_class = self.get_type_class();
        let type_name = self.get_type_name().to_string();
        f.debug_struct("Type")
            .field("type_class", &type_class)
            .field("type_name", &type_name)
            .finish()
    }
}

/// Destructor: Automatic cleanup when Type goes out of scope
///
/// Automatically releases the reference to the underlying type description
/// when the Type instance is dropped. This ensures proper memory management
/// without manual intervention.
impl Drop for Type {
    fn drop(&mut self) {
        unsafe {
            typelib_typedescriptionreference_release(self._pType);
        }
    }
}

/// From trait implementation for typelib_TypeClass (primitive types)
///
/// Creates a Type from a primitive type class. This uses LibreOffice's static
/// type system to get pre-defined type references for standard UNO types like
/// void, boolean, string, etc. This is the most efficient way to create
/// primitive types since they're statically allocated.
impl From<typelib_TypeClass> for Type {
    fn from(type_class: typelib_TypeClass) -> Self {
        unsafe {
            let type_ref = typelib_static_type_getByTypeClass(type_class);
            typelib_typedescriptionreference_acquire(*type_ref);
            Type { _pType: *type_ref }
        }
    }
}

/// From trait implementation for (typelib_TypeClass, &str) tuples
///
/// Convenient conversion from type class and string slice to Type. The string
/// is automatically converted to OUString using UTF-8 encoding. This is useful
/// for creating complex types with explicit type names.
impl From<(typelib_TypeClass, &str)> for Type {
    fn from((type_class, type_name): (typelib_TypeClass, &str)) -> Self {
        Self::new_with_name(type_class, type_name)
    }
}

/// From trait implementation for (typelib_TypeClass, String) tuples
///
/// Convenient conversion from type class and owned String to Type. The string
/// is automatically converted to OUString using UTF-8 encoding. This allows
/// using dynamically generated type names.
impl From<(typelib_TypeClass, String)> for Type {
    fn from((type_class, type_name): (typelib_TypeClass, String)) -> Self {
        Self::new_with_name(type_class, type_name)
    }
}

/// From trait implementation for (typelib_TypeClass, OUString) tuples
///
/// Direct conversion from type class and OUString to Type. This is the most
/// efficient conversion when you already have an OUString, as it avoids
/// additional string conversions.
impl From<(typelib_TypeClass, OUString)> for Type {
    fn from((type_class, type_name): (typelib_TypeClass, OUString)) -> Self {
        Self::new_with_name(type_class, type_name)
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
