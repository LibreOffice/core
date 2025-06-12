/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Safe Rust wrappers for UNO interfaces
//!
//! This module provides memory-safe, RAII-based wrappers around raw UNO interface pointers.
//! For specific UNO interface method calls, use the auto-generated FFI functions
//! in the `generated` module directly.

use crate::ffi::uno_bridge;
use crate::generated::com::sun::star::uno::XComponentContext::XComponentContext;
use std::ffi::NulError;

/// Error types for UNO operations
#[derive(Debug)]
pub enum UnoError {
    /// Failed to create C string (contains null byte)
    InvalidString(NulError),
    /// UNO operation failed
    OperationFailed,
    /// Interface not supported
    InterfaceNotSupported,
    /// Null pointer encountered
    NullPointer,
    /// Bridge initialization failed
    BridgeInitializationFailed,
}

impl From<NulError> for UnoError {
    fn from(err: NulError) -> Self {
        UnoError::InvalidString(err)
    }
}

pub type UnoResult<T> = Result<T, UnoError>;

/// Initialize the UNO bridge by directly calling LibreOffice bootstrap
/// Returns the initial component context
#[allow(non_snake_case)]
pub fn defaultBootstrap_InitialComponentContext() -> UnoResult<XComponentContext> {
    unsafe {
        let context_ptr = uno_bridge::defaultBootstrap_InitialComponentContext();
        if context_ptr.is_null() {
            Err(UnoError::BridgeInitializationFailed)
        } else {
            // The bootstrap function returns Reference<XComponentContext>* directly
            // Use from_ptr which now handles Reference<T>* correctly
            XComponentContext::from_ptr(context_ptr).ok_or(UnoError::BridgeInitializationFailed)
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
