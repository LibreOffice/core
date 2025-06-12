/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Integration tests for Rust UNO Language Binding
//!
//! These tests verify that the UNO binding works correctly with the
//! actual LibreOffice UNO components through FFI/C interop.
//!
//! **Note**: Rust-side unit tests (traits, constructors, etc.) are located
//! in separate test files like `string_tests.rs`.
//!
//! # Test Coverage
//!
//! ## Core FFI Functionality Tests
//! - `test_xtest_interface_creation()` - Basic interface instantiation
//! - `test_boolean_operations()` - Boolean data type FFI handling (both true/false)
//! - `test_integer_operations()` - Signed integer types FFI with wrong values
//! - `test_unsigned_integer_operations()` - Unsigned integer types FFI with wrong values
//! - `test_64bit_operations()` - 64-bit integer types FFI with wrong values
//! - `test_floating_point_operations()` - Float and double types FFI with wrong values
//! - `test_char_operations()` - Unicode character FFI handling with wrong values
//! - `test_string_operations()` - OUString FFI handling with UNO components
//! - `test_type_operations()` - UNO Type FFI interop with C++ components
//! - `test_rust_type_creation()` - Rust Type wrapper construction and basic operations
//! - `test_type_assignment_compatibility()` - Type assignment compatibility testing
//! - `test_type_cloning_and_memory()` - Type cloning and memory management validation
//!
//! ## Comprehensive UNO Component Validation Tests
//! - `test_wrong_values_comprehensive()` - Extensive wrong value testing for all data types
//! - `test_edge_cases_and_boundary_values()` - Boundary conditions and special values
//! - `test_consistency_across_multiple_calls()` - Stability and consistency verification
//! - `test_all_operations_comprehensive()` - Cross-functional integration testing
//!
//! ## Test Strategy
//! Each FFI function is tested with:
//! - Correct expected values (should return true)
//! - Multiple wrong values (should return false)
//! - Boundary values and edge cases
//! - Consistency across multiple calls
//!
//! This ensures robust validation of the UNO binding's FFI data type handling
//! and error detection capabilities with actual UNO components.

use crate::ffi::rtl_string::rtl_uString;
use crate::ffi::sal_types::*;
use crate::ffi::type_ffi::typelib_TypeDescriptionReference;
use crate::core::{OUString, Type, typelib_TypeClass};

