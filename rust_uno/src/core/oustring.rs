/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO String Types and Functions
//!
//! This module provides the high-level safe Rust wrapper around LibreOffice's rtl_uString.
//! It includes automatic memory management and safe conversions to/from Rust strings.
//!
//! ## Key Components
//! - `OUString` - Safe Rust wrapper around LibreOffice's rtl_uString
//! - String conversion functions with automatic encoding handling
//! - Memory management with automatic reference counting
//!
//! ## Features
//! - UTF-8, UTF-16, and ASCII string creation methods
//! - Automatic memory management via RAII
//! - Safe conversion to/from Rust strings
//! - Clone, equality, and display trait implementations
//! - Direct FFI interop with raw pointer access methods

use std::ptr::NonNull;

use crate::ffi::rtl_string::*;
use crate::ffi::sal_types::*;

// === Safe UNO String Wrapper ===

/// UNO String wrapper - a safe Rust wrapper around LibreOffice's rtl_uString
///
/// OUString provides a safe, memory-managed interface to LibreOffice's native
/// string type. It automatically handles reference counting and memory cleanup,
/// and provides conversions to/from standard Rust string types.
pub struct OUString {
    inner: NonNull<rtl_uString>,
}

impl OUString {
    /// Create a new empty UNO string
    ///
    /// This creates an empty OUString with zero length.
    pub fn new() -> Self {
        unsafe {
            let mut ptr: *mut rtl_uString = std::ptr::null_mut();
            rtl_uString_new(&mut ptr);

            // Convert to NonNull, panic if allocation failed
            let inner =
                NonNull::new(ptr).expect("RTL string allocation failed - system out of memory");

            OUString { inner }
        }
    }

    /// Utility function to create a slice for the string.
    ///
    /// Take len as parameter to avoid double call in PartialEq impl.
    fn get_ustring_slice(data: &NonNull<rtl_uString>, len: usize) -> &[u16] {
        unsafe {
            let data_ptr = rtl_uString_getStr(data.as_ptr());
            std::slice::from_raw_parts(data_ptr, len)
        }
    }


    /// Create a UNO string from UTF-8 text
    ///
    /// This uses LibreOffice's direct UTF-8 to UString conversion function,
    /// which is more efficient than converting through UTF-16. Handles all
    /// valid UTF-8 including Unicode characters and emojis.
    pub fn from_utf8(text: &str) -> Self {
        unsafe {
            let mut ptr: *mut rtl_uString = std::ptr::null_mut();
            rtl_string2UString(
                &mut ptr,
                text.as_ptr() as *const std::os::raw::c_char,
                text.len() as sal_Int32,
                RTL_TEXTENCODING_UTF8,
                OSTRING_TO_OUSTRING_CVTFLAGS,
            );

            let inner = NonNull::new(ptr).expect("RTL UTF-8 string creation failed");

            OUString { inner }
        }
    }

    /// Create a UNO string from ASCII text
    ///
    /// This method uses LibreOffice's native ASCII conversion function for
    /// actual ASCII text (7-bit characters only). More efficient than UTF-8
    /// conversion when you know the text is pure ASCII.
    pub fn from_ascii(text: &str) -> Self {
        unsafe {
            let mut ptr: *mut rtl_uString = std::ptr::null_mut();
            // Ensure the string is null-terminated for the C function
            let c_string = std::ffi::CString::new(text).expect("CString creation failed");
            rtl_uString_newFromAscii(&mut ptr, c_string.as_ptr());

            let inner = NonNull::new(ptr).expect("RTL ASCII string creation failed");

            OUString { inner }
        }
    }

    /// Create OUString from UTF-16 data
    ///
    /// Creates a UNO string directly from UTF-16 data. This is the most efficient
    /// method when you already have UTF-16 data, since LibreOffice uses UTF-16
    /// internally (sal_Unicode is UTF-16 code units).
    pub fn from_utf16(data: &[sal_Unicode]) -> Self {
        unsafe {
            let mut ptr: *mut rtl_uString = std::ptr::null_mut();
            rtl_uString_newFromStr_WithLength(&mut ptr, data.as_ptr(), data.len() as sal_Int32);

            let inner = NonNull::new(ptr).expect("RTL UTF-16 string creation failed");

            OUString { inner }
        }
    }

