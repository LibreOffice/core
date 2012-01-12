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



// -=-= includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef _SV_SALSYS_H
#define _SV_SALSYS_H

#include <vcl/sv.h>
#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#include <salsys.hxx>
#include <vclpluginapi.h>

#include <list>

class VCLPLUG_GEN_PUBLIC X11SalSystem : public SalSystem
{
public:
    X11SalSystem() {}
    virtual ~X11SalSystem();

    // overload pure virtual methods
    virtual unsigned int GetDisplayScreenCount();
    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetScreenName( unsigned int nScreen );
    virtual int ShowNativeDialog( const String& rTitle,
                                  const String& rMessage,
                                  const std::list< String >& rButtons,
                                  int nDefButton );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};

#endif // _SV_SALSYS_H
