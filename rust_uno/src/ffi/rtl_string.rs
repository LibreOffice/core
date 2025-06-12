/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! RTL String FFI Bindings
//!
//! This module contains the raw FFI bindings for LibreOffice's rtl_uString
//! functions and types. These are low-level C API bindings that should
//! typically be used through the high-level OUString wrapper.

// Allow non-standard naming for FFI types that must match C API exactly
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use crate::ffi::sal_types::*;

// === RTL String Types and FFI Declarations ===

/// Interlocked count type for reference counting and atomic operations
pub type oslInterlockedCount = sal_Int32;

/// Text encoding constants
pub type rtl_TextEncoding = sal_Int32;
pub const RTL_TEXTENCODING_UTF8: rtl_TextEncoding = 76;

/// Conversion flags for string conversion
pub const OSTRING_TO_OUSTRING_CVTFLAGS: sal_uInt32 = 0x00000003;

/// Internal structure for LibreOffice UNO strings
#[repr(C)]
pub struct rtl_uString {
    /// reference count of string
    pub refCount: oslInterlockedCount,
    /// length of the string in UTF-16 code units
    pub length: sal_Int32,
    /// UTF-16 buffer array
    pub buffer: [sal_Unicode; 1usize],
}

unsafe extern "C" {
    /// Create a new empty string
    pub fn rtl_uString_new(newStr: *mut *mut rtl_uString);

    /// Create string from ASCII
    pub fn rtl_uString_newFromAscii(
        newStr: *mut *mut rtl_uString,
        value: *const std::os::raw::c_char,
    );

    /// Create string from existing string
    pub fn rtl_uString_newFromString(newStr: *mut *mut rtl_uString, value: *const rtl_uString);

    /// Create string from UTF-16 data with length
    pub fn rtl_uString_newFromStr_WithLength(
        newStr: *mut *mut rtl_uString,
        value: *const sal_Unicode,
        len: sal_Int32,
    );

    /// Create UNO string from byte string with encoding
    pub fn rtl_string2UString(
        newStr: *mut *mut rtl_uString,
        str: *const std::os::raw::c_char,
        len: sal_Int32,
        encoding: rtl_TextEncoding,
        convertFlags: sal_uInt32,
    );

    /// Release string reference
    pub fn rtl_uString_release(str: *mut rtl_uString);

    /// Acquire string reference
    pub fn rtl_uString_acquire(str: *mut rtl_uString);

    /// Get string length
    pub fn rtl_uString_getLength(str: *const rtl_uString) -> sal_Int32;

    /// Get string data pointer
    pub fn rtl_uString_getStr(str: *const rtl_uString) -> *const sal_Unicode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