// === FFI Declarations for Test Functions ===
#[allow(unused)]
unsafe extern "C" {
    /// Print a greeting message from the UNO test component
    pub fn xtest_hello();

    /// Get a boolean value from UNO test component (always returns true)
    pub fn xtest_getBoolean() -> sal_Bool;

    /// Test if a boolean value matches the expected UNO boolean
    pub fn xtest_isBoolean(value: sal_Bool) -> sal_Bool;

    /// Get a byte value from UNO test component (always returns -12)
    pub fn xtest_getByte() -> sal_Int8;

    /// Test if a byte value matches the expected UNO byte (-12)
    pub fn xtest_isByte(value: sal_Int8) -> sal_Bool;

    /// Get a short value from UNO test component (always returns -1234)
    pub fn xtest_getShort() -> sal_Int16;

    /// Test if a short value matches the expected UNO short (-1234)
    pub fn xtest_isShort(value: sal_Int16) -> sal_Bool;

    /// Get a long value from UNO test component (always returns -123456)
    pub fn xtest_getLong() -> sal_Int32;

    /// Test if a long value matches the expected UNO long (-123456)
    pub fn xtest_isLong(value: sal_Int32) -> sal_Bool;

    /// Get an unsigned short value from UNO test component (always returns 54321)
    pub fn xtest_getUnsignedShort() -> sal_uInt16;

    /// Test if an unsigned short value matches the expected UNO unsigned short (54321)
    pub fn xtest_isUnsignedShort(value: sal_uInt16) -> sal_Bool;

    /// Get an unsigned long value from UNO test component (always returns 3456789012)
    pub fn xtest_getUnsignedLong() -> sal_uInt32;

    /// Test if an unsigned long value matches the expected UNO unsigned long (3456789012)
    pub fn xtest_isUnsignedLong(value: sal_uInt32) -> sal_Bool;

    /// Get a hyper (64-bit) value from UNO test component (always returns -123456789)
    pub fn xtest_getHyper() -> sal_Int64;

    /// Test if a hyper value matches the expected UNO hyper (-123456789)
    pub fn xtest_isHyper(value: sal_Int64) -> sal_Bool;

    /// Get an unsigned hyper (64-bit) value from UNO test component (always returns 9876543210)
    pub fn xtest_getUnsignedHyper() -> sal_uInt64;

    /// Test if an unsigned hyper value matches the expected UNO unsigned hyper (9876543210)
    pub fn xtest_isUnsignedHyper(value: sal_uInt64) -> sal_Bool;

    /// Get a float value from UNO test component (always returns -10.25)
    pub fn xtest_getFloat() -> f32;

    /// Test if a float value matches the expected UNO float (-10.25)
    pub fn xtest_isFloat(value: f32) -> sal_Bool;

    /// Get a double value from UNO test component (always returns 100.5)
    pub fn xtest_getDouble() -> f64;

    /// Test if a double value matches the expected UNO double (100.5)
    pub fn xtest_isDouble(value: f64) -> sal_Bool;

    /// Test if a character value matches the expected UNO character ('Ö')
    pub fn xtest_isChar(value: sal_Unicode) -> sal_Bool;

    /// Get a character value from UNO test component (always returns 'Ö')
    pub fn xtest_getChar() -> sal_Unicode;

    /// Get a string value from UNO test component - returns pointer to rtl_uString
    pub fn xtest_getString() -> *mut rtl_uString;

    /// Test if a string value matches the expected string ("hä") - takes pointer to rtl_uString
    pub fn xtest_isString(value: *mut rtl_uString) -> sal_Bool;

    /// Get a Type value from UNO test component - returns pointer to typelib_TypeDescriptionReference
    pub fn xtest_getType() -> *mut typelib_TypeDescriptionReference;

    /// Test if a Type value matches the expected Type (cppu::UnoType<sal_Int32>) - takes pointer to typelib_TypeDescriptionReference
    pub fn xtest_isType(value: *mut typelib_TypeDescriptionReference) -> sal_Bool;
}

// === Test Interface Wrapper ===

/// Rust wrapper for the XTest UNO interface (TEST ONLY)
///
/// This interface provides methods for testing basic UNO data types
/// and serves as a demonstration of the Rust UNO binding functionality.
///
/// **Note**: This is a test interface, not part of the core UNO API.
pub struct XTest;

#[allow(unused)]
impl XTest {
    /// Create a new XTest interface instance
    pub fn new() -> Self {
        XTest
    }

    /// Print a greeting message from the UNO component
    pub fn hello(&self) {
        unsafe {
            xtest_hello();
        }
    }

    /// Get a boolean value from UNO
    pub fn get_boolean(&self) -> bool {
        unsafe {
            let result = xtest_getBoolean();
            result != 0
        }
    }

    /// Test if a boolean value matches the expected UNO boolean
    pub fn is_boolean(&self, value: bool) -> bool {
        unsafe {
            let sal_value = if value { 1 } else { 0 };
            let result = xtest_isBoolean(sal_value);
            result != 0
        }
    }

    /// Get a byte value from UNO
    pub fn get_byte(&self) -> i8 {
        unsafe { xtest_getByte() }
    }

    /// Test if a byte value matches the expected UNO byte
    pub fn is_byte(&self, value: i8) -> bool {
        unsafe {
            let result = xtest_isByte(value);
            result != 0
        }
    }

    /// Get a short value from UNO
    pub fn get_short(&self) -> i16 {
        unsafe { xtest_getShort() }
    }

    /// Test if a short value matches the expected UNO short
    pub fn is_short(&self, value: i16) -> bool {
        unsafe {
            let result = xtest_isShort(value);
            result != 0
        }
    }

    /// Get a long value from UNO
    pub fn get_long(&self) -> i32 {
        unsafe { xtest_getLong() }
    }

