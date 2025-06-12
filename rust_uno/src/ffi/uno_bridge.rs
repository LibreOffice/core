/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! FFI bindings for UNO bridge functionality.
//!
//! This module contains direct bindings to C++ functions
//! in the UNO bridge for core functionality like initialization
//! and interface management.
//!
//! For specific UNO interface method calls, use the auto-generated
//! FFI functions in the `generated` module instead.

// c_char import removed - no longer needed

// Basic UNO types
pub use crate::ffi::sal_types::*;

/// Opaque pointer to a UNO XInterface
pub type XInterface = std::ffi::c_void;

/// Function types for acquire/release callbacks
#[allow(non_camel_case_types)]
pub type uno_AcquireFunc = unsafe extern "C" fn(*mut std::ffi::c_void);
#[allow(non_camel_case_types)]
pub type uno_ReleaseFunc = unsafe extern "C" fn(*mut std::ffi::c_void);

unsafe extern "C" {
    /// Direct LibreOffice bootstrap function from cppuhelper
    /// Returns Reference<XComponentContext> as opaque pointer
    pub fn defaultBootstrap_InitialComponentContext() -> *mut XInterface;
}
