/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Simple UNO String Examples
//!
//! This example shows the basic usage of OUString for UNO operations.
//!
//! ## String Creation Methods:
//! - `OUString::from(str)` - Uses UTF-8 via From trait (most common)
//! - `OUString::from_utf8(str)` - Explicit UTF-8 conversion (recommended)
//! - `OUString::from_ascii(str)` - For pure ASCII text only
//! - `OUString::from_utf16(data)` - When you already have UTF-16 data

use crate::core::oustring::OUString;
use crate::ffi::rtl_string::rtl_uString;
use crate::ffi::sal_types::sal_Unicode;
use std::ptr::NonNull;

/// Test what happens with allocation failure
/// NOTE: This is just for demonstration - real failure is hard to trigger
pub fn test_allocation_failure_behavior() {
    println!("=== Testing Allocation Failure Behavior ===");

    // Simulate what happens when NonNull::new gets a null pointer
    let null_ptr: *mut rtl_uString = std::ptr::null_mut();

    println!("Simulating allocation failure...");

    // This will demonstrate the behavior:
    match NonNull::new(null_ptr) {
        Some(_) => println!("Got valid pointer"),
        None => {
            println!("Got null pointer - this would cause panic in OUString::new()");
            println!("In real code: .expect(\"RTL string allocation failed\") would panic here");
            println!(
                "Program would terminate with: 'RTL string allocation failed - system out of memory'"
            );
            return;
        }
    }

    println!("If you see this message, allocation succeeded");
}

/// Run the UNO string example
///
/// This function demonstrates the usage of OUString for UNO operations,
/// including various creation methods, operations, and traits.
pub fn run_example() {
    println!("=== UNO String Examples ===");

    // Create strings using different methods
    let empty = OUString::new();
    let hello = OUString::from("Hello UNO!"); // Uses from_utf8 via From trait (most common)
    let unicode = OUString::from_utf8("Unicode: café 中文 🚀"); // Explicit UTF-8 (recommended)
    let ascii_only = OUString::from_ascii("ASCII Only!"); // For pure ASCII text only

    // UTF-16 example (for when you already have UTF-16 data)
    let utf16_data: Vec<sal_Unicode> = "UTF-16 example"
        .encode_utf16()
        .map(|c| c as sal_Unicode)
        .collect();
    let utf16_string = OUString::from_utf16(&utf16_data);

    println!("Empty string: '{}' (len: {})", empty, empty.len());
    println!("Hello string: '{}' (len: {})", hello, hello.len());
    println!("UTF-8 string: '{}' (len: {})", unicode, unicode.len());
    println!("ASCII string: '{}' (len: {})", ascii_only, ascii_only.len());
    println!(
        "UTF-16 string: '{}' (len: {})",
        utf16_string,
        utf16_string.len()
    );

    // String operations and traits
    let cloned = hello.clone();
    println!("Cloned == Original: {}", cloned == hello);

    // Convert to Rust String (inherited from Display trait)
    let rust_str = unicode.to_string();
    println!("As Rust String: '{rust_str}'");

    // Trait examples
    println!("\n--- Trait Examples ---");

    // Default trait
    let default_str = OUString::default();
    println!(
        "Default string: '{}' (empty: {})",
        default_str,
        default_str.is_empty()
    );

    // Debug trait
    println!("Debug output: {hello:?}");

    // PartialEq trait
    let test1 = OUString::from("test");
    let test2 = OUString::from("test");
    let test3 = OUString::from("different");
    println!("'{}' == '{}': {}", test1, test2, test1 == test2);
    println!("'{}' != '{}': {}", test1, test2, test1 != test2);
    println!("'{}' == '{}': {}", test1, test3, test1 == test3);
    println!("'{}' != '{}': {}", test1, test3, test1 != test3);

    // Additional string examples
    println!("\n--- Additional String Operations ---");

    let special_chars = OUString::from_utf8("Special: αβγ, ñañá, العربية");
    println!("Special characters: '{special_chars}'");

    let mixed = OUString::from("Mixed content: 123 + αβγ = 🎉");
    println!("Mixed content: '{mixed}' (len: {})", mixed.len());

    // Test empty string operations
    println!("Empty string is empty: {}", empty.is_empty());
    println!("Hello string is empty: {}", hello.is_empty());

    println!("\n--- Testing Allocation Failure Behavior ---");
    test_allocation_failure_behavior();

    println!("\n✓ OUString examples completed!");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
