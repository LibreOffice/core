/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Example usage of the Rust UNO bridge

pub mod any_example;
pub mod basic_example;
pub mod load_writer;
pub mod string_example;
pub mod type_example;
// pub mod autogen_test; // Temporarily disabled

/// Run all examples
pub fn run_all() {
    println!("=== Running all UNO examples ===");

    println!("\n--- Load Writer example ---");
    load_writer::run();

    println!("\n--- Basic example ---");
    basic_example::run_example();

    println!("\n--- String example ---");
    string_example::run_example();

    println!("\n--- Any example ---");
    any_example::run_example();

    println!("\n--- Type example ---");
    type_example::run_example();

    println!("\n=== All examples completed ===");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
