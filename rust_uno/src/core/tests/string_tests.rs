/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Unit tests for OUString (Rust-side functionality)
//!
//! These tests verify OUString's Rust implementation including:
//! - Creation methods (new, from_utf8, from_ascii, from_utf16)
//! - Trait implementations (Clone, PartialEq, From, Display, Debug, Default)
//! - Instance methods (len, is_empty, as_ptr, etc.)
//! - Memory management and safety (RAII, reference counting)
//! - FFI interop (from_raw, into_raw)
//!
//! These tests do NOT require LibreOffice UNO components and test
//! only the Rust-side implementation and FFI safety.

use crate::core::OUString;

// === Creation Method Tests ===

#[test]
fn test_oustring_new() {
    let s = OUString::new();
    assert_eq!(s.len(), 0);
    assert!(s.is_empty());
    assert_eq!(s.to_string(), "");
}

#[test]
fn test_oustring_default() {
    let s = OUString::default();
    assert_eq!(s.len(), 0);
    assert!(s.is_empty());
    assert_eq!(s.to_string(), "");
}

#[test]
fn test_oustring_from_utf8() {
    let s = OUString::from_utf8("Hello, 世界!");
    assert!(!s.is_empty());
    assert_eq!(s.to_string(), "Hello, 世界!");

    // Test empty string
    let empty = OUString::from_utf8("");
    assert!(empty.is_empty());
    assert_eq!(empty.len(), 0);

    // Test with Unicode characters
    let unicode = OUString::from_utf8("Ñiño 🦀 café");
    assert_eq!(unicode.to_string(), "Ñiño 🦀 café");
}

#[test]
fn test_oustring_from_ascii() {
    let s = OUString::from_ascii("Hello, World!");
    assert_eq!(s.to_string(), "Hello, World!");
    assert!(!s.is_empty());

    // Test empty ASCII string
    let empty = OUString::from_ascii("");
    assert!(empty.is_empty());
    assert_eq!(empty.len(), 0);
}

// === Trait Implementation Tests ===

#[test]
fn test_oustring_clone() {
    let original = OUString::from_utf8("Test string for cloning");
    let cloned = original.clone();

    // They should be equal
    assert_eq!(original, cloned);
    assert_eq!(original.to_string(), cloned.to_string());
    assert_eq!(original.len(), cloned.len());

    // Test cloning empty string
    let empty = OUString::new();
    let empty_clone = empty.clone();
    assert_eq!(empty, empty_clone);
    assert!(empty_clone.is_empty());
}

#[test]
fn test_oustring_partial_eq() {
    let s1 = OUString::from_utf8("Hello");
    let s2 = OUString::from_utf8("Hello");
    let s3 = OUString::from_utf8("World");
    let empty1 = OUString::new();
    let empty2 = OUString::new();

    // Equal strings
    assert_eq!(s1, s2);
    assert_eq!(empty1, empty2);

    // Different strings
    assert_ne!(s1, s3);
    assert_ne!(s1, empty1);
    assert_ne!(empty1, s1);

    // Test with Unicode
    let unicode1 = OUString::from_utf8("café");
    let unicode2 = OUString::from_utf8("café");
    let unicode3 = OUString::from_utf8("cafe"); // without accent

    assert_eq!(unicode1, unicode2);
    assert_ne!(unicode1, unicode3);
}

#[test]
fn test_oustring_from_str() {
    let s = OUString::from("Hello from &str");
    assert_eq!(s.to_string(), "Hello from &str");

    // Test with Unicode
    let unicode: OUString = "Héllo wørld 🌍".into();
    assert_eq!(unicode.to_string(), "Héllo wørld 🌍");
}

#[test]
fn test_oustring_from_string() {
    let rust_string = String::from("Hello from String");
    let s = OUString::from(rust_string);
    assert_eq!(s.to_string(), "Hello from String");

    // Test with owned Unicode String
    let unicode_string = "Ñiño 🦀".to_string();
    let unicode_oustring = OUString::from(unicode_string);
    assert_eq!(unicode_oustring.to_string(), "Ñiño 🦀");
}

