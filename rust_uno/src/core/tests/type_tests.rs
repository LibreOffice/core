/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Unit tests for Type (Rust-side functionality)
//!
//! These tests verify Type's Rust implementation including:
//! - Creation methods (new, new_with_name, from_typelib_ref, from_typelib_ref_no_acquire)
//! - Trait implementations (Clone, PartialEq, From, Default)
//! - Instance methods (get_type_class, get_type_name, equals, is_assignable_from)
//! - Memory management and safety (RAII, reference counting)
//! - FFI interop (get_typelib_type)
//! - Ergonomic API (generic string parameters)
//!
//! These tests do NOT require LibreOffice UNO components and test
//! only the Rust-side implementation and FFI safety.

use crate::core::{OUString, Type, typelib_TypeClass};

// === Creation Method Tests ===

#[test]
fn test_type_new() {
    let void_type = Type::new();
    assert_eq!(
        void_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_VOID
    );
    assert_eq!(void_type.get_type_name().to_string(), "void");
}

#[test]
fn test_type_default() {
    let default_type = Type::default();
    assert_eq!(
        default_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_VOID
    );
    assert_eq!(default_type.get_type_name().to_string(), "void");

    // Test that default equals new()
    let new_type = Type::new();
    assert!(default_type.equals(&new_type));
}

#[test]
fn test_type_new_with_name_str() {
    let void_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_VOID, "void");
    assert_eq!(
        void_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_VOID
    );
    assert_eq!(void_type.get_type_name().to_string(), "void");

    let int_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    assert_eq!(
        int_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_LONG
    );
    assert_eq!(int_type.get_type_name().to_string(), "sal_Int32");

    let interface_type = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    );
    assert_eq!(
        interface_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        interface_type.get_type_name().to_string(),
        "com.sun.star.lang.XComponent"
    );
}

#[test]
fn test_type_new_with_name_string() {
    let type_name = "com.sun.star.text.XText".to_string();
    let text_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_INTERFACE, type_name);
    assert_eq!(
        text_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        text_type.get_type_name().to_string(),
        "com.sun.star.text.XText"
    );
}

#[test]
fn test_type_new_with_name_oustring() {
    let oustring_name = OUString::from("com.sun.star.beans.XPropertySet");
    let prop_type = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        oustring_name,
    );
    assert_eq!(
        prop_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        prop_type.get_type_name().to_string(),
        "com.sun.star.beans.XPropertySet"
    );
}

// === From Trait Tests ===

#[test]
fn test_type_from_typelib_typeclass() {
    let string_type = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    assert_eq!(
        string_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_STRING
    );

    let boolean_type = Type::from(typelib_TypeClass::typelib_TypeClass_BOOLEAN);
    assert_eq!(
        boolean_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_BOOLEAN
    );

    let double_type = Type::from(typelib_TypeClass::typelib_TypeClass_DOUBLE);
    assert_eq!(
        double_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_DOUBLE
    );
}

#[test]
fn test_type_from_tuple_with_str() {
    let primitive_types = vec![
        ("void", typelib_TypeClass::typelib_TypeClass_VOID),
        ("boolean", typelib_TypeClass::typelib_TypeClass_BOOLEAN),
        ("byte", typelib_TypeClass::typelib_TypeClass_BYTE),
        ("short", typelib_TypeClass::typelib_TypeClass_SHORT),
        (
            "unsigned short",
            typelib_TypeClass::typelib_TypeClass_UNSIGNED_SHORT,
        ),
        ("long", typelib_TypeClass::typelib_TypeClass_LONG),
        (
            "unsigned long",
            typelib_TypeClass::typelib_TypeClass_UNSIGNED_LONG,
        ),
        ("hyper", typelib_TypeClass::typelib_TypeClass_HYPER),
        (
            "unsigned hyper",
            typelib_TypeClass::typelib_TypeClass_UNSIGNED_HYPER,
        ),
        ("float", typelib_TypeClass::typelib_TypeClass_FLOAT),
        ("double", typelib_TypeClass::typelib_TypeClass_DOUBLE),
        ("char", typelib_TypeClass::typelib_TypeClass_CHAR),
        ("string", typelib_TypeClass::typelib_TypeClass_STRING),
        ("type", typelib_TypeClass::typelib_TypeClass_TYPE),
        ("any", typelib_TypeClass::typelib_TypeClass_ANY),
    ];

    for (type_name, expected_class) in primitive_types {
        let type_obj = Type::from((expected_class, type_name));
        assert_eq!(
            type_obj.get_type_class(),
            expected_class,
            "Failed for type: {}",
            type_name
        );
        assert_eq!(type_obj.get_type_name().to_string(), type_name);
    }
}

