/// *************************************************************
///  
///  Licensed to the Apache Software Foundation (ASF) under one
///  or more contributor license agreements.  See the NOTICE file
///  distributed with this work for additional information
///  regarding copyright ownership.  The ASF licenses this file
///  to you under the Apache License, Version 2.0 (the
///  "License"); you may not use this file except in compliance
///  with the License.  You may obtain a copy of the License at
///  
///    http://www.apache.org/licenses/LICENSE-2.0
///  
///  Unless required by applicable law or agreed to in writing,
///  software distributed under the License is distributed on an
///  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
///  KIND, either express or implied.  See the License for the
///  specific language governing permissions and limitations
///  under the License.
///  
/// *************************************************************
#include <types.h>

namespace gb
{
    using namespace types;

    class LinkTarget;

    class StaticLibrary : public IsCleanable, public HasDependencies, public IsLinking, public DeliversHeaders, public HasCompileSettings, public Target
    {
        public:
            StaticLibrary(String name);
            /// Returns how the library is named in the filesystem on the current platform.
            /// StaticLibrary filename schemes are defined in solenv/gbuild/libnames.mk (with some platformdependant hacks in solenv/gbuild/platform/*.mk.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            Path get_filename();

        private:
            /// private helper function for the constructor
            void StaticLibrary_impl(LinkTarget library_linktarget);
            /// helper function to wrap LinkTargets functions (this is more or less pimpl ...).
            void forward_to_linktarget(Function f);

            /// List of buildable static libraries (i.e. static libraries that are not expected to exist outside of \$(OUTDIR) on the system).
            static const List<StaticLibrary> TARGETS;
            /// List of additional defines for compiling objects for static libraries
            static const List<String> DEFS;
            /// List of additional flags for linking a static library
            static const List<String> TARGETTYPEFLAGS;
            /// Mapping from symbolic static library names to filenames
            static const Map<StaticLibrary,Path> FILENAMES;
            /// location to place static libraries in the \$(OUTDIR)
            static const Path OUTDIRLOCATION;
            /// platformdependant file extension for static libraries
            static const String PLAINEXT;
    };
}
/* vim: set filetype=cpp : */