    /// Test if a long value matches the expected UNO long
    pub fn is_long(&self, value: i32) -> bool {
        unsafe {
            let result = xtest_isLong(value);
            result != 0
        }
    }

    /// Get an unsigned short value from UNO
    pub fn get_unsigned_short(&self) -> u16 {
        unsafe { xtest_getUnsignedShort() }
    }

    /// Test if an unsigned short value matches the expected UNO unsigned short
    pub fn is_unsigned_short(&self, value: u16) -> bool {
        unsafe {
            let result = xtest_isUnsignedShort(value);
            result != 0
        }
    }

    /// Get an unsigned long value from UNO
    pub fn get_unsigned_long(&self) -> u32 {
        unsafe { xtest_getUnsignedLong() }
    }

    /// Test if an unsigned long value matches the expected UNO unsigned long
    pub fn is_unsigned_long(&self, value: u32) -> bool {
        unsafe {
            let result = xtest_isUnsignedLong(value);
            result != 0
        }
    }

    /// Get a hyper (64-bit) value from UNO
    pub fn get_hyper(&self) -> i64 {
        unsafe { xtest_getHyper() }
    }

    /// Test if a hyper value matches the expected UNO hyper
    pub fn is_hyper(&self, value: i64) -> bool {
        unsafe {
            let result = xtest_isHyper(value);
            result != 0
        }
    }

    /// Get an unsigned hyper (64-bit) value from UNO
    pub fn get_unsigned_hyper(&self) -> u64 {
        unsafe { xtest_getUnsignedHyper() }
    }

    /// Test if an unsigned hyper value matches the expected UNO unsigned hyper
    pub fn is_unsigned_hyper(&self, value: u64) -> bool {
        unsafe {
            let result = xtest_isUnsignedHyper(value);
            result != 0
        }
    }

    /// Get a float value from UNO
    pub fn get_float(&self) -> f32 {
        unsafe { xtest_getFloat() }
    }

    /// Test if a float value matches the expected UNO float
    pub fn is_float(&self, value: f32) -> bool {
        unsafe {
            let result = xtest_isFloat(value);
            result != 0
        }
    }

    /// Get a double value from UNO
    pub fn get_double(&self) -> f64 {
        unsafe { xtest_getDouble() }
    }

    /// Test if a double value matches the expected UNO double
    pub fn is_double(&self, value: f64) -> bool {
        unsafe {
            let result = xtest_isDouble(value);
            result != 0
        }
    }
    /// Get a char value from UNO
    pub fn get_char(&self) -> sal_Unicode {
        unsafe { xtest_getChar() }
    }

    /// Test if a char value matches the expected UNO char
    pub fn is_char(&self, value: sal_Unicode) -> bool {
        unsafe {
            let result = xtest_isChar(value);
            result != 0
        }
    }

    /// Get a string value from UNO
    pub fn get_string(&self) -> OUString {
        unsafe {
            let result = xtest_getString();
            OUString::from_raw(result)
        }
    }

    /// Test if a string value matches the expected UNO string
    pub fn is_string(&self, value: OUString) -> bool {
        unsafe {
            let result = xtest_isString(value.into_raw());
            result != 0
        }
    }

    /// Get a Type value from UNO (returns cppu::UnoType<sal_Int32>)
    pub fn get_type(&self) -> Type {
        unsafe { Type::from_typelib_ref_no_acquire(xtest_getType()) }
    }

    /// Test if a Type value matches the expected UNO Type
    ///
    /// Note: This test uses raw pointers for C++ Type interop.
    /// For Rust Type instances, use direct comparison methods.
    pub fn is_type(&self, cpp_type: Type) -> bool {
        unsafe {
            let result = xtest_isType(cpp_type.into_raw());
            result != 0
        }
    }
}

impl Default for XTest {
    fn default() -> Self {
        Self::new()
    }
}

// === Integration Tests ===

#[test]
fn test_xtest_interface_creation() {
    let xtest = XTest::new();
    // Test that we can create the interface without panicking
    xtest.hello(); // This should print a message
}