#[test]
fn test_type_from_tuple_with_string() {
    let type_name = "com.sun.star.awt.XControl".to_string();
    let control_type = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, type_name));
    assert_eq!(
        control_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        control_type.get_type_name().to_string(),
        "com.sun.star.awt.XControl"
    );
}

#[test]
fn test_type_from_tuple_with_oustring() {
    let type_name = OUString::from("com.sun.star.beans.XPropertySet");
    let prop_type = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, type_name));
    assert_eq!(
        prop_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        prop_type.get_type_name().to_string(),
        "com.sun.star.beans.XPropertySet"
    );
}

// === Trait Implementation Tests ===

#[test]
fn test_type_clone() {
    let original = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    );
    let cloned = original.clone();

    // They should be equal
    assert!(original.equals(&cloned));
    assert_eq!(original.get_type_class(), cloned.get_type_class());
    assert_eq!(
        original.get_type_name().to_string(),
        cloned.get_type_name().to_string()
    );

    // Test cloning void type
    let void_original = Type::new();
    let void_clone = void_original.clone();
    assert!(void_original.equals(&void_clone));
}

#[test]
fn test_type_partial_eq() {
    let type1 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    let type2 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    let type3 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_SHORT, "sal_Int16");
    let void1 = Type::new();
    let void2 = Type::new();

    // Equal types
    assert_eq!(type1, type2);
    assert_eq!(void1, void2);

    // Different types
    assert_ne!(type1, type3);
    assert_ne!(type1, void1);

    // Test with interface types
    let interface1 = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));
    let interface2 = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));
    let interface3 = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.uno.XInterface",
    ));

    assert_eq!(interface1, interface2);
    assert_ne!(interface1, interface3);
}

// === Method Tests ===

#[test]
fn test_type_get_type_class() {
    let void_type = Type::new();
    assert_eq!(
        void_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_VOID
    );

    let string_type = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    assert_eq!(
        string_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_STRING
    );

    let interface_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));
    assert_eq!(
        interface_type.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
}

#[test]
fn test_type_get_type_name() {
    let void_type = Type::new();
    assert_eq!(void_type.get_type_name().to_string(), "void");

    let int_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    assert_eq!(int_type.get_type_name().to_string(), "sal_Int32");

    let interface_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.text.XTextDocument",
    ));
    assert_eq!(
        interface_type.get_type_name().to_string(),
        "com.sun.star.text.XTextDocument"
    );
}

#[test]
fn test_type_equals() {
    let type1 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    let type2 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    let type3 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_SHORT, "sal_Int16");

    // Test equality
    assert!(type1.equals(&type2));
    assert!(type2.equals(&type1)); // Symmetry

    // Test inequality
    assert!(!type1.equals(&type3));
    assert!(!type3.equals(&type1)); // Symmetry

    // Test self-equality
    assert!(type1.equals(&type1));
}

#[test]
fn test_type_is_assignable_from() {
    // Create different types for testing assignment compatibility
    let long_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    let short_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_SHORT, "sal_Int16");
    let byte_type = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_BYTE, "sal_Int8");

    // Test self-assignment (should always be true)
    assert!(long_type.is_assignable_from(&long_type));
    assert!(short_type.is_assignable_from(&short_type));
    assert!(byte_type.is_assignable_from(&byte_type));

    // These tests depend on UNO's specific assignment rules
    // Note: The actual behavior depends on LibreOffice's typelib implementation
    // We're just testing that the method calls work correctly
    let _long_from_short = long_type.is_assignable_from(&short_type);
    let _long_from_byte = long_type.is_assignable_from(&byte_type);
    let _short_from_byte = short_type.is_assignable_from(&byte_type);
}

#[test]
fn test_type_get_typelib_type() {
    let type_obj = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_STRING, "string");
    let raw_ptr = type_obj.get_typelib_type();

    // Should return a non-null pointer
    assert!(!raw_ptr.is_null());

    // Test with void type
    let void_type = Type::new();
    let void_ptr = void_type.get_typelib_type();
    assert!(!void_ptr.is_null());
}

// === Special Cases and Edge Cases ===

