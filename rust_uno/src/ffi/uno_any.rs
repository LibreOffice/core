/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! FFI bindings for UNO Any type operations.
//!
//! This module contains direct bindings to LibreOffice's uno_Any
//! C functions for creating, manipulating, and destroying Any values.
//!
//! These are low-level FFI bindings. For safe Rust wrapper, use
//! the `Any` type from the `core` module instead.

// Allow non-standard naming for FFI types that must match C API exactly
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

// Note: sal_types are included via typelib types
use crate::ffi::type_ffi::{
    typelib_TypeClass, typelib_TypeDescription, typelib_TypeDescriptionReference,
};

/// UNO Any type - represents a value of any UNO type
/// This matches the C structure _uno_Any from include/uno/any2.h
#[repr(C)]
pub struct uno_Any {
    /// type of value
    pub pType: *mut typelib_TypeDescriptionReference,
    /// pointer to value; this may point to pReserved
    pub pData: *mut std::ffi::c_void,
    /// reserved space for storing value
    pub pReserved: *mut std::ffi::c_void,
}

/// Function types for acquire/release callbacks
pub type uno_AcquireFunc = unsafe extern "C" fn(*mut std::ffi::c_void);
pub type uno_ReleaseFunc = unsafe extern "C" fn(*mut std::ffi::c_void);

unsafe extern "C" {
    /// Assign an any with a given value
    pub fn uno_any_assign(
        pDest: *mut uno_Any,
        pSource: *mut std::ffi::c_void,
        pTypeDescr: *mut typelib_TypeDescription,
        acquire: Option<uno_AcquireFunc>,
        release: Option<uno_ReleaseFunc>,
    );

    /// Assign an any with a given value (using type reference)
    pub fn uno_type_any_assign(
        pDest: *mut uno_Any,
        pSource: *mut std::ffi::c_void,
        pType: *mut typelib_TypeDescriptionReference,
        acquire: Option<uno_AcquireFunc>,
        release: Option<uno_ReleaseFunc>,
    );

    /// Constructs an any with a given value
    pub fn uno_any_construct(
        pDest: *mut uno_Any,
        pSource: *mut std::ffi::c_void,
        pTypeDescr: *mut typelib_TypeDescription,
        acquire: Option<uno_AcquireFunc>,
    );

    /// Constructs an any with a given value (using type reference)
    pub fn uno_type_any_construct(
        pDest: *mut uno_Any,
        pSource: *mut std::ffi::c_void,
        pType: *mut typelib_TypeDescriptionReference,
        acquire: Option<uno_AcquireFunc>,
    );

    /// Destructs an any
    pub fn uno_any_destruct(pValue: *mut uno_Any, release: Option<uno_ReleaseFunc>);

    /// Sets value to void (clears the any)
    pub fn uno_any_clear(pValue: *mut uno_Any, release: Option<uno_ReleaseFunc>);

    // Get static type descriptors by type class (returns pointer to pointer)
    pub fn typelib_static_type_getByTypeClass(
        eTypeClass: typelib_TypeClass,
    ) -> *mut *mut typelib_TypeDescriptionReference;

}

/// Get type descriptor for boolean values
pub fn get_boolean_type() -> *mut typelib_TypeDescriptionReference {
    unsafe {
        let ptr_ptr =
            typelib_static_type_getByTypeClass(typelib_TypeClass::typelib_TypeClass_BOOLEAN);
        *ptr_ptr
    }
}

/// Get type descriptor for 32-bit integer values
pub fn get_long_type() -> *mut typelib_TypeDescriptionReference {
    unsafe {
        let ptr_ptr = typelib_static_type_getByTypeClass(typelib_TypeClass::typelib_TypeClass_LONG);
        *ptr_ptr
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