#[test]
fn test_boolean_operations() {
    let xtest = XTest::new();

    // Test getBoolean
    let result = xtest.get_boolean();
    assert!(result, "getBoolean should return true");

    // Test isBoolean with true
    let is_true = xtest.is_boolean(true);
    assert!(is_true, "isBoolean(true) should return true");

    // Test isBoolean with false
    let is_false = xtest.is_boolean(false);
    assert!(!is_false, "isBoolean(false) should return false");
}

#[test]
fn test_integer_operations() {
    let xtest = XTest::new();

    // Test byte operations
    let byte_val = xtest.get_byte();
    assert_eq!(byte_val, -12, "getByte should return -12");

    let is_byte = xtest.is_byte(-12);
    assert!(is_byte, "isByte(-12) should return true");

    let is_wrong_byte = xtest.is_byte(0);
    assert!(!is_wrong_byte, "isByte(0) should return false");

    // Test short operations
    let short_val = xtest.get_short();
    assert_eq!(short_val, -1234, "getShort should return -1234");

    let is_short = xtest.is_short(-1234);
    assert!(is_short, "isShort(-1234) should return true");

    let is_wrong_short = xtest.is_short(0);
    assert!(!is_wrong_short, "isShort(0) should return false");

    // Test long operations
    let long_val = xtest.get_long();
    assert_eq!(long_val, -123456, "getLong should return -123456");

    let is_long = xtest.is_long(-123456);
    assert!(is_long, "isLong(-123456) should return true");

    let is_wrong_long = xtest.is_long(0);
    assert!(!is_wrong_long, "isLong(0) should return false");
}

#[test]
fn test_unsigned_integer_operations() {
    let xtest = XTest::new();

    // Test unsigned short operations
    let ushort_val = xtest.get_unsigned_short();
    assert_eq!(ushort_val, 54321, "getUnsignedShort should return 54321");

    let is_ushort = xtest.is_unsigned_short(54321);
    assert!(is_ushort, "isUnsignedShort(54321) should return true");

    let is_wrong_ushort = xtest.is_unsigned_short(0);
    assert!(!is_wrong_ushort, "isUnsignedShort(0) should return false");

    // Test unsigned long operations
    let ulong_val = xtest.get_unsigned_long();
    assert_eq!(
        ulong_val, 3456789012,
        "getUnsignedLong should return 3456789012"
    );

    let is_ulong = xtest.is_unsigned_long(3456789012);
    assert!(is_ulong, "isUnsignedLong(3456789012) should return true");

    let is_wrong_ulong = xtest.is_unsigned_long(0);
    assert!(!is_wrong_ulong, "isUnsignedLong(0) should return false");
}

#[test]
fn test_64bit_operations() {
    let xtest = XTest::new();

    // Test hyper operations
    let hyper_val = xtest.get_hyper();
    assert_eq!(hyper_val, -123456789, "getHyper should return -123456789");

    let is_hyper = xtest.is_hyper(-123456789);
    assert!(is_hyper, "isHyper(-123456789) should return true");

    let is_wrong_hyper = xtest.is_hyper(0);
    assert!(!is_wrong_hyper, "isHyper(0) should return false");

    // Test unsigned hyper operations
    let uhyper_val = xtest.get_unsigned_hyper();
    assert_eq!(
        uhyper_val, 9876543210,
        "getUnsignedHyper should return 9876543210"
    );

    let is_uhyper = xtest.is_unsigned_hyper(9876543210);
    assert!(is_uhyper, "isUnsignedHyper(9876543210) should return true");

    let is_wrong_uhyper = xtest.is_unsigned_hyper(0);
    assert!(!is_wrong_uhyper, "isUnsignedHyper(0) should return false");
}

