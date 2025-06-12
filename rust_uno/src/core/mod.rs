/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Core Module
//!
//! This module contains the high-level safe Rust wrappers for LibreOffice UNO types.
//! These provide memory-safe, idiomatic Rust interfaces to the underlying C APIs.

pub mod any;
pub mod oustring;
pub mod sequence;
pub mod r#type;
pub mod uno_wrapper;

// Re-export the main types for convenience
pub use crate::ffi::type_ffi::typelib_TypeClass;
pub use any::Any;
pub use oustring::OUString;
pub use sequence::Sequence;
pub use r#type::Type;
// UnoInterface replaced with generated XInterface from rustmaker
pub use uno_wrapper::{UnoError, UnoResult, defaultBootstrap_InitialComponentContext};

// Include unit tests
#[cfg(test)]
mod tests {
    mod string_tests;
    mod type_tests;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
