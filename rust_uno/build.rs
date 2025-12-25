/* -*- Mode: rust; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//! Build script for rust_uno crate
//!
//! Links the Rust cdylib against the generated C++ bridge library (librust_uno-cpp.so)

fn main() {
    // Tell cargo to link against the rust_uno-cpp library
    // This library contains the generated C++ bridge functions
    if cfg!(windows) {
        println!("cargo:rustc-link-lib=irust_uno-cpp");
        // It requires linking against sal3.dll on Windows
        println!("cargo:rustc-link-lib=isal");
    } else {
        println!("cargo:rustc-link-lib=rust_uno-cpplo");
        // requires sal & co to link
        println!("cargo:rustc-link-lib=uno_cppu");
        println!("cargo:rustc-link-lib=uno_sal");
        println!("cargo:rustc-link-lib=uno_salhelpergcc3");

    }

    // Add the LibreOffice instdir/program directory to the library search path
    // This is where librust_uno-cpplo.so is located
    // the UREs dll's .libs/.so symlinks are under sdk/lib
    if let Ok(instdir) = std::env::var("INSTDIR") {
        println!("cargo:rustc-link-search=native={}/program", instdir);
        println!("cargo:rustc-link-search=native={}/sdk/lib", instdir);
    }

    // Also try the workdir path where the library might be during build
    if let Ok(workdir) = std::env::var("WORKDIR") {
        println!(
            "cargo:rustc-link-search=native={}/LinkTarget/Library",
            workdir
        );
    }

    // Fallback: try relative paths from the rust_uno directory
    println!("cargo:rustc-link-search=native=../instdir/program");
    println!("cargo:rustc-link-search=native=../instdir/sdk/lib");
    println!("cargo:rustc-link-search=native=../workdir/LinkTarget/Library");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