#[test]
fn test_floating_point_operations() {
    let xtest = XTest::new();

    // Test float operations
    let float_val = xtest.get_float();
    assert_eq!(float_val, -10.25, "getFloat should return -10.25");

    let is_float = xtest.is_float(-10.25);
    assert!(is_float, "isFloat(-10.25) should return true");

    let is_wrong_float = xtest.is_float(0.0);
    assert!(!is_wrong_float, "isFloat(0.0) should return false");

    // Test double operations
    let double_val = xtest.get_double();
    assert_eq!(double_val, 100.5, "getDouble should return 100.5");

    let is_double = xtest.is_double(100.5);
    assert!(is_double, "isDouble(100.5) should return true");

    let is_wrong_double = xtest.is_double(0.0);
    assert!(!is_wrong_double, "isDouble(0.0) should return false");
}

#[test]
fn test_char_operations() {
    let xtest = XTest::new();

    // Test char operations - 'Ö' is Unicode codepoint U+00D6 (214)
    let char_val = xtest.get_char();
    assert_eq!(
        char_val, 214u16,
        "getChar should return 214 (Unicode for 'Ö')"
    );

    let is_char = xtest.is_char(214u16);
    assert!(is_char, "isChar(214) should return true");

    let is_wrong_char = xtest.is_char(65u16); // 'A'
    assert!(!is_wrong_char, "isChar(65) should return false");
}

#[test]
fn test_string_operations() {
    let xtest = XTest::new();

    // Test string operations - expected value is "hä"
    let string_val = xtest.get_string();
    assert_eq!(string_val.to_string(), "hä", "getString should return 'hä'");

    // Test string validation with correct value
    let test_string = OUString::from_utf8("hä");
    let is_string = xtest.is_string(test_string);
    assert!(is_string, "isString('hä') should return true");

    // Test string validation with wrong value
    let wrong_string = OUString::from_utf8("wrong");
    let is_wrong_string = xtest.is_string(wrong_string);
    assert!(!is_wrong_string, "isString('wrong') should return false");
}

#[test]
fn test_type_operations() {
    let xtest = XTest::new();

    // Test getting a Type value from C++ component
    let cpp_type = xtest.get_type();
    // Test that we got a valid Type (non-void type in this case)
    assert_ne!(
        cpp_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_VOID,
        "C++ Type should not be void type"
    );
    println!("Got C++ Type: {:?}", cpp_type);

    // Test that the returned Type matches the expected Type (cppu::UnoType<sal_Int32>)
    let is_correct = xtest.is_type(cpp_type);
    assert!(is_correct, "C++ Type should match expected sal_Int32 type");
}

