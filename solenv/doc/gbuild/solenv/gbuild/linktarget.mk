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

#include <types.h>

namespace gb
{
    using namespace types;

    class LinkTarget;
    class Library;
    class StaticLibrary;
    class SdiTarget;
    class Package;

    /// CObjects are never used standalone. They only exist as part of a 
    /// LinkTarget.
    class CObject : public HasSource, public HasDependencies, public Target
    {
        public:
            Path get_source();
        private:
            /// CObjects do not need to be explicitly constructed.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            CObject(String name);
            friend class LinkTarget;

            /// Platformdependent command to compile a plain C object.
            static const Command command(
                Path objectfile,
                String name,
                Path sourcefile,
                List<String> defs,
                List<String> cxxflags,
                List<Path> include);
            /// Platformdependent command to generate plain C object dependencies.
            static const Command command_dep(
                Path depfile,
                String name,
                Path sourcefile,
                List<String> defs,
                List<String> cxxflags,
                List<Path> include);
    };

    /// CxxObjects are never used standalone. They only exist as part of a 
    /// LinkTarget.
    class CxxObject : public HasSource, public HasDependencies, public Target
    {
        public:
            Path get_source();
        private:
            /// CxxObjects do not need to be explicitly constructed.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            CxxObject(String name);
            friend class LinkTarget;

            /// Platformdependent command to compile a C++ object.
            static const Command command(
                Path objectfile,
                String name,
                Path sourcefile,
                List<String> defs,
                List<String> cxxflags,
                List<Path> include);
            /// Platformdependent command to generate C++ object dependencies.
            static const Command command_dep(
                Path objectfile,
                String name,
                Path sourcefile,
                List<String> defs,
                List<String> cxxflags,
                List<Path> include);
    };

    class LinkTarget : public IsCleanable, public HasDependencies, public IsLinking, public DeliversHeaders, public HasCompileSettings, public Target
    {
        public:
            LinkTarget(String name);

        private:
            void get_external_headers_check();
            void add_internal_headers(const List<Target>& internal_headers);

            /// @warning Evil Hack: SELF is set to the name of the LinkTarget
            /// in the constructor. If SELF is not set to the LinkTarget name in
            /// the execution of the header rule, the LinkTarget is used (linked
            /// against) but was never defined. This might work out, if the
            /// LinkTarget has been provided by other means (for example:
            /// build.pl/dmake), but it should never happen in a project where
            /// all LinkTarget s are controlled by gbuild.
            LinkTarget& SELF;
            List<CObject> COBJECTS;
            List<CxxObject> CXXOBJECTS;
            List<Library> LINKED_LIBS;
            List<Path> AUXTARGETS;
            List<Path> INCLUDE;
            List<StaticLibrary> LINKED_STATIC_LIBS;
            List<String> CFLAGS;
            List<String> CXXFLAGS;
            List<String> DEFS;
            List<String> LDFLAGS;
            List<String> TARGETTYPE_FLAGS;
            Path DLLTARGET;

            /// Platformdependent command for linking.
            static const Command command (
                Path linktargetfile,
                String linktargetname,
                List<String> linkflags,
                List<Library> linked_libs,
                List<StaticLibrary> linked_static_libs,
                List<CObject> cobjects,
                List<CxxObject> cxxobjects);
            /// Command to collect all dependencies of this LinkTarget.
            static const Command command_dep(
                Path depfile,
                String linktargetname,
                List<CObject> cobjects,
                List<CxxObject> cxxobjects);
            static const List<String> DEFAULTDEFS;
            static const List<String> CXXFLAGS;
            static const List<String> LDFLAGS;
            static const List<Path> INCLUDE;
    };
}
/* vim: set filetype=cpp : */
