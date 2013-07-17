/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

namespace gb {
    class CObject;
    class CxxObject;
    class Library;
    class StaticLibrary;
    class Package;
    class SdiTarget;
};

namespace gb { namespace types
{
    /// A type of target, with its set of rules etc., as used in the build system.
    class TargetType {};
    /// A plain old string.
    class String {};
    /// A partial, relative or absolute filesystem path.
    class Path {};
    /// A target to be build.
    class Target
    {
        public:
            /// The absolute filesystem path representing the target.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_target();
    };
    /// A partial or complete shell-command.
    class Command {};
    /// A integer number.
    class Integer {};
    /// A boolean value.

    /// There are const boolean values true and false for
    /// comparisons (written as $(true) and $(false) in makefiles.
    class Bool {};
    /// A language (for localisation)

    /// A language represented by its ISO 639-1:2002 code.
    class Language {};
    /// A List of objects.

    /// A List of objects represented by a whitespace separated list
    /// in makefiles.
    template <typename T>
        class List<T> {};

    /// A Mapping from with a key of type K and a value of type V

    /// A Mapping is represented by a whitespace separated list
    /// of key-value pairs. Key and value are separated by a colon (:).
    template <typename K, typename V>
        class Map<K,V> {};
    /// A target that can be cleaned.
    class IsCleanable
    {
        public:
            /// The (phony) absolute filesystem path to clean the target.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_clean_target();
    };
    /// A target that has generated dependencies.
    class HasDependencies
    {
        public:
            /// The absolute filesystem path to the file containing the dependencies.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_dep_target();
    };
    /// A target that has a source file from which it is generated.
    class HasSource
    {
        public:
            /// The absolute filesystem path to the source file.
            Path get_source();
    };
    /// A target that links objects and libraries.
    class IsLinking
    {
        public:
            /// Add a CObject to be compiled and linked.
            void add_cobject(CObject cobject);
            /// Add multiple CObject s to be compiled and linked.
            void add_cobjects(List<CObject> cobjects);
            /// Add a CxxObject to be compiled and linked.
            void add_cxxobject(CxxObject cobject);
            /// Add multiple CxxObject s to be compiled and linked.
            void add_cxxobjects(List<CxxObject> cobjects);
            /// Add multiple CxxObject s to be compiled and linked (with exceptions enabled).
            void add_exception_objects(List<CxxObject> cobject);
            /// Add libraries to link against dynamically.
            void add_linked_libs(List<Library> linked_libs);
            /// Add libraries to link against statically.
            void add_linked_static_libs(List<StaticLibrary> linked_static_libs);
            /// Set auxiliary files that are produced by linking (for cleanup and copying).
            void set_auxtargets(List<Path> auxtargets);
            /// Set the location for the produced DLL (used on Windows only).
            void set_dll_target(Path dlltarget);
            /// Set additional flags for the link command.
            void set_ldflags(List<Path> ldflags);
    };
    /// A target that delivers headers of some kind.
    class DeliversHeaders
    {
        public:
            /// The absolute filesystem path which is touched when all headers for this target are prepared.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_headers_target();
            /// The absolute filesystem path which is touched when all external headers for this target are prepared.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_external_headers_target();
            /// Add multiple Packages that need to be delivered/generated
            /// before compilation or dependency generation can start.
            void add_package_headers(List<Package> packages);
            /// Add multiple SdiTargets that need to be delivered/generated
            /// before compilation or dependency generation can start.
            void add_sdi_headers(List<SdiTarget> sdis);
    };
    /// A target where settings for the compilation can be set.
    class HasCompileSettings
    {
        public:
            /// Sets flags for plain C compilation.
            /// \$\$(CFLAGS) contains the current flags and can be used if
            /// just a few need to be modified.
            void set_cflags(List<String> cflags);
            /// Sets flags for C++ compilation.
            /// \$\$(CXXFLAGS) contains the current flags and can be used if
            /// just a few need to be modified.
            void set_cxxflags(List<String> cxxflags);
            /// Sets defines for C/C++ compilation.
            /// \$\$(DEFS) contains the current flags and can be used if
            /// just a few need to be modified.
            void set_defs(List<String> defs);
            /// Sets the include paths for C/C++ compilation.
            /// \$\$(INCLUDE) contains the current paths and can be used if
            /// just a few need to be modified.
            void set_include(List<Path> include);
    };
    /// A target using the resource compiler
    class UsesRsc
    {
        public:
            /// The command to execute rsc.
            static const Command RSCCOMMAND;
            /// The target on with to depend to make sure the rsc executable is available.
            static const Path RSCTARGET;
    };
    /// A target processing a set of srs files
    class HasSrs
    {
        public:
            /// Add a SrsTarget to be processed.
            void add_one_srs(SrsTarget srs);
            /// Add multiple SrsTarget s to be processed.
            void add_srs(List<SrsTarget> srs);
    };
}};
/* vim: set filetype=cpp : */
