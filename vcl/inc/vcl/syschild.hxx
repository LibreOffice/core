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



#ifndef _SV_SYSCHILD_HXX
#define _SV_SYSCHILD_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>

struct SystemEnvData;
struct SystemWindowData;

// ---------------------
// - SystemChildWindow -
// ---------------------

class VCL_DLLPUBLIC SystemChildWindow : public Window
{
private:
    using Window::ImplInit;
    SAL_DLLPRIVATE void     ImplInitSysChild( Window* pParent, WinBits nStyle, SystemWindowData *pData, sal_Bool bShow = sal_False );
    SAL_DLLPRIVATE void     ImplTestJavaException( void* pEnv );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          SystemChildWindow (const SystemChildWindow &);
    SAL_DLLPRIVATE          SystemChildWindow & operator= (const SystemChildWindow &);

public:
    explicit                SystemChildWindow( Window* pParent, WinBits nStyle = 0 );
                            // create a SystemChildWindow using the given SystemWindowData
    explicit                SystemChildWindow( Window* pParent, WinBits nStyle, SystemWindowData* pData, sal_Bool bShow = sal_True );
    explicit                SystemChildWindow( Window* pParent, const ResId& );
    virtual                 ~SystemChildWindow();

    const SystemEnvData*    GetSystemData() const;

    //  per default systemchildwindows erase their background for better plugin support
    //  however, this might not always be required
    void                    EnableEraseBackground( sal_Bool bEnable = sal_True );
    sal_Bool                    IsEraseBackgroundEnabled();

    // return the platform specific handle/id of this window;
    // in case the flag bUseJava is set, a java compatible overlay window
    // is created on which other java windows can be created (plugin interface)
    sal_IntPtr              GetParentWindowHandle( sal_Bool bUseJava = sal_False );
};

#endif // _SV_SYSCHILD_HXX
