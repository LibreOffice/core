/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Simple UNO Any Examples
//!
//! This example shows the basic usage of Any for UNO operations.
//!
//! ## Any Creation Methods:
//! - `Any::new()` - Creates an empty Any (void type)
//! - `Any::from_bool(value)` - Creates an Any containing a boolean
//! - `Any::from_i32(value)` - Creates an Any containing a 32-bit integer
//! - `Any::from(value)` - Uses From trait for supported types

use crate::core::Any;

/// Run the UNO Any example
///
/// This function demonstrates the usage of Any for UNO operations,
/// including various creation methods, operations, and traits.
pub fn run_example() {
    println!("=== UNO Any Examples ===\n");

    // Create Any values using different methods
    let empty = Any::new();
    let bool_any = Any::from_bool(true);
    let int_any = Any::from_i32(42);
    let false_any = Any::from_bool(false);
    let zero_any = Any::from_i32(0);

    println!("Empty Any: {} (has value: {})", empty, empty.has_value());
    println!(
        "Boolean Any (true): {} (has value: {})",
        bool_any,
        bool_any.has_value()
    );
    println!(
        "Integer Any (42): {} (has value: {})",
        int_any,
        int_any.has_value()
    );
    println!(
        "Boolean Any (false): {} (has value: {})",
        false_any,
        false_any.has_value()
    );
    println!(
        "Integer Any (0): {} (has value: {})",
        zero_any,
        zero_any.has_value()
    );

    // Test From trait implementations
    println!("\n--- From Trait Examples ---");
    let from_bool: Any = true.into();
    let from_int: Any = 123i32.into();

    println!(
        "From bool (true): {} (has value: {})",
        from_bool,
        from_bool.has_value()
    );
    println!(
        "From i32 (123): {} (has value: {})",
        from_int,
        from_int.has_value()
    );

    // Any operations and traits
    println!("\n--- Any Operations ---");
    let cloned = bool_any.clone();
    println!("Cloned == Original: {}", cloned == bool_any);

    // Trait examples
    println!("\n--- Trait Examples ---");

    // Default trait
    let default_any = Any::default();
    println!(
        "Default Any: '{}' (has value: {})",
        default_any,
        default_any.has_value()
    );

    // Debug trait
    println!("Debug output: {bool_any:?}");

    // PartialEq trait
    let test1 = Any::from_bool(true);
    let test2 = Any::from_bool(true);
    let test3 = Any::from_bool(false);
    let test4 = Any::new(); // void
    let test5 = Any::new(); // void

    println!("true == true: {}", test1 == test2);
    println!("true != false: {}", test1 != test3);
    println!("void == void: {}", test4 == test5);
    println!("true != void: {}", test1 != test4);

    // Test clear operation
    println!("\n--- Clear Operation ---");
    let mut mutable_any = Any::from_i32(999);
    println!(
        "Before clear: {} (has value: {})",
        mutable_any,
        mutable_any.has_value()
    );
    mutable_any.clear();
    println!(
        "After clear: {} (has value: {})",
        mutable_any,
        mutable_any.has_value()
    );

    // Raw conversion examples
    println!("\n--- Raw Conversion Examples ---");
    let original = Any::from_bool(true);
    println!(
        "Original: {} (has value: {})",
        original,
        original.has_value()
    );

    // Convert to raw and back (unsafe operations)
    let raw_any = original.into_raw();
    let restored = unsafe { Any::from_raw(raw_any) };
    println!(
        "Restored: {} (has value: {})",
        restored,
        restored.has_value()
    );

    // Test various integer values
    println!("\n--- Integer Value Tests ---");
    let numbers = vec![-1, 0, 1, 42, 2024, i32::MAX, i32::MIN];
    for num in numbers {
        let any = Any::from_i32(num);
        println!("Any({num}): {} (has value: {})", any, any.has_value());
    }

    // Test memory safety with multiple clones
    println!("\n--- Memory Safety Test ---");
    let original = Any::from_i32(12345);
    let clones: Vec<Any> = (0..5).map(|_| original.clone()).collect();
    println!("Created {} clones of Any(12345)", clones.len());
    for (i, clone) in clones.iter().enumerate() {
        println!("Clone {}: {} (has value: {})", i, clone, clone.has_value());
    }

    println!("\n✓ Any examples completed!");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