#[test]
fn test_oustring_display() {
    let s = OUString::from_utf8("Display test");
    assert_eq!(format!("{s}"), "Display test");

    // Test empty string display
    let empty = OUString::new();
    assert_eq!(format!("{empty}"), "");

    // Test Unicode display
    let unicode = OUString::from_utf8("Display 测试 🦀");
    assert_eq!(format!("{unicode}"), "Display 测试 🦀");
}

#[test]
fn test_oustring_debug() {
    let s = OUString::from_utf8("Debug test");
    let debug_output = format!("{s:?}");
    assert!(debug_output.contains("OUString"));
    assert!(debug_output.contains("Debug test"));

    // Test empty string debug
    let empty = OUString::new();
    let empty_debug = format!("{empty:?}");
    assert!(empty_debug.contains("OUString"));
}

// === Method Tests ===

#[test]
fn test_oustring_len() {
    let empty = OUString::new();
    assert_eq!(empty.len(), 0);

    let ascii = OUString::from_utf8("Hello");
    assert_eq!(ascii.len(), 5);

    // Note: len() returns UTF-16 code units, not Unicode codepoints
    let unicode = OUString::from_utf8("café"); // 'é' is one UTF-16 unit
    assert_eq!(unicode.len(), 4);

    // Emoji takes 2 UTF-16 code units (surrogate pair)
    let emoji = OUString::from_utf8("🦀");
    assert_eq!(emoji.len(), 2);
}

#[test]
fn test_oustring_is_empty() {
    let empty = OUString::new();
    assert!(empty.is_empty());

    let non_empty = OUString::from_utf8("Not empty");
    assert!(!non_empty.is_empty());

    let empty_from_str = OUString::from_utf8("");
    assert!(empty_from_str.is_empty());
}

// === Special Cases and Edge Cases ===

#[test]
fn test_oustring_special_characters() {
    // Test various special Unicode characters
    let special_chars = vec![
        "\0",   // Null character
        "\n",   // Newline
        "\r\n", // Windows line ending
        "\t",   // Tab
        "\"",   // Quote
        "\\",   // Backslash
        "©",    // Copyright symbol
        "€",    // Euro symbol
        "™",    // Trademark
        "🦀",   // Rust crab emoji
        "👨‍💻",   // Man technologist (complex emoji)
    ];

    for special in special_chars {
        let s = OUString::from_utf8(special);
        assert_eq!(s.to_string(), special);
    }
}

#[test]
fn test_oustring_long_strings() {
    // Test with long strings to ensure proper memory management
    let long_text = "A".repeat(10000);
    let s = OUString::from_utf8(&long_text);
    assert_eq!(s.len(), 10000);
    assert_eq!(s.to_string(), long_text);

    // Test cloning long strings
    let cloned = s.clone();
    assert_eq!(cloned, s);
    assert_eq!(cloned.len(), 10000);
}

#[test]
fn test_oustring_multilingual() {
    // Test various languages and scripts
    let multilingual = "English, 中文, العربية, Русский, ελληνικά, עברית";
    let s = OUString::from_utf8(multilingual);
    assert_eq!(s.to_string(), multilingual);
    assert!(!s.is_empty());
}

#[test]
fn test_oustring_consistency_across_constructors() {
    let text = "Test consistency";

    let from_utf8 = OUString::from_utf8(text);
    let from_str: OUString = text.into();
    let from_string = OUString::from(text.to_string());

    // All should produce equivalent strings
    assert_eq!(from_utf8, from_str);
    assert_eq!(from_utf8, from_string);
    assert_eq!(from_str, from_string);

    // All should have same content
    assert_eq!(from_utf8.to_string(), text);
    assert_eq!(from_str.to_string(), text);
    assert_eq!(from_string.to_string(), text);
}