#[test]
fn test_wrong_values_comprehensive() {
    let xtest = XTest::new();

    // Test multiple wrong values for each function to ensure robust validation

    // Test multiple wrong byte values
    let is_wrong_byte1 = xtest.is_byte(100);
    assert!(!is_wrong_byte1, "isByte(100) should return false");

    let is_wrong_byte2 = xtest.is_byte(-100);
    assert!(!is_wrong_byte2, "isByte(-100) should return false");

    // Test multiple wrong short values
    let is_wrong_short1 = xtest.is_short(1000);
    assert!(!is_wrong_short1, "isShort(1000) should return false");

    let is_wrong_short2 = xtest.is_short(-5000);
    assert!(!is_wrong_short2, "isShort(-5000) should return false");

    // Test multiple wrong long values
    let is_wrong_long1 = xtest.is_long(999999);
    assert!(!is_wrong_long1, "isLong(999999) should return false");

    let is_wrong_long2 = xtest.is_long(-999999);
    assert!(!is_wrong_long2, "isLong(-999999) should return false");

    // Test multiple wrong unsigned short values
    let is_wrong_ushort1 = xtest.is_unsigned_short(12345);
    assert!(
        !is_wrong_ushort1,
        "isUnsignedShort(12345) should return false"
    );

    let is_wrong_ushort2 = xtest.is_unsigned_short(65535);
    assert!(
        !is_wrong_ushort2,
        "isUnsignedShort(65535) should return false"
    );

    // Test multiple wrong unsigned long values
    let is_wrong_ulong1 = xtest.is_unsigned_long(1234567890);
    assert!(
        !is_wrong_ulong1,
        "isUnsignedLong(1234567890) should return false"
    );

    let is_wrong_ulong2 = xtest.is_unsigned_long(4294967295);
    assert!(
        !is_wrong_ulong2,
        "isUnsignedLong(4294967295) should return false"
    );

    // Test multiple wrong hyper values
    let is_wrong_hyper1 = xtest.is_hyper(987654321);
    assert!(!is_wrong_hyper1, "isHyper(987654321) should return false");

    let is_wrong_hyper2 = xtest.is_hyper(-987654321);
    assert!(!is_wrong_hyper2, "isHyper(-987654321) should return false");

    // Test multiple wrong unsigned hyper values
    let is_wrong_uhyper1 = xtest.is_unsigned_hyper(1234567890);
    assert!(
        !is_wrong_uhyper1,
        "isUnsignedHyper(1234567890) should return false"
    );

    let is_wrong_uhyper2 = xtest.is_unsigned_hyper(18446744073709551615);
    assert!(
        !is_wrong_uhyper2,
        "isUnsignedHyper(18446744073709551615) should return false"
    );

    // Test multiple wrong float values
    let is_wrong_float1 = xtest.is_float(std::f32::consts::PI);
    assert!(!is_wrong_float1, "isFloat(3.14159) should return false");

    let is_wrong_float2 = xtest.is_float(-99.99);
    assert!(!is_wrong_float2, "isFloat(-99.99) should return false");

    // Test multiple wrong double values
    let is_wrong_double1 = xtest.is_double(std::f64::consts::E);
    assert!(!is_wrong_double1, "isDouble(2.71828) should return false");

    let is_wrong_double2 = xtest.is_double(-200.75);
    assert!(!is_wrong_double2, "isDouble(-200.75) should return false");

    // Test multiple wrong char values
    let is_wrong_char1 = xtest.is_char(72u16); // 'H'
    assert!(!is_wrong_char1, "isChar(72) should return false");

    let is_wrong_char2 = xtest.is_char(8364u16); // Euro symbol '€'
    assert!(!is_wrong_char2, "isChar(8364) should return false");
}

#[test]
fn test_all_operations_comprehensive() {
    let xtest = XTest::new();

    // This test verifies that all methods work together
    // and the component maintains state correctly

    // Test multiple calls to the same method
    let bool1 = xtest.get_boolean();
    let bool2 = xtest.get_boolean();
    assert_eq!(
        bool1, bool2,
        "Multiple calls to getBoolean should return same value"
    );

    // Test that different data types work independently
    let _byte_val = xtest.get_byte();
    let _short_val = xtest.get_short();
    let _long_val = xtest.get_long();
    let _float_val = xtest.get_float();
    let _double_val = xtest.get_double();

    // Verify boolean still works after other operations
    let bool3 = xtest.get_boolean();
    assert_eq!(bool1, bool3, "Boolean operations should be consistent");
}

