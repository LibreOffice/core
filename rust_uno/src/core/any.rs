/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Any Types and Functions
//!
//! This module provides the high-level safe Rust wrapper around LibreOffice's uno_Any.
//! It includes automatic memory management and safe conversions to/from Rust values.
//!
//! ## Key Components
//! - `Any` - Safe Rust wrapper around LibreOffice's uno_Any
//! - Value conversion functions with automatic type handling
//! - Memory management with automatic cleanup
//!
//! ## Features
//! - Safe construction from various Rust types
//! - Automatic memory management via RAII
//! - Safe value extraction with type checking
//! - Clone, equality, and display trait implementations
//! - Direct FFI interop with raw pointer access methods

use crate::ffi::uno_any::*;
// Interface operations now use generated XInterface wrapper methods
// TODO: Replace with generated XInterface::from_ptr(ptr).acquire() pattern

// === Safe UNO Any Wrapper ===

/// UNO Any wrapper - a safe Rust wrapper around LibreOffice's uno_Any
///
/// Any provides a safe, memory-managed interface to LibreOffice's native
/// any type. It automatically handles reference counting and memory cleanup.
pub struct Any {
    inner: uno_Any,
}

impl Any {
    /// Create a new empty UNO Any (void type)
    ///
    /// This creates an empty Any that contains no value (void type).
    pub fn new() -> Self {
        unsafe {
            let mut any = Any {
                inner: uno_Any {
                    pType: std::ptr::null_mut(),
                    pData: std::ptr::null_mut(),
                    pReserved: std::ptr::null_mut(),
                },
            };

            // Initialize as void using uno_any_construct
            uno_any_construct(
                &mut any.inner,
                std::ptr::null_mut(), // pSource (null for void)
                std::ptr::null_mut(), // pTypeDescr (null for void)
                None,
            );
            any
        }
    }

    /// Create Any from a boolean value
    ///
    /// This creates an Any containing a boolean value.
    pub fn from_bool(value: bool) -> Self {
        unsafe {
            let mut any = Any {
                inner: uno_Any {
                    pType: std::ptr::null_mut(),
                    pData: std::ptr::null_mut(),
                    pReserved: std::ptr::null_mut(),
                },
            };

            let bool_val = if value { 1u8 } else { 0u8 };

            // Initialize with boolean data using uno_type_any_construct
            uno_type_any_construct(
                &mut any.inner,
                &bool_val as *const u8 as *mut std::ffi::c_void,
                get_boolean_type(),
                None,
            );
            any
        }
    }

    /// Create Any from a 32-bit integer
    ///
    /// This creates an Any containing a 32-bit signed integer.
    pub fn from_i32(value: i32) -> Self {
        unsafe {
            let mut any = Any {
                inner: uno_Any {
                    pType: std::ptr::null_mut(),
                    pData: std::ptr::null_mut(),
                    pReserved: std::ptr::null_mut(),
                },
            };

            // Initialize with integer data using uno_type_any_construct
            uno_type_any_construct(
                &mut any.inner,
                &value as *const i32 as *mut std::ffi::c_void,
                get_long_type(),
                None,
            );

            any
        }
    }

    /// Create Any from raw uno_Any (takes ownership)
    ///
    /// # Safety
    /// The caller must ensure that:
    /// - The uno_Any is valid and properly initialized
    /// - The uno_Any is not used elsewhere after this call
    /// - Proper reference counting is maintained
    pub unsafe fn from_raw(any: uno_Any) -> Self {
        Any { inner: any }
    }

    /// Convert to raw uno_Any (releases ownership)
    ///
    /// The caller becomes responsible for calling uno_any_destruct
    pub fn into_raw(self) -> uno_Any {
        unsafe {
            let inner = std::ptr::read(&self.inner);
            std::mem::forget(self); // Don't run destructor
            inner
        }
    }

    /// Get the raw pointer for FFI calls (retains ownership)
    pub fn as_ptr(&self) -> *const uno_Any {
        &self.inner
    }

    /// Get the mutable raw pointer for FFI calls (retains ownership)
    pub fn as_mut_ptr(&mut self) -> *mut uno_Any {
        &mut self.inner
    }

    /// Check if the Any contains a value (not void)
    pub fn has_value(&self) -> bool {
        !self.inner.pType.is_null()
    }

    /// Clear the Any (set to void)
    pub fn clear(&mut self) {
        unsafe {
            uno_any_clear(&mut self.inner, None);
        }
    }
}

impl Default for Any {
    fn default() -> Self {
        Self::new()
    }
}

/// Clone implementation: Create a copy of another Any
///
/// This creates a new Any that contains the same value but is independently
/// managed (proper deep copy with reference counting).
impl Clone for Any {
    fn clone(&self) -> Self {
        let mut new_any = Any::new();

        if self.has_value() {
            unsafe {
                // Clone by assigning from the existing any's data using type reference
                uno_type_any_assign(
                    &mut new_any.inner,
                    self.inner.pData,
                    self.inner.pType,
                    None,
                    None,
                );
            }
        }

        new_any
    }
}

/// Comparison with other Any
impl PartialEq for Any {
    fn eq(&self, other: &Self) -> bool {
        // Handle cases where one or both are void
        match (self.has_value(), other.has_value()) {
            (false, false) => true,                 // Both void = equal
            (false, true) | (true, false) => false, // One void, one not = not equal
            (true, true) => {
                // Both have values - basic pointer comparison for now
                self.inner.pType == other.inner.pType && self.inner.pData == other.inner.pData
            }
        }
    }
}

impl Eq for Any {}

/// From trait implementation for bool
impl From<bool> for Any {
    fn from(value: bool) -> Self {
        Self::from_bool(value)
    }
}

/// From trait implementation for i32
impl From<i32> for Any {
    fn from(value: i32) -> Self {
        Self::from_i32(value)
    }
}

/// Display trait for printing
impl std::fmt::Display for Any {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if !self.has_value() {
            write!(f, "Any(void)")
        } else {
            write!(f, "Any(<value>)")
        }
    }
}

/// Debug trait for debugging
impl std::fmt::Debug for Any {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Any")
            .field("has_value", &self.has_value())
            .field("pType", &self.inner.pType)
            .field("pData", &self.inner.pData)
            .finish()
    }
}

/// Destructor: Automatic cleanup when Any goes out of scope
impl Drop for Any {
    fn drop(&mut self) {
        if self.has_value() {
            unsafe {
                uno_any_destruct(&mut self.inner, None);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
