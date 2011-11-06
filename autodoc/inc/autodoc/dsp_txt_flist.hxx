/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef AUTODOC_DSP_TXT_FLIST_HXX
#define AUTODOC_DSP_TXT_FLIST_HXX

#include <iostream>


namespace ary
{
    namespace cpp
    {
        class DisplayGate;
    }
}


namespace autodoc
{

class TextDisplay_FunctionList_Ifc
{
  public:
    virtual             ~TextDisplay_FunctionList_Ifc() {}

    /** Displays the names of all C++ functions and methods within the
        given namespace (or the global namespace as default). All
        subnamespaces are included.
    */
    virtual void        Run(
                            ostream &           o_rStream,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate ) = 0; /// If i_pNamespace == 0, the global namespace is displayed.
};


}   // namespace autodoc

#endif

