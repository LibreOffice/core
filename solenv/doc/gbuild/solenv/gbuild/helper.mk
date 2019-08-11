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

String NEWLINE;
String COMMA;

namespace gb
{
    using namespace types;

    class Helper
    {
        public:
            /// Abbreviates the common directory roots in a command
            static Command abbreviate_dirs(Command c);
            /// Announces the start/end of a task.
            static void announce(String announcement);
            /// Creates the default get_clean_target() function for the type.
            static void make_clean_target(TargetType t);
            /// Creates the default get_clean_target() function for the types.
            static void make_clean_targets(List<TargetType> ts);
            /// Creates the default get_dep_target() function for the type.
            static void make_dep_target(TargetType t);
            /// Creates the default get_dep_target() function for the types.
            static void make_dep_targets(List<TargetType> ts);

            /// The path to a zero length file.
            static const Path NULLFILE;
            /// The path to the misc directory for temporary files etc.
            static const Path MISC;
            /// The path to a phony target, that is always outdated.
            static const Path PHONY;
            /// Internally used dummy file. 
            static const Path MISCDUMMY;
    };
}
/* vim: set filetype=cpp : */
