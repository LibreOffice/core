/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

use crate::{
    core::{
        OUString,
        uno_wrapper::{UnoError, defaultBootstrap_InitialComponentContext},
    },
    generated::rustmaker::com::sun::star::{
        frame::{Desktop::Desktop, XComponentLoader::XComponentLoader},
        text::{XSimpleText::XSimpleText, XTextDocument::XTextDocument, XTextRange::XTextRange},
    },
};

/// LibreOffice Writer automation demo using generated Rust UNO bindings.
/// Creates a Writer document and inserts text using auto-generated interface wrappers.
pub fn run() {
    println!("=== Rust UNO Bridge Test ===");
    println!("-> Running 'load_writer' example...");

    match run_internal() {
        Ok(()) => {
            println!("=== Rust UNO Bridge Test Done ===");
            println!("<- 'load_writer' example completed successfully.");
        }
        Err(e) => {
            println!("=== Rust UNO Bridge Test Done ===");
            eprintln!("   ERROR: 'load_writer' example failed: {e:?}");
        }
    }
}

fn run_internal() -> Result<(), UnoError> {
    // Initialize UNO bridge
    println!("   Initializing UNO bridge...");
    let context = defaultBootstrap_InitialComponentContext()?;
    println!("   ✓ UNO bridge initialized successfully!");

    // Create Desktop service
    println!("   Creating Desktop service using generated wrapper...");
    let desktop = Desktop::create(context.as_ptr()).ok_or(UnoError::OperationFailed)?;
    println!("   ✓ Desktop service created: {:p}", desktop.as_ptr());

    // Get XComponentLoader interface
    let component_loader =
        XComponentLoader::from_ptr(desktop.as_ptr()).ok_or(UnoError::InterfaceNotSupported)?;
    println!(
        "   ✓ XComponentLoader interface acquired: {:p}",
        component_loader.as_ptr()
    );

    // Load Writer document
    println!("   Loading Writer document via XComponentLoader::loadComponentFromURL...");
    let url = OUString::from("private:factory/swriter");
    let target = OUString::from("_blank");
    let search_flags: i32 = 0;
    let empty_args = std::ptr::null_mut();

    let document_interface_ptr = component_loader
        .loadComponentFromURL(
            url,    // Direct OUString - no casting needed!
            target, // Direct OUString - no casting needed!
            search_flags,
            empty_args,
        )
        .ok_or(UnoError::OperationFailed)?;

    if document_interface_ptr.as_ptr().is_null() {
        return Err(UnoError::OperationFailed);
    }

    println!("   ✓ Writer document loaded successfully!");
    println!(
        "   Document interface pointer: {:p}",
        document_interface_ptr.as_ptr()
    );

    // Cast to XTextDocument
    println!("   Casting document to XTextDocument...");
    let text_document = XTextDocument::from_ptr(document_interface_ptr.as_ptr())
        .ok_or(UnoError::InterfaceNotSupported)?;
    println!(
        "   ✓ XTextDocument interface acquired: {:p}",
        text_document.as_ptr()
    );

    // Get text object
    println!("   Getting text object via XTextDocument::getText...");
    let text = text_document.getText().ok_or(UnoError::OperationFailed)?;
    println!("   ✓ Text object acquired: {:p}", text.as_ptr());

    // Cast to XSimpleText
    println!("   Casting XText to XSimpleText...");
    let simple_text =
        XSimpleText::from_ptr(text.as_ptr()).ok_or(UnoError::InterfaceNotSupported)?;
    println!(
        "   ✓ XSimpleText interface acquired: {:p}",
        simple_text.as_ptr()
    );

    // Create text cursor
    println!("   Creating text cursor via XSimpleText::createTextCursor...");
    let cursor = simple_text
        .createTextCursor()
        .ok_or(UnoError::OperationFailed)?;
    println!("   ✓ Text cursor created: {:p}", cursor.as_ptr());

    // Insert text
    println!("   Inserting text via XSimpleText::insertString...");
    let hello_text = OUString::from(
        "Hello from Generated Rust UNO Bindings with Typed Parameters! \n\n\
         This text was inserted using auto-generated service and interface wrappers:\n\n\
         - Desktop::create() - Generated service wrapper\n\
         - XComponentLoader::loadComponentFromURL() - Generated interface method\n\
         - XTextDocument::getText() - Generated interface method\n\
         - XSimpleText::createTextCursor() - Generated interface method\n\
         - XSimpleText::insertString() - Generated interface method\n\n\
          NEW: Typed Parameter API Achievements:\n\
         - Boolean parameters now use native 'bool' instead of void* casting\n\
         - Integer parameters use direct 'i32' instead of pointer arithmetic\n\
         - Natural Rust syntax: 'absorb as u8' vs '*ptr as *mut c_void'\n\
         - Direction-aware: input by value, input/output by pointer\n\
         - Type-safe at compile time with better IDE support\n\n\
         FFI Architecture Features:\n\
         - Automatic null pointer checking in all generated methods\n\
         - Reference validity validation (is() checking) \n\
         - Memory cleanup on validation failure (delete + return nullptr)\n\
         - Detailed debug logging for troubleshooting\n\
         - Type-safe opaque pointer architecture prevents crashes\n\
         - Progressive migration: primitives typed, complex types remain compatible",
    );
    let absorb: bool = false;

    // Convert XTextCursor to XTextRange (interface inheritance)
    let text_range =
        XTextRange::from_ptr(cursor.as_ptr()).ok_or(UnoError::InterfaceNotSupported)?;

    simple_text.insertString(
        text_range,   // Direct typed interface - no void* casting needed!
        hello_text,   // Direct OUString - no casting needed!
        absorb as u8, //  Direct boolean parameter (natural Rust syntax!)
    );

    println!("   ✓ Text inserted successfully using generated interfaces!");

    Ok(())
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
