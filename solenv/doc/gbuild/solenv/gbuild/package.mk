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
    class PackagePart
    {
        private:
            /// PackagePart s do not need to be explicitly constructed.
            /// They are named after the path of their source file (without
            /// file extension) from the root of their source repository.
            PackagePart(String name, Path Source);
            friend Package;
            /// Returns a list of absolute paths where files are copied to with the PackagePart class.

            /// This function needs to be defined in solenv/gbuild/target_names.mk so that it is available to everywhere.
            static const List<Path> get_destinations();
    };

    class Package : public Target, public IsCleanable
    {
        public:
            /// Creates a new package that copies files from source_dir to the \$(INSTDIR).
            Package(String name, Path source_dir);
            /// Adds a file to the package. destination is the relative path in
            /// the \$(INSTROOT) and source is the relative path in the source_dir.
            add_file(Path destination, Path source);
    }
}
/* vim: set filetype=cpp : */