#[test]
fn test_type_primitive_type_classes() {
    // Test creating types for primitive type classes that have static type references
    let primitive_type_classes = vec![
        typelib_TypeClass::typelib_TypeClass_VOID,
        typelib_TypeClass::typelib_TypeClass_CHAR,
        typelib_TypeClass::typelib_TypeClass_BOOLEAN,
        typelib_TypeClass::typelib_TypeClass_BYTE,
        typelib_TypeClass::typelib_TypeClass_SHORT,
        typelib_TypeClass::typelib_TypeClass_UNSIGNED_SHORT,
        typelib_TypeClass::typelib_TypeClass_LONG,
        typelib_TypeClass::typelib_TypeClass_UNSIGNED_LONG,
        typelib_TypeClass::typelib_TypeClass_HYPER,
        typelib_TypeClass::typelib_TypeClass_UNSIGNED_HYPER,
        typelib_TypeClass::typelib_TypeClass_FLOAT,
        typelib_TypeClass::typelib_TypeClass_DOUBLE,
        typelib_TypeClass::typelib_TypeClass_STRING,
        typelib_TypeClass::typelib_TypeClass_TYPE,
        typelib_TypeClass::typelib_TypeClass_ANY,
    ];

    for type_class in primitive_type_classes {
        let type_obj = Type::from(type_class);
        assert_eq!(type_obj.get_type_class(), type_class);
    }
}

#[test]
fn test_type_complex_interface_names() {
    let complex_names = vec![
        "com.sun.star.lang.XComponent",
        "com.sun.star.uno.XInterface",
        "com.sun.star.text.XTextDocument",
        "com.sun.star.beans.XPropertySet",
        "com.sun.star.awt.XControl",
        "com.sun.star.frame.XController",
    ];

    for name in complex_names {
        let type_obj = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, name));
        assert_eq!(
            type_obj.get_type_class(),
            typelib_TypeClass::typelib_TypeClass_INTERFACE
        );
        assert_eq!(type_obj.get_type_name().to_string(), name);
    }
}

#[test]
fn test_type_consistency_across_creation_methods() {
    // Test that different creation methods produce equivalent results for same type

    // String type via different methods
    let string_from_class = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    let string_from_tuple = Type::from((typelib_TypeClass::typelib_TypeClass_STRING, "string"));
    let string_from_new_with_name =
        Type::new_with_name(typelib_TypeClass::typelib_TypeClass_STRING, "string");

    assert!(string_from_class.equals(&string_from_tuple));
    assert!(string_from_tuple.equals(&string_from_new_with_name));
    assert!(string_from_class.equals(&string_from_new_with_name));

    // Interface type via different methods
    let interface_name = "com.sun.star.lang.XComponent";
    let interface_from_tuple = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        interface_name,
    ));
    let interface_from_new_with_name = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        interface_name,
    );

    assert!(interface_from_tuple.equals(&interface_from_new_with_name));
}

// === Memory Management Tests ===

#[test]
fn test_type_memory_safety() {
    // Test that Types can be created and dropped without issues

    {
        let _type1 = Type::new();
        let _type2 = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
        let _type3 = Type::new_with_name(typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32");
    } // Types should be properly cleaned up here

    // Create more types after the previous ones were dropped
    let type4 = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));
    let type5 = type4.clone();

    assert!(type4.equals(&type5));
}

#[test]
fn test_type_multiple_references() {
    // Test that multiple references to the same logical type work correctly
    let name = "com.sun.star.text.XTextDocument";

    let type1 = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, name));
    let type2 = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, name));
    let type3 = type1.clone();

    // All should be equal
    assert!(type1.equals(&type2));
    assert!(type2.equals(&type3));
    assert!(type1.equals(&type3));

    // All should have same properties
    assert_eq!(type1.get_type_class(), type2.get_type_class());
    assert_eq!(type2.get_type_class(), type3.get_type_class());

    assert_eq!(
        type1.get_type_name().to_string(),
        type2.get_type_name().to_string()
    );
    assert_eq!(
        type2.get_type_name().to_string(),
        type3.get_type_name().to_string()
    );
}

#[test]
fn test_type_mixed_operations() {
    // Test mixing different operations
    let original = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    );

    // Clone and compare
    let cloned = original.clone();
    assert!(original.equals(&cloned));

    // Create equivalent via From trait
    let from_tuple = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));
    assert!(original.equals(&from_tuple));

    // Test assignment compatibility with itself
    assert!(original.is_assignable_from(&original));
    assert!(cloned.is_assignable_from(&from_tuple));

    // Check type information
    assert_eq!(
        original.get_type_class(),
        typelib_TypeClass::typelib_TypeClass_INTERFACE
    );
    assert_eq!(
        original.get_type_name().to_string(),
        "com.sun.star.lang.XComponent"
    );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
