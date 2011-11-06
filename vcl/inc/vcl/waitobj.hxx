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



#ifndef _SV_WAITOBJ_HXX
#define _SV_WAITOBJ_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

// --------------
// - WaitObject -
// --------------

class VCL_DLLPUBLIC WaitObject
{
private:
    Window*     mpWindow;
public:
                WaitObject( Window* pWindow )
                {
                    mpWindow = pWindow;
                    if ( mpWindow )
                        mpWindow->EnterWait();
                }
                ~WaitObject();
};

#endif // _SV_WAITOBJ_HXX