#[test]
fn test_edge_cases_and_boundary_values() {
    let xtest = XTest::new();

    // Test edge cases for each data type to ensure robust handling

    // Test maximum and minimum values for signed integers
    let is_max_byte = xtest.is_byte(i8::MAX);
    assert!(!is_max_byte, "isByte(i8::MAX) should return false");

    let is_min_byte = xtest.is_byte(i8::MIN);
    assert!(!is_min_byte, "isByte(i8::MIN) should return false");

    let is_max_short = xtest.is_short(i16::MAX);
    assert!(!is_max_short, "isShort(i16::MAX) should return false");

    let is_min_short = xtest.is_short(i16::MIN);
    assert!(!is_min_short, "isShort(i16::MIN) should return false");

    let is_max_long = xtest.is_long(i32::MAX);
    assert!(!is_max_long, "isLong(i32::MAX) should return false");

    let is_min_long = xtest.is_long(i32::MIN);
    assert!(!is_min_long, "isLong(i32::MIN) should return false");

    // Test maximum values for unsigned integers
    let is_max_ushort = xtest.is_unsigned_short(u16::MAX);
    assert!(
        !is_max_ushort,
        "isUnsignedShort(u16::MAX) should return false"
    );

    let is_max_ulong = xtest.is_unsigned_long(u32::MAX);
    assert!(
        !is_max_ulong,
        "isUnsignedLong(u32::MAX) should return false"
    );

    // Test edge cases for 64-bit integers
    let is_max_hyper = xtest.is_hyper(i64::MAX);
    assert!(!is_max_hyper, "isHyper(i64::MAX) should return false");

    let is_min_hyper = xtest.is_hyper(i64::MIN);
    assert!(!is_min_hyper, "isHyper(i64::MIN) should return false");

    let is_max_uhyper = xtest.is_unsigned_hyper(u64::MAX);
    assert!(
        !is_max_uhyper,
        "isUnsignedHyper(u64::MAX) should return false"
    );

    // Test floating point edge cases
    let is_infinity = xtest.is_float(f32::INFINITY);
    assert!(!is_infinity, "isFloat(INFINITY) should return false");

    let is_neg_infinity = xtest.is_float(f32::NEG_INFINITY);
    assert!(
        !is_neg_infinity,
        "isFloat(NEG_INFINITY) should return false"
    );

    let is_double_infinity = xtest.is_double(f64::INFINITY);
    assert!(
        !is_double_infinity,
        "isDouble(INFINITY) should return false"
    );

    let is_double_neg_infinity = xtest.is_double(f64::NEG_INFINITY);
    assert!(
        !is_double_neg_infinity,
        "isDouble(NEG_INFINITY) should return false"
    );

    // Test special Unicode characters
    let is_null_char = xtest.is_char(0u16); // Null character
    assert!(!is_null_char, "isChar(0) should return false");

    let is_high_unicode = xtest.is_char(0xFFFF); // Maximum Unicode BMP
    assert!(!is_high_unicode, "isChar(0xFFFF) should return false");

    // Test empty and special strings
    let empty_string = OUString::from_utf8("");
    let is_empty_string = xtest.is_string(empty_string);
    assert!(!is_empty_string, "isString(\"\") should return false");

    let unicode_string = OUString::from_utf8("🦀"); // Rust crab emoji
    let is_unicode_string = xtest.is_string(unicode_string);
    assert!(!is_unicode_string, "isString(\"🦀\") should return false");
}

#[test]
fn test_consistency_across_multiple_calls() {
    let xtest = XTest::new();

    // Test that all getter functions return consistent values across multiple calls

    // Test consistency for all data types
    for _ in 0..5 {
        // Boolean consistency
        let bool_val = xtest.get_boolean();
        assert!(bool_val, "getBoolean should always return true");

        // Integer consistency
        let byte_val = xtest.get_byte();
        assert_eq!(byte_val, -12, "getByte should always return -12");

        let short_val = xtest.get_short();
        assert_eq!(short_val, -1234, "getShort should always return -1234");

        let long_val = xtest.get_long();
        assert_eq!(long_val, -123456, "getLong should always return -123456");

        // Unsigned integer consistency
        let ushort_val = xtest.get_unsigned_short();
        assert_eq!(
            ushort_val, 54321,
            "getUnsignedShort should always return 54321"
        );

        let ulong_val = xtest.get_unsigned_long();
        assert_eq!(
            ulong_val, 3456789012,
            "getUnsignedLong should always return 3456789012"
        );

        // 64-bit consistency
        let hyper_val = xtest.get_hyper();
        assert_eq!(
            hyper_val, -123456789,
            "getHyper should always return -123456789"
        );

        let uhyper_val = xtest.get_unsigned_hyper();
        assert_eq!(
            uhyper_val, 9876543210,
            "getUnsignedHyper should always return 9876543210"
        );

        // Floating point consistency
        let float_val = xtest.get_float();
        assert_eq!(float_val, -10.25, "getFloat should always return -10.25");

        let double_val = xtest.get_double();
        assert_eq!(double_val, 100.5, "getDouble should always return 100.5");

        // Character consistency
        let char_val = xtest.get_char();
        assert_eq!(char_val, 214u16, "getChar should always return 214");

        // String consistency
        let string_val = xtest.get_string();
        assert_eq!(
            string_val.to_string(),
            "hä",
            "getString should always return 'hä'"
        );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