    /// Create OUString from raw rtl_uString pointer (takes ownership)
    ///
    /// # Safety
    /// The caller must ensure that:
    /// - The pointer is valid and points to a properly initialized rtl_uString
    /// - The pointer is not used elsewhere after this call
    /// - The rtl_uString has the correct reference count
    pub unsafe fn from_raw(str: *mut rtl_uString) -> Self {
        let inner = NonNull::new(str).expect("Cannot create OUString from null pointer");

        unsafe {
            rtl_uString_acquire(inner.as_ptr());
        }
        OUString { inner }
    }

    /// Convert to raw rtl_uString pointer (releases ownership)
    ///
    /// The caller becomes responsible for calling rtl_uString_release
    pub fn into_raw(self) -> *mut rtl_uString {
        let ptr = self.inner.as_ptr();
        std::mem::forget(self); // Don't run destructor
        ptr
    }

    /// Get the raw pointer for FFI calls (retains ownership)
    pub fn as_ptr(&self) -> *const rtl_uString {
        self.inner.as_ptr() as *const _
    }

    /// Get the mutable raw pointer for FFI calls (retains ownership)
    pub fn as_mut_ptr(&mut self) -> *mut rtl_uString {
        self.inner.as_ptr()
    }

    /// Get the length of the string
    pub fn len(&self) -> usize {
        unsafe { rtl_uString_getLength(self.inner.as_ptr()) as usize }
    }

    /// Check if the string is empty
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

impl Default for OUString {
    fn default() -> Self {
        Self::new()
    }
}

/// Clone implementation: Create a copy of another OUString
///
/// This creates a new OUString that shares the same content but is independently
/// managed (proper deep copy with reference counting).
impl Clone for OUString {
    fn clone(&self) -> Self {
        unsafe {
            rtl_uString_acquire(self.inner.as_ptr());
            OUString { inner: self.inner }
        }
    }
}

/// From trait implementation for &str (UTF-8)
///
/// Convenient conversion from string slices to OUString using UTF-8 encoding.
impl<T: AsRef<str>> From<T> for OUString {
    fn from(text: T) -> Self {
        Self::from_utf8(text.as_ref())
    }
}

/// Comparison with other OUString
impl PartialEq for OUString {
    fn eq(&self, other: &Self) -> bool {
        // Compare pointers first (fast check)
        if self.inner == other.inner {
            return true;
        }

        // Compare lengths (fast check)
        let self_len = self.len();
        let other_len = other.len();

        if self_len != other_len {
            return false;
        }

        // If lengths are equal, compare UTF-16 data directly
        if self_len == 0 {
            return true; // Both empty strings
        }
        // Compare UTF-16 data byte by byte
        let self_slice = Self::get_ustring_slice(&self.inner, self_len);
        let other_slice = Self::get_ustring_slice(&other.inner, other_len);

        self_slice == other_slice
    }
}

impl Eq for OUString {}

/// Display trait for printing
impl std::fmt::Display for OUString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.is_empty() {
            return Ok(());
        }

        // Convert from UTF-16 to UTF-8
        let utf16_slice = Self::get_ustring_slice(&self.inner, self.len());
        let string = String::from_utf16_lossy(utf16_slice);
        write!(f, "{string}")
    }
}

/// Debug trait for debugging - shows type and content for UNO string identification
impl std::fmt::Debug for OUString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_tuple("OUString").field(&self.to_string()).finish()
    }
}

/// Destructor: Automatic cleanup when OUString goes out of scope
impl Drop for OUString {
    fn drop(&mut self) {
        unsafe {
            rtl_uString_release(self.inner.as_ptr());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
