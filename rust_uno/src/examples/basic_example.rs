/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Basic usage example of the Rust UNO Language Binding
//!
//! This example demonstrates how to use the Rust UNO bindings to interact
//! with LibreOffice UNO components, including the OUString wrapper for safe
//! string handling and cross-platform SAL types.

// Import from the current crate since examples are now part of the library
use crate::core::OUString;

/// Example function demonstrating OUString creation and operations
pub fn oustring_basic_example() {
    println!("=== LibreOffice UNO Rust Binding - OUString Demonstration ===");

    // Basic string creation
    let empty = OUString::new();
    let hello = OUString::from("Hello UNO!");
    let unicode = OUString::from_utf8("Unicode: café 中文 🚀");

    println!("Empty string: '{}' (len: {})", empty, empty.len());
    println!("Hello string: '{}' (len: {})", hello, hello.len());
    println!("Unicode string: '{}' (len: {})", unicode, unicode.len());

    // String operations
    let cloned = hello.clone();
    println!("Cloned == Original: {}", cloned == hello);

    // Display string conversion
    println!("Display conversion: '{unicode}'");

    println!("✓ OUString operations completed successfully!");
    println!("✓ Successfully demonstrated Rust UNO String Handling");
}

/// Example function demonstrating advanced OUString operations
///
/// Shows various ways to create and manipulate OUString instances,
/// including UTF-8, ASCII, and Unicode handling.
pub fn advanced_string_example() {
    println!("=== Advanced UNO String Types (OUString) Examples ===");

    // Basic string creation using different methods
    let empty_string = OUString::new();
    let hello_string = OUString::from("Hello, LibreOffice!");
    let unicode_string = OUString::from_utf8("Unicode: αβγ, 中文, 🚀");

    println!("Empty: '{empty_string}' (length: {})", empty_string.len());
    println!("Hello: '{hello_string}' (length: {})", hello_string.len());
    println!(
        "Unicode: '{unicode_string}' (length: {})",
        unicode_string.len()
    );

    // String operations
    let cloned = hello_string.clone();
    println!("Cloned == Original: {}", cloned == hello_string);
    println!("Convert to String: '{unicode_string}'");

    // More string examples
    let ascii_only = OUString::from_ascii("ASCII Only!");
    println!(
        "ASCII string: '{ascii_only}' (length: {})",
        ascii_only.len()
    );

    // Comparison operations
    let same_hello = OUString::from("Hello, LibreOffice!");
    println!("String equality: {}", hello_string == same_hello);

    println!("✓ Advanced string operations completed successfully!");
}

/// Run the basic UNO example
///
/// This function demonstrates the complete basic usage of the Rust UNO bindings,
/// focusing on OUString operations and core library functionality.
pub fn run_example() {
    println!("=== Rust UNO Language Binding - Basic Usage Example ===\n");

    // Run the basic OUString demonstration
    oustring_basic_example();

    println!("\n=== Advanced String Examples ===");

    // Run advanced string examples
    advanced_string_example();

    println!("\n=== Data Type Examples ===");

    // Demonstrate various data type operations
    println!("UNO Type System Examples:");

    // Basic type demonstrations (without requiring UNO runtime)
    println!("  Rust types compatible with UNO:");
    println!("    i8 (sal_Int8/byte): {}", i8::MIN);
    println!("    i16 (sal_Int16/short): {}", i16::MIN);
    println!("    i32 (sal_Int32/long): {}", i32::MIN);
    println!("    i64 (sal_Int64/hyper): {}", i64::MIN);
    println!("    u16 (sal_uInt16/unsigned short): {}", u16::MAX);
    println!("    u32 (sal_uInt32/unsigned long): {}", u32::MAX);
    println!("    u64 (sal_uInt64/unsigned hyper): {}", u64::MAX);
    println!("    f32 (float): {}", std::f32::consts::PI);
    println!("    f64 (double): {}", std::f64::consts::E);
    println!("    bool (sal_Bool): true");
    println!("    u16 (sal_Unicode/char): {}", 'Ö' as u16);

    println!("\n✓ All UNO basic operations completed successfully!");
    println!("✓ Rust UNO Language Binding core functionality demonstrated!");
    println!("✓ OUString operations are fully functional!");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
