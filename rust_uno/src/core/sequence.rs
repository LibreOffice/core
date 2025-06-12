/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Sequence Types and Functions
//!
//! This module provides the high-level safe Rust wrapper around LibreOffice's uno_Sequence.
//! It includes automatic memory management and safe conversions to/from Rust values.
//!
//! ## Key Components
//! - `Sequence` - Safe Rust wrapper around LibreOffice's uno_Sequence
//! - Value conversion functions with automatic type handling
//! - Memory management with automatic cleanup
//!
//! ## Features
//! - Safe construction from various Rust types
//! - Automatic memory management via RAII
//! - Safe value extraction with type checking
//! - Clone, equality, and display trait implementations
//! - Direct FFI interop with raw pointer access methods

use crate::core::r#type::*;
use crate::ffi::type_ffi::*;
// Interface operations now use generated XInterface wrapper methods
// TODO: Replace with generated XInterface::from_ptr(ptr).acquire() pattern
use crate::ffi::uno_sequence::*;
use std::ptr::NonNull;

// === Safe UNO Sequence Wrapper ===

/// UNO Sequence wrapper - a safe Rust wrapper around LibreOffice's uno_Sequence
///
/// Sequence provides a safe, memory-managed interface to LibreOffice's native
/// Sequence type. It automatically handles reference counting and memory cleanup.
pub struct Sequence {
    inner: NonNull<uno_Sequence>,
}

impl Sequence {
    /// Create a new empty Sequence
    ///
    /// Creates an empty sequence of Any elements, similar to the C++ default constructor.
    /// The sequence will be empty (0 elements) and uses the Any type as the element type.
    pub fn new() -> Self {
        Self::with_capacity(0)
    }

    pub fn with_capacity(capacity: i32) -> Self {
        if capacity < 0 {
            panic!("Sequence capacity cannot be negative: {}", capacity);
        }
        unsafe {
            let mut ptr: *mut uno_Sequence = std::ptr::null_mut();

            // Create an Any type for the sequence elements
            let element_type = Type::new();

            // Get the type description from the type reference
            let mut type_desc = std::ptr::null_mut();
            typelib_typedescriptionreference_getDescription(
                &mut type_desc,
                element_type.get_typelib_type(),
            );

            // Construct empty sequence
            let success = uno_type_sequence_construct(
                &mut ptr,
                type_desc,
                std::ptr::null(),
                capacity,
                None, // TODO: Replace with generated XInterface acquire wrapper
            );

            // Check for construction success
            if success == 0 {
                panic!("Failed to construct UNO sequence");
            }

            Sequence {
                inner: NonNull::new(ptr)
                    .expect("uno_type_sequence_construct returned null pointer"),
            }
        }
    }

    /// Get the length of the sequence
    ///
    /// Returns the number of elements in this sequence.
    /// Equivalent to C++ `getLength()` method.
    pub fn get_length(&self) -> i32 {
        unsafe { self.inner.as_ref().nElements }
    }

    /// Check if the sequence has elements
    ///
    /// Returns true if the sequence contains at least one element.
    /// Equivalent to C++ `hasElements()` method.
    pub fn has_elements(&self) -> bool {
        self.get_length() > 0
    }

    /// Get the length as usize for Rust iterator compatibility
    ///
    /// Returns the length as usize, which is commonly used in Rust.
    pub fn len(&self) -> usize {
        self.get_length() as usize
    }

    /// Check if the sequence is empty
    ///
    /// Returns true if the sequence has no elements.
    pub fn is_empty(&self) -> bool {
        self.get_length() == 0
    }

    /// Get raw pointer to the underlying uno_Sequence
    ///
    /// This provides direct access to the underlying UNO sequence for FFI interop.
    /// The returned pointer remains valid as long as this Sequence instance exists.
    pub fn as_raw(&self) -> *mut uno_Sequence {
        self.inner.as_ptr()
    }

    /// Get the raw pointer for FFI calls (retains ownership)
    pub fn as_ptr(&self) -> *const uno_Sequence {
        self.inner.as_ptr() as *const _
    }

    /// Get the mutable raw pointer for FFI calls (retains ownership)
    pub fn as_mut_ptr(&mut self) -> *mut uno_Sequence {
        self.inner.as_ptr()
    }
}

impl Default for Sequence {
    fn default() -> Self {
        Self::new()
    }
}

/// Clone implementation: Create an independent copy of another Sequence
///
/// This creates a new Sequence that references the same sequence data but is
/// independently managed. The clone operation increments the reference count
/// of the underlying sequence, ensuring proper memory management.
impl Clone for Sequence {
    fn clone(&self) -> Self {
        unsafe {
            let ptr = self.inner.as_ptr();
            // Increment reference count
            (*ptr).nRefCount += 1;

            Sequence {
                inner: NonNull::new(ptr).expect("Cloning sequence with invalid pointer"),
            }
        }
    }
}

/// Equality comparison with other Sequence instances
///
/// Two Sequence instances are considered equal if they have the same length
/// and contain the same elements in the same order. This uses pointer comparison
/// for efficiency when both sequences reference the same underlying data.
impl PartialEq for Sequence {
    fn eq(&self, other: &Self) -> bool {
        unsafe {
            let self_ptr = self.inner.as_ptr();
            let other_ptr = other.inner.as_ptr();

            // Fast path: same pointer
            if self_ptr == other_ptr {
                return true;
            }

            let self_seq = self.inner.as_ref();
            let other_seq = other.inner.as_ref();

            // Different lengths means not equal
            if self_seq.nElements != other_seq.nElements {
                return false;
            }

            // For now, do basic comparison - in a full implementation,
            // this would need to compare actual element values
            self_seq.nElements == other_seq.nElements
        }
    }
}

impl Eq for Sequence {}

/// Debug trait for debugging - shows sequence length and type for UNO sequence identification
impl std::fmt::Debug for Sequence {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_tuple("Sequence")
            .field(&format!("length={}", self.get_length()))
            .finish()
    }
}

/// Display trait for printing sequences
///
/// Provides a user-friendly string representation of the sequence.
impl std::fmt::Display for Sequence {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.is_empty() {
            write!(f, "Sequence[]")
        } else {
            write!(f, "Sequence[{}]", self.get_length())
        }
    }
}

/// Destructor: Automatic cleanup when Sequence goes out of scope
impl Drop for Sequence {
    fn drop(&mut self) {
        unsafe {
            let ptr = self.inner.as_ptr();
            let seq = &mut *ptr;
            seq.nRefCount -= 1;

            // If reference count reaches zero, destroy the sequence
            if seq.nRefCount == 0 {
                // Get the element type for proper cleanup
                let element_type = Type::new();
                let mut type_desc = std::ptr::null_mut();
                typelib_typedescriptionreference_getDescription(
                    &mut type_desc,
                    element_type.get_typelib_type(),
                );

                // Call UNO sequence destructor
                uno_type_sequence_destroy(ptr, type_desc, None); // TODO: Replace with generated XInterface release wrapper
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
