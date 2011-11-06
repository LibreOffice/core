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



#ifndef _SV_JAVACHILD_HXX
#define _SV_JAVACHILD_HXX

#include <vcl/dllapi.h>
#include <vcl/syschild.hxx>

// -------------------
// - JavaChildWindow -
// -------------------

class VCL_DLLPUBLIC JavaChildWindow : public SystemChildWindow
{
public:

                    JavaChildWindow( Window* pParent, WinBits nStyle = 0 );
                    JavaChildWindow( Window* pParent, const ResId& rResId );
                   ~JavaChildWindow();

    sal_IntPtr      getParentWindowHandleForJava();

private:

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE      JavaChildWindow (const JavaChildWindow &);
    SAL_DLLPRIVATE      JavaChildWindow & operator= (const JavaChildWindow &);
};

#endif // _SV_JAVACHILD_HXX
