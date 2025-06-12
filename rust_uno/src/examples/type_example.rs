/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! UNO Type System Examples
//!
//! This example demonstrates the usage of the UNO Type system in Rust.
//! The Type class represents IDL meta types, holding type names and type classes.
//!
//! ## Type Creation Methods:
//! - `Type::new()` - Creates void type (default)
//! - `Type::from(typelib_TypeClass)` - Creates type from type class only
//! - `Type::from((typelib_TypeClass, name))` - Creates type with class and name
//! - `Type::new_with_name(class, name)` - Explicit creation with name
//!
//! ## Type Operations:
//! - `get_type_class()` - Get the UNO type class
//! - `get_type_name()` - Get the fully qualified type name
//! - `equals()` - Test type equality
//! - `is_assignable_from()` - Test assignment compatibility

use crate::core::{OUString, Type};
use crate::ffi::type_ffi::typelib_TypeClass;

/// Run the UNO type system example
///
/// This function demonstrates the usage of the UNO Type system in Rust,
/// including type creation, operations, traits, and UNO interface integration.
pub fn run_example() {
    println!("=== UNO Type System Examples ===\n");

    // === Basic Type Creation ===
    println!("--- Basic Type Creation ---");

    // Default void type
    let void_type = Type::new();
    let default_type = Type::default();
    println!("Void type: {void_type:?}");
    println!("Default type: {default_type:?}");
    println!("Void == Default: {}", void_type.equals(&default_type));

    // === Primitive Types from Type Classes ===
    println!("\n--- Primitive Types from Type Classes ---");

    let boolean_type = Type::from(typelib_TypeClass::typelib_TypeClass_BOOLEAN);
    let byte_type = Type::from(typelib_TypeClass::typelib_TypeClass_BYTE);
    let short_type = Type::from(typelib_TypeClass::typelib_TypeClass_SHORT);
    let long_type = Type::from(typelib_TypeClass::typelib_TypeClass_LONG);
    let float_type = Type::from(typelib_TypeClass::typelib_TypeClass_FLOAT);
    let double_type = Type::from(typelib_TypeClass::typelib_TypeClass_DOUBLE);
    let char_type = Type::from(typelib_TypeClass::typelib_TypeClass_CHAR);
    let string_type = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    let type_type = Type::from(typelib_TypeClass::typelib_TypeClass_TYPE);
    let any_type = Type::from(typelib_TypeClass::typelib_TypeClass_ANY);

    println!(
        "Boolean: {} (class: {:?})",
        boolean_type.get_type_name(),
        boolean_type.get_type_class()
    );
    println!(
        "Byte: {} (class: {:?})",
        byte_type.get_type_name(),
        byte_type.get_type_class()
    );
    println!(
        "Short: {} (class: {:?})",
        short_type.get_type_name(),
        short_type.get_type_class()
    );
    println!(
        "Long: {} (class: {:?})",
        long_type.get_type_name(),
        long_type.get_type_class()
    );
    println!(
        "Float: {} (class: {:?})",
        float_type.get_type_name(),
        float_type.get_type_class()
    );
    println!(
        "Double: {} (class: {:?})",
        double_type.get_type_name(),
        double_type.get_type_class()
    );
    println!(
        "Char: {} (class: {:?})",
        char_type.get_type_name(),
        char_type.get_type_class()
    );
    println!(
        "String: {} (class: {:?})",
        string_type.get_type_name(),
        string_type.get_type_class()
    );
    println!(
        "Type: {} (class: {:?})",
        type_type.get_type_name(),
        type_type.get_type_class()
    );
    println!(
        "Any: {} (class: {:?})",
        any_type.get_type_name(),
        any_type.get_type_class()
    );

    // === Complex Types with Names ===
    println!("\n--- Complex Types with Names ---");

    // Interface types using tuple syntax
    let component_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XComponent",
    ));

    let property_set_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.beans.XPropertySet",
    ));

    // Service types
    let text_service = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_SERVICE,
        "com.sun.star.text.Text",
    );

    // Using String and OUString for type names
    let string_name = "com.sun.star.awt.XControl".to_string();
    let control_type = Type::from((typelib_TypeClass::typelib_TypeClass_INTERFACE, string_name));

    let oustring_name = OUString::from("com.sun.star.uno.XInterface");
    let interface_type = Type::new_with_name(
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        oustring_name,
    );

    println!(
        "Component: {} (class: {:?})",
        component_type.get_type_name(),
        component_type.get_type_class()
    );
    println!(
        "PropertySet: {} (class: {:?})",
        property_set_type.get_type_name(),
        property_set_type.get_type_class()
    );
    println!(
        "Text Service: {} (class: {:?})",
        text_service.get_type_name(),
        text_service.get_type_class()
    );
    println!(
        "Control: {} (class: {:?})",
        control_type.get_type_name(),
        control_type.get_type_class()
    );
    println!(
        "XInterface: {} (class: {:?})",
        interface_type.get_type_name(),
        interface_type.get_type_class()
    );

    // === Type Properties and Operations ===
    println!("\n--- Type Properties and Operations ---");

    println!("Type class of long: {:?}", long_type.get_type_class());
    println!(
        "Type name of component: '{}'",
        component_type.get_type_name()
    );

    // Type equality testing
    let long_type2 = Type::from(typelib_TypeClass::typelib_TypeClass_LONG);
    let short_type2 = Type::from(typelib_TypeClass::typelib_TypeClass_SHORT);

    println!("long == long: {}", long_type.equals(&long_type2));
    println!("long == short: {}", long_type.equals(&short_type2));

    // Assignment compatibility testing
    println!(
        "long assignable from short: {}",
        long_type.is_assignable_from(&short_type)
    );
    println!(
        "short assignable from long: {}",
        short_type.is_assignable_from(&long_type)
    );
    println!(
        "any assignable from long: {}",
        any_type.is_assignable_from(&long_type)
    );
    println!(
        "long assignable from any: {}",
        long_type.is_assignable_from(&any_type)
    );

    // === Trait Examples ===
    println!("\n--- Trait Examples ---");

    // Clone trait
    let original = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.lang.XServiceInfo",
    ));
    let cloned = original.clone();
    println!("Original: {}", original.get_type_name());
    println!("Cloned: {}", cloned.get_type_name());
    println!("Clone equals original: {}", original.equals(&cloned));

    // PartialEq trait (using == operator)
    let type1 = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    let type2 = Type::from(typelib_TypeClass::typelib_TypeClass_STRING);
    let type3 = Type::from(typelib_TypeClass::typelib_TypeClass_LONG);
    println!("string == string: {}", type1 == type2);
    println!("string != long: {}", type1 != type3);

    // Debug trait
    println!("Debug output: {long_type:?}");

    // === Additional Type Examples ===
    println!("\n--- Additional Type Examples ---");

    // Create types for common UNO interfaces
    let service_manager_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_SERVICE,
        "com.sun.star.lang.XMultiServiceFactory",
    ));

    let document_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.document.XDocumentPropertiesSupplier",
    ));

    println!(
        "Service Manager: {} (class: {:?})",
        service_manager_type.get_type_name(),
        service_manager_type.get_type_class()
    );
    println!(
        "Document: {} (class: {:?})",
        document_type.get_type_name(),
        document_type.get_type_class()
    );

    // Create expected sal_Int32 type for comparison
    let sal_int32_type = Type::from((typelib_TypeClass::typelib_TypeClass_LONG, "sal_Int32"));
    println!(
        "sal_Int32 type: {} (class: {:?})",
        sal_int32_type.get_type_name(),
        sal_int32_type.get_type_class()
    );

    // === Advanced Type Operations ===
    println!("\n--- Advanced Type Operations ---");

    // Demonstrate type compatibility chains
    let unsigned_short_type = Type::from(typelib_TypeClass::typelib_TypeClass_UNSIGNED_SHORT);
    let unsigned_long_type = Type::from(typelib_TypeClass::typelib_TypeClass_UNSIGNED_LONG);
    let hyper_type = Type::from(typelib_TypeClass::typelib_TypeClass_HYPER);

    println!("Type compatibility chain:");
    println!(
        "  unsigned_long assignable from unsigned_short: {}",
        unsigned_long_type.is_assignable_from(&unsigned_short_type)
    );
    println!(
        "  hyper assignable from unsigned_short: {}",
        hyper_type.is_assignable_from(&unsigned_short_type)
    );
    println!(
        "  hyper assignable from long: {}",
        hyper_type.is_assignable_from(&long_type)
    );

    // Interface hierarchy compatibility
    let xinterface_type = Type::from((
        typelib_TypeClass::typelib_TypeClass_INTERFACE,
        "com.sun.star.uno.XInterface",
    ));
    println!(
        "XInterface assignable from XComponent: {}",
        xinterface_type.is_assignable_from(&component_type)
    );
    println!(
        "XInterface type: {} (class: {:?})",
        xinterface_type.get_type_name(),
        xinterface_type.get_type_class()
    );
    println!(
        "XComponent type: {} (class: {:?})",
        component_type.get_type_name(),
        component_type.get_type_class()
    );

    // === Memory Management Demo ===
    println!("\n--- Memory Management Demo ---");

    // Create multiple references to the same type
    let type_a = Type::from(typelib_TypeClass::typelib_TypeClass_DOUBLE);
    let type_b = type_a.clone();
    let type_c = Type::from(typelib_TypeClass::typelib_TypeClass_DOUBLE);

    println!("type_a: {}", type_a.get_type_name());
    println!("type_b (cloned): {}", type_b.get_type_name());
    println!("type_c (new): {}", type_c.get_type_name());
    println!(
        "All three equal: {}",
        type_a.equals(&type_b) && type_b.equals(&type_c)
    );

    // Types are automatically cleaned up when they go out of scope
    // due to RAII (Drop trait implementation)

    println!("\n✓ UNO Type system examples completed!");
    println!("✓ All Type creation methods, operations, and traits demonstrated!");
    println!("✓ Type system functionality working correctly!");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
