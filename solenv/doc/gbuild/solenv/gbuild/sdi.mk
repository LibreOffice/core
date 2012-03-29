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