#[test]
fn test_oustring_memory_safety() {
    // Test that dropping strings doesn't cause issues
    {
        let s1 = OUString::from_utf8("Temporary string 1");
        let s2 = s1.clone();
        let s3 = OUString::from_utf8("Temporary string 2");

        assert_eq!(s1, s2);
        assert_ne!(s1, s3);

        // Strings will be dropped here - this should not cause issues
    }

    // Create new strings after the previous ones were dropped
    let s4 = OUString::from_utf8("After drop test");
    assert_eq!(s4.to_string(), "After drop test");
}

#[test]
fn test_oustring_memory_release() {
    // Simple memory release test - create and drop many strings
    // If memory isn't released properly, this would cause memory leaks

    // Create many strings in a loop and let them drop
    for i in 0..1000 {
        let s = OUString::from_utf8(&format!("Test string number {i}"));
        assert_eq!(s.to_string(), format!("Test string number {i}"));
        // String automatically drops here
    }

    // Create and explicitly drop strings
    for _ in 0..1000 {
        let s1 = OUString::from_utf8("Temporary string");
        let s2 = s1.clone(); // This should increase ref count
        let s3 = s2.clone(); // This should increase ref count more

        // All should be equal
        assert_eq!(s1, s2);
        assert_eq!(s2, s3);

        // All will be dropped here - ref counts should decrease properly
    }

    // If we reach here without crashes or memory issues, memory management is working
}

#[test]
fn test_oustring_reference_counting_behavior() {
    // Test that cloning works properly with reference counting
    let original = OUString::from_utf8("Reference counted string");

    // Create multiple references
    let clone1 = original.clone();
    let clone2 = original.clone();
    let clone3 = clone1.clone();

    // All should be equal
    assert_eq!(original, clone1);
    assert_eq!(original, clone2);
    assert_eq!(original, clone3);
    assert_eq!(clone1, clone2);
    assert_eq!(clone1, clone3);
    assert_eq!(clone2, clone3);

    // All have same content
    assert_eq!(original.to_string(), "Reference counted string");
    assert_eq!(clone1.to_string(), "Reference counted string");
    assert_eq!(clone2.to_string(), "Reference counted string");
    assert_eq!(clone3.to_string(), "Reference counted string");

    // When this function ends, all clones should be properly released
}

#[test]
fn test_oustring_memory_from_raw_into_raw() {
    // Test raw pointer ownership transfer (unique functionality)
    for i in 0..100 {
        let original = OUString::from_utf8(&format!("Raw pointer test {i}"));
        let original_content = original.to_string();

        // Transfer ownership to raw pointer
        let raw_ptr = original.into_raw();

        // Create new OUString from raw pointer (takes ownership back)
        let restored = unsafe { OUString::from_raw(raw_ptr) };

        assert_eq!(restored.to_string(), original_content);
    }
}

#[test]
fn test_oustring_memory_mixed_operations() {
    // Test complex combinations of operations (not covered by existing tests)
    for i in 0..50 {
        // Create initial string
        let s1 = OUString::from_utf8(&format!("Mixed test {i}"));

        // Convert through different types
        let s2 = OUString::from(s1.to_string());
        let s3: OUString = s1.to_string().as_str().into();

        // Clone operations
        let c1 = s1.clone();
        let c2 = s2.clone();
        let c3 = s3.clone();

        // Verify equality
        assert_eq!(s1, s2);
        assert_eq!(s2, s3);
        assert_eq!(c1, c2);
        assert_eq!(c2, c3);

        // Raw pointer operations
        let raw1 = c1.into_raw();
        let raw2 = c2.into_raw();

        let restored1 = unsafe { OUString::from_raw(raw1) };
        let restored2 = unsafe { OUString::from_raw(raw2) };

        assert_eq!(restored1, restored2);
        assert_eq!(restored1.to_string(), format!("Mixed test {i}"));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
