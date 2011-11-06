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



#ifndef _SV_TIMER_HXX
#define _SV_TIMER_HXX

#include <tools/link.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

struct ImplTimerData;
struct ImplSVData;

// ---------
// - Timer -
// ---------

class VCL_DLLPUBLIC Timer
{
protected:
    ImplTimerData*  mpTimerData;
    sal_uLong           mnTimeout;
    sal_Bool            mbActive;
    sal_Bool            mbAuto;
    Link            maTimeoutHdl;

public:
                    Timer();
                    Timer( const Timer& rTimer );
    virtual         ~Timer();

    virtual void    Timeout();

    void            Start();
    void            Stop();

    void            SetTimeout( sal_uLong nTimeout );
    sal_uLong           GetTimeout() const { return mnTimeout; }
    sal_Bool            IsActive() const { return mbActive; }

    void            SetTimeoutHdl( const Link& rLink ) { maTimeoutHdl = rLink; }
    const Link&     GetTimeoutHdl() const { return maTimeoutHdl; }

    Timer&          operator=( const Timer& rTimer );

//    #ifdef _SOLAR__PRIVATE
    static void ImplDeInitTimer();
    static void ImplTimerCallbackProc();
//    #endif
};

// -------------
// - AutoTimer -
// -------------

class VCL_DLLPUBLIC AutoTimer : public Timer
{
public:
                    AutoTimer();
                    AutoTimer( const AutoTimer& rTimer );

    AutoTimer&      operator=( const AutoTimer& rTimer );
};

#endif // _SV_TIMER_HXX
