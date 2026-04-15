//! Generated Rust bindings for LibreOffice UNO types
//!
//! This crate contains automatically generated Rust bindings
//! for LibreOffice UNO (Universal Network Objects) types.

// Core UNO functionality
pub mod core;
pub mod examples;
pub mod ffi;

// Auto-generated FFI bindings
pub mod generated;

/// Entry point function called by LibreOffice to test Rust UNO bindings
/// This function is called from desktop/source/app/app.cxx during LibreOffice startup
#[unsafe(no_mangle)]
pub extern "C" fn run_rust_uno_test() {
    println!("=== Rust UNO Bridge Test with Auto-Generated FFI ===");

    // Run the load_writer example to demonstrate service-style FFI functions
    examples::load_writer::run();

    println!("=== Rust UNO Bridge Test Done ===");
}
