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

    class SdiTarget : public Target, public IsCleanable
    {
        public:
            /// Creates a new SdiTarget. 
            SdiTarget(String name, String exports);
            /// Sets the include paths for this SdiTarget.
            set_include(List<Path> includes);
        private:
            /// The command to execute svidl.
            static const Command SVIDLCOMMAND;
            /// The target on with to depend to make sure the svidl executable is available.
            static const Path SVIDLTARGET;
            /// The target on with to depend to make sure the auxiliary files (libraries etc.) for the svidl executable are available.
            static const Path SVIDLAUXDEPS;
    };
}
/* vim: set filetype=cpp : */
