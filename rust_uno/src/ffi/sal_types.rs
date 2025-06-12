/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Basic Types - Cross-Platform Implementation
//!
//! This module contains the fundamental SAL (System Abstraction Layer) types
//! that form the foundation of UNO's type system. These types use Rust native
//! types to ensure consistent behavior across all platforms.

// Allow non-standard naming for FFI types that must match C API exactly
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

// === Fixed-size integer types ===
// These types have guaranteed sizes across all platforms

pub type sal_Bool = u8; // Always 8-bit boolean
pub type sal_Int8 = i8; // Always 8-bit signed
pub type sal_uInt8 = u8; // Always 8-bit unsigned
pub type sal_Int16 = i16; // Always 16-bit signed
pub type sal_uInt16 = u16; // Always 16-bit unsigned
pub type sal_Int32 = i32; // Always 32-bit signed
pub type sal_uInt32 = u32; // Always 32-bit unsigned
pub type sal_Int64 = i64; // Always 64-bit signed
pub type sal_uInt64 = u64; // Always 64-bit unsigned

// === Platform-dependent types ===
// These types adapt to the target platform's pointer width

pub type sal_Size = usize; // Platform pointer width (32/64-bit)
pub type sal_sSize = isize; // Signed platform pointer width
pub type sal_PtrDiff = isize; // Pointer difference type
pub type sal_IntPtr = isize; // Integer type that can hold a pointer
pub type sal_uIntPtr = usize; // Unsigned integer type that can hold a pointer

// === Special types ===

pub type sal_Unicode = u16; // UTF-16 code unit (always 16-bit)
pub type sal_Handle = *mut std::ffi::c_void; // Generic opaque handle

pub const SAL_FALSE: sal_Bool = 0;
pub const SAL_TRUE: sal_Bool = 1;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
