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
//! This module contains direct bindings to LibreOffice's uno_Sequence
//! C functions for creating, manipulating, and destroying Sequence values.
//!
//! These are low-level FFI bindings. For safe Rust wrapper, use
//! the `Sequence` type from the `core` module instead.

// Allow non-standard naming for FFI types that must match C API exactly
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use std::os::raw::c_void;

use crate::ffi::sal_types::sal_Bool;
use crate::ffi::type_ffi::*;
use crate::ffi::uno_bridge::{uno_AcquireFunc, uno_ReleaseFunc};

/// UNO Sequence type - a dynamically sized array type.
/// This matches the C structure _uno_Sequence from include/uno/sequence.h
#[repr(C)]
pub struct uno_Sequence {
    /// Reference count of sequence
    pub nRefCount: i32,
    /// element count
    pub nElements: i32,
    /// elements array
    pub elements: [u8; 0],
}

unsafe extern "C" {
    pub fn uno_type_sequence_construct(
        ppSequence: *mut *mut uno_Sequence,
        pType: *const typelib_TypeDescription,
        pElements: *const c_void,
        nElements: i32,
        acquire: Option<uno_AcquireFunc>,
    ) -> sal_Bool;

    pub fn uno_type_sequence_destroy(
        pSequence: *mut uno_Sequence,
        pType: *const typelib_TypeDescription,
        release: Option<uno_ReleaseFunc>,
    );
}
