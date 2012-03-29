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
    class PackagePart
    {
        private:
            /// PackagePart s do not need to be explicitly constructed.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            PackagePart(String name, Path Source);
            friend Package;
            /// Returns a list of aboslute paths where files are copied to with the PackagePart class.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            static const List<Path> get_destinations();
    };

    class Package : public Target, public IsCleanable
    {
        public:
            /// Creates a new package that copies files from source_dir to the \$(OUTDIR).
            Package(String name, Path source_dir);
            /// Adds a file to the package. destination is the relative path in
            /// the \$(OUTDIR) and source is the relative path in the source_dir.
            add_file(Path destination, Path source);
    }
}
/* vim: set filetype=cpp : */
